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

#ifndef DONT_HAVE_GDIPLUS


#include <global.hxx>

#include <thumbviewer.hxx>
#include <shlxthdl.hxx>
#include <registry.hxx>
#include <fileextensions.hxx>
#include <config.hxx>
#include <zipfile.hxx>
#include <utilities.hxx>

#include <resource.h>

#include <stdio.h>
#include <utility>
#include <stdlib.h>

#include <shellapi.h>

#include <memory>

namespace internal
{
    /* The signet.png used for thumbnails of signed documents
       is contained as resource in this module, the resource
       id is 2000 */
    static void LoadSignetImageFromResource(ZipFile::ZipContentBuffer_t& buffer)
    {
        HRSRC hrc = FindResourceW(g_hModule, L"#2000", RT_RCDATA);
        DWORD size = SizeofResource(g_hModule, hrc);
        HGLOBAL hglob = LoadResource(g_hModule, hrc);
        char* data = static_cast<char*>(LockResource(hglob));
        buffer = ZipFile::ZipContentBuffer_t(data, data + size);
    }

    static bool IsSignedDocument(const ZipFile* zipfile)
    {
        return zipfile->HasContent("META-INF/documentsignatures.xml");
    }

    static Gdiplus::Point CalcSignetPosition(
        const Gdiplus::Rect& canvas, const Gdiplus::Rect& thumbnail_border, const Gdiplus::Rect& signet)
    {
        int x = 0;
        int y = 0;
        int hoffset = canvas.GetRight() - thumbnail_border.GetRight();
        int voffset = canvas.GetBottom() - thumbnail_border.GetBottom();

        if (hoffset > voffset)
        {
            x = thumbnail_border.GetRight() - signet.GetRight() + min(signet.GetRight() / 2, hoffset);
            y = thumbnail_border.GetBottom() - signet.GetBottom();
        }
        else
        {
            x = thumbnail_border.GetRight() - signet.GetRight();
            y = thumbnail_border.GetBottom() - signet.GetBottom() + min(signet.GetBottom() / 2, voffset);
        }

        return Gdiplus::Point(x,y);
    }
}

namespace {

Gdiplus::Rect CalcScaledAspectRatio(const Gdiplus::Rect& src, const Gdiplus::Rect& dest)
{
    Gdiplus::Rect result;
    if (src.Width >= src.Height)
        result = Gdiplus::Rect(0, 0, dest.Width, src.Height * dest.Width / src.Width);
    else
        result = Gdiplus::Rect(0, 0, src.Width * dest.Height / src.Height, dest.Height);

    return result;
}

}

class StreamOnZipBuffer final : public IStream
{
public:
    explicit StreamOnZipBuffer(const ZipFile::ZipContentBuffer_t& zip_buffer);

    // IUnknown
    virtual ULONG STDMETHODCALLTYPE AddRef() override;
    virtual ULONG STDMETHODCALLTYPE Release() override;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) override;

    // IStream
    virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) override;
    virtual HRESULT STDMETHODCALLTYPE Write(void const *pv, ULONG cb, ULONG *pcbWritten) override;
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) override;
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) override;
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) override;
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) override;
    virtual HRESULT STDMETHODCALLTYPE Revert() override;
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag) override;
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm) override;

private:
    LONG ref_count_;
    const ZipFile::ZipContentBuffer_t& ref_zip_buffer_;
    size_t pos_;
};

StreamOnZipBuffer::StreamOnZipBuffer(const ZipFile::ZipContentBuffer_t& zip_buffer) :
    ref_count_(1),
    ref_zip_buffer_(zip_buffer),
    pos_(0)
{
}

