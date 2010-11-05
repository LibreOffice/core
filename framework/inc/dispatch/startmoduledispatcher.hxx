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

#ifndef __FRAMEWORK_DISPATCH_STARTMODULEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_STARTMODULEDISPATCHER_HXX_

//_______________________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>
#include <general.h>

//_______________________________________________
// interface includes
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

//_______________________________________________
// other includes
#include <cppuhelper/weak.hxx>
#include <vcl/evntpost.hxx>

//_______________________________________________
// namespace

namespace framework{

//-----------------------------------------------
/**
    @short          helper to handle all URLs related to the StartModule
 */
class StartModuleDispatcher : public css::lang::XTypeProvider
                            , public css::frame::XNotifyingDispatch             // => XDispatch
                            , public css::frame::XDispatchInformationProvider
                            // baseclasses ... order is neccessary for right initialization!
                            , private ThreadHelpBase
                            , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short reference to an uno service manager,
                   which can be used to create own needed
                   uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  our "context" frame. */
        css::uno::WeakReference< css::frame::XFrame > m_xOwner;

        //---------------------------------------
        /** @short  the original queryDispatch() target. */
        ::rtl::OUString m_sDispatchTarget;

        //---------------------------------------
        /** @short  list of registered status listener */
        ListenerHash m_lStatusListener;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        /** @short  connect a new StartModuleDispatcher instance to its "owner frame".

            @descr  Such "owner frame" is used as context for all related operations.

            @param  xSMGR
                    an uno service manager, which is needed to create uno resource
                    internaly.

            @param  xFrame
                    the frame where the corresponding dispatch was started.

            @param  sTarget
                    the original target information used for the related queryDispatch() call.
         */
        StartModuleDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                              const css::uno::Reference< css::frame::XFrame >&              xFrame ,
                              const ::rtl::OUString&                                        sTarget);

        //---------------------------------------
        /** @short  does nothing real. */
        virtual ~StartModuleDispatcher();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //---------------------------------------
        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatchInformationProvider
        virtual css::uno::Sequence< sal_Int16 >                       SAL_CALL getSupportedCommandGroups         (                         ) throw (css::uno::RuntimeException);
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) throw (css::uno::RuntimeException);

    //-------------------------------------------
    // internal helper

    private:

        //---------------------------------------
        /** @short  check if StartModule can be shown.
         */
        ::sal_Bool implts_isBackingModePossible();

        //---------------------------------------
        /** @short  open the special BackingComponent (now StartModule)

            @return [bool]
                    sal_True if operation was successfully.
         */
        ::sal_Bool implts_establishBackingMode();

        //---------------------------------------
        /** @short  notify a DispatchResultListener.

            @descr  We check the listener reference before we use it.
                    So this method can be called everytimes!

            @parama xListener
                    the listener, which should be notified.
                    Can be null!

            @param  nState
                    directly used as css::frame::DispatchResultState value.

            @param  aResult
                    not used yet realy ...
         */
        void implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                               sal_Int16                                                   nState   ,
                                         const css::uno::Any&                                              aResult  );

}; // class StartModuleDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_STARTMODULEDISPATCHER_HXX_
