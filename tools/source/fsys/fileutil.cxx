/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/fileutil.hxx>
#include <tools/urlobj.hxx>
#if defined _WIN32
#include <osl/file.hxx>
#include <string.h>
#include <o3tl/char16_t2wchar_t.hxx>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace tools
{
bool IsMappedWebDAVPath(const INetURLObject& aURL)
{
#if defined _WIN32
    if (aURL.GetProtocol() == INetProtocol::File)
    {
        OUString sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        OUString aSystemPath;
        if (osl::FileBase::getSystemPathFromFileURL(sURL, aSystemPath) == osl::FileBase::E_None)
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
                        return true;
                }
            }
        }
    }
#else
    (void)aURL;
#endif
    return false;
}

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
