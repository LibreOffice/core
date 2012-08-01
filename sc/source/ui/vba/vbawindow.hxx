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
#ifndef SC_VBA_WINDOW_HXX
#define SC_VBA_WINDOW_HXX

#include <ooo/vba/excel/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/sheet/XViewSplitable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XPane.hpp>
#include <com/sun/star/awt/XDevice.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbawindowbase.hxx>
#include "vbaworkbook.hxx"

typedef cppu::ImplInheritanceHelper1< VbaWindowBase, ov::excel::XWindow > WindowImpl_BASE;

class ScVbaWindow : public WindowImpl_BASE
{
private:
    css::uno::Reference< ov::excel::XPane > m_xPane;

    void init();
    css::uno::Reference< css::beans::XPropertySet > getControllerProps() throw (css::uno::RuntimeException);
    css::uno::Reference< css::beans::XPropertySet > getFrameProps() throw (css::uno::RuntimeException);
    css::uno::Reference< css::awt::XDevice > getDevice() throw (css::uno::RuntimeException);

protected:
    void SplitAtDefinedPosition(sal_Bool _bUnFreezePane);

public:
    void Scroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft, bool bLargeScroll = false ) throw (css::uno::RuntimeException);

public:
    ScVbaWindow(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController )
        throw (css::uno::RuntimeException);
    ScVbaWindow(
        const css::uno::Sequence< css::uno::Any >& aArgs,
        const css::uno::Reference< css::uno::XComponentContext >& xContext )
        throw (css::uno::RuntimeException);

    // XWindow
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL ActiveCell(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
     virtual css::uno::Reference< ov::excel::XPane > SAL_CALL ActivePane() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL ActiveSheet(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const css::uno::Any& _caption ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayGridlines() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayGridlines( ::sal_Bool _displaygridlines ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayHeadings() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayHeadings( ::sal_Bool _bDisplayHeadings ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayHorizontalScrollBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayHorizontalScrollBar( ::sal_Bool _bDisplayHorizontalScrollBar ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayOutline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayOutline( ::sal_Bool _bDisplayOutline ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayVerticalScrollBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayVerticalScrollBar( ::sal_Bool _bDisplayVerticalScrollBar ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayWorkbookTabs() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayWorkbookTabs( ::sal_Bool _bDisplayWorkbookTabs ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getFreezePanes() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFreezePanes( ::sal_Bool _bFreezePanes ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getSplit() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSplit( ::sal_Bool _bSplit ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSplitColumn() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setSplitColumn( sal_Int32 _splitcolumn ) throw (css::uno::RuntimeException) ;
    virtual double SAL_CALL getSplitHorizontal() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setSplitHorizontal( double _splithorizontal ) throw (css::uno::RuntimeException) ;
    virtual sal_Int32 SAL_CALL getSplitRow() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setSplitRow( sal_Int32 _splitrow ) throw (css::uno::RuntimeException) ;
    virtual double SAL_CALL getSplitVertical() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setSplitVertical( double _splitvertical ) throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getScrollRow() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setScrollRow( const css::uno::Any& _scrollrow ) throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getScrollColumn() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setScrollColumn( const css::uno::Any& _scrollcolumn ) throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getView() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setView( const css::uno::Any& _view ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getVisibleRange() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getWindowState() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWindowState( const css::uno::Any& _windowstate ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getZoom() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setZoom( const css::uno::Any& _zoom ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTabRatio() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setTabRatio( double _tabratio ) throw (css::uno::RuntimeException) ;

    // Methods
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SelectedSheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScrollWorkbookTabs( const css::uno::Any& Sheets, const css::uno::Any& Position ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& FileName, const css::uno::Any& RouteWorkBook ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Selection(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL RangeSelection() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL PointsToScreenPixelsX(sal_Int32 _points) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL PointsToScreenPixelsY(sal_Int32 _points) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any&To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL PrintPreview( const css::uno::Any& EnableChanges ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
