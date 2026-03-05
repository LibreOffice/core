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

#ifndef INCLUDED_SVX_AUTOFORMATHELPER_HXX
#define INCLUDED_SVX_AUTOFORMATHELPER_HXX

#include <svx/svxdllapi.h>
#include <memory>

class SvStream;
class SvxFontItem;
class SvxFontHeightItem;
class SvxWeightItem;
class SvxPostureItem;
class SvxUnderlineItem;
class SvxOverlineItem;
class SvxCrossedOutItem;
class SvxContourItem;
class SvxShadowedItem;
class SvxColorItem;
class SvxBoxItem;
class SvxLineItem;
class SvxBrushItem;
class SvxAdjustItem;
class SvxHorJustifyItem;
class SvxVerJustifyItem;
class SfxBoolItem;
class SvxMarginItem;
class SfxInt32Item;
class SvxRotateModeItem;

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC AutoFormatBase
{
protected:
    // common attributes of Calc and Writer
    // --- from 641 on: CJK and CTL font settings
    std::unique_ptr<SvxFontItem>            m_aFont;
    std::unique_ptr<SvxFontHeightItem>      m_aHeight;
    std::unique_ptr<SvxWeightItem>          m_aWeight;
    std::unique_ptr<SvxPostureItem>         m_aPosture;

    std::unique_ptr<SvxFontItem>            m_aCJKFont;
    std::unique_ptr<SvxFontHeightItem>      m_aCJKHeight;
    std::unique_ptr<SvxWeightItem>          m_aCJKWeight;
    std::unique_ptr<SvxPostureItem>         m_aCJKPosture;

    std::unique_ptr<SvxFontItem>            m_aCTLFont;
    std::unique_ptr<SvxFontHeightItem>      m_aCTLHeight;
    std::unique_ptr<SvxWeightItem>          m_aCTLWeight;
    std::unique_ptr<SvxPostureItem>         m_aCTLPosture;

    std::unique_ptr<SvxUnderlineItem>       m_aUnderline;
    std::unique_ptr<SvxOverlineItem>        m_aOverline;
    std::unique_ptr<SvxCrossedOutItem>      m_aCrossedOut;
    std::unique_ptr<SvxContourItem>         m_aContour;
    std::unique_ptr<SvxShadowedItem>        m_aShadowed;
    std::unique_ptr<SvxColorItem>           m_aColor;
    std::unique_ptr<SvxBoxItem>             m_aBox;
    std::unique_ptr<SvxLineItem>            m_aTLBR;
    std::unique_ptr<SvxLineItem>            m_aBLTR;
    std::unique_ptr<SvxBrushItem>           m_aBackground;

    // Writer specific
    std::unique_ptr<SvxAdjustItem>          m_aAdjust;

    std::unique_ptr<SvxHorJustifyItem>      m_aHorJustify;
    std::unique_ptr<SvxVerJustifyItem>      m_aVerJustify;

    AutoFormatBase();
    AutoFormatBase( const AutoFormatBase& rNew );
    ~AutoFormatBase();

    /// Comparing based of boxes backgrounds.
    bool operator==(const AutoFormatBase& rRight) const;

public:
    // The get-methods.
    const SvxFontItem       &GetFont() const        { return *m_aFont; }
    const SvxFontHeightItem &GetHeight() const      { return *m_aHeight; }
    const SvxWeightItem     &GetWeight() const      { return *m_aWeight; }
    const SvxPostureItem    &GetPosture() const     { return *m_aPosture; }
    const SvxFontItem       &GetCJKFont() const     { return *m_aCJKFont; }
    const SvxFontHeightItem &GetCJKHeight() const   { return *m_aCJKHeight; }
    const SvxWeightItem     &GetCJKWeight() const   { return *m_aCJKWeight; }
    const SvxPostureItem    &GetCJKPosture() const  { return *m_aCJKPosture; }
    const SvxFontItem       &GetCTLFont() const     { return *m_aCTLFont; }
    const SvxFontHeightItem &GetCTLHeight() const   { return *m_aCTLHeight; }
    const SvxWeightItem     &GetCTLWeight() const   { return *m_aCTLWeight; }
    const SvxPostureItem    &GetCTLPosture() const  { return *m_aCTLPosture; }
    const SvxUnderlineItem  &GetUnderline() const   { return *m_aUnderline; }
    const SvxOverlineItem   &GetOverline() const    { return *m_aOverline; }
    const SvxCrossedOutItem &GetCrossedOut() const  { return *m_aCrossedOut; }
    const SvxContourItem    &GetContour() const     { return *m_aContour; }
    const SvxShadowedItem   &GetShadowed() const    { return *m_aShadowed; }
    const SvxColorItem      &GetColor() const       { return *m_aColor; }
    const SvxBoxItem        &GetBox() const         { return *m_aBox; }
    const SvxLineItem&      GetTLBR() const         { return *m_aTLBR; }
    const SvxLineItem&      GetBLTR() const         { return *m_aBLTR; }
    const SvxBrushItem      &GetBackground() const  { return *m_aBackground; }
    const SvxAdjustItem     &GetAdjust() const      { return *m_aAdjust; }
    const SvxHorJustifyItem& GetHorJustify() const { return *m_aHorJustify; }
    const SvxVerJustifyItem& GetVerJustify() const { return *m_aVerJustify; }

    // The set-methods.
    void SetFont( const SvxFontItem& rNew );
    void SetHeight( const SvxFontHeightItem& rNew );
    void SetWeight( const SvxWeightItem& rNew );
    void SetPosture( const SvxPostureItem& rNew );
    void SetCJKFont( const SvxFontItem& rNew );
    void SetCJKHeight( const SvxFontHeightItem& rNew );
    void SetCJKWeight( const SvxWeightItem& rNew );
    void SetCJKPosture( const SvxPostureItem& rNew );
    void SetCTLFont( const SvxFontItem& rNew );
    void SetCTLHeight( const SvxFontHeightItem& rNew );
    void SetCTLWeight( const SvxWeightItem& rNew );
    void SetCTLPosture( const SvxPostureItem& rNew );
    void SetUnderline( const SvxUnderlineItem& rNew );
    void SetOverline( const SvxOverlineItem& rNew );
    void SetCrossedOut( const SvxCrossedOutItem& rNew );
    void SetContour( const SvxContourItem& rNew );
    void SetShadowed( const SvxShadowedItem& rNew );
    void SetColor( const SvxColorItem& rNew );
    void SetBox( const SvxBoxItem& rNew );
    void SetTLBR( const SvxLineItem& rNew );
    void SetBLTR( const SvxLineItem& rNew );
    void SetBackground( const SvxBrushItem& rNew );
    void SetAdjust( const SvxAdjustItem& rNew );
    void SetHorJustify( const SvxHorJustifyItem& rNew );
    void SetVerJustify( const SvxVerJustifyItem& rNew );


};

#endif // INCLUDED_SVX_AUTOFORMATHELPER_HXX

//////////////////////////////////////////////////////////////////////////////
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
