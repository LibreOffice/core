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

#ifndef __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_
#define __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_

#include <classes/filtercache.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <general.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short      factory to create handler-objects
    @descr      These class can be used to create new handler for specified contents.
                We use cached values of our configuration to lay down, which handler match
                a given URL or handlername. (use service TypeDetection to do that)
                With a detected type name you can search a registered handler and create it with these factory.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XMultiServiceFactory
                XNameContainer
                XNameReplace
                XNameAccess
                XElementAccess
                XFlushable

    @base       ThreadHelpBase
                OWeakObject

    @devstatus  ready to use
    @threadsafe yes
*//*-*************************************************************************************************************/

class ContentHandlerFactory :   // interfaces
                        public  css::lang::XTypeProvider            ,
                        public  css::lang::XServiceInfo             ,
                        public  css::lang::XMultiServiceFactory     ,
                        public  css::container::XNameContainer      ,       // => XNameReplace => XNameAccess => XElementAccess
                        public  css::util::XFlushable               ,
                        // base classes
                        // Order is necessary for right initialization of it!
                        private ThreadHelpBase                      ,
                        private TransactionBase                     ,
                        public  ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                 ContentHandlerFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~ContentHandlerFactory(                                                                        );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XMultiServiceFactory
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance             ( const OUString&                        sTypeName   ) throw( css::uno::Exception        ,
                                                                                                                                                                     css::uno::RuntimeException );
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString&                        sTypeName   ,
                                                                                                  const css::uno::Sequence< css::uno::Any >&    lArguments  ) throw( css::uno::Exception, css::uno::RuntimeException );
        virtual css::uno::Sequence< OUString >       SAL_CALL getAvailableServiceNames   (                                                           ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XNameContainer
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL insertByName( const OUString&  sHandlerName        ,
                                            const css::uno::Any&    aHandlerProperties  ) throw( css::lang::IllegalArgumentException     ,
                                                                                                 css::container::ElementExistException   ,
                                                                                                 css::lang::WrappedTargetException       ,
                                                                                                 css::uno::RuntimeException              );
        virtual void SAL_CALL removeByName( const OUString&  sHandlerName        ) throw( css::container::NoSuchElementException  ,
                                                                                                 css::lang::WrappedTargetException       ,
                                                                                                 css::uno::RuntimeException              );

        //---------------------------------------------------------------------------------------------------------
        //  XNameReplace
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL replaceByName( const OUString& sHandlerName       ,
                                             const css::uno::Any&   aHandlerProperties ) throw( css::lang::IllegalArgumentException    ,
                                                                                                css::container::NoSuchElementException ,
                                                                                                css::lang::WrappedTargetException      ,
                                                                                                css::uno::RuntimeException             );

        //---------------------------------------------------------------------------------------------------------
        //  XNameAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any                         SAL_CALL getByName      ( const OUString& sName ) throw( css::container::NoSuchElementException ,
                                                                                                                      css::lang::WrappedTargetException      ,
                                                                                                                      css::uno::RuntimeException             );
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(                              ) throw( css::uno::RuntimeException             );
        virtual sal_Bool                              SAL_CALL hasByName      ( const OUString& sName ) throw( css::uno::RuntimeException             );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );
        virtual sal_Bool       SAL_CALL hasElements   () throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XFlushable
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL flush              (                                                                   ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL addFlushListener   ( const css::uno::Reference< css::util::XFlushListener >& xListener ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL removeFlushListener( const css::uno::Reference< css::util::XFlushListener >& xListener ) throw ( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory              ;
        FilterCache                                                 m_aCache                ;

};      //  class ContentHandlerFactory

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_CONTENTHANDLERFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
