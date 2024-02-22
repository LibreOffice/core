/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <officecfg/Setup.hxx>
#include <unotools/configmgr.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace utl
{
/** This method is called when there's a need to determine if the
 * current version of LibreOffice has been upgraded to a newer one.

    @param aUpdateVersion This variable is used to determine if
    LibreOffice's previous version should be updated.
 */
static bool isProductVersionUpgraded()
{
    OUString sSetupVersion = utl::ConfigManager::getProductVersion();
    sal_Int32 iCurrent = o3tl::toInt32(o3tl::getToken(sSetupVersion, 0, '.')) * 10
                         + o3tl::toInt32(o3tl::getToken(sSetupVersion, 1, '.'));
    OUString sLastVersion = officecfg::Setup::Product::ooSetupLastVersion::get().value_or("0.0");
    sal_Int32 iLast = o3tl::toInt32(o3tl::getToken(sLastVersion, 0, '.')) * 10
                      + o3tl::toInt32(o3tl::getToken(sLastVersion, 1, '.'));
    return (iCurrent > iLast);
}
}
