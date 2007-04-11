/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xbitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:48:51 $
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

#ifndef _SVX_XBITMAP_HXX
#define _SVX_XBITMAP_HXX

#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _GRFMGR_HXX //autogen
#include <goodies/grfmgr.hxx>
#endif
#include <svx/xenum.hxx>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//---------------
// class XOBitmap
//---------------

#if defined HP9000 || defined SINIX
static Size aXOBitmapDefaultSize( 8, 8 );
#endif

class SVX_DLLPUBLIC XOBitmap
{
protected:
    XBitmapType     eType;
    XBitmapStyle    eStyle;
    GraphicObject   aGraphicObject;
    USHORT*         pPixelArray;
    Size            aArraySize;
    Color           aPixelColor;
    Color           aBckgrColor;
    BOOL            bGraphicDirty;

public:
    XOBitmap();
    XOBitmap( const GraphicObject& rGraphicObject, XBitmapStyle eStyle = XBITMAP_TILE );
    XOBitmap( const Bitmap& rBitmap, XBitmapStyle eStyle = XBITMAP_TILE );
#if defined HP9000 || defined SINIX
    XOBitmap( const USHORT* pArray, const Color& aPixelColor,
             const Color& aBckgrColor, const Size& rSize = aXOBitmapDefaultSize,
             XBitmapStyle eStyle = XBITMAP_TILE );
#else
    XOBitmap( const USHORT* pArray, const Color& aPixelColor,
             const Color& aBckgrColor, const Size& rSize = Size( 8, 8 ),
             XBitmapStyle eStyle = XBITMAP_TILE );
#endif
    XOBitmap( const XOBitmap& rXBmp );
    ~XOBitmap();

    XOBitmap& operator=( const XOBitmap& rXOBitmap );
    int      operator==( const XOBitmap& rXOBitmap ) const;

    void Bitmap2Array();
    void Array2Bitmap();

    void SetGraphicObject( const GraphicObject& rObj )  { aGraphicObject = rObj; bGraphicDirty = FALSE; }
    void SetBitmap( const Bitmap& rBmp )                { aGraphicObject = GraphicObject( Graphic( rBmp ) ); bGraphicDirty = FALSE; }
    void SetBitmapType( XBitmapType eNewType )          { eType = eNewType; }
    void SetBitmapStyle( XBitmapStyle eNewStyle )       { eStyle = eNewStyle; }
    void SetPixelArray( const USHORT* pArray );
    void SetPixelSize( const Size& rSize )              { aArraySize  = rSize;  bGraphicDirty = TRUE; }
    void SetPixelColor( const Color& rColor )           { aPixelColor = rColor; bGraphicDirty = TRUE; }
    void SetBackgroundColor( const Color& rColor )      { aBckgrColor = rColor; bGraphicDirty = TRUE; }

    XBitmapType             GetBitmapType() const               { return eType; }
    XBitmapStyle            GetBitmapStyle() const              { return eStyle; }
    const GraphicObject&    GetGraphicObject() const;
    Bitmap                  GetBitmap() const;
    USHORT*                 GetPixelArray() const               { return pPixelArray; }
    Color                   GetPixelColor() const               { return aPixelColor; }
    Color                   GetBackgroundColor() const          { return aBckgrColor; }
};

#endif

