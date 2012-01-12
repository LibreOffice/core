/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#include <tools/gen.hxx>
#include <salbmp.hxx>

// --------------
// - SalBitmap    -
// --------------

struct    BitmapBuffer;
class    BitmapColor;
class    BitmapPalette;
class    SalGraphics;

#define HANDLE ULONG
#define HBITMAP ULONG

class Os2SalBitmap : public SalBitmap
{
private:

    Size                maSize;
    HANDLE              mhDIB;
    HANDLE              mhDIB1Subst;
    HBITMAP             mhDDB;
    USHORT              mnBitCount;

public:

    HANDLE              ImplGethDIB() const { return mhDIB; }
    HBITMAP             ImplGethDDB() const { return mhDDB; }
    HANDLE              ImplGethDIB1Subst() const { return mhDIB1Subst; }

    void                ImplReplacehDIB1Subst( HANDLE hDIB1Subst );

    static HANDLE       ImplCreateDIB( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
    static HANDLE       ImplCreateDIB4FromDIB1( HANDLE hDIB1 );
    static HANDLE       ImplCopyDIBOrDDB( HANDLE hHdl, bool bDIB );
    static USHORT       ImplGetDIBColorCount( HANDLE hDIB );
    static void         ImplDecodeRLEBuffer( const PM_BYTE* pSrcBuf, PM_BYTE* pDstBuf,
                                             const Size& rSizePixel, bool bRLE4 );

    //BOOL                Create( HANDLE hBitmap, BOOL bDIB, BOOL bCopyHandle );

public:

                        Os2SalBitmap();
                        ~Os2SalBitmap();

public:

    //BOOL                Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
    //BOOL                Create( const SalBitmap& rSalBmpImpl );
    //BOOL                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics );
    //BOOL                Create( const SalBitmap& rSalBmpImpl, USHORT nNewBitCount );

    //void                Destroy();

    //Size                GetSize() const { return maSize; }
    //USHORT              GetBitCount() const { return mnBitCount; }

    //BitmapBuffer*     AcquireBuffer( bool bReadOnly );
    //void              ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    bool                        Create( HANDLE hBitmap, bool bDIB, bool bCopyHandle );
    virtual bool                Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
    virtual bool                Create( const SalBitmap& rSalBmpImpl );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, USHORT nNewBitCount );

    virtual void                Destroy();

    virtual Size                GetSize() const { return maSize; }
    virtual USHORT              GetBitCount() const { return mnBitCount; }

    virtual BitmapBuffer*       AcquireBuffer( bool bReadOnly );
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    virtual bool                GetSystemData( BitmapSystemData& rData );
};

#endif // _SV_SALBMP_H
