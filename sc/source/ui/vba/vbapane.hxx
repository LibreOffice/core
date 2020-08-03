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
#pragma once

#include <com/sun/star/sheet/XViewPane.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <ooo/vba/excel/XPane.hpp>
#include <vbahelper/vbahelper.hxx>

class ScVbaPane final : public cppu::WeakImplHelper< ov::excel::XPane >
{
public:
    /// @throws css::uno::RuntimeException
    ScVbaPane(
        const css::uno::Reference< ov::XHelperInterface >& rParent,
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::frame::XModel >& rModel,
        const css::uno::Reference< css::sheet::XViewPane >& rViewPane );

    // XPane attributes
    virtual sal_Int32 SAL_CALL getScrollColumn() override;
    virtual void SAL_CALL setScrollColumn( sal_Int32 _scrollcolumn ) override;
    virtual sal_Int32 SAL_CALL getScrollRow() override;
    virtual void SAL_CALL setScrollRow( sal_Int32 _scrollrow ) override;
     virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getVisibleRange() override;

    // XPane methods
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) override;
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) override;

private:
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::sheet::XViewPane > m_xViewPane;
    css::uno::WeakReference< ov::XHelperInterface > m_xParent;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
