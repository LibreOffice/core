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

#ifndef INCLUDED_EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

#include <functional>
#include <map>
#include <utility>

namespace extensions { namespace resource
{
    typedef ::std::pair< OUString, css::lang::Locale> ResourceBundleDescriptor;

    struct ResourceBundleDescriptorLess : public ::std::binary_function<ResourceBundleDescriptor, ResourceBundleDescriptor, bool>
    {
        bool operator()( const ResourceBundleDescriptor& _lhs, const ResourceBundleDescriptor& _rhs ) const
        {
            if ( _lhs.first < _rhs.first )
                return true;
            if ( _lhs.second.Language < _rhs.second.Language )
                return true;
            if ( _lhs.second.Country < _rhs.second.Country )
                return true;
            if ( _lhs.second.Variant < _rhs.second.Variant )
                return true;
            return false;
        }
    };

    class OpenOfficeResourceLoader : public ::cppu::WeakImplHelper< css::resource::XResourceBundleLoader>
    {
    public:
        typedef ::std::map<
            ResourceBundleDescriptor,
            css::uno::WeakReference< css::resource::XResourceBundle>,
            ResourceBundleDescriptorLess> ResourceBundleCache;

        OpenOfficeResourceLoader(css::uno::Reference< css::uno::XComponentContext> const&);
        // XResourceBundleLoader
        virtual  css::uno::Reference< css::resource::XResourceBundle> SAL_CALL loadBundle_Default( const OUString& aBaseName ) throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual  css::uno::Reference<  css::resource::XResourceBundle> SAL_CALL loadBundle( const OUString& abaseName, const css::lang::Locale& aLocale ) throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        OpenOfficeResourceLoader( const OpenOfficeResourceLoader& ) = delete;
        OpenOfficeResourceLoader& operator=( const OpenOfficeResourceLoader& ) = delete;
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
        ::osl::Mutex m_aMutex;
        ResourceBundleCache m_aBundleCache;
    };
}}

#endif // INCLUDED_EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
