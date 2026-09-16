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

#include <ooo/vba/excel/XWindow.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XPane.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <cppuhelper/implbase.hxx>

#include <vbahelper/vbawindowbase.hxx>

class ScModelObj;

typedef cppu::ImplInheritanceHelper< VbaWindowBase, ov::excel::XWindow > WindowImpl_BASE;

class ScVbaWindow : public WindowImpl_BASE
{
private:
    rtl::Reference<ScModelObj> m_xModel;
    cpo::uno::Reference< ov::excel::XPane > m_xPane;

    void init();
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::beans::XPropertySet > getControllerProps() const;
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::beans::XPropertySet > getFrameProps() const;
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::awt::XDevice > getDevice() const;

protected:
    void SplitAtDefinedPosition( sal_Int32 nColumns, sal_Int32 nRows );

public:
    /// @throws cpo::uno::RuntimeException
    void Scroll( const cpo::uno::Any& Down, const cpo::uno::Any& Up, const cpo::uno::Any& ToRight, const cpo::uno::Any& ToLeft, bool bLargeScroll );

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaWindow(
        const cpo::uno::Reference< ov::XHelperInterface >& xParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const rtl::Reference< ScModelObj >& xModel,
        const cpo::uno::Reference< css::frame::XController >& xController );
    /// @throws cpo::uno::RuntimeException
    ScVbaWindow(
        const cpo::uno::Sequence< cpo::uno::Any >& aArgs,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext );

    // XWindow
    virtual cpo::uno::Reference< ov::excel::XRange > ActiveCell(  ) override;
     virtual cpo::uno::Reference< ov::excel::XPane > ActivePane() override;
    virtual cpo::uno::Reference< ov::excel::XWorksheet > ActiveSheet(  ) override;
    virtual void setCaption( const cpo::uno::Any& _caption ) override;
    virtual cpo::uno::Any getCaption() override;
    virtual bool getDisplayGridlines() override;
    virtual void setDisplayGridlines( bool _displaygridlines ) override;
    virtual bool getDisplayHeadings() override;
    virtual void setDisplayHeadings( bool _bDisplayHeadings ) override;
    virtual bool getDisplayHorizontalScrollBar() override;
    virtual void setDisplayHorizontalScrollBar( bool _bDisplayHorizontalScrollBar ) override;
    virtual bool getDisplayOutline() override;
    virtual void setDisplayOutline( bool _bDisplayOutline ) override;
    virtual bool getDisplayVerticalScrollBar() override;
    virtual void setDisplayVerticalScrollBar( bool _bDisplayVerticalScrollBar ) override;
    virtual bool getDisplayWorkbookTabs() override;
    virtual void setDisplayWorkbookTabs( bool _bDisplayWorkbookTabs ) override;
    virtual bool getFreezePanes() override;
    virtual void setFreezePanes( bool _bFreezePanes ) override;
    virtual bool getSplit() override;
    virtual void setSplit( bool _bSplit ) override;
    virtual sal_Int32 getSplitColumn() override ;
    virtual void setSplitColumn( sal_Int32 _splitcolumn ) override ;
    virtual double getSplitHorizontal() override ;
    virtual void setSplitHorizontal( double _splithorizontal ) override ;
    virtual sal_Int32 getSplitRow() override ;
    virtual void setSplitRow( sal_Int32 _splitrow ) override ;
    virtual double getSplitVertical() override ;
    virtual void setSplitVertical( double _splitvertical ) override ;
    virtual cpo::uno::Any getScrollRow() override ;
    virtual void setScrollRow( const cpo::uno::Any& _scrollrow ) override ;
    virtual cpo::uno::Any getScrollColumn() override ;
    virtual void setScrollColumn( const cpo::uno::Any& _scrollcolumn ) override ;
    virtual cpo::uno::Any getView() override;
    virtual void setView( const cpo::uno::Any& _view ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > getVisibleRange() override;
    virtual cpo::uno::Any getWindowState() override;
    virtual void setWindowState( const cpo::uno::Any& _windowstate ) override;
    virtual cpo::uno::Any getZoom() override;
    virtual void setZoom(const cpo::uno::Any& _zoom) override;
    virtual double getTabRatio() override ;
    virtual void setTabRatio( double _tabratio ) override ;

    // Methods
    virtual void SmallScroll( const cpo::uno::Any& Down, const cpo::uno::Any& Up, const cpo::uno::Any& ToRight, const cpo::uno::Any& ToLeft ) override;
    virtual void LargeScroll( const cpo::uno::Any& Down, const cpo::uno::Any& Up, const cpo::uno::Any& ToRight, const cpo::uno::Any& ToLeft ) override;
    virtual cpo::uno::Any SelectedSheets( const cpo::uno::Any& aIndex ) override;
    virtual void ScrollWorkbookTabs( const cpo::uno::Any& Sheets, const cpo::uno::Any& Position ) override;
    virtual void Activate(  ) override;
    virtual void Close( const cpo::uno::Any& SaveChanges, const cpo::uno::Any& FileName, const cpo::uno::Any& RouteWorkBook ) override;
    virtual cpo::uno::Any Selection(  ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > RangeSelection() override;
    virtual sal_Int32 PointsToScreenPixelsX(sal_Int32 _points) override;
    virtual sal_Int32 PointsToScreenPixelsY(sal_Int32 _points) override;
    virtual void PrintOut( const cpo::uno::Any& From, const cpo::uno::Any&To, const cpo::uno::Any& Copies, const cpo::uno::Any& Preview, const cpo::uno::Any& ActivePrinter, const cpo::uno::Any& PrintToFile, const cpo::uno::Any& Collate, const cpo::uno::Any& PrToFileName ) override;
    virtual void PrintPreview( const cpo::uno::Any& EnableChanges ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
