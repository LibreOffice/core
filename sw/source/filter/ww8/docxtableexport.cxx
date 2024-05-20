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

#include "docxattributeoutput.hxx"

#include <com/sun/star/text/XTextTable.hpp>

#include <comphelper/sequence.hxx>
#include <svl/grabbagitem.hxx>
#include <sax/fshelper.hxx>
#include <editeng/ulspitem.hxx>
#include <comphelper/string.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <tools/datetimeutils.hxx>

#include <fmtfsize.hxx>
#include <unocoll.hxx>
#include <formatflysplit.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <swmodule.hxx>
#include <fmtrowsplt.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <unotbl.hxx>

#include "docxexportfilter.hxx"
#include "docxhelper.hxx"

using namespace com::sun::star;
using namespace sax_fastparser;
using namespace oox;

namespace
{
/// Does the same as comphelper::string::padToLength(), but extends the start, not the end.
OString lcl_padStartToLength(OString const& aString, sal_Int32 nLen, char cFill)
{
    if (nLen > aString.getLength())
    {
        sal_Int32 nDiff = nLen - aString.getLength();
        OStringBuffer aBuffer;
        comphelper::string::padToLength(aBuffer, nDiff, cFill);
        aBuffer.append(aString);
        return aBuffer.makeStringAndClear();
    }
    else
        return aString;
}

//Keep this function in-sync with the one in writerfilter/.../SettingsTable.cxx
//Since this is not import code, "-1" needs to be handled as the mode that LO will save as.
//To identify how your code should handle a "-1", look in DocxExport::WriteSettings().
sal_Int32 lcl_getWordCompatibilityMode(const DocxExport& rDocExport)
{
    sal_Int32 nWordCompatibilityMode = rDocExport.getWordCompatibilityModeFromGrabBag();

    // TODO: this is duplicated, better store it in DocxExport member?
    if (!rDocExport.m_rDoc.getIDocumentSettingAccess().get(DocumentSettingId::ADD_EXT_LEADING))
    {
        if (nWordCompatibilityMode == -1 || 14 < nWordCompatibilityMode)
        {
            nWordCompatibilityMode = 14;
        }
    }

    return nWordCompatibilityMode;
}

void CollectFloatingTableAttributes(DocxExport& rExport, const ww8::Frame& rFrame,
                                    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner,
                                    rtl::Reference<FastAttributeList>& pAttributes)
{
    // we export the values of the surrounding Frame
    OString sOrientation;
    sal_Int32 nValue;

    // If tblpXSpec or tblpYSpec are present, we do not write tblpX or tblpY!
    OString sTblpXSpec = DocxAttributeOutput::convertToOOXMLHoriOrient(
        rFrame.GetFrameFormat().GetHoriOrient().GetHoriOrient(),
        rFrame.GetFrameFormat().GetHoriOrient().IsPosToggle());
    OString sTblpYSpec = DocxAttributeOutput::convertToOOXMLVertOrient(
        rFrame.GetFrameFormat().GetVertOrient().GetVertOrient());

    sOrientation = DocxAttributeOutput::convertToOOXMLVertOrientRel(
        rFrame.GetFrameFormat().GetVertOrient().GetRelationOrient());
    pAttributes->add(FSNS(XML_w, XML_vertAnchor), sOrientation);

    if (!sTblpYSpec.isEmpty())
        pAttributes->add(FSNS(XML_w, XML_tblpYSpec), sTblpYSpec);

    sOrientation = DocxAttributeOutput::convertToOOXMLHoriOrientRel(
        rFrame.GetFrameFormat().GetHoriOrient().GetRelationOrient());
    pAttributes->add(FSNS(XML_w, XML_horzAnchor), sOrientation);

    if (!sTblpXSpec.isEmpty())
        pAttributes->add(FSNS(XML_w, XML_tblpXSpec), sTblpXSpec);

    nValue = rFrame.GetFrameFormat().GetULSpace().GetLower();
    if (nValue != 0)
        pAttributes->add(FSNS(XML_w, XML_bottomFromText), OString::number(nValue));

    nValue = rFrame.GetFrameFormat().GetLRSpace().GetLeft();
    if (nValue != 0)
        pAttributes->add(FSNS(XML_w, XML_leftFromText), OString::number(nValue));

    nValue = rFrame.GetFrameFormat().GetLRSpace().GetRight();
    if (nValue != 0)
        pAttributes->add(FSNS(XML_w, XML_rightFromText), OString::number(nValue));

    nValue = rFrame.GetFrameFormat().GetULSpace().GetUpper();
    if (nValue != 0)
        pAttributes->add(FSNS(XML_w, XML_topFromText), OString::number(nValue));

    if (sTblpXSpec.isEmpty()) // do not write tblpX if tblpXSpec is present
    {
        nValue = rFrame.GetFrameFormat().GetHoriOrient().GetPos();
        // we need to revert the additional shift introduced by
        // lcl_DecrementHoriOrientPosition() in writerfilter
        // 1st: left distance of the table
        const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
        const SwFrameFormat* pFrameFormat = pTabBox->GetFrameFormat();
        const SvxBoxItem& rBox = pFrameFormat->GetBox();
        sal_Int32 nMode = lcl_getWordCompatibilityMode(rExport);
        if (nMode < 15)
        {
            sal_uInt16 nLeftDistance = rBox.GetDistance(SvxBoxItemLine::LEFT);
            nValue += nLeftDistance;
        }

        // 2nd: if a left border is given, revert the shift by half the width
        // from lcl_DecrementHoriOrientPosition() in writerfilter
        if (const editeng::SvxBorderLine* pLeftBorder = rBox.GetLeft())
        {
            tools::Long nWidth = pLeftBorder->GetWidth();
            nValue += (nWidth / 2);
        }

        pAttributes->add(FSNS(XML_w, XML_tblpX), OString::number(nValue));
    }

    if (sTblpYSpec.isEmpty()) // do not write tblpY if tblpYSpec is present
    {
        nValue = rFrame.GetFrameFormat().GetVertOrient().GetPos();
        pAttributes->add(FSNS(XML_w, XML_tblpY), OString::number(nValue));
    }
}
}

