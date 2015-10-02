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

typedef ::cppu::ImplHelper1 <   css::awt::XTopWindow2
                            >   VCLXTopWindow_XBase;
typedef ::cppu::ImplHelper1 <   css::awt::XSystemDependentWindowPeer
                            >   VCLXTopWindow_SBase;

class TOOLKIT_DLLPUBLIC VCLXTopWindow_Base  :public VCLXTopWindow_XBase
                                            ,public VCLXTopWindow_SBase
{
private:
    const bool  m_bWHWND;

protected:
    css::uno::Reference< css::awt::XMenuBar> mxMenuBar;


    virtual vcl::Window* GetWindowImpl() = 0;
    virtual ::comphelper::OInterfaceContainerHelper2& GetTopWindowListenersImpl() = 0;

    VCLXTopWindow_Base( const bool _bSupportSystemWindowPeer );

public:
    virtual ~VCLXTopWindow_Base();

    // XInterface equivalents
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider equivalents
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

    // css::awt::XSystemDependentWindowPeer
    css::uno::Any SAL_CALL getWindowHandle( const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL toFront() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL toBack() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& xMenu ) throw(css::uno::RuntimeException, std::exception) override;

    // XTopWindow2
    virtual sal_Bool SAL_CALL getIsMaximized() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIsMaximized( sal_Bool _ismaximized ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getIsMinimized() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIsMinimized( sal_Bool _isminimized ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getDisplay() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDisplay( ::sal_Int32 _display ) throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;
};


//  class VCLXTopWindow


class TOOLKIT_DLLPUBLIC VCLXTopWindow: public VCLXTopWindow_Base,
                     public VCLXContainer
{
protected:
    virtual vcl::Window* GetWindowImpl() override;
    virtual ::comphelper::OInterfaceContainerHelper2& GetTopWindowListenersImpl() override;

public:
    VCLXTopWindow(bool bWHWND = false);
    virtual ~VCLXTopWindow() override;

    // css::uno::XInterface
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC() override  { OWeakObject::acquire(); }
    void SAL_CALL release() SAL_THROW_IfNotObjectiveC() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;
    css::uno::Sequence< sal_Int8 >       SAL_CALL getImplementationId() SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
