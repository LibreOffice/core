/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <export/SparklineExt.hxx>

#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace oox;

namespace xcl::exp
{
SparklineExt::SparklineExt(const XclExpRoot& rRoot,
                           std::vector<std::shared_ptr<sc::Sparkline>> const& pSparklines)
    : XclExpExt(rRoot)
{
    maURI = "{05C60535-1F16-4fd2-B633-F4F36F0B64E0}";

    for (auto const& pSparkline : pSparklines)
    {
        auto* pGroupPointer = pSparkline->getSparklineGroup().get();

        auto aIterator = m_aSparklineGroupMap.find(pGroupPointer);
        if (aIterator == m_aSparklineGroupMap.end())
        {
            std::vector<std::shared_ptr<sc::Sparkline>> aSparklineVector;
            aSparklineVector.push_back(pSparkline);
            m_aSparklineGroupMap.emplace(pGroupPointer, aSparklineVector);
        }
        else
        {
            aIterator->second.push_back(pSparkline);
        }
    }
}

void SparklineExt::SaveXml(XclExpXmlStream& rStream)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();
    rWorksheet->startElement(XML_ext, FSNS(XML_xmlns, XML_x14),
                             rStream.getNamespaceURL(OOX_NS(xls14Lst)), XML_uri, maURI);

    rWorksheet->startElementNS(XML_x14, XML_sparklineGroups, FSNS(XML_xmlns, XML_xm),
                               rStream.getNamespaceURL(OOX_NS(xm)));

    for (auto const & [ pSparklineGroup, rSparklineVector ] : m_aSparklineGroupMap)
    {
        addSparklineGroup(rStream, *pSparklineGroup, rSparklineVector);
    }

    rWorksheet->endElementNS(XML_x14, XML_sparklineGroups);
    rWorksheet->endElement(XML_ext);
}

void SparklineExt::addSparklineGroupAttributes(
    rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList,
    sc::SparklineGroup& rSparklineGroup)
{
    if (rSparklineGroup.m_fLineWeight != 0.75)
        pAttrList->add(XML_lineWeight, OString::number(rSparklineGroup.m_fLineWeight));

    if (rSparklineGroup.m_eType != sc::SparklineType::Line)
    {
        if (rSparklineGroup.m_eType == sc::SparklineType::Column)
            pAttrList->add(XML_type, "column");
        else if (rSparklineGroup.m_eType == sc::SparklineType::Stacked)
            pAttrList->add(XML_type, "stacked");
    }

    if (rSparklineGroup.m_bDateAxis)
        pAttrList->add(XML_dateAxis, "1");

    if (rSparklineGroup.m_eDisplayEmptyCellsAs != sc::DisplayEmptyCellAs::Zero)
    {
        if (rSparklineGroup.m_eDisplayEmptyCellsAs == sc::DisplayEmptyCellAs::Gap)
            pAttrList->add(XML_displayEmptyCellsAs, "gap");
        else if (rSparklineGroup.m_eDisplayEmptyCellsAs == sc::DisplayEmptyCellAs::Span)
            pAttrList->add(XML_displayEmptyCellsAs, "span");
    }

    if (rSparklineGroup.m_bMarkers)
        pAttrList->add(XML_markers, "1");
    if (rSparklineGroup.m_bHigh)
        pAttrList->add(XML_high, "1");
    if (rSparklineGroup.m_bLow)
        pAttrList->add(XML_low, "1");
    if (rSparklineGroup.m_bFirst)
        pAttrList->add(XML_first, "1");
    if (rSparklineGroup.m_bLast)
        pAttrList->add(XML_last, "1");
    if (rSparklineGroup.m_bNegative)
        pAttrList->add(XML_negative, "1");
    if (rSparklineGroup.m_bDisplayXAxis)
        pAttrList->add(XML_displayXAxis, "1");
    if (rSparklineGroup.m_bDisplayHidden)
        pAttrList->add(XML_displayHidden, "1");

    if (rSparklineGroup.m_eMinAxisType != sc::AxisType::Individual)
    {
        if (rSparklineGroup.m_eMinAxisType == sc::AxisType::Group)
            pAttrList->add(XML_minAxisType, "group");
        else if (rSparklineGroup.m_eMinAxisType == sc::AxisType::Custom)
            pAttrList->add(XML_minAxisType, "custom");
    }

    if (rSparklineGroup.m_eMaxAxisType != sc::AxisType::Individual)
    {
        if (rSparklineGroup.m_eMaxAxisType == sc::AxisType::Group)
            pAttrList->add(XML_maxAxisType, "group");
        else if (rSparklineGroup.m_eMaxAxisType == sc::AxisType::Custom)
            pAttrList->add(XML_maxAxisType, "custom");
    }

    if (rSparklineGroup.m_bRightToLeft)
        pAttrList->add(XML_rightToLeft, "1");

    if (rSparklineGroup.m_aManualMax && rSparklineGroup.m_eMaxAxisType == sc::AxisType::Custom)
        pAttrList->add(XML_manualMax, OString::number(*rSparklineGroup.m_aManualMax));

    if (rSparklineGroup.m_aManualMin && rSparklineGroup.m_eMinAxisType == sc::AxisType::Custom)
        pAttrList->add(XML_manualMin, OString::number(*rSparklineGroup.m_aManualMin));
}

