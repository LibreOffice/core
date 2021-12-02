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

#include <memory>
#include "docxattributeoutput.hxx"
#include "docxhelper.hxx"
#include "docxsdrexport.hxx"
#include "docxexportfilter.hxx"
#include "docxfootnotes.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <fchrfmt.hxx>
#include <tgrditem.hxx>
#include <fmtruby.hxx>
#include <fmtfollowtextflow.hxx>
#include <fmtanchr.hxx>
#include <breakit.hxx>
#include <redline.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <textboxhelper.hxx>
#include <rdfhelper.hxx>
#include "wrtww8.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <comphelper/string.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/sequence.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <oox/mathml/export.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/ole/olehelper.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/prntitem.hxx>
#include <sax/tools/converter.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/svdouno.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svl/grabbagitem.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/datetimeutils.hxx>
#include <tools/UnitConversion.hxx>
#include <svl/whiter.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <sot/exchange.hxx>

#include <docufld.hxx>
#include <authfld.hxx>
#include <flddropdown.hxx>
#include <fmtclds.hxx>
#include <fmtinfmt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtline.hxx>
#include <ftninfo.hxx>
#include <htmltbl.hxx>
#include <lineinfo.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <swmodule.hxx>
#include <swtable.hxx>
#include <txtftn.hxx>
#include <fmtautofmt.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <fmtclbl.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <grfatr.hxx>
#include <frmatr.hxx>
#include <txtatr.hxx>
#include <frameformats.hxx>

#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/file.hxx>
#include <utility>
#include <vcl/embeddedfontshelper.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <algorithm>
#include <cstddef>
#include <stdarg.h>
#include <string_view>

#include <toolkit/helper/vclunohelper.hxx>
#include <unicode/regex.h>

using ::editeng::SvxBorderLine;

using namespace oox;
using namespace docx;
using namespace sax_fastparser;
using namespace nsSwDocInfoSubType;
using namespace sw::util;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;

const sal_Int32 Tag_StartParagraph_1 = 1;
const sal_Int32 Tag_StartParagraph_2 = 2;
const sal_Int32 Tag_WriteSdtBlock = 3;
const sal_Int32 Tag_StartParagraphProperties = 4;
const sal_Int32 Tag_InitCollectedParagraphProperties = 5;
const sal_Int32 Tag_StartRun_1 = 6;
const sal_Int32 Tag_StartRun_2 = 7;
const sal_Int32 Tag_StartRun_3 = 8;
const sal_Int32 Tag_EndRun_1 = 9;
const sal_Int32 Tag_EndRun_2 = 10;
const sal_Int32 Tag_StartRunProperties = 11;
const sal_Int32 Tag_InitCollectedRunProperties = 12;
const sal_Int32 Tag_Redline_1 = 13;
const sal_Int32 Tag_Redline_2 = 14;
const sal_Int32 Tag_TableDefinition = 15;
const sal_Int32 Tag_OutputFlyFrame = 16;
const sal_Int32 Tag_StartSection = 17;

namespace {

class FFDataWriterHelper
{
    ::sax_fastparser::FSHelperPtr m_pSerializer;
    void writeCommonStart( const OUString& rName,
                           const OUString& rEntryMacro,
                           const OUString& rExitMacro,
                           const OUString& rHelp,
                           const OUString& rHint )
    {
        m_pSerializer->startElementNS(XML_w, XML_ffData);
        m_pSerializer->singleElementNS(XML_w, XML_name, FSNS(XML_w, XML_val), rName);
        m_pSerializer->singleElementNS(XML_w, XML_enabled);
        m_pSerializer->singleElementNS(XML_w, XML_calcOnExit, FSNS(XML_w, XML_val), "0");

        if ( !rEntryMacro.isEmpty() )
            m_pSerializer->singleElementNS( XML_w, XML_entryMacro,
                FSNS(XML_w, XML_val), rEntryMacro );

        if ( !rExitMacro.isEmpty() )
            m_pSerializer->singleElementNS(XML_w, XML_exitMacro, FSNS(XML_w, XML_val), rExitMacro);

        if ( !rHelp.isEmpty() )
            m_pSerializer->singleElementNS( XML_w, XML_helpText,
                FSNS(XML_w, XML_type), "text",
                FSNS(XML_w, XML_val), rHelp );

        if ( !rHint.isEmpty() )
            m_pSerializer->singleElementNS( XML_w, XML_statusText,
                FSNS(XML_w, XML_type), "text",
                FSNS(XML_w, XML_val), rHint );

    }
    void writeFinish()
    {
        m_pSerializer->endElementNS( XML_w, XML_ffData );
    }
public:
    explicit FFDataWriterHelper( ::sax_fastparser::FSHelperPtr  rSerializer ) : m_pSerializer(std::move( rSerializer )){}
    void WriteFormCheckbox( const OUString& rName,
                            const OUString& rEntryMacro,
                            const OUString& rExitMacro,
                            const OUString& rHelp,
                            const OUString& rHint,
                            bool bChecked )
    {
        writeCommonStart( rName, rEntryMacro, rExitMacro, rHelp, rHint );
        // Checkbox specific bits
        m_pSerializer->startElementNS(XML_w, XML_checkBox);
        // currently hardcoding autosize
        // #TODO check if this defaulted
        m_pSerializer->startElementNS(XML_w, XML_sizeAuto);
        m_pSerializer->endElementNS( XML_w, XML_sizeAuto );
        if ( bChecked )
            m_pSerializer->singleElementNS(XML_w, XML_checked);
        m_pSerializer->endElementNS( XML_w, XML_checkBox );
        writeFinish();
    }

    void WriteFormText(  const OUString& rName,
                         const OUString& rEntryMacro,
                         const OUString& rExitMacro,
                         const OUString& rHelp,
                         const OUString& rHint,
                         const OUString& rType,
                         const OUString& rDefaultText,
                         sal_uInt16 nMaxLength,
                         const OUString& rFormat )
    {
        writeCommonStart( rName, rEntryMacro, rExitMacro, rHelp, rHint );

        m_pSerializer->startElementNS(XML_w, XML_textInput);
        if ( !rType.isEmpty() )
            m_pSerializer->singleElementNS(XML_w, XML_type, FSNS(XML_w, XML_val), rType);
        if ( !rDefaultText.isEmpty() )
            m_pSerializer->singleElementNS(XML_w, XML_default, FSNS(XML_w, XML_val), rDefaultText);
        if ( nMaxLength )
            m_pSerializer->singleElementNS( XML_w, XML_maxLength,
                FSNS(XML_w, XML_val), OString::number(nMaxLength) );
        if ( !rFormat.isEmpty() )
            m_pSerializer->singleElementNS(XML_w, XML_format, FSNS(XML_w, XML_val), rFormat);
        m_pSerializer->endElementNS( XML_w, XML_textInput );

        writeFinish();
    }
};

class FieldMarkParamsHelper
{
    const sw::mark::IFieldmark& mrFieldmark;
    public:
    explicit FieldMarkParamsHelper( const sw::mark::IFieldmark& rFieldmark ) : mrFieldmark( rFieldmark ) {}
    OUString const & getName() const { return mrFieldmark.GetName(); }
    template < typename T >
    bool extractParam( const OUString& rKey, T& rResult )
    {
        bool bResult = false;
        if ( mrFieldmark.GetParameters() )
        {
            sw::mark::IFieldmark::parameter_map_t::const_iterator it = mrFieldmark.GetParameters()->find( rKey );
            if ( it != mrFieldmark.GetParameters()->end() )
                bResult = ( it->second >>= rResult );
        }
        return bResult;
    }
};

// [ISO/IEC29500-1:2016] 17.18.50 ST_LongHexNumber (Eight Digit Hexadecimal Value)
OUString NumberToHexBinary(sal_Int32 n)
{
    OUStringBuffer aBuf;
    sax::Converter::convertNumberToHexBinary(aBuf, n);
    return aBuf.makeStringAndClear();
}

}

void DocxAttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 /*nScript*/ )
{
    if (bIsRTL)
        m_pSerializer->singleElementNS(XML_w, XML_rtl, FSNS(XML_w, XML_val), "true");
}

/// Are multiple paragraphs disallowed inside this type of SDT?
static bool lcl_isOnelinerSdt(std::u16string_view rName)
{
    return rName == u"Title" || rName == u"Subtitle" || rName == u"Company";
}

// write a floating table directly to docx without the surrounding frame
void DocxAttributeOutput::WriteFloatingTable(ww8::Frame const* pParentFrame)
{
    const SwFrameFormat& rFrameFormat = pParentFrame->GetFrameFormat();
    m_aFloatingTablesOfParagraph.insert(&rFrameFormat);
    const SwNodeIndex* pNodeIndex = rFrameFormat.GetContent().GetContentIdx();

    SwNodeOffset nStt = pNodeIndex ? pNodeIndex->GetIndex() + 1 : SwNodeOffset(0);
    SwNodeOffset nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : SwNodeOffset(0);

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(GetExport(), nStt, nEnd, pParentFrame);

    // set a floatingTableFrame AND unset parent frame,
    // otherwise exporter thinks we are still in a frame
    m_rExport.SetFloatingTableFrame(pParentFrame);
    m_rExport.m_pParentFrame = nullptr;

    GetExport().WriteText();

    m_rExport.SetFloatingTableFrame(nullptr);
}

static void checkAndWriteFloatingTables(DocxAttributeOutput& rDocxAttributeOutput)
{
    const auto& rExport = rDocxAttributeOutput.GetExport();
    // iterate though all SpzFrameFormats and check whether they are anchored to the current text node
    for( sal_uInt16 nCnt = rExport.m_rDoc.GetSpzFrameFormats()->size(); nCnt; )
    {
        const SwFrameFormat* pFrameFormat = (*rExport.m_rDoc.GetSpzFrameFormats())[ --nCnt ];
        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        const SwPosition* pPosition = rAnchor.GetContentAnchor();

        if (!pPosition || ! rExport.m_pCurPam->GetNode().GetTextNode())
            continue;

        if (pPosition->nNode != rExport.m_pCurPam->GetNode().GetTextNode()->GetIndex())
            continue;

        const SwNodeIndex* pStartNode = pFrameFormat->GetContent().GetContentIdx();
        if (!pStartNode)
            continue;

        SwNodeIndex aStartNode = *pStartNode;

        // go to the next node (actual content)
        ++aStartNode;

        // this has to be a table
        if (!aStartNode.GetNode().IsTableNode())
            continue;

        // go to the end of the table
        SwNodeOffset aEndIndex = aStartNode.GetNode().EndOfSectionIndex();
        // go one deeper
        aEndIndex++;
        // this has to be the end of the content
        if (aEndIndex != pFrameFormat->GetContent().GetContentIdx()->GetNode().EndOfSectionIndex())
            continue;

        // check for a grabBag and "TablePosition" attribute -> then we can export the table directly
        SwTableNode* pTableNode = aStartNode.GetNode().GetTableNode();
        SwTable& rTable = pTableNode->GetTable();
        SwFrameFormat* pTableFormat = rTable.GetFrameFormat();
        const SfxGrabBagItem* pTableGrabBag = pTableFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG);
        std::map<OUString, css::uno::Any> aTableGrabBag = pTableGrabBag->GetGrabBag();
        // no grabbag?
        if (aTableGrabBag.find("TablePosition") == aTableGrabBag.end())
            continue;

        // write table to docx
        ww8::Frame aFrame(*pFrameFormat,*pPosition);
        rDocxAttributeOutput.WriteFloatingTable(&aFrame);
    }
}

sal_Int32 DocxAttributeOutput::StartParagraph(ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo,
                                              bool bGenerateParaId)
{
    // Paragraphs (in headers/footers/comments/frames etc) can start before another finishes.
    // So a stack is needed to keep track of each paragraph's status separately.
    // Complication: Word can't handle nested text boxes, so those need to be collected together.
    if ( !m_aFramesOfParagraph.size() || !m_nTextFrameLevel )
        m_aFramesOfParagraph.push(std::vector<ww8::Frame>());

    // look ahead for floating tables that were put into a frame during import
    // floating tables in shapes are not supported: exclude this case
    if (!pTextNodeInfo && !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen())
    {
        checkAndWriteFloatingTables(*this);
    }

    if ( m_nColBreakStatus == COLBRK_POSTPONE )
        m_nColBreakStatus = COLBRK_WRITE;

    // Output table/table row/table cell starts if needed
    if ( pTextNodeInfo )
    {
        // New cell/row?
        if ( m_tableReference->m_nTableDepth > 0 && !m_tableReference->m_bTableCellOpen )
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pDeepInner( pTextNodeInfo->getInnerForDepth( m_tableReference->m_nTableDepth ) );
            if ( pDeepInner->getCell() == 0 )
                StartTableRow( pDeepInner );

            const sal_uInt32 nCell = pDeepInner->getCell();
            const sal_uInt32 nRow = pDeepInner->getRow();

            SyncNodelessCells(pDeepInner, nCell, nRow);
            StartTableCell(pDeepInner, nCell, nRow);
        }

        sal_uInt32 nRow = pTextNodeInfo->getRow();
        sal_uInt32 nCell = pTextNodeInfo->getCell();
        if (nCell == 0)
        {
            // Do we have to start the table?
            // [If we are at the right depth already, it means that we
            // continue the table cell]
            sal_uInt32 nCurrentDepth = pTextNodeInfo->getDepth();

            if ( nCurrentDepth > m_tableReference->m_nTableDepth )
            {
                // Start all the tables that begin here
                for ( sal_uInt32 nDepth = m_tableReference->m_nTableDepth + 1; nDepth <= nCurrentDepth; ++nDepth )
                {
                    ww8::WW8TableNodeInfoInner::Pointer_t pInner( pTextNodeInfo->getInnerForDepth( nDepth ) );

                    StartTable( pInner );
                    StartTableRow( pInner );

                    StartTableCell(pInner, 0, nDepth == nCurrentDepth ? nRow : 0);
                }

                m_tableReference->m_nTableDepth = nCurrentDepth;
            }
        }
    }

    // Look up the "sdt end before this paragraph" property early, when it
    // would normally arrive, it would be too late (would be after the
    // paragraph start has been written).
    bool bEndParaSdt = false;
    if (m_bStartedParaSdt)
    {
        SwTextNode* pTextNode = m_rExport.m_pCurPam->GetNode().GetTextNode();
        if (pTextNode && pTextNode->GetpSwAttrSet())
        {
            const SfxItemSet* pSet = pTextNode->GetpSwAttrSet();
            if (const SfxPoolItem* pItem = pSet->GetItem(RES_PARATR_GRABBAG))
            {
                const SfxGrabBagItem& rParaGrabBag = static_cast<const SfxGrabBagItem&>(*pItem);
                const std::map<OUString, css::uno::Any>& rMap = rParaGrabBag.GetGrabBag();
                bEndParaSdt = m_bStartedParaSdt && rMap.find("ParaSdtEndBefore") != rMap.end();
            }
        }
    }
    // TODO also avoid multiline paragraphs in those SDT types for shape text
    bool bOneliner = m_bStartedParaSdt && !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen() && lcl_isOnelinerSdt(m_aStartedParagraphSdtPrAlias);
    if (bEndParaSdt || (m_bStartedParaSdt && m_bHadSectPr) || bOneliner)
    {
        // This is the common case: "close sdt before the current paragraph" was requested by the next paragraph.
        EndSdtBlock();
        m_bStartedParaSdt = false;
        m_aStartedParagraphSdtPrAlias.clear();
    }
    m_bHadSectPr = false;

    // this mark is used to be able to enclose the paragraph inside a sdr tag.
    // We will only know if we have to do that later.
    m_pSerializer->mark(Tag_StartParagraph_1);

    std::optional<OUString> aParaId;
    sal_Int32 nParaId = 0;
    if (bGenerateParaId)
    {
        nParaId = m_nNextParaId++;
        aParaId = NumberToHexBinary(nParaId);
    }
    m_pSerializer->startElementNS(XML_w, XML_p, FSNS(XML_w14, XML_paraId), aParaId);

    // postpone the output of the run (we get it before the paragraph
    // properties, but must write it after them)
    m_pSerializer->mark(Tag_StartParagraph_2);

    // no section break in this paragraph yet; can be set in SectionBreak()
    m_pSectionInfo.reset();

    m_bParagraphOpened = true;
    m_bIsFirstParagraph = false;

    return nParaId;
}

static OString convertToOOXMLVertOrient(sal_Int16 nOrient)
{
    switch( nOrient )
    {
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            return "center";
        case text::VertOrientation::BOTTOM:
            return "bottom";
        case text::VertOrientation::LINE_BOTTOM:
            return "outside";
        case text::VertOrientation::TOP:
            return "top";
        case text::VertOrientation::LINE_TOP:
            return "inside";
        default:
            return OString();
    }
}

static OString convertToOOXMLHoriOrient(sal_Int16 nOrient, bool bIsPosToggle)
{
    switch( nOrient )
    {
        case text::HoriOrientation::LEFT:
            return bIsPosToggle ? "inside" : "left";
        case text::HoriOrientation::INSIDE:
            return "inside";
        case text::HoriOrientation::RIGHT:
            return bIsPosToggle ? "outside" : "right";
        case text::HoriOrientation::OUTSIDE:
            return "outside";
        case text::HoriOrientation::CENTER:
        case text::HoriOrientation::FULL:
            return "center";
        default:
            return OString();
    }
}

static OString convertToOOXMLVertOrientRel(sal_Int16 nOrientRel)
{
    switch (nOrientRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            return "margin";
        case text::RelOrientation::PAGE_FRAME:
            return "page";
        case text::RelOrientation::FRAME:
        case text::RelOrientation::TEXT_LINE:
        default:
            return "text";
    }
}

static OString convertToOOXMLHoriOrientRel(sal_Int16 nOrientRel)
{
    switch (nOrientRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            return "margin";
        case text::RelOrientation::PAGE_FRAME:
            return "page";
        case text::RelOrientation::CHAR:
        case text::RelOrientation::PAGE_RIGHT:
        case text::RelOrientation::FRAME:
        default:
            return "text";
    }
}

static void lcl_deleteAndResetTheLists(
    rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenChildren,
    rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrDataBindingAttrs,
    rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTextAttrs,
    OUString& rSdtPrAlias, OUString& rSdtPrPlaceholderDocPart, OUString& rColor)
{
    if( pSdtPrTokenChildren.is() )
        pSdtPrTokenChildren.clear();
    if( pSdtPrDataBindingAttrs.is() )
        pSdtPrDataBindingAttrs.clear();
    if (pSdtPrTextAttrs.is())
        pSdtPrTextAttrs.clear();
    if (!rSdtPrAlias.isEmpty())
        rSdtPrAlias.clear();
    if (!rSdtPrPlaceholderDocPart.isEmpty())
        rSdtPrPlaceholderDocPart.clear();
    if (!rColor.isEmpty())
        rColor.clear();
}

void DocxAttributeOutput::PopulateFrameProperties(const SwFrameFormat* pFrameFormat, const Size& rSize)
{
    rtl::Reference<sax_fastparser::FastAttributeList> attrList = FastSerializerHelper::createAttrList();

    awt::Point aPos(pFrameFormat->GetHoriOrient().GetPos(), pFrameFormat->GetVertOrient().GetPos());

    attrList->add( FSNS( XML_w, XML_w), OString::number(rSize.Width()));
    attrList->add( FSNS( XML_w, XML_h), OString::number(rSize.Height()));

    attrList->add( FSNS( XML_w, XML_x), OString::number(aPos.X));
    attrList->add( FSNS( XML_w, XML_y), OString::number(aPos.Y));

    sal_Int16 nLeft = pFrameFormat->GetLRSpace().GetLeft();
    sal_Int16 nRight = pFrameFormat->GetLRSpace().GetRight();
    sal_Int16 nUpper = pFrameFormat->GetULSpace().GetUpper();
    sal_Int16 nLower = pFrameFormat->GetULSpace().GetLower();

    attrList->add(FSNS(XML_w, XML_hSpace), OString::number((nLeft + nRight) / 2));
    attrList->add(FSNS(XML_w, XML_vSpace), OString::number((nUpper + nLower) / 2));

    OString relativeFromH = convertToOOXMLHoriOrientRel( pFrameFormat->GetHoriOrient().GetRelationOrient() );
    OString relativeFromV = convertToOOXMLVertOrientRel( pFrameFormat->GetVertOrient().GetRelationOrient() );

    switch (pFrameFormat->GetSurround().GetValue())
    {
    case css::text::WrapTextMode_NONE:
        attrList->add( FSNS( XML_w, XML_wrap), "notBeside");
        break;
    case css::text::WrapTextMode_DYNAMIC:
        attrList->add(FSNS(XML_w, XML_wrap), "auto");
        break;
    case css::text::WrapTextMode_PARALLEL:
    default:
        attrList->add(FSNS(XML_w, XML_wrap), "around");
        break;
    }
    attrList->add( FSNS( XML_w, XML_vAnchor), relativeFromV );
    attrList->add( FSNS( XML_w, XML_hAnchor), relativeFromH );
    attrList->add( FSNS( XML_w, XML_hRule), "exact");

    m_pSerializer->singleElementNS( XML_w, XML_framePr, attrList );
}

bool DocxAttributeOutput::TextBoxIsFramePr(const SwFrameFormat& rFrameFormat)
{
    uno::Reference< drawing::XShape > xShape;
    const SdrObject* pSdrObj = rFrameFormat.FindRealSdrObject();
    if (pSdrObj)
        xShape.set(const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
    if (xPropertySet.is())
        xPropSetInfo = xPropertySet->getPropertySetInfo();
    uno::Any aFrameProperties ;
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("FrameInteropGrabBag"))
    {
        uno::Sequence< beans::PropertyValue > propList;
        xPropertySet->getPropertyValue("FrameInteropGrabBag") >>= propList;
        auto pProp = std::find_if(std::cbegin(propList), std::cend(propList),
            [](const beans::PropertyValue& rProp) { return rProp.Name == "ParaFrameProperties"; });
        if (pProp != std::cend(propList))
            aFrameProperties = pProp->Value;
    }
    bool bFrameProperties = false;
    aFrameProperties >>= bFrameProperties;
    return bFrameProperties;
}

void DocxAttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    // write the paragraph properties + the run, already in the correct order
    m_pSerializer->mergeTopMarks(Tag_StartParagraph_2);
    std::vector<  std::shared_ptr <ww8::Frame> > aFramePrTextbox;
    // Write the anchored frame if any
    // Word can't handle nested text boxes, so write them on the same level.
    ++m_nTextFrameLevel;
    if( m_nTextFrameLevel == 1 && !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen() )
    {
        comphelper::FlagRestorationGuard aStartedParaSdtGuard(m_bStartedParaSdt, false);

        assert(!m_pPostponedCustomShape);
        m_pPostponedCustomShape.reset(new std::vector<PostponedDrawing>);

        // The for loop can change the size of m_aFramesOfParagraph, so the max size cannot be set in stone before the loop.
        size_t nFrames = m_aFramesOfParagraph.size() ? m_aFramesOfParagraph.top().size() : 0;
        for (size_t nIndex = 0; nIndex < nFrames; ++nIndex)
        {
            m_bParagraphFrameOpen = true;
            ww8::Frame aFrame = m_aFramesOfParagraph.top()[nIndex];
            const SwFrameFormat& rFrameFormat = aFrame.GetFrameFormat();

            if (!TextBoxIsFramePr(rFrameFormat) || m_bWritingHeaderFooter)
            {
                if (m_bStartedCharSdt)
                {
                    // Run-level SDT still open? Close it before AlternateContent.
                    EndSdtBlock();
                    m_bStartedCharSdt = false;
                }
                m_pSerializer->startElementNS(XML_w, XML_r);
                m_pSerializer->startElementNS(XML_mc, XML_AlternateContent);
                m_pSerializer->startElementNS(XML_mc, XML_Choice, XML_Requires, "wps");
                /**
                    This is to avoid AlternateContent within another AlternateContent.
                       So when Choice is Open, only write the DML Drawing instead of both DML
                       and VML Drawing in another AlternateContent.
                 **/
                SetAlternateContentChoiceOpen( true );
                /** Save the table info's before writing the shape
                        as there might be a new table that might get
                        spawned from within the VML & DML block and alter
                        the contents.
                */
                ww8::WW8TableInfo::Pointer_t pOldTableInfo = m_rExport.m_pTableInfo;
                //Reset the table infos after saving.
                m_rExport.m_pTableInfo = std::make_shared<ww8::WW8TableInfo>();

                /** FDO#71834 :
                       Save the table reference attributes before calling WriteDMLTextFrame,
                       otherwise the StartParagraph function will use the previous existing
                       table reference attributes since the variable is being shared.
                */
                {
                    DocxTableExportContext aDMLTableExportContext(*this);
                    m_rExport.SdrExporter().writeDMLTextFrame(&aFrame, m_anchorId++);
                }
                m_pSerializer->endElementNS(XML_mc, XML_Choice);
                SetAlternateContentChoiceOpen( false );

                // Reset table infos, otherwise the depth of the cells will be incorrect,
                // in case the text frame had table(s) and we try to export the
                // same table second time.
                m_rExport.m_pTableInfo = std::make_shared<ww8::WW8TableInfo>();
                //reset the tableReference.

                m_pSerializer->startElementNS(XML_mc, XML_Fallback);
                {
                    DocxTableExportContext aVMLTableExportContext(*this);
                    m_rExport.SdrExporter().writeVMLTextFrame(&aFrame);
                }
                m_rExport.m_pTableInfo = pOldTableInfo;

                m_pSerializer->endElementNS(XML_mc, XML_Fallback);
                m_pSerializer->endElementNS(XML_mc, XML_AlternateContent);
                m_pSerializer->endElementNS( XML_w, XML_r );
                m_bParagraphFrameOpen = false;
            }
            else
            {
                std::shared_ptr<ww8::Frame> pFramePr = std::make_shared<ww8::Frame>(aFrame);
                aFramePrTextbox.push_back(pFramePr);
            }

            nFrames = m_aFramesOfParagraph.size() ? m_aFramesOfParagraph.top().size() : 0;
        }
        if (!m_pPostponedCustomShape->empty())
        {
            m_pSerializer->startElementNS(XML_w, XML_r);
            WritePostponedCustomShape();
            m_pSerializer->endElementNS( XML_w, XML_r );
        }
        m_pPostponedCustomShape.reset();

        if ( m_aFramesOfParagraph.size() )
            m_aFramesOfParagraph.top().clear();

        if (!pTextNodeInfoInner)
        {
            // Ending a non-table paragraph, clear floating tables before paragraph.
            m_aFloatingTablesOfParagraph.clear();
        }
    }

    --m_nTextFrameLevel;
    if ( m_aFramesOfParagraph.size() && !m_nTextFrameLevel )
        m_aFramesOfParagraph.pop();

    /* If m_nHyperLinkCount > 0 that means hyperlink tag is not yet closed.
     * This is due to nested hyperlink tags. So close it before end of paragraph.
     */
    if(m_nHyperLinkCount > 0)
    {
        for(sal_Int32 nHyperLinkToClose = 0; nHyperLinkToClose < m_nHyperLinkCount; ++nHyperLinkToClose)
            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
        m_nHyperLinkCount = 0;
    }

    if (m_bStartedCharSdt)
    {
        // Run-level SDT still open? Close it now.
        EndSdtBlock();
        m_bStartedCharSdt = false;
    }

    if (m_bPageBreakAfter)
    {
        // tdf#128889 Trailing page break
        SectionBreak(msword::PageBreak, false);
        m_bPageBreakAfter = false;
    }

    m_pSerializer->endElementNS( XML_w, XML_p );
    // on export sdt blocks are never nested ATM
    if( !m_bAnchorLinkedToNode && !m_bStartedParaSdt )
        WriteSdtBlock(m_nParagraphSdtPrToken, m_pParagraphSdtPrTokenChildren,
                      m_pParagraphSdtPrTokenAttributes, m_pParagraphSdtPrDataBindingAttrs,
                      m_pParagraphSdtPrTextAttrs, m_aParagraphSdtPrAlias,
                      m_aParagraphSdtPrPlaceHolderDocPart, m_aParagraphSdtPrColor, /*bPara=*/true);
    else
    {
        //These should be written out to the actual Node and not to the anchor.
        //Clear them as they will be repopulated when the node is processed.
        m_nParagraphSdtPrToken = 0;
        m_bParagraphSdtHasId = false;
        lcl_deleteAndResetTheLists(m_pParagraphSdtPrTokenChildren,
                                   m_pParagraphSdtPrDataBindingAttrs, m_pParagraphSdtPrTextAttrs,
                                   m_aParagraphSdtPrAlias, m_aParagraphSdtPrPlaceHolderDocPart,
                                   m_aParagraphSdtPrColor);
    }

    m_pSerializer->mark(Tag_StartParagraph_2);

    // Write framePr
    for ( const auto & pFrame : aFramePrTextbox )
    {
        DocxTableExportContext aTableExportContext(*this);
        m_pCurrentFrame = pFrame.get();
        m_rExport.SdrExporter().writeOnlyTextOfFrame(pFrame.get());
        m_pCurrentFrame = nullptr;
    }

    m_pSerializer->mergeTopMarks(Tag_StartParagraph_2, sax_fastparser::MergeMarks::PREPEND);

    //sdtcontent is written so Set m_bParagraphHasDrawing to false
    m_rExport.SdrExporter().setParagraphHasDrawing(false);
    m_bRunTextIsOn = false;
    m_pSerializer->mergeTopMarks(Tag_StartParagraph_1);

    aFramePrTextbox.clear();
    // Check for end of cell, rows, tables here
    FinishTableRowCell( pTextNodeInfoInner );

    if( !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen() )
        m_bParagraphOpened = false;

    // Clear bookmarks of the current paragraph
    m_aBookmarksOfParagraphStart.clear();
    m_aBookmarksOfParagraphEnd.clear();
}

void DocxAttributeOutput::WriteSdtBlock( sal_Int32& nSdtPrToken,
                                         rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenChildren,
                                         rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenAttributes,
                                         rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrDataBindingAttrs,
                                         rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTextAttrs,
                                         OUString& rSdtPrAlias,
                                         OUString& rSdtPrPlaceholderDocPart,
                                         OUString& rColor,
                                         bool bPara )
{
    if( nSdtPrToken <= 0 && !pSdtPrDataBindingAttrs.is() && !m_bParagraphSdtHasId)
        return;

    // sdt start mark
    m_pSerializer->mark(Tag_WriteSdtBlock);

    m_pSerializer->startElementNS(XML_w, XML_sdt);

    // output sdt properties
    m_pSerializer->startElementNS(XML_w, XML_sdtPr);

    if( nSdtPrToken > 0 && pSdtPrTokenChildren.is() )
    {
        if (!pSdtPrTokenAttributes.is())
            m_pSerializer->startElement(nSdtPrToken);
        else
        {
            rtl::Reference<FastAttributeList> xAttrList = std::move(pSdtPrTokenAttributes);
            m_pSerializer->startElement(nSdtPrToken, xAttrList);
        }

        if (nSdtPrToken ==  FSNS( XML_w, XML_date ) || nSdtPrToken ==  FSNS( XML_w, XML_docPartObj ) || nSdtPrToken ==  FSNS( XML_w, XML_docPartList ) || nSdtPrToken ==  FSNS( XML_w14, XML_checkbox )) {
            const uno::Sequence<xml::FastAttribute> aChildren = pSdtPrTokenChildren->getFastAttributes();
            for( const auto& rChild : aChildren )
                m_pSerializer->singleElement(rChild.Token, FSNS(XML_w, XML_val), rChild.Value);
        }

        m_pSerializer->endElement( nSdtPrToken );
    }
    else if( (nSdtPrToken > 0) && nSdtPrToken != FSNS( XML_w, XML_id ) && !(m_bRunTextIsOn && m_rExport.SdrExporter().IsParagraphHasDrawing()))
    {
        if (!pSdtPrTokenAttributes.is())
            m_pSerializer->singleElement(nSdtPrToken);
        else
        {
            rtl::Reference<FastAttributeList> xAttrList = std::move(pSdtPrTokenAttributes);
            m_pSerializer->singleElement(nSdtPrToken, xAttrList);
        }
    }

    if( nSdtPrToken == FSNS( XML_w, XML_id ) || ( bPara && m_bParagraphSdtHasId ) )
        //Word won't open a document with an empty id tag, we fill it with a random number
        m_pSerializer->singleElementNS(XML_w, XML_id, FSNS(XML_w, XML_val),
                                      OString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max())));

    if( pSdtPrDataBindingAttrs.is() && !m_rExport.SdrExporter().IsParagraphHasDrawing())
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( pSdtPrDataBindingAttrs );
        m_pSerializer->singleElementNS(XML_w, XML_dataBinding, xAttrList);
    }

    if (pSdtPrTextAttrs.is())
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move(pSdtPrTextAttrs);
        m_pSerializer->singleElementNS(XML_w, XML_text, xAttrList);
    }

    if (!rSdtPrPlaceholderDocPart.isEmpty())
    {
        m_pSerializer->startElementNS(XML_w, XML_placeholder);
        m_pSerializer->singleElementNS(XML_w, XML_docPart, FSNS(XML_w, XML_val), rSdtPrPlaceholderDocPart);
        m_pSerializer->endElementNS(XML_w, XML_placeholder);
    }
    if (!rColor.isEmpty())
    {
        m_pSerializer->singleElementNS(XML_w15, XML_color, FSNS(XML_w, XML_val), rColor);
    }

    if (!rSdtPrAlias.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_alias, FSNS(XML_w, XML_val), rSdtPrAlias);

    m_pSerializer->endElementNS( XML_w, XML_sdtPr );

    // sdt contents start tag
    m_pSerializer->startElementNS(XML_w, XML_sdtContent);

    // prepend the tags since the sdt start mark before the paragraph
    m_pSerializer->mergeTopMarks(Tag_WriteSdtBlock, sax_fastparser::MergeMarks::PREPEND);

    // write the ending tags after the paragraph
    if (bPara)
    {
        m_bStartedParaSdt = true;
        if (m_tableReference->m_bTableCellOpen)
            m_tableReference->m_bTableCellParaSdtOpen = true;
        if (m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen())
            m_rExport.SdrExporter().setParagraphSdtOpen(true);
    }
    else
        // Support multiple runs inside a run-level SDT: don't close the SDT block yet.
        m_bStartedCharSdt = true;

    // clear sdt status
    nSdtPrToken = 0;
    pSdtPrTokenChildren.clear();
    pSdtPrDataBindingAttrs.clear();
    pSdtPrTextAttrs.clear();
    rSdtPrAlias.clear();
    m_bParagraphSdtHasId = false;
}

void DocxAttributeOutput::EndSdtBlock()
{
    m_pSerializer->endElementNS( XML_w, XML_sdtContent );
    m_pSerializer->endElementNS( XML_w, XML_sdt );
}

#define MAX_CELL_IN_WORD 62

void DocxAttributeOutput::SyncNodelessCells(ww8::WW8TableNodeInfoInner::Pointer_t const & pInner, sal_Int32 nCell, sal_uInt32 nRow)
{
    sal_Int32 nOpenCell = lastOpenCell.back();
    if (nOpenCell != -1 && nOpenCell != nCell && nOpenCell < MAX_CELL_IN_WORD)
        EndTableCell(nOpenCell);

    sal_Int32 nClosedCell = lastClosedCell.back();
    for (sal_Int32 i = nClosedCell+1; i < nCell; ++i)
    {
        if (i >= MAX_CELL_IN_WORD)
            break;

        if (i == 0)
            StartTableRow(pInner);

        StartTableCell(pInner, i, nRow);
        m_pSerializer->singleElementNS(XML_w, XML_p);
        EndTableCell(i);
    }
}

void DocxAttributeOutput::FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t const & pInner, bool bForceEmptyParagraph )
{
    if ( !pInner )
        return;

    // Where are we in the table
    sal_uInt32 nRow = pInner->getRow();
    sal_Int32 nCell = pInner->getCell();

    InitTableHelper( pInner );

    // HACK
    // msoffice seems to have an internal limitation of 63 columns for tables
    // and refuses to load .docx with more, even though the spec seems to allow that;
    // so simply if there are more columns, don't close the last one msoffice will handle
    // and merge the contents of the remaining ones into it (since we don't close the cell
    // here, following ones will not be opened)
    const bool limitWorkaround = (nCell >= MAX_CELL_IN_WORD && !pInner->isEndOfLine());
    const bool bEndCell = pInner->isEndOfCell() && !limitWorkaround;
    const bool bEndRow = pInner->isEndOfLine();

    if (bEndCell)
    {
        while (pInner->getDepth() < m_tableReference->m_nTableDepth)
        {
            //we expect that the higher depth row was closed, and
            //we are just missing the table close
            assert(lastOpenCell.back() == -1 && lastClosedCell.back() == -1);
            EndTable();
        }

        SyncNodelessCells(pInner, nCell, nRow);

        sal_Int32 nClosedCell = lastClosedCell.back();
        if (nCell == nClosedCell)
        {
            //Start missing trailing cell(s)
            ++nCell;
            StartTableCell(pInner, nCell, nRow);

            //Continue on missing next trailing cell(s)
            ww8::RowSpansPtr xRowSpans = pInner->getRowSpansOfRow();
            sal_Int32 nRemainingCells = xRowSpans->size() - nCell;
            for (sal_Int32 i = 1; i < nRemainingCells; ++i)
            {
                if (bForceEmptyParagraph)
                {
                    m_pSerializer->singleElementNS(XML_w, XML_p);
                }

                EndTableCell(nCell);

                StartTableCell(pInner, nCell, nRow);
            }
        }

        if (bForceEmptyParagraph)
        {
            m_pSerializer->singleElementNS(XML_w, XML_p);
        }

        EndTableCell(nCell);
    }

    // This is a line end
    if (bEndRow)
        EndTableRow();

    // This is the end of the table
    if (pInner->isFinalEndOfLine())
        EndTable();
}

void DocxAttributeOutput::EmptyParagraph()
{
    m_pSerializer->singleElementNS(XML_w, XML_p);
}

void DocxAttributeOutput::SectionBreaks(const SwNode& rNode)
{
    // output page/section breaks
    // Writer can have them at the beginning of a paragraph, or at the end, but
    // in docx, we have to output them in the paragraph properties of the last
    // paragraph in a section.  To get it right, we have to switch to the next
    // paragraph, and detect the section breaks there.
    SwNodeIndex aNextIndex( rNode, 1 );

    if (rNode.IsTextNode() || rNode.IsSectionNode())
    {
        if (aNextIndex.GetNode().IsTextNode())
        {
            const SwTextNode* pTextNode = static_cast<SwTextNode*>(&aNextIndex.GetNode());
            m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode, m_tableReference->m_bTableCellOpen);
        }
        else if (aNextIndex.GetNode().IsTableNode())
        {
            const SwTableNode* pTableNode = static_cast<SwTableNode*>(&aNextIndex.GetNode());
            const SwFrameFormat *pFormat = pTableNode->GetTable().GetFrameFormat();
            m_rExport.OutputSectionBreaks(&(pFormat->GetAttrSet()), *pTableNode);
        }
    }
    else if (rNode.IsEndNode())
    {
        if (aNextIndex.GetNode().IsTextNode())
        {
            // Handle section break between a table and a text node following it.
            // Also handle section endings
            const SwTextNode* pTextNode = aNextIndex.GetNode().GetTextNode();
            if (rNode.StartOfSectionNode()->IsTableNode() || rNode.StartOfSectionNode()->IsSectionNode())
                m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode, m_tableReference->m_bTableCellOpen);
        }
        else if (aNextIndex.GetNode().IsTableNode())
        {
            // Handle section break between tables.
            const SwTableNode* pTableNode = static_cast<SwTableNode*>(&aNextIndex.GetNode());
            const SwFrameFormat *pFormat = pTableNode->GetTable().GetFrameFormat();
            m_rExport.OutputSectionBreaks(&(pFormat->GetAttrSet()), *pTableNode);
        }
    }
}

void DocxAttributeOutput::StartParagraphProperties()
{
    m_pSerializer->mark(Tag_StartParagraphProperties);

    m_pSerializer->startElementNS(XML_w, XML_pPr);

    // and output the section break now (if it appeared)
    if (m_pSectionInfo && m_rExport.m_nTextTyp == TXT_MAINTEXT)
    {
        m_rExport.SectionProperties( *m_pSectionInfo );
        m_pSectionInfo.reset();
    }

    InitCollectedParagraphProperties();
}

void DocxAttributeOutput::InitCollectedParagraphProperties()
{
    m_pParagraphSpacingAttrList.clear();

    // Write the elements in the spec order
    static const sal_Int32 aOrder[] =
    {
        FSNS( XML_w, XML_pStyle ),
        FSNS( XML_w, XML_keepNext ),
        FSNS( XML_w, XML_keepLines ),
        FSNS( XML_w, XML_pageBreakBefore ),
        FSNS( XML_w, XML_framePr ),
        FSNS( XML_w, XML_widowControl ),
        FSNS( XML_w, XML_numPr ),
        FSNS( XML_w, XML_suppressLineNumbers ),
        FSNS( XML_w, XML_pBdr ),
        FSNS( XML_w, XML_shd ),
        FSNS( XML_w, XML_tabs ),
        FSNS( XML_w, XML_suppressAutoHyphens ),
        FSNS( XML_w, XML_kinsoku ),
        FSNS( XML_w, XML_wordWrap ),
        FSNS( XML_w, XML_overflowPunct ),
        FSNS( XML_w, XML_topLinePunct ),
        FSNS( XML_w, XML_autoSpaceDE ),
        FSNS( XML_w, XML_autoSpaceDN ),
        FSNS( XML_w, XML_bidi ),
        FSNS( XML_w, XML_adjustRightInd ),
        FSNS( XML_w, XML_snapToGrid ),
        FSNS( XML_w, XML_spacing ),
        FSNS( XML_w, XML_ind ),
        FSNS( XML_w, XML_contextualSpacing ),
        FSNS( XML_w, XML_mirrorIndents ),
        FSNS( XML_w, XML_suppressOverlap ),
        FSNS( XML_w, XML_jc ),
        FSNS( XML_w, XML_textDirection ),
        FSNS( XML_w, XML_textAlignment ),
        FSNS( XML_w, XML_textboxTightWrap ),
        FSNS( XML_w, XML_outlineLvl ),
        FSNS( XML_w, XML_divId ),
        FSNS( XML_w, XML_cnfStyle ),
        FSNS( XML_w, XML_rPr ),
        FSNS( XML_w, XML_sectPr ),
        FSNS( XML_w, XML_pPrChange )
    };

    // postpone the output so that we can later [in EndParagraphProperties()]
    // prepend the properties before the run
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    m_pSerializer->mark(Tag_InitCollectedParagraphProperties, comphelper::containerToSequence(aOrder));
}

void DocxAttributeOutput::WriteCollectedParagraphProperties()
{
    if ( m_rExport.SdrExporter().getFlyAttrList().is() )
    {
        rtl::Reference<FastAttributeList> xAttrList( m_rExport.SdrExporter().getFlyAttrList() );
        m_rExport.SdrExporter().getFlyAttrList().clear();

        m_pSerializer->singleElementNS( XML_w, XML_framePr, xAttrList );
    }

    if ( m_pParagraphSpacingAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pParagraphSpacingAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_spacing, xAttrList );
    }

    if ( m_pBackgroundAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pBackgroundAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_shd, xAttrList );
    }
}

namespace
{

/// Outputs an item set, that contains the formatting of the paragraph marker.
void lcl_writeParagraphMarkerProperties(DocxAttributeOutput& rAttributeOutput, const SfxItemSet& rParagraphMarkerProperties)
{
    const SfxItemSet* pOldI = rAttributeOutput.GetExport().GetCurItemSet();
    rAttributeOutput.GetExport().SetCurItemSet(&rParagraphMarkerProperties);

    SfxWhichIter aIter(rParagraphMarkerProperties);
    sal_uInt16 nWhichId = aIter.FirstWhich();
    const SfxPoolItem* pItem = nullptr;
    // Did we already produce a <w:sz> element?
    bool bFontSizeWritten = false;
    while (nWhichId)
    {
        if (rParagraphMarkerProperties.GetItemState(nWhichId, true, &pItem) == SfxItemState::SET)
        {
            if (isCHRATR(nWhichId) || nWhichId == RES_TXTATR_CHARFMT)
            {
                // Will this item produce a <w:sz> element?
                bool bFontSizeItem = nWhichId == RES_CHRATR_FONTSIZE || nWhichId == RES_CHRATR_CJK_FONTSIZE;
                if (!bFontSizeWritten || !bFontSizeItem)
                    rAttributeOutput.OutputItem(*pItem);
                if (bFontSizeItem)
                    bFontSizeWritten = true;
            }
            else if (nWhichId == RES_TXTATR_AUTOFMT)
            {
                const SwFormatAutoFormat* pAutoFormat = static_cast<const SwFormatAutoFormat*>(pItem);
                lcl_writeParagraphMarkerProperties(rAttributeOutput, *pAutoFormat->GetStyleHandle());
            }
        }
        nWhichId = aIter.NextWhich();
    }
    rAttributeOutput.GetExport().SetCurItemSet(pOldI);
}

const char *RubyAlignValues[] =
{
    "center",
    "distributeLetter",
    "distributeSpace",
    "left",
    "right",
    "rightVertical"
};


const char *lclConvertWW8JCToOOXMLRubyAlign(sal_Int32 nJC)
{
    const sal_Int32 nElements = SAL_N_ELEMENTS(RubyAlignValues);
    if ( nJC >=0 && nJC < nElements )
        return RubyAlignValues[nJC];
    return RubyAlignValues[0];
}

}

void DocxAttributeOutput::EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted)
{
    // Call the 'Redline' function. This will add redline (change-tracking) information that regards to paragraph properties.
    // This includes changes like 'Bold', 'Underline', 'Strikethrough' etc.

    // If there is RedlineData present, call WriteCollectedParagraphProperties() for writing pPr before calling Redline().
    // As there will be another pPr for redline and LO might mix both.
    if(pRedlineData)
        WriteCollectedParagraphProperties();
    Redline( pRedlineData );

    WriteCollectedParagraphProperties();

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_InitCollectedParagraphProperties);

    // Write 'Paragraph Mark' properties
    m_pSerializer->startElementNS(XML_w, XML_rPr);
    // mark() before paragraph mark properties child elements.
    InitCollectedRunProperties();

    // The 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList' are used to hold information
    // that should be collected by different properties in the core, and are all flushed together
    // to the DOCX when the function 'WriteCollectedRunProperties' gets called.
    // So we need to store the current status of these lists, so that we can revert back to them when
    // we are done exporting the redline attributes.
    rtl::Reference<sax_fastparser::FastAttributeList> pFontsAttrList_Original(m_pFontsAttrList);
    m_pFontsAttrList.clear();
    rtl::Reference<sax_fastparser::FastAttributeList> pEastAsianLayoutAttrList_Original(m_pEastAsianLayoutAttrList);
    m_pEastAsianLayoutAttrList.clear();
    rtl::Reference<sax_fastparser::FastAttributeList> pCharLangAttrList_Original(m_pCharLangAttrList);
    m_pCharLangAttrList.clear();

    lcl_writeParagraphMarkerProperties(*this, rParagraphMarkerProperties);

    // Write the collected run properties that are stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
    WriteCollectedRunProperties();

    // Revert back the original values that were stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
    m_pFontsAttrList = pFontsAttrList_Original.get();
    m_pEastAsianLayoutAttrList = pEastAsianLayoutAttrList_Original.get();
    m_pCharLangAttrList = pCharLangAttrList_Original.get();

    if ( pRedlineParagraphMarkerDeleted )
    {
        StartRedline( pRedlineParagraphMarkerDeleted );
        EndRedline( pRedlineParagraphMarkerDeleted );
    }
    if ( pRedlineParagraphMarkerInserted )
    {
        StartRedline( pRedlineParagraphMarkerInserted );
        EndRedline( pRedlineParagraphMarkerInserted );
    }

    // mergeTopMarks() after paragraph mark properties child elements.
    m_pSerializer->mergeTopMarks(Tag_InitCollectedRunProperties);
    m_pSerializer->endElementNS( XML_w, XML_rPr );

    if (!m_bWritingHeaderFooter && m_pCurrentFrame)
    {
        const SwFrameFormat& rFrameFormat = m_pCurrentFrame->GetFrameFormat();
        const SvxBoxItem& rBox = rFrameFormat.GetBox();
        if (TextBoxIsFramePr(rFrameFormat))
        {
            const Size aSize = m_pCurrentFrame->GetSize();
            PopulateFrameProperties(&rFrameFormat, aSize);
            FormatBox(rBox);
        }
    }

    m_pSerializer->endElementNS( XML_w, XML_pPr );

    // RDF metadata for this text node.
    SwTextNode* pTextNode = m_rExport.m_pCurPam->GetNode().GetTextNode();
    std::map<OUString, OUString> aStatements = SwRDFHelper::getTextNodeStatements("urn:bails", *pTextNode);
    if (!aStatements.empty())
    {
        m_pSerializer->startElementNS(XML_w, XML_smartTag,
                                      FSNS(XML_w, XML_uri), "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
                                      FSNS(XML_w, XML_element), "RDF");
        m_pSerializer->startElementNS(XML_w, XML_smartTagPr);
        for (const auto& rStatement : aStatements)
            m_pSerializer->singleElementNS(XML_w, XML_attr,
                                           FSNS(XML_w, XML_name), rStatement.first,
                                           FSNS(XML_w, XML_val), rStatement.second);
        m_pSerializer->endElementNS(XML_w, XML_smartTagPr);
        m_pSerializer->endElementNS(XML_w, XML_smartTag);
    }

    if ( m_nColBreakStatus == COLBRK_WRITE || m_nColBreakStatus == COLBRK_WRITEANDPOSTPONE )
    {
        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->singleElementNS(XML_w, XML_br, FSNS(XML_w, XML_type), "column");
        m_pSerializer->endElementNS( XML_w, XML_r );

        if ( m_nColBreakStatus == COLBRK_WRITEANDPOSTPONE )
            m_nColBreakStatus = COLBRK_POSTPONE;
        else
            m_nColBreakStatus = COLBRK_NONE;
    }

    if ( m_bPostponedPageBreak && !m_bWritingHeaderFooter )
    {
        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->singleElementNS(XML_w, XML_br, FSNS(XML_w, XML_type), "page");
        m_pSerializer->endElementNS( XML_w, XML_r );

        m_bPostponedPageBreak = false;
    }

    // merge the properties _before_ the run (strictly speaking, just
    // after the start of the paragraph)
    m_pSerializer->mergeTopMarks(Tag_StartParagraphProperties, sax_fastparser::MergeMarks::PREPEND);
}

void DocxAttributeOutput::SetStateOfFlyFrame( FlyProcessingState nStateOfFlyFrame )
{
    m_nStateOfFlyFrame = nStateOfFlyFrame;
}

void DocxAttributeOutput::SetAnchorIsLinkedToNode( bool bAnchorLinkedToNode )
{
    m_bAnchorLinkedToNode = bAnchorLinkedToNode ;
}

void DocxAttributeOutput::ResetFlyProcessingFlag()
{
    m_bPostponedProcessingFly = false ;
}

bool DocxAttributeOutput::IsFlyProcessingPostponed()
{
    return m_bPostponedProcessingFly;
}

void DocxAttributeOutput::StartRun( const SwRedlineData* pRedlineData, sal_Int32 /*nPos*/, bool /*bSingleEmptyRun*/ )
{
    // Don't start redline data here, possibly there is a hyperlink later, and
    // that has to be started first.
    m_pRedlineData = pRedlineData;

    // this mark is used to be able to enclose the run inside a sdr tag.
    m_pSerializer->mark(Tag_StartRun_1);

    // postpone the output of the start of a run (there are elements that need
    // to be written before the start of the run, but we learn which they are
    // _inside_ of the run)
    m_pSerializer->mark(Tag_StartRun_2); // let's call it "postponed run start"

    // postpone the output of the text (we get it before the run properties,
    // but must write it after them)
    m_pSerializer->mark(Tag_StartRun_3); // let's call it "postponed text"
}

void DocxAttributeOutput::EndRun(const SwTextNode* pNode, sal_Int32 nPos, bool /*bLastRun*/)
{
    int nFieldsInPrevHyperlink = m_nFieldsInHyperlink;
    // Reset m_nFieldsInHyperlink if a new hyperlink is about to start
    if ( m_pHyperlinkAttrList.is() )
    {
        m_nFieldsInHyperlink = 0;
    }

    // Write field starts
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin() + nFieldsInPrevHyperlink; pIt != m_Fields.end(); )
    {
        // Add the fields starts for all but hyperlinks and TOCs
        if (pIt->bOpen && pIt->pField && pIt->eType != ww::eFORMDROPDOWN)
        {
            StartField_Impl( pNode, nPos, *pIt );

            // Remove the field from the stack if only the start has to be written
            // Unknown fields should be removed too
            if ( !pIt->bClose || ( pIt->eType == ww::eUNKNOWN ) )
            {
                pIt = m_Fields.erase( pIt );
                continue;
            }

            if (m_startedHyperlink || m_pHyperlinkAttrList.is())
            {
                ++m_nFieldsInHyperlink;
            }
        }
        ++pIt;
    }

    // write the run properties + the text, already in the correct order
    m_pSerializer->mergeTopMarks(Tag_StartRun_3); // merges with "postponed text", see above

    // level down, to be able to prepend the actual run start attribute (just
    // before "postponed run start")
    m_pSerializer->mark(Tag_EndRun_1); // let's call it "actual run start"
    bool bCloseEarlierSDT = false;

    if (m_bEndCharSdt)
    {
        // This is the common case: "close sdt before the current run" was requested by the next run.

        // if another sdt starts in this run, then wait
        // as closing the sdt now, might cause nesting of sdts
        if (m_nRunSdtPrToken > 0)
            bCloseEarlierSDT = true;
        else
            EndSdtBlock();
        m_bEndCharSdt = false;
        m_bStartedCharSdt = false;
    }

    if ( m_closeHyperlinkInPreviousRun )
    {
        if ( m_startedHyperlink )
        {
            for ( int i = 0; i < nFieldsInPrevHyperlink; i++ )
            {
                // If fields begin before hyperlink then
                // it should end before hyperlink close
                EndField_Impl( pNode, nPos, m_Fields.back( ) );
                m_Fields.pop_back();
            }
            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
            m_startedHyperlink = false;
            m_endPageRef = false;
            m_nHyperLinkCount--;
        }
        m_closeHyperlinkInPreviousRun = false;
    }

    // Write the hyperlink and toc fields starts
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin(); pIt != m_Fields.end(); )
    {
        // Add the fields starts for hyperlinks, TOCs and index marks
        if (pIt->bOpen && (!pIt->pField || pIt->eType == ww::eFORMDROPDOWN))
        {
            StartRedline( m_pRedlineData );
            StartField_Impl( pNode, nPos, *pIt, true );
            EndRedline( m_pRedlineData );

            if (m_startedHyperlink)
                ++m_nFieldsInHyperlink;

            // Remove the field if no end needs to be written
            if (!pIt->bSep)
            {
                pIt = m_Fields.erase( pIt );
                continue;
            }
        }
        if (pIt->bSep && !pIt->pField)
        {
            // for TOXMark:
            // Word ignores bookmarks in field result that is empty;
            // work around this by writing bookmark into field command.
            if (!m_sFieldBkm.isEmpty())
            {
                DoWriteBookmarkTagStart(m_sFieldBkm);
                DoWriteBookmarkTagEnd(m_nNextBookmarkId);
                m_nNextBookmarkId++;
                m_sFieldBkm.clear();
            }
            CmdEndField_Impl(pNode, nPos, true);
            // Remove the field if no end needs to be written
            if (!pIt->bClose)
            {
                pIt = m_Fields.erase( pIt );
                continue;
            }
        }
        ++pIt;
    }

    // Start the hyperlink after the fields separators or we would generate invalid file
    bool newStartedHyperlink(false);
    if ( m_pHyperlinkAttrList.is() )
    {
        // if we are ending a hyperlink and there's another one starting here,
        // don't do this, so that the fields are closed further down when
        // the end hyperlink is handled, which is more likely to put the end in
        // the right place, as far as i can tell (not very far in this muck)
        if (!m_closeHyperlinkInThisRun)
        {
            // end ToX fields that want to end _before_ starting the hyperlink
            for (auto it = m_Fields.rbegin(); it != m_Fields.rend(); )
            {
                if (it->bClose && !it->pField)
                {
                    EndField_Impl( pNode, nPos, *it );
                    it = decltype(m_Fields)::reverse_iterator(m_Fields.erase(it.base() - 1));
                }
                else
                {
                    ++it;
                }
            }
        }
        newStartedHyperlink = true;

        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pHyperlinkAttrList );

        m_pSerializer->startElementNS( XML_w, XML_hyperlink, xAttrList );
        m_startedHyperlink = true;
        m_nHyperLinkCount++;
    }

    // if there is some redlining in the document, output it
    StartRedline( m_pRedlineData );

    // XML_r node should be surrounded with bookmark-begin and bookmark-end nodes if it has bookmarks.
    // The same is applied for permission ranges.
    // But due to unit test "testFdo85542" let's output bookmark-begin with bookmark-end.
    DoWriteBookmarksStart(m_rBookmarksStart, m_pMoveRedlineData);
    DoWriteBookmarksEnd(m_rBookmarksEnd);
    DoWritePermissionsStart();
    DoWriteAnnotationMarks();

    if( m_closeHyperlinkInThisRun && m_startedHyperlink && !m_hyperLinkAnchor.isEmpty() && m_hyperLinkAnchor.startsWith("_Toc"))
    {
        OUString sToken;
        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        m_pSerializer->singleElementNS(XML_w, XML_webHidden);
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        m_pSerializer->startElementNS(XML_w, XML_fldChar, FSNS(XML_w, XML_fldCharType), "begin");
        m_pSerializer->endElementNS( XML_w, XML_fldChar );
        m_pSerializer->endElementNS( XML_w, XML_r );


        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        m_pSerializer->singleElementNS(XML_w, XML_webHidden);
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        sToken = "PAGEREF " + m_hyperLinkAnchor + " \\h"; // '\h' Creates a hyperlink to the bookmarked paragraph.
        DoWriteCmd( sToken );
        m_pSerializer->endElementNS( XML_w, XML_r );

        // Write the Field separator
        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        m_pSerializer->singleElementNS(XML_w, XML_webHidden);
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "separate" );
        m_pSerializer->endElementNS( XML_w, XML_r );
        // At start of every "PAGEREF" field m_endPageRef value should be true.
        m_endPageRef = true;
    }

    DoWriteBookmarkStartIfExist(nPos);

    m_pSerializer->startElementNS(XML_w, XML_r);
    if(GetExport().m_bTabInTOC && m_pHyperlinkAttrList.is())
    {
        RunText("\t") ;
    }
    m_pSerializer->mergeTopMarks(Tag_EndRun_1, sax_fastparser::MergeMarks::PREPEND); // merges with "postponed run start", see above

    if ( !m_sRawText.isEmpty() )
    {
        RunText( m_sRawText );
        m_sRawText.clear();
    }

    // write the run start + the run content
    m_pSerializer->mergeTopMarks(Tag_StartRun_2); // merges the "actual run start"
    // append the actual run end
    m_pSerializer->endElementNS( XML_w, XML_r );

    // if there is some redlining in the document, output it
    // (except in the case of fields with multiple runs)
    EndRedline( m_pRedlineData );

    // enclose in a sdt block, if necessary: if one is already started, then don't do it for now
    // (so on export sdt blocks are never nested ATM)
    if ( !m_bAnchorLinkedToNode && !m_bStartedCharSdt )
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pRunSdtPrTokenAttributes;
        WriteSdtBlock(m_nRunSdtPrToken, m_pRunSdtPrTokenChildren, pRunSdtPrTokenAttributes,
                      m_pRunSdtPrDataBindingAttrs, m_pRunSdtPrTextAttrs, m_aRunSdtPrAlias,
                      m_aRunSdtPrPlaceHolderDocPart, m_aRunSdtPrColor, /*bPara=*/false);
    }
    else
    {
        //These should be written out to the actual Node and not to the anchor.
        //Clear them as they will be repopulated when the node is processed.
        m_nRunSdtPrToken = 0;
        lcl_deleteAndResetTheLists(m_pRunSdtPrTokenChildren, m_pRunSdtPrDataBindingAttrs,
                                   m_pRunSdtPrTextAttrs, m_aRunSdtPrAlias,
                                   m_aRunSdtPrPlaceHolderDocPart, m_aRunSdtPrColor);
    }

    if (bCloseEarlierSDT)
    {
        m_pSerializer->mark(Tag_EndRun_2);
        EndSdtBlock();
        m_pSerializer->mergeTopMarks(Tag_EndRun_2, sax_fastparser::MergeMarks::PREPEND);
    }

    m_pSerializer->mergeTopMarks(Tag_StartRun_1);

    // XML_r node should be surrounded with permission-begin and permission-end nodes if it has permission.
    DoWritePermissionsEnd();

    for (const auto& rpMath : m_aPostponedMaths)
        WritePostponedMath(rpMath.pMathObject, rpMath.nMathObjAlignment);
    m_aPostponedMaths.clear();

    for (const auto& rpControl : m_aPostponedFormControls)
        WritePostponedFormControl(rpControl);
    m_aPostponedFormControls.clear();

    WritePostponedActiveXControl(false);

    WritePendingPlaceholder();

    if ( !m_bWritingField )
    {
        m_pRedlineData = nullptr;
    }

    if ( m_closeHyperlinkInThisRun )
    {
        if ( m_startedHyperlink )
        {
            if( m_endPageRef )
            {
                // Hyperlink is started and fldchar "end" needs to be written for PAGEREF
                m_pSerializer->startElementNS(XML_w, XML_r);
                m_pSerializer->startElementNS(XML_w, XML_rPr);
                m_pSerializer->singleElementNS(XML_w, XML_webHidden);
                m_pSerializer->endElementNS( XML_w, XML_rPr );
                m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                        FSNS( XML_w, XML_fldCharType ), "end" );
                m_pSerializer->endElementNS( XML_w, XML_r );
                m_endPageRef = false;
                m_hyperLinkAnchor.clear();
            }
            for ( int i = 0; i < m_nFieldsInHyperlink; i++ )
            {
                // If fields begin after hyperlink start then
                // it should end before hyperlink close
                EndField_Impl( pNode, nPos, m_Fields.back( ) );
                m_Fields.pop_back();
            }
            m_nFieldsInHyperlink = 0;

            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
            m_startedHyperlink = false;
            m_nHyperLinkCount--;
        }
        m_closeHyperlinkInThisRun = false;
    }

    if (!newStartedHyperlink)
    {
        while ( m_Fields.begin() != m_Fields.end() )
        {
            EndField_Impl( pNode, nPos, m_Fields.front( ) );
            m_Fields.erase( m_Fields.begin( ) );
        }
        m_nFieldsInHyperlink = 0;
    }

    // end ToX fields
    for (auto it = m_Fields.rbegin(); it != m_Fields.rend(); )
    {
        if (it->bClose && !it->pField)
        {
            EndField_Impl( pNode, nPos, *it );
            it = decltype(m_Fields)::reverse_iterator(m_Fields.erase(it.base() - 1));
        }
        else
        {
            ++it;
        }
    }

    if ( m_pRedlineData )
    {
        EndRedline( m_pRedlineData );
        m_pRedlineData = nullptr;
    }

    DoWriteBookmarksStart(m_rFinalBookmarksStart);
    DoWriteBookmarksEnd(m_rFinalBookmarksEnd);
    DoWriteBookmarkEndIfExist(nPos);
}

void DocxAttributeOutput::DoWriteBookmarkTagStart(const OUString & bookmarkName)
{
    m_pSerializer->singleElementNS(XML_w, XML_bookmarkStart,
        FSNS(XML_w, XML_id), OString::number(m_nNextBookmarkId),
        FSNS(XML_w, XML_name), BookmarkToWord(bookmarkName));
}

void DocxAttributeOutput::DoWriteBookmarkTagEnd(sal_Int32 const nId)
{
    m_pSerializer->singleElementNS(XML_w, XML_bookmarkEnd,
        FSNS(XML_w, XML_id), OString::number(nId));
}

void DocxAttributeOutput::DoWriteMoveRangeTagStart(const OString & bookmarkName,
    bool bFrom, const SwRedlineData* pRedlineData)
{
    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aDate( DateTimeToOString( pRedlineData->GetTimeStamp() ) );

    m_pSerializer->singleElementNS(XML_w, bFrom
                ? XML_moveFromRangeStart
                : XML_moveToRangeStart,
        FSNS(XML_w, XML_id), OString::number(m_nNextBookmarkId),
        FSNS(XML_w, XML_author ), OUStringToOString(rAuthor, RTL_TEXTENCODING_UTF8),
        FSNS(XML_w, XML_date ), aDate,
        FSNS(XML_w, XML_name), bookmarkName);
}

void DocxAttributeOutput::DoWriteMoveRangeTagEnd(sal_Int32 const nId, bool bFrom)
{
    m_pSerializer->singleElementNS(XML_w, bFrom
            ? XML_moveFromRangeEnd
            : XML_moveToRangeEnd,
        FSNS(XML_w, XML_id), OString::number(nId));
}

void DocxAttributeOutput::DoWriteBookmarkStartIfExist(sal_Int32 nRunPos)
{
    auto aRange = m_aBookmarksOfParagraphStart.equal_range(nRunPos);
    for( auto aIter = aRange.first; aIter != aRange.second; ++aIter)
    {
        DoWriteBookmarkTagStart(aIter->second);
        m_rOpenedBookmarksIds[aIter->second] = m_nNextBookmarkId;
        m_sLastOpenedBookmark = OUStringToOString(BookmarkToWord(aIter->second), RTL_TEXTENCODING_UTF8);
        m_nNextBookmarkId++;
    }
}

void DocxAttributeOutput::DoWriteBookmarkEndIfExist(sal_Int32 nRunPos)
{
    auto aRange = m_aBookmarksOfParagraphEnd.equal_range(nRunPos);
    for( auto aIter = aRange.first; aIter != aRange.second; ++aIter)
    {
        // Get the id of the bookmark
        auto pPos = m_rOpenedBookmarksIds.find(aIter->second);
        if (pPos != m_rOpenedBookmarksIds.end())
        {
            // Output the bookmark
            DoWriteBookmarkTagEnd(pPos->second);
            m_rOpenedBookmarksIds.erase(aIter->second);
        }
    }
}

/// Write the start bookmarks
void DocxAttributeOutput::DoWriteBookmarksStart(std::vector<OUString>& rStarts, const SwRedlineData* pRedlineData)
{
    for (const OUString & bookmarkName : rStarts)
    {
        // Output the bookmark (including MoveBookmark of the tracked moving)
        bool bMove = false;
        bool bFrom = false;
        OString sBookmarkName = OUStringToOString(
                BookmarkToWord(bookmarkName, &bMove, &bFrom), RTL_TEXTENCODING_UTF8);
        if ( bMove )
        {
            // TODO: redline data of MoveBookmark is restored from the first redline of the bookmark
            // range. But a later deletion within a tracked moving is still imported as plain
            // deletion, so check IsMoved() and skip the export of the tracked moving to avoid
            // export with bad author or date
            if ( pRedlineData && pRedlineData->IsMoved() )
                DoWriteMoveRangeTagStart(sBookmarkName, bFrom, pRedlineData);
        }
        else
            DoWriteBookmarkTagStart(bookmarkName);

        m_rOpenedBookmarksIds[bookmarkName] = m_nNextBookmarkId;
        m_sLastOpenedBookmark = sBookmarkName;
        m_nNextBookmarkId++;
    }
    rStarts.clear();
}

/// export the end bookmarks
void DocxAttributeOutput::DoWriteBookmarksEnd(std::vector<OUString>& rEnds)
{
    for (const OUString & bookmarkName : rEnds)
    {
        // Get the id of the bookmark
        auto pPos = m_rOpenedBookmarksIds.find(bookmarkName);

        if (pPos != m_rOpenedBookmarksIds.end())
        {
            bool bMove = false;
            bool bFrom = false;
            BookmarkToWord(bookmarkName, &bMove, &bFrom);
            // Output the bookmark (including MoveBookmark of the tracked moving)
            if ( bMove )
                DoWriteMoveRangeTagEnd(pPos->second, bFrom);
            else
                DoWriteBookmarkTagEnd(pPos->second);

            m_rOpenedBookmarksIds.erase(bookmarkName);
        }
    }
    rEnds.clear();
}

// For construction of the special bookmark name template for permissions:
// see, PermInsertPosition::createBookmarkName()
//
// Syntax:
// - "permission-for-user:<permission-id>:<permission-user-name>"
// - "permission-for-group:<permission-id>:<permission-group-name>"
//
void DocxAttributeOutput::DoWritePermissionTagStart(std::u16string_view permission)
{
    std::u16string_view permissionIdAndName;

    if (o3tl::starts_with(permission, u"permission-for-group:", &permissionIdAndName))
    {
        const std::size_t separatorIndex = permissionIdAndName.find(u':');
        assert(separatorIndex != std::u16string_view::npos);
        const std::u16string_view permissionId   = permissionIdAndName.substr(0, separatorIndex);
        const std::u16string_view permissionName = permissionIdAndName.substr(separatorIndex + 1);

        m_pSerializer->singleElementNS(XML_w, XML_permStart,
            FSNS(XML_w, XML_id), BookmarkToWord(OUString(permissionId)),
            FSNS(XML_w, XML_edGrp), BookmarkToWord(OUString(permissionName)));
    }
    else
    {
        auto const ok = o3tl::starts_with(
            permission, u"permission-for-user:", &permissionIdAndName);
        assert(ok); (void)ok;
        const std::size_t separatorIndex = permissionIdAndName.find(u':');
        assert(separatorIndex != std::u16string_view::npos);
        const std::u16string_view permissionId   = permissionIdAndName.substr(0, separatorIndex);
        const std::u16string_view permissionName = permissionIdAndName.substr(separatorIndex + 1);

        m_pSerializer->singleElementNS(XML_w, XML_permStart,
            FSNS(XML_w, XML_id), BookmarkToWord(OUString(permissionId)),
            FSNS(XML_w, XML_ed), BookmarkToWord(OUString(permissionName)));
    }
}


// For construction of the special bookmark name template for permissions:
// see, PermInsertPosition::createBookmarkName()
//
// Syntax:
// - "permission-for-user:<permission-id>:<permission-user-name>"
// - "permission-for-group:<permission-id>:<permission-group-name>"
//
void DocxAttributeOutput::DoWritePermissionTagEnd(std::u16string_view permission)
{
    std::u16string_view permissionIdAndName;

    auto const ok = o3tl::starts_with(permission, u"permission-for-group:", &permissionIdAndName) ||
        o3tl::starts_with(permission, u"permission-for-user:", &permissionIdAndName);
    assert(ok); (void)ok;

    const std::size_t separatorIndex = permissionIdAndName.find(u':');
    assert(separatorIndex != std::u16string_view::npos);
    const std::u16string_view permissionId   = permissionIdAndName.substr(0, separatorIndex);

    m_pSerializer->singleElementNS(XML_w, XML_permEnd,
        FSNS(XML_w, XML_id), BookmarkToWord(OUString(permissionId)));
}

/// Write the start permissions
void DocxAttributeOutput::DoWritePermissionsStart()
{
    for (const OUString & permission : m_rPermissionsStart)
    {
        DoWritePermissionTagStart(permission);
    }
    m_rPermissionsStart.clear();
}

/// export the end permissions
void DocxAttributeOutput::DoWritePermissionsEnd()
{
    for (const OUString & permission : m_rPermissionsEnd)
    {
        DoWritePermissionTagEnd(permission);
    }
    m_rPermissionsEnd.clear();
}

void DocxAttributeOutput::DoWriteAnnotationMarks()
{
    // Write the start annotation marks
    for ( const auto & rName : m_rAnnotationMarksStart )
    {
        // Output the annotation mark
        /* Ensure that the existing Annotation Marks are not overwritten
           as it causes discrepancy when DocxAttributeOutput::PostitField
           refers to this map & while mapping comment id's in document.xml &
           comment.xml.
        */
        if ( m_rOpenedAnnotationMarksIds.end() == m_rOpenedAnnotationMarksIds.find( rName ) )
        {
            const sal_Int32 nId = m_nNextAnnotationMarkId++;
            m_rOpenedAnnotationMarksIds[rName] = nId;
            m_pSerializer->singleElementNS( XML_w, XML_commentRangeStart,
                FSNS( XML_w, XML_id ), OString::number(nId) );
            m_sLastOpenedAnnotationMark = rName;
        }
    }
    m_rAnnotationMarksStart.clear();

    // export the end annotation marks
    for ( const auto & rName : m_rAnnotationMarksEnd )
    {
        // Get the id of the annotation mark
        std::map< OString, sal_Int32 >::iterator pPos = m_rOpenedAnnotationMarksIds.find( rName );
        if ( pPos != m_rOpenedAnnotationMarksIds.end(  ) )
        {
            const sal_Int32 nId = ( *pPos ).second;
            m_pSerializer->singleElementNS( XML_w, XML_commentRangeEnd,
                FSNS( XML_w, XML_id ), OString::number(nId) );
            m_rOpenedAnnotationMarksIds.erase( rName );

            m_pSerializer->startElementNS(XML_w, XML_r);
            m_pSerializer->singleElementNS( XML_w, XML_commentReference, FSNS( XML_w, XML_id ),
                                            OString::number(nId) );
            m_pSerializer->endElementNS(XML_w, XML_r);
        }
    }
    m_rAnnotationMarksEnd.clear();
}

void DocxAttributeOutput::WriteFFData(  const FieldInfos& rInfos )
{
    const ::sw::mark::IFieldmark& rFieldmark = *rInfos.pFieldmark;
    FieldMarkParamsHelper params( rFieldmark );

    OUString sEntryMacro;
    params.extractParam("EntryMacro", sEntryMacro);
    OUString sExitMacro;
    params.extractParam("ExitMacro", sExitMacro);
    OUString sHelp;
    params.extractParam("Help", sHelp);
    OUString sHint;
    params.extractParam("Hint", sHint); // .docx StatusText
    if ( sHint.isEmpty() )
        params.extractParam("Description", sHint); // .doc StatusText

    if ( rInfos.eType == ww::eFORMDROPDOWN )
    {
        uno::Sequence< OUString> vListEntries;
        OUString sName, sSelected;

        params.extractParam( ODF_FORMDROPDOWN_LISTENTRY, vListEntries );
        if (vListEntries.getLength() > ODF_FORMDROPDOWN_ENTRY_COUNT_LIMIT)
            vListEntries = uno::Sequence< OUString>(vListEntries.getArray(), ODF_FORMDROPDOWN_ENTRY_COUNT_LIMIT);

        sName = params.getName();
        sal_Int32 nSelectedIndex = 0;

        if ( params.extractParam( ODF_FORMDROPDOWN_RESULT, nSelectedIndex ) )
        {
            if (nSelectedIndex < vListEntries.getLength() )
                sSelected = vListEntries[ nSelectedIndex ];
        }

        GetExport().DoComboBox( sName, OUString(), OUString(), sSelected, vListEntries );
    }
    else if ( rInfos.eType == ww::eFORMCHECKBOX )
    {
        OUString sName;
        bool bChecked = false;

        params.extractParam( ODF_FORMCHECKBOX_NAME, sName );

        const sw::mark::ICheckboxFieldmark* pCheckboxFm = dynamic_cast<const sw::mark::ICheckboxFieldmark*>(&rFieldmark);
        if ( pCheckboxFm && pCheckboxFm->IsChecked() )
            bChecked = true;

        FFDataWriterHelper ffdataOut( m_pSerializer );
        ffdataOut.WriteFormCheckbox( sName, sEntryMacro, sExitMacro, sHelp, sHint, bChecked );
    }
    else if ( rInfos.eType == ww::eFORMTEXT )
    {
        OUString sType;
        params.extractParam("Type", sType);
        OUString sDefaultText;
        params.extractParam("Content", sDefaultText);
        sal_uInt16 nMaxLength = 0;
        params.extractParam("MaxLength", nMaxLength);
        OUString sFormat;
        params.extractParam("Format", sFormat);
        FFDataWriterHelper ffdataOut( m_pSerializer );
        ffdataOut.WriteFormText( params.getName(), sEntryMacro, sExitMacro, sHelp, sHint,
                                 sType, sDefaultText, nMaxLength, sFormat );
    }
}

void DocxAttributeOutput::WriteFormDateStart(const OUString& sFullDate, const OUString& sDateFormat, const OUString& sLang)
{
    m_pSerializer->startElementNS(XML_w, XML_sdt);
    m_pSerializer->startElementNS(XML_w, XML_sdtPr);

    if(!sFullDate.isEmpty())
        m_pSerializer->startElementNS(XML_w, XML_date, FSNS(XML_w, XML_fullDate), sFullDate);
    else
        m_pSerializer->startElementNS(XML_w, XML_date);

    // Replace quotation mark used for marking static strings in date format
    OUString sDateFormat1 = sDateFormat.replaceAll("\"", "'");
    m_pSerializer->singleElementNS(XML_w, XML_dateFormat,
                                   FSNS(XML_w, XML_val), sDateFormat1);
    m_pSerializer->singleElementNS(XML_w, XML_lid,
                                   FSNS(XML_w, XML_val), sLang);
    m_pSerializer->singleElementNS(XML_w, XML_storeMappedDataAs,
                                   FSNS(XML_w, XML_val), "dateTime");
    m_pSerializer->singleElementNS(XML_w, XML_calendar,
                                   FSNS(XML_w, XML_val), "gregorian");

    m_pSerializer->endElementNS(XML_w, XML_date);
    m_pSerializer->endElementNS(XML_w, XML_sdtPr);

    m_pSerializer->startElementNS(XML_w, XML_sdtContent);
}

void DocxAttributeOutput::WriteSdtEnd()
{
    m_pSerializer->endElementNS(XML_w, XML_sdtContent);
    m_pSerializer->endElementNS(XML_w, XML_sdt);
}

void DocxAttributeOutput::WriteSdtDropDownStart(
        std::u16string_view rName,
        OUString const& rSelected,
        uno::Sequence<OUString> const& rListItems)
{
    m_pSerializer->startElementNS(XML_w, XML_sdt);
    m_pSerializer->startElementNS(XML_w, XML_sdtPr);

    m_pSerializer->singleElementNS(XML_w, XML_alias,
        FSNS(XML_w, XML_val), OUStringToOString(rName, RTL_TEXTENCODING_UTF8));

    sal_Int32 nId = comphelper::findValue(rListItems, rSelected);
    if (nId == -1)
    {
        nId = 0;
    }

    m_pSerializer->startElementNS(XML_w, XML_dropDownList,
            FSNS(XML_w, XML_lastValue), OString::number(nId));

    for (auto const& rItem : rListItems)
    {
        auto const item(OUStringToOString(rItem, RTL_TEXTENCODING_UTF8));
        m_pSerializer->singleElementNS(XML_w, XML_listItem,
                FSNS(XML_w, XML_value), item,
                FSNS(XML_w, XML_displayText), item);
    }

    m_pSerializer->endElementNS(XML_w, XML_dropDownList);
    m_pSerializer->endElementNS(XML_w, XML_sdtPr);

    m_pSerializer->startElementNS(XML_w, XML_sdtContent);
}

void DocxAttributeOutput::WriteSdtDropDownEnd(OUString const& rSelected,
        uno::Sequence<OUString> const& rListItems)
{
    // note: rSelected might be empty?
    sal_Int32 nId = comphelper::findValue(rListItems, rSelected);
    if (nId == -1)
    {
        nId = 0;
    }

    // the lastValue only identifies the entry in the list, also export
    // currently selected item's displayText as run content (if one exists)
    if (rListItems.size())
    {
        m_pSerializer->startElementNS(XML_w, XML_r);
        m_pSerializer->startElementNS(XML_w, XML_t);
        m_pSerializer->writeEscaped(rListItems[nId]);
        m_pSerializer->endElementNS(XML_w, XML_t);
        m_pSerializer->endElementNS(XML_w, XML_r);
    }

    WriteSdtEnd();
}

void DocxAttributeOutput::StartField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun )
{
    if ( rInfos.pField && rInfos.eType == ww::eUNKNOWN )
    {
        // Expand unsupported fields
        RunText( rInfos.pField->GetFieldName() );
    }
    else if ( rInfos.eType == ww::eFORMDATE )
    {
        const sw::mark::IDateFieldmark& rFieldmark = dynamic_cast<const sw::mark::IDateFieldmark&>(*rInfos.pFieldmark);
        FieldMarkParamsHelper params(rFieldmark);

        OUString sFullDate;
        OUString sCurrentDate;
        params.extractParam( ODF_FORMDATE_CURRENTDATE, sCurrentDate );
        if(!sCurrentDate.isEmpty())
        {
            sFullDate = sCurrentDate + "T00:00:00Z";
        }
        else
        {
            std::pair<bool, double> aResult = rFieldmark.GetCurrentDate();
            if(aResult.first)
            {
                sFullDate = rFieldmark.GetDateInStandardDateFormat(aResult.second) + "T00:00:00Z";
            }
        }

        OUString sDateFormat;
        params.extractParam( ODF_FORMDATE_DATEFORMAT, sDateFormat );
        OUString sLang;
        params.extractParam( ODF_FORMDATE_DATEFORMAT_LANGUAGE, sLang );

        WriteFormDateStart( sFullDate, sDateFormat, sLang );
    }
    else if (rInfos.eType == ww::eFORMDROPDOWN && rInfos.pField)
    {
        assert(!rInfos.pFieldmark);
        SwDropDownField const& rField2(*static_cast<SwDropDownField const*>(rInfos.pField.get()));
        WriteSdtDropDownStart(rField2.GetName(),
                rField2.GetSelectedItem(),
                rField2.GetItemSequence());
    }
    else if ( rInfos.eType != ww::eNONE ) // HYPERLINK fields are just commands
    {
        if ( bWriteRun )
            m_pSerializer->startElementNS(XML_w, XML_r);

        if ( rInfos.eType == ww::eFORMDROPDOWN )
        {
            m_pSerializer->startElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin" );
            assert( rInfos.pFieldmark && !rInfos.pField );
            WriteFFData(rInfos);
            m_pSerializer->endElementNS( XML_w, XML_fldChar );

            if ( bWriteRun )
                m_pSerializer->endElementNS( XML_w, XML_r );

            CmdField_Impl( pNode, nPos, rInfos, bWriteRun );
        }
        else
        {
            // Write the field start
            if ( rInfos.pField && (rInfos.pField->Which() == SwFieldIds::DateTime) && rInfos.pField->GetSubType() & FIXEDFLD )
            {
                m_pSerializer->startElementNS( XML_w, XML_fldChar,
                    FSNS( XML_w, XML_fldCharType ), "begin",
                    FSNS( XML_w, XML_fldLock ), "true" );
            }
            else
            {
                m_pSerializer->startElementNS( XML_w, XML_fldChar,
                    FSNS( XML_w, XML_fldCharType ), "begin" );
            }

            if ( rInfos.pFieldmark )
                WriteFFData(  rInfos );

            m_pSerializer->endElementNS( XML_w, XML_fldChar );

            if ( bWriteRun )
                m_pSerializer->endElementNS( XML_w, XML_r );

            // The hyperlinks fields can't be expanded: the value is
            // normally in the text run
            if ( !rInfos.pField )
                CmdField_Impl( pNode, nPos, rInfos, bWriteRun );
            else
                m_bWritingField = true;
        }
    }
}

void DocxAttributeOutput::DoWriteCmd( const OUString& rCmd )
{
    OUString sCmd = rCmd.trim();
    if (sCmd.startsWith("SEQ"))
    {
        OUString sSeqName = msfilter::util::findQuotedText(sCmd, "SEQ ", '\\').trim();
        m_aSeqBookmarksNames[sSeqName].push_back(m_sLastOpenedBookmark);
    }
    // Write the Field command
    sal_Int32 nTextToken = XML_instrText;
    if ( m_pRedlineData && m_pRedlineData->GetType() == RedlineType::Delete )
        nTextToken = XML_delInstrText;

    m_pSerializer->startElementNS(XML_w, nTextToken, FSNS(XML_xml, XML_space), "preserve");
    m_pSerializer->writeEscaped( rCmd );
    m_pSerializer->endElementNS( XML_w, nTextToken );

}

void DocxAttributeOutput::CmdField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun )
{
    // Write the Field instruction
    if ( bWriteRun )
    {
        bool bWriteCombChars(false);
        m_pSerializer->startElementNS(XML_w, XML_r);

        if (rInfos.eType == ww::eEQ)
            bWriteCombChars = true;

        DoWriteFieldRunProperties( pNode, nPos, bWriteCombChars );
    }

    sal_Int32 nIdx { rInfos.sCmd.isEmpty() ? -1 : 0 };
    while ( nIdx >= 0 )
    {
        OUString sToken = rInfos.sCmd.getToken( 0, '\t', nIdx );
        if ( rInfos.eType ==  ww::eCREATEDATE
          || rInfos.eType ==  ww::eSAVEDATE
          || rInfos.eType ==  ww::ePRINTDATE
          || rInfos.eType ==  ww::eDATE
          || rInfos.eType ==  ww::eTIME )
        {
           sToken = sToken.replaceAll("NNNN", "dddd");
           sToken = sToken.replaceAll("NN", "ddd");
        }
        else if ( rInfos.eType == ww::eEquals )
        {
            // Use original OOXML formula, if it exists and its conversion hasn't been changed
            bool bIsChanged = true;
            if ( pNode->GetTableBox() )
            {
                if ( const SfxGrabBagItem* pItem = pNode->GetTableBox()->GetFrameFormat()->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG) )
                {
                    OUString sActualFormula = sToken.trim();
                    const std::map<OUString, uno::Any>& rGrabBag = pItem->GetGrabBag();
                    std::map<OUString, uno::Any>::const_iterator aStoredFormula = rGrabBag.find("CellFormulaConverted");
                    if ( aStoredFormula != rGrabBag.end() && sActualFormula.indexOf('=') == 0 &&
                                    sActualFormula.copy(1).trim() == aStoredFormula->second.get<OUString>().trim() )
                    {
                        aStoredFormula = rGrabBag.find("CellFormula");
                        if ( aStoredFormula != rGrabBag.end() )
                        {
                            sToken = " =" + aStoredFormula->second.get<OUString>();
                            bIsChanged = false;
                        }
                    }
                }
            }

            if ( bIsChanged )
            {
                UErrorCode nErr(U_ZERO_ERROR);
                icu::UnicodeString sInput(sToken.getStr());
                // remove < and > around cell references, e.g. <A1> to A1, <A1:B2> to A1:B2
                icu::RegexMatcher aMatcher("<([A-Z]{1,3}[0-9]+(:[A-Z]{1,3}[0-9]+)?)>", sInput, 0, nErr);
                sInput = aMatcher.replaceAll(icu::UnicodeString("$1"), nErr);
                // convert MEAN to AVERAGE
                icu::RegexMatcher aMatcher2("\\bMEAN\\b", sInput, UREGEX_CASE_INSENSITIVE, nErr);
                sToken = aMatcher2.replaceAll(icu::UnicodeString("AVERAGE"), nErr).getTerminatedBuffer();
            }
        }

        // Write the Field command
        DoWriteCmd( sToken );

        // Replace tabs by </instrText><tab/><instrText>
        if ( nIdx > 0 ) // Is another token expected?
            RunText( "\t" );
    }

    if ( bWriteRun )
    {
        m_pSerializer->endElementNS( XML_w, XML_r );
    }
}

void DocxAttributeOutput::CmdEndField_Impl(SwTextNode const*const pNode,
        sal_Int32 const nPos, bool const bWriteRun)
{
    // Write the Field separator
        if ( bWriteRun )
        {
            m_pSerializer->startElementNS(XML_w, XML_r);
            DoWriteFieldRunProperties( pNode, nPos );
        }

        m_pSerializer->singleElementNS( XML_w, XML_fldChar,
              FSNS( XML_w, XML_fldCharType ), "separate" );

        if ( bWriteRun )
        {
            m_pSerializer->endElementNS( XML_w, XML_r );
        }
}

/// Writes properties for run that is used to separate field implementation.
/// There are several runs are used:
///     <w:r>
///         <w:rPr>
///             <!-- properties written with StartRunProperties() / EndRunProperties().
///         </w:rPr>
///         <w:fldChar w:fldCharType="begin" />
///     </w:r>
///         <w:r>
///         <w:rPr>
///             <!-- properties written with DoWriteFieldRunProperties()
///         </w:rPr>
///         <w:instrText>TIME \@"HH:mm:ss"</w:instrText>
///     </w:r>
///     <w:r>
///         <w:rPr>
///             <!-- properties written with DoWriteFieldRunProperties()
///         </w:rPr>
///         <w:fldChar w:fldCharType="separate" />
///     </w:r>
///     <w:r>
///         <w:rPr>
///             <!-- properties written with DoWriteFieldRunProperties()
///         </w:rPr>
///         <w:t>14:01:13</w:t>
///         </w:r>
///     <w:r>
///         <w:rPr>
///             <!-- properties written with DoWriteFieldRunProperties()
///         </w:rPr>
///         <w:fldChar w:fldCharType="end" />
///     </w:r>
/// See, tdf#38778
void DocxAttributeOutput::DoWriteFieldRunProperties( const SwTextNode * pNode, sal_Int32 nPos, bool bWriteCombChars)
{
    if (! pNode)
    {
        // nothing to do
        return;
    }

    m_bPreventDoubleFieldsHandling = true;

    {
        m_pSerializer->startElementNS(XML_w, XML_rPr);

        // 1. output webHidden flag
        if(GetExport().m_bHideTabLeaderAndPageNumbers && m_pHyperlinkAttrList.is() )
        {
            m_pSerializer->singleElementNS(XML_w, XML_webHidden);
        }

        // 2. find all active character properties
        SwWW8AttrIter aAttrIt( m_rExport, *pNode );
        aAttrIt.OutAttr( nPos, bWriteCombChars );

        // 3. write the character properties
        WriteCollectedRunProperties();

        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }

    m_bPreventDoubleFieldsHandling = false;
}

void DocxAttributeOutput::EndField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos& rInfos )
{
    if (rInfos.eType == ww::eFORMDATE)
    {
        WriteSdtEnd();
        return;
    }
    if (rInfos.eType == ww::eFORMDROPDOWN && rInfos.pField)
    {
        // write selected item from End not Start to ensure that any bookmarks
        // precede it
        SwDropDownField const& rField(*static_cast<SwDropDownField const*>(rInfos.pField.get()));
        WriteSdtDropDownEnd(rField.GetSelectedItem(), rField.GetItemSequence());
        return;
    }

    // The command has to be written before for the hyperlinks
    if ( rInfos.pField )
    {
        CmdField_Impl( pNode, nPos, rInfos, true );
        CmdEndField_Impl( pNode, nPos, true );
    }

    // Write the bookmark start if any
    if ( !m_sFieldBkm.isEmpty() )
    {
        DoWriteBookmarkTagStart(m_sFieldBkm);
    }

    if (rInfos.pField ) // For hyperlinks and TOX
    {
        // Write the Field latest value
        m_pSerializer->startElementNS(XML_w, XML_r);
        DoWriteFieldRunProperties( pNode, nPos );

        OUString sExpand;
        if(rInfos.eType == ww::eCITATION)
        {
            sExpand = static_cast<SwAuthorityField const*>(rInfos.pField.get())
                        ->ExpandCitation(AUTH_FIELD_TITLE, nullptr);
        }
        else if(rInfos.eType != ww::eFORMDROPDOWN)
        {
            sExpand = rInfos.pField->ExpandField(true, nullptr);
        }
        // newlines embedded in fields are 0x0B in MSO and 0x0A for us
        RunText(sExpand.replace(0x0A, 0x0B));

        m_pSerializer->endElementNS( XML_w, XML_r );
    }

    // Write the bookmark end if any
    if ( !m_sFieldBkm.isEmpty() )
    {
        DoWriteBookmarkTagEnd(m_nNextBookmarkId);

        m_nNextBookmarkId++;
    }

    // Write the Field end
    if ( rInfos.bClose  )
    {
        m_bWritingField = false;
        m_pSerializer->startElementNS(XML_w, XML_r);
        DoWriteFieldRunProperties( pNode, nPos );
        m_pSerializer->singleElementNS(XML_w, XML_fldChar, FSNS(XML_w, XML_fldCharType), "end");
        m_pSerializer->endElementNS( XML_w, XML_r );
    }
    // Write the ref field if a bookmark had to be set and the field
    // should be visible
    if ( !rInfos.pField )
    {
        m_sFieldBkm.clear();
        return;
    }

    sal_uInt16 nSubType = rInfos.pField->GetSubType( );
    bool bIsSetField = rInfos.pField->GetTyp( )->Which( ) == SwFieldIds::SetExp;
    bool bShowRef = bIsSetField && ( nSubType & nsSwExtendedSubType::SUB_INVISIBLE ) == 0;

    if (!bShowRef)
    {
        m_sFieldBkm.clear();
    }

    if (m_sFieldBkm.isEmpty())
        return;

    // Write the field beginning
    m_pSerializer->startElementNS(XML_w, XML_r);
    m_pSerializer->singleElementNS( XML_w, XML_fldChar,
        FSNS( XML_w, XML_fldCharType ), "begin" );
    m_pSerializer->endElementNS( XML_w, XML_r );

    rInfos.sCmd = FieldString( ww::eREF );
    rInfos.sCmd += "\"";
    rInfos.sCmd += m_sFieldBkm;
    rInfos.sCmd += "\" ";

    // Clean the field bookmark data to avoid infinite loop
    m_sFieldBkm = OUString( );

    // Write the end of the field
    EndField_Impl( pNode, nPos, rInfos );
}

void DocxAttributeOutput::StartRunProperties()
{
    // postpone the output so that we can later [in EndRunProperties()]
    // prepend the properties before the text
    m_pSerializer->mark(Tag_StartRunProperties);

    m_pSerializer->startElementNS(XML_w, XML_rPr);

    if(GetExport().m_bHideTabLeaderAndPageNumbers && m_pHyperlinkAttrList.is() )
    {
        m_pSerializer->singleElementNS(XML_w, XML_webHidden);
    }
    InitCollectedRunProperties();

    assert( !m_pPostponedGraphic );
    m_pPostponedGraphic.reset(new std::vector<PostponedGraphic>);

    assert( !m_pPostponedDiagrams );
    m_pPostponedDiagrams.reset(new std::vector<PostponedDiagram>);

    assert(!m_pPostponedDMLDrawings);
    m_pPostponedDMLDrawings.reset(new std::vector<PostponedDrawing>);

    assert( !m_pPostponedOLEs );
    m_pPostponedOLEs.reset(new std::vector<PostponedOLE>);
}

void DocxAttributeOutput::InitCollectedRunProperties()
{
    m_pFontsAttrList = nullptr;
    m_pEastAsianLayoutAttrList = nullptr;
    m_pCharLangAttrList = nullptr;

    // Write the elements in the spec order
    static const sal_Int32 aOrder[] =
    {
        FSNS( XML_w, XML_rStyle ),
        FSNS( XML_w, XML_rFonts ),
        FSNS( XML_w, XML_b ),
        FSNS( XML_w, XML_bCs ),
        FSNS( XML_w, XML_i ),
        FSNS( XML_w, XML_iCs ),
        FSNS( XML_w, XML_caps ),
        FSNS( XML_w, XML_smallCaps ),
        FSNS( XML_w, XML_strike ),
        FSNS( XML_w, XML_dstrike ),
        FSNS( XML_w, XML_outline ),
        FSNS( XML_w, XML_shadow ),
        FSNS( XML_w, XML_emboss ),
        FSNS( XML_w, XML_imprint ),
        FSNS( XML_w, XML_noProof ),
        FSNS( XML_w, XML_snapToGrid ),
        FSNS( XML_w, XML_vanish ),
        FSNS( XML_w, XML_webHidden ),
        FSNS( XML_w, XML_color ),
        FSNS( XML_w, XML_spacing ),
        FSNS( XML_w, XML_w ),
        FSNS( XML_w, XML_kern ),
        FSNS( XML_w, XML_position ),
        FSNS( XML_w, XML_sz ),
        FSNS( XML_w, XML_szCs ),
        FSNS( XML_w, XML_highlight ),
        FSNS( XML_w, XML_u ),
        FSNS( XML_w, XML_effect ),
        FSNS( XML_w, XML_bdr ),
        FSNS( XML_w, XML_shd ),
        FSNS( XML_w, XML_fitText ),
        FSNS( XML_w, XML_vertAlign ),
        FSNS( XML_w, XML_rtl ),
        FSNS( XML_w, XML_cs ),
        FSNS( XML_w, XML_em ),
        FSNS( XML_w, XML_lang ),
        FSNS( XML_w, XML_eastAsianLayout ),
        FSNS( XML_w, XML_specVanish ),
        FSNS( XML_w, XML_oMath ),
        FSNS( XML_w, XML_rPrChange ),
        FSNS( XML_w, XML_del ),
        FSNS( XML_w14, XML_glow ),
        FSNS( XML_w14, XML_shadow ),
        FSNS( XML_w14, XML_reflection ),
        FSNS( XML_w14, XML_textOutline ),
        FSNS( XML_w14, XML_textFill ),
        FSNS( XML_w14, XML_scene3d ),
        FSNS( XML_w14, XML_props3d ),
        FSNS( XML_w14, XML_ligatures ),
        FSNS( XML_w14, XML_numForm ),
        FSNS( XML_w14, XML_numSpacing ),
        FSNS( XML_w14, XML_stylisticSets ),
        FSNS( XML_w14, XML_cntxtAlts ),
    };

    // postpone the output so that we can later [in EndParagraphProperties()]
    // prepend the properties before the run
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    m_pSerializer->mark(Tag_InitCollectedRunProperties, comphelper::containerToSequence(aOrder));
}

namespace
{

struct NameToId
{
    OUString  maName;
    sal_Int32 maId;
};

const NameToId constNameToIdMapping[] =
{
    { OUString("glow"),         FSNS( XML_w14, XML_glow ) },
    { OUString("shadow"),       FSNS( XML_w14, XML_shadow ) },
    { OUString("reflection"),   FSNS( XML_w14, XML_reflection ) },
    { OUString("textOutline"),  FSNS( XML_w14, XML_textOutline ) },
    { OUString("textFill"),     FSNS( XML_w14, XML_textFill ) },
    { OUString("scene3d"),      FSNS( XML_w14, XML_scene3d ) },
    { OUString("props3d"),      FSNS( XML_w14, XML_props3d ) },
    { OUString("ligatures"),    FSNS( XML_w14, XML_ligatures ) },
    { OUString("numForm"),      FSNS( XML_w14, XML_numForm ) },
    { OUString("numSpacing"),   FSNS( XML_w14, XML_numSpacing ) },
    { OUString("stylisticSets"),FSNS( XML_w14, XML_stylisticSets ) },
    { OUString("cntxtAlts"),    FSNS( XML_w14, XML_cntxtAlts ) },

    { OUString("val"),          FSNS( XML_w14, XML_val ) },
    { OUString("rad"),          FSNS( XML_w14, XML_rad ) },
    { OUString("blurRad"),      FSNS( XML_w14, XML_blurRad ) },
    { OUString("stA"),          FSNS( XML_w14, XML_stA ) },
    { OUString("stPos"),        FSNS( XML_w14, XML_stPos ) },
    { OUString("endA"),         FSNS( XML_w14, XML_endA ) },
    { OUString("endPos"),       FSNS( XML_w14, XML_endPos ) },
    { OUString("dist"),         FSNS( XML_w14, XML_dist ) },
    { OUString("dir"),          FSNS( XML_w14, XML_dir ) },
    { OUString("fadeDir"),      FSNS( XML_w14, XML_fadeDir ) },
    { OUString("sx"),           FSNS( XML_w14, XML_sx ) },
    { OUString("sy"),           FSNS( XML_w14, XML_sy ) },
    { OUString("kx"),           FSNS( XML_w14, XML_kx ) },
    { OUString("ky"),           FSNS( XML_w14, XML_ky ) },
    { OUString("algn"),         FSNS( XML_w14, XML_algn ) },
    { OUString("w"),            FSNS( XML_w14, XML_w ) },
    { OUString("cap"),          FSNS( XML_w14, XML_cap ) },
    { OUString("cmpd"),         FSNS( XML_w14, XML_cmpd ) },
    { OUString("pos"),          FSNS( XML_w14, XML_pos ) },
    { OUString("ang"),          FSNS( XML_w14, XML_ang ) },
    { OUString("scaled"),       FSNS( XML_w14, XML_scaled ) },
    { OUString("path"),         FSNS( XML_w14, XML_path ) },
    { OUString("l"),            FSNS( XML_w14, XML_l ) },
    { OUString("t"),            FSNS( XML_w14, XML_t ) },
    { OUString("r"),            FSNS( XML_w14, XML_r ) },
    { OUString("b"),            FSNS( XML_w14, XML_b ) },
    { OUString("lim"),          FSNS( XML_w14, XML_lim ) },
    { OUString("prst"),         FSNS( XML_w14, XML_prst ) },
    { OUString("rig"),          FSNS( XML_w14, XML_rig ) },
    { OUString("lat"),          FSNS( XML_w14, XML_lat ) },
    { OUString("lon"),          FSNS( XML_w14, XML_lon ) },
    { OUString("rev"),          FSNS( XML_w14, XML_rev ) },
    { OUString("h"),            FSNS( XML_w14, XML_h ) },
    { OUString("extrusionH"),   FSNS( XML_w14, XML_extrusionH ) },
    { OUString("contourW"),     FSNS( XML_w14, XML_contourW ) },
    { OUString("prstMaterial"), FSNS( XML_w14, XML_prstMaterial ) },
    { OUString("id"),           FSNS( XML_w14, XML_id ) },

    { OUString("schemeClr"),    FSNS( XML_w14, XML_schemeClr ) },
    { OUString("srgbClr"),      FSNS( XML_w14, XML_srgbClr ) },
    { OUString("tint"),         FSNS( XML_w14, XML_tint ) },
    { OUString("shade"),        FSNS( XML_w14, XML_shade ) },
    { OUString("alpha"),        FSNS( XML_w14, XML_alpha ) },
    { OUString("hueMod"),       FSNS( XML_w14, XML_hueMod ) },
    { OUString("sat"),          FSNS( XML_w14, XML_sat ) },
    { OUString("satOff"),       FSNS( XML_w14, XML_satOff ) },
    { OUString("satMod"),       FSNS( XML_w14, XML_satMod ) },
    { OUString("lum"),          FSNS( XML_w14, XML_lum ) },
    { OUString("lumOff"),       FSNS( XML_w14, XML_lumOff ) },
    { OUString("lumMod"),       FSNS( XML_w14, XML_lumMod ) },
    { OUString("noFill"),       FSNS( XML_w14, XML_noFill ) },
    { OUString("solidFill"),    FSNS( XML_w14, XML_solidFill ) },
    { OUString("gradFill"),     FSNS( XML_w14, XML_gradFill ) },
    { OUString("gsLst"),        FSNS( XML_w14, XML_gsLst ) },
    { OUString("gs"),           FSNS( XML_w14, XML_gs ) },
    { OUString("pos"),          FSNS( XML_w14, XML_pos ) },
    { OUString("lin"),          FSNS( XML_w14, XML_lin ) },
    { OUString("path"),         FSNS( XML_w14, XML_path ) },
    { OUString("fillToRect"),   FSNS( XML_w14, XML_fillToRect ) },
    { OUString("prstDash"),     FSNS( XML_w14, XML_prstDash ) },
    { OUString("round"),        FSNS( XML_w14, XML_round ) },
    { OUString("bevel"),        FSNS( XML_w14, XML_bevel ) },
    { OUString("miter"),        FSNS( XML_w14, XML_miter ) },
    { OUString("camera"),       FSNS( XML_w14, XML_camera ) },
    { OUString("lightRig"),     FSNS( XML_w14, XML_lightRig ) },
    { OUString("rot"),          FSNS( XML_w14, XML_rot ) },
    { OUString("bevelT"),       FSNS( XML_w14, XML_bevelT ) },
    { OUString("bevelB"),       FSNS( XML_w14, XML_bevelB ) },
    { OUString("extrusionClr"), FSNS( XML_w14, XML_extrusionClr ) },
    { OUString("contourClr"),   FSNS( XML_w14, XML_contourClr ) },
    { OUString("styleSet"),     FSNS( XML_w14, XML_styleSet ) },
};

std::optional<sal_Int32> lclGetElementIdForName(std::u16string_view rName)
{
    for (auto const & i : constNameToIdMapping)
    {
        if (rName == i.maName)
        {
            return i.maId;
        }
    }
    return std::optional<sal_Int32>();
}

void lclProcessRecursiveGrabBag(sal_Int32 aElementId, const css::uno::Sequence<css::beans::PropertyValue>& rElements, sax_fastparser::FSHelperPtr const & pSerializer)
{
    css::uno::Sequence<css::beans::PropertyValue> aAttributes;
    rtl::Reference<FastAttributeList> pAttributes = FastSerializerHelper::createAttrList();

    for (const auto& rElement : rElements)
    {
        if (rElement.Name == "attributes")
        {
            rElement.Value >>= aAttributes;
        }
    }

    for (const auto& rAttribute : std::as_const(aAttributes))
    {
        uno::Any aAny = rAttribute.Value;
        OString aValue;

        if(aAny.getValueType() == cppu::UnoType<sal_Int32>::get())
        {
            aValue = OString::number(aAny.get<sal_Int32>());
        }
        else if(aAny.getValueType() == cppu::UnoType<OUString>::get())
        {
            aValue =  OUStringToOString(aAny.get<OUString>(), RTL_TEXTENCODING_ASCII_US);
        }

        std::optional<sal_Int32> aSubElementId = lclGetElementIdForName(rAttribute.Name);
        if(aSubElementId)
            pAttributes->add(*aSubElementId, aValue);
    }

    pSerializer->startElement(aElementId, pAttributes);

    for (const auto& rElement : rElements)
    {
        css::uno::Sequence<css::beans::PropertyValue> aSumElements;

        std::optional<sal_Int32> aSubElementId = lclGetElementIdForName(rElement.Name);
        if(aSubElementId)
        {
            rElement.Value >>= aSumElements;
            lclProcessRecursiveGrabBag(*aSubElementId, aSumElements, pSerializer);
        }
    }

    pSerializer->endElement(aElementId);
}

}

void DocxAttributeOutput::WriteCollectedRunProperties()
{
    // Write all differed properties
    if ( m_pFontsAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pFontsAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_rFonts, xAttrList );
    }

    if ( m_pColorAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList( m_pColorAttrList );

        m_pSerializer->singleElementNS( XML_w, XML_color, xAttrList );
    }

    if ( m_pEastAsianLayoutAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pEastAsianLayoutAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_eastAsianLayout, xAttrList );
    }

    if ( m_pCharLangAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pCharLangAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_lang, xAttrList );
    }

    if (m_nCharTransparence != 0 && m_pColorAttrList && m_aTextEffectsGrabBag.empty())
    {
        const char* pVal = nullptr;
        m_pColorAttrList->getAsChar(FSNS(XML_w, XML_val), pVal);
        if (std::string_view("auto") != pVal)
        {
            m_pSerializer->startElementNS(XML_w14, XML_textFill);
            m_pSerializer->startElementNS(XML_w14, XML_solidFill);
            m_pSerializer->startElementNS(XML_w14, XML_srgbClr, FSNS(XML_w14, XML_val), pVal);
            sal_Int32 nTransparence = m_nCharTransparence * oox::drawingml::MAX_PERCENT / 255.0;
            m_pSerializer->singleElementNS(XML_w14, XML_alpha, FSNS(XML_w14, XML_val), OString::number(nTransparence));
            m_pSerializer->endElementNS(XML_w14, XML_srgbClr);
            m_pSerializer->endElementNS(XML_w14, XML_solidFill);
            m_pSerializer->endElementNS(XML_w14, XML_textFill);
            m_nCharTransparence = 0;
        }
    }
    m_pColorAttrList.clear();
    for (const beans::PropertyValue & i : m_aTextEffectsGrabBag)
    {
        std::optional<sal_Int32> aElementId = lclGetElementIdForName(i.Name);
        if(aElementId)
        {
            uno::Sequence<beans::PropertyValue> aGrabBagSeq;
            i.Value >>= aGrabBagSeq;
            lclProcessRecursiveGrabBag(*aElementId, aGrabBagSeq, m_pSerializer);
        }
    }
    m_aTextEffectsGrabBag.clear();
}

void DocxAttributeOutput::EndRunProperties( const SwRedlineData* pRedlineData )
{
    // Call the 'Redline' function. This will add redline (change-tracking) information that regards to run properties.
    // This includes changes like 'Bold', 'Underline', 'Strikethrough' etc.

    // If there is RedlineData present, call WriteCollectedRunProperties() for writing rPr before calling Redline().
    // As there will be another rPr for redline and LO might mix both.
    if(pRedlineData)
        WriteCollectedRunProperties();
    Redline( pRedlineData );

    WriteCollectedRunProperties();

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_InitCollectedRunProperties);

    m_pSerializer->endElementNS( XML_w, XML_rPr );

    // write footnotes/endnotes if we have any
    FootnoteEndnoteReference();

    // merge the properties _before_ the run text (strictly speaking, just
    // after the start of the run)
    m_pSerializer->mergeTopMarks(Tag_StartRunProperties, sax_fastparser::MergeMarks::PREPEND);

    WritePostponedGraphic();

    WritePostponedDiagram();
    //We need to write w:drawing tag after the w:rPr.
    WritePostponedChart();

    //We need to write w:pict tag after the w:rPr.
    WritePostponedDMLDrawing();

    WritePostponedOLE();

    WritePostponedActiveXControl(true);
}

void DocxAttributeOutput::GetSdtEndBefore(const SdrObject* pSdrObj)
{
    if (!pSdrObj)
        return;

    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pSdrObj)->getUnoShape());
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( !xPropSet.is() )
        return;

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    uno::Sequence< beans::PropertyValue > aGrabBag;
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("FrameInteropGrabBag"))
    {
        xPropSet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
    }
    else if(xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("InteropGrabBag"))
    {
        xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    }

    auto pProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
        [this](const beans::PropertyValue& rProp) {
            return "SdtEndBefore" == rProp.Name && m_bStartedCharSdt && !m_bEndCharSdt; });
    if (pProp != std::cend(aGrabBag))
        pProp->Value >>= m_bEndCharSdt;
}

void DocxAttributeOutput::WritePostponedGraphic()
{
    for (const auto & rPostponedDiagram : *m_pPostponedGraphic)
        FlyFrameGraphic(rPostponedDiagram.grfNode, rPostponedDiagram.size,
            nullptr, nullptr,
            rPostponedDiagram.pSdrObj);
    m_pPostponedGraphic.reset();
}

void DocxAttributeOutput::WritePostponedDiagram()
{
    for( const auto & rPostponedDiagram : *m_pPostponedDiagrams )
        m_rExport.SdrExporter().writeDiagram(rPostponedDiagram.object,
            *rPostponedDiagram.frame, m_anchorId++);
    m_pPostponedDiagrams.reset();
}

bool DocxAttributeOutput::FootnoteEndnoteRefTag()
{
    if( m_footnoteEndnoteRefTag == 0 )
        return false;

    // output the character style for MS Word's benefit
    const SwEndNoteInfo& rInfo = m_footnoteEndnoteRefTag == XML_footnoteRef ?
        m_rExport.m_rDoc.GetFootnoteInfo() : m_rExport.m_rDoc.GetEndNoteInfo();
    const SwCharFormat* pCharFormat = rInfo.GetCharFormat( m_rExport.m_rDoc );
    if ( pCharFormat )
    {
        const OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(pCharFormat)));
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        m_pSerializer->singleElementNS(XML_w, XML_rStyle, FSNS(XML_w, XML_val), aStyleId);
        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }

    if (m_footnoteCustomLabel.isEmpty())
        m_pSerializer->singleElementNS(XML_w, m_footnoteEndnoteRefTag);
    else
        RunText(m_footnoteCustomLabel);
    m_footnoteEndnoteRefTag = 0;
    return true;
}

/** Output sal_Unicode* as a run text (<t>the text</t>).

    When bMove is true, update rBegin to point _after_ the end of the text +
    1, meaning that it skips one character after the text.  This is to make
    the switch in DocxAttributeOutput::RunText() nicer ;-)
 */
static bool impl_WriteRunText( FSHelperPtr const & pSerializer, sal_Int32 nTextToken,
        const sal_Unicode* &rBegin, const sal_Unicode* pEnd, bool bMove = true )
{
    const sal_Unicode *pBegin = rBegin;

    // skip one character after the end
    if ( bMove )
        rBegin = pEnd + 1;

    if ( pBegin >= pEnd )
        return false; // we want to write at least one character

    // we have to add 'preserve' when starting/ending with space
    if ( *pBegin == ' ' || *( pEnd - 1 ) == ' ' )
    {
        pSerializer->startElementNS(XML_w, nTextToken, FSNS(XML_xml, XML_space), "preserve");
    }
    else
        pSerializer->startElementNS(XML_w, nTextToken);

    pSerializer->writeEscaped( std::u16string_view( pBegin, pEnd - pBegin ) );

    pSerializer->endElementNS( XML_w, nTextToken );

    return true;
}

void DocxAttributeOutput::RunText( const OUString& rText, rtl_TextEncoding /*eCharSet*/ )
{
    if( m_closeHyperlinkInThisRun )
    {
        m_closeHyperlinkInPreviousRun = true;
    }
    m_bRunTextIsOn = true;
    // one text can be split into more <w:t>blah</w:t>'s by line breaks etc.
    const sal_Unicode *pBegin = rText.getStr();
    const sal_Unicode *pEnd = pBegin + rText.getLength();

    // the text run is usually XML_t, with the exception of the deleted (and not moved) text
    sal_Int32 nTextToken = XML_t;
    if ( m_pRedlineData && !m_pRedlineData->IsMoved() &&
            m_pRedlineData->GetType() == RedlineType::Delete )
    {
        nTextToken = XML_delText;
    }

    sal_Unicode prevUnicode = *pBegin;

    for ( const sal_Unicode *pIt = pBegin; pIt < pEnd; ++pIt )
    {
        switch ( *pIt )
        {
            case 0x09: // tab
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS(XML_w, XML_tab);
                prevUnicode = *pIt;
                break;
            case 0x0b: // line break
                {
                    if (impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt ) || prevUnicode < 0x0020)
                    {
                        m_pSerializer->singleElementNS(XML_w, XML_br);
                        prevUnicode = *pIt;
                    }
                }
                break;
            case 0x1E: //non-breaking hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS(XML_w, XML_noBreakHyphen);
                prevUnicode = *pIt;
                break;
            case 0x1F: //soft (on demand) hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS(XML_w, XML_softHyphen);
                prevUnicode = *pIt;
                break;
            default:
                if ( *pIt < 0x0020 ) // filter out the control codes
                {
                    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                    SAL_INFO("sw.ww8", "Ignored control code in a text run: " << unsigned(*pIt) );
                }
                prevUnicode = *pIt;
                break;
        }
    }

    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pEnd, false );
}

void DocxAttributeOutput::RawText(const OUString& rText, rtl_TextEncoding /*eCharSet*/)
{
    m_sRawText = rText;
}

void DocxAttributeOutput::StartRuby( const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby )
{
    WW8Ruby aWW8Ruby( rNode, rRuby, GetExport() );
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::StartRuby( const SwTextNode& rNode, const SwFormatRuby& rRuby )" );
    EndRun( &rNode, nPos ); // end run before starting ruby to avoid nested runs, and overlap
    assert(!m_closeHyperlinkInThisRun); // check that no hyperlink overlaps ruby
    assert(!m_closeHyperlinkInPreviousRun);
    m_pSerializer->startElementNS(XML_w, XML_r);
    m_pSerializer->startElementNS(XML_w, XML_ruby);
    m_pSerializer->startElementNS(XML_w, XML_rubyPr);

    m_pSerializer->singleElementNS( XML_w, XML_rubyAlign,
            FSNS( XML_w, XML_val ), lclConvertWW8JCToOOXMLRubyAlign(aWW8Ruby.GetJC()) );
    sal_uInt32   nHps = (aWW8Ruby.GetRubyHeight() + 5) / 10;
    sal_uInt32   nHpsBaseText = (aWW8Ruby.GetBaseHeight() + 5) / 10;
    m_pSerializer->singleElementNS(XML_w, XML_hps, FSNS(XML_w, XML_val), OString::number(nHps));

    m_pSerializer->singleElementNS( XML_w, XML_hpsRaise,
            FSNS( XML_w, XML_val ), OString::number(nHpsBaseText) );

    m_pSerializer->singleElementNS( XML_w, XML_hpsBaseText,
            FSNS( XML_w, XML_val ), OString::number(nHpsBaseText) );

    lang::Locale aLocale( SwBreakIt::Get()->GetLocale(
                rNode.GetLang( nPos ) ) );
    OUString sLang( LanguageTag::convertToBcp47( aLocale) );
    m_pSerializer->singleElementNS(XML_w, XML_lid, FSNS(XML_w, XML_val), sLang);

    m_pSerializer->endElementNS( XML_w, XML_rubyPr );

    m_pSerializer->startElementNS(XML_w, XML_rt);
    StartRun( nullptr, nPos );
    StartRunProperties( );

    if (rRuby.GetTextRuby() && rRuby.GetTextRuby()->GetCharFormat())
    {
        const SwCharFormat* pFormat = rRuby.GetTextRuby()->GetCharFormat();
        sal_uInt16 nScript = g_pBreakIt->GetBreakIter()->getScriptType(rRuby.GetText(), 0);
        sal_uInt16 nWhichFont = (nScript == i18n::ScriptType::LATIN) ?  RES_CHRATR_FONT : RES_CHRATR_CJK_FONT;
        sal_uInt16 nWhichFontSize = (nScript == i18n::ScriptType::LATIN) ?  RES_CHRATR_FONTSIZE : RES_CHRATR_CJK_FONTSIZE;

        CharFont(ItemGet<SvxFontItem>(*pFormat, nWhichFont));
        CharFontSize(ItemGet<SvxFontHeightItem>(*pFormat, nWhichFontSize));
        CharFontSize(ItemGet<SvxFontHeightItem>(*pFormat, RES_CHRATR_CTL_FONTSIZE));
    }

    EndRunProperties( nullptr );
    RunText( rRuby.GetText( ) );
    EndRun( &rNode, nPos );
    m_pSerializer->endElementNS( XML_w, XML_rt );

    m_pSerializer->startElementNS(XML_w, XML_rubyBase);
    StartRun( nullptr, nPos );
}

void DocxAttributeOutput::EndRuby(const SwTextNode& rNode, sal_Int32 nPos)
{
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::EndRuby()" );
    EndRun( &rNode, nPos );
    m_pSerializer->endElementNS( XML_w, XML_rubyBase );
    m_pSerializer->endElementNS( XML_w, XML_ruby );
    m_pSerializer->endElementNS( XML_w, XML_r );
    StartRun(nullptr, nPos); // open Run again so OutputTextNode loop can close it
}

bool DocxAttributeOutput::AnalyzeURL( const OUString& rUrl, const OUString& rTarget, OUString* pLinkURL, OUString* pMark )
{
    bool bBookMarkOnly = AttributeOutputBase::AnalyzeURL( rUrl, rTarget, pLinkURL, pMark );

    if ( !pMark->isEmpty() )
    {
        OUString sURL = *pLinkURL;

        if ( bBookMarkOnly )
            sURL = FieldString( ww::eHYPERLINK );
        else
            sURL = FieldString( ww::eHYPERLINK ) + "\"" + sURL + "\"";

        sURL += " \\l \"" + *pMark + "\"";

        if ( !rTarget.isEmpty() )
            sURL += " \\n " + rTarget;

        *pLinkURL = sURL;
    }

    return bBookMarkOnly;
}

void DocxAttributeOutput::WriteBookmarkInActParagraph( const OUString& rName, sal_Int32 nFirstRunPos, sal_Int32 nLastRunPos )
{
    m_aBookmarksOfParagraphStart.insert(std::pair<sal_Int32, OUString>(nFirstRunPos, rName));
    m_aBookmarksOfParagraphEnd.insert(std::pair<sal_Int32, OUString>(nLastRunPos, rName));
}

bool DocxAttributeOutput::StartURL( const OUString& rUrl, const OUString& rTarget )
{
    OUString sMark;
    OUString sUrl;

    bool bBookmarkOnly = AnalyzeURL( rUrl, rTarget, &sUrl, &sMark );

    m_hyperLinkAnchor = sMark;

    if ( !sMark.isEmpty() && !bBookmarkOnly )
    {
        m_rExport.OutputField( nullptr, ww::eHYPERLINK, sUrl );
    }
    else
    {
        // Output a hyperlink XML element
        m_pHyperlinkAttrList = FastSerializerHelper::createAttrList();

        if ( !bBookmarkOnly )
        {
            OString sId = OUStringToOString( GetExport().GetFilter().addRelation( m_pSerializer->getOutputStream(),
                        oox::getRelationship(Relationship::HYPERLINK),
                        sUrl, true ), RTL_TEXTENCODING_UTF8 );

            m_pHyperlinkAttrList->add(FSNS(XML_r, XML_id), sId);
        }
        else
        {
            // Is this a link to a sequence? Then try to replace that with a
            // normal bookmark, as Word won't understand our special
            // <seqname>!<index>|sequence syntax.
            if (sMark.endsWith("|sequence"))
            {
                sal_Int32 nPos = sMark.indexOf('!');
                if (nPos != -1)
                {
                    // Extract <seqname>, the field instruction text has the name quoted.
                    OUString aSequenceName = sMark.copy(0, nPos);
                    // Extract <index>.
                    sal_uInt32 nIndex = sMark.copy(nPos + 1, sMark.getLength() - nPos - sizeof("|sequence")).toUInt32();
                    std::map<OUString, std::vector<OString> >::iterator it = m_aSeqBookmarksNames.find(aSequenceName);
                    if (it != m_aSeqBookmarksNames.end())
                    {
                        std::vector<OString>& rNames = it->second;
                        if (rNames.size() > nIndex)
                            // We know the bookmark name for this sequence and this index, do the replacement.
                            sMark = OStringToOUString(rNames[nIndex], RTL_TEXTENCODING_UTF8);
                    }
                }
            }
            else if (sMark.endsWith("|toxmark"))
            {
                if (auto const it = GetExport().m_TOXMarkBookmarksByURL.find(sMark);
                    it != GetExport().m_TOXMarkBookmarksByURL.end())
                {
                    sMark = it->second;
                }
            }
            // Spaces are prohibited in bookmark name.
            sMark = sMark.replace(' ', '_');
            m_pHyperlinkAttrList->add( FSNS( XML_w, XML_anchor ),
                    OUStringToOString( sMark, RTL_TEXTENCODING_UTF8 ) );
        }

        if ( !rTarget.isEmpty() )
        {
            OString soTarget = OUStringToOString( rTarget, RTL_TEXTENCODING_UTF8 );
            m_pHyperlinkAttrList->add(FSNS(XML_w, XML_tgtFrame), soTarget);
        }
    }

    return true;
}

bool DocxAttributeOutput::EndURL(bool const)
{
    m_closeHyperlinkInThisRun = true;
    if(m_startedHyperlink && !m_hyperLinkAnchor.isEmpty() && m_hyperLinkAnchor.startsWith("_Toc"))
    {
        m_endPageRef = true;
    }
    return true;
}

void DocxAttributeOutput::FieldVanish(const OUString& rText,
        ww::eField const eType, OUString const*const pBookmarkName)
{
    WriteField_Impl(nullptr, eType, rText, FieldFlags::All, pBookmarkName);
}

// The difference between 'Redline' and 'StartRedline'+'EndRedline' is that:
// 'Redline' is used for tracked changes of formatting information of a run like Bold, Underline. (the '<w:rPrChange>' is inside the 'run' node)
// 'StartRedline' is used to output tracked changes of run insertion and deletion (the run is inside the '<w:ins>' node)
void DocxAttributeOutput::Redline( const SwRedlineData* pRedlineData)
{
    if ( !pRedlineData )
        return;

    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
        SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

    OString aId( OString::number( pRedlineData->GetSeqNo() ) );
    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aDate( DateTimeToOString( bRemovePersonalInfo
            ? DateTime(Date( 1, 1, 1970 )) // Epoch time
            : pRedlineData->GetTimeStamp() ) );

    switch( pRedlineData->GetType() )
    {
    case RedlineType::Insert:
        break;

    case RedlineType::Delete:
        break;

    case RedlineType::Format:
        m_pSerializer->startElementNS( XML_w, XML_rPrChange,
                FSNS( XML_w, XML_id ), aId,
                FSNS( XML_w, XML_author ), bRemovePersonalInfo
                    ? "Author" + OUString::number( GetExport().GetInfoID(rAuthor) )
                    : rAuthor,
                FSNS( XML_w, XML_date ), aDate );

        // Check if there is any extra data stored in the redline object
        if (pRedlineData->GetExtraData())
        {
            const SwRedlineExtraData* pExtraData = pRedlineData->GetExtraData();
            const SwRedlineExtraData_FormatColl* pFormattingChanges = dynamic_cast<const SwRedlineExtraData_FormatColl*>(pExtraData);

            // Check if the extra data is of type 'formatting changes'
            if (pFormattingChanges)
            {
                 // Get the item set that holds all the changes properties
                const SfxItemSet *pChangesSet = pFormattingChanges->GetItemSet();
                if (pChangesSet)
                {
                    m_pSerializer->mark(Tag_Redline_1);

                    m_pSerializer->startElementNS(XML_w, XML_rPr);

                    // Output the redline item set
                    if (pChangesSet)
                        m_rExport.OutputItemSet( *pChangesSet, false, true, i18n::ScriptType::LATIN, m_rExport.m_bExportModeRTF );

                    m_pSerializer->endElementNS( XML_w, XML_rPr );

                    m_pSerializer->mergeTopMarks(Tag_Redline_1, sax_fastparser::MergeMarks::PREPEND);
                }
            }
        }

        m_pSerializer->endElementNS( XML_w, XML_rPrChange );
        break;

    case RedlineType::ParagraphFormat:
        m_pSerializer->startElementNS( XML_w, XML_pPrChange,
                FSNS( XML_w, XML_id ), aId,
                FSNS( XML_w, XML_author ), bRemovePersonalInfo
                    ? "Author" + OUString::number( GetExport().GetInfoID(rAuthor) )
                    : rAuthor,
                FSNS( XML_w, XML_date ), aDate );

        // Check if there is any extra data stored in the redline object
        if (pRedlineData->GetExtraData())
        {
            const SwRedlineExtraData* pExtraData = pRedlineData->GetExtraData();
            const SwRedlineExtraData_FormatColl* pFormattingChanges = dynamic_cast<const SwRedlineExtraData_FormatColl*>(pExtraData);

            // Check if the extra data is of type 'formatting changes'
            if (pFormattingChanges)
            {
                // Get the item set that holds all the changes properties
                const SfxItemSet *pChangesSet = pFormattingChanges->GetItemSet();
                const OUString & sParaStyleName = pFormattingChanges->GetFormatName();
                if (pChangesSet || !sParaStyleName.isEmpty())
                {
                    m_pSerializer->mark(Tag_Redline_2);

                    m_pSerializer->startElementNS(XML_w, XML_pPr);

                    OString sStyleName = MSWordStyles::CreateStyleId( sParaStyleName );
                    if ( !sStyleName.isEmpty() )
                        m_pSerializer->singleElementNS(XML_w, XML_pStyle, FSNS(XML_w, XML_val), sStyleName);

                    // The 'm_rExport.SdrExporter().getFlyAttrList()', 'm_pParagraphSpacingAttrList' are used to hold information
                    // that should be collected by different properties in the core, and are all flushed together
                    // to the DOCX when the function 'WriteCollectedParagraphProperties' gets called.
                    // So we need to store the current status of these lists, so that we can revert back to them when
                    // we are done exporting the redline attributes.
                    rtl::Reference<sax_fastparser::FastAttributeList> pFlyAttrList_Original(m_rExport.SdrExporter().getFlyAttrList());
                    m_rExport.SdrExporter().getFlyAttrList().clear();
                    rtl::Reference<sax_fastparser::FastAttributeList> pParagraphSpacingAttrList_Original(m_pParagraphSpacingAttrList);
                    m_pParagraphSpacingAttrList.clear();

                    // Output the redline item set
                    if (pChangesSet)
                        m_rExport.OutputItemSet( *pChangesSet, true, false, i18n::ScriptType::LATIN, m_rExport.m_bExportModeRTF );

                    // Write the collected paragraph properties that are stored in 'm_rExport.SdrExporter().getFlyAttrList()', 'm_pParagraphSpacingAttrList'
                    WriteCollectedParagraphProperties();

                    // Revert back the original values that were stored in 'm_rExport.SdrExporter().getFlyAttrList()', 'm_pParagraphSpacingAttrList'
                    m_rExport.SdrExporter().getFlyAttrList() = pFlyAttrList_Original;
                    m_pParagraphSpacingAttrList = pParagraphSpacingAttrList_Original;

                    m_pSerializer->endElementNS( XML_w, XML_pPr );

                    m_pSerializer->mergeTopMarks(Tag_Redline_2, sax_fastparser::MergeMarks::PREPEND);
                }
            }
        }
        m_pSerializer->endElementNS( XML_w, XML_pPrChange );
        break;

    default:
        SAL_WARN("sw.ww8", "Unhandled redline type for export " << SwRedlineTypeToOUString(pRedlineData->GetType()));
        break;
    }
}

// The difference between 'Redline' and 'StartRedline'+'EndRedline' is that:
// 'Redline' is used for tracked changes of formatting information of a run like Bold, Underline. (the '<w:rPrChange>' is inside the 'run' node)
// 'StartRedline' is used to output tracked changes of run insertion and deletion (the run is inside the '<w:ins>' node)
void DocxAttributeOutput::StartRedline( const SwRedlineData * pRedlineData )
{
    if ( !pRedlineData )
        return;

    // write out stack of this redline recursively (first the oldest)
    StartRedline( pRedlineData->Next() );

    OString aId( OString::number( m_nRedlineId++ ) );

    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
        SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aAuthor( OUStringToOString( bRemovePersonalInfo
                        ? "Author" + OUString::number( GetExport().GetInfoID(rAuthor) )
                        : rAuthor, RTL_TEXTENCODING_UTF8 ) );

    OString aDate( DateTimeToOString( bRemovePersonalInfo
            ? DateTime(Date( 1, 1, 1970 )) // Epoch time
            : pRedlineData->GetTimeStamp() ) );

    bool bMoved = pRedlineData->IsMoved();
    switch ( pRedlineData->GetType() )
    {
        case RedlineType::Insert:
            m_pSerializer->startElementNS( XML_w, bMoved ? XML_moveTo : XML_ins,
                    FSNS( XML_w, XML_id ), aId,
                    FSNS( XML_w, XML_author ), aAuthor,
                    FSNS( XML_w, XML_date ), aDate );
            break;

        case RedlineType::Delete:
            m_pSerializer->startElementNS( XML_w, bMoved ? XML_moveFrom : XML_del,
                    FSNS( XML_w, XML_id ), aId,
                    FSNS( XML_w, XML_author ), aAuthor,
                    FSNS( XML_w, XML_date ), aDate );
            break;

        case RedlineType::Format:
            SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::StartRedline()" );
            break;
        default:
            break;
    }
}

void DocxAttributeOutput::EndRedline( const SwRedlineData * pRedlineData )
{
    if ( !pRedlineData || m_bWritingField )
        return;

    bool bMoved = pRedlineData->IsMoved();
    switch ( pRedlineData->GetType() )
    {
        case RedlineType::Insert:
            m_pSerializer->endElementNS( XML_w, bMoved ? XML_moveTo : XML_ins );
            break;

        case RedlineType::Delete:
            m_pSerializer->endElementNS( XML_w, bMoved ? XML_moveFrom : XML_del );
            break;

        case RedlineType::Format:
            SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::EndRedline()" );
            break;
        default:
            break;
    }

    // write out stack of this redline recursively (first the newest)
    EndRedline( pRedlineData->Next() );
}

void DocxAttributeOutput::FormatDrop( const SwTextNode& /*rNode*/, const SwFormatDrop& /*rSwFormatDrop*/, sal_uInt16 /*nStyle*/, ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/, ww8::WW8TableNodeInfoInner::Pointer_t )
{
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::FormatDrop( const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle )" );
}

void DocxAttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OString aStyleId(m_rExport.m_pStyles->GetStyleId(nStyle));

    m_pSerializer->singleElementNS(XML_w, XML_pStyle, FSNS(XML_w, XML_val), aStyleId);
}

static void impl_borderLine( FSHelperPtr const & pSerializer, sal_Int32 elementToken, const SvxBorderLine* pBorderLine, sal_uInt16 nDist,
                             bool bWriteShadow, const table::BorderLine2* rStyleProps = nullptr )
{
    // Compute val attribute value
    // Can be one of:
    //      single, double,
    //      basicWideOutline, basicWideInline
    // OOXml also supports those types of borders, but we'll try to play with the first ones.
    //      thickThinMediumGap, thickThinLargeGap, thickThinSmallGap
    //      thinThickLargeGap, thinThickMediumGap, thinThickSmallGap
    const char* pVal = "nil";
    if ( pBorderLine && !pBorderLine->isEmpty( ) )
    {
        switch (pBorderLine->GetBorderLineStyle())
        {
            case SvxBorderLineStyle::SOLID:
                pVal = "single";
                break;
            case SvxBorderLineStyle::DOTTED:
                pVal = "dotted";
                break;
            case SvxBorderLineStyle::DASHED:
                pVal = "dashed";
                break;
            case SvxBorderLineStyle::DOUBLE:
            case SvxBorderLineStyle::DOUBLE_THIN:
                pVal = "double";
                break;
            case SvxBorderLineStyle::THINTHICK_SMALLGAP:
                pVal = "thinThickSmallGap";
                break;
            case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
                pVal = "thinThickMediumGap";
                break;
            case SvxBorderLineStyle::THINTHICK_LARGEGAP:
                pVal = "thinThickLargeGap";
                break;
            case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
                pVal = "thickThinSmallGap";
                break;
            case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
                pVal = "thickThinMediumGap";
                break;
            case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
                pVal = "thickThinLargeGap";
                break;
            case SvxBorderLineStyle::EMBOSSED:
                pVal = "threeDEmboss";
                break;
            case SvxBorderLineStyle::ENGRAVED:
                pVal = "threeDEngrave";
                break;
            case SvxBorderLineStyle::OUTSET:
                pVal = "outset";
                break;
            case SvxBorderLineStyle::INSET:
                pVal = "inset";
                break;
            case SvxBorderLineStyle::FINE_DASHED:
                pVal = "dashSmallGap";
                break;
            case SvxBorderLineStyle::DASH_DOT:
                pVal = "dotDash";
                break;
            case SvxBorderLineStyle::DASH_DOT_DOT:
                pVal = "dotDotDash";
                break;
            case SvxBorderLineStyle::NONE:
            default:
                break;
        }
    }
    else if ( !rStyleProps || !rStyleProps->LineWidth )
        // no line, and no line set by the style either:
        // there is no need to write the property
        return;

    // compare the properties with the theme properties before writing them:
    // if they are equal, it means that they were style-defined and there is
    // no need to write them.
    if( rStyleProps != nullptr && pBorderLine && !pBorderLine->isEmpty() &&
            pBorderLine->GetBorderLineStyle() == static_cast<SvxBorderLineStyle>(rStyleProps->LineStyle) &&
            pBorderLine->GetColor() == Color(ColorTransparency, rStyleProps->Color) &&
            pBorderLine->GetWidth() == o3tl::toTwips(rStyleProps->LineWidth, o3tl::Length::mm100) )
        return;

    rtl::Reference<FastAttributeList> pAttr = FastSerializerHelper::createAttrList();
    pAttr->add( FSNS( XML_w, XML_val ), OString( pVal ) );

    if ( pBorderLine && !pBorderLine->isEmpty() )
    {
        // Compute the sz attribute

        double const fConverted( ::editeng::ConvertBorderWidthToWord(
                pBorderLine->GetBorderLineStyle(), pBorderLine->GetWidth()));
        // The unit is the 8th of point
        sal_Int32 nWidth = sal_Int32( fConverted / 2.5 );
        const sal_Int32 nMinWidth = 2;
        const sal_Int32 nMaxWidth = 96;

        if ( nWidth > nMaxWidth )
            nWidth = nMaxWidth;
        else if ( nWidth < nMinWidth )
            nWidth = nMinWidth;

        pAttr->add( FSNS( XML_w, XML_sz ), OString::number( nWidth ) );

        // Get the distance (in pt)
        pAttr->add(FSNS(XML_w, XML_space), OString::number(rtl::math::round(nDist / 20.0)));

        // Get the color code as an RRGGBB hex value
        OString sColor( msfilter::util::ConvertColor( pBorderLine->GetColor( ) ) );
        pAttr->add( FSNS( XML_w, XML_color ), sColor );
    }

    if (bWriteShadow)
    {
        // Set the shadow value
        pAttr->add( FSNS( XML_w, XML_shadow ), "1" );
    }

    pSerializer->singleElementNS( XML_w, elementToken, pAttr );
}

static OutputBorderOptions lcl_getTableCellBorderOptions(bool bEcma)
{
    OutputBorderOptions rOptions;

    rOptions.tag = XML_tcBorders;
    rOptions.bUseStartEnd = !bEcma;
    rOptions.bWriteTag = true;
    rOptions.bWriteDistance = false;

    return rOptions;
}

static OutputBorderOptions lcl_getBoxBorderOptions()
{
    OutputBorderOptions rOptions;

    rOptions.tag = XML_pBdr;
    rOptions.bUseStartEnd = false;
    rOptions.bWriteTag = false;
    rOptions.bWriteDistance = true;

    return rOptions;
}

static void impl_borders( FSHelperPtr const & pSerializer,
                          const SvxBoxItem& rBox,
                          const OutputBorderOptions& rOptions,
                          std::map<SvxBoxItemLine,
                          css::table::BorderLine2> &rTableStyleConf )
{
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
    };

    const sal_Int32 aXmlElements[] =
    {
        XML_top,
        rOptions.bUseStartEnd ? XML_start : XML_left,
        XML_bottom,
        rOptions.bUseStartEnd ? XML_end : XML_right
    };
    bool tagWritten = false;
    const SvxBoxItemLine* pBrd = aBorders;

    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        const table::BorderLine2 *aStyleProps = nullptr;
        if( rTableStyleConf.find( *pBrd ) != rTableStyleConf.end() )
            aStyleProps = &rTableStyleConf[ *pBrd ];

        if (!tagWritten && rOptions.bWriteTag)
        {
            pSerializer->startElementNS(XML_w, rOptions.tag);
            tagWritten = true;
        }

        bool bWriteShadow = false;
        if (rOptions.aShadowLocation == SvxShadowLocation::NONE)
        {
            // The border has no shadow
        }
        else if (rOptions.aShadowLocation == SvxShadowLocation::BottomRight)
        {
            // Special case of 'Bottom-Right' shadow:
            // If the shadow location is 'Bottom-Right' - then turn on the shadow
            // for ALL the sides. This is because in Word - if you select a shadow
            // for a border - it turn on the shadow for ALL the sides (but shows only
            // the bottom-right one).
            // This is so that no information will be lost if passed through LibreOffice
            bWriteShadow = true;
        }
        else
        {
            // If there is a shadow, and it's not the regular 'Bottom-Right',
            // then write only the 'shadowed' sides of the border
            if  (
                    ((rOptions.aShadowLocation == SvxShadowLocation::TopLeft    || rOptions.aShadowLocation == SvxShadowLocation::TopRight  ) && *pBrd == SvxBoxItemLine::TOP   ) ||
                    ((rOptions.aShadowLocation == SvxShadowLocation::TopLeft    || rOptions.aShadowLocation == SvxShadowLocation::BottomLeft) && *pBrd == SvxBoxItemLine::LEFT  ) ||
                    ((rOptions.aShadowLocation == SvxShadowLocation::BottomLeft                                                             ) && *pBrd == SvxBoxItemLine::BOTTOM) ||
                    ((rOptions.aShadowLocation == SvxShadowLocation::TopRight                                                               ) && *pBrd == SvxBoxItemLine::RIGHT )
                )
            {
                bWriteShadow = true;
            }
        }

        sal_uInt16 nDist = 0;
        if (rOptions.bWriteDistance)
        {
            if (rOptions.pDistances)
            {
                if ( *pBrd == SvxBoxItemLine::TOP)
                    nDist = rOptions.pDistances->nTop;
                else if ( *pBrd == SvxBoxItemLine::LEFT)
                    nDist = rOptions.pDistances->nLeft;
                else if ( *pBrd == SvxBoxItemLine::BOTTOM)
                    nDist = rOptions.pDistances->nBottom;
                else if ( *pBrd == SvxBoxItemLine::RIGHT)
                    nDist = rOptions.pDistances->nRight;
            }
            else
            {
                nDist = rBox.GetDistance(*pBrd);
            }
        }

        impl_borderLine( pSerializer, aXmlElements[i], pLn, nDist, bWriteShadow, aStyleProps );
    }
    if (tagWritten && rOptions.bWriteTag) {
        pSerializer->endElementNS( XML_w, rOptions.tag );
    }
}

static void impl_cellMargins( FSHelperPtr const & pSerializer, const SvxBoxItem& rBox, sal_Int32 tag, bool bUseStartEnd, const SvxBoxItem* pDefaultMargins = nullptr)
{
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
    };

    const sal_Int32 aXmlElements[] =
    {
        XML_top,
        bUseStartEnd ? XML_start : XML_left,
        XML_bottom,
        bUseStartEnd ? XML_end : XML_right
    };
    bool tagWritten = false;
    const SvxBoxItemLine* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        sal_Int32 nDist = sal_Int32( rBox.GetDistance( *pBrd ) );

        if (pDefaultMargins)
        {
            // Skip output if cell margin == table default margin
            if (sal_Int32( pDefaultMargins->GetDistance( *pBrd ) ) == nDist)
                continue;
        }

        if (!tagWritten) {
            pSerializer->startElementNS(XML_w, tag);
            tagWritten = true;
        }
        pSerializer->singleElementNS( XML_w, aXmlElements[i],
               FSNS( XML_w, XML_w ), OString::number(nDist),
               FSNS( XML_w, XML_type ), "dxa" );
    }
    if (tagWritten) {
        pSerializer->endElementNS( XML_w, tag );
    }
}

void DocxAttributeOutput::TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow )
{
    m_pSerializer->startElementNS(XML_w, XML_tcPr);

    const SwTableBox *pTableBox = pTableTextNodeInfoInner->getTableBox( );

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Output any table cell redlines if there are any attached to this specific cell
    TableCellRedline( pTableTextNodeInfoInner );

    // Cell preferred width
    SwTwips nWidth = GetGridCols( pTableTextNodeInfoInner )->at( nCell );
    if ( nCell )
        nWidth = nWidth - GetGridCols( pTableTextNodeInfoInner )->at( nCell - 1 );
    m_pSerializer->singleElementNS( XML_w, XML_tcW,
           FSNS( XML_w, XML_w ), OString::number(nWidth),
           FSNS( XML_w, XML_type ), "dxa" );

    // Horizontal spans
    const SwWriteTableRows& rRows = m_xTableWrt->GetRows( );
    SwWriteTableRow *pRow = rRows[ nRow ].get();
    const SwWriteTableCells& rTableCells =  pRow->GetCells();
    if (nCell < rTableCells.size() )
    {
        const SwWriteTableCell& rCell = *rTableCells[nCell];
        const sal_uInt16 nColSpan = rCell.GetColSpan();
        if ( nColSpan > 1 )
            m_pSerializer->singleElementNS( XML_w, XML_gridSpan,
                    FSNS( XML_w, XML_val ), OString::number(nColSpan) );
    }

    // Vertical merges
    ww8::RowSpansPtr xRowSpans = pTableTextNodeInfoInner->getRowSpansOfRow();
    sal_Int32 vSpan = (*xRowSpans)[nCell];
    if ( vSpan > 1 )
    {
        m_pSerializer->singleElementNS(XML_w, XML_vMerge, FSNS(XML_w, XML_val), "restart");
    }
    else if ( vSpan < 0 )
    {
        m_pSerializer->singleElementNS(XML_w, XML_vMerge, FSNS(XML_w, XML_val), "continue");
    }

    if (const SfxGrabBagItem* pItem = pTableBox->GetFrameFormat()->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG))
    {
        const std::map<OUString, uno::Any>& rGrabBag = pItem->GetGrabBag();
        std::map<OUString, uno::Any>::const_iterator it = rGrabBag.find("CellCnfStyle");
        if (it != rGrabBag.end())
        {
            uno::Sequence<beans::PropertyValue> aAttributes = it->second.get< uno::Sequence<beans::PropertyValue> >();
            m_pTableStyleExport->CnfStyle(aAttributes);
        }
    }


    const SvxBoxItem& rBox = pTableBox->GetFrameFormat( )->GetBox( );
    const SvxBoxItem& rDefaultBox = (*tableFirstCells.rbegin())->getTableBox( )->GetFrameFormat( )->GetBox( );
    {
        // The cell borders
        impl_borders(m_pSerializer, rBox, lcl_getTableCellBorderOptions(bEcma),
                     m_aTableStyleConfs.back());
    }

    TableBackgrounds( pTableTextNodeInfoInner );

    {
        // Cell margins
        impl_cellMargins( m_pSerializer, rBox, XML_tcMar, !bEcma, &rDefaultBox );
    }

    TableVerticalCell( pTableTextNodeInfoInner );

    m_pSerializer->endElementNS( XML_w, XML_tcPr );
}

void DocxAttributeOutput::InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    if (m_xTableWrt && pTable == m_xTableWrt->GetTable())
        return;

    tools::Long nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans (and maybe other infos)
    GetTablePageSize( pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize );

    const SwFrameFormat *pFormat = pTable->GetFrameFormat( );
    const sal_uInt32 nTableSz = static_cast<sal_uInt32>(pFormat->GetFrameSize( ).GetWidth( ));

    const SwHTMLTableLayout *pLayout = pTable->GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        m_xTableWrt.reset(new SwWriteTable(pTable, pLayout));
    else
        m_xTableWrt.reset(new SwWriteTable(pTable, pTable->GetTabLines(), nPageSize, nTableSz, false));
}

void DocxAttributeOutput::StartTable( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    m_aTableStyleConfs.push_back({});

    // In case any paragraph SDT's are open, close them here.
    EndParaSdtBlock();

    m_pSerializer->startElementNS(XML_w, XML_tbl);

    tableFirstCells.push_back(pTableTextNodeInfoInner);
    lastOpenCell.push_back(-1);
    lastClosedCell.push_back(-1);

    InitTableHelper( pTableTextNodeInfoInner );
    TableDefinition( pTableTextNodeInfoInner );
}

void DocxAttributeOutput::EndTable()
{
    m_pSerializer->endElementNS( XML_w, XML_tbl );

    if ( m_tableReference->m_nTableDepth > 0 )
        --m_tableReference->m_nTableDepth;

    lastClosedCell.pop_back();
    lastOpenCell.pop_back();
    tableFirstCells.pop_back();

    // We closed the table; if it is a nested table, the cell that contains it
    // still continues
    // set to true only if we were in a nested table, not otherwise.
    if( !tableFirstCells.empty() )
        m_tableReference->m_bTableCellOpen = true;

    // Cleans the table helper
    m_xTableWrt.reset();

    m_aTableStyleConfs.pop_back();
}

void DocxAttributeOutput::StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    m_pSerializer->startElementNS(XML_w, XML_tr);

    // Output the row properties
    m_pSerializer->startElementNS(XML_w, XML_trPr);

    // Header row: tblHeader
    const SwTable *pTable = pTableTextNodeInfoInner->getTable( );
    if ( pTable->GetRowsToRepeat( ) > pTableTextNodeInfoInner->getRow( ) )
        m_pSerializer->singleElementNS(XML_w, XML_tblHeader, FSNS(XML_w, XML_val), "true"); // TODO to overwrite table style may need explicit false

    TableRowRedline( pTableTextNodeInfoInner );
    TableHeight( pTableTextNodeInfoInner );
    TableCanSplit( pTableTextNodeInfoInner );

    const SwTableBox *pTableBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTableLine = pTableBox->GetUpper();
    if (const SfxGrabBagItem* pItem = pTableLine->GetFrameFormat()->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG))
    {
        const std::map<OUString, uno::Any>& rGrabBag = pItem->GetGrabBag();
        std::map<OUString, uno::Any>::const_iterator it = rGrabBag.find("RowCnfStyle");
        if (it != rGrabBag.end())
        {
            uno::Sequence<beans::PropertyValue> aAttributes = it->second.get< uno::Sequence<beans::PropertyValue> >();
            m_pTableStyleExport->CnfStyle(aAttributes);
        }
    }


    m_pSerializer->endElementNS( XML_w, XML_trPr );
}

void DocxAttributeOutput::EndTableRow( )
{
    m_pSerializer->endElementNS( XML_w, XML_tr );
    lastOpenCell.back() = -1;
    lastClosedCell.back() = -1;
}

void DocxAttributeOutput::StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow )
{
    lastOpenCell.back() = nCell;

    InitTableHelper( pTableTextNodeInfoInner );

    m_pSerializer->startElementNS(XML_w, XML_tc);

    // Write the cell properties here
    TableCellProperties( pTableTextNodeInfoInner, nCell, nRow );

    m_tableReference->m_bTableCellOpen = true;
}

void DocxAttributeOutput::EndTableCell(sal_uInt32 nCell)
{
    lastClosedCell.back() = nCell;
    lastOpenCell.back() = -1;

    if (m_tableReference->m_bTableCellParaSdtOpen)
        EndParaSdtBlock();

    m_pSerializer->endElementNS( XML_w, XML_tc );

    m_tableReference->m_bTableCellOpen = false;
    m_tableReference->m_bTableCellParaSdtOpen = false;
}

void DocxAttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
}

void DocxAttributeOutput::TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfo*/ )
{
}

namespace
{

/// Does the same as comphelper::string::padToLength(), but extends the start, not the end.
OString lcl_padStartToLength(OString const & aString, sal_Int32 nLen, char cFill)
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

}

void DocxAttributeOutput::TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Write the table properties
    m_pSerializer->startElementNS(XML_w, XML_tblPr);

    static const sal_Int32 aOrder[] =
    {
        FSNS( XML_w, XML_tblStyle ),
        FSNS( XML_w, XML_tblpPr ),
        FSNS( XML_w, XML_tblOverlap ),
        FSNS( XML_w, XML_bidiVisual ),
        FSNS( XML_w, XML_tblStyleRowBandSize ),
        FSNS( XML_w, XML_tblStyleColBandSize ),
        FSNS( XML_w, XML_tblW ),
        FSNS( XML_w, XML_jc ),
        FSNS( XML_w, XML_tblCellSpacing ),
        FSNS( XML_w, XML_tblInd ),
        FSNS( XML_w, XML_tblBorders ),
        FSNS( XML_w, XML_shd ),
        FSNS( XML_w, XML_tblLayout ),
        FSNS( XML_w, XML_tblCellMar ),
        FSNS( XML_w, XML_tblLook ),
        FSNS( XML_w, XML_tblPrChange )
    };

    // postpone the output so that we can later []
    // prepend the properties before the run
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    m_pSerializer->mark(Tag_TableDefinition, comphelper::containerToSequence(aOrder));

    tools::Long nPageSize = 0;
    const char* widthType = "dxa";

    // If actual width of table is relative it should export is as "pct".`
    const SwTable *pTable = pTableTextNodeInfoInner->getTable();
    SwFrameFormat *pTableFormat = pTable->GetFrameFormat( );
    const SwFormatFrameSize &rSize = pTableFormat->GetFrameSize();
    int nWidthPercent = rSize.GetWidthPercent();
    // If we export a floating table: we use the widthPercent of the surrounding frame
    const ww8::Frame* pFloatingTableFrame = m_rExport.GetFloatingTableFrame();
    if (pFloatingTableFrame)
    {
        const SwFormatFrameSize &rFrameSize = pFloatingTableFrame->GetFrameFormat().GetFrameSize();
        nWidthPercent = rFrameSize.GetWidthPercent();
    }

    uno::Reference<beans::XPropertySet> xPropertySet(SwXTextTables::GetObject(*pTable->GetFrameFormat( )),uno::UNO_QUERY);
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

    if(isWidthRelative)
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
        nPageSize = nWidthPercent * 50 ;
        widthType = "pct" ;
    }
    else
    {
        bool bRelBoxSize = false;
        // Create the SwWriteTable instance to use col spans (and maybe other infos)
        GetTablePageSize( pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize );
        if(nPageSize == 0)
            widthType = "auto";
    }

    // Output the table preferred width
    m_pSerializer->singleElementNS( XML_w, XML_tblW,
            FSNS( XML_w, XML_w ), OString::number(nPageSize),
            FSNS( XML_w, XML_type ), widthType );

    // Disable layout autofit, as it does not exist in LibreOffice yet
    m_pSerializer->singleElementNS( XML_w, XML_tblLayout,
            FSNS( XML_w, XML_type ), "fixed" );

    // Look for the table style property in the table grab bag
    std::map<OUString, css::uno::Any> aGrabBag =
            pTableFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    // We should clear the TableStyle map. In case of Table inside multiple tables it contains the
    // table border style of the previous table.
    std::map<SvxBoxItemLine, css::table::BorderLine2>& rTableStyleConf = m_aTableStyleConfs.back();
    rTableStyleConf.clear();

    // Extract properties from grab bag
    for( const auto & rGrabBagElement : aGrabBag )
    {
        if( rGrabBagElement.first == "TableStyleName")
        {
            OString sStyleName = OUStringToOString( rGrabBagElement.second.get<OUString>(), RTL_TEXTENCODING_UTF8 );
            m_pSerializer->singleElementNS(XML_w, XML_tblStyle, FSNS(XML_w, XML_val), sStyleName);
        }
        else if( rGrabBagElement.first == "TableStyleTopBorder" )
            rTableStyleConf[SvxBoxItemLine::TOP] = rGrabBagElement.second.get<table::BorderLine2>();
        else if( rGrabBagElement.first == "TableStyleBottomBorder" )
            rTableStyleConf[SvxBoxItemLine::BOTTOM]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if( rGrabBagElement.first == "TableStyleLeftBorder" )
            rTableStyleConf[SvxBoxItemLine::LEFT]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if( rGrabBagElement.first == "TableStyleRightBorder" )
            rTableStyleConf[SvxBoxItemLine::RIGHT]
                = rGrabBagElement.second.get<table::BorderLine2>();
        else if (rGrabBagElement.first == "TableStyleLook")
        {
            rtl::Reference<FastAttributeList> pAttributeList = FastSerializerHelper::createAttrList();
            const uno::Sequence<beans::PropertyValue> aAttributeList = rGrabBagElement.second.get< uno::Sequence<beans::PropertyValue> >();

            for (const auto& rAttribute : aAttributeList)
            {
                if (rAttribute.Name == "val")
                    pAttributeList->add(FSNS(XML_w, XML_val), lcl_padStartToLength(OString::number(rAttribute.Value.get<sal_Int32>(), 16), 4, '0'));
                else
                {
                    static DocxStringTokenMap const aTokens[] =
                    {
                        {"firstRow", XML_firstRow},
                        {"lastRow", XML_lastRow},
                        {"firstColumn", XML_firstColumn},
                        {"lastColumn", XML_lastColumn},
                        {"noHBand", XML_noHBand},
                        {"noVBand", XML_noVBand},
                        {nullptr, 0}
                    };

                    if (sal_Int32 nToken = DocxStringGetToken(aTokens, rAttribute.Name))
                        pAttributeList->add(FSNS(XML_w, nToken), (rAttribute.Value.get<sal_Int32>() ? "1" : "0"));
                }
            }

            m_pSerializer->singleElementNS(XML_w, XML_tblLook, pAttributeList);
        }
        else if (rGrabBagElement.first == "TablePosition" &&
                        // skip empty table position (tables in footnotes converted to
                        // floating tables temporarily, don't export this)
                        rGrabBagElement.second != uno::Any() )
        {
            rtl::Reference<FastAttributeList> attrListTablePos = FastSerializerHelper::createAttrList( );
            const uno::Sequence<beans::PropertyValue> aTablePosition = rGrabBagElement.second.get<uno::Sequence<beans::PropertyValue> >();
            // look for a surrounding frame and take it's position values
            const ww8::Frame* pFrame = m_rExport.GetFloatingTableFrame();
            if( pFrame )
            {
                // we export the values of the surrounding Frame
                OString sOrientation;
                sal_Int32 nValue;

                // If tblpXSpec or tblpYSpec are present, we do not write tblpX or tblpY!
                OString sTblpXSpec = convertToOOXMLHoriOrient( pFrame->GetFrameFormat().GetHoriOrient().GetHoriOrient(), pFrame->GetFrameFormat().GetHoriOrient().IsPosToggle() );
                OString sTblpYSpec = convertToOOXMLVertOrient( pFrame->GetFrameFormat().GetVertOrient().GetVertOrient() );

                sOrientation = convertToOOXMLVertOrientRel( pFrame->GetFrameFormat().GetVertOrient().GetRelationOrient() );
                attrListTablePos->add(FSNS(XML_w, XML_vertAnchor), sOrientation);

                if( !sTblpYSpec.isEmpty() )
                    attrListTablePos->add(FSNS(XML_w, XML_tblpYSpec), sTblpYSpec);

                sOrientation = convertToOOXMLHoriOrientRel( pFrame->GetFrameFormat().GetHoriOrient().GetRelationOrient() );
                attrListTablePos->add(FSNS(XML_w, XML_horzAnchor), sOrientation);

                if( !sTblpXSpec.isEmpty() )
                    attrListTablePos->add(FSNS(XML_w, XML_tblpXSpec), sTblpXSpec);

                nValue = pFrame->GetFrameFormat().GetULSpace().GetLower();
                if( nValue != 0 )
                    attrListTablePos->add( FSNS( XML_w, XML_bottomFromText ), OString::number( nValue ) );

                nValue = pFrame->GetFrameFormat().GetLRSpace().GetLeft();
                if( nValue != 0 )
                    attrListTablePos->add( FSNS( XML_w, XML_leftFromText ), OString::number( nValue ) );

                nValue = pFrame->GetFrameFormat().GetLRSpace().GetRight();
                if( nValue != 0 )
                    attrListTablePos->add( FSNS( XML_w, XML_rightFromText ), OString::number( nValue ) );

                nValue = pFrame->GetFrameFormat().GetULSpace().GetUpper();
                if( nValue != 0 )
                    attrListTablePos->add( FSNS( XML_w, XML_topFromText ), OString::number( nValue ) );

                if( sTblpXSpec.isEmpty() ) // do not write tblpX if tblpXSpec is present
                {
                    nValue = pFrame->GetFrameFormat().GetHoriOrient().GetPos();
                    // we need to revert the additional shift introduced by
                    // lcl_DecrementHoriOrientPosition() in writerfilter
                    // 1st: left distance of the table
                    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
                    const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();
                    const SvxBoxItem& rBox = pFrameFormat->GetBox( );
                    sal_uInt16 nLeftDistance = rBox.GetDistance(SvxBoxItemLine::LEFT);
                    nValue += nLeftDistance;

                    // 2nd: if a left border is given, revert the shift by half the width
                    // from lcl_DecrementHoriOrientPosition() in writerfilter
                    if (const editeng::SvxBorderLine* pLeftBorder = rBox.GetLeft())
                    {
                        tools::Long nWidth = pLeftBorder->GetWidth();
                        nValue += (nWidth / 2);
                    }

                    attrListTablePos->add( FSNS( XML_w, XML_tblpX ), OString::number( nValue ) );
                }

                if( sTblpYSpec.isEmpty() ) // do not write tblpY if tblpYSpec is present
                {
                    nValue = pFrame->GetFrameFormat().GetVertOrient().GetPos();
                    attrListTablePos->add( FSNS( XML_w, XML_tblpY ), OString::number( nValue ) );
                }
            }
            else // ( pFrame = 0 )
            {
                // we export the values from the grabBag
                for (const auto& rProp : aTablePosition)
                {
                    if (rProp.Name == "vertAnchor" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation = OUStringToOString( rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_vertAnchor), sOrientation);
                    }
                    else if (rProp.Name == "tblpYSpec" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation = OUStringToOString( rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_tblpYSpec), sOrientation);
                    }
                    else if (rProp.Name == "horzAnchor" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation = OUStringToOString( rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_horzAnchor), sOrientation);
                    }
                    else if (rProp.Name == "tblpXSpec" && !rProp.Value.get<OUString>().isEmpty())
                    {
                        OString sOrientation = OUStringToOString( rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                        attrListTablePos->add(FSNS(XML_w, XML_tblpXSpec), sOrientation);
                    }
                    else if (rProp.Name == "bottomFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_bottomFromText ), OString::number( nValue ) );
                    }
                    else if (rProp.Name == "leftFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_leftFromText ), OString::number( nValue ) );
                    }
                    else if (rProp.Name == "rightFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_rightFromText ), OString::number( nValue ) );
                    }
                    else if (rProp.Name == "topFromText")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_topFromText ), OString::number( nValue ) );
                    }
                    else if (rProp.Name == "tblpX")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_tblpX ), OString::number( nValue ) );
                    }
                    else if (rProp.Name == "tblpY")
                    {
                        sal_Int32 nValue = rProp.Value.get<sal_Int32>();
                        attrListTablePos->add( FSNS( XML_w, XML_tblpY ), OString::number( nValue ) );
                    }
                }
            }

            m_pSerializer->singleElementNS( XML_w, XML_tblpPr, attrListTablePos);
            attrListTablePos = nullptr;
        }
        else
            SAL_WARN("sw.ww8", "DocxAttributeOutput::TableDefinition: unhandled property: " << rGrabBagElement.first);
    }

    // Output the table alignment
    const char* pJcVal;
    sal_Int32 nIndent = 0;
    switch ( pTableFormat->GetHoriOrient( ).GetHoriOrient( ) )
    {
        case text::HoriOrientation::CENTER:
            pJcVal = "center";
            break;
        case text::HoriOrientation::RIGHT:
            if ( bEcma )
                pJcVal = "right";
            else
                pJcVal = "end";
            break;
        default:
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
        {
            if ( bEcma )
                pJcVal = "left";
            else
                pJcVal = "start";
            nIndent = sal_Int32( pTableFormat->GetLRSpace().GetLeft() );

            // Table indentation has different meaning in Word, depending if the table is nested or not.
            // If nested, tblInd is added to parent table's left spacing and defines left edge position
            // If not nested, text position of left-most cell must be at absolute X = tblInd
            // so, table_spacing + table_spacing_to_content = tblInd

            // tdf#106742: since MS Word 2013 (compatibilityMode >= 15), top-level tables are handled the same as nested tables;
            // if no compatibilityMode is defined (which now should only happen on a new export to .docx),
            // LO uses a higher compatibility than 2010's 14.
            sal_Int32 nMode = lcl_getWordCompatibilityMode(m_rExport);

            const SwFrameFormat* pFrameFormat = pTableTextNodeInfoInner->getTableBox()->GetFrameFormat();
            if ((0 < nMode && nMode <= 14) && m_tableReference->m_nTableDepth == 0)
                nIndent += pFrameFormat->GetBox().GetDistance( SvxBoxItemLine::LEFT );
            else
            {
                // adjust for SW considering table to start mid-border instead of nested/2013's left-side-of-border.
                nIndent -= pFrameFormat->GetBox().CalcLineWidth( SvxBoxItemLine::LEFT ) / 2;
            }

            break;
        }
    }
    m_pSerializer->singleElementNS(XML_w, XML_jc, FSNS(XML_w, XML_val), pJcVal);

    // Output the table background color (although cell value still needs to be specified)
    const SvxBrushItem *pColorProp = pTableFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    Color aColor = pColorProp ? pColorProp->GetColor() : COL_AUTO;
    if ( aColor != COL_AUTO )
    {
        OString sColor = msfilter::util::ConvertColor( aColor );
        m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor,
                FSNS( XML_w, XML_val ), "clear" );
    }

    // Output the table borders
    TableDefaultBorders( pTableTextNodeInfoInner );

    // Output the default cell margins
    TableDefaultCellMargins( pTableTextNodeInfoInner );

    TableBidi( pTableTextNodeInfoInner );

    // Table indent (need to get written even if == 0)
    m_pSerializer->singleElementNS( XML_w, XML_tblInd,
            FSNS( XML_w, XML_w ), OString::number(nIndent),
            FSNS( XML_w, XML_type ), "dxa" );

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_TableDefinition);

    m_pSerializer->endElementNS( XML_w, XML_tblPr );

    // Write the table grid infos
    m_pSerializer->startElementNS(XML_w, XML_tblGrid);
    sal_Int32 nPrv = 0;
    ww8::WidthsPtr pColumnWidths = GetColumnWidths( pTableTextNodeInfoInner );
    for ( auto aColumnWidth : *pColumnWidths )
    {
        sal_Int32 nWidth  =  sal_Int32( aColumnWidth ) - nPrv;
        m_pSerializer->singleElementNS( XML_w, XML_gridCol,
               FSNS( XML_w, XML_w ), OString::number(nWidth) );
        nPrv = sal_Int32( aColumnWidth );
    }

    m_pSerializer->endElementNS( XML_w, XML_tblGrid );
}

void DocxAttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    // Table defaults should only be created IF m_aTableStyleConf contents haven't come from a table style.
    // Previously this function wrote out Cell A1 as the table default, causing problems with no benefit.
}

void DocxAttributeOutput::TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();
    const SvxBoxItem& rBox = pFrameFormat->GetBox( );
    const bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    impl_cellMargins(m_pSerializer, rBox, XML_tblCellMar, !bEcma);
}

void DocxAttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable *pTable = pTableTextNodeInfoInner->getTable();
    const SwTableBox *pTableBox = pTableTextNodeInfoInner->getTableBox( );
    const SwTableLine *pTableRow = pTableBox->GetUpper();
    const SwFrameFormat *pFormat = pTableBox->GetFrameFormat( );

    const SvxBrushItem *pColorProp = pFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    Color aColor = pColorProp ? pColorProp->GetColor() : COL_AUTO;

    const SwFrameFormat *pRowFormat = pTableRow->GetFrameFormat( );
    const SvxBrushItem *pRowColorProp = pRowFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if ( pRowColorProp && aColor == COL_AUTO)
        aColor = pRowColorProp->GetColor();

    const SwFrameFormat *pTableFormat = pTable->GetFrameFormat( );
    const SvxBrushItem *pTableColorProp = pTableFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if ( pTableColorProp && aColor == COL_AUTO )
        aColor = pTableColorProp->GetColor();

    const OString sColor = msfilter::util::ConvertColor( aColor );

    std::map<OUString, css::uno::Any> aGrabBag =
            pFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    OString sOriginalColor;
    std::map<OUString, css::uno::Any>::iterator aGrabBagElement = aGrabBag.find("originalColor");
    if( aGrabBagElement != aGrabBag.end() )
        sOriginalColor = OUStringToOString( aGrabBagElement->second.get<OUString>(), RTL_TEXTENCODING_UTF8 );

    if ( sOriginalColor != sColor )
    {
        // color changed by the user, or no grab bag: write sColor
        if ( sColor != "auto" )
        {
            m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor,
                FSNS( XML_w, XML_val ), "clear" );
        }
    }
    else
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pAttrList;

        for( const auto & rGrabBagElement : aGrabBag )
        {
            if (!rGrabBagElement.second.has<OUString>())
                continue;

            OString sValue = OUStringToOString( rGrabBagElement.second.get<OUString>(), RTL_TEXTENCODING_UTF8 );
            if( rGrabBagElement.first == "themeFill")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFill ), sValue.getStr() );
            else if( rGrabBagElement.first == "themeFillTint")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFillTint ), sValue.getStr() );
            else if( rGrabBagElement.first == "themeFillShade")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFillShade ), sValue.getStr() );
            else if( rGrabBagElement.first == "fill" )
                AddToAttrList( pAttrList, FSNS( XML_w, XML_fill ), sValue.getStr() );
            else if( rGrabBagElement.first == "themeColor")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeColor ), sValue.getStr() );
            else if( rGrabBagElement.first == "themeTint")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeTint ), sValue.getStr() );
            else if( rGrabBagElement.first == "themeShade")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeShade ), sValue.getStr() );
            else if( rGrabBagElement.first == "color")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_color ), sValue.getStr() );
            else if( rGrabBagElement.first == "val")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_val ), sValue.getStr() );
        }
        m_pSerializer->singleElementNS( XML_w, XML_shd, pAttrList.get() );
    }
}

void DocxAttributeOutput::TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();

    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
        SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

    // check table row property "HasTextChangesOnly"
    SwRedlineTable::size_type nPos(0);
    SwRedlineTable::size_type nChange = pTabLine->UpdateTextChangesOnly(nPos);
    if ( nChange != SwRedlineTable::npos )
    {
        const SwRedlineTable& aRedlineTable = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        const SwRangeRedline* pRedline = aRedlineTable[ nChange ];
        SwTableRowRedline* pTableRowRedline = nullptr;
        bool bIsInExtra = false;

        // use the original DOCX redline data stored in ExtraRedlineTable,
        // if it exists and its type wasn't changed
        const SwExtraRedlineTable& aExtraRedlineTable = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetExtraRedlineTable();
        for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize(); ++nCurRedlinePos )
        {
            SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
            pTableRowRedline = dynamic_cast<SwTableRowRedline*>(pExtraRedline);
            if (pTableRowRedline && &pTableRowRedline->GetTableLine() == pTabLine)
            {
                bIsInExtra = true;
                break;
            }
        }

        const SwRedlineData& aRedlineData = bIsInExtra &&
            // still the same type (an inserted row could become a tracked deleted one)
            pTableRowRedline->GetRedlineData().GetType() == pRedline->GetRedlineData().GetType()
                ? pTableRowRedline->GetRedlineData()
                : pRedline->GetRedlineData();

        // Note: all redline ranges and table row redline (with the same author and timestamp)
        // use the same redline id in OOXML exported by MSO, but it seems, the recent solution
        // (different IDs for different ranges, also row changes) is also portable.
        OString aId( OString::number( m_nRedlineId++ ) );
        const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( aRedlineData.GetAuthor() ) );
        OString aAuthor( OUStringToOString( bRemovePersonalInfo
                        ? "Author" + OUString::number( GetExport().GetInfoID(rAuthor) )
                        : rAuthor, RTL_TEXTENCODING_UTF8 ) );

        OString aDate( DateTimeToOString( bRemovePersonalInfo
                    ? DateTime(Date( 1, 1, 1970 )) // Epoch time
                    : aRedlineData.GetTimeStamp() ) );

        m_pSerializer->singleElementNS( XML_w,
                            RedlineType::Delete == pRedline->GetType() ? XML_del : XML_ins,
                            FSNS( XML_w, XML_id ), aId,
                            FSNS( XML_w, XML_author ), aAuthor,
                            FSNS( XML_w, XML_date ), aDate );
        return;
    }
}

void DocxAttributeOutput::TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();

    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
        SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

    // search next Redline
    const SwExtraRedlineTable& aExtraRedlineTable = m_rExport.m_rDoc.getIDocumentRedlineAccess().GetExtraRedlineTable();
    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
        const SwTableCellRedline* pTableCellRedline = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
        if (pTableCellRedline && &pTableCellRedline->GetTableBox() == pTabBox)
        {
            // Redline for this table cell
            const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
            RedlineType nRedlineType = aRedlineData.GetType();
            switch (nRedlineType)
            {
                case RedlineType::TableCellInsert:
                case RedlineType::TableCellDelete:
                {
                    OString aId( OString::number( m_nRedlineId++ ) );
                    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( aRedlineData.GetAuthor() ) );
                    OString aAuthor( OUStringToOString( bRemovePersonalInfo
                        ? "Author" + OUString::number( GetExport().GetInfoID(rAuthor) )
                        : rAuthor, RTL_TEXTENCODING_UTF8 ) );

                    OString aDate( DateTimeToOString( bRemovePersonalInfo
                            ? DateTime(Date( 1, 1, 1970 )) // Epoch time
                            : aRedlineData.GetTimeStamp() ) );

                    if (nRedlineType == RedlineType::TableCellInsert)
                        m_pSerializer->singleElementNS( XML_w, XML_cellIns,
                            FSNS( XML_w, XML_id ), aId,
                            FSNS( XML_w, XML_author ), aAuthor,
                            FSNS( XML_w, XML_date ), aDate );
                    else if (nRedlineType == RedlineType::TableCellDelete)
                        m_pSerializer->singleElementNS( XML_w, XML_cellDel,
                            FSNS( XML_w, XML_id ), aId,
                            FSNS( XML_w, XML_author ), aAuthor,
                            FSNS( XML_w, XML_date ), aDate );
                }
                break;
                default: break;
            }
        }
    }
}

void DocxAttributeOutput::TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrameFormat * pLineFormat = pTabLine->GetFrameFormat();

    const SwFormatFrameSize& rLSz = pLineFormat->GetFrameSize();
    if ( !(SwFrameSize::Variable != rLSz.GetHeightSizeType() && rLSz.GetHeight()) )
        return;

    sal_Int32 nHeight = rLSz.GetHeight();
    const char *pRule = nullptr;

    switch ( rLSz.GetHeightSizeType() )
    {
        case SwFrameSize::Fixed: pRule = "exact"; break;
        case SwFrameSize::Minimum: pRule = "atLeast"; break;
        default:           break;
    }

    if ( pRule )
        m_pSerializer->singleElementNS( XML_w, XML_trHeight,
                FSNS( XML_w, XML_val ), OString::number(nHeight),
                FSNS( XML_w, XML_hRule ), pRule );
}

void DocxAttributeOutput::TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrameFormat * pLineFormat = pTabLine->GetFrameFormat();

    const SwFormatRowSplit& rSplittable = pLineFormat->GetRowSplit( );
    // if rSplittable is true then no need to write <w:cantSplit w:val="false"/>
    // as default row prop is allow row to break across page.
    if( !rSplittable.GetValue( ) )
        m_pSerializer->singleElementNS(XML_w, XML_cantSplit, FSNS(XML_w, XML_val), "true");
}

void DocxAttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrameFormat * pFrameFormat = pTable->GetFrameFormat();

    if ( m_rExport.TrueFrameDirection( *pFrameFormat ) == SvxFrameDirection::Horizontal_RL_TB )
    {
        m_pSerializer->singleElementNS(XML_w, XML_bidiVisual, FSNS(XML_w, XML_val), "true");
    }
}

void DocxAttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat *pFrameFormat = pTabBox->GetFrameFormat( );

    if ( SvxFrameDirection::Vertical_RL_TB == m_rExport.TrueFrameDirection( *pFrameFormat ) )
        m_pSerializer->singleElementNS(XML_w, XML_textDirection, FSNS(XML_w, XML_val), "tbRl");
    else if ( SvxFrameDirection::Vertical_LR_BT == m_rExport.TrueFrameDirection( *pFrameFormat ) )
    {
        m_pSerializer->singleElementNS(XML_w, XML_textDirection, FSNS(XML_w, XML_val), "btLr");
    }

    const SwWriteTableRows& rRows = m_xTableWrt->GetRows( );
    SwWriteTableRow *pRow = rRows[ pTableTextNodeInfoInner->getRow( ) ].get();
    sal_uInt32 nCell = pTableTextNodeInfoInner->getCell();
    const SwWriteTableCells& rTableCells =  pRow->GetCells();
    if (nCell >= rTableCells.size() )
        return;

    const SwWriteTableCell *const pCell = pRow->GetCells()[ nCell ].get();
    switch( pCell->GetVertOri())
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

void DocxAttributeOutput::TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner )
{
    // This is called when the nested table ends in a cell, and there's no
    // paragraph behind that; so we must check for the ends of cell, rows,
    // tables
    // ['true' to write an empty paragraph, MS Word insists on that]
    FinishTableRowCell( pNodeInfoInner, true );
}

void DocxAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )" );
}

void DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )" );
}

void DocxAttributeOutput::TableRowEnd( sal_uInt32 /*nDepth*/ )
{
    SAL_INFO("sw.ww8", "TODO: DocxAttributeOutput::TableRowEnd( sal_uInt32 nDepth = 1 )" );
}

void DocxAttributeOutput::StartStyles()
{
    m_pSerializer->startElementNS( XML_w, XML_styles,
            FSNS( XML_xmlns, XML_w ),   GetExport().GetFilter().getNamespaceURL(OOX_NS(doc)),
            FSNS( XML_xmlns, XML_w14 ), GetExport().GetFilter().getNamespaceURL(OOX_NS(w14)),
            FSNS( XML_xmlns, XML_mc ),  GetExport().GetFilter().getNamespaceURL(OOX_NS(mce)),
            FSNS( XML_mc, XML_Ignorable ), "w14" );

    DocDefaults();
    LatentStyles();
}

sal_Int32 DocxStringGetToken(DocxStringTokenMap const * pMap, std::u16string_view rName)
{
    OString sName = OUStringToOString(rName, RTL_TEXTENCODING_UTF8);
    while (pMap->pToken)
    {
        if (sName == pMap->pToken)
            return pMap->nToken;
        ++pMap;
    }
    return 0;
}

namespace
{

DocxStringTokenMap const aDefaultTokens[] = {
    {"defQFormat", XML_defQFormat},
    {"defUnhideWhenUsed", XML_defUnhideWhenUsed},
    {"defSemiHidden", XML_defSemiHidden},
    {"count", XML_count},
    {"defUIPriority", XML_defUIPriority},
    {"defLockedState", XML_defLockedState},
    {nullptr, 0}
};

DocxStringTokenMap const aExceptionTokens[] = {
    {"name", XML_name},
    {"locked", XML_locked},
    {"uiPriority", XML_uiPriority},
    {"semiHidden", XML_semiHidden},
    {"unhideWhenUsed", XML_unhideWhenUsed},
    {"qFormat", XML_qFormat},
    {nullptr, 0}
};

}

void DocxAttributeOutput::LatentStyles()
{
    // Do we have latent styles available?
    uno::Reference<beans::XPropertySet> xPropertySet(m_rExport.m_rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xPropertySet->getPropertyValue("InteropGrabBag") >>= aInteropGrabBag;
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    auto pProp = std::find_if(std::cbegin(aInteropGrabBag), std::cend(aInteropGrabBag),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "latentStyles"; });
    if (pProp != std::cend(aInteropGrabBag))
        pProp->Value >>= aLatentStyles;
    if (!aLatentStyles.hasElements())
        return;

    // Extract default attributes first.
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList = FastSerializerHelper::createAttrList();
    uno::Sequence<beans::PropertyValue> aLsdExceptions;
    for (const auto& rLatentStyle : std::as_const(aLatentStyles))
    {
        if (sal_Int32 nToken = DocxStringGetToken(aDefaultTokens, rLatentStyle.Name))
            pAttributeList->add(FSNS(XML_w, nToken), OUStringToOString(rLatentStyle.Value.get<OUString>(), RTL_TEXTENCODING_UTF8));
        else if (rLatentStyle.Name == "lsdExceptions")
            rLatentStyle.Value >>= aLsdExceptions;
    }

    m_pSerializer->startElementNS(XML_w, XML_latentStyles, pAttributeList);
    pAttributeList = nullptr;

    // Then handle the exceptions.
    for (const auto& rLsdException : std::as_const(aLsdExceptions))
    {
        pAttributeList = FastSerializerHelper::createAttrList();

        uno::Sequence<beans::PropertyValue> aAttributes;
        rLsdException.Value >>= aAttributes;
        for (const auto& rAttribute : std::as_const(aAttributes))
            if (sal_Int32 nToken = DocxStringGetToken(aExceptionTokens, rAttribute.Name))
                pAttributeList->add(FSNS(XML_w, nToken), OUStringToOString(rAttribute.Value.get<OUString>(), RTL_TEXTENCODING_UTF8));

        m_pSerializer->singleElementNS(XML_w, XML_lsdException, pAttributeList);
        pAttributeList = nullptr;
    }

    m_pSerializer->endElementNS(XML_w, XML_latentStyles);
}

void DocxAttributeOutput::OutputDefaultItem(const SfxPoolItem& rHt)
{
    bool bMustWrite = true;
    switch (rHt.Which())
    {
        case RES_CHRATR_CASEMAP:
            bMustWrite = static_cast< const SvxCaseMapItem& >(rHt).GetCaseMap() != SvxCaseMap::NotMapped;
            break;
        case RES_CHRATR_COLOR:
            bMustWrite = static_cast< const SvxColorItem& >(rHt).GetValue() != COL_AUTO;
            break;
        case RES_CHRATR_CONTOUR:
            bMustWrite = static_cast< const SvxContourItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_CROSSEDOUT:
            bMustWrite = static_cast< const SvxCrossedOutItem& >(rHt).GetStrikeout() != STRIKEOUT_NONE;
            break;
        case RES_CHRATR_ESCAPEMENT:
            bMustWrite = static_cast< const SvxEscapementItem& >(rHt).GetEscapement() != SvxEscapement::Off;
            break;
        case RES_CHRATR_FONT:
            bMustWrite = true;
            break;
        case RES_CHRATR_FONTSIZE:
            bMustWrite = static_cast< const SvxFontHeightItem& >(rHt).GetHeight() != 200; // see StyleSheetTable_Impl::StyleSheetTable_Impl() where we set this default
            break;
        case RES_CHRATR_KERNING:
            bMustWrite = static_cast< const SvxKerningItem& >(rHt).GetValue() != 0;
            break;
        case RES_CHRATR_LANGUAGE:
            bMustWrite = true;
            break;
        case RES_CHRATR_POSTURE:
            bMustWrite = static_cast< const SvxPostureItem& >(rHt).GetPosture() != ITALIC_NONE;
            break;
        case RES_CHRATR_SHADOWED:
            bMustWrite = static_cast< const SvxShadowedItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_UNDERLINE:
            bMustWrite = static_cast< const SvxUnderlineItem& >(rHt).GetLineStyle() != LINESTYLE_NONE;
            break;
        case RES_CHRATR_WEIGHT:
            bMustWrite = static_cast< const SvxWeightItem& >(rHt).GetWeight() != WEIGHT_NORMAL;
            break;
        case RES_CHRATR_AUTOKERN:
            bMustWrite = static_cast< const SvxAutoKernItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_BLINK:
            bMustWrite = static_cast< const SvxBlinkItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_BACKGROUND:
            {
                const SvxBrushItem& rBrushItem = static_cast< const SvxBrushItem& >(rHt);
                bMustWrite = (rBrushItem.GetColor() != COL_AUTO ||
                              rBrushItem.GetShadingValue() != ShadingPattern::CLEAR ||
                              rBrushItem.GetGraphic() != nullptr ||
                              rBrushItem.GetGraphicObject() != nullptr);
            }
            break;

        case RES_CHRATR_CJK_FONT:
            bMustWrite = true;
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            bMustWrite = false; // we have written it already as RES_CHRATR_FONTSIZE
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            bMustWrite = true;
            break;
        case RES_CHRATR_CJK_POSTURE:
            bMustWrite = false; // we have written it already as RES_CHRATR_POSTURE
            break;
        case RES_CHRATR_CJK_WEIGHT:
            bMustWrite = false; // we have written it already as RES_CHRATR_WEIGHT
            break;

        case RES_CHRATR_CTL_FONT:
            bMustWrite = true;
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            bMustWrite = static_cast< const SvxFontHeightItem& >(rHt).GetHeight() != 200; // see StyleSheetTable_Impl::StyleSheetTable_Impl() where we set this default
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            bMustWrite = true;
            break;
        case RES_CHRATR_CTL_POSTURE:
            bMustWrite = static_cast< const SvxPostureItem& >(rHt).GetPosture() != ITALIC_NONE;
            break;
        case RES_CHRATR_CTL_WEIGHT:
            bMustWrite = static_cast< const SvxWeightItem& >(rHt).GetWeight() != WEIGHT_NORMAL;
            break;

        case RES_CHRATR_ROTATE:
            bMustWrite = static_cast< const SvxCharRotateItem& >(rHt).GetValue() != 0_deg10;
            break;
        case RES_CHRATR_EMPHASIS_MARK:
            bMustWrite = static_cast< const SvxEmphasisMarkItem& >(rHt).GetEmphasisMark() != FontEmphasisMark::NONE;
            break;
        case RES_CHRATR_TWO_LINES:
            bMustWrite = static_cast< const SvxTwoLinesItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_SCALEW:
            bMustWrite = static_cast< const SvxCharScaleWidthItem& >(rHt).GetValue() != 100;
            break;
        case RES_CHRATR_RELIEF:
            bMustWrite = static_cast< const SvxCharReliefItem& >(rHt).GetValue() != FontRelief::NONE;
            break;
        case RES_CHRATR_HIDDEN:
            bMustWrite = static_cast< const SvxCharHiddenItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_BOX:
            {
                const SvxBoxItem& rBoxItem = static_cast< const SvxBoxItem& >(rHt);
                bMustWrite = rBoxItem.GetTop() || rBoxItem.GetLeft() ||
                             rBoxItem.GetBottom() || rBoxItem.GetRight() ||
                             rBoxItem.GetSmallestDistance();
            }
            break;
        case RES_CHRATR_HIGHLIGHT:
            {
                const SvxBrushItem& rBrushItem = static_cast< const SvxBrushItem& >(rHt);
                bMustWrite = (rBrushItem.GetColor() != COL_AUTO ||
                              rBrushItem.GetShadingValue() != ShadingPattern::CLEAR ||
                              rBrushItem.GetGraphic() != nullptr ||
                              rBrushItem.GetGraphicObject() != nullptr);
            }
            break;

        case RES_PARATR_LINESPACING:
            bMustWrite = static_cast< const SvxLineSpacingItem& >(rHt).GetInterLineSpaceRule() != SvxInterLineSpaceRule::Off;
            break;
        case RES_PARATR_ADJUST:
            bMustWrite = static_cast< const SvxAdjustItem& >(rHt).GetAdjust() != SvxAdjust::Left;
            break;
        case RES_PARATR_SPLIT:
            bMustWrite = !static_cast< const SvxFormatSplitItem& >(rHt).GetValue();
            break;
        case RES_PARATR_WIDOWS:
            bMustWrite = static_cast< const SvxWidowsItem& >(rHt).GetValue();
            break;
        case RES_PARATR_TABSTOP:
            bMustWrite = static_cast< const SvxTabStopItem& >(rHt).Count() != 0;
            break;
        case RES_PARATR_HYPHENZONE:
            bMustWrite = true;
            break;
        case RES_PARATR_NUMRULE:
            bMustWrite = !static_cast< const SwNumRuleItem& >(rHt).GetValue().isEmpty();
            break;
        case RES_PARATR_SCRIPTSPACE:
            bMustWrite = !static_cast< const SfxBoolItem& >(rHt).GetValue();
            break;
        case RES_PARATR_HANGINGPUNCTUATION:
            bMustWrite = !static_cast< const SfxBoolItem& >(rHt).GetValue();
            break;
        case RES_PARATR_FORBIDDEN_RULES:
            bMustWrite = !static_cast< const SfxBoolItem& >(rHt).GetValue();
            break;
        case RES_PARATR_VERTALIGN:
            bMustWrite = static_cast< const SvxParaVertAlignItem& >(rHt).GetValue() != SvxParaVertAlignItem::Align::Automatic;
            break;
        case RES_PARATR_SNAPTOGRID:
            bMustWrite = !static_cast< const SvxParaGridItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_GRABBAG:
            bMustWrite = true;
            break;

        default:
            SAL_INFO("sw.ww8", "Unhandled SfxPoolItem with id " << rHt.Which() );
            break;
    }

    if (bMustWrite)
        OutputItem(rHt);
}

void DocxAttributeOutput::DocDefaults( )
{
    // Write the '<w:docDefaults>' section here
    m_pSerializer->startElementNS(XML_w, XML_docDefaults);

    // Output the default run properties
    m_pSerializer->startElementNS(XML_w, XML_rPrDefault);

    StartStyleProperties(false, 0);

    for (int i = int(RES_CHRATR_BEGIN); i < int(RES_CHRATR_END); ++i)
        OutputDefaultItem(m_rExport.m_rDoc.GetDefault(i));

    EndStyleProperties(false);

    m_pSerializer->endElementNS(XML_w, XML_rPrDefault);

    // Output the default paragraph properties
    m_pSerializer->startElementNS(XML_w, XML_pPrDefault);

    StartStyleProperties(true, 0);

    for (int i = int(RES_PARATR_BEGIN); i < int(RES_PARATR_END); ++i)
        OutputDefaultItem(m_rExport.m_rDoc.GetDefault(i));

    EndStyleProperties(true);

    m_pSerializer->endElementNS(XML_w, XML_pPrDefault);

    m_pSerializer->endElementNS(XML_w, XML_docDefaults);
}

void DocxAttributeOutput::EndStyles( sal_uInt16 nNumberOfStyles )
{
    // HACK
    // Ms Office seems to have an internal limitation of 4091 styles
    // and refuses to load .docx with more, even though the spec seems to allow that;
    // so simply if there are more styles, don't export those
    const sal_Int32 nCountStylesToWrite = MSWORD_MAX_STYLES_LIMIT - nNumberOfStyles;
    m_pTableStyleExport->TableStyles(nCountStylesToWrite);
    m_pSerializer->endElementNS( XML_w, XML_styles );
}

void DocxAttributeOutput::DefaultStyle()
{
    // are these the values of enum ww::sti (see ../inc/wwstyles.hxx)?
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::DefaultStyle()");
}

/* Writes <a:srcRect> tag back to document.xml if a file contains a cropped image.
*  NOTE : Tested on images of type JPEG,EMF/WMF,BMP, PNG and GIF.
*/
void DocxAttributeOutput::WriteSrcRect(
    const css::uno::Reference<css::beans::XPropertySet>& xShapePropSet,
    const SwFrameFormat* pFrameFormat)
{
    uno::Reference<graphic::XGraphic> xGraphic;
    xShapePropSet->getPropertyValue("Graphic") >>= xGraphic;
    const Graphic aGraphic(xGraphic);

    Size aOriginalSize(aGraphic.GetPrefSize());

    const MapMode aMap100mm( MapUnit::Map100thMM );
    const MapMode& rMapMode = aGraphic.GetPrefMapMode();
    if (rMapMode.GetMapUnit() == MapUnit::MapPixel)
    {
        aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aOriginalSize, aMap100mm);
    }

    css::text::GraphicCrop aGraphicCropStruct;
    xShapePropSet->getPropertyValue("GraphicCrop") >>= aGraphicCropStruct;
    sal_Int32 nCropL = aGraphicCropStruct.Left;
    sal_Int32 nCropR = aGraphicCropStruct.Right;
    sal_Int32 nCropT = aGraphicCropStruct.Top;
    sal_Int32 nCropB = aGraphicCropStruct.Bottom;

    // simulate border padding as a negative crop.
    const SfxPoolItem* pItem;
    if (pFrameFormat && SfxItemState::SET == pFrameFormat->GetItemState(RES_BOX, false, &pItem))
    {
        const SvxBoxItem& rBox = *static_cast<const SvxBoxItem*>(pItem);
        nCropL -= rBox.GetDistance( SvxBoxItemLine::LEFT );
        nCropR -= rBox.GetDistance( SvxBoxItemLine::RIGHT );
        nCropT -= rBox.GetDistance( SvxBoxItemLine::TOP );
        nCropB -= rBox.GetDistance( SvxBoxItemLine::BOTTOM );
    }

    if ( !((0 != nCropL) || (0 != nCropT) || (0 != nCropR) || (0 != nCropB)) )
        return;

    double  widthMultiplier  = 100000.0/aOriginalSize.Width();
    double  heightMultiplier = 100000.0/aOriginalSize.Height();

    sal_Int32 left   = static_cast<sal_Int32>(rtl::math::round(nCropL * widthMultiplier));
    sal_Int32 right  = static_cast<sal_Int32>(rtl::math::round(nCropR * widthMultiplier));
    sal_Int32 top    = static_cast<sal_Int32>(rtl::math::round(nCropT * heightMultiplier));
    sal_Int32 bottom = static_cast<sal_Int32>(rtl::math::round(nCropB * heightMultiplier));

    m_pSerializer->singleElementNS( XML_a, XML_srcRect,
         XML_l, OString::number(left),
         XML_t, OString::number(top),
         XML_r, OString::number(right),
         XML_b, OString::number(bottom) );
}

void DocxAttributeOutput::PushRelIdCache()
{
    m_aRelIdCache.emplace();
    m_aSdrRelIdCache.emplace();
}

OUString DocxAttributeOutput::FindRelId(BitmapChecksum nChecksum)
{
    OUString aRet;

    if (!m_aSdrRelIdCache.empty() && m_aSdrRelIdCache.top().find(nChecksum) != m_aSdrRelIdCache.top().end())
        aRet = m_aSdrRelIdCache.top()[nChecksum].first;

    return aRet;
}

OUString DocxAttributeOutput::FindFileName(BitmapChecksum nChecksum)
{
    OUString aRet;

    if (!m_aSdrRelIdCache.empty() && m_aSdrRelIdCache.top().find(nChecksum) != m_aSdrRelIdCache.top().end())
        aRet = m_aSdrRelIdCache.top()[nChecksum].second;

    return aRet;
}

void DocxAttributeOutput::CacheRelId(BitmapChecksum nChecksum, const OUString& rRelId, const OUString& rFileName)
{
    if (!m_aSdrRelIdCache.empty())
        m_aSdrRelIdCache.top()[nChecksum] = std::pair(rRelId, rFileName);
}

uno::Reference<css::text::XTextFrame> DocxAttributeOutput::GetUnoTextFrame(
    css::uno::Reference<css::drawing::XShape> xShape)
{
    return SwTextBoxHelper::getUnoTextFrame(xShape);
}

std::pair<OString, OUString> DocxAttributeOutput::getExistingGraphicRelId(BitmapChecksum nChecksum)
{
    std::pair<OString, OUString> aResult;

    if (m_aRelIdCache.empty())
        return aResult;

    auto pIterator = m_aRelIdCache.top().find(nChecksum);

    if (pIterator != m_aRelIdCache.top().end())
    {
        aResult = pIterator->second;
    }

    return aResult;
}

void DocxAttributeOutput::cacheGraphicRelId(BitmapChecksum nChecksum, OString const & rRelId, OUString const & rFileName)
{
    if (!m_aRelIdCache.empty())
        m_aRelIdCache.top().emplace(nChecksum, std::pair(rRelId, rFileName));
}

void DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj )
{
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj  ) - some stuff still missing" );

    GetSdtEndBefore(pSdrObj);

    // detect mis-use of the API
    assert(pGrfNode || (pOLEFrameFormat && pOLENode));
    const SwFrameFormat* pFrameFormat = pGrfNode ? pGrfNode->GetFlyFormat() : pOLEFrameFormat;
    // create the relation ID
    OString aRelId;
    sal_Int32 nImageType;
    if ( pGrfNode && pGrfNode->IsLinkedFile() )
    {
        // linked image, just create the relation
        OUString aFileName;
        pGrfNode->GetFileFilterNms( &aFileName, nullptr );

        sal_Int32 const nFragment(aFileName.indexOf('#'));
        sal_Int32 const nForbiddenU(aFileName.indexOf("%5C"));
        sal_Int32 const nForbiddenL(aFileName.indexOf("%5c"));
        if (   (nForbiddenU != -1 && (nFragment == -1 || nForbiddenU < nFragment))
            || (nForbiddenL != -1 && (nFragment == -1 || nForbiddenL < nFragment)))
        {
            SAL_WARN("sw.ww8", "DocxAttributeOutput::FlyFrameGraphic: ignoring image with invalid link URL");
            return;
        }

        // TODO Convert the file name to relative for better interoperability

        aRelId = m_rExport.AddRelation(
                    oox::getRelationship(Relationship::IMAGE),
                    aFileName );

        nImageType = XML_link;
    }
    else
    {
        // inline, we also have to write the image itself
        Graphic aGraphic;
        if (pGrfNode)
            aGraphic = pGrfNode->GetGrf();
        else
            aGraphic = *pOLENode->GetGraphic();

        BitmapChecksum aChecksum = aGraphic.GetChecksum();
        OUString aFileName;
        std::tie(aRelId, aFileName) = getExistingGraphicRelId(aChecksum);
        OUString aImageId;

        if (aRelId.isEmpty())
        {
            // Not in cache, then need to write it.
            m_rDrawingML.SetFS( m_pSerializer ); // to be sure that we write to the right stream

            aImageId = m_rDrawingML.WriteImage(aGraphic, false, &aFileName);

            aRelId = OUStringToOString( aImageId, RTL_TEXTENCODING_UTF8 );
            cacheGraphicRelId(aChecksum, aRelId, aFileName);
        }
        else
        {
            // Include the same relation again. This makes it possible to
            // reuse an image across different headers.
            aImageId = m_rDrawingML.GetFB()->addRelation( m_pSerializer->getOutputStream(),
                oox::getRelationship(Relationship::IMAGE),
                aFileName );

            aRelId = OUStringToOString( aImageId, RTL_TEXTENCODING_UTF8 );
        }

        nImageType = XML_embed;
    }

    // In case there are any grab-bag items on the graphic frame, emit them now.
    // These are always character grab-bags, as graphics are at-char or as-char in Word.
    const SfxPoolItem* pItem = nullptr;
    if (pFrameFormat->GetAttrSet().HasItem(RES_FRMATR_GRABBAG, &pItem))
    {
        const SfxGrabBagItem* pGrabBag = static_cast<const SfxGrabBagItem*>(pItem);
        CharGrabBag(*pGrabBag);
    }

    rtl::Reference<sax_fastparser::FastAttributeList> xFrameAttributes(
        FastSerializerHelper::createAttrList());
    if (pGrfNode)
    {
        const SwAttrSet& rSet = pGrfNode->GetSwAttrSet();
        MirrorGraph eMirror = rSet.Get(RES_GRFATR_MIRRORGRF).GetValue();
        if (eMirror == MirrorGraph::Vertical || eMirror == MirrorGraph::Both)
            // Mirror on the vertical axis is a horizontal flip.
            xFrameAttributes->add(XML_flipH, "1");
        // RES_GRFATR_ROTATION is sal_uInt16; use sal_uInt32 for multiplication later
        if (Degree10 nRot = rSet.Get(RES_GRFATR_ROTATION).GetValue())
        {
            // RES_GRFATR_ROTATION is in 10ths of degree; convert to 100ths for macro
            sal_uInt32 mOOXMLRot = oox::drawingml::ExportRotateClockwisify(to<Degree100>(nRot));
            xFrameAttributes->add(XML_rot, OString::number(mOOXMLRot));
        }
    }

    css::uno::Reference<css::beans::XPropertySet> xShapePropSet;
    if (pSdrObj)
    {
        css::uno::Reference<css::drawing::XShape> xShape(
            const_cast<SdrObject*>(pSdrObj)->getUnoShape(), css::uno::UNO_QUERY);
        xShapePropSet.set(xShape, css::uno::UNO_QUERY);
        assert(xShapePropSet);
    }

    Size aSize = rSize;
    // We need the original (cropped, but unrotated) size of object. So prefer the object data,
    // and only use passed frame size as fallback.
    if (xShapePropSet)
    {
        if (css::awt::Size val; xShapePropSet->getPropertyValue("Size") >>= val)
            aSize = Size(o3tl::toTwips(val.Width, o3tl::Length::mm100), o3tl::toTwips(val.Height, o3tl::Length::mm100));
    }

    m_rExport.SdrExporter().startDMLAnchorInline(pFrameFormat, aSize);

    // picture description (used for pic:cNvPr later too)
    rtl::Reference<::sax_fastparser::FastAttributeList> docPrattrList = FastSerializerHelper::createAttrList();
    docPrattrList->add( XML_id, OString::number( m_anchorId++).getStr());
    docPrattrList->add( XML_name, OUStringToOString( pFrameFormat->GetName(), RTL_TEXTENCODING_UTF8 ) );
    docPrattrList->add( XML_descr, OUStringToOString( pGrfNode ? pGrfNode->GetDescription() : pOLEFrameFormat->GetObjDescription(), RTL_TEXTENCODING_UTF8 ));
    if( GetExport().GetFilter().getVersion( ) != oox::core::ECMA_DIALECT )
        docPrattrList->add( XML_title, OUStringToOString( pGrfNode ? pGrfNode->GetTitle() : pOLEFrameFormat->GetObjTitle(), RTL_TEXTENCODING_UTF8 ));
    m_pSerializer->startElementNS( XML_wp, XML_docPr, docPrattrList );

    OUString sURL, sRelId;
    if (xShapePropSet)
    {
        xShapePropSet->getPropertyValue("HyperLinkURL") >>= sURL;
        if(!sURL.isEmpty())
        {
            if (sURL.startsWith("#") && sURL.indexOf(' ') != -1 && !sURL.endsWith("|outline") && !sURL.endsWith("|table") &&
                !sURL.endsWith("|frame") && !sURL.endsWith("|graphic") && !sURL.endsWith("|ole") && !sURL.endsWith("|region"))
            {
                // Spaces are prohibited in bookmark name.
                sURL = sURL.replace(' ', '_');
            }
            sRelId = GetExport().GetFilter().addRelation( m_pSerializer->getOutputStream(),
                        oox::getRelationship(Relationship::HYPERLINK),
                        sURL, !sURL.startsWith("#") );
            m_pSerializer->singleElementNS( XML_a, XML_hlinkClick,
                FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
                FSNS( XML_r, XML_id ), sRelId);
        }
    }

    m_pSerializer->endElementNS( XML_wp, XML_docPr );

    m_pSerializer->startElementNS(XML_wp, XML_cNvGraphicFramePr);
    // TODO change aspect?
    m_pSerializer->singleElementNS( XML_a, XML_graphicFrameLocks,
            FSNS( XML_xmlns, XML_a ), GetExport().GetFilter().getNamespaceURL(OOX_NS(dml)),
            XML_noChangeAspect, "1" );
    m_pSerializer->endElementNS( XML_wp, XML_cNvGraphicFramePr );

    m_pSerializer->startElementNS( XML_a, XML_graphic,
            FSNS( XML_xmlns, XML_a ), GetExport().GetFilter().getNamespaceURL(OOX_NS(dml)) );
    m_pSerializer->startElementNS( XML_a, XML_graphicData,
            XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/picture" );

    m_pSerializer->startElementNS( XML_pic, XML_pic,
            FSNS( XML_xmlns, XML_pic ), GetExport().GetFilter().getNamespaceURL(OOX_NS(dmlPicture)) );

    m_pSerializer->startElementNS(XML_pic, XML_nvPicPr);
    // It seems pic:cNvpr and wp:docPr are pretty much the same thing with the same attributes
    m_pSerializer->startElementNS(XML_pic, XML_cNvPr, docPrattrList);

    if(!sURL.isEmpty())
        m_pSerializer->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);

    m_pSerializer->endElementNS( XML_pic, XML_cNvPr );

    m_pSerializer->startElementNS(XML_pic, XML_cNvPicPr);
    // TODO change aspect?
    m_pSerializer->singleElementNS( XML_a, XML_picLocks,
            XML_noChangeAspect, "1", XML_noChangeArrowheads, "1" );
    m_pSerializer->endElementNS( XML_pic, XML_cNvPicPr );
    m_pSerializer->endElementNS( XML_pic, XML_nvPicPr );

    // the actual picture
    m_pSerializer->startElementNS(XML_pic, XML_blipFill);

/* At this point we are certain that, WriteImage returns empty RelId
   for unhandled graphic type. Therefore we write the picture description
   and not the relation( coz there ain't any), so that the user knows
   there is an image/graphic in the doc but it is broken instead of
   completely discarding it.
*/
    if ( aRelId.isEmpty() )
        m_pSerializer->startElementNS(XML_a, XML_blip);
    else
        m_pSerializer->startElementNS(XML_a, XML_blip, FSNS(XML_r, nImageType), aRelId);

    pItem = nullptr;

    if ( pGrfNode && SfxItemState::SET == pGrfNode->GetSwAttrSet().GetItemState(RES_GRFATR_DRAWMODE, true, &pItem))
    {
        GraphicDrawMode nMode = static_cast<GraphicDrawMode>(static_cast<const SfxEnumItemInterface*>(pItem)->GetEnumValue());
        if (nMode == GraphicDrawMode::Greys)
            m_pSerializer->singleElementNS (XML_a, XML_grayscl);
        else if (nMode == GraphicDrawMode::Mono) //black/white has a 0,5 threshold in LibreOffice
            m_pSerializer->singleElementNS (XML_a, XML_biLevel, XML_thresh, OString::number(50000));
        else if (nMode == GraphicDrawMode::Watermark) //watermark has a brightness/luminance of 0,5 and contrast of -0.7 in LibreOffice
            m_pSerializer->singleElementNS( XML_a, XML_lum, XML_bright, OString::number(70000), XML_contrast, OString::number(-70000) );
    }
    m_pSerializer->endElementNS( XML_a, XML_blip );

    if (xShapePropSet)
        WriteSrcRect(xShapePropSet, pFrameFormat);

    m_pSerializer->startElementNS(XML_a, XML_stretch);
    m_pSerializer->singleElementNS(XML_a, XML_fillRect);
    m_pSerializer->endElementNS( XML_a, XML_stretch );
    m_pSerializer->endElementNS( XML_pic, XML_blipFill );

    // TODO setup the right values below
    m_pSerializer->startElementNS(XML_pic, XML_spPr, XML_bwMode, "auto");

    m_pSerializer->startElementNS(XML_a, XML_xfrm, xFrameAttributes);

    m_pSerializer->singleElementNS(XML_a, XML_off, XML_x, "0", XML_y, "0");
    OString aWidth( OString::number( TwipsToEMU( aSize.Width() ) ) );
    OString aHeight( OString::number( TwipsToEMU( aSize.Height() ) ) );
    m_pSerializer->singleElementNS(XML_a, XML_ext, XML_cx, aWidth, XML_cy, aHeight);
    m_pSerializer->endElementNS( XML_a, XML_xfrm );
    m_pSerializer->startElementNS(XML_a, XML_prstGeom, XML_prst, "rect");
    m_pSerializer->singleElementNS(XML_a, XML_avLst);
    m_pSerializer->endElementNS( XML_a, XML_prstGeom );

    const SvxBoxItem& rBoxItem = pFrameFormat->GetBox();
    const SvxBorderLine* pLeft = rBoxItem.GetLine(SvxBoxItemLine::LEFT);
    const SvxBorderLine* pRight = rBoxItem.GetLine(SvxBoxItemLine::RIGHT);
    const SvxBorderLine* pTop = rBoxItem.GetLine(SvxBoxItemLine::TOP);
    const SvxBorderLine* pBottom = rBoxItem.GetLine(SvxBoxItemLine::BOTTOM);
    if (pLeft || pRight || pTop || pBottom)
        m_rExport.SdrExporter().writeBoxItemLine(rBoxItem);

    m_rExport.SdrExporter().writeDMLEffectLst(*pFrameFormat);

    m_pSerializer->endElementNS( XML_pic, XML_spPr );

    m_pSerializer->endElementNS( XML_pic, XML_pic );

    m_pSerializer->endElementNS( XML_a, XML_graphicData );
    m_pSerializer->endElementNS( XML_a, XML_graphic );
    m_rExport.SdrExporter().endDMLAnchorInline(pFrameFormat);
}

void DocxAttributeOutput::WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rOLENode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat, const sal_Int8 nFormulaAlignment )
{
    if( WriteOLEChart( pSdrObj, rSize, pFlyFrameFormat ))
        return;
    if( WriteOLEMath( rOLENode , nFormulaAlignment))
        return;
    PostponeOLE( rOLENode, rSize, pFlyFrameFormat );
}

bool DocxAttributeOutput::WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat )
{
    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY );
    if (!xShape.is())
        return false;

    uno::Reference<beans::XPropertySet> const xPropSet(xShape, uno::UNO_QUERY);
    if (!xPropSet.is())
        return false;

    OUString clsid; // why is the property of type string, not sequence<byte>?
    xPropSet->getPropertyValue("CLSID") >>= clsid;
    assert(!clsid.isEmpty());
    SvGlobalName aClassID;
    bool const isValid(aClassID.MakeId(clsid));
    assert(isValid); (void)isValid;

    if (!SotExchange::IsChart(aClassID))
        return false;

    m_aPostponedCharts.push_back(PostponedChart(pSdrObj, rSize, pFlyFrameFormat));
    return true;
}

/*
 * Write chart hierarchy in w:drawing after end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedChart()
{
    if (m_aPostponedCharts.empty())
        return;

    for (const PostponedChart& rChart : m_aPostponedCharts)
    {
        uno::Reference< chart2::XChartDocument > xChartDoc;
        uno::Reference< drawing::XShape > xShape(const_cast<SdrObject*>(rChart.object)->getUnoShape(), uno::UNO_QUERY );
        if( xShape.is() )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is() )
                xChartDoc.set( xPropSet->getPropertyValue( "Model" ), uno::UNO_QUERY );
        }

        if( xChartDoc.is() )
        {
            SAL_INFO("sw.ww8", "DocxAttributeOutput::WriteOLE2Obj: export chart ");

            m_rExport.SdrExporter().startDMLAnchorInline(rChart.frame, rChart.size);

            OUString sName("Object 1");
            uno::Reference< container::XNamed > xNamed( xShape, uno::UNO_QUERY );
            if( xNamed.is() )
                sName = xNamed->getName();

            /* If there is a scenario where a chart is followed by a shape
               which is being exported as an alternate content then, the
               docPr Id is being repeated, ECMA 20.4.2.5 says that the
               docPr Id should be unique, ensuring the same here.
               */
            m_pSerializer->singleElementNS( XML_wp, XML_docPr,
                    XML_id, OString::number(m_anchorId++),
                    XML_name, sName );

            m_pSerializer->singleElementNS(XML_wp, XML_cNvGraphicFramePr);

            m_pSerializer->startElementNS( XML_a, XML_graphic,
                    FSNS( XML_xmlns, XML_a ), GetExport().GetFilter().getNamespaceURL(OOX_NS(dml)) );

            m_pSerializer->startElementNS( XML_a, XML_graphicData,
                    XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/chart" );

            OString aRelId;
            m_nChartCount++;
            aRelId = m_rExport.OutputChart( xChartDoc, m_nChartCount, m_pSerializer );

            m_pSerializer->singleElementNS( XML_c, XML_chart,
                    FSNS( XML_xmlns, XML_c ), GetExport().GetFilter().getNamespaceURL(OOX_NS(dmlChart)),
                    FSNS( XML_xmlns, XML_r ), GetExport().GetFilter().getNamespaceURL(OOX_NS(officeRel)),
                    FSNS( XML_r, XML_id ), aRelId );

            m_pSerializer->endElementNS( XML_a, XML_graphicData );
            m_pSerializer->endElementNS( XML_a, XML_graphic );

            m_rExport.SdrExporter().endDMLAnchorInline(rChart.frame);
        }
    }

    m_aPostponedCharts.clear();
}

bool DocxAttributeOutput::WriteOLEMath( const SwOLENode& rOLENode ,const sal_Int8 nAlign)
{
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode&>(rOLENode).GetOLEObj().GetOleRef());
    SvGlobalName aObjName(xObj->getClassID());

    if( !SotExchange::IsMath(aObjName) )
        return false;

    try
    {
        PostponedMathObjects aPostponedMathObject;
        aPostponedMathObject.pMathObject = const_cast<SwOLENode*>( &rOLENode);
        aPostponedMathObject.nMathObjAlignment = nAlign;
        m_aPostponedMaths.push_back(aPostponedMathObject);
    }
    catch (const uno::Exception&)
    {
    }
    return true;
}

void DocxAttributeOutput::WritePostponedMath(const SwOLENode* pPostponedMath, sal_Int8 nAlign)
{
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode*>(pPostponedMath)->GetOLEObj().GetOleRef());
    if (embed::EmbedStates::LOADED == xObj->getCurrentState())
    {
        // must be running so there is a Component
        try
        {
            xObj->changeState(embed::EmbedStates::RUNNING);
        }
        catch (const uno::Exception&)
        {
        }
    }
    uno::Reference< uno::XInterface > xInterface( xObj->getComponent(), uno::UNO_QUERY );
    if (!xInterface.is())
    {
        SAL_WARN("sw.ww8", "Broken math object");
        return;
    }
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaExportBase* formulaexport = dynamic_cast<oox::FormulaExportBase*>(dynamic_cast<SfxBaseModel*>(xInterface.get()));
    assert( formulaexport != nullptr );
    if (formulaexport)
        formulaexport->writeFormulaOoxml( m_pSerializer, GetExport().GetFilter().getVersion(),
                oox::drawingml::DOCUMENT_DOCX, nAlign);
}

void DocxAttributeOutput::WritePostponedFormControl(const SdrObject* pObject)
{
    if (!pObject || pObject->GetObjInventor() != SdrInventor::FmForm)
        return;

    SdrUnoObj *pFormObj = const_cast<SdrUnoObj*>(dynamic_cast< const SdrUnoObj*>(pObject));
    if (!pFormObj)
        return;

    uno::Reference<awt::XControlModel> xControlModel = pFormObj->GetUnoControlModel();
    uno::Reference<lang::XServiceInfo> xInfo(xControlModel, uno::UNO_QUERY);
    if (!xInfo.is())
        return;

    if (xInfo->supportsService("com.sun.star.form.component.DateField"))
    {
        // gather component properties

        OUString sDateFormat;
        uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);

        OString sDate;
        OUString aContentText;
        bool bHasDate = false;
        css::util::Date aUNODate;
        if (xPropertySet->getPropertyValue("Date") >>= aUNODate)
        {
            bHasDate = true;
            Date aDate(aUNODate.Day, aUNODate.Month, aUNODate.Year);
            sDate = DateToOString(aDate);
            aContentText = OUString::createFromAscii(DateToDDMMYYYYOString(aDate).getStr());
            sDateFormat = "dd/MM/yyyy";
        }
        else
        {
            aContentText = xPropertySet->getPropertyValue("HelpText").get<OUString>();
            if(sDateFormat.isEmpty())
                sDateFormat = "dd/MM/yyyy"; // Need to set date format even if there is no date set
        }

        // output component

        m_pSerializer->startElementNS(XML_w, XML_sdt);
        m_pSerializer->startElementNS(XML_w, XML_sdtPr);

        if (bHasDate)
            m_pSerializer->startElementNS(XML_w, XML_date, FSNS(XML_w, XML_fullDate), sDate);
        else
            m_pSerializer->startElementNS(XML_w, XML_date);

        m_pSerializer->singleElementNS(XML_w, XML_dateFormat, FSNS(XML_w, XML_val), sDateFormat);
        m_pSerializer->singleElementNS(XML_w, XML_lid,
                                       FSNS(XML_w, XML_val), "en-US");
        m_pSerializer->singleElementNS(XML_w, XML_storeMappedDataAs,
                                       FSNS(XML_w, XML_val), "dateTime");
        m_pSerializer->singleElementNS(XML_w, XML_calendar,
                                       FSNS(XML_w, XML_val), "gregorian");

        m_pSerializer->endElementNS(XML_w, XML_date);
        m_pSerializer->endElementNS(XML_w, XML_sdtPr);

        m_pSerializer->startElementNS(XML_w, XML_sdtContent);
        m_pSerializer->startElementNS(XML_w, XML_r);

        RunText(aContentText);
        m_pSerializer->endElementNS(XML_w, XML_r);
        m_pSerializer->endElementNS(XML_w, XML_sdtContent);

        m_pSerializer->endElementNS(XML_w, XML_sdt);
    }
    else if (xInfo->supportsService("com.sun.star.form.component.ComboBox"))
    {
        // gather component properties

        uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
        OUString sText = xPropertySet->getPropertyValue("Text").get<OUString>();
        const uno::Sequence<OUString> aItems = xPropertySet->getPropertyValue("StringItemList").get< uno::Sequence<OUString> >();

        // output component

        m_pSerializer->startElementNS(XML_w, XML_sdt);
        m_pSerializer->startElementNS(XML_w, XML_sdtPr);

        m_pSerializer->startElementNS(XML_w, XML_dropDownList);

        for (const auto& rItem : aItems)
        {
            m_pSerializer->singleElementNS(XML_w, XML_listItem,
                                           FSNS(XML_w, XML_displayText), rItem,
                                           FSNS(XML_w, XML_value), rItem);
        }

        m_pSerializer->endElementNS(XML_w, XML_dropDownList);
        m_pSerializer->endElementNS(XML_w, XML_sdtPr);

        m_pSerializer->startElementNS(XML_w, XML_sdtContent);
        m_pSerializer->startElementNS(XML_w, XML_r);
        RunText(sText);
        m_pSerializer->endElementNS(XML_w, XML_r);
        m_pSerializer->endElementNS(XML_w, XML_sdtContent);

        m_pSerializer->endElementNS(XML_w, XML_sdt);
    }
}

void DocxAttributeOutput::WritePostponedActiveXControl(bool bInsideRun)
{
    for( const auto & rPostponedDrawing : m_aPostponedActiveXControls )
    {
        WriteActiveXControl(rPostponedDrawing.object, *rPostponedDrawing.frame, bInsideRun);
    }
    m_aPostponedActiveXControls.clear();
}


void DocxAttributeOutput::WriteActiveXControl(const SdrObject* pObject, const SwFrameFormat& rFrameFormat, bool bInsideRun)
{
    SdrUnoObj *pFormObj = const_cast<SdrUnoObj*>(dynamic_cast< const SdrUnoObj*>(pObject));
    if (!pFormObj)
        return;

    uno::Reference<awt::XControlModel> xControlModel = pFormObj->GetUnoControlModel();
    if (!xControlModel.is())
        return;

    const bool bAnchoredInline = rFrameFormat.GetAnchor().GetAnchorId() == static_cast<RndStdIds>(css::text::TextContentAnchorType_AS_CHARACTER);

    if(!bInsideRun)
    {
        m_pSerializer->startElementNS(XML_w, XML_r);
    }

    // w:pict for floating embedded control and w:object for inline embedded control
    if(bAnchoredInline)
        m_pSerializer->startElementNS(XML_w, XML_object);
    else
        m_pSerializer->startElementNS(XML_w, XML_pict);

    // write ActiveX fragment and ActiveX binary
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pObject)->getUnoShape(), uno::UNO_QUERY);
    std::pair<OString,OString> sRelIdAndName = m_rExport.WriteActiveXObject(xShape, xControlModel);

    // VML shape definition
    m_rExport.VMLExporter().SetSkipwzName(true);
    m_rExport.VMLExporter().SetHashMarkForType(true);
    m_rExport.VMLExporter().OverrideShapeIDGen(true, "control_shape_");
    OString sShapeId;
    if(bAnchoredInline)
    {
        sShapeId = m_rExport.VMLExporter().AddInlineSdrObject(*pObject, true);
    }
    else
    {
        SwFormatFollowTextFlow const& rFlow(rFrameFormat.GetFollowTextFlow());
        const SwFormatHoriOrient& rHoriOri = rFrameFormat.GetHoriOrient();
        const SwFormatVertOrient& rVertOri = rFrameFormat.GetVertOrient();
        SwFormatSurround const& rSurround(rFrameFormat.GetSurround());
        rtl::Reference<sax_fastparser::FastAttributeList> pAttrList(docx::SurroundToVMLWrap(rSurround));
        sShapeId = m_rExport.VMLExporter().AddSdrObject(*pObject,
            rFlow.GetValue(),
            rHoriOri.GetHoriOrient(), rVertOri.GetVertOrient(),
            rHoriOri.GetRelationOrient(),
            rVertOri.GetRelationOrient(),
            pAttrList.get(),
            true);
    }
    // Restore default values
    m_rExport.VMLExporter().SetSkipwzName(false);
    m_rExport.VMLExporter().SetHashMarkForType(false);
    m_rExport.VMLExporter().OverrideShapeIDGen(false);

    // control
    m_pSerializer->singleElementNS(XML_w, XML_control,
                                    FSNS(XML_r, XML_id), sRelIdAndName.first,
                                    FSNS(XML_w, XML_name), sRelIdAndName.second,
                                    FSNS(XML_w, XML_shapeid), sShapeId);

    if(bAnchoredInline)
        m_pSerializer->endElementNS(XML_w, XML_object);
    else
        m_pSerializer->endElementNS(XML_w, XML_pict);

    if(!bInsideRun)
    {
        m_pSerializer->endElementNS(XML_w, XML_r);
    }
}

bool DocxAttributeOutput::ExportAsActiveXControl(const SdrObject* pObject) const
{
    SdrUnoObj *pFormObj = const_cast<SdrUnoObj*>(dynamic_cast< const SdrUnoObj*>(pObject));
    if (!pFormObj)
        return false;

    uno::Reference<awt::XControlModel> xControlModel = pFormObj->GetUnoControlModel();
    if (!xControlModel.is())
        return false;

    uno::Reference< css::frame::XModel > xModel( m_rExport.m_rDoc.GetDocShell() ? m_rExport.m_rDoc.GetDocShell()->GetModel() : nullptr );
    if (!xModel.is())
        return false;

    uno::Reference<lang::XServiceInfo> xInfo(xControlModel, uno::UNO_QUERY);
    if (!xInfo.is())
        return false;

    // See WritePostponedFormControl
    // By now date field and combobox is handled on a different way, so let's not interfere with the other method.
    if(xInfo->supportsService("com.sun.star.form.component.DateField") ||
       xInfo->supportsService("com.sun.star.form.component.ComboBox"))
        return false;

    oox::ole::OleFormCtrlExportHelper exportHelper(comphelper::getProcessComponentContext(), xModel, xControlModel);
    return exportHelper.isValid();
}

void DocxAttributeOutput::PostponeOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat )
{
    if( !m_pPostponedOLEs )
        //cannot be postponed, try to write now
        WriteOLE( rNode, rSize, pFlyFrameFormat );
    else
        m_pPostponedOLEs->push_back( PostponedOLE( &rNode, rSize, pFlyFrameFormat ) );
}

/*
 * Write w:object hierarchy for embedded objects after end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedOLE()
{
    if( !m_pPostponedOLEs )
        return;

    for( const auto & rPostponedOLE : *m_pPostponedOLEs )
    {
        WriteOLE( *rPostponedOLE.object, rPostponedOLE.size, rPostponedOLE.frame );
    }

    // clear list of postponed objects
    m_pPostponedOLEs.reset();
}

void DocxAttributeOutput::WriteOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat )
{
    OSL_ASSERT(pFlyFrameFormat);

    // get interoperability information about embedded objects
    uno::Reference< beans::XPropertySet > xPropSet( m_rExport.m_rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aGrabBag, aObjectsInteropList,aObjectInteropAttributes;
    xPropSet->getPropertyValue( UNO_NAME_MISC_OBJ_INTEROPGRABBAG ) >>= aGrabBag;
    auto pProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "EmbeddedObjects"; });
    if (pProp != std::cend(aGrabBag))
        pProp->Value >>= aObjectsInteropList;

    SwOLEObj& aObject = rNode.GetOLEObj();
    uno::Reference < embed::XEmbeddedObject > xObj( aObject.GetOleRef() );
    comphelper::EmbeddedObjectContainer* aContainer = aObject.GetObject().GetContainer();
    OUString sObjectName = aContainer->GetEmbeddedObjectName( xObj );

    // set some attributes according to the type of the embedded object
    OUString sProgID, sDrawAspect;
    switch (rNode.GetAspect())
    {
        case embed::Aspects::MSOLE_CONTENT: sDrawAspect = "Content"; break;
        case embed::Aspects::MSOLE_DOCPRINT: sDrawAspect = "DocPrint"; break;
        case embed::Aspects::MSOLE_ICON: sDrawAspect = "Icon"; break;
        case embed::Aspects::MSOLE_THUMBNAIL: sDrawAspect = "Thumbnail"; break;
        default:
            SAL_WARN("sw.ww8", "DocxAttributeOutput::WriteOLE: invalid aspect value");
    }
    auto pObjectsInterop = std::find_if(std::cbegin(aObjectsInteropList), std::cend(aObjectsInteropList),
        [&sObjectName](const beans::PropertyValue& rProp) { return rProp.Name == sObjectName; });
    if (pObjectsInterop != std::cend(aObjectsInteropList))
        pObjectsInterop->Value >>= aObjectInteropAttributes;

    for( const auto& rObjectInteropAttribute : std::as_const(aObjectInteropAttributes) )
    {
        if ( rObjectInteropAttribute.Name == "ProgID" )
        {
            rObjectInteropAttribute.Value >>= sProgID;
        }
    }

    // write embedded file
    OString sId = m_rExport.WriteOLEObject(aObject, sProgID);

    if( sId.isEmpty() )
    {
        // the embedded file could not be saved
        // fallback: save as an image
        FlyFrameGraphic( nullptr, rSize, pFlyFrameFormat, &rNode );
        return;
    }

    // write preview image
    const Graphic* pGraphic = rNode.GetGraphic();
    m_rDrawingML.SetFS(m_pSerializer);
    OUString sImageId = m_rDrawingML.WriteImage( *pGraphic );

    if ( sDrawAspect == "Content" )
    {
        try
        {
            awt::Size aSize = xObj->getVisualAreaSize( rNode.GetAspect() );

            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( rNode.GetAspect() ) );
            Size aOriginalSize( OutputDevice::LogicToLogic(Size( aSize.Width, aSize.Height),
                                                MapMode(aUnit), MapMode(MapUnit::MapTwip)));

            m_pSerializer->startElementNS( XML_w, XML_object,
                                   FSNS(XML_w, XML_dxaOrig), OString::number(aOriginalSize.Width()),
                                   FSNS(XML_w, XML_dyaOrig), OString::number(aOriginalSize.Height()) );
        }
        catch ( uno::Exception& )
        {
            m_pSerializer->startElementNS(XML_w, XML_object);
        }
    }
    else
    {
        m_pSerializer->startElementNS(XML_w, XML_object);
    }

    OString sShapeId = "ole_" + sId;

    //OLE Shape definition
    WriteOLEShape(*pFlyFrameFormat, rSize, sShapeId, sImageId);

    //OLE Object definition
    m_pSerializer->singleElementNS(XML_o, XML_OLEObject,
                                   XML_Type, "Embed",
                                   XML_ProgID, sProgID,
                                   XML_ShapeID, sShapeId.getStr(),
                                   XML_DrawAspect, sDrawAspect,
                                   XML_ObjectID, "_" + OString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max())),
                                   FSNS( XML_r, XML_id ), sId );

    m_pSerializer->endElementNS(XML_w, XML_object);
}

void DocxAttributeOutput::WriteOLEShape(const SwFlyFrameFormat& rFrameFormat, const Size& rSize,
                                        const OString& rShapeId, const OUString& rImageId)
{
    assert(m_pSerializer);

    //Here is an attribute list where we collect the attributes what we want to export
    rtl::Reference<FastAttributeList> pAttr = FastSerializerHelper::createAttrList();
    pAttr->add(XML_id, rShapeId);

    //export the fixed shape type for picture frame
    m_pSerializer->write(vml::VMLExport::GetVMLShapeTypeDefinition(rShapeId, true));
    pAttr->add(XML_type, "_x0000_t" + rShapeId);

    //Export the style attribute for position and size
    pAttr->add(XML_style, GetOLEStyle(rFrameFormat, rSize));
    //Get the OLE frame
    const SvxBoxItem& rBox = rFrameFormat.GetAttrSet().GetBox();
    OString sLineType;
    OString sDashType;
    //Word does not handle differently the four sides,
    //so we have to choose, and the left one is the winner:
    if (rBox.GetLeft())
    {
        //Get the left border color and width
        const Color aLineColor = rBox.GetLeft()->GetColor();
        const tools::Long aLineWidth = rBox.GetLeft()->GetWidth();

        //Convert the left OLE border style to OOXML
        //FIXME improve if it's necessary
        switch (rBox.GetLeft()->GetBorderLineStyle())
        {
            case SvxBorderLineStyle::SOLID:
                sLineType = OString("Single");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::DASHED:
                sLineType = OString("Single");
                sDashType = OString("Dash");
                break;
            case SvxBorderLineStyle::DASH_DOT:
                sLineType = OString("Single");
                sDashType = OString("DashDot");
                break;
            case SvxBorderLineStyle::DASH_DOT_DOT:
                sLineType = OString("Single");
                sDashType = OString("ShortDashDotDot");
                break;
            case SvxBorderLineStyle::DOTTED:
                sLineType = OString("Single");
                sDashType = OString("Dot");
                break;
            case SvxBorderLineStyle::DOUBLE:
                sLineType = OString("ThinThin");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::DOUBLE_THIN:
                sLineType = OString("ThinThin");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::EMBOSSED:
                sLineType = OString("Single");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::ENGRAVED:
                sLineType = OString("Single");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::FINE_DASHED:
                sLineType = OString("Single");
                sDashType = OString("Dot");
                break;
            case SvxBorderLineStyle::INSET:
                sLineType = OString("Single");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::OUTSET:
                sLineType = OString("Single");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
            case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
                sLineType = OString("ThickThin");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
            case SvxBorderLineStyle::THINTHICK_SMALLGAP:
                sLineType = OString("ThinThick");
                sDashType = OString("Solid");
                break;
            case SvxBorderLineStyle::NONE:
                sLineType = OString("");
                sDashType = OString("");
                break;
            default:
                SAL_WARN("sw.ww8", "Unknown line type on OOXML ELE export!");
                break;
        }

        //If there is a line add it for export
        if (!sLineType.isEmpty() && !sDashType.isEmpty())
        {
            pAttr->add(XML_stroked, "t");
            pAttr->add(XML_strokecolor, "#" + msfilter::util::ConvertColor(aLineColor));
            pAttr->add(XML_strokeweight, OString::number(aLineWidth / 20) + "pt");
        }
    }

    //Let's check the filltype of the OLE
    switch (rFrameFormat.GetAttrSet().Get(XATTR_FILLSTYLE).GetValue())
    {
        case drawing::FillStyle::FillStyle_SOLID:
        {
            //If solid, we get the color and add it to the exporter
            const Color rShapeColor = rFrameFormat.GetAttrSet().Get(XATTR_FILLCOLOR).GetColorValue();
            pAttr->add(XML_filled, "t");
            pAttr->add(XML_fillcolor, "#" + msfilter::util::ConvertColor(rShapeColor));
            break;
        }
        case drawing::FillStyle::FillStyle_GRADIENT:
        case drawing::FillStyle::FillStyle_HATCH:
        case drawing::FillStyle::FillStyle_BITMAP:
            //TODO
            break;
        case drawing::FillStyle::FillStyle_NONE:
        {
            pAttr->add(XML_filled, "f");
            break;
        }
        default:
            SAL_WARN("sw.ww8", "Unknown fill type on OOXML OLE export!");
            break;
    }
    pAttr->addNS(XML_o, XML_ole, ""); //compulsory, even if it's empty
    m_pSerializer->startElementNS(XML_v, XML_shape, pAttr);//Write the collected attrs...

    if (!sLineType.isEmpty() && !sDashType.isEmpty()) //If there is a line/dash style it is time to export it
    {
        m_pSerializer->singleElementNS(XML_v, XML_stroke, XML_linestyle, sLineType, XML_dashstyle, sDashType);
    }

    // shape filled with the preview image
    m_pSerializer->singleElementNS(XML_v, XML_imagedata,
                                   FSNS(XML_r, XML_id), rImageId,
                                   FSNS(XML_o, XML_title), "");

    //export wrap settings
    if (rFrameFormat.GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR) //As-char objs does not have surround.
        ExportOLESurround(rFrameFormat.GetSurround());

    m_pSerializer->endElementNS(XML_v, XML_shape);
}

OString DocxAttributeOutput::GetOLEStyle(const SwFlyFrameFormat& rFormat, const Size& rSize)
{
    //tdf#131539: Export OLE positions in docx:
    //This string will store the position output for the xml
    OString aPos;
    //This string will store the relative position for aPos
    OString aAnch;

    if (rFormat.GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
    {
        //Get the horizontal alignment of the OLE via the frame format, to aHAlign
        OString aHAlign = convertToOOXMLHoriOrient(rFormat.GetHoriOrient().GetHoriOrient(),
            rFormat.GetHoriOrient().IsPosToggle());
        //Get the vertical alignment of the OLE via the frame format to aVAlign
        OString aVAlign = convertToOOXMLVertOrient(rFormat.GetVertOrient().GetVertOrient());

        // Check if the OLE anchored to page:
        const bool bIsPageAnchor = rFormat.GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE;

        //Get the relative horizontal positions for the anchors
        OString aHAnch
            = bIsPageAnchor
                  ? OString("page")
                  : convertToOOXMLHoriOrientRel(rFormat.GetHoriOrient().GetRelationOrient());
        //Get the relative vertical positions for the anchors
        OString aVAnch = convertToOOXMLVertOrientRel(rFormat.GetVertOrient().GetRelationOrient());

        //Choice that the horizontal position is relative or not
        if (!aHAlign.isEmpty())
            aHAlign = ";mso-position-horizontal:" + aHAlign;
        aHAlign = ";mso-position-horizontal-relative:" + aHAnch;

        //Choice that the vertical position is relative or not
        if (!aVAlign.isEmpty())
            aVAlign = ";mso-position-vertical:" + aVAlign;
        aVAlign = ";mso-position-vertical-relative:" + aVAnch;

        //Set the anchoring information into one string for aPos
        aAnch = aHAlign + aVAlign;

        //Query the positions to aPos from frameformat
        aPos =
            "position:absolute;margin-left:" + OString::number(double(rFormat.GetHoriOrient().GetPos()) / 20) +
            "pt;margin-top:" + OString::number(double(rFormat.GetVertOrient().GetPos()) / 20) + "pt;";
    }

    OString sShapeStyle = "width:" + OString::number( double( rSize.Width() ) / 20 ) +
                        "pt;height:" + OString::number( double( rSize.Height() ) / 20 ) +
                        "pt"; //from VMLExport::AddRectangleDimensions(), it does: value/20

    const SvxLRSpaceItem& rLRSpace = rFormat.GetLRSpace();
    if (rLRSpace.IsExplicitZeroMarginValLeft() || rLRSpace.GetLeft())
        sShapeStyle += ";mso-wrap-distance-left:" + OString::number(double(rLRSpace.GetLeft()) / 20) + "pt";
    if (rLRSpace.IsExplicitZeroMarginValRight() || rLRSpace.GetRight())
        sShapeStyle += ";mso-wrap-distance-right:" + OString::number(double(rLRSpace.GetRight()) / 20) + "pt";
    const SvxULSpaceItem& rULSpace = rFormat.GetULSpace();
    if (rULSpace.GetUpper())
        sShapeStyle += ";mso-wrap-distance-top:" + OString::number(double(rULSpace.GetUpper()) / 20) + "pt";
    if (rULSpace.GetLower())
        sShapeStyle += ";mso-wrap-distance-bottom:" + OString::number(double(rULSpace.GetLower()) / 20) + "pt";

    //Export anchor setting, if it exists
    if (!aPos.isEmpty() && !aAnch.isEmpty())
        sShapeStyle = aPos + sShapeStyle  + aAnch;

    return sShapeStyle;
}

void DocxAttributeOutput::ExportOLESurround(const SwFormatSurround& rWrap)
{
    const bool bIsContour = rWrap.IsContour(); //Has the shape contour or not
    OString sSurround;
    OString sSide;

    //Map the ODF wrap settings to OOXML one
    switch (rWrap.GetSurround())
    {
        case text::WrapTextMode::WrapTextMode_NONE:
            sSurround = OString("topAndBottom");
            break;
        case text::WrapTextMode::WrapTextMode_PARALLEL:
            sSurround = bIsContour ? OString("tight") : OString("square");
            break;
        case text::WrapTextMode::WrapTextMode_DYNAMIC:
            sSide = OString("largest");
            sSurround = bIsContour ? OString("tight") : OString("square");
            break;
        case text::WrapTextMode::WrapTextMode_LEFT:
            sSide = OString("left");
            sSurround = bIsContour ? OString("tight") : OString("square");
            break;
        case text::WrapTextMode::WrapTextMode_RIGHT:
            sSide = OString("right");
            sSurround = bIsContour ? OString("tight") : OString("square");
            break;
        default:
            SAL_WARN("sw.ww8", "Unknown surround type on OOXML export!");
            break;
    }

    //if there is a setting export it:
    if (!sSurround.isEmpty())
    {
        if (sSide.isEmpty())
            m_pSerializer->singleElementNS(XML_w10, XML_wrap, XML_type, sSurround);
        else
            m_pSerializer->singleElementNS(XML_w10, XML_wrap, XML_type, sSurround, XML_side, sSide);
    }
}

void DocxAttributeOutput::WritePostponedCustomShape()
{
    if (!m_pPostponedCustomShape)
        return;

    for( const auto & rPostponedDrawing : *m_pPostponedCustomShape)
    {
        if ( IsAlternateContentChoiceOpen() )
            m_rExport.SdrExporter().writeDMLDrawing(rPostponedDrawing.object, rPostponedDrawing.frame, m_anchorId++);
        else
            m_rExport.SdrExporter().writeDMLAndVMLDrawing(rPostponedDrawing.object, *rPostponedDrawing.frame, m_anchorId++);
    }
    m_pPostponedCustomShape.reset();
}

void DocxAttributeOutput::WritePostponedDMLDrawing()
{
    if (!m_pPostponedDMLDrawings)
        return;

    // Clear the list early, this method may be called recursively.
    std::unique_ptr< std::vector<PostponedDrawing> > pPostponedDMLDrawings(std::move(m_pPostponedDMLDrawings));
    std::unique_ptr< std::vector<PostponedOLE> > pPostponedOLEs(std::move(m_pPostponedOLEs));

    for( const auto & rPostponedDrawing : *pPostponedDMLDrawings )
    {
        // Avoid w:drawing within another w:drawing.
        if ( IsAlternateContentChoiceOpen() && !( m_rExport.SdrExporter().IsDrawingOpen()) )
           m_rExport.SdrExporter().writeDMLDrawing(rPostponedDrawing.object, rPostponedDrawing.frame, m_anchorId++);
        else
            m_rExport.SdrExporter().writeDMLAndVMLDrawing(rPostponedDrawing.object, *rPostponedDrawing.frame, m_anchorId++);
    }

    m_pPostponedOLEs = std::move(pPostponedOLEs);
}

void DocxAttributeOutput::WriteFlyFrame(const ww8::Frame& rFrame)
{
    m_pSerializer->mark(Tag_OutputFlyFrame);

    switch ( rFrame.GetWriterType() )
    {
        case ww8::Frame::eGraphic:
            {
                const SdrObject* pSdrObj = rFrame.GetFrameFormat().FindRealSdrObject();
                const SwNode *pNode = rFrame.GetContent();
                const SwGrfNode *pGrfNode = pNode ? pNode->GetGrfNode() : nullptr;
                if ( pGrfNode )
                {
                    if (!m_pPostponedGraphic)
                    {
                        m_bPostponedProcessingFly = false ;
                        FlyFrameGraphic( pGrfNode, rFrame.GetLayoutSize(), nullptr, nullptr, pSdrObj);
                    }
                    else // we are writing out attributes, but w:drawing should not be inside w:rPr,
                    {    // so write it out later
                        m_bPostponedProcessingFly = true ;
                        m_pPostponedGraphic->push_back(PostponedGraphic(pGrfNode, rFrame.GetLayoutSize(), pSdrObj));
                    }
                }
            }
            break;
        case ww8::Frame::eDrawing:
            {
                const SdrObject* pSdrObj = rFrame.GetFrameFormat().FindRealSdrObject();
                if ( pSdrObj )
                {
                    uno::Reference<drawing::XShape> xShape(
                        const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY);

                    if (xShape.is() && oox::drawingml::DrawingML::IsDiagram(xShape))
                    {
                        if ( !m_pPostponedDiagrams )
                        {
                            m_bPostponedProcessingFly = false ;
                            m_rExport.SdrExporter().writeDiagram( pSdrObj, rFrame.GetFrameFormat(), m_anchorId++);
                        }
                        else // we are writing out attributes, but w:drawing should not be inside w:rPr,
                        {    // so write it out later
                            m_bPostponedProcessingFly = true ;
                            m_pPostponedDiagrams->push_back( PostponedDiagram( pSdrObj, &(rFrame.GetFrameFormat()) ));
                        }
                    }
                    else
                    {
                        if (!m_pPostponedDMLDrawings)
                        {
                            if ( IsAlternateContentChoiceOpen() )
                            {
                                // Do not write w:drawing inside w:drawing. Instead Postpone the Inner Drawing.
                                if( m_rExport.SdrExporter().IsDrawingOpen() )
                                    m_pPostponedCustomShape->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat())));
                                else
                                    m_rExport.SdrExporter().writeDMLDrawing( pSdrObj, &rFrame.GetFrameFormat(), m_anchorId++);
                            }
                            else
                                m_rExport.SdrExporter().writeDMLAndVMLDrawing( pSdrObj, rFrame.GetFrameFormat(), m_anchorId++);

                            m_bPostponedProcessingFly = false ;
                        }
                        // IsAlternateContentChoiceOpen(): check is to ensure that only one object is getting added. Without this check, plus one object gets added
                        // m_bParagraphFrameOpen: check if the frame is open.
                        else if (IsAlternateContentChoiceOpen() && m_bParagraphFrameOpen)
                            m_pPostponedCustomShape->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat())));
                        else
                        {
                            // we are writing out attributes, but w:drawing should not be inside w:rPr, so write it out later
                            m_bPostponedProcessingFly = true ;
                            m_pPostponedDMLDrawings->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat())));
                        }
                    }
                }
            }
            break;
        case ww8::Frame::eTextBox:
            {
                // If this is a TextBox of a shape, then ignore: it's handled in WriteTextBox().
                if (DocxSdrExport::isTextBox(rFrame.GetFrameFormat()))
                    break;

                // If this is a TextBox containing a table which we already exported directly, ignore it
                if (m_aFloatingTablesOfParagraph.find(&rFrame.GetFrameFormat()) != m_aFloatingTablesOfParagraph.end())
                    break;

                // The frame output is postponed to the end of the anchor paragraph
                bool bDuplicate = false;
                const OUString& rName = rFrame.GetFrameFormat().GetName();
                unsigned nSize = m_aFramesOfParagraph.size() ? m_aFramesOfParagraph.top().size() : 0;
                for( unsigned nIndex = 0; nIndex < nSize; ++nIndex )
                {
                    const OUString& rNameExisting = m_aFramesOfParagraph.top()[nIndex].GetFrameFormat().GetName();

                    if (!rName.isEmpty() && !rNameExisting.isEmpty())
                    {
                        if (rName == rNameExisting)
                            bDuplicate = true;
                    }
                }

                if( !bDuplicate )
                {
                    m_bPostponedProcessingFly = true ;
                    if ( m_aFramesOfParagraph.size() )
                        m_aFramesOfParagraph.top().emplace_back(rFrame);
                }
            }
            break;
        case ww8::Frame::eOle:
            {
                const SwFrameFormat &rFrameFormat = rFrame.GetFrameFormat();
                const SdrObject *pSdrObj = rFrameFormat.FindRealSdrObject();
                if ( pSdrObj )
                {
                    SwNodeIndex aIdx(*rFrameFormat.GetContent().GetContentIdx(), 1);
                    SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();

                    //output variable for the formula alignment (default inline)
                    sal_Int8 nAlign(FormulaExportBase::eFormulaAlign::INLINE);
                    auto xObj(rOLENd.GetOLEObj().GetOleRef()); //get the xObject of the formula

                    //tdf133030: Export formula position
                    //If we have a formula with inline anchor...
                    if(SotExchange::IsMath(xObj->getClassID()) && rFrame.IsInline())
                    {
                        SwPosition const* const aAPos = rFrameFormat.GetAnchor().GetContentAnchor();
                        if(aAPos)
                        {
                            //Get the text node what the formula anchored to
                            const SwTextNode* pTextNode = aAPos->nNode.GetNode().GetTextNode();
                            if(pTextNode && pTextNode->Len() == 1)
                            {
                                //Get the paragraph alignment
                                auto aParaAdjust = pTextNode->GetSwAttrSet().GetAdjust().GetAdjust();
                                //And set the formula according to the paragraph alignment
                                if (aParaAdjust == SvxAdjust::Center)
                                    nAlign = FormulaExportBase::eFormulaAlign::CENTER;
                                else if (aParaAdjust == SvxAdjust::Right)
                                    nAlign = FormulaExportBase::eFormulaAlign::RIGHT;
                                else // left in the case of left and justified paragraph alignments
                                    nAlign = FormulaExportBase::eFormulaAlign::LEFT;
                            }
                        }
                    }
                    WriteOLE2Obj( pSdrObj, rOLENd, rFrame.GetLayoutSize(), dynamic_cast<const SwFlyFrameFormat*>( &rFrameFormat ), nAlign);
                    m_bPostponedProcessingFly = false ;
                }
            }
            break;
        case ww8::Frame::eFormControl:
            {
                const SdrObject* pObject = rFrame.GetFrameFormat().FindRealSdrObject();
                if(ExportAsActiveXControl(pObject))
                    m_aPostponedActiveXControls.emplace_back(pObject, &(rFrame.GetFrameFormat()));
                else
                    m_aPostponedFormControls.push_back(pObject);
                m_bPostponedProcessingFly = true ;
            }
            break;
        default:
            SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::OutputFlyFrame_Impl( const ww8::Frame& rFrame ) - frame type " <<
                    ( rFrame.GetWriterType() == ww8::Frame::eTextBox ? "eTextBox":
                      ( rFrame.GetWriterType() == ww8::Frame::eOle ? "eOle": "???" ) ) );
            break;
    }

    m_pSerializer->mergeTopMarks(Tag_OutputFlyFrame);
}

void DocxAttributeOutput::OutputFlyFrame_Impl(const ww8::Frame& rFrame, const Point& /*rNdTopLeft*/)
{
    /// The old OutputFlyFrame_Impl() moved to WriteFlyFrame().
    /// Now if a frame anchored inside another frame, it will
    /// not be exported immediately, because OOXML does not
    /// support that feature, instead it postponed and exported
    /// later when the original shape closed.

    if (rFrame.GetFrameFormat().GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR
        || rFrame.IsInline())
    {
        m_nEmbedFlyLevel++;
        WriteFlyFrame(rFrame);
        m_nEmbedFlyLevel--;
        return;
    }

    if (m_nEmbedFlyLevel == 0)
    {
        if (m_vPostponedFlys.empty())
        {
            m_nEmbedFlyLevel++;
            WriteFlyFrame(rFrame);
            m_nEmbedFlyLevel--;
        }
        else
            for (auto it = m_vPostponedFlys.begin(); it != m_vPostponedFlys.end();)
            {
                m_nEmbedFlyLevel++;
                WriteFlyFrame(*it);
                it = m_vPostponedFlys.erase(it);
                m_nEmbedFlyLevel--;
            }
    }
    else
    {
        bool bFound = false;
        for (const auto& i : m_vPostponedFlys)
        {
            if (i.RefersToSameFrameAs(rFrame))
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            if (auto pParentFly = rFrame.GetContentNode()->GetFlyFormat())
            {
                auto aHori(rFrame.GetFrameFormat().GetHoriOrient());
                aHori.SetPos(aHori.GetPos() + pParentFly->GetHoriOrient().GetPos());
                auto aVori(rFrame.GetFrameFormat().GetVertOrient());
                aVori.SetPos(aVori.GetPos() + pParentFly->GetVertOrient().GetPos());

                const_cast<SwFrameFormat&>(rFrame.GetFrameFormat()).SetFormatAttr(aHori);
                const_cast<SwFrameFormat&>(rFrame.GetFrameFormat()).SetFormatAttr(aVori);
                const_cast<SwFrameFormat&>(rFrame.GetFrameFormat()).SetFormatAttr(pParentFly->GetAnchor());

                m_vPostponedFlys.push_back(rFrame);
            }

        }
    }
}

void DocxAttributeOutput::WriteOutliner(const OutlinerParaObject& rParaObj)
{
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( m_rExport, rEditObj, TXT_HFTXTBOX );

    sal_Int32 nPara = rEditObj.GetParagraphCount();

    m_pSerializer->startElementNS(XML_w, XML_txbxContent);
    for (sal_Int32 n = 0; n < nPara; ++n)
    {
        if( n )
            aAttrIter.NextPara( n );

        OUString aStr( rEditObj.GetText( n ));
        sal_Int32 nCurrentPos = 0;
        sal_Int32 nEnd = aStr.getLength();

        StartParagraph(ww8::WW8TableNodeInfo::Pointer_t(), false);

        // Write paragraph properties.
        StartParagraphProperties();
        aAttrIter.OutParaAttr(false);
        SfxItemSet aParagraphMarkerProperties(m_rExport.m_rDoc.GetAttrPool());
        EndParagraphProperties(aParagraphMarkerProperties, nullptr, nullptr, nullptr);

        do {
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);

            m_pSerializer->startElementNS(XML_w, XML_r);

            // Write run properties.
            m_pSerializer->startElementNS(XML_w, XML_rPr);
            aAttrIter.OutAttr(nCurrentPos);
            WriteCollectedRunProperties();
            m_pSerializer->endElementNS(XML_w, XML_rPr);

            bool bTextAtr = aAttrIter.IsTextAttr( nCurrentPos );
            if( !bTextAtr )
            {
                OUString aOut( aStr.copy( nCurrentPos, nNextAttr - nCurrentPos ) );
                RunText(aOut);
            }

            if ( !m_sRawText.isEmpty() )
            {
                RunText( m_sRawText );
                m_sRawText.clear();
            }

            m_pSerializer->endElementNS( XML_w, XML_r );

            nCurrentPos = nNextAttr;
            aAttrIter.NextPos();
        }
        while( nCurrentPos < nEnd );
        EndParagraph(ww8::WW8TableNodeInfoInner::Pointer_t());
    }
    m_pSerializer->endElementNS( XML_w, XML_txbxContent );
}

void DocxAttributeOutput::pushToTableExportContext(DocxTableExportContext& rContext)
{
    rContext.m_pTableInfo = m_rExport.m_pTableInfo;
    m_rExport.m_pTableInfo = std::make_shared<ww8::WW8TableInfo>();

    rContext.m_bTableCellOpen = m_tableReference->m_bTableCellOpen;
    m_tableReference->m_bTableCellOpen = false;

    rContext.m_nTableDepth = m_tableReference->m_nTableDepth;
    m_tableReference->m_nTableDepth = 0;

    rContext.m_bStartedParaSdt = m_bStartedParaSdt;
    m_bStartedParaSdt = false;
}

void DocxAttributeOutput::popFromTableExportContext(DocxTableExportContext const & rContext)
{
    m_rExport.m_pTableInfo = rContext.m_pTableInfo;
    m_tableReference->m_bTableCellOpen = rContext.m_bTableCellOpen;
    m_tableReference->m_nTableDepth = rContext.m_nTableDepth;
    m_bStartedParaSdt = rContext.m_bStartedParaSdt;
}

void DocxAttributeOutput::WriteTextBox(uno::Reference<drawing::XShape> xShape)
{
    DocxTableExportContext aTableExportContext(*this);

    SwFrameFormat* pTextBox = SwTextBoxHelper::getOtherTextBoxFormat(xShape);
    assert(pTextBox);
    const SwPosition* pAnchor = nullptr;
    const bool bFlyAtPage = pTextBox->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE;
    if (bFlyAtPage) //tdf135711
    {
        auto pNdIdx = pTextBox->GetContent().GetContentIdx();
        if (pNdIdx) //Is that possible it is null?
            pAnchor = new SwPosition(*pNdIdx);
    }
    else
    {
        pAnchor = pTextBox->GetAnchor().GetContentAnchor();//This might be null
    }

    if (pAnchor) //pAnchor can be null, so that's why not assert here.
    {
        ww8::Frame aFrame(*pTextBox, *pAnchor);
        m_rExport.SdrExporter().writeDMLTextFrame(&aFrame, m_anchorId++, /*bTextBoxOnly=*/true);
        if (bFlyAtPage)
        {
            delete pAnchor;
        }
    }
}

void DocxAttributeOutput::WriteVMLTextBox(uno::Reference<drawing::XShape> xShape)
{
    DocxTableExportContext aTableExportContext(*this);

    SwFrameFormat* pTextBox = SwTextBoxHelper::getOtherTextBoxFormat(xShape);
    assert(pTextBox);
    const SwPosition* pAnchor = nullptr;
    if (pTextBox->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE) //tdf135711
    {
        auto pNdIdx = pTextBox->GetContent().GetContentIdx();
        if (pNdIdx) //Is that possible it is null?
            pAnchor = new SwPosition(*pNdIdx);
    }
    else
    {
        pAnchor = pTextBox->GetAnchor().GetContentAnchor();//This might be null
    }

    if (pAnchor) //pAnchor can be null, so that's why not assert here.
    {
        ww8::Frame aFrame(*pTextBox, *pAnchor);
        m_rExport.SdrExporter().writeVMLTextFrame(&aFrame, /*bTextBoxOnly=*/true);
        if (pTextBox->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE)
        {
            delete pAnchor;
        }
    }
}

oox::drawingml::DrawingML& DocxAttributeOutput::GetDrawingML()
{
    return m_rDrawingML;
}

bool DocxAttributeOutput::MaybeOutputBrushItem(SfxItemSet const& rSet)
{
    const XFillStyleItem* pXFillStyleItem(rSet.GetItem<XFillStyleItem>(XATTR_FILLSTYLE));

    if ((pXFillStyleItem && pXFillStyleItem->GetValue() != drawing::FillStyle_NONE)
        || !m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        return false;
    }

    // sw text frames are opaque by default, even with fill none!
    std::unique_ptr<SfxItemSet> const pClone(rSet.Clone());
    XFillColorItem const aColor(OUString(), COL_WHITE);
    pClone->Put(aColor);
    // call getSvxBrushItemForSolid - this also takes XFillTransparenceItem into account
    XFillStyleItem const aSolid(drawing::FillStyle_SOLID);
    pClone->Put(aSolid);
    std::unique_ptr<SvxBrushItem> const pBrush(getSvxBrushItemFromSourceSet(*pClone, RES_BACKGROUND));
    FormatBackground(*pBrush);
    return true;
}

namespace {

/// Functor to do case-insensitive ordering of OUString instances.
struct OUStringIgnoreCase
{
    bool operator() (const OUString& lhs, std::u16string_view rhs) const
    {
        return lhs.compareToIgnoreAsciiCase(rhs) < 0;
    }
};

}

/// Guesses if a style created in Writer (no grab-bag) should be qFormat or not.
static bool lcl_guessQFormat(const OUString& rName, sal_uInt16 nWwId)
{
    // If the style has no dedicated STI number, then it's probably a custom style -> qFormat.
    if (nWwId == ww::stiUser)
        return true;

    // Allow exported built-in styles UI language neutral
    if ( nWwId == ww::stiNormal ||
        ( nWwId>= ww::stiLev1 && nWwId <= ww::stiLev9 ) ||
            nWwId == ww::stiCaption || nWwId == ww::stiTitle ||
            nWwId == ww::stiSubtitle || nWwId == ww::stiStrong ||
            nWwId == ww::stiEmphasis )
        return true;

    static o3tl::sorted_vector<OUString, OUStringIgnoreCase> const aAllowlist
    {
        "No Spacing",
        "List Paragraph",
        "Quote",
        "Intense Quote",
        "Subtle Emphasis,",
        "Intense Emphasis",
        "Subtle Reference",
        "Intense Reference",
        "Book Title",
        "TOC Heading",
    };
    // Not custom style? Then we have a list of standard styles which should be qFormat.
    return aAllowlist.find(rName) != aAllowlist.end();
}

void DocxAttributeOutput::StartStyle( const OUString& rName, StyleType eType,
        sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nLink, sal_uInt16 nWwId, sal_uInt16 nId, bool bAutoUpdate )
{
    bool bQFormat = false, bUnhideWhenUsed = false, bSemiHidden = false, bLocked = false, bDefault = false, bCustomStyle = false;
    OUString aRsid, aUiPriority;
    rtl::Reference<FastAttributeList> pStyleAttributeList = FastSerializerHelper::createAttrList();
    uno::Any aAny;
    if (eType == STYLE_TYPE_PARA || eType == STYLE_TYPE_CHAR)
    {
        const SwFormat* pFormat = m_rExport.m_pStyles->GetSwFormat(nId);
        pFormat->GetGrabBagItem(aAny);
    }
    else
    {
        const SwNumRule* pRule = m_rExport.m_pStyles->GetSwNumRule(nId);
        pRule->GetGrabBagItem(aAny);
    }
    const uno::Sequence<beans::PropertyValue>& rGrabBag = aAny.get< uno::Sequence<beans::PropertyValue> >();

    for (const auto& rProp : rGrabBag)
    {
        if (rProp.Name == "uiPriority")
            aUiPriority = rProp.Value.get<OUString>();
        else if (rProp.Name == "qFormat")
            bQFormat = true;
        else if (rProp.Name == "rsid")
            aRsid = rProp.Value.get<OUString>();
        else if (rProp.Name == "unhideWhenUsed")
            bUnhideWhenUsed = true;
        else if (rProp.Name == "semiHidden")
            bSemiHidden = true;
        else if (rProp.Name == "locked")
            bLocked = true;
        else if (rProp.Name == "default")
            bDefault = rProp.Value.get<bool>();
        else if (rProp.Name == "customStyle")
            bCustomStyle = rProp.Value.get<bool>();
        else
            SAL_WARN("sw.ww8", "Unhandled style property: " << rProp.Name);
    }

    // MSO exports English names and writerfilter only recognize them.
    const char *pEnglishName = nullptr;
    const char* pType = nullptr;
    switch (eType)
    {
        case STYLE_TYPE_PARA:
            pType = "paragraph";
            if ( nWwId < ww::stiMax)
                pEnglishName = ww::GetEnglishNameFromSti( static_cast<ww::sti>(nWwId ) );
            break;
        case STYLE_TYPE_CHAR: pType = "character"; break;
        case STYLE_TYPE_LIST: pType = "numbering"; break;
    }
    pStyleAttributeList->add(FSNS( XML_w, XML_type ), pType);
    pStyleAttributeList->add(FSNS(XML_w, XML_styleId), m_rExport.m_pStyles->GetStyleId(nId));
    if (bDefault)
        pStyleAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pStyleAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    m_pSerializer->startElementNS( XML_w, XML_style, pStyleAttributeList);
    m_pSerializer->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), pEnglishName ? pEnglishName : rName.toUtf8() );

    if ( nBase != 0x0FFF && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_basedOn,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nBase) );
    }

    if ( nNext != nId && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_next,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nNext) );
    }

    if (nLink != 0x0FFF && (eType == STYLE_TYPE_PARA || eType == STYLE_TYPE_CHAR))
    {
        m_pSerializer->singleElementNS(XML_w, XML_link, FSNS(XML_w, XML_val),
                                       m_rExport.m_pStyles->GetStyleId(nLink));
    }

    if ( bAutoUpdate )
        m_pSerializer->singleElementNS(XML_w, XML_autoRedefine);

    if (!aUiPriority.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority, FSNS(XML_w, XML_val), aUiPriority);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed);

    if (bQFormat || lcl_guessQFormat(rName, nWwId))
        m_pSerializer->singleElementNS(XML_w, XML_qFormat);
    if (bLocked)
        m_pSerializer->singleElementNS(XML_w, XML_locked);
    if (!aRsid.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_rsid, FSNS(XML_w, XML_val), aRsid);
}

void DocxAttributeOutput::EndStyle()
{
    m_pSerializer->endElementNS( XML_w, XML_style );
}

void DocxAttributeOutput::StartStyleProperties( bool bParProp, sal_uInt16 /*nStyle*/ )
{
    if ( bParProp )
    {
        m_pSerializer->startElementNS(XML_w, XML_pPr);
        InitCollectedParagraphProperties();
    }
    else
    {
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        InitCollectedRunProperties();
    }
}

void DocxAttributeOutput::EndStyleProperties( bool bParProp )
{
    if ( bParProp )
    {
        WriteCollectedParagraphProperties();

        // Merge the marks for the ordered elements
        m_pSerializer->mergeTopMarks(Tag_InitCollectedParagraphProperties);

        m_pSerializer->endElementNS( XML_w, XML_pPr );
    }
    else
    {
        WriteCollectedRunProperties();

        // Merge the marks for the ordered elements
        m_pSerializer->mergeTopMarks(Tag_InitCollectedRunProperties);

        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }
}

void DocxAttributeOutput::OutlineNumbering(sal_uInt8 const /*nLvl*/)
{
    // Handled by ParaOutlineLevel() instead.
}

void DocxAttributeOutput::ParaOutlineLevel(const SfxUInt16Item& rItem)
{
    sal_uInt16 nOutLvl = std::min(rItem.GetValue(), sal_uInt16(WW8ListManager::nMaxLevel));
    // Outline Level: in LO Body Text = 0, in MS Body Text = 9
    nOutLvl = nOutLvl ? nOutLvl - 1 : 9;
    m_pSerializer->singleElementNS(XML_w, XML_outlineLvl, FSNS(XML_w, XML_val), OString::number(nOutLvl));
}

void DocxAttributeOutput::PageBreakBefore( bool bBreak )
{
    if ( bBreak )
        m_pSerializer->singleElementNS(XML_w, XML_pageBreakBefore);
    else
        m_pSerializer->singleElementNS( XML_w, XML_pageBreakBefore,
                FSNS( XML_w, XML_val ), "false" );
}

void DocxAttributeOutput::SectionBreak( sal_uInt8 nC, bool bBreakAfter, const WW8_SepInfo* pSectionInfo, bool bExtraPageBreak)
{
    switch ( nC )
    {
        case msword::ColumnBreak:
            // The column break should be output in the next paragraph...
            if ( m_nColBreakStatus == COLBRK_WRITE )
                m_nColBreakStatus = COLBRK_WRITEANDPOSTPONE;
            else
                m_nColBreakStatus = COLBRK_POSTPONE;
            break;
        case msword::PageBreak:
            if ( pSectionInfo )
            {
                // Detect when the current node is the last node in the
                // document: the last section is written explicitly in
                // DocxExport::WriteMainText(), don't duplicate that here.
                SwNodeIndex aCurrentNode(m_rExport.m_pCurPam->GetNode());
                SwNodeIndex aLastNode(m_rExport.m_rDoc.GetNodes().GetEndOfContent(), -1);
                bool bEmit = aCurrentNode != aLastNode;

                if (!bEmit)
                {
                    // Need to still emit an empty section at the end of the
                    // document in case balanced columns are wanted, since the last
                    // section in Word is always balanced.
                    sal_uInt16 nColumns = 1;
                    bool bBalance = false;
                    if (const SwSectionFormat* pFormat = pSectionInfo->pSectionFormat)
                    {
                        if (pFormat != reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1)))
                        {
                            nColumns = pFormat->GetCol().GetNumCols();
                            const SwFormatNoBalancedColumns& rNoBalanced = pFormat->GetBalancedColumns();
                            bBalance = !rNoBalanced.GetValue();
                        }
                    }
                    bEmit = (nColumns > 1 && bBalance);
                }

                // don't add section properties if this will be the first
                // paragraph in the document
                if ( !m_bParagraphOpened && !m_bIsFirstParagraph && bEmit )
                {
                    // Create a dummy paragraph if needed
                    m_pSerializer->startElementNS(XML_w, XML_p);
                    m_pSerializer->startElementNS(XML_w, XML_pPr);

                    m_rExport.SectionProperties( *pSectionInfo );

                    m_pSerializer->endElementNS( XML_w, XML_pPr );
                    if (bExtraPageBreak)
                    {
                        m_pSerializer->startElementNS(XML_w, XML_r);
                        m_pSerializer->singleElementNS(XML_w, XML_br, FSNS(XML_w, XML_type), "page");
                        m_pSerializer->endElementNS(XML_w, XML_r);
                    }
                    m_pSerializer->endElementNS( XML_w, XML_p );
                }
                else
                {
                    if (bExtraPageBreak && m_bParagraphOpened)
                    {
                        m_pSerializer->startElementNS(XML_w, XML_r);
                        m_pSerializer->singleElementNS(XML_w, XML_br, FSNS(XML_w, XML_type), "page");
                        m_pSerializer->endElementNS(XML_w, XML_r);
                    }
                    // postpone the output of this; it has to be done inside the
                    // paragraph properties, so remember it until then
                    m_pSectionInfo.reset( new WW8_SepInfo( *pSectionInfo ));
                }
            }
            else if ( m_bParagraphOpened )
            {
                if (bBreakAfter)
                    // tdf#128889
                    m_bPageBreakAfter = true;
                else
                {
                    m_pSerializer->startElementNS(XML_w, XML_r);
                    m_pSerializer->singleElementNS(XML_w, XML_br, FSNS(XML_w, XML_type), "page");
                    m_pSerializer->endElementNS(XML_w, XML_r);
                }
            }
            else
                m_bPostponedPageBreak = true;

            break;
        default:
            SAL_INFO("sw.ww8", "Unknown section break to write: " << nC );
            break;
    }
}

void DocxAttributeOutput::EndParaSdtBlock()
{
    if (m_bStartedParaSdt)
    {
        // Paragraph-level SDT still open? Close it now.
        EndSdtBlock();
        m_bStartedParaSdt = false;
    }
}

void DocxAttributeOutput::StartSection()
{
    m_pSerializer->startElementNS(XML_w, XML_sectPr);
    m_bOpenedSectPr = true;

    // Write the elements in the spec order
    static const sal_Int32 aOrder[] =
    {
        FSNS( XML_w, XML_headerReference ),
        FSNS( XML_w, XML_footerReference ),
        FSNS( XML_w, XML_footnotePr ),
        FSNS( XML_w, XML_endnotePr ),
        FSNS( XML_w, XML_type ),
        FSNS( XML_w, XML_pgSz ),
        FSNS( XML_w, XML_pgMar ),
        FSNS( XML_w, XML_paperSrc ),
        FSNS( XML_w, XML_pgBorders ),
        FSNS( XML_w, XML_lnNumType ),
        FSNS( XML_w, XML_pgNumType ),
        FSNS( XML_w, XML_cols ),
        FSNS( XML_w, XML_formProt ),
        FSNS( XML_w, XML_vAlign ),
        FSNS( XML_w, XML_noEndnote ),
        FSNS( XML_w, XML_titlePg ),
        FSNS( XML_w, XML_textDirection ),
        FSNS( XML_w, XML_bidi ),
        FSNS( XML_w, XML_rtlGutter ),
        FSNS( XML_w, XML_docGrid ),
        FSNS( XML_w, XML_printerSettings ),
        FSNS( XML_w, XML_sectPrChange )
    };

    // postpone the output so that we can later [in EndParagraphProperties()]
    // prepend the properties before the run
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    m_pSerializer->mark(Tag_StartSection, comphelper::containerToSequence(aOrder));
    m_bHadSectPr = true;
}

void DocxAttributeOutput::EndSection()
{
    // Write the section properties
    if ( m_pSectionSpacingAttrList.is() )
    {
        rtl::Reference<FastAttributeList> xAttrList = std::move( m_pSectionSpacingAttrList );
        m_pSerializer->singleElementNS( XML_w, XML_pgMar, xAttrList );
    }

    // Order the elements
    m_pSerializer->mergeTopMarks(Tag_StartSection);

    m_pSerializer->endElementNS( XML_w, XML_sectPr );
    m_bOpenedSectPr = false;
}

void DocxAttributeOutput::SectionFormProtection( bool bProtected )
{
    if ( bProtected )
        m_pSerializer->singleElementNS(XML_w, XML_formProt, FSNS(XML_w, XML_val), "true");
    else
        m_pSerializer->singleElementNS(XML_w, XML_formProt, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::SectionRtlGutter(const SfxBoolItem& rRtlGutter)
{
    if (!rRtlGutter.GetValue())
    {
        return;
    }

    m_pSerializer->singleElementNS(XML_w, XML_rtlGutter);
}

void DocxAttributeOutput::SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo )
{
    rtl::Reference<FastAttributeList> pAttr = FastSerializerHelper::createAttrList();
    pAttr->add( FSNS( XML_w, XML_countBy ), OString::number(rLnNumInfo.GetCountBy()).getStr());
    pAttr->add( FSNS( XML_w, XML_restart ), rLnNumInfo.IsRestartEachPage() ? "newPage" : "continuous" );
    if( rLnNumInfo.GetPosFromLeft())
        pAttr->add( FSNS( XML_w, XML_distance ), OString::number(rLnNumInfo.GetPosFromLeft()).getStr());
    if (nRestartNo > 0)
        // Writer is 1-based, Word is 0-based.
        pAttr->add(FSNS(XML_w, XML_start), OString::number(nRestartNo - 1).getStr());
    m_pSerializer->singleElementNS( XML_w, XML_lnNumType, pAttr );
}

void DocxAttributeOutput::SectionTitlePage()
{
    m_pSerializer->singleElementNS(XML_w, XML_titlePg);
}

void DocxAttributeOutput::SectionPageBorders( const SwFrameFormat* pFormat, const SwFrameFormat* /*pFirstPageFormat*/ )
{
    // Output the margins

    const SvxBoxItem& rBox = pFormat->GetBox( );

    const SvxBorderLine* pLeft = rBox.GetLeft( );
    const SvxBorderLine* pTop = rBox.GetTop( );
    const SvxBorderLine* pRight = rBox.GetRight( );
    const SvxBorderLine* pBottom = rBox.GetBottom( );

    if ( !(pBottom || pTop || pLeft || pRight) )
        return;

    OutputBorderOptions aOutputBorderOptions = lcl_getBoxBorderOptions();

    // Check if there is a shadow item
    const SfxPoolItem* pItem = GetExport().HasItem( RES_SHADOW );
    if ( pItem )
    {
        const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
        aOutputBorderOptions.aShadowLocation = pShadowItem->GetLocation();
    }

    // By top margin, impl_borders() means the distance between the top of the page and the header frame.
    editeng::WordPageMargins aMargins = m_pageMargins;
    HdFtDistanceGlue aGlue(pFormat->GetAttrSet());
    if (aGlue.HasHeader())
        aMargins.nTop = aGlue.dyaHdrTop;
    // Ditto for bottom margin.
    if (aGlue.HasFooter())
        aMargins.nBottom = aGlue.dyaHdrBottom;

    if (pFormat->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::GUTTER_AT_TOP))
    {
        aMargins.nTop += pFormat->GetLRSpace().GetGutterMargin();
    }
    else
    {
        aMargins.nLeft += pFormat->GetLRSpace().GetGutterMargin();
    }

    aOutputBorderOptions.pDistances = std::make_shared<editeng::WordBorderDistances>();
    editeng::BorderDistancesToWord(rBox, aMargins, *aOutputBorderOptions.pDistances);

    // All distances are relative to the text margins
    m_pSerializer->startElementNS(XML_w, XML_pgBorders,
        FSNS(XML_w, XML_display), "allPages",
        FSNS(XML_w, XML_offsetFrom), aOutputBorderOptions.pDistances->bFromEdge ? "page" : "text");

    std::map<SvxBoxItemLine, css::table::BorderLine2> aEmptyMap; // empty styles map
    impl_borders( m_pSerializer, rBox, aOutputBorderOptions, aEmptyMap );

    m_pSerializer->endElementNS( XML_w, XML_pgBorders );

}

void DocxAttributeOutput::SectionBiDi( bool bBiDi )
{
    if ( bBiDi )
        m_pSerializer->singleElementNS(XML_w, XML_bidi);
}

// Converting Numbering Format Code to string
static OString lcl_ConvertNumberingType(sal_Int16 nNumberingType, const SfxItemSet* pOutSet, OString& rFormat, const OString& sDefault = "" )
{
    OString aType = sDefault;

    switch ( nNumberingType )
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  aType = "upperLetter"; break;

        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  aType = "lowerLetter"; break;

        case SVX_NUM_ROMAN_UPPER:           aType = "upperRoman";  break;
        case SVX_NUM_ROMAN_LOWER:           aType = "lowerRoman";  break;
        case SVX_NUM_ARABIC:                aType = "decimal";     break;

        case SVX_NUM_BITMAP:
        case SVX_NUM_CHAR_SPECIAL:          aType = "bullet";      break;

        case style::NumberingType::CHARS_HEBREW: aType = "hebrew2"; break;
        case style::NumberingType::NUMBER_HEBREW: aType = "hebrew1"; break;
        case style::NumberingType::NUMBER_NONE: aType = "none"; break;
        case style::NumberingType::FULLWIDTH_ARABIC: aType="decimalFullWidth"; break;
        case style::NumberingType::TIAN_GAN_ZH: aType="ideographTraditional"; break;
        case style::NumberingType::DI_ZI_ZH: aType="ideographZodiac"; break;
        case style::NumberingType::NUMBER_LOWER_ZH:
            aType="taiwaneseCountingThousand";
            if (pOutSet) {
                const SvxLanguageItem& rLang = pOutSet->Get( RES_CHRATR_CJK_LANGUAGE);
                const LanguageType eLang = rLang.GetLanguage();

                if (LANGUAGE_CHINESE_SIMPLIFIED == eLang) {
                    aType="chineseCountingThousand";
                }
            }
        break;
        case style::NumberingType::NUMBER_UPPER_ZH_TW: aType="ideographLegalTraditional";break;
        case style::NumberingType::NUMBER_UPPER_ZH: aType="chineseLegalSimplified"; break;
        case style::NumberingType::NUMBER_TRADITIONAL_JA: aType="japaneseLegal";break;
        case style::NumberingType::AIU_FULLWIDTH_JA: aType="aiueoFullWidth";break;
        case style::NumberingType::AIU_HALFWIDTH_JA: aType="aiueo";break;
        case style::NumberingType::IROHA_FULLWIDTH_JA: aType="iroha";break;
        case style::NumberingType::IROHA_HALFWIDTH_JA: aType="irohaFullWidth";break;
        case style::NumberingType::HANGUL_SYLLABLE_KO: aType="ganada";break;
        case style::NumberingType::HANGUL_JAMO_KO: aType="chosung";break;
        case style::NumberingType::NUMBER_HANGUL_KO: aType="koreanCounting"; break;
        case style::NumberingType::NUMBER_LEGAL_KO: aType = "koreanLegal"; break;
        case style::NumberingType::NUMBER_DIGITAL_KO: aType = "koreanDigital"; break;
        case style::NumberingType::NUMBER_DIGITAL2_KO: aType = "koreanDigital2"; break;
        case style::NumberingType::CIRCLE_NUMBER: aType="decimalEnclosedCircle"; break;
        case style::NumberingType::CHARS_ARABIC: aType="arabicAlpha"; break;
        case style::NumberingType::CHARS_ARABIC_ABJAD: aType="arabicAbjad"; break;
        case style::NumberingType::CHARS_THAI: aType="thaiLetters"; break;
        case style::NumberingType::CHARS_PERSIAN:
        case style::NumberingType::CHARS_NEPALI: aType="hindiVowels"; break;
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_RU: aType = "russianUpper"; break;
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_RU: aType = "russianLower"; break;
        case style::NumberingType::TEXT_NUMBER: aType="ordinal"; break;
        case style::NumberingType::TEXT_CARDINAL: aType="cardinalText"; break;
        case style::NumberingType::TEXT_ORDINAL: aType="ordinalText"; break;
        case style::NumberingType::SYMBOL_CHICAGO: aType="chicago"; break;
        case style::NumberingType::ARABIC_ZERO: aType = "decimalZero"; break;
        case style::NumberingType::ARABIC_ZERO3:
            aType = "custom";
            rFormat = "001, 002, 003, ...";
            break;
        case style::NumberingType::ARABIC_ZERO4:
            aType = "custom";
            rFormat = "0001, 0002, 0003, ...";
            break;
        case style::NumberingType::ARABIC_ZERO5:
            aType = "custom";
            rFormat = "00001, 00002, 00003, ...";
            break;
/*
        Fallback the rest to the suggested default.
        case style::NumberingType::NATIVE_NUMBERING:
        case style::NumberingType::HANGUL_CIRCLED_JAMO_KO:
        case style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO:
        case style::NumberingType::CHARS_GREEK_UPPER_LETTER:
        case style::NumberingType::CHARS_GREEK_LOWER_LETTER:
        case style::NumberingType::PAGE_DESCRIPTOR:
        case style::NumberingType::TRANSLITERATION:
        case style::NumberingType::CHARS_KHMER:
        case style::NumberingType::CHARS_LAO:
        case style::NumberingType::CHARS_TIBETAN:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_BG:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_BG:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_BG:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_BG:
        case style::NumberingType::CHARS_MYANMAR:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_SR:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_SR:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_SR:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_SR:
*/
        default: break;
    }
    return aType;
}


void DocxAttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, const ::std::optional<sal_uInt16>& oPageRestartNumber )
{
    // FIXME Not called properly with page styles like "First Page"

    rtl::Reference<FastAttributeList> pAttr = FastSerializerHelper::createAttrList();

    // std::nullopt means no restart: then don't output that attribute if it is negative
    if ( oPageRestartNumber )
       pAttr->add( FSNS( XML_w, XML_start ), OString::number( *oPageRestartNumber ) );

    // nNumType corresponds to w:fmt. See WW8Export::GetNumId() for more precisions
    OString aCustomFormat;
    OString aFormat(lcl_ConvertNumberingType(nNumType, nullptr, aCustomFormat));
    if (!aFormat.isEmpty() && aCustomFormat.isEmpty())
        pAttr->add(FSNS(XML_w, XML_fmt), aFormat);

    m_pSerializer->singleElementNS( XML_w, XML_pgNumType, pAttr );

    // see 2.6.12 pgNumType (Page Numbering Settings)
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::SectionPageNumbering()" );
}

void DocxAttributeOutput::SectionType( sal_uInt8 nBreakCode )
{
    /*  break code:   0 No break, 1 New column
        2 New page, 3 Even page, 4 Odd page
        */
    const char* pType;
    switch ( nBreakCode )
    {
        case 1:  pType = "nextColumn"; break;
        case 2:  pType = "nextPage";   break;
        case 3:  pType = "evenPage";   break;
        case 4:  pType = "oddPage";    break;
        default: pType = "continuous"; break;
    }

    m_pSerializer->singleElementNS(XML_w, XML_type, FSNS(XML_w, XML_val), pType);
}

void DocxAttributeOutput::TextVerticalAdjustment( const drawing::TextVerticalAdjust nVA )
{
    switch( nVA )
    {
        case drawing::TextVerticalAdjust_CENTER:
            m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), "center");
            break;
        case drawing::TextVerticalAdjust_BOTTOM:
            m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), "bottom");
            break;
        case drawing::TextVerticalAdjust_BLOCK:  //justify
            m_pSerializer->singleElementNS(XML_w, XML_vAlign, FSNS(XML_w, XML_val), "both");
            break;
        default:
            break;
    }
}

void DocxAttributeOutput::StartFont( const OUString& rFamilyName ) const
{
    m_pSerializer->startElementNS(XML_w, XML_font, FSNS(XML_w, XML_name), rFamilyName);
}

void DocxAttributeOutput::EndFont() const
{
    m_pSerializer->endElementNS( XML_w, XML_font );
}

void DocxAttributeOutput::FontAlternateName( const OUString& rName ) const
{
    m_pSerializer->singleElementNS(XML_w, XML_altName, FSNS(XML_w, XML_val), rName);
}

void DocxAttributeOutput::FontCharset( sal_uInt8 nCharSet, rtl_TextEncoding nEncoding ) const
{
    rtl::Reference<FastAttributeList> pAttr = FastSerializerHelper::createAttrList();

    OString aCharSet( OString::number( nCharSet, 16 ) );
    if ( aCharSet.getLength() == 1 )
        aCharSet = "0" + aCharSet;
    pAttr->add(FSNS(XML_w, XML_val), aCharSet);

    if( GetExport().GetFilter().getVersion( ) != oox::core::ECMA_DIALECT )
    {
        if( const char* charset = rtl_getMimeCharsetFromTextEncoding( nEncoding ))
            pAttr->add( FSNS( XML_w, XML_characterSet ), charset );
    }

    m_pSerializer->singleElementNS( XML_w, XML_charset, pAttr );
}

void DocxAttributeOutput::FontFamilyType( FontFamily eFamily ) const
{
    const char* pFamily;
    switch ( eFamily )
    {
        case FAMILY_ROMAN:      pFamily = "roman"; break;
        case FAMILY_SWISS:      pFamily = "swiss"; break;
        case FAMILY_MODERN:     pFamily = "modern"; break;
        case FAMILY_SCRIPT:     pFamily = "script"; break;
        case FAMILY_DECORATIVE: pFamily = "decorative"; break;
        default:                pFamily = "auto"; break; // no font family
    }

    m_pSerializer->singleElementNS(XML_w, XML_family, FSNS(XML_w, XML_val), pFamily);
}

void DocxAttributeOutput::FontPitchType( FontPitch ePitch ) const
{
    const char* pPitch;
    switch ( ePitch )
    {
        case PITCH_VARIABLE: pPitch = "variable"; break;
        case PITCH_FIXED:    pPitch = "fixed"; break;
        default:             pPitch = "default"; break; // no info about the pitch
    }

    m_pSerializer->singleElementNS(XML_w, XML_pitch, FSNS(XML_w, XML_val), pPitch);
}

void DocxAttributeOutput::EmbedFont( std::u16string_view name, FontFamily family, FontPitch pitch )
{
    if( !m_rExport.m_rDoc.getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        return; // no font embedding with this document
    EmbedFontStyle( name, XML_embedRegular, family, ITALIC_NONE, WEIGHT_NORMAL, pitch );
    EmbedFontStyle( name, XML_embedBold, family, ITALIC_NONE, WEIGHT_BOLD, pitch );
    EmbedFontStyle( name, XML_embedItalic, family, ITALIC_NORMAL, WEIGHT_NORMAL, pitch );
    EmbedFontStyle( name, XML_embedBoldItalic, family, ITALIC_NORMAL, WEIGHT_BOLD, pitch );
}

static char toHexChar( int value )
{
    return value >= 10 ? value + 'A' - 10 : value + '0';
}

void DocxAttributeOutput::EmbedFontStyle( std::u16string_view name, int tag, FontFamily family, FontItalic italic,
    FontWeight weight, FontPitch pitch )
{
    // Embed font if at least viewing is allowed (in which case the opening app must check
    // the font license rights too and open either read-only or not use the font for editing).
    OUString fontUrl = EmbeddedFontsHelper::fontFileUrl( name, family, italic, weight, pitch,
        EmbeddedFontsHelper::FontRights::ViewingAllowed );
    if( fontUrl.isEmpty())
        return;
    // TODO IDocumentSettingAccess::EMBED_SYSTEM_FONTS
    if( !fontFilesMap.count( fontUrl ))
    {
        osl::File file( fontUrl );
        if( file.open( osl_File_OpenFlag_Read ) != osl::File::E_None )
            return;
        uno::Reference< css::io::XOutputStream > xOutStream = m_rExport.GetFilter().openFragmentStream(
            "word/fonts/font" + OUString::number(m_nextFontId) + ".odttf",
            "application/vnd.openxmlformats-officedocument.obfuscatedFont" );
        // Not much point in trying hard with the obfuscation key, whoever reads the spec can read the font anyway,
        // so just alter the first and last part of the key.
        char fontKeyStr[] = "{00014A78-CABC-4EF0-12AC-5CD89AEFDE00}";
        sal_uInt8 fontKey[ 16 ] = { 0, 0xDE, 0xEF, 0x9A, 0xD8, 0x5C, 0xAC, 0x12, 0xF0, 0x4E,
            0xBC, 0xCA, 0x78, 0x4A, 0x01, 0 };
        fontKey[ 0 ] = fontKey[ 15 ] = m_nextFontId % 256;
        fontKeyStr[ 1 ] = fontKeyStr[ 35 ] = toHexChar(( m_nextFontId % 256 ) / 16 );
        fontKeyStr[ 2 ] = fontKeyStr[ 36 ] = toHexChar(( m_nextFontId % 256 ) % 16 );
        unsigned char buffer[ 4096 ];
        sal_uInt64 readSize;
        file.read( buffer, 32, readSize );
        if( readSize < 32 )
        {
            SAL_WARN( "sw.ww8", "Font file size too small (" << fontUrl << ")" );
            xOutStream->closeOutput();
            return;
        }
        for( int i = 0;
             i < 16;
             ++i )
        {
            buffer[ i ] ^= fontKey[ i ];
            buffer[ i + 16 ] ^= fontKey[ i ];
        }
        xOutStream->writeBytes( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( buffer ), 32 ));
        for(;;)
        {
            sal_Bool eof;
            if( file.isEndOfFile( &eof ) != osl::File::E_None )
            {
                SAL_WARN( "sw.ww8", "Error reading font file " << fontUrl );
                xOutStream->closeOutput();
                return;
            }
            if( eof )
                break;
            if( file.read( buffer, 4096, readSize ) != osl::File::E_None )
            {
                SAL_WARN( "sw.ww8", "Error reading font file " << fontUrl );
                xOutStream->closeOutput();
                return;
            }
            if( readSize == 0 )
                break;
            // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
            xOutStream->writeBytes( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( buffer ), readSize ));
        }
        xOutStream->closeOutput();
        OString relId = OUStringToOString( GetExport().GetFilter().addRelation( m_pSerializer->getOutputStream(),
            oox::getRelationship(Relationship::FONT),
            OUStringConcatenation("fonts/font" + OUString::number( m_nextFontId ) + ".odttf") ), RTL_TEXTENCODING_UTF8 );
        EmbeddedFontRef ref;
        ref.relId = relId;
        ref.fontKey = fontKeyStr;
        fontFilesMap[ fontUrl ] = ref;
        ++m_nextFontId;
    }
    m_pSerializer->singleElementNS( XML_w, tag,
        FSNS( XML_r, XML_id ), fontFilesMap[ fontUrl ].relId,
        FSNS( XML_w, XML_fontKey ), fontFilesMap[ fontUrl ].fontKey );
}

OString DocxAttributeOutput::TransHighlightColor( sal_uInt8 nIco )
{
    switch (nIco)
    {
        case 0: return "none"; break;
        case 1: return "black"; break;
        case 2: return "blue"; break;
        case 3: return "cyan"; break;
        case 4: return "green"; break;
        case 5: return "magenta"; break;
        case 6: return "red"; break;
        case 7: return "yellow"; break;
        case 8: return "white"; break;
        case 9: return "darkBlue"; break;
        case 10: return "darkCyan"; break;
        case 11: return "darkGreen"; break;
        case 12: return "darkMagenta"; break;
        case 13: return "darkRed"; break;
        case 14: return "darkYellow"; break;
        case 15: return "darkGray"; break;
        case 16: return "lightGray"; break;
        default: return OString(); break;
    }
}

void DocxAttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule )
{
    // nId is the same both for abstract numbering definition as well as the
    // numbering definition itself
    // TODO check that this is actually true & fix if not ;-)
    OString aId( OString::number( nId ) );

    m_pSerializer->startElementNS(XML_w, XML_num, FSNS(XML_w, XML_numId), aId);

    m_pSerializer->singleElementNS(XML_w, XML_abstractNumId, FSNS(XML_w, XML_val), aId);

#if OSL_DEBUG_LEVEL > 1
    // TODO ww8 version writes this, anything to do about it here?
    if ( rRule.IsContinusNum() )
        SAL_INFO("sw", "TODO DocxAttributeOutput::NumberingDefinition()" );
#else
    (void) rRule; // to quiet the warning...
#endif

    m_pSerializer->endElementNS( XML_w, XML_num );
}

// Not all attributes of SwNumFormat are important for export, so can't just use embedded in
// that classes comparison.
static bool lcl_ListLevelsAreDifferentForExport(const SwNumFormat & rFormat1, const SwNumFormat & rFormat2)
{
    if (rFormat1 == rFormat2)
        // They are equal, nothing to do
        return false;

    if (!rFormat1.GetCharFormat() != !rFormat2.GetCharFormat())
        // One has charformat, other not. they are different
        return true;

    if (rFormat1.GetCharFormat() && rFormat2.GetCharFormat())
    {
        const SwAttrSet & a1 = rFormat1.GetCharFormat()->GetAttrSet();
        const SwAttrSet & a2 = rFormat2.GetCharFormat()->GetAttrSet();

        if (!(a1 == a2))
            // Difference in charformat: they are different
            return true;
    }

    // Compare numformats with empty charformats
    SwNumFormat modified1 = rFormat1;
    SwNumFormat modified2 = rFormat2;
    modified1.SetCharFormatName(OUString());
    modified2.SetCharFormatName(OUString());
    modified1.SetCharFormat(nullptr);
    modified2.SetCharFormat(nullptr);
    return modified1 != modified2;
}

void DocxAttributeOutput::OverrideNumberingDefinition(
        SwNumRule const& rRule,
        sal_uInt16 const nNum, sal_uInt16 const nAbstractNum, const std::map< size_t, size_t > & rLevelOverrides )
{
    m_pSerializer->startElementNS(XML_w, XML_num, FSNS(XML_w, XML_numId), OString::number(nNum));

    m_pSerializer->singleElementNS(XML_w, XML_abstractNumId, FSNS(XML_w, XML_val), OString::number(nAbstractNum));

    SwNumRule const& rAbstractRule = *(*m_rExport.m_pUsedNumTable)[nAbstractNum - 1];
    sal_uInt8 const nLevels = static_cast<sal_uInt8>(rRule.IsContinusNum()
        ? WW8ListManager::nMinLevel : WW8ListManager::nMaxLevel);
    for (sal_uInt8 nLevel = 0; nLevel < nLevels; ++nLevel)
    {
        const auto levelOverride = rLevelOverrides.find(nLevel);
        bool bListsAreDifferent = lcl_ListLevelsAreDifferentForExport(rRule.Get(nLevel), rAbstractRule.Get(nLevel));

        // Export list override only if it is different to abstract one
        // or we have a level numbering override
        if (bListsAreDifferent || levelOverride != rLevelOverrides.end())
        {
            m_pSerializer->startElementNS(XML_w, XML_lvlOverride, FSNS(XML_w, XML_ilvl), OString::number(nLevel));

            if (bListsAreDifferent)
            {
                GetExport().NumberingLevel(rRule, nLevel);
            }
            if (levelOverride != rLevelOverrides.end())
            {
                // list numbering restart override
                m_pSerializer->singleElementNS(XML_w, XML_startOverride,
                    FSNS(XML_w, XML_val), OString::number(levelOverride->second));
            }

            m_pSerializer->endElementNS(XML_w, XML_lvlOverride);
        }
    }

    m_pSerializer->endElementNS( XML_w, XML_num );
}

void DocxAttributeOutput::StartAbstractNumbering( sal_uInt16 nId )
{
    const SwNumRule* pRule = (*m_rExport.m_pUsedNumTable)[nId - 1];
    m_bExportingOutline = pRule && pRule->IsOutlineRule();
    m_pSerializer->startElementNS( XML_w, XML_abstractNum,
            FSNS( XML_w, XML_abstractNumId ), OString::number(nId) );
}

void DocxAttributeOutput::EndAbstractNumbering()
{
    m_pSerializer->endElementNS( XML_w, XML_abstractNum );
}

void DocxAttributeOutput::NumberingLevel( sal_uInt8 nLevel,
        sal_uInt16 nStart,
        sal_uInt16 nNumberingType,
        SvxAdjust eAdjust,
        const sal_uInt8 * /*pNumLvlPos*/,
        sal_uInt8 nFollow,
        const wwFont *pFont,
        const SfxItemSet *pOutSet,
        sal_Int16 nIndentAt,
        sal_Int16 nFirstLineIndex,
        sal_Int16 nListTabPos,
        const OUString &rNumberingString,
        const SvxBrushItem* pBrush)
{
    m_pSerializer->startElementNS(XML_w, XML_lvl, FSNS(XML_w, XML_ilvl), OString::number(nLevel));

    // start with the nStart value. Do not write w:start if Numbered Lists
    // starts from zero.As it's an optional parameter.
    // refer ECMA 376 Second edition Part-1
    if(0 != nLevel || 0 != nStart)
    {
        m_pSerializer->singleElementNS( XML_w, XML_start,
                FSNS( XML_w, XML_val ), OString::number(nStart) );
    }

    if (m_bExportingOutline)
    {
        sal_uInt16 nId = m_rExport.m_pStyles->GetHeadingParagraphStyleId( nLevel );
        if ( nId != SAL_MAX_UINT16 )
            m_pSerializer->singleElementNS( XML_w, XML_pStyle ,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nId) );
    }
    // format
    OString aCustomFormat;
    OString aFormat(lcl_ConvertNumberingType(nNumberingType, pOutSet, aCustomFormat, "decimal"));

    {
        if (aCustomFormat.isEmpty())
        {
            m_pSerializer->singleElementNS(XML_w, XML_numFmt, FSNS(XML_w, XML_val), aFormat);
        }
        else
        {
            m_pSerializer->startElementNS(XML_mc, XML_AlternateContent);
            m_pSerializer->startElementNS(XML_mc, XML_Choice, XML_Requires, "w14");

            m_pSerializer->singleElementNS(XML_w, XML_numFmt, FSNS(XML_w, XML_val), aFormat,
                                           FSNS(XML_w, XML_format), aCustomFormat);

            m_pSerializer->endElementNS(XML_mc, XML_Choice);
            m_pSerializer->startElementNS(XML_mc, XML_Fallback);
            m_pSerializer->singleElementNS(XML_w, XML_numFmt, FSNS(XML_w, XML_val), "decimal");
            m_pSerializer->endElementNS(XML_mc, XML_Fallback);
            m_pSerializer->endElementNS(XML_mc, XML_AlternateContent);
        }
    }

    // suffix
    const char *pSuffix = nullptr;
    switch ( nFollow )
    {
        case 1:  pSuffix = "space";   break;
        case 2:  pSuffix = "nothing"; break;
        default: /*pSuffix = "tab";*/ break;
    }
    if ( pSuffix )
        m_pSerializer->singleElementNS(XML_w, XML_suff, FSNS(XML_w, XML_val), pSuffix);

    // text
    OUStringBuffer aBuffer( rNumberingString.getLength() + WW8ListManager::nMaxLevel );

    const sal_Unicode *pPrev = rNumberingString.getStr();
    const sal_Unicode *pIt = rNumberingString.getStr();
    while ( pIt < rNumberingString.getStr() + rNumberingString.getLength() )
    {
        // convert the level values to %NUMBER form
        // (we don't use pNumLvlPos at all)
        // FIXME so far we support the ww8 limit of levels only
        if ( *pIt < sal_Unicode( WW8ListManager::nMaxLevel ) )
        {
            aBuffer.append( pPrev, pIt - pPrev );
            aBuffer.append( '%' );
            aBuffer.append( sal_Int32( *pIt ) + 1 );

            pPrev = pIt + 1;
        }
        ++pIt;
    }
    if ( pPrev < pIt )
        aBuffer.append( pPrev, pIt - pPrev );

    // If bullet char is empty, set lvlText as empty
    if ( rNumberingString == OUStringChar('\0') && nNumberingType == SVX_NUM_CHAR_SPECIAL )
    {
        m_pSerializer->singleElementNS(XML_w, XML_lvlText, FSNS(XML_w, XML_val), "");
    }
    else
    {
        // Writer's "zero width space" suffix is necessary, so that LabelFollowedBy shows up, but Word doesn't require that.
        OUString aLevelText = aBuffer.makeStringAndClear();
        static OUString aZeroWidthSpace(u'\x200B');
        if (aLevelText == aZeroWidthSpace)
            aLevelText.clear();
        m_pSerializer->singleElementNS(XML_w, XML_lvlText, FSNS(XML_w, XML_val), aLevelText);
    }

    // bullet
    if (nNumberingType == SVX_NUM_BITMAP && pBrush)
    {
        int nIndex = m_rExport.GetGrfIndex(*pBrush);
        if (nIndex != -1)
        {
            m_pSerializer->singleElementNS(XML_w, XML_lvlPicBulletId,
                    FSNS(XML_w, XML_val), OString::number(nIndex));
        }
    }

    // justification
    const char *pJc;
    bool ecmaDialect = ( m_rExport.GetFilter().getVersion() == oox::core::ECMA_DIALECT );
    switch ( eAdjust )
    {
        case SvxAdjust::Center: pJc = "center"; break;
        case SvxAdjust::Right:  pJc = !ecmaDialect ? "end" : "right";  break;
        default:                pJc = !ecmaDialect ? "start" : "left";   break;
    }
    m_pSerializer->singleElementNS(XML_w, XML_lvlJc, FSNS(XML_w, XML_val), pJc);

    // indentation
    m_pSerializer->startElementNS(XML_w, XML_pPr);
    if( nListTabPos >= 0 )
    {
        m_pSerializer->startElementNS(XML_w, XML_tabs);
        m_pSerializer->singleElementNS( XML_w, XML_tab,
                FSNS( XML_w, XML_val ), "num",
                FSNS( XML_w, XML_pos ), OString::number(nListTabPos) );
        m_pSerializer->endElementNS( XML_w, XML_tabs );
    }

    sal_Int32 nToken = ecmaDialect ? XML_left : XML_start;
    sal_Int32 nIndentToken = nFirstLineIndex > 0 ? XML_firstLine : XML_hanging;
    m_pSerializer->singleElementNS( XML_w, XML_ind,
            FSNS( XML_w, nToken ), OString::number(nIndentAt),
            FSNS( XML_w, nIndentToken ), OString::number(abs(nFirstLineIndex)) );
    m_pSerializer->endElementNS( XML_w, XML_pPr );

    // font
    if ( pOutSet )
    {
        m_pSerializer->startElementNS(XML_w, XML_rPr);

        SfxItemSet aTempSet(*pOutSet);
        if ( pFont )
        {
            GetExport().GetId( *pFont ); // ensure font info is written to fontTable.xml
            OString aFamilyName( OUStringToOString( pFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ) );
            m_pSerializer->singleElementNS( XML_w, XML_rFonts,
                    FSNS( XML_w, XML_ascii ), aFamilyName,
                    FSNS( XML_w, XML_hAnsi ), aFamilyName,
                    FSNS( XML_w, XML_cs ), aFamilyName,
                    FSNS( XML_w, XML_hint ), "default" );
            aTempSet.ClearItem(RES_CHRATR_FONT);
            aTempSet.ClearItem(RES_CHRATR_CTL_FONT);
        }
        m_rExport.OutputItemSet(aTempSet, false, true, i18n::ScriptType::LATIN, m_rExport.m_bExportModeRTF);

        WriteCollectedRunProperties();

        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }

    // TODO anything to do about nListTabPos?

    m_pSerializer->endElementNS( XML_w, XML_lvl );
}

void DocxAttributeOutput::CharCaseMap( const SvxCaseMapItem& rCaseMap )
{
    switch ( rCaseMap.GetValue() )
    {
        case SvxCaseMap::SmallCaps:
            m_pSerializer->singleElementNS(XML_w, XML_smallCaps);
            break;
        case SvxCaseMap::Uppercase:
            m_pSerializer->singleElementNS(XML_w, XML_caps);
            break;
        default: // Something that ooxml does not support
            m_pSerializer->singleElementNS(XML_w, XML_smallCaps, FSNS(XML_w, XML_val), "false");
            m_pSerializer->singleElementNS(XML_w, XML_caps, FSNS(XML_w, XML_val), "false");
            break;
    }
}

void DocxAttributeOutput::CharColor( const SvxColorItem& rColor )
{
    const Color aColor( rColor.GetValue() );
    OString aColorString = msfilter::util::ConvertColor( aColor );

    const char* pExistingValue(nullptr);
    if (m_pColorAttrList.is() && m_pColorAttrList->getAsChar(FSNS(XML_w, XML_val), pExistingValue))
    {
        assert(aColorString.equalsL(pExistingValue, rtl_str_getLength(pExistingValue)));
        return;
    }

    AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_val ), aColorString.getStr() );
    m_nCharTransparence = 255 - aColor.GetAlpha();
}

void DocxAttributeOutput::CharContour( const SvxContourItem& rContour )
{
    if ( rContour.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_outline);
    else
        m_pSerializer->singleElementNS(XML_w, XML_outline, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharCrossedOut( const SvxCrossedOutItem& rCrossedOut )
{
    switch ( rCrossedOut.GetStrikeout() )
    {
        case STRIKEOUT_DOUBLE:
            m_pSerializer->singleElementNS(XML_w, XML_dstrike);
            break;
        case STRIKEOUT_NONE:
            m_pSerializer->singleElementNS(XML_w, XML_dstrike, FSNS(XML_w, XML_val), "false");
            m_pSerializer->singleElementNS(XML_w, XML_strike, FSNS(XML_w, XML_val), "false");
            break;
        default:
            m_pSerializer->singleElementNS(XML_w, XML_strike);
            break;
    }
}

void DocxAttributeOutput::CharEscapement( const SvxEscapementItem& rEscapement )
{
    OString sIss;
    short nEsc = rEscapement.GetEsc(), nProp = rEscapement.GetProportionalHeight();

    // Simplify styles to avoid impossible complexity. Import and export as defaults only
    if ( m_rExport.m_bStyDef && nEsc )
    {
        nProp = DFLT_ESC_PROP;
        nEsc = (nEsc > 0) ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
    }

    if ( !nEsc )
    {
        sIss = OString( "baseline" );
        nEsc = 0;
        nProp = 100;
    }
    else if ( DFLT_ESC_PROP == nProp || nProp < 1 || nProp > 100 )
    {
        if ( DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            sIss = OString( "subscript" );
        else if ( DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc )
            sIss = OString( "superscript" );
    }
    else if ( DFLT_ESC_AUTO_SUPER == nEsc )
    {
        // Raised by the differences between the ascenders (ascent = baseline to top of highest letter).
        // The ascent is generally about 80% of the total font height.
        // That is why DFLT_ESC_PROP (58) leads to 33% (DFLT_ESC_SUPER)
        nEsc = .8 * (100 - nProp);
    }
    else if ( DFLT_ESC_AUTO_SUB == nEsc )
    {
        // Lowered by the differences between the descenders (descent = baseline to bottom of lowest letter).
        // The descent is generally about 20% of the total font height.
        // That is why DFLT_ESC_PROP (58) leads to 8% (DFLT_ESC_SUB)
        nEsc = .2 * -(100 - nProp);
    }

    if ( !sIss.isEmpty() )
        m_pSerializer->singleElementNS(XML_w, XML_vertAlign, FSNS(XML_w, XML_val), sIss);

    if (!(sIss.isEmpty() || sIss.match("baseline")))
        return;

    const SvxFontHeightItem& rItem = m_rExport.GetItem(RES_CHRATR_FONTSIZE);
    float fHeight = rItem.GetHeight();
    OString sPos = OString::number( round(( fHeight * nEsc ) / 1000) );
    m_pSerializer->singleElementNS(XML_w, XML_position, FSNS(XML_w, XML_val), sPos);

    if( ( 100 != nProp || sIss.match( "baseline" ) ) && !m_rExport.m_bFontSizeWritten )
    {
        OString sSize = OString::number( round(( fHeight * nProp ) / 1000) );
        m_pSerializer->singleElementNS(XML_w, XML_sz, FSNS(XML_w, XML_val), sSize);
    }
}

void DocxAttributeOutput::CharFont( const SvxFontItem& rFont)
{
    GetExport().GetId( rFont ); // ensure font info is written to fontTable.xml
    const OUString& sFontName(rFont.GetFamilyName());
    const OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    if (sFontNameUtf8.isEmpty())
        return;

    if (m_pFontsAttrList &&
        (   m_pFontsAttrList->hasAttribute(FSNS( XML_w, XML_ascii )) ||
            m_pFontsAttrList->hasAttribute(FSNS( XML_w, XML_hAnsi ))    )
        )
    {
        // tdf#38778: do to fields output into DOC the font could be added before and after field declaration
        // that all sub runs of the field will have correct font inside.
        // For DOCX we should do not add the same font information twice in the same node
        return;
    }

    AddToAttrList( m_pFontsAttrList, 2,
        FSNS( XML_w, XML_ascii ), sFontNameUtf8.getStr(),
        FSNS( XML_w, XML_hAnsi ), sFontNameUtf8.getStr() );
}

void DocxAttributeOutput::CharFontSize( const SvxFontHeightItem& rFontSize)
{
    OString fontSize = OString::number( ( rFontSize.GetHeight() + 5 ) / 10 );

    switch ( rFontSize.Which() )
    {
        case RES_CHRATR_FONTSIZE:
        case RES_CHRATR_CJK_FONTSIZE:
            m_pSerializer->singleElementNS(XML_w, XML_sz, FSNS(XML_w, XML_val), fontSize);
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            m_pSerializer->singleElementNS(XML_w, XML_szCs, FSNS(XML_w, XML_val), fontSize);
            break;
    }
}

void DocxAttributeOutput::CharKerning( const SvxKerningItem& rKerning )
{
    OString aKerning = OString::number(  rKerning.GetValue() );
    m_pSerializer->singleElementNS(XML_w, XML_spacing, FSNS(XML_w, XML_val), aKerning);
}

void DocxAttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    OString aLanguageCode( OUStringToOString(
                LanguageTag( rLanguage.GetLanguage()).getBcp47MS(),
                RTL_TEXTENCODING_UTF8));

    switch ( rLanguage.Which() )
    {
        case RES_CHRATR_LANGUAGE:
            AddToAttrList( m_pCharLangAttrList, FSNS( XML_w, XML_val ), aLanguageCode.getStr() );
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            AddToAttrList( m_pCharLangAttrList, FSNS( XML_w, XML_eastAsia ), aLanguageCode.getStr() );
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            AddToAttrList( m_pCharLangAttrList, FSNS( XML_w, XML_bidi ), aLanguageCode.getStr() );
            break;
    }
}

void DocxAttributeOutput::CharPosture( const SvxPostureItem& rPosture )
{
    if ( rPosture.GetPosture() != ITALIC_NONE )
        m_pSerializer->singleElementNS(XML_w, XML_i);
    else
        m_pSerializer->singleElementNS(XML_w, XML_i, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    if ( rShadow.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_shadow);
    else
        m_pSerializer->singleElementNS(XML_w, XML_shadow, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    const char *pUnderlineValue;

    switch ( rUnderline.GetLineStyle() )
    {
        case LINESTYLE_SINGLE:         pUnderlineValue = "single";          break;
        case LINESTYLE_BOLD:           pUnderlineValue = "thick";           break;
        case LINESTYLE_DOUBLE:         pUnderlineValue = "double";          break;
        case LINESTYLE_DOTTED:         pUnderlineValue = "dotted";          break;
        case LINESTYLE_DASH:           pUnderlineValue = "dash";            break;
        case LINESTYLE_DASHDOT:        pUnderlineValue = "dotDash";         break;
        case LINESTYLE_DASHDOTDOT:     pUnderlineValue = "dotDotDash";      break;
        case LINESTYLE_WAVE:           pUnderlineValue = "wave";            break;
        case LINESTYLE_BOLDDOTTED:     pUnderlineValue = "dottedHeavy";     break;
        case LINESTYLE_BOLDDASH:       pUnderlineValue = "dashedHeavy";     break;
        case LINESTYLE_LONGDASH:       pUnderlineValue = "dashLongHeavy";   break;
        case LINESTYLE_BOLDLONGDASH:   pUnderlineValue = "dashLongHeavy";   break;
        case LINESTYLE_BOLDDASHDOT:    pUnderlineValue = "dashDotHeavy";    break;
        case LINESTYLE_BOLDDASHDOTDOT: pUnderlineValue = "dashDotDotHeavy"; break;
        case LINESTYLE_BOLDWAVE:       pUnderlineValue = "wavyHeavy";       break;
        case LINESTYLE_DOUBLEWAVE:     pUnderlineValue = "wavyDouble";      break;
        case LINESTYLE_NONE:           // fall through
        default:                       pUnderlineValue = "none";            break;
    }

    Color aUnderlineColor = rUnderline.GetColor();
    bool  bUnderlineHasColor = !aUnderlineColor.IsTransparent();
    if (bUnderlineHasColor)
    {
        // Underline has a color
        m_pSerializer->singleElementNS( XML_w, XML_u,
                                        FSNS( XML_w, XML_val ), pUnderlineValue,
                                        FSNS( XML_w, XML_color ), msfilter::util::ConvertColor(aUnderlineColor) );
    }
    else
    {
        // Underline has no color
        m_pSerializer->singleElementNS(XML_w, XML_u, FSNS(XML_w, XML_val), pUnderlineValue);
    }
}

void DocxAttributeOutput::CharWeight( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS(XML_w, XML_b);
    else
        m_pSerializer->singleElementNS(XML_w, XML_b, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharAutoKern( const SvxAutoKernItem& rAutoKern )
{
    // auto kerning is bound to a minimum font size in Word - but is just a boolean in Writer :-(
    // kerning is based on half-point sizes, so 2 enables kerning for fontsize 1pt or higher. (1 is treated as size 12, and 0 is treated as disabled.)
    const OString sFontSize = OString::number( static_cast<sal_uInt32>(rAutoKern.GetValue()) * 2 );
    m_pSerializer->singleElementNS(XML_w, XML_kern, FSNS(XML_w, XML_val), sFontSize);
}

void DocxAttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    if ( rBlink.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS(XML_w, XML_val), "blinkBackground");
    else
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS(XML_w, XML_val), "none");
}

constexpr OUStringLiteral MSWORD_CH_SHADING_FILL = u"FFFFFF"; // The attribute w:fill of w:shd, for MS-Word's character shading,
constexpr OUStringLiteral MSWORD_CH_SHADING_COLOR = u"auto"; // The attribute w:color of w:shd, for MS-Word's character shading,
constexpr OUStringLiteral MSWORD_CH_SHADING_VAL = u"pct15"; // The attribute w:value of w:shd, for MS-Word's character shading,

void DocxAttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    // Check if the brush shading pattern is 'PCT15'. If so - write it back to the DOCX
    if (rBrush.GetShadingValue() == ShadingPattern::PCT15)
    {
        m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_val ), MSWORD_CH_SHADING_VAL,
            FSNS( XML_w, XML_color ), MSWORD_CH_SHADING_COLOR,
            FSNS( XML_w, XML_fill ), MSWORD_CH_SHADING_FILL );
    }
    else
    {
        m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_fill ), msfilter::util::ConvertColor(rBrush.GetColor()),
            FSNS( XML_w, XML_val ), "clear" );
    }
}

void DocxAttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    if (m_pFontsAttrList && m_pFontsAttrList->hasAttribute(FSNS(XML_w, XML_eastAsia)))
    {
        // tdf#38778: do to fields output into DOC the font could be added before and after field declaration
        // that all sub runs of the field will have correct font inside.
        // For DOCX we should do not add the same font information twice in the same node
        return;
    }

    const OUString& sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_eastAsia ), sFontNameUtf8.getStr() );
}

void DocxAttributeOutput::CharPostureCJK( const SvxPostureItem& rPosture )
{
    if ( rPosture.GetPosture() != ITALIC_NONE )
        m_pSerializer->singleElementNS(XML_w, XML_i);
    else
        m_pSerializer->singleElementNS(XML_w, XML_i, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharWeightCJK( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS(XML_w, XML_b);
    else
        m_pSerializer->singleElementNS(XML_w, XML_b, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharFontCTL( const SvxFontItem& rFont )
{
    if (m_pFontsAttrList && m_pFontsAttrList->hasAttribute(FSNS(XML_w, XML_cs)))
    {
        // tdf#38778: do to fields output into DOC the font could be added before and after field declaration
        // that all sub runs of the field will have correct font inside.
        // For DOCX we should do not add the same font information twice in the same node
        return;
    }

    const OUString& sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_cs ), sFontNameUtf8.getStr() );
}

void DocxAttributeOutput::CharPostureCTL( const SvxPostureItem& rPosture)
{
    if ( rPosture.GetPosture() != ITALIC_NONE )
        m_pSerializer->singleElementNS(XML_w, XML_iCs);
    else
        m_pSerializer->singleElementNS(XML_w, XML_iCs, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS(XML_w, XML_bCs);
    else
        m_pSerializer->singleElementNS(XML_w, XML_bCs, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharBidiRTL( const SfxPoolItem& )
{
}

void DocxAttributeOutput::CharIdctHint( const SfxPoolItem& )
{
}

void DocxAttributeOutput::CharRotate( const SvxCharRotateItem& rRotate)
{
    // Not rotated?
    if ( !rRotate.GetValue())
        return;

    AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_vert ), "true" );

    if (rRotate.IsFitToLine())
        AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_vertCompress ), "true" );
}

void DocxAttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
{
    const char *pEmphasis;
    const FontEmphasisMark v = rEmphasisMark.GetEmphasisMark();

    if (v == (FontEmphasisMark::Dot | FontEmphasisMark::PosAbove))
        pEmphasis = "dot";
    else if (v == (FontEmphasisMark::Accent | FontEmphasisMark::PosAbove))
        pEmphasis = "comma";
    else if (v == (FontEmphasisMark::Circle | FontEmphasisMark::PosAbove))
        pEmphasis = "circle";
    else if (v == (FontEmphasisMark::Dot|FontEmphasisMark::PosBelow))
        pEmphasis = "underDot";
    else
        pEmphasis = "none";

    m_pSerializer->singleElementNS(XML_w, XML_em, FSNS(XML_w, XML_val), pEmphasis);
}

void DocxAttributeOutput::CharTwoLines( const SvxTwoLinesItem& rTwoLines )
{
    if ( !rTwoLines.GetValue() )
        return;

    AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_combine ), "true" );

    sal_Unicode cStart = rTwoLines.GetStartBracket();
    sal_Unicode cEnd = rTwoLines.GetEndBracket();

    if (!cStart && !cEnd)
        return;

    OString sBracket;
    if ((cStart == '{') || (cEnd == '}'))
        sBracket = const_cast<char *>("curly");
    else if ((cStart == '<') || (cEnd == '>'))
        sBracket = const_cast<char *>("angle");
    else if ((cStart == '[') || (cEnd == ']'))
        sBracket = const_cast<char *>("square");
    else
        sBracket = const_cast<char *>("round");
    AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_combineBrackets ), sBracket.getStr() );
}

void DocxAttributeOutput::CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth )
{
    // Clamp CharScaleWidth to OOXML limits ([1..600])
    const sal_Int16 nScaleWidth( std::max<sal_Int16>( 1,
        std::min<sal_Int16>( rScaleWidth.GetValue(), 600 ) ) );
    m_pSerializer->singleElementNS( XML_w, XML_w,
        FSNS( XML_w, XML_val ), OString::number(nScaleWidth) );
}

void DocxAttributeOutput::CharRelief( const SvxCharReliefItem& rRelief )
{
    switch ( rRelief.GetValue() )
    {
        case FontRelief::Embossed:
            m_pSerializer->singleElementNS(XML_w, XML_emboss);
            break;
        case FontRelief::Engraved:
            m_pSerializer->singleElementNS(XML_w, XML_imprint);
            break;
        default:
            m_pSerializer->singleElementNS(XML_w, XML_emboss, FSNS(XML_w, XML_val), "false");
            m_pSerializer->singleElementNS(XML_w, XML_imprint, FSNS(XML_w, XML_val), "false");
            break;
    }
}

void DocxAttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    if ( rHidden.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_vanish);
    else
        m_pSerializer->singleElementNS(XML_w, XML_vanish, FSNS(XML_w, XML_val), "false");
}

void DocxAttributeOutput::CharBorder(
    const SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow )
{
    css::table::BorderLine2 rStyleBorder;
    const SvxBoxItem* pInherited = nullptr;
    if ( GetExport().m_bStyDef && GetExport().m_pCurrentStyle && GetExport().m_pCurrentStyle->DerivedFrom() )
        pInherited = GetExport().m_pCurrentStyle->DerivedFrom()->GetAttrSet().GetItem<SvxBoxItem>(RES_CHRATR_BOX);
    else if ( m_rExport.m_pChpIter ) // incredibly undocumented, but this is the character-style info, right?
    {
        if (const SfxPoolItem* pPoolItem = GetExport().m_pChpIter->HasTextItem(RES_CHRATR_BOX))
        {
            pInherited = &pPoolItem->StaticWhichCast(RES_CHRATR_BOX);
        }
    }

    if ( pInherited )
        rStyleBorder = SvxBoxItem::SvxLineToLine(pInherited->GetRight(), false);

    impl_borderLine( m_pSerializer, XML_bdr, pAllBorder, nDist, bShadow, &rStyleBorder );
}

void DocxAttributeOutput::CharHighlight( const SvxBrushItem& rHighlight )
{
    const OString sColor = TransHighlightColor( msfilter::util::TransColToIco(rHighlight.GetColor()) );
    if ( !sColor.isEmpty() )
    {
        m_pSerializer->singleElementNS(XML_w, XML_highlight, FSNS(XML_w, XML_val), sColor);
    }
}

void DocxAttributeOutput::TextINetFormat( const SwFormatINetFormat& rLink )
{
    OString aStyleId = MSWordStyles::CreateStyleId(rLink.GetINetFormat());
    if (!aStyleId.isEmpty() && !aStyleId.equalsIgnoreAsciiCase("DefaultStyle"))
        m_pSerializer->singleElementNS(XML_w, XML_rStyle, FSNS(XML_w, XML_val), aStyleId);
}

void DocxAttributeOutput::TextCharFormat( const SwFormatCharFormat& rCharFormat )
{
    OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(rCharFormat.GetCharFormat())));

    m_pSerializer->singleElementNS(XML_w, XML_rStyle, FSNS(XML_w, XML_val), aStyleId);
}

void DocxAttributeOutput::RefField( const SwField&  rField, const OUString& rRef )
{
    SwFieldIds nType = rField.GetTyp( )->Which( );
    if ( nType == SwFieldIds::GetExp )
    {
        OUString sCmd = FieldString( ww::eREF ) +
            "\"" + rRef + "\" ";

        m_rExport.OutputField( &rField, ww::eREF, sCmd );
    }

    // There is nothing to do here for the set fields
}

void DocxAttributeOutput::HiddenField(const SwField& rField)
{
    auto eSubType = static_cast<SwFieldTypesEnum>(rField.GetSubType());
    if (eSubType == SwFieldTypesEnum::ConditionalText)
    {
        OUString aCond = rField.GetPar1();
        OUString aTrueFalse = rField.GetPar2();
        sal_Int32 nPos = aTrueFalse.indexOf('|');
        OUString aTrue;
        OUString aFalse;
        if (nPos == -1)
        {
            aTrue = aTrueFalse;
        }
        else
        {
            aTrue = aTrueFalse.subView(0, nPos);
            aFalse = aTrueFalse.subView(nPos + 1);
        }
        OUString aCmd = FieldString(ww::eIF) + aCond + " \"" + aTrue + "\" \"" + aFalse + "\"";
        m_rExport.OutputField(&rField, ww::eIF, aCmd);
        return;
    }

    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::HiddenField()" );
}

void DocxAttributeOutput::PostitField( const SwField* pField )
{
    assert( dynamic_cast< const SwPostItField* >( pField ));
    const SwPostItField* pPostItField = static_cast<const SwPostItField*>(pField);
    OString aName = OUStringToOString(pPostItField->GetName(), RTL_TEXTENCODING_UTF8);
    sal_Int32 nId = 0;
    std::map< OString, sal_Int32 >::iterator it = m_rOpenedAnnotationMarksIds.find(aName);
    if (it != m_rOpenedAnnotationMarksIds.end())
        // If the postit field has an annotation mark associated, we already have an id.
        nId = it->second;
    else
        // Otherwise get a new one.
        nId = m_nNextAnnotationMarkId++;
    m_postitFields.emplace_back(pPostItField, PostItDOCXData{ nId });
}

void DocxAttributeOutput::WritePostitFieldReference()
{
    while( m_postitFieldsMaxId < m_postitFields.size())
    {
        OString idstr = OString::number(m_postitFields[m_postitFieldsMaxId].second.id);

        // In case this file is inside annotation marks, we want to write the
        // comment reference after the annotation mark is closed, not here.
        OString idname = OUStringToOString(m_postitFields[m_postitFieldsMaxId].first->GetName(), RTL_TEXTENCODING_UTF8);
        std::map< OString, sal_Int32 >::iterator it = m_rOpenedAnnotationMarksIds.find( idname );
        if ( it == m_rOpenedAnnotationMarksIds.end(  ) )
            m_pSerializer->singleElementNS(XML_w, XML_commentReference, FSNS(XML_w, XML_id), idstr);
        ++m_postitFieldsMaxId;
    }
}

DocxAttributeOutput::hasResolved DocxAttributeOutput::WritePostitFields()
{
    bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
        SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

    hasResolved eResult = hasResolved::no;
    for (auto& [f, data] : m_postitFields)
    {
        OString idstr = OString::number(data.id);
        m_pSerializer->startElementNS( XML_w, XML_comment, FSNS( XML_w, XML_id ), idstr,
            FSNS( XML_w, XML_author ), bRemovePersonalInfo
                     ? "Author" + OUString::number( GetExport().GetInfoID(f->GetPar1()) )
                     : f->GetPar1(),
            FSNS( XML_w, XML_date ), DateTimeToOString( bRemovePersonalInfo
                     ? util::DateTime() // "no date" time
                     : f->GetDateTime() ),
            FSNS( XML_w, XML_initials ), bRemovePersonalInfo
                     ? OUString::number( GetExport().GetInfoID(f->GetInitials()) )
                     : f->GetInitials() );

        const bool bNeedParaId = f->GetResolved();
        if (bNeedParaId)
            eResult = hasResolved::yes;

        if (f->GetTextObject() != nullptr)
        {
            // richtext
            data.lastParaId = GetExport().WriteOutliner(*f->GetTextObject(), TXT_ATN, bNeedParaId);
        }
        else
        {
            // just plain text - eg. when the field was created via the
            // .uno:InsertAnnotation API
            std::optional<OUString> aParaId;
            if (bNeedParaId)
            {
                data.lastParaId = m_nNextParaId++;
                aParaId = NumberToHexBinary(data.lastParaId);
            }
            m_pSerializer->startElementNS(XML_w, XML_p, FSNS(XML_w14, XML_paraId), aParaId);
            m_pSerializer->startElementNS(XML_w, XML_r);
            RunText(f->GetText());
            m_pSerializer->endElementNS(XML_w, XML_r);
            m_pSerializer->endElementNS(XML_w, XML_p);
        }

        m_pSerializer->endElementNS( XML_w, XML_comment );
    }
    return eResult;
}

void DocxAttributeOutput::WritePostItFieldsResolved()
{
    for (auto& [f, data] : m_postitFields)
    {
        if (!f->GetResolved())
            continue;
        OUString idstr = NumberToHexBinary(data.lastParaId);
        m_pSerializer->singleElementNS(XML_w15, XML_commentEx, FSNS(XML_w15, XML_paraId), idstr,
                                       FSNS(XML_w15, XML_done), "1");
    }
}

bool DocxAttributeOutput::DropdownField( const SwField* pField )
{
    ww::eField eType = ww::eFORMDROPDOWN;
    OUString sCmd = FieldString( eType  );
    GetExport( ).OutputField( pField, eType, sCmd );

    return false;
}

bool DocxAttributeOutput::PlaceholderField( const SwField* pField )
{
    assert( pendingPlaceholder == nullptr );
    pendingPlaceholder = pField;
    return false; // do not expand
}

void DocxAttributeOutput::WritePendingPlaceholder()
{
    if( pendingPlaceholder == nullptr )
        return;
    const SwField* pField = pendingPlaceholder;
    pendingPlaceholder = nullptr;
    m_pSerializer->startElementNS(XML_w, XML_sdt);
    m_pSerializer->startElementNS(XML_w, XML_sdtPr);
    if( !pField->GetPar2().isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_alias, FSNS(XML_w, XML_val), pField->GetPar2());
    m_pSerializer->singleElementNS(XML_w, XML_temporary);
    m_pSerializer->singleElementNS(XML_w, XML_showingPlcHdr);
    m_pSerializer->singleElementNS(XML_w, XML_text);
    m_pSerializer->endElementNS( XML_w, XML_sdtPr );
    m_pSerializer->startElementNS(XML_w, XML_sdtContent);
    m_pSerializer->startElementNS(XML_w, XML_r);
    RunText( pField->GetPar1());
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_sdtContent );
    m_pSerializer->endElementNS( XML_w, XML_sdt );
}

void DocxAttributeOutput::SetField( const SwField& rField, ww::eField eType, const OUString& rCmd )
{
    // field bookmarks are handled in the EndRun method
    GetExport().OutputField(&rField, eType, rCmd );
}

void DocxAttributeOutput::WriteExpand( const SwField* pField )
{
    // Will be written in the next End Run
    m_rExport.OutputField( pField, ww::eUNKNOWN, OUString() );
}

void DocxAttributeOutput::WriteField_Impl(const SwField *const pField,
    ww::eField const eType, const OUString& rFieldCmd, FieldFlags const nMode,
    OUString const*const pBookmarkName)
{
    if (m_bPreventDoubleFieldsHandling)
        return;

    struct FieldInfos infos;
    if (pField)
        infos.pField = pField->CopyField();
    infos.sCmd = rFieldCmd;
    infos.eType = eType;
    infos.bClose = bool(FieldFlags::Close & nMode);
    infos.bSep = bool(FieldFlags::CmdEnd & nMode);
    infos.bOpen = bool(FieldFlags::Start & nMode);
    m_Fields.push_back( infos );

    if (pBookmarkName)
    {
        m_sFieldBkm = *pBookmarkName;
    }

    if ( !pField )
        return;

    SwFieldIds nType = pField->GetTyp( )->Which( );
    sal_uInt16 nSubType = pField->GetSubType();

    // TODO Any other field types here ?
    if ( ( nType == SwFieldIds::SetExp ) && ( nSubType & nsSwGetSetExpType::GSE_STRING ) )
    {
        const SwSetExpField *pSet = static_cast<const SwSetExpField*>( pField );
        m_sFieldBkm = pSet->GetPar1( );
    }
    else if ( nType == SwFieldIds::Dropdown )
    {
        const SwDropDownField* pDropDown = static_cast<const SwDropDownField*>( pField );
        m_sFieldBkm = pDropDown->GetName( );
    }
}

void DocxAttributeOutput::WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark )
{
    if ( !m_Fields.empty() )
        m_Fields.begin()->pFieldmark = &rFieldmark;
}

void DocxAttributeOutput::WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds, const SwRedlineData* pRedlineData )
{
    for ( const OUString & name : rStarts )
    {
        if (name.startsWith("permission-for-group:") ||
            name.startsWith("permission-for-user:"))
        {
            m_rPermissionsStart.push_back(name);
        }
        else
        {
            m_rBookmarksStart.push_back(name);
            m_pMoveRedlineData = const_cast<SwRedlineData*>(pRedlineData);
        }
    }
    rStarts.clear();

    for ( const OUString & name : rEnds )
    {
        if (name.startsWith("permission-for-group:") ||
            name.startsWith("permission-for-user:"))
        {
            m_rPermissionsEnd.push_back(name);
        }
        else
        {
            m_rBookmarksEnd.push_back(name);
        }
    }
    rEnds.clear();
}

void DocxAttributeOutput::WriteFinalBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds )
{
    for ( const OUString & name : rStarts )
    {
        if (name.startsWith("permission-for-group:") ||
            name.startsWith("permission-for-user:"))
        {
            m_rPermissionsStart.push_back(name);
        }
        else
        {
            m_rFinalBookmarksStart.push_back(name);
        }
    }
    rStarts.clear();

    for ( const OUString & name : rEnds )
    {
        if (name.startsWith("permission-for-group:") ||
            name.startsWith("permission-for-user:"))
        {
            m_rPermissionsEnd.push_back(name);
        }
        else
        {
            m_rFinalBookmarksEnd.push_back(name);
        }
    }
    rEnds.clear();
}

void DocxAttributeOutput::WriteAnnotationMarks_Impl( std::vector< OUString >& rStarts,
        std::vector< OUString >& rEnds )
{
    for ( const auto & rAnnotationName : rStarts )
    {
        OString rName = OUStringToOString(rAnnotationName, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rAnnotationMarksStart.push_back( rName );
    }
    rStarts.clear();

    for ( const auto & rAnnotationName : rEnds )
    {
        OString rName = OUStringToOString( rAnnotationName, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rAnnotationMarksEnd.push_back( rName );
    }
    rEnds.clear();
}

void DocxAttributeOutput::TextFootnote_Impl( const SwFormatFootnote& rFootnote )
{
    const SwEndNoteInfo& rInfo = rFootnote.IsEndNote()?
        m_rExport.m_rDoc.GetEndNoteInfo(): m_rExport.m_rDoc.GetFootnoteInfo();

    // footnote/endnote run properties
    const SwCharFormat* pCharFormat = rInfo.GetAnchorCharFormat( m_rExport.m_rDoc );

    OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(pCharFormat)));

    m_pSerializer->singleElementNS(XML_w, XML_rStyle, FSNS(XML_w, XML_val), aStyleId);

    // remember the footnote/endnote to
    // 1) write the footnoteReference/endnoteReference in EndRunProperties()
    // 2) be able to dump them all to footnotes.xml/endnotes.xml
    if ( !rFootnote.IsEndNote() && m_rExport.m_rDoc.GetFootnoteInfo().m_ePos != FTNPOS_CHAPTER )
        m_pFootnotesList->add( rFootnote );
    else
        m_pEndnotesList->add( rFootnote );
}

void DocxAttributeOutput::FootnoteEndnoteReference()
{
    sal_Int32 nId;
    const SwFormatFootnote *pFootnote = m_pFootnotesList->getCurrent( nId );
    sal_Int32 nToken = XML_footnoteReference;

    // both cannot be set at the same time - if they are, it's a bug
    if ( !pFootnote )
    {
        pFootnote = m_pEndnotesList->getCurrent( nId );
        nToken = XML_endnoteReference;
    }

    if ( !pFootnote )
        return;

    // write it
    if ( pFootnote->GetNumStr().isEmpty() )
    {
        // autonumbered
        m_pSerializer->singleElementNS(XML_w, nToken, FSNS(XML_w, XML_id), OString::number(nId));
    }
    else
    {
        // not autonumbered
        m_pSerializer->singleElementNS( XML_w, nToken,
                FSNS( XML_w, XML_customMarkFollows ), "1",
                FSNS( XML_w, XML_id ), OString::number(nId) );

        RunText( pFootnote->GetNumStr() );
    }
}

static void WriteFootnoteSeparatorHeight(
    ::sax_fastparser::FSHelperPtr const& pSerializer, SwTwips const nHeight)
{
    // try to get the height by setting font size of the paragraph
    if (nHeight != 0)
    {
        pSerializer->startElementNS(XML_w, XML_pPr);
        pSerializer->startElementNS(XML_w, XML_rPr);
        pSerializer->singleElementNS(XML_w, XML_sz, FSNS(XML_w, XML_val),
            OString::number((nHeight + 5) / 10));
        pSerializer->endElementNS(XML_w, XML_rPr);
        pSerializer->endElementNS(XML_w, XML_pPr);
    }
}

void DocxAttributeOutput::FootnotesEndnotes( bool bFootnotes )
{
    const FootnotesVector& rVector = bFootnotes? m_pFootnotesList->getVector(): m_pEndnotesList->getVector();

    sal_Int32 nBody = bFootnotes? XML_footnotes: XML_endnotes;
    sal_Int32 nItem = bFootnotes? XML_footnote:  XML_endnote;

    m_pSerializer->startElementNS( XML_w, nBody, m_rExport.MainXmlNamespaces() );

    sal_Int32 nIndex = 0;

    // separator
    // note: can only be defined for the whole document, not per section
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::number(nIndex++),
            FSNS( XML_w, XML_type ), "separator" );
    m_pSerializer->startElementNS(XML_w, XML_p);

    bool bSeparator = true;
    SwTwips nHeight(0);
    if (bFootnotes)
    {
        const SwPageFootnoteInfo& rFootnoteInfo = m_rExport.m_rDoc.GetPageDesc(0).GetFootnoteInfo();
        // Request separator only if both width and thickness are non-zero.
        bSeparator = rFootnoteInfo.GetLineStyle() != SvxBorderLineStyle::NONE
                  && rFootnoteInfo.GetLineWidth() > 0
                  && double(rFootnoteInfo.GetWidth()) > 0;
        nHeight = sw::FootnoteSeparatorHeight(rFootnoteInfo);
    }

    WriteFootnoteSeparatorHeight(m_pSerializer, nHeight);

    m_pSerializer->startElementNS(XML_w, XML_r);
    if (bSeparator)
        m_pSerializer->singleElementNS(XML_w, XML_separator);
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // separator
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::number(nIndex++),
            FSNS( XML_w, XML_type ), "continuationSeparator" );
    m_pSerializer->startElementNS(XML_w, XML_p);

    WriteFootnoteSeparatorHeight(m_pSerializer, nHeight);

    m_pSerializer->startElementNS(XML_w, XML_r);
    if (bSeparator)
    {
        m_pSerializer->singleElementNS(XML_w, XML_continuationSeparator);
    }
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // if new special ones are added, update also WriteFootnoteEndnotePr()

    // footnotes/endnotes themselves
    for ( const auto& rpItem : rVector )
    {
        m_footnoteEndnoteRefTag = bFootnotes ? XML_footnoteRef : XML_endnoteRef;
        m_footnoteCustomLabel = rpItem->GetNumStr();

        m_pSerializer->startElementNS(XML_w, nItem, FSNS(XML_w, XML_id), OString::number(nIndex));

        const SwNodeIndex* pIndex = rpItem->GetTextFootnote()->GetStartNode();
        m_rExport.WriteSpecialText( pIndex->GetIndex() + 1,
                pIndex->GetNode().EndOfSectionIndex(),
                bFootnotes? TXT_FTN: TXT_EDN );

        m_pSerializer->endElementNS( XML_w, nItem );
        ++nIndex;
    }

    m_pSerializer->endElementNS( XML_w, nBody );

}

void DocxAttributeOutput::WriteFootnoteEndnotePr( ::sax_fastparser::FSHelperPtr const & fs, int tag,
    const SwEndNoteInfo& info, int listtag )
{
    fs->startElementNS(XML_w, tag);
    OString aCustomFormat;
    OString fmt = lcl_ConvertNumberingType(info.m_aFormat.GetNumberingType(), nullptr, aCustomFormat);
    if (!fmt.isEmpty() && aCustomFormat.isEmpty())
        fs->singleElementNS(XML_w, XML_numFmt, FSNS(XML_w, XML_val), fmt);
    if( info.m_nFootnoteOffset != 0 )
        fs->singleElementNS( XML_w, XML_numStart, FSNS( XML_w, XML_val ),
            OString::number(info.m_nFootnoteOffset + 1) );

    const SwFootnoteInfo* pFootnoteInfo = dynamic_cast<const SwFootnoteInfo*>(&info);
    if( pFootnoteInfo )
    {
        switch( pFootnoteInfo->m_eNum )
        {
            case FTNNUM_PAGE:       fmt = "eachPage"; break;
            case FTNNUM_CHAPTER:    fmt = "eachSect"; break;
            default:                fmt.clear();      break;
        }
        if (!fmt.isEmpty())
            fs->singleElementNS(XML_w, XML_numRestart, FSNS(XML_w, XML_val), fmt);
    }

    if( listtag != 0 ) // we are writing to settings.xml, write also special footnote/endnote list
    { // there are currently only two hardcoded ones ( see FootnotesEndnotes())
        fs->singleElementNS(XML_w, listtag, FSNS(XML_w, XML_id), "0");
        fs->singleElementNS(XML_w, listtag, FSNS(XML_w, XML_id), "1");
    }
    fs->endElementNS( XML_w, tag );
}

void DocxAttributeOutput::SectFootnoteEndnotePr()
{
    if( HasFootnotes())
        WriteFootnoteEndnotePr( m_pSerializer, XML_footnotePr, m_rExport.m_rDoc.GetFootnoteInfo(), 0 );
    if( HasEndnotes())
        WriteFootnoteEndnotePr( m_pSerializer, XML_endnotePr, m_rExport.m_rDoc.GetEndNoteInfo(), 0 );
}

void DocxAttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    if ( nSpace < 0 )
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "exact",
                FSNS( XML_w, XML_line ), OString::number( -nSpace ).getStr() );
    }
    else if( nSpace > 0 && nMulti )
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "auto",
                FSNS( XML_w, XML_line ), OString::number( nSpace ).getStr() );
    }
    else
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "atLeast",
                FSNS( XML_w, XML_line ), OString::number( nSpace ).getStr() );
    }
}

void DocxAttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    const char *pAdjustString;

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    const SfxItemSet* pItems = GetExport().GetCurItemSet();
    const SvxFrameDirectionItem* rFrameDir = pItems?
        pItems->GetItem( RES_FRAMEDIR ) : nullptr;

    SvxFrameDirection nDir = SvxFrameDirection::Environment;
    if( rFrameDir != nullptr )
        nDir = rFrameDir->GetValue();
    if ( nDir == SvxFrameDirection::Environment )
        nDir = GetExport( ).GetDefaultFrameDirection( );
    bool bRtl = ( nDir == SvxFrameDirection::Horizontal_RL_TB );

    switch ( rAdjust.GetAdjust() )
    {
        case SvxAdjust::Left:
            if ( bEcma )
            {
                if ( bRtl )
                    pAdjustString = "right";
                else
                    pAdjustString = "left";
            }
            else if ( bRtl )
                pAdjustString = "end";
            else
                pAdjustString = "start";
            break;
        case SvxAdjust::Right:
            if ( bEcma )
            {
                if ( bRtl )
                    pAdjustString = "left";
                else
                    pAdjustString = "right";
            }
            else if ( bRtl )
                pAdjustString = "start";
            else
                pAdjustString = "end";
            break;
        case SvxAdjust::BlockLine:
        case SvxAdjust::Block:
            if (rAdjust.GetLastBlock() == SvxAdjust::Block)
                pAdjustString = "distribute";
            else
                pAdjustString = "both";
            break;
        case SvxAdjust::Center:
            pAdjustString = "center";
            break;
        default:
            return; // not supported attribute
    }
    m_pSerializer->singleElementNS(XML_w, XML_jc, FSNS(XML_w, XML_val), pAdjustString);
}

void DocxAttributeOutput::ParaSplit( const SvxFormatSplitItem& rSplit )
{
    if (rSplit.GetValue())
        m_pSerializer->singleElementNS(XML_w, XML_keepLines, FSNS(XML_w, XML_val), "false");
    else
        m_pSerializer->singleElementNS(XML_w, XML_keepLines);
}

void DocxAttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
    if (rWidows.GetValue())
        m_pSerializer->singleElementNS(XML_w, XML_widowControl);
    else
        m_pSerializer->singleElementNS(XML_w, XML_widowControl, FSNS(XML_w, XML_val), "false");
}

static void impl_WriteTabElement( FSHelperPtr const & pSerializer,
                                  const SvxTabStop& rTab, tools::Long tabsOffset )
{
    rtl::Reference<FastAttributeList> pTabElementAttrList = FastSerializerHelper::createAttrList();

    switch (rTab.GetAdjustment())
    {
    case SvxTabAdjust::Right:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "right" ) );
        break;
    case SvxTabAdjust::Decimal:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "decimal" ) );
        break;
    case SvxTabAdjust::Center:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "center" ) );
        break;
    case SvxTabAdjust::Default:
    case SvxTabAdjust::Left:
    default:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "left" ) );
        break;
    }

    // Write position according to used offset of the whole paragraph.
    // In DOCX, w:pos specifies the position of the current custom tab stop with respect to the current page margins.
    // But in ODT, zero position could be page margins or paragraph indent according to used settings.
    // This is handled outside of this method and provided for us in tabsOffset parameter.
    pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::number( rTab.GetTabPos() + tabsOffset ) );

    sal_Unicode cFillChar = rTab.GetFill();

    if ('.' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "dot" ) );
    else if ( '-' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "hyphen" ) );
    else if ( u'\x00B7' == cFillChar ) // middle dot
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "middleDot" ) );
    else if ( '_' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "underscore" ) );
    else
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "none" ) );

    pSerializer->singleElementNS(XML_w, XML_tab, pTabElementAttrList);
}

void DocxAttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStop )
{
    const SvxTabStopItem* pInheritedTabs = nullptr;
    if ( GetExport().m_pStyAttr )
        pInheritedTabs = GetExport().m_pStyAttr->GetItem<SvxTabStopItem>(RES_PARATR_TABSTOP);
    else if ( GetExport().m_pCurrentStyle && GetExport().m_pCurrentStyle->DerivedFrom() )
        pInheritedTabs = GetExport().m_pCurrentStyle->DerivedFrom()->GetAttrSet().GetItem<SvxTabStopItem>(RES_PARATR_TABSTOP);
    const sal_uInt16 nInheritedTabCount = pInheritedTabs ? pInheritedTabs->Count() : 0;
    const sal_uInt16 nCount = rTabStop.Count();

    // <w:tabs> must contain at least one <w:tab>, so don't write it empty
    if ( !nCount && !nInheritedTabCount )
        return;
    if( nCount == 1 && rTabStop[ 0 ].GetAdjustment() == SvxTabAdjust::Default )
    {
        GetExport().setDefaultTabStop( rTabStop[ 0 ].GetTabPos());
        return;
    }

    // do not output inherited tabs twice (inside styles and inside inline properties)
    if ( nCount == nInheritedTabCount && nCount > 0 )
    {
        if ( *pInheritedTabs == rTabStop )
            return;
    }

    m_pSerializer->startElementNS(XML_w, XML_tabs);

    // Get offset for tabs
    // In DOCX, w:pos specifies the position of the current custom tab stop with respect to the current page margins.
    // But in ODT, zero position could be page margins or paragraph indent according to used settings.
    tools::Long tabsOffset = 0;
    if (m_rExport.m_rDoc.getIDocumentSettingAccess().get(DocumentSettingId::TABS_RELATIVE_TO_INDENT))
        tabsOffset = m_rExport.GetItem(RES_LR_SPACE).GetTextLeft();

    // clear unused inherited tabs - otherwise the style will add them back in
    sal_Int32 nCurrTab = 0;
    for ( sal_uInt16 i = 0; i < nInheritedTabCount; ++i )
    {
        while ( nCurrTab < nCount && rTabStop[nCurrTab] < pInheritedTabs->At(i) )
            ++nCurrTab;

        if ( nCurrTab == nCount || pInheritedTabs->At(i) < rTabStop[nCurrTab] )
        {
            m_pSerializer->singleElementNS( XML_w, XML_tab,
                FSNS( XML_w, XML_val ), "clear",
                FSNS( XML_w, XML_pos ), OString::number(pInheritedTabs->At(i).GetTabPos()) );
        }
    }

    for (sal_uInt16 i = 0; i < nCount; i++ )
    {
        if( rTabStop[i].GetAdjustment() != SvxTabAdjust::Default )
            impl_WriteTabElement( m_pSerializer, rTabStop[i], tabsOffset );
        else
            GetExport().setDefaultTabStop( rTabStop[i].GetTabPos());
    }

    m_pSerializer->endElementNS( XML_w, XML_tabs );
}

void DocxAttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    m_pSerializer->singleElementNS( XML_w, XML_suppressAutoHyphens,
            FSNS( XML_w, XML_val ), OString::boolean( !rHyphenZone.IsHyphen() ) );
}

void DocxAttributeOutput::ParaNumRule_Impl( const SwTextNode* pTextNd, sal_Int32 nLvl, sal_Int32 nNumId )
{
    if ( USHRT_MAX == nNumId )
        return;

    // LibreOffice is not very flexible with "Outline Numbering" (aka "Outline" numbering style).
    // Only ONE numbering rule ("Outline") can be associated with a style-assigned-listLevel,
    // and no other style is able to inherit these numId/nLvl settings - only text nodes can.
    // So listLevel only exists in paragraph properties EXCEPT for up to ten styles that have been
    // assigned to one of these special Chapter Numbering listlevels (by default Heading 1-10).
    const sal_Int32 nTableSize = m_rExport.m_pUsedNumTable ? m_rExport.m_pUsedNumTable->size() : 0;
    const SwNumRule* pRule = nNumId > 0 && nNumId <= nTableSize ? (*m_rExport.m_pUsedNumTable)[nNumId-1] : nullptr;
    const SwTextFormatColl* pColl = pTextNd ? pTextNd->GetTextColl() : nullptr;
    // Do not duplicate numbering that is inherited from the (Chapter numbering) style
    // (since on import we duplicate style numbering/listlevel to the paragraph).
    if (pColl && pColl->IsAssignedToListLevelOfOutlineStyle()
        && nLvl == pColl->GetAssignedOutlineStyleLevel() && pRule && pRule->IsOutlineRule())
    {
        // By definition of how LO is implemented, assignToListLevel is only possible
        // when the style is also using OutlineRule for numbering. Adjust logic if that changes.
        assert(pRule->GetName() == pColl->GetNumRule(true).GetValue());
        return;
    }

    m_pSerializer->startElementNS(XML_w, XML_numPr);
    m_pSerializer->singleElementNS(XML_w, XML_ilvl, FSNS(XML_w, XML_val), OString::number(nLvl));
    m_pSerializer->singleElementNS(XML_w, XML_numId, FSNS(XML_w, XML_val), OString::number(nNumId));
    m_pSerializer->endElementNS(XML_w, XML_numPr);
}

void DocxAttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    m_pSerializer->singleElementNS( XML_w, XML_autoSpaceDE,
           FSNS( XML_w, XML_val ), OString::boolean( rScriptSpace.GetValue() ) );
}

void DocxAttributeOutput::ParaHangingPunctuation( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_overflowPunct,
           FSNS( XML_w, XML_val ), OString::boolean( rItem.GetValue() ) );
}

void DocxAttributeOutput::ParaForbiddenRules( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_kinsoku,
           FSNS( XML_w, XML_val ), OString::boolean( rItem.GetValue() ) );
}

void DocxAttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    const char *pAlignString;

    switch ( rAlign.GetValue() )
    {
        case SvxParaVertAlignItem::Align::Baseline:
            pAlignString = "baseline";
            break;
        case SvxParaVertAlignItem::Align::Top:
            pAlignString = "top";
            break;
        case SvxParaVertAlignItem::Align::Center:
            pAlignString = "center";
            break;
        case SvxParaVertAlignItem::Align::Bottom:
            pAlignString = "bottom";
            break;
        case SvxParaVertAlignItem::Align::Automatic:
            pAlignString = "auto";
            break;
        default:
            return; // not supported attribute
    }
    m_pSerializer->singleElementNS(XML_w, XML_textAlignment, FSNS(XML_w, XML_val), pAlignString);
}

void DocxAttributeOutput::ParaSnapToGrid( const SvxParaGridItem& rGrid )
{
    m_pSerializer->singleElementNS( XML_w, XML_snapToGrid,
            FSNS( XML_w, XML_val ), OString::boolean( rGrid.GetValue() ) );
}

void DocxAttributeOutput::FormatFrameSize( const SwFormatFrameSize& rSize )
{
    if (m_rExport.SdrExporter().getTextFrameSyntax() && m_rExport.SdrExporter().getFlyFrameSize())
    {
        const Size* pSize = m_rExport.SdrExporter().getFlyFrameSize();
        m_rExport.SdrExporter().getTextFrameStyle().append(";width:" + OString::number(double(pSize->Width()) / 20));
        m_rExport.SdrExporter().getTextFrameStyle().append("pt;height:" + OString::number(double(pSize->Height()) / 20) + "pt");
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == SwFrameSize::Fixed )
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(),
                    FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ).getStr() );

        if ( rSize.GetHeight() )
        {
            OString sRule( "exact" );
            if ( rSize.GetHeightSizeType() == SwFrameSize::Minimum )
                sRule = OString( "atLeast" );
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), 2,
                    FSNS( XML_w, XML_hRule ), sRule.getStr(),
                    FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ).getStr() );
        }
    }
    else if ( m_rExport.m_bOutPageDescs )
    {
        rtl::Reference<FastAttributeList> attrList = FastSerializerHelper::createAttrList( );
        if ( m_rExport.m_pCurrentPageDesc->GetLandscape( ) )
            attrList->add( FSNS( XML_w, XML_orient ), "landscape" );

        attrList->add( FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ) );
        attrList->add( FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ) );

        m_pSerializer->singleElementNS( XML_w, XML_pgSz, attrList );
    }
}

void DocxAttributeOutput::FormatPaperBin( const SvxPaperBinItem& )
{
    SAL_INFO("sw.ww8", "TODO DocxAttributeOutput::FormatPaperBin()" );
}

void DocxAttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLRSpace )
{
    bool bEcma = m_rExport.GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-left:" + OString::number(double(rLRSpace.GetLeft()) / 20) + "pt");
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-right:" + OString::number(double(rLRSpace.GetRight()) / 20) + "pt");
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_hSpace ),
                OString::number(
                    ( rLRSpace.GetLeft() + rLRSpace.GetRight() ) / 2 ).getStr() );
    }
    else if ( m_rExport.m_bOutPageDescs )
    {
        m_pageMargins.nLeft = 0;
        m_pageMargins.nRight = 0;

        const SfxPoolItem* pPoolItem = m_rExport.HasItem(RES_BOX);
        const SvxBoxItem* pBoxItem = pPoolItem ? &pPoolItem->StaticWhichCast(RES_BOX) : nullptr;
        if (pBoxItem)
        {
            m_pageMargins.nLeft = pBoxItem->CalcLineSpace( SvxBoxItemLine::LEFT, /*bEvenIfNoLine*/true );
            m_pageMargins.nRight = pBoxItem->CalcLineSpace( SvxBoxItemLine::RIGHT, /*bEvenIfNoLine*/true );
        }

        m_pageMargins.nLeft += sal::static_int_cast<sal_uInt16>(rLRSpace.GetLeft());
        m_pageMargins.nRight += sal::static_int_cast<sal_uInt16>(rLRSpace.GetRight());
        sal_uInt16 nGutter = rLRSpace.GetGutterMargin();

        AddToAttrList( m_pSectionSpacingAttrList, 3,
                FSNS( XML_w, XML_left ), OString::number( m_pageMargins.nLeft ).getStr(),
                FSNS( XML_w, XML_right ), OString::number( m_pageMargins.nRight ).getStr(),
                FSNS( XML_w, XML_gutter ), OString::number( nGutter ).getStr() );
    }
    else
    {
        SvxLRSpaceItem const* pLRSpace(&rLRSpace);
        ::std::optional<SvxLRSpaceItem> oLRSpace;
        if (dynamic_cast<SwContentNode const*>(GetExport().m_pOutFormatNode) != nullptr)
        {
            auto pTextNd(static_cast<SwTextNode const*>(GetExport().m_pOutFormatNode));
            // WW doesn't have a concept of a pararaph that's in a list but not
            // counted in the list - see AttributeOutputBase::ParaNumRule()
            // forcing non-existent numId="0" in this case.
            // This means WW won't apply the indents from the numbering,
            // so try to add them as paragraph properties here.
            if (!pTextNd->IsCountedInList())
            {
                SfxItemSetFixed<RES_LR_SPACE, RES_LR_SPACE> temp(m_rExport.m_rDoc.GetAttrPool());
                pTextNd->GetParaAttr(temp, 0, 0, false, true, true, nullptr);
                if (auto *const pItem = temp.GetItem(RES_LR_SPACE))
                {
                    // but don't use first-line offset from list (should it be 0 or from node?)
                    oLRSpace.emplace(*pItem);
                    oLRSpace->SetTextFirstLineOffset(pLRSpace->GetTextFirstLineOffset());
                    pLRSpace = &*oLRSpace;
                }
            }
        }
        rtl::Reference<FastAttributeList> pLRSpaceAttrList = FastSerializerHelper::createAttrList();
        if ((0 != pLRSpace->GetTextLeft()) || (pLRSpace->IsExplicitZeroMarginValLeft()))
        {
            pLRSpaceAttrList->add( FSNS(XML_w, (bEcma ? XML_left : XML_start)), OString::number(pLRSpace->GetTextLeft()) );
        }
        if ((0 != pLRSpace->GetRight()) || (pLRSpace->IsExplicitZeroMarginValRight()))
        {
            pLRSpaceAttrList->add( FSNS(XML_w, (bEcma ? XML_right : XML_end)), OString::number(pLRSpace->GetRight()) );
        }
        sal_Int32 const nFirstLineAdjustment = pLRSpace->GetTextFirstLineOffset();
        if (nFirstLineAdjustment > 0)
            pLRSpaceAttrList->add( FSNS( XML_w, XML_firstLine ), OString::number( nFirstLineAdjustment ) );
        else
            pLRSpaceAttrList->add( FSNS( XML_w, XML_hanging ), OString::number( - nFirstLineAdjustment ) );
        m_pSerializer->singleElementNS( XML_w, XML_ind, pLRSpaceAttrList );
    }
}

void DocxAttributeOutput::FormatULSpace( const SvxULSpaceItem& rULSpace )
{

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-top:" + OString::number(double(rULSpace.GetUpper()) / 20) + "pt");
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-bottom:" + OString::number(double(rULSpace.GetLower()) / 20) + "pt");
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_vSpace ),
                OString::number(
                    ( rULSpace.GetLower() + rULSpace.GetUpper() ) / 2 ).getStr() );
    }
    else if (m_rExport.m_bOutPageDescs )
    {
        OSL_ENSURE( m_rExport.GetCurItemSet(), "Impossible" );
        if ( !m_rExport.GetCurItemSet() )
            return;

        HdFtDistanceGlue aDistances( *m_rExport.GetCurItemSet() );

        sal_Int32 nHeader = 0;
        if ( aDistances.HasHeader() )
            nHeader = sal_Int32( aDistances.dyaHdrTop );
        else if (m_rExport.m_pFirstPageFormat)
        {
            HdFtDistanceGlue aFirstPageDistances(m_rExport.m_pFirstPageFormat->GetAttrSet());
            if (aFirstPageDistances.HasHeader())
            {
                // The follow page style has no header, but the first page style has. In Word terms,
                // this means that the header margin of "the" section is coming from the first page
                // style.
                nHeader = sal_Int32(aFirstPageDistances.dyaHdrTop);
            }
        }

        // Page top
        m_pageMargins.nTop = aDistances.dyaTop;

        sal_Int32 nFooter = 0;
        if ( aDistances.HasFooter() )
            nFooter = sal_Int32( aDistances.dyaHdrBottom );
        else if (m_rExport.m_pFirstPageFormat)
        {
            HdFtDistanceGlue aFirstPageDistances(m_rExport.m_pFirstPageFormat->GetAttrSet());
            if (aFirstPageDistances.HasFooter())
            {
                // The follow page style has no footer, but the first page style has. In Word terms,
                // this means that the footer margin of "the" section is coming from the first page
                // style.
                nFooter = sal_Int32(aFirstPageDistances.dyaHdrBottom);
            }
        }

        // Page Bottom
        m_pageMargins.nBottom = aDistances.dyaBottom;

        AddToAttrList( m_pSectionSpacingAttrList, 4,
                FSNS( XML_w, XML_header ), OString::number( nHeader ).getStr(),
                FSNS( XML_w, XML_top ), OString::number( m_pageMargins.nTop ).getStr(),
                FSNS( XML_w, XML_footer ), OString::number( nFooter ).getStr(),
                FSNS( XML_w, XML_bottom ), OString::number( m_pageMargins.nBottom ).getStr() );
    }
    else
    {
        SAL_INFO("sw.ww8", "DocxAttributeOutput::FormatULSpace: setting spacing" << rULSpace.GetUpper() );
        // check if before auto spacing was set during import and spacing we get from actual object is same
        // that we set in import. If yes just write beforeAutoSpacing tag.
        if (m_bParaBeforeAutoSpacing && m_nParaBeforeSpacing == rULSpace.GetUpper())
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_beforeAutospacing ), "1" );
        }
        else if (m_bParaBeforeAutoSpacing && m_nParaBeforeSpacing == -1)
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_beforeAutospacing ), "0" );
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_before ), OString::number( rULSpace.GetUpper() ).getStr() );
        }
        else
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_before ), OString::number( rULSpace.GetUpper() ).getStr() );
        }
        m_bParaBeforeAutoSpacing = false;
        // check if after auto spacing was set during import and spacing we get from actual object is same
        // that we set in import. If yes just write afterAutoSpacing tag.
        if (m_bParaAfterAutoSpacing && m_nParaAfterSpacing == rULSpace.GetLower())
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_afterAutospacing ), "1" );
        }
        else if (m_bParaAfterAutoSpacing && m_nParaAfterSpacing == -1)
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_afterAutospacing ), "0" );
            AddToAttrList( m_pParagraphSpacingAttrList,
                                FSNS( XML_w, XML_after ), OString::number( rULSpace.GetLower()).getStr() );
        }
        else
        {
            AddToAttrList( m_pParagraphSpacingAttrList,
                    FSNS( XML_w, XML_after ), OString::number( rULSpace.GetLower()).getStr() );
        }
        m_bParaAfterAutoSpacing = false;

        if (rULSpace.GetContext())
            m_pSerializer->singleElementNS(XML_w, XML_contextualSpacing);
        else
        {
            // Write out Contextual Spacing = false if it would have inherited a true.
            const SvxULSpaceItem* pInherited = nullptr;
            if (auto pNd = dynamic_cast<const SwContentNode*>(m_rExport.m_pOutFormatNode)) //paragraph
                pInherited = &static_cast<SwTextFormatColl&>(pNd->GetAnyFormatColl()).GetAttrSet().GetULSpace();
            else if (m_rExport.m_bStyDef && m_rExport.m_pCurrentStyle && m_rExport.m_pCurrentStyle->DerivedFrom()) //style
                pInherited = &m_rExport.m_pCurrentStyle->DerivedFrom()->GetULSpace();

            if (pInherited && pInherited->GetContext())
                m_pSerializer->singleElementNS(XML_w, XML_contextualSpacing, FSNS(XML_w, XML_val), "false");
        }
    }
}

namespace docx {

rtl::Reference<FastAttributeList> SurroundToVMLWrap(SwFormatSurround const& rSurround)
{
    rtl::Reference<FastAttributeList> pAttrList;
    OString sType;
    OString sSide;
    switch (rSurround.GetSurround())
    {
        case css::text::WrapTextMode_NONE:
            sType = "topAndBottom";
            break;
        case css::text::WrapTextMode_PARALLEL:
            sType = "square";
            break;
        case css::text::WrapTextMode_DYNAMIC:
            sType = "square";
            sSide = "largest";
            break;
        case css::text::WrapTextMode_LEFT:
            sType = "square";
            sSide = "left";
            break;
        case css::text::WrapTextMode_RIGHT:
            sType = "square";
            sSide = "right";
            break;
        case css::text::WrapTextMode_THROUGH:
            /* empty type and side means through */
        default:
            sType = "none";
            break;
    }
    if (!sType.isEmpty() || !sSide.isEmpty())
    {
        pAttrList = FastSerializerHelper::createAttrList();
        if (!sType.isEmpty())
        {
            pAttrList->add(XML_type, sType);
        }
        if (!sSide.isEmpty())
        {
            pAttrList->add(XML_side, sSide);
        }
    }
    return pAttrList;
}

} // namespace docx

void DocxAttributeOutput::FormatSurround( const SwFormatSurround& rSurround )
{
    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        rtl::Reference<FastAttributeList> pAttrList(docx::SurroundToVMLWrap(rSurround));
        if (pAttrList)
        {
            m_rExport.SdrExporter().setFlyWrapAttrList(pAttrList);
        }
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        OString sWrap( "auto" );
        switch ( rSurround.GetSurround( ) )
        {
            case css::text::WrapTextMode_NONE:
                sWrap = OString( "none" );
                break;
            case css::text::WrapTextMode_THROUGH:
                sWrap = OString( "through" );
                break;
            case css::text::WrapTextMode_DYNAMIC:
            case css::text::WrapTextMode_PARALLEL:
            case css::text::WrapTextMode_LEFT:
            case css::text::WrapTextMode_RIGHT:
            default:
                sWrap = OString( "around" );
        }

        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_wrap ), sWrap.getStr() );
    }
}

void DocxAttributeOutput::FormatVertOrientation( const SwFormatVertOrient& rFlyVert )
{
    OString sAlign   = convertToOOXMLVertOrient( rFlyVert.GetVertOrient() );
    OString sVAnchor = convertToOOXMLVertOrientRel( rFlyVert.GetRelationOrient() );

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";margin-top:" + OString::number(double(rFlyVert.GetPos()) / 20) + "pt");
        if ( !sAlign.isEmpty() )
            m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-vertical:" + sAlign);
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-vertical-relative:" + sVAnchor);
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( !sAlign.isEmpty() )
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_yAlign ), sAlign.getStr() );
        else
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_y ),
                OString::number( rFlyVert.GetPos() ).getStr() );
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_vAnchor ), sVAnchor.getStr() );
    }
}

void DocxAttributeOutput::FormatHorizOrientation( const SwFormatHoriOrient& rFlyHori )
{
    OString sAlign   = convertToOOXMLHoriOrient( rFlyHori.GetHoriOrient(), rFlyHori.IsPosToggle() );
    OString sHAnchor = convertToOOXMLHoriOrientRel( rFlyHori.GetRelationOrient() );

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";margin-left:" + OString::number(double(rFlyHori.GetPos()) / 20) + "pt");
        if ( !sAlign.isEmpty() )
            m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-horizontal:" + sAlign);
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-horizontal-relative:" + sHAnchor);
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( !sAlign.isEmpty() )
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_xAlign ), sAlign.getStr() );
        else
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_x ),
                OString::number( rFlyHori.GetPos() ).getStr() );
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_hAnchor ), sHAnchor.getStr() );
    }
}

void DocxAttributeOutput::FormatAnchor( const SwFormatAnchor& )
{
    // Fly frames: anchors here aren't matching the anchors in docx
}

static std::optional<sal_Int32> lcl_getDmlAlpha(const SvxBrushItem& rBrush)
{
    std::optional<sal_Int32> oRet;
    sal_Int32 nTransparency = 255 - rBrush.GetColor().GetAlpha();
    if (nTransparency)
    {
        // Convert transparency to percent
        sal_Int8 nTransparencyPercent = SvxBrushItem::TransparencyToPercent(nTransparency);

        // Calculate alpha value
        // Consider oox/source/drawingml/color.cxx : getTransparency() function.
        sal_Int32 nAlpha = ::oox::drawingml::MAX_PERCENT - ( ::oox::drawingml::PER_PERCENT * nTransparencyPercent );
        oRet = nAlpha;
    }
    return oRet;
}

void DocxAttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    const Color aColor = rBrush.GetColor();
    OString sColor = msfilter::util::ConvertColor( aColor.GetRGBColor() );
    std::optional<sal_Int32> oAlpha = lcl_getDmlAlpha(rBrush);
    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        // Handle 'Opacity'
        if (oAlpha)
        {
            // Calculate opacity value
            // Consider oox/source/vml/vmlformatting.cxx : decodeColor() function.
            double fOpacity = static_cast<double>(*oAlpha) * 65535 / ::oox::drawingml::MAX_PERCENT;
            OUString sOpacity = OUString::number(fOpacity) + "f";

            AddToAttrList( m_rExport.SdrExporter().getFlyFillAttrList(), XML_opacity, OUStringToOString(sOpacity, RTL_TEXTENCODING_UTF8).getStr() );
        }

        sColor = "#" + sColor;
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), XML_fillcolor, sColor.getStr() );
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        bool bImageBackground = false;
        const SfxPoolItem* pItem = GetExport().HasItem(XATTR_FILLSTYLE);
        if (pItem)
        {
            const XFillStyleItem* pFillStyle = static_cast<const XFillStyleItem*>(pItem);
            if(pFillStyle->GetValue() == drawing::FillStyle_BITMAP)
            {
                bImageBackground = true;
            }
        }
        if (!bImageBackground)
        {
            m_pSerializer->startElementNS(XML_a, XML_solidFill);
            m_pSerializer->startElementNS(XML_a, XML_srgbClr, XML_val, sColor);
            if (oAlpha)
                m_pSerializer->singleElementNS(XML_a, XML_alpha,
                                              XML_val, OString::number(*oAlpha));
            m_pSerializer->endElementNS(XML_a, XML_srgbClr);
            m_pSerializer->endElementNS(XML_a, XML_solidFill);
        }
    }
    else if ( !m_rExport.m_bOutPageDescs )
    {
        // compare fill color with the original fill color
        OString sOriginalFill = OUStringToOString(
                m_sOriginalBackgroundColor, RTL_TEXTENCODING_UTF8 );

        if ( aColor == COL_AUTO )
            sColor = "auto";

        if( !m_pBackgroundAttrList.is() )
        {
            m_pBackgroundAttrList = FastSerializerHelper::createAttrList();
            m_pBackgroundAttrList->add(FSNS(XML_w, XML_fill), sColor);
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_val ), "clear" );
        }
        else if ( sOriginalFill != sColor )
        {
            // fill was modified during edition, theme fill attribute must be dropped
            m_pBackgroundAttrList = FastSerializerHelper::createAttrList();
            m_pBackgroundAttrList->add(FSNS(XML_w, XML_fill), sColor);
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_val ), "clear" );
        }
        m_sOriginalBackgroundColor.clear();
    }
}

void DocxAttributeOutput::FormatFillStyle( const XFillStyleItem& rFillStyle )
{
    if (!m_bIgnoreNextFill)
        m_oFillStyle = rFillStyle.GetValue();
    else
        m_bIgnoreNextFill = false;

    // Don't round-trip grabbag OriginalBackground if the background has been cleared.
    if ( m_pBackgroundAttrList.is() && m_sOriginalBackgroundColor != "auto" && rFillStyle.GetValue() == drawing::FillStyle_NONE )
        m_pBackgroundAttrList.clear();
}

void DocxAttributeOutput::FormatFillGradient( const XFillGradientItem& rFillGradient )
{
    if (m_oFillStyle && *m_oFillStyle == drawing::FillStyle_GRADIENT && !m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        AddToAttrList( m_rExport.SdrExporter().getFlyFillAttrList(), XML_type, "gradient" );

        const XGradient& rGradient = rFillGradient.GetGradientValue();
        OString sStartColor = msfilter::util::ConvertColor(rGradient.GetStartColor());
        OString sEndColor = msfilter::util::ConvertColor(rGradient.GetEndColor());

        // Calculate the angle that was originally in the imported DOCX file
        // (reverse calculate the angle that was converted in the file
        //     /oox/source/vml/vmlformatting.cxx :: FillModel::pushToPropMap
        // and also in
        //     /oox/source/drawingml/fillproperties.cxx :: FillProperties::pushToPropMap
        sal_Int32 nReverseAngle = toDegrees(4500_deg10 - rGradient.GetAngle());
        nReverseAngle = (270 - nReverseAngle) % 360;
        if (nReverseAngle != 0)
            AddToAttrList( m_rExport.SdrExporter().getFlyFillAttrList(),
                    XML_angle, OString::number( nReverseAngle ).getStr() );

        OString sColor1 = sStartColor;
        OString sColor2 = sEndColor;

        switch (rGradient.GetGradientStyle())
        {
            case css::awt::GradientStyle_AXIAL:
                AddToAttrList( m_rExport.SdrExporter().getFlyFillAttrList(), XML_focus, "50%" );
                // If it is an 'axial' gradient - swap the colors
                // (because in the import process they were imported swapped)
                sColor1 = sEndColor;
                sColor2 = sStartColor;
                break;
            case css::awt::GradientStyle_LINEAR: break;
            case css::awt::GradientStyle_RADIAL: break;
            case css::awt::GradientStyle_ELLIPTICAL: break;
            case css::awt::GradientStyle_SQUARE: break;
            case css::awt::GradientStyle_RECT: break;
            default:
                break;
        }

        sColor1 = "#" + sColor1;
        sColor2 = "#" + sColor2;
        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), XML_fillcolor, sColor1.getStr() );
        AddToAttrList( m_rExport.SdrExporter().getFlyFillAttrList(), XML_color2, sColor2.getStr() );
    }
    else if (m_oFillStyle && *m_oFillStyle == drawing::FillStyle_GRADIENT && m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        SwFrameFormat & rFormat(
                const_cast<SwFrameFormat&>(m_rExport.m_pParentFrame->GetFrameFormat()));
        uno::Reference<beans::XPropertySet> const xPropertySet(
            SwXTextFrame::CreateXTextFrame(*rFormat.GetDoc(), &rFormat),
            uno::UNO_QUERY);
        m_rDrawingML.SetFS(m_pSerializer);
        m_rDrawingML.WriteGradientFill(xPropertySet);
    }
    m_oFillStyle.reset();
}

void DocxAttributeOutput::FormatBox( const SvxBoxItem& rBox )
{
    if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        // ugh, exporting fill here is quite some hack... this OutputItemSet abstraction is quite leaky
        // <a:gradFill> should be before <a:ln>.
        const SfxPoolItem* pItem = GetExport().HasItem(XATTR_FILLSTYLE);
        if (pItem)
        {
            const XFillStyleItem* pFillStyle = static_cast<const XFillStyleItem*>(pItem);
            FormatFillStyle(*pFillStyle);
            if (m_oFillStyle && *m_oFillStyle == drawing::FillStyle_BITMAP)
            {
                const SdrObject* pSdrObj = m_rExport.m_pParentFrame->GetFrameFormat().FindRealSdrObject();
                if (pSdrObj)
                {
                    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY );
                    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY );
                    m_rDrawingML.SetFS(m_pSerializer);
                    m_rDrawingML.WriteBlipFill(xPropertySet, "BackGraphic");
                }
            }
        }

        pItem = GetExport().HasItem(XATTR_FILLGRADIENT);
        if (pItem)
        {
            const XFillGradientItem* pFillGradient = static_cast<const XFillGradientItem*>(pItem);
            FormatFillGradient(*pFillGradient);
        }
        m_bIgnoreNextFill = true;
    }
    if (m_rExport.SdrExporter().getTextFrameSyntax() || m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
        const SvxBorderLine* pLeft = rBox.GetLeft( );
        const SvxBorderLine* pTop = rBox.GetTop( );
        const SvxBorderLine* pRight = rBox.GetRight( );
        const SvxBorderLine* pBottom = rBox.GetBottom( );

        if (pLeft && pRight && pTop && pBottom &&
                *pLeft == *pRight && *pLeft == *pTop && *pLeft == *pBottom)
        {
            // Check border style
            SvxBorderLineStyle eBorderStyle = pTop->GetBorderLineStyle();
            if (eBorderStyle == SvxBorderLineStyle::NONE)
            {
                if (m_rExport.SdrExporter().getTextFrameSyntax())
                {
                    AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), 2,
                            XML_stroked, "f", XML_strokeweight, "0pt" );
                }
            }
            else
            {
                OString sColor(msfilter::util::ConvertColor(pTop->GetColor()));
                double const fConverted(editeng::ConvertBorderWidthToWord(pTop->GetBorderLineStyle(), pTop->GetWidth()));

                if (m_rExport.SdrExporter().getTextFrameSyntax())
                {
                    sColor = "#" + sColor;
                    sal_Int32 nWidth = sal_Int32(fConverted / 20);
                    OString sWidth = OString::number(nWidth) + "pt";
                    AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), 2,
                            XML_strokecolor, sColor.getStr(),
                            XML_strokeweight, sWidth.getStr() );
                    if( SvxBorderLineStyle::DASHED == pTop->GetBorderLineStyle() ) // Line Style is Dash type
                        AddToAttrList( m_rExport.SdrExporter().getDashLineStyle(),
                            XML_dashstyle, "dash" );
                }
                else
                    m_rExport.SdrExporter().writeBoxItemLine(rBox);
            }
        }

        if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
        {
            m_rExport.SdrExporter().getBodyPrAttrList()->add(XML_lIns, OString::number(TwipsToEMU(rBox.GetDistance(SvxBoxItemLine::LEFT))));
            m_rExport.SdrExporter().getBodyPrAttrList()->add(XML_tIns, OString::number(TwipsToEMU(rBox.GetDistance(SvxBoxItemLine::TOP))));
            m_rExport.SdrExporter().getBodyPrAttrList()->add(XML_rIns, OString::number(TwipsToEMU(rBox.GetDistance(SvxBoxItemLine::RIGHT))));
            m_rExport.SdrExporter().getBodyPrAttrList()->add(XML_bIns, OString::number(TwipsToEMU(rBox.GetDistance(SvxBoxItemLine::BOTTOM))));
            return;
        }

        // v:textbox's inset attribute: inner margin values for textbox text - write only non-default values
        double fDistanceLeftTwips = double(rBox.GetDistance(SvxBoxItemLine::LEFT));
        double fDistanceTopTwips = double(rBox.GetDistance(SvxBoxItemLine::TOP));
        double fDistanceRightTwips = double(rBox.GetDistance(SvxBoxItemLine::RIGHT));
        double fDistanceBottomTwips = double(rBox.GetDistance(SvxBoxItemLine::BOTTOM));

        // Convert 'TWIPS' to 'INCH' (because in Word the default values are in Inches)
        double fDistanceLeftInch = o3tl::convert(fDistanceLeftTwips, o3tl::Length::twip, o3tl::Length::in);
        double fDistanceTopInch = o3tl::convert(fDistanceTopTwips, o3tl::Length::twip, o3tl::Length::in);
        double fDistanceRightInch = o3tl::convert(fDistanceRightTwips, o3tl::Length::twip, o3tl::Length::in);
        double fDistanceBottomInch = o3tl::convert(fDistanceBottomTwips, o3tl::Length::twip, o3tl::Length::in);

        // This code will write ONLY the non-default values. The values are in 'left','top','right','bottom' order.
        // so 'bottom' is checked if it is default and if it is non-default - all the values will be written
        // otherwise - 'right' is checked if it is default and if it is non-default - all the values except for 'bottom' will be written
        // and so on.
        OStringBuffer aInset;
        if(!aInset.isEmpty() || fDistanceBottomInch != 0.05)
            aInset.insert(0, OStringConcatenation("," + OString::number(fDistanceBottomInch) + "in"));

        if(!aInset.isEmpty() || fDistanceRightInch != 0.1)
            aInset.insert(0, OStringConcatenation("," + OString::number(fDistanceRightInch) + "in"));

        if(!aInset.isEmpty() || fDistanceTopInch != 0.05)
            aInset.insert(0, OStringConcatenation("," + OString::number(fDistanceTopInch) + "in"));

        if(!aInset.isEmpty() || fDistanceLeftInch != 0.1)
            aInset.insert(0, OStringConcatenation(OString::number(fDistanceLeftInch) + "in"));

        if (!aInset.isEmpty())
            m_rExport.SdrExporter().getTextboxAttrList()->add(XML_inset, aInset.makeStringAndClear());

        return;
    }

    OutputBorderOptions aOutputBorderOptions = lcl_getBoxBorderOptions();
    // Check if there is a shadow item
    const SfxPoolItem* pItem = GetExport().HasItem( RES_SHADOW );
    if ( pItem )
    {
        const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
        aOutputBorderOptions.aShadowLocation = pShadowItem->GetLocation();
    }

    if ( m_bOpenedSectPr && !GetWritingHeaderFooter())
        return;

    // Not inside a section

    // Open the paragraph's borders tag
    m_pSerializer->startElementNS(XML_w, XML_pBdr);

    std::map<SvxBoxItemLine, css::table::BorderLine2> aStyleBorders;
    const SvxBoxItem* pInherited = nullptr;
    if ( GetExport().m_pStyAttr )
        pInherited = GetExport().m_pStyAttr->GetItem<SvxBoxItem>(RES_BOX);
    else if ( GetExport().m_pCurrentStyle && GetExport().m_pCurrentStyle->DerivedFrom() )
        pInherited = GetExport().m_pCurrentStyle->DerivedFrom()->GetAttrSet().GetItem<SvxBoxItem>(RES_BOX);

    if ( pInherited )
    {
        aStyleBorders[ SvxBoxItemLine::TOP ] = SvxBoxItem::SvxLineToLine(pInherited->GetTop(), /*bConvert=*/false);
        aStyleBorders[ SvxBoxItemLine::BOTTOM ] = SvxBoxItem::SvxLineToLine(pInherited->GetBottom(), false);
        aStyleBorders[ SvxBoxItemLine::LEFT ] = SvxBoxItem::SvxLineToLine(pInherited->GetLeft(), false);
        aStyleBorders[ SvxBoxItemLine::RIGHT ] = SvxBoxItem::SvxLineToLine(pInherited->GetRight(), false);
    }

    impl_borders( m_pSerializer, rBox, aOutputBorderOptions, aStyleBorders );

    // Close the paragraph's borders tag
    m_pSerializer->endElementNS( XML_w, XML_pBdr );
}

void DocxAttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol& rCol, bool bEven, SwTwips nPageSize )
{
    // Get the columns attributes
    rtl::Reference<FastAttributeList> pColsAttrList = FastSerializerHelper::createAttrList();

    pColsAttrList->add( FSNS( XML_w, XML_num ),
            OString::number( nCols ). getStr( ) );

    const char* pEquals = "false";
    if ( bEven )
    {
        sal_uInt16 nWidth = rCol.GetGutterWidth( true );
        pColsAttrList->add( FSNS( XML_w, XML_space ),
               OString::number( nWidth ).getStr( ) );

        pEquals = "true";
    }

    pColsAttrList->add( FSNS( XML_w, XML_equalWidth ), pEquals );

    bool bHasSep = (COLADJ_NONE != rCol.GetLineAdj());

    pColsAttrList->add( FSNS( XML_w, XML_sep ), OString::boolean( bHasSep ) );

    // Write the element
    m_pSerializer->startElementNS( XML_w, XML_cols, pColsAttrList );

    // Write the columns width if non-equals
    const SwColumns & rColumns = rCol.GetColumns(  );
    if ( !bEven )
    {
        for ( sal_uInt16 n = 0; n < nCols; ++n )
        {
            rtl::Reference<FastAttributeList> pColAttrList = FastSerializerHelper::createAttrList();
            sal_uInt16 nWidth = rCol.CalcPrtColWidth( n, o3tl::narrowing<sal_uInt16>(nPageSize) );
            pColAttrList->add( FSNS( XML_w, XML_w ),
                    OString::number( nWidth ).getStr( ) );

            if ( n + 1 != nCols )
            {
                sal_uInt16 nSpacing = rColumns[n].GetRight( ) + rColumns[n + 1].GetLeft( );
                pColAttrList->add( FSNS( XML_w, XML_space ),
                    OString::number( nSpacing ).getStr( ) );
            }

            m_pSerializer->singleElementNS( XML_w, XML_col, pColAttrList );
        }
    }

    m_pSerializer->endElementNS( XML_w, XML_cols );
}

void DocxAttributeOutput::FormatKeep( const SvxFormatKeepItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_keepNext,
            FSNS( XML_w, XML_val ), OString::boolean( rItem.GetValue() ) );
}

void DocxAttributeOutput::FormatTextGrid( const SwTextGridItem& rGrid )
{
    rtl::Reference<FastAttributeList> pGridAttrList = FastSerializerHelper::createAttrList();

    OString sGridType;
    switch ( rGrid.GetGridType( ) )
    {
        default:
        case GRID_NONE:
            sGridType = OString( "default" );
            break;
        case GRID_LINES_ONLY:
            sGridType = OString( "lines" );
            break;
        case GRID_LINES_CHARS:
            if ( rGrid.IsSnapToChars( ) )
                sGridType = OString( "snapToChars" );
            else
                sGridType = OString( "linesAndChars" );
            break;
    }
    pGridAttrList->add(FSNS(XML_w, XML_type), sGridType);

    sal_uInt16 nHeight = rGrid.GetBaseHeight() + rGrid.GetRubyHeight();
    pGridAttrList->add( FSNS( XML_w, XML_linePitch ),
            OString::number( nHeight ).getStr( ) );

    pGridAttrList->add( FSNS( XML_w, XML_charSpace ),
            OString::number( GridCharacterPitch( rGrid ) ).getStr( ) );

    m_pSerializer->singleElementNS( XML_w, XML_docGrid, pGridAttrList );
}

void DocxAttributeOutput::FormatLineNumbering( const SwFormatLineNumber& rNumbering )
{
    if ( !rNumbering.IsCount( ) )
        m_pSerializer->singleElementNS(XML_w, XML_suppressLineNumbers);
}

void DocxAttributeOutput::FormatFrameDirection( const SvxFrameDirectionItem& rDirection )
{
    OString sTextFlow;
    bool bBiDi = false;
    SvxFrameDirection nDir = rDirection.GetValue();

    if ( nDir == SvxFrameDirection::Environment )
        nDir = GetExport( ).GetDefaultFrameDirection( );

    switch ( nDir )
    {
        default:
        case SvxFrameDirection::Horizontal_LR_TB:
            sTextFlow = OString( "lrTb" );
            break;
        case SvxFrameDirection::Horizontal_RL_TB:
            sTextFlow = OString( "lrTb" );
            bBiDi = true;
            break;
        case SvxFrameDirection::Vertical_LR_TB: // many things but not this one
        case SvxFrameDirection::Vertical_RL_TB:
            sTextFlow = OString( "tbRl" );
            break;
    }

    if ( m_rExport.m_bOutPageDescs )
    {
        m_pSerializer->singleElementNS(XML_w, XML_textDirection, FSNS(XML_w, XML_val), sTextFlow);
        if ( bBiDi )
            m_pSerializer->singleElementNS(XML_w, XML_bidi);
    }
    else if ( !m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( bBiDi )
            m_pSerializer->singleElementNS(XML_w, XML_bidi, FSNS(XML_w, XML_val), "1");
        else
            m_pSerializer->singleElementNS(XML_w, XML_bidi, FSNS(XML_w, XML_val), "0");
    }
}

void DocxAttributeOutput::ParaGrabBag(const SfxGrabBagItem& rItem)
{
    const std::map<OUString, css::uno::Any>& rMap = rItem.GetGrabBag();
    for ( const auto & rGrabBagElement : rMap )
    {
        if (rGrabBagElement.first == "MirrorIndents")
            m_pSerializer->singleElementNS(XML_w, XML_mirrorIndents);
        else if (rGrabBagElement.first == "ParaTopMarginBeforeAutoSpacing")
        {
            m_bParaBeforeAutoSpacing = true;
            // get fixed value which was set during import
            rGrabBagElement.second >>= m_nParaBeforeSpacing;
            m_nParaBeforeSpacing = o3tl::toTwips(m_nParaBeforeSpacing, o3tl::Length::mm100);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << rGrabBagElement.first << " : m_nParaBeforeSpacing= " << m_nParaBeforeSpacing);
        }
        else if (rGrabBagElement.first == "ParaBottomMarginAfterAutoSpacing")
        {
            m_bParaAfterAutoSpacing = true;
            // get fixed value which was set during import
            rGrabBagElement.second >>= m_nParaAfterSpacing;
            m_nParaAfterSpacing = o3tl::toTwips(m_nParaAfterSpacing, o3tl::Length::mm100);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << rGrabBagElement.first << " : m_nParaBeforeSpacing= " << m_nParaAfterSpacing);
        }
        else if (rGrabBagElement.first == "CharThemeFill")
        {
            uno::Sequence<beans::PropertyValue> aGrabBagSeq;
            rGrabBagElement.second >>= aGrabBagSeq;

            for (const auto& rProp : std::as_const(aGrabBagSeq))
            {
                OString sVal = OUStringToOString(rProp.Value.get<OUString>(), RTL_TEXTENCODING_UTF8);

                if (sVal.isEmpty())
                    continue;

                if (rProp.Name == "val")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_val), sVal.getStr());
                else if (rProp.Name == "color")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_color), sVal.getStr());
                else if (rProp.Name == "themeColor")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeColor), sVal.getStr());
                else if (rProp.Name == "themeTint")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeTint), sVal.getStr());
                else if (rProp.Name == "themeShade")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeShade), sVal.getStr());
                else if (rProp.Name == "fill")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_fill), sVal.getStr());
                else if (rProp.Name == "themeFill")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFill), sVal.getStr());
                else if (rProp.Name == "themeFillTint")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFillTint), sVal.getStr());
                else if (rProp.Name == "themeFillShade")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFillShade), sVal.getStr());
                else if (rProp.Name == "originalColor")
                    rProp.Value >>= m_sOriginalBackgroundColor;
            }
        }
        else if (rGrabBagElement.first == "SdtPr")
        {
            const uno::Sequence<beans::PropertyValue> aGrabBagSdt =
                    rGrabBagElement.second.get< uno::Sequence<beans::PropertyValue> >();
            for (const beans::PropertyValue& aPropertyValue : aGrabBagSdt)
            {
                if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartObj" ||
                        aPropertyValue.Name == "ooxml:CT_SdtPr_docPartList")
                {
                    if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartObj")
                        m_nParagraphSdtPrToken = FSNS( XML_w, XML_docPartObj );
                    else if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartList")
                        m_nParagraphSdtPrToken = FSNS( XML_w, XML_docPartList );

                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtDocPart_docPartGallery")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w, XML_docPartGallery ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtDocPart_docPartCategory")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w, XML_docPartCategory ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtDocPart_docPartUnique")
                        {
                            if (sValue.isEmpty())
                                sValue = "true";
                            AddToAttrList( m_pParagraphSdtPrTokenChildren, FSNS( XML_w, XML_docPartUnique ),
                            OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        }
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_equation")
                    m_nParagraphSdtPrToken = FSNS( XML_w, XML_equation );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_picture")
                    m_nParagraphSdtPrToken = FSNS( XML_w, XML_picture );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_citation")
                    m_nParagraphSdtPrToken = FSNS( XML_w, XML_citation );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_group")
                    m_nParagraphSdtPrToken = FSNS( XML_w, XML_group );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_text")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    if (aGrabBag.hasElements())
                    {
                        for (const auto& rProp : std::as_const(aGrabBag))
                        {
                            OUString sValue = rProp.Value.get<OUString>();
                            if (rProp.Name == "ooxml:CT_SdtText_multiLine")
                                AddToAttrList(m_pParagraphSdtPrTextAttrs,
                                    FSNS(XML_w, XML_multiLine),
                                    OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
                        }
                    }
                    else
                    {
                        // We still have w:text, but no attrs
                        m_nParagraphSdtPrToken = FSNS(XML_w, XML_text);
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_dataBinding" && !m_pParagraphSdtPrDataBindingAttrs.is())
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_DataBinding_prefixMappings")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_prefixMappings ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_DataBinding_xpath")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_xpath ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_DataBinding_storeItemID")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_storeItemID ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPlaceholder_docPart")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtPlaceholder_docPart_val")
                            m_aParagraphSdtPrPlaceHolderDocPart = sValue;
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_color")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtColor_val")
                            m_aParagraphSdtPrColor = sValue;
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_alias" && m_aParagraphSdtPrAlias.isEmpty())
                {
                    if (!(aPropertyValue.Value >>= m_aParagraphSdtPrAlias))
                        SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unexpected sdt alias value");
                    m_aStartedParagraphSdtPrAlias = m_aParagraphSdtPrAlias;
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_checkbox")
                {
                    m_nParagraphSdtPrToken = FSNS( XML_w14, XML_checkbox );
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtCheckbox_checked")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checked ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtCheckbox_checkedState")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checkedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtCheckbox_uncheckedState")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_uncheckedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_id")
                    m_bParagraphSdtHasId = true;
                else
                    SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled SdtPr grab bag property " << aPropertyValue.Name);
            }
        }
        else if (rGrabBagElement.first == "ParaCnfStyle")
        {
            uno::Sequence<beans::PropertyValue> aAttributes = rGrabBagElement.second.get< uno::Sequence<beans::PropertyValue> >();
            m_pTableStyleExport->CnfStyle(aAttributes);
        }
        else if (rGrabBagElement.first == "ParaSdtEndBefore")
        {
            // Handled already in StartParagraph().
        }
        else
            SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled grab bag property " << rGrabBagElement.first );
    }
}

void DocxAttributeOutput::CharGrabBag( const SfxGrabBagItem& rItem )
{
    if (m_bPreventDoubleFieldsHandling)
        return;

    const std::map< OUString, css::uno::Any >& rMap = rItem.GetGrabBag();

    // get original values of theme-derived properties to check if they have changed during the edition
    bool bWriteCSTheme = true;
    bool bWriteAsciiTheme = true;
    bool bWriteEastAsiaTheme = true;
    bool bWriteThemeFontColor = true;
    OUString sOriginalValue;
    for ( const auto & rGrabBagElement : rMap )
    {
        if ( m_pFontsAttrList.is() && rGrabBagElement.first == "CharThemeFontNameCs" )
        {
            if ( rGrabBagElement.second >>= sOriginalValue )
                bWriteCSTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_cs ) ) == sOriginalValue );
        }
        else if ( m_pFontsAttrList.is() && rGrabBagElement.first == "CharThemeFontNameAscii" )
        {
            if ( rGrabBagElement.second >>= sOriginalValue )
                bWriteAsciiTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_ascii ) ) == sOriginalValue );
        }
        else if ( m_pFontsAttrList.is() && rGrabBagElement.first == "CharThemeFontNameEastAsia" )
        {
            if ( rGrabBagElement.second >>= sOriginalValue )
                bWriteEastAsiaTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_eastAsia ) ) == sOriginalValue );
        }
        else if ( m_pColorAttrList.is() && rGrabBagElement.first == "CharThemeOriginalColor" )
        {
            if ( rGrabBagElement.second >>= sOriginalValue )
                bWriteThemeFontColor =
                        ( m_pColorAttrList->getOptionalValue( FSNS( XML_w, XML_val ) ) == sOriginalValue );
        }
    }

    // save theme attributes back to the run properties
    OUString str;
    for ( const auto & rGrabBagElement : rMap )
    {
        if ( rGrabBagElement.first == "CharThemeNameAscii" && bWriteAsciiTheme )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_asciiTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeNameCs" && bWriteCSTheme )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_cstheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeNameEastAsia" && bWriteEastAsiaTheme )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_eastAsiaTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeNameHAnsi" && bWriteAsciiTheme )
        // this is not a mistake: in LibO we don't directly support the hAnsi family
        // of attributes so we save the same value from ascii attributes instead
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_hAnsiTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeColor" && bWriteThemeFontColor )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeColor ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeColorShade" )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeShade ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( rGrabBagElement.first == "CharThemeColorTint" )
        {
            rGrabBagElement.second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeTint ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if( rGrabBagElement.first == "CharThemeFontNameCs"   ||
                rGrabBagElement.first == "CharThemeFontNameAscii" ||
                rGrabBagElement.first == "CharThemeFontNameEastAsia" ||
                rGrabBagElement.first == "CharThemeOriginalColor" )
        {
            // just skip these, they were processed before
        }
        else if(rGrabBagElement.first == "CharGlowTextEffect" ||
                rGrabBagElement.first == "CharShadowTextEffect" ||
                rGrabBagElement.first == "CharReflectionTextEffect" ||
                rGrabBagElement.first == "CharTextOutlineTextEffect" ||
                rGrabBagElement.first == "CharTextFillTextEffect" ||
                rGrabBagElement.first == "CharScene3DTextEffect" ||
                rGrabBagElement.first == "CharProps3DTextEffect" ||
                rGrabBagElement.first == "CharLigaturesTextEffect" ||
                rGrabBagElement.first == "CharNumFormTextEffect" ||
                rGrabBagElement.first == "CharNumSpacingTextEffect" ||
                rGrabBagElement.first == "CharStylisticSetsTextEffect" ||
                rGrabBagElement.first == "CharCntxtAltsTextEffect")
        {
            beans::PropertyValue aPropertyValue;
            rGrabBagElement.second >>= aPropertyValue;
            m_aTextEffectsGrabBag.push_back(aPropertyValue);
        }
        else if (rGrabBagElement.first == "SdtEndBefore")
        {
            if (m_bStartedCharSdt)
                m_bEndCharSdt = true;
        }
        else if (rGrabBagElement.first == "SdtPr" && FLY_NOT_PROCESSED != m_nStateOfFlyFrame )
        {
            const uno::Sequence<beans::PropertyValue> aGrabBagSdt =
                    rGrabBagElement.second.get< uno::Sequence<beans::PropertyValue> >();
            for (const beans::PropertyValue& aPropertyValue : aGrabBagSdt)
            {
                if (aPropertyValue.Name == "ooxml:CT_SdtPr_checkbox")
                {
                    m_nRunSdtPrToken = FSNS( XML_w14, XML_checkbox );
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtCheckbox_checked")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checked ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtCheckbox_checkedState")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checkedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_SdtCheckbox_uncheckedState")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_uncheckedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_text")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtText_multiLine")
                            AddToAttrList(m_pRunSdtPrTextAttrs,
                                FSNS(XML_w, XML_multiLine),
                                OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_dataBinding" && !m_pRunSdtPrDataBindingAttrs.is())
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_DataBinding_prefixMappings")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_prefixMappings ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_DataBinding_xpath")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_xpath ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (rProp.Name == "ooxml:CT_DataBinding_storeItemID")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_storeItemID ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPlaceholder_docPart")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtPlaceholder_docPart_val")
                            m_aRunSdtPrPlaceHolderDocPart = sValue;
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_color")
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (const auto& rProp : std::as_const(aGrabBag))
                    {
                        OUString sValue = rProp.Value.get<OUString>();
                        if (rProp.Name == "ooxml:CT_SdtColor_val")
                            m_aRunSdtPrColor = sValue;
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_alias" && m_aRunSdtPrAlias.isEmpty())
                {
                    if (!(aPropertyValue.Value >>= m_aRunSdtPrAlias))
                        SAL_WARN("sw.ww8", "DocxAttributeOutput::CharGrabBag: unexpected sdt alias value");
                }
                //do not overwrite the parent node.
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_text" && !m_pRunSdtPrTokenChildren.is())
                    m_nRunSdtPrToken = FSNS( XML_w, XML_text );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_id" && m_nRunSdtPrToken == 0)
                    // only write id token as a marker if no other exist
                    m_nRunSdtPrToken = FSNS( XML_w, XML_id );
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_citation")
                    m_nRunSdtPrToken = FSNS( XML_w, XML_citation );
            }
        }
        else
            SAL_INFO("sw.ww8", "DocxAttributeOutput::CharGrabBag: unhandled grab bag property " << rGrabBagElement.first);
    }
}

DocxAttributeOutput::DocxAttributeOutput( DocxExport &rExport, const FSHelperPtr& pSerializer, oox::drawingml::DrawingML* pDrawingML )
    : AttributeOutputBase(rExport.GetFilter().getFileUrl()),
      m_rExport( rExport ),
      m_pSerializer( pSerializer ),
      m_rDrawingML( *pDrawingML ),
      m_bEndCharSdt(false),
      m_bStartedCharSdt(false),
      m_bStartedParaSdt(false),
      m_endPageRef( false ),
      m_pFootnotesList( new ::docx::FootnotesList() ),
      m_pEndnotesList( new ::docx::FootnotesList() ),
      m_footnoteEndnoteRefTag( 0 ),
      m_pRedlineData( nullptr ),
      m_nRedlineId( 0 ),
      m_bOpenedSectPr( false ),
      m_bHadSectPr(false),
      m_bRunTextIsOn( false ),
      m_bWritingHeaderFooter( false ),
      m_bAnchorLinkedToNode(false),
      m_bWritingField( false ),
      m_bPreventDoubleFieldsHandling( false ),
      m_nNextBookmarkId( 0 ),
      m_nNextAnnotationMarkId( 0 ),
      m_nEmbedFlyLevel(0),
      m_pMoveRedlineData(nullptr),
      m_pCurrentFrame( nullptr ),
      m_bParagraphOpened( false ),
      m_bParagraphFrameOpen( false ),
      m_bIsFirstParagraph( true ),
      m_bAlternateContentChoiceOpen( false ),
      m_bPostponedProcessingFly( false ),
      m_nColBreakStatus( COLBRK_NONE ),
      m_bPostponedPageBreak( false ),
      m_nTextFrameLevel( 0 ),
      m_closeHyperlinkInThisRun( false ),
      m_closeHyperlinkInPreviousRun( false ),
      m_startedHyperlink( false ),
      m_nHyperLinkCount(0),
      m_nFieldsInHyperlink( 0 ),
      m_bExportingOutline(false),
      m_nChartCount(0),
      pendingPlaceholder( nullptr ),
      m_postitFieldsMaxId( 0 ),
      m_anchorId( 1 ),
      m_nextFontId( 1 ),
      m_tableReference(new TableReference()),
      m_bIgnoreNextFill(false),
      m_pTableStyleExport(std::make_shared<DocxTableStyleExport>(rExport.m_rDoc, pSerializer)),
      m_bParaBeforeAutoSpacing(false),
      m_bParaAfterAutoSpacing(false),
      m_nParaBeforeSpacing(0),
      m_nParaAfterSpacing(0)
    , m_nParagraphSdtPrToken(0)
    , m_nRunSdtPrToken(0)
    , m_nStateOfFlyFrame( FLY_NOT_PROCESSED )
    , m_bParagraphSdtHasId(false)
{
    // Push initial items to the RelId cache. In case the document contains no
    // special streams (headers, footers, etc.) then these items are used
    // during the full export.
    PushRelIdCache();
}

DocxAttributeOutput::~DocxAttributeOutput()
{
}

DocxExport& DocxAttributeOutput::GetExport()
{
    return m_rExport;
}

void DocxAttributeOutput::SetSerializer( ::sax_fastparser::FSHelperPtr const & pSerializer )
{
    m_pSerializer = pSerializer;
    m_pTableStyleExport->SetSerializer(pSerializer);
}

bool DocxAttributeOutput::HasFootnotes() const
{
    return !m_pFootnotesList->isEmpty();
}

bool DocxAttributeOutput::HasEndnotes() const
{
    return !m_pEndnotesList->isEmpty();
}

bool DocxAttributeOutput::HasPostitFields() const
{
    return !m_postitFields.empty();
}

void DocxAttributeOutput::BulletDefinition(int nId, const Graphic& rGraphic, Size aSize)
{
    m_pSerializer->startElementNS(XML_w, XML_numPicBullet,
            FSNS(XML_w, XML_numPicBulletId), OString::number(nId));

    OStringBuffer aStyle;
    // Size is in twips, we need it in points.
    aStyle.append("width:" + OString::number(double(aSize.Width()) / 20));
    aStyle.append("pt;height:" + OString::number(double(aSize.Height()) / 20) + "pt");
    m_pSerializer->startElementNS(XML_w, XML_pict);
    m_pSerializer->startElementNS( XML_v, XML_shape,
            XML_style, aStyle.getStr(),
            FSNS(XML_o, XML_bullet), "t");

    OUString aRelId = m_rDrawingML.WriteImage(rGraphic);
    m_pSerializer->singleElementNS( XML_v, XML_imagedata,
            FSNS(XML_r, XML_id), OUStringToOString(aRelId, RTL_TEXTENCODING_UTF8),
            FSNS(XML_o, XML_title), "");

    m_pSerializer->endElementNS(XML_v, XML_shape);
    m_pSerializer->endElementNS(XML_w, XML_pict);

    m_pSerializer->endElementNS(XML_w, XML_numPicBullet);
}

void DocxAttributeOutput::AddToAttrList( rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nAttrName, const char* sAttrValue )
{
    AddToAttrList( pAttrList, 1, nAttrName, sAttrValue );
}

void DocxAttributeOutput::AddToAttrList( rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nAttrs, ... )
{
    if( !pAttrList.is() )
        pAttrList = FastSerializerHelper::createAttrList();

    va_list args;
    va_start( args, nAttrs );
    for( sal_Int32 i = 0; i<nAttrs; i++)
    {
        sal_Int32 nName = va_arg( args, sal_Int32 );
        const char* pValue = va_arg( args, const char* );
        if( pValue )
            pAttrList->add( nName, pValue );
    }
    va_end( args );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
