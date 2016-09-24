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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPANE_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPANE_HXX

#include <com/sun/star/sheet/XViewPane.hpp>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XPane.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include "excelvbahelper.hxx"

class ScVbaPane : public cppu::WeakImplHelper< ov::excel::XPane >
{
public:
    ScVbaPane(
        const css::uno::Reference< ov::XHelperInterface >& rParent,
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::frame::XModel >& rModel,
        const css::uno::Reference< css::sheet::XViewPane >& rViewPane ) throw (css::uno::RuntimeException);

    // XPane attributes
    virtual sal_Int32 SAL_CALL getScrollColumn() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setScrollColumn( sal_Int32 _scrollcolumn ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getScrollRow() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setScrollRow( sal_Int32 _scrollrow ) throw (css::uno::RuntimeException, std::exception) override;
     virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getVisibleRange() throw (css::uno::RuntimeException, std::exception) override;

    // XPane methods
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::sheet::XViewPane > m_xViewPane;

private:
    css::uno::WeakReference< ov::XHelperInterface > m_xParent;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
