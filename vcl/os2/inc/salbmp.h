/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salbmp.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:43:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _SV_SALBMP_HXX
#include <vcl/salbmp.hxx>
#endif

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
    static HANDLE       ImplCopyDIBOrDDB( HANDLE hHdl, BOOL bDIB );
    static USHORT       ImplGetDIBColorCount( HANDLE hDIB );
    static void         ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                             const Size& rSizePixel, BOOL bRLE4 );

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
