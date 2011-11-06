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



#ifndef _SVX_XBITMAP_HXX
#define _SVX_XBITMAP_HXX

#include <tools/color.hxx>
#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#include <svtools/grfmgr.hxx>
#include <svx/xenum.hxx>
#include "svx/svxdllapi.h"

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
    sal_uInt16*         pPixelArray;
    Size            aArraySize;
    Color           aPixelColor;
    Color           aBckgrColor;
    sal_Bool            bGraphicDirty;

public:
    XOBitmap();
    XOBitmap( const GraphicObject& rGraphicObject, XBitmapStyle eStyle = XBITMAP_TILE );
    XOBitmap( const Bitmap& rBitmap, XBitmapStyle eStyle = XBITMAP_TILE );
#if defined HP9000 || defined SINIX
    XOBitmap( const sal_uInt16* pArray, const Color& aPixelColor,
             const Color& aBckgrColor, const Size& rSize = aXOBitmapDefaultSize,
             XBitmapStyle eStyle = XBITMAP_TILE );
#else
    XOBitmap( const sal_uInt16* pArray, const Color& aPixelColor,
             const Color& aBckgrColor, const Size& rSize = Size( 8, 8 ),
             XBitmapStyle eStyle = XBITMAP_TILE );
#endif
    XOBitmap( const XOBitmap& rXBmp );
    ~XOBitmap();

    XOBitmap& operator=( const XOBitmap& rXOBitmap );
    int      operator==( const XOBitmap& rXOBitmap ) const;

    void Bitmap2Array();
    void Array2Bitmap();

    void SetGraphicObject( const GraphicObject& rObj )  { aGraphicObject = rObj; bGraphicDirty = sal_False; }
    void SetBitmap( const Bitmap& rBmp )                { aGraphicObject = GraphicObject( Graphic( rBmp ) ); bGraphicDirty = sal_False; }
    void SetBitmapType( XBitmapType eNewType )          { eType = eNewType; }
    void SetBitmapStyle( XBitmapStyle eNewStyle )       { eStyle = eNewStyle; }
    void SetPixelArray( const sal_uInt16* pArray );
    void SetPixelSize( const Size& rSize )              { aArraySize  = rSize;  bGraphicDirty = sal_True; }
    void SetPixelColor( const Color& rColor )           { aPixelColor = rColor; bGraphicDirty = sal_True; }
    void SetBackgroundColor( const Color& rColor )      { aBckgrColor = rColor; bGraphicDirty = sal_True; }

    XBitmapType             GetBitmapType() const               { return eType; }
    XBitmapStyle            GetBitmapStyle() const              { return eStyle; }
    const GraphicObject&    GetGraphicObject() const;
    Bitmap                  GetBitmap() const;
    sal_uInt16*                 GetPixelArray() const               { return pPixelArray; }
    Color                   GetPixelColor() const               { return aPixelColor; }
    Color                   GetBackgroundColor() const          { return aBckgrColor; }
};

#endif

