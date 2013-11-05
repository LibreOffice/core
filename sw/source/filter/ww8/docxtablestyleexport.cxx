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
#include <comphelper/string.hxx>
#include <filter/msfilter/util.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <boost/optional.hpp>

using namespace com::sun::star;
using namespace oox;

struct DocxTableStyleExport::Impl
{
    SwDoc* m_pDoc;
    sax_fastparser::FSHelperPtr m_pSerializer;

    void TableStyle(uno::Sequence<beans::PropertyValue>& rStyle);
};

void DocxTableStyleExport::TableStyles()
{
    // Do we have table styles from InteropGrabBag available?
    uno::Reference<beans::XPropertySet> xPropertySet(m_pImpl->m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
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

    for (sal_Int32 i = 0; i < aTableStyles.getLength(); ++i)
    {
        uno::Sequence<beans::PropertyValue> aTableStyle;
        aTableStyles[i].Value >>= aTableStyle;
        m_pImpl->TableStyle(aTableStyle);
    }
}

DocxStringTokenMap const aTblCellMarTokens[] = {
    {"left", XML_left},
    {"right", XML_right},
    {"start", XML_start},
    {"end", XML_end},
    {"top", XML_top},
    {"bottom", XML_bottom},
    {0, 0}
};

/// Export of w:tblCellMar in a table style.
void lcl_TableStyleTblCellMar(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTblCellMar, sal_Int32 nType = XML_tblCellMar)
{
    if (!rTblCellMar.hasElements())
        return;

    pSerializer->startElementNS(XML_w, nType, FSEND);
    for (sal_Int32 i = 0; i < rTblCellMar.getLength(); ++i)
    {
        if (sal_Int32 nToken = DocxStringGetToken(aTblCellMarTokens, rTblCellMar[i].Name))
        {
            comphelper::SequenceAsHashMap aMap(rTblCellMar[i].Value.get< uno::Sequence<beans::PropertyValue> >());
            pSerializer->singleElementNS(XML_w, nToken,
                    FSNS(XML_w, XML_w), OString::number(aMap["w"].get<sal_Int32>()),
                    FSNS(XML_w, XML_type), OUStringToOString(aMap["type"].get<OUString>(), RTL_TEXTENCODING_UTF8).getStr(),
                    FSEND);
        }
    }
    pSerializer->endElementNS(XML_w, nType);
}

static DocxStringTokenMap const aTcBorderTokens[] = {
    {"val", XML_val},
    {"sz", XML_sz},
    {"color", XML_color},
    {"space", XML_space},
    {"themeColor", XML_themeColor},
    {"themeTint", XML_themeTint},
    {0, 0}
};

/// Export of a given table cell border type in a table style.
void lcl_TableStyleTcBorder(sax_fastparser::FSHelperPtr pSerializer, sal_Int32 nToken, const uno::Sequence<beans::PropertyValue>& rTcBorder)
{
    if (!rTcBorder.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rTcBorder.getLength(); ++i)
        if (sal_Int32 nAttrToken = DocxStringGetToken(aTcBorderTokens, rTcBorder[i].Name))
            pAttributeList->add(FSNS(XML_w, nAttrToken), OUStringToOString(rTcBorder[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());

    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, nToken, xAttributeList);
}

DocxStringTokenMap const aTcBordersTokens[] = {
    {"left", XML_left},
    {"right", XML_right},
    {"start", XML_start},
    {"end", XML_end},
    {"top", XML_top},
    {"bottom", XML_bottom},
    {"insideH", XML_insideH},
    {"insideV", XML_insideV},
    {"tl2br", XML_tl2br},
    {"tr2bl", XML_tr2bl},
    {0, 0}
};

/// Export of w:tcBorders (and w:tblBorders) in a table style.
void lcl_TableStyleTcBorders(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTcBorders, sal_Int32 nToken = XML_tcBorders)
{
    if (!rTcBorders.hasElements())
        return;

    pSerializer->startElementNS(XML_w, nToken, FSEND);
    for (sal_Int32 i = 0; i < rTcBorders.getLength(); ++i)
        if (sal_Int32 nSubToken = DocxStringGetToken(aTcBordersTokens, rTcBorders[i].Name))
            lcl_TableStyleTcBorder(pSerializer, nSubToken, rTcBorders[i].Value.get< uno::Sequence<beans::PropertyValue> >());
    pSerializer->endElementNS(XML_w, nToken);
}

/// Export of w:shd in a table style.
void lcl_TableStyleShd(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rShd)
{
    if (!rShd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rShd.getLength(); ++i)
    {
        if (rShd[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), OUStringToOString(rShd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rShd[i].Name == "color")
            pAttributeList->add(FSNS(XML_w, XML_color), msfilter::util::ConvertColor(rShd[i].Value.get<sal_Int32>(), /*bAutoColor =*/ true));
        else if (rShd[i].Name == "fill")
            pAttributeList->add(FSNS(XML_w, XML_fill), msfilter::util::ConvertColor(rShd[i].Value.get<sal_Int32>(), /*bAutoColor =*/ true));
        else if (rShd[i].Name == "themeFill")
            pAttributeList->add(FSNS(XML_w, XML_themeFill), OUStringToOString(rShd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rShd[i].Name == "themeFillShade")
            pAttributeList->add(FSNS(XML_w, XML_themeFillShade), OUStringToOString(rShd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_shd, xAttributeList);
}

/// Export of w:color in a table style.
void lcl_TableStyleRColor(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rColor)
{
    if (!rColor.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rColor.getLength(); ++i)
    {
        if (rColor[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), OUStringToOString(rColor[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rColor[i].Name == "themeColor")
            pAttributeList->add(FSNS(XML_w, XML_themeColor), OUStringToOString(rColor[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rColor[i].Name == "themeTint")
            pAttributeList->add(FSNS(XML_w, XML_themeTint), OUStringToOString(rColor[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rColor[i].Name == "themeShade")
            pAttributeList->add(FSNS(XML_w, XML_themeShade), OUStringToOString(rColor[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_color, xAttributeList);
}

/// Export of w:lang in a table style.
void lcl_TableStyleRLang(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rLang)
{
    if (!rLang.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rLang.getLength(); ++i)
    {
        if (rLang[i].Name == "eastAsia")
            pAttributeList->add(FSNS(XML_w, XML_eastAsia), OUStringToOString(rLang[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rLang[i].Name == "val")
            pAttributeList->add(FSNS(XML_w, XML_val), OUStringToOString(rLang[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rLang[i].Name == "bidi")
            pAttributeList->add(FSNS(XML_w, XML_bidi), OUStringToOString(rLang[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_lang, xAttributeList);
}

/// Export of w:rFonts in a table style.
void lcl_TableStyleRRFonts(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rRFonts)
{
    if (!rRFonts.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rRFonts.getLength(); ++i)
    {
        if (rRFonts[i].Name == "eastAsiaTheme")
            pAttributeList->add(FSNS(XML_w, XML_eastAsiaTheme), OUStringToOString(rRFonts[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rRFonts[i].Name == "asciiTheme")
            pAttributeList->add(FSNS(XML_w, XML_asciiTheme), OUStringToOString(rRFonts[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rRFonts[i].Name == "cstheme")
            pAttributeList->add(FSNS(XML_w, XML_cstheme), OUStringToOString(rRFonts[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rRFonts[i].Name == "hAnsiTheme")
            pAttributeList->add(FSNS(XML_w, XML_hAnsiTheme), OUStringToOString(rRFonts[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_rFonts, xAttributeList);
}

/// Export of w:spacing in a table style.
void lcl_TableStylePSpacing(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rSpacing)
{
    if (!rSpacing.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rSpacing.getLength(); ++i)
    {
        if (rSpacing[i].Name == "after")
            pAttributeList->add(FSNS(XML_w, XML_after), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "before")
            pAttributeList->add(FSNS(XML_w, XML_before), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "line")
            pAttributeList->add(FSNS(XML_w, XML_line), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "lineRule")
            pAttributeList->add(FSNS(XML_w, XML_lineRule), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "beforeLines")
            pAttributeList->add(FSNS(XML_w, XML_beforeLines), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "beforeAutospacing")
            pAttributeList->add(FSNS(XML_w, XML_beforeAutospacing), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "afterLines")
            pAttributeList->add(FSNS(XML_w, XML_afterLines), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rSpacing[i].Name == "afterAutospacing")
            pAttributeList->add(FSNS(XML_w, XML_afterAutospacing), OUStringToOString(rSpacing[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_spacing, xAttributeList);
}

/// Export of w:ind in a table style's pPr.
void lcl_TableStylePInd(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rInd)
{
    if (!rInd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rInd.getLength(); ++i)
    {
        if (rInd[i].Name == "rightChars")
            pAttributeList->add(FSNS(XML_w, XML_rightChars), OUStringToOString(rInd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
        else if (rInd[i].Name == "right")
            pAttributeList->add(FSNS(XML_w, XML_right), OUStringToOString(rInd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_ind, xAttributeList);
}

/// Export of w:tblInd in a table style.
void lcl_TableStyleTblInd(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTblInd)
{
    if (!rTblInd.hasElements())
        return;

    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    for (sal_Int32 i = 0; i < rTblInd.getLength(); ++i)
    {
        if (rTblInd[i].Name == "w")
            pAttributeList->add(FSNS(XML_w, XML_w), OString::number(rTblInd[i].Value.get<sal_Int32>()));
        else if (rTblInd[i].Name == "type")
            pAttributeList->add(FSNS(XML_w, XML_type), OUStringToOString(rTblInd[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8).getStr());
    }
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, XML_tblInd, xAttributeList);
}

void lcl_handleBoolean(OUString aValue, sal_Int32 nToken, sax_fastparser::FSHelperPtr pSerializer)
{
    if (aValue.isEmpty())
        return;
    sax_fastparser::FastAttributeList* pAttributeList = pSerializer->createAttrList();
    if (aValue != "1")
        pAttributeList->add(FSNS(XML_w, XML_val), OUStringToOString(aValue, RTL_TEXTENCODING_UTF8).getStr());
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    pSerializer->singleElementNS(XML_w, nToken, xAttributeList);
}

/// Export of w:rPr in a table style.
void lcl_TableStyleRPr(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rRPr)
{
    if (!rRPr.hasElements())
        return;

    pSerializer->startElementNS(XML_w, XML_rPr, FSEND);

    uno::Sequence<beans::PropertyValue> aRFonts, aLang, aColor;
    OUString aB, aBCs, aI, aSz, aSzCs, aCaps, aSmallCaps, aSpacing;
    for (sal_Int32 i = 0; i < rRPr.getLength(); ++i)
    {
        if (rRPr[i].Name == "rFonts")
            aRFonts = rRPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rRPr[i].Name == "lang")
            aLang = rRPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rRPr[i].Name == "b")
            aB = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "bCs")
            aBCs = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "i")
            aI = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "color")
            aColor = rRPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rRPr[i].Name == "sz")
            aSz = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "szCs")
            aSzCs = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "caps")
            aCaps = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "smallCaps")
            aSmallCaps = rRPr[i].Value.get<OUString>();
        else if (rRPr[i].Name == "spacing")
            aSpacing = rRPr[i].Value.get<OUString>();
    }
    lcl_TableStyleRRFonts(pSerializer, aRFonts);
    lcl_TableStyleRLang(pSerializer, aLang);
    lcl_handleBoolean(aB, XML_b, pSerializer);
    lcl_handleBoolean(aBCs, XML_bCs, pSerializer);
    lcl_handleBoolean(aI, XML_i, pSerializer);
    lcl_handleBoolean(aCaps, XML_caps, pSerializer);
    lcl_handleBoolean(aSmallCaps, XML_smallCaps, pSerializer);
    lcl_TableStyleRColor(pSerializer, aColor);
    if (!aSpacing.isEmpty())
        pSerializer->singleElementNS(XML_w, XML_spacing,
                FSNS(XML_w, XML_val), OUStringToOString(aSpacing, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (!aSz.isEmpty())
        pSerializer->singleElementNS(XML_w, XML_sz,
                FSNS(XML_w, XML_val), OUStringToOString(aSz, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (!aSzCs.isEmpty())
        pSerializer->singleElementNS(XML_w, XML_szCs,
                FSNS(XML_w, XML_val), OUStringToOString(aSzCs, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);

    pSerializer->endElementNS(XML_w, XML_rPr);
}

/// Export of w:pPr in a table style.
void lcl_TableStylePPr(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rPPr)
{
    if (!rPPr.hasElements())
        return;

    pSerializer->startElementNS(XML_w, XML_pPr, FSEND);

    uno::Sequence<beans::PropertyValue> aSpacing, aInd;
    bool bWordWrap = false;
    OUString aJc, aSnapToGrid;
    for (sal_Int32 i = 0; i < rPPr.getLength(); ++i)
    {
        if (rPPr[i].Name == "spacing")
            aSpacing = rPPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rPPr[i].Name == "ind")
            aInd = rPPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rPPr[i].Name == "wordWrap")
            bWordWrap = true;
        else if (rPPr[i].Name == "jc")
            aJc = rPPr[i].Value.get<OUString>();
        else if (rPPr[i].Name == "snapToGrid")
            aSnapToGrid = rPPr[i].Value.get<OUString>();
    }
    if (bWordWrap)
        pSerializer->singleElementNS(XML_w, XML_wordWrap, FSEND);
    lcl_TableStylePInd(pSerializer, aInd);
    lcl_handleBoolean(aSnapToGrid, XML_snapToGrid, pSerializer);
    lcl_TableStylePSpacing(pSerializer, aSpacing);
    if (!aJc.isEmpty())
        pSerializer->singleElementNS(XML_w, XML_jc,
                FSNS(XML_w, XML_val), OUStringToOString(aJc, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);

    pSerializer->endElementNS(XML_w, XML_pPr);
}

/// Export of w:tblPr in a table style.
void lcl_TableStyleTblPr(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTblPr)
{
    if (!rTblPr.hasElements())
        return;

    pSerializer->startElementNS(XML_w, XML_tblPr, FSEND);

    uno::Sequence<beans::PropertyValue> aTblInd, aTblBorders, aTblCellMar;
    boost::optional<sal_Int32> oTblStyleRowBandSize, oTblStyleColBandSize;
    for (sal_Int32 i = 0; i < rTblPr.getLength(); ++i)
    {
        if (rTblPr[i].Name == "tblStyleRowBandSize")
            oTblStyleRowBandSize = rTblPr[i].Value.get<sal_Int32>();
        else if (rTblPr[i].Name == "tblStyleColBandSize")
            oTblStyleColBandSize = rTblPr[i].Value.get<sal_Int32>();
        else if (rTblPr[i].Name == "tblInd")
            aTblInd = rTblPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTblPr[i].Name == "tblBorders")
            aTblBorders = rTblPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTblPr[i].Name == "tblCellMar")
            aTblCellMar = rTblPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    if (oTblStyleRowBandSize)
        pSerializer->singleElementNS(XML_w, XML_tblStyleRowBandSize,
                FSNS(XML_w, XML_val), OString::number(oTblStyleRowBandSize.get()),
                FSEND);
    if (oTblStyleColBandSize)
        pSerializer->singleElementNS(XML_w, XML_tblStyleColBandSize,
                FSNS(XML_w, XML_val), OString::number(oTblStyleColBandSize.get()),
                FSEND);
    lcl_TableStyleTblInd(pSerializer, aTblInd);
    lcl_TableStyleTcBorders(pSerializer, aTblBorders, XML_tblBorders);
    lcl_TableStyleTblCellMar(pSerializer, aTblCellMar);

    pSerializer->endElementNS(XML_w, XML_tblPr);
}

/// Export of w:tcPr in a table style.
void lcl_TableStyleTcPr(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTcPr)
{
    if (!rTcPr.hasElements())
        return;

    pSerializer->startElementNS(XML_w, XML_tcPr, FSEND);

    uno::Sequence<beans::PropertyValue> aShd, aTcBorders, aTcMar;
    OUString aVAlign;
    for (sal_Int32 i = 0; i < rTcPr.getLength(); ++i)
    {
        if (rTcPr[i].Name == "shd")
            aShd = rTcPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTcPr[i].Name == "tcBorders")
            aTcBorders = rTcPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTcPr[i].Name == "tcMar")
            aTcMar = rTcPr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTcPr[i].Name == "vAlign")
            aVAlign = rTcPr[i].Value.get<OUString>();
    }
    lcl_TableStyleTcBorders(pSerializer, aTcBorders);
    lcl_TableStyleTblCellMar(pSerializer, aTcMar, XML_tcMar);
    lcl_TableStyleShd(pSerializer, aShd);
    if (!aVAlign.isEmpty())
        pSerializer->singleElementNS(XML_w, XML_vAlign,
                FSNS(XML_w, XML_val), OUStringToOString(aVAlign, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);

    pSerializer->endElementNS(XML_w, XML_tcPr);
}

/// Export of w:tblStylePr in a table style.
void lcl_TableStyleTblStylePr(sax_fastparser::FSHelperPtr pSerializer, uno::Sequence<beans::PropertyValue>& rTblStylePr)
{
    if (!rTblStylePr.hasElements())
        return;

    OUString aType;
    uno::Sequence<beans::PropertyValue> aPPr, aRPr, aTblPr, aTcPr;
    for (sal_Int32 i = 0; i < rTblStylePr.getLength(); ++i)
    {
        if (rTblStylePr[i].Name == "type")
            aType = rTblStylePr[i].Value.get<OUString>();
        else if (rTblStylePr[i].Name == "pPr")
            aPPr = rTblStylePr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTblStylePr[i].Name == "rPr")
            aRPr = rTblStylePr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTblStylePr[i].Name == "tblPr")
            aTblPr = rTblStylePr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rTblStylePr[i].Name == "tcPr")
            aTcPr = rTblStylePr[i].Value.get< uno::Sequence<beans::PropertyValue> >();
    }

    pSerializer->startElementNS(XML_w, XML_tblStylePr,
            FSNS(XML_w, XML_type), OUStringToOString(aType, RTL_TEXTENCODING_UTF8).getStr(),
            FSEND);

    lcl_TableStylePPr(pSerializer, aPPr);
    lcl_TableStyleRPr(pSerializer, aRPr);
    if (aTblPr.hasElements())
        lcl_TableStyleTblPr(pSerializer, aTblPr);
    else
    {
        // Even if we have an empty container, write it out, as Word does.
        pSerializer->singleElementNS(XML_w, XML_tblPr, FSEND);
    }
    lcl_TableStyleTcPr(pSerializer, aTcPr);

    pSerializer->endElementNS(XML_w, XML_tblStylePr);
}

void DocxTableStyleExport::Impl::TableStyle(uno::Sequence<beans::PropertyValue>& rStyle)
{
    bool bDefault = false, bCustomStyle = false, bQFormat = false, bSemiHidden = false, bUnhideWhenUsed = false;
    OUString aStyleId, aName, aBasedOn;
    sal_Int32 nUiPriority = 0, nRsid = 0;
    uno::Sequence<beans::PropertyValue> aPPr, aRPr, aTblPr, aTcPr;
    std::vector< uno::Sequence<beans::PropertyValue> > aTblStylePrs;
    for (sal_Int32 i = 0; i < rStyle.getLength(); ++i)
    {
        if (rStyle[i].Name == "default")
            bDefault = rStyle[i].Value.get<sal_Bool>();
        else if (rStyle[i].Name == "customStyle")
            bCustomStyle = rStyle[i].Value.get<sal_Bool>();
        else if (rStyle[i].Name == "styleId")
            aStyleId = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "name")
            aName = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "basedOn")
            aBasedOn = rStyle[i].Value.get<OUString>();
        else if (rStyle[i].Name == "uiPriority")
            nUiPriority = rStyle[i].Value.get<sal_Int32>();
        else if (rStyle[i].Name == "qFormat")
            bQFormat = true;
        else if (rStyle[i].Name == "semiHidden")
            bSemiHidden = true;
        else if (rStyle[i].Name == "unhideWhenUsed")
            bUnhideWhenUsed = true;
        else if (rStyle[i].Name == "rsid")
            nRsid = rStyle[i].Value.get<sal_Int32>();
        else if (rStyle[i].Name == "pPr")
            aPPr = rStyle[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rStyle[i].Name == "rPr")
            aRPr = rStyle[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rStyle[i].Name == "tblPr")
            aTblPr = rStyle[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rStyle[i].Name == "tcPr")
            aTcPr = rStyle[i].Value.get< uno::Sequence<beans::PropertyValue> >();
        else if (rStyle[i].Name == "tblStylePr")
            aTblStylePrs.push_back(rStyle[i].Value.get< uno::Sequence<beans::PropertyValue> >());
    }

    sax_fastparser::FastAttributeList* pAttributeList = m_pSerializer->createAttrList();
    pAttributeList->add(FSNS(XML_w, XML_type), "table");
    if (bDefault)
        pAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    if (!aStyleId.isEmpty())
        pAttributeList->add(FSNS(XML_w, XML_styleId), OUStringToOString(aStyleId, RTL_TEXTENCODING_UTF8).getStr());
    sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->startElementNS(XML_w, XML_style, xAttributeList);

    m_pSerializer->singleElementNS(XML_w, XML_name,
            FSNS(XML_w, XML_val), OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr(),
            FSEND);
    if (!aBasedOn.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_basedOn,
                FSNS(XML_w, XML_val), OUStringToOString(aBasedOn, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (nUiPriority)
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority,
                FSNS(XML_w, XML_val), OString::number(nUiPriority),
                FSEND);
    if (bQFormat)
        m_pSerializer->singleElementNS(XML_w, XML_qFormat, FSEND);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden, FSEND);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed, FSEND);
    if (nRsid)
    {
        // We want the rsid as a hex string, but always with the length of 8.
        OStringBuffer aBuf = OString::number(nRsid, 16);
        OStringBuffer aStr;
        comphelper::string::padToLength(aStr, 8 - aBuf.getLength(), '0');
        aStr.append(aBuf.getStr());
        m_pSerializer->singleElementNS(XML_w, XML_rsid,
                FSNS(XML_w, XML_val), aStr.getStr(),
                FSEND);
    }

    lcl_TableStylePPr(m_pSerializer, aPPr);
    lcl_TableStyleRPr(m_pSerializer, aRPr);
    lcl_TableStyleTblPr(m_pSerializer, aTblPr);
    lcl_TableStyleTcPr(m_pSerializer, aTcPr);
    for (size_t i = 0; i < aTblStylePrs.size(); ++i)
        lcl_TableStyleTblStylePr(m_pSerializer, aTblStylePrs[i]);

    m_pSerializer->endElementNS(XML_w, XML_style);
}

void DocxTableStyleExport::SetSerializer(sax_fastparser::FSHelperPtr pSerializer)
{
    m_pImpl->m_pSerializer = pSerializer;
}

DocxTableStyleExport::DocxTableStyleExport(SwDoc* pDoc, sax_fastparser::FSHelperPtr pSerializer)
    : m_pImpl(new Impl)
{
    m_pImpl->m_pDoc = pDoc;
    m_pImpl->m_pSerializer = pSerializer;
}

DocxTableStyleExport::~DocxTableStyleExport()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
