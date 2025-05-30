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

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/implbase.hxx>
#include <vector>

namespace framework{

/*-************************************************************************************************************
    @short          implement a helper for a oneway enumeration of components
    @descr          You can step during this list only for one time! It's a snapshot.
                    Don't forget to release the reference. You are the owner of an instance of this implementation.
                    You can't use this as a baseclass. Please use it as a dynamical object for return.

    @implements     XInterface
                    XTypeProvider
                    XEventListener
                    XEnumeration

    @base           OWeakObject

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/

class OComponentEnumeration final : public ::cppu::WeakImplHelper< css::container::XEnumeration,css::lang::XEventListener >
{

    //  public methods

    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      constructor to initialize this enumeration
            @descr      An enumeration is a list with oneway-access! You can get every member only for one time.
                        This method allow to initialize this oneway list with values.
            @param      "seqComponents" is a sequence of interfaces, which are components.
            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OComponentEnumeration( std::vector< css::uno::Reference< css::lang::XComponent > >&& seqComponents );

        //  XEventListener

        /*-****************************************************************************************************
            @short      last chance to release all references and free memory
            @descr      This method is called, if the enumeration is used completely and has no more elements.
                        Then we must destroy our list and release all references to other objects.

            @seealso    interface XEventListener

            @param      "aEvent" describe the source of this event.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

        //  XEnumeration

        /*-****************************************************************************************************
            @short      check count of accessible elements of enumeration
            @descr      You can call this method to get information about accessible elements in future.
                        Elements you have already gotten are not accessible!

            @seealso    interface XEnumeration
            @return     sal_True  = if more elements accessible<BR>
                        sal_False = other way

            @onerror    sal_False<BR>
                        (List is empty and there no accessible elements ...)
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasMoreElements() override;

        /*-****************************************************************************************************
            @short      give the next element, if some exist
            @descr      If a call "hasMoreElements()" return true, you can get the next element of list.

            @seealso    interface XEnumeration
            @return     A Reference to a component, saved in an Any-structure.

            @onerror    If end of enumeration is arrived or there are no elements in list => a NoSuchElementException is thrown.
        *//*-*****************************************************************************************************/

        virtual css::uno::Any SAL_CALL nextElement() override;

    //  protected methods

    private:

        /*-****************************************************************************************************
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        We make it protected, because it's not supported to use this class as normal instance!
                        You must create it dynamical in memory and use a pointer.
        *//*-*****************************************************************************************************/

        virtual ~OComponentEnumeration() override;

        /*-****************************************************************************************************
            @short      reset instance to default values

            @descr      There are two ways to delete an instance of this class.<BR>
                        1) delete with destructor<BR>
                        2) dispose from parent or factory or...<BR>
                        This method does the same for both ways! It frees used memory and releases references...

            @seealso    method dispose()
            @seealso    destructor ~TaskEnumeration()
        *//*-*****************************************************************************************************/

        void impl_resetObject();


    //  debug methods
    //  (should be private everyway!)

        /*-****************************************************************************************************
            @short      debug-method to check incoming parameter of some other methods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway
        *//*-*****************************************************************************************************/

        sal_uInt32                                                       m_nPosition;   /// current position in enumeration
        std::vector< css::uno::Reference< css::lang::XComponent > >      m_seqComponents;   /// list of current components

};      //  class OComponentEnumeration

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
