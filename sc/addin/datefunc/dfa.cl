/*------------------------------------------------------------------------

$Workfile:   DFA.CL  $

$Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sc/addin/datefunc/dfa.cl,v 1.1.1.1 2000-09-18 16:44:46 hr Exp $

Description:	StarCalc Datefunc AddIn Example

(c) Copyright 1998 - 2000, Sun Microsystems, Inc.

------------------------------------------------------------------------*/

static char datefunc_Id[]="@(#) StarCalc Datefunc AddIn (c) 1998-2000 Sun Microsystems, Inc.";

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <xlang.h>
#include <addin.h>
#include <dfa.hrc>


/**
 * the current language the Addin is using
 */
static USHORT _nLanguage=LANGUAGE_ENGLISH;

/**
 * StarCalc calls this function to set a new current Language for the Addin
 *
 * @param *nLanguage
 *
 */
void CALLTYPE SetLanguage( USHORT* nLanguage )
{
	_nLanguage = GetNeutralLanguage( *nLanguage );
}


/**
 * Null Date, initialized in GetFunctionCount
 *
 * StarCalc uses a BaseDate 12/30/1899
 * If not specified otherwise in the Settings for the Spreedsheet Document.
 *
 * There's no way to get the Spreadsheet settings from whithin a simple addin,
 * so this Addin can only be used by documents using the default BaseDate setting.
 *
 * The functions in this Addin use a BaseDate 01/01/0001
 * The nNullDate Variable is the StarCalc BaseDate converted to
 * this internal date representation.
 *
 * @see #GetFunctionCount
 *
 */

static ULONG nNullDate=0;

#define NULLDATE_Year  1899
#define NULLDATE_Month 12
#define NULLDATE_Day   30


/**
 * Array holding values for month length, used in DaysInMonth() function
 *
 * @see #DaysInMonth
 *
 */
static USHORT aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

/**
 * Check if a year is a leap year in the Gregorian calendar
 *
 * @param nYear the year which should be checked
 * @return true if the year is a leap year, false otherwise.
 *
 * @see #DaysInMonth, #IsLeapYear,
 * @see #ScDate_DaysInMonth, #ScDate_IsLeapYear, #ScDate_WeeksInYear
 *
 */
static BOOL IsLeapYear( USHORT nYear )
{
    return (((nYear % 4) == 0) && ((nYear % 100) != 0) || ((nYear % 400) == 0));
}


/**
 * Get the number of days in a specified month
 *
 * @param nMonth the number of the Month
 * @param nYear the year
 * @return number of days
 *
 */
static USHORT DaysInMonth( USHORT nMonth, USHORT nYear )
{
    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if ( IsLeapYear(nYear) )
            return aDaysInMonth[nMonth-1] + 1;
        else
            return aDaysInMonth[nMonth-1];
    }
}


/**
 * Convert a date to a count of days starting from 01/01/0001
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts a Day , Month, Year representation
 * to this internal Date value.
 *
 * @param nDay the day of the Month
 * @param nMonth the number of the Month
 * @param nYear the Year
 * @return count of days from 01/01/0001 to the date specified
 *
 */
static long DateToDays( USHORT nDay, USHORT nMonth, USHORT nYear )
{
    long nDays;
    USHORT i;

    nDays = ((ULONG)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);

    for( i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;

    return nDays;
}


/**
 * Convert a count of days starting from 01/01/0001 to a date
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts this internal Date value
 * to a Day , Month, Year representation of a Date.
 *
 * @param nDay count of days from 01/01/0001
 * @param *pDay pointer to a variable for the day of the month
 * @param *pMonth pointer to a variable for the month
 * @param *pYear pointer to a variable for the year
 *
 */
static void DaysToDate( long nDays,
                        USHORT *pDay, USHORT *pMonth, USHORT *pYear )
{
    long    nTempDays;
    long    i = 0;
    BOOL    bCalc;

    do
    {
        nTempDays = (long)nDays;
        *pYear = (USHORT)((nTempDays / 365) - i);
        nTempDays -= ((ULONG) *pYear -1) * 365;
        nTempDays -= (( *pYear -1) / 4) - (( *pYear -1) / 100) + ((*pYear -1) / 400);
        bCalc = FALSE;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = TRUE;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !IsLeapYear( *pYear ) )
                {
                    i--;
                    bCalc = TRUE;
                }
            }
        }
    }
    while ( bCalc );

    *pMonth = 1;
    while ( (ULONG)nTempDays > DaysInMonth( *pMonth, *pYear ) )
    {
        nTempDays -= DaysInMonth( *pMonth, *pYear );
        (*pMonth)++;
    }
    *pDay = (USHORT)nTempDays;
}

