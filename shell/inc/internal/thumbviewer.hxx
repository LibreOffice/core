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

#ifndef INCLUDED_SHELL_INC_INTERNAL_THUMBVIEWER_HXX
#define INCLUDED_SHELL_INC_INTERNAL_THUMBVIEWER_HXX

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4917)
#endif
#include <objidl.h>
#include <shlobj.h>
#ifdef __MINGW32__
#include <algorithm>
using std::min;
using std::max;
#endif
#ifndef DONT_HAVE_GDIPLUS
#include <gdiplus.h>
#endif
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string>

class CThumbviewer : public IPersistFile, public IExtractImage
{
public:
    CThumbviewer(long RefCnt = 1);
    virtual ~CThumbviewer();


    // IUnknown methods


    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);


    // IExtractImage methods


    virtual HRESULT STDMETHODCALLTYPE Extract(HBITMAP *phBmpImage);

    virtual HRESULT STDMETHODCALLTYPE GetLocation(
        LPWSTR pszPathBuffer,
        DWORD cchMax,
        DWORD *pdwPriority,
        const SIZE *prgSize,
        DWORD dwRecClrDepth,
        DWORD *pdwFlags);


    // IPersist methods


    virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID* pClassID);


    // IPersistFile methods


    virtual HRESULT STDMETHODCALLTYPE IsDirty();

    virtual HRESULT STDMETHODCALLTYPE Load(
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode);

    virtual HRESULT STDMETHODCALLTYPE Save(
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember);

    virtual HRESULT STDMETHODCALLTYPE SaveCompleted(
            /* [unique][in] */ LPCOLESTR pszFileName);

    virtual HRESULT STDMETHODCALLTYPE GetCurFile(
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);

private:
    Gdiplus::Rect CalcScaledAspectRatio(Gdiplus::Rect src, Gdiplus::Rect dest);

private:
    long         ref_count_;
    std::wstring filename_;
    SIZE         thumbnail_size_;
    DWORD        color_depth_;
    ULONG_PTR    gdiplus_token_;
    Gdiplus::Bitmap* signet_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
