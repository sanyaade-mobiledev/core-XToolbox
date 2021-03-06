/*
* This file is part of Wakanda software, licensed by 4D under
*  (i) the GNU General Public License version 3 (GNU GPL v3), or
*  (ii) the Affero General Public License version 3 (AGPL v3) or
*  (iii) a commercial license.
* This file remains the exclusive property of 4D and/or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
* such license does not include any other license or rights on this file,
* 4D's and/or its licensors' trademarks and/or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/
#include "VKernelPrecompiled.h"

#include "VIntlMgr.h"

#include "XLinuxIntlMgr.h"
#include "VMemory.h"
#include "XLinuxTextConverter.h"
#include "VString.h"



XLinuxIntlMgr::XLinuxIntlMgr( DialectCode inDialect)
{
	fLocale=icu::Locale::getDefault();	// TBD: use inDialect
}


XLinuxIntlMgr::XLinuxIntlMgr( const XLinuxIntlMgr& inOther)
{
	fLocale=icu::Locale::getDefault();	// TBD: clone inOther.fLocale
}


XLinuxIntlMgr::~XLinuxIntlMgr()
{
	// TBD: dispose fLocale
}


/*
	static
*/
VToUnicodeConverter* XLinuxIntlMgr::NewToUnicodeConverter(CharSet inCharSet)
{
	XLinuxToUnicodeConverter* converter=new XLinuxToUnicodeConverter(inCharSet);

	if(converter!=NULL && !converter->IsValid())
	{
		delete converter;
		converter=NULL;
	}

	return converter;
}

/*
	static
*/
VFromUnicodeConverter* XLinuxIntlMgr::NewFromUnicodeConverter(CharSet inCharSet)
{
	XLinuxFromUnicodeConverter* converter=new XLinuxFromUnicodeConverter(inCharSet);

	if(converter!=NULL && !converter->IsValid())
	{
		delete converter;
		converter=NULL;
	}

	return converter;
}


void XLinuxIntlMgr::Init(const icu::Locale& inLocale)
{
	fLocale=inLocale;
}


VString XLinuxIntlMgr::GetSymbol(Symbol inSymbolType) const
{
	UErrorCode err=U_ZERO_ERROR;

	icu::DecimalFormatSymbols decimalFmt(fLocale, err);

	//jmo - We see U_USING_DEFAULT_WARNING with ICU 4.8 ; I don't uderstand really why ; I don't like it !
	//		But according to ICU doc and to this bug report https://bugs.webkit.org/show_bug.cgi?id=27220
	//		(and various other sources !)... It seems it's not really an error/warning we should care about.
	xbox_assert(err==U_ZERO_ERROR || err==U_USING_FALLBACK_WARNING || U_USING_DEFAULT_WARNING);

	icu::DecimalFormatSymbols::ENumberFormatSymbol icuSymbol;

	switch(inSymbolType)
	{
	case DECIMAL :
		icuSymbol=icu::DecimalFormatSymbols::kDecimalSeparatorSymbol;
		break;

	case THOUSAND :
		//(ICU doc) The grouping separator is commonly used for thousands, but in some countries for ten-thousands.
		icuSymbol=icu::DecimalFormatSymbols::kGroupingSeparatorSymbol;
		break;

	case CURRENCY :
		icuSymbol=icu::DecimalFormatSymbols::kCurrencySymbol;
		break;

	default :
		xbox_assert(0);

		return VString("");
	}

	icu::UnicodeString symbolTmp=decimalFmt.getSymbol(icuSymbol);

	VString symb(symbolTmp.getTerminatedBuffer());

	return symb;
}


VString XLinuxIntlMgr::GetDecimalSeparator() const
{
	return GetSymbol(DECIMAL);
}


VString XLinuxIntlMgr::GetThousandSeparator() const
{
	return GetSymbol(THOUSAND);
}


VString XLinuxIntlMgr::GetCurrency() const
{
	return GetSymbol(CURRENCY);
}


// void XLinuxIntlMgr::GetMonthNames(EMonthDayList inListType, VArrayString* outArray) const
// {
//	if (outArray==NULL)
//		return;

//	UErrorCode err=U_ZERO_ERROR;

//	icu::DateFormatSymbols dateFmt(fLocale, err);
//	   xbox_assert(err==U_ZERO_ERROR || err==U_USING_FALLBACK_WARNING);

//	int32_t count=0;
//	UnicodeString* months=dateFmt.getMonths(&count, icu::FORMAT, (inListType==eMonthList) ? icu::WIDE : icu::ABBREVIATED);

//	for(int i=0 ; i<count ; i++)
//	{
//		VString month(months[i].getbuffer(), months[i].length());
//		outArray->AppendString(month);
//	}
// }


