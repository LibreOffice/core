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



#ifndef _SV_ALPHA_HXX
#define _SV_ALPHA_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>

// -------------
// - AlphaMask -
// -------------

class ImageList;
class BitmapEx;

class VCL_DLLPUBLIC AlphaMask : private Bitmap
{
private:
    friend class BitmapEx;
    friend class OutputDevice;
    friend bool VCL_DLLPUBLIC ReadDIBBitmapEx(BitmapEx& rTarget, SvStream& rIStm);

    SAL_DLLPRIVATE const Bitmap&    ImplGetBitmap() const;
    SAL_DLLPRIVATE void             ImplSetBitmap( const Bitmap& rBitmap );

public:

                            AlphaMask();
                            AlphaMask( const Bitmap& rBitmap );
                            AlphaMask( const AlphaMask& rAlphaMask );
                            AlphaMask( const Size& rSizePixel, sal_uInt8* pEraseTransparency = NULL );
                            ~AlphaMask();

    AlphaMask&              operator=( const Bitmap& rBitmap );
    AlphaMask&              operator=( const AlphaMask& rAlphaMask ) { return (AlphaMask&) Bitmap::operator=( rAlphaMask ); }
    sal_Bool                    operator!() const { return Bitmap::operator!(); }
    sal_Bool                    operator==( const AlphaMask& rAlphaMask ) const { return Bitmap::operator==( rAlphaMask ); }
    sal_Bool                    operator!=( const AlphaMask& rAlphaMask ) const { return Bitmap::operator!=( rAlphaMask ); }

    const MapMode&          GetPrefMapMode() const { return Bitmap::GetPrefMapMode(); }
    void                    SetPrefMapMode( const MapMode& rMapMode ) { Bitmap::SetPrefMapMode( rMapMode ); }

    const Size&             GetPrefSize() const { return Bitmap::GetPrefSize(); }
    void                    SetPrefSize( const Size& rSize ) { Bitmap::SetPrefSize( rSize ); }

    Size                    GetSizePixel() const { return Bitmap::GetSizePixel(); }
    void                    SetSizePixel( const Size& rNewSize, sal_uInt32 nScaleFlag = BMP_SCALE_FASTESTINTERPOLATE )
    {
        Bitmap::SetSizePixel( rNewSize, nScaleFlag );
    }

    sal_uLong                   GetSizeBytes() const { return Bitmap::GetSizeBytes(); }
    sal_uLong                   GetChecksum() const { return Bitmap::GetChecksum(); }

    Bitmap                  GetBitmap() const;

public:

    sal_Bool                    Crop( const Rectangle& rRectPixel );
    sal_Bool                    Expand( sal_uLong nDX, sal_uLong nDY, sal_uInt8* pInitTransparency = NULL );
    sal_Bool                    CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc, const AlphaMask* pAlphaSrc = NULL );
    sal_Bool                    Erase( sal_uInt8 cTransparency );
    sal_Bool                    Invert();
    sal_Bool                    Mirror( sal_uLong nMirrorFlags );
    sal_Bool                    Scale( const Size& rNewSize, sal_uInt32 nScaleFlag = BMP_SCALE_FASTESTINTERPOLATE );
    sal_Bool                    Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag = BMP_SCALE_FASTESTINTERPOLATE );
    sal_Bool                    Rotate( long nAngle10, sal_uInt8 cFillTransparency );
    sal_Bool                    Replace( const Bitmap& rMask, sal_uInt8 rReplaceTransparency );
    sal_Bool                    Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency, sal_uLong nTol = 0UL );
    sal_Bool                    Replace( sal_uInt8* pSearchTransparencies, sal_uInt8* pReplaceTransparencies,
                                     sal_uLong nColorCount, sal_uLong* pTols = NULL );

public:

    BitmapReadAccess*       AcquireReadAccess() { return Bitmap::AcquireReadAccess(); }
    BitmapWriteAccess*      AcquireWriteAccess() { return Bitmap::AcquireWriteAccess(); }
    void                    ReleaseAccess( BitmapReadAccess* pAccess );
};

#endif // _SV_ALPHA_HXX
// eof