void DocxAttributeOutput::TableInfoCell(
    const ww8::WW8TableNodeInfoInner::Pointer_t& /*pTableTextNodeInfoInner*/)
{
}

void DocxAttributeOutput::TableInfoRow(
    const ww8::WW8TableNodeInfoInner::Pointer_t& /*pTableTextNodeInfo*/)
{
}

void DocxAttributeOutput::TableDefinition(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    bool const bEcma = GetExport().GetFilter().getVersion() == oox::core::ECMA_376_1ST_EDITION;

    // Write the table properties
    m_pSerializer->startElementNS(XML_w, XML_tblPr);

    static const sal_Int32 aOrder[] = { FSNS(XML_w, XML_tblStyle),
                                        FSNS(XML_w, XML_tblpPr),
                                        FSNS(XML_w, XML_tblOverlap),
                                        FSNS(XML_w, XML_bidiVisual),
                                        FSNS(XML_w, XML_tblStyleRowBandSize),
                                        FSNS(XML_w, XML_tblStyleColBandSize),
                                        FSNS(XML_w, XML_tblW),
                                        FSNS(XML_w, XML_jc),
                                        FSNS(XML_w, XML_tblCellSpacing),
                                        FSNS(XML_w, XML_tblInd),
                                        FSNS(XML_w, XML_tblBorders),
                                        FSNS(XML_w, XML_shd),
                                        FSNS(XML_w, XML_tblLayout),
                                        FSNS(XML_w, XML_tblCellMar),
                                        FSNS(XML_w, XML_tblLook),
                                        FSNS(XML_w, XML_tblPrChange) };

    // postpone the output so that we can later []
    // prepend the properties before the run
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    m_pSerializer->mark(Tag_TableDefinition, comphelper::containerToSequence(aOrder));

    tools::Long nPageSize = 0;
    const char* widthType = "dxa";

    // If actual width of table is relative it should export is as "pct".`
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    SwFrameFormat* pTableFormat = pTable->GetFrameFormat();
    const SwFormatFrameSize& rSize = pTableFormat->GetFrameSize();
    int nWidthPercent = rSize.GetWidthPercent();
    // If we export a floating table: we use the widthPercent of the surrounding frame
    const ww8::Frame* pFloatingTableFrame = m_rExport.GetFloatingTableFrame();
    if (pFloatingTableFrame)
    {
        const SwFormatFrameSize& rFrameSize = pFloatingTableFrame->GetFrameFormat().GetFrameSize();
        nWidthPercent = rFrameSize.GetWidthPercent();
    }

    rtl::Reference<SwXTextTable> xPropertySet = SwXTextTables::GetObject(*pTable->GetFrameFormat());
    bool isWidthRelative = false;
    xPropertySet->getPropertyValue("IsWidthRelative") >>= isWidthRelative;
    if (!isWidthRelative && !nWidthPercent)
    {
        // The best fit for "automatic" table placement is relative 100%
        short nHoriOrient = -1;
        xPropertySet->getPropertyValue("HoriOrient") >>= nHoriOrient;
        isWidthRelative = nHoriOrient == text::HoriOrientation::FULL;
        if (isWidthRelative)
            nWidthPercent = 100;
    }

    if (isWidthRelative)
    {
        /**
       * As per ECMA Specification : ECMA-376, Second Edition, Part 1 - Fundamentals And Markup Language Reference [ 17.18.90 ST_TableWidth (Table Width Units)]
       * http://www.schemacentral.com/sc/ooxml/a-w_type-7.html
       *
       * Fiftieths of a Percent :
       * http://startbigthinksmall.wordpress.com/2010/01/04/points-inches-and-emus-measuring-units-in-office-open-xml/
       * pct Width is in Fiftieths of a Percent
       *
       * ex. If the Table width is 50% then
       * Width in Fiftieths of a percent is (50 * 50) % or 0.5 * 5000 = 2500pct
       **/
        nPageSize = nWidthPercent * 50;
        widthType = "pct";
    }
    else
    {
        bool bRelBoxSize = false;
        // Create the SwWriteTable instance to use col spans (and maybe other infos)
        GetTablePageSize(pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize);
        if (nPageSize == 0)
            widthType = "auto";
    }

    // Output the table preferred width
    m_pSerializer->singleElementNS(XML_w, XML_tblW, FSNS(XML_w, XML_w), OString::number(nPageSize),
                                   FSNS(XML_w, XML_type), widthType);

    // Disable layout autofit, as it does not exist in LibreOffice yet
    m_pSerializer->singleElementNS(XML_w, XML_tblLayout, FSNS(XML_w, XML_type), "fixed");

    // Look for the table style property in the table grab bag
    const std::map<OUString, css::uno::Any>& rGrabBag
        = pTableFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    // We should clear the TableStyle map. In case of Table inside multiple tables it contains the
    // table border style of the previous table.
    std::map<SvxBoxItemLine, css::table::BorderLine2>& rTableStyleConf = m_aTableStyleConfs.back();
    rTableStyleConf.clear();

    bool bFloatingTableWritten = false;
    if (pFloatingTableFrame && pFloatingTableFrame->GetFrameFormat().GetFlySplit().GetValue())
    {
        rtl::Reference<FastAttributeList> pAttributes = FastSerializerHelper::createAttrList();
        CollectFloatingTableAttributes(m_rExport, *pFloatingTableFrame, pTableTextNodeInfoInner,
                                       pAttributes);
        m_pSerializer->singleElementNS(XML_w, XML_tblpPr, pAttributes);
        bFloatingTableWritten = true;
        // The outer table was floating, make sure potential inner tables are not floating.
        m_rExport.SetFloatingTableFrame(nullptr);

        const SwFrameFormat& rFloatingTableFormat = pFloatingTableFrame->GetFrameFormat();
        if (!rFloatingTableFormat.GetWrapInfluenceOnObjPos().GetAllowOverlap())
        {
            // Allowing overlap is the default, both in OOXML and in Writer.
            m_pSerializer->singleElementNS(XML_w, XML_tblOverlap, FSNS(XML_w, XML_val), "never");
        }
    }

    // Extract properties from grab bag
    for (const auto& rGrabBagElement : rGrabBag)
    {
        if (rGrabBagElement.first == "TableStyleName")
        {
            OString sStyleName
                = OUStringToOString(rGrabBagElement.second.get<OUString>(), RTL_TEXTENCODING_UTF8);
            m_pSerializer->singleElementNS(XML_w, XML_tblStyle, FSNS(XML_w, XML_val), sStyleName);
        }
        else if (rGrabBagElement.first == "TableStyleTopBorder")
            rTableStyleConf[SvxBoxItemLine::TOP] = rGrabBagElement.second.get<table::BorderLine2>();
        else if (rGrabBagElement.first == "TableStyleBottomBorder")
            rTableStyleConf[SvxBoxItemLine::BOTTOM]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if (rGrabBagElement.first == "TableStyleLeftBorder")
            rTableStyleConf[SvxBoxItemLine::LEFT]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if (rGrabBagElement.first == "TableStyleRightBorder")
            rTableStyleConf[SvxBoxItemLine::RIGHT]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if (rGrabBagElement.first == "TableStyleLook")
        {
            rtl::Reference<FastAttributeList> pAttributeList
                = FastSerializerHelper::createAttrList();
            const uno::Sequence<beans::PropertyValue> aAttributeList
                = rGrabBagElement.second.get<uno::Sequence<beans::PropertyValue>>();

            for (const auto& rAttribute : aAttributeList)
            {
                if (rAttribute.Name == "val")
                    pAttributeList->add(
                        FSNS(XML_w, XML_val),
                        lcl_padStartToLength(OString::number(rAttribute.Value.get<sal_Int32>(), 16),
                                             4, '0'));
                else
                {
                    static DocxStringTokenMap const aTokens[]
                        = { { "firstRow", XML_firstRow },
                            { "lastRow", XML_lastRow },
                            { "firstColumn", XML_firstColumn },
                            { "lastColumn", XML_lastColumn },
                            { "noHBand", XML_noHBand },
                            { "noVBand", XML_noVBand },
                            { nullptr, 0 } };

                    if (sal_Int32 nToken = DocxStringGetToken(aTokens, rAttribute.Name))
                        pAttributeList->add(FSNS(XML_w, nToken),
                                            (rAttribute.Value.get<sal_Int32>() ? "1" : "0"));
                }
            }

            m_pSerializer->singleElementNS(XML_w, XML_tblLook, pAttributeList);
        }
        else if (rGrabBagElement.first == "TablePosition" &&
                 // skip empty table position (tables in footnotes converted to
                 // floating tables temporarily, don't export this)
                 rGrabBagElement.second != uno::Any())
        {
            rtl::Reference<FastAttributeList> attrListTablePos
                = FastSerializerHelper::createAttrList();
            const uno::Sequence<beans::PropertyValue> aTablePosition
                = rGrabBagElement.second.get<uno::Sequence<beans::PropertyValue>>();
            // look for a surrounding frame and take it's position values
            const ww8::Frame* pFrame = m_rExport.GetFloatingTableFrame();
            if (pFrame)
            {
                CollectFloatingTableAttributes(m_rExport, *pFrame, pTableTextNodeInfoInner,
                                               attrListTablePos);
            }
            else // ( pFrame = 0 )
            {
                // we export the values from the grabBag
                for (const auto& rProp : aTablePosition)
                {
                    if (rProp.Name == "vertAnchor" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation
                            = OUStringToOString(rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_vertAnchor), sOrientation);
                    }
                    else if (rProp.Name == "tblpYSpec" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation
                            = OUStringToOString(rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_tblpYSpec), sOrientation);
                    }
                    else if (rProp.Name == "horzAnchor" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation
                            = OUStringToOString(rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_horzAnchor), sOrientation);
                    }
                    else if (rProp.Name == "tblpXSpec" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation
                            = OUStringToOString(rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_tblpXSpec), sOrientation);
                    }
                    else if (rProp.Name == "bottomFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_bottomFromText),
                                              OString::number(nValue));
                    }
                    else if (rProp.Name == "leftFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_leftFromText),
                                              OString::number(nValue));
                    }
                    else if (rProp.Name == "rightFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_rightFromText),
                                              OString::number(nValue));
                    }
                    else if (rProp.Name == "topFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_topFromText),
                                              OString::number(nValue));
                    }
                    else if (rProp.Name == "tblpX")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_tblpX), OString::number(nValue));
                    }
                    else if (rProp.Name == "tblpY")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add(FSNS(XML_w, XML_tblpY), OString::number(nValue));
                    }
                }
            }

            if (!bFloatingTableWritten)
            {
                m_pSerializer->singleElementNS(XML_w, XML_tblpPr, attrListTablePos);
            }
        }
        else
            SAL_WARN("sw.ww8", "DocxAttributeOutput::TableDefinition: unhandled property: "
                                   << rGrabBagElement.first);
    }

    // Output the table alignment
    const char* pJcVal;
    sal_Int32 nIndent = 0;
    switch (pTableFormat->GetHoriOrient().GetHoriOrient())
    {
        case text::HoriOrientation::CENTER:
            pJcVal = "center";
            break;
        case text::HoriOrientation::RIGHT:
            if (bEcma)
                pJcVal = "right";
            else
                pJcVal = "end";
            break;
        default:
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
        {
            if (bEcma)
                pJcVal = "left";
            else
                pJcVal = "start";
            nIndent = sal_Int32(pTableFormat->GetLRSpace().GetLeft());

            // Table indentation has different meaning in Word, depending if the table is nested or not.
            // If nested, tblInd is added to parent table's left spacing and defines left edge position
            // If not nested, text position of left-most cell must be at absolute X = tblInd
            // so, table_spacing + table_spacing_to_content = tblInd

            // tdf#106742: since MS Word 2013 (compatibilityMode >= 15), top-level tables are handled the same as nested tables;
            // if no compatibilityMode is defined (which now should only happen on a new export to .docx),
            // LO uses a higher compatibility than 2010's 14.
            sal_Int32 nMode = lcl_getWordCompatibilityMode(m_rExport);

            const SwFrameFormat* pFrameFormat
                = pTableTextNodeInfoInner->getTableBox()->GetFrameFormat();
            if ((0 < nMode && nMode <= 14) && m_tableReference.m_nTableDepth == 0)
                nIndent += pFrameFormat->GetBox().GetDistance(SvxBoxItemLine::LEFT);
            else
            {
                // adjust for SW considering table to start mid-border instead of nested/2013's left-side-of-border.
                nIndent -= pFrameFormat->GetBox().CalcLineWidth(SvxBoxItemLine::LEFT) / 2;
            }

            break;
        }
    }
    m_pSerializer->singleElementNS(XML_w, XML_jc, FSNS(XML_w, XML_val), pJcVal);

    // Output the table background color (although cell value still needs to be specified)
    const SvxBrushItem* pColorProp
        = pTableFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    Color aColor = pColorProp ? pColorProp->GetColor() : COL_AUTO;
    if (aColor != COL_AUTO)
    {
        OString sColor = msfilter::util::ConvertColor(aColor);
        m_pSerializer->singleElementNS(XML_w, XML_shd, FSNS(XML_w, XML_fill), sColor,
                                       FSNS(XML_w, XML_val), "clear");
    }

    // Output the table borders
    TableDefaultBorders(pTableTextNodeInfoInner);

    // Output the default cell margins
    TableDefaultCellMargins(pTableTextNodeInfoInner);

    TableBidi(pTableTextNodeInfoInner);

    // Table indent (need to get written even if == 0)
    m_pSerializer->singleElementNS(XML_w, XML_tblInd, FSNS(XML_w, XML_w), OString::number(nIndent),
                                   FSNS(XML_w, XML_type), "dxa");

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_TableDefinition);

    m_pSerializer->endElementNS(XML_w, XML_tblPr);

    // Write the table grid infos
    m_pSerializer->startElementNS(XML_w, XML_tblGrid);
    sal_Int32 nPrv = 0;
    ww8::WidthsPtr pColumnWidths = GetColumnWidths(pTableTextNodeInfoInner);
    for (auto aColumnWidth : *pColumnWidths)
    {
        sal_Int32 nWidth = sal_Int32(aColumnWidth) - nPrv;
        m_pSerializer->singleElementNS(XML_w, XML_gridCol, FSNS(XML_w, XML_w),
                                       OString::number(nWidth));
        nPrv = sal_Int32(aColumnWidth);
    }

    m_pSerializer->endElementNS(XML_w, XML_tblGrid);
}

