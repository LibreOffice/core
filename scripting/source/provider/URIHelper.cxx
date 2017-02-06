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

static const char SHARE[] = "share";

static const char SHARE_UNO_PACKAGES_URI[] =
    "vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE";

static const char USER[] = "user";
static const char USER_URI[] =
    "vnd.sun.star.expand:${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";



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

    SCRIPTS_PART = "/Scripts/";
    SCRIPTS_PART = SCRIPTS_PART.concat( m_sLanguage.toAsciiLowerCase() );

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
        uri = OUStringLiteral(USER_URI) + "/user/uno_packages/cache";
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
        m_sBaseURI = m_sLocation.concat( SCRIPTS_PART );
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

    for ( sal_Int32 i = 0; i < children.getLength(); i++ )
    {
        OUString child = children[i];
        sal_Int32 idx = child.lastIndexOf(test);

        if ( idx != -1 && (idx + test.getLength()) == child.getLength() )
        {
            if ( bAppendScriptsPart )
            {
                m_sBaseURI = child.concat( SCRIPTS_PART );
            }
            else
            {
                m_sBaseURI = child;
            }
            return true;
        }
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
    OUString result;
    result = rLanguagePart.replace('|', '/');
    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getScriptURI(const OUString& rStorageURI)
{
    OUStringBuffer buf(120);

    buf.append("vnd.sun.star.script:");
    buf.append(getLanguagePart(rStorageURI));
    buf.append("?language=");
    buf.append(m_sLanguage);
    buf.append("&location=");
    buf.append(m_sLocation);

    return buf.makeStringAndClear();
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

    OUStringBuffer buf(120);
    buf.append(m_sBaseURI);
    buf.append("/");
    buf.append(getLanguagePath(sLanguagePart));

    OUString result = buf.makeStringAndClear();

    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getRootStorageURI()
{
    return m_sBaseURI;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getImplementationName()
{
    return OUString(
        "com.sun.star.script.provider.ScriptURIHelper" );
}

sal_Bool SAL_CALL
ScriptingFrameworkURIHelper::supportsService( const OUString& serviceName )
{
    return cppu::supportsService( this, serviceName );
}

uno::Sequence< OUString > SAL_CALL
ScriptingFrameworkURIHelper::getSupportedServiceNames()
{
    OUString serviceNameList[] = {
        OUString(
            "com.sun.star.script.provider.ScriptURIHelper" ) };

    uno::Sequence< OUString > serviceNames = uno::Sequence <
        OUString > ( serviceNameList, 1 );

    return serviceNames;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
