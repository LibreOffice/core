/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:58:17 $
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
#ifndef SC_VBA_PANE_HXX
#define SC_VBA_PANE_HXX

#include<cppuhelper/implbase1.hxx>
#include<com/sun/star/sheet/XViewPane.hpp>
#include<org/openoffice/excel/XPane.hpp>

#include"vbahelper.hxx"

typedef cppu::WeakImplHelper1< oo::excel::XPane > PaneImpl_Base;

class ScVbaPane : public PaneImpl_Base
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::sheet::XViewPane > m_xViewPane;
public:
    ScVbaPane( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XViewPane > xViewPane ) : m_xContext( xContext ), m_xViewPane( xViewPane ) {}

    css::uno::Reference< css::sheet::XViewPane > getViewPane() { return m_xViewPane; }

    //Attribute
    virtual sal_Int32 SAL_CALL getScrollColumn() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScrollColumn( sal_Int32 _scrollcolumn ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getScrollRow() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScrollRow( sal_Int32 _scrollrow ) throw (css::uno::RuntimeException);

    //Method
    virtual void SAL_CALL SmallScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL LargeScroll( const css::uno::Any& Down, const css::uno::Any& Up, const css::uno::Any& ToRight, const css::uno::Any& ToLeft ) throw (css::uno::RuntimeException);

};

#endif//SC_VBA_PANE_HXX
