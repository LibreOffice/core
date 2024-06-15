/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>

#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/VersionConfig.hxx>

namespace utl
{
bool isProductVersionUpgraded()
{
    static const bool bUpgraded = []() {
        OUString sSetupVersion = utl::ConfigManager::getProductVersion();
        sal_Int32 iCurrent = o3tl::toInt32(o3tl::getToken(sSetupVersion, 0, '.')) * 10
                             + o3tl::toInt32(o3tl::getToken(sSetupVersion, 1, '.'));
        OUString sLastVersion
            = officecfg::Setup::Product::ooSetupLastVersion::get().value_or("0.0");
        sal_Int32 iLast = o3tl::toInt32(o3tl::getToken(sLastVersion, 0, '.')) * 10
                          + o3tl::toInt32(o3tl::getToken(sLastVersion, 1, '.'));
        if (iCurrent > iLast)
        {
            //update lastversion
            try
            {
                auto batch(comphelper::ConfigurationChanges::create());
                officecfg::Setup::Product::ooSetupLastVersion::set(sSetupVersion, batch);
                // tdf#35568: an upgrade must repeat the first run routine
                officecfg::Office::Common::Misc::FirstRun::set(true, batch);
                batch->commit();
            }
            catch (css::lang::IllegalArgumentException&)
            { //If the value was readOnly.
                SAL_WARN("desktop.updater", "Updating property ooSetupLastVersion to version "
                                                << sSetupVersion
                                                << " failed (read-only property?)");
            }
            return true;
        }
        return false;
    }();
    return bUpgraded;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
