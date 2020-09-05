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
    auto bufURL(std::make_unique<wchar_t[]>(nSize));
    DWORD nResult = DavGetHTTPFromUNCPath(sUNC, bufURL.get(), &nSize);
    if (nResult == ERROR_INSUFFICIENT_BUFFER)
    {
        bufURL = std::make_unique<wchar_t[]>(nSize);
        nResult = DavGetHTTPFromUNCPath(sUNC, bufURL.get(), &nSize);
    }
    return nResult == ERROR_SUCCESS ? OUString(o3tl::toU(bufURL.get())) : OUString();
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
            auto bufUNC(std::make_unique<char[]>(nSize));
            DWORD nResult = WNetGetUniversalNameW(o3tl::toW(aSystemPath.getStr()),
                                                  UNIVERSAL_NAME_INFO_LEVEL, bufUNC.get(), &nSize);
            if (nResult == ERROR_MORE_DATA)
            {
                bufUNC = std::make_unique<char[]>(nSize);
                nResult = WNetGetUniversalNameW(o3tl::toW(aSystemPath.getStr()),
                                                UNIVERSAL_NAME_INFO_LEVEL, bufUNC.get(), &nSize);
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
                auto bufInfo(std::make_unique<char[]>(nSize));
                LPWSTR pSystem = nullptr;
                nResult = WNetGetResourceInformationW(&aReq, bufInfo.get(), &nSize, &pSystem);
                if (nResult == ERROR_MORE_DATA)
                {
                    bufInfo = std::make_unique<char[]>(nSize);
                    nResult = WNetGetResourceInformationW(&aReq, bufInfo.get(), &nSize, &pSystem);
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
