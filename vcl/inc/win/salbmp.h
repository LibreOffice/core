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

#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#include <tools/gen.hxx>
#include <win/wincomp.hxx>
#include <salbmp.hxx>

// --------------
// - SalBitmap  -
// --------------

struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   SalGraphics;

class WinSalBitmap : public SalBitmap
{
private:

    Size                maSize;
    HGLOBAL             mhDIB;
    HBITMAP             mhDDB;
    sal_uInt16              mnBitCount;

public:

    HGLOBAL             ImplGethDIB() const { return mhDIB; }
    HBITMAP             ImplGethDDB() const { return mhDDB; }

    static HGLOBAL      ImplCreateDIB( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    static HANDLE       ImplCopyDIBOrDDB( HANDLE hHdl, bool bDIB );
    static sal_uInt16       ImplGetDIBColorCount( HGLOBAL hDIB );
    static void         ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                             const Size& rSizePixel, bool bRLE4 );

public:

                        WinSalBitmap();
    virtual             ~WinSalBitmap();

public:

    bool                        Create( HANDLE hBitmap, bool bDIB, bool bCopyHandle );
    virtual bool                Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    virtual bool                Create( const SalBitmap& rSalBmpImpl );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, sal_uInt16 nNewBitCount );
    virtual bool                Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas,
                                           Size& rSize,
                                           bool bMask = false );

    virtual void                Destroy();

    virtual Size                GetSize() const { return maSize; }
    virtual sal_uInt16              GetBitCount() const { return mnBitCount; }

    virtual BitmapBuffer*       AcquireBuffer( bool bReadOnly );
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    virtual bool                GetSystemData( BitmapSystemData& rData );
};

#endif // _SV_SALBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
