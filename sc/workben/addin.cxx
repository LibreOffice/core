/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <usr/factoryhlp.hxx>
#include <usr/macros.hxx>
#include <usr/reflserv.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/date.hxx>
#include <svl/itemprop.hxx>
#include <usr/proptypehlp.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/util/date.hpp>

#include "addin.hxx"
#include "result.hxx"

using namespace com::sun::star;

SMART_UNO_IMPLEMENTATION( ScTestAddIn, UsrObject );

#define SCTESTADDIN_SERVICE     L"stardiv.one.sheet.DemoAddIn"

extern "C" {

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

            uno::Sequence <OUString> aSequ = ScTestAddIn::getSupportedServiceNames_Static();
            const OUString * pArray = aSequ.getConstArray();
            for( INT32 i = 0; i < aSequ.getLength(); i++ )
                xNewKey->createKey( pArray[i] );

            return sal_True;
        }
        catch (registry::InvalidRegistryException&)
        {
            OSL_ENSHURE( false, "### InvalidRegistryException!" );
        }
    }
    return false;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
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

css::uno::Sequence< OUString > ScTestAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = L"com.sun.star.sheet.AddIn";
    pArray[1] = SCTESTADDIN_SERVICE;
    return aRet;
}

uno::Reference<uno::XInterface> ScTestAddIn_CreateInstance(
        const uno::Reference<lang::XMultiServiceFactory>& )
{
    return (cppu::OWeakObject*)new ScTestAddIn();
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
}

css::lang::Locale SAL_CALL ScTestAddIn::getLocale(  ) throw(css::uno::RuntimeException)
{
    return aFuncLoc;
}

// XTestAddIn

sal_Int32 SAL_CALL ScTestAddIn::countParams( const css::uno::Sequence< css::uno::Any >& aArgs ) throw(css::uno::RuntimeException)
{
    return aArgs.getLength();
}

double SAL_CALL ScTestAddIn::addOne( double fValue ) throw(css::uno::RuntimeException)
{
    return fValue + 1.0;
}

OUString SAL_CALL ScTestAddIn::repeatStr( const OUString& aStr, sal_Int32 nCount ) throw(css::uno::RuntimeException)
{
    String aRet;
    String aStrStr = OUStringToString( aStr, CHARSET_SYSTEM );
    for (long i=0; i<nCount; i++)
        aRet += aStrStr;

    return StringToOUString( aRet, CHARSET_SYSTEM );
}

OUString SAL_CALL ScTestAddIn::getDateString( const css::uno::Reference< css::beans::XPropertySet >& xCaller, double fValue ) throw(css::uno::RuntimeException)
{
    uno::Any aDateAny = xCaller->getPropertyValue( L"NullDate" );
    {
        util::Date aDate;
        aDateAny >>= aDate;
        {
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

sal_Int32 SAL_CALL ScTestAddIn::getColorValue( const css::uno::Reference< css::table::XCellRange >& xRange ) throw(css::uno::RuntimeException)
{
    uno::Reference<beans::XPropertySet> xProp( xRange, uno::UNO_QUERY );
    if (xProp.is())
    {
        long nRet = -1;
        uno::Any aTrans = xProp->getPropertyValue( L"IsCellBackgroundTransparent" );
        sal_Bool bIsTrans;
        aTrans >>= bIsTrans;        //! don't use >>= for BOOL
        if (!bIsTrans)
        {
            uno::Any aCol = xProp->getPropertyValue( L"CellBackColor" );
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

css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL ScTestAddIn::transpose( const css::uno::Sequence< css::uno::Sequence< double > >& aMatrix ) throw(css::uno::RuntimeException)
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

css::uno::Sequence< css::uno::Sequence< sal_Int32 > > SAL_CALL ScTestAddIn::transposeInt( const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& aMatrix ) throw(css::uno::RuntimeException)
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

OUString SAL_CALL ScTestAddIn::repeatMultiple( sal_Int32 nCount, const css::uno::Any& aFirst, const css::uno::Sequence< css::uno::Any >& aFollow ) throw(css::uno::RuntimeException)
{
    String aSeparator;
    if ( !aFirst.hasValue() )   // not specified
        aSeparator = ';';
    else
    {
        OUString aUStr;
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
        OUString aUStr;
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

css::uno::Any SAL_CALL ScTestAddIn::getStrOrVal( sal_Int32 nFlag ) throw(css::uno::RuntimeException)
{
    uno::Any aRet;

    //! Test
    if ( nFlag == 42 )
    {
        uno::Sequence<OUString> aInner(3);
        aInner.getArray()[0] = L"Bla";
        aInner.getArray()[1] = L"Fasel";
        aInner.getArray()[2] = L"Suelz";
        uno::Sequence< uno::Sequence<OUString> > aOuter( &aInner, 1 );

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

css::uno::Reference< css::sheet::XVolatileResult > SAL_CALL ScTestAddIn::callAsync( const OUString& aString ) throw(css::uno::RuntimeException)
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
OUString SAL_CALL ScTestAddIn::getServiceName(  ) throw(css::uno::RuntimeException)
{
    return SCTESTADDIN_SERVICE;     // name of specific AddIn service
}

// XServiceInfo
OUString SAL_CALL ScTestAddIn::getImplementationName(  ) throw(css::uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ScTestAddIn::supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ScTestAddIn::getSupportedServiceNames(  ) throw(css::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
