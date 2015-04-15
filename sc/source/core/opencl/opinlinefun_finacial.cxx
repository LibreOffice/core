/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OPINLINFUN_finacial
#define SC_OPENCL_OPINLINFUN_finacial
std::string approxEqualDecl="bool approxEqual(double a, double b);\n";
std::string approxEqual =
"bool approxEqual(double a, double b)\n"
"{\n"
"     if (a == b)\n"
"        return true;\n"
"     double x = a - b;\n"
"     return (x < 0.0 ? -x : x) < ((a < 0.0 ? -a : a) * (1.0 / (16777216.0 *"
"16777216.0)));\n"
"}\n";

std::string nKorrValDecl ="double constant nKorrVal[]"
"= {0, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, "
"9e-8,9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14, 9e-15};\n";

std::string SCdEpsilonDecl =
"constant double SCdEpsilon = 1.0E-7;\n";

std::string RoundDecl = "double  Round(double fValue);\n";

std::string Round =
"double  Round(double fValue)\n"
"{\n"
"    if ( fValue == 0.0  )\n"
"        return fValue;\n"
"\n"
"    double fFac = 0;\n"
"    int nExp;\n"
"    if ( fValue > 0.0 )\n"
"        nExp = ( floor( log10( fValue ) ) );\n"
"    else\n"
"        nExp = 0;\n"
"    int nIndex = 15 - nExp;\n"
"    if ( nIndex > 15 )\n"
"        nIndex = 15;\n"
"    else if ( nIndex <= 1 )\n"
"        nIndex = 0;\n"
"    fValue = floor( fValue + 0.5 + nKorrVal[nIndex] );\n"
"    return fValue;\n"
"}\n";

std::string GetRmzDecl =
"double GetRmz( double fZins, double fZzr, double fBw, double fZw,int nF );\n";

std::string GetRmz=
"double GetRmz( double fZins, double fZzr, double fBw, double fZw, int nF )\n"
"{\n"
"    double      fRmz;\n"
"    if( fZins == 0.0 )\n"
"        fRmz = ( fBw + fZw ) / fZzr;\n"
"    else\n"
"    {\n"
"        double  fTerm = pow( 1.0 + fZins, fZzr );\n"
"        if( nF > 0 )\n"
"            fRmz = ( fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins / ( 1.0 - 1."
"0 / fTerm ) ) / ( 1.0 + fZins );\n"
"        else\n"
"            fRmz = fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins /( 1.0 - 1.0 "
"/ fTerm );\n"
"    }\n"
"    return -fRmz;\n"
"}\n";

std::string GetRmz_newDecl =
"double GetRmz_new( double fZins, double fZzr, double fBw, double fZw,"
"int nF );\n";
std::string GetRmz_new=
"double GetRmz_new( double fZins, double fZzr, double fBw, double fZw,"
"int nF)\n"
"{\n"
"    double fRmz;\n"
"        double  fTerm = pow( 1.0 + fZins, fZzr );\n"
"        if( nF > 0 )\n"
"            fRmz = ( fZw * fZins *pow ( fTerm - 1.0,-1 ) + fBw * fZins *pow( "
"( 1.0 - pow( fTerm,-1) ),-1) )* pow ( 1.0 + fZins,-1 );\n"
"        else\n"
"            fRmz = fZw * fZins *pow ( fTerm - 1.0 ,-1) + fBw * fZins *pow( "
"1.0 - pow( fTerm,-1),-1 );\n"
"    return -fRmz;\n"
"}\n";
std::string GetZwDecl =
"double GetZw( double fZins, double fZzr, double fRmz,"
"double fBw, int nF );\n";

std::string GetZw =
"double GetZw( double fZins, double fZzr, double fRmz,"
"double fBw, int nF )\n"
"{\n"
"    double      fZw;\n"
"    if( fZins == 0.0 )\n"
"        fZw = fBw + fRmz * fZzr;\n"
"    else\n"
"    {\n"
"        double  fTerm = pow( 1.0 + fZins, fZzr );\n"
"        if( nF > 0 )\n"
"                fZw = fBw * fTerm + fRmz * ( 1.0 + fZins ) *( fTerm - 1.0 ) "
"/ fZins;\n"
"        else\n"
"                fZw = fBw * fTerm + fRmz * ( fTerm - 1.0 ) / fZins;\n"
"    }\n"
"    return -fZw;\n"
"}\n";

std::string GetZw_newDecl =
"double GetZw_new( double fZins, double fZzr, double fRmz,"
"double fBw, int nF );\n";

std::string GetZw_new =
"double GetZw_new( double fZins, double fZzr, double fRmz,"
"double fBw, int nF )\n"
"{\n"
"    double fZw;\n"
"    double  fTerm = pow( 1.0 + fZins, fZzr );\n"
"    if( nF > 0 )\n"
"        fZw = fBw * fTerm + fRmz * ( 1.0 + fZins ) *( fTerm - 1.0 ) "
"*pow( fZins,-1);\n"
"    else\n"
"        fZw = fBw * fTerm + fRmz * ( fTerm - 1.0 ) *pow( fZins,-1);\n"
"    return -fZw;\n"
"}\n";

std::string IsLeapYearDecl =
"bool IsLeapYear( int n );\n";

std::string IsLeapYear =
"bool IsLeapYear( int n )\n"
"{\n"
"    return ( (( ( n % 4 ) == 0 ) && ( ( n % 100 ) != 0)) || ( ( n % 400 ) == "
"0 ) );\n"
"}\n";

std::string DaysInMonthDecl=
"int DaysInMonth( int nMonth, int nYear );\n";

std::string DaysInMonth =
"int DaysInMonth( int nMonth, int nYear )\n"
"{\n"
"    int aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,\n"
"                31, 31, 30, 31, 30, 31 };\n"
"\n"
"    if ( nMonth != 2 )\n"
"        return aDaysInMonth[nMonth-1];\n"
"    else\n"
"    {\n"
"        if ( IsLeapYear(nYear) )\n"
"            return aDaysInMonth[nMonth-1] + 1;\n"
"        else\n"
"            return aDaysInMonth[nMonth-1];\n"
"    }\n"
"}\n";
std::string DaysInMonth_newDecl=
"int DaysInMonth( int nMonth, int nYear );\n";

std::string DaysInMonth_new =
"int DaysInMonth( int nMonth, int nYear )\n"
"{\n"
"    int tmp = 0;\n"
"    switch(nMonth)\n"
"    {\n"
"    case 1:\n"
"    case 3:\n"
"    case 5:\n"
"    case 7:\n"
"    case 8:\n"
"    case 10:\n"
"    case 12:\n"
"        tmp = 31;\n"
"        break;\n"
"    case 4:\n"
"    case 6:\n"
"    case 9:\n"
"    case 11:\n"
"        tmp =30;\n"
"        break;\n"
"    case 2:\n"
"        if ( IsLeapYear(nYear)==1)\n"
"            tmp = 29;\n"
"        else\n"
"            tmp = 28;\n"
"        break;\n"
"    }\n"
"    return tmp;\n"
"}\n";

std::string DaysToDateDecl =
"void DaysToDate( int nDays, int *rDay, int* rMonth, int* rYear );\n";

std::string DaysToDate =
"void DaysToDate( int nDays, int *rDay, int* rMonth, int* rYear )\n"
"{\n"
"\n"
"    int   nTempDays;\n"
"    int   i = 0;\n"
"    bool    bCalc;\n"

"    do\n"
"    {\n"
"        nTempDays = nDays;\n"
"        *rYear = (int)((nTempDays / 365) - i);\n"
"        nTempDays -= ((int) *rYear -1) * 365;\n"
"        nTempDays -= (( *rYear -1) / 4) - (( *rYear -1) / 100) +"
"((*rYear -1) / 400);\n"
"        bCalc = false;\n"
"        if ( nTempDays < 1 )\n"
"        {\n"
"            i++;\n"
"            bCalc = true;\n"
"        }\n"
"        else\n"
"        {\n"
"            if ( nTempDays > 365 )\n"
"            {\n"
"                if ( (nTempDays != 366) || !IsLeapYear( *rYear ) )\n"
"                {\n"
"                    i--;\n"
"                    bCalc = true;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    while ( bCalc );\n"
"     if(nTempDays!=0){\n"
"      for (*rMonth = 1; (int)nTempDays > DaysInMonth( *rMonth, *rYear );"
"*rMonth+=1)\n"
"      {\n"
"       nTempDays -= DaysInMonth( *rMonth, *rYear );\n"
"      }\n"
"      *rDay = (int)nTempDays;\n"
"     }\n"
"}\n";

std::string DateToDaysDecl=
"int  DateToDays( int nDay, int nMonth, int nYear );\n";

std::string DateToDays=
"int  DateToDays( int nDay, int nMonth, int nYear )\n"
"{\n"
"    int nDays = ((int)nYear-1) * 365;\n"
"    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);\n"
"    for( int i = 1; i < nMonth; i++ )\n"
"        nDays += DaysInMonth(i,nYear);\n"
"    nDays += nDay;\n"
"\n"
"    return nDays;\n"
"}\n";

std::string DateToDays_newDecl=
"int  DateToDays_new( int nDay, int nMonth, int nYear );\n";

std::string DateToDays_new=
"int  DateToDays_new( int nDay, int nMonth, int nYear )\n"
"{\n"
"    int nDays = (nYear-1) * 365;\n"
"    nDays += (int)((nYear-1) *pow(4.0,-1.0)- (nYear-1) *pow( 100.0,-1.0)"
"+ (nYear-1) *pow(400.0,-1.0));\n"
"    for( int i = 1; i < nMonth; i++ )\n"
"        nDays += DaysInMonth(i,nYear);\n"
"    nDays += nDay;\n"
"\n"
"    return nDays;\n"
"}\n";

std::string GetNullDateDecl=
"int GetNullDate();\n";

