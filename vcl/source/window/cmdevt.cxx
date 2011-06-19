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
#include "precompiled_vcl.hxx"

#include <string.h>

#include <vcl/cmdevt.hxx>

// =======================================================================

CommandExtTextInputData::CommandExtTextInputData()
{
    mpTextAttr      = NULL;
    mnCursorPos     = 0;
    mnDeltaStart    = 0;
    mnOldTextLen    = 0;
    mnCursorFlags   = 0;
    mbOnlyCursor    = sal_False;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const XubString& rText,
                                                  const sal_uInt16* pTextAttr,
                                                  xub_StrLen nCursorPos,
                                                  sal_uInt16 nCursorFlags,
                                                  xub_StrLen nDeltaStart,
                                                  xub_StrLen nOldTextLen,
                                                  sal_Bool bOnlyCursor ) :
    maText( rText )
{
    if ( pTextAttr && maText.Len() )
    {
        mpTextAttr = new sal_uInt16[maText.Len()];
        memcpy( mpTextAttr, pTextAttr, maText.Len()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = nCursorPos;
    mnDeltaStart    = nDeltaStart;
    mnOldTextLen    = nOldTextLen;
    mnCursorFlags   = nCursorFlags;
    mbOnlyCursor    = bOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const CommandExtTextInputData& rData ) :
    maText( rData.maText )
{
    if ( rData.mpTextAttr && maText.Len() )
    {
        mpTextAttr = new sal_uInt16[maText.Len()];
        memcpy( mpTextAttr, rData.mpTextAttr, maText.Len()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = rData.mnCursorPos;
    mnDeltaStart    = rData.mnDeltaStart;
    mnOldTextLen    = rData.mnOldTextLen;
    mnCursorFlags   = rData.mnCursorFlags;
    mbOnlyCursor    = rData.mbOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::~CommandExtTextInputData()
{
    if ( mpTextAttr )
        delete [] mpTextAttr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
