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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_VIEWSHELLWRAPPER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_VIEWSHELLWRAPPER_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XRelocatableResource.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>

#include <memory>

namespace {

typedef ::cppu::WeakComponentImplHelper    <   ::com::sun::star::lang::XUnoTunnel
                                            ,   ::com::sun::star::awt::XWindowListener
                                            ,   ::com::sun::star::view::XSelectionSupplier
                                            ,   ::com::sun::star::drawing::framework::XRelocatableResource
                                            ,   ::com::sun::star::drawing::framework::XView
                                            >   ViewShellWrapperInterfaceBase;

} // end of anonymous namespace.

namespace sd { class ViewShell; }
namespace sd { namespace slidesorter { class SlideSorterViewShell; } }

namespace sd { namespace framework {

/** This class wraps ViewShell objects and makes them look like an XView.
    Most importantly it provides a tunnel to the ViewShell implementation.
    Then it forwards size changes of the pane window to the view shell.
*/
class ViewShellWrapper  :private sd::MutexOwner
                        ,public ViewShellWrapperInterfaceBase
{
public:
    /** Create a new ViewShellWrapper object that wraps the given ViewShell
        object.
        @param pViewShell
            The ViewShell object to wrap.
        @param rsViewURL
            URL of the view type of the wrapped view shell.
        @param rxWindow
            This window reference is optional.  When a valid reference is
            given then size changes of the referenced window are forwarded
            to the ViewShell object.
    */
    ViewShellWrapper (
        ::std::shared_ptr<ViewShell> pViewShell,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxViewId,
        const ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow>& rxWindow);
    virtual ~ViewShellWrapper();

    virtual void SAL_CALL disposing() override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

    /** This method is typically used together with the XUnoTunnel interface
        to obtain a pointer to the wrapped ViewShell object for a given
        XView object.
    */
    ::std::shared_ptr<ViewShell> GetViewShell() { return mpViewShell;}

    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XResource

    virtual ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>
        SAL_CALL getResourceId()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XRelocatableResource

    virtual sal_Bool SAL_CALL relocateToAnchor (
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResource>& xResource)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XWindowListener

    virtual void SAL_CALL windowResized(
        const ::com::sun::star::awt::WindowEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowMoved(
        const ::com::sun::star::awt::WindowEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowShown(
        const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowHidden(
        const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener

    virtual void SAL_CALL disposing(
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::std::shared_ptr< ViewShell >                                                            mpViewShell;
    ::std::shared_ptr< ::sd::slidesorter::SlideSorterViewShell >                              mpSlideSorterViewShell;
    const ::com::sun::star::uno::Reference< com::sun::star::drawing::framework::XResourceId >   mxViewId;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow >                             mxWindow;
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
