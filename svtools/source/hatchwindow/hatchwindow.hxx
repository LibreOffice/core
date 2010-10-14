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

#ifndef _SVT_HATCHWINDOW_HXX
#define _SVT_HATCHWINDOW_HXX

#include <com/sun/star/embed/XHatchWindow.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/typeprovider.hxx>

class SvResizeWindow;
class VCLXHatchWindow : public ::com::sun::star::embed::XHatchWindow,
                        public VCLXWindow
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XHatchWindowController > m_xController;
    ::com::sun::star::awt::Size aHatchBorderSize;
    SvResizeWindow* pHatchWindow;

public:
    VCLXHatchWindow();
    ~VCLXHatchWindow();

    void initializeWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
                const ::com::sun::star::awt::Rectangle& aBounds,
                const ::com::sun::star::awt::Size& aSize );

    void QueryObjAreaPixel( Rectangle & );
    void RequestObjAreaPixel( const Rectangle & );
    void InplaceDeactivate();
    void Activated();
    void Deactivated();

    // XInterface
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL acquire() throw();
    void SAL_CALL release() throw();

    // XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XHatchWindow
    virtual void SAL_CALL setController( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XHatchWindowController >& xController ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getHatchBorderSize() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHatchBorderSize( const ::com::sun::star::awt::Size& _hatchbordersize ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // _SVT_HATCHWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
