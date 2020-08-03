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

#pragma once

#include "basecontainer.hxx"
#include <com/sun/star/frame/XLoaderFactory.hpp>
#include <cppuhelper/implbase.hxx>


namespace filter::config {


/** @short      implements the service <type scope="com.sun.star.document">ContentHandlerFactory</type>.
 */
class ContentHandlerFactory : public ::cppu::ImplInheritanceHelper< BaseContainer                   ,
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
        explicit ContentHandlerFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext);


        /** @short  standard dtor.
         */
        virtual ~ContentHandlerFactory() override;


    // uno interface

    public:


        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(const OUString& sHandler) override;

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const OUString&                     sHandler  ,
                                                                                                 const css::uno::Sequence< css::uno::Any >& lArguments) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;


    public:

      // Overrides to resolve ambiguity
      virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override
        { return BaseContainer::getByName(aName); }
      virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override
        { return BaseContainer::getElementNames(); }
      virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
        { return BaseContainer::hasByName(aName); }

      virtual css::uno::Type SAL_CALL getElementType() override
        { return BaseContainer::getElementType(); }
      virtual sal_Bool SAL_CALL hasElements() override
        { return BaseContainer::hasElements(); }

      virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery( const OUString& Query ) override
        { return BaseContainer::createSubSetEnumerationByQuery(Query); }
      virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByProperties( const css::uno::Sequence< css::beans::NamedValue >& Properties ) override
        { return BaseContainer::createSubSetEnumerationByProperties(Properties); }


};

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
