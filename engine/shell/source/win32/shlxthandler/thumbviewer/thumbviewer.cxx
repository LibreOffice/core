/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <global.hxx>

#include <thumbviewer.hxx>
#include <shlxthdl.hxx>
#include <fileextensions.hxx>
#include <config.hxx>
#include <zipfile.hxx>
#include <stream_helper.hxx>
#include <utilities.hxx>

#include <resource.h>

#include <stdio.h>
#include <utility>
#include <stdlib.h>

#include <shellapi.h>
#include <objidl.h>
#include <shlobj.h>
#include <thumbcache.h>
#include <gdiplus.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>

namespace
{
    /* The signet.png used for thumbnails of signed documents
       is contained as resource in this module, the resource
       id is 2000 */
    static ZipFile::ZipContentBuffer_t LoadSignetImageFromResource()
    {
        HRSRC hrc = FindResourceW(g_hModule, L"#2000", reinterpret_cast<LPWSTR>(RT_RCDATA));
        DWORD size = SizeofResource(g_hModule, hrc);
        HGLOBAL hglob = LoadResource(g_hModule, hrc);
        char* data = static_cast<char*>(LockResource(hglob));
        return ZipFile::ZipContentBuffer_t(data, data + size);
    }

    static bool IsSignedDocument(const ZipFile& zipfile)
    {
        return zipfile.HasContent("META-INF/documentsignatures.xml");
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

Gdiplus::Rect CalcScaledAspectRatio(const Gdiplus::Rect& src, const Gdiplus::Rect& dest)
{
    Gdiplus::Rect result;
    if (src.Width >= src.Height)
        result = Gdiplus::Rect(0, 0, dest.Width, src.Height * dest.Width / src.Width);
    else
        result = Gdiplus::Rect(0, 0, src.Width * dest.Height / src.Height, dest.Height);

    return result;
}

class StreamOnZipBuffer final : public IStream
{
public:
    struct SmartPtr
    {
        ~SmartPtr() { p->Release(); }
        operator IStream*() { return p; }

        IStream* p;
    };

    static SmartPtr Create(ZipFile::ZipContentBuffer_t&& zip_buffer)
    {
        return { new StreamOnZipBuffer(std::move(zip_buffer)) };
    }

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
    explicit StreamOnZipBuffer(ZipFile::ZipContentBuffer_t&& zip_buffer)
        : zip_buffer_(std::move(zip_buffer))
    {
    }

    std::atomic<ULONG> ref_count_ = 1;
    const ZipFile::ZipContentBuffer_t zip_buffer_;
    size_t pos_ = 0;
};

// IUnknown methods

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::AddRef()
{
    return ++ref_count_;
}

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::Release()
{
    const ULONG refcnt = --ref_count_;

    if (0 == refcnt)
        delete this;

    return refcnt;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;

    if ((IID_IUnknown == riid) || (IID_IStream == riid))
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    if (pv == nullptr)
        return STG_E_INVALIDPOINTER;

    char* p = static_cast<char*>(pv);
    const size_t available = pos_ < zip_buffer_.size() ? zip_buffer_.size() - pos_ : 0;
    const ULONG read = min(available, cb);
    if (read > 0)
    {
        std::copy_n(zip_buffer_.data() + pos_, read, p);
        pos_ += read;
    }

    if (pcbRead)
        *pcbRead = read;

    return read == cb ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                                                  ULARGE_INTEGER* plibNewPosition)
{
    const __int64 size = zip_buffer_.size();
    __int64 p;

    switch (dwOrigin)
    {
        case STREAM_SEEK_SET:
            p = 0;
            break;
        case STREAM_SEEK_CUR:
            p = pos_;
            break;
        case STREAM_SEEK_END:
            p = size;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    p += dlibMove.QuadPart;
    if (p < 0)
        return STG_E_INVALIDFUNCTION;

    if (p > size)
        p = size;

    pos_ = p;
    if (plibNewPosition)
        *plibNewPosition = { .QuadPart = pos_ };
    return S_OK;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (pstatstg == nullptr)
        return STG_E_INVALIDPOINTER;
    if (grfStatFlag != STATFLAG_DEFAULT && grfStatFlag != STATFLAG_NONAME)
        return STG_E_INVALIDFLAG;

    ZeroMemory(pstatstg, sizeof(STATSTG));

    if (grfStatFlag != STATFLAG_NONAME)
    {
        size_t sz = 4 * sizeof(wchar_t);
        wchar_t* name = static_cast<wchar_t*>(CoTaskMemAlloc(sz));
        if (!name)
            return STG_E_INSUFFICIENTMEMORY;
        wcscpy(name, L"png");
        pstatstg->pwcsName = name;
    }

    pstatstg->type = STGTY_LOCKBYTES;

    static_assert(std::is_integral_v<decltype(pstatstg->cbSize.QuadPart)>);
    pstatstg->cbSize = { .QuadPart = zip_buffer_.size() };

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

class CThumbviewer : public IInitializeWithStream, public IThumbnailProvider
{
public:
    CThumbviewer();
    virtual ~CThumbviewer();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef() override;
    virtual ULONG STDMETHODCALLTYPE Release() override;

    // IInitializeWithStream
    virtual HRESULT STDMETHODCALLTYPE Initialize(IStream* pStream, DWORD grfMode) override;

    // IThumbnailProvider
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(UINT cx, HBITMAP* phbmp,
                                                   WTS_ALPHATYPE* pdwAlpha) override;

private:
    std::atomic<ULONG> ref_count_ = 1;
    ULONG_PTR gdiplus_token_;
    Gdiplus::Bitmap* signet_;
    IStream* stream_ = nullptr;
};

CThumbviewer::CThumbviewer()
{
    InterlockedIncrement(&g_DllRefCnt);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplusStartupInput, nullptr);

    signet_ = new Gdiplus::Bitmap(
        StreamOnZipBuffer::Create(LoadSignetImageFromResource()), TRUE);
}

CThumbviewer::~CThumbviewer()
{
    if (stream_)
        stream_->Release();
    delete signet_;
    Gdiplus::GdiplusShutdown(gdiplus_token_);
    InterlockedDecrement(&g_DllRefCnt);
}

// IUnknown

HRESULT STDMETHODCALLTYPE CThumbviewer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;

    if (IID_IUnknown == riid || IID_IInitializeWithStream == riid)
    {
        *ppvObject = static_cast<IInitializeWithStream*>(this);
        AddRef();
        return S_OK;
    }
    else if (IID_IThumbnailProvider == riid)
    {
        *ppvObject = static_cast<IThumbnailProvider*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CThumbviewer::AddRef()
{
    return ++ref_count_;
}

ULONG STDMETHODCALLTYPE CThumbviewer::Release()
{
    const ULONG refcnt = --ref_count_;

    if (0 == refcnt)
        delete this;

    return refcnt;
}

// IInitializeWithStream

HRESULT STDMETHODCALLTYPE CThumbviewer::Initialize(IStream* pStream, DWORD grfMode)
{
    if (grfMode & STGM_READWRITE)
        return STG_E_ACCESSDENIED;
    if (!pStream)
        return E_INVALIDARG;
    if (stream_)
        return E_UNEXPECTED;  // Initialize called twice

    stream_ = pStream;
    stream_->AddRef();
    return S_OK;
}

// IThumbnailProvider

const std::string THUMBNAIL_CONTENT = "Thumbnails/thumbnail.png";

HRESULT STDMETHODCALLTYPE CThumbviewer::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    if (!phbmp || !pdwAlpha)
        return E_INVALIDARG;
    if (!stream_)
        return E_UNEXPECTED;

    *phbmp = nullptr;
    *pdwAlpha = WTSAT_UNKNOWN;

    HRESULT hr = E_FAIL;

    try
    {
        BufferStream zipStream(stream_);
        ZipFile zipfile(&zipStream);

        if (!zipfile.HasContent(THUMBNAIL_CONTENT))
            return E_FAIL;

        ZipFile::ZipContentBuffer_t thumbnail;
        zipfile.GetUncompressedContent(THUMBNAIL_CONTENT, thumbnail);

        Gdiplus::Bitmap thumbnail_png(StreamOnZipBuffer::Create(std::move(thumbnail)), TRUE);
        if (thumbnail_png.GetHeight() == 0 || thumbnail_png.GetWidth() == 0)
            return E_FAIL;

        // The IThumbnailProvider contract: target is cx-by-cx square,
        // 32-bit BGRA top-down DIB with premultiplied alpha (WTSAT_ARGB).
        const SIZE thumbnail_size = { static_cast<LONG>(cx), static_cast<LONG>(cx) };

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = thumbnail_size.cx;
        bmi.bmiHeader.biHeight = -thumbnail_size.cy;  // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        LPVOID lpBits = nullptr;
        HBITMAP hMemBmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &lpBits, nullptr, 0);
        if (!hMemBmp)
            return E_OUTOFMEMORY;

        // Wrap the DIB pixels in a GDI+ Bitmap so we can render straight into it
        // without bouncing through a DC.
        Gdiplus::Bitmap canvasBmp(thumbnail_size.cx, thumbnail_size.cy,
                                  thumbnail_size.cx * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(lpBits));
        Gdiplus::Graphics graphics(&canvasBmp);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        const UINT offset = 3;  // border space
        Gdiplus::Rect canvas(0, 0, thumbnail_size.cx, thumbnail_size.cy);
        Gdiplus::Rect canvas_thumbnail(offset, offset,
                                       thumbnail_size.cx - 2 * offset,
                                       thumbnail_size.cy - 2 * offset);

        Gdiplus::Rect scaledRect = CalcScaledAspectRatio(
            Gdiplus::Rect(0, 0, thumbnail_png.GetWidth(), thumbnail_png.GetHeight()),
            canvas_thumbnail);

        Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
        graphics.FillRectangle(&whiteBrush, canvas);

        scaledRect.X = (canvas.Width - scaledRect.Width) / 2;
        scaledRect.Y = (canvas.Height - scaledRect.Height) / 2;

        Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0), 1);
        Gdiplus::Rect border_rect(scaledRect.X, scaledRect.Y, scaledRect.Width, scaledRect.Height);
        graphics.DrawRectangle(&blackPen, border_rect);

        scaledRect.X += 1;
        scaledRect.Y += 1;
        scaledRect.Width -= 1;
        scaledRect.Height -= 1;

        Gdiplus::Status stat = graphics.DrawImage(
            &thumbnail_png, scaledRect, 0, 0,
            thumbnail_png.GetWidth(), thumbnail_png.GetHeight(),
            Gdiplus::UnitPixel);

        // Overlay the signet for signed documents.
        if (IsSignedDocument(zipfile))
        {
            const double SCALING_FACTOR = 0.6;
            Gdiplus::Rect signet_scaled(
                0, 0,
                static_cast<INT>(signet_->GetWidth() * SCALING_FACTOR),
                static_cast<INT>(signet_->GetHeight() * SCALING_FACTOR));
            Gdiplus::Point pos_signet = CalcSignetPosition(canvas_thumbnail, border_rect, signet_scaled);
            Gdiplus::Rect dest(pos_signet.X, pos_signet.Y, signet_scaled.GetRight(), signet_scaled.GetBottom());

            stat = graphics.DrawImage(
                signet_, dest,
                0, 0, signet_->GetWidth(), signet_->GetHeight(),
                Gdiplus::UnitPixel);
        }

        if (stat == Gdiplus::Ok)
        {
            *phbmp = hMemBmp;
            *pdwAlpha = WTSAT_RGB;  // background is opaque white
            hr = S_OK;
        }
        else
        {
            DeleteObject(hMemBmp);
        }
    }
    catch (std::exception&)
    {
        OutputDebugStringFormatW(L"CThumbviewer::GetThumbnail ERROR!\n");
        hr = E_FAIL;
    }

    return hr;
}

} // namespace

IThumbnailProvider* CreateThumbviewer() { return new CThumbviewer; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