std::string GetNullDate=
"int GetNullDate()\n"
"{\n"
"    return DateToDays(30,12,1899 );\n"
"}\n";
std::string GetNullDate_newDecl=
"int GetNullDate_new();\n";

std::string GetNullDate_new=
"int GetNullDate_new()\n"
"{\n"
"    return DateToDays_new(30,12,1899 );\n"
"}\n";

std::string ScaDateDecl=
"void ScaDate( int nNullDate, int nDate, int nBase,int *nOrigDay, "
"int *nMonth,int *nYear,int *bLastDayMode,int *bLastDay,"
"int *b30Days,int *bUSMode,int *nDay);\n";

std::string ScaDate=
"void ScaDate( int nNullDate, int nDate, int nBase,int *nOrigDay, "
"int *nMonth,int *nYear,int *bLastDayMode,int *bLastDay,"
"int *b30Days,int *bUSMode,int *nDay)\n"
"{\n"
"    DaysToDate( nNullDate + nDate, nOrigDay, nMonth, nYear );\n"
"    *bLastDayMode = (nBase != 5);\n"
"    *bLastDay = (*nOrigDay >= DaysInMonth( *nMonth, *nYear ));\n"
"    *b30Days = (nBase == 0) || (nBase == 4);\n"
"    *bUSMode = (nBase == 0);\n"
"    if( *b30Days)\n"
"    {\n"
"        *nDay = min( *nOrigDay, 30);\n"
"        if( *bLastDay || (*nDay >=DaysInMonth( *nMonth, *nYear )) )\n"
"        *nDay = 30;\n"
"    }\n"
"    else\n"
"    {\n"
"        int nLastDay = DaysInMonth( *nMonth, *nYear );\n"
"        *nDay = *bLastDay ? nLastDay : min( *nOrigDay, nLastDay );\n"
"    }\n"
"}\n";

std::string ScaDate2Decl=
"void ScaDate2( int nNullDate, int nDate, int nBase,int *bLastDayMode,int *"
"bLastDay,int *b30Days,int *bUSMode);\n";

std::string ScaDate2=
"void ScaDate2( int nNullDate, int nDate, int nBase,int *bLastDayMode,int *"
"bLastDay,int *b30Days,int *bUSMode)\n"
"{\n"
"    int nOrigDay=0,  nMonth=0,  nYear=0;\n"
"    DaysToDate( nNullDate + nDate, &nOrigDay, &nMonth, &nYear );\n"
"    *bLastDayMode = (nBase != 5);\n"
"    *bLastDay = (nOrigDay >= DaysInMonth( nMonth, nYear ));\n"
"    *b30Days = (nBase == 0) || (nBase == 4);\n"
"    *bUSMode = (nBase == 0);\n"
"}\n";

std::string lcl_GetCouppcdDecl=
"int lcl_GetCouppcd(int nNullDate,int nSettle,int nMat,int nFreq,int nBase);\n";

std::string lcl_GetCouppcd=
"int lcl_GetCouppcd(int nNullDate,int nSettle, int nMat,int nFreq,int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    if(checklessthan(rYear,sYear,rMonth,sMonth,rnDay,snDay,rbLastDay,"
"sbLastDay,rDay,sDay))\n"
"    {\n"
"        rYear+=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    }\n"
"    while(checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay))\n"
"    {\n"
"        double d = -1*(12/nFreq);\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,d,&rYear);\n"
"    }\n"
"    int nLastDay = DaysInMonth( rMonth, rYear );\n"
"    int nRealDay = (rbLastDayMode && rbLastDay) ? nLastDay :"
"min( nLastDay, rDay );\n"
"    return DateToDays( nRealDay, rMonth, rYear ) - nNullDate;\n"
"}\n";

std::string lcl_GetCoupncdDecl=
"int lcl_GetCoupncd(int nNullDate,int nSettle,int nMat,int nFreq,int nBase);\n";

std::string lcl_GetCoupncd=
"int lcl_GetCoupncd(int nNullDate,int nSettle, int nMat,int nFreq,int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    if(checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,rbLastDay"
",sDay,rDay))\n"
"    {\n"
"        rYear-=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    }\n"
"    while(!checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay))\n"
"    {\n"
"      addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,12/nFreq,&rYear);\n"
"    }\n"
"    int nLastDay = DaysInMonth( rMonth, rYear );\n"
"    int nRealDay = (rbLastDayMode && rbLastDay) ? nLastDay :"
"min( nLastDay, rDay );\n"
"    return DateToDays( nRealDay, rMonth, rYear ) - nNullDate;\n"
"}\n";

std::string addMonthsDecl=
"void addMonths(int b30Days,int bLastDay,int *nDay,int nOrigDay,"
"int *nMonth,int nMonthCount,int *year);\n";

std::string addMonths=
"void addMonths(int b30Days,int bLastDay,int *nDay,int nOrigDay,"
"int *nMonth,int nMonthCount,int *year)\n"
"{\n"
"    int nNewMonth = nMonthCount + *nMonth;\n"
"    if( nNewMonth > 12 )\n"
"    {\n"
"        --nNewMonth;\n"
"        *year+=nNewMonth / 12 ;\n"
"        *nMonth = ( nNewMonth % 12 ) + 1;\n"
"    }\n"
"    else if( nNewMonth < 1 )\n"
"    {\n"
"        *year+= nNewMonth / 12 - 1 ;\n"
"        *nMonth =  nNewMonth % 12 + 12 ;\n"
"    }\n"
"    else\n"
"        *nMonth = nNewMonth ;\n"
"    if( b30Days )\n"
"    {\n"
"        *nDay = min( nOrigDay, 30);\n"
"        if( bLastDay || (*nDay >= DaysInMonth( *nMonth, *year )) )\n"
"            *nDay = 30;\n"
"    }\n"
"    else\n"
"    {\n"
"        int nLastDay = DaysInMonth( *nMonth, *year );\n"
"        *nDay = bLastDay ? nLastDay : min( nOrigDay, nLastDay );\n"
"    }\n"
"}\n";

std::string getDaysInMonthRangeDecl=
"int getDaysInMonthRange( int nFrom, int nTo,int b30Days,int year);\n";

std::string getDaysInMonthRange=
"int getDaysInMonthRange( int nFrom, int nTo,int b30Days,int year)\n"
"{\n"
"    if( nFrom > nTo )\n"
"        return 0;\n"
"    int nRet = 0;\n"
"    if( b30Days )\n"
"        nRet = (nTo - nFrom + 1) * 30;\n"
"    else\n"
"    {\n"
"        for( int nMonthIx = nFrom; nMonthIx <= nTo; ++nMonthIx )\n"
"            nRet += b30Days ? 30 : DaysInMonth( nMonthIx, year );\n"
"    }\n"
"    return nRet;\n"
"}\n";

std::string GetDaysInYearsDecl=
"int GetDaysInYears( int nYear1, int nYear2 );\n";

std::string GetDaysInYears=
"int GetDaysInYears( int nYear1, int nYear2 )\n"
"{\n"
"    int  nLeaps = 0;\n"
"    for( int n = nYear1 ; n <= nYear2 ; n++ )\n"
"    {\n"
"        if( IsLeapYear( n ) )\n"
"            nLeaps++;\n"
"    }\n"
"    int  nSum = 1;\n"
"    nSum += nYear2;\n"
"    nSum -= nYear1;\n"
"    nSum *= 365;\n"
"    nSum += nLeaps;\n"
"    return nSum;\n"
"}\n";

std::string GetDaysInYearDecl=
"int GetDaysInYear( int nNullDate, int nDate, int nMode );\n";

std::string GetDaysInYear=
"int GetDaysInYear( int nNullDate, int nDate, int nMode )\n"
"{\n"
"    switch( nMode )\n"
"    {\n"
"    case 0:\n"
"    case 2:\n"
"    case 4:\n"
"        return 360;\n"
"    case 1:\n"
"        {\n"
"            int  nD=0, nM=0, nY=0;\n"
"            nDate += nNullDate;\n"
"            DaysToDate( nDate, &nD, &nM, &nY );\n"
"            return IsLeapYear( nY )? 366 : 365;\n"
"        }\n"
"    case 3:\n"
"        return 365;\n"
"    }\n"
"}\n";

std::string getDaysInYearRangeDecl =
"int getDaysInYearRange( int nFrom, int nTo,int b30Days );\n";

std::string getDaysInYearRange=
"int getDaysInYearRange( int nFrom, int nTo,int b30Days )\n"
"{\n"
"    if( nFrom > nTo )\n"
"        return 0;\n"
"    return b30Days ? ((nTo - nFrom + 1) * 360) : GetDaysInYears( nFrom, nTo)"
";\n"
"}\n";

std::string getDiffDecl=
"int getDiff(int rFrom,int rTo,int fDay,int fMonth,int fYear,int fbLastDayMode,"
"int fbLastDay,int fb30Days,int fbUSMode,int fnDay,int tDay,int tMonth,"
"int tYear,int tbLastDayMode,int tbLastDay,int tb30Days,"
"int tbUSMode,int tnDay);\n";

