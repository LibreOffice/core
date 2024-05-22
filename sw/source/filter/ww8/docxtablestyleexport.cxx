/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "docxtablestyleexport.hxx"
#include "docxhelper.hxx"
#include <doc.hxx>
#include <docsh.hxx>
#include <oox/token/tokens.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sax/fastattribs.hxx>

#include <optional>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>

using namespace com::sun::star;
using namespace oox;

/// Methods in this class handle values in a table style.
struct DocxTableStyleExport::Impl
{
private:
    SwDoc& m_rDoc;
    sax_fastparser::FSHelperPtr m_pSerializer;

public:
    Impl(SwDoc& rDoc)
        : m_rDoc(rDoc)
    {
    }

    void TableStyle(const uno::Sequence<beans::PropertyValue>& rStyle);

    void setSerializer(sax_fastparser::FSHelperPtr pSerializer)
    {
        m_pSerializer = std::move(pSerializer);
    }

    const sax_fastparser::FSHelperPtr& getSerializer() const { return m_pSerializer; }

    SwDoc& getDoc() const { return m_rDoc; }

    /// Handles a boolean value.
    void handleBoolean(std::u16string_view aValue, sal_Int32 nToken);

    /// Export of w:pPr.
    void tableStylePPr(const uno::Sequence<beans::PropertyValue>& rPPr);
    /// Export of w:tblStylePr.
    void tableStyleTableStylePr(const uno::Sequence<beans::PropertyValue>& rTableStylePr);
    /// Export of w:rPr.
    void tableStyleRPr(const uno::Sequence<beans::PropertyValue>& rRPr);
    /// Export of w:rFonts.
    void tableStyleRRFonts(const uno::Sequence<beans::PropertyValue>& rRFonts);
    /// Export of w:lang.
    void tableStyleRLang(const uno::Sequence<beans::PropertyValue>& rLang);
    /// Export of w:ind in a pPr.
    void tableStylePInd(const uno::Sequence<beans::PropertyValue>& rInd);
    /// Export of w:spacing.
    void tableStylePSpacing(const uno::Sequence<beans::PropertyValue>& rSpacing);
    /// Export of w:tblPr.
    void tableStyleTablePr(const uno::Sequence<beans::PropertyValue>& rTablePr);
    /// Export of w:trPr.
    void tableStyleTrPr(const uno::Sequence<beans::PropertyValue>& rTrPr);
    /// Export of w:tcPr.
    void tableStyleTcPr(const uno::Sequence<beans::PropertyValue>& rTcPr);
    /// Export of w:tcBorders (and w:tblBorders).
    void tableStyleTcBorders(const uno::Sequence<beans::PropertyValue>& rTcBorders,
                             sal_Int32 nToken = XML_tcBorders);
    /// Export of w:tblInd.
    void tableStyleTableInd(const uno::Sequence<beans::PropertyValue>& rTableInd);
    /// Export of w:tblCellMar (and w:tcMar).
    void tableStyleTableCellMar(const uno::Sequence<beans::PropertyValue>& rTableCellMar,
                                sal_Int32 nType = XML_tblCellMar);
    /// Export of a given table cell border type.
    void tableStyleTcBorder(sal_Int32 nToken, const uno::Sequence<beans::PropertyValue>& rTcBorder);
    /// Export of w:shd.
    void tableStyleShd(const uno::Sequence<beans::PropertyValue>& rShd);
    /// Export of w:color.
    void tableStyleRColor(const uno::Sequence<beans::PropertyValue>& rColor);
};

void DocxTableStyleExport::CnfStyle(const uno::Sequence<beans::PropertyValue>& rAttributeList)
{
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();

    for (const auto& rAttribute : rAttributeList)
    {
        if (rAttribute.Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rAttribute.Value.get<OUString>());
        else
        {
            static DocxStringTokenMap const aTokens[]
                = { { "firstRow", XML_firstRow },
                    { "lastRow", XML_lastRow },
                    { "firstColumn", XML_firstColumn },
                    { "lastColumn", XML_lastColumn },
                    { "oddVBand", XML_oddVBand },
                    { "evenVBand", XML_evenVBand },
                    { "oddHBand", XML_oddHBand },
                    { "evenHBand", XML_evenHBand },
                    { "firstRowFirstColumn", XML_firstRowFirstColumn },
                    { "firstRowLastColumn", XML_firstRowLastColumn },
                    { "lastRowFirstColumn", XML_lastRowFirstColumn },
                    { "lastRowLastColumn", XML_lastRowLastColumn },
                    { nullptr, 0 } };

            if (sal_Int32 nToken = DocxStringGetToken(aTokens, rAttribute.Name))
                pAttributeList->add(FSNS(XML_w, nToken), rAttribute.Value.get<OUString>());
        }
    }

    m_pImpl->getSerializer()->singleElementNS(XML_w, XML_cnfStyle, pAttributeList);
}

