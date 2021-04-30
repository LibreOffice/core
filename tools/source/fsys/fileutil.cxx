/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/fileutil.hxx>
#if defined _WIN32
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <davclnt.h>
#endif

namespace
{
#if defined _WIN32
OUString UNCToDavURL(LPCWSTR sUNC)
{
    DWORD nSize = 1024;
    wchar_t bufURL[1024];
    DWORD nResult = DavGetHTTPFromUNCPath(sUNC, bufURL, &nSize);
    if (nResult == ERROR_INSUFFICIENT_BUFFER)
    {
        SAL_WARN("tools", "UNCToDavURL: Buffer not big enough to retrieve HTTP from UNC path");
    }
    return nResult == ERROR_SUCCESS ? OUString(o3tl::toU(bufURL)) : OUString();
}
#endif
}

namespace tools
{
bool IsMappedWebDAVPath([[maybe_unused]] const OUString& rURL, [[maybe_unused]] OUString* pRealURL)
{
#if defined _WIN32
    if (rURL.startsWithIgnoreAsciiCase("file:"))
    {
        OUString aSystemPath;
        if (osl::FileBase::getSystemPathFromFileURL(rURL, aSystemPath) == osl::FileBase::E_None)
        {
            DWORD nSize = MAX_PATH;
            char bufUNC[MAX_PATH];
            DWORD nResult = WNetGetUniversalNameW(o3tl::toW(aSystemPath.getStr()),
                                                  UNIVERSAL_NAME_INFO_LEVEL, bufUNC, &nSize);
            if (nResult == ERROR_MORE_DATA)
            {
                SAL_WARN("tools",
                         "IsMappedWebDAVPath: Buffer not big enough to retrieve UniversalNameW");
            }
            if (nResult == NO_ERROR || nResult == ERROR_BAD_DEVICE)
            {
                NETRESOURCEW aReq{};
                if (nResult == ERROR_BAD_DEVICE) // The path could already be an UNC
                    aReq.lpRemoteName = const_cast<LPWSTR>(o3tl::toW(aSystemPath.getStr()));
                else
                {
                    auto pInfo = reinterpret_cast<LPUNIVERSAL_NAME_INFOW>(bufUNC.get());
                    aReq.lpRemoteName = pInfo->lpUniversalName;
                }
                nSize = 1024;
                char bufInfo[1024];
                LPWSTR pSystem = nullptr;
                nResult = WNetGetResourceInformationW(&aReq, bufInfo, &nSize, &pSystem);
                if (nResult == ERROR_MORE_DATA)
                {
                    SAL_WARN("tools", "IsMappedWebDAVPath: Buffer not big enough to retrieve "
                                      "ResourceInformationW");
                }
                if (nResult == NO_ERROR)
                {
                    LPNETRESOURCEW pInfo = reinterpret_cast<LPNETRESOURCEW>(bufInfo.get());
                    if (wcscmp(pInfo->lpProvider, L"Web Client Network") == 0)
                    {
                        if (pRealURL)
                            *pRealURL = UNCToDavURL(aReq.lpRemoteName);
                        return true;
                    }
                }
            }
        }
    }
#endif
    return false;
}

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
