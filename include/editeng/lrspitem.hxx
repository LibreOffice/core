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
#include <com/sun/star/util/MeasureUnit.hpp>


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

/// helper struct used for resolving font-relative indentation
struct SvxFontUnitMetrics
{
    double m_dEmTwips = 0.0;
    double m_dIcTwips = 0.0;
    bool m_bInitialized = false;

    SvxFontUnitMetrics() = default;
    SvxFontUnitMetrics(double dEmTwips, double dIcTwips)
        : m_dEmTwips(dEmTwips)
        , m_dIcTwips(dIcTwips)
        , m_bInitialized(true)
    {
    }
};

/// helper struct for passing indentation along with units
struct SvxIndentValue
{
    double m_dValue;
    sal_Int16 m_nUnit;

    SvxIndentValue() = delete;
    SvxIndentValue(double dValue, sal_Int16 nUnit)
        : m_dValue(dValue)
        , m_nUnit(nUnit)
    {
    }

    static SvxIndentValue twips(double dValue) { return { dValue, css::util::MeasureUnit::TWIP }; }

    static SvxIndentValue zero() { return twips(0.0); }

    double ResolveDouble(const SvxFontUnitMetrics& rMetrics) const;
    sal_Int32 Resolve(const SvxFontUnitMetrics& rMetrics) const;
    sal_Int32 ResolveFixedPart() const;
    sal_Int32 ResolveVariablePart(const SvxFontUnitMetrics& rMetrics) const;

    void ScaleMetrics(tools::Long nMult, tools::Long nDiv);

    size_t hashCode() const;
    bool operator==(SvxIndentValue const&) const = default;
};

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

    DECLARE_ITEM_TYPE_FUNCTION(SvxLeftMarginItem)
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
    SvxIndentValue m_stTextLeftMargin = SvxIndentValue::zero();
    sal_uInt16 m_nPropLeftMargin = 100;

public:
    /// get left margin without negative first-line indent
    sal_Int32 ResolveLeft(const SvxFirstLineIndentItem& rFirstLine,
                          const SvxFontUnitMetrics& rMetrics) const;
    sal_Int32 ResolveLeftFixedPart(const SvxFirstLineIndentItem& rFirstLine) const;
    sal_Int32 ResolveLeftVariablePart(const SvxFirstLineIndentItem& rFirstLine,
                                      const SvxFontUnitMetrics& rMetrics) const;
    sal_uInt16 GetPropLeft() const;

    void SetTextLeft(SvxIndentValue stL, const sal_uInt16 nProp = 100);
    sal_Int32 ResolveTextLeft(const SvxFontUnitMetrics& rMetrics) const;
    SvxIndentValue GetTextLeft() const;

    DECLARE_ITEM_TYPE_FUNCTION(SvxTextLeftMarginItem)
    explicit SvxTextLeftMarginItem(const sal_uInt16 nId);
    SvxTextLeftMarginItem(SvxIndentValue stLeft, const sal_uInt16 nId);
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
    SvxIndentValue m_stFirstLineOffset = SvxIndentValue::zero();
    sal_uInt16 m_nPropFirstLineOffset = 100;
    /// Automatic calculation of the first line indent
    bool m_bAutoFirst = false;

public:
    bool IsAutoFirst() const;
    void SetAutoFirst(const bool bNew);

    void SetPropTextFirstLineOffset(sal_uInt16 nProp);
    sal_uInt16 GetPropTextFirstLineOffset() const;

    void SetTextFirstLineOffset(SvxIndentValue stValue, sal_uInt16 nProp = 100);
    SvxIndentValue GetTextFirstLineOffset() const;
    sal_Int32 ResolveTextFirstLineOffset(const SvxFontUnitMetrics& rMetrics) const;

    DECLARE_ITEM_TYPE_FUNCTION(SvxFirstLineIndentItem)
    explicit SvxFirstLineIndentItem(const sal_uInt16 nId);
    SvxFirstLineIndentItem(SvxIndentValue stValue, const sal_uInt16 nId);
    SvxFirstLineIndentItem(SvxFirstLineIndentItem const &) = default; // SfxPoolItem copy function dichotomy

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
    SvxIndentValue m_stRightMargin = SvxIndentValue::zero();
    sal_uInt16 m_nPropRightMargin = 100;

