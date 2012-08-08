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

#ifndef __FILTER_CONFIG_FILTERFACTORY_HXX_
#define __FILTER_CONFIG_FILTERFACTORY_HXX_

#include "basecontainer.hxx"
#include "querytokenizer.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/implbase1.hxx>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;


//_______________________________________________

/** @short      implements the service <type scope="com.sun.star.document">FilterFactory</type>.
 */
class FilterFactory : public ::cppu::ImplInheritanceHelper1< BaseContainer                   ,
                                                             css::lang::XMultiServiceFactory >
{
    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor to connect this interface wrapper to
                    the global filter cache instance ...

            @param  xSMGR
                    reference to the uno service manager, which created this service instance.
         */
        FilterFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~FilterFactory();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(const ::rtl::OUString& sFilter)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(const ::rtl::OUString&                     sFilter   ,
                                                                                                 const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XContainerQuery

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const ::rtl::OUString& sQuery)
            throw (css::uno::RuntimeException);

    //-------------------------------------------
    // internal helper!

    private:

        //---------------------------------------

        /** @short  implement the container string query: "matchByDocumentService=:iflags=:eflags=:..."

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        OUStringList impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const;

        //---------------------------------------

        /** TODO document me
         */
        OUStringList impl_getListOfInstalledModules() const;

        //---------------------------------------

        /** @short  implement the container string query:
                    "getSortedFilterList()[:module=<xxx>]:[iflags=<xxx>][:eflags=<xxx>]"

            @param  lTokens
                    the list of query tokens and its values.

            @return A string list of internal filter names, including
                    all filters, which match this query.
         */
        OUStringList impl_getSortedFilterList(const QueryTokenizer& lTokens) const;

        //---------------------------------------

        /** TODO document me
         */
        OUStringList impl_getSortedFilterListForModule(const ::rtl::OUString& sModule,
                                                             sal_Int32        nIFlags,
                                                             sal_Int32        nEFlags) const;

        //---------------------------------------

        /** @short  read a specialized and sorted list of filter names from
                    the configuration (matching the specified module)

            @param  sModule
                    the module for which the sorted list should be retrieved for.

            @return A string list of internal filter names.
                    Can be empty.
         */
        OUStringList impl_readSortedFilterListFromConfig(const ::rtl::OUString& sModule) const;

    //-------------------------------------------
    // static uno helper!

    public:

        //---------------------------------------

        /** @short  return the uno implementation name of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix uno implementation name of this class.
         */
        static ::rtl::OUString impl_getImplementationName();

        //---------------------------------------

        /** @short  return the list of supported uno services of this class.

            @descr  Because this information is used at several places
                    (and mostly an object instance of this class is not possible)
                    its implemented as a static function!

            @return The fix list of uno services supported by this class.
         */
        static css::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames();

        //---------------------------------------

        /** @short  return a new intsnace of this class.

            @descr  This method is used by the uno service manager, to create
                    a new instance of this service if needed.

            @param  xSMGR
                    reference to the uno service manager, which require
                    this new instance. It should be passed to the new object
                    so it can be used internaly to create own needed uno resources.

            @return The new instance of this service as an uno reference.
         */
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_FILTERFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
