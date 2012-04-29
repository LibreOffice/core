/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Cédric Bosdonnat <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _FPS_OFFICE_RESMGR_HXX_
#define _FPS_OFFICE_RESMGR_HXX_

#include <rtl/instance.hxx>
#include <tools/resmgr.hxx>

namespace
{
    struct ResMgrHolder
    {
        ResMgr * operator ()()
        {
            return ResMgr::CreateResMgr ("fps_office");
        }

        static ResMgr * getOrCreate()
        {
            return rtl_Instance<
                ResMgr, ResMgrHolder,
                osl::MutexGuard, osl::GetGlobalMutex >::create (
                    ResMgrHolder(), osl::GetGlobalMutex());
        }
    };

    struct SvtResId : public ResId
    {
        SvtResId (sal_uInt16 nId) : ResId (nId, *ResMgrHolder::getOrCreate()) {}
    };
}

#define SVT_RESSTR(i)    SvtResId(i).toString()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
