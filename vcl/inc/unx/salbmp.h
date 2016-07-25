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

#ifndef INCLUDED_VCL_INC_UNX_SALBMP_H
#define INCLUDED_VCL_INC_UNX_SALBMP_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <vcl/salgtype.hxx>
#include <unx/saldisp.hxx>
#include <salbmp.hxx>
#include <vclpluginapi.h>
#include <list>

struct  BitmapBuffer;
class   BitmapPalette;
class   SalGraphics;
class   ImplSalDDB;
class   ImplSalBitmapCache;


class VCLPLUG_GEN_PUBLIC X11SalBitmap : public SalBitmap
{
private:

    static BitmapBuffer*        ImplCreateDIB(
                                    const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal
                                );

    static BitmapBuffer*        ImplCreateDIB(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    long nDrawableDepth,
                                    long nX,
                                    long nY,
                                    long nWidth,
                                    long nHeight,
                                    bool bGrey
                                );

public:

    static ImplSalBitmapCache*  mpCache;
    static sal_uIntPtr          mnCacheInstCount;

    static void                 ImplCreateCache();
    static void                 ImplDestroyCache();
    void                        ImplRemovedFromCache();

private:

    BitmapBuffer*   mpDIB;
    ImplSalDDB*     mpDDB;
    bool            mbGrey;

public:

    SAL_DLLPRIVATE bool         ImplCreateFromDrawable(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    long nDrawableDepth,
                                    long nX,
                                    long nY,
                                    long nWidth,
                                    long nHeight
                                );

    SAL_DLLPRIVATE XImage*      ImplCreateXImage(
                                    SalDisplay* pSalDisp,
                                    SalX11Screen nXScreen,
                                    long nDepth,
                                    const SalTwoRect& rTwoRect
                                ) const;

    SAL_DLLPRIVATE ImplSalDDB*  ImplGetDDB(
                                    Drawable,
                                    SalX11Screen nXScreen,
                                    long nDrawableDepth,
                                    const SalTwoRect&
                                ) const;

    void                        ImplDraw(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    long nDrawableDepth,
                                    const SalTwoRect& rTwoRect,
                                    const GC& rGC
                                ) const;

public:

                                X11SalBitmap();
    virtual                     ~X11SalBitmap();

    // override pure virtual methods
    virtual bool                Create(
                                    const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal
                                ) override;

    virtual bool                Create( const SalBitmap& rSalBmp ) override;
    virtual bool                Create(
                                    const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics
                                ) override;

    virtual bool                Create(
                                    const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount
                                ) override;

    virtual bool                Create(
                                    const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false
                                ) override;

    virtual void                Destroy() override;

    virtual Size                GetSize() const override;
    virtual sal_uInt16          GetBitCount() const override;

    virtual BitmapBuffer*       AcquireBuffer( BitmapAccessMode nMode ) override;
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;
    virtual bool                GetSystemData( BitmapSystemData& rData ) override;

    virtual bool                Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    virtual bool                Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) override;
};


class ImplSalDDB
{
private:

    Pixmap          maPixmap;
    SalTwoRect      maTwoRect;
    long            mnDepth;
    SalX11Screen    mnXScreen;

    static void     ImplDraw(
                        Drawable aSrcDrawable,
                        long nSrcDrawableDepth,
                        Drawable aDstDrawable,
                        long nDstDrawableDepth,
                        long nSrcX,
                        long nSrcY,
                        long nDestWidth,
                        long nDestHeight,
                        long nDestX,
                        long nDestY,
                        const GC& rGC
                    );

public:

                    ImplSalDDB(
                        XImage* pImage,
                        Drawable aDrawable,
                        SalX11Screen nXScreen,
                        const SalTwoRect& rTwoRect
                    );

                    ImplSalDDB(
                        Drawable aDrawable,
                        SalX11Screen nXScreen,
                        long nDrawableDepth,
                        long nX,
                        long nY,
                        long nWidth,
                        long nHeight
                    );

                    ~ImplSalDDB();

    Pixmap          ImplGetPixmap() const { return maPixmap; }
    long            ImplGetWidth() const { return maTwoRect.mnDestWidth; }
    long            ImplGetHeight() const { return maTwoRect.mnDestHeight; }
    long            ImplGetDepth() const { return mnDepth; }
    sal_uIntPtr     ImplGetMemSize() const
                    {
                        return( ( maTwoRect.mnDestWidth * maTwoRect.mnDestHeight * mnDepth ) >> 3 );
                    }
    const SalX11Screen& ImplGetScreen() const { return mnXScreen; }

    bool            ImplMatches( SalX11Screen nXScreen, long nDepth, const SalTwoRect& rTwoRect ) const;

    void            ImplDraw(
                        Drawable aDrawable,
                        long nDrawableDepth,
                        const SalTwoRect& rTwoRect,
                        const GC& rGC
                    ) const;
};


struct ImplBmpObj;

class ImplSalBitmapCache
{
private:
    typedef ::std::list< ImplBmpObj* > BmpList_impl;

    BmpList_impl    maBmpList;
    sal_uIntPtr     mnTotalSize;

public:

                    ImplSalBitmapCache();
                    ~ImplSalBitmapCache();

    void            ImplAdd( X11SalBitmap* pBmp, sal_uIntPtr nMemSize );
    void            ImplRemove( X11SalBitmap* pBmp );
    void            ImplClear();
};

#endif // INCLUDED_VCL_INC_UNX_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
