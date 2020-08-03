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

#include <ooo/vba/excel/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XPane.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <cppuhelper/implbase.hxx>

#include <vbahelper/vbawindowbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaWindowBase, ov::excel::XWindow > WindowImpl_BASE;

class ScVbaWindow : public WindowImpl_BASE
{
private:
    css::uno::Reference< ov::excel::XPane > m_xPane;

    void init();
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::beans::XPropertySet > getControllerProps() const;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::beans::XPropertySet > getFrameProps() const;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::awt::XDevice > getDevice() const;

protected:
    void SplitAtDefinedPosition( sal_Int32 nColumns, sal_Int32 nRows );

public:
    /// @throws css::uno::RuntimeException
    void Scroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft, bool bLargeScroll );

public:
    /// @throws css::uno::RuntimeException
    ScVbaWindow(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController );
    /// @throws css::uno::RuntimeException
    ScVbaWindow(
        const css::uno::Sequence< css::uno::Any >& aArgs,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XWindow
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL ActiveCell(  ) override;
     virtual css::uno::Reference< ov::excel::XPane > SAL_CALL ActivePane() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL ActiveSheet(  ) override;
    virtual void SAL_CALL setCaption( const css::uno::Any& _caption ) override;
    virtual css::uno::Any SAL_CALL getCaption() override;
    virtual sal_Bool SAL_CALL getDisplayGridlines() override;
    virtual void SAL_CALL setDisplayGridlines( sal_Bool _displaygridlines ) override;
    virtual sal_Bool SAL_CALL getDisplayHeadings() override;
    virtual void SAL_CALL setDisplayHeadings( sal_Bool _bDisplayHeadings ) override;
    virtual sal_Bool SAL_CALL getDisplayHorizontalScrollBar() override;
    virtual void SAL_CALL setDisplayHorizontalScrollBar( sal_Bool _bDisplayHorizontalScrollBar ) override;
    virtual sal_Bool SAL_CALL getDisplayOutline() override;
    virtual void SAL_CALL setDisplayOutline( sal_Bool _bDisplayOutline ) override;
    virtual sal_Bool SAL_CALL getDisplayVerticalScrollBar() override;
    virtual void SAL_CALL setDisplayVerticalScrollBar( sal_Bool _bDisplayVerticalScrollBar ) override;
    virtual sal_Bool SAL_CALL getDisplayWorkbookTabs() override;
    virtual void SAL_CALL setDisplayWorkbookTabs( sal_Bool _bDisplayWorkbookTabs ) override;
    virtual sal_Bool SAL_CALL getFreezePanes() override;
    virtual void SAL_CALL setFreezePanes( sal_Bool _bFreezePanes ) override;
    virtual sal_Bool SAL_CALL getSplit() override;
    virtual void SAL_CALL setSplit( sal_Bool _bSplit ) override;
    virtual sal_Int32 SAL_CALL getSplitColumn() override ;
    virtual void SAL_CALL setSplitColumn( sal_Int32 _splitcolumn ) override ;
    virtual double SAL_CALL getSplitHorizontal() override ;
    virtual void SAL_CALL setSplitHorizontal( double _splithorizontal ) override ;
    virtual sal_Int32 SAL_CALL getSplitRow() override ;
    virtual void SAL_CALL setSplitRow( sal_Int32 _splitrow ) override ;
    virtual double SAL_CALL getSplitVertical() override ;
    virtual void SAL_CALL setSplitVertical( double _splitvertical ) override ;
    virtual css::uno::Any SAL_CALL getScrollRow() override ;
    virtual void SAL_CALL setScrollRow( const css::uno::Any& _scrollrow ) override ;
    virtual css::uno::Any SAL_CALL getScrollColumn() override ;
    virtual void SAL_CALL setScrollColumn( const css::uno::Any& _scrollcolumn ) override ;
    virtual css::uno::Any SAL_CALL getView() override;
    virtual void SAL_CALL setView( const css::uno::Any& _view ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getVisibleRange() override;
    virtual css::uno::Any SAL_CALL getWindowState() override;
    virtual void SAL_CALL setWindowState( const css::uno::Any& _windowstate ) override;
    virtual css::uno::Any SAL_CALL getZoom() override;
    virtual void SAL_CALL setZoom(const css::uno::Any& _zoom) override;
    virtual double SAL_CALL getTabRatio() override ;
    virtual void SAL_CALL setTabRatio( double _tabratio ) override ;

    // Methods
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) override;
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) override;
    virtual css::uno::Any SAL_CALL SelectedSheets( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL ScrollWorkbookTabs( const css::uno::Any& Sheets, const css::uno::Any& Position ) override;
    virtual void SAL_CALL Activate(  ) override;
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& FileName, const css::uno::Any& RouteWorkBook ) override;
    virtual css::uno::Any SAL_CALL Selection(  ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL RangeSelection() override;
    virtual sal_Int32 SAL_CALL PointsToScreenPixelsX(sal_Int32 _points) override;
    virtual sal_Int32 SAL_CALL PointsToScreenPixelsY(sal_Int32 _points) override;
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any&To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) override;
    virtual void SAL_CALL PrintPreview( const css::uno::Any& EnableChanges ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
