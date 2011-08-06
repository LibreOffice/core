/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
#include "precompiled_extensions.hxx"

#include "errormail.hxx"
#include "config.hxx"
#include <unotools/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#if defined(UNIX)
    #include <sys/utsname.h>
#endif
#ifdef WIN32
    #include <windows.h>
#endif


using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace oooimprovement;
using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringBuffer;


namespace {
    static OString replaceAll(const OString& str, sal_Char old, const OString& replacement)
    {
        OStringBuffer result;
        sal_Int32 idx = 0;
        do {
            result.append(str.getToken(0, old, idx));
            if(idx>=0) result.append(replacement);
        } while(idx >= 0);
        return result.makeStringAndClear();
    };

    static OString xmlAttrEncode(const OString& input)
    {
        OString result = replaceAll(input, '&', OString("&amp;"));
        result = replaceAll(result, '<', OString("&lt;"));
        result = replaceAll(result, '"', OString("&quot;"));
        return replaceAll(result, '>', OString("&gt;"));
    }

    static OString getPlatform()
    {
        #ifdef SOLARIS
            return "Solaris";
        #elif defined LINUX
            return "Linux";
        #elif defined WIN32
            return "Win32";
        #elif defined UNIX
            return "Unix";
        #else
            return "Unknown";
        #endif
    };

#if defined(UNIX)
    static const OString getLocale()
    {
        const char * locale = getenv( "LC_ALL" );
        if( NULL == locale )
            locale = getenv( "LC_CTYPE" );

        if( NULL == locale )
            locale = getenv( "LANG" );

        if( NULL == locale )
            locale = "C";
        return locale;
    };

    static OString getSystemInfoXml(const Reference<XMultiServiceFactory>&)
    {
        struct utsname info;
        //memset(&info, 0, sizeof(info));
        uname(&info);
        OStringBuffer result =
            "<systeminfo:systeminfo xmlns:systeminfo=\"http://openoffice.org/2002/systeminfo\">\n"
            "<systeminfo:System name=\""
            + xmlAttrEncode(OString(info.sysname)) + "\" version=\""
            + xmlAttrEncode(OString(info.version)) + "\" build=\""
            + xmlAttrEncode(OString(info.release)) + "\" locale=\""
            + xmlAttrEncode(OString(getLocale())) + "\"/>\n"
            "<systeminfo:CPU type=\""
            + xmlAttrEncode(OString(info.machine)) + "\"/>\n"
            "</systeminfo:systeminfo>\n";
        return result.makeStringAndClear();
    };
#else
    static OString getSystemInfoXml(const Reference<XMultiServiceFactory>&)
    {
        OSVERSIONINFO info;
        ZeroMemory(&info, sizeof(OSVERSIONINFO));
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&info);
        OStringBuffer result = OString(
            "<systeminfo:systeminfo xmlns:systeminfo=\"http://openoffice.org/2002/systeminfo\">\n"
            "<systeminfo:System name=\"");
        if(VER_PLATFORM_WIN32_NT == info.dwPlatformId)
            result.append(OString("Windows NT"));
        else
            result.append(OString("Windows"));
        result.append("\" version=\"").append(static_cast<long>(info.dwMajorVersion));
        result.append(".").append(static_cast<long>(info.dwMinorVersion));
        result.append("\" build=\"").append(static_cast<long>(info.dwBuildNumber));
        result.append("\" locale=\"").append(static_cast<long>(GetUserDefaultLangID()));
        result.append("\"/>\n");
        result.append("<systeminfo:CPU type=\""
            /* x86 or AMD64 */  "\"/>\n"
            "</systeminfo:systeminfo>\n");
        return result.makeStringAndClear();
    };
#endif

    static OString getOfficeInfoXml(const Reference<XMultiServiceFactory>& sf)
    {
        Config config(sf);
        const OString product = OUStringToOString(config.getCompleteProductname(), RTL_TEXTENCODING_ASCII_US);
        const OString platform = getPlatform();
        const OString language = OUStringToOString(config.getSetupLocale(), RTL_TEXTENCODING_ASCII_US);
        // If the oooimprovement lib is packaged in an extension, this needs to
        // be done in another way: The build version string needs to be made
        // available in an UNO service (if no better place is found for this,
        // com.sun.star.comp.extensions.oooimprovecore.Core in oooimprovecore
        // is likely the best fit)
        const OString build = OUStringToOString(::utl::Bootstrap::getBuildIdData(OUString()), RTL_TEXTENCODING_ASCII_US);
        const OString exceptiontype = "";
        OStringBuffer result =
            "<officeinfo:officeinfo xmlns:officeinfo=\"http://openoffice.org/2002/officeinfo\" platform=\""
            + xmlAttrEncode(platform) + "\" language=\""
            + xmlAttrEncode(language) + "\" build=\""
            + xmlAttrEncode(build) + "\" exceptiontype=\""
            + xmlAttrEncode(exceptiontype) + "\" product=\""
            + xmlAttrEncode(product) + " \" />\n";
        return result.makeStringAndClear();
    };
}

namespace oooimprovement
{
    Errormail::Errormail(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    {}

    OString Errormail::getXml()
    {
        Config config(m_ServiceFactory);
        const OString usertype;
        const OString email = OUStringToOString(config.getReporterEmail(), RTL_TEXTENCODING_ASCII_US);
        OString feedback;
        {
            OStringBuffer temp;
            temp.append(config.getReportCount());
            feedback = temp.makeStringAndClear();
        }
        const OString title;
        OStringBuffer result =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE errormail:errormail PUBLIC \"-//OpenOffice.org//DTD ErrorMail 1.0//EN\" \"errormail.dtd\">\n"
            "<errormail:errormail xmlns:errormail=\"http://openoffice.org/2002/errormail\" usertype=\""
            + xmlAttrEncode(usertype) + "\">\n"
            "<reportmail:mail xmlns:reportmail=\"http://openoffice.org/2002/reportmail\" version=\"1.1\" feedback=\""
            + xmlAttrEncode(feedback) + "\" email=\""
            + xmlAttrEncode(email) + "\">\n"
            "<reportmail:title>"
            + xmlAttrEncode(title) + "</reportmail:title>\n"
           "<reportmail:attachment name=\"data.zip\" media-type=\"application/zip\" class=\"OOoImprovementLog\"/>\n"
            "</reportmail:mail>\n"
            + getOfficeInfoXml(m_ServiceFactory)
            + getSystemInfoXml(m_ServiceFactory) +
            "</errormail:errormail>\n";
        return result.makeStringAndClear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
