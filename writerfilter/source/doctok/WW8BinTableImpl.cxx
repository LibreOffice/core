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

#include "WW8BinTableImpl.hxx"

namespace writerfilter {
namespace doctok
{
using namespace ::std;

void PageNumber::dump(OutputWithDepth<string> & /*output*/) const
{
}

sal_uInt32 WW8BinTableImpl::getPageNumber(const Fc & rFc) const
{
    sal_uInt32 nResult = 0;

    if (mPageMap.find(rFc) == mPageMap.end())
    {
        sal_uInt32 left = 0;
        sal_uInt32 right = getEntryCount();

        while (right - left > 1)
        {
            sal_uInt32 middle = (right + left) / 2;

            Fc aFc = getFc(middle);

            if (rFc < aFc)
                right = middle;
            else
                left = middle;

        }

        nResult = getPageNumber(left);
        mPageMap[rFc] = nResult;

    }
    else
        nResult = mPageMap[rFc];

    return nResult;
}

string WW8BinTableImpl::toString() const
{
    string aResult;
    char sBuffer[255];

    aResult += "(";

    for (sal_uInt32 n = 0; n < getEntryCount(); n++)
    {
        if (n > 0)
            aResult += ", ";

        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getFc(n).get());
        aResult += sBuffer;
        aResult += "->";
        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getPageNumber(n));
        aResult += sBuffer;
    }

    aResult += ")";

    return aResult;
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
