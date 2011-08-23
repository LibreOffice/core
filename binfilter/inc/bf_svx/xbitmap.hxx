/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_XBITMAP_HXX
#define _SVX_XBITMAP_HXX

#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <bf_goodies/graphicobject.hxx>
#include <bf_svx/xenum.hxx>
namespace binfilter {

//---------------
// class XOBitmap
//---------------

#if defined HP9000 || defined SINIX
static Size aXOBitmapDefaultSize( 8, 8 );
#endif

class XOBitmap
{
protected:
    XBitmapType     eType;
    XBitmapStyle    eStyle;
    BfGraphicObject aGraphicObject;
    USHORT*         pPixelArray;
    Size            aArraySize;
    Color           aPixelColor;
    Color           aBckgrColor;
    BOOL            bGraphicDirty;

public:
    XOBitmap();
    XOBitmap( const BfGraphicObject& rGraphicObject, XBitmapStyle eStyle = XBITMAP_TILE );
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

    void SetGraphicObject( const BfGraphicObject& rObj )	{ aGraphicObject = rObj; bGraphicDirty = FALSE; }
    void SetBitmap( const Bitmap& rBmp )				{ aGraphicObject = BfGraphicObject( Graphic( rBmp ) ); bGraphicDirty = FALSE; }
    void SetBitmapType( XBitmapType eNewType )			{ eType = eNewType; }
    void SetBitmapStyle( XBitmapStyle eNewStyle )		{ eStyle = eNewStyle; }
    void SetPixelArray( const USHORT* pArray );
    void SetPixelSize( const Size& rSize )				{ aArraySize  = rSize;  bGraphicDirty = TRUE; }
    void SetPixelColor( const Color& rColor )			{ aPixelColor = rColor; bGraphicDirty = TRUE; }
    void SetBackgroundColor( const Color& rColor )		{ aBckgrColor = rColor; bGraphicDirty = TRUE; }

    XBitmapType				GetBitmapType() const				{ return eType; }
    XBitmapStyle			GetBitmapStyle() const				{ return eStyle; }
    const BfGraphicObject&	GetGraphicObject() const;
    Bitmap					GetBitmap() const;
    USHORT*					GetPixelArray() const				{ return pPixelArray; }
    Color					GetPixelColor() const				{ return aPixelColor; }
    Color					GetBackgroundColor() const			{ return aBckgrColor; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
