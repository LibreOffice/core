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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/anytostring.hxx>

#include "vbawsfunction.hxx"
#include "compiler.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

namespace {

void lclConvertDoubleToBoolean( uno::Any& rAny )
{
    if( rAny.has< double >() )
    {
        double fValue = rAny.get< double >();
        if( fValue == 0.0 )
            rAny <<= false;
        else if( fValue == 1.0 )
            rAny <<= true;
        // do nothing for other values or types
    }
}

void lclConvertBooleanToDouble( uno::Any& rAny )
{
    sal_Bool bValue( false );
    if ( rAny >>= bValue )
    {
        if ( bValue )
           rAny <<= double( 1.0 );
        else
           rAny <<= double( 0.0 );
    }
}

} // namespace

ScVbaWSFunction::ScVbaWSFunction( const uno::Reference< XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    ScVbaWSFunction_BASE( xParent, xContext )
{
}

uno::Reference< beans::XIntrospectionAccess >
ScVbaWSFunction::getIntrospection(void)  throw(uno::RuntimeException)
{
    return uno::Reference<beans::XIntrospectionAccess>();
}

uno::Any SAL_CALL
ScVbaWSFunction::invoke(const OUString& FunctionName, const uno::Sequence< uno::Any >& Params, uno::Sequence< sal_Int16 >& /*OutParamIndex*/, uno::Sequence< uno::Any >& /*OutParam*/) throw(lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    // create copy of parameters, replace Excel range objects with UNO range objects
    uno::Sequence< uno::Any > aParamTemp( Params );
    if( aParamTemp.hasElements() )
    {
        uno::Any* pArray = aParamTemp.getArray();
        uno::Any* pArrayEnd = pArray + aParamTemp.getLength();
        for( ; pArray < pArrayEnd; ++pArray )
        {
            switch( pArray->getValueType().getTypeClass()  )
            {
                case uno::TypeClass_BOOLEAN:
                    lclConvertBooleanToDouble( *pArray );
                    break;
                case uno::TypeClass_INTERFACE:
                {
                    uno::Reference< excel::XRange > myRange( *pArray, uno::UNO_QUERY );
                    if( myRange.is() )
                        *pArray = myRange->getCellRange();
                }
                    break;
                case uno::TypeClass_SEQUENCE:
                {
                    // the sheet.FunctionAccess service doesn't deal with Sequences, only Sequences of Sequence
                    uno::Type aType = pArray->getValueType();
                    if ( aType.equals( getCppuType( (uno::Sequence<sal_Int16>*)0 ) ) )
                    {
                        uno::Sequence< uno::Sequence< sal_Int16 > >  aTmp(1);
                        (*pArray) >>= aTmp[ 0 ];
                        (*pArray) <<= aTmp;
                    }
                    else if ( aType.equals( getCppuType( (uno::Sequence<sal_Int32>*)0 ) ) )
                    {
                        uno::Sequence< uno::Sequence< sal_Int32 > > aTmp(1);
                        (*pArray) >>= aTmp[ 0 ];
                        (*pArray) <<= aTmp;
                    }
                    else if ( aType.equals( getCppuType( (uno::Sequence<double>*)0 ) ) )
                    {
                        uno::Sequence< uno::Sequence< double > > aTmp(1);
                        (*pArray) >>= aTmp[ 0 ];
                        (*pArray) <<= aTmp;
                    }
                    else if ( aType.equals( getCppuType( (uno::Sequence<OUString>*)0 ) ) )
                    {
                        uno::Sequence< uno::Sequence< OUString > > aTmp(1);
                        (*pArray) >>= aTmp[ 0 ];
                        (*pArray) <<= aTmp;
                    }
                    else if ( aType.equals( getCppuType( (uno::Sequence<uno::Any>*)0 ) ) )
                    {
                        uno::Sequence< uno::Sequence<uno::Any > > aTmp(1);
                        (*pArray) >>= aTmp[ 0 ];
                        (*pArray) <<= aTmp;
                    }
                }
                    break;
                default:
                    break;
            }
            OSL_TRACE("Param[%d] is %s", (int)(pArray - aParamTemp.getConstArray()), OUStringToOString( comphelper::anyToString( *pArray ), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    uno::Any aRet;
    bool bAsArray = true;

    // special handing for some functions that don't work correctly in FunctionAccess
    ScCompiler aCompiler( 0, ScAddress() );
    OpCode eOpCode = aCompiler.GetEnglishOpCode( FunctionName.toAsciiUpperCase() );
    switch( eOpCode )
    {
        // ISLOGICAL does not work in array formula mode
        case ocIsLogical:
        {
            if( aParamTemp.getLength() != 1 )
                throw lang::IllegalArgumentException();
            const uno::Any& rParam = aParamTemp[ 0 ];
            if( rParam.has< bool >() )
            {
                aRet <<= true;
            }
            else if( rParam.has< uno::Reference< table::XCellRange > >() ) try
            {
                uno::Reference< sheet::XCellRangeAddressable > xRangeAddr( rParam, uno::UNO_QUERY_THROW );
                table::CellRangeAddress aRangeAddr = xRangeAddr->getRangeAddress();
                bAsArray = (aRangeAddr.StartColumn != aRangeAddr.EndColumn) || (aRangeAddr.StartRow != aRangeAddr.EndRow);
            }
            catch( uno::Exception& )
            {
            }
        }
        break;
        default:;
    }

    if( !aRet.hasValue() )
    {
        uno::Reference< lang::XMultiComponentFactory > xSMgr( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XFunctionAccess > xFunctionAccess( xSMgr->createInstanceWithContext(
            OUString( "com.sun.star.sheet.FunctionAccess" ), mxContext ),
            uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xPropSet( xFunctionAccess, uno::UNO_QUERY_THROW );
        xPropSet->setPropertyValue("IsArrayFunction", uno::Any( bAsArray ) );
        aRet = xFunctionAccess->callFunction( FunctionName, aParamTemp );
    }

    /*  Convert return value from double to to Boolean for some functions that
        return Booleans. */
    typedef uno::Sequence< uno::Sequence< uno::Any > > AnySeqSeq;
    if( (eOpCode == ocIsEmpty) || (eOpCode == ocIsString) || (eOpCode == ocIsNonString) || (eOpCode == ocIsLogical) ||
        (eOpCode == ocIsRef) || (eOpCode == ocIsValue) || (eOpCode == ocIsFormula) || (eOpCode == ocIsNA) ||
        (eOpCode == ocIsErr) || (eOpCode == ocIsError) || (eOpCode == ocIsEven) || (eOpCode == ocIsOdd) ||
        (eOpCode == ocAnd) || (eOpCode == ocOr) || (eOpCode == ocXor) || (eOpCode == ocNot) || (eOpCode == ocTrue) || (eOpCode == ocFalse) )
    {
        if( aRet.has< AnySeqSeq >() )
        {
            AnySeqSeq aAnySeqSeq = aRet.get< AnySeqSeq >();
            for( sal_Int32 nRow = 0; nRow < aAnySeqSeq.getLength(); ++nRow )
                for( sal_Int32 nCol = 0; nCol < aAnySeqSeq[ nRow ].getLength(); ++nCol )
                    lclConvertDoubleToBoolean( aAnySeqSeq[ nRow ][ nCol ] );
            aRet <<= aAnySeqSeq;
        }
        else
        {
            lclConvertDoubleToBoolean( aRet );
        }
    }

    /*  Hack/workaround (?): shorten single-row matrix to simple array, shorten
        1x1 matrix to single value. */
    if( aRet.has< AnySeqSeq >() )
    {
        AnySeqSeq aAnySeqSeq = aRet.get< AnySeqSeq >();
        if( aAnySeqSeq.getLength() == 1 )
        {
            if( aAnySeqSeq[ 0 ].getLength() == 1 )
                aRet = aAnySeqSeq[ 0 ][ 0 ];
            else
                aRet <<= aAnySeqSeq[ 0 ];
        }
    }

#if 0
    // MATCH function should alwayse return a double value, but currently if the first argument is XCellRange, MATCH function returns an array instead of a double value. Don't know why?
    // To fix this issue in safe, current solution is to convert this array to a double value just for MATCH function.
    OUString aUpper( FunctionName.toAsciiUpperCase() );
    ScCompiler aCompiler( NULL, ScAddress() );
    OpCode eOp = aCompiler.GetEnglishOpCode( aUpper );
    if( eOp == ocMatch )
    {
        double fVal = 0.0;
        if( aRet >>= fVal )
            return aRet;
        uno::Sequence< uno::Sequence< uno::Any > > aSequence;
        if( !( ( aRet >>= aSequence ) && ( aSequence.getLength() > 0 ) &&
            ( aSequence[0].getLength() > 0 ) && ( aSequence[0][0] >>= fVal ) ) )
                throw uno::RuntimeException();
        aRet <<= fVal;
    }
#endif

    return aRet;
}

void SAL_CALL
ScVbaWSFunction::setValue(const OUString& /*PropertyName*/, const uno::Any& /*Value*/) throw(beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL
ScVbaWSFunction::getValue(const OUString& /*PropertyName*/) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL
ScVbaWSFunction::hasMethod(const OUString& Name)  throw(uno::RuntimeException)
{
    sal_Bool bIsFound = false;
    try
    {
    // the function name contained in the com.sun.star.sheet.FunctionDescription service is alwayse localized.
        // but the function name used in WorksheetFunction is a programmatic name (seems English).
        // So m_xNameAccess->hasByName( Name ) may fail to find name when a function name has a localized name.
        ScCompiler aCompiler( NULL, ScAddress() );
        if( aCompiler.IsEnglishSymbol( Name ) )
            bIsFound = sal_True;
    }
    catch( uno::Exception& /*e*/ )
    {
        // failed to find name
    }
    return bIsFound;
}

sal_Bool SAL_CALL
ScVbaWSFunction::hasProperty(const OUString& /*Name*/)  throw(uno::RuntimeException)
{
     return false;
}

OUString SAL_CALL
ScVbaWSFunction::getExactName( const OUString& aApproximateName ) throw (css::uno::RuntimeException)
{
    OUString sName = aApproximateName.toAsciiUpperCase();
    if ( !hasMethod( sName ) )
        return OUString();
    return sName;
}

OUString
ScVbaWSFunction::getServiceImplName()
{
    return OUString("ScVbaWSFunction");
}

uno::Sequence< OUString >
ScVbaWSFunction::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.WorksheetFunction" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
