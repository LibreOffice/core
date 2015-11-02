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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_OCOMPONENTACCESS_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_OCOMPONENTACCESS_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          implement XEnumerationAccess interface as helper to create many oneway enumeration of components
    @descr          We share mutex and framecontainer with our owner and have full access to his child tasks.
                    (Our owner can be the Desktop only!) We create oneway enumerations on demand. These "lists"
                    can be used for one time only. Step during the list from first to last element.
                    (The type of created enumerations is OComponentEnumeration.)

    @implements     XInterface
                    XTypeProvider
                    XEnumerationAccess
                    XElementAccess

    @base           OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class OComponentAccess  :   public ::cppu::WeakImplHelper< css::container::XEnumerationAccess >
{

    //  public methods

    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      constructor to initialize this instance
            @descr      A desktop will create an enumeration-access-object. An enumeration is a oneway-list and a
                        snapshot of the components of current tasks under the desktop.
                        But we need a instance to create more than one enumerations at different times!

            @seealso    class Desktop
            @seealso    class OComponentEnumeration

            @param      "xOwner" is a reference to our owner and must be the desktop!
            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

        OComponentAccess( const css::uno::Reference< css::frame::XDesktop >& xOwner );

        //  XEnumerationAccess

        /*-****************************************************************************************************
            @short      create a new enumeration of components
            @descr      You can call this method to get a new snapshot from all components of all tasks of the desktop as an enumeration.

            @seealso    interface XEnumerationAccess
            @seealso    interface XEnumeration
            @seealso    class Desktop
            @return     If the desktop and some components exist => a valid reference to an enumeration<BR>
                        An NULL-reference, other way.
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw( css::uno::RuntimeException, std::exception ) override;

        //  XElementAccess

        /*-****************************************************************************************************
            @short      get the type of elements in enumeration
            @seealso    interface XElementAccess
            @seealso    class OComponentEnumeration
            @return     The uno-type XComponent.
        *//*-*****************************************************************************************************/

        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException, std::exception ) override;

        /*-****************************************************************************************************
            @short      get state of componentlist of enumeration.
            @seealso    interface XElementAccess
            @return     sal_True  ,if more than 0 elements exist.
            @return     sal_False ,otherwise.
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException, std::exception ) override;

    //  protected methods

    protected:

        /*-****************************************************************************************************
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        Don't use an instance of this class as normal member. Use it dynamicly with a pointer.
                        We hold a weakreference to our owner and not to our superclass!
                        Thats the reason for a protected dtor.

            @seealso    class Desktop
        *//*-*****************************************************************************************************/

        virtual ~OComponentAccess();


    private:

        /*-****************************************************************************************************
            @short      recursive method (!) to collect all components of all frames from the subtree of given node
            @descr      This is necessary to create the enumeration.

            @seealso    method createEnumeration

            @param      "xNode"        , root of subtree and start point of search
            @param      "seqComponents", result list of search. We can't use a return value, we search recursive
                                         and must collect all information.
        *//*-*****************************************************************************************************/

        void impl_collectAllChildComponents(    const   css::uno::Reference< css::frame::XFramesSupplier >&                 xNode           ,
                                                          css::uno::Sequence< css::uno::Reference< css::lang::XComponent > >&   seqComponents   );

        /*-****************************************************************************************************
            @short      get the component of a frame
            @descr      The component of a frame can be the window, the controller or the model.

            @seealso    method createEnumeration

            @param      "xFrame", frame which contains the component
            @return     A reference to the component of given frame.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        css::uno::Reference< css::lang::XComponent > impl_getFrameComponent( const css::uno::Reference< css::frame::XFrame >& xFrame ) const;

    //  debug methods
    //  (should be private everyway!)

        /*-****************************************************************************************************
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter.
            @return     sal_True  ,otherwise
        *//*-*****************************************************************************************************/

    private:

        static bool impldbg_checkParameter_OComponentAccessCtor( const css::uno::Reference< css::frame::XDesktop >& xOwner );

    //  variables
    //  (should be private everyway!)

    private:

        css::uno::WeakReference< css::frame::XDesktop >     m_xOwner;   /// weak reference to the desktop object!

};      //  class OComponentAccess

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_HELPER_OCOMPONENTACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