/**
 * Get week difference between 2 dates
 *
 * new Weeks(date1,date2,mode) function for StarCalc
 *
 * Two modes of operation are provided.
 * The first is just a simple division by 7 calculation.
 *
 * The second calculates the diffence by week of year.
 *
 * The International Standard IS-8601 has decreed that Monday
 * shall be the first day of the week.
 *
 * A week that lies partly in one year and partly in annother
 * is assigned a number in the the year in which most of its days lie.
 *
 * That means that week 1 of any year is the week that contains the 4. January
 *
 * The internal representation of a Date used in the Addin is the number of days based on 01/01/0001
 *
 * A WeekDay can be then calculated by substracting 1 and calculating the rest of
 * a division by 7, which gives a 0 - 6 value for Monday - Sunday
 *
 * Using the 4. January rule explained above the formula
 *
 *	nWeek1= ( nDays1 - nJan4 + ( (nJan4-1) % 7 ) ) / 7 + 1;
 *
 * calculates a number between 0-53 for each day which is in the same year as nJan4
 * where 0 means that this week belonged to the year before.
 *
 * If a day in the same or annother year is used in this formula this calculates
 * an calendar week offset from a given 4. January
 *
 *	nWeek2 = ( nDays2 - nJan4 + ( (nJan4-1) % 7 ) ) / 7 + 1;
 *
 * The 4.January of first Date Argument can thus be used to calculate
 * the week difference by calendar weeks which is then nWeek = nWeek2 - nWeek1
 *
 * which can be optimized to
 *
 * nWeek = ( (nDays2-nJan4+((nJan4-1)%7))/7 ) - ( (nDays1-nJan4+((nJan4-1)%7))/7 )
 *
 * Note: All calculations are operating on the long integer data type
 * % is the modulo operator in C which calculates the rest of an Integer division
 *
 *
 * @param *r - return value for the StarCalc function
 * @param d1 - date value (in StarCalc representation based 12/30/1899), usually the older date
 * @param d2 - date value (in StarCalc representation based 12/30/1899), usually the younger date
 * @param dMode - mode of operation
 *
 * mode 0 is the interval between the dates in month, that is days / 7
 *
 * mode 1 is the difference by week of year
 *
 */
void CALLTYPE ScDate_GetDiffWeeks(double *r, double *d1, double *d2, double *dMode)
{
  long nDays1=0;
  long nDays2=0;
  int nMode=0;

  if ( d1 ) nDays1=(long)floor(*d1)+nNullDate;
  if ( d2 ) nDays2=(long)floor(*d2)+nNullDate;


  if ( dMode) nMode=(int)*dMode;

  if ( nMode == 1 ) {

	USHORT nDay,nMonth,nYear;
	long nJan4;

	DaysToDate(nDays1,&nDay,&nMonth,&nYear);
	nJan4=DateToDays(4,1,nYear);

	*r=(double) ( ( (nDays2-nJan4+((nJan4-1)%7))/7 ) - ( (nDays1-nJan4+((nJan4-1)%7))/7 ) );

  } else {

	*r= (double) ( (nDays2 - nDays1) / 7 ) ;
  }

}

/**
 * Get month difference between 2 dates
 * =Month(start, end, mode) Function for StarCalc
 *
 * two modes are provided
 *
 * @param *r - return value for the StarCalc function
 * @param d1 - date value, start date
 * @param d2 - date value, end date
 * @param dMode - mode of operation
 *
 * mode 0 is the interval between the dates in month
 *
 * mode 1 is the difference in calendar month
 *
 */
