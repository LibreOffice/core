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
#include <svl/lngmisc.hxx>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

using namespace rtl;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

sal_Int32 GetNumControlChars( const OUString &rTxt )
{
    sal_Int32 nCnt = 0;
    sal_Int32 nLen = rTxt.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (IsControlChar( rTxt[i] ))
            ++nCnt;
    }
    return nCnt;
}


sal_Bool RemoveHyphens( OUString &rTxt )
{
    sal_Bool bModified = sal_False;
    if (HasHyphens( rTxt ))
    {
        String aTmp( rTxt );
        aTmp.EraseAllChars( SVT_SOFT_HYPHEN );
        aTmp.EraseAllChars( SVT_HARD_HYPHEN );
        rTxt = aTmp;
        bModified = sal_True;
    }
    return bModified;
}


sal_Bool RemoveControlChars( OUString &rTxt )
{
    sal_Bool bModified = sal_False;
    sal_Int32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        sal_Int32 nLen  = rTxt.getLength();
        sal_Int32 nSize = nLen - nCtrlChars;
        OUStringBuffer aBuf( nSize );
        aBuf.setLength( nSize );
        sal_Int32 nCnt = 0;
        for (sal_Int32 i = 0;  i < nLen;  ++i)
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
        bModified = sal_True;
    }
    return bModified;
}


// non breaking field character
#define CH_TXTATR_INWORD    ((sal_Char) 0x02)

sal_Bool ReplaceControlChars( rtl::OUString &rTxt, sal_Char /*aRplcChar*/ )
{
    // the resulting string looks like this:
    // 1. non breaking field characters get removed
    // 2. remaining control characters will be replaced by ' '

    sal_Bool bModified = sal_False;
    sal_Int32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        sal_Int32 nLen  = rTxt.getLength();
        OUStringBuffer aBuf( nLen );
        sal_Int32 nCnt = 0;
        for (sal_Int32 i = 0;  i < nLen;  ++i)
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
        bModified = sal_True;
    }
    return bModified;
}


String GetThesaurusReplaceText( const String &rText )
{
    // The strings for synonyms returned by the thesaurus sometimes have some
    // explanation text put in between '(' and ')' or a trailing '*'.
    // These parts should not be put in the ReplaceEdit Text that may get
    // inserted into the document. Thus we strip them from the text.

    String aText( rText );

    xub_StrLen nPos = aText.Search( sal_Unicode('(') );
    while (STRING_NOTFOUND != nPos)
    {
        xub_StrLen nEnd = aText.Search( sal_Unicode(')'), nPos );
        if (STRING_NOTFOUND != nEnd)
            aText.Erase( nPos, nEnd-nPos+1 );
        else
            break;
        nPos = aText.Search( sal_Unicode('(') );
    }

    nPos = aText.Search( sal_Unicode('*') );
    if (STRING_NOTFOUND != nPos)
        aText.Erase( nPos );

    // remove any possible remaining ' ' that may confuse the thesaurus
    // when it gets called with the text
    aText.EraseLeadingAndTrailingChars( sal_Unicode(' ') );

    return aText;
}

///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

