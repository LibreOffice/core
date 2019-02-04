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

#include <boost/optional.hpp>

using namespace com::sun::star;
using namespace oox;

/// Methods in this class handle values in a table style.
struct DocxTableStyleExport::Impl
{
private:
    SwDoc* m_pDoc;
    sax_fastparser::FSHelperPtr m_pSerializer;

public:
    Impl(SwDoc* pDoc)
        : m_pDoc(pDoc)
    {
    }

    void TableStyle(uno::Sequence<beans::PropertyValue>& rStyle);

    void setSerializer(sax_fastparser::FSHelperPtr pSerializer)
    {
        m_pSerializer = std::move(pSerializer);
    }

    sax_fastparser::FSHelperPtr getSerializer() const { return m_pSerializer; }

    SwDoc* getDoc() const { return m_pDoc; }

    /// Handles a boolean value.
    void handleBoolean(const OUString& aValue, sal_Int32 nToken);

    /// Export of w:pPr.
    void tableStylePPr(uno::Sequence<beans::PropertyValue>& rPPr);
    /// Export of w:tblStylePr.
    void tableStyleTableStylePr(uno::Sequence<beans::PropertyValue>& rTableStylePr);
    /// Export of w:rPr.
    void tableStyleRPr(uno::Sequence<beans::PropertyValue>& rRPr);
    /// Export of w:rFonts.
    void tableStyleRRFonts(uno::Sequence<beans::PropertyValue>& rRFonts);
    /// Export of w:lang.
    void tableStyleRLang(uno::Sequence<beans::PropertyValue>& rLang);
    /// Export of w:ind in a pPr.
    void tableStylePInd(uno::Sequence<beans::PropertyValue>& rInd);
    /// Export of w:spacing.
    void tableStylePSpacing(uno::Sequence<beans::PropertyValue>& rSpacing);
    /// Export of w:tblPr.
    void tableStyleTablePr(uno::Sequence<beans::PropertyValue>& rTablePr);
    /// Export of w:tcPr.
    void tableStyleTcPr(uno::Sequence<beans::PropertyValue>& rTcPr);
    /// Export of w:tcBorders (and w:tblBorders).
    void tableStyleTcBorders(uno::Sequence<beans::PropertyValue>& rTcBorders,
                             sal_Int32 nToken = XML_tcBorders);
    /// Export of w:tblInd.
    void tableStyleTableInd(uno::Sequence<beans::PropertyValue>& rTableInd);
    /// Export of w:tblCellMar (and w:tcMar).
    void tableStyleTableCellMar(uno::Sequence<beans::PropertyValue>& rTableCellMar,
                                sal_Int32 nType = XML_tblCellMar);
    /// Export of a given table cell border type.
    void tableStyleTcBorder(sal_Int32 nToken, const uno::Sequence<beans::PropertyValue>& rTcBorder);
    /// Export of w:shd.
    void tableStyleShd(uno::Sequence<beans::PropertyValue>& rShd);
    /// Export of w:color.
    void tableStyleRColor(uno::Sequence<beans::PropertyValue>& rColor);
};

void DocxTableStyleExport::CharFormat(css::uno::Sequence<css::beans::PropertyValue>& rRPr)
{
    m_pImpl->tableStyleRPr(rRPr);
}

void DocxTableStyleExport::CnfStyle(uno::Sequence<beans::PropertyValue>& rAttributeList)
{
    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();

    for (sal_Int32 j = 0; j < rAttributeList.getLength(); ++j)
    {
        if (rAttributeList[j].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val),
                                rAttributeList[j].Value.get<OUString>().toUtf8());
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

            if (sal_Int32 nToken = DocxStringGetToken(aTokens, rAttributeList[j].Name))
                pAttributeList->add(FSNS(XML_w, nToken),
                                    rAttributeList[j].Value.get<OUString>().toUtf8());
        }
    }

    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pImpl->getSerializer()->singleElementNS(XML_w, XML_cnfStyle, xAttributeList);
}