void CALLTYPE ScDate_GetDiffMonths(double *r, double *d1, double *d2, double *dMode)
{
  USHORT nDay1,nMonth1,nYear1;
  USHORT nDay2,nMonth2,nYear2;
  long nDays1=0;
  long nDays2=0;
  int nMode=0;

  if ( dMode) nMode=(int)*dMode;

  if ( d1 ) nDays1=(long)floor(*d1)+nNullDate;
  if ( d2 ) nDays2=(long)floor(*d2)+nNullDate;

  DaysToDate(nDays1,&nDay1,&nMonth1,&nYear1);
  DaysToDate(nDays2,&nDay2,&nMonth2,&nYear2);

  *r=(double) ( nMonth2 - nMonth1 + (nYear2 - nYear1) * 12 );
  if ( nMode == 1 || nDays1 == nDays2 ) return;

  if ( nDays1 < nDays2 ) {
	if ( nDay1 > nDay2 ) {
		*r -= 1;
	}
  } else {
	if ( nDay1 < nDay2 ) {
		*r += 1;
	}
  }

}


/**
 * Get Year difference between 2 dates
 *
 * two modes are provided
 *
 * @param *r - return value for the StarCalc function
 * @param d1 - date value, start date
 * @param d2 - date value, end date
 * @param dMode - mode of operation
 *
 * mode 0 is the interval between the dates in years
 *
 * mode 1 is the difference in calendar years
 *
 */
void CALLTYPE ScDate_GetDiffYears(double *r, double *d1, double *d2, double *dMode)
{
  USHORT nDay1,nMonth1,nYear1;
  USHORT nDay2,nMonth2,nYear2;
  long nDays1=0;
  long nDays2=0;
  int nMode=0;

  if ( dMode) nMode=(int)*dMode;

  if ( d1 ) nDays1=(long)floor(*d1)+nNullDate;
  if ( d2 ) nDays2=(long)floor(*d2)+nNullDate;

  DaysToDate(nDays1,&nDay1,&nMonth1,&nYear1);
  DaysToDate(nDays2,&nDay2,&nMonth2,&nYear2);
  if ( nMode != 1 ) {
	ScDate_GetDiffMonths(r,d1,d2,dMode);
	*r= (double) ( ((int) *r) / 12 );
  } else {
	  *r=(double) ( nYear2 - nYear1 );
  }
}

/**
 * Check if a Date is in a leap year in the Gregorian calendar
 *
 * @param *r - return value for the StarCalc function
 * @param d - date value (in StarCalc representation based 12/30/1899)
 *
 */
void CALLTYPE ScDate_IsLeapYear(double *r, double *d)
{
  ULONG nDays;
  USHORT nDay, nMonth, nYear;
  double v=0.0;

  if ( d ) v=*d;
  nDays=(int) v + nNullDate;

  DaysToDate(nDays,&nDay,&nMonth,&nYear);

  *r=(double) ( IsLeapYear(nYear) );

}

/**
 * Get the Number of Days in the month for a date
 *
 * @param *r - return value for the StarCalc function
 * @param d - date value (in StarCalc representation based 12/30/1899)
 *
 */
void CALLTYPE ScDate_DaysInMonth(double *r, double *d)
{
  ULONG nDays;
  USHORT nDay, nMonth, nYear;
  double v=0.0;

  if ( d ) v=*d;
  nDays=(int) v + nNullDate;

  DaysToDate(nDays,&nDay,&nMonth,&nYear);
  *r=(double) ( DaysInMonth( nMonth, nYear) );

}


/**
 * Get number of weeks in the year for a date
 *
 * Most years have 52 weeks, but years that start on a Thursday
 * and leep years that start on a Wednesday have 53 weeks
 *
 * The International Standard IS-8601 has decreed that Monday
 * shall be the first day of the week.
 *
 * A WeekDay can be calculated by substracting 1 and calculating the rest of
 * a division by 7 from the internal date represention
 * which gives a 0 - 6 value for Monday - Sunday
 *
 * @param *r - return value for the StarCalc function
 * @param d - date value (in StarCalc represantaion based 30.12.1899)
 *
 * @see #IsLeapYear #WeekNumber
 *
 */
void CALLTYPE ScDate_WeeksInYear(double *r, double *d)
{
  ULONG nDays;
  USHORT nDay, nMonth, nYear;
  double v=0.0;
  long nJan1WeekDay;

  if ( d ) v=*d;
  nDays=(int) v + nNullDate;

  DaysToDate(nDays,&nDay,&nMonth,&nYear);

  nJan1WeekDay= ( DateToDays(1,1,nYear) - 1) % 7;

  if ( nJan1WeekDay == 3 ) { /* Thursday */
	*r=(double) 53;
	return;
  } else if ( nJan1WeekDay == 2 ) { /* Wednesday */
	*r= (double) ( IsLeapYear(nYear) ? 53 : 52 );
  } else {
	*r= (double) 52;
  }
}