void DocxTableStyleExport::TableStyles(sal_Int32 nCountStylesToWrite)
{
    // Do we have table styles from InteropGrabBag available?
    uno::Reference<beans::XPropertySet> xPropertySet(
        m_pImpl->getDoc().GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aInteropGrabBag;
    uno::Sequence<beans::PropertyValue> aTableStyles;
    auto pProp = std::find_if(
        std::cbegin(aInteropGrabBag), std::cend(aInteropGrabBag),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "tableStyles"; });
    if (pProp != std::cend(aInteropGrabBag))
        pProp->Value >>= aTableStyles;
    if (!aTableStyles.hasElements())
        return;

    if (nCountStylesToWrite > aTableStyles.getLength())
        nCountStylesToWrite = aTableStyles.getLength();

    for (sal_Int32 i = 0; i < nCountStylesToWrite; ++i)
    {
        uno::Sequence<beans::PropertyValue> aTableStyle;
        aTableStyles[i].Value >>= aTableStyle;
        m_pImpl->TableStyle(aTableStyle);
    }
}

void DocxTableStyleExport::Impl::tableStyleTableCellMar(
    const uno::Sequence<beans::PropertyValue>& rTableCellMar, sal_Int32 nType)
{
    static DocxStringTokenMap const aTableCellMarTokens[]
        = { { "left", XML_left }, { "right", XML_right }, { "start", XML_start },
            { "end", XML_end },   { "top", XML_top },     { "bottom", XML_bottom },
            { nullptr, 0 } };

    if (!rTableCellMar.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, nType);
    for (const auto& rProp : rTableCellMar)
    {
        if (sal_Int32 nToken = DocxStringGetToken(aTableCellMarTokens, rProp.Name))
        {
            comphelper::SequenceAsHashMap aMap(
                rProp.Value.get<uno::Sequence<beans::PropertyValue>>());
            m_pSerializer->singleElementNS(XML_w, nToken, FSNS(XML_w, XML_w),
                                           OString::number(aMap[u"w"_ustr].get<sal_Int32>()),
                                           FSNS(XML_w, XML_type),
                                           aMap[u"type"_ustr].get<OUString>());
        }
    }
    m_pSerializer->endElementNS(XML_w, nType);
}

