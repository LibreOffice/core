/*************************************************************************
 *
 *  $RCSfile: analysis.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: gt $ $Date: 2001-04-06 13:59:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ANALYSIS_HXX
#define ANALYSIS_HXX


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>

#include <cppuhelper/implbase4.hxx> // helper for implementations

#include "analysisdefs.hxx"


class FuncDataList;
class ConvertDataList;


REF( CSS::uno::XInterface ) SAL_CALL AnalysisAddIn_CreateInstance( const REF( CSS::lang::XMultiServiceFactory )& );


class AnalysisAddIn : public cppu::WeakImplHelper4<
                            CSS::sheet::XAddIn,
                            CSS::sheet::addin::XAnalysis,
                            CSS::lang::XServiceName,
                            CSS::lang::XServiceInfo >
{
private:
    CSS::lang::Locale           aFuncLoc;
    FuncDataList*               pFD;
    double*                     pFactDoubles;
    ConvertDataList*            pCDL;
public:
                                AnalysisAddIn();
    virtual                     ~AnalysisAddIn();

    double                      FactDouble( sal_Int32 nNum ) THROWDEF_RTE_IAE;

    static STRING               getImplementationName_Static();
    static SEQ( STRING )        getSupportedServiceNames_Static();

                                // XAddIn
    virtual STRING SAL_CALL     getProgrammaticFuntionName( const STRING& aDisplayName ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getDisplayFunctionName( const STRING& aProgrammaticName ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getFunctionDescription( const STRING& aProgrammaticName ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getDisplayArgumentName( const STRING& aProgrammaticFunctionName, sal_Int32 nArgument ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getArgumentDescription( const STRING& aProgrammaticFunctionName, sal_Int32 nArgument ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getProgrammaticCategoryName( const STRING& aProgrammaticFunctionName ) THROWDEF_RTE;
    virtual STRING SAL_CALL     getDisplayCategoryName( const STRING& aProgrammaticFunctionName ) THROWDEF_RTE;

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const CSS::lang::Locale& eLocale ) THROWDEF_RTE;
    virtual CSS::lang::Locale SAL_CALL getLocale(  ) THROWDEF_RTE;

                                // XServiceName
    virtual STRING SAL_CALL     getServiceName(  ) THROWDEF_RTE;

                                // XServiceInfo
    virtual STRING SAL_CALL     getImplementationName(  ) THROWDEF_RTE;
    virtual sal_Bool SAL_CALL   supportsService( const STRING& ServiceName ) THROWDEF_RTE;
    virtual CSS::uno::Sequence< STRING > SAL_CALL getSupportedServiceNames(  ) THROWDEF_RTE;

    //  methods from own interfaces start here

                            // XAnalysis
    virtual double SAL_CALL     get_Test( constREFXPS&, sal_Int32 nMode, double f1, double f2, double f3 ) THROWDEF_RTE;

    virtual sal_Int32 SAL_CALL  getWorkday( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nDays, const SEQSEQ( sal_Int32 )& aHDay ) THROWDEF_RTE;
    virtual double SAL_CALL     getYearfrac( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) THROWDEF_RTE;
    virtual sal_Int32 SAL_CALL  getEdate( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE;
    virtual sal_Int32 SAL_CALL  getWeeknum( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMode ) THROWDEF_RTE;
    virtual sal_Int32 SAL_CALL  getEomonth( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE;
    virtual sal_Int32 SAL_CALL  getNetworkdays( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nEndDate, const SEQSEQ( sal_Int32 )& aHDay ) THROWDEF_RTE;

    virtual sal_Int32 SAL_CALL  getIseven( constREFXPS&, sal_Int32 nVal ) THROWDEF_RTE;
    virtual sal_Int32 SAL_CALL  getIsodd( constREFXPS&, sal_Int32 nVal ) THROWDEF_RTE;

    virtual double SAL_CALL     getMultinomial( constREFXPS&, const SEQSEQ( sal_Int32 )& aValueList ) THROWDEF_RTE;
    virtual double SAL_CALL     getSeriessum( constREFXPS&, double fX, double fN, double fM, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE;
    virtual double SAL_CALL     getQuotient( constREFXPS&, double fNum, double fDenum ) THROWDEF_RTE;

    virtual double SAL_CALL     getMround( constREFXPS&, double fNum, double fMult ) THROWDEF_RTE;
    virtual double SAL_CALL     getSqrtpi( constREFXPS&, double fNum ) THROWDEF_RTE;

    virtual double SAL_CALL     getRandbetween( constREFXPS&, double fMin, double fMax ) THROWDEF_RTE;

    virtual double SAL_CALL     getGcd( constREFXPS&, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE;
    virtual double SAL_CALL     getLcm( constREFXPS&, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE;

    virtual double SAL_CALL     getBesseli( constREFXPS&, double fNum, sal_Int32 nOrder ) THROWDEF_RTE;
    virtual double SAL_CALL     getBesselj( constREFXPS&, double fNum, sal_Int32 nOrder ) THROWDEF_RTE;
    virtual double SAL_CALL     getBesselk( constREFXPS&, double fNum, sal_Int32 nOrder ) THROWDEF_RTE;
    virtual double SAL_CALL     getBessely( constREFXPS&, double fNum, sal_Int32 nOrder ) THROWDEF_RTE;

    virtual STRING SAL_CALL     getBin2oct( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getBin2dec( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getBin2hex( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;

    virtual STRING SAL_CALL     getOct2bin( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOct2dec( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getOct2hex( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;

    virtual STRING SAL_CALL     getDec2bin( constREFXPS&, sal_Int32 fNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getDec2oct( constREFXPS&, sal_Int32 fNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getDec2hex( constREFXPS&, double fNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;

    virtual STRING SAL_CALL     getHex2bin( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getHex2dec( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getHex2oct( constREFXPS&, const STRING& aNum, sal_Int32 nPlaces ) THROWDEF_RTE_IAE;

    virtual sal_Int32 SAL_CALL  getDelta( constREFXPS&, double fNum1, double fNum2 ) THROWDEF_RTE;

    virtual double SAL_CALL     getErf( constREFXPS&, double fLowerLimit, double fUpperLimit ) THROWDEF_RTE;
    virtual double SAL_CALL     getErfc( constREFXPS&, double fLowerLimit ) THROWDEF_RTE;

    virtual sal_Int32 SAL_CALL  getGestep( constREFXPS&, double fNum, double fStep ) THROWDEF_RTE;

    virtual double SAL_CALL     getFactdouble( constREFXPS&, sal_Int32 nNum ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getImabs( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImaginary( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImpower( constREFXPS&, const STRING& aNum, double fPower ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImargument( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImcos( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImdiv( constREFXPS&, const STRING& aDivident, const STRING& aDivisor ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImexp( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImconjugate( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImln( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImlog10( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImlog2( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImproduct( constREFXPS&, const STRING& aNum1, const STRING& aNum2 ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImreal( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImsin( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImsub( constREFXPS&, const STRING& aNum1, const STRING& aNum2 ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImsum( constREFXPS&, const STRING& aNum1, const STRING& aNum2 ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getImsqrt( constREFXPS&, const STRING& aNum ) THROWDEF_RTE_IAE;
    virtual STRING SAL_CALL     getComplex( constREFXPS&, double fReal, double fImaginary, const STRING& rSuffix ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getConvert( constREFXPS&, double fVal, const STRING& aFromUnit, const STRING& aToUnit ) THROWDEF_RTE_IAE;
};

//------------------------------------------------------------------

#endif

