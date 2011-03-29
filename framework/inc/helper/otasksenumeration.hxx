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

#ifndef __FRAMEWORK_HELPER_OTASKSENUMERATION_HXX_
#define __FRAMEWORK_HELPER_OTASKSENUMERATION_HXX_

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
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XTask.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

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
    @short          implement a helper for a oneway enumeration of tasks
    @descr          You can step during this list only for one time! Its a snapshot.
                    Don't forget to release the reference. You are the owner of an instance of this implementation.
                    You cant use this as a baseclass. Please use it as a dynamical object for return.

    @implements     XInterface
                    XTypeProvider
                    XEventListener
                    XEnumeration

    @base           ThreadHelpBase
                    OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class OTasksEnumeration :   public css::lang::XTypeProvider     ,
                            public css::lang::XEventListener    ,
                            public css::container::XEnumeration ,
                            public ThreadHelpBase                   ,
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
            @short      constructor to initialize this enumeration
            @descr      An enumeration is a list with oneway-access! You can get every member only for one time.
                        This method allow to initialize this oneway list with values.

            @seealso    -

            @param      "seqTasks" is a sequence of interfaces, which are tasks.
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OTasksEnumeration( const css::uno::Sequence< css::uno::Reference< css::frame::XFrame > >& seqTasks );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      last chance to release all references and free memory
            @descr      This method is called, if the enumeration is used completly and has no more elements.
                        Then we must destroy ouer list and release all references to other objects.

            @seealso    interface XEventListener

            @param      "aEvent" describe the source of this event.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XEnumeration
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      check count of accessible elements of enumeration
            @descr      You can call this method to get information about accessible elements in future.
                        Elements you have already getted are not accessible!

            @seealso    interface XEnumeration

            @param      -
            @return     sal_True  = if more elements accessible<BR>
                        sal_False = other way

            @onerror    sal_False<BR>
                        (List is emtpy and there no accessible elements ...)
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasMoreElements() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      give the next element, if some exist
            @descr      If a call "hasMoreElements()" return true, you can get the next element of list.

            @seealso    interface XEnumeration

            @param      -
            @return     A Reference to a task, safed in an Any-structure.

            @onerror    If end of enumeration is arrived or there are no elements in list => a NoSuchElementException is thrown.
        *//*-*****************************************************************************************************/

        virtual css::uno::Any SAL_CALL nextElement() throw( css::container::NoSuchElementException  ,
                                                             css::lang::WrappedTargetException      ,
                                                            css::uno::RuntimeException              );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        We make it protected, because its not supported to use this class as normal instance!
                        You must create it dynamical in memory and use a pointer.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~OTasksEnumeration();

        /*-****************************************************************************************************//**
            @short      reset instance to default values

            @descr      There are two ways to delete an instance of this class.<BR>
                        1) delete with destructor<BR>
                        2) dispose from parent or factory ore ...<BR>
                        This method do the same for both ways! It free used memory and release references ...

            @seealso    method dispose()
            @seealso    destructor ~TaskEnumeration()

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void impl_resetObject();

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

            @seealso    ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool impldbg_checkParameter_OTasksEnumerationCtor    (   const   css::uno::Sequence< css::uno::Reference< css::frame::XFrame > >&    seqTasks    );
        static sal_Bool impldbg_checkParameter_disposing                (   const   css::lang::EventObject&                                             aEvent      );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_uInt32                                                          m_nPosition         ;   /// current position in enumeration
        css::uno::Sequence< css::uno::Reference< css::frame::XTask > >      m_seqTasks          ;   /// list of current tasks

};      //  class OTasksEnumeration

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OTASKSENUMERATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
