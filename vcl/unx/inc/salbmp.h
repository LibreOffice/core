/*************************************************************************
 *
 *  $RCSfile: salbmp.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:36:58 $
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

#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif
#ifndef _SV_SALGTYPE
#include <salgtype.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif

struct  BitmapBuffer;
class   BitmapPalette;
class   SalGraphics;
class   ImplSalDDB;
class   ImplSalBitmapCache;

// -------------
// - SalBitmap -
// -------------

class X11SalBitmap : public SalBitmap
{
private:

    static BitmapBuffer*        ImplCreateDIB( const Size& rSize,
                                               USHORT nBitCount,
                                               const BitmapPalette& rPal );
    static BitmapBuffer*        ImplCreateDIB( Drawable aDrawable,
                                               long nDrawableDepth,
                                               long nX, long nY,
                                               long nWidth, long nHeight );

public:

    static ImplSalBitmapCache*  mpCache;
    static ULONG                mnCacheInstCount;

    static void                 ImplCreateCache();
    static void                 ImplDestroyCache();
    void                        ImplRemovedFromCache();

    bool                        SnapShot (Display* pDisplay, XLIB_Window hWindow);
    bool                        ImplCreateFromXImage (Display* pDisplay, XLIB_Window hWindow,
                                                      XImage* pImage);
private:


    BitmapBuffer*   mpDIB;
    ImplSalDDB*     mpDDB;

public:

    bool            ImplCreateFromDrawable(
#ifdef _USE_PRINT_EXTENSION_
                                            SalDisplay* pDisplay,
#endif
                                            Drawable aDrawable,
                                            long nDrawableDepth,
                                            long nX, long nY,
                                            long nWidth, long nHeight );

    XImage*         ImplCreateXImage(
                                            SalDisplay* pSalDisp, long nDepth,
                                              const SalTwoRect& rTwoRect ) const;
#ifdef _USE_PRINT_EXTENSION_
    void            ImplDraw( SalDisplay *pDisplay, Drawable aDrawable, long nDrawableDepth,
                              const SalTwoRect& rTwoRect, const GC& rGC ) const;
#else
    void            ImplDraw( Drawable aDrawable, long nDrawableDepth,
                              const SalTwoRect& rTwoRect, const GC& rGC ) const;
#endif

public:

    X11SalBitmap();
    virtual ~X11SalBitmap();

    // overload pure virtual methods
    virtual bool            Create( const Size& rSize,
                            USHORT nBitCount,
                            const BitmapPalette& rPal );
    virtual bool            Create( const SalBitmap& rSalBmp );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    USHORT nNewBitCount );

    virtual void            Destroy();

    virtual Size            GetSize() const;
    virtual USHORT          GetBitCount() const;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly );
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
};

// --------------
// - ImplSalDDB -
// --------------

class ImplSalDDB
{
private:

    Pixmap          maPixmap;
    SalTwoRect      maTwoRect;
    long            mnDepth;

                    ImplSalDDB() {}

    static void ImplDraw(
#ifdef _USE_PRINT_EXTENSION_
                                SalDisplay* pDisplay,
#endif
    Drawable aSrcDrawable, long nSrcDrawableDepth,
                              Drawable aDstDrawable, long nDstDrawableDepth,
                              long nSrcX, long nSrcY,
                              long nDestWidth, long nDestHeight,
                              long nDestX, long nDestY, const GC& rGC );

public:

                    ImplSalDDB(
#ifdef _USE_PRINT_EXTENSION_
                                SalDisplay* pDisplay,
#endif
                                XImage* pImage, Drawable aDrawable,
                                const SalTwoRect& rTwoRect );
                    ImplSalDDB(
#ifdef _USE_PRINT_EXTENSION_
                                SalDisplay* pDisplay,
#endif
                                Drawable aDrawable, long nDrawableDepth,
                                long nX, long nY, long nWidth, long nHeight );
                    ImplSalDDB (Display* pDisplay, XLIB_Window hWindow, XImage* pImage);
                    ~ImplSalDDB();

    Pixmap          ImplGetPixmap() const { return maPixmap; }
    long            ImplGetWidth() const { return maTwoRect.mnDestWidth; }
    long            ImplGetHeight() const { return maTwoRect.mnDestHeight; }
    long            ImplGetDepth() const { return mnDepth; }
    ULONG           ImplGetMemSize() const { return( ( maTwoRect.mnDestWidth * maTwoRect.mnDestHeight * mnDepth ) >> 3 ); }

    bool            ImplMatches( long nDepth, const SalTwoRect& rTwoRect ) const;
    void            ImplDraw(
#ifdef _USE_PRINT_EXTENSION_
                                SalDisplay* pDisplay,
#endif
                                Drawable aDrawable, long nDrawableDepth,
                                const SalTwoRect& rTwoRect, const GC& rGC ) const;
};

// ----------------------
// - ImplSalBitmapCache -
// ----------------------

class ImplSalBitmapCache
{
private:

    List            maBmpList;
    ULONG           mnTotalSize;

public:

                    ImplSalBitmapCache();
                    ~ImplSalBitmapCache();

    void            ImplAdd( X11SalBitmap* pBmp, ULONG nMemSize = 0UL, ULONG nFlags = 0UL );
    void            ImplRemove( X11SalBitmap* pBmp );
    void            ImplClear();
};

#endif // _SV_SALBMP_HXX












































