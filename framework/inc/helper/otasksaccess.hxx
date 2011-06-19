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

#ifndef __FRAMEWORK_HELPER_OTASKSACCESS_HXX_
#define __FRAMEWORK_HELPER_OTASKSACCESS_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <classes/framecontainer.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
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
    @short          implement XEnumerationAccess interface as helper to create many oneway enumeration of tasks
    @descr          We share mutex and framecontainer with ouer owner and have full access to his child tasks.
                    (Ouer owner can be the Desktop only!) We create oneway enumerations on demand. These "lists"
                    can be used for one time only. Step during the list from first to last element.
                    (The type of created enumerations is OTasksEnumeration.)

    @implements     XInterface
                    XEnumerationAccess
                    XElementAccess

    @base           OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class OTasksAccess  :   public css::lang::XTypeProvider             ,
                        public css::container::XEnumerationAccess   ,   // => XElementAccess
                        private ThreadHelpBase                      ,
                        public ::cppu::OWeakObject
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
                        snapshot of the tasklist of current tasks of desktop.
                        But we need a instance to create more then one enumerations to the same tasklist!

            @seealso    class Desktop
            @seealso    class OTasksEnumeration

            @param      "xOwner" is a reference to ouer owner and must be the desktop!
            @param      "pTasks" is a pointer to the taskcontainer of the desktop. We need it to create a new enumeration.
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OTasksAccess(  const   css::uno::Reference< css::frame::XDesktop >&    xOwner  ,
                                FrameContainer*                                 pTasks  );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //---------------------------------------------------------------------------------------------------------
        //  XEnumerationAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new enumeration of tasks
            @descr      You can call this method to get a new snapshot to all tasks of the desktop as an enumeration.

            @seealso    interface XEnumerationAccess
            @seealso    interface XEnumeration
            @seealso    class Desktop

            @param      -
            @return     If the desktop and some tasks exist => a valid reference to an enumeration<BR>
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
            @seealso    class TasksEnumeration

            @param      -
            @return     The uno-type XTask.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      get state of tasklist of enumeration.
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

        virtual ~OTasksAccess();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

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

        static sal_Bool impldbg_checkParameter_OTasksAccessCtor(    const   css::uno::Reference< css::frame::XDesktop >&    xOwner  ,
                                                                            FrameContainer*                                 pTasks  );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::WeakReference< css::frame::XDesktop >     m_xOwner            ;   /// weak reference to the desktop object!
        FrameContainer*                                     m_pTasks            ;   /// pointer to list of current tasks on desktop (is a member of class Desktop!)
                                                                                    /// This pointer is valid only, if weakreference can be locked.

};      //  class OTasksAccess

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OTASKSACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
