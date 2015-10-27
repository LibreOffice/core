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

#ifndef INCLUDED_SCRIPTING_SOURCE_PROVIDER_URIHELPER_HXX
#define INCLUDED_SCRIPTING_SOURCE_PROVIDER_URIHELPER_HXX

#include <com/sun/star/script/provider/XScriptURIHelper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

namespace func_provider
{

class ScriptingFrameworkURIHelper :
    public ::cppu::WeakImplHelper<
        css::script::provider::XScriptURIHelper,
        css::lang::XServiceInfo,
        css::lang::XInitialization >
{
private:

    css::uno::Reference< css::ucb::XSimpleFileAccess3 > m_xSimpleFileAccess;
    css::uno::Reference<css::uri::XUriReferenceFactory> m_xUriReferenceFactory;

    OUString m_sLanguage;
    OUString m_sLocation;
    OUString m_sBaseURI;

    OUString SCRIPTS_PART;

    bool initBaseURI();
    OUString getLanguagePart(const OUString& rStorageURI);
    static OUString getLanguagePath(const OUString& rLanguagePart);

public:

    ScriptingFrameworkURIHelper(
        const css::uno::Reference< css::uno::XComponentContext >& xContext )
            throw( css::uno::RuntimeException );

    virtual ~ScriptingFrameworkURIHelper();

    virtual void SAL_CALL
        initialize( const css::uno::Sequence < css::uno::Any > & args )
            throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
        getRootStorageURI()
            throw ( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getScriptURI( const OUString& rStorageURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getStorageURI( const OUString& rScriptURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;
};

} // namespace func_provider
#endif // INCLUDED_SCRIPTING_SOURCE_PROVIDER_URIHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
