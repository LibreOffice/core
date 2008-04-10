/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Style.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART_STYLE_HXX
#define CHART_STYLE_HXX

// helper classes
#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include "ServiceMacros.hxx"

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyle.hpp>

namespace com { namespace sun { namespace star { namespace container {
    class XNameContainer;
}}}}

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper2<
    ::com::sun::star::style::XStyle,
    ::com::sun::star::lang::XServiceInfo > Style_Base;
}

class Style :
        public ::comphelper::OMutexAndBroadcastHelper,
        public ::comphelper::OPropertyContainer,
        public ::comphelper::OPropertyArrayUsageHelper< Style >,
        public impl::Style_Base
{
public:
    Style( const ::com::sun::star::uno::Reference<
               ::com::sun::star::container::XNameContainer > & xStyleFamiliyToAddTo );
    virtual ~Style();

    void RegisterProperties(
        const ::std::vector< ::com::sun::star::beans::Property > & rProperties,
        ::std::vector< ::com::sun::star::uno::Any > & rOutProperties );

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ style::XStyle ____
    virtual sal_Bool SAL_CALL isUserDefined() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isInUse() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getParentStyle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParentStyle( const ::rtl::OUString& aParentStyle ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // ____ container::XNamed (base of XStyle) ____
    virtual ::rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // ____ beans::XPropertySet ____
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameContainer > m_xStyleFamily;

    ::rtl::OUString    m_aName;
    ::rtl::OUString    m_aParentStyleName;
    sal_Bool           m_bUserDefined;
};

} //  namespace chart

// CHART_STYLE_HXX
#endif
