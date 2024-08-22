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

class SvxFirstLineIndentItem;

/// GetLeft() - for everything that's not applied to a paragraph
class EDITENG_DLLPUBLIC SvxLeftMarginItem final : public SfxPoolItem
{
private:
    /// left margin: nothing special
    tools::Long m_nLeftMargin = 0;
    sal_uInt16 m_nPropLeftMargin = 100;

public:
    // The "layout interface":
    void SetLeft(const tools::Long nL, const sal_uInt16 nProp = 100);

    // Query/direct setting of the absolute values
    tools::Long GetLeft() const { return m_nLeftMargin; }

    sal_uInt16 GetPropLeft() const { return m_nPropLeftMargin; }

    explicit SvxLeftMarginItem(const sal_uInt16 nId);
    SvxLeftMarginItem(const tools::Long nLeft, const sal_uInt16 nId);
    SvxLeftMarginItem(SvxLeftMarginItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxLeftMarginItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

/// GetTextLeft() - for everything that's applied to a paragraph
class EDITENG_DLLPUBLIC SvxTextLeftMarginItem final : public SfxPoolItem
{
private:
    friend class SvxFirstLineIndentItem;
    /// left margin including negative first-line indent
    tools::Long m_nTextLeftMargin = 0;
    sal_uInt16 m_nPropLeftMargin = 100;

public:
    //TODO: need this?
    //void SetLeft(SvxFirstLineIndentItem const& rFirstLine, const tools::Long nL, const sal_uInt16 nProp = 100);
    /// get left margin without negative first-line indent
    tools::Long GetLeft(SvxFirstLineIndentItem const& rFirstLine) const;
    sal_uInt16 GetPropLeft() const { return m_nPropLeftMargin; }

    void SetTextLeft(const tools::Long nL, const sal_uInt16 nProp = 100);
    tools::Long GetTextLeft() const;

    explicit SvxTextLeftMarginItem(const sal_uInt16 nId);
    SvxTextLeftMarginItem(const tools::Long nLeft, const sal_uInt16 nId);
    SvxTextLeftMarginItem(SvxTextLeftMarginItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxTextLeftMarginItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

/// first line indent that may be applied to paragraphs
class EDITENG_DLLPUBLIC SvxFirstLineIndentItem final : public SfxPoolItem
{
private:
    /// First-line indent always relative to GetTextLeft()
    short m_nFirstLineOffset = 0;
    sal_uInt16 m_nPropFirstLineOffset = 100;
    /// Automatic calculation of the first line indent
    bool m_bAutoFirst = false;

public:
    bool IsAutoFirst()  const { return m_bAutoFirst; }
    void SetAutoFirst(const bool bNew) { m_bAutoFirst = bNew; }

    void SetTextFirstLineOffset(const short nF, const sal_uInt16 nProp = 100);
    short GetTextFirstLineOffset() const { return m_nFirstLineOffset; }
    void SetPropTextFirstLineOffset(const sal_uInt16 nProp)
                    { m_nPropFirstLineOffset = nProp; }
    sal_uInt16 GetPropTextFirstLineOffset() const
                    { return m_nPropFirstLineOffset; }
    void SetTextFirstLineOffsetValue(const short nValue)
                    { m_nFirstLineOffset = nValue; }

    explicit SvxFirstLineIndentItem(const sal_uInt16 nId);
    SvxFirstLineIndentItem(const short nOffset, const sal_uInt16 nId);
    SvxFirstLineIndentItem(SvxFirstLineIndentItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxFirstLineIndentItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

class EDITENG_DLLPUBLIC SvxRightMarginItem final : public SfxPoolItem
{
private:
    /// right margin: nothing special
    tools::Long m_nRightMargin = 0;
    sal_uInt16 m_nPropRightMargin = 100;

public:
    // The "layout interface":
    void SetRight(const tools::Long nR, const sal_uInt16 nProp = 100);

    // Query/direct setting of the absolute values
    tools::Long GetRight() const { return m_nRightMargin;}

    sal_uInt16 GetPropRight() const { return m_nPropRightMargin; }

    explicit SvxRightMarginItem(const sal_uInt16 nId);
    SvxRightMarginItem(const tools::Long nRight, const sal_uInt16 nId);
    SvxRightMarginItem(SvxRightMarginItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxRightMarginItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

/// gutter margin - for page styles
class EDITENG_DLLPUBLIC SvxGutterLeftMarginItem final : public SfxPoolItem
{
private:
    /// The amount of extra space added to the left margin.
    tools::Long m_nGutterMargin = 0;

public:
    void SetGutterMargin(const tools::Long nGutterMargin) { m_nGutterMargin = nGutterMargin; }
    tools::Long GetGutterMargin() const { return m_nGutterMargin; }

    explicit SvxGutterLeftMarginItem(const sal_uInt16 nId);
    SvxGutterLeftMarginItem(SvxGutterLeftMarginItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxGutterLeftMarginItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

/// gutter margin - for page styles
class EDITENG_DLLPUBLIC SvxGutterRightMarginItem final : public SfxPoolItem
{
private:
    /// The amount of extra space added to the right margin, on mirrored pages.
    tools::Long m_nRightGutterMargin = 0;

public:
    tools::Long GetRightGutterMargin() const { return m_nRightGutterMargin; }

    explicit SvxGutterRightMarginItem(const sal_uInt16 nId);
    SvxGutterRightMarginItem(SvxGutterRightMarginItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual SvxGutterRightMarginItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual void ScaleMetrics(tools::Long nMult, tools::Long nDiv) override;
    virtual bool HasMetrics() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

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
