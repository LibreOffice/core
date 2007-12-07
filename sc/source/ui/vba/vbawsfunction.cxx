/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbawsfunction.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:07:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

using namespace com::sun::star;
using namespace org::openoffice;

ScVbaWSFunction::ScVbaWSFunction( const uno::Reference< vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext): ScVbaWSFunction_BASE( xParent, xContext )
{
    m_xNameAccess.set(  mxContext->getServiceManager()->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.FunctionDescriptions") ), mxContext ), uno::UNO_QUERY_THROW );
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
    return xFunctionAccess->callFunction(FunctionName,aParamTemp);
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
        if ( m_xNameAccess->hasByName( Name ) )
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.WorksheetFunction" ) );
    }
    return aServiceNames;
}