void SparklineExt::addSparklineGroupColors(XclExpXmlStream& rStream,
                                           sc::SparklineGroup& rSparklineGroup)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();

    rWorksheet->singleElementNS(XML_x14, XML_colorSeries, XML_rgb,
                                XclXmlUtils::ToOString(rSparklineGroup.m_aColorSeries));

    if (rSparklineGroup.m_aColorSeries != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorNegative, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorNegative));
    }

    if (rSparklineGroup.m_aColorAxis != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorAxis, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorAxis));
    }

    if (rSparklineGroup.m_aColorMarkers != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorMarkers, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorMarkers));
    }

    if (rSparklineGroup.m_aColorFirst != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorFirst, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorFirst));
    }

    if (rSparklineGroup.m_aColorLast != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorLast, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorLast));
    }

    if (rSparklineGroup.m_aColorHigh != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorHigh, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorHigh));
    }

    if (rSparklineGroup.m_aColorLow != COL_TRANSPARENT)
    {
        rWorksheet->singleElementNS(XML_x14, XML_colorLow, XML_rgb,
                                    XclXmlUtils::ToOString(rSparklineGroup.m_aColorLow));
    }
}

void SparklineExt::addSparklineGroup(XclExpXmlStream& rStream, sc::SparklineGroup& rSparklineGroup,
                                     std::vector<std::shared_ptr<sc::Sparkline>> const& rSparklines)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();

    // Sparkline Group Attributes
    auto pAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
    addSparklineGroupAttributes(pAttrList, rSparklineGroup);

    rWorksheet->startElementNS(XML_x14, XML_sparklineGroup, pAttrList);

    addSparklineGroupColors(rStream, rSparklineGroup);

    // Sparklines

    rWorksheet->startElementNS(XML_x14, XML_sparklines);
    for (auto const& rSparkline : rSparklines)
    {
        rWorksheet->startElementNS(XML_x14, XML_sparkline);

        {
            rWorksheet->startElementNS(XML_xm, XML_f);

            OUString sRangeFormula;
            ScRefFlags eFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D;
            rSparkline->getInputRange().Format(sRangeFormula, eFlags, GetDoc(),
                                               formula::FormulaGrammar::CONV_XL_OOX, ' ', true);

            rWorksheet->writeEscaped(sRangeFormula);
            rWorksheet->endElementNS(XML_xm, XML_f);
        }

        {
            rWorksheet->startElementNS(XML_xm, XML_sqref);

            ScAddress::Details detailsXL(formula::FormulaGrammar::CONV_XL_OOX);
            ScAddress aAddress(rSparkline->getColumn(), rSparkline->getRow(), GetCurrScTab());
            OUString sLocation = aAddress.Format(ScRefFlags::VALID, &GetDoc(), detailsXL);

            rWorksheet->writeEscaped(sLocation);
            rWorksheet->endElementNS(XML_xm, XML_sqref);
        }

        rWorksheet->endElementNS(XML_x14, XML_sparkline);
    }
    rWorksheet->endElementNS(XML_x14, XML_sparklines);
    rWorksheet->endElementNS(XML_x14, XML_sparklineGroup);
}

SparklineBuffer::SparklineBuffer(const XclExpRoot& rRoot, XclExtLstRef const& xExtLst)
    : XclExpRoot(rRoot)
{
    if (sc::SparklineList* pSparklineList = GetDoc().GetSparklineList(GetCurrScTab()))
    {
        auto pSparklines = pSparklineList->getSparklines();
        if (!pSparklines.empty())
        {
            xExtLst->AddRecord(new xcl::exp::SparklineExt(GetRoot(), pSparklines));
        }
    }
}

} // end namespace xcl::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
