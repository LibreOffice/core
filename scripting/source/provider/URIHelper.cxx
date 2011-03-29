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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"

#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <rtl/ustrbuf.hxx>
#include "URIHelper.hxx"

#define PRTSTR(x) ::rtl::OUStringToOString(x, RTL_TEXTENCODING_ASCII_US).pData->buffer

namespace func_provider
{

using ::rtl::OUString;
namespace uno = ::com::sun::star::uno;
namespace ucb = ::com::sun::star::ucb;
namespace lang = ::com::sun::star::lang;
namespace uri = ::com::sun::star::uri;
namespace script = ::com::sun::star::script;

static const char SHARE[] = "share";
static const char SHARE_URI[] =
    "vnd.sun.star.expand:${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::BaseInstallation}";

static const char SHARE_UNO_PACKAGES[] = "share:uno_packages";
static const char SHARE_UNO_PACKAGES_URI[] =
    "vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE";

static const char USER[] = "user";
static const char USER_URI[] =
    "vnd.sun.star.expand:${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";

static const char USER_UNO_PACKAGES[] = "user:uno_packages";
static const char USER_UNO_PACKAGES_DIR[] =
    "/user/uno_packages/cache";

static const char DOCUMENT[] = "document";
static const char TDOC_SCHEME[] = "vnd.sun.star.tdoc";

ScriptingFrameworkURIHelper::ScriptingFrameworkURIHelper(
    const uno::Reference< uno::XComponentContext >& xContext)
        throw( uno::RuntimeException )
{
    try
    {
        m_xSimpleFileAccess = uno::Reference< ucb::XSimpleFileAccess >(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.ucb.SimpleFileAccess")),
                xContext), uno::UNO_QUERY_THROW);
    }
    catch (uno::Exception&)
    {
        OSL_FAIL("Scripting Framework error initialising XSimpleFileAccess");
    }

    try
    {
        m_xUriReferenceFactory = uno::Reference< uri::XUriReferenceFactory >(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uri.UriReferenceFactory")),
            xContext ), uno::UNO_QUERY_THROW );
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
throw ( uno::Exception, uno::RuntimeException )
{
    if ( args.getLength() != 2 ||
         args[0].getValueType() != ::getCppuType((const OUString*)NULL) ||
         args[1].getValueType() != ::getCppuType((const OUString*)NULL) )
    {
        throw uno::RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ScriptingFrameworkURIHelper got invalid argument list" )),
                uno::Reference< uno::XInterface >() );
    }

    if ( (args[0] >>= m_sLanguage) == sal_False ||
         (args[1] >>= m_sLocation) == sal_False )
    {
        throw uno::RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ScriptingFrameworkURIHelper error parsing args" )),
                uno::Reference< uno::XInterface >() );
    }

    SCRIPTS_PART = OUString(RTL_CONSTASCII_USTRINGPARAM("/Scripts/"));
    SCRIPTS_PART = SCRIPTS_PART.concat( m_sLanguage.toAsciiLowerCase() );

    if ( !initBaseURI() )
    {
        throw uno::RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ScriptingFrameworkURIHelper cannot find script directory")),
                uno::Reference< uno::XInterface >() );
    }
}

bool
ScriptingFrameworkURIHelper::initBaseURI()
{
    OUString uri, test;
    bool bAppendScriptsPart = false;

    if ( m_sLocation.equalsAscii(USER))
    {
        test = OUString(RTL_CONSTASCII_USTRINGPARAM(USER));
        uri = OUString(RTL_CONSTASCII_USTRINGPARAM(USER_URI));
        bAppendScriptsPart = true;
    }
    else if ( m_sLocation.equalsAscii(USER_UNO_PACKAGES))
    {
        test = OUString(RTL_CONSTASCII_USTRINGPARAM("uno_packages"));
        uri = OUString(RTL_CONSTASCII_USTRINGPARAM(USER_URI));
        uri = uri.concat(OUString(RTL_CONSTASCII_USTRINGPARAM(USER_UNO_PACKAGES_DIR)));
    }
    else if (m_sLocation.equalsAscii(SHARE))
    {
        test = OUString(RTL_CONSTASCII_USTRINGPARAM(SHARE));
        uri = OUString(RTL_CONSTASCII_USTRINGPARAM(SHARE_URI));
        bAppendScriptsPart = true;
    }
    else if (m_sLocation.equalsAscii(SHARE_UNO_PACKAGES))
    {
        test = OUString(RTL_CONSTASCII_USTRINGPARAM("uno_packages"));
        uri = OUString(RTL_CONSTASCII_USTRINGPARAM(SHARE_UNO_PACKAGES_URI));
    }
    else if (m_sLocation.indexOf(OUString(RTL_CONSTASCII_USTRINGPARAM(TDOC_SCHEME))) == 0)
    {
        m_sBaseURI = m_sLocation.concat( SCRIPTS_PART );
        m_sLocation = OUString(RTL_CONSTASCII_USTRINGPARAM( DOCUMENT ));
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

        // OSL_TRACE("Trying: %s", PRTSTR(child));
        // OSL_TRACE("idx=%d, testlen=%d, children=%d",
        //     idx, test.getLength(), child.getLength());

        if ( idx != -1 && (idx + test.getLength()) == child.getLength() )
        {
            // OSL_TRACE("FOUND PATH: %s", PRTSTR(child));
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
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    ::rtl::OUStringBuffer buf(120);

    buf.appendAscii("vnd.sun.star.script:");
    buf.append(getLanguagePart(rStorageURI));
    buf.appendAscii("?language=");
    buf.append(m_sLanguage);
    buf.appendAscii("&location=");
    buf.append(m_sLocation);

    return buf.makeStringAndClear();
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getStorageURI(const OUString& rScriptURI)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
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
            OUString(RTL_CONSTASCII_USTRINGPARAM("Script URI not valid")),
                uno::Reference< uno::XInterface >(), 1 );
    }

    ::rtl::OUStringBuffer buf(120);
    buf.append(m_sBaseURI);
    buf.append(OUString(RTL_CONSTASCII_USTRINGPARAM("/")));
    buf.append(getLanguagePath(sLanguagePart));

    OUString result = buf.makeStringAndClear();

    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getRootStorageURI()
    throw( uno::RuntimeException )
{
    return m_sBaseURI;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.script.provider.ScriptURIHelper" ));
}

sal_Bool SAL_CALL
ScriptingFrameworkURIHelper::supportsService( const OUString& serviceName )
    throw( uno::RuntimeException )
{
    OUString m_sServiceName(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.script.provider.ScriptURIHelper" ));

    if ( serviceName.equals( m_sServiceName ) )
    {
        return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL
ScriptingFrameworkURIHelper::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    ::rtl::OUString serviceNameList[] = {
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.script.provider.ScriptURIHelper" )) };

    uno::Sequence< ::rtl::OUString > serviceNames = uno::Sequence <
        ::rtl::OUString > ( serviceNameList, 1 );

    return serviceNames;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
