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

#ifndef INCLUDED_FRAMEWORK_INC_DISPATCH_POPUPMENUDISPATCHER_HXX
#define INCLUDED_FRAMEWORK_INC_DISPATCH_POPUPMENUDISPATCHER_HXX

#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.h>

namespace framework{

typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
    IMPL_ListenerHashContainer;

/*-************************************************************************************************************
    @short          helper for desktop only(!) to create new tasks on demand for dispatches
    @descr          Use this class as member only! Never use it as baseclass.
                    XInterface will be ambiguous and we hold a weakcss::uno::Reference to our OWNER - not to our SUPERCLASS!

    @implements     XInterface
                    XDispatch
                    XLoadEventListener
                    XFrameActionListener
                    XEventListener
    @base           OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class PopupMenuDispatcher :     public  ::cppu::WeakImplHelper<
                                           css::lang::XServiceInfo,
                                           css::frame::XDispatchProvider,
                                           css::frame::XDispatch,
                                           css::frame::XFrameActionListener,
                                           css::lang::XInitialization >
{

    //  public methods

    public:

        //  constructor / destructor
        PopupMenuDispatcher( const css::uno::Reference< css::uno::XComponentContext >& xContext );

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO_NOFACTORY
        /* Helper for registry */
        /// @throws css::uno::Exception
        static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
        static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) override;
        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(
            const css::util::URL&  aURL        ,
            const OUString& sTarget     ,
            sal_Int32              nFlags      ) override;

        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(
            const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) override;

        //  XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL&                                  aURL,
                                        const css::uno::Sequence< css::beans::PropertyValue >& seqProperties ) override;

        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                 const css::util::URL&                                     aURL ) override;

        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const css::util::URL&                                     aURL  ) override;

        //   XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) override;

        //   XEventListener
        void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

    //  protected methods
    protected:
        virtual ~PopupMenuDispatcher() override;

        void impl_RetrievePopupControllerQuery();


    //  variables
    private:
        css::uno::WeakReference< css::frame::XFrame >           m_xWeakFrame;   /// css::uno::WeakReference to frame (Don't use a hard css::uno::Reference. Owner can't delete us then!)
        css::uno::Reference< css::container::XNameAccess >      m_xPopupCtrlQuery;   /// reference to query for popup controller
        css::uno::Reference< css::uri::XUriReferenceFactory >   m_xUriRefFactory;   /// reference to the uri reference factory
        css::uno::Reference< css::uno::XComponentContext >      m_xContext;   /// factory shared with our owner to create new services!
        osl::Mutex                                              m_mutex;
        IMPL_ListenerHashContainer                              m_aListenerContainer;   /// hash table for listener at specified URLs
        bool                                                    m_bAlreadyDisposed;   /// Protection against multiple disposing calls.
        bool                                                    m_bActivateListener;   /// dispatcher is listener for frame activation

}; //  class PopupMenuDispatcher

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_DISPATCH_POPUPMENUDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
