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

#ifndef INCLUDED_UNOCONTROLS_INC_BASECONTAINERCONTROL_HXX
#define INCLUDED_UNOCONTROLS_INC_BASECONTAINERCONTROL_HXX

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vector>

#include "basecontrol.hxx"

//  "namespaces"

namespace unocontrols{

//  structs, types, forwards

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

    virtual ~BaseContainerControl();

    //  XInterface

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType
    ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XTypeProvider

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XAggregation

    virtual css::uno::Any SAL_CALL queryAggregation(
        const css::uno::Type& aType
    ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit >&      xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer >&   xParent
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

    //  XEventListener

    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) throw( css::uno::RuntimeException, std::exception ) override;

    //  XControlContainer

    virtual void SAL_CALL addControl(
        const OUString&                                     sName    ,
        const css::uno::Reference< css::awt::XControl >&    xControl
    ) throw( css::uno::RuntimeException, std::exception  ) override;

    virtual void SAL_CALL removeControl(
        const css::uno::Reference< css::awt::XControl >& xControl
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setStatusText(
        const OUString& sStatusText
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::awt::XControl > SAL_CALL getControl(
        const OUString& sName
    ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControl > > SAL_CALL getControls()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XWindow

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( css::uno::RuntimeException, std::exception ) override;

protected:
    using OComponentHelper::disposing;

    virtual css::awt::WindowDescriptor* impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    ) override;


    virtual void impl_paint(
        sal_Int32                                           nX ,
        sal_Int32                                           nY ,
        const   css::uno::Reference< css::awt::XGraphics >&   xGraphics
    ) override;

private:

    void impl_activateTabControllers();

    void impl_cleanMemory();

    // list of pointer of "struct IMPL_ControlInfo" to hold child-controls
    ::std::vector< IMPL_ControlInfo* > maControlInfoList;

    // list of references of XTabController to hold tab-order in this container
    css::uno::Sequence< css::uno::Reference< css::awt::XTabController > >  m_xTabControllerList;

    ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListeners;

};  // class BaseContainerControl

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_INC_BASECONTAINERCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