VString XLinuxIntlMgr::GetShortDatePattern() const
{
	return GetDateOrTimePattern(SHORT_DATE);
}


VString XLinuxIntlMgr::GetLongDatePattern() const
{
	return GetDateOrTimePattern(LONG_DATE);
}


VString XLinuxIntlMgr::GetShortTimePattern() const
{
	return GetDateOrTimePattern(SHORT_TIME);
}


VString XLinuxIntlMgr::GetMediumTimePattern() const
{
	return GetDateOrTimePattern(MEDIUM_TIME);
}


VString XLinuxIntlMgr::GetLongTimePattern() const
{
	return GetDateOrTimePattern(LONG_TIME);
}


VString XLinuxIntlMgr::GetDateOrTimePattern(Pattern inPatternType) const
{
	icu::DateFormat* dateOrTimeFmt=NULL;

	switch(inPatternType)
	{
	case SHORT_DATE :
	case AM_STRING :	//For AM and PM strings, any SimpleDateFormat should do the job.
	case PM_STRING :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::SHORT, fLocale);
		break;

	case MEDIUM_DATE :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::MEDIUM, fLocale);
		break;

	case LONG_DATE :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::LONG, fLocale);
		break;

	case SHORT_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::SHORT, fLocale);
		break;

	case MEDIUM_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::MEDIUM, fLocale);
		break;

	case LONG_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::LONG, fLocale);
		break;

	default :
		xbox_assert(0);
	}

	xbox_assert(dateOrTimeFmt!=NULL);


	VString pattern;

	if(dateOrTimeFmt!=NULL)
	{
		icu::SimpleDateFormat* simpleDateOrTimeFmt=reinterpret_cast<icu::SimpleDateFormat*>(dateOrTimeFmt);
		xbox_assert(simpleDateOrTimeFmt!=NULL);

		if(simpleDateOrTimeFmt!=NULL)
		{
			if(inPatternType==AM_STRING || inPatternType==PM_STRING)
			{
				//symbols is owned by simpleDateOrTimeFmt - Do not delete it manually !
				const icu::DateFormatSymbols* symbols=simpleDateOrTimeFmt->getDateFormatSymbols();
				xbox_assert(symbols!=NULL);

				if(symbols!=NULL)
				{
					sLONG count=0;
					const UnicodeString* amPmStringArray=symbols->getAmPmStrings(count);

					xbox_assert(count==2);

					if(count==2)
					{
						const UniChar* uniPtr=NULL;
						VSize uniPtrLen=0;

						if(inPatternType==AM_STRING)
							uniPtr=amPmStringArray[0].getBuffer(), uniPtrLen=amPmStringArray[0].length();
						else
							uniPtr=amPmStringArray[1].getBuffer(), uniPtrLen=amPmStringArray[1].length();

						pattern.FromBlock(uniPtr, uniPtrLen*sizeof(UniChar), VTC_UTF_16);
					}
				}
			}
			else
			{
				UErrorCode err=U_ZERO_ERROR;

				UnicodeString tmpPattern;
				simpleDateOrTimeFmt->toLocalizedPattern(tmpPattern, err);
				xbox_assert(err==U_ZERO_ERROR);

				pattern=tmpPattern.getTerminatedBuffer();
				xbox_assert(!pattern.IsEmpty());
			}
		}
	}

	if(dateOrTimeFmt!=NULL)
		delete dateOrTimeFmt;

	return pattern;
}


VString XLinuxIntlMgr::GetAMString() const
{
	return GetDateOrTimePattern(AM_STRING);
}


VString XLinuxIntlMgr::GetPMString() const
{
	return GetDateOrTimePattern(PM_STRING);
}


VString XLinuxIntlMgr::GetDateSeparator() const
{
	VString dateSeparator;

	icu::DateFormat* dateFmt=icu::DateFormat::createDateInstance(icu::DateFormat::SHORT, fLocale);
	xbox_assert(dateFmt!=NULL);

	icu::SimpleDateFormat* simpleDateFmt=reinterpret_cast<icu::SimpleDateFormat*>(dateFmt);
	xbox_assert(simpleDateFmt!=NULL);

	if(simpleDateFmt!=NULL)
	{
		UErrorCode err=U_ZERO_ERROR;

		UnicodeString tmpPattern;
		simpleDateFmt->toLocalizedPattern(tmpPattern, err);
		xbox_assert(err==U_ZERO_ERROR);

		VString datePattern(tmpPattern.getTerminatedBuffer());
		bool isQuoted=false;

		for(int i=0 ; i<datePattern.GetLength() ; i++)
		{
			UniChar c=datePattern[i];

			if(c=='\'')
				isQuoted=!isQuoted;

			if(isQuoted)
				continue;

			//ICU works with patterns ("M/d/yy" for ex.) and doesn't have a notion of date separator.
			//As a work around, we try to get a localized date pattern and pick the first char that looks like a separator.
			if(!(c>='A' && c<='Z') && !(c>='a' && c<='z'))
			{
				dateSeparator.AppendUniChar(c);
				break;
			}
		}
	}

	if(dateFmt!=NULL)
		delete dateFmt;

	xbox_assert(!dateSeparator.IsEmpty());

	if(dateSeparator.IsEmpty())
		return VString("/");

	return dateSeparator;
}