void DocxTableStyleExport::TableStyles(sal_Int32 nCountStylesToWrite)
{
    // Do we have table styles from InteropGrabBag available?
    uno::Reference<beans::XPropertySet> xPropertySet(
        m_pImpl->getDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xPropertySet->getPropertyValue("InteropGrabBag") >>= aInteropGrabBag;
    uno::Sequence<beans::PropertyValue> aTableStyles;
    for (sal_Int32 i = 0; i < aInteropGrabBag.getLength(); ++i)
    {
        if (aInteropGrabBag[i].Name == "tableStyles")
        {
            aInteropGrabBag[i].Value >>= aTableStyles;
            break;
        }
    }
    if (!aTableStyles.getLength())
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
    uno::Sequence<beans::PropertyValue>& rTableCellMar, sal_Int32 nType)
{
    static DocxStringTokenMap const aTableCellMarTokens[]
        = { { "left", XML_left }, { "right", XML_right }, { "start", XML_start },
            { "end", XML_end },   { "top", XML_top },     { "bottom", XML_bottom },
            { nullptr, 0 } };

    if (!rTableCellMar.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, nType, FSEND);
    for (sal_Int32 i = 0; i < rTableCellMar.getLength(); ++i)
    {
        if (sal_Int32 nToken = DocxStringGetToken(aTableCellMarTokens, rTableCellMar[i].Name))
        {
            comphelper::SequenceAsHashMap aMap(
                rTableCellMar[i].Value.get<uno::Sequence<beans::PropertyValue>>());
            m_pSerializer->singleElementNS(
                XML_w, nToken, FSNS(XML_w, XML_w), OString::number(aMap["w"].get<sal_Int32>()),
                FSNS(XML_w, XML_type), aMap["type"].get<OUString>().toUtf8(), FSEND);
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

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rTcBorder.getLength(); ++i)
        if (sal_Int32 nAttrToken = DocxStringGetToken(aTcBorderTokens, rTcBorder[i].Name))
            pAttributeList->add(FSNS(XML_w, nAttrToken),
                                rTcBorder[i].Value.get<OUString>().toUtf8());

    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, nToken, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleTcBorders(
    uno::Sequence<beans::PropertyValue>& rTcBorders, sal_Int32 nToken)
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

    m_pSerializer->startElementNS(XML_w, nToken, FSEND);
    for (sal_Int32 i = 0; i < rTcBorders.getLength(); ++i)
        if (sal_Int32 nSubToken = DocxStringGetToken(aTcBordersTokens, rTcBorders[i].Name))
            tableStyleTcBorder(nSubToken,
                               rTcBorders[i].Value.get<uno::Sequence<beans::PropertyValue>>());
    m_pSerializer->endElementNS(XML_w, nToken);
}

void DocxTableStyleExport::Impl::tableStyleShd(uno::Sequence<beans::PropertyValue>& rShd)
{
    if (!rShd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rShd.getLength(); ++i)
    {
        if (rShd[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rShd[i].Value.get<OUString>().toUtf8());
        else if (rShd[i].Name == "color")
            pAttributeList->add(FSNS(XML_w, XML_color), rShd[i].Value.get<OUString>().toUtf8());
        else if (rShd[i].Name == "fill")
            pAttributeList->add(FSNS(XML_w, XML_fill), rShd[i].Value.get<OUString>().toUtf8());
        else if (rShd[i].Name == "themeFill")
            pAttributeList->add(FSNS(XML_w, XML_themeFill), rShd[i].Value.get<OUString>().toUtf8());
        else if (rShd[i].Name == "themeFillShade")
            pAttributeList->add(FSNS(XML_w, XML_themeFillShade),
                                rShd[i].Value.get<OUString>().toUtf8());
        else if (rShd[i].Name == "themeFillTint")
            pAttributeList->add(FSNS(XML_w, XML_themeFillTint),
                                rShd[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_shd, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRColor(uno::Sequence<beans::PropertyValue>& rColor)
{
    if (!rColor.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rColor.getLength(); ++i)
    {
        if (rColor[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rColor[i].Value.get<OUString>().toUtf8());
        else if (rColor[i].Name == "themeColor")
            pAttributeList->add(FSNS(XML_w, XML_themeColor),
                                rColor[i].Value.get<OUString>().toUtf8());
        else if (rColor[i].Name == "themeTint")
            pAttributeList->add(FSNS(XML_w, XML_themeTint),
                                rColor[i].Value.get<OUString>().toUtf8());
        else if (rColor[i].Name == "themeShade")
            pAttributeList->add(FSNS(XML_w, XML_themeShade),
                                rColor[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_color, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRLang(uno::Sequence<beans::PropertyValue>& rLang)
{
    if (!rLang.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rLang.getLength(); ++i)
    {
        if (rLang[i].Name == "eastAsia")
            pAttributeList->add(FSNS(XML_w, XML_eastAsia), rLang[i].Value.get<OUString>().toUtf8());
        else if (rLang[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), rLang[i].Value.get<OUString>().toUtf8());
        else if (rLang[i].Name == "bidi")
            pAttributeList->add(FSNS(XML_w, XML_bidi), rLang[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_lang, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRRFonts(uno::Sequence<beans::PropertyValue>& rRFonts)
{
    if (!rRFonts.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rRFonts.getLength(); ++i)
    {
        if (rRFonts[i].Name == "eastAsiaTheme")
            pAttributeList->add(FSNS(XML_w, XML_eastAsiaTheme),
                                rRFonts[i].Value.get<OUString>().toUtf8());
        else if (rRFonts[i].Name == "asciiTheme")
            pAttributeList->add(FSNS(XML_w, XML_asciiTheme),
                                rRFonts[i].Value.get<OUString>().toUtf8());
        else if (rRFonts[i].Name == "cstheme")
            pAttributeList->add(FSNS(XML_w, XML_cstheme),
                                rRFonts[i].Value.get<OUString>().toUtf8());
        else if (rRFonts[i].Name == "hAnsiTheme")
            pAttributeList->add(FSNS(XML_w, XML_hAnsiTheme),
                                rRFonts[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_rFonts, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStylePSpacing(uno::Sequence<beans::PropertyValue>& rSpacing)
{
    if (!rSpacing.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rSpacing.getLength(); ++i)
    {
        if (rSpacing[i].Name == "after")
            pAttributeList->add(FSNS(XML_w, XML_after), rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "before")
            pAttributeList->add(FSNS(XML_w, XML_before),
                                rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "line")
            pAttributeList->add(FSNS(XML_w, XML_line), rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "lineRule")
            pAttributeList->add(FSNS(XML_w, XML_lineRule),
                                rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "beforeLines")
            pAttributeList->add(FSNS(XML_w, XML_beforeLines),
                                rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "ParaTopMarginBeforeAutoSpacing")
            // Auto spacing will be available in grab bag only if it was set to true
            pAttributeList->add(FSNS(XML_w, XML_beforeAutospacing), "1");
        else if (rSpacing[i].Name == "afterLines")
            pAttributeList->add(FSNS(XML_w, XML_afterLines),
                                rSpacing[i].Value.get<OUString>().toUtf8());
        else if (rSpacing[i].Name == "ParaBottomMarginAfterAutoSpacing")
            // Auto spacing will be available in grab bag only if it was set to true
            pAttributeList->add(FSNS(XML_w, XML_afterAutospacing), "1");
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_spacing, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStylePInd(uno::Sequence<beans::PropertyValue>& rInd)
{
    if (!rInd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rInd.getLength(); ++i)
    {
        if (rInd[i].Name == "rightChars")
            pAttributeList->add(FSNS(XML_w, XML_rightChars),
                                rInd[i].Value.get<OUString>().toUtf8());
        else if (rInd[i].Name == "right")
            pAttributeList->add(FSNS(XML_w, XML_right), rInd[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_ind, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleTableInd(uno::Sequence<beans::PropertyValue>& rTableInd)
{
    if (!rTableInd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    for (sal_Int32 i = 0; i < rTableInd.getLength(); ++i)
    {
        if (rTableInd[i].Name == "w")
            pAttributeList->add(FSNS(XML_w, XML_w),
                                OString::number(rTableInd[i].Value.get<sal_Int32>()));
        else if (rTableInd[i].Name == "type")
            pAttributeList->add(FSNS(XML_w, XML_type), rTableInd[i].Value.get<OUString>().toUtf8());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, XML_tblInd, xAttributeList);
}

void DocxTableStyleExport::Impl::handleBoolean(const OUString& aValue, sal_Int32 nToken)
{
    if (aValue.isEmpty())
        return;
    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    if (aValue != "1")
        pAttributeList->add(FSNS(XML_w, XML_val), aValue.toUtf8());
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->singleElementNS(XML_w, nToken, xAttributeList);
}

void DocxTableStyleExport::Impl::tableStyleRPr(uno::Sequence<beans::PropertyValue>& rRPr)
{
    if (!rRPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_rPr, FSEND);

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
    for (sal_Int32 i = 0; i < rRPr.getLength(); ++i)
    {
        if (rRPr[i].Name == "rFonts")
            aRFonts = rRPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rRPr[i].Name == "lang")
            aLang = rRPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rRPr[i].Name == "b")
            aB = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "bCs")
            aBCs = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "i")
            aI = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "color")
            aColor = rRPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rRPr[i].Name == "sz")
            aSz = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "szCs")
            aSzCs = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "caps")
            aCaps = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "smallCaps")
            aSmallCaps = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "spacing")
        {
            if (rRPr[i].Value.has<OUString>())
            {
                aSpacing = rRPr[i].Value.get<OUString>();
            }
            else
            {
                aSpacingSequence = rRPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
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
                                       aSpacingSequence[0].Value.get<OUString>().toUtf8(), FSEND);
    }
    if (!aSpacing.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_spacing, FSNS(XML_w, XML_val), aSpacing.toUtf8(),
                                       FSEND);
    if (!aSz.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_sz, FSNS(XML_w, XML_val), aSz.toUtf8(), FSEND);
    if (!aSzCs.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_szCs, FSNS(XML_w, XML_val), aSzCs.toUtf8(),
                                       FSEND);

    m_pSerializer->endElementNS(XML_w, XML_rPr);
}

void DocxTableStyleExport::Impl::tableStylePPr(uno::Sequence<beans::PropertyValue>& rPPr)
{
    if (!rPPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_pPr, FSEND);

    uno::Sequence<beans::PropertyValue> aSpacing;
    uno::Sequence<beans::PropertyValue> aInd;
    bool bWordWrap = false;
    OUString aJc;
    OUString aSnapToGrid;
    for (sal_Int32 i = 0; i < rPPr.getLength(); ++i)
    {
        if (rPPr[i].Name == "spacing")
            aSpacing = rPPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rPPr[i].Name == "ind")
            aInd = rPPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rPPr[i].Name == "wordWrap")
            bWordWrap = true;
        else if (rPPr[i].Name == "jc")
            aJc = rPPr[i].Value.get<OUString>();
        else if (rPPr[i].Name == "snapToGrid")
            aSnapToGrid = rPPr[i].Value.get<OUString>();
    }
    if (bWordWrap)
        m_pSerializer->singleElementNS(XML_w, XML_wordWrap, FSEND);
    tableStylePInd(aInd);
    handleBoolean(aSnapToGrid, XML_snapToGrid);
    tableStylePSpacing(aSpacing);
    if (!aJc.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_jc, FSNS(XML_w, XML_val), aJc.toUtf8(), FSEND);

    m_pSerializer->endElementNS(XML_w, XML_pPr);
}

void DocxTableStyleExport::Impl::tableStyleTablePr(uno::Sequence<beans::PropertyValue>& rTablePr)
{
    if (!rTablePr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_tblPr, FSEND);

    uno::Sequence<beans::PropertyValue> aTableInd;
    uno::Sequence<beans::PropertyValue> aTableBorders;
    uno::Sequence<beans::PropertyValue> aTableCellMar;
    boost::optional<sal_Int32> oTableStyleRowBandSize;
    boost::optional<sal_Int32> oTableStyleColBandSize;
    for (sal_Int32 i = 0; i < rTablePr.getLength(); ++i)
    {
        if (rTablePr[i].Name == "tblStyleRowBandSize")
            oTableStyleRowBandSize = rTablePr[i].Value.get<sal_Int32>();
        else if (rTablePr[i].Name == "tblStyleColBandSize")
            oTableStyleColBandSize = rTablePr[i].Value.get<sal_Int32>();
        else if (rTablePr[i].Name == "tblInd")
            aTableInd = rTablePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTablePr[i].Name == "tblBorders")
            aTableBorders = rTablePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTablePr[i].Name == "tblCellMar")
            aTableCellMar = rTablePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
    }
    if (oTableStyleRowBandSize)
        m_pSerializer->singleElementNS(XML_w, XML_tblStyleRowBandSize, FSNS(XML_w, XML_val),
                                       OString::number(oTableStyleRowBandSize.get()), FSEND);
    if (oTableStyleColBandSize)
        m_pSerializer->singleElementNS(XML_w, XML_tblStyleColBandSize, FSNS(XML_w, XML_val),
                                       OString::number(oTableStyleColBandSize.get()), FSEND);
    tableStyleTableInd(aTableInd);
    tableStyleTcBorders(aTableBorders, XML_tblBorders);
    tableStyleTableCellMar(aTableCellMar);

    m_pSerializer->endElementNS(XML_w, XML_tblPr);
}

void DocxTableStyleExport::Impl::tableStyleTcPr(uno::Sequence<beans::PropertyValue>& rTcPr)
{
    if (!rTcPr.hasElements())
        return;

    m_pSerializer->startElementNS(XML_w, XML_tcPr, FSEND);

    uno::Sequence<beans::PropertyValue> aShd;
    uno::Sequence<beans::PropertyValue> aTcBorders;
    uno::Sequence<beans::PropertyValue> aTcMar;
    OUString aVAlign;
    for (sal_Int32 i = 0; i < rTcPr.getLength(); ++i)
    {
        if (rTcPr[i].Name == "shd")
            aShd = rTcPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTcPr[i].Name == "tcBorders")
            aTcBorders = rTcPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTcPr[i].Name == "tcMar")
            aTcMar = rTcPr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTcPr[i].Name == "vAlign")
            aVAlign = rTcPr[i].Value.get<OUString>();
    }
    tableStyleTcBorders(aTcBorders);
    tableStyleTableCellMar(aTcMar, XML_tcMar);
    tableStyleShd(aShd);
    if (!aVAlign.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), aVAlign.toUtf8(),
                                       FSEND);

    m_pSerializer->endElementNS(XML_w, XML_tcPr);
}

void DocxTableStyleExport::Impl::tableStyleTableStylePr(
    uno::Sequence<beans::PropertyValue>& rTableStylePr)
{
    if (!rTableStylePr.hasElements())
        return;

    OUString aType;
    uno::Sequence<beans::PropertyValue> aPPr;
    uno::Sequence<beans::PropertyValue> aRPr;
    uno::Sequence<beans::PropertyValue> aTablePr;
    uno::Sequence<beans::PropertyValue> aTcPr;
    for (sal_Int32 i = 0; i < rTableStylePr.getLength(); ++i)
    {
        if (rTableStylePr[i].Name == "type")
            aType = rTableStylePr[i].Value.get<OUString>();
        else if (rTableStylePr[i].Name == "pPr")
            aPPr = rTableStylePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTableStylePr[i].Name == "rPr")
            aRPr = rTableStylePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTableStylePr[i].Name == "tblPr")
            aTablePr = rTableStylePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rTableStylePr[i].Name == "tcPr")
            aTcPr = rTableStylePr[i].Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    m_pSerializer->startElementNS(XML_w, XML_tblStylePr, FSNS(XML_w, XML_type), aType.toUtf8(),
                                  FSEND);

    tableStylePPr(aPPr);
    tableStyleRPr(aRPr);
    if (aTablePr.hasElements())
        tableStyleTablePr(aTablePr);
    else
    {
        // Even if we have an empty container, write it out, as Word does.
        m_pSerializer->singleElementNS(XML_w, XML_tblPr, FSEND);
    }
    tableStyleTcPr(aTcPr);

    m_pSerializer->endElementNS(XML_w, XML_tblStylePr);
}

void DocxTableStyleExport::Impl::TableStyle(uno::Sequence<beans::PropertyValue>& rStyle)
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
    for (sal_Int32 i = 0; i < rStyle.getLength(); ++i)
    {
        if (rStyle[i].Name == "default")
            bDefault = rStyle[i].Value.get<bool>();
        else if (rStyle[i].Name == "customStyle")
            bCustomStyle = rStyle[i].Value.get<bool>();
        else if (rStyle[i].Name == "styleId")
            aStyleId = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "name")
            aName = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "basedOn")
            aBasedOn = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "uiPriority")
            aUiPriority = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "qFormat")
            bQFormat = true;
        else if (rStyle[i].Name == "semiHidden")
            bSemiHidden = true;
        else if (rStyle[i].Name == "unhideWhenUsed")
            bUnhideWhenUsed = true;
        else if (rStyle[i].Name == "rsid")
            aRsid = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "pPr")
            aPPr = rStyle[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rStyle[i].Name == "rPr")
            aRPr = rStyle[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rStyle[i].Name == "tblPr")
            aTablePr = rStyle[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rStyle[i].Name == "tcPr")
            aTcPr = rStyle[i].Value.get<uno::Sequence<beans::PropertyValue>>();
        else if (rStyle[i].Name == "tblStylePr")
            aTableStylePrs.push_back(rStyle[i].Value.get<uno::Sequence<beans::PropertyValue>>());
    }

    sax_fastparser::FastAttributeList* pAttributeList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    pAttributeList->add(FSNS(XML_w, XML_type), "table");
    if (bDefault)
        pAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    if (!aStyleId.isEmpty())
        pAttributeList->add(FSNS(XML_w, XML_styleId), aStyleId.toUtf8());
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->startElementNS(XML_w, XML_style, xAttributeList);

    m_pSerializer->singleElementNS(XML_w, XML_name, FSNS(XML_w, XML_val), aName.toUtf8(), FSEND);
    if (!aBasedOn.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_basedOn, FSNS(XML_w, XML_val), aBasedOn.toUtf8(),
                                       FSEND);
    if (!aUiPriority.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority, FSNS(XML_w, XML_val),
                                       aUiPriority.toUtf8(), FSEND);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden, FSEND);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed, FSEND);
    if (bQFormat)
        m_pSerializer->singleElementNS(XML_w, XML_qFormat, FSEND);
    if (!aRsid.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_rsid, FSNS(XML_w, XML_val), aRsid.toUtf8(),
                                       FSEND);

    tableStylePPr(aPPr);
    tableStyleRPr(aRPr);
    tableStyleTablePr(aTablePr);
    tableStyleTcPr(aTcPr);
    for (uno::Sequence<beans::PropertyValue>& i : aTableStylePrs)
        tableStyleTableStylePr(i);

    m_pSerializer->endElementNS(XML_w, XML_style);
}

void DocxTableStyleExport::SetSerializer(const sax_fastparser::FSHelperPtr& pSerializer)
{
    m_pImpl->setSerializer(pSerializer);
}

DocxTableStyleExport::DocxTableStyleExport(SwDoc* pDoc,
                                           const sax_fastparser::FSHelperPtr& pSerializer)
    : m_pImpl(std::make_unique<Impl>(pDoc))
{
    m_pImpl->setSerializer(pSerializer);
}

DocxTableStyleExport::~DocxTableStyleExport() = default;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
