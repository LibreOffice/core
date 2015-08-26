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
#include <vcl/dllapi.h>
#include <vcl/salbtype.hxx>

#include <checksum.hxx>

#include <com/sun/star/rendering/XBitmapCanvas.hpp>

struct BitmapBuffer;
class Color;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;
enum class BmpScaleFlag;

class VCL_PLUGIN_PUBLIC SalBitmap
{
public:

    typedef sal_uInt64      ChecksumType;

                            SalBitmap() : mbChecksumValid(false) {}
    virtual                 ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount ) = 0;
    virtual bool            Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) = 0;
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual sal_uInt16      GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( BitmapAccessMode nMode ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;

    virtual bool            Crop( const Rectangle& rRectPixel ) = 0;
    virtual bool            Erase( const Color& rFillColor ) = 0;
    virtual bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) = 0;
    virtual bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) = 0;


    virtual bool GetChecksum(ChecksumType& rChecksum) const
    {
        updateChecksum();
        assert(mbChecksumValid);
        rChecksum = maChecksum;
        return mbChecksumValid;
    }

    virtual void InvalidateChecksum()
    {
        mbChecksumValid = false;
    }

protected:
    ChecksumType           maChecksum;
    bool               mbChecksumValid;

protected:
    virtual void updateChecksum() const
    {
        if (mbChecksumValid)
            return;

        ChecksumType nCrc = 0;
        SalBitmap* pThis = const_cast<SalBitmap*>(this);
        BitmapBuffer* pBuf = pThis->AcquireBuffer(BITMAP_READ_ACCESS);
        if (pBuf)
        {
            pThis->ReleaseBuffer(pBuf, BITMAP_READ_ACCESS);
            nCrc = vcl_crc64(0, pBuf->mpBits, pBuf->mnScanlineSize * pBuf->mnHeight);
            pThis->maChecksum = nCrc;
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