/**
 * Get number of days in the year of a date specified
 *
 * @param *r - return value for the StarCalc function
 * @param d - date value (in StarCalc represantaion based 30.12.1899)
 *
 */
void CALLTYPE ScDate_DaysInYear(double *r, double *d)
{
  ULONG nDays;
  USHORT nDay, nMonth, nYear;
  double v=0.0;

  if ( d ) v=*d;
  nDays=(int) v + nNullDate;

  DaysToDate(nDays,&nDay,&nMonth,&nYear);
  *r=(double) ( IsLeapYear(nYear) ? 366 : 365 );

}


/**
 * Tell StarCalc how many new functions this Addin provides.
 *
 * It's called before any of these new functions is actually
 * executed and is also used to initialize the NullDate here.
 *
 * StarCalc uses a Date Base 12/30/1899
 * If not specified otherwise in the Options for the Spreedsheet Document
 *
 *
 * @param *nCount - returns the number of functions which are exported to StarCalc
 *
 */
void CALLTYPE GetFunctionCount( USHORT *nCount )
{

  /* initialize nNullDate Value 0 is 12/30/1899 */
  nNullDate=DateToDays(NULLDATE_Day, NULLDATE_Month, NULLDATE_Year);

  *nCount = 7;
}

/**
 * Provides neccessary data for each new function to StarCalc
 *
 * @param *nNo Input: Function number between 0 and nCount - 1
 * @param *pFuncName Output: Functionname which should be called in the AddIn-DLL
 * @param *nParamCount Output: Number of Parameter. Must be greater than 0, because there's always a return-Value. Maximum is 16.
 * @param *peType Output: Pointer to arrray with exactly 16 variables of typ Paramtype. nParamCount Entries are set to the type of the corresponding Parameters.
 * @param *pInternalName Output: Functionname as seen by the Spreadsheet user
 *
 * @see #GetFunctionCount, #GetParameterDescription
 *
 */