public:
    // The "layout interface":
    void SetRight(SvxIndentValue stR, const sal_uInt16 nProp = 100);

    // Query/direct setting of the absolute values
    SvxIndentValue GetRight() const;
    sal_Int32 ResolveRight(const SvxFontUnitMetrics& rMetrics) const;
    sal_Int32 ResolveRightFixedPart() const;
    sal_Int32 ResolveRightVariablePart(const SvxFontUnitMetrics& rMetrics) const;

    sal_uInt16 GetPropRight() const;

    DECLARE_ITEM_TYPE_FUNCTION(SvxRightMarginItem)
    explicit SvxRightMarginItem(const sal_uInt16 nId);
    SvxRightMarginItem(SvxIndentValue stRight, const sal_uInt16 nId);
    SvxRightMarginItem(SvxRightMarginItem const &) = default; // SfxPoolItem copy function dichotomy

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

    DECLARE_ITEM_TYPE_FUNCTION(SvxGutterLeftMarginItem)
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

    DECLARE_ITEM_TYPE_FUNCTION(SvxGutterRightMarginItem)
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
    SvxIndentValue m_stFirstLineOffset = SvxIndentValue::zero();
    SvxIndentValue m_stLeftMargin
        = SvxIndentValue::zero(); // nLeft or the negative first-line indent
    SvxIndentValue m_stRightMargin = SvxIndentValue::zero();
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

    DECLARE_ITEM_TYPE_FUNCTION(SvxLRSpaceItem)
    explicit SvxLRSpaceItem( const sal_uInt16 nId  );
    SvxLRSpaceItem(SvxIndentValue stLeft, SvxIndentValue stRight, SvxIndentValue stValue,
                   const sal_uInt16 nId);
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
    void SetLeft(SvxIndentValue stL, const sal_uInt16 nProp = 100);
    void SetRight(SvxIndentValue stR, const sal_uInt16 nProp = 100);

    // Query/direct setting of the absolute values
    SvxIndentValue GetLeft() const;
    sal_Int32 ResolveLeft(const SvxFontUnitMetrics& rMetrics) const;
    SvxIndentValue GetRight() const;
    sal_Int32 ResolveRight(const SvxFontUnitMetrics& rMetrics) const;
    bool IsAutoFirst()  const { return bAutoFirst; }
    void SetAutoFirst( const bool bNew ) { bAutoFirst = bNew; }

    bool IsExplicitZeroMarginValRight()  const { return bExplicitZeroMarginValRight; }
    bool IsExplicitZeroMarginValLeft()  const { return bExplicitZeroMarginValLeft; }
    void SetExplicitZeroMarginValRight( const bool eR ) { bExplicitZeroMarginValRight = eR; }
    void SetExplicitZeroMarginValLeft( const bool eL ) { bExplicitZeroMarginValLeft = eL; }
    sal_uInt16 GetPropLeft()  const { return nPropLeftMargin; }
    sal_uInt16 GetPropRight() const { return nPropRightMargin;}

    // The UI/text interface:
    void SetTextLeft(SvxIndentValue stL, const sal_uInt16 nProp = 100);
    SvxIndentValue GetTextLeft() const;
    sal_Int32 ResolveTextLeft(const SvxFontUnitMetrics& rMetrics) const;

    void SetTextFirstLineOffset(SvxIndentValue stValue, sal_uInt16 nProp = 100);
    SvxIndentValue GetTextFirstLineOffset() const;
    sal_Int32 ResolveTextFirstLineOffset(const SvxFontUnitMetrics& rMetrics) const;

    void SetPropTextFirstLineOffset( const sal_uInt16 nProp )
                    { nPropFirstLineOffset = nProp; }
    sal_uInt16 GetPropTextFirstLineOffset() const
                    { return nPropFirstLineOffset; }
    void SetGutterMargin(const tools::Long nGutterMargin) { m_nGutterMargin = nGutterMargin; }
    tools::Long GetGutterMargin() const { return m_nGutterMargin; }
    void SetRightGutterMargin(const tools::Long nRightGutterMargin) { m_nRightGutterMargin = nRightGutterMargin; }
    tools::Long GetRightGutterMargin() const { return m_nRightGutterMargin; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
