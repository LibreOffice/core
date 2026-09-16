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

#include <com/sun/star/sheet/XViewPane.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <ooo/vba/excel/XPane.hpp>
#include <vbahelper/vbahelper.hxx>

class ScVbaPane final : public cppu::WeakImplHelper< ov::excel::XPane >
{
public:
    /// @throws cpo::uno::RuntimeException
    ScVbaPane(
        const cpo::uno::Reference< ov::XHelperInterface >& rParent,
        cpo::uno::Reference< cpo::uno::XComponentContext > xContext,
        const cpo::uno::Reference< css::frame::XModel >& rModel,
        const cpo::uno::Reference< css::sheet::XViewPane >& rViewPane );

    // XPane attributes
    virtual sal_Int32 getScrollColumn() override;
    virtual void setScrollColumn( sal_Int32 _scrollcolumn ) override;
    virtual sal_Int32 getScrollRow() override;
    virtual void setScrollRow( sal_Int32 _scrollrow ) override;
     virtual cpo::uno::Reference< ov::excel::XRange > getVisibleRange() override;

    // XPane methods
    virtual void SmallScroll( const cpo::uno::Any& Down, const cpo::uno::Any& Up, const cpo::uno::Any& ToRight, const cpo::uno::Any& ToLeft ) override;
    virtual void LargeScroll( const cpo::uno::Any& Down, const cpo::uno::Any& Up, const cpo::uno::Any& ToRight, const cpo::uno::Any& ToLeft ) override;

private:
    cpo::uno::Reference< css::frame::XModel > m_xModel;
    cpo::uno::Reference< css::sheet::XViewPane > m_xViewPane;
    cpo::uno::WeakReference< ov::XHelperInterface > m_xParent;
    cpo::uno::Reference< cpo::uno::XComponentContext > m_xContext;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
