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

#ifndef __FRAMEWORK_HANDLER_OXT_HANDLER_HXX_
#define __FRAMEWORK_HANDLER_OXT_HANDLER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

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
class Oxt_Handler  :   // interfaces
                        public  css::lang::XTypeProvider
                    ,   public  css::lang::XServiceInfo
                    ,   public  css::frame::XNotifyingDispatch // => XDispatch
                    ,   public  css::document::XExtendedFilterDetection
                        // baseclasses
                        // Order is neccessary for right initialization!
                    ,   private ThreadHelpBase
                    ,   public  ::cppu::OWeakObject
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
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
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
        virtual ::rtl::OUString SAL_CALL detect     (        css::uno::Sequence< css::beans::PropertyValue >&    lDescriptor ) throw( css::uno::RuntimeException );

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
