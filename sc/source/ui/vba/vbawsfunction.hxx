/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbawsfunction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:07:56 $
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
#ifndef SC_VBA_INTERIOR_HXX
#define SC_VBA_INTERIOR_HXX

#include <org/openoffice/excel/XWorksheetFunction.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "vbarange.hxx"

#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

typedef InheritedHelperInterfaceImpl1< oo::excel::XWorksheetFunction > ScVbaWSFunction_BASE;

class ScVbaWSFunction :  public ScVbaWSFunction_BASE
{
    css::uno::Reference< css::container::XNameAccess > m_xNameAccess;
public:
    ScVbaWSFunction( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~ScVbaWSFunction(){}

    virtual css::uno::Reference< css::beans::XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL invoke(const rtl::OUString& FunctionName, const css::uno::Sequence< css::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< css::uno::Any >& OutParam) throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void  SAL_CALL setValue(const rtl::OUString& PropertyName, const css::uno::Any& Value) throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL getValue(const rtl::OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasMethod(const rtl::OUString& Name)  throw(css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasProperty(const rtl::OUString& Name)  throw(css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getExactName( const ::rtl::OUString& aApproximateName ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};
#endif

