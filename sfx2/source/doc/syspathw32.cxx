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



#ifdef WNT
#ifdef _MSC_VER
#pragma warning(disable:4917)
#endif

#undef WB_LEFT
#undef WB_RIGHT

#include <shlobj.h>

static bool _SHGetSpecialFolderW32( int nFolderID, WCHAR* pszFolder, int nSize )
{
    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( NULL, nFolderID, &pidl );

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolder = static_cast< WCHAR* >( HeapAlloc( GetProcessHeap(), 0, 16000 ));

        SHGetPathFromIDListW( pidl, lpFolder );
        wcsncpy( pszFolder, lpFolder, nSize );

        HeapFree( GetProcessHeap(), 0, lpFolder );
        IMalloc *pMalloc;
        if( NOERROR == SHGetMalloc(&pMalloc) )
        {
            pMalloc->Free( pidl );
            pMalloc->Release();
        }
    }
    return true;
}

#endif

// Copied from sal/types.h to circumvent problems with precompiled headers
// and redefinitions of BOOL, INT32 and other types. Unfortunately tools
// also define these type incompatible with Win32 types which leads from
// time to time to very nasty compilation errors. If someone finds a better
// way to solve these probs please remove this copied part!
typedef unsigned short sal_uInt16;
#if ( defined(WIN32) && !defined(__MINGW32__) )
    typedef wchar_t             sal_Unicode;
#else
    typedef sal_uInt16          sal_Unicode;
#endif

extern "C" bool GetUserTemplateLocation(sal_Unicode* pFolder, int nSize)
{
#ifdef WNT
    return _SHGetSpecialFolderW32( CSIDL_TEMPLATES, reinterpret_cast<LPWSTR>(pFolder), nSize );
#else
    (void)pFolder;
    (void)nSize;
    return false;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
