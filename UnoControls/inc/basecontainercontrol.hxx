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

#ifndef _UNOCONTROLS_BASECONTAINERCONTROL_CTRL_HXX
#define _UNOCONTROLS_BASECONTAINERCONTROL_CTRL_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

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

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________
#include "basecontrol.hxx"

//____________________________________________________________________________________________________________
//  "namespaces"
//____________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO         ::com::sun::star::uno
#define CSS_AWT         ::com::sun::star::awt
#define CSS_LANG        ::com::sun::star::lang
#define CSS_CONTAINER   ::com::sun::star::container

//____________________________________________________________________________________________________________
//  structs, types, forwards
//____________________________________________________________________________________________________________

struct IMPL_ControlInfo
{
    CSS_UNO::Reference< CSS_AWT::XControl > xControl ;
    ::rtl::OUString                         sName    ;
};

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class BaseContainerControl  : public CSS_AWT::XControlModel
                            , public CSS_AWT::XControlContainer
                            , public BaseControl
{

//____________________________________________________________________________________________________________
//  public methods
//____________________________________________________________________________________________________________

public:

    //________________________________________________________________________________________________________
    //  construct/destruct
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

       BaseContainerControl( const CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory >& xFactory );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual ~BaseContainerControl();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Any SAL_CALL queryInterface(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XTypeProvider
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @descr      -

        @seealso    XTypeProvider

        @param      -

        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Type > SAL_CALL getTypes()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XAggregation
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Any SAL_CALL queryAggregation(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XControl
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL createPeer(
        const CSS_UNO::Reference< CSS_AWT::XToolkit >&      xToolkit ,
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >&   xParent
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setModel(
        const CSS_UNO::Reference< CSS_AWT::XControlModel >& xModel
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XControlModel > SAL_CALL getModel()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XComponent
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XEventListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disposing( const CSS_LANG::EventObject& rEvent ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XControlContainer
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addControl(
        const ::rtl::OUString&                            sName    ,
        const CSS_UNO::Reference< CSS_AWT::XControl >&    xControl
    ) throw( CSS_UNO::RuntimeException  );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addContainerListener(
        const CSS_UNO::Reference< CSS_CONTAINER::XContainerListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeControl(
        const CSS_UNO::Reference< CSS_AWT::XControl >& xControl
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeContainerListener(
        const CSS_UNO::Reference< CSS_CONTAINER::XContainerListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setStatusText(
        const ::rtl::OUString& sStatusText
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XControl > SAL_CALL getControl(
        const ::rtl::OUString& sName
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Reference< CSS_AWT::XControl > > SAL_CALL getControls()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XUnoControlContainer
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addTabController(
        const CSS_UNO::Reference< CSS_AWT::XTabController >& xTabController
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeTabController(
        const CSS_UNO::Reference< CSS_AWT::XTabController >& xTabController
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setTabControllers(
        const CSS_UNO::Sequence< CSS_UNO::Reference< CSS_AWT::XTabController > >& xTabControllers
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Reference< CSS_AWT::XTabController > > SAL_CALL getTabControllers()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XWindow
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( CSS_UNO::RuntimeException );

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

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

    virtual CSS_AWT::WindowDescriptor* impl_getWindowDescriptor(
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >& xParentPeer
    );

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
        const   CSS_UNO::Reference< CSS_AWT::XGraphics >&   xGraphics
    );

//____________________________________________________________________________________________________________
//  private methods
//____________________________________________________________________________________________________________

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

//____________________________________________________________________________________________________________
//  private variables
//____________________________________________________________________________________________________________

private:
    // list of pointer of "struct IMPL_ControlInfo" to hold child-controls
    ::std::vector< IMPL_ControlInfo* > maControlInfoList;

    // list of references of XTabController to hold tab-order in this container
    CSS_UNO::Sequence< CSS_UNO::Reference< CSS_AWT::XTabController > >  m_xTabControllerList    ;

    ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListeners            ;

};  // class BaseContainerControl

// The namespace aliases are only used in this header
#undef CSS_UNO
#undef CSS_AWT
#undef CSS_LANG
#undef CSS_CONTAINER

}   // namespace unocontrols

#endif  // ifndef _UNOCONTROLS_BASECONTAINERCONTROL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
