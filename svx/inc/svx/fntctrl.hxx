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
#ifndef _SVX_FNTCTRL_HXX
#define _SVX_FNTCTRL_HXX

#include <vcl/window.hxx>
#include <editeng/svxfont.hxx>
#include "svx/svxdllapi.h"
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

#include <rtl/ustring.hxx>

// forward ---------------------------------------------------------------

class SfxItemSet;
class FontPrevWin_Impl;

// class SvxFontPrevWindow -----------------------------------------------

class SVX_DLLPUBLIC SvxFontPrevWindow : public Window
{
    using OutputDevice::SetFont;
private:
    FontPrevWin_Impl*   pImpl;

    SVX_DLLPRIVATE void InitSettings( sal_Bool bForeground, sal_Bool bBackground );
    SVX_DLLPRIVATE void Init ();
    SVX_DLLPRIVATE void SetFontSize(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont);
    SVX_DLLPRIVATE void SetFontLang(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont);

    Size m_aInitialSize;

public:
                        SvxFontPrevWindow( Window* pParent, const ResId& rId );
                        SvxFontPrevWindow(Window* pParent);
    virtual             ~SvxFontPrevWindow();

    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                Init( const SfxItemSet& rSet );

    // for reasons of efficiency not const
    SvxFont&            GetFont();
    const SvxFont&      GetFont() const;
    void                SetFont( const SvxFont& rNormalFont, const SvxFont& rCJKFont, const SvxFont& rCTLFont );
    SvxFont&            GetCJKFont();
    SvxFont&            GetCTLFont();
    void                SetColor( const Color& rColor );
    void                ResetColor();
    void                SetBackColor( const Color& rColor );
    void                UseResourceText( sal_Bool bUse = sal_True );
    void                Paint( const Rectangle& );

    sal_Bool            IsTwoLines() const;
    void                SetTwoLines(sal_Bool bSet);

    void                SetBrackets(sal_Unicode cStart, sal_Unicode cEnd);

    void                SetFontWidthScale( sal_uInt16 nScaleInPercent );

    void                AutoCorrectFontColor( void );

    void                SetPreviewText( const ::rtl::OUString& rString );
    void                SetFontNameAsPreviewText();

    void                SetFont( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont );
    void                SetFontStyle( const SfxItemSet& rSet, sal_uInt16 nSlotPosture, sal_uInt16 nSlotWeight, SvxFont& rFont ); // posture/weight
    void                SetFontWidthScale( const SfxItemSet& rSet );
    void                SetFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );

    void                SetFromItemSet( const SfxItemSet &rSet,
                                        bool bPreviewBackgroundToCharacter = false );

    virtual Size GetOptimalSize(WindowSizeType eType) const;
};

#endif // #ifndef _SVX_FNTCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
