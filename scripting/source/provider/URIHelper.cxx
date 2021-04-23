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

#include <config_folders.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "URIHelper.hxx"

namespace func_provider
{

namespace uno = ::com::sun::star::uno;
namespace ucb = ::com::sun::star::ucb;
namespace lang = ::com::sun::star::lang;
namespace uri = ::com::sun::star::uri;

constexpr OUStringLiteral SHARE = u"share";

constexpr OUStringLiteral SHARE_UNO_PACKAGES_URI =
    u"vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE";

constexpr OUStringLiteral USER = u"user";
constexpr OUStringLiteral USER_URI =
    u"vnd.sun.star.expand:${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";



ScriptingFrameworkURIHelper::ScriptingFrameworkURIHelper(
    const uno::Reference< uno::XComponentContext >& xContext)
{
    try
    {
        m_xSimpleFileAccess = ucb::SimpleFileAccess::create(xContext);
    }
    catch (uno::Exception&)
    {
        OSL_FAIL("Scripting Framework error initialising XSimpleFileAccess");
    }

    try
    {
        m_xUriReferenceFactory = uri::UriReferenceFactory::create( xContext );
    }
    catch (uno::Exception&)
    {
        OSL_FAIL("Scripting Framework error initialising XUriReferenceFactory");
    }
}

ScriptingFrameworkURIHelper::~ScriptingFrameworkURIHelper()
{
    // currently does nothing
}

void SAL_CALL
ScriptingFrameworkURIHelper::initialize(
    const uno::Sequence < uno::Any >& args )
{
    if ( args.getLength() != 2 ||
         args[0].getValueType() != ::cppu::UnoType<OUString>::get() ||
         args[1].getValueType() != ::cppu::UnoType<OUString>::get() )
    {
        throw uno::RuntimeException( "ScriptingFrameworkURIHelper got invalid argument list" );
    }

    if ( !(args[0] >>= m_sLanguage) || !(args[1] >>= m_sLocation) )
    {
        throw uno::RuntimeException( "ScriptingFrameworkURIHelper error parsing args" );
    }

    SCRIPTS_PART = "/Scripts/" + m_sLanguage.toAsciiLowerCase();

    if ( !initBaseURI() )
    {
        throw uno::RuntimeException( "ScriptingFrameworkURIHelper cannot find script directory" );
    }
}

bool
ScriptingFrameworkURIHelper::initBaseURI()
{
    OUString uri, test;
    bool bAppendScriptsPart = false;

    if ( m_sLocation == USER )
    {
        test = USER;
        uri = USER_URI;
        bAppendScriptsPart = true;
    }
    else if ( m_sLocation == "user:uno_packages" )
    {
        test = "uno_packages";
        uri = USER_URI + "/user/uno_packages/cache";
    }
    else if (m_sLocation == SHARE)
    {
        test = SHARE;
        uri = "vnd.sun.star.expand:$BRAND_BASE_DIR";
        bAppendScriptsPart = true;
    }
    else if (m_sLocation == "share:uno_packages")
    {
        test = "uno_packages";
        uri = SHARE_UNO_PACKAGES_URI;
    }
    else if (m_sLocation.startsWith("vnd.sun.star.tdoc"))
    {
        m_sBaseURI = m_sLocation + SCRIPTS_PART;
        m_sLocation = "document";
        return true;
    }
    else
    {
        return false;
    }

    if ( !m_xSimpleFileAccess->exists( uri ) ||
         !m_xSimpleFileAccess->isFolder( uri ) )
    {
        return false;
    }

    uno::Sequence< OUString > children =
        m_xSimpleFileAccess->getFolderContents( uri, true );

    auto pChild = std::find_if(children.begin(), children.end(), [&test](const OUString& child) {
        sal_Int32 idx = child.lastIndexOf(test);
        return idx != -1 && (idx + test.getLength()) == child.getLength();
    });
    if (pChild != children.end())
    {
        if ( bAppendScriptsPart )
        {
            m_sBaseURI = *pChild + SCRIPTS_PART;
        }
        else
        {
            m_sBaseURI = *pChild;
        }
        return true;
    }
    return false;
}

OUString
ScriptingFrameworkURIHelper::getLanguagePart(const OUString& rStorageURI)
{
    OUString result;

    sal_Int32 idx = rStorageURI.indexOf(m_sBaseURI);
    sal_Int32 len = m_sBaseURI.getLength() + 1;

    if ( idx != -1 )
    {
        result = rStorageURI.copy(idx + len);
        result = result.replace('/', '|');
    }
    return result;
}

OUString
ScriptingFrameworkURIHelper::getLanguagePath(const OUString& rLanguagePart)
{
    OUString result = rLanguagePart.replace('|', '/');
    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getScriptURI(const OUString& rStorageURI)
{
    return
        "vnd.sun.star.script:" +
        getLanguagePart(rStorageURI) +
        "?language=" +
        m_sLanguage +
        "&location=" +
        m_sLocation;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getStorageURI(const OUString& rScriptURI)
{
    OUString sLanguagePart;
    try
    {
        uno::Reference < uri::XVndSunStarScriptUrl > xURI(
            m_xUriReferenceFactory->parse( rScriptURI ), uno::UNO_QUERY_THROW );
        sLanguagePart = xURI->getName();
    }
    catch ( uno::Exception& )
    {
        throw lang::IllegalArgumentException(
            "Script URI not valid",
            uno::Reference< uno::XInterface >(), 1 );
    }

    return m_sBaseURI + "/" + getLanguagePath(sLanguagePart);
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getRootStorageURI()
{
    return m_sBaseURI;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getImplementationName()
{
    return
        "com.sun.star.script.provider.ScriptURIHelper";
}

sal_Bool SAL_CALL
ScriptingFrameworkURIHelper::supportsService( const OUString& serviceName )
{
    return cppu::supportsService( this, serviceName );
}

uno::Sequence< OUString > SAL_CALL
ScriptingFrameworkURIHelper::getSupportedServiceNames()
{
    return { "com.sun.star.script.provider.ScriptURIHelper" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_ScriptingFrameworkURIHelper_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ScriptingFrameworkURIHelper(context));
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
