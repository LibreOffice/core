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

public:
                        SvxFontPrevWindow( Window* pParent, const ResId& rId );
                        SvxFontPrevWindow(Window* pParent, WinBits nStyle);
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
    void                UseResourceText( bool bUse = true );
    void                Paint( const Rectangle& );

    bool            IsTwoLines() const;
    void                SetTwoLines(bool bSet);

    void                SetBrackets(sal_Unicode cStart, sal_Unicode cEnd);

    void                SetFontWidthScale( sal_uInt16 nScaleInPercent );

    void                AutoCorrectFontColor( void );

    void                SetPreviewText( const OUString& rString );
    void                SetFontNameAsPreviewText();

    void                SetFont( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont );
    void                SetFontStyle( const SfxItemSet& rSet, sal_uInt16 nSlotPosture, sal_uInt16 nSlotWeight, SvxFont& rFont ); // posture/weight
    void                SetFontWidthScale( const SfxItemSet& rSet );
    void                SetFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );

    void                SetFromItemSet( const SfxItemSet &rSet,
                                        bool bPreviewBackgroundToCharacter = false );

    virtual Size GetOptimalSize() const;
};

#endif // #ifndef _SVX_FNTCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