void DocxAttributeOutput::TableDefaultBorders(
    const ww8::WW8TableNodeInfoInner::Pointer_t& /*pTableTextNodeInfoInner*/)
{
    // Table defaults should only be created IF m_aTableStyleConf contents haven't come from a table style.
    // Previously this function wrote out Cell A1 as the table default, causing problems with no benefit.
}

void DocxAttributeOutput::TableDefaultCellMargins(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat* pFrameFormat = pTabBox->GetFrameFormat();
    const SvxBoxItem& rBox = pFrameFormat->GetBox();
    const bool bEcma = GetExport().GetFilter().getVersion() == oox::core::ECMA_376_1ST_EDITION;

    DocxAttributeOutput::ImplCellMargins(m_pSerializer, rBox, XML_tblCellMar, !bEcma);
}

void DocxAttributeOutput::TableBackgrounds(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    const SwTableBox* pTableBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTableRow = pTableBox->GetUpper();
    const SwFrameFormat* pFormat = pTableBox->GetFrameFormat();

    const SvxBrushItem* pColorProp = pFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    Color aColor = pColorProp ? pColorProp->GetColor() : COL_AUTO;

    const SwFrameFormat* pRowFormat = pTableRow->GetFrameFormat();
    const SvxBrushItem* pRowColorProp
        = pRowFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if (pRowColorProp && aColor == COL_AUTO)
        aColor = pRowColorProp->GetColor();

    const SwFrameFormat* pTableFormat = pTable->GetFrameFormat();
    const SvxBrushItem* pTableColorProp
        = pTableFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if (pTableColorProp && aColor == COL_AUTO)
        aColor = pTableColorProp->GetColor();

    const OString sColor = msfilter::util::ConvertColor(aColor);

    const std::map<OUString, css::uno::Any>& rGrabBag
        = pFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    OString sOriginalColor;
    auto aGrabBagIt = rGrabBag.find("originalColor");
    if (aGrabBagIt != rGrabBag.end())
        sOriginalColor
            = OUStringToOString(aGrabBagIt->second.get<OUString>(), RTL_TEXTENCODING_UTF8);

    if (sOriginalColor != sColor)
    {
        // color changed by the user, or no grab bag: write sColor
        if (sColor != "auto")
        {
            m_pSerializer->singleElementNS(XML_w, XML_shd, FSNS(XML_w, XML_fill), sColor,
                                           FSNS(XML_w, XML_val), "clear");
        }
    }
    else
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pAttrList;

        for (const auto & [ name, val ] : rGrabBag)
        {
            if (!val.has<OUString>())
                continue;

            if (name == "themeFill")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeFill), val.get<OUString>());
            else if (name == "themeFillTint")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeFillTint), val.get<OUString>());
            else if (name == "themeFillShade")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeFillShade), val.get<OUString>());
            else if (name == "fill")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_fill), val.get<OUString>());
            else if (name == "themeColor")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeColor), val.get<OUString>());
            else if (name == "themeTint")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeTint), val.get<OUString>());
            else if (name == "themeShade")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_themeShade), val.get<OUString>());
            else if (name == "color")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_color), val.get<OUString>());
            else if (name == "val")
                AddToAttrList(pAttrList, FSNS(XML_w, XML_val), val.get<OUString>());
        }
        m_pSerializer->singleElementNS(XML_w, XML_shd, pAttrList);
    }
}

