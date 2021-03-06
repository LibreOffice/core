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

#pragma once

#include <MutexOwner.hxx>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XRelocatableResource.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/compbase.hxx>

#include <memory>

namespace sd { class ViewShell; }
namespace sd::slidesorter { class SlideSorterViewShell; }
namespace com::sun::star::awt { class XWindow; }

namespace sd::framework {

typedef ::cppu::WeakComponentImplHelper    <   css::lang::XUnoTunnel
                                            ,   css::awt::XWindowListener
                                            ,   css::view::XSelectionSupplier
                                            ,   css::drawing::framework::XRelocatableResource
                                            ,   css::drawing::framework::XView
                                            >   ViewShellWrapperInterfaceBase;

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
        const ::std::shared_ptr<ViewShell>& pViewShell,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::awt::XWindow>& rxWindow);
    virtual ~ViewShellWrapper() override;

    virtual void SAL_CALL disposing() override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();

    /** This method is typically used together with the XUnoTunnel interface
        to obtain a pointer to the wrapped ViewShell object for a given
        XView object.
    */
    const ::std::shared_ptr<ViewShell>& GetViewShell() const { return mpViewShell;}

    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const css::uno::Sequence<sal_Int8>& rId) override;

    // XResource

    virtual css::uno::Reference<css::drawing::framework::XResourceId>
        SAL_CALL getResourceId() override;

    virtual sal_Bool SAL_CALL isAnchorOnly() override;

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection() override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    // XRelocatableResource

    virtual sal_Bool SAL_CALL relocateToAnchor (
        const css::uno::Reference<
            css::drawing::framework::XResource>& xResource) override;

    // XWindowListener

    virtual void SAL_CALL windowResized(
        const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved(
        const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown(
        const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden(
        const css::lang::EventObject& rEvent) override;

    // XEventListener

    virtual void SAL_CALL disposing(
        const css::lang::EventObject& rEvent) override;

private:
    ::std::shared_ptr< ViewShell >                                      mpViewShell;
    ::std::shared_ptr< ::sd::slidesorter::SlideSorterViewShell >        mpSlideSorterViewShell;
    const css::uno::Reference< css::drawing::framework::XResourceId >   mxViewId;
    css::uno::Reference<css::awt::XWindow >                             mxWindow;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
