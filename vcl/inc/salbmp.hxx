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

#ifndef INCLUDED_VCL_INC_SALBMP_HXX
#define INCLUDED_VCL_INC_SALBMP_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/checksum.hxx>
#include <vcl/salbtype.hxx>

#include <com/sun/star/rendering/XBitmapCanvas.hpp>

struct BitmapBuffer;
class Color;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;
enum class BmpScaleFlag;

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

class VCL_PLUGIN_PUBLIC SalBitmap
{
public:

    SalBitmap()
        : mnChecksum(0)
        , mbChecksumValid(false)
    {
    }

    virtual                 ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount ) = 0;
    virtual bool            Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) = 0;
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual sal_uInt16      GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( BitmapAccessMode nMode ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;

    virtual bool            ScalingSupported() const = 0;
    virtual bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) = 0;
    void                    DropScaledCache();

    virtual bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) = 0;

    virtual bool            ConvertToGreyscale()
    {
        return false;
    }

    void GetChecksum(BitmapChecksum& rChecksum) const
    {
        updateChecksum();
        if (!mbChecksumValid)
            rChecksum = 0; // back-compat
        else
            rChecksum = mnChecksum;
    }

    void InvalidateChecksum()
    {
        mbChecksumValid = false;
    }

protected:
    BitmapChecksum mnChecksum;
    bool           mbChecksumValid;

protected:
    virtual void updateChecksum() const
    {
        if (mbChecksumValid)
            return;

        BitmapChecksum nCrc = 0;
        SalBitmap* pThis = const_cast<SalBitmap*>(this);
        BitmapBuffer* pBuf = pThis->AcquireBuffer(BitmapAccessMode::Read);
        if (pBuf)
        {
            nCrc = pBuf->maPalette.GetChecksum();
            nCrc = vcl_get_checksum(nCrc, pBuf->mpBits, pBuf->mnScanlineSize * pBuf->mnHeight);
            pThis->ReleaseBuffer(pBuf, BitmapAccessMode::Read);
            pThis->mnChecksum = nCrc;
            pThis->mbChecksumValid = true;
        }
        else
        {
            pThis->mbChecksumValid = false;
        }
    }


};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
