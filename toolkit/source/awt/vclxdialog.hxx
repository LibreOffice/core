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

#ifndef LAYOUT_AWT_VCLXDIALOG_HXX
#define LAYOUT_AWT_VCLXDIALOG_HXX

#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <comphelper/uno3.hxx>
#include <layout/core/bin.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>

namespace layoutimpl
{

typedef ::cppu::ImplHelper1 < ::com::sun::star::awt::XDialog2 > VCLXDialog_Base;

class TOOLKIT_DLLPUBLIC VCLXDialog : public VCLXWindow
                                   , public VCLXTopWindow_Base
                                   , public VCLXDialog_Base
                                   , public Bin
{
private:
    bool bRealized, bResizeSafeguard;
    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;

    VCLXDialog( const VCLXDialog& );            // never implemented
    VCLXDialog& operator=( const VCLXDialog& ); // never implemented

protected:
    Window* GetWindowImpl();
    ::cppu::OInterfaceContainerHelper& GetTopWindowListenersImpl();

    ~VCLXDialog();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    void resizedCb();

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &/*aIds*/ )
    {
    }
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }


public:
    VCLXDialog();

    // ::com::sun::star::awt::XDialog2
    void SAL_CALL endDialog( sal_Int32 nResult ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setHelpId( const rtl::OUString& id ) throw(::com::sun::star::uno::RuntimeException);

};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXDIALOG_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
