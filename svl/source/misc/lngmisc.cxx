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


#include "svl/lngmisc.hxx"

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

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

    bool ReplaceControlChars(rtl::OUString &rTxt)
    {
        // non breaking field character
        static const sal_Char CH_TXTATR_INWORD = static_cast<sal_Char>(0x02);

        // the resulting string looks like this:
        // 1. non breaking field characters get removed
        // 2. remaining control characters will be replaced by ' '

        if (GetNumControlChars(rTxt) == 0)
            return false;

        sal_Int32 n = rTxt.getLength();

        rtl::OUStringBuffer aBuf(n);
        aBuf.setLength(n);

        sal_Int32 j = 0;
        for (sal_Int32 i = 0; i < n && j < n; ++i)
        {
            if (CH_TXTATR_INWORD == rTxt[i])
                continue;

            aBuf[j++] = IsControlChar(rTxt[i]) ? ' ' : rTxt[i];
        }

        aBuf.setLength(j);
        rTxt = aBuf.makeStringAndClear();

        return true;
    }

    ::rtl::OUString GetThesaurusReplaceText(const ::rtl::OUString &rText)
    {
        // The strings for synonyms returned by the thesaurus sometimes have some
        // explanation text put in between '(' and ')' or a trailing '*'.
        // These parts should not be put in the ReplaceEdit Text that may get
        // inserted into the document. Thus we strip them from the text.

        ::rtl::OUString aText(rText);

        sal_Int32 nPos = aText.indexOf('(');
        while (nPos >= 0)
        {
            sal_Int32 nEnd = aText.indexOf(')', nPos);
            if (nEnd >= 0)
            {
                ::rtl::OUStringBuffer aTextBuf(aText);
                aTextBuf.remove(nPos, nEnd - nPos + 1);
                aText = aTextBuf.makeStringAndClear();
            }
            else
                break;
            nPos = aText.indexOf('(');
        }

        nPos = aText.indexOf('*');
        if(nPos == 0)
            return ::rtl::OUString();
        else if(nPos > 0)
            aText = aText.copy(0, nPos);

        // remove any possible remaining ' ' that may confuse the thesaurus
        // when it gets called with the text
        return comphelper::string::strip(aText, ' ');
    }
} // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