std::string getDiff=
"int getDiff(int rFrom,int rTo,int fDay,int fMonth,int fYear,int fbLastDayMode,"
"int fbLastDay,int fb30Days,int fbUSMode,int fnDay,int tDay,int tMonth,"
"int tYear,int tbLastDayMode,int tbLastDay,int tb30Days,"
"int tbUSMode,int tnDay)\n"
"{\n"
"    if(rFrom>rTo)\n"
"    {\n"
"        int d=fDay;fDay=tDay;tDay=d;\n"
"        int m=fMonth;fMonth=tMonth;tMonth=m;\n"
"        int y=fYear;fYear=tYear;tYear=y;\n"
"        int a=fbLastDayMode;fbLastDayMode=tbLastDayMode;tbLastDayMode=a;\n"
"        int b=fbLastDay;fbLastDay=tbLastDay;tbLastDay=b;\n"
"        int c=fb30Days;fb30Days=tb30Days;tb30Days=c;\n"
"        int e=fbUSMode;fbUSMode=tbUSMode;tbUSMode=e;\n"
"        int f=fnDay;fnDay=tnDay;tnDay=f;\n"
"    }\n"
"    int nDiff=0;\n"
"    if( tb30Days )\n"
"    {\n"
"        if( tbUSMode )\n"
"        {\n"
"            if( ((fMonth == 2) || (fnDay < 30)) && (tDay == 31) )\n"
"                tnDay = 31;\n"
"            else if( (tMonth == 2) && tbLastDay )\n"
"                tnDay = DaysInMonth( 2, tYear );\n"
"        }\n"
"        else\n"
"        {\n"
"            if( (fMonth == 2) && (fnDay == 30) )\n"
"                fnDay = DaysInMonth( 2, fYear );\n"
"            if( (tMonth == 2) && (tnDay == 30) )\n"
"                tnDay = DaysInMonth( 2, tYear );\n"
"        }\n"
"    }\n"
"    if( (fYear < tYear) || ((fYear == tYear) && (fMonth < tMonth)) )\n"
"    {\n"
"        int d = fb30Days ? 30:DaysInMonth(fMonth,fYear);\n"
"        nDiff = d- fnDay + 1;\n"
"        fDay = fnDay = 1;\n"
"        fbLastDay = 0;\n"
"        addMonths(fb30Days,fbLastDay,&fnDay,fDay,&fMonth,1,&fYear);\n"
"        if( fYear < tYear )\n"
"        {\n"
"            nDiff += getDaysInMonthRange( fMonth, 12,fb30Days,fYear);\n"
"            addMonths(fb30Days,fbLastDay,&fnDay,fDay,&fMonth,13-fMonth,&fYear"
");\n"
"            nDiff += getDaysInYearRange( fYear, tYear - 1,fb30Days);\n"
"            fYear+=tYear - fYear;\n"
"        }\n"
"        nDiff += getDaysInMonthRange(fMonth, tMonth - 1,fb30Days ,fYear );\n"
"        addMonths(fb30Days,fbLastDay,&fnDay,fDay,&fMonth,tMonth-fMonth,&fYear"
");\n"
"    }\n"
"    nDiff += tnDay - fnDay;\n"
"    return nDiff > 0 ? nDiff : 0;\n"
"}\n";

std::string lcl_GetcoupdaybsDecl=
"int lcl_Getcoupdaybs(int nNullDate,int nSettle, int nMat,int nFreq,int nBase);\n";

std::string lcl_Getcoupdaybs=
"int lcl_Getcoupdaybs(int nNullDate,int nSettle, int nMat,int nFreq,int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0;int mDay=0,mMonth=0, mYear=0;int sDay=0,"
"sMonth=0, sYear=0;\n"
"    int rbLastDayMode=0, rbLastDay=0,rb30Days=0,rbUSMode=0,rnDay=0;\n"
"    int sbLastDayMode=0, sbLastDay=0,sb30Days=0,sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,&"
"rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,&"
"sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    rYear= sYear;\n"
"    nSettle=nSettle+nNullDate;\n"
"    aDate=DateToDays( rDay,rMonth,rYear );\n"
"    if( aDate < nSettle )\n"
"    {\n"
"        rYear+= 1;\n"
"        aDate=DateToDays( rDay,rMonth,rYear );\n"
"    }\n"
"    while(aDate > nSettle )\n"
"    {\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,-1*(12/nFreq),&rYear"
");\n"
"        aDate=DateToDays( rDay,rMonth,rYear );\n"
"    }\n"
"    return getDiff( aDate, nSettle, rDay, rMonth, rYear, rbLastDayMode, "
"rbLastDay, rb30Days, rbUSMode, rnDay, sDay, sMonth, sYear, sbLastDayMode,"
"sbLastDay, sb30Days, sbUSMode, snDay);\n"
"}\n";

std::string lcl_Getcoupdaybs_newDecl=
"int lcl_Getcoupdaybs_new(int nNullDate,int nSettle,int nMat,int nFreq,"
"int nBase);\n";

std::string lcl_Getcoupdaybs_new=
"int lcl_Getcoupdaybs_new(int nNullDate,int nSettle,int nMat,int nFreq,"
"int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    aDate=DateToDays_new( rnDay,rMonth,rYear);\n"
"    if(checklessthan(rYear,sYear,rMonth,sMonth,rnDay,snDay,rbLastDay,"
"sbLastDay,rDay,sDay))\n"
"    {\n"
"        rYear+=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"        aDate=DateToDays_new( rnDay,rMonth,rYear );\n"
"    }\n"
"    while(checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay))\n"
"    {\n"
"        double d = -1*(12/nFreq);\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,d,&rYear);\n"
"        aDate=DateToDays_new( rnDay,rMonth,rYear );\n"
"    }\n"
"    return getDiff( aDate,nSettle+nNullDate,rDay,rMonth,rYear,rbLastDayMode,"
"rbLastDay,rb30Days,rbUSMode,rnDay,sDay,sMonth,sYear,sbLastDayMode,sbLastDay,"
"sb30Days,sbUSMode, snDay);\n"
"}\n";

std::string lcl_GetcoupdaysDecl=
"int lcl_Getcoupdays(int nNullDate,int nSettle, "
"int nMat,int nFreq,int nBase);\n";

std::string lcl_Getcoupdays=
"int lcl_Getcoupdays(int nNullDate,int nSettle, "
"int nMat,int nFreq,int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0;int mDay=0,mMonth=0, mYear=0;int sDay=0,"
"sMonth=0, sYear=0;\n"
"    int rbLastDayMode=0, rbLastDay=0,rb30Days=0,rbUSMode=0,rnDay=0;\n"
"    int sbLastDayMode=0, sbLastDay=0,sb30Days=0,sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,&"
"rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,&"
"sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    rYear= sYear;\n"
"    nSettle=nSettle+nNullDate;\n"
"    aDate=DateToDays( rDay,rMonth,rYear );\n"
"    if( aDate < nSettle )\n"
"    {  \n"
"        rYear+= 1;\n"
"        aDate=DateToDays( rDay,rMonth,rYear );\n"
"    }\n"
"    while(aDate > nSettle )\n"
"    {\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,-1*(12/nFreq),&rYear"
");\n"
"        aDate=DateToDays( rDay,rMonth,rYear );\n"
"    }\n"
"    int aNextDate=aDate;int aDay=rDay,aMonth=rMonth, aYear=rYear;\n"
"    int abLastDayMode=rbLastDayMode, abLastDay=rbLastDay,ab30Days=rb30Days,"
"abUSMode=rbUSMode,anDay=rnDay;\n"
"    addMonths(ab30Days,abLastDay,&anDay,aDay,&aMonth,12/nFreq,&aYear);\n"
"    return getDiff( aDate, aNextDate, rDay, rMonth, rYear, rbLastDayMode, "
"rbLastDay, rb30Days, rbUSMode, rnDay, aDay, aMonth, aYear, abLastDayMode,"
"abLastDay, ab30Days, abUSMode, anDay);\n"
"}\n";

std::string lcl_Getcoupdays_newDecl=
"int lcl_Getcoupdays_new(int nNullDate,int nSettle, "
"int nMat,int nFreq,int nBase);\n";

std::string lcl_Getcoupdays_new=
"int lcl_Getcoupdays_new(int nNullDate,int nSettle, "
"int nMat,int nFreq,int nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    aDate=DateToDays_new( rnDay,rMonth,rYear);\n"
"    if(checklessthan(rYear,sYear,rMonth,sMonth,rnDay,snDay,rbLastDay,"
"sbLastDay,rDay,sDay))\n"
"    {\n"
"        rYear+=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"        aDate=DateToDays_new( rnDay,rMonth,rYear );\n"
"    }\n"
"    while(checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay))\n"
"    {\n"
"        double d = -1*12*pow((double)nFreq,-1.0);\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,d,&rYear);\n"
"        aDate=DateToDays_new( rnDay,rMonth,rYear );\n"
"    }\n"
"    int aNextDate=aDate;int aDay=rDay,aMonth=rMonth, aYear=rYear;\n"
"    int abLastDayMode=rbLastDayMode, abLastDay=rbLastDay,ab30Days=rb30Days,"
"abUSMode=rbUSMode,anDay=rnDay;\n"
"    int tmp = (int)(12*pow((double)nFreq,-1.0));\n"
"    addMonths(ab30Days,abLastDay,&anDay,aDay,&aMonth,tmp,&aYear);\n"
"    return getDiff( aDate, aNextDate, rDay, rMonth, rYear, rbLastDayMode, "
"rbLastDay, rb30Days, rbUSMode, rnDay, aDay, aMonth, aYear, abLastDayMode,"
"abLastDay, ab30Days, abUSMode, anDay);\n"
"}\n";

std::string lcl_GetcoupnumDecl=
"int lcl_Getcoupnum(int nNullDate,int nSettle, int nMat,int nFreq);\n";

