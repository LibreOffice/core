/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbawsfunction.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
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

ScVbaWSFunction::ScVbaWSFunction( const uno::Reference< XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext): ScVbaWSFunction_BASE( xParent, xContext )
{
}


uno::Reference< beans::XIntrospectionAccess >
ScVbaWSFunction::getIntrospection(void)  throw(uno::RuntimeException)
{
    return uno::Reference<beans::XIntrospectionAccess>();
}

uno::Any SAL_CALL
ScVbaWSFunction::invoke(const rtl::OUString& FunctionName, const uno::Sequence< uno::Any >& Params, uno::Sequence< sal_Int16 >& /*OutParamIndex*/, uno::Sequence< uno::Any >& /*OutParam*/) throw(lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    uno::Reference< lang::XMultiComponentFactory > xSMgr( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XFunctionAccess > xFunctionAccess(
                        xSMgr->createInstanceWithContext(::rtl::OUString::createFromAscii(
                        "com.sun.star.sheet.FunctionAccess"), mxContext),
                        ::uno::UNO_QUERY_THROW);
    uno::Sequence< uno::Any > aParamTemp;
    sal_Int32 nParamCount = Params.getLength();
    aParamTemp.realloc(nParamCount);
    const uno::Any* aArray = Params.getConstArray();
    uno::Any* aArrayTemp = aParamTemp.getArray();

    for (int i=0; i < Params.getLength();i++)
    {
        uno::Reference<excel::XRange> myRange( aArray[ i ], uno::UNO_QUERY );
        if ( myRange.is() )
        {
            aArrayTemp[i] = myRange->getCellRange();
            continue;
        }
        aArrayTemp[i]= aArray[i];
    }

    for ( int count=0; count < aParamTemp.getLength(); ++count )
        OSL_TRACE("Param[%d] is %s",
            count, rtl::OUStringToOString( comphelper::anyToString( aParamTemp[count] ), RTL_TEXTENCODING_UTF8 ).getStr()  );

    uno::Any aRet = xFunctionAccess->callFunction(FunctionName,aParamTemp);
    // MATCH function should alwayse return a double value, but currently if the first argument is XCellRange, MATCH function returns an array instead of a double value. Don't know why?
    // To fix this issue in safe, current solution is to convert this array to a double value just for MATCH function.
    String aUpper( FunctionName );
    ScCompiler aCompiler( NULL, ScAddress() );
    OpCode eOp = aCompiler.GetEnglishOpCode( aUpper.ToUpperAscii() );
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
    return aRet;
}

void SAL_CALL
ScVbaWSFunction::setValue(const rtl::OUString& /*PropertyName*/, const uno::Any& /*Value*/) throw(beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL
ScVbaWSFunction::getValue(const rtl::OUString& /*PropertyName*/) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL
ScVbaWSFunction::hasMethod(const rtl::OUString& Name)  throw(uno::RuntimeException)
{
    sal_Bool bIsFound = sal_False;
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
ScVbaWSFunction::hasProperty(const rtl::OUString& /*Name*/)  throw(uno::RuntimeException)
{
     return sal_False;
}

::rtl::OUString SAL_CALL
ScVbaWSFunction::getExactName( const ::rtl::OUString& aApproximateName ) throw (css::uno::RuntimeException)
{
    rtl::OUString sName = aApproximateName.toAsciiUpperCase();
    if ( !hasMethod( sName ) )
        return rtl::OUString();
    return sName;
}

rtl::OUString&
ScVbaWSFunction::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaWSFunction") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaWSFunction::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.WorksheetFunction" ) );
    }
    return aServiceNames;
}
