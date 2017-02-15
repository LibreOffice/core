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
#ifndef INCLUDED_CHART2_SOURCE_INC_CONFIGCOLORSCHEME_HXX
#define INCLUDED_CHART2_SOURCE_INC_CONFIGCOLORSCHEME_HXX

#include "charttoolsdllapi.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XColorScheme.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{

OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::chart2::XColorScheme > createConfigColorScheme(
    const css::uno::Reference< css::uno::XComponentContext > & xContext );

namespace impl
{
class ChartConfigItem;
}

class ConfigColorScheme :
    public ::cppu::WeakImplHelper<
        css::chart2::XColorScheme,
        css::lang::XServiceInfo >
{
public:
    explicit ConfigColorScheme( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~ConfigColorScheme() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ ConfigItemListener ____
    void notify( const OUString & rPropertyName );

protected:
    // ____ XColorScheme ____
    virtual ::sal_Int32 SAL_CALL getColorByIndex( ::sal_Int32 nIndex ) override;

private:
    void retrieveConfigColors();

    // member variables
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    std::unique_ptr< impl::ChartConfigItem >            m_apChartConfigItem;
    mutable css::uno::Sequence< sal_Int64 >               m_aColorSequence;
    mutable sal_Int32                                     m_nNumberOfColors;
    bool                                                  m_bNeedsUpdate;
};

} // namespace chart

// INCLUDED_CHART2_SOURCE_INC_CONFIGCOLORSCHEME_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