std::string lcl_Getcoupnum=
"int lcl_Getcoupnum(int nNullDate,int nSettle, int nMat,int nFreq)\n"
"{\n"
"    int aDate = nMat;int rDay=0,rMonth=0, rYear=0;int mDay=0,mMonth=0, mYear="
"0;\n"
"    int sDay=0,sMonth=0, sYear=0;\n"
"    DaysToDate(aDate+nNullDate,&rDay, &rMonth, &rYear );\n"
"    DaysToDate(nMat+nNullDate,&mDay, &mMonth, &mYear );\n"
"    DaysToDate(nSettle+nNullDate,&sDay, &sMonth, &sYear );\n"
"    rYear= sYear;\n"
"    nSettle=nSettle+nNullDate;\n"
"    aDate=DateToDays( rDay,rMonth,rYear );\n"
"    if( aDate < nSettle )\n"
"        rYear+= 1;\n"
"    int d=DateToDays( rDay,rMonth,rYear );\n"
"    int nMonthCount=-1*(12 / nFreq);\n"
"    while(d > nSettle )\n"
"    {\n"
"        int nNewMonth = nMonthCount + rMonth;\n"
"        if( nNewMonth > 12 )\n"
"        {\n"
"            --nNewMonth;\n"
"            rYear+=nNewMonth / 12;\n"
"            rMonth = nNewMonth % 12 + 1;\n"
"        }\n"
"        else if( nNewMonth < 1 )\n"
"        {\n"
"            rYear+= nNewMonth / 12 - 1;\n"
"            rMonth = nNewMonth % 12 + 12;\n"
"        }\n"
"        else\n"
"            rMonth =  nNewMonth;\n"
"        d=DateToDays( rDay,rMonth,rYear );\n"
"    }\n"
"    int n=(mYear-rYear)*12+mMonth-rMonth;\n"
"    n=n*nFreq/12;\n"
"    return n;\n"
"}\n";
std::string lcl_Getcoupnum_newDecl=
"double lcl_Getcoupnum_new(int nNullDate,int nSettle,int nMat,int nFreq,int"
" nBase);\n";
std::string lcl_Getcoupnum_new=
"double lcl_Getcoupnum_new(int nNullDate,int nSettle, int nMat,int nFreq,int"
" nBase)\n"
"{\n"
"    int aDate = nMat;\n"
"    int mDay=0,mMonth=0, mYear=0;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    mMonth = rMonth, mYear = rYear;\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    if(checklessthan(rYear,sYear,rMonth,sMonth,rnDay,snDay,rbLastDay,"
"sbLastDay,rDay,sDay))\n"
"    {\n"
"        rYear+=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    }\n"
"    int m= checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay);\n"
"    while(m)\n"
"    {\n"
"        double d = -1*(12/nFreq);\n"
"        addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,d,&rYear);\n"
"        m = checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay);\n"
"    }\n"
"    int n=(mYear-rYear)*12+mMonth-rMonth;\n"
"    double tmp = (double)(n*nFreq)/12.0;\n"
"    return tmp;\n"
"}\n";

std::string setDayDecl=
"void setDay(int nOrigDay, int nMonth,int nYear,int bLastDay,int b30Days,"
"int *nDay);\n";
std::string setDay=
"void setDay(int nOrigDay, int nMonth,int nYear,int bLastDay,int b30Days,"
"int *nDay)\n"
"{\n"
"    if( b30Days )\n"
"    {\n"
"        *nDay = min( nOrigDay, 30);\n"
"        if( bLastDay || (*nDay >= DaysInMonth( nMonth, nYear )) )\n"
"            *nDay = 30;\n"
"    }\n"
"    else\n"
"    {\n"
"        int nLastDay = DaysInMonth( nMonth, nYear );\n"
"        *nDay = bLastDay ? nLastDay : min( nOrigDay, nLastDay );\n"
"    }\n"
"}\n";

std::string coupdaysDecl=
"double coupdays(int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdays=
"double coupdays(int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=GetNullDate();\n"
"    if( nBase == 1 )\n"
"        return lcl_Getcoupdays(nNullDate, nSettle, nMat,nFreq, nBase);\n"
"    else\n"
"        return (double)GetDaysInYear(0,0,nBase)/nFreq;\n"
"}\n";
std::string coupdays_newDecl=
"double coupdays_new(int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdays_new=
"double coupdays_new(int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=693594;\n"
"    if( nBase == 1 )\n"
"        return lcl_Getcoupdays_new(nNullDate, nSettle, nMat,nFreq, nBase);\n"
"    else\n"
"        return (double)GetDaysInYear(0,0,nBase)*pow((double)nFreq,-1.0);\n"
"}\n";

std::string coupdaybsDecl=
"double coupdaybs( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdaybs=
"double coupdaybs( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=GetNullDate();\n"
"    return lcl_Getcoupdaybs(nNullDate, nSettle, nMat,nFreq, nBase);\n"
"}\n";

std::string coupdaybs_newDecl=
"double coupdaybs_new( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdaybs_new=
"double coupdaybs_new( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=693594;\n"
"    return lcl_Getcoupdaybs_new(nNullDate, nSettle, nMat,nFreq, nBase);\n"
"}\n";

std::string coupdaysncDecl=
"double coupdaysnc( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdaysnc=
    "double coupdaysnc( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=GetNullDate();\n"
"    if((nBase != 0) && (nBase != 4))\n"
"    {\n"
"        int aDate = nMat;\n"
"        int rDay=0,rMonth=0, rYear=0;int mDay=0,mMonth=0, mYear=0;int sDay=0,"
"sMonth=0, sYear=0;\n"
"        int rbLastDayMode=0, rbLastDay=0,rb30Days=0,rbUSMode=0,rnDay=0;\n"
"        int sbLastDayMode=0, sbLastDay=0,sb30Days=0,sbUSMode=0,snDay=0;\n"
"        ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,&"
"rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"        ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"        rYear= sYear;\n"
"        nSettle=nSettle+nNullDate;\n"
"        aDate=DateToDays( rDay,rMonth,rYear );\n"
"        if( aDate > nSettle )\n"
"        {\n"
"            rYear-= 1;\n"
"            aDate=DateToDays( rDay,rMonth,rYear );\n"
"        }\n"
"        while(aDate <= nSettle )\n"
"        {\n"
"            addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,12/nFreq,&rYear)"
";\n"
"            aDate=DateToDays( rDay,rMonth,rYear );\n"
"        }\n"
"        return getDiff( nSettle, aDate, sDay, sMonth, sYear, sbLastDayMode, "
"sbLastDay, sb30Days, sbUSMode, snDay, rDay, rMonth, rYear, rbLastDayMode, "
"rbLastDay, rb30Days, rbUSMode, rnDay);\n"
"    }\n"
"    else\n"
"        return coupdays(nSettle,nMat,nFreq,nBase)- coupdaybs( nSettle,nMat,"
"nFreq,nBase);\n"
"}\n";
std::string coupdaysnc_newDecl=
"double coupdaysnc_new( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupdaysnc_new=
"double coupdaysnc_new( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=693594;\n"
"    if((nBase != 0) && (nBase != 4))\n"
"    {\n"
"    int aDate = nMat;\n"
"    int rDay=0,rMonth=0, rYear=0,rbLastDayMode=0, rbLastDay=0,rb30Days=0,"
"rbUSMode=0,rnDay=0;\n"
"    int sDay=0,sMonth=0, sYear=0,sbLastDayMode=0, sbLastDay=0,sb30Days=0,"
"sbUSMode=0,snDay=0;\n"
"    ScaDate( nNullDate,aDate,nBase,&rDay,&rMonth,&rYear,&rbLastDayMode,"
"&rbLastDay,&rb30Days,&rbUSMode,&rnDay);\n"
"    ScaDate( nNullDate,nSettle,nBase,&sDay,&sMonth,&sYear,&sbLastDayMode,"
"&sbLastDay,&sb30Days,&sbUSMode,&snDay);\n"
"    rYear=sYear;\n"
"    setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"    aDate=DateToDays( rnDay,rMonth,rYear);\n"
"    if(checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,rbLastDay"
",sDay,rDay))\n"
"    {\n"
"        rYear-=1;\n"
"        setDay(rDay,rMonth,rYear,rbLastDay,rb30Days,&rnDay);\n"
"        aDate=DateToDays( rnDay,rMonth,rYear );\n"
"    }\n"
"    while(!checklessthan(sYear,rYear,sMonth,rMonth,snDay,rnDay,sbLastDay,"
"rbLastDay,sDay,rDay))\n"
"    {\n"
"      addMonths(rb30Days,rbLastDay,&rnDay,rDay,&rMonth,12/nFreq,&rYear);\n"
"      aDate=DateToDays( rnDay,rMonth,rYear );\n"
"    }\n"
"    return getDiff( nSettle+nNullDate,aDate,sDay,sMonth,sYear,sbLastDayMode, "
"sbLastDay, sb30Days, sbUSMode, snDay, rDay, rMonth, rYear, rbLastDayMode, "
"rbLastDay, rb30Days, rbUSMode, rnDay);\n"
"    }\n"
"    else\n"
"        return coupdays_new(nSettle,nMat,nFreq,nBase)- coupdaybs_new( nSettle,"
"nMat,nFreq,nBase);\n"
"}\n";

std::string checklessthanDecl=
"int checklessthan(int aYear,int bYear,int aMonth,int bMonth,int anDay,int "
"bnDay,int abLastDay,int bbLastDay,int anOrigDay,int bnOrigDay);\n";
std::string checklessthan=
"int checklessthan(int aYear,int bYear,int aMonth,int bMonth,int anDay,int "
"bnDay,int abLastDay,int bbLastDay,int anOrigDay,int bnOrigDay)\n"
"{\n"
"    if( aYear != bYear )\n"
"        return aYear < bYear;\n"
"    if( aMonth != bMonth )\n"
"        return aMonth < bMonth;\n"
"    if( anDay != bnDay )\n"
"        return anDay < bnDay;\n"
"    if( abLastDay || bbLastDay )\n"
"        return !abLastDay && bbLastDay;\n"
"    return anOrigDay < bnOrigDay;\n"
"}\n";

std::string coupnumDecl=
"double coupnum( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupnum=
"double coupnum( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=GetNullDate();\n"
"    return lcl_Getcoupnum(nNullDate,nSettle,nMat,nFreq);\n"
"}\n";
std::string coupnum_newDecl=
"double coupnum_new( int nSettle,int nMat,int nFreq,int nBase);\n";