void DocxAttributeOutput::TableRowRedline(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTabLine = pTabBox->GetUpper();

    bool bRemovePersonalInfo
        = SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo)
          && !SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo);

    // check table row property "HasTextChangesOnly"
    SwRedlineTable::size_type nPos(0);
    SwRedlineTable::size_type nChange = pTabLine->UpdateTextChangesOnly(nPos);
    // tdf#150824 if no tracked table row, is the table in a single redline?
    // if yes, convert the row to a tracked table row instead of losing its tracking
    if (nChange == SwRedlineTable::npos)
        nChange = pTabLine->GetTableRedline();
    if (nChange != SwRedlineTable::npos)
    {
        const SwRedlineTable& aRedlineTable
            = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        const SwRangeRedline* pRedline = aRedlineTable[nChange];
        SwTableRowRedline* pTableRowRedline = nullptr;
        bool bIsInExtra = false;

        // use the original DOCX redline data stored in ExtraRedlineTable,
        // if it exists and its type wasn't changed
        const SwExtraRedlineTable& aExtraRedlineTable
            = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetExtraRedlineTable();
        for (sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize();
             ++nCurRedlinePos)
        {
            SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
            pTableRowRedline = dynamic_cast<SwTableRowRedline*>(pExtraRedline);
            if (pTableRowRedline && &pTableRowRedline->GetTableLine() == pTabLine)
            {
                bIsInExtra = true;
                break;
            }
        }

        const SwRedlineData& aRedlineData
            = bIsInExtra &&
                      // still the same type (an inserted row could become a tracked deleted one)
                      pTableRowRedline->GetRedlineData().GetType()
                          == pRedline->GetRedlineData().GetType()
                  ? pTableRowRedline->GetRedlineData()
                  : pRedline->GetRedlineData();

        // Note: all redline ranges and table row redline (with the same author and timestamp)
        // use the same redline id in OOXML exported by MSO, but it seems, the recent solution
        // (different IDs for different ranges, also row changes) is also portable.
        OString aId(OString::number(m_nRedlineId++));
        const OUString& rAuthor(SW_MOD()->GetRedlineAuthor(aRedlineData.GetAuthor()));
        OString aAuthor(OUStringToOString(
            bRemovePersonalInfo ? "Author" + OUString::number(GetExport().GetInfoID(rAuthor))
                                : rAuthor,
            RTL_TEXTENCODING_UTF8));

        const DateTime aDateTime = aRedlineData.GetTimeStamp();
        bool bNoDate = bRemovePersonalInfo
                       || (aDateTime.GetYear() == 1970 && aDateTime.GetMonth() == 1
                           && aDateTime.GetDay() == 1);

        if (bNoDate)
            m_pSerializer->singleElementNS(
                XML_w, RedlineType::Delete == pRedline->GetType() ? XML_del : XML_ins,
                FSNS(XML_w, XML_id), aId, FSNS(XML_w, XML_author), aAuthor);
        else
            m_pSerializer->singleElementNS(
                XML_w, RedlineType::Delete == pRedline->GetType() ? XML_del : XML_ins,
                FSNS(XML_w, XML_id), aId, FSNS(XML_w, XML_author), aAuthor, FSNS(XML_w, XML_date),
                DateTimeToOString(aDateTime));
        return;
    }
}

