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

#ifndef INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FRAMELOADERFACTORY_HXX
#define INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FRAMELOADERFACTORY_HXX

#include "basecontainer.hxx"
#include <com/sun/star/frame/XLoaderFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>


namespace filter{
    namespace config{




/** @short      implements the service <type scope="com.sun.star.document">FrameLoaderFactory</type>.
 */
class FrameLoaderFactory : public ::cppu::ImplInheritanceHelper< BaseContainer                   ,
                                                                  css::frame::XLoaderFactory >
{

    // native interface

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    public:


        // ctor/dtor

        /** @short  standard ctor to connect this interface wrapper to
                    the global filter cache instance ...

            @param  rxContext
                    reference to the uno service manager, which created this service instance.
         */
        FrameLoaderFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext);



        /** @short  standard dtor.
         */
        virtual ~FrameLoaderFactory();


    // uno interface

    public:


        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(const OUString& sLoader)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const OUString&                     sLoader   ,
                                                                                                 const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // static uno helper!

    public:



        /** @short  return the uno implementation name of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix uno implementation name of this class.
         */
        static OUString impl_getImplementationName();



        /** @short  return the list of supported uno services of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix list of uno services supported by this class.
         */
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames();



        /** @short  return a new intsnace of this class.

            @descr  This method is used by the uno service manager, to create
                    a new instance of this service if needed.

            @param  xSMGR
                    reference to the uno service manager, which require
                    this new instance. It should be passed to the new object
                    so it can be used internally to create own needed uno resources.

            @return The new instance of this service as an uno reference.
         */
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

      // Overrides to resolve ambiguity
      virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::getByName(aName); }
      virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::getElementNames(); }
      virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::hasByName(aName); }

      virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::getElementType(); }
      virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::hasElements(); }

      virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery( const ::rtl::OUString& Query ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::createSubSetEnumerationByQuery(Query); }
      virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByProperties( const css::uno::Sequence< css::beans::NamedValue >& Properties ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return BaseContainer::createSubSetEnumerationByProperties(Properties); }

};

    } // namespace config
} // namespace filter

#endif // INCLUDED_FILTER_SOURCE_CONFIG_CACHE_FRAMELOADERFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
