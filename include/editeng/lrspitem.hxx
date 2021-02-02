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

    SvxLRSpaceItem offers two interfaces for views from the left margin and
    first line indent. The get methods return the member, with the layout also
    as expected: the left edge shifts to the negative first line indent to the
    left. The SetTxt/Gettxt methods assume that the left side represents the
    0 coordinate for the first line indent:

    UI         UI       LAYOUT   UI/TEXT      UI/TEXT    (Where?)
    SetTextLeft SetTxtFirst GetLeft  GetTextLeft  GetTxtFirst  (What?)
    500       -500        0        500         -500      (How much?)
    500         0        500       500           0
    500       +500       500       500         +500
    700       -500       200       700         -500
*/

#define LRSPACE_TXTLEFT_VERSION (sal_uInt16(0x0002))
#define LRSPACE_NEGATIVE_VERSION (sal_uInt16(0x0004))

class EDITENG_DLLPUBLIC SvxLRSpaceItem final : public SfxPoolItem
{
    tools::Long    nTxtLeft;           // We spend a sal_uInt16
    tools::Long    nLeftMargin;        // nLeft or the negative first-line indent
    tools::Long    nRightMargin;       // The unproblematic right edge
    /// The amount of extra space added to the left margin.
    tools::Long    m_nGutterMargin;

    sal_uInt16  nPropFirstLineOffset, nPropLeftMargin, nPropRightMargin;
    short   nFirstLineOffset;     // First-line indent _always_ relative to nTxtLeft
    bool        bAutoFirst;    // Automatic calculation of the first line indent
    bool        bExplicitZeroMarginValRight;
    bool        bExplicitZeroMarginValLeft;

    void   AdjustLeft();        // nLeftMargin and nTxtLeft are being adjusted.

public:

    static SfxPoolItem* CreateDefault();

    explicit SvxLRSpaceItem( const sal_uInt16 nId  );
    SvxLRSpaceItem( const tools::Long nLeft, const tools::Long nRight,
                    const tools::Long nTLeft /*= 0*/, const short nOfset /*= 0*/,
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
    inline void   SetLeft ( const tools::Long nL, const sal_uInt16 nProp = 100 );
    inline void   SetRight( const tools::Long nR, const sal_uInt16 nProp = 100 );

    // Query/direct setting of the absolute values
    tools::Long GetLeft()  const { return nLeftMargin; }
    tools::Long GetRight() const { return nRightMargin;}
    void SetLeftValue( const tools::Long nL ) { nTxtLeft = nLeftMargin = nL; }
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
    inline void SetTextLeft( const tools::Long nL, const sal_uInt16 nProp = 100 );
    tools::Long GetTextLeft() const { return nTxtLeft; }

    inline void   SetTextFirstLineOffset( const short nF, const sal_uInt16 nProp = 100 );
    short  GetTextFirstLineOffset() const { return nFirstLineOffset; }
    void SetPropTextFirstLineOffset( const sal_uInt16 nProp )
                    { nPropFirstLineOffset = nProp; }
    sal_uInt16 GetPropTextFirstLineOffset() const
                    { return nPropFirstLineOffset; }
    void SetTextFirstLineOffsetValue( const short nValue )
                    { nFirstLineOffset = nValue; }
    void SetGutterMargin(const tools::Long nGutterMargin) { m_nGutterMargin = nGutterMargin; }
    tools::Long GetGutterMargin() const { return m_nGutterMargin; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

inline void SvxLRSpaceItem::SetLeft( const tools::Long nL, const sal_uInt16 nProp )
{
    nLeftMargin = (nL * nProp) / 100;
    nTxtLeft = nLeftMargin;
    nPropLeftMargin = nProp;
}
inline void SvxLRSpaceItem::SetRight( const tools::Long nR, const sal_uInt16 nProp )
{
    if (0 == nR)
    {
        SetExplicitZeroMarginValRight(true);
    }
    nRightMargin = (nR * nProp) / 100;
    nPropRightMargin = nProp;
}
inline void SvxLRSpaceItem::SetTextFirstLineOffset( const short nF,
                                                 const sal_uInt16 nProp )
{
    nFirstLineOffset = short((tools::Long(nF) * nProp ) / 100);
    nPropFirstLineOffset = nProp;
    AdjustLeft();
}

inline void SvxLRSpaceItem::SetTextLeft( const tools::Long nL, const sal_uInt16 nProp )
{
    if (0 == nL)
    {
        SetExplicitZeroMarginValLeft(true);
    }
    nTxtLeft = (nL * nProp) / 100;
    nPropLeftMargin = nProp;
    AdjustLeft();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
