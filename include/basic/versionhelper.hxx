/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>

namespace versionhelper
{
//Function to check if the version is updated or not.
static bool isVersionUpdated(bool aUpdateVersion)
{
    OUString sSetupVersion = utl::ConfigManager::getProductVersion();
    sal_Int32 iCurrent
        = sSetupVersion.getToken(0, '.').toInt32() * 10 + sSetupVersion.getToken(1, '.').toInt32();
    OUString sLastVersion = officecfg::Setup::Product::ooSetupLastVersion::get().value_or("0.0");
    sal_Int32 iLast
        = sLastVersion.getToken(0, '.').toInt32() * 10 + sLastVersion.getToken(1, '.').toInt32();
    if (iCurrent > iLast)
    {
        if (aUpdateVersion)
        { //update lastversion
            std::shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create());
            officecfg::Setup::Product::ooSetupLastVersion::set(sSetupVersion, batch);
            batch->commit();
        }
        return true;
    }
    return false;
}
}