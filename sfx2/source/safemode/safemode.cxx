/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/safemode.hxx>

#include <config_folders.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

using namespace osl;

namespace sfx2
{
bool SafeMode::putFlag()
{
    File safeModeFile(getFilePath(u"safemode"_ustr));
    if (safeModeFile.open(osl_File_OpenFlag_Create) == FileBase::E_None)
    {
        safeModeFile.close();
        return true;
    }
    return false;
}
bool SafeMode::hasFlag()
{
    File safeModeFile(getFilePath(u"safemode"_ustr));
    if (safeModeFile.open(osl_File_OpenFlag_Read) == FileBase::E_None)
    {
        safeModeFile.close();
        return true;
    }
    return false;
}
bool SafeMode::removeFlag()
{
    return File::remove(getFilePath(u"safemode"_ustr)) == FileBase::E_None;
}

bool SafeMode::putRestartFlag()
{
    File restartFile(getFilePath(u"safemode_restart"_ustr));
    if (restartFile.open(osl_File_OpenFlag_Create) == FileBase::E_None)
    {
        restartFile.close();
        return true;
    }
    return false;
}
bool SafeMode::hasRestartFlag()
{
    File restartFile(getFilePath(u"safemode_restart"_ustr));
    if (restartFile.open(osl_File_OpenFlag_Read) == FileBase::E_None)
    {
        restartFile.close();
        return true;
    }
    return false;
}
bool SafeMode::removeRestartFlag()
{
    return File::remove(getFilePath(u"safemode_restart"_ustr)) == FileBase::E_None;
}

OUString SafeMode::getFilePath(const OUString& sFilename)
{
    OUString url(u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                 "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/"_ustr);
    rtl::Bootstrap::expandMacros(url);

    OUString aProfilePath;
    FileBase::getSystemPathFromFileURL(url, aProfilePath);
    (void)FileBase::getAbsoluteFileURL(url, sFilename, aProfilePath);
    return aProfilePath;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
