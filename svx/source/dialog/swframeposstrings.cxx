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

#include <svx/swframeposstrings.hxx>
#include <tools/rc.hxx>
#include <tools/debug.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

class SvxSwFramePosString_Impl : public Resource
{
    friend class SvxSwFramePosString;
    OUString aStrings[SvxSwFramePosString::STR_MAX];
public:
    SvxSwFramePosString_Impl();
};
SvxSwFramePosString_Impl::SvxSwFramePosString_Impl() :
    Resource(SVX_RES(RID_SVXSW_FRAMEPOSITIONS))
{
    for(sal_uInt16 i = 0; i < SvxSwFramePosString::STR_MAX; i++)
    {
        //string ids have to start at 1
        aStrings[i] = SVX_RESSTR(i + 1);
    }
    FreeResource();
}

SvxSwFramePosString::SvxSwFramePosString() :
    pImpl(new SvxSwFramePosString_Impl)
{
}

SvxSwFramePosString::~SvxSwFramePosString()
{
}

const OUString& SvxSwFramePosString::GetString(StringId eId)
{
    DBG_ASSERT(eId >= 0 && eId < STR_MAX, "invalid StringId");
    if(!(eId >= 0 && eId < STR_MAX))
        eId = LEFT;
    return pImpl->aStrings[eId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
