/*************************************************************************
 *
 *  $RCSfile: thumbviewer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-09-08 14:35:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef INFOTIPS_HXX_INCLUDED
#include "internal/thumbviewer.hxx"
#endif

#ifndef SHLXTHDL_HXX_INCLUDED
#include "internal/shlxthdl.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef REGISTRY_HXX_INCLUDED
#include "internal/registry.hxx"
#endif

#ifndef FILEEXTENSIONS_HXX_INCLUDED
#include "internal/fileextensions.hxx"
#endif

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef ZIPFILE_HXX_INCLUDED
#include "internal/zipfile.hxx"
#endif

#include "internal/resource.h"
#include <stdio.h>
#include <utility>
#include <stdlib.h>

#include <shellapi.h>
#include <memory>

class StreamOnZipBuffer : public IStream
{
public:
    StreamOnZipBuffer(const ZipFile::ZipContentBuffer_t& zip_buffer);

    // IUnknown
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release( void);
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);

    // IStream
    virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
    virtual HRESULT STDMETHODCALLTYPE Write(void const *pv, ULONG cb, ULONG *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
    virtual HRESULT STDMETHODCALLTYPE Revert(void);
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);

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

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::AddRef(void)
{
    return InterlockedIncrement(&ref_count_);
}

ULONG STDMETHODCALLTYPE StreamOnZipBuffer::Release( void)
{
    long refcnt = InterlockedDecrement(&ref_count_);

    if (0 == ref_count_)
        delete this;

    return refcnt;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = 0;
    IUnknown* pUnk = 0;

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
    if (pv == NULL)
        return STG_E_INVALIDPOINTER;

    size_t size = ref_zip_buffer_.size();

    if (pos_ > size)
        return S_FALSE;

    char* p = reinterpret_cast<char*>(pv);
    ULONG read = 0;

    for ( ;(pos_ < size) && (cb > 0); pos_++, cb--, read++)
        *p++ = ref_zip_buffer_[pos_];

    if (pcbRead)
        *pcbRead = read;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    size_t size = ref_zip_buffer_.size();
    size_t p = 0;

    switch (dwOrigin)
    {
        case STREAM_SEEK_SET:
            break;
        case STREAM_SEEK_CUR:
            p = pos_;
            break;
        case STREAM_SEEK_END:
            p = size - 1;
            break;
   }

   HRESULT hr = STG_E_INVALIDFUNCTION;

   p += dlibMove.LowPart;
   if ((p >= 0) && (p < size))
   {
        pos_ = p;
        hr = S_OK;
   }
   return hr;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (pstatstg == NULL)
        return STG_E_INVALIDPOINTER;

    ZeroMemory(pstatstg, sizeof(STATSTG));

    if (grfStatFlag == STATFLAG_DEFAULT)
    {
        size_t sz = 4 * sizeof(wchar_t);
        wchar_t* name = reinterpret_cast<wchar_t*>(CoTaskMemAlloc(sz));
        ZeroMemory(name, sz);
        memcpy(name, L"png", 3 * sizeof(wchar_t));
        pstatstg->pwcsName = name;
    }

    pstatstg->type = STGTY_LOCKBYTES;

    ULARGE_INTEGER uli;
    uli.LowPart = ref_zip_buffer_.size();
    uli.HighPart = 0;

    pstatstg->cbSize = uli;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Write(void const *pv, ULONG cb, ULONG *pcbWritten)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::SetSize(ULARGE_INTEGER libNewSize)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Commit(DWORD grfCommitFlags)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Revert(void)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE StreamOnZipBuffer::Clone(IStream **ppstm)
{  return E_NOTIMPL; }


//#########################################


CThumbviewer::CThumbviewer(long RefCnt) :
    ref_count_(RefCnt)
{
    InterlockedIncrement(&g_DllRefCnt);

    thumbnail_size_.cx = 0;
    thumbnail_size_.cy = 0;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplusStartupInput, NULL);
}

CThumbviewer::~CThumbviewer()
{
    Gdiplus::GdiplusShutdown(gdiplus_token_);
    InterlockedDecrement(&g_DllRefCnt);
}

// IUnknown methods

HRESULT STDMETHODCALLTYPE CThumbviewer::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = 0;
    IUnknown* pUnk = 0;

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

ULONG STDMETHODCALLTYPE CThumbviewer::AddRef(void)
{
    return InterlockedIncrement(&ref_count_);
}

ULONG STDMETHODCALLTYPE CThumbviewer::Release( void)
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
        std::auto_ptr<ZipFile> zipfile(new ZipFile(filename_));

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
                UINT offset = 2; // reserve a little border space

                Gdiplus::Rect canvas(0, 0, thumbnail_size_.cx, thumbnail_size_.cy);
                Gdiplus::Rect canvas_thumbnail(0, 0, thumbnail_size_.cx - 2*offset, thumbnail_size_.cy - 2*offset);

                Gdiplus::Rect scaledRect = CalcScaledAspectRatio(
                    Gdiplus::Rect(0, 0, thumbnail_png.GetWidth(), thumbnail_png.GetHeight()), canvas_thumbnail);

                struct {
                    BITMAPINFOHEADER bi;
                    DWORD ct[256];
                } dib;

                dib.bi.biSize = sizeof(BITMAPINFOHEADER);
                dib.bi.biWidth = thumbnail_size_.cx;
                dib.bi.biHeight = thumbnail_size_.cy;
                dib.bi.biPlanes = 1;
                dib.bi.biBitCount = color_depth_;
                dib.bi.biCompression = BI_RGB;
                dib.bi.biSizeImage = 0;
                dib.bi.biXPelsPerMeter = 0;
                dib.bi.biYPelsPerMeter = 0;
                dib.bi.biClrUsed = 0;
                dib.bi.biClrImportant = 0;

                LPVOID lpBits;
                HBITMAP hMemBmp = CreateDIBSection(memDC, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, &lpBits, NULL, 0);

                HGDIOBJ hOldObj = SelectObject(memDC, hMemBmp);

                Gdiplus::Graphics graphics(memDC);
                Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0), 1);

                Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
                graphics.FillRectangle(&whiteBrush, canvas);

                scaledRect.X = (canvas.Width - scaledRect.Width) / 2;
                scaledRect.Y = (canvas.Height - scaledRect.Height) / 2;

                graphics.DrawRectangle(&blackPen, scaledRect);

                scaledRect.X += 1;
                scaledRect.Y += 1;
                scaledRect.Width -= 1;
                scaledRect.Height -= 1;

                graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                Gdiplus::Status stat = graphics.DrawImage(
                    &thumbnail_png, scaledRect, canvas_thumbnail.X, canvas_thumbnail.Y,
                    thumbnail_png.GetWidth(), thumbnail_png.GetHeight(), Gdiplus::UnitPixel);

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
        hr = E_FAIL;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CThumbviewer::GetLocation(
    LPWSTR pszPathBuffer, DWORD cchMax, DWORD *pdwPriority, const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    if ((prgSize == NULL) || (pdwFlags == NULL) || ((*pdwFlags & IEIFLAG_ASYNC) && (pdwPriority == NULL)))
        return E_INVALIDARG;

    thumbnail_size_ = *prgSize;
    color_depth_ = dwRecClrDepth;

    *pdwFlags = IEIFLAG_CACHE; // we don't cache the image

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
    filename_ = WStringToString(pszFileName);
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


Gdiplus::Rect CThumbviewer::CalcScaledAspectRatio(Gdiplus::Rect src, Gdiplus::Rect dest)
{
    Gdiplus::Rect result;
    if (src.Width >= src.Height)
        result = Gdiplus::Rect(0, 0, dest.Width, src.Height * dest.Width / src.Width);
    else
        result = Gdiplus::Rect(0, 0, src.Width * dest.Height / src.Height, dest.Height);

    return result;
}