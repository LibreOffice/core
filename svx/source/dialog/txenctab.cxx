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


#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include "svx/txenctab.hxx"
#include "txenctab.hrc"

const OUString SvxTextEncodingTable::GetTextString(const rtl_TextEncoding nEnc)
{
    const size_t nCount = SAL_N_ELEMENTS(RID_SVXSTR_TEXTENCODING_TABLE);

    for (size_t i = 0; i < nCount; ++i)
    {
        if (RID_SVXSTR_TEXTENCODING_TABLE[i].second == nEnc)
            return SvxResId(RID_SVXSTR_TEXTENCODING_TABLE[i].first);
    }

    return OUString();
}

rtl_TextEncoding SvxTextEncodingTable::GetTextEncoding(const OUString& rStr)
{
    const size_t nCount = SAL_N_ELEMENTS(RID_SVXSTR_TEXTENCODING_TABLE);

    for (size_t i = 0; i < nCount; ++i)
    {
        if (SvxResId(RID_SVXSTR_TEXTENCODING_TABLE[i].first).equals(rStr))
            return RID_SVXSTR_TEXTENCODING_TABLE[i].second;
    }
    return RTL_TEXTENCODING_DONTKNOW;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
