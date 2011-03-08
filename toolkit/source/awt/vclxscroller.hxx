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

#ifndef LAYOUT_AWT_VCLXSCROLLER_HXX
#define LAYOUT_AWT_VCLXSCROLLER_HXX

#include <comphelper/uno3.hxx>
#include <layout/core/bin.hxx>
#include <toolkit/awt/vclxwindow.hxx>

class ScrollBar;
class FixedImage;

namespace layoutimpl
{

class VCLXScroller :public VCLXWindow
                   ,public Bin
{
public:
    VCLXScroller();

protected:
    ~VCLXScroller();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const ::com::sun::star::awt::Rectangle &rArea )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

private:
    VCLXScroller( const VCLXScroller& );            // never implemented
    VCLXScroller& operator=( const VCLXScroller& ); // never implemented

    // because the underlying window is only setup-ed after construction, init
    // scrollbars at play-time
    void ensureScrollBars();

    FixedImage *mpContent;  // dummy
    ScrollBar *mpHorScrollBar, *mpVerScrollBar;
    DECL_LINK( ScrollHdl, ScrollBar* );
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXSCROLLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
