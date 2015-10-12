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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX

#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow2.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <cppuhelper/implbase1.hxx>

#include <toolkit/awt/vclxcontainer.hxx>

typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XTopWindow2
                            >   VCLXTopWindow_XBase;
typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSystemDependentWindowPeer
                            >   VCLXTopWindow_SBase;

class TOOLKIT_DLLPUBLIC VCLXTopWindow_Base  :public VCLXTopWindow_XBase
                                            ,public VCLXTopWindow_SBase
{
private:
    const bool  m_bWHWND;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar> mxMenuBar;


    virtual vcl::Window* GetWindowImpl() = 0;
    virtual ::cppu::OInterfaceContainerHelper& GetTopWindowListenersImpl() = 0;

    VCLXTopWindow_Base( const bool _bSupportSystemWindowPeer );

public:
    virtual ~VCLXTopWindow_Base();

    // XInterface equivalents
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    // XTypeProvider equivalents
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XSystemDependentWindowPeer
    ::com::sun::star::uno::Any SAL_CALL getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL toFront() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL toBack() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTopWindow2
    virtual sal_Bool SAL_CALL getIsMaximized() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIsMaximized( sal_Bool _ismaximized ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getIsMinimized() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIsMinimized( sal_Bool _isminimized ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getDisplay() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDisplay( ::sal_Int32 _display ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;
};


//  class VCLXTopWindow


class TOOLKIT_DLLPUBLIC VCLXTopWindow: public VCLXTopWindow_Base,
                     public VCLXContainer
{
protected:
    virtual vcl::Window* GetWindowImpl() override;
    virtual ::cppu::OInterfaceContainerHelper& GetTopWindowListenersImpl() override;

public:
    VCLXTopWindow(bool bWHWND = false);
    virtual ~VCLXTopWindow();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};




#endif // INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
