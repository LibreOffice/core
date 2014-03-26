/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_FIXED_HXX
#define INCLUDED_VCL_FIXED_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/edit.hxx>
#include <vcl/image.hxx>

class UserDrawEvent;


// - FixedText -


class VCL_DLLPUBLIC FixedText : public Control
{
private:
    sal_Int32 m_nMaxWidthChars;
    sal_Int32 m_nMinWidthChars;
    Window *m_pMnemonicWindow;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize, bool bFillLayout = false ) const;
public:
    SAL_DLLPRIVATE static sal_uInt16   ImplGetTextStyle( WinBits nWinBits );
protected:
    virtual void    FillLayoutData() const SAL_OVERRIDE;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const SAL_OVERRIDE;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const SAL_OVERRIDE;

    virtual Window* getAccessibleRelationLabelFor() const SAL_OVERRIDE;

public:
    explicit        FixedText( Window* pParent, WinBits nStyle = 0 );
    explicit        FixedText( Window* pParent, const ResId& rResId );
    virtual         ~FixedText();

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void setMaxWidthChars(sal_Int32 nWidth);
    sal_Int32 getMaxWidthChars() const { return m_nMaxWidthChars; }
    void setMinWidthChars(sal_Int32 nWidth);
    sal_Int32 getMinWidthChars() const { return m_nMinWidthChars; }
    static Size     CalcMinimumTextSize(Control const* pControl, long nMaxWidth = 0x7fffffff);
    static Size     getTextDimensions(Control const *pControl, const OUString &rTxt, long nMaxWidth);
    Size            CalcMinimumSize(long nMaxWidth = 0x7fffffff) const;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    void set_mnemonic_widget(Window *pWindow);
    Window* get_mnemonic_widget() const { return m_pMnemonicWindow; }
};

class VCL_DLLPUBLIC SelectableFixedText : public Edit
{
public:
    explicit SelectableFixedText( Window* pParent, WinBits nStyle = 0 );

    virtual void    LoseFocus() SAL_OVERRIDE;
};


// - FixedLine -


class VCL_DLLPUBLIC FixedLine : public Control
{
private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void    ImplDraw( bool bLayout = false );

protected:
    virtual void    FillLayoutData() const SAL_OVERRIDE;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const SAL_OVERRIDE;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const SAL_OVERRIDE;

public:
    explicit        FixedLine( Window* pParent, WinBits nStyle = WB_HORZ );
    explicit        FixedLine( Window* pParent, const ResId& );

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
};


// - FixedBitmap -


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
    explicit        FixedBitmap( Window* pParent, WinBits nStyle = 0 );
    explicit        FixedBitmap( Window* pParent, const ResId& );
    virtual         ~FixedBitmap();

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void            SetBitmap( const Bitmap& rBitmap );
    using OutputDevice::GetBitmap;
    const Bitmap&   GetBitmap() const { return maBitmap; }
};


// - FixedImage -


class VCL_DLLPUBLIC FixedImage : public Control
{
private:
    Image           maImage;
    bool            mbInUserDraw;

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

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;

    void            SetImage( const Image& rImage );
    const Image&    GetImage() const { return maImage; }

    bool        SetModeImage( const Image& rImage );
    const Image&    GetModeImage( ) const;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;

    static Image loadThemeImage(const OString &rFileName);
};

#endif // INCLUDED_VCL_FIXED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