VString XLinuxIntlMgr::GetTimeSeparator() const
{
	VString timeSeparator;

	VString timePattern=GetDateOrTimePattern(SHORT_TIME);

	bool isQuoted=false;

	for(int i=0 ; i<timePattern.GetLength() ; i++)
	{
		UniChar c=timePattern[i];

		if(c=='\'')
			isQuoted=!isQuoted;

		if(isQuoted)
			continue;

		if(!(c>='A' && c<='Z') && !(c>='a' && c<='z'))
		{
			timeSeparator.AppendUniChar(c);
			break;
		}
	}

	if(timeSeparator.IsEmpty())
		return VString(":");

	return timeSeparator;
}


DateOrder XLinuxIntlMgr::GetDateOrder() const
{
	DateOrder order=DO_MONTH_DAY_YEAR;

	VString datePattern=GetDateOrTimePattern(SHORT_DATE);

	VString dateSeparator();
	bool	isQuoted=false;

	UniChar fmt[3];
	memset(fmt, 0, sizeof(fmt));

	bool dayFound	= false;
	bool monthFound = false;
	bool yearFound	= false;

	for(int i=0, p=0 ; i<datePattern.GetLength() && p<3 ; i++)
	{
		UniChar c=datePattern[i];

		if(c=='\'')
			isQuoted=!isQuoted;

		if(isQuoted)
			continue;

		if(!dayFound && (c=='d' || c=='D'))
			dayFound=true, fmt[p++]=c;

		else if(!monthFound && (c=='m' || c=='M'))
			monthFound=true, fmt[p++]=c;

		else if(!yearFound && (c=='y' || c=='Y'))
			yearFound=true, fmt[p++]=c;

	}


	xbox_assert(dayFound && monthFound && yearFound);


	switch(fmt[0])
	{
	case 'd' :
	case 'D' :
		order=(fmt[1]=='m' || fmt[1]=='M' ? DO_DAY_MONTH_YEAR : DO_DAY_YEAR_MONTH);
		break;

	case 'm' :
	case 'M' :
		order=(fmt[1]=='y' || fmt[1]=='Y' ? DO_MONTH_YEAR_DAY : DO_MONTH_DAY_YEAR);
		break;

	case 'y' :
	case 'Y' :
		order=(fmt[1]=='d' || fmt[1]=='D' ? DO_YEAR_DAY_MONTH : DO_YEAR_MONTH_DAY);
		break;
	}

	return order;
}


bool XLinuxIntlMgr::UseAmPm() const
{
	bool useAmPm=false;

	VString timePattern=GetDateOrTimePattern(SHORT_TIME);

	bool isQuoted=false;

	for(int i=0 ; i<timePattern.GetLength() ; i++)
	{
		UniChar c=timePattern[i];

		if(c=='\'')
			isQuoted=!isQuoted;

		if(isQuoted)
			continue;

		if(c=='a')
		{
			useAmPm=true;
			break;
		}
	}

	return useAmPm;
}


//static
icu::GregorianCalendar XLinuxIntlMgr::VTimeToGregorianCalendar(const VTime& inDate)
{
	//VTime is in GMT time zone
	icu::TimeZone* gmt=TimeZone::getGMT()->clone();

	UErrorCode err=U_ZERO_ERROR;
	GregorianCalendar cal(gmt, err);	//cal owns gmt and should destroy it
	xbox_assert(err==U_ZERO_ERROR);

	sWORD year, month, day, hour, minute, second, millisecond;
	inDate.GetUTCTime(year, month, day, hour, minute, second, millisecond);

	cal.set(year, month, day, hour, minute, second);

	return cal;
}


//static
bool XLinuxIntlMgr::EOSFormatsToPattern(EOSFormats inFmt, XLinuxIntlMgr::Pattern* outDateFmt, XLinuxIntlMgr::Pattern* outTimeFmt)
{
	bool rv=false;

	switch(inFmt)
	{
	case eOS_SHORT_FORMAT:
		if(outDateFmt!=NULL) *outDateFmt=SHORT_DATE;
		if(outTimeFmt!=NULL) *outTimeFmt=SHORT_TIME;
		rv=true;
		break;

	case eOS_MEDIUM_FORMAT:
		if(outDateFmt!=NULL) *outDateFmt=MEDIUM_DATE;
		if(outTimeFmt!=NULL) *outTimeFmt=MEDIUM_TIME;
		rv=true;
		break;

	case eOS_LONG_FORMAT:
		if(outDateFmt!=NULL) *outDateFmt=LONG_DATE;
		if(outTimeFmt!=NULL) *outTimeFmt=LONG_TIME;
		rv=true;
		break;

	default:
		xbox_assert(0);
		if(outDateFmt!=NULL) *outDateFmt=SHORT_DATE;
		if(outTimeFmt!=NULL) *outTimeFmt=SHORT_TIME;
		break;
	}

	return rv;
}


