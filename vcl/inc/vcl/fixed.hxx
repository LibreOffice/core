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

#ifndef _SV_FIXED_HXX
#define _SV_FIXED_HXX

#include <tools/solar.h>
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
private:
    sal_Int32 m_nMaxWidthChars;
    sal_Int32 m_nMinWidthChars;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize, bool bFillLayout = false ) const;
public:
    SAL_DLLPRIVATE static sal_uInt16   ImplGetTextStyle( WinBits nWinBits );
protected:
    virtual void    FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

public:
                    FixedText( Window* pParent, WinBits nStyle = 0 );
                    FixedText( Window* pParent, const ResId& rResId );
                    FixedText( Window* pParent, const ResId& rResId, bool bDisableAccessibleLabelForRelation );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void setMaxWidthChars(sal_Int32 nWidth);
    sal_Int32 getMaxWidthChars() const { return m_nMaxWidthChars; }
    void setMinWidthChars(sal_Int32 nWidth);
    sal_Int32 getMinWidthChars() const { return m_nMinWidthChars; }
    static Size     CalcMinimumTextSize(Control const* pControl, long nMaxWidth = 0x7fffffff);
    static Size     getTextDimensions(Control const *pControl, const OUString &rTxt, long nMaxWidth);
    Size            CalcMinimumSize(long nMaxWidth = 0x7fffffff) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    virtual void take_properties(Window &rOther);
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
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
                    FixedLine( Window* pParent, WinBits nStyle = WB_HORZ );
                    FixedLine( Window* pParent, const ResId& rResId );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    virtual void take_properties(Window &rOther);
};

// ---------------
// - FixedBitmap -
// ---------------

class VCL_DLLPUBLIC FixedBitmap : public Control
{
private:
    Bitmap          maBitmap;

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
                    FixedBitmap( Window* pParent, WinBits nStyle = 0 );
                    FixedBitmap( Window* pParent, const ResId& rResId );
                    ~FixedBitmap();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetBitmap( const Bitmap& rBitmap );
    using OutputDevice::GetBitmap;
    const Bitmap&   GetBitmap() const { return maBitmap; }
};

// --------------
// - FixedImage -
// --------------

class VCL_DLLPUBLIC FixedImage : public Control
{
private:
    Image           maImage;
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
                    FixedImage( Window* pParent, WinBits nStyle = 0 );
                    FixedImage( Window* pParent, const ResId& rResId );
                    ~FixedImage();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    void            SetImage( const Image& rImage );
    const Image&    GetImage() const { return maImage; }

    sal_Bool        SetModeImage( const Image& rImage );
    const Image&    GetModeImage( ) const;
;
};

#endif  // _SV_FIXED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
