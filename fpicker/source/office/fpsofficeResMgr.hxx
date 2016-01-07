/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_FPSOFFICERESMGR_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_FPSOFFICERESMGR_HXX

#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <tools/resmgr.hxx>

namespace fpicker
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
        explicit SvtResId (sal_uInt16 nId) : ResId (nId, *ResMgrHolder::getOrCreate()) {}
    };
}

#define SVT_RESSTR(i)    SvtResId(i).toString()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
