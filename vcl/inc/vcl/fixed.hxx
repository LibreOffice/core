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



#ifndef _SV_FIXED_HXX
#define _SV_FIXED_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>

class UserDrawEvent;

// -------------
// - FixedText -
// -------------

class VCL_DLLPUBLIC FixedText : public Control
{
//#if 0 // _SOLAR__PRIVATE
private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize, bool bFillLayout = false ) const;
public:
    SAL_DLLPRIVATE static sal_uInt16   ImplGetTextStyle( WinBits nWinBits );
//#endif
protected:
    virtual void    FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

public:
    explicit        FixedText( Window* pParent, WinBits nStyle = 0 );
    explicit        FixedText( Window* pParent, const ResId& );
    explicit        FixedText( Window* pParent, const ResId&, bool bDisableAccessibleLabelForRelation );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    static Size     CalcMinimumTextSize( Control const* pControl, long nMaxWidth = 0 );
    Size            CalcMinimumSize( long nMaxWidth = 0 ) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
};

// -------------
// - FixedLine -
// -------------

class VCL_DLLPUBLIC FixedLine : public Control
{
private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void    ImplDraw( bool bLayout = false );

protected:
    virtual void    FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

public:
    explicit        FixedLine( Window* pParent, WinBits nStyle = WB_HORZ );
    explicit        FixedLine( Window* pParent, const ResId& );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size    GetOptimalSize(WindowSizeType eType) const;
};

// ---------------
// - FixedBitmap -
// ---------------

class VCL_DLLPUBLIC FixedBitmap : public Control
{
private:
    Bitmap          maBitmap;
    Bitmap          maBitmapHC;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize );

protected:
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
    explicit        FixedBitmap( Window* pParent, WinBits nStyle = 0 );
    explicit        FixedBitmap( Window* pParent, const ResId& );
    virtual         ~FixedBitmap();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetBitmap( const Bitmap& rBitmap );
    using OutputDevice::GetBitmap;
    const Bitmap&   GetBitmap() const { return maBitmap; }
    sal_Bool            SetModeBitmap( const Bitmap& rBitmap, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Bitmap&   GetModeBitmap( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;
};

// --------------
// - FixedImage -
// --------------

class VCL_DLLPUBLIC FixedImage : public Control
{
private:
    Image           maImage;
    Image           maImageHC;
    sal_Bool            mbInUserDraw;

private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();

protected:
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize );
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
    explicit        FixedImage( Window* pParent, WinBits nStyle = 0 );
    explicit        FixedImage( Window* pParent, const ResId& );
    virtual         ~FixedImage();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    void            SetImage( const Image& rImage );
    const Image&    GetImage() const { return maImage; }

    sal_Bool            SetModeImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image&    GetModeImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;

    Point           CalcImagePos( const Point& rPos,
                                  const Size& rObjSize, const Size& rWinSize );
};

#endif  // _SV_FIXED_HXX