std::string coupnum_new=
"double coupnum_new( int nSettle,int nMat,int nFreq,int nBase)\n"
"{\n"
"    int nNullDate=693594;\n"
"    return lcl_Getcoupnum_new(nNullDate,nSettle,nMat,nFreq,nBase);\n"
"}\n";

std::string getPrice_Decl=
"double getPrice_(int nSettle, int nMat, double fRate, double fYield,\n"
    "double fRedemp, int nFreq, int nBase );\n";

std::string getPrice_=
"double getPrice_(int nSettle, int nMat, double fRate, double fYield,\n"
    "double fRedemp, int nFreq, int nBase )\n"
"{\n"
"    double      fFreq = nFreq;\n"
"    double      fE = coupdays( nSettle, nMat, nFreq, nBase );\n"
"    double      fDSC_E = coupdaysnc(  nSettle, nMat, nFreq, nBase ) / fE;\n"
"    double      fN = coupnum( nSettle, nMat, nFreq, nBase );\n"
"    double      fA = coupdaybs( nSettle, nMat, nFreq, nBase );\n"
"    double      fRet = fRedemp / ( pow( 1.0 + fYield / fFreq, fN - 1.0 + "
"fDSC_E ) );\n"
"    fRet -= 100.0 * fRate / fFreq * fA / fE;\n"
"    double      fT1 = 100.0 * fRate / fFreq;\n"
"    double      fT2 = 1.0 + fYield / fFreq;\n"
"    for( double fK = 0.0 ; fK < fN ; fK+=1.0 )\n"
"        fRet += fT1 / pow( fT2, fK + fDSC_E );\n"
"    return fRet;\n"
"}\n";
std::string getPrice_new_Decl=
"double getPrice_(int nSettle, int nMat, double fRate, double fYield,\n"
    "double fRedemp, int nFreq, int nBase );\n";

std::string getPrice_new=
"double getPrice_(int nSettle, int nMat, double fRate, double fYield,\n"
    "double fRedemp, int nFreq, int nBase )\n"
"{\n"
"    double      fFreq = nFreq;\n"
"    double      fE = coupdays_new( nSettle, nMat, nFreq, nBase );\n"
"    double      fDSC_E = coupdaysnc_new(  nSettle, nMat, nFreq, nBase ) / fE;\n"
"    double      fN = coupnum_new( nSettle, nMat, nFreq, nBase );\n"
"    double      fA = coupdaybs_new( nSettle, nMat, nFreq, nBase );\n"
"    double      fRet = fRedemp / ( pow( 1.0 + fYield / fFreq, fN - 1.0 + "
"fDSC_E ) );\n"
"    fRet -= 100.0 * fRate / fFreq * fA / fE;\n"
"    double      fT1 = 100.0 * fRate / fFreq;\n"
"    double      fT2 = 1.0 + fYield / fFreq;\n"
"    for( double fK = 0.0 ; fK < fN ; fK+=1.0 )\n"
"        fRet += fT1 / pow( fT2, fK + fDSC_E );\n"
"    return fRet;\n"
"}\n";

std::string getYield_Decl=
"double getYield_( int nNullDate, int nSettle, int nMat, double fCoup,"
"double fPrice,double fRedemp, int nFreq, int nBase);\n";

std::string getYield_=
"double getYield_( int nNullDate, int nSettle, int nMat, double fCoup,"
"double fPrice,double fRedemp, int nFreq, int nBase )\n"
"{\n"
"    double      fRate = fCoup;\n"
"    double      fPriceN = 0.0;\n"
"    double      fYield1 = 0.0;\n"
"    double      fYield2 = 1.0;\n"
"    double      fPrice1 = getPrice_(nSettle, nMat, fRate, fYield1, fRedemp, "
"nFreq, nBase );\n"
"    double      fPrice2 = getPrice_(nSettle, nMat, fRate, fYield2, fRedemp, "
"nFreq, nBase );\n"
"    double      fYieldN = ( fYield2 - fYield1 ) * 0.5;\n"
"    for( unsigned int nIter = 0 ; nIter < 100 && fPriceN != fPrice ; nIter++ "
")\n"
"    {\n"
"        fPriceN = getPrice_(nSettle, nMat, fRate, fYieldN, fRedemp, nFreq, "
"nBase );\n"
"        if( fPrice == fPrice1 )\n"
"            return fYield1;\n"
"        else if( fPrice == fPrice2 )\n"
"            return fYield2;\n"
"        else if( fPrice == fPriceN )\n"
"            return fYieldN;\n"
"        else if( fPrice < fPrice2 )\n"
"        {\n"
"            fYield2 *= 2.0;\n"
"            fPrice2 = getPrice_(nSettle, nMat, fRate, fYield2, fRedemp, nFreq"
", nBase );\n"
"            fYieldN = ( fYield2 - fYield1 ) * 0.5;\n"
"        }\n"
"        else\n"
"        {\n"
"            if( fPrice < fPriceN )\n"
"            {\n"
"                fYield1 = fYieldN;\n"
"                fPrice1 = fPriceN;\n"
"            }\n"
"            else\n"
"            {\n"
"                fYield2 = fYieldN;\n"
"                fPrice2 = fPriceN;\n"
"            }\n"
"            fYieldN = fYield2 - ( fYield2 - fYield1 ) * ( ( fPrice - fPrice2 "
") / ( fPrice1 - fPrice2 ) );\n"
"        }\n"
"    }\n"
"    return fYieldN;\n"
"}\n";

std::string GetYearFracDecl=
"double GetYearFrac( int nNullDate, int nStartDate, int nEndDate,"
"int nMode );\n";

std::string GetYearFrac=
"double GetYearFrac( int nNullDate, int nStartDate, int nEndDate,"
"int nMode ) \n"
"{\n"
"    if( nStartDate == nEndDate )\n"
"        return 0.0;     \n"

 "   if( nStartDate > nEndDate )\n"
 "   {\n"
 "       int   n = nEndDate;\n"
 "       nEndDate = nStartDate;\n"
 "       nStartDate = n;\n"
 "   }\n"

  "  int nDate1 = nStartDate + nNullDate;\n"
  "  int nDate2 = nEndDate + nNullDate;\n"

  "  int  nDay1, nDay2;\n"
  "  int  nMonth1, nMonth2;\n"
  "  int  nYear1, nYear2;\n"

  "  DaysToDate( nDate1, &nDay1, &nMonth1, &nYear1 );\n"
  "  DaysToDate( nDate2, &nDay2, &nMonth2, &nYear2 );\n"

  "  int nDayDiff;\n"
  "  switch( nMode )\n"
  "  {\n"
  "      case 0:         \n"
  "          if ( nDay1 == 31 )\n"
  "          {\n"
  "              nDay1--;\n"
  "          }\n"
  "          if ( nDay1 == 30 && nDay2 == 31 )\n"
  "          {\n"
  "              nDay2--;\n"
  "          }\n"
  "          else\n"
  "          {\n"
  "              if ( nMonth1 == 2 && nDay1 == "
  "( IsLeapYear( nYear1 ) ? 29 : 28 ) )\n"
  "              {\n"
  "                  nDay1 = 30;\n"
  "                  if ( nMonth2 == 2 && nDay2 == "
  "( IsLeapYear( nYear2 ) ? 29 : 28 ) )\n"
  "                  {\n"
  "                      nDay2 = 30;\n"
  "                  }\n"
  "              }\n"
  "          }\n"
  "          nDayDiff = ( nYear2 - nYear1 ) * 360 + "
  "( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );\n"
  "          break;\n"
  "      case 1:         \n"
  "      case 2:        \n"
  "      case 3:         \n"
  "         nDayDiff = nDate2 - nDate1;\n"
  "          break;\n"
  "      case 4:         \n"
  "          if ( nDay1 == 31 )\n"
  "          {\n"
  "              nDay1--;\n"
  "          }\n"
  "          if ( nDay2 == 31 )\n"
  "          {\n"
  "              nDay2--;\n"
  "          }\n"
  "          nDayDiff = ( nYear2 - nYear1 ) * 360 + "
  "( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );\n"
  "          break;\n"
  "  }\n"

  "  double nDaysInYear;\n"
  "  switch( nMode )\n"
  "  {\n"
  "      case 0:         \n"
  "      case 2:         \n"
  "      case 4:         \n"
  "          nDaysInYear = 360;\n"
  "          break;\n"
  "      case 1:         \n"
  "          {\n"
  "              bool isYearDifferent = ( nYear1 != nYear2 );\n"
  "              if ( isYearDifferent &&\n"
  "                   ( ( nYear2 != nYear1 + 1 ) ||\n"
  "                     ( nMonth1 < nMonth2 ) ||\n"
  "                     ( nMonth1 == nMonth2 && nDay1 < nDay2 ) ) )\n"
  "              {\n"

  "                  int nDayCount = 0;\n"
  "                 for ( int i = nYear1; i <= nYear2; i++ )\n"
  "                      nDayCount += ( IsLeapYear( i ) ? 366 : 365 );\n"

  "                  nDaysInYear = ( double ) nDayCount / "
  "( double ) ( nYear2 - nYear1 + 1 );\n"
  "              }\n"
  "             else\n"
  "              {\n"
  "                  if ( isYearDifferent && IsLeapYear( nYear1 ) )\n"
  "                  {\n"
  "                      nDaysInYear = 366;\n"
  "                  }\n"
  "                   else\n"
  "                  {\n"

  "                      if ( ( IsLeapYear( nYear1 ) && nMonth1 <= 2 "
  "&& nDay1 <= 29 ) ||\n"
  "                           ( IsLeapYear( nYear2 ) && ( nMonth2 > 3 || "
  "( nMonth2 == 2 && nDay1 == 29 ) ) ) )\n"
   "                     {\n"
   "                         nDaysInYear = 366;\n"
   "                     }\n"
  "                      else\n"
  "                      {\n"
  "                          nDaysInYear = 365;\n"
 "                           for ( int i = nYear1; i <= nYear2; i++ )\n"
  "                          {\n"
   "                             if ( IsLeapYear( i ) )\n"
  "                              {\n"
  "                                  nDaysInYear = 366;\n"
   "                                 break;\n"
   "                             }\n"
  "                          }\n"
   "                     }\n"
   "                 }\n"
   "             }\n"
   "         }\n"
  "          break;\n"
  "      case 3:         \n"
  "          nDaysInYear = 365;\n"
 "           break;\n"
  "  }\n"
  "  return (double)( nDayDiff ) / (nDaysInYear);\n"
