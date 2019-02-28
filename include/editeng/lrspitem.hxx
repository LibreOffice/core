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

class EDITENG_DLLPUBLIC SvxLRSpaceItem : public SfxPoolItem
{
    long    nTxtLeft;           // We spend a sal_uInt16
    long    nLeftMargin;        // nLeft or the negative first-line indent
    long    nRightMargin;       // The unproblematic right edge

    sal_uInt16  nPropFirstLineOfst, nPropLeftMargin, nPropRightMargin;
    short   nFirstLineOfst;     // First-line indent _always_ relative to nTxtLeft
    bool        bAutoFirst;    // Automatic calculation of the first line indent
    bool        bExplicitZeroMarginValRight;
    bool        bExplicitZeroMarginValLeft;

    void   AdjustLeft();        // nLeftMargin and nTxtLeft are being adjusted.

public:

    static SfxPoolItem* CreateDefault();

    explicit SvxLRSpaceItem( const sal_uInt16 nId  );
    SvxLRSpaceItem( const long nLeft, const long nRight,
                    const long nTLeft /*= 0*/, const short nOfset /*= 0*/,
                    const sal_uInt16 nId  );
    inline SvxLRSpaceItem& operator=( const SvxLRSpaceItem &rCpy );
    SvxLRSpaceItem(SvxLRSpaceItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual void                 ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool                 HasMetrics() const override;

    // The "layout interface":
    inline void   SetLeft ( const long nL, const sal_uInt16 nProp = 100 );
    inline void   SetRight( const long nR, const sal_uInt16 nProp = 100 );

    // Query/direct setting of the absolute values
    long GetLeft()  const { return nLeftMargin; }
    long GetRight() const { return nRightMargin;}
    void SetLeftValue( const long nL ) { nTxtLeft = nLeftMargin = nL; }
    void SetRightValue( const long nR ) { nRightMargin = nR; }
    bool IsAutoFirst()  const { return bAutoFirst; }
    void SetAutoFirst( const bool bNew ) { bAutoFirst = bNew; }

    bool IsExplicitZeroMarginValRight()  const { return bExplicitZeroMarginValRight; }
    bool IsExplicitZeroMarginValLeft()  const { return bExplicitZeroMarginValLeft; }
    void SetExplicitZeroMarginValRight( const bool eR ) { bExplicitZeroMarginValRight = eR; }
    void SetExplicitZeroMarginValLeft( const bool eL ) { bExplicitZeroMarginValLeft = eL; }
    sal_uInt16 GetPropLeft()  const { return nPropLeftMargin; }
    sal_uInt16 GetPropRight() const { return nPropRightMargin;}

    // The UI/text interface:
    inline void SetTextLeft( const long nL, const sal_uInt16 nProp = 100 );
    long GetTextLeft() const { return nTxtLeft; }

    inline void   SetTextFirstLineOfst( const short nF, const sal_uInt16 nProp = 100 );
    short  GetTextFirstLineOfst() const { return nFirstLineOfst; }
    void SetPropTextFirstLineOfst( const sal_uInt16 nProp )
                    { nPropFirstLineOfst = nProp; }
    sal_uInt16 GetPropTextFirstLineOfst() const
                    { return nPropFirstLineOfst; }
    void SetTextFirstLineOfstValue( const short nValue )
                    { nFirstLineOfst = nValue; }
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline SvxLRSpaceItem &SvxLRSpaceItem::operator=( const SvxLRSpaceItem &rCpy )
{
    nFirstLineOfst = rCpy.nFirstLineOfst;
    nTxtLeft = rCpy.nTxtLeft;
    nLeftMargin = rCpy.nLeftMargin;
    nRightMargin = rCpy.nRightMargin;
    nPropFirstLineOfst = rCpy.nPropFirstLineOfst;
    nPropLeftMargin = rCpy.nPropLeftMargin;
    nPropRightMargin = rCpy.nPropRightMargin;
    bAutoFirst = rCpy.bAutoFirst;
    bExplicitZeroMarginValRight = rCpy.bExplicitZeroMarginValRight;
    bExplicitZeroMarginValLeft = rCpy.bExplicitZeroMarginValLeft;
    return *this;
}

inline void SvxLRSpaceItem::SetLeft( const long nL, const sal_uInt16 nProp )
{
    nLeftMargin = (nL * nProp) / 100;
    nTxtLeft = nLeftMargin;
    nPropLeftMargin = nProp;
}
inline void SvxLRSpaceItem::SetRight( const long nR, const sal_uInt16 nProp )
{
    if (0 == nR)
    {
        SetExplicitZeroMarginValRight(true);
    }
    nRightMargin = (nR * nProp) / 100;
    nPropRightMargin = nProp;
}
inline void SvxLRSpaceItem::SetTextFirstLineOfst( const short nF,
                                                 const sal_uInt16 nProp )
{
    nFirstLineOfst = short((long(nF) * nProp ) / 100);
    nPropFirstLineOfst = nProp;
    AdjustLeft();
}

inline void SvxLRSpaceItem::SetTextLeft( const long nL, const sal_uInt16 nProp )
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
