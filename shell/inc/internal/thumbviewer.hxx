/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef THUMBVIEWER_HXX_INCLUDED
#define THUMBVIEWER_HXX_INCLUDED

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
#include <gdiplus.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string>

class CThumbviewer : public IPersistFile, public IExtractImage
{
public:
    CThumbviewer(long RefCnt = 1);
    virtual ~CThumbviewer();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    //----------------------------
    // IExtractImage methods
    //----------------------------

    virtual HRESULT STDMETHODCALLTYPE Extract(HBITMAP *phBmpImage);

    virtual HRESULT STDMETHODCALLTYPE GetLocation(
        LPWSTR pszPathBuffer,
        DWORD cchMax,
        DWORD *pdwPriority,
        const SIZE *prgSize,
        DWORD dwRecClrDepth,
        DWORD *pdwFlags);

    //----------------------------
    // IPersist methods
    //----------------------------

    virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID* pClassID);

    //----------------------------
    // IPersistFile methods
    //----------------------------

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
