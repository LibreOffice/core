/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/mnemonic.hxx>

OUString removeMnemonicFromString(OUString const& rStr)
{
    sal_Int32 nDummy;
    return removeMnemonicFromString(rStr, nDummy);
}

OUString removeMnemonicFromString(OUString const& rStr, sal_Int32& rMnemonicPos)
{
    OUString aStr = rStr;
    sal_Int32 nLen = aStr.getLength();
    sal_Int32 i = 0;

    rMnemonicPos = -1;
    while (i < nLen)
    {
        if (aStr[i] == '~')
        {
            if (nLen <= i + 1)
                break;

            if (aStr[i + 1] != '~')
            {
                if (rMnemonicPos == -1)
                    rMnemonicPos = i;
                aStr = aStr.replaceAt(i, 1, u"");
                nLen--;
            }
            else
            {
                aStr = aStr.replaceAt(i, 1, u"");
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
