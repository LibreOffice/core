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

#include "svl/lngmisc.hxx"

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>

namespace linguistic
{
    sal_Int32 GetNumControlChars(const rtl::OUString &rTxt)
    {
        sal_Int32 nCnt = 0;
        for (sal_Int32 i = 0; i < rTxt.getLength(); ++i)
            if (IsControlChar(rTxt[i]))
                ++nCnt;
        return nCnt;
    }

    bool RemoveHyphens(rtl::OUString &rTxt)
    {
        sal_Int32 n = rTxt.getLength();
        rTxt = comphelper::string::remove(rTxt, SVT_SOFT_HYPHEN);
        rTxt = comphelper::string::remove(rTxt, SVT_HARD_HYPHEN);
        return n != rTxt.getLength();
    }

    bool RemoveControlChars(rtl::OUString &rTxt)
    {
        sal_Int32 nSize = rTxt.getLength() - GetNumControlChars(rTxt);
        if(nSize == rTxt.getLength())
            return false;

        rtl::OUStringBuffer aBuf(nSize);
        aBuf.setLength(nSize);
        for (sal_Int32 i = 0, j = 0; i < rTxt.getLength() && j < nSize; ++i)
            if (!IsControlChar(rTxt[i]))
                aBuf[j++] = rTxt[i];

        rTxt = aBuf.makeStringAndClear();
        DBG_ASSERT(rTxt.getLength() == nSize, "GetNumControlChars returned a different number of control characters than were actually removed.");

        return true;
    }

    bool ReplaceControlChars( rtl::OUString &rTxt, sal_Char /*aRplcChar*/ )
    {
        // non breaking field character
        static const sal_Char CH_TXTATR_INWORD = static_cast<sal_Char>(0x02);

        // the resulting string looks like this:
        // 1. non breaking field characters get removed
        // 2. remaining control characters will be replaced by ' '

        bool bModified = false;
        sal_Int32 nCtrlChars = GetNumControlChars( rTxt );
        if (nCtrlChars)
        {
            sal_Int32 nLen  = rTxt.getLength();
            rtl::OUStringBuffer aBuf( nLen );
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
            bModified = true;
        }
        return bModified;
    }

    String GetThesaurusReplaceText(const String &rText)
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
        aText = comphelper::string::strip(aText, ' ');

        return aText;
    }
} // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
