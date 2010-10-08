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
