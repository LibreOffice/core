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

#ifndef _SV_SALBMP_HXX
#define _SV_SALBMP_HXX

#include <tools/gen.hxx>
#include <vcl/dllapi.h>

#include <com/sun/star/rendering/XBitmapCanvas.hpp>

struct BitmapBuffer;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;

class VCL_PLUGIN_PUBLIC SalBitmap
{
public:
    SalBitmap() {}
    virtual ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount ) = 0;
    virtual bool            Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) = 0;
    virtual bool            HasAlpha() const { return false; }
    virtual void            SetHasAlpha( bool ) { }
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual sal_uInt16      GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
