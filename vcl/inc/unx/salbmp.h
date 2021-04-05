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

#include <vcl/salgtype.hxx>
#include <unx/saldisp.hxx>
#include <salbmp.hxx>
#include <vclpluginapi.h>

struct  BitmapBuffer;
class   BitmapPalette;
class   SalGraphics;
class   ImplSalDDB;
class   ImplSalBitmapCache;


class X11SalBitmap final : public SalBitmap
{
private:

    static std::unique_ptr<BitmapBuffer>
                                ImplCreateDIB(
                                    const Size& rSize,
                                    vcl::PixelFormat ePixelFormat,
                                    const BitmapPalette& rPal
                                );

    static std::unique_ptr<BitmapBuffer>
                                ImplCreateDIB(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    tools::Long nDrawableDepth,
                                    tools::Long nX,
                                    tools::Long nY,
                                    tools::Long nWidth,
                                    tools::Long nHeight,
                                    bool bGrey
                                );

public:

    static ImplSalBitmapCache*  mpCache;
    static unsigned int         mnCacheInstCount;

    static void                 ImplCreateCache();
    static void                 ImplDestroyCache();
    void                        ImplRemovedFromCache();

private:

    std::unique_ptr<BitmapBuffer> mpDIB;
    mutable std::unique_ptr<ImplSalDDB> mpDDB;
    bool            mbGrey;

public:

    bool         ImplCreateFromDrawable(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    tools::Long nDrawableDepth,
                                    tools::Long nX,
                                    tools::Long nY,
                                    tools::Long nWidth,
                                    tools::Long nHeight
                                );

    XImage*      ImplCreateXImage(
                                    SalDisplay const * pSalDisp,
                                    SalX11Screen nXScreen,
                                    tools::Long nDepth,
                                    const SalTwoRect& rTwoRect
                                ) const;

    ImplSalDDB*  ImplGetDDB(
                                    Drawable,
                                    SalX11Screen nXScreen,
                                    tools::Long nDrawableDepth,
                                    const SalTwoRect&
                                ) const;

    void                        ImplDraw(
                                    Drawable aDrawable,
                                    SalX11Screen nXScreen,
                                    tools::Long nDrawableDepth,
                                    const SalTwoRect& rTwoRect,
                                    const GC& rGC
                                ) const;

public:

                                X11SalBitmap();
    virtual                     ~X11SalBitmap() override;

    // override pure virtual methods
    virtual bool                Create(
                                    const Size& rSize,
                                    vcl::PixelFormat ePixelFormat,
                                    const BitmapPalette& rPal
                                ) override;

    virtual bool                Create( const SalBitmap& rSalBmp ) override;
    virtual bool                Create(
                                    const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics
                                ) override;

    virtual bool                Create(
                                    const SalBitmap& rSalBmp,
                                    vcl::PixelFormat ePixelFormat
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

    virtual bool                ScalingSupported() const override;
    virtual bool                Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    virtual bool                Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) override;
};


class ImplSalDDB
{
private:

    Pixmap          maPixmap;
    SalTwoRect      maTwoRect;
    tools::Long            mnDepth;
    SalX11Screen    mnXScreen;

    static void     ImplDraw(
                        Drawable aSrcDrawable,
                        tools::Long nSrcDrawableDepth,
                        Drawable aDstDrawable,
                        tools::Long nSrcX,
                        tools::Long nSrcY,
                        tools::Long nDestWidth,
                        tools::Long nDestHeight,
                        tools::Long nDestX,
                        tools::Long nDestY,
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
                        tools::Long nDrawableDepth,
                        tools::Long nX,
                        tools::Long nY,
                        tools::Long nWidth,
                        tools::Long nHeight
                    );

                    ~ImplSalDDB();

    Pixmap          ImplGetPixmap() const { return maPixmap; }
    tools::Long            ImplGetWidth() const { return maTwoRect.mnDestWidth; }
    tools::Long            ImplGetHeight() const { return maTwoRect.mnDestHeight; }
    tools::Long            ImplGetDepth() const { return mnDepth; }
    const SalX11Screen& ImplGetScreen() const { return mnXScreen; }

    bool            ImplMatches( SalX11Screen nXScreen, tools::Long nDepth, const SalTwoRect& rTwoRect ) const;

    void            ImplDraw(
                        Drawable aDrawable,
                        const SalTwoRect& rTwoRect,
                        const GC& rGC
                    ) const;
};


class X11SalBitmap;

class ImplSalBitmapCache
{
private:
    std::vector<X11SalBitmap*>  maBmpList;

public:

                    ImplSalBitmapCache();
                    ~ImplSalBitmapCache();

    void            ImplAdd( X11SalBitmap* pBmp );
    void            ImplRemove( X11SalBitmap const * pBmp );
    void            ImplClear();
};

#endif // INCLUDED_VCL_INC_UNX_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
