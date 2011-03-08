/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef CHART2_CONFIG_COLOR_SCHEME_HXX
#define CHART2_CONFIG_COLOR_SCHEME_HXX

#include "ServiceMacros.hxx"
#include "ConfigItemListener.hxx"
#include "charttoolsdllapi.hxx"
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XColorScheme.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>


namespace chart
{

OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme > createConfigColorScheme(
    const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

namespace impl
{
class ChartConfigItem;
}

class ConfigColorScheme :
    public ConfigItemListener,
    public ::cppu::WeakImplHelper2<
        ::com::sun::star::chart2::XColorScheme,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit ConfigColorScheme(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    SAL_DLLPRIVATE virtual ~ConfigColorScheme();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ConfigColorScheme )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XColorScheme ____
    SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getColorByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ConfigItemListener ____
    SAL_DLLPRIVATE virtual void notify( const ::rtl::OUString & rPropertyName );

private:
    SAL_DLLPRIVATE void retrieveConfigColors();

    // member variables
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::std::auto_ptr< impl::ChartConfigItem >              m_apChartConfigItem;
    mutable ::com::sun::star::uno::Sequence< sal_Int64 >  m_aColorSequence;
    mutable sal_Int32                                     m_nNumberOfColors;
    bool                                                  m_bNeedsUpdate;
};

} // namespace chart

// CHART2_CONFIG_COLOR_SCHEME_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
