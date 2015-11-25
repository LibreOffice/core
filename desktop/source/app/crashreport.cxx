/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <desktop/crashreport.hxx>

#include <string>
#include <fstream>

osl::Mutex CrashReporter::maMutex;

#if HAVE_FEATURE_BREAKPAD
void CrashReporter::AddKeyValue(const OUString& rKey, const OUString& rValue)
{
    osl::MutexGuard aGuard(maMutex);
    std::string ini_path = getIniFileName();
    std::ofstream ini_file(ini_path, std::ios_base::app);
    ini_file << rtl::OUStringToOString(rKey, RTL_TEXTENCODING_UTF8).getStr() << "=";
    ini_file << rtl::OUStringToOString(rValue, RTL_TEXTENCODING_UTF8).getStr() << "\n";
}
#endif

const char* CrashReporter::getIniFileName()
{
    return "/tmp/dump.ini";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
