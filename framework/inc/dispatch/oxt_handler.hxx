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

#ifndef __FRAMEWORK_HANDLER_OXT_HANDLER_HXX_
#define __FRAMEWORK_HANDLER_OXT_HANDLER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>

#include <cppuhelper/implbase3.hxx>


namespace framework{

/*-************************************************************************************************************//**
    @short          handler to detect and play sounds ("wav" and "au" only!)
    @descr          Register this implementation as a content handler to detect and/or play wav- and au-sounds.
                    It doesn't depend from the target platform. But one instance of this class
                    can play one sound at the same time only. Means every new dispatch request will stop the
                    might still running one. So we support one operation/one URL/one listener at the same time
                    only.

    @devstatus      ready
    @threadsafe     yes
*//*-*************************************************************************************************************/
class Oxt_Handler  :    // baseclasses
                        // Order is necessary for right initialization!
                        private ThreadHelpBase
                        // interfaces
                    ,   public  ::cppu::WeakImplHelper3<
                                    css::lang::XServiceInfo,
                                    css::frame::XNotifyingDispatch, // => XDispatch
                                    css::document::XExtendedFilterDetection >
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                 Oxt_Handler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~Oxt_Handler(                                                                        );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XNotifyingDispatch
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatch
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL dispatch              (  const css::util::URL&                                     aURL        ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&    lArguments  ) throw( css::uno::RuntimeException );
        // not supported !
        virtual void SAL_CALL addStatusListener     (  const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                       const css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException ) {};
        virtual void SAL_CALL removeStatusListener  (  const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                       const css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException ) {};

        //---------------------------------------------------------------------------------------------------------
        //  XExtendedFilterDetection
        //---------------------------------------------------------------------------------------------------------
        virtual OUString SAL_CALL detect     (        css::uno::Sequence< css::beans::PropertyValue >&    lDescriptor ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >     m_xFactory          ;   /// global uno service factory to create new services
        css::uno::Reference< css::uno::XInterface >                m_xSelfHold         ;   /// we must protect us against dieing during async(!) dispatch() call!
        css::uno::Reference< css::frame::XDispatchResultListener > m_xListener         ;

};      //  class Oxt_Handler

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HANDLER_OXT_HANDLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