void DocxAttributeOutput::TableCellRedline(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();

    bool bRemovePersonalInfo
        = SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo)
          && !SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo);

    // check table row property "HasTextChangesOnly"
    SwRedlineTable::size_type nChange = pTabBox->GetRedline();
    if (nChange != SwRedlineTable::npos)
    {
        const SwRedlineTable& aRedlineTable
            = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        const SwRangeRedline* pRedline = aRedlineTable[nChange];
        SwTableCellRedline* pTableCellRedline = nullptr;
        bool bIsInExtra = false;

        // use the original DOCX redline data stored in ExtraRedlineTable,
        // if it exists and its type wasn't changed
        const SwExtraRedlineTable& aExtraRedlineTable
            = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetExtraRedlineTable();
        for (sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize();
             ++nCurRedlinePos)
        {
            SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
            pTableCellRedline = dynamic_cast<SwTableCellRedline*>(pExtraRedline);
            if (pTableCellRedline && &pTableCellRedline->GetTableBox() == pTabBox)
            {
                bIsInExtra = true;
                break;
            }
        }

        const SwRedlineData& aRedlineData
            = bIsInExtra &&
                      // still the same type (an inserted cell could become a tracked deleted one)
                      pRedline->GetRedlineData().GetType() == pRedline->GetRedlineData().GetType()
                  ? pTableCellRedline->GetRedlineData()
                  : pRedline->GetRedlineData();

        // Note: all redline ranges and table row redline (with the same author and timestamp)
        // use the same redline id in OOXML exported by MSO, but it seems, the recent solution
        // (different IDs for different ranges, also row changes) is also portable.
        OString aId(OString::number(m_nRedlineId++));
        const OUString& rAuthor(SW_MOD()->GetRedlineAuthor(aRedlineData.GetAuthor()));
        OString aAuthor(OUStringToOString(
            bRemovePersonalInfo ? "Author" + OUString::number(GetExport().GetInfoID(rAuthor))
                                : rAuthor,
            RTL_TEXTENCODING_UTF8));

        const DateTime aDateTime = aRedlineData.GetTimeStamp();
        bool bNoDate = bRemovePersonalInfo
                       || (aDateTime.GetYear() == 1970 && aDateTime.GetMonth() == 1
                           && aDateTime.GetDay() == 1);

        if (bNoDate)
            m_pSerializer->singleElementNS(
                XML_w, RedlineType::Delete == pRedline->GetType() ? XML_cellDel : XML_cellIns,
                FSNS(XML_w, XML_id), aId, FSNS(XML_w, XML_author), aAuthor);
        else
            m_pSerializer->singleElementNS(
                XML_w, RedlineType::Delete == pRedline->GetType() ? XML_cellDel : XML_cellIns,
                FSNS(XML_w, XML_id), aId, FSNS(XML_w, XML_author), aAuthor, FSNS(XML_w, XML_date),
                DateTimeToOString(aDateTime));
        return;
    }
}

