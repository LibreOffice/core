/*************************************************************************
 *
 *  $RCSfile: addin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:17 $
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

#pragma hdrstop

#include <usr/factoryhlp.hxx>
#include <usr/macros.hxx>
#include <usr/reflserv.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <tools/date.hxx>
#include <svtools/itemprop.hxx>
#include <usr/proptypehlp.hxx>
#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
#include <osl/diagnose.h>

#include <com/sun/star/util/date.hpp>

#include "addin.hxx"
#include "result.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------


//------------------------------------------------------------------------

SMART_UNO_IMPLEMENTATION( ScTestAddIn, UsrObject );

#define SCADDIN_SERVICE         L"com.sun.star.sheet.AddIn"
#define SCTESTADDIN_SERVICE     L"stardiv.one.sheet.DemoAddIn"

//------------------------------------------------------------------------

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, registry::XRegistryKey * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            UString aImpl = L"/";
            aImpl += ScTestAddIn::getImplementationName_Static();
            aImpl += L"/UNO/SERVICES";

            uno::Reference<registry::XRegistryKey> xNewKey(
                reinterpret_cast<registry::XRegistryKey*>(pRegistryKey)->createKey(aImpl) );

            uno::Sequence <rtl::OUString> aSequ = ScTestAddIn::getSupportedServiceNames_Static();
            const rtl::OUString * pArray = aSequ.getConstArray();
            for( INT32 i = 0; i < aSequ.getLength(); i++ )
                xNewKey->createKey( pArray[i] );

            return sal_True;
        }
        catch (registry::InvalidRegistryException&)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void* pRet = 0;

    if ( pServiceManager && UString(pImplName) == ScTestAddIn::getImplementationName_Static() )
    {
        uno::Reference<lang::XSingleServiceFactory> xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScTestAddIn::getImplementationName_Static(),
                ScTestAddIn_CreateInstance,
                ScTestAddIn::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern C

//------------------------------------------------------------------------

ScTestAddIn::ScTestAddIn()
{
}

ScTestAddIn::~ScTestAddIn()
{
}

UString ScTestAddIn::getImplementationName_Static()
{
    return L"stardiv.StarCalc.ScTestAddIn";
}

::com::sun::star::uno::Sequence< ::rtl::OUString > ScTestAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = SCADDIN_SERVICE;
    pArray[1] = SCTESTADDIN_SERVICE;
    return aRet;
}

uno::Reference<uno::XInterface> ScTestAddIn_CreateInstance(
        const uno::Reference<lang::XMultiServiceFactory>& )
{
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new ScTestAddIn();
    return xInst;
}


// XAddIn

UString ScTestAddIn::getProgrammaticFuntionName(const UString& aDisplayName)
                                THROWS( (UsrSystemException) )
{
    //!...
    return UString();
}

UString ScTestAddIn::getDisplayFunctionName(const UString& aProgrammaticName)
                                THROWS( (UsrSystemException) )
{
    //  return translated strings

    UString aRet;
    if ( aProgrammaticName == L"countParams" )        aRet = L"ParamAnzahl";
    else if ( aProgrammaticName == L"addOne" )        aRet = L"PlusEins";
    else if ( aProgrammaticName == L"repeatStr" )     aRet = L"WiederholeString";
    else if ( aProgrammaticName == L"getDateString" ) aRet = L"Datumsstring";
    else if ( aProgrammaticName == L"getColorValue" ) aRet = L"Farbwert";
    else if ( aProgrammaticName == L"transpose" )     aRet = L"Transponieren";
    else if ( aProgrammaticName == L"transposeInt" )  aRet = L"IntegerTransponieren";
    else if ( aProgrammaticName == L"repeatMultiple" )aRet = L"Mehrfach";
    else if ( aProgrammaticName == L"getStrOrVal" )   aRet = L"StringOderWert";
    else if ( aProgrammaticName == L"callAsync" )     aRet = L"Asynchron";
    return aRet;
}

UString ScTestAddIn::getFunctionDescription(const UString& aProgrammaticName)
                                THROWS( (UsrSystemException) )
{
    //  return translated strings

    UString aRet;
    if ( aProgrammaticName == L"countParams" )        aRet = L"Gibt die Anzahl der Parameter zurueck.";
    else if ( aProgrammaticName == L"addOne" )        aRet = L"Addiert 1 zur uebergebenen Zahl.";
    else if ( aProgrammaticName == L"repeatStr" )     aRet = L"Wiederholt eine Zeichenkette.";
    else if ( aProgrammaticName == L"getDateString" ) aRet = L"Wandelt ein Datum in eine Zeichenkette.";
    else if ( aProgrammaticName == L"getColorValue" ) aRet = L"Gibt den Farbwert eines Zellbereichs zurueck. Bei transparentem Hintergrund wird -1 zurueckgegeben";
    else if ( aProgrammaticName == L"transpose" )     aRet = L"Transponiert eine Matrix.";
    else if ( aProgrammaticName == L"transposeInt" )  aRet = L"Transponiert eine Matrix mit Ganzzahlen.";
    else if ( aProgrammaticName == L"repeatMultiple" )aRet = L"Wiederholt mehrere Bestandteile.";
    else if ( aProgrammaticName == L"getStrOrVal" )   aRet = L"Gibt einen String oder einen Wert zurueck.";
    else if ( aProgrammaticName == L"callAsync" )     aRet = L"Test fuer asynchrone Funktion.";
    return aRet;
}

UString ScTestAddIn::getDisplayArgumentName(const UString& aProgrammaticFunctionName,
                                INT32 nArgument) THROWS( (UsrSystemException) )
{
    //  return translated strings

    UString aRet;
    if ( aProgrammaticFunctionName == L"countParams" )
    {
        if ( nArgument == 0 )       aRet = L"Parameter";
    }
    else if ( aProgrammaticFunctionName == L"addOne" )
    {
        if ( nArgument == 0 )       aRet = L"Wert";
    }
    else if ( aProgrammaticFunctionName == L"repeatStr" )
    {
        if ( nArgument == 0 )       aRet = L"String";
        else if ( nArgument == 1 )  aRet = L"Anzahl";
    }
    else if ( aProgrammaticFunctionName == L"getDateString" )
    {
        if ( nArgument == 0 )       aRet = L"Dokument";
        else if ( nArgument == 1 )  aRet = L"Wert";
    }
    else if ( aProgrammaticFunctionName == L"getColorValue" )
    {
        if ( nArgument == 0 )       aRet = L"Bereich";
    }
    else if ( aProgrammaticFunctionName == L"transpose" )
    {
        if ( nArgument == 0 )       aRet = L"Matrix";
    }
    else if ( aProgrammaticFunctionName == L"transposeInt" )
    {
        if ( nArgument == 0 )       aRet = L"Matrix";
    }
    else if ( aProgrammaticFunctionName == L"repeatMultiple" )
    {
        if ( nArgument == 0 )       aRet = L"Anzahl";
        else if ( nArgument == 1 )  aRet = L"Trenner";
        else if ( nArgument == 2 )  aRet = L"Inhalt";
    }
    else if ( aProgrammaticFunctionName == L"getStrOrVal" )
    {
        if ( nArgument == 0 )       aRet = L"Flag";
    }
    else if ( aProgrammaticFunctionName == L"callAsync" )
    {
        if ( nArgument == 0 )       aRet = L"Name";
    }
    return aRet;
}

UString ScTestAddIn::getArgumentDescription(const UString& aProgrammaticFunctionName,
                                INT32 nArgument) THROWS( (UsrSystemException) )
{
    //  return translated strings

    UString aRet;
    if ( aProgrammaticFunctionName == L"countParams" )
    {
        if ( nArgument == 0 )       aRet = L"Beliebiger Parameter";
    }
    else if ( aProgrammaticFunctionName == L"addOne" )
    {
        if ( nArgument == 0 )       aRet = L"Der Wert, zu dem 1 addiert wird";
    }
    else if ( aProgrammaticFunctionName == L"repeatStr" )
    {
        if ( nArgument == 0 )       aRet = L"Der Text, der wiederholt wird";
        else if ( nArgument == 1 )  aRet = L"Die Anzahl der Wiederholungen";
    }
    else if ( aProgrammaticFunctionName == L"getDateString" )
    {
        if ( nArgument == 0 )       aRet = L"(intern)";
        else if ( nArgument == 1 )  aRet = L"Der Wert, der als Datum formatiert wird";
    }
    else if ( aProgrammaticFunctionName == L"getColorValue" )
    {
        if ( nArgument == 0 )       aRet = L"Der Bereich, dessen Hintergrundfarbe abgefragt wird";
    }
    else if ( aProgrammaticFunctionName == L"transpose" )
    {
        if ( nArgument == 0 )       aRet = L"Die Matrix, die transponiert werden soll";
    }
    else if ( aProgrammaticFunctionName == L"transposeInt" )
    {
        if ( nArgument == 0 )       aRet = L"Die Matrix, die transponiert werden soll";
    }
    else if ( aProgrammaticFunctionName == L"repeatMultiple" )
    {
        if ( nArgument == 0 )       aRet = L"Anzahl der Wiederholungen";
        else if ( nArgument == 1 )  aRet = L"Text, der zwischen den Inhalten erscheint";
        else if ( nArgument == 2 )  aRet = L"Mehrere Inhalte";
    }
    else if ( aProgrammaticFunctionName == L"getStrOrVal" )
    {
        if ( nArgument == 0 )       aRet = L"Wenn Flag 0 ist, wird ein Wert zurueckgegeben, sonst ein String.";
    }
    else if ( aProgrammaticFunctionName == L"callAsync" )
    {
        if ( nArgument == 0 )       aRet = L"Ein String";
    }
    return aRet;
}

UString ScTestAddIn::getProgrammaticCategoryName(const UString& aProgrammaticFunctionName)
                                THROWS( (UsrSystemException) )
{
    //  return non-translated strings

    UString aRet;
    if ( aProgrammaticFunctionName == L"countParams" )        aRet = L"Information";
    else if ( aProgrammaticFunctionName == L"addOne" )        aRet = L"Mathematical";
    else if ( aProgrammaticFunctionName == L"repeatStr" )     aRet = L"Text";
    else if ( aProgrammaticFunctionName == L"getDateString" ) aRet = L"Date&Time";
    else if ( aProgrammaticFunctionName == L"getColorValue" ) aRet = L"Spreadsheet";
    else if ( aProgrammaticFunctionName == L"transpose" )     aRet = L"Matrix";
    else if ( aProgrammaticFunctionName == L"transposeInt" )  aRet = L"Matrix";
    else if ( aProgrammaticFunctionName == L"repeatMultiple" )aRet = L"Text";
    else if ( aProgrammaticFunctionName == L"getStrOrVal" )   aRet = L"Add-In";
    else if ( aProgrammaticFunctionName == L"callAsync" )     aRet = L"Realtime";       // new group
    return aRet;
}

UString ScTestAddIn::getDisplayCategoryName(const UString& aProgrammaticFunctionName)
                                THROWS( (UsrSystemException) )
{
    //  return translated strings

    return L"irgendwas";    // not used for predefined categories
}

// XLocalizable

void ScTestAddIn::setLocale(const lang::Locale& eLocale) THROWS( (UsrSystemException) )
{
    aFuncLoc = eLocale;
//  DBG_ERROR( UStringToString(aFuncLoc.Language, CHARSET_SYSTEM) + String("-") +
//             UStringToString(aFuncLoc.Country, CHARSET_SYSTEM) );
}

::com::sun::star::lang::Locale SAL_CALL ScTestAddIn::getLocale(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return aFuncLoc;
}

// XTestAddIn

sal_Int32 SAL_CALL ScTestAddIn::countParams( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArgs ) throw(::com::sun::star::uno::RuntimeException)
{
    return aArgs.getLength();
}

double SAL_CALL ScTestAddIn::addOne( double fValue ) throw(::com::sun::star::uno::RuntimeException)
{
    return fValue + 1.0;
}

::rtl::OUString SAL_CALL ScTestAddIn::repeatStr( const ::rtl::OUString& aStr, sal_Int32 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    String aRet;
    String aStrStr = OUStringToString( aStr, CHARSET_SYSTEM );
    for (long i=0; i<nCount; i++)
        aRet += aStrStr;

    return StringToOUString( aRet, CHARSET_SYSTEM );
}

::rtl::OUString SAL_CALL ScTestAddIn::getDateString( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCaller, double fValue ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aDateAny = xCaller->getPropertyValue( L"NullDate" );
//! if ( aDateAny.getReflection()->equals( *Date_getReflection() ) )
    {
        util::Date aDate;
        aDateAny >>= aDate;
        //const Date* pDate = (const Date*)aDateAny.get();
        //if (pDate)
        {
            //Date aNewDate = *pDate;
            Date aNewDate( aDate.Day, aDate.Month, aDate.Year );

            aNewDate += (long)(fValue+0.5);

            String aRet;
            aRet += aNewDate.GetDay();
            aRet += '.';
            aRet += aNewDate.GetMonth();
            aRet += '.';
            aRet += aNewDate.GetYear();
            return StringToOUString( aRet, CHARSET_SYSTEM );
        }
    }

    return L"**ERROR**";
}

sal_Int32 SAL_CALL ScTestAddIn::getColorValue( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >& xRange ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference<beans::XPropertySet> xProp( xRange, uno::UNO_QUERY );
    if (xProp.is())
    {
        long nRet = -1;
        uno::Any aTrans = xProp->getPropertyValue( L"IsCellBackgroundTransparent" );
        BOOL bIsTrans;
        aTrans >>= bIsTrans;        //! dont use >>= for BOOL
        if (!bIsTrans)
        {
            uno::Any aCol = xProp->getPropertyValue( L"CellBackColor" );
            //nRet = NAMESPACE_USR(OPropertyTypeConversion)::toINT32( aCol );
            aCol >>= nRet;
        }
        return nRet;
    }
    return 0;
}

double lcl_GetDoubleElement( const uno::Sequence< uno::Sequence<double> >& aMatrix, long nCol, long nRow )
{
    if ( nRow < aMatrix.getLength() )
    {
        const uno::Sequence<double>& rRowSeq = aMatrix.getConstArray()[nRow];
        if ( nCol < rRowSeq.getLength() )
            return rRowSeq.getConstArray()[nCol];
    }
    return 0.0;     // error
}

INT32 lcl_GetLongElement( const uno::Sequence< uno::Sequence<INT32> >& aMatrix, long nCol, long nRow )
{
    if ( nRow < aMatrix.getLength() )
    {
        const uno::Sequence<INT32>& rRowSeq = aMatrix.getConstArray()[nRow];
        if ( nCol < rRowSeq.getLength() )
            return rRowSeq.getConstArray()[nCol];
    }
    return 0.0;     // error
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL ScTestAddIn::transpose( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aMatrix ) throw(::com::sun::star::uno::RuntimeException)
{
    long nRowCount = aMatrix.getLength();
    long nColCount = 0;
    if ( nRowCount )
        nColCount = aMatrix.getConstArray()[0].getLength();

    uno::Sequence< uno::Sequence<double> > aRet( nColCount );
    for (long nCol=0; nCol<nColCount; nCol++)
    {
        uno::Sequence<double> aSubSeq(nRowCount);
        for (long nRow=0; nRow<nRowCount; nRow++)
            aSubSeq.getArray()[nRow] = lcl_GetDoubleElement( aMatrix, nCol, nRow );

        aRet.getArray()[nCol] = aSubSeq;
    }

    return aRet;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > > SAL_CALL ScTestAddIn::transposeInt( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& aMatrix ) throw(::com::sun::star::uno::RuntimeException)
{
    long nRowCount = aMatrix.getLength();
    long nColCount = 0;
    if ( nRowCount )
        nColCount = aMatrix.getConstArray()[0].getLength();

    uno::Sequence< uno::Sequence<INT32> > aRet( nColCount );
    for (long nCol=0; nCol<nColCount; nCol++)
    {
        uno::Sequence<INT32> aSubSeq(nRowCount);
        for (long nRow=0; nRow<nRowCount; nRow++)
            aSubSeq.getArray()[nRow] = lcl_GetLongElement( aMatrix, nCol, nRow );

        aRet.getArray()[nCol] = aSubSeq;
    }

    return aRet;
}

::rtl::OUString SAL_CALL ScTestAddIn::repeatMultiple( sal_Int32 nCount, const ::com::sun::star::uno::Any& aFirst, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aFollow ) throw(::com::sun::star::uno::RuntimeException)
{
    String aSeparator;
    if ( !aFirst.hasValue() )   // not specified
        aSeparator = ';';
    else
    {
        rtl::OUString aUStr;
        aFirst >>= aUStr;
        aSeparator = OUStringToString( aUStr, CHARSET_SYSTEM );
    }

    String aContent;
    long nContCount = aFollow.getLength();
    const uno::Any* pArr = aFollow.getConstArray();
    for (long nPos=0; nPos<nContCount; nPos++)
    {
        if ( nPos > 0 )
            aContent += ' ';
        rtl::OUString aUStr;
        pArr[nPos] >>= aUStr;
        aContent += OUStringToString( aUStr, CHARSET_SYSTEM );
    }

    String aRet;

    for (long i=0; i<nCount; i++)
    {
        if (i>0)
            aRet += aSeparator;
        aRet += aContent;
    }

    return StringToOUString(aRet, CHARSET_SYSTEM);
}

::com::sun::star::uno::Any SAL_CALL ScTestAddIn::getStrOrVal( sal_Int32 nFlag ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aRet;

    //! Test
    if ( nFlag == 42 )
    {
        uno::Sequence<rtl::OUString> aInner(3);
        aInner.getArray()[0] = L"Bla";
        aInner.getArray()[1] = L"Fasel";
        aInner.getArray()[2] = L"Suelz";
        uno::Sequence< uno::Sequence<rtl::OUString> > aOuter( &aInner, 1 );

        //return uno::Any( &aOuter, Sequence< Sequence<UString> >::getReflection() );

        aRet <<= aOuter;
        return aRet;
    }
    //! Test

    if ( nFlag )
        aRet <<= UString(L"This is a string.");
    else
        aRet <<= (INT32)42;

    return aRet;
}

::com::sun::star::uno::Reference< ::com::sun::star::sheet::XVolatileResult > SAL_CALL ScTestAddIn::callAsync( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException)
{
    String aStr = OUStringToString( aString, CHARSET_SYSTEM );
    char c = (char) aStr;
    if ( c >= '0' && c <= '9' )
    {
        if (!xNumResult.is())
            xNumResult = new ScAddInResult( "Num" );
        return xNumResult;
    }
    else
    {
        if (!xAlphaResult.is())
            xAlphaResult = new ScAddInResult( "Str" );
        return xAlphaResult;
    }
}


// XServiceName

::rtl::OUString SAL_CALL ScTestAddIn::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return SCTESTADDIN_SERVICE;     // name of specific AddIn service
}

// XServiceInfo

::rtl::OUString SAL_CALL ScTestAddIn::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ScTestAddIn::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    return ServiceName == SCADDIN_SERVICE ||
            ServiceName == SCTESTADDIN_SERVICE;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ScTestAddIn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------------



