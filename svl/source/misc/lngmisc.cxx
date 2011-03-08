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
#include "precompiled_svl.hxx"
#include <lngmisc.hxx>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

INT32 GetNumControlChars( const OUString &rTxt )
{
    INT32 nCnt = 0;
    INT32 nLen = rTxt.getLength();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        if (IsControlChar( rTxt[i] ))
            ++nCnt;
    }
    return nCnt;
}


BOOL RemoveHyphens( OUString &rTxt )
{
    BOOL bModified = FALSE;
    if (HasHyphens( rTxt ))
    {
        String aTmp( rTxt );
        aTmp.EraseAllChars( SVT_SOFT_HYPHEN );
        aTmp.EraseAllChars( SVT_HARD_HYPHEN );
        rTxt = aTmp;
        bModified = TRUE;
    }
    return bModified;
}


BOOL RemoveControlChars( OUString &rTxt )
{
    BOOL bModified = FALSE;
    INT32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        INT32 nLen  = rTxt.getLength();
        INT32 nSize = nLen - nCtrlChars;
        OUStringBuffer aBuf( nSize );
        aBuf.setLength( nSize );
        INT32 nCnt = 0;
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            sal_Unicode cChar = rTxt[i];
            if (!IsControlChar( cChar ))
            {
                DBG_ASSERT( nCnt < nSize, "index out of range" );
                aBuf.setCharAt( nCnt++, cChar );
            }
        }
        DBG_ASSERT( nCnt == nSize, "wrong size" );
        rTxt = aBuf.makeStringAndClear();
        bModified = TRUE;
    }
    return bModified;
}


// non breaking field character
#define CH_TXTATR_INWORD    ((sal_Char) 0x02)

BOOL ReplaceControlChars( rtl::OUString &rTxt, sal_Char /*aRplcChar*/ )
{
    // the resulting string looks like this:
    // 1. non breaking field characters get removed
    // 2. remaining control characters will be replaced by ' '

    BOOL bModified = FALSE;
    INT32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        INT32 nLen  = rTxt.getLength();
        OUStringBuffer aBuf( nLen );
        INT32 nCnt = 0;
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            sal_Unicode cChar = rTxt[i];
            if (CH_TXTATR_INWORD != cChar)
            {
                if (IsControlChar( cChar ))
                    cChar = ' ';
                DBG_ASSERT( nCnt < nLen, "index out of range" );
                aBuf.setCharAt( nCnt++, cChar );
            }
        }
        aBuf.setLength( nCnt );
        rTxt = aBuf.makeStringAndClear();
        bModified = TRUE;
    }
    return bModified;
}

///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