"}\n";

std::string GetYieldmatDecl=
    "double GetYieldmat( int nNullDate, int nSettle, int nMat, int nIssue,\n"
        "double fRate, double fPrice, int nBase );\n";

std::string GetYieldmat=
    "double GetYieldmat( int nNullDate, int nSettle, int nMat, int nIssue,\n"
        "double fRate, double fPrice, int nBase )\n"
"{\n"
"    double      fIssMat = GetYearFrac_new( nNullDate, nIssue, nMat, nBase );\n"
"    double      fIssSet = GetYearFrac_new( nNullDate, nIssue, nSettle, nBase );\n"
"    double      fSetMat = GetYearFrac_new( nNullDate, nSettle, nMat, nBase );\n"
"    double      y = 1.0 + fIssMat * fRate;\n"
"    y =y * pow( (fPrice / 100.0 + fIssSet * fRate),-1);\n"
"    y-=1.0;\n"
"    y = y * pow(fSetMat,-1);\n"
"    return y;\n"
"}\n";

std::string GetDiffDateDecl=
"int GetDiffDate( int nNullDate, int nStartDate, int nEndDate, int nMode,"
"    int* pOptDaysIn1stYear );\n";

std::string GetDiffDate=
"int GetDiffDate( int nNullDate, int nStartDate, int nEndDate, int nMode,"
"    int* pOptDaysIn1stYear )\n"
"{\n"
"    bool    bNeg = nStartDate > nEndDate;\n"
"    if( bNeg )\n"
"    {\n"
"        int   n = nEndDate;\n"
"        nEndDate = nStartDate;\n"
"        nStartDate = n;\n"
"    }\n"
"    int       nRet;\n"
"    switch( nMode )\n"
"    {\n"
"    case 0:   \n"
"    case 4:   \n"
"        {\n"
"        int      nD1, nM1, nY1, nD2, nM2, nY2;\n"
"        nStartDate += nNullDate;\n"
"        nEndDate += nNullDate;\n"
"        DaysToDate( nStartDate, &nD1, &nM1, &nY1 );\n"
"        DaysToDate( nEndDate, &nD2, &nM2, &nY2 );\n"
"        bool        bLeap = IsLeapYear( nY1 );\n"
"        int       nDays, nMonths;\n"
"        nMonths = nM2 - nM1;\n"
"        nDays = nD2 - nD1;\n"
"        nMonths += ( nY2 - nY1 ) * 12;\n"
"        nRet = nMonths * 30 + nDays;\n"
"        if( nMode == 0 && nM1 == 2 && nM2 != 2 && nY1 == nY2 )\n"
"            nRet -= bLeap? 1 : 2;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 360;\n"
"        }\n"
"        break;\n"
"    case 1:    \n"
"        if( pOptDaysIn1stYear )\n"
"        {\n"
"            int      nD, nM, nY;\n"
"            DaysToDate( nStartDate + nNullDate, &nD, &nM, &nY );\n"
"            *pOptDaysIn1stYear = IsLeapYear( nY )? 366 : 365;\n"
"        }\n"
"        nRet = nEndDate - nStartDate;\n"
"        break;\n"
"    case 2:      \n"
"        nRet = nEndDate - nStartDate;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 360;\n"
"        break;\n"
"    case 3:        \n"
"        nRet = nEndDate - nStartDate;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 365;\n"
"        break;\n"
"    }\n"
"    return bNeg? -nRet : nRet;\n"
"}\n";

std::string GetYearDiffDecl=
"double GetYearDiff( int nNullDate, int nStartDate, int nEndDate,"
"int nMode);\n";

std::string GetYearDiff=
"double GetYearDiff( int nNullDate, int nStartDate, int nEndDate,"
"int nMode )\n"
"{\n"
"    int   nDays1stYear;\n"
"    int   nTotalDays = GetDiffDate( nNullDate, nStartDate, nEndDate,"
"nMode, &"
"nDays1stYear );\n"
"     return (double)(nTotalDays)*pow((double)nDays1stYear,-1);\n"
"}\n";

std::string GetDiffDate360_Decl=
"int GetDiffDate360_(\n"
"                int nDay1, int nMonth1, int nYear1, bool bLeapYear1,\n"
"                int nDay2, int nMonth2, int nYear2,\n"
"                bool bUSAMethod );\n";

std::string GetDiffDate360_=
"int GetDiffDate360_(\n"
"                int nDay1, int nMonth1, int nYear1, bool bLeapYear1,\n"
"                int nDay2, int nMonth2, int nYear2,\n"
"                bool bUSAMethod )\n"
"{\n"
"    if( nDay1 == 31 )\n"
"        nDay1--;\n"
"    else if( bUSAMethod && ( nMonth1 == 2 && ( nDay1 == 29 || ( nDay1 == 28 "
"&& !bLeapYear1 ) ) ) )\n"
"        nDay1 = 30;\n"
"    if( nDay2 == 31 )\n"
"    {\n"
"        if( bUSAMethod && nDay1 != 30 )\n"
"        {\n"
"            nDay2 = 1;\n"
"            if( nMonth2 == 12 )\n"
"            {\n"
"                nYear2++;\n"
"                nMonth2 = 1;\n"
"            }\n"
"            else\n"
"                nMonth2++;\n"
"        }\n"
"        else\n"
"            nDay2 = 30;\n"
"    }\n"
"    return nDay2 + nMonth2 * 30 + nYear2 * 360 - nDay1 - nMonth1 * 30 - "
"nYear1 * 360;\n"
"}\n";

std::string GetDiffDate360Decl=
"int GetDiffDate360( int nNullDate, int nDate1, int nDate2,"
"bool bUSAMethod);\n";

std::string GetDiffDate360=
"int GetDiffDate360( int nNullDate, int nDate1, int nDate2,"
"bool bUSAMethod )\n"
"{\n"
"    nDate1 += nNullDate;\n"
"    nDate2 += nNullDate;\n"
"    int nDay1, nMonth1, nYear1, nDay2, nMonth2, nYear2;\n"
"    DaysToDate( nDate1, &nDay1, &nMonth1, &nYear1 );\n"
"    DaysToDate( nDate2, &nDay2, &nMonth2, &nYear2 );\n"
"    return GetDiffDate360_( nDay1, nMonth1, nYear1, IsLeapYear( nYear1 ), "
"nDay2, nMonth2, nYear2, bUSAMethod );\n"
"}\n";

std::string GetDurationDecl=
"double GetDuration( \n"
"                int nNullDate, int nSettle, int nMat, double fCoup,\n"
"                double fYield, int nFreq, int nBase );\n";

std::string GetDuration=
"double GetDuration( \n"
"                int nNullDate, int nSettle, int nMat, double fCoup,\n"
"                double fYield, int nFreq, int nBase )\n"
"{\n"
"    double fYearfrac = GetYearFrac( nNullDate, nSettle, nMat, nBase );\n"
"    double fNumOfCoups = lcl_Getcoupnum(nNullDate,nSettle,nMat,nFreq);\n"
"    double fDur = 0.0;\n"
"    double f100 = 100.0;\n"
"    fCoup *= f100 / nFreq;\n"
"    fYield /= nFreq;\n"
"    fYield += 1.0;\n"
"    double nDiff = fYearfrac * nFreq - fNumOfCoups;\n"
"    int  t;\n"
"    for( t = 1 ; t < fNumOfCoups ; t++ )\n"
"        fDur += ( t + nDiff ) * ( fCoup ) / pow( fYield, t + nDiff );\n"
"    fDur += ( fNumOfCoups + nDiff ) * ( fCoup + f100 ) /"
" pow( fYield, fNumOfCoups + nDiff );\n"
"    double  p = 0.0;\n"
"    for( t = 1 ; t < fNumOfCoups ; t++ )\n"
"      p += fCoup / pow( fYield, t + nDiff );\n"
"    p += ( fCoup + f100 ) / pow( fYield, fNumOfCoups + nDiff );\n"
"    fDur /= p;\n"
"    fDur /= nFreq;\n"
"    return fDur;\n""}\n";

std::string GetDuration_newDecl=
"double GetDuration_new( \n"
"                int nNullDate, int nSettle, int nMat, double fCoup,\n"
"                double fYield, int nFreq, int nBase );\n";

std::string GetDuration_new=
"double GetDuration_new( \n"
"                int nNullDate, int nSettle, int nMat, double fCoup,\n"
"                double fYield, int nFreq, int nBase )\n"
"    {\n"
"        double fYearfrac = GetYearFrac(nNullDate,nSettle,nMat,nBase);\n"
"        double fNumOfCoups = lcl_Getcoupnum_new(nNullDate,nSettle,nMat,"
"nFreq,nBase);\n"
"        double fDur = 0.0;\n"
"        fCoup = fCoup * 100.0 * pow(nFreq, -1.0);\n"
"        fYield = fYield * pow(nFreq, -1.0);\n"
"        fYield += 1.0;\n"
"        double nDiff = fYearfrac * nFreq - fNumOfCoups;\n"
"        int  t;\n"
"        double tmp0 = 0, tmp1 = 0, tmp2 = 0;\n"
"        for( t = 1 ; t < fNumOfCoups ; t++ ){\n"
"            tmp0 = (t + nDiff) * ( fCoup ) ;\n"
"            tmp1 = pow( fYield, t + nDiff ) ;\n"
"            tmp2 = tmp0 * pow(tmp1, -1);\n"
"            fDur += tmp2;\n"
"        }\n"
"        fDur += (fNumOfCoups + nDiff) * (fCoup + 100.0) * pow(pow(fYield,"
" fNumOfCoups + nDiff ),-1);\n"
"        double  p = 0.0;\n"
"        for( t = 1 ; t < fNumOfCoups ; t++ ){\n"
"            tmp0 = pow( fYield, t + nDiff );\n"
"            p += fCoup * pow(tmp0, -1);}\n"
"        p += (fCoup + 100.0) * pow(pow(fYield, fNumOfCoups + nDiff), -1);\n"
"        fDur = fDur * pow(p, -1.0);\n"
"        fDur = fDur * pow(nFreq, -1.0);\n"
"        return fDur;\n"
"    }\n";