// IUnknown methods

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::AddRef()
{
    return InterlockedIncrement(&ref_count_);
}

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::Release()
{
    long refcnt = InterlockedDecrement(&ref_count_);

    if (0 == ref_count_)
        delete this;

    return refcnt;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;
    IUnknown* pUnk = nullptr;

    if ((IID_IUnknown == riid) || (IID_IStream == riid))
    {
        pUnk = static_cast<IStream*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    if (pv == nullptr)
        return STG_E_INVALIDPOINTER;

    size_t size = ref_zip_buffer_.size();

    if (pos_ > size)
        return S_FALSE;

    char* p = static_cast<char*>(pv);
    ULONG read = 0;

    for ( ;(pos_ < size) && (cb > 0); pos_++, cb--, read++)
        *p++ = ref_zip_buffer_[pos_];

    if (pcbRead)
        *pcbRead = read;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *)
{
    __int64 size = static_cast<__int64>(ref_zip_buffer_.size());
    __int64 p = 0;

    switch (dwOrigin)
    {
        case STREAM_SEEK_SET:
            break;
        case STREAM_SEEK_CUR:
            p = static_cast<__int64>(pos_);
            break;
        case STREAM_SEEK_END:
            p = size - 1;
            break;
    }

    HRESULT hr = STG_E_INVALIDFUNCTION;

    p += dlibMove.QuadPart;

    if ( ( p >= 0 ) && (p < size) )
    {
        pos_ = static_cast<size_t>(p);
        hr = S_OK;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (pstatstg == nullptr)
        return STG_E_INVALIDPOINTER;

    ZeroMemory(pstatstg, sizeof(STATSTG));

    if (grfStatFlag == STATFLAG_DEFAULT)
    {
        size_t sz = 4 * sizeof(wchar_t);
        wchar_t* name = static_cast<wchar_t*>(CoTaskMemAlloc(sz));
        ZeroMemory(name, sz);
        memcpy(name, L"png", 3 * sizeof(wchar_t));
        pstatstg->pwcsName = name;
    }

    pstatstg->type = STGTY_LOCKBYTES;

    ULARGE_INTEGER uli;
    uli.LowPart = static_cast<DWORD>(ref_zip_buffer_.size());
    uli.HighPart = 0;

    pstatstg->cbSize = uli;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Write(void const *, ULONG, ULONG *)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::SetSize(ULARGE_INTEGER)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Commit(DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Revert()
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Clone(IStream **)
{  return E_NOTIMPL; }


CThumbviewer::CThumbviewer(long RefCnt) :
    ref_count_(RefCnt)
{
    InterlockedIncrement(&g_DllRefCnt);

    thumbnail_size_.cx = 0;
    thumbnail_size_.cy = 0;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplusStartupInput, nullptr);

    ZipFile::ZipContentBuffer_t img_data;
    internal::LoadSignetImageFromResource(img_data);
    IStream* stream = new StreamOnZipBuffer(img_data);
    signet_ = new Gdiplus::Bitmap(stream, TRUE);
    stream->Release();
}

CThumbviewer::~CThumbviewer()
{
    delete signet_;
    Gdiplus::GdiplusShutdown(gdiplus_token_);
    InterlockedDecrement(&g_DllRefCnt);
}

// IUnknown methods

HRESULT STDMETHODCALLTYPE CThumbviewer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;
    IUnknown* pUnk = nullptr;

    if ((IID_IUnknown == riid) || (IID_IPersistFile == riid))
    {
        pUnk = static_cast<IPersistFile*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    else if (IID_IExtractImage == riid)
    {
        pUnk = static_cast<IExtractImage*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CThumbviewer::AddRef()
{
    return InterlockedIncrement(&ref_count_);
}

ULONG STDMETHODCALLTYPE CThumbviewer::Release()
{
    long refcnt = InterlockedDecrement(&ref_count_);

    if (0 == ref_count_)
        delete this;

    return refcnt;
}

// IExtractImage2 methods

const std::string THUMBNAIL_CONTENT = "Thumbnails/thumbnail.png";

HRESULT STDMETHODCALLTYPE CThumbviewer::Extract(HBITMAP *phBmpImage)
{
    HRESULT hr = E_FAIL;

    try
    {
        std::wstring fname = getShortPathName( filename_ );
        std::unique_ptr<ZipFile> zipfile( new ZipFile( fname ) );

        if (zipfile->HasContent(THUMBNAIL_CONTENT))
        {
            ZipFile::ZipContentBuffer_t thumbnail;
            zipfile->GetUncompressedContent(THUMBNAIL_CONTENT, thumbnail);
            IStream* stream = new StreamOnZipBuffer(thumbnail);

            Gdiplus::Bitmap thumbnail_png(stream, TRUE);

            if ((thumbnail_png.GetHeight() == 0) || (thumbnail_png.GetWidth() == 0))
            {
                stream->Release();
                return E_FAIL;
            }

            HWND hwnd = GetDesktopWindow();
            HDC hdc = GetDC(hwnd);
            HDC memDC = CreateCompatibleDC(hdc);

            if (memDC)
            {
                UINT offset = 3; // reserve a little border space

                Gdiplus::Rect canvas(0, 0, thumbnail_size_.cx, thumbnail_size_.cy);
                Gdiplus::Rect canvas_thumbnail(offset, offset, thumbnail_size_.cx - 2 * offset, thumbnail_size_.cy - 2 * offset);

                Gdiplus::Rect scaledRect = CalcScaledAspectRatio(
                    Gdiplus::Rect(0, 0, thumbnail_png.GetWidth(), thumbnail_png.GetHeight()), canvas_thumbnail);

                struct {
                    BITMAPINFOHEADER bi;
                    DWORD ct[256];
                } dib;

                ZeroMemory(&dib, sizeof(dib));

                dib.bi.biSize = sizeof(BITMAPINFOHEADER);
                dib.bi.biWidth = thumbnail_size_.cx;
                dib.bi.biHeight = thumbnail_size_.cy;
                dib.bi.biPlanes = 1;
                dib.bi.biBitCount = static_cast<WORD>(color_depth_);
                dib.bi.biCompression = BI_RGB;

                LPVOID lpBits;
                HBITMAP hMemBmp = CreateDIBSection(memDC, reinterpret_cast<LPBITMAPINFO>(&dib), DIB_RGB_COLORS, &lpBits, nullptr, 0);
                HGDIOBJ hOldObj = SelectObject(memDC, hMemBmp);

                Gdiplus::Graphics graphics(memDC);
                Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0), 1);

                Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
                graphics.FillRectangle(&whiteBrush, canvas);

                scaledRect.X = (canvas.Width - scaledRect.Width) / 2;
                scaledRect.Y = (canvas.Height - scaledRect.Height) / 2;

                Gdiplus::Rect border_rect(scaledRect.X, scaledRect.Y, scaledRect.Width, scaledRect.Height);
                graphics.DrawRectangle(&blackPen, border_rect);

                scaledRect.X += 1;
                scaledRect.Y += 1;
                scaledRect.Width -= 1;
                scaledRect.Height -= 1;

                graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                Gdiplus::Status stat = graphics.DrawImage(
                    &thumbnail_png, scaledRect, 0 , 0,
                    thumbnail_png.GetWidth(), thumbnail_png.GetHeight(), Gdiplus::UnitPixel);

                /* Add a signet sign to the thumbnail of signed documents */
                if (internal::IsSignedDocument(zipfile.get()))
                {
                    double SCALING_FACTOR = 0.6;
                    Gdiplus::Rect signet_scaled(
                        0, 0, static_cast<INT>(signet_->GetWidth() * SCALING_FACTOR), static_cast<INT>(signet_->GetHeight() * SCALING_FACTOR));
                    Gdiplus::Point pos_signet = internal::CalcSignetPosition(canvas_thumbnail, border_rect, signet_scaled);
                    Gdiplus::Rect dest(pos_signet.X, pos_signet.Y, signet_scaled.GetRight(), signet_scaled.GetBottom());

                    stat = graphics.DrawImage(
                        signet_, dest,
                        0, 0, signet_->GetWidth(), signet_->GetHeight(),
                        Gdiplus::UnitPixel);
                }

                if (stat == Gdiplus::Ok)
                {
                    *phBmpImage = hMemBmp;
                    hr = NOERROR;
                }

                SelectObject(memDC, hOldObj);
                DeleteDC(memDC);
            }

            ReleaseDC(hwnd, hdc);
            stream->Release();
        }
    }
    catch(std::exception&)
    {
        OutputDebugStringFormatW( L"CThumbviewer Extract ERROR!\n" );
        hr = E_FAIL;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CThumbviewer::GetLocation(
    LPWSTR pszPathBuffer, DWORD cchMax, DWORD *pdwPriority, const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    if ((prgSize == nullptr) || (pdwFlags == nullptr) || ((*pdwFlags & IEIFLAG_ASYNC) && (pdwPriority == nullptr)))
        return E_INVALIDARG;

    thumbnail_size_ = *prgSize;
    color_depth_ = dwRecClrDepth;

    *pdwFlags = IEIFLAG_CACHE; // we don't cache the image

    wcsncpy(pszPathBuffer, filename_.c_str(), cchMax);

    return NOERROR;
}

// IPersist methods

HRESULT STDMETHODCALLTYPE CThumbviewer::GetClassID(CLSID* pClassID)
{
    pClassID = const_cast<CLSID*>(&CLSID_THUMBVIEWER_HANDLER);
    return S_OK;
}

// IPersistFile methods

HRESULT STDMETHODCALLTYPE CThumbviewer::Load(LPCOLESTR pszFileName, DWORD)
{
    filename_ = pszFileName;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CThumbviewer::IsDirty()
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE CThumbviewer::Save(LPCOLESTR, BOOL)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE CThumbviewer::SaveCompleted(LPCOLESTR)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE CThumbviewer::GetCurFile(LPOLESTR __RPC_FAR*)
{ return E_NOTIMPL; }


#endif // DONT_HAVE_GDIPLUS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
