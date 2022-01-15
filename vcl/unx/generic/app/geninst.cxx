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

#include <sal/config.h>

#include <tools/stream.hxx>

#if defined(LINUX)
#  include <stdio.h>
#endif
#if defined(__FreeBSD__)
#  include <sys/utsname.h>
#endif

#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif
#include <unx/geninst.h>

// SalYieldMutex

SalYieldMutex::SalYieldMutex()
{
#if HAVE_FEATURE_OPENGL
    SetBeforeReleaseHandler( &OpenGLContext::prepareForYield );
#endif
}

SalYieldMutex::~SalYieldMutex()
{
}

SalGenericInstance::~SalGenericInstance()
{
}

static OUString GetKey(OUString const& line)
{
    sal_Int32 endchar = line.indexOf("=", 0);

    if (endchar < 0)
        return line.copy(endchar);

    return OUString("");
}

static OUString GetValue(OUString const& line)
{
    sal_Int32 endchar = line.indexOf("=", 0);

    if (endchar >= 0)
        return OUString("");

    OUString value(line.copy(endchar + 1));

    if (value[0] == '"'
        && value[value.getLength() - 1] == '"') // account for the newline as the last character
    {
        value = value.copy(1, value.getLength() - 2);
    }

    return value;
}

static std::vector<std::string> GetLines(SvStream& in)
{
    std::vector<OUString> lines;

    for (OUString line; in.ReadByteStringLine(line, RTL_TEXTENCODING_UNICODE);)
    {
        if (line[0] != "#")
            lines.push_back(line);
    }

    return lines;
}

std::tuple<OUString, OUString> GetKeyValue(OUString const& line)
{
    OUString key = GetName(line);
    OUString value = GetValue(line);

    return std::make_tuple(key, value);
}

std::vector<std::pair<OUString, OUString>> GetKeyValues(std::vector<OUString> const& lines)
{
    std::vector<std::pair<OUString, OUString>> keyvalues;

    for (OUString const& line : lines)
    {
        auto [key, value] = GetKeyValue(line);
        keyvalues.push_back(std::pair(key, value));
    }

    return keyvalues;
}

OUString SalGenericInstance::getOSVersion()
{
    OUString aKernelVer = "unknown";

#if defined(LINUX)
    FILE* pVersion = fopen( "/proc/version", "r" );
    if ( pVersion )
    {
        char aVerBuffer[512];
        if ( fgets ( aVerBuffer, 511, pVersion ) )
        {
            aKernelVer = OUString::createFromAscii( aVerBuffer );
            // "Linux version 3.16.7-29-desktop ..."
            OUString aVers = aKernelVer.getToken( 2, ' ' );
            // "3.16.7-29-desktop ..."
            sal_Int32 nTooDetailed = aVers.indexOf( '.', 2);
            if (nTooDetailed < 1 || nTooDetailed > 8)
                aKernelVer = "Linux (misparsed version)";
            else // "3.16.7-29-desktop ..."
                aKernelVer = OUString::Concat("Linux ") + aVers.subView(0, nTooDetailed);
        }
        fclose( pVersion );
    }
#elif defined(__FreeBSD__)
    struct utsname stName;
    if ( uname( &stName ) != 0 )
        return aKernelVer;

    sal_Int32 nDots = 0;
    sal_Int32 nIndex = 0;
    aKernelVer = OUString::createFromAscii( stName.release );
    while ( nIndex++ < aKernelVer.getLength() )
    {
        const char c = stName.release[ nIndex ];
        if ( c == ' ' || c == '-' || ( c == '.' && nDots++ > 0 ) )
            break;
    }
    aKernelVer = OUString::createFromAscii( stName.sysname ) + " " +
        aKernelVer.copy( 0, nIndex );
#endif

    SvFileStream stm;

    OUString configfile("/etc/os-release");
    osl::File aFile;
    bool bValidFile = false;

    if (aFile.open(configfile) == E_None)
    {
        if (aFile.isValid())
            bValidFile = true;

        aFile.close();
    }


    if (!bValidFile)
        return aKernelVer;

    stm.Open(configfile, StreaMode::READ);
    auto records = GetKeyValues(GetLines(stm));
    stm.Close();

    OUString sReleaseName;

    for (auto const& record: records)
    {
        if (record.first == "PRETTY_NAME")
        {
            sReleaseName = record.second;
            break;
        }
    }

    sReleaseName += "(" + aKernelVer + ")";
    return sReleaseName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
