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

#ifndef EXTENSIONS_RESOURCE_OOORESOURCELOADER_HXX
#define EXTENSIONS_RESOURCE_OOORESOURCELOADER_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>

#include <functional>
#include <map>
#include <utility>

namespace extensions { namespace resource
{
    typedef ::std::pair< OUString, ::com::sun::star::lang::Locale> ResourceBundleDescriptor;

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

    class OpenOfficeResourceLoader : public ::cppu::WeakImplHelper1< ::com::sun::star::resource::XResourceBundleLoader>
    {
    public:
        typedef ::std::map<
            ResourceBundleDescriptor,
            ::com::sun::star::uno::WeakReference< ::com::sun::star::resource::XResourceBundle>,
            ResourceBundleDescriptorLess> ResourceBundleCache;

        OpenOfficeResourceLoader(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const&);
        // XResourceBundleLoader
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle> SAL_CALL loadBundle_Default( const OUString& aBaseName ) throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual  ::com::sun::star::uno::Reference<  ::com::sun::star::resource::XResourceBundle> SAL_CALL loadBundle( const OUString& abaseName, const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException, std::exception);

    private:
        OpenOfficeResourceLoader();                                             // never implemented
        OpenOfficeResourceLoader( const OpenOfficeResourceLoader& );            // never implemented
        OpenOfficeResourceLoader& operator=( const OpenOfficeResourceLoader& ); // never implemented
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;
        ::osl::Mutex m_aMutex;
        ResourceBundleCache m_aBundleCache;
    };
}}

#endif // EXTENSIONS_RESOURCE_OOORESOURCELOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
