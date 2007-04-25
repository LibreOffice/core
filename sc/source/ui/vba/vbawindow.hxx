/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbawindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:12:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_WINDOW_HXX
#define SC_VBA_WINDOW_HXX
#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/excel/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "vbahelper.hxx"
#include "vbaworkbook.hxx"

typedef ::cppu::WeakImplHelper1<oo::excel::XWindow > WindowImpl_BASE;

class ScVbaWindow : public WindowImpl_BASE
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XModel > m_xModel;
    void  Scroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft, bool bLargeScroll = false ) throw (css::uno::RuntimeException);
public:
    ScVbaWindow( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel ) : m_xContext(xContext), m_xModel( xModel ) {}
    // XWindow
    virtual void SAL_CALL setCaption( const css::uno::Any& _caption ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getScrollRow() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setScrollRow( const css::uno::Any& _scrollrow ) throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getScrollColumn() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setScrollColumn( const css::uno::Any& _scrollcolumn ) throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getWindowState() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWindowState( const css::uno::Any& _windowstate ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SelectedSheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScrollWorkbookTabs( const css::uno::Any& Sheets, const css::uno::Any& Position ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& FileName, const css::uno::Any& RouteWorkBook ) throw (css::uno::RuntimeException);
};

#endif //SC_VBA_WINDOW_HXX
