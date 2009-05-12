/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basecontainercontrol.hxx,v $
 * $Revision: 1.5 $
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
#include <tools/list.hxx>

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________
#include "basecontrol.hxx"

//____________________________________________________________________________________________________________
//  "namespaces"
//____________________________________________________________________________________________________________

namespace unocontrols{

#define UNO3_REFERENCE                              ::com::sun::star::uno::Reference
#define UNO3_XCONTROL                               ::com::sun::star::awt::XControl
#define UNO3_OUSTRING                               ::rtl::OUString
#define UNO3_XCONTROLMODEL                          ::com::sun::star::awt::XControlModel
#define UNO3_XCONTROLCONTAINER                      ::com::sun::star::awt::XControlContainer
#define UNO3_XMULTISERVICEFACTORY                   ::com::sun::star::lang::XMultiServiceFactory
#define UNO3_TYPE                                   ::com::sun::star::uno::Type
#define UNO3_RUNTIMEEXCEPTION                       ::com::sun::star::uno::RuntimeException
#define UNO3_XTOOLKIT                               ::com::sun::star::awt::XToolkit
#define UNO3_XWINDOWPEER                            ::com::sun::star::awt::XWindowPeer
#define UNO3_EVENTOBJECT                            ::com::sun::star::lang::EventObject
#define UNO3_SEQUENCE                               ::com::sun::star::uno::Sequence
#define UNO3_XCONTAINERLISTENER                     ::com::sun::star::container::XContainerListener
#define UNO3_ANY                                    ::com::sun::star::uno::Any
#define UNO3_XTABCONTROLLER                         ::com::sun::star::awt::XTabController
#define UNO3_WINDOWDESCRIPTOR                       ::com::sun::star::awt::WindowDescriptor
#define UNO3_XGRAPHICS                              ::com::sun::star::awt::XGraphics
#define UNO3_OMULTITYPEINTERFACECONTAINERHELPER     ::cppu::OMultiTypeInterfaceContainerHelper
#define UNO3_ILLEGALARGUMENTEXCEPTION               ::com::sun::star::lang::IllegalArgumentException

//____________________________________________________________________________________________________________
//  structs, types, forwards
//____________________________________________________________________________________________________________

struct IMPL_ControlInfo
{
    UNO3_REFERENCE< UNO3_XCONTROL >     xControl    ;
    UNO3_OUSTRING                       sName       ;
};

// makro define a list-class for struct IMPL_ControlInfo!
class IMPL_ControlInfoList ;
DECLARE_LIST( IMPL_ControlInfoList, IMPL_ControlInfo* )

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class BaseContainerControl  : public UNO3_XCONTROLMODEL
                            , public UNO3_XCONTROLCONTAINER
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

       BaseContainerControl( const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >& xFactory );

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

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getTypes() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual UNO3_ANY SAL_CALL queryAggregation( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL createPeer(   const   UNO3_REFERENCE< UNO3_XTOOLKIT >&    xToolkit    ,
                                        const   UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParent     ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setModel( const UNO3_REFERENCE< UNO3_XCONTROLMODEL >& xModel ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XCONTROLMODEL > SAL_CALL getModel() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL dispose() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL disposing( const UNO3_EVENTOBJECT& rEvent ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL addControl(   const   UNO3_OUSTRING&                      sName       ,
                                        const   UNO3_REFERENCE< UNO3_XCONTROL >&    xControl    ) throw( UNO3_RUNTIMEEXCEPTION  );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addContainerListener( const UNO3_REFERENCE< UNO3_XCONTAINERLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeControl( const UNO3_REFERENCE< UNO3_XCONTROL >& xControl ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeContainerListener( const UNO3_REFERENCE< UNO3_XCONTAINERLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setStatusText( const UNO3_OUSTRING& sStatusText ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XCONTROL > SAL_CALL getControl( const UNO3_OUSTRING& sName) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SEQUENCE< UNO3_REFERENCE< UNO3_XCONTROL > > SAL_CALL getControls() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL addTabController( const UNO3_REFERENCE< UNO3_XTABCONTROLLER >& xTabController ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeTabController( const UNO3_REFERENCE< UNO3_XTABCONTROLLER >& xTabController ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setTabControllers( const UNO3_SEQUENCE< UNO3_REFERENCE< UNO3_XTABCONTROLLER > >& xTabControllers ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SEQUENCE< UNO3_REFERENCE< UNO3_XTABCONTROLLER > > SAL_CALL getTabControllers() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual UNO3_WINDOWDESCRIPTOR* impl_getWindowDescriptor( const UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParentPeer );

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void impl_paint(        sal_Int32                           nX          ,
                                    sal_Int32                           nY          ,
                            const   UNO3_REFERENCE< UNO3_XGRAPHICS >&   xGraphics   );

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

    IMPL_ControlInfoList*                                   m_pControlInfoList      ;   /// list of pointer of "struct IMPL_ControlInfo" to hold child-controls
    UNO3_SEQUENCE< UNO3_REFERENCE< UNO3_XTABCONTROLLER > >  m_xTabControllerList    ;   /// list of references of XTabController to hold tab-order in this container
    UNO3_OMULTITYPEINTERFACECONTAINERHELPER                 m_aListeners            ;

};  // class BaseContainerControl

}   // namespace unocontrols

#endif  // ifndef _UNOCONTROLS_BASECONTAINERCONTROL_CTRL_HXX
