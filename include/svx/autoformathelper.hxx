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
/// Struct with version numbers of the Items

struct SVX_DLLPUBLIC AutoFormatVersions
{
public:
    // BlockA
    sal_uInt16 nFontVersion;
    sal_uInt16 nFontHeightVersion;
    sal_uInt16 nWeightVersion;
    sal_uInt16 nPostureVersion;
    sal_uInt16 nUnderlineVersion;
    sal_uInt16 nOverlineVersion;
    sal_uInt16 nCrossedOutVersion;
    sal_uInt16 nContourVersion;
    sal_uInt16 nShadowedVersion;
    sal_uInt16 nColorVersion;
    sal_uInt16 nBoxVersion;
    sal_uInt16 nLineVersion;
    sal_uInt16 nBrushVersion;
    sal_uInt16 nAdjustVersion;

    // BlockB
    sal_uInt16 nHorJustifyVersion;
    sal_uInt16 nVerJustifyVersion;
    sal_uInt16 nOrientationVersion;
    sal_uInt16 nMarginVersion;
    sal_uInt16 nBoolVersion;
    sal_uInt16 nInt32Version;
    sal_uInt16 nRotateModeVersion;
    sal_uInt16 nNumFormatVersion;

    AutoFormatVersions();

    void LoadBlockA( SvStream& rStream, sal_uInt16 nVer );
    void LoadBlockB( SvStream& rStream, sal_uInt16 nVer );

    static void WriteBlockA(SvStream& rStream, sal_uInt16 fileVersion);
    static void WriteBlockB(SvStream& rStream, sal_uInt16 fileVersion);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC AutoFormatBase
{
protected:
    // common attributes of Calc and Writer
    // --- from 641 on: CJK and CTL font settings
    std::shared_ptr<SvxFontItem>            m_aFont;
    std::shared_ptr<SvxFontHeightItem>      m_aHeight;
    std::shared_ptr<SvxWeightItem>          m_aWeight;
    std::shared_ptr<SvxPostureItem>         m_aPosture;

    std::shared_ptr<SvxFontItem>            m_aCJKFont;
    std::shared_ptr<SvxFontHeightItem>      m_aCJKHeight;
    std::shared_ptr<SvxWeightItem>          m_aCJKWeight;
    std::shared_ptr<SvxPostureItem>         m_aCJKPosture;

    std::shared_ptr<SvxFontItem>            m_aCTLFont;
    std::shared_ptr<SvxFontHeightItem>      m_aCTLHeight;
    std::shared_ptr<SvxWeightItem>          m_aCTLWeight;
    std::shared_ptr<SvxPostureItem>         m_aCTLPosture;

    std::shared_ptr<SvxUnderlineItem>       m_aUnderline;
    std::shared_ptr<SvxOverlineItem>        m_aOverline;
    std::shared_ptr<SvxCrossedOutItem>      m_aCrossedOut;
    std::shared_ptr<SvxContourItem>         m_aContour;
    std::shared_ptr<SvxShadowedItem>        m_aShadowed;
    std::shared_ptr<SvxColorItem>           m_aColor;
    std::shared_ptr<SvxBoxItem>             m_aBox;
    std::shared_ptr<SvxLineItem>            m_aTLBR;
    std::shared_ptr<SvxLineItem>            m_aBLTR;
    std::shared_ptr<SvxBrushItem>           m_aBackground;

    // Writer specific
    std::shared_ptr<SvxAdjustItem>          m_aAdjust;

    // Calc specific
    std::shared_ptr<SvxHorJustifyItem>      m_aHorJustify;
    std::shared_ptr<SvxVerJustifyItem>      m_aVerJustify;
    std::shared_ptr<SfxBoolItem>            m_aStacked;
    std::shared_ptr<SvxMarginItem>          m_aMargin;
    std::shared_ptr<SfxBoolItem>            m_aLinebreak;

    // from SO5, 504k on, rotated text
    std::shared_ptr<SfxInt32Item>           m_aRotateAngle;
    std::shared_ptr<SvxRotateModeItem>      m_aRotateMode;

public:
    AutoFormatBase();
    AutoFormatBase( const AutoFormatBase& rNew );
    ~AutoFormatBase();

    /// Comparing based of boxes backgrounds.
    bool operator==(const AutoFormatBase& rRight);

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
    const SfxBoolItem& GetStacked() const { return *m_aStacked; }
    const SvxMarginItem& GetMargin() const { return *m_aMargin; }
    const SfxBoolItem& GetLinebreak() const { return *m_aLinebreak; }
    const SfxInt32Item& GetRotateAngle() const { return *m_aRotateAngle; }
    const SvxRotateModeItem& GetRotateMode() const { return *m_aRotateMode; }

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
    void SetStacked( const SfxBoolItem& rNew );
    void SetMargin( const SvxMarginItem& rNew );
    void SetLinebreak( const SfxBoolItem& rNew );
    void SetRotateAngle( const SfxInt32Item& rNew );
    void SetRotateMode( const SvxRotateModeItem& rNew );

    bool LoadBlockA( SvStream& rStream, const AutoFormatVersions& rVersions, sal_uInt16 nVer );
    bool LoadBlockB( SvStream& rStream, const AutoFormatVersions& rVersions, sal_uInt16 nVer );

    bool SaveBlockA( SvStream& rStream, sal_uInt16 fileVersion ) const;
    bool SaveBlockB( SvStream& rStream, sal_uInt16 fileVersion ) const;
};

#endif // INCLUDED_SVX_AUTOFORMATHELPER_HXX

//////////////////////////////////////////////////////////////////////////////
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
