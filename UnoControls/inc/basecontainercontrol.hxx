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
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl;
    OUString                                sName;
};

class BaseContainerControl  : public ::com::sun::star::awt::XControlModel
                            , public ::com::sun::star::awt::XControlContainer
                            , public BaseControl
{

//  public methods

public:

    //  construct/destruct

    /**_______________________________________________________________________________________________________
    */

       BaseContainerControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    /**_______________________________________________________________________________________________________
    */

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

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XTypeProvider

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XAggregation

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XControl

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL createPeer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >&      xToolkit ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >&   xParent
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XComponent

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XEventListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XControlContainer

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addControl(
        const OUString&                                   sName    ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >&    xControl
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception  ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xControl
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setStatusText(
        const OUString& sStatusText
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL getControl(
        const OUString& sName
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL getControls()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  XWindow

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

//  protected methods

protected:
    using OComponentHelper::disposing;
    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
    ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void impl_paint(
        sal_Int32                                           nX ,
        sal_Int32                                           nY ,
        const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >&   xGraphics
    ) SAL_OVERRIDE;

//  private methods

private:

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_activateTabControllers();

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_cleanMemory();

//  private variables

private:
    // list of pointer of "struct IMPL_ControlInfo" to hold child-controls
    ::std::vector< IMPL_ControlInfo* > maControlInfoList;

    // list of references of XTabController to hold tab-order in this container
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > >  m_xTabControllerList;

    ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListeners;

};  // class BaseContainerControl

}

#endif // INCLUDED_UNOCONTROLS_INC_BASECONTAINERCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
