/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updater.hxx"

#include <unistd.h>
#include <errno.h>

#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>

namespace {

const char* pUpdaterName = "updater";

void CopyFileToDir(const OUString& rTempDirURL, const OUString rFileName, const OUString& rOldDir)
{
    OUString aSourceURL = rOldDir + "/" + rFileName;
    OUString aDestURL = rTempDirURL + "/" + rFileName;

    osl::File::RC eError = osl::File::copy(aSourceURL, aDestURL);
    if (eError != osl::File::E_None)
    {
        SAL_WARN("desktop.updater", "could not copy the file to a temp directory: " << rFileName);
        throw std::exception();
    }
}

void CopyUpdaterToTempDir(const OUString& rInstallDirURL, const OUString& rTempDirURL)
{
    OUString aUpdaterName = OUString::fromUtf8(pUpdaterName);
    CopyFileToDir(rTempDirURL, aUpdaterName, rInstallDirURL);
}

void createStr(const char* pSrc, char** pArgs, size_t i)
{
    size_t nLength = std::strlen(pSrc);
    char* pFinalStr = new char[nLength + 1];
    std::strncpy(pFinalStr, pSrc, nLength);
    pFinalStr[nLength] = '\0';
    pArgs[i] = pFinalStr;
}

void createStr(const OUString& rStr, char** pArgs, size_t i)
{
    OString aStr = OUStringToOString(rStr, RTL_TEXTENCODING_UTF8);
    char* pStr = new char[aStr.getLength() + 1];
    std::strncpy(pStr, aStr.getStr(), aStr.getLength());
    pStr[aStr.getLength()] = '\0';
    pArgs[i] = pStr;
}

char** createCommandLine(const OUString& rInstallDir)
{
    size_t nArgs = 6;
    char** pArgs = new char*[nArgs];
    {
        createStr(pUpdaterName, pArgs, 0);
    }
    {
        const char* pPatchDir = "/lo/users/moggi/patch";
        createStr(pPatchDir, pArgs, 1);
    }
    {
        createStr(rInstallDir, pArgs, 2);
    }
    {
        OUString aWorkingDir = rInstallDir + "/updated";
        createStr(aWorkingDir, pArgs, 3);
    }
    {
        const char* pPID = "/replace";
        createStr(pPID, pArgs, 4);
    }
    pArgs[nArgs - 1] = nullptr;

    return pArgs;
}

OUString getPathFromURL(const OUString& rURL)
{
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(rURL, aPath);

    return aPath;
}

}

void Update(const OUString& rInstallDirURL)
{
    utl::TempFile aTempDir(nullptr, true);
    OUString aTempDirURL = aTempDir.GetURL();
    CopyUpdaterToTempDir(rInstallDirURL, aTempDirURL);

    OUString aTempDirPath = getPathFromURL(aTempDirURL);
    OString aPath = OUStringToOString(aTempDirPath + "/" + OUString::fromUtf8(pUpdaterName), RTL_TEXTENCODING_UTF8);

    char** pArgs = createCommandLine("/lo/users/moggi/test-inst");

    if (execv(aPath.getStr(), pArgs))
    {
        printf("execv failed with error %d %s\n",errno,strerror(errno));
    }
    for (size_t i = 0; i < 6; ++i)
    {
        delete[] pArgs[i];
    }
    delete[] pArgs;
}

void CreateValidUpdateDir(const OUString& /*rInstallDir*/)
{
    OUString rInstallDir = "file:///lo/users/moggi/test-inst";
    OUString aInstallPath = getPathFromURL(rInstallDir);
    OUString aWorkdirPath = getPathFromURL(rInstallDir + "/updated");
    // OUString aPatchDir = getPathFromURL(rUserProfileDir + "/patch");
    OUString aPatchDir = getPathFromURL("file:///lo/users/moggi/patch");

    OUString aUpdaterPath = getPathFromURL(rInstallDir + "/program/" + OUString::fromUtf8(pUpdaterName));

    OUString aCommand = aUpdaterPath + " " + aPatchDir + " " + aInstallPath + " " + aWorkdirPath + " -1";

    OString aOCommand = OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8);

    int nResult = std::system(aOCommand.getStr());
    SAL_WARN_IF(nResult, "desktop.updater", "failed to update");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
