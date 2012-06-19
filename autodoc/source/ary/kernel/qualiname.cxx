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

#include <precomp.h>
#include <ary/qualiname.hxx>


// NOT FULLY DECLARED SERVICES


namespace ary
{


QualifiedName::QualifiedName( uintt i_nSize )
    :   aNamespace(),
        sLocalName(),
        bIsAbsolute(false),
        bIsFunction()
{
    if (i_nSize > 0)
        aNamespace.reserve(i_nSize);
}

QualifiedName::QualifiedName( const char *        i_sText,
                              const char *        i_sSeparator )
    :   aNamespace(),
        sLocalName(),
        bIsAbsolute(false),
        bIsFunction()
{
    AssignText(i_sText,i_sSeparator);
}

QualifiedName::~QualifiedName()
{
}

void
QualifiedName::AssignText(  const char *        i_sText,
                            const char *        i_sSeparator )
{
    csv_assert(NOT csv::no_str(i_sText) AND NOT csv::no_str(i_sSeparator));
    bIsAbsolute = false;
    bIsFunction = false;
    csv::erase_container( aNamespace );

    uintt nSepLen = strlen(i_sSeparator);
    const char * sNext = i_sText;

    const char * ps = strstr( i_sText, i_sSeparator );
    if (ps == i_sText)
    {
        bIsAbsolute = true;
        sNext = ps + nSepLen;
    }

    for ( ps = strstr(sNext, i_sSeparator);
          ps != 0;
          ps = strstr(sNext, i_sSeparator) )
    {
        String sPart(sNext, ps - sNext);
        aNamespace.push_back(sPart);
        sNext = ps + nSepLen;
    }

    uintt sNameLen = strlen(sNext);
    if ( sNameLen > 2 )
    {
        ps = sNext + sNameLen - 2;
        if (*ps == '(' AND *(ps+1) == ')')
        {
            sNameLen -= 2;
            bIsFunction = true;
        }
    }
    sLocalName = String(sNext,sNameLen);
}


}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
