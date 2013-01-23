/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#include <windows.h>
#include <string>
//#include <malloc.h>
#include <msiquery.h>
#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif

using namespace std;

namespace {
inline bool GetMsiProp(MSIHANDLE hMSI, LPCTSTR pPropName, LPTSTR* ppValue)
{
    DWORD sz = 0;
    ppValue = NULL;
    if (MsiGetProperty(hMSI, pPropName, TEXT(""), &sz) == ERROR_MORE_DATA)
    {
        DWORD nBytes = ++sz * sizeof(TCHAR); // add 1 for null termination
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca( nBytes ));
        ZeroMemory(buffer, nBytes);
        MsiGetProperty(hMSI, pPropName, buffer, &sz);
        *ppValue = buffer;
    }

    return ppValue?true:false ;

}

//std::_tstring GMPV(  ,  const std::_tstring& sProperty)
inline string GetMsiPropValue(MSIHANDLE hMSI, LPCTSTR pPropName)
{
    LPTSTR ppValue = NULL;
    (void)GetMsiProp(hMSI, pPropName, &ppValue);
    string toto =  reinterpret_cast<const char *> (ppValue);
    return toto;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */