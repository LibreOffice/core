/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salbmp.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:43:15 $
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

#ifndef _VCL_DLLAPI_H
#include "dllapi.h"
#endif

struct  BitmapBuffer;
class   BitmapPalette;
class   SalGraphics;
class   ImplSalDDB;
class   ImplSalBitmapCache;

// -------------
// - SalBitmap -
// -------------

class VCL_DLLPUBLIC X11SalBitmap : public SalBitmap
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

    SAL_DLLPRIVATE bool    ImplCreateFromDrawable(
#ifdef _USE_PRINT_EXTENSION_
                                            SalDisplay* pDisplay,
#endif
                                            Drawable aDrawable,
                                            long nDrawableDepth,
                                            long nX, long nY,
                                            long nWidth, long nHeight );

    SAL_DLLPRIVATE XImage* ImplCreateXImage(
                                            SalDisplay* pSalDisp, long nDepth,
                                              const SalTwoRect& rTwoRect ) const;
#ifdef _USE_PRINT_EXTENSION_
    void    ImplDraw( SalDisplay *pDisplay, Drawable aDrawable, long nDrawableDepth,
                          const SalTwoRect& rTwoRect, const GC& rGC, bool bDstIsWindow ) const;
#else
    void    ImplDraw( Drawable aDrawable, long nDrawableDepth,
                          const SalTwoRect& rTwoRect, const GC& rGC, bool bDstIsWindow ) const;
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
    virtual bool            GetSystemData( BitmapSystemData& rData );
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
                              long nDestX, long nDestY, const GC& rGC, bool bDstIsWindow );

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
                                const SalTwoRect& rTwoRect, const GC& rGC, bool bDstIsWindow ) const;
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












































