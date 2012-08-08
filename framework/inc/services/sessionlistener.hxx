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

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>

namespace framework{

/// @HTML
/** @short  implements flat/deep detection of file/stream formats and provides
            further read/write access to the global office type configuration.

    @descr  Using of this class makes it possible to get information about the
            format type of a given URL or stream. The returned internal type name
            can be used to get more informations about this format. Further this
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
/// @NOHTML

class SessionListener :   // interfaces
                        public css::lang::XTypeProvider,
                        public css::lang::XInitialization,
                        public css::frame::XSessionManagerListener2,
                        public css::frame::XStatusListener,
                        public css::lang::XServiceInfo,
                        // baseclasses (order important for initialization!)
                        // Struct for right initalization of mutex member! Must be the first one of baseclasses!
                        private ThreadHelpBase,
                        public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        /** reference to the uno service manager, which created this service.
            It can be used to create own needed helper services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

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

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------

        /** @short  initialize new instance of this class.

            @param  xSMGR
                    reference to the global uno service manager, which created this new
                    factory instance. It must be used during runtime to create own
                    needed services.
         */

        SessionListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

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

#endif // #ifndef __FRAMEWORK_SERVICES_TYPEDETECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
