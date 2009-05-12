/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbapane.hxx,v $
 * $Revision: 1.3 $
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
#ifndef SC_VBA_PANE_HXX
#define SC_VBA_PANE_HXX

#include<cppuhelper/implbase1.hxx>
#include<com/sun/star/sheet/XViewPane.hpp>
#include<ooo/vba/excel/XPane.hpp>

#include"vbahelper.hxx"

typedef cppu::WeakImplHelper1< ov::excel::XPane > PaneImpl_Base;

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