void DocxTableStyleExport::Impl::tableStyleTcBorder(
    sal_Int32 nToken, const uno::Sequence<beans::PropertyValue>& rTcBorder)
{
    static DocxStringTokenMap const aTcBorderTokens[] = { { "val", XML_val },
                                                          { "sz", XML_sz },
                                                          { "color", XML_color },
                                                          { "space", XML_space },
                                                          { "themeColor", XML_themeColor },
                                                          { "themeTint", XML_themeTint },
                                                          { nullptr, 0 } };

    if (!rTcBorder.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rTcBorder)
        if (sal_Int32 nAttrToken = DocxStringGetToken(aTcBorderTokens, rProp.Name))
            pAttributeList->add(FSNS(XML_w, nAttrToken), rProp.Value.get<OUString>());

    m_pSerializer->singleElementNS(XML_w, nToken, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleTcBorders(
    const uno::Sequence<beans::PropertyValue>& rTcBorders, sal_Int32 nToken)
{
    static DocxStringTokenMap const aTcBordersTokens[] = { { "left", XML_left },
                                                           { "right", XML_right },
                                                           { "start", XML_start },
                                                           { "end", XML_end },
                                                           { "top", XML_top },
                                                           { "bottom", XML_bottom },
                                                           { "insideH", XML_insideH },
                                                           { "insideV", XML_insideV },
                                                           { "tl2br", XML_tl2br },
                                                           { "tr2bl", XML_tr2bl },
                                                           { nullptr, 0 } };

    if (!rTcBorders.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, nToken);
    for (const auto& rTcBorder : rTcBorders)
        if (sal_Int32 nSubToken = DocxStringGetToken(aTcBordersTokens, rTcBorder.Name))
            tableStyleTcBorder(nSubToken,
                               rTcBorder.Value.get<uno::Sequence<beans::PropertyValue>>());
    m_pSerializer->endElementNS(XML_w, nToken);
}

void DocxTableStyleExport::Impl::tableStyleShd(const uno::Sequence<beans::PropertyValue>& rShd)
{
    if (!rShd.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rShd)
    {
        if (rProp.Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rProp.Value.get<OUString>());
        else if (rProp.Name == "color")
            pAttributeList->add(FSNS(XML_w, XML_color), rProp.Value.get<OUString>());
        else if (rProp.Name == "fill")
            pAttributeList->add(FSNS(XML_w, XML_fill), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeFill")
            pAttributeList->add(FSNS(XML_w, XML_themeFill), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeFillShade")
            pAttributeList->add(FSNS(XML_w, XML_themeFillShade), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeFillTint")
            pAttributeList->add(FSNS(XML_w, XML_themeFillTint), rProp.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_shd, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRColor(const uno::Sequence<beans::PropertyValue>& rColor)
{
    if (!rColor.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rColor)
    {
        if (rProp.Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeColor")
            pAttributeList->add(FSNS(XML_w, XML_themeColor), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeTint")
            pAttributeList->add(FSNS(XML_w, XML_themeTint), rProp.Value.get<OUString>());
        else if (rProp.Name == "themeShade")
            pAttributeList->add(FSNS(XML_w, XML_themeShade), rProp.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_color, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRLang(const uno::Sequence<beans::PropertyValue>& rLang)
{
    if (!rLang.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rLang)
    {
        if (rProp.Name == "eastAsia")
            pAttributeList->add(FSNS(XML_w, XML_eastAsia), rProp.Value.get<OUString>());
        else if (rProp.Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rProp.Value.get<OUString>());
        else if (rProp.Name == "bidi")
            pAttributeList->add(FSNS(XML_w, XML_bidi), rProp.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_lang, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRRFonts(
    const uno::Sequence<beans::PropertyValue>& rRFonts)
{
    if (!rRFonts.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rRFont : rRFonts)
    {
        if (rRFont.Name == "eastAsiaTheme")
            pAttributeList->add(FSNS(XML_w, XML_eastAsiaTheme), rRFont.Value.get<OUString>());
        else if (rRFont.Name == "asciiTheme")
            pAttributeList->add(FSNS(XML_w, XML_asciiTheme), rRFont.Value.get<OUString>());
        else if (rRFont.Name == "cstheme")
            pAttributeList->add(FSNS(XML_w, XML_cstheme), rRFont.Value.get<OUString>());
        else if (rRFont.Name == "hAnsiTheme")
            pAttributeList->add(FSNS(XML_w, XML_hAnsiTheme), rRFont.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_rFonts, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStylePSpacing(
    const uno::Sequence<beans::PropertyValue>& rSpacing)
{
    if (!rSpacing.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rSpacing)
    {
        if (rProp.Name == "after")
            pAttributeList->add(FSNS(XML_w, XML_after), rProp.Value.get<OUString>());
        else if (rProp.Name == "before")
            pAttributeList->add(FSNS(XML_w, XML_before), rProp.Value.get<OUString>());
        else if (rProp.Name == "line")
            pAttributeList->add(FSNS(XML_w, XML_line), rProp.Value.get<OUString>());
        else if (rProp.Name == "lineRule")
            pAttributeList->add(FSNS(XML_w, XML_lineRule), rProp.Value.get<OUString>());
        else if (rProp.Name == "beforeLines")
            pAttributeList->add(FSNS(XML_w, XML_beforeLines), rProp.Value.get<OUString>());
        else if (rProp.Name == "ParaTopMarginBeforeAutoSpacing")
            // Auto spacing will be available in grab bag only if it was set to true
            pAttributeList->add(FSNS(XML_w, XML_beforeAutospacing), "1");
        else if (rProp.Name == "afterLines")
            pAttributeList->add(FSNS(XML_w, XML_afterLines), rProp.Value.get<OUString>());
        else if (rProp.Name == "ParaBottomMarginAfterAutoSpacing")
            // Auto spacing will be available in grab bag only if it was set to true
            pAttributeList->add(FSNS(XML_w, XML_afterAutospacing), "1");
    }
    m_pSerializer->singleElementNS(XML_w, XML_spacing, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStylePInd(const uno::Sequence<beans::PropertyValue>& rInd)
{
    if (!rInd.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rInd)
    {
        if (rProp.Name == "rightChars")
            pAttributeList->add(FSNS(XML_w, XML_rightChars), rProp.Value.get<OUString>());
        else if (rProp.Name == "right")
            pAttributeList->add(FSNS(XML_w, XML_right), rProp.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_ind, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleTableInd(
    const uno::Sequence<beans::PropertyValue>& rTableInd)
{
    if (!rTableInd.hasElements())
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (const auto& rProp : rTableInd)
    {
        if (rProp.Name == "w")
            pAttributeList->add(FSNS(XML_w, XML_w), OString::number(rProp.Value.get<sal_Int32>()));
        else if (rProp.Name == "type")
            pAttributeList->add(FSNS(XML_w, XML_type), rProp.Value.get<OUString>());
    }
    m_pSerializer->singleElementNS(XML_w, XML_tblInd, pAttributeList);
}

void DocxTableStyleExport::Impl::handleBoolean(std::u16string_view aValue, sal_Int32 nToken)
{
    if (aValue.empty())
        return;
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    if (aValue != u"1")
        pAttributeList->add(FSNS(XML_w, XML_val), aValue);
    m_pSerializer->singleElementNS(XML_w, nToken, pAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRPr(const uno::Sequence<beans::PropertyValue>& rRPr)
{
    if (!rRPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_rPr);

    uno::Sequence<beans::PropertyValue> aRFonts;
    uno::Sequence<beans::PropertyValue> aLang;
    uno::Sequence<beans::PropertyValue> aColor;
    uno::Sequence<beans::PropertyValue> aSpacingSequence;
    bool bSequenceFlag = false;
    OUString aB;
    OUString aBCs;
    OUString aI;
    OUString aSz;
    OUString aSzCs;
    OUString aCaps;
    OUString aSmallCaps;
    OUString aSpacing;
    for (const auto& rProp : rRPr)
    {
        if (rProp.Name == "rFonts")
            aRFonts = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "lang")
            aLang = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "b")
            aB = rProp.Value.get<OUString>();
        else if (rProp.Name == "bCs")
            aBCs = rProp.Value.get<OUString>();
        else if (rProp.Name == "i")
            aI = rProp.Value.get<OUString>();
        else if (rProp.Name == "color")
            aColor = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "sz")
            aSz = rProp.Value.get<OUString>();
        else if (rProp.Name == "szCs")
            aSzCs = rProp.Value.get<OUString>();
        else if (rProp.Name == "caps")
            aCaps = rProp.Value.get<OUString>();
        else if (rProp.Name == "smallCaps")
            aSmallCaps = rProp.Value.get<OUString>();
        else if (rProp.Name == "spacing")
        {
            if (rProp.Value.has<OUString>())
            {
                aSpacing = rProp.Value.get<OUString>();
            }
            else
            {
                aSpacingSequence = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
                bSequenceFlag = true; // set the uno::Sequence flag.
            }
        }
    }
    tableStyleRRFonts(aRFonts);
    tableStyleRLang(aLang);
    handleBoolean(aB, XML_b);
    handleBoolean(aBCs, XML_bCs);
    handleBoolean(aI, XML_i);
    handleBoolean(aCaps, XML_caps);
    handleBoolean(aSmallCaps, XML_smallCaps);
    tableStyleRColor(aColor);
    if (bSequenceFlag)
    {
        m_pSerializer->singleElementNS(XML_w, XML_spacing, FSNS(XML_w, XML_val),
                                       aSpacingSequence[0].Value.get<OUString>());
    }
    if (!aSpacing.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_spacing, FSNS(XML_w, XML_val), aSpacing);
    if (!aSz.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_sz, FSNS(XML_w, XML_val), aSz);
    if (!aSzCs.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_szCs, FSNS(XML_w, XML_val), aSzCs);

    m_pSerializer->endElementNS(XML_w, XML_rPr);
}

void DocxTableStyleExport::Impl::tableStylePPr(const uno::Sequence<beans::PropertyValue>& rPPr)
{
    if (!rPPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_pPr);

    uno::Sequence<beans::PropertyValue> aSpacing;
    uno::Sequence<beans::PropertyValue> aInd;
    bool bWordWrap = false;
    OUString aJc;
    OUString aSnapToGrid;
    for (const auto& rProp : rPPr)
    {
        if (rProp.Name == "spacing")
            aSpacing = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "ind")
            aInd = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "wordWrap")
            bWordWrap = true;
        else if (rProp.Name == "jc")
            aJc = rProp.Value.get<OUString>();
        else if (rProp.Name == "snapToGrid")
            aSnapToGrid = rProp.Value.get<OUString>();
    }
    if (bWordWrap)
        m_pSerializer->singleElementNS(XML_w, XML_wordWrap);
    tableStylePInd(aInd);
    handleBoolean(aSnapToGrid, XML_snapToGrid);
    tableStylePSpacing(aSpacing);
    if (!aJc.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_jc, FSNS(XML_w, XML_val), aJc);

    m_pSerializer->endElementNS(XML_w, XML_pPr);
}

void DocxTableStyleExport::Impl::tableStyleTablePr(
    const uno::Sequence<beans::PropertyValue>& rTablePr)
{
    if (!rTablePr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_tblPr);

    uno::Sequence<beans::PropertyValue> aTableInd;
    uno::Sequence<beans::PropertyValue> aTableBorders;
    uno::Sequence<beans::PropertyValue> aTableCellMar;
    std::optional<sal_Int32> oTableStyleRowBandSize;
    std::optional<sal_Int32> oTableStyleColBandSize;
    for (const auto& rProp : rTablePr)
    {
        if (rProp.Name == "tblStyleRowBandSize")
            oTableStyleRowBandSize = rProp.Value.get<sal_Int32>();
        else if (rProp.Name == "tblStyleColBandSize")
            oTableStyleColBandSize = rProp.Value.get<sal_Int32>();
        else if (rProp.Name == "tblInd")
            aTableInd = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tblBorders")
            aTableBorders = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tblCellMar")
            aTableCellMar = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }
    if (oTableStyleRowBandSize)
        m_pSerializer->singleElementNS(XML_w, XML_tblStyleRowBandSize, FSNS(XML_w, XML_val),
                                       OString::number(*oTableStyleRowBandSize));
    if (oTableStyleColBandSize)
        m_pSerializer->singleElementNS(XML_w, XML_tblStyleColBandSize, FSNS(XML_w, XML_val),
                                       OString::number(*oTableStyleColBandSize));
    tableStyleTableInd(aTableInd);
    tableStyleTcBorders(aTableBorders, XML_tblBorders);
    tableStyleTableCellMar(aTableCellMar);

    m_pSerializer->endElementNS(XML_w, XML_tblPr);
}

void DocxTableStyleExport::Impl::tableStyleTrPr(const uno::Sequence<beans::PropertyValue>& rTrPr)
{
    if (!rTrPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_trPr);

    for (const auto& rProp : rTrPr)
    {
        if (rProp.Name == "tblHeader")
            m_pSerializer->singleElementNS(XML_w, XML_tblHeader);
    }

    m_pSerializer->endElementNS(XML_w, XML_trPr);
}

void DocxTableStyleExport::Impl::tableStyleTcPr(const uno::Sequence<beans::PropertyValue>& rTcPr)
{
    if (!rTcPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_tcPr);

    uno::Sequence<beans::PropertyValue> aShd;
    uno::Sequence<beans::PropertyValue> aTcBorders;
    uno::Sequence<beans::PropertyValue> aTcMar;
    OUString aVAlign;
    for (const auto& rProp : rTcPr)
    {
        if (rProp.Name == "shd")
            aShd = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tcBorders")
            aTcBorders = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tcMar")
            aTcMar = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "vAlign")
            aVAlign = rProp.Value.get<OUString>();
    }
    tableStyleTcBorders(aTcBorders);
    tableStyleTableCellMar(aTcMar, XML_tcMar);
    tableStyleShd(aShd);
    if (!aVAlign.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), aVAlign);

    m_pSerializer->endElementNS(XML_w, XML_tcPr);
}

void DocxTableStyleExport::Impl::tableStyleTableStylePr(
    const uno::Sequence<beans::PropertyValue>& rTableStylePr)
{
    if (!rTableStylePr.hasElements())
        return;

    OUString aType;
    uno::Sequence<beans::PropertyValue> aPPr;
    uno::Sequence<beans::PropertyValue> aRPr;
    uno::Sequence<beans::PropertyValue> aTablePr;
    uno::Sequence<beans::PropertyValue> aTrPr;
    uno::Sequence<beans::PropertyValue> aTcPr;
    for (const auto& rProp : rTableStylePr)
    {
        if (rProp.Name == "type")
            aType = rProp.Value.get<OUString>();
        else if (rProp.Name == "pPr")
            aPPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "rPr")
            aRPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tblPr")
            aTablePr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "trPr")
            aTrPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tcPr")
            aTcPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    m_pSerializer->startElementNS(XML_w, XML_tblStylePr, FSNS(XML_w, XML_type), aType);

    tableStylePPr(aPPr);
    tableStyleRPr(aRPr);
    if (aTablePr.hasElements())
        tableStyleTablePr(aTablePr);
    else
    {
        // Even if we have an empty container, write it out, as Word does.
        m_pSerializer->singleElementNS(XML_w, XML_tblPr);
    }
    tableStyleTrPr(aTrPr);
    tableStyleTcPr(aTcPr);

    m_pSerializer->endElementNS(XML_w, XML_tblStylePr);
}

void DocxTableStyleExport::Impl::TableStyle(const uno::Sequence<beans::PropertyValue>& rStyle)
{
    bool bDefault = false;
    bool bCustomStyle = false;
    bool bQFormat = false;
    bool bSemiHidden = false;
    bool bUnhideWhenUsed = false;
    OUString aStyleId;
    OUString aName;
    OUString aBasedOn;
    OUString aRsid;
    OUString aUiPriority;
    uno::Sequence<beans::PropertyValue> aPPr;
    uno::Sequence<beans::PropertyValue> aRPr;
    uno::Sequence<beans::PropertyValue> aTablePr;
    uno::Sequence<beans::PropertyValue> aTcPr;
    std::vector<uno::Sequence<beans::PropertyValue>> aTableStylePrs;
    for (const auto& rProp : rStyle)
    {
        if (rProp.Name == "default")
            bDefault = rProp.Value.get<bool>();
        else if (rProp.Name == "customStyle")
            bCustomStyle = rProp.Value.get<bool>();
        else if (rProp.Name == "styleId")
            aStyleId = rProp.Value.get<OUString>();
        else if (rProp.Name == "name")
            aName = rProp.Value.get<OUString>();
        else if (rProp.Name == "basedOn")
            aBasedOn = rProp.Value.get<OUString>();
        else if (rProp.Name == "uiPriority")
            aUiPriority = rProp.Value.get<OUString>();
        else if (rProp.Name == "qFormat")
            bQFormat = true;
        else if (rProp.Name == "semiHidden")
            bSemiHidden = true;
        else if (rProp.Name == "unhideWhenUsed")
            bUnhideWhenUsed = true;
        else if (rProp.Name == "rsid")
            aRsid = rProp.Value.get<OUString>();
        else if (rProp.Name == "pPr")
            aPPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "rPr")
            aRPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tblPr")
            aTablePr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tcPr")
            aTcPr = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rProp.Name == "tblStylePr")
            aTableStylePrs.push_back(rProp.Value.get<uno::Sequence<beans::PropertyValue>>());
    }

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    pAttributeList->add(FSNS(XML_w, XML_type), "table");
    if (bDefault)
        pAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    if (!aStyleId.isEmpty())
        pAttributeList->add(FSNS(XML_w, XML_styleId), aStyleId);
    m_pSerializer->startElementNS(XML_w, XML_style, pAttributeList);

    m_pSerializer->singleElementNS(XML_w, XML_name, FSNS(XML_w, XML_val), aName);
    if (!aBasedOn.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_basedOn, FSNS(XML_w, XML_val), aBasedOn);
    if (!aUiPriority.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority, FSNS(XML_w, XML_val), aUiPriority);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed);
    if (bQFormat)
        m_pSerializer->singleElementNS(XML_w, XML_qFormat);
    if (!aRsid.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_rsid, FSNS(XML_w, XML_val), aRsid);

    tableStylePPr(aPPr);
    tableStyleRPr(aRPr);
    tableStyleTablePr(aTablePr);
    tableStyleTcPr(aTcPr);
    for (const uno::Sequence<beans::PropertyValue>& i : aTableStylePrs)
        tableStyleTableStylePr(i);

    m_pSerializer->endElementNS(XML_w, XML_style);
}

void DocxTableStyleExport::SetSerializer(const sax_fastparser::FSHelperPtr& pSerializer)
{
    m_pImpl->setSerializer(pSerializer);
}

DocxTableStyleExport::DocxTableStyleExport(SwDoc& rDoc,
                                           const sax_fastparser::FSHelperPtr& pSerializer)
    : m_pImpl(std::make_unique<Impl>(rDoc))
{
    m_pImpl->setSerializer(pSerializer);
}

DocxTableStyleExport::~DocxTableStyleExport() = default;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