std::string ScGetGDADecl=
"double ScGetGDA(double fWert, double fRest, double fDauer, double fPeriode,"
"double fFaktor);\n";

std::string ScGetGDA=
"double ScGetGDA(double fWert, double fRest, double fDauer, double fPeriode,"
"double fFaktor)\n"
"{\n"
"    double fGda, fZins, fAlterWert, fNeuerWert;\n"
"    fZins = fFaktor / fDauer;\n"
"    if (fZins >= 1.0)\n"
"    {\n"
"        fZins = 1.0;\n"
"        if (fPeriode == 1.0)\n"
"            fAlterWert = fWert;\n"
"        else\n"
"            fAlterWert = 0.0;\n"
"    }\n"
"    else\n"
"        fAlterWert = fWert * pow(1.0 - fZins, fPeriode - 1.0);\n"
"    fNeuerWert = fWert * pow(1.0 - fZins, fPeriode);\n"

"    if (fNeuerWert < fRest)\n"
"        fGda = fAlterWert - fRest;\n"
"    else\n"
"        fGda = fAlterWert - fNeuerWert;\n"
"    if (fGda < 0.0)\n"
"        fGda = 0.0;\n"
"    return fGda;\n"
"}\n";

std::string DblMinDecl=
"inline double DblMin( double a, double b );\n";

std::string DblMin=
"inline double DblMin( double a, double b )\n"
"{\n"
"    return (a < b) ? a : b;\n"
"}\n";

std::string ScInterVDBDecl=
"double ScInterVDB(double fWert,double fRest,double fDauer, double fDauer1,"
"double fPeriode,double fFaktor);\n";

std::string ScInterVDB=
"double ScInterVDB(double fWert,double fRest,double fDauer, double fDauer1,"
"double fPeriode,double fFaktor)\n"
"{\n"
"    double fVdb=0;\n"
"    double fIntEnd   = ceil(fPeriode);\n"
"    int nLoopEnd   = fIntEnd;\n"

"    double fTerm, fLia;\n"
"    double fRestwert = fWert - fRest;\n"
"    int bNowLia = 0;\n"
"    double fGda;\n"
"    int i;\n"
"    fLia=0;\n"
"    for ( i = 1; i <= nLoopEnd; i++)\n"
"    {\n"
"        if(!bNowLia)\n"
"        {\n"
"            fGda = ScGetGDA(fWert, fRest, fDauer, (double) i, fFaktor);\n"
"            fLia = fRestwert/ (fDauer1 - (double) (i-1));\n"
"            if (fLia > fGda)\n"
"            {\n"
"                fTerm = fLia;\n"
"                bNowLia = 1;\n"
"            }\n"
"            else\n"
"            {\n"
"                fTerm = fGda;\n"
"                fRestwert =fRestwert- fGda;\n"
"            }\n"
"        }\n"
"        else\n"
"        {\n"
"            fTerm = fLia;\n"
"        }\n"

"        if ( i == nLoopEnd)\n"
"            fTerm *= ( fPeriode + 1.0 - fIntEnd );\n"

"        fVdb += fTerm;\n"
"    }\n"
"    return fVdb;\n"
"}\n";

std::string VDBImplementDecl=
"double VDBImplement(double fWert,double fRest, double fDauer, double fAnfang"
", double fEnde, double fFaktor,int bFlag);\n";

std::string VDBImplement=
"double VDBImplement(double fWert,double fRest, double fDauer, double fAnfang"
", double fEnde, double fFaktor,int bFlag)\n"
"{\n"
"    double result=0;\n"
"    double fIntStart = floor(fAnfang);\n"
"    double fIntEnd   = ceil(fEnde);\n"
"    int nLoopStart = (int) fIntStart;\n"
"    int nLoopEnd   = (int) fIntEnd;\n"
"    if (bFlag)\n"
"    {\n"
"        for (int i = nLoopStart + 1; i <= nLoopEnd; i++)\n"
"        {\n"
"            double fTerm = ScGetGDA(fWert, fRest, fDauer, (double) i, fFaktor"
");\n"
"            if ( i == nLoopStart+1 )\n"
"                fTerm *= ( DblMin( fEnde, fIntStart + 1.0 ) - fAnfang );\n"
"            else if ( i == nLoopEnd )\n"
"                fTerm *= ( fEnde + 1.0 - fIntEnd );\n"
"            result += fTerm;\n"
"        }\n"
"    }\n"
"    else\n"
"    {\n"
"        double fDauer1=fDauer;\n"
"        if(!isequal(fAnfang,floor(fAnfang)))\n"
"        {\n"
"            if(fFaktor>1)\n"
"            {\n"
"                if(fAnfang>fDauer/2 || isequal(fAnfang,fDauer/2))\n"
"                {\n"
"                    double fPart=fAnfang-fDauer/2;\n"
"                    fAnfang=fDauer/2;\n"
"                    fEnde-=fPart;\n"
"                    fDauer1+=1;\n"
"                }\n"
"            }\n"
"        }\n"
"        fWert-=ScInterVDB(fWert,fRest,fDauer,fDauer1,fAnfang,fFaktor);\n"
"        result=ScInterVDB(fWert,fRest,fDauer,fDauer-fAnfang,fEnde-fAnfang,"
"fFaktor);\n"
"    }\n"
"    return result;\n"
"}\n";

std::string GetOddlpriceDecl=
"double GetOddlprice( int nNullDate, int nSettle, int nMat, int nLastCoup,\n"
"    double fRate, double fYield, double fRedemp, int nFreq, int nBase );\n";

std::string GetOddlprice=
"double GetOddlprice( int nNullDate, int nSettle, int nMat, int nLastCoup,\n"
"    double fRate, double fYield, double fRedemp, int nFreq, int nBase )\n"
"{\n"
"    double   fFreq =  nFreq ;\n"
"    double   fDCi = GetYearFrac( nNullDate, nLastCoup,"
"nMat, nBase ) * fFreq;\n"
"    double   fDSCi = GetYearFrac( nNullDate, nSettle,"
"nMat, nBase ) * fFreq;\n"
"    double   fAi = GetYearFrac( nNullDate, nLastCoup,"
"nSettle, nBase ) * fFreq;\n"
"    double   p = fRedemp + fDCi * 100.0 * fRate / fFreq;\n"
"    p /= fDSCi * fYield / fFreq + 1.0;\n"
"    p -= fAi * 100.0 * fRate / fFreq;\n"
"    return p;\n"
"}\n";

std::string GetOddlyieldDecl=
"double GetOddlyield( int nNullDate, int nSettle, int nMat, int nLastCoup,\n"
"    double fRate, double fPrice, double fRedemp, int nFreq, int nBase );\n";

std::string GetOddlyield=
"double GetOddlyield( int nNullDate, int nSettle, int nMat, int nLastCoup,\n"
"    double fRate, double fPrice, double fRedemp, int nFreq, int nBase ) \n"
"{\n"
"    double fFreq =  nFreq ;\n"
"    double fDCi= GetYearFrac( nNullDate, nLastCoup, nMat, nBase ) * fFreq;\n"
"    double fDSCi= GetYearFrac( nNullDate, nSettle, nMat, nBase ) * fFreq;\n"
"    double fAi= GetYearFrac( nNullDate, nLastCoup, nSettle, nBase )*fFreq;\n"
"    double y = fRedemp + fDCi * 100.0 * fRate / fFreq;\n"
"    y /= fPrice + fAi * 100.0 * fRate / fFreq;\n"
"    y -= 1.0;\n"
"    y *= fFreq / fDSCi;\n"
"    return y;\n"
"}\n";

std::string GetYearFrac_newDecl=
"double GetYearFrac_new( int nNullDate, int nStartDate, int nEndDate,"
"int nMode );\n";