void CALLTYPE GetFunctionData( USHORT *    nNo,
                               char *      pFuncName,
                               USHORT *    nParamCount,
                               ParamType * peType,
                               char *      pInternalName )
{


	 switch( *nNo ) {
	 case 0:
	 SO_StringCopy( pInternalName, getText(DFA_WEEK_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_GetDiffWeeks" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 peType[2] = PTR_DOUBLE;
	 peType[3] = PTR_DOUBLE;
	 *nParamCount=4;
	 break;

	 case 1:
	 SO_StringCopy( pInternalName, getText(DFA_MONTHS_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_GetDiffMonths" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 peType[2] = PTR_DOUBLE;
	 peType[3] = PTR_DOUBLE;
	 *nParamCount=4;
	 break;

	 case 2:
	 SO_StringCopy( pInternalName, getText(DFA_YEARS_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_GetDiffYears" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 peType[2] = PTR_DOUBLE;
	 peType[3] = PTR_DOUBLE;
	 *nParamCount=4;
	 break;

	 case 3:
	 SO_StringCopy( pInternalName, getText(DFA_ISLEAPYEAR_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_IsLeapYear" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 *nParamCount=2;
	 break;

	 case 4:
	 SO_StringCopy( pInternalName, getText(DFA_DAYSINMONTH_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_DaysInMonth" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 *nParamCount=2;
	 break;

	 case 5:
	 SO_StringCopy( pInternalName, getText(DFA_DAYSINYEAR_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_DaysInYear" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 *nParamCount=2;
	 break;

	 case 6:
	 SO_StringCopy( pInternalName, getText(DFA_WEEKSINYEAR_NAME) );
	 SO_StringCopy( pFuncName,     "ScDate_WeeksInYear" );
	 peType[0] = PTR_DOUBLE;
	 peType[1] = PTR_DOUBLE;
	 *nParamCount=2;
	 break;

	 default:
            *nParamCount    = 0;
            *pFuncName     = 0;
            *pInternalName = 0;
            break;
    }
}

/**
 * Provides descriptions for each new function to StarCalc
 * which are shown is the autopilot
 *
 * @param *nNo Input Parameter, Function number between 0 and nCount - 1
 * @param *nParam Parameter Number
 * @param *pName Output: Name of the parameter
 * @param *pDesc Output: Description of the parameter
 *
 * @see #GetFunctionCount, #GetParameterDescription
 */
void CALLTYPE GetParameterDescription( USHORT* nNo, USHORT* nParam,
char* pName, char* pDesc )
{
	*pName = 0;
	*pDesc = 0;

	switch ( *nNo ) {
	case 0:	/* Weeks */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_WEEK_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE1_NAME));
			SO_StringCopy(pDesc,getText(DFA_WEEK_PAR1_DESC));
			break;
		case 2:
			SO_StringCopy(pName,getText(DFA_PAR_DATE2_NAME));
			SO_StringCopy(pDesc,getText(DFA_WEEK_PAR2_DESC));
			break;
		case 3:
			SO_StringCopy(pName,getText(DFA_PAR_MODE_NAME));
			SO_StringCopy(pDesc,getText(DFA_WEEK_PAR3_DESC));
			break;
		}
		break;
	case 1: /* Months */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_MONTHS_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE1_NAME));
			SO_StringCopy(pDesc,getText(DFA_MONTHS_PAR1_DESC));
			break;
		case 2:
			SO_StringCopy(pName,getText(DFA_PAR_DATE2_NAME));
			SO_StringCopy(pDesc,getText(DFA_MONTHS_PAR2_DESC));
			break;
		case 3:
			SO_StringCopy(pName,getText(DFA_PAR_MODE_NAME));
			SO_StringCopy(pDesc,getText(DFA_MONTHS_PAR3_DESC));
			break;
		}
		break;
	case 2: /* Years */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_YEARS_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE1_NAME));
			SO_StringCopy(pDesc,getText(DFA_YEARS_PAR1_DESC));
			break;
		case 2:
			SO_StringCopy(pName,getText(DFA_PAR_DATE2_NAME));
			SO_StringCopy(pDesc,getText(DFA_YEARS_PAR2_DESC));
			break;
		case 3:
			SO_StringCopy(pName,getText(DFA_PAR_MODE_NAME));
			SO_StringCopy(pDesc,getText(DFA_YEARS_PAR3_DESC));
			break;
		}
	   break;
	case 3:	/* IsLeapYear */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_ISLEAPYEAR_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE_NAME));
			SO_StringCopy(pDesc,getText(DFA_PAR_DATE_DESC)); /* StarCalc Value */
			break;
		}
		break;
	case 4:	/* DaysInMonth */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_DAYSINMONTH_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE_NAME));
			SO_StringCopy(pDesc,getText(DFA_PAR_DATE_DESC)); /* StarCalc Value */
			break;
		}
		break;
	case 5:	/* DaysInYear */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_DAYSINYEAR_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE_NAME));
			SO_StringCopy(pDesc,getText(DFA_PAR_DATE_DESC)); /* StarCalc Value */
			break;
		}
		break;

	case 6:	/* WeeksInYear */
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(DFA_WEEKSINYEAR_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(DFA_PAR_DATE_NAME));
			SO_StringCopy(pDesc,getText(DFA_PAR_DATE_DESC)); /* StarCalc Value */
			break;
		}
		break;
	}

}

/*------------------------------------------------------------------------

$Log: not supported by cvs2svn $
Revision 1.6  1999/08/10 12:47:12  NN
#68039# copyright

   
      Rev 1.5   10 Aug 1999 14:47:12   NN
   #68039# copyright
   
      Rev 1.4   20 May 1999 12:59:40   ER
   #66274# GetFunctionData: DFA_WEEKSINYEAR_NAME statt DFA_DAYSINYEAR_NAME

      Rev 1.3   29 Apr 1999 14:55:50   ER
   #57689# xlang.h statt tools/lang.hxx

      Rev 1.3   29 Apr 1999 14:45:32   ER
   #57689# xlang.hxx statt tools/lang.hxx

      Rev 1.2   29 Apr 1999 12:54:04   ER
   #57689# Internationalisierung

      Rev 1.1   15 Mar 1999 16:27:50   HJS
   aufgeraeumt

      Rev 1.0   10 Mar 1999 12:47:04   BEI
   Initial revision.

      Rev 1.1   13 Jul 1998 18:15:40   BEI
   2 Languages

      Rev 1.0   07 Jul 1998 20:36:22   NN
   Initial revision.

------------------------------------------------------------------------*/


