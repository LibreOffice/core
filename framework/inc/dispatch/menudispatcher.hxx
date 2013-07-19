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

#ifndef __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_

#include <classes/taskcreator.hxx>
#include <services/frame.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <classes/menumanager.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.h>

namespace framework{

/*-************************************************************************************************************//**
    We must save information about our listener and URL for listening.
    We implement this as a hashtable for strings.
*//*-*************************************************************************************************************/

typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  OUString         ,
                                                        OUStringHash,
                                                        std::equal_to< OUString > > IMPL_ListenerHashContainer;


/*-************************************************************************************************************//**
    @short          helper for desktop only(!) to create new tasks on demand for dispatches
    @descr          Use this class as member only! Never use it as baseclass.
                    XInterface will be ambigous and we hold a weakcss::uno::Reference to ouer OWNER - not to ouer SUPERCLASS!

    @implements     XInterface
                    XDispatch
                    XLoadEventListener
                    XFrameActionListener
                    XEventListener
    @base           ThreadHelpBase
                    OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class MenuDispatcher   :        // baseclasses
                                // Order is necessary for right initialization!
                                public ThreadHelpBase                       ,
                                // interfaces
                                public  ::cppu::WeakImplHelper2<
                                            css::frame::XDispatch           ,
                                            css::frame::XFrameActionListener >
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of ths class with needed information for work.

            @seealso    using at owner

            @param      "rxContext"  , css::uno::Reference to servicemanager for creation of new services
            @param      "xOwner"    , css::uno::Reference to our owner, the Desktop!!!
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        MenuDispatcher(    const   css::uno::Reference< css::uno::XComponentContext >& rxContext    ,
                            const   css::uno::Reference< css::frame::XFrame >&              xOwner      );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatch
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      dispatch URL with arguments
            @descr      Every dispatch create a new task. If load of URL failed task will deleted automaticly!

            @seealso    -

            @param      "aURL"          , URL to dispatch.
            @param      "seqArguments"  , list of optional arguments.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispatch( const   css::util::URL&                                     aURL            ,
                                        const   css::uno::Sequence< css::beans::PropertyValue >&    seqProperties   ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      add listener for state events
            @descr      You can add a listener to get information about status of dispatch: OK or Failed.

            @seealso    method loadFinished()
            @seealso    method loadCancelled()

            @param      "xControl"  , css::uno::Reference to a valid listener for state events.
            @param      "aURL"      , URL about listener will be informed, if something occurred.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addStatusListener(    const   css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const   css::util::URL&                                     aURL    ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      remove listener for state events
            @descr      You can remove a listener if information of dispatch isn't important for you any longer.

            @seealso    method loadFinished()
            @seealso    method loadCancelled()

            @param      "xControl"  , css::uno::Reference to a valid listener.
            @param      "aURL"      , URL on which listener has registered.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeStatusListener( const   css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const   css::util::URL&                                     aURL    ) throw( css::uno::RuntimeException );


        //---------------------------------------------------------------------------------------------------------
        //   XFrameActionListener
        //---------------------------------------------------------------------------------------------------------

        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw ( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      dispose current instance
            @descr      If service helper isn't required any longer call this method to release all used resources.

            @seealso    -

            @param      "aEvent", information about source of this event.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        This method is protected, because its not allowed to use an instance of this class as a member!
                        You MUST use a pointer.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~MenuDispatcher();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_setMenuBar( MenuBar* pMenuBar, sal_Bool bMenuFromResource = sal_False );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_setAccelerators( Menu* pMenu, const Accelerator& aAccel );

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::WeakReference< css::frame::XFrame >           m_xOwnerWeak        ;   /// css::uno::WeakReference to owner (Don't use a hard css::uno::Reference. Owner can't delete us then!)
        css::uno::Reference< css::uno::XComponentContext >      m_xContext          ;   /// factory shared with our owner to create new services!
        IMPL_ListenerHashContainer                              m_aListenerContainer;   /// hash table for listener at specified URLs
        sal_Bool                                                m_bAlreadyDisposed  ;   /// Protection against multiple disposing calls.
        sal_Bool                                                m_bActivateListener ;   /// dispatcher is listener for frame activation
        MenuManager*                                            m_pMenuManager      ;   /// menu manager controlling menu dispatches

};      //  class MenuDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
