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
#include "querytokenizer.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>


namespace filter::config {


/** @short      implements the service <type scope="com.sun.star.document">FilterFactory</type>.
 */
class FilterFactory : public ::cppu::ImplInheritanceHelper< BaseContainer                   ,
                                                             css::lang::XMultiServiceFactory >
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
        explicit FilterFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext);


        /** @short  standard dtor.
         */
        virtual ~FilterFactory() override;


    // uno interface

    public:


        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(const OUString& sFilter) override;

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const OUString&                     sFilter   ,
                                                                                                 const css::uno::Sequence< css::uno::Any >& lArguments) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;


        // XContainerQuery

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const OUString& sQuery) override;


    // internal helper!

    private:


        /** @short  implement the container string query: "matchByDocumentService=:iflags=:eflags=:..."

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        std::vector<OUString> impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const;


        /** TODO document me
         */
        std::vector<OUString> impl_getListOfInstalledModules() const;


        /** @short  implement the container string query:
                    "getSortedFilterList()[:module=<xxx>]:[iflags=<xxx>][:eflags=<xxx>]"

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        std::vector<OUString> impl_getSortedFilterList(const QueryTokenizer& lTokens) const;


        /** TODO document me
         */
        std::vector<OUString> impl_getSortedFilterListForModule(const OUString& sModule,
                                                             sal_Int32        nIFlags,
                                                             sal_Int32        nEFlags) const;


        /** @short  read a specialized and sorted list of filter names from
                    the configuration (matching the specified module)

            @param  sModule
                    the module for which the sorted list should be retrieved for.

            @return A string list of internal filter names.
                    Can be empty.
         */
        std::vector<OUString> impl_readSortedFilterListFromConfig(const OUString& sModule) const;

};

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
