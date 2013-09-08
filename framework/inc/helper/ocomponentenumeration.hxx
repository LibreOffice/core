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

#ifndef __FRAMEWORK_HELPER_OCOMPONENTENUMERATION_HXX_
#define __FRAMEWORK_HELPER_OCOMPONENTENUMERATION_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/implbase2.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          implement a helper for a oneway enumeration of components
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
    @threadsafe     yes
*//*-*************************************************************************************************************/

class OComponentEnumeration :   public ThreadHelpBase               ,
                                public ::cppu::WeakImplHelper2< ::com::sun::star::container::XEnumeration,::com::sun::star::lang::XEventListener >
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

            @param      "seqComponents" is a sequence of interfaces, which are components.
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OComponentEnumeration( const css::uno::Sequence< css::uno::Reference< css::lang::XComponent > >& seqComponents );

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      last chance to release all references and free memory
            @descr      This method is called, if the enumeration is used completely and has no more elements.
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
            @return     A Reference to a component, safed in an Any-structure.

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

        virtual ~OComponentEnumeration();

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

    private:

        static sal_Bool impldbg_checkParameter_OComponentEnumerationCtor    (   const   css::uno::Sequence< css::uno::Reference< css::lang::XComponent > >& seqComponents   );
        static sal_Bool impldbg_checkParameter_disposing                    (   const   css::lang::EventObject&                                             aEvent          );

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_uInt32                                                              m_nPosition         ;   /// current position in enumeration
        css::uno::Sequence< css::uno::Reference< css::lang::XComponent > >      m_seqComponents     ;   /// list of current components

};      //  class OComponentEnumeration

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OCOMPONENTENUMERATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
