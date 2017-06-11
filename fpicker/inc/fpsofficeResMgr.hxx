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
#include <tools/simplerm.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace fpicker
{
    struct ResLocaleHolder
    {
        std::locale* operator ()()
        {
            return new std::locale(Translate::Create("fps", Application::GetSettings().GetUILanguageTag()));
        }

        static std::locale* getOrCreate()
        {
            return rtl_Instance<
                std::locale, ResLocaleHolder,
                osl::MutexGuard, osl::GetGlobalMutex >::create (
                    ResLocaleHolder(), osl::GetGlobalMutex());
        }
    };
}

inline OUString FpsResId(const char* pId)
{
    return Translate::get(pId, *fpicker::ResLocaleHolder::getOrCreate());
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
