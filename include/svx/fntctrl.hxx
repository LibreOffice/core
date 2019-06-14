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
#ifndef INCLUDED_SVX_FNTCTRL_HXX
#define INCLUDED_SVX_FNTCTRL_HXX

#include <memory>
#include <vcl/customweld.hxx>
#include <vcl/window.hxx>
#include <editeng/svxfont.hxx>
#include <svx/svxdllapi.h>

#include <rtl/ustring.hxx>

class SfxItemSet;
class FontPrevWin_Impl;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxFontPrevWindow : public vcl::Window
{
    using OutputDevice::SetFont;
private:
    std::unique_ptr<FontPrevWin_Impl> pImpl;
    bool mbResetForeground : 1;
    bool mbResetBackground : 1;

    SVX_DLLPRIVATE void ResetSettings(bool bForeground, bool bBackground);
    SVX_DLLPRIVATE void ApplySettings(vcl::RenderContext& rRenderContext) override;
    SVX_DLLPRIVATE void Init ();

public:
                        SvxFontPrevWindow(vcl::Window* pParent, WinBits nStyle);
    virtual             ~SvxFontPrevWindow() override;
    virtual void        dispose() override;

    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    // for reasons of efficiency not const
    SvxFont&            GetFont();
    const SvxFont&      GetFont() const;
    void                SetFont( const SvxFont& rNormalFont, const SvxFont& rCJKFont, const SvxFont& rCTLFont );
    SvxFont&            GetCJKFont();
    SvxFont&            GetCTLFont();
    void                SetColor( const Color& rColor );
    void                ResetColor();
    void                SetBackColor( const Color& rColor );
    void                UseResourceText();
    void                Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

    bool                IsTwoLines() const;

    virtual Size GetOptimalSize() const override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC FontPrevWindow : public weld::CustomWidgetController
{
private:
    std::unique_ptr<FontPrevWin_Impl> pImpl;

    SVX_DLLPRIVATE static void ApplySettings(vcl::RenderContext& rRenderContext);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    SVX_DLLPRIVATE static void SetFontSize(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont);
    SVX_DLLPRIVATE static void SetFontLang(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont);

public:
                        FontPrevWindow();
    virtual             ~FontPrevWindow() override;

    SvxFont&            GetFont();
    const SvxFont&      GetFont() const;
    void                SetFont( const SvxFont& rNormalFont, const SvxFont& rCJKFont, const SvxFont& rCTLFont );
    SvxFont&            GetCJKFont();
    SvxFont&            GetCTLFont();
    void                SetBackColor( const Color& rColor );
    void                SetTextLineColor(const Color& rColor);
    void                SetOverlineColor(const Color& rColor);
    void                Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

    bool                IsTwoLines() const;
    void                SetTwoLines(bool bSet);

    void                SetBrackets(sal_Unicode cStart, sal_Unicode cEnd);

    void                SetFontWidthScale( sal_uInt16 nScaleInPercent );

    void                AutoCorrectFontColor();

    void                SetPreviewText( const OUString& rString );
    void                SetFontNameAsPreviewText();

    void                SetFromItemSet( const SfxItemSet &rSet,
                                        bool bPreviewBackgroundToCharacter );
};

#endif // INCLUDED_SVX_FNTCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
