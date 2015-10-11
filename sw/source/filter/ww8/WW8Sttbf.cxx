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

#include <iostream>
#include <dbgoutsw.hxx>
#include "WW8Sttbf.hxx"
#include "ww8scan.hxx"
#include <cstdio>
#include <osl/endian.h>
#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>

namespace ww8
{
    WW8Struct::WW8Struct(SvStream& rSt, sal_uInt32 nPos, sal_uInt32 nSize)
        : mn_offset(0), mn_size(0)
    {
        if (checkSeek(rSt, nPos))
        {
            sal_Size nRemainingSize = rSt.remainingSize();
            nSize = std::min<sal_uInt32>(nRemainingSize, nSize);
            mp_data.reset(new sal_uInt8[nSize]);
            mn_size = rSt.Read(mp_data.get(), nSize);
        }
        OSL_ENSURE(mn_size == nSize, "short read in WW8Struct::WW8Struct");
    }

    WW8Struct::WW8Struct(WW8Struct * pStruct, sal_uInt32 nPos, sal_uInt32 nSize)
        : mp_data(pStruct->mp_data), mn_offset(pStruct->mn_offset + nPos)
        , mn_size(nSize)
    {
    }

    WW8Struct::~WW8Struct()
    {
    }

    sal_uInt8 WW8Struct::getU8(sal_uInt32 nOffset)
    {
        sal_uInt8 nResult = 0;

        if (nOffset < mn_size)
        {
            nResult = mp_data[mn_offset + nOffset];
        }

        return nResult;
    }

    OUString WW8Struct::getUString(sal_uInt32 nOffset,
                                          sal_uInt32 nCount)
    {
        OUString aResult;

        if (nCount > 0)
        {
            //clip to available
            sal_uInt32 nStartOff = mn_offset + nOffset;
            if (nStartOff >= mn_size)
                return aResult;
            sal_uInt32 nAvailable = (mn_size - nStartOff)/sizeof(sal_Unicode);
            if (nCount > nAvailable)
                nCount = nAvailable;
#if defined OSL_LITENDIAN
            aResult = OUString(reinterpret_cast<const sal_Unicode *>(
                mp_data.get() + nStartOff), nCount);
#else
            OUStringBuffer aBuf;
            for (sal_uInt32 i = 0; i < nCount; ++i)
                aBuf.append(static_cast<sal_Unicode>(getU16(nStartOff+i*2)));
            aResult = aBuf.makeStringAndClear();
#endif
        }

        SAL_INFO( "sw.ww8.level2", "<WW8Struct-getUString offset=\"" << nOffset
            << "\" count=\"" << nCount << "\">" << aResult << "</WW8Struct-getUString>" );

        return aResult;

    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