void DocxAttributeOutput::TableHeight(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTabLine = pTabBox->GetUpper();
    const SwFrameFormat* pLineFormat = pTabLine->GetFrameFormat();

    const SwFormatFrameSize& rLSz = pLineFormat->GetFrameSize();
    if (!(SwFrameSize::Variable != rLSz.GetHeightSizeType() && rLSz.GetHeight()))
        return;

    sal_Int32 nHeight = rLSz.GetHeight();
    const char* pRule = nullptr;

    switch (rLSz.GetHeightSizeType())
    {
        case SwFrameSize::Fixed:
            pRule = "exact";
            break;
        case SwFrameSize::Minimum:
            pRule = "atLeast";
            break;
        default:
            break;
    }

    if (pRule)
        m_pSerializer->singleElementNS(XML_w, XML_trHeight, FSNS(XML_w, XML_val),
                                       OString::number(nHeight), FSNS(XML_w, XML_hRule), pRule);
}

void DocxAttributeOutput::TableCanSplit(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTabLine = pTabBox->GetUpper();
    const SwFrameFormat* pLineFormat = pTabLine->GetFrameFormat();

    const SwFormatRowSplit& rSplittable = pLineFormat->GetRowSplit();
    // if rSplittable is true then no need to write <w:cantSplit w:val="false"/>
    // as default row prop is allow row to break across page.
    if (!rSplittable.GetValue())
        m_pSerializer->singleElementNS(XML_w, XML_cantSplit, FSNS(XML_w, XML_val), "true");
}

