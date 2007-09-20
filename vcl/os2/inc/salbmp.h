/*************************************************************************
 *
 *  $RCSfile: salbmp.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:01:23 $
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

#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
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