void XLinuxIntlMgr::FormatDateOrTime(const VTime& inDateOrTime, VString& outDateOrTime, Pattern inPatternType, bool inUseGMTTimeZoneForDisplay) const
{
	icu::DateFormat* dateOrTimeFmt=NULL;

	switch(inPatternType)
	{
	case SHORT_DATE :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::SHORT, fLocale);
		break;

	case MEDIUM_DATE :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::MEDIUM, fLocale);
		break;

	case LONG_DATE :
		dateOrTimeFmt=icu::DateFormat::createDateInstance(icu::DateFormat::LONG, fLocale);
		break;

	case SHORT_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::SHORT, fLocale);
		break;

	case MEDIUM_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::MEDIUM, fLocale);
		break;

	case LONG_TIME :
		dateOrTimeFmt=icu::DateFormat::createTimeInstance(icu::DateFormat::LONG, fLocale);
		break;

	default :
		xbox_assert(0);
	}

	xbox_assert(dateOrTimeFmt!=NULL);


	icu::GregorianCalendar cal=VTimeToGregorianCalendar(inDateOrTime);

	icu::UnicodeString tmpDateOrTime;

	if(dateOrTimeFmt!=NULL)
	{
		icu::TimeZone* tz=NULL;

		if(inUseGMTTimeZoneForDisplay)
			tz=TimeZone::createDefault();
		else
			tz=icu::TimeZone::getGMT()->clone();

		xbox_assert(tz!=NULL);

		dateOrTimeFmt->adoptTimeZone(tz);	//dateOrTimeFmt owns tz and should destroy it

		icu::FieldPosition fp;

		dateOrTimeFmt->format(cal, tmpDateOrTime, fp);

		delete dateOrTimeFmt;
	}

	outDateOrTime=VString(tmpDateOrTime.getTerminatedBuffer());
}


void XLinuxIntlMgr::FormatDate(const VTime& inDate, VString& outDate, EOSFormats inFormat, bool inUseGMTTimeZoneForDisplay) const
{
	Pattern fmt;

	bool res=EOSFormatsToPattern(inFormat, &fmt, NULL);

	FormatDateOrTime(inDate, outDate, fmt, inUseGMTTimeZoneForDisplay);
}


void XLinuxIntlMgr::FormatTime(const VTime& inTime, VString& outTime, EOSFormats inFormat, bool inUseGMTTimeZoneForDisplay) const
{
	Pattern fmt;

	bool res=EOSFormatsToPattern(inFormat, NULL, &fmt);

	FormatDateOrTime(inTime, outTime, fmt, inUseGMTTimeZoneForDisplay);
}


void XLinuxIntlMgr::FormatNumber(const VValueSingle& inValue, VString& outNumber) const
{
	double val=inValue.GetReal();	//(from mac code)

	UErrorCode err=U_ZERO_ERROR;
	NumberFormat* numberFmt=NumberFormat::createPercentInstance(fLocale, err);
	xbox_assert(err==U_ZERO_ERROR);

	if(numberFmt!=NULL)
	{
		icu::UnicodeString tmpNumber;
		numberFmt->format(val, tmpNumber);

		outNumber=VString(tmpNumber.getTerminatedBuffer());
	}
}


//If it doesn't format as expected, try to use time format, as Pierre suggests...
void XLinuxIntlMgr::FormatDuration(const VDuration& inDuration, VString& outDuration) const
{
	double val=inDuration.GetReal()/1000;	//We want sec, not ms

	UErrorCode err=U_ZERO_ERROR;
	RuleBasedNumberFormat durationFmt (URBNF_DURATION, fLocale, err);

	xbox_assert(err==U_ZERO_ERROR);

	icu::UnicodeString tmpDuration;
	durationFmt.format(val, tmpDuration);

	outDuration=VString(tmpDuration.getTerminatedBuffer());
}

// void XLinuxIntlMgr::FormatDuration(const VDuration& inTime, VString& outTime, EOSFormats inFormat) const
// {
//	   Pattern fmt;
//	   bool res=EOSFormatsToPattern(inFormat, NULL, &fmt);
//	   //(fix)FormatDateOrTime(inTime, outTime, fmt, inUseGMTTimeZoneForDisplay);
// }
