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

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_

#include <com/sun/star/script/provider/XScriptURIHelper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase3.hxx>

namespace func_provider
{

#define css ::com::sun::star

class ScriptingFrameworkURIHelper :
    public ::cppu::WeakImplHelper3<
        css::script::provider::XScriptURIHelper,
        css::lang::XServiceInfo,
        css::lang::XInitialization >
{
private:

    css::uno::Reference< css::ucb::XSimpleFileAccess > m_xSimpleFileAccess;
    css::uno::Reference<css::uri::XUriReferenceFactory> m_xUriReferenceFactory;

    ::rtl::OUString m_sLanguage;
    ::rtl::OUString m_sLocation;
    ::rtl::OUString m_sBaseURI;

    ::rtl::OUString SCRIPTS_PART;

    bool initBaseURI();
    ::rtl::OUString getLanguagePart(const ::rtl::OUString& rStorageURI);
    ::rtl::OUString getLanguagePath(const ::rtl::OUString& rLanguagePart);

public:

    ScriptingFrameworkURIHelper(
        const css::uno::Reference< css::uno::XComponentContext >& xContext )
            throw( css::uno::RuntimeException );

    ~ScriptingFrameworkURIHelper();

    virtual void SAL_CALL
        initialize( const css::uno::Sequence < css::uno::Any > & args )
            throw ( css::uno::Exception, css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getRootStorageURI()
            throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getScriptURI( const ::rtl::OUString& rStorageURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getStorageURI( const ::rtl::OUString& rScriptURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException );
};

} // namespace func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