void DocxAttributeOutput::TableBidi(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    const SwFrameFormat* pFrameFormat = pTable->GetFrameFormat();

    if (m_rExport.TrueFrameDirection(*pFrameFormat) == SvxFrameDirection::Horizontal_RL_TB)
    {
        m_pSerializer->singleElementNS(XML_w, XML_bidiVisual, FSNS(XML_w, XML_val), "true");
    }
}

void DocxAttributeOutput::TableVerticalCell(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat* pFrameFormat = pTabBox->GetFrameFormat();

    if (SvxFrameDirection::Vertical_RL_TB == m_rExport.TrueFrameDirection(*pFrameFormat))
        m_pSerializer->singleElementNS(XML_w, XML_textDirection, FSNS(XML_w, XML_val), "tbRl");
    else if (SvxFrameDirection::Vertical_LR_BT == m_rExport.TrueFrameDirection(*pFrameFormat))
    {
        m_pSerializer->singleElementNS(XML_w, XML_textDirection, FSNS(XML_w, XML_val), "btLr");
    }

    const SwWriteTableRows& rRows = m_xTableWrt->GetRows();
    const auto nRow = pTableTextNodeInfoInner->getRow();
    if (nRow >= rRows.size())
    {
        SAL_WARN("sw.ww8", "DocxAttributeOutput::TableCellProperties: out of range row: " << nRow);
        return;
    }
    SwWriteTableRow* pRow = rRows[nRow].get();
    sal_uInt32 nCell = pTableTextNodeInfoInner->getCell();
    const SwWriteTableCells& rTableCells = pRow->GetCells();
    if (nCell >= rTableCells.size())
        return;

    const SwWriteTableCell* const pCell = pRow->GetCells()[nCell].get();
    switch (pCell->GetVertOri())
    {
        case text::VertOrientation::TOP:
            break;
        case text::VertOrientation::CENTER:
            m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), "center");
            break;
        case text::VertOrientation::BOTTOM:
            m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), "bottom");
            break;
    }
}

void DocxAttributeOutput::TableNodeInfoInner(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pNodeInfoInner)
{
    // This is called when the nested table ends in a cell, and there's no
    // paragraph behind that; so we must check for the ends of cell, rows,
    // tables
    // ['true' to write an empty paragraph, MS Word insists on that]
    FinishTableRowCell(pNodeInfoInner, true);
}

void DocxAttributeOutput::TableOrientation(
    const ww8::WW8TableNodeInfoInner::Pointer_t& /*pTableTextNodeInfoInner*/)
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableOrientation( "
                       "ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )");
}

void DocxAttributeOutput::TableSpacing(
    const ww8::WW8TableNodeInfoInner::Pointer_t& /*pTableTextNodeInfoInner*/)
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableSpacing( "
                       "ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )");
}

void DocxAttributeOutput::TableRowEnd(sal_uInt32 /*nDepth*/)
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableRowEnd( sal_uInt32 nDepth = 1 )");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
