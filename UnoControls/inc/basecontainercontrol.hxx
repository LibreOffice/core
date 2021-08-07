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

#include <memory>
#include <vector>

#include "basecontrol.hxx"
#include <com/sun/star/awt/XControlContainer.hpp>
#include <comphelper/multicontainer2.hxx>

namespace unocontrols {

struct IMPL_ControlInfo
{
    css::uno::Reference< css::awt::XControl > xControl;
    OUString                                  sName;
};

class BaseContainerControl  : public css::awt::XControlModel
                            , public css::awt::XControlContainer
                            , public BaseControl
{
public:

    BaseContainerControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~BaseContainerControl() override;

    //  XInterface

    /**
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType
    ) override;

    //  XTypeProvider

    /**
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    //  XAggregation

    virtual css::uno::Any SAL_CALL queryAggregation(
        const css::uno::Type& aType
    ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit >&      xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer >&   xParent
    ) override;

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    //  XEventListener

    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) override;

    //  XControlContainer

    virtual void SAL_CALL addControl(
        const OUString&                                     sName    ,
        const css::uno::Reference< css::awt::XControl >&    xControl
    ) override;

    virtual void SAL_CALL removeControl(
        const css::uno::Reference< css::awt::XControl >& xControl
    ) override;

    virtual void SAL_CALL setStatusText(
        const OUString& sStatusText
    ) override;

    virtual css::uno::Reference< css::awt::XControl > SAL_CALL getControl(
        const OUString& sName
    ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControl > > SAL_CALL getControls() override;

    //  XWindow

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) override;

protected:
    using OComponentHelper::disposing;

    virtual css::awt::WindowDescriptor impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    ) override;


    virtual void impl_paint(
        sal_Int32                                           nX ,
        sal_Int32                                           nY ,
        const   css::uno::Reference< css::awt::XGraphics >&   xGraphics
    ) override;

private:
    // list of pointer of "struct IMPL_ControlInfo" to hold child-controls
    ::std::vector< IMPL_ControlInfo > maControlInfoList;

    comphelper::OMultiTypeInterfaceContainerHelper2   m_aListeners;

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
