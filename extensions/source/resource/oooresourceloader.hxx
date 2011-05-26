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
    typedef ::std::pair< ::rtl::OUString, ::com::sun::star::lang::Locale> ResourceBundleDescriptor;

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
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle> SAL_CALL loadBundle_Default( const ::rtl::OUString& aBaseName ) throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);
        virtual  ::com::sun::star::uno::Reference<  ::com::sun::star::resource::XResourceBundle> SAL_CALL loadBundle( const ::rtl::OUString& abaseName, const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);

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
