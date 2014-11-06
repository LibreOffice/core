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

#ifndef INCLUDED_AVMEDIA_SOURCE_FRAMEWORK_SOUNDHANDLER_HXX
#define INCLUDED_AVMEDIA_SOURCE_FRAMEWORK_SOUNDHANDLER_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/weak.hxx>

#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <avmedia/mediawindow.hxx>
#include <osl/mutex.hxx>

namespace avmedia{

struct ThreadHelpBase
{
    public:
        mutable ::osl::Mutex m_aLock;
};

/*-************************************************************************************************************
    @short          handler to detect and play sounds ("wav" and "au" only!)
    @descr          Register this implementation as a content handler to detect and/or play wav- and au-sounds.
                    It doesn't depend from the target platform. But one instance of this class
                    can play one sound at the same time only. Means every new dispatch request will stop the
                    might still running one. So we support one operation/one URL/one listener at the same time
                    only.

    @devstatus      ready
    @threadsafe     yes
*//*-*************************************************************************************************************/
class SoundHandler  :   // interfaces
                        public  css::lang::XTypeProvider
                    ,   public  css::lang::XServiceInfo
                    ,   public  css::frame::XNotifyingDispatch // => XDispatch
                    ,   public  css::document::XExtendedFilterDetection
                        // baseclasses
                        // Order is necessary for right initialization!
                    ,   private ThreadHelpBase
                    ,   public  ::cppu::OWeakObject
{
    // public methods
    public:

        // constructor / destructor
                 SoundHandler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~SoundHandler(                                                                        );

        //  XInterface, XTypeProvider, XServiceInfo
        virtual css::uno::Any  SAL_CALL queryInterface( const css::uno::Type& aType   ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;
        virtual css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes () throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /* interface XServiceInfo */
       virtual OUString                                               SAL_CALL getImplementationName              (                                                                               ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
       virtual sal_Bool                                               SAL_CALL supportsService                    ( const OUString&                                               sServiceName    ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
       virtual css::uno::Sequence< OUString >                         SAL_CALL getSupportedServiceNames           (                                                                               ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    /* Helper for XServiceInfo */
       static css::uno::Sequence< OUString >                          SAL_CALL impl_getStaticSupportedServiceNames(                                                                               );
       static OUString                                                SAL_CALL impl_getStaticImplementationName   (                                                                               );
    /* Helper for registry */
       static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception );
       static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
    /* Helper for initialization of service by using own reference! */
       virtual void                                                   SAL_CALL impl_initService                   (                                                                               );

        //  XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        //  XDispatch
        virtual void SAL_CALL dispatch              (   const   css::util::URL&                                     aURL        ,
                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lArguments  ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        // not supported !
        virtual void SAL_CALL addStatusListener     (   const   css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                        const   css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE {};
        virtual void SAL_CALL removeStatusListener  (   const   css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                        const   css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE {};

        //  XExtendedFilterDetection
        virtual OUString SAL_CALL detect           (           css::uno::Sequence< css::beans::PropertyValue >&    lDescriptor ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //  protected methods
    protected:

    //  private methods
    private:
        DECL_LINK( implts_PlayerNotify, void* );

    //  variables
    //  (should be private everyway!)
    private:

        bool m_bError;
        css::uno::Reference< css::lang::XMultiServiceFactory >     m_xFactory          ;   // global uno service factory to create new services
        css::uno::Reference< css::uno::XInterface >                m_xSelfHold         ;   // we must protect us against dying during async(!) dispatch() call!
        css::uno::Reference< css::media::XPlayer >                 m_xPlayer           ;   // uses avmedia player to play sounds...

        css::uno::Reference< css::frame::XDispatchResultListener > m_xListener         ;
        Timer m_aUpdateTimer;

};      //  class SoundHandler

}       //  namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_FRAMEWORK_SOUNDHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
