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

#ifndef __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_
#define __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_

#include <classes/filtercache.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>


#include <com/sun/star/frame/XSessionManagerListener2.hpp>
#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <cppuhelper/implbase4.hxx>

namespace framework{

/// @HTML
/** @short  implements flat/deep detection of file/stream formats and provides
            further read/write access to the global office type configuration.

    @descr  Using of this class makes it possible to get information about the
            format type of a given URL or stream. The returned internal type name
            can be used to get more information about this format. Further this
            class provides full access to the configuration data and following
            implementations will support some special query modes.

    @author     as96863

    @docdate    10.03.2003 by as96863

    @todo       <ul>
                    <li>implementation of query mode</li>
                    <li>simple restore mechanism of last consistent cache state,
                        if flush failed</li>
                </ul>
 */

class SessionListener :
                        // baseclasses (order important for initialization!)
                        // Struct for right initalization of mutex member! Must be the first one of baseclasses!
                        private ThreadHelpBase,
                        // interfaces
                        public cppu::WeakImplHelper4<
                            css::lang::XInitialization,
                            css::frame::XSessionManagerListener2,
                            css::frame::XStatusListener,
                            css::lang::XServiceInfo>
{
    //-------------------------------------------
    // member

    private:

        /** reference to the uno service manager, which created this service.
            It can be used to create own needed helper services. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        css::uno::Reference< css::frame::XSessionManagerClient > m_rSessionManager;

        // restore handling
        sal_Bool m_bRestored;

        sal_Bool m_bSessionStoreRequested;

        sal_Bool m_bAllowUserInteractionOnQuit;
        sal_Bool m_bTerminated;


        // in case of synchronous call the caller should do saveDone() call himself!
        void StoreSession( sal_Bool bAsync );

        // let session quietly close the documents, remove lock files, store configuration and etc.
        void QuitSessionQuietly();

    public:

        //---------------------------------------
        // XInterface, XTypeProvider, XServiceInfo

        DECLARE_XSERVICEINFO

        //---------------------------------------

        /** @short  initialize new instance of this class.

            @param  xSMGR
                    reference to the global uno service manager, which created this new
                    factory instance. It must be used during runtime to create own
                    needed services.
         */

        SessionListener( const css::uno::Reference< css::uno::XComponentContext >& xContext );

        virtual ~SessionListener();

        virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject&) throw (css::uno::RuntimeException);


        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any  >& args) throw (css::uno::RuntimeException);

        // XSessionManagerListener
        virtual void SAL_CALL doSave( sal_Bool bShutdown, sal_Bool bCancelable )
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL approveInteraction( sal_Bool bInteractionGranted )
            throw (css::uno::RuntimeException);
       virtual void SAL_CALL shutdownCanceled()
            throw (css::uno::RuntimeException);
       virtual sal_Bool SAL_CALL doRestore()
            throw (css::uno::RuntimeException);

        // XSessionManagerListener2
        virtual void SAL_CALL doQuit()
            throw (::com::sun::star::uno::RuntimeException);

       // XStatusListener
       virtual void SAL_CALL statusChanged(const com::sun::star::frame::FeatureStateEvent& event)
           throw (css::uno::RuntimeException);

        void doSaveImpl( sal_Bool bShutdown, sal_Bool bCancelable ) throw (css::uno::RuntimeException);
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
