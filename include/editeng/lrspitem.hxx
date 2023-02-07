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
#ifndef INCLUDED_EDITENG_LRSPITEM_HXX
#define INCLUDED_EDITENG_LRSPITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>


// class SvxLRSpaceItem --------------------------------------------------

/*  [Description]

    Left/Right margin and first line indent

    SvxLRSpaceItem offers two interfaces to get the left margin and first line
    indent.
    - The Get* methods return the member in the way the layout used to expect:
      with a negative first line indent, the left margin shifts to the left.
    - The SetText*,GetText* methods assume that the left margin represents
      the 0 coordinate for the first line indent:

    UI         UI       LAYOUT   UI/TEXT      UI/TEXT    (Where?)
SetTextLeft SetTextFirst GetLeft GetTextLeft  GetTextFirst  (What?)
    500       -500        0        500         -500      (How much?)
    500         0        500       500           0
    500       +500       500       500         +500
    700       -500       200       700         -500
*/

#define LRSPACE_TXTLEFT_VERSION (sal_uInt16(0x0002))
#define LRSPACE_NEGATIVE_VERSION (sal_uInt16(0x0004))

class EDITENG_DLLPUBLIC SvxLRSpaceItem final : public SfxPoolItem
{
    /// First-line indent always relative to GetTextLeft()
    short   nFirstLineOffset;
    tools::Long    nLeftMargin;        // nLeft or the negative first-line indent
    tools::Long    nRightMargin;       // The unproblematic right edge
    /// The amount of extra space added to the left margin.
    tools::Long    m_nGutterMargin;
    /// The amount of extra space added to the right margin, on mirrored pages.
    tools::Long    m_nRightGutterMargin;

    sal_uInt16  nPropFirstLineOffset, nPropLeftMargin, nPropRightMargin;
    bool        bAutoFirst;    // Automatic calculation of the first line indent
    bool        bExplicitZeroMarginValRight;
    bool        bExplicitZeroMarginValLeft;

public:

    static SfxPoolItem* CreateDefault();

    explicit SvxLRSpaceItem( const sal_uInt16 nId  );
    SvxLRSpaceItem( const tools::Long nLeft, const tools::Long nRight,
                    const short nOfset /*= 0*/,
                    const sal_uInt16 nId  );
    SvxLRSpaceItem(SvxLRSpaceItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxLRSpaceItem*      Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual void                 ScaleMetrics( tools::Long nMult, tools::Long nDiv ) override;
    virtual bool                 HasMetrics() const override;

    // The "layout interface":
    void   SetLeft (const tools::Long nL, const sal_uInt16 nProp = 100);
    void   SetRight(const tools::Long nR, const sal_uInt16 nProp = 100);

    // Query/direct setting of the absolute values
    tools::Long GetLeft()  const { return nLeftMargin; }
    tools::Long GetRight() const { return nRightMargin;}
    void SetLeftValue( const tools::Long nL ) { assert(nFirstLineOffset == 0); nLeftMargin = nL; }
    void SetRightValue( const tools::Long nR ) { nRightMargin = nR; }
    bool IsAutoFirst()  const { return bAutoFirst; }
    void SetAutoFirst( const bool bNew ) { bAutoFirst = bNew; }

    bool IsExplicitZeroMarginValRight()  const { return bExplicitZeroMarginValRight; }
    bool IsExplicitZeroMarginValLeft()  const { return bExplicitZeroMarginValLeft; }
    void SetExplicitZeroMarginValRight( const bool eR ) { bExplicitZeroMarginValRight = eR; }
    void SetExplicitZeroMarginValLeft( const bool eL ) { bExplicitZeroMarginValLeft = eL; }
    sal_uInt16 GetPropLeft()  const { return nPropLeftMargin; }
    sal_uInt16 GetPropRight() const { return nPropRightMargin;}

    // The UI/text interface:
    void SetTextLeft(const tools::Long nL, const sal_uInt16 nProp = 100);
    tools::Long GetTextLeft() const;

    void SetTextFirstLineOffset(const short nF, const sal_uInt16 nProp = 100);
    short  GetTextFirstLineOffset() const { return nFirstLineOffset; }
    void SetPropTextFirstLineOffset( const sal_uInt16 nProp )
                    { nPropFirstLineOffset = nProp; }
    sal_uInt16 GetPropTextFirstLineOffset() const
                    { return nPropFirstLineOffset; }
    void SetTextFirstLineOffsetValue( const short nValue )
                    { nFirstLineOffset = nValue; }
    void SetGutterMargin(const tools::Long nGutterMargin) { m_nGutterMargin = nGutterMargin; }
    tools::Long GetGutterMargin() const { return m_nGutterMargin; }
    void SetRightGutterMargin(const tools::Long nRightGutterMargin) { m_nRightGutterMargin = nRightGutterMargin; }
    tools::Long GetRightGutterMargin() const { return m_nRightGutterMargin; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
