/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <BorderCacheOwner.hxx>

#include <hintids.hxx>
#include <frame.hxx>
#include <swcache.hxx>

using namespace sw;

BorderCacheOwner::~BorderCacheOwner()
{
    if (m_bInCache)
        SwFrame::GetCache().Delete(this);
}

void BorderCacheOwner::InvalidateInSwCache(const sal_uInt16 nWhich)
{
    switch (nWhich)
    {
        case RES_OBJECTDYING:
        case RES_FMT_CHG:
        case RES_ATTRSET_CHG:
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
        case RES_FRM_SIZE:
        case RES_KEEP:
        case RES_BREAK:
            if (m_bInCache)
            {
                SwFrame::GetCache().Delete(this);
                m_bInCache = false;
            }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
