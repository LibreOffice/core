/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include "BaseColorScheme.hxx"

#include <memory>

namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{

css::uno::Reference< css::chart2::XColorScheme > createConfigColorScheme(
    const css::uno::Reference< css::uno::XComponentContext > & xContext );

namespace impl
{
class ChartConfigItem;
}

class ConfigColorScheme final : public BaseColorScheme
{
public:
    explicit ConfigColorScheme( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~ConfigColorScheme() override;

    // ____ ConfigItemListener ____
    void notify();

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;

protected:
    // ____ XColorScheme ____
    virtual ::sal_Int32 SAL_CALL getColorByIndex( ::sal_Int32 nIndex ) override;

private:
    void retrieveConfigColors();

    // member variables
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    std::unique_ptr< impl::ChartConfigItem >            m_apChartConfigItem;
    bool                                                  m_bNeedsUpdate;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
