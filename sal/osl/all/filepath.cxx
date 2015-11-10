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

#include <osl/file.h>
#include <rtl/ustring.h>

static sal_uInt32 SAL_CALL osl_defCalcTextWidth( rtl_uString *ustrText )
{
    return ustrText ? ustrText->length : 0;
}

oslFileError SAL_CALL osl_abbreviateSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrCompacted, sal_uInt32 uMaxWidth, oslCalcTextWidthFunc pfnCalcWidth )
{
    oslFileError    error = osl_File_E_None;
    rtl_uString     *ustrPath = nullptr;
    rtl_uString     *ustrFile = nullptr;
    sal_uInt32      uPathWidth, uFileWidth;

    if ( !pfnCalcWidth )
        pfnCalcWidth = osl_defCalcTextWidth;

    {
        sal_Int32   iLastSlash = rtl_ustr_lastIndexOfChar_WithLength( ustrSystemPath->buffer, ustrSystemPath->length, SAL_PATHDELIMITER );

        if ( iLastSlash >= 0 )
        {
            rtl_uString_newFromStr_WithLength( &ustrPath, ustrSystemPath->buffer, iLastSlash );
            rtl_uString_newFromStr_WithLength( &ustrFile, &ustrSystemPath->buffer[iLastSlash], ustrSystemPath->length - iLastSlash );
        }
        else
        {
            rtl_uString_new( &ustrPath );
            rtl_uString_newFromString( &ustrFile, ustrSystemPath );
        }
    }

    uPathWidth = pfnCalcWidth( ustrPath );
    uFileWidth = pfnCalcWidth( ustrFile );

    /* First abbreviate the directory component of the path */

    while ( uPathWidth + uFileWidth > uMaxWidth )
    {
        if ( ustrPath->length > 3 )
        {
            ustrPath->length--;
            ustrPath->buffer[ustrPath->length-3] = '.';
            ustrPath->buffer[ustrPath->length-2] = '.';
            ustrPath->buffer[ustrPath->length-1] = '.';
            ustrPath->buffer[ustrPath->length] = 0;

            uPathWidth = pfnCalcWidth( ustrPath );
        }
        else
            break;
    }

    /* Now abbreviate file component */

    while ( uPathWidth + uFileWidth > uMaxWidth )
    {
        if ( ustrFile->length > 4 )
        {
            ustrFile->length--;
            ustrFile->buffer[ustrFile->length-3] = '.';
            ustrFile->buffer[ustrFile->length-2] = '.';
            ustrFile->buffer[ustrFile->length-1] = '.';
            ustrFile->buffer[ustrFile->length] = 0;

            uFileWidth = pfnCalcWidth( ustrFile );
        }
        else
            break;
    }

    rtl_uString_newConcat( pustrCompacted, ustrPath, ustrFile );

    /* Event now if path was compacted to ".../..." it can be to large */

    uPathWidth += uFileWidth;

    while ( uPathWidth > uMaxWidth )
    {
        (*pustrCompacted)->length--;
        (*pustrCompacted)->buffer[(*pustrCompacted)->length] = 0;
        uPathWidth = pfnCalcWidth( *pustrCompacted );
    }

    if ( ustrPath )
        rtl_uString_release( ustrPath );

    if ( ustrFile )
        rtl_uString_release( ustrFile );

    return error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