std::string GetYearFrac_new=
"double GetYearFrac_new( int nNullDate, int nStartDate, int nEndDate,"
"int nMode ) \n"
"{\n"
"    if( nStartDate == nEndDate )\n"
"        return 0.0;     \n"
"   if( nStartDate > nEndDate )\n"
"   {\n"
"       int   n = nEndDate;\n"
"       nEndDate = nStartDate;\n"
"       nStartDate = n;\n"
"   }\n"
"  int nDate1 = nStartDate + nNullDate;\n"
"  int nDate2 = nEndDate + nNullDate;\n"
"  int  nDay1, nDay2;\n"
"  int  nMonth1, nMonth2;\n"
"  int  nYear1, nYear2;\n"
"  DaysToDate_new( nDate1, &nDay1, &nMonth1, &nYear1 );\n"
"  DaysToDate_new( nDate2, &nDay2, &nMonth2, &nYear2 );\n"
"  int nDayDiff;\n"
"  switch( nMode )\n"
"  {\n"
"      case 0:         \n"
"          if ( nDay1 == 31 )\n"
"          {\n"
"              nDay1--;\n"
"          }\n"
"          if ( nDay1 == 30 && nDay2 == 31 )\n"
"          {\n"
"              nDay2--;\n"
"          }\n"
"          else\n"
"          {\n"
"              if ( nMonth1 == 2 && nDay1 == "
"( IsLeapYear( nYear1 ) ? 29 : 28 ) )\n"
"              {\n"
"                  nDay1 = 30;\n"
"                  if ( nMonth2 == 2 && nDay2 == "
"( IsLeapYear( nYear2 ) ? 29 : 28 ) )\n"
"                  {\n"
"                      nDay2 = 30;\n"
"                  }\n"
"              }\n"
"          }\n"
"          nDayDiff = ( nYear2 - nYear1 ) * 360 + "
"( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );\n"
"          break;\n"
"      case 1:         \n"
"      case 2:        \n"
"      case 3:         \n"
"         nDayDiff = nDate2 - nDate1;\n"
"          break;\n"
"      case 4:         \n"
"          if ( nDay1 == 31 )\n"
"          {\n"
"              nDay1--;\n"
"          }\n"
"          if ( nDay2 == 31 )\n"
"          {\n"
"              nDay2--;\n"
"          }\n"
"          nDayDiff = ( nYear2 - nYear1 ) * 360 + "
"( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );\n"
"          break;\n"
"  }\n"
"  double nDaysInYear;\n"
"  switch( nMode )\n"
"  {\n"
"      case 0:         \n"
"      case 2:         \n"
"      case 4:         \n"
"          nDaysInYear = 360;\n"
"          break;\n"
"      case 1:         \n"
"          {\n"
"              bool isYearDifferent = ( nYear1 != nYear2 );\n"
"              if ( isYearDifferent &&\n"
"                   ( ( nYear2 != nYear1 + 1 ) ||\n"
"                     ( nMonth1 < nMonth2 ) ||\n"
"                     ( nMonth1 == nMonth2 && nDay1 < nDay2 ) ) )\n"
"              {\n"
"                  int nDayCount = 0;\n"
"                 for ( int i = nYear1; i <= nYear2; i++ )\n"
"                      nDayCount += ( IsLeapYear( i ) ? 366 : 365 );\n"
"                  nDaysInYear = ( double ) nDayCount / "
"( double ) ( nYear2 - nYear1 + 1 );\n"
"              }\n"
"             else\n"
"              {\n"
"                  if ( isYearDifferent && IsLeapYear( nYear1 ) )\n"
"                  {\n"
"                      nDaysInYear = 366;\n"
"                  }\n"
"                   else\n"
"                  {\n"
"                      if ( ( IsLeapYear( nYear1 ) && nMonth1 <= 2 "
"&& nDay1 <= 29 ) ||\n"
"                           ( IsLeapYear( nYear2 ) && ( nMonth2 > 3 || "
"( nMonth2 == 2 && nDay1 == 29 ) ) ) )\n"
"                     {\n"
"                         nDaysInYear = 366;\n"
"                     }\n"
"                      else\n"
"                      {\n"
"                          nDaysInYear = 365;\n"
"                           for ( int i = nYear1; i <= nYear2; i++ )\n"
"                          {\n"
"                             if ( IsLeapYear( i ) )\n"
"                              {\n"
"                                  nDaysInYear = 366;\n"
"                                 break;\n"
"                             }\n"
"                          }\n"
"                     }\n"
"                 }\n"
"             }\n"
"         }\n"
"          break;\n"
"      case 3:         \n"
"          nDaysInYear = 365;\n"
"           break;\n"
"  }\n"
"  return (double)( nDayDiff ) / (nDaysInYear);\n"
"}\n";

std::string DaysToDate_newDecl =
"void DaysToDate_new( int nDays, int *rDay, int* rMonth, int* rYear );\n";

std::string DaysToDate_new =
"void DaysToDate_new( int nDays, int *rDay, int* rMonth, int* rYear )\n"
"{\n"
"    int   nTempDays;\n"
"    int   i = 0;\n"
"    bool    bCalc;\n"
"    do\n"
"    {\n"
"        nTempDays = nDays;\n"
"        *rYear = (int)((nTempDays / 365) - i);\n"
"        nTempDays -= ((int) *rYear -1) * 365;\n"
"        nTempDays -= ((*rYear -1) / 4) - ((*rYear -1) / 100) + ((*rYear -1)"
" / 400);\n"
"        bCalc = false;\n"
"        if ( nTempDays < 1 )\n"
"        {\n"
"            i++;\n"
"            bCalc = true;\n"
"        }\n"
"        else\n"
"        {\n"
"            if ( nTempDays > 365 )\n"
"            {\n"
"                if ( (nTempDays != 366) || !IsLeapYear( *rYear ) )\n"
"                {\n"
"                    i--;\n"
"                    bCalc = true;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    while ( bCalc );\n"
"    if(nTempDays != 0){\n"
"        for (*rMonth = 1; (int)nTempDays > DaysInMonth( *rMonth, *rYear );"
" *rMonth += 1)\n"
"        {\n"
"            nTempDays -= DaysInMonth( *rMonth, *rYear ); \n"
"        }\n"
"        *rDay = (int)nTempDays;\n"
"    }\n"
"}\n";

std::string DaysToDate_LocalBarrierDecl =
"void DaysToDate( int nDays, int *rDay, int* rMonth, int* rYear );\n";

std::string DaysToDate_LocalBarrier =
"void DaysToDate( int nDays, int *rDay, int* rMonth, int* rYear )\n"
"{\n"
"    int   nTempDays;\n"
"    int   i = 0;\n"
"    bool    bCalc;\n"
"    do\n"
"    {\n"
"        nTempDays = nDays;\n"
"        *rYear = (int)((nTempDays / 365) - i);\n"
"        nTempDays -= ((int) *rYear -1) * 365;\n"
"        nTempDays -= ((*rYear -1) / 4) - ((*rYear -1) / 100) + ((*rYear -1)"
" / 400);\n"
"        bCalc = false;\n"
"        if ( nTempDays < 1 )\n"
"        {\n"
"            i++;\n"
"            bCalc = true;\n"
"        }\n"
"        else\n"
"        {\n"
"            if ( nTempDays > 365 )\n"
"            {\n"
"                if ( (nTempDays != 366) || !IsLeapYear( *rYear ) )\n"
"                {\n"
"                    i--;\n"
"                    bCalc = true;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    while ( bCalc );\n"
"    barrier(CLK_LOCAL_MEM_FENCE);\n"
"    if(nTempDays != 0){\n"
"        for (*rMonth = 1; (int)nTempDays > DaysInMonth( *rMonth, *rYear );"
" *rMonth += 1)\n"
"        {\n"
"            nTempDays -= DaysInMonth( *rMonth, *rYear ); \n"
"        }\n"
"        *rDay = (int)nTempDays;\n"
"    }\n"
"}\n";

std::string GetYearDiff_newDecl=
"double GetYearDiff_new( int nNullDate, int nStartDate, int nEndDate,"
"int nMode);\n";

std::string GetYearDiff_new=
"double GetYearDiff_new( int nNullDate, int nStartDate, int nEndDate,"
"int nMode )\n"
"{\n"
"    int   nDays1stYear;\n"
"    int   nTotalDays = GetDiffDate_new( nNullDate, nStartDate, nEndDate,"
"nMode, &"
"nDays1stYear );\n"
"    return (double)(nTotalDays)* pow((double)nDays1stYear,-1);\n"
"}\n";

std::string GetDiffDate_newDecl=
"int GetDiffDate_new( int nNullDate, int nStartDate, int nEndDate, int nMode,"
"    int* pOptDaysIn1stYear );\n";

std::string GetDiffDate_new=
"int GetDiffDate_new( int nNullDate, int nStartDate, int nEndDate, int nMode,"
"    int* pOptDaysIn1stYear )\n"
"{\n"
"    bool    bNeg = nStartDate > nEndDate;\n"
"    if( bNeg )\n"
"    {\n"
"        int   n = nEndDate;\n"
"        nEndDate = nStartDate;\n"
"        nStartDate = n;\n"
"    }\n"
"    int       nRet;\n"
"    switch( nMode )\n"
"    {\n"
"    case 0:   \n"
"    case 4:   \n"
"        {\n"
"        int      nD1, nM1, nY1, nD2, nM2, nY2;\n"
"        nStartDate += nNullDate;\n"
"        nEndDate += nNullDate;\n"
"        DaysToDate_new( nStartDate, &nD1, &nM1, &nY1 );\n"
"        DaysToDate_new( nEndDate, &nD2, &nM2, &nY2 );\n"
"        bool        bLeap = IsLeapYear( nY1 );\n"
"        int       nDays, nMonths;\n"
"        nMonths = nM2 - nM1;\n"
"        nDays = nD2 - nD1;\n"
"        nMonths += ( nY2 - nY1 ) * 12;\n"
"        nRet = nMonths * 30 + nDays;\n"
"        if( nMode == 0 && nM1 == 2 && nM2 != 2 && nY1 == nY2 )\n"
"            nRet -= bLeap? 1 : 2;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 360;\n"
"        }\n"
"        break;\n"
"    case 1:    \n"
"        if( pOptDaysIn1stYear )\n"
"        {\n"
"            int      nD, nM, nY;\n"
"            DaysToDate_new( nStartDate + nNullDate, &nD, &nM, &nY );\n"
"            *pOptDaysIn1stYear = IsLeapYear( nY )? 366 : 365;\n"
"        }\n"
"        nRet = nEndDate - nStartDate;\n"
"        break;\n"
"    case 2:      \n"
"        nRet = nEndDate - nStartDate;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 360;\n"
"        break;\n"
"    case 3:        \n"
"        nRet = nEndDate - nStartDate;\n"
"        if( pOptDaysIn1stYear )\n"
"            *pOptDaysIn1stYear = 365;\n"
"        break;\n"
"    }\n"
"    return bNeg? -nRet : nRet;\n"
"}\n";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
