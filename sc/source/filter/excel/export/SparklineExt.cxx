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
#include <SparklineGroup.hxx>
#include <SparklineList.hxx>
#include <export/ExportTools.hxx>

using namespace oox;

namespace xcl::exp
{
SparklineExt::SparklineExt(const XclExpRoot& rRoot)
    : XclExpExt(rRoot)
{
    maURI = "{05C60535-1F16-4fd2-B633-F4F36F0B64E0}"_ostr;
}

void SparklineExt::SaveXml(XclExpXmlStream& rStream)
{
    auto& rDocument = GetDoc();

    auto* pSparklineList = rDocument.GetSparklineList(GetCurrScTab());
    if (!pSparklineList)
        return;

    auto const aSparklineGroups = pSparklineList->getSparklineGroups();

    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();
    rWorksheet->startElement(XML_ext, FSNS(XML_xmlns, XML_x14),
                             rStream.getNamespaceURL(OOX_NS(xls14Lst)), XML_uri, maURI);

    rWorksheet->startElementNS(XML_x14, XML_sparklineGroups, FSNS(XML_xmlns, XML_xm),
                               rStream.getNamespaceURL(OOX_NS(xm)));

    for (auto const& pSparklineGroup : aSparklineGroups)
    {
        auto const aSparklineVector = pSparklineList->getSparklinesFor(pSparklineGroup);
        addSparklineGroup(rStream, *pSparklineGroup, aSparklineVector);
    }

    rWorksheet->endElementNS(XML_x14, XML_sparklineGroups);
    rWorksheet->endElement(XML_ext);
}

void SparklineExt::addSparklineGroupAttributes(
    const rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList,
    const sc::SparklineAttributes& rAttributes)
{
    if (rAttributes.getLineWeight() != 0.75)
        pAttrList->add(XML_lineWeight, OString::number(rAttributes.getLineWeight()));

    if (rAttributes.getType() != sc::SparklineType::Line)
    {
        if (rAttributes.getType() == sc::SparklineType::Column)
            pAttrList->add(XML_type, "column");
        else if (rAttributes.getType() == sc::SparklineType::Stacked)
            pAttrList->add(XML_type, "stacked");
    }

    if (rAttributes.isDateAxis())
        pAttrList->add(XML_dateAxis, "1");

    if (rAttributes.getDisplayEmptyCellsAs() != sc::DisplayEmptyCellsAs::Zero)
    {
        if (rAttributes.getDisplayEmptyCellsAs() == sc::DisplayEmptyCellsAs::Gap)
            pAttrList->add(XML_displayEmptyCellsAs, "gap");
        else if (rAttributes.getDisplayEmptyCellsAs() == sc::DisplayEmptyCellsAs::Span)
            pAttrList->add(XML_displayEmptyCellsAs, "span");
    }

    if (rAttributes.isMarkers())
        pAttrList->add(XML_markers, "1");
    if (rAttributes.isHigh())
        pAttrList->add(XML_high, "1");
    if (rAttributes.isLow())
        pAttrList->add(XML_low, "1");
    if (rAttributes.isFirst())
        pAttrList->add(XML_first, "1");
    if (rAttributes.isLast())
        pAttrList->add(XML_last, "1");
    if (rAttributes.isNegative())
        pAttrList->add(XML_negative, "1");
    if (rAttributes.shouldDisplayXAxis())
        pAttrList->add(XML_displayXAxis, "1");
    if (rAttributes.shouldDisplayHidden())
        pAttrList->add(XML_displayHidden, "1");

    if (rAttributes.getMinAxisType() != sc::AxisType::Individual)
    {
        if (rAttributes.getMinAxisType() == sc::AxisType::Group)
            pAttrList->add(XML_minAxisType, "group");
        else if (rAttributes.getMinAxisType() == sc::AxisType::Custom)
            pAttrList->add(XML_minAxisType, "custom");
    }

    if (rAttributes.getMaxAxisType() != sc::AxisType::Individual)
    {
        if (rAttributes.getMaxAxisType() == sc::AxisType::Group)
            pAttrList->add(XML_maxAxisType, "group");
        else if (rAttributes.getMaxAxisType() == sc::AxisType::Custom)
            pAttrList->add(XML_maxAxisType, "custom");
    }

    if (rAttributes.isRightToLeft())
        pAttrList->add(XML_rightToLeft, "1");

    if (rAttributes.getManualMax() && rAttributes.getMaxAxisType() == sc::AxisType::Custom)
        pAttrList->add(XML_manualMax, OString::number(*rAttributes.getManualMax()));

    if (rAttributes.getManualMin() && rAttributes.getMinAxisType() == sc::AxisType::Custom)
        pAttrList->add(XML_manualMin, OString::number(*rAttributes.getManualMin()));
}

void SparklineExt::addSparklineGroupColors(XclExpXmlStream& rStream,
                                           const sc::SparklineAttributes& rAttributes)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();

    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorSeries),
                                rAttributes.getColorSeries());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorNegative),
                                rAttributes.getColorNegative());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorAxis),
                                rAttributes.getColorAxis());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorMarkers),
                                rAttributes.getColorMarkers());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorFirst),
                                rAttributes.getColorFirst());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorLast),
                                rAttributes.getColorLast());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorHigh),
                                rAttributes.getColorHigh());
    oox::xls::writeComplexColor(rWorksheet, FSNS(XML_x14, XML_colorLow), rAttributes.getColorLow());
}

void SparklineExt::addSparklineGroup(XclExpXmlStream& rStream, sc::SparklineGroup& rSparklineGroup,
                                     std::vector<std::shared_ptr<sc::Sparkline>> const& rSparklines)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStream.GetCurrentStream();

    // Sparkline Group Attributes
    auto pAttrList = sax_fastparser::FastSerializerHelper::createAttrList();

    // Write ID
    OString sUID = rSparklineGroup.getID().getString();
    pAttrList->addNS(XML_xr2, XML_uid, sUID);

    // Write attributes
    addSparklineGroupAttributes(pAttrList, rSparklineGroup.getAttributes());

    rWorksheet->startElementNS(XML_x14, XML_sparklineGroup, pAttrList);

    addSparklineGroupColors(rStream, rSparklineGroup.getAttributes());

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
    auto& rDocument = GetDoc();
    auto* pSparklineList = rDocument.GetSparklineList(GetCurrScTab());
    if (pSparklineList && !pSparklineList->getSparklineGroups().empty())
    {
        xExtLst->AddRecord(new xcl::exp::SparklineExt(GetRoot()));
    }
}

} // end namespace xcl::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
