/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>
#include <set>
#include <string_view>

namespace comphelper
{
class COMPHELPER_DLLPUBLIC DirectoryHelper
{
public:
    static OUString splitAtLastToken(const OUString& rSrc, sal_Unicode aToken, OUString& rRight);
    static bool fileExists(const OUString& rBaseURL);
    static bool dirExists(const OUString& rDirURL);
    static void scanDirsAndFiles(const OUString& rDirURL, std::set<OUString>& rDirs,
                                 std::set<std::pair<OUString, OUString>>& rFiles);
    static bool deleteDirRecursively(const OUString& rDirURL);
    static bool moveDirContent(const OUString& rSourceDirURL, std::u16string_view rTargetDirURL,
                               const std::set<OUString>& rExcludeList);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
