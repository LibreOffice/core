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
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <ooo/vba/excel/XRange.hpp>

#include "vbawsfunction.hxx"
#include <compiler.hxx>

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
    bool bValue( false );
    if ( rAny >>= bValue )
    {
        if ( bValue )
           rAny <<= 1.0;
        else
           rAny <<= 0.0;
    }
}

} // namespace

ScVbaWSFunction::ScVbaWSFunction( const uno::Reference< XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    ScVbaWSFunction_BASE( xParent, xContext )
{
}

uno::Reference< beans::XIntrospectionAccess >
ScVbaWSFunction::getIntrospection()
{
    return uno::Reference<beans::XIntrospectionAccess>();
}

uno::Any SAL_CALL
ScVbaWSFunction::invoke(const OUString& FunctionName, const uno::Sequence< uno::Any >& Params, uno::Sequence< sal_Int16 >& /*OutParamIndex*/, uno::Sequence< uno::Any >& /*OutParam*/)
{
    // create copy of parameters, replace Excel range objects with UNO range objects
    uno::Sequence< uno::Any > aParamTemp( Params );
    if( aParamTemp.hasElements() )
    {
        for( uno::Any & rArray : aParamTemp )
        {
            switch( rArray.getValueType().getTypeClass()  )
            {
                case uno::TypeClass_BOOLEAN:
                    lclConvertBooleanToDouble( rArray );
                    break;
                case uno::TypeClass_INTERFACE:
                {
                    uno::Reference< excel::XRange > myRange( rArray, uno::UNO_QUERY );
                    if( myRange.is() )
                        rArray = myRange->getCellRange();
                }
                    break;
                case uno::TypeClass_SEQUENCE:
                {
                    // the sheet.FunctionAccess service doesn't deal with Sequences, only Sequences of Sequence
                    uno::Type aType = rArray.getValueType();
                    if ( aType.equals( cppu::UnoType<uno::Sequence<sal_Int16>>::get() ) )
                    {
                        uno::Sequence< uno::Sequence< sal_Int16 > >  aTmp(1);
                        rArray >>= aTmp[ 0 ];
                        rArray <<= aTmp;
                    }
                    else if ( aType.equals( cppu::UnoType<uno::Sequence<sal_Int32>>::get() ) )
                    {
                        uno::Sequence< uno::Sequence< sal_Int32 > > aTmp(1);
                        rArray >>= aTmp[ 0 ];
                        rArray <<= aTmp;
                    }
                    else if ( aType.equals( cppu::UnoType<uno::Sequence<double>>::get() ) )
                    {
                        uno::Sequence< uno::Sequence< double > > aTmp(1);
                        rArray >>= aTmp[ 0 ];
                        rArray <<= aTmp;
                    }
                    else if ( aType.equals( cppu::UnoType<uno::Sequence<OUString>>::get() ) )
                    {
                        uno::Sequence< uno::Sequence< OUString > > aTmp(1);
                        rArray >>= aTmp[ 0 ];
                        rArray <<= aTmp;
                    }
                    else if ( aType.equals( cppu::UnoType<uno::Sequence<uno::Any>>::get() ) )
                    {
                        uno::Sequence< uno::Sequence<uno::Any > > aTmp(1);
                        rArray >>= aTmp[ 0 ];
                        rArray <<= aTmp;
                    }
                }
                    break;
                default:
                    break;
            }
        }
    }

    uno::Any aRet;
    bool bAsArray = true;

    // special handing for some functions that don't work correctly in FunctionAccess
    formula::FormulaCompiler aCompiler;
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
        uno::Reference< lang::XMultiComponentFactory > xSMgr( mxContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference< sheet::XFunctionAccess > xFunctionAccess( xSMgr->createInstanceWithContext(
            "com.sun.star.sheet.FunctionAccess", mxContext ),
            uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xPropSet( xFunctionAccess, uno::UNO_QUERY_THROW );
        xPropSet->setPropertyValue("IsArrayFunction", uno::Any( bAsArray ) );
        aRet = xFunctionAccess->callFunction( FunctionName, aParamTemp );
    }

    /*  Convert return value from double to Boolean for some functions that
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
            for( auto& rAnySeq : aAnySeqSeq )
                for( auto& rAny : rAnySeq )
                    lclConvertDoubleToBoolean( rAny );
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
ScVbaWSFunction::setValue(const OUString& /*PropertyName*/, const uno::Any& /*Value*/)
{
    throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL
ScVbaWSFunction::getValue(const OUString& /*PropertyName*/)
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL
ScVbaWSFunction::hasMethod(const OUString& Name)
{
    bool bIsFound = false;
    try
    {
        // the function name contained in the com.sun.star.sheet.FunctionDescription service is alwayse localized.
        // but the function name used in WorksheetFunction is a programmatic name (seems English).
        // So m_xNameAccess->hasByName( Name ) may fail to find name when a function name has a localized name.
        if( ScCompiler::IsEnglishSymbol( Name ) )
            bIsFound = true;
    }
    catch( uno::Exception& /*e*/ )
    {
        // failed to find name
    }
    return bIsFound;
}

sal_Bool SAL_CALL
ScVbaWSFunction::hasProperty(const OUString& /*Name*/)
{
     return false;
}

OUString SAL_CALL
ScVbaWSFunction::getExactName( const OUString& aApproximateName )
{
    OUString sName = aApproximateName.toAsciiUpperCase();
    if ( !hasMethod( sName ) )
        return OUString();
    return sName;
}

OUString
ScVbaWSFunction::getServiceImplName()
{
    return "ScVbaWSFunction";
}

uno::Sequence< OUString >
ScVbaWSFunction::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.WorksheetFunction"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
