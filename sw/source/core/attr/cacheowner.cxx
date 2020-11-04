/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cacheowner.hxx>

#include <frame.hxx>
#include <hintids.hxx>
#include <swcache.hxx>
#include <swfntcch.hxx>

void sw::CacheOwner::CheckCaching(const sal_uInt16 nWhich)
{
    if(!IsInCache() && !IsInSwFntCache())
        return;
    if(isCHRATR(nWhich))
    {
        SetInSwFntCache(false);
    }
    else
    {
        switch(nWhich)
        {
            case RES_OBJECTDYING:
            case RES_FMT_CHG:
            case RES_ATTRSET_CHG:
                SetInSwFntCache(false);
                [[fallthrough]];
            case RES_UL_SPACE:
            case RES_LR_SPACE:
            case RES_BOX:
            case RES_SHADOW:
            case RES_FRM_SIZE:
            case RES_KEEP:
            case RES_BREAK:
                if(IsInCache())
                {
                    SwFrame::GetCache().Delete(this);
                    SetInCache(false);
                }
                break;
        }
    }
}

void sw::CacheOwner::ClearFromCaches()
{
    if(IsInCache())
        SwFrame::GetCache().Delete(this);
    if(IsInSwFntCache())
        pSwFontCache->Delete(this);
}
