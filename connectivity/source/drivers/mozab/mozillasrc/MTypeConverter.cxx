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


#include <MNSInclude.hxx>
#include "MTypeConverter.hxx"
#include <ucbhelper/content.hxx>

using namespace connectivity::mozab;


void MTypeConverter::ouStringToNsString(OUString const &ous, nsString &nss)
{
    // Convert to OString (utf-8 encoding).
    OString os(OUStringToOString(ous, RTL_TEXTENCODING_UTF8));

    const char *cs = os.getStr();
    PRUint32 csLen = os.getLength();

    NS_ConvertUTF8toUCS2 mozString(cs, csLen);
    //const PRUnichar* uniMozString = (const PRUnichar*) mozString;
    nss = mozString; // temp.
}

OString MTypeConverter::nsACStringToOString( const nsACString& _source )
{
    const char* buffer = _source.BeginReading();
    const char* bufferEnd = _source.EndReading();
    return OString( buffer, static_cast<sal_Int32>(bufferEnd - buffer) );
}

void MTypeConverter::asciiToNsACString( const sal_Char* _asciiString, nsACString& _dest )
{
    _dest.Truncate();
    _dest.AppendASCII( _asciiString );
}

void MTypeConverter::nsStringToOUString(nsString const &nss, OUString &ous)
{
    // Get clone of buffer.
    PRUnichar *uc = ToNewUnicode(nss);
    sal_Int32 nssLen = nss.Length();

    // TODO check if this is ok.
    // PRUnichar != sal_Unicode in mingw
    OUString _ous(reinterpret_cast_mingw_only<sal_Unicode *>(uc), nssLen);
    ous = _ous;

    nsMemory::Free(uc);
}

void MTypeConverter::prUnicharToOUString(PRUnichar const *pru, OUString &ous)
{
    // TODO, specify length.
    // PRUnichar != sal_Unicode in mingw
    OUString _ous(reinterpret_cast_mingw_only<const sal_Unicode *>(pru));
    ous = _ous;
}

char *MTypeConverter::ouStringToCCharStringAscii(OUString const &ous)
{
    // Convert OUString to OString,
    OString os(OUStringToOString(ous, RTL_TEXTENCODING_ASCII_US));

    return(strdup(os.getStr()));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
