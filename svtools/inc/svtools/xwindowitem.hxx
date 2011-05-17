/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#ifndef _XWINDOWITEM_HXX_
#define _XWINDOWITEM_HXX_


#include "svtools/svtdllapi.h"

#include <svl/poolitem.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/XWindow.hpp>

class Window;

//////////////////////////////////////////////////////////////////////

class SVT_DLLPUBLIC XWindowItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xWin;

    // disallow use assignment operator
    XWindowItem & operator = ( const XWindowItem & );

public:
    TYPEINFO();
    XWindowItem();
    explicit XWindowItem( sal_uInt16 nWhich, Window * pWin );
    XWindowItem( sal_uInt16 nWhich, com::sun::star::uno::Reference< com::sun::star::awt::XWindow > & rxWin );
    XWindowItem( const XWindowItem &rItem );
    ~XWindowItem();

    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual int operator == ( const SfxPoolItem& rAttr ) const;

    Window *        GetWindowPtr() const    { return VCLUnoHelper::GetWindow( m_xWin ); }
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >  GetXWindow() const  { return m_xWin; }
};

//////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
