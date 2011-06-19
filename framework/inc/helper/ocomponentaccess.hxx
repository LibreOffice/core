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

#ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
#define __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement XEnumerationAccess interface as helper to create many oneway enumeration of components
    @descr          We share mutex and framecontainer with ouer owner and have full access to his child tasks.
                    (Ouer owner can be the Desktop only!) We create oneway enumerations on demand. These "lists"
                    can be used for one time only. Step during the list from first to last element.
                    (The type of created enumerations is OComponentEnumeration.)

    @implements     XInterface
                    XTypeProvider
                    XEnumerationAccess
                    XElementAccess

    @base           ThreadHelpBase
                    OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class OComponentAccess  :   private ThreadHelpBase                      ,   // Must be the first of baseclasses - Is neccessary for right initialization of objects!
                            public ::cppu::WeakImplHelper1< ::com::sun::star::container::XEnumerationAccess >
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      constructor to initialize this instance
            @descr      A desktop will create an enumeration-access-object. An enumeration is a oneway-list and a
                        snapshot of the components of current tasks under the desktop.
                        But we need a instance to create more then one enumerations at different times!

            @seealso    class Desktop
            @seealso    class OComponentEnumeration

            @param      "xOwner" is a reference to ouer owner and must be the desktop!
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

        OComponentAccess( const css::uno::Reference< css::frame::XDesktop >& xOwner );

        //---------------------------------------------------------------------------------------------------------
        //  XEnumerationAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new enumeration of components
            @descr      You can call this method to get a new snapshot from all components of all tasks of the desktop as an enumeration.

            @seealso    interface XEnumerationAccess
            @seealso    interface XEnumeration
            @seealso    class Desktop

            @param      -
            @return     If the desktop and some components exist => a valid reference to an enumeration<BR>
                        An NULL-reference, other way.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      get the type of elements in enumeration
            @descr      -

            @seealso    interface XElementAccess
            @seealso    class OComponentEnumeration

            @param      -
            @return     The uno-type XComponent.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      get state of componentlist of enumeration.
            @descr      -

            @seealso    interface XElementAccess

            @param      -
            @return     sal_True  ,if more then 0 elements exist.
            @return     sal_False ,otherwise.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        Don't use an instance of this class as normal member. Use it dynamicly with a pointer.
                        We hold a weakreference to ouer owner and not to ouer superclass!
                        Thats the reason for a protected dtor.

            @seealso    class Desktop

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~OComponentAccess();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      recursive method (!) to collect all components of all frames from the subtree of given node
            @descr      This is neccessary to create the enumeration.

            @seealso    method createEnumeration

            @param      "xNode"        , root of subtree and start point of search
            @param      "seqComponents", result list of search. We cant use a return value, we search recursive
                                         and must collect all informations.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_collectAllChildComponents(    const   css::uno::Reference< css::frame::XFramesSupplier >&                 xNode           ,
                                                          css::uno::Sequence< css::uno::Reference< css::lang::XComponent > >&   seqComponents   );

        /*-****************************************************************************************************//**
            @short      get the component of a frame
            @descr      The component of a frame can be the window, the controller or the model.

            @seealso    method createEnumeration

            @param      "xFrame", frame which contains the component
            @return     A reference to the component of given frame.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        css::uno::Reference< css::lang::XComponent > impl_getFrameComponent( const css::uno::Reference< css::frame::XFrame >& xFrame ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter.
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool impldbg_checkParameter_OComponentAccessCtor( const css::uno::Reference< css::frame::XDesktop >& xOwner );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::WeakReference< css::frame::XDesktop >     m_xOwner    ;   /// weak reference to the desktop object!

};      //  class OComponentAccess

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
