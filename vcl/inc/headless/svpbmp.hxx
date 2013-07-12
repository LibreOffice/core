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

#ifndef SVP_SVBMP_HXX
#define SVP_SVBMP_HXX

#include "sal/config.h"

#include "basebmp/bitmapdevice.hxx"

#include <salbmp.hxx>

#define SVP_DEFAULT_BITMAP_FORMAT basebmp::FORMAT_TWENTYFOUR_BIT_TC_MASK

class SvpSalBitmap : public SalBitmap
{
    basebmp::BitmapDeviceSharedPtr     m_aBitmap;
public:
    SvpSalBitmap() {}
    virtual ~SvpSalBitmap();

    const basebmp::BitmapDeviceSharedPtr& getBitmap() const { return m_aBitmap; }
    void setBitmap( const basebmp::BitmapDeviceSharedPtr& rSrc ) { m_aBitmap = rSrc; }

    // SalBitmap
    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal );
    virtual bool            Create( const SalBitmap& rSalBmp );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount );
    virtual bool            Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false );
    virtual void            Destroy();
    virtual Size            GetSize() const;
    virtual sal_uInt16      GetBitCount() const;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly );
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    virtual bool            GetSystemData( BitmapSystemData& rData );

    static sal_uInt32 getBitCountFromScanlineFormat( basebmp::Format nFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
