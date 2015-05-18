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


#include <svl/lngmisc.hxx>

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

namespace linguistic
{
    sal_Int32 GetNumControlChars(const OUString &rTxt)
    {
        sal_Int32 nCnt = 0;
        for (sal_Int32 i = 0; i < rTxt.getLength(); ++i)
            if (IsControlChar(rTxt[i]))
                ++nCnt;
        return nCnt;
    }

    bool RemoveHyphens(OUString &rTxt)
    {
        sal_Int32 n = rTxt.getLength();
        rTxt = comphelper::string::remove(rTxt, SVT_SOFT_HYPHEN);
        rTxt = comphelper::string::remove(rTxt, SVT_HARD_HYPHEN);
        return n != rTxt.getLength();
    }

    bool RemoveControlChars(OUString &rTxt)
    {
        sal_Int32 nSize = rTxt.getLength() - GetNumControlChars(rTxt);
        if(nSize == rTxt.getLength())
            return false;

        OUStringBuffer aBuf(nSize);
        aBuf.setLength(nSize);
        for (sal_Int32 i = 0, j = 0; i < rTxt.getLength() && j < nSize; ++i)
            if (!IsControlChar(rTxt[i]))
                aBuf[j++] = rTxt[i];

        rTxt = aBuf.makeStringAndClear();
        DBG_ASSERT(rTxt.getLength() == nSize, "GetNumControlChars returned a different number of control characters than were actually removed.");

        return true;
    }

    bool ReplaceControlChars(OUString &rTxt)
    {
        // non breaking field character
        static const sal_Char CH_TXTATR_INWORD = static_cast<sal_Char>(0x02);

        // the resulting string looks like this:
        // 1. non breaking field characters get removed
        // 2. remaining control characters will be replaced by ' '

        if (GetNumControlChars(rTxt) == 0)
            return false;

        sal_Int32 n = rTxt.getLength();

        OUStringBuffer aBuf(n);
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

    OUString GetThesaurusReplaceText(const OUString &rText)
    {
        // The strings for synonyms returned by the thesaurus sometimes have some
        // explanation text put in between '(' and ')' or a trailing '*'.
        // These parts should not be put in the ReplaceEdit Text that may get
        // inserted into the document. Thus we strip them from the text.

        OUString aText(rText);

        sal_Int32 nPos = aText.indexOf('(');
        while (nPos >= 0)
        {
            sal_Int32 nEnd = aText.indexOf(')', nPos);
            if (nEnd >= 0)
            {
                OUStringBuffer aTextBuf(aText);
                aTextBuf.remove(nPos, nEnd - nPos + 1);
                aText = aTextBuf.makeStringAndClear();
            }
            else
                break;
            nPos = aText.indexOf('(');
        }

        nPos = aText.indexOf('*');
        if(nPos == 0)
            return OUString();
        else if(nPos > 0)
            aText = aText.copy(0, nPos);

        // remove any possible remaining ' ' that may confuse the thesaurus
        // when it gets called with the text
        return comphelper::string::strip(aText, ' ');
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
