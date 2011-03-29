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
#include "precompiled_svx.hxx"
#include <svx/swframeposstrings.hxx>
#include <tools/rc.hxx>
#include <tools/debug.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

class SvxSwFramePosString_Impl : public Resource
{
    friend class SvxSwFramePosString;
    String aStrings[SvxSwFramePosString::STR_MAX];
public:
    SvxSwFramePosString_Impl();
};
SvxSwFramePosString_Impl::SvxSwFramePosString_Impl() :
    Resource(SVX_RES(RID_SVXSW_FRAMEPOSITIONS))
{
    for(sal_uInt16 i = 0; i < SvxSwFramePosString::STR_MAX; i++)
    {
        //string ids have to start at 1
        aStrings[i] = String(SVX_RES(i + 1));
    }
    FreeResource();
}

SvxSwFramePosString::SvxSwFramePosString() :
    pImpl(new SvxSwFramePosString_Impl)
{
}

SvxSwFramePosString::~SvxSwFramePosString()
{
    delete pImpl;
}

const String& SvxSwFramePosString::GetString(StringId eId)
{
    DBG_ASSERT(eId >= 0 && eId < STR_MAX, "invalid StringId");
    if(!(eId >= 0 && eId < STR_MAX))
        eId = LEFT;
    return pImpl->aStrings[eId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
