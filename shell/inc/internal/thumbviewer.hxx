/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
