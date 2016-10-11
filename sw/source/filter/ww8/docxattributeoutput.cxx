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
#include "docxhelper.hxx"
#include "docxsdrexport.hxx"
#include "docxexportfilter.hxx"
#include "docxfootnotes.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include "fmtcntnt.hxx"
#include "fmtftn.hxx"
#include "fchrfmt.hxx"
#include "tgrditem.hxx"
#include "fmtruby.hxx"
#include "fmtanchr.hxx"
#include "breakit.hxx"
#include "redline.hxx"
#include "unocoll.hxx"
#include "unoframe.hxx"
#include "unodraw.hxx"
#include "textboxhelper.hxx"
#include "rdfhelper.hxx"
#include "wrtww8.hxx"

#include <comphelper/random.hxx>
#include <comphelper/string.hxx>
#include <comphelper/flagguard.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <oox/mathml/export.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

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
#include <editeng/boxitem.hxx>
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
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svl/grabbagitem.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <tools/datetimeutils.hxx>
#include <svl/whiter.hxx>

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
#include <charatr.hxx>
#include <swmodule.hxx>
#include <swtable.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <fmtautofmt.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentRedlineAccess.hxx>

#include <osl/file.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <svtools/miscopt.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <algorithm>

using ::editeng::SvxBorderLine;

using namespace oox;
using namespace docx;
using namespace sax_fastparser;
using namespace nsSwDocInfoSubType;
using namespace nsFieldFlags;
using namespace sw::util;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;

static const sal_Int32 Tag_StartParagraph_1 = 1;
static const sal_Int32 Tag_StartParagraph_2 = 2;
static const sal_Int32 Tag_WriteSdtBlock = 3;
static const sal_Int32 Tag_StartParagraphProperties = 4;
static const sal_Int32 Tag_InitCollectedParagraphProperties = 5;
static const sal_Int32 Tag_StartRun_1 = 6;
static const sal_Int32 Tag_StartRun_2 = 7;
static const sal_Int32 Tag_StartRun_3 = 8;
static const sal_Int32 Tag_EndRun_1 = 9;
static const sal_Int32 Tag_EndRun_2 = 10;
static const sal_Int32 Tag_StartRunProperties = 11;
static const sal_Int32 Tag_InitCollectedRunProperties = 12;
static const sal_Int32 Tag_Redline_1 = 13;
static const sal_Int32 Tag_Redline_2 = 14;
static const sal_Int32 Tag_TableDefinition = 15;
static const sal_Int32 Tag_OutputFlyFrame = 16;
static const sal_Int32 Tag_StartSection = 17;

class FFDataWriterHelper
{
    ::sax_fastparser::FSHelperPtr m_pSerializer;
    void writeCommonStart( const OUString& rName )
    {
        m_pSerializer->startElementNS( XML_w, XML_ffData, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_enabled, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_calcOnExit,
            FSNS( XML_w, XML_val ),
            "0", FSEND );
    }
    void writeFinish()
    {
        m_pSerializer->endElementNS( XML_w, XML_ffData );
    }
public:
    explicit FFDataWriterHelper( const ::sax_fastparser::FSHelperPtr& rSerializer ) : m_pSerializer( rSerializer ){}
    void WriteFormCheckbox( const OUString& rName, const OUString& rDefault, bool bChecked )
    {
       writeCommonStart( rName );
       // Checkbox specific bits
       m_pSerializer->startElementNS( XML_w, XML_checkBox, FSEND );
       // currently hardcoding autosize
       // #TODO check if this defaulted
       m_pSerializer->startElementNS( XML_w, XML_sizeAuto, FSEND );
       m_pSerializer->endElementNS( XML_w, XML_sizeAuto );
       if ( !rDefault.isEmpty() )
       {
           m_pSerializer->singleElementNS( XML_w, XML_default,
               FSNS( XML_w, XML_val ),
                   OUStringToOString( rDefault, RTL_TEXTENCODING_UTF8 ).getStr(), FSEND );
       }
       if ( bChecked )
            m_pSerializer->singleElementNS( XML_w, XML_checked, FSEND );
        m_pSerializer->endElementNS( XML_w, XML_checkBox );
       writeFinish();
    }
    void WriteFormText(  const OUString& rName, const OUString& rDefault )
    {
       writeCommonStart( rName );
       if ( !rDefault.isEmpty() )
       {
           m_pSerializer->startElementNS( XML_w, XML_textInput, FSEND );
           m_pSerializer->singleElementNS( XML_w, XML_default,
               FSNS( XML_w, XML_val ),
               OUStringToOString( rDefault, RTL_TEXTENCODING_UTF8 ).getStr(), FSEND );
           m_pSerializer->endElementNS( XML_w, XML_textInput );
       }
       writeFinish();
    }
};

class FieldMarkParamsHelper
{
    const sw::mark::IFieldmark& mrFieldmark;
    public:
    explicit FieldMarkParamsHelper( const sw::mark::IFieldmark& rFieldmark ) : mrFieldmark( rFieldmark ) {}
    OUString getName() { return mrFieldmark.GetName(); }
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
void DocxAttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 /*nScript*/ )
{
    if (bIsRTL)
        m_pSerializer->singleElementNS( XML_w, XML_rtl, FSNS( XML_w, XML_val ), "true", FSEND );
}

/// Are multiple paragraphs disallowed inside this type of SDT?
static bool lcl_isOnelinerSdt(const OUString& rName)
{
    return rName == "Title" || rName == "Subtitle" || rName == "Company";
}

void DocxAttributeOutput::StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo )
{
    if ( m_nColBreakStatus == COLBRK_POSTPONE )
        m_nColBreakStatus = COLBRK_WRITE;

    // Output table/table row/table cell starts if needed
    if ( pTextNodeInfo.get() )
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
    // TODO also avoid multiline paragarphs in those SDT types for shape text
    bool bOneliner = m_bStartedParaSdt && !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen() && lcl_isOnelinerSdt(m_aStartedParagraphSdtPrAlias);
    if (bEndParaSdt || (m_bStartedParaSdt && m_bHadSectPr) || bOneliner)
    {
        // This is the common case: "close sdt before the current paragraph" was requrested by the next paragraph.
        EndSdtBlock();
        m_bStartedParaSdt = false;
        m_aStartedParagraphSdtPrAlias.clear();
    }
    m_bHadSectPr = false;

    // this mark is used to be able to enclose the paragraph inside a sdr tag.
    // We will only know if we have to do that later.
    m_pSerializer->mark(Tag_StartParagraph_1);

    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );

    // postpone the output of the run (we get it before the paragraph
    // properties, but must write it after them)
    m_pSerializer->mark(Tag_StartParagraph_2);

    // no section break in this paragraph yet; can be set in SectionBreak()
    m_pSectionInfo.reset();

    m_bParagraphOpened = true;
    m_bIsFirstParagraph = false;
}

static void lcl_deleteAndResetTheLists( uno::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenChildren, uno::Reference<sax_fastparser::FastAttributeList>& pSdtPrDataBindingAttrs, OUString& rSdtPrAlias)
{
    if( pSdtPrTokenChildren.is() )
        pSdtPrTokenChildren.clear();
    if( pSdtPrDataBindingAttrs.is() )
        pSdtPrDataBindingAttrs.clear();
    if (!rSdtPrAlias.isEmpty())
        rSdtPrAlias.clear();
}

void DocxAttributeOutput::PopulateFrameProperties(const SwFrameFormat* pFrameFormat, const Size& rSize)
{

    sax_fastparser::FastAttributeList* attrList = FastSerializerHelper::createAttrList();

    awt::Point aPos(pFrameFormat->GetHoriOrient().GetPos(), pFrameFormat->GetVertOrient().GetPos());

    attrList->add( FSNS( XML_w, XML_w), OString::number(rSize.Width()));
    attrList->add( FSNS( XML_w, XML_h), OString::number(rSize.Height()));

    attrList->add( FSNS( XML_w, XML_x), OString::number(aPos.X));
    attrList->add( FSNS( XML_w, XML_y), OString::number(aPos.Y));

    const char* relativeFromH;
    const char* relativeFromV;
    switch (pFrameFormat->GetVertOrient().GetRelationOrient())
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            relativeFromV = "margin";
            break;
        case text::RelOrientation::PAGE_FRAME:
            relativeFromV = "page";
            break;
        case text::RelOrientation::FRAME:
        case text::RelOrientation::TEXT_LINE:
        default:
            relativeFromV = "text";
            break;
    }

    switch (pFrameFormat->GetHoriOrient().GetRelationOrient())
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            relativeFromH = "margin";
            break;
        case text::RelOrientation::PAGE_FRAME:
            relativeFromH = "page";
            break;
        case text::RelOrientation::CHAR:
        case text::RelOrientation::PAGE_RIGHT:
        case text::RelOrientation::FRAME:
        default:
            relativeFromH = "text";
            break;
    }

    switch (pFrameFormat->GetSurround().GetValue())
    {
    case SURROUND_NONE:
        attrList->add( FSNS( XML_w, XML_wrap), "none");
        break;
    case SURROUND_THROUGHT:
        attrList->add( FSNS( XML_w, XML_wrap), "through");
        break;
    case SURROUND_PARALLEL:
        attrList->add( FSNS( XML_w, XML_wrap), "notBeside");
        break;
    case SURROUND_IDEAL:
    default:
        attrList->add( FSNS( XML_w, XML_wrap), "auto");
        break;
    }
    attrList->add( FSNS( XML_w, XML_vAnchor), relativeFromV);
    attrList->add( FSNS( XML_w, XML_hAnchor), relativeFromH);
    attrList->add( FSNS( XML_w, XML_hRule), "exact");

    sax_fastparser::XFastAttributeListRef xAttrList(attrList);
    m_pSerializer->singleElementNS( XML_w, XML_framePr, xAttrList );
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
        for (sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp)
        {
            OUString propName = propList[nProp].Name;
            if (propName == "ParaFrameProperties")
            {
                aFrameProperties = propList[nProp].Value ;
                break;
            }
        }
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
        m_pPostponedCustomShape.reset(new std::list<PostponedDrawing>());
        for (size_t nIndex = 0; nIndex < m_aFramesOfParagraph.size(); ++nIndex)
        {
            m_bParagraphFrameOpen = true;
            ww8::Frame aFrame = m_aFramesOfParagraph[nIndex];
            const SwFrameFormat& rFrameFormat = aFrame.GetFrameFormat();

            if (!TextBoxIsFramePr(rFrameFormat) || m_bWritingHeaderFooter)
            {
                if (m_bStartedCharSdt)
                {
                    // Run-level SDT still open? Close it befor AlternateContent.
                    EndSdtBlock();
                    m_bStartedCharSdt = false;
                }
                m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
                m_pSerializer->startElementNS(XML_mc, XML_AlternateContent, FSEND);
                m_pSerializer->startElementNS(XML_mc, XML_Choice,
                        XML_Requires, "wps",
                        FSEND);
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
                m_rExport.m_pTableInfo = ww8::WW8TableInfo::Pointer_t(new ww8::WW8TableInfo());

                /** FDO#71834 :
                       Save the table reference attributes before calling WriteDMLTextFrame,
                       otherwise the StartParagraph function will use the previous existing
                       table reference attributes since the variable is being shared.
                */
                DocxTableExportContext aDMLTableExportContext;
                pushToTableExportContext(aDMLTableExportContext);
                m_rExport.SdrExporter().writeDMLTextFrame(&aFrame, m_anchorId++);
                popFromTableExportContext(aDMLTableExportContext);
                m_pSerializer->endElementNS(XML_mc, XML_Choice);
                SetAlternateContentChoiceOpen( false );

                // Reset table infos, otherwise the depth of the cells will be incorrect,
                // in case the text frame had table(s) and we try to export the
                // same table second time.
                m_rExport.m_pTableInfo = ww8::WW8TableInfo::Pointer_t(new ww8::WW8TableInfo());
                //reset the tableReference.

                m_pSerializer->startElementNS(XML_mc, XML_Fallback, FSEND);
                DocxTableExportContext aVMLTableExportContext;
                pushToTableExportContext(aVMLTableExportContext);
                m_rExport.SdrExporter().writeVMLTextFrame(&aFrame);
                popFromTableExportContext(aVMLTableExportContext);
                m_rExport.m_pTableInfo = pOldTableInfo;

                m_pSerializer->endElementNS(XML_mc, XML_Fallback);
                m_pSerializer->endElementNS(XML_mc, XML_AlternateContent);
                m_pSerializer->endElementNS( XML_w, XML_r );
                m_bParagraphFrameOpen = false;
            }
            else
            {
                std::shared_ptr<ww8::Frame>  pFramePr;
                pFramePr.reset(new ww8::Frame(aFrame));
                aFramePrTextbox.push_back(pFramePr);
            }
        }
        if (!m_pPostponedCustomShape->empty())
        {
            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
            WritePostponedCustomShape();
            m_pSerializer->endElementNS( XML_w, XML_r );
        }
        m_pPostponedCustomShape.reset(nullptr);

        m_aFramesOfParagraph.clear();
    }

    --m_nTextFrameLevel;

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

    m_pSerializer->endElementNS( XML_w, XML_p );
    // on export sdt blocks are never nested ATM
    if( !m_bAnchorLinkedToNode && !m_bStartedParaSdt )
        WriteSdtBlock( m_nParagraphSdtPrToken, m_pParagraphSdtPrTokenChildren, m_pParagraphSdtPrTokenAttributes, m_pParagraphSdtPrDataBindingAttrs, m_aParagraphSdtPrAlias, /*bPara=*/true );
    else
    {
        //These should be written out to the actual Node and not to the anchor.
        //Clear them as they will be repopulated when the node is processed.
        m_nParagraphSdtPrToken = 0;
        m_bParagraphSdtHasId = false;
        lcl_deleteAndResetTheLists( m_pParagraphSdtPrTokenChildren, m_pParagraphSdtPrDataBindingAttrs, m_aParagraphSdtPrAlias );
    }

    //sdtcontent is written so Set m_bParagraphHasDrawing to false
    m_rExport.SdrExporter().setParagraphHasDrawing( false );
    m_bRunTextIsOn = false;
    m_pSerializer->mergeTopMarks(Tag_StartParagraph_1);

    // Write framePr
    if(!aFramePrTextbox.empty())
    {
        for (std::vector< std::shared_ptr<ww8::Frame> > ::iterator it = aFramePrTextbox.begin() ; it != aFramePrTextbox.end(); ++it)
        {
            DocxTableExportContext aTableExportContext;
            pushToTableExportContext(aTableExportContext);
            m_pCurrentFrame = it->get();
            m_rExport.SdrExporter().writeOnlyTextOfFrame(it->get());
            m_pCurrentFrame = nullptr;
            popFromTableExportContext(aTableExportContext);
        }
        aFramePrTextbox.clear();
    }
    // Check for end of cell, rows, tables here
    FinishTableRowCell( pTextNodeInfoInner );

    if( !m_rExport.SdrExporter().IsDMLAndVMLDrawingOpen() )
        m_bParagraphOpened = false;

}

void DocxAttributeOutput::WriteSdtBlock( sal_Int32& nSdtPrToken,
                                         uno::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenChildren,
                                         uno::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenAttributes,
                                         uno::Reference<sax_fastparser::FastAttributeList>& pSdtPrDataBindingAttrs,
                                         OUString& rSdtPrAlias,
                                         bool bPara )
{
    if( nSdtPrToken > 0 || pSdtPrDataBindingAttrs.is() )
    {
        // sdt start mark
        m_pSerializer->mark(Tag_WriteSdtBlock);

        m_pSerializer->startElementNS( XML_w, XML_sdt, FSEND );

        // output sdt properties
        m_pSerializer->startElementNS( XML_w, XML_sdtPr, FSEND );

        if( nSdtPrToken > 0 && pSdtPrTokenChildren.is() )
        {
            if (!pSdtPrTokenAttributes.is())
                m_pSerializer->startElement( nSdtPrToken, FSEND );
            else
            {
                XFastAttributeListRef xAttrList(pSdtPrTokenAttributes);
                pSdtPrTokenAttributes.clear();
                m_pSerializer->startElement(nSdtPrToken, xAttrList);
            }

            if (nSdtPrToken ==  FSNS( XML_w, XML_date ) || nSdtPrToken ==  FSNS( XML_w, XML_docPartObj ) || nSdtPrToken ==  FSNS( XML_w, XML_docPartList ) || nSdtPrToken ==  FSNS( XML_w14, XML_checkbox )) {
                uno::Sequence<xml::FastAttribute> aChildren = pSdtPrTokenChildren->getFastAttributes();
                for( sal_Int32 i=0; i < aChildren.getLength(); ++i )
                    m_pSerializer->singleElement( aChildren[i].Token,
                                                  FSNS(XML_w, XML_val),
                                                  OUStringToOString( aChildren[i].Value, RTL_TEXTENCODING_UTF8 ).getStr(),
                                                  FSEND );
            }

            m_pSerializer->endElement( nSdtPrToken );
        }
        else if( (nSdtPrToken > 0) && nSdtPrToken != FSNS( XML_w, XML_id ) && !(m_bRunTextIsOn && m_rExport.SdrExporter().IsParagraphHasDrawing()))
        {
            if (!pSdtPrTokenAttributes.is())
                m_pSerializer->singleElement( nSdtPrToken, FSEND );
            else
            {
                XFastAttributeListRef xAttrList(pSdtPrTokenAttributes);
                pSdtPrTokenAttributes.clear();
                m_pSerializer->singleElement(nSdtPrToken, xAttrList);
            }
        }

        if( nSdtPrToken == FSNS( XML_w, XML_id ) || ( bPara && m_bParagraphSdtHasId ) )
            //Word won't open a document with an empty id tag, we fill it with a random number
            m_pSerializer->singleElementNS(XML_w, XML_id, FSNS(XML_w, XML_val),
                                          OString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max())),
                                          FSEND);

        if( pSdtPrDataBindingAttrs.is() && !m_rExport.SdrExporter().IsParagraphHasDrawing())
        {
            XFastAttributeListRef xAttrList( pSdtPrDataBindingAttrs );
            pSdtPrDataBindingAttrs.clear();
            m_pSerializer->singleElementNS( XML_w, XML_dataBinding, xAttrList );
        }

        if (!rSdtPrAlias.isEmpty())
            m_pSerializer->singleElementNS(XML_w, XML_alias, FSNS(XML_w, XML_val),
                                           OUStringToOString(rSdtPrAlias, RTL_TEXTENCODING_UTF8).getStr(),
                                           FSEND);

        m_pSerializer->endElementNS( XML_w, XML_sdtPr );

        // sdt contents start tag
        m_pSerializer->startElementNS( XML_w, XML_sdtContent, FSEND );

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
        rSdtPrAlias.clear();
    }
}

void DocxAttributeOutput::EndSdtBlock()
{
    m_pSerializer->endElementNS( XML_w, XML_sdtContent );
    m_pSerializer->endElementNS( XML_w, XML_sdt );
}

#define MAX_CELL_IN_WORD 62

void DocxAttributeOutput::SyncNodelessCells(ww8::WW8TableNodeInfoInner::Pointer_t pInner, sal_Int32 nCell, sal_uInt32 nRow)
{
    sal_Int32 nOpenCell = lastOpenCell.back();
    if (nOpenCell != -1 && nOpenCell != nCell && nOpenCell < MAX_CELL_IN_WORD)
        EndTableCell(pInner, nOpenCell, nRow);

    sal_Int32 nClosedCell = lastClosedCell.back();
    for (sal_Int32 i = nClosedCell+1; i < nCell; ++i)
    {
        if (i >= MAX_CELL_IN_WORD)
            break;

        if (i == 0)
            StartTableRow(pInner);

        StartTableCell(pInner, i, nRow);
        m_pSerializer->singleElementNS( XML_w, XML_p, FSEND );
        EndTableCell(pInner, i, nRow);
    }
}

void DocxAttributeOutput::FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool bForceEmptyParagraph )
{
    if ( pInner.get() )
    {
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
                //Start missing trailing cell
                ++nCell;
                StartTableCell(pInner, nCell, nRow);
            }

            if (bForceEmptyParagraph)
            {
                m_pSerializer->singleElementNS( XML_w, XML_p, FSEND );
            }

            EndTableCell(pInner, nCell, nRow);
        }

        // This is a line end
        if (bEndRow)
            EndTableRow();

        // This is the end of the table
        if (pInner->isFinalEndOfLine())
            EndTable();
    }
}

void DocxAttributeOutput::EmptyParagraph()
{
    m_pSerializer->singleElementNS( XML_w, XML_p, FSEND );
}

void DocxAttributeOutput::SectionBreaks(const SwNode& rNode)
{
    // output page/section breaks
    // Writer can have them at the beginning of a paragraph, or at the end, but
    // in docx, we have to output them in the paragraph properties of the last
    // paragraph in a section.  To get it right, we have to switch to the next
    // paragraph, and detect the section breaks there.
    SwNodeIndex aNextIndex( rNode, 1 );

    if (rNode.IsTextNode())
    {
        if (aNextIndex.GetNode().IsTextNode())
        {
            const SwTextNode* pTextNode = static_cast<SwTextNode*>(&aNextIndex.GetNode());
            m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode, m_tableReference->m_bTableCellOpen, pTextNode->GetText().isEmpty());
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
        // End of something: make sure that it's the end of a table.
        assert(rNode.StartOfSectionNode()->IsTableNode());
        if (aNextIndex.GetNode().IsTextNode())
        {
            // Handle section break between a table and a text node following it.
            const SwTextNode* pTextNode = aNextIndex.GetNode().GetTextNode();
            m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode, m_tableReference->m_bTableCellOpen, pTextNode->GetText().isEmpty());
        }
    }
}

void DocxAttributeOutput::StartParagraphProperties()
{
    m_pSerializer->mark(Tag_StartParagraphProperties);

    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );

    // and output the section break now (if it appeared)
    if ( m_pSectionInfo && (!m_setFootnote))
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
    sal_Int32 len = sizeof ( aOrder ) / sizeof( sal_Int32 );
    uno::Sequence< sal_Int32 > aSeqOrder( len );
    for ( sal_Int32 i = 0; i < len; i++ )
        aSeqOrder[i] = aOrder[i];

    m_pSerializer->mark(Tag_InitCollectedParagraphProperties, aSeqOrder);
}

void DocxAttributeOutput::WriteCollectedParagraphProperties()
{
    if ( m_rExport.SdrExporter().getFlyAttrList().is() )
    {
        XFastAttributeListRef xAttrList( m_rExport.SdrExporter().getFlyAttrList() );
        m_rExport.SdrExporter().getFlyAttrList().clear();

        m_pSerializer->singleElementNS( XML_w, XML_framePr, xAttrList );
    }

    if ( m_pParagraphSpacingAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pParagraphSpacingAttrList );
        m_pParagraphSpacingAttrList.clear();

        m_pSerializer->singleElementNS( XML_w, XML_spacing, xAttrList );
    }

    if ( m_pBackgroundAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pBackgroundAttrList );
        m_pBackgroundAttrList.clear();

        m_pSerializer->singleElementNS( XML_w, XML_shd, xAttrList );
    }
}

namespace
{

/// Outputs an item set, that contains the formatting of the paragraph marker.
void lcl_writeParagraphMarkerProperties(DocxAttributeOutput& rAttributeOutput, const SfxItemSet& rParagraphMarkerProperties)
{
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
    m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
    // mark() before paragraph mark properties child elements.
    InitCollectedRunProperties();

    // The 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList' are used to hold information
    // that should be collected by different properties in the core, and are all flushed together
    // to the DOCX when the function 'WriteCollectedRunProperties' gets called.
    // So we need to store the current status of these lists, so that we can revert back to them when
    // we are done exporting the redline attributes.
    uno::Reference<sax_fastparser::FastAttributeList> pFontsAttrList_Original(m_pFontsAttrList);
    m_pFontsAttrList.clear();
    uno::Reference<sax_fastparser::FastAttributeList> pEastAsianLayoutAttrList_Original(m_pEastAsianLayoutAttrList);
    m_pEastAsianLayoutAttrList.clear();
    uno::Reference<sax_fastparser::FastAttributeList> pCharLangAttrList_Original(m_pCharLangAttrList);
    m_pCharLangAttrList.clear();

    lcl_writeParagraphMarkerProperties(*this, rParagraphMarkerProperties);

    // Write the collected run properties that are stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
    WriteCollectedRunProperties();

    // Revert back the original values that were stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
    m_pFontsAttrList = pFontsAttrList_Original;
    m_pEastAsianLayoutAttrList = pEastAsianLayoutAttrList_Original;
    m_pCharLangAttrList = pCharLangAttrList_Original;

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
        if (TextBoxIsFramePr(rFrameFormat))
        {
            const Size aSize = m_pCurrentFrame->GetSize();
            PopulateFrameProperties(&rFrameFormat, aSize);
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
                                      FSNS(XML_w, XML_element), "RDF",
                                      FSEND);
        m_pSerializer->startElementNS(XML_w, XML_smartTagPr, FSEND);
        for (const std::pair<OUString, OUString>& rStatement : aStatements)
            m_pSerializer->singleElementNS(XML_w, XML_attr,
                                           FSNS(XML_w, XML_name), rStatement.first.toUtf8(),
                                           FSNS(XML_w, XML_val), rStatement.second.toUtf8(),
                                           FSEND);
        m_pSerializer->endElementNS(XML_w, XML_smartTagPr);
        m_pSerializer->endElementNS(XML_w, XML_smartTag);
    }

    if ( m_nColBreakStatus == COLBRK_WRITE )
    {
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_br,
                FSNS( XML_w, XML_type ), "column", FSEND );
        m_pSerializer->endElementNS( XML_w, XML_r );

        m_nColBreakStatus = COLBRK_NONE;
    }

    if ( m_bPostponedPageBreak )
    {
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_br,
                FSNS( XML_w, XML_type ), "page", FSEND );
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

void DocxAttributeOutput::StartRun( const SwRedlineData* pRedlineData, bool /*bSingleEmptyRun*/ )
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

void DocxAttributeOutput::EndRun()
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
        if ( pIt->bOpen && pIt->pField )
        {
            StartField_Impl( *pIt );

            // Remove the field from the stack if only the start has to be written
            // Unknown fields should be removed too
            if ( !pIt->bClose || ( pIt->eType == ww::eUNKNOWN ) )
            {
                pIt = m_Fields.erase( pIt );
                continue;
            }

            if (m_startedHyperlink)
                ++m_nFieldsInHyperlink;

            if ( m_pHyperlinkAttrList.is() )
            {
                m_nFieldsInHyperlink++;
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
        // This is the common case: "close sdt before the current run" was requrested by the next run.

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
                EndField_Impl( m_Fields.back( ) );
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
        if ( pIt->bOpen && !pIt->pField )
        {
            StartField_Impl( *pIt, true );

            if (m_startedHyperlink)
                ++m_nFieldsInHyperlink;

            // Remove the field if no end needs to be written
            if ( !pIt->bClose ) {
                pIt = m_Fields.erase( pIt );
                continue;
            }
        }
        ++pIt;
    }

    // Start the hyperlink after the fields separators or we would generate invalid file
    if ( m_pHyperlinkAttrList.is() )
    {
        XFastAttributeListRef xAttrList ( m_pHyperlinkAttrList );
        m_pHyperlinkAttrList.clear();

        m_pSerializer->startElementNS( XML_w, XML_hyperlink, xAttrList );
        m_startedHyperlink = true;
        m_nHyperLinkCount++;
    }

    // if there is some redlining in the document, output it
    StartRedline( m_pRedlineData );

    DoWriteBookmarks( );
    DoWriteAnnotationMarks( );

    if( m_closeHyperlinkInThisRun && m_startedHyperlink && !m_hyperLinkAnchor.isEmpty() && m_hyperLinkAnchor.startsWith("_Toc"))
    {
        OUString sToken;
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_webHidden, FSEND );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        m_pSerializer->startElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin",
                FSEND );
        m_pSerializer->endElementNS( XML_w, XML_fldChar );
        m_pSerializer->endElementNS( XML_w, XML_r );


        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_webHidden, FSEND );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        sToken = "PAGEREF " + m_hyperLinkAnchor + " \\h"; // '\h' Creates a hyperlink to the bookmarked paragraph.
        DoWriteCmd( sToken );
        m_pSerializer->endElementNS( XML_w, XML_r );

        // Write the Field separator
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_webHidden, FSEND );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
        m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "separate",
                FSEND );
        m_pSerializer->endElementNS( XML_w, XML_r );
        // At start of every "PAGEREF" field m_endPageRef value should be true.
        m_endPageRef = true;
    }

    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    if(GetExport().m_bTabInTOC && m_pHyperlinkAttrList.is())
    {
        RunText("\t") ;
    }
    m_pSerializer->mergeTopMarks(Tag_EndRun_1, sax_fastparser::MergeMarks::PREPEND); // merges with "postponed run start", see above

    // write the run start + the run content
    m_pSerializer->mergeTopMarks(Tag_StartRun_2); // merges the "actual run start"
    // append the actual run end
    m_pSerializer->endElementNS( XML_w, XML_r );

    // if there is some redlining in the document, output it
    EndRedline( m_pRedlineData );

    // enclose in a sdt block, if necessary: if one is already started, then don't do it for now
    // (so on export sdt blocks are never nested ATM)
    if ( !m_bAnchorLinkedToNode && !m_bStartedCharSdt )
    {
        uno::Reference<sax_fastparser::FastAttributeList> pRunSdtPrTokenAttributes;
        WriteSdtBlock( m_nRunSdtPrToken, m_pRunSdtPrTokenChildren, pRunSdtPrTokenAttributes, m_pRunSdtPrDataBindingAttrs, m_aRunSdtPrAlias, /*bPara=*/false );
    }
    else
    {
        //These should be written out to the actual Node and not to the anchor.
        //Clear them as they will be repopulated when the node is processed.
        m_nRunSdtPrToken = 0;
        lcl_deleteAndResetTheLists( m_pRunSdtPrTokenChildren, m_pRunSdtPrDataBindingAttrs, m_aRunSdtPrAlias );
    }

    if (bCloseEarlierSDT)
    {
        m_pSerializer->mark(Tag_EndRun_2);
        EndSdtBlock();
        m_pSerializer->mergeTopMarks(Tag_EndRun_2, sax_fastparser::MergeMarks::PREPEND);
    }

    m_pSerializer->mergeTopMarks(Tag_StartRun_1);

    for (std::vector<const SwOLENode*>::iterator it = m_aPostponedMaths.begin(); it != m_aPostponedMaths.end(); ++it)
        WritePostponedMath(*it);
    m_aPostponedMaths.clear();

    for (std::vector<const SdrObject*>::iterator it = m_aPostponedFormControls.begin(); it != m_aPostponedFormControls.end(); ++it)
        WritePostponedFormControl(*it);
    m_aPostponedFormControls.clear();

    WritePendingPlaceholder();

    m_pRedlineData = nullptr;

    if ( m_closeHyperlinkInThisRun )
    {
        if ( m_startedHyperlink )
        {
            if( m_endPageRef )
            {
                // Hyperlink is started and fldchar "end" needs to be written for PAGEREF
                m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
                m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
                m_pSerializer->singleElementNS( XML_w, XML_webHidden, FSEND );
                m_pSerializer->endElementNS( XML_w, XML_rPr );
                m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                        FSNS( XML_w, XML_fldCharType ), "end",
                        FSEND );
                m_pSerializer->endElementNS( XML_w, XML_r );
                m_endPageRef = false;
                m_hyperLinkAnchor.clear();
            }
            for ( int i = 0; i < m_nFieldsInHyperlink; i++ )
            {
                // If fields begin after hyperlink start then
                // it should end before hyperlink close
                EndField_Impl( m_Fields.back( ) );
                m_Fields.pop_back();
            }
            m_nFieldsInHyperlink = 0;

            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
            m_startedHyperlink = false;
            m_nHyperLinkCount--;
        }
        m_closeHyperlinkInThisRun = false;
    }

    if (!m_startedHyperlink)
    {
        while ( m_Fields.begin() != m_Fields.end() )
        {
            EndField_Impl( m_Fields.front( ) );
            m_Fields.erase( m_Fields.begin( ) );
        }
        m_nFieldsInHyperlink = 0;
    }
}

void DocxAttributeOutput::DoWriteBookmarks()
{
    // Write the start bookmarks
    for ( std::vector< OString >::const_iterator it = m_rBookmarksStart.begin(), end = m_rBookmarksStart.end();
          it != end; ++it )
    {
        const OString& rName = *it;

        // Output the bookmark
        const sal_Int32 nId = m_nNextBookmarkId++;
        m_rOpenedBookmarksIds[rName] = nId;
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkStart,
            FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
            FSNS( XML_w, XML_name ), rName.getStr(),
            FSEND );
        m_sLastOpenedBookmark = rName;
    }
    m_rBookmarksStart.clear();

    // export the end bookmarks
    for ( std::vector< OString >::const_iterator it = m_rBookmarksEnd.begin(), end = m_rBookmarksEnd.end();
          it != end; ++it )
    {
        const OString& rName = *it;

        // Get the id of the bookmark
        std::map< OString, sal_Int32 >::iterator pPos = m_rOpenedBookmarksIds.find( rName );
        if ( pPos != m_rOpenedBookmarksIds.end(  ) )
        {
            const sal_Int32 nId = ( *pPos ).second;
            m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
                FSEND );
            m_rOpenedBookmarksIds.erase( rName );
        }
    }
    m_rBookmarksEnd.clear();
}

void DocxAttributeOutput::DoWriteAnnotationMarks()
{
    // Write the start annotation marks
    for ( std::vector< OString >::const_iterator it = m_rAnnotationMarksStart.begin(), end = m_rAnnotationMarksStart.end();
          it != end; ++it )
    {
        const OString& rName = *it;

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
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
                FSEND );
            m_sLastOpenedAnnotationMark = rName;
        }
    }
    m_rAnnotationMarksStart.clear();

    // export the end annotation marks
    for ( std::vector< OString >::const_iterator it = m_rAnnotationMarksEnd.begin(), end = m_rAnnotationMarksEnd.end();
          it != end; ++it )
    {
        const OString& rName = *it;

        // Get the id of the annotation mark
        std::map< OString, sal_Int32 >::iterator pPos = m_rOpenedAnnotationMarksIds.find( rName );
        if ( pPos != m_rOpenedAnnotationMarksIds.end(  ) )
        {
            const sal_Int32 nId = ( *pPos ).second;
            m_pSerializer->singleElementNS( XML_w, XML_commentRangeEnd,
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
                FSEND );
            m_rOpenedAnnotationMarksIds.erase( rName );

            m_pSerializer->startElementNS(XML_w, XML_r, FSEND);
            m_pSerializer->singleElementNS( XML_w, XML_commentReference, FSNS( XML_w, XML_id ),
                                            OString::number( nId ).getStr(),
                                            FSEND );
            m_pSerializer->endElementNS(XML_w, XML_r);
        }
    }
    m_rAnnotationMarksEnd.clear();
}

void DocxAttributeOutput::WriteFFData(  const FieldInfos& rInfos )
{
    const ::sw::mark::IFieldmark& rFieldmark = *rInfos.pFieldmark;
    if ( rInfos.eType == ww::eFORMDROPDOWN )
    {
        uno::Sequence< OUString> vListEntries;
        OUString sName, sHelp, sToolTip, sSelected;

        FieldMarkParamsHelper params( rFieldmark );
        params.extractParam( ODF_FORMDROPDOWN_LISTENTRY, vListEntries );
        sName = params.getName();
        sal_Int32 nSelectedIndex = 0;

        if ( params.extractParam( ODF_FORMDROPDOWN_RESULT, nSelectedIndex ) )
        {
            if (nSelectedIndex < vListEntries.getLength() )
                sSelected = vListEntries[ nSelectedIndex ];
        }

        GetExport().DoComboBox( sName, sHelp, sToolTip, sSelected, vListEntries );
    }
    else if ( rInfos.eType == ww::eFORMCHECKBOX )
    {
        OUString sName;
        bool bChecked = false;

        FieldMarkParamsHelper params( rFieldmark );
        params.extractParam( ODF_FORMCHECKBOX_NAME, sName );

        const sw::mark::ICheckboxFieldmark* pCheckboxFm = dynamic_cast<const sw::mark::ICheckboxFieldmark*>(&rFieldmark);
        if ( pCheckboxFm && pCheckboxFm->IsChecked() )
            bChecked = true;

        FFDataWriterHelper ffdataOut( m_pSerializer );
        ffdataOut.WriteFormCheckbox( sName, OUString(), bChecked );
    }
    else if ( rInfos.eType == ww::eFORMTEXT )
    {
        FieldMarkParamsHelper params( rFieldmark );
        FFDataWriterHelper ffdataOut( m_pSerializer );
        ffdataOut.WriteFormText( params.getName(), OUString() );
    }
}

void DocxAttributeOutput::StartField_Impl( FieldInfos& rInfos, bool bWriteRun )
{
    if ( rInfos.pField && rInfos.eType == ww::eUNKNOWN )
    {
        // Expand unsupported fields
        RunText( rInfos.pField->GetFieldName() );
    }
    else if ( rInfos.eType != ww::eNONE ) // HYPERLINK fields are just commands
    {
        if ( bWriteRun )
            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

        if ( rInfos.eType == ww::eFORMDROPDOWN )
        {
                m_pSerializer->startElementNS( XML_w, XML_fldChar,
                    FSNS( XML_w, XML_fldCharType ), "begin",
                    FSEND );
                if ( rInfos.pFieldmark && !rInfos.pField )
                    WriteFFData(  rInfos );
                if ( rInfos.pField )
                {
                    const SwDropDownField& rField2 = *static_cast<const SwDropDownField*>(rInfos.pField.get());
                    uno::Sequence<OUString> aItems =
                        rField2.GetItemSequence();
                    GetExport().DoComboBox(rField2.GetName(),
                               rField2.GetHelp(),
                               rField2.GetToolTip(),
                               rField2.GetSelectedItem(), aItems);
                }
                m_pSerializer->endElementNS( XML_w, XML_fldChar );

                if ( bWriteRun )
                    m_pSerializer->endElementNS( XML_w, XML_r );
                if ( !rInfos.pField )
                    CmdField_Impl( rInfos );

        }
        else
        {
            // Write the field start
            if ( rInfos.pField && rInfos.pField->GetSubType() & FIXEDFLD )
            {
                m_pSerializer->startElementNS( XML_w, XML_fldChar,
                    FSNS( XML_w, XML_fldCharType ), "begin",
                    FSNS( XML_w, XML_fldLock ), "true",
                    FSEND );
            }
            else
            {
                m_pSerializer->startElementNS( XML_w, XML_fldChar,
                    FSNS( XML_w, XML_fldCharType ), "begin",
                    FSEND );
            }

            if ( rInfos.pFieldmark )
                WriteFFData(  rInfos );

            m_pSerializer->endElementNS( XML_w, XML_fldChar );

            if ( bWriteRun )
                m_pSerializer->endElementNS( XML_w, XML_r );

            // The hyperlinks fields can't be expanded: the value is
            // normally in the text run
            if ( !rInfos.pField )
                CmdField_Impl( rInfos );
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
    m_pSerializer->startElementNS( XML_w, XML_instrText, FSEND );
    m_pSerializer->writeEscaped( rCmd );
    m_pSerializer->endElementNS( XML_w, XML_instrText );

}

void DocxAttributeOutput::CmdField_Impl( FieldInfos& rInfos )
{
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    sal_Int32 nNbToken = comphelper::string::getTokenCount(rInfos.sCmd, '\t');

    for ( sal_Int32 i = 0; i < nNbToken; i++ )
    {
        OUString sToken = rInfos.sCmd.getToken( i, '\t' );
        if ( rInfos.eType ==  ww::eCREATEDATE
          || rInfos.eType ==  ww::eSAVEDATE
          || rInfos.eType ==  ww::ePRINTDATE
          || rInfos.eType ==  ww::eDATE
          || rInfos.eType ==  ww::eTIME )
        {
           sToken = sToken.replaceAll("NNNN", "dddd");
           sToken = sToken.replaceAll("NN", "ddd");
        }

        // Write the Field command
        DoWriteCmd( sToken );

        // Replace tabs by </instrText><tab/><instrText>
        if ( i < ( nNbToken - 1 ) )
            RunText( "\t" );
    }

    m_pSerializer->endElementNS( XML_w, XML_r );

    // Write the Field separator
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_fldChar,
          FSNS( XML_w, XML_fldCharType ), "separate",
          FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );
}

void DocxAttributeOutput::EndField_Impl( FieldInfos& rInfos )
{
    // The command has to be written before for the hyperlinks
    if ( rInfos.pField )
    {
        CmdField_Impl( rInfos );
    }

    // Write the bookmark start if any
    OUString aBkmName( m_sFieldBkm );
    if ( !aBkmName.isEmpty() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkStart,
               FSNS( XML_w, XML_id ), OString::number( m_nNextBookmarkId ).getStr( ),
               FSNS( XML_w, XML_name ), OUStringToOString( aBkmName, RTL_TEXTENCODING_UTF8 ).getStr( ),
               FSEND );
    }

    if (rInfos.pField ) // For hyperlinks and TOX
    {
        // Write the Field latest value
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        OUString sExpand;
        if(rInfos.eType == ww::eCITATION)
        {
            sExpand = static_cast<SwAuthorityField const*>(rInfos.pField.get())
                        ->ExpandCitation(AUTH_FIELD_TITLE);
        }
        else
        {
            sExpand = rInfos.pField->ExpandField( true );
        }
        // newlines embedded in fields are 0x0B in MSO and 0x0A for us
        RunText(sExpand.replace(0x0A, 0x0B));

        m_pSerializer->endElementNS( XML_w, XML_r );
    }

    // Write the bookmark end if any
    if ( !aBkmName.isEmpty() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
               FSNS( XML_w, XML_id ), OString::number( m_nNextBookmarkId ).getStr( ),
               FSEND );

        m_nNextBookmarkId++;
    }

    // Write the Field end
    if ( rInfos.bClose  )
    {
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_fldChar,
              FSNS( XML_w, XML_fldCharType ), "end",
              FSEND );
        m_pSerializer->endElementNS( XML_w, XML_r );
    }
    // Write the ref field if a bookmark had to be set and the field
    // should be visible
    if ( rInfos.pField )
    {
        sal_uInt16 nSubType = rInfos.pField->GetSubType( );
        bool bIsSetField = rInfos.pField->GetTyp( )->Which( ) == RES_SETEXPFLD;
        bool bShowRef = bIsSetField && ( nSubType & nsSwExtendedSubType::SUB_INVISIBLE ) == 0;

        if ( ( !m_sFieldBkm.isEmpty() ) && bShowRef )
        {
            // Write the field beginning
            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin",
                FSEND );
            m_pSerializer->endElementNS( XML_w, XML_r );

            rInfos.sCmd = FieldString( ww::eREF );
            rInfos.sCmd += "\"";
            rInfos.sCmd += m_sFieldBkm;
            rInfos.sCmd += "\" ";

            // Clean the field bookmark data to avoid infinite loop
            m_sFieldBkm = OUString( );

            // Write the end of the field
            EndField_Impl( rInfos );
        }
    }
}

void DocxAttributeOutput::StartRunProperties()
{
    // postpone the output so that we can later [in EndRunProperties()]
    // prepend the properties before the text
    m_pSerializer->mark(Tag_StartRunProperties);

    m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );

    if(GetExport().m_bHideTabLeaderAndPageNumbers && m_pHyperlinkAttrList.is() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_webHidden, FSEND );
    }
    InitCollectedRunProperties();

    OSL_ASSERT( !m_pPostponedGraphic );
    m_pPostponedGraphic.reset(new std::list<PostponedGraphic>());

    OSL_ASSERT( !m_pPostponedDiagrams );
    m_pPostponedDiagrams.reset(new std::list<PostponedDiagram>());

    OSL_ASSERT( !m_pPostponedVMLDrawings );
    m_pPostponedVMLDrawings.reset(new std::list<PostponedDrawing>());

    assert(!m_pPostponedDMLDrawings);
    m_pPostponedDMLDrawings.reset(new std::list<PostponedDrawing>());

    assert( !m_pPostponedOLEs );
    m_pPostponedOLEs.reset(new std::list<PostponedOLE>());
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
    sal_Int32 len = sizeof ( aOrder ) / sizeof( sal_Int32 );
    uno::Sequence< sal_Int32 > aSeqOrder( len );
    for ( sal_Int32 i = 0; i < len; i++ )
        aSeqOrder[i] = aOrder[i];

    m_pSerializer->mark(Tag_InitCollectedRunProperties, aSeqOrder);
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

boost::optional<sal_Int32> lclGetElementIdForName(const OUString& rName)
{
    sal_Int32 aLength = sizeof (constNameToIdMapping) / sizeof(NameToId);
    for (sal_Int32 i=0; i < aLength; ++i)
    {
        if (rName == constNameToIdMapping[i].maName)
        {
            return constNameToIdMapping[i].maId;
        }
    }
    return boost::optional<sal_Int32>();
}

void lclProcessRecursiveGrabBag(sal_Int32 aElementId, const css::uno::Sequence<css::beans::PropertyValue>& rElements, sax_fastparser::FSHelperPtr pSerializer)
{
    css::uno::Sequence<css::beans::PropertyValue> aAttributes;
    FastAttributeList* pAttributes = FastSerializerHelper::createAttrList();

    for (sal_Int32 j=0; j < rElements.getLength(); ++j)
    {
        if (rElements[j].Name == "attributes")
        {
            rElements[j].Value >>= aAttributes;
        }
    }

    for (sal_Int32 j=0; j < aAttributes.getLength(); ++j)
    {
        uno::Any aAny = aAttributes[j].Value;
        OString aValue;

        if(aAny.getValueType() == cppu::UnoType<sal_Int32>::get())
        {
            aValue = OString::number(aAny.get<sal_Int32>());
        }
        else if(aAny.getValueType() == cppu::UnoType<OUString>::get())
        {
            aValue =  OUStringToOString(aAny.get<OUString>(), RTL_TEXTENCODING_ASCII_US);
        }

        boost::optional<sal_Int32> aSubElementId = lclGetElementIdForName(aAttributes[j].Name);
        if(aSubElementId)
            pAttributes->add(*aSubElementId, aValue.getStr());
    }

    XFastAttributeListRef xAttributesList( pAttributes );

    pSerializer->startElement(aElementId, xAttributesList);

    for (sal_Int32 j=0; j < rElements.getLength(); ++j)
    {
        css::uno::Sequence<css::beans::PropertyValue> aSumElements;

        boost::optional<sal_Int32> aSubElementId = lclGetElementIdForName(rElements[j].Name);
        if(aSubElementId)
        {
            rElements[j].Value >>= aSumElements;
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
        XFastAttributeListRef xAttrList( m_pFontsAttrList );
        m_pFontsAttrList.clear();
        m_pSerializer->singleElementNS( XML_w, XML_rFonts, xAttrList );
    }

    if ( m_pColorAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pColorAttrList );
        m_pColorAttrList.clear();

        m_pSerializer->singleElementNS( XML_w, XML_color, xAttrList );
    }

    if ( m_pEastAsianLayoutAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pEastAsianLayoutAttrList );
        m_pEastAsianLayoutAttrList.clear();
        m_pSerializer->singleElementNS( XML_w, XML_eastAsianLayout, xAttrList );
    }

    if ( m_pCharLangAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pCharLangAttrList );
        m_pCharLangAttrList.clear();
        m_pSerializer->singleElementNS( XML_w, XML_lang, xAttrList );
    }

    if (!m_aTextEffectsGrabBag.empty())
    {
        for (size_t i = 0; i < m_aTextEffectsGrabBag.size(); ++i)
        {
            boost::optional<sal_Int32> aElementId = lclGetElementIdForName(m_aTextEffectsGrabBag[i].Name);
            if(aElementId)
            {
                uno::Sequence<beans::PropertyValue> aGrabBagSeq;
                m_aTextEffectsGrabBag[i].Value >>= aGrabBagSeq;
                lclProcessRecursiveGrabBag(*aElementId, aGrabBagSeq, m_pSerializer);
            }
        }
        m_aTextEffectsGrabBag.clear();
    }
}

void DocxAttributeOutput::EndRunProperties( const SwRedlineData* pRedlineData )
{
    // Call the 'Redline' function. This will add redline (change-tracking) information that regards to run properties.
    // This includes changes like 'Bold', 'Underline', 'Strikethrough' etc.
    Redline( pRedlineData );

    WriteCollectedRunProperties();

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_InitCollectedRunProperties);

    m_pSerializer->endElementNS( XML_w, XML_rPr );

    // write footnotes/endnotes if we have any
    FootnoteEndnoteReference();

    WritePostponedGraphic();

    WritePostponedDiagram();
    //We need to write w:drawing tag after the w:rPr.
    WritePostponedChart();

    //We need to write w:pict tag after the w:rPr.
    WritePostponedVMLDrawing();
    WritePostponedDMLDrawing();

    WritePostponedOLE();

    // merge the properties _before_ the run text (strictly speaking, just
    // after the start of the run)
    m_pSerializer->mergeTopMarks(Tag_StartRunProperties, sax_fastparser::MergeMarks::PREPEND);
}

void DocxAttributeOutput::GetSdtEndBefore(const SdrObject* pSdrObj)
{
    if (pSdrObj)
    {
        uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY_THROW);
        if( xShape.is() )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
            if( xPropSet.is() )
            {
                xPropSetInfo = xPropSet->getPropertySetInfo();
                uno::Sequence< beans::PropertyValue > aGrabBag;
                if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("FrameInteropGrabBag"))
                {
                    xPropSet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                }
                else if(xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("InteropGrabBag"))
                {
                    xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                }

                for (sal_Int32 nProp=0; nProp < aGrabBag.getLength(); ++nProp)
                {
                    if ("SdtEndBefore" == aGrabBag[nProp].Name && m_bStartedCharSdt && !m_bEndCharSdt)
                    {
                        aGrabBag[nProp].Value >>= m_bEndCharSdt;
                        break;
                    }
                }
            }
        }
    }
}

void DocxAttributeOutput::WritePostponedGraphic()
{
    for( std::list< PostponedGraphic >::const_iterator it = m_pPostponedGraphic->begin();
         it != m_pPostponedGraphic->end();
         ++it )
        FlyFrameGraphic( it->grfNode, it->size, it->mOLEFrameFormat, it->mOLENode, it->pSdrObj );
    m_pPostponedGraphic.reset(nullptr);
}

void DocxAttributeOutput::WritePostponedDiagram()
{
    for( std::list< PostponedDiagram >::const_iterator it = m_pPostponedDiagrams->begin();
         it != m_pPostponedDiagrams->end();
         ++it )
        m_rExport.SdrExporter().writeDiagram( it->object, *(it->frame), m_anchorId++ );
    m_pPostponedDiagrams.reset(nullptr);
}

void DocxAttributeOutput::FootnoteEndnoteRefTag()
{
    if( m_footnoteEndnoteRefTag == 0 )
        return;
    m_pSerializer->singleElementNS( XML_w, m_footnoteEndnoteRefTag, FSEND );
    m_footnoteEndnoteRefTag = 0;
}

/** Output sal_Unicode* as a run text (<t>the text</t>).

    When bMove is true, update rBegin to point _after_ the end of the text +
    1, meaning that it skips one character after the text.  This is to make
    the switch in DocxAttributeOutput::RunText() nicer ;-)
 */
static bool impl_WriteRunText( FSHelperPtr pSerializer, sal_Int32 nTextToken,
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
        pSerializer->startElementNS( XML_w, nTextToken, FSNS( XML_xml, XML_space ), "preserve", FSEND );
    }
    else
        pSerializer->startElementNS( XML_w, nTextToken, FSEND );

    pSerializer->writeEscaped( OUString( pBegin, pEnd - pBegin ) );

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

    // the text run is usually XML_t, with the exception of the deleted text
    sal_Int32 nTextToken = XML_t;
    if ( m_pRedlineData && m_pRedlineData->GetType() == nsRedlineType_t::REDLINE_DELETE )
        nTextToken = XML_delText;

    sal_Unicode prevUnicode = *pBegin;

    for ( const sal_Unicode *pIt = pBegin; pIt < pEnd; ++pIt )
    {
        switch ( *pIt )
        {
            case 0x09: // tab
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_tab, FSEND );
                prevUnicode = *pIt;
                break;
            case 0x0b: // line break
                {
                    if (impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt ) || (prevUnicode == *pIt))
                    {
                        m_pSerializer->singleElementNS( XML_w, XML_br, FSEND );
                        prevUnicode = *pIt;
                    }
                }
                break;
            case 0x1E: //non-breaking hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_noBreakHyphen, FSEND );
                prevUnicode = *pIt;
                break;
            case 0x1F: //soft (on demand) hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_softHyphen, FSEND );
                prevUnicode = *pIt;
                break;
            default:
                if ( *pIt < 0x0020 ) // filter out the control codes
                {
                    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                    OSL_TRACE( "Ignored control code %x in a text run.", *pIt );
                }
                prevUnicode = *pIt;
                break;
        }
    }

    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pEnd, false );
}

void DocxAttributeOutput::RawText(const OUString& /*rText*/, rtl_TextEncoding /*eCharSet*/)
{
    OSL_TRACE("TODO DocxAttributeOutput::RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet )" );
}

void DocxAttributeOutput::StartRuby( const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby )
{
    OSL_TRACE("TODO DocxAttributeOutput::StartRuby( const SwTextNode& rNode, const SwFormatRuby& rRuby )" );
    EndRun(); // end run before starting ruby to avoid nested runs, and overlap
    assert(!m_closeHyperlinkInThisRun); // check that no hyperlink overlaps ruby
    assert(!m_closeHyperlinkInPreviousRun);
    m_pSerializer->startElementNS( XML_w, XML_ruby, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_rubyPr, FSEND );
    // hps
    // hpsBaseText
    // hpsRaise
    // lid
    lang::Locale aLocale( SwBreakIt::Get()->GetLocale(
                rNode.GetLang( nPos ) ) );
    OUString sLang( LanguageTag::convertToBcp47( aLocale) );
    m_pSerializer->singleElementNS( XML_w, XML_lid,
            FSNS( XML_w, XML_val ),
            OUStringToOString( sLang, RTL_TEXTENCODING_UTF8 ).getStr( ), FSEND );

    OString sAlign ( "center" );
    switch ( rRuby.GetAdjustment( ) )
    {
        case 0:
            sAlign = OString( "left" );
            break;
        case 1:
            // Defaults to center
            break;
        case 2:
            sAlign = OString( "right" );
            break;
        case 3:
            sAlign = OString( "distributeLetter" );
            break;
        case 4:
            sAlign = OString( "distributeSpace" );
            break;
        default:
            break;
    }
    m_pSerializer->singleElementNS( XML_w, XML_rubyAlign,
            FSNS( XML_w, XML_val ), sAlign.getStr(), FSEND );
    m_pSerializer->endElementNS( XML_w, XML_rubyPr );

    m_pSerializer->startElementNS( XML_w, XML_rt, FSEND );
    StartRun( nullptr );
    StartRunProperties( );
    SwWW8AttrIter aAttrIt( m_rExport, rNode );
    aAttrIt.OutAttr( nPos, true );

    sal_uInt16 nStyle = m_rExport.GetId( rRuby.GetTextRuby()->GetCharFormat() );
    OString aStyleId(m_rExport.m_pStyles->GetStyleId(nStyle));
    m_pSerializer->singleElementNS( XML_w, XML_rStyle,
            FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );

    EndRunProperties( nullptr );
    RunText( rRuby.GetText( ) );
    EndRun( );
    m_pSerializer->endElementNS( XML_w, XML_rt );

    m_pSerializer->startElementNS( XML_w, XML_rubyBase, FSEND );
    StartRun( nullptr );
}

void DocxAttributeOutput::EndRuby()
{
    OSL_TRACE( "TODO DocxAttributeOutput::EndRuby()" );
    EndRun( );
    m_pSerializer->endElementNS( XML_w, XML_rubyBase );
    m_pSerializer->endElementNS( XML_w, XML_ruby );
    StartRun(nullptr); // open Run again so OutputTextNode loop can close it
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
                        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink",
                        sUrl, true ), RTL_TEXTENCODING_UTF8 );

            m_pHyperlinkAttrList->add( FSNS( XML_r, XML_id), sId.getStr());
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
                    sal_uInt32 nIndex = sMark.copy(nPos + 1, sMark.getLength() - nPos - sizeof("|sequence")).toInt32();
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
            m_pHyperlinkAttrList->add( FSNS( XML_w, XML_anchor ),
                    OUStringToOString( sMark, RTL_TEXTENCODING_UTF8 ).getStr( ) );
        }

        OUString sTarget( rTarget );
        if ( !sTarget.isEmpty() )
        {
            OString soTarget = OUStringToOString( sTarget, RTL_TEXTENCODING_UTF8 );
            m_pHyperlinkAttrList->add(FSNS( XML_w, XML_tgtFrame ), soTarget.getStr());
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

void DocxAttributeOutput::FieldVanish( const OUString& rText, ww::eField eType )
{
    WriteField_Impl( nullptr, eType, rText, WRITEFIELD_ALL );
}

// The difference between 'Redline' and 'StartRedline'+'EndRedline' is that:
// 'Redline' is used for tracked changes of formatting information of a run like Bold, Underline. (the '<w:rPrChange>' is inside the 'run' node)
// 'StartRedline' is used to output tracked changes of run insertion and deletion (the run is inside the '<w:ins>' node)
void DocxAttributeOutput::Redline( const SwRedlineData* pRedlineData)
{
    if ( !pRedlineData )
        return;

    OString aId( OString::number( pRedlineData->GetSeqNo() ) );
    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aAuthor( rAuthor.toUtf8() );
    OString aDate( DateTimeToOString( pRedlineData->GetTimeStamp() ) );

    switch( pRedlineData->GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:
        break;

    case nsRedlineType_t::REDLINE_DELETE:
        break;

    case nsRedlineType_t::REDLINE_FORMAT:
        m_pSerializer->startElementNS( XML_w, XML_rPrChange,
                FSNS( XML_w, XML_id ), aId.getStr(),
                FSNS( XML_w, XML_author ), aAuthor.getStr(),
                FSNS( XML_w, XML_date ), aDate.getStr(),
                FSEND );

        // Check if there is any extra data stored in the redline object
        if (pRedlineData->GetExtraData())
        {
            const SwRedlineExtraData* pExtraData = pRedlineData->GetExtraData();
            const SwRedlineExtraData_FormattingChanges* pFormattingChanges = dynamic_cast<const SwRedlineExtraData_FormattingChanges*>(pExtraData);

            // Check if the extra data is of type 'formatting changes'
            if (pFormattingChanges)
            {
                // Get the item set that holds all the changes properties
                const SfxItemSet *pChangesSet = pFormattingChanges->GetItemSet();
                if (pChangesSet)
                {
                    m_pSerializer->mark(Tag_Redline_1);

                    m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );

                    // The 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList' are used to hold information
                    // that should be collected by different properties in the core, and are all flushed together
                    // to the DOCX when the function 'WriteCollectedRunProperties' gets called.
                    // So we need to store the current status of these lists, so that we can revert back to them when
                    // we are done exporting the redline attributes.
                    uno::Reference<sax_fastparser::FastAttributeList> pFontsAttrList_Original(m_pFontsAttrList);
                    m_pFontsAttrList.clear();
                    uno::Reference<sax_fastparser::FastAttributeList> pEastAsianLayoutAttrList_Original(m_pEastAsianLayoutAttrList);
                    m_pEastAsianLayoutAttrList.clear();
                    uno::Reference<sax_fastparser::FastAttributeList> pCharLangAttrList_Original(m_pCharLangAttrList);
                    m_pCharLangAttrList.clear();

                    // Output the redline item set
                    m_rExport.OutputItemSet( *pChangesSet, false, true, i18n::ScriptType::LATIN, m_rExport.m_bExportModeRTF );

                    // Write the collected run properties that are stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
                    WriteCollectedRunProperties();

                    // Revert back the original values that were stored in 'm_pFontsAttrList', 'm_pEastAsianLayoutAttrList', 'm_pCharLangAttrList'
                    m_pFontsAttrList = pFontsAttrList_Original;
                    m_pEastAsianLayoutAttrList = pEastAsianLayoutAttrList_Original;
                    m_pCharLangAttrList = pCharLangAttrList_Original;

                    m_pSerializer->endElementNS( XML_w, XML_rPr );

                    m_pSerializer->mergeTopMarks(Tag_Redline_1, sax_fastparser::MergeMarks::PREPEND);
                }
            }
        }
        m_pSerializer->endElementNS( XML_w, XML_rPrChange );
        break;

    case nsRedlineType_t::REDLINE_PARAGRAPH_FORMAT:
        m_pSerializer->startElementNS( XML_w, XML_pPrChange,
                FSNS( XML_w, XML_id ), aId.getStr(),
                FSNS( XML_w, XML_author ), aAuthor.getStr(),
                FSNS( XML_w, XML_date ), aDate.getStr(),
                FSEND );

        // Check if there is any extra data stored in the redline object
        if (pRedlineData->GetExtraData())
        {
            const SwRedlineExtraData* pExtraData = pRedlineData->GetExtraData();
            const SwRedlineExtraData_FormattingChanges* pFormattingChanges = dynamic_cast<const SwRedlineExtraData_FormattingChanges*>(pExtraData);

            // Check if the extra data is of type 'formatting changes'
            if (pFormattingChanges)
            {
                // Get the item set that holds all the changes properties
                const SfxItemSet *pChangesSet = pFormattingChanges->GetItemSet();
                if (pChangesSet)
                {
                    m_pSerializer->mark(Tag_Redline_2);

                    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );

                    // The 'm_rExport.SdrExporter().getFlyAttrList()', 'm_pParagraphSpacingAttrList' are used to hold information
                    // that should be collected by different properties in the core, and are all flushed together
                    // to the DOCX when the function 'WriteCollectedParagraphProperties' gets called.
                    // So we need to store the current status of these lists, so that we can revert back to them when
                    // we are done exporting the redline attributes.
                    uno::Reference<sax_fastparser::FastAttributeList> pFlyAttrList_Original(m_rExport.SdrExporter().getFlyAttrList());
                    m_rExport.SdrExporter().getFlyAttrList().clear();
                    uno::Reference<sax_fastparser::FastAttributeList> pParagraphSpacingAttrList_Original(m_pParagraphSpacingAttrList);
                    m_pParagraphSpacingAttrList.clear();

                    // Output the redline item set
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
        SAL_WARN("sw.ww8", "Unhandled redline type for export " << pRedlineData->GetType());
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

    // FIXME check if it's necessary to travel over the Next()'s in pRedlineData

    OString aId( OString::number( m_nRedlineId++ ) );

    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );

    OString aDate( DateTimeToOString( pRedlineData->GetTimeStamp() ) );

    switch ( pRedlineData->GetType() )
    {
        case nsRedlineType_t::REDLINE_INSERT:
            m_pSerializer->startElementNS( XML_w, XML_ins,
                    FSNS( XML_w, XML_id ), aId.getStr(),
                    FSNS( XML_w, XML_author ), aAuthor.getStr(),
                    FSNS( XML_w, XML_date ), aDate.getStr(),
                    FSEND );
            break;

        case nsRedlineType_t::REDLINE_DELETE:
            m_pSerializer->startElementNS( XML_w, XML_del,
                    FSNS( XML_w, XML_id ), aId.getStr(),
                    FSNS( XML_w, XML_author ), aAuthor.getStr(),
                    FSNS( XML_w, XML_date ), aDate.getStr(),
                    FSEND );
            break;

        case nsRedlineType_t::REDLINE_FORMAT:
            OSL_TRACE( "TODO DocxAttributeOutput::StartRedline()" );
        default:
            break;
    }
}

void DocxAttributeOutput::EndRedline( const SwRedlineData * pRedlineData )
{
    if ( !pRedlineData )
        return;

    switch ( pRedlineData->GetType() )
    {
        case nsRedlineType_t::REDLINE_INSERT:
            m_pSerializer->endElementNS( XML_w, XML_ins );
            break;

        case nsRedlineType_t::REDLINE_DELETE:
            m_pSerializer->endElementNS( XML_w, XML_del );
            break;

        case nsRedlineType_t::REDLINE_FORMAT:
            OSL_TRACE( "TODO DocxAttributeOutput::EndRedline()" );
            break;
        default:
            break;
    }
}

void DocxAttributeOutput::FormatDrop( const SwTextNode& /*rNode*/, const SwFormatDrop& /*rSwFormatDrop*/, sal_uInt16 /*nStyle*/, ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/, ww8::WW8TableNodeInfoInner::Pointer_t )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FormatDrop( const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle )" );
}

void DocxAttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OString aStyleId(m_rExport.m_pStyles->GetStyleId(nStyle));

    m_pSerializer->singleElementNS( XML_w, XML_pStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

static void impl_borderLine( FSHelperPtr pSerializer, sal_Int32 elementToken, const SvxBorderLine* pBorderLine, sal_uInt16 nDist,
                             bool bWriteShadow = false, const table::BorderLine2* rStyleProps = nullptr )
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
            case table::BorderLineStyle::SOLID:
                pVal = "single";
                break;
            case table::BorderLineStyle::DOTTED:
                pVal = "dotted";
                break;
            case table::BorderLineStyle::DASHED:
                pVal = "dashed";
                break;
            case table::BorderLineStyle::DOUBLE:
                pVal = "double";
                break;
            case table::BorderLineStyle::THINTHICK_SMALLGAP:
                pVal = "thinThickSmallGap";
                break;
            case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
                pVal = "thinThickMediumGap";
                break;
            case table::BorderLineStyle::THINTHICK_LARGEGAP:
                pVal = "thinThickLargeGap";
                break;
            case table::BorderLineStyle::THICKTHIN_SMALLGAP:
                pVal = "thickThinSmallGap";
                break;
            case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
                pVal = "thickThinMediumGap";
                break;
            case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                pVal = "thickThinLargeGap";
                break;
            case table::BorderLineStyle::EMBOSSED:
                pVal = "threeDEmboss";
                break;
            case table::BorderLineStyle::ENGRAVED:
                pVal = "threeDEngrave";
                break;
            case table::BorderLineStyle::OUTSET:
                pVal = "outset";
                break;
            case table::BorderLineStyle::INSET:
                pVal = "inset";
                break;
            case table::BorderLineStyle::FINE_DASHED:
                pVal = "dashSmallGap";
                break;
            case table::BorderLineStyle::NONE:
            default:
                break;
        }
    }
    else if( rStyleProps == nullptr )
        // no line, and no line set by the style either:
        // there is no need to write the property
        return;

    // compare the properties with the theme properties before writing them:
    // if they are equal, it means that they were style-defined and there is
    // no need to write them.
    if( rStyleProps != nullptr && pBorderLine && !pBorderLine->isEmpty() &&
            pBorderLine->GetBorderLineStyle() == rStyleProps->LineStyle &&
            pBorderLine->GetColor() == rStyleProps->Color &&
            pBorderLine->GetWidth() == convertMm100ToTwip( rStyleProps->LineWidth ) )
        return;

    FastAttributeList* pAttr = FastSerializerHelper::createAttrList();
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
        pAttr->add( FSNS( XML_w, XML_space ), OString::number( nDist / 20 ) );

        // Get the color code as an RRGGBB hex value
        OString sColor( msfilter::util::ConvertColor( pBorderLine->GetColor( ) ) );
        pAttr->add( FSNS( XML_w, XML_color ), sColor );
    }

    if (bWriteShadow)
    {
        // Set the shadow value
        pAttr->add( FSNS( XML_w, XML_shadow ), "1" );
    }

    XFastAttributeListRef xAttrs( pAttr );
    pSerializer->singleElementNS( XML_w, elementToken, xAttrs );
}

static OutputBorderOptions lcl_getTableDefaultBorderOptions(bool bEcma)
{
    OutputBorderOptions rOptions;

    rOptions.tag = XML_tblBorders;
    rOptions.bUseStartEnd = !bEcma;
    rOptions.bWriteTag = true;
    rOptions.bWriteInsideHV = true;
    rOptions.bWriteDistance = false;
    rOptions.aShadowLocation = SVX_SHADOW_NONE;
    rOptions.bCheckDistanceSize = false;

    return rOptions;
}

static OutputBorderOptions lcl_getTableCellBorderOptions(bool bEcma)
{
    OutputBorderOptions rOptions;

    rOptions.tag = XML_tcBorders;
    rOptions.bUseStartEnd = !bEcma;
    rOptions.bWriteTag = true;
    rOptions.bWriteInsideHV = true;
    rOptions.bWriteDistance = false;
    rOptions.aShadowLocation = SVX_SHADOW_NONE;
    rOptions.bCheckDistanceSize = false;

    return rOptions;
}

static OutputBorderOptions lcl_getBoxBorderOptions()
{
    OutputBorderOptions rOptions;

    rOptions.tag = XML_pBdr;
    rOptions.bUseStartEnd = false;
    rOptions.bWriteTag = false;
    rOptions.bWriteInsideHV = false;
    rOptions.bWriteDistance = true;
    rOptions.aShadowLocation = SVX_SHADOW_NONE;
    rOptions.bCheckDistanceSize = false;

    return rOptions;
}

static bool boxHasLineLargerThan31(const SvxBoxItem& rBox)
{
    return  (
                ( rBox.GetDistance( SvxBoxItemLine::TOP ) / 20 ) > 31 ||
                ( rBox.GetDistance( SvxBoxItemLine::LEFT ) / 20 ) > 31 ||
                ( rBox.GetDistance( SvxBoxItemLine::BOTTOM ) / 20 ) > 31 ||
                ( rBox.GetDistance( SvxBoxItemLine::RIGHT ) / 20 ) > 31
            );
}

static void impl_borders( FSHelperPtr pSerializer, const SvxBoxItem& rBox, const OutputBorderOptions& rOptions, PageMargins* pageMargins,
                          std::map<SvxBoxItemLine, css::table::BorderLine2> &rTableStyleConf )
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

    bool bExportDistanceFromPageEdge = false;
    if ( rOptions.bCheckDistanceSize && boxHasLineLargerThan31(rBox) )
    {
        // The distance is larger than '31'. This cannot be exported as 'distance from text'.
        // Instead - it should be exported as 'distance from page edge'.
        // This is based on http://wiki.openoffice.org/wiki/Writer/MSInteroperability/PageBorder
        // Specifically 'export case #2'
        bExportDistanceFromPageEdge = true;
    }

    bool bWriteInsideH = false;
    bool bWriteInsideV = false;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        const table::BorderLine2 *aStyleProps = nullptr;
        if( rTableStyleConf.find( *pBrd ) != rTableStyleConf.end() )
            aStyleProps = &rTableStyleConf[ *pBrd ];

        if (!tagWritten && rOptions.bWriteTag)
        {
            pSerializer->startElementNS( XML_w, rOptions.tag, FSEND );
            tagWritten = true;
        }

        bool bWriteShadow = false;
        if (rOptions.aShadowLocation == SVX_SHADOW_NONE)
        {
            // The border has no shadow
        }
        else if (rOptions.aShadowLocation == SVX_SHADOW_BOTTOMRIGHT)
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
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPLEFT     || rOptions.aShadowLocation == SVX_SHADOW_TOPRIGHT      )    &&  *pBrd == SvxBoxItemLine::TOP   )  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPLEFT     || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMLEFT    )    &&  *pBrd == SvxBoxItemLine::LEFT  )  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_BOTTOMLEFT  || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMRIGHT   )    &&  *pBrd == SvxBoxItemLine::BOTTOM)  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPRIGHT    || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMRIGHT   )    &&  *pBrd == SvxBoxItemLine::RIGHT )
                )
            {
                bWriteShadow = true;
            }
        }

        sal_uInt16 nDist = 0;
        if (rOptions.bWriteDistance)
        {
            if (bExportDistanceFromPageEdge)
            {
                // Export 'Distance from Page Edge'
                if ( *pBrd == SvxBoxItemLine::TOP)
                    nDist = pageMargins->nPageMarginTop - rBox.GetDistance( *pBrd );
                else if ( *pBrd == SvxBoxItemLine::LEFT)
                    nDist = pageMargins->nPageMarginLeft - rBox.GetDistance( *pBrd );
                else if ( *pBrd == SvxBoxItemLine::BOTTOM)
                    nDist = pageMargins->nPageMarginBottom - rBox.GetDistance( *pBrd );
                else if ( *pBrd == SvxBoxItemLine::RIGHT)
                    nDist = pageMargins->nPageMarginRight - rBox.GetDistance( *pBrd );
            }
            else
            {
                // Export 'Distance from text'
                nDist = rBox.GetDistance( *pBrd );
            }
        }

        impl_borderLine( pSerializer, aXmlElements[i], pLn, nDist, bWriteShadow, aStyleProps );

        // When exporting default borders, we need to export these 2 attr
        if ( rOptions.bWriteInsideHV) {
            if ( i == 2 )
                bWriteInsideH = true;
            else if ( i == 3 )
                bWriteInsideV = true;
        }
    }
    if (bWriteInsideH)
    {
        const table::BorderLine2 *aStyleProps = nullptr;
        if( rTableStyleConf.find( SvxBoxItemLine::BOTTOM ) != rTableStyleConf.end() )
            aStyleProps = &rTableStyleConf[ SvxBoxItemLine::BOTTOM ];
        impl_borderLine( pSerializer, XML_insideH, rBox.GetLine(SvxBoxItemLine::BOTTOM), 0, false, aStyleProps );
    }
    if (bWriteInsideV)
    {
        const table::BorderLine2 *aStyleProps = nullptr;
        if( rTableStyleConf.find( SvxBoxItemLine::RIGHT ) != rTableStyleConf.end() )
            aStyleProps = &rTableStyleConf[ SvxBoxItemLine::RIGHT ];
        impl_borderLine( pSerializer, XML_insideV, rBox.GetLine(SvxBoxItemLine::RIGHT), 0, false, aStyleProps );
    }
    if (tagWritten && rOptions.bWriteTag) {
        pSerializer->endElementNS( XML_w, rOptions.tag );
    }
}

static void impl_cellMargins( FSHelperPtr pSerializer, const SvxBoxItem& rBox, sal_Int32 tag, bool bUseStartEnd = false, const SvxBoxItem* pDefaultMargins = nullptr)
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

        if ( aBorders[i] == SvxBoxItemLine::LEFT ) {
            // Office's cell margin is measured from the right of the border.
            // While LO's cell spacing is measured from the center of the border.
            // So we add half left-border width to tblIndent value
            const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
            if (pLn)
                nDist -= pLn->GetWidth() * 0.5;
        }

        if (pDefaultMargins)
        {
            // Skip output if cell margin == table default margin
            if (sal_Int32( pDefaultMargins->GetDistance( *pBrd ) ) == nDist)
                continue;
        }

        if (!tagWritten) {
            pSerializer->startElementNS( XML_w, tag, FSEND );
            tagWritten = true;
        }
        pSerializer->singleElementNS( XML_w, aXmlElements[i],
               FSNS( XML_w, XML_w ), OString::number( nDist ).getStr( ),
               FSNS( XML_w, XML_type ), "dxa",
               FSEND );
    }
    if (tagWritten) {
        pSerializer->endElementNS( XML_w, tag );
    }
}

void DocxAttributeOutput::TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow )
{
    m_pSerializer->startElementNS( XML_w, XML_tcPr, FSEND );

    const SwTableBox *pTableBox = pTableTextNodeInfoInner->getTableBox( );

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Output any table cell redlines if there are any attached to this specific cell
    TableCellRedline( pTableTextNodeInfoInner );

    // Cell preferred width
    SwTwips nWidth = GetGridCols( pTableTextNodeInfoInner )->at( nCell );
    if ( nCell )
        nWidth = nWidth - GetGridCols( pTableTextNodeInfoInner )->at( nCell - 1 );
    m_pSerializer->singleElementNS( XML_w, XML_tcW,
           FSNS( XML_w, XML_w ), OString::number( nWidth ).getStr( ),
           FSNS( XML_w, XML_type ), "dxa",
           FSEND );

    // Horizontal spans
    const SwWriteTableRows& rRows = m_xTableWrt->GetRows( );
    SwWriteTableRow *pRow = rRows[ nRow ];
    const SwWriteTableCells& rTableCells =  pRow->GetCells();
    if (nCell < rTableCells.size() )
    {
        const SwWriteTableCell& rCell = *rTableCells[nCell];
        const sal_uInt16 nColSpan = rCell.GetColSpan();
        if ( nColSpan > 1 )
            m_pSerializer->singleElementNS( XML_w, XML_gridSpan,
                    FSNS( XML_w, XML_val ), OString::number( nColSpan ).getStr(),
                    FSEND );
    }

    // Vertical merges
    ww8::RowSpansPtr xRowSpans = pTableTextNodeInfoInner->getRowSpansOfRow();
    sal_Int32 vSpan = (*xRowSpans)[nCell];
    if ( vSpan > 1 )
    {
        m_pSerializer->singleElementNS( XML_w, XML_vMerge,
                FSNS( XML_w, XML_val ), "restart",
                FSEND );
    }
    else if ( vSpan < 0 )
    {
        m_pSerializer->singleElementNS( XML_w, XML_vMerge,
                FSNS( XML_w, XML_val ), "continue",
                FSEND );
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
        impl_borders( m_pSerializer, rBox, lcl_getTableCellBorderOptions(bEcma), nullptr, m_aTableStyleConf );
    }

    TableBackgrounds( pTableTextNodeInfoInner );

    {
        // Cell margins
        impl_cellMargins( m_pSerializer, rBox, XML_tcMar, !bEcma, &rDefaultBox );
    }

    TableVerticalCell( pTableTextNodeInfoInner );

    m_pSerializer->endElementNS( XML_w, XML_tcPr );
}

void DocxAttributeOutput::InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    if (m_xTableWrt && pTable == m_xTableWrt->GetTable())
        return;

    long nPageSize = 0;
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

void DocxAttributeOutput::StartTable( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    // In case any paragraph SDT's are open, close them here.
    EndParaSdtBlock();

    m_pSerializer->startElementNS( XML_w, XML_tbl, FSEND );

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
    if( 0 != tableFirstCells.size() )
        m_tableReference->m_bTableCellOpen = true;

    // Cleans the table helper
    m_xTableWrt.reset(nullptr);

    m_aTableStyleConf.clear();
}

void DocxAttributeOutput::StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    m_pSerializer->startElementNS( XML_w, XML_tr, FSEND );

    // Output the row properties
    m_pSerializer->startElementNS( XML_w, XML_trPr, FSEND );

    // Header row: tblHeader
    const SwTable *pTable = pTableTextNodeInfoInner->getTable( );
    if ( pTable->GetRowsToRepeat( ) > pTableTextNodeInfoInner->getRow( ) )
        m_pSerializer->singleElementNS( XML_w, XML_tblHeader,
               FSNS( XML_w, XML_val ), "true",
               FSEND );

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

void DocxAttributeOutput::StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow )
{
    lastOpenCell.back() = nCell;

    InitTableHelper( pTableTextNodeInfoInner );

    m_pSerializer->startElementNS( XML_w, XML_tc, FSEND );

    // Write the cell properties here
    TableCellProperties( pTableTextNodeInfoInner, nCell, nRow );

    m_tableReference->m_bTableCellOpen = true;
}

void DocxAttributeOutput::EndTableCell(ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/, sal_uInt32 nCell, sal_uInt32 /*nRow*/)
{
    lastClosedCell.back() = nCell;
    lastOpenCell.back() = -1;

    if (m_tableReference->m_bTableCellParaSdtOpen)
        EndParaSdtBlock();

    m_pSerializer->endElementNS( XML_w, XML_tc );

    m_bBtLr = false;
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
OString lcl_padStartToLength(OString const & aString, sal_Int32 nLen, sal_Char cFill)
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

}

void DocxAttributeOutput::TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Write the table properties
    m_pSerializer->startElementNS( XML_w, XML_tblPr, FSEND );

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
    sal_Int32 len = sizeof ( aOrder ) / sizeof( sal_Int32 );
    uno::Sequence< sal_Int32 > aSeqOrder( len );
    for ( sal_Int32 i = 0; i < len; i++ )
        aSeqOrder[i] = aOrder[i];

    m_pSerializer->mark(Tag_TableDefinition, aSeqOrder);

    long nPageSize = 0;
    const char* widthType = "dxa";

    // If actual width of table is relative it should export is as "pct".`
    const SwTable *pTable = pTableTextNodeInfoInner->getTable();
    SwFrameFormat *pTableFormat = pTable->GetFrameFormat( );
    const SwFormatFrameSize &rSize = pTableFormat->GetFrameSize();
    int nWidthPercent = rSize.GetWidthPercent();
    uno::Reference<beans::XPropertySet> xPropertySet(SwXTextTables::GetObject(const_cast<SwTableFormat&>(*pTable->GetFrameFormat( ))),uno::UNO_QUERY);
    bool isWidthRelative = false;
    xPropertySet->getPropertyValue("IsWidthRelative") >>= isWidthRelative;

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
            FSNS( XML_w, XML_w ), OString::number( nPageSize ).getStr( ),
            FSNS( XML_w, XML_type ), widthType,
            FSEND );

    // Look for the table style property in the table grab bag
    std::map<OUString, css::uno::Any> aGrabBag =
            pTableFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    // We should clear the TableStyle map. In case of Table inside multiple tables it contains the
    // table border style of the previous table.
    if (! m_aTableStyleConf.empty())
        m_aTableStyleConf.clear();

    // Extract properties from grab bag
    std::map<OUString, css::uno::Any>::iterator aGrabBagElement;
    for( aGrabBagElement = aGrabBag.begin(); aGrabBagElement != aGrabBag.end(); ++aGrabBagElement )
    {
        if( aGrabBagElement->first == "TableStyleName")
        {
            OString sStyleName = OUStringToOString( aGrabBagElement->second.get<OUString>(), RTL_TEXTENCODING_UTF8 );
            m_pSerializer->singleElementNS( XML_w, XML_tblStyle,
                    FSNS( XML_w, XML_val ), sStyleName.getStr(),
                    FSEND );
        }
        else if( aGrabBagElement->first == "TableStyleTopBorder" )
            m_aTableStyleConf[ SvxBoxItemLine::TOP ] = aGrabBagElement->second.get<table::BorderLine2>();
        else if( aGrabBagElement->first == "TableStyleBottomBorder" )
            m_aTableStyleConf[ SvxBoxItemLine::BOTTOM ] = aGrabBagElement->second.get<table::BorderLine2>();
        else if( aGrabBagElement->first == "TableStyleLeftBorder" )
            m_aTableStyleConf[ SvxBoxItemLine::LEFT ] = aGrabBagElement->second.get<table::BorderLine2>();
        else if( aGrabBagElement->first == "TableStyleRightBorder" )
            m_aTableStyleConf[ SvxBoxItemLine::RIGHT ] = aGrabBagElement->second.get<table::BorderLine2>();
        else if (aGrabBagElement->first == "TableStyleLook")
        {
            FastAttributeList* pAttributeList = FastSerializerHelper::createAttrList();
            uno::Sequence<beans::PropertyValue> aAttributeList = aGrabBagElement->second.get< uno::Sequence<beans::PropertyValue> >();

            for (sal_Int32 i = 0; i < aAttributeList.getLength(); ++i)
            {
                if (aAttributeList[i].Name == "val")
                    pAttributeList->add(FSNS(XML_w, XML_val), lcl_padStartToLength(OString::number(aAttributeList[i].Value.get<sal_Int32>(), 16), 4, '0'));
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

                    if (sal_Int32 nToken = DocxStringGetToken(aTokens, aAttributeList[i].Name))
                        pAttributeList->add(FSNS(XML_w, nToken), (aAttributeList[i].Value.get<sal_Int32>() ? "1" : "0"));
                }
            }

            XFastAttributeListRef xAttributeList(pAttributeList);
            m_pSerializer->singleElementNS(XML_w, XML_tblLook, xAttributeList);
        }
        else if (aGrabBagElement->first == "TablePosition" )
        {
            FastAttributeList *attrListTablePos = FastSerializerHelper::createAttrList( );
            uno::Sequence<beans::PropertyValue> aTablePosition = aGrabBagElement->second.get<uno::Sequence<beans::PropertyValue> >();
            for (sal_Int32 i = 0; i < aTablePosition.getLength(); ++i)
            {
                if (aTablePosition[i].Name == "vertAnchor" && !aTablePosition[i].Value.get<OUString>().isEmpty())
                {
                    OString strTemp = OUStringToOString(aTablePosition[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                    attrListTablePos->add( FSNS( XML_w, XML_vertAnchor ), strTemp.getStr() );
                }
                else if (aTablePosition[i].Name == "tblpYSpec" && !aTablePosition[i].Value.get<OUString>().isEmpty())
                {
                    OString strTemp = OUStringToOString(aTablePosition[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                    attrListTablePos->add( FSNS( XML_w, XML_tblpYSpec ), strTemp.getStr() );
                }
                else if (aTablePosition[i].Name == "horzAnchor" && !aTablePosition[i].Value.get<OUString>().isEmpty())
                {
                    OString strTemp = OUStringToOString(aTablePosition[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                    attrListTablePos->add( FSNS( XML_w, XML_horzAnchor ), strTemp.getStr() );
                }
                else if (aTablePosition[i].Name == "tblpXSpec" && !aTablePosition[i].Value.get<OUString>().isEmpty())
                {
                    OString strTemp = OUStringToOString(aTablePosition[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);
                    attrListTablePos->add( FSNS( XML_w, XML_tblpXSpec ), strTemp.getStr() );
                }
                else if (aTablePosition[i].Name == "bottomFromText")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_bottomFromText ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
                else if (aTablePosition[i].Name == "leftFromText")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_leftFromText ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
                else if (aTablePosition[i].Name == "rightFromText")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_rightFromText ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
                else if (aTablePosition[i].Name == "topFromText")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_topFromText ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
                else if (aTablePosition[i].Name == "tblpX")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_tblpX ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
                else if (aTablePosition[i].Name == "tblpY")
                {
                    attrListTablePos->add( FSNS( XML_w, XML_tblpY ), OString::number( aTablePosition[i].Value.get<sal_Int32>() ) );
                }
            }

            XFastAttributeListRef xAttrListTablePosRef( attrListTablePos );

            m_pSerializer->singleElementNS( XML_w, XML_tblpPr, xAttrListTablePosRef);
            attrListTablePos = nullptr;
        }
        else
            SAL_WARN("sw.ww8", "DocxAttributeOutput::TableDefinition: unhandled property: " << aGrabBagElement->first);
    }

    // Output the table alignement
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
            nIndent = sal_Int32( pTableFormat->GetLRSpace( ).GetLeft( ) );
            // Table indentation has different meaning in Word, depending if the table is nested or not.
            // If nested, tblInd is added to parent table's left spacing and defines left edge position
            // If not nested, text position of left-most cell must be at absolute X = tblInd
            // so, table_spacing + table_spacing_to_content = tblInd
            if (m_tableReference->m_nTableDepth == 0)
            {
                const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
                const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();
                nIndent += sal_Int32( pFrameFormat->GetBox( ).GetDistance( SvxBoxItemLine::LEFT ) );
            }
            break;
        }
    }
    m_pSerializer->singleElementNS( XML_w, XML_jc,
            FSNS( XML_w, XML_val ), pJcVal,
            FSEND );

    // Output the table borders
    TableDefaultBorders( pTableTextNodeInfoInner );

    // Output the default cell margins
    TableDefaultCellMargins( pTableTextNodeInfoInner );

    TableBidi( pTableTextNodeInfoInner );

    // Table indent (need to get written even if == 0)
    m_pSerializer->singleElementNS( XML_w, XML_tblInd,
            FSNS( XML_w, XML_w ), OString::number( nIndent ).getStr( ),
            FSNS( XML_w, XML_type ), "dxa",
            FSEND );

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks(Tag_TableDefinition);

    m_pSerializer->endElementNS( XML_w, XML_tblPr );

    // Write the table grid infos
    m_pSerializer->startElementNS( XML_w, XML_tblGrid, FSEND );
    sal_Int32 nPrv = 0;
    ww8::WidthsPtr pColumnWidths = GetColumnWidths( pTableTextNodeInfoInner );
    for ( ww8::Widths::const_iterator it = pColumnWidths->begin(); it != pColumnWidths->end(); ++it )
    {
        sal_Int32 nWidth  =  sal_Int32( *it ) - nPrv;
        m_pSerializer->singleElementNS( XML_w, XML_gridCol,
               FSNS( XML_w, XML_w ), OString::number( nWidth ).getStr( ),
               FSEND );
        nPrv = sal_Int32( *it );
    }

    m_pSerializer->endElementNS( XML_w, XML_tblGrid );
}

void DocxAttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Don't write table defaults based on the top-left cell if we have a table style available.
    if (m_aTableStyleConf.empty())
    {
        // the defaults of the table are taken from the top-left cell
        impl_borders(m_pSerializer, pFrameFormat->GetBox(), lcl_getTableDefaultBorderOptions(bEcma), nullptr, m_aTableStyleConf);
    }
}

void DocxAttributeOutput::TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();
    const SvxBoxItem& rBox = pFrameFormat->GetBox( );
    const bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    impl_cellMargins(m_pSerializer, rBox, XML_tblCellMar, !bEcma);
}

void DocxAttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox *pTableBox = pTableTextNodeInfoInner->getTableBox( );
    const SwFrameFormat *pFormat = pTableBox->GetFrameFormat( );

    const SvxBrushItem *pColorProp = pFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    Color aColor = pColorProp ? pColorProp->GetColor() : COL_AUTO;
    OString sColor = msfilter::util::ConvertColor( aColor );

    std::map<OUString, css::uno::Any> aGrabBag =
            pFormat->GetAttrSet().GetItem<SfxGrabBagItem>(RES_FRMATR_GRABBAG)->GetGrabBag();

    OString sOriginalColor;
    std::map<OUString, css::uno::Any>::iterator aGrabBagElement = aGrabBag.find("originalColor");
    if( aGrabBagElement != aGrabBag.end() )
        sOriginalColor = OUStringToOString( aGrabBagElement->second.get<OUString>(), RTL_TEXTENCODING_UTF8 );

    if ( sOriginalColor != sColor )
    {
        // color changed by the user, or no grab bag: write sColor
        m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor.getStr( ),
                FSNS( XML_w, XML_val ), "clear",
                FSEND );
    }
    else
    {
        css::uno::Reference<sax_fastparser::FastAttributeList> pAttrList;

        for( aGrabBagElement = aGrabBag.begin(); aGrabBagElement != aGrabBag.end(); ++aGrabBagElement )
        {
            if (!aGrabBagElement->second.has<OUString>())
                continue;

            OString sValue = OUStringToOString( aGrabBagElement->second.get<OUString>(), RTL_TEXTENCODING_UTF8 );
            if( aGrabBagElement->first == "themeFill")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFill ), sValue.getStr() );
            else if( aGrabBagElement->first == "themeFillTint")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFillTint ), sValue.getStr() );
            else if( aGrabBagElement->first == "themeFillShade")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeFillShade ), sValue.getStr() );
            else if( aGrabBagElement->first == "fill" )
                AddToAttrList( pAttrList, FSNS( XML_w, XML_fill ), sValue.getStr() );
            else if( aGrabBagElement->first == "themeColor")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeColor ), sValue.getStr() );
            else if( aGrabBagElement->first == "themeTint")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeTint ), sValue.getStr() );
            else if( aGrabBagElement->first == "themeShade")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_themeShade ), sValue.getStr() );
            else if( aGrabBagElement->first == "color")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_color ), sValue.getStr() );
            else if( aGrabBagElement->first == "val")
                AddToAttrList( pAttrList, FSNS( XML_w, XML_val ), sValue.getStr() );
        }
        m_pSerializer->singleElementNS( XML_w, XML_shd, pAttrList );
    }
}

void DocxAttributeOutput::TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();

    // search next Redline
    const SwExtraRedlineTable& aExtraRedlineTable = m_rExport.m_pDoc->getIDocumentRedlineAccess().GetExtraRedlineTable();
    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
        const SwTableRowRedline* pTableRowRedline = dynamic_cast<const SwTableRowRedline*>(pExtraRedline);
        const SwTableLine *pRedTabLine = pTableRowRedline ? &pTableRowRedline->GetTableLine() : nullptr;
        if (pRedTabLine == pTabLine)
        {
            // Redline for this table row
            const SwRedlineData& aRedlineData = pTableRowRedline->GetRedlineData();
            sal_uInt16 nRedlineType = aRedlineData.GetType();
            switch (nRedlineType)
            {
                case nsRedlineType_t::REDLINE_TABLE_ROW_INSERT:
                case nsRedlineType_t::REDLINE_TABLE_ROW_DELETE:
                {
                    OString aId( OString::number( m_nRedlineId++ ) );
                    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( aRedlineData.GetAuthor() ) );
                    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );

                    OString aDate( DateTimeToOString( aRedlineData.GetTimeStamp() ) );

                    if (nRedlineType == nsRedlineType_t::REDLINE_TABLE_ROW_INSERT)
                        m_pSerializer->singleElementNS( XML_w, XML_ins,
                            FSNS( XML_w, XML_id ), aId.getStr(),
                            FSNS( XML_w, XML_author ), aAuthor.getStr(),
                            FSNS( XML_w, XML_date ), aDate.getStr(),
                            FSEND );
                    else if (nRedlineType == nsRedlineType_t::REDLINE_TABLE_ROW_DELETE)
                        m_pSerializer->singleElementNS( XML_w, XML_del,
                            FSNS( XML_w, XML_id ), aId.getStr(),
                            FSNS( XML_w, XML_author ), aAuthor.getStr(),
                            FSNS( XML_w, XML_date ), aDate.getStr(),
                            FSEND );
                }
                break;
            }
        }
    }
}

void DocxAttributeOutput::TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();

    // search next Redline
    const SwExtraRedlineTable& aExtraRedlineTable = m_rExport.m_pDoc->getIDocumentRedlineAccess().GetExtraRedlineTable();
    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < aExtraRedlineTable.GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = aExtraRedlineTable.GetRedline(nCurRedlinePos);
        const SwTableCellRedline* pTableCellRedline = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
        const SwTableBox *pRedTabBox = pTableCellRedline ? &pTableCellRedline->GetTableBox() : nullptr;
        if (pRedTabBox == pTabBox)
        {
            // Redline for this table cell
            const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
            sal_uInt16 nRedlineType = aRedlineData.GetType();
            switch (nRedlineType)
            {
                case nsRedlineType_t::REDLINE_TABLE_CELL_INSERT:
                case nsRedlineType_t::REDLINE_TABLE_CELL_DELETE:
                {
                    OString aId( OString::number( m_nRedlineId++ ) );
                    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( aRedlineData.GetAuthor() ) );
                    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );

                    OString aDate( DateTimeToOString( aRedlineData.GetTimeStamp() ) );

                    if (nRedlineType == nsRedlineType_t::REDLINE_TABLE_CELL_INSERT)
                        m_pSerializer->singleElementNS( XML_w, XML_cellIns,
                            FSNS( XML_w, XML_id ), aId.getStr(),
                            FSNS( XML_w, XML_author ), aAuthor.getStr(),
                            FSNS( XML_w, XML_date ), aDate.getStr(),
                            FSEND );
                    else if (nRedlineType == nsRedlineType_t::REDLINE_TABLE_CELL_DELETE)
                        m_pSerializer->singleElementNS( XML_w, XML_cellDel,
                            FSNS( XML_w, XML_id ), aId.getStr(),
                            FSNS( XML_w, XML_author ), aAuthor.getStr(),
                            FSNS( XML_w, XML_date ), aDate.getStr(),
                            FSEND );
                }
                break;
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
    if ( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
    {
        sal_Int32 nHeight = rLSz.GetHeight();
        const char *pRule = nullptr;

        switch ( rLSz.GetHeightSizeType() )
        {
            case ATT_FIX_SIZE: pRule = "exact"; break;
            case ATT_MIN_SIZE: pRule = "atLeast"; break;
            default:           break;
        }

        if ( pRule )
            m_pSerializer->singleElementNS( XML_w, XML_trHeight,
                    FSNS( XML_w, XML_val ), OString::number( nHeight ).getStr( ),
                    FSNS( XML_w, XML_hRule ), pRule,
                    FSEND );
    }
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
        m_pSerializer->singleElementNS( XML_w, XML_cantSplit,
                FSNS( XML_w, XML_val ), "true",
                FSEND );
}

void DocxAttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrameFormat * pFrameFormat = pTable->GetFrameFormat();

    if ( m_rExport.TrueFrameDirection( *pFrameFormat ) == FRMDIR_HORI_RIGHT_TOP )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bidiVisual,
                FSNS( XML_w, XML_val ), "true",
                FSEND );
    }
}

void DocxAttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat *pFrameFormat = pTabBox->GetFrameFormat( );

    if ( FRMDIR_VERT_TOP_RIGHT == m_rExport.TrueFrameDirection( *pFrameFormat ) )
        m_pSerializer->singleElementNS( XML_w, XML_textDirection,
               FSNS( XML_w, XML_val ), "tbRl",
               FSEND );
    else if ( FRMDIR_HORI_LEFT_TOP == m_rExport.TrueFrameDirection( *pFrameFormat ) )
    {
        // Undo the text direction mangling done by the btLr handler in writerfilter::dmapper::DomainMapperTableManager::sprm()
        const SwStartNode* pSttNd = pTabBox->GetSttNd();
        if (pSttNd)
        {
            SwPaM aPam(*pSttNd, 0);
            ++aPam.GetPoint()->nNode;
            if (aPam.GetPoint()->nNode.GetNode().IsTextNode())
            {
                const SwTextNode& rTextNode = static_cast<const SwTextNode&>(aPam.GetPoint()->nNode.GetNode());
                if( const SwAttrSet* pAttrSet = rTextNode.GetpSwAttrSet())
                {
                    const SvxCharRotateItem& rCharRotate = pAttrSet->GetCharRotate();
                    if (rCharRotate.GetValue() == 900)
                    {
                        m_pSerializer->singleElementNS( XML_w, XML_textDirection, FSNS( XML_w, XML_val ), "btLr", FSEND );
                        m_bBtLr = true;
                    }
                }
            }
        }
    }

    const SwWriteTableRows& rRows = m_xTableWrt->GetRows( );
    SwWriteTableRow *pRow = rRows[ pTableTextNodeInfoInner->getRow( ) ];
    sal_uInt32 nCell = pTableTextNodeInfoInner->getCell();
    const SwWriteTableCells& rTableCells =  pRow->GetCells();
    if (nCell < rTableCells.size() )
    {
        const SwWriteTableCell *const pCell = pRow->GetCells()[ nCell ].get();
        switch( pCell->GetVertOri())
        {
        case text::VertOrientation::TOP:
            break;
        case text::VertOrientation::CENTER:
            m_pSerializer->singleElementNS( XML_w, XML_vAlign,
            FSNS( XML_w, XML_val ), "center", FSEND );
            break;
        case text::VertOrientation::BOTTOM:
            m_pSerializer->singleElementNS( XML_w, XML_vAlign,
                    FSNS( XML_w, XML_val ), "bottom", FSEND );
            break;
        }
    }
}

void DocxAttributeOutput::TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner )
{
    // This is called when the nested table ends in a cell, and there's no
    // paragraph benhind that; so we must check for the ends of cell, rows,
    // tables
    // ['true' to write an empty paragraph, MS Word insists on that]
    FinishTableRowCell( pNodeInfoInner, true );
}

void DocxAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )" );
}

void DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )\n" );
}

void DocxAttributeOutput::TableRowEnd( sal_uInt32 /*nDepth*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableRowEnd( sal_uInt32 nDepth = 1 )" );
}

void DocxAttributeOutput::StartStyles()
{
    m_pSerializer->startElementNS( XML_w, XML_styles,
            FSNS( XML_xmlns, XML_w ),   "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSNS( XML_xmlns, XML_w14 ), "http://schemas.microsoft.com/office/word/2010/wordml",
            FSNS( XML_xmlns, XML_mc ),  "http://schemas.openxmlformats.org/markup-compatibility/2006",
            FSNS( XML_mc, XML_Ignorable ), "w14",
            FSEND );

    DocDefaults();
    LatentStyles();
}

sal_Int32 DocxStringGetToken(DocxStringTokenMap const * pMap, const OUString& rName)
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
    uno::Reference<beans::XPropertySet> xPropertySet(m_rExport.m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xPropertySet->getPropertyValue("InteropGrabBag") >>= aInteropGrabBag;
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    for (sal_Int32 i = 0; i < aInteropGrabBag.getLength(); ++i)
    {
        if (aInteropGrabBag[i].Name == "latentStyles")
        {
            aInteropGrabBag[i].Value >>= aLatentStyles;
            break;
        }
    }
    if (!aLatentStyles.getLength())
        return;

    // Extract default attributes first.
    sax_fastparser::FastAttributeList* pAttributeList = FastSerializerHelper::createAttrList();
    uno::Sequence<beans::PropertyValue> aLsdExceptions;
    for (sal_Int32 i = 0; i < aLatentStyles.getLength(); ++i)
    {
        if (sal_Int32 nToken = DocxStringGetToken(aDefaultTokens, aLatentStyles[i].Name))
            pAttributeList->add(FSNS(XML_w, nToken), OUStringToOString(aLatentStyles[i].Value.get<OUString>(), RTL_TEXTENCODING_UTF8));
        else if (aLatentStyles[i].Name == "lsdExceptions")
            aLatentStyles[i].Value >>= aLsdExceptions;
    }

    XFastAttributeListRef xAttributeList(pAttributeList);
    m_pSerializer->startElementNS(XML_w, XML_latentStyles, xAttributeList);
    pAttributeList = nullptr;

    // Then handle the exceptions.
    for (sal_Int32 i = 0; i < aLsdExceptions.getLength(); ++i)
    {
        pAttributeList = FastSerializerHelper::createAttrList();

        uno::Sequence<beans::PropertyValue> aAttributes;
        aLsdExceptions[i].Value >>= aAttributes;
        for (sal_Int32 j = 0; j < aAttributes.getLength(); ++j)
            if (sal_Int32 nToken = DocxStringGetToken(aExceptionTokens, aAttributes[j].Name))
                pAttributeList->add(FSNS(XML_w, nToken), OUStringToOString(aAttributes[j].Value.get<OUString>(), RTL_TEXTENCODING_UTF8));

        xAttributeList = pAttributeList;
        m_pSerializer->singleElementNS(XML_w, XML_lsdException, xAttributeList);
        pAttributeList = nullptr;
    }

    m_pSerializer->endElementNS(XML_w, XML_latentStyles);
}

namespace
{

/// Should the font size we have written out as a default one?
bool lcl_isDefaultFontSize(const SvxFontHeightItem& rFontHeight, SwDoc* pDoc)
{
    bool bRet = rFontHeight.GetHeight() != 200; // see StyleSheetTable_Impl::StyleSheetTable_Impl() where we set this default
    // Additionally, if the default para style has the same font size, then don't write it here.
    SwTextFormatColl* pDefaultStyle = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
    if (pDefaultStyle)
    {
        const SfxPoolItem* pItem = nullptr;
        if (pDefaultStyle->GetAttrSet().HasItem(RES_CHRATR_FONTSIZE, &pItem))
            return static_cast<const SvxFontHeightItem*>(pItem)->GetHeight() != rFontHeight.GetHeight();
    }
    return bRet;
}

}

void DocxAttributeOutput::OutputDefaultItem(const SfxPoolItem& rHt)
{
    bool bMustWrite = true;
    switch (rHt.Which())
    {
        case RES_CHRATR_CASEMAP:
            bMustWrite = static_cast< const SvxCaseMapItem& >(rHt).GetCaseMap() != SVX_CASEMAP_NOT_MAPPED;
            break;
        case RES_CHRATR_COLOR:
            bMustWrite = static_cast< const SvxColorItem& >(rHt).GetValue().GetColor() != COL_AUTO;
            break;
        case RES_CHRATR_CONTOUR:
            bMustWrite = static_cast< const SvxContourItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_CROSSEDOUT:
            bMustWrite = static_cast< const SvxCrossedOutItem& >(rHt).GetStrikeout() != STRIKEOUT_NONE;
            break;
        case RES_CHRATR_ESCAPEMENT:
            bMustWrite = static_cast< const SvxEscapementItem& >(rHt).GetEscapement() != SVX_ESCAPEMENT_OFF;
            break;
        case RES_CHRATR_FONT:
            bMustWrite = true;
            break;
        case RES_CHRATR_FONTSIZE:
            bMustWrite = lcl_isDefaultFontSize(static_cast< const SvxFontHeightItem& >(rHt), m_rExport.m_pDoc);
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
            bMustWrite = static_cast< const SvxUnderlineItem& >(rHt).GetLineStyle() != UNDERLINE_NONE;
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
            bMustWrite = static_cast< const SvxCharRotateItem& >(rHt).GetValue() != 0;
            break;
        case RES_CHRATR_EMPHASIS_MARK:
            bMustWrite = static_cast< const SvxEmphasisMarkItem& >(rHt).GetValue() != EMPHASISMARK_NONE;
            break;
        case RES_CHRATR_TWO_LINES:
            bMustWrite = static_cast< const SvxTwoLinesItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_SCALEW:
            bMustWrite = static_cast< const SvxCharScaleWidthItem& >(rHt).GetValue() != 100;
            break;
        case RES_CHRATR_RELIEF:
            bMustWrite = static_cast< const SvxCharReliefItem& >(rHt).GetValue() != RELIEF_NONE;
            break;
        case RES_CHRATR_HIDDEN:
            bMustWrite = static_cast< const SvxCharHiddenItem& >(rHt).GetValue();
            break;
        case RES_CHRATR_BOX:
            {
                const SvxBoxItem& rBoxItem = static_cast< const SvxBoxItem& >(rHt);
                bMustWrite = rBoxItem.GetTop() || rBoxItem.GetLeft() ||
                             rBoxItem.GetBottom() || rBoxItem.GetRight() ||
                             rBoxItem.GetDistance();
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
            bMustWrite = static_cast< const SvxLineSpacingItem& >(rHt).GetInterLineSpaceRule() != SVX_INTER_LINE_SPACE_OFF;
            break;
        case RES_PARATR_ADJUST:
            bMustWrite = static_cast< const SvxAdjustItem& >(rHt).GetAdjust() != SVX_ADJUST_LEFT;
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
            bMustWrite = static_cast< const SvxHyphenZoneItem& >(rHt).IsHyphen();
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
            bMustWrite = static_cast< const SvxParaVertAlignItem& >(rHt).GetValue() != SvxParaVertAlignItem::AUTOMATIC;
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
    m_pSerializer->startElementNS(XML_w, XML_docDefaults, FSEND);

    // Output the default run properties
    m_pSerializer->startElementNS(XML_w, XML_rPrDefault, FSEND);

    StartStyleProperties(false, 0);

    for (int i = int(RES_CHRATR_BEGIN); i < int(RES_CHRATR_END); ++i)
        OutputDefaultItem(m_rExport.m_pDoc->GetDefault(i));

    EndStyleProperties(false);

    m_pSerializer->endElementNS(XML_w, XML_rPrDefault);

    // Output the default paragraph properties
    m_pSerializer->startElementNS(XML_w, XML_pPrDefault, FSEND);

    StartStyleProperties(true, 0);

    for (int i = int(RES_PARATR_BEGIN); i < int(RES_PARATR_END); ++i)
        OutputDefaultItem(m_rExport.m_pDoc->GetDefault(i));

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

void DocxAttributeOutput::DefaultStyle( sal_uInt16 nStyle )
{
    // are these the values of enum ww::sti (see ../inc/wwstyles.hxx)?
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "TODO DocxAttributeOutput::DefaultStyle( sal_uInt16 nStyle )- %d", nStyle );
#else
    (void) nStyle; // to quiet the warning
#endif
}

/* Writes <a:srcRect> tag back to document.xml if a file conatins a cropped image.
*  NOTE : Tested on images of type JPEG,EMF/WMF,BMP, PNG and GIF.
*/
void DocxAttributeOutput::WriteSrcRect(const SdrObject* pSdrObj )
{
    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );

    OUString sUrl;
    xPropSet->getPropertyValue("GraphicURL") >>= sUrl;
    Size aOriginalSize( GraphicObject::CreateGraphicObjectFromURL( sUrl ).GetPrefSize() );

    css::text::GraphicCrop aGraphicCropStruct;
    xPropSet->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    const MapMode aMap100mm( MAP_100TH_MM );
    const MapMode& mapMode = GraphicObject::CreateGraphicObjectFromURL( sUrl ).GetPrefMapMode();
    if( mapMode.GetMapUnit() == MAP_PIXEL )
    {
        aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aOriginalSize, aMap100mm );
    }

    if ( (0 != aGraphicCropStruct.Left) || (0 != aGraphicCropStruct.Top) || (0 != aGraphicCropStruct.Right) || (0 != aGraphicCropStruct.Bottom) )
    {
        double  widthMultiplier  = 100000.0/aOriginalSize.Width();
        double  heightMultiplier = 100000.0/aOriginalSize.Height();

        double left   = aGraphicCropStruct.Left * widthMultiplier;
        double right  = aGraphicCropStruct.Right * widthMultiplier;
        double top    = aGraphicCropStruct.Top * heightMultiplier;
        double bottom = aGraphicCropStruct.Bottom * heightMultiplier;

        m_pSerializer->singleElementNS( XML_a, XML_srcRect,
             XML_l, I32S(left),
             XML_t, I32S(top),
             XML_r, I32S(right),
             XML_b, I32S(bottom),
             FSEND );
    }
}

void DocxAttributeOutput::PopRelIdCache()
{
    if (!m_aRelIdCache.empty())
        m_aRelIdCache.pop();
    if (!m_aSdrRelIdCache.empty())
        m_aSdrRelIdCache.pop();
}

void DocxAttributeOutput::PushRelIdCache()
{
    m_aRelIdCache.push(std::map<const Graphic*, OString>());
    m_aSdrRelIdCache.push(std::map<BitmapChecksum, OUString>());
}

OUString DocxAttributeOutput::FindRelId(BitmapChecksum nChecksum)
{
    OUString aRet;

    if (!m_aSdrRelIdCache.empty() && m_aSdrRelIdCache.top().find(nChecksum) != m_aSdrRelIdCache.top().end())
        aRet = m_aSdrRelIdCache.top()[nChecksum];

    return aRet;
}

void DocxAttributeOutput::CacheRelId(BitmapChecksum nChecksum, const OUString& rRelId)
{
    if (!m_aSdrRelIdCache.empty())
        m_aSdrRelIdCache.top()[nChecksum] = rRelId;
}

void DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj  ) - some stuff still missing" );

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

        // TODO Convert the file name to relative for better interoperability

        aRelId = m_rExport.AddRelation(
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image",
                    aFileName );

        nImageType = XML_link;
    }
    else
    {
        // inline, we also have to write the image itself
        const Graphic* pGraphic = nullptr;
        if (pGrfNode)
            pGraphic = &pGrfNode->GetGrf();
        else
            pGraphic = pOLENode->GetGraphic();

        if (!m_aRelIdCache.empty() && m_aRelIdCache.top().find(pGraphic) != m_aRelIdCache.top().end())
            // We already have a RelId for this Graphic.
            aRelId = m_aRelIdCache.top()[pGraphic];
        else
        {
            // Not in cache, then need to write it.
            m_rDrawingML.SetFS( m_pSerializer ); // to be sure that we write to the right stream

            OUString aImageId = m_rDrawingML.WriteImage( *pGraphic );

            aRelId = OUStringToOString( aImageId, RTL_TEXTENCODING_UTF8 );
            if (!m_aRelIdCache.empty())
                m_aRelIdCache.top()[pGraphic] = aRelId;
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

    m_rExport.SdrExporter().startDMLAnchorInline(pFrameFormat, rSize);

    // picture description (used for pic:cNvPr later too)
    ::sax_fastparser::FastAttributeList* docPrattrList = FastSerializerHelper::createAttrList();
    docPrattrList->add( XML_id, OString::number( m_anchorId++).getStr());
    docPrattrList->add( XML_name, OUStringToOString( pFrameFormat->GetName(), RTL_TEXTENCODING_UTF8 ) );
    docPrattrList->add( XML_descr, OUStringToOString( pGrfNode ? pGrfNode->GetDescription() : pOLEFrameFormat->GetObjDescription(), RTL_TEXTENCODING_UTF8 ).getStr());
    if( GetExport().GetFilter().getVersion( ) != oox::core::ECMA_DIALECT )
        docPrattrList->add( XML_title, OUStringToOString( pGrfNode ? pGrfNode->GetTitle() : pOLEFrameFormat->GetObjTitle(), RTL_TEXTENCODING_UTF8 ).getStr());
    XFastAttributeListRef docPrAttrListRef( docPrattrList );
    m_pSerializer->startElementNS( XML_wp, XML_docPr, docPrAttrListRef );
    // TODO hyperlink
    // m_pSerializer->singleElementNS( XML_a, XML_hlinkClick,
    //         FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
    //         FSNS( XML_r, XML_id ), "rId4",
    //         FSEND );
    m_pSerializer->endElementNS( XML_wp, XML_docPr );

    m_pSerializer->startElementNS( XML_wp, XML_cNvGraphicFramePr,
            FSEND );
    // TODO change aspect?
    m_pSerializer->singleElementNS( XML_a, XML_graphicFrameLocks,
            FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
            XML_noChangeAspect, "1",
            FSEND );
    m_pSerializer->endElementNS( XML_wp, XML_cNvGraphicFramePr );

    m_pSerializer->startElementNS( XML_a, XML_graphic,
            FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
            FSEND );
    m_pSerializer->startElementNS( XML_a, XML_graphicData,
            XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/picture",
            FSEND );

    m_pSerializer->startElementNS( XML_pic, XML_pic,
            FSNS( XML_xmlns, XML_pic ), "http://schemas.openxmlformats.org/drawingml/2006/picture",
            FSEND );

    m_pSerializer->startElementNS( XML_pic, XML_nvPicPr,
            FSEND );
    // It seems pic:cNvpr and wp:docPr are pretty much the same thing with the same attributes
    m_pSerializer->startElementNS( XML_pic, XML_cNvPr, docPrAttrListRef );

    // TODO hyperlink
    // m_pSerializer->singleElementNS( XML_a, XML_hlinkClick,
    //     FSNS( XML_r, XML_id ), "rId4",
    //     FSEND );
    m_pSerializer->endElementNS( XML_pic, XML_cNvPr );

    m_pSerializer->startElementNS( XML_pic, XML_cNvPicPr,
            FSEND );
    // TODO change aspect?
    m_pSerializer->singleElementNS( XML_a, XML_picLocks,
            XML_noChangeAspect, "1", XML_noChangeArrowheads, "1",
            FSEND );
    m_pSerializer->endElementNS( XML_pic, XML_cNvPicPr );
    m_pSerializer->endElementNS( XML_pic, XML_nvPicPr );

    // the actual picture
    m_pSerializer->startElementNS( XML_pic, XML_blipFill,
            FSEND );

/* At this point we are certain that, WriteImage returns empty RelId
   for unhandled graphic type. Therefore we write the picture description
   and not the relation( coz there ain't any), so that the user knows
   there is a image/graphic in the doc but it is broken instead of
   completely discarding it.
*/
    if ( aRelId.isEmpty() )
        m_pSerializer->startElementNS( XML_a, XML_blip,
            FSEND );
    else
        m_pSerializer->startElementNS( XML_a, XML_blip,
            FSNS( XML_r, nImageType ), aRelId.getStr(),
            FSEND );

    pItem = nullptr;
    sal_uInt32 nMode = GRAPHICDRAWMODE_STANDARD;

    if ( pGrfNode && SfxItemState::SET == pGrfNode->GetSwAttrSet().GetItemState(RES_GRFATR_DRAWMODE, true, &pItem))
    {
        nMode = static_cast<const SfxEnumItem*>(pItem)->GetValue();
        if (nMode == GRAPHICDRAWMODE_GREYS)
            m_pSerializer->singleElementNS (XML_a, XML_grayscl, FSEND);
        else if (nMode == GRAPHICDRAWMODE_MONO) //black/white has a 0,5 threshold in LibreOffice
            m_pSerializer->singleElementNS (XML_a, XML_biLevel, XML_thresh, OString::number(50000), FSEND);
        else if (nMode == GRAPHICDRAWMODE_WATERMARK) //watermark has a brightness/luminance of 0,5 and contrast of -0.7 in LibreOffice
            m_pSerializer->singleElementNS( XML_a, XML_lum, XML_bright, OString::number(70000), XML_contrast, OString::number(-70000), FSEND );
    }
    m_pSerializer->endElementNS( XML_a, XML_blip );

    if (pSdrObj){
        WriteSrcRect(pSdrObj);
    }

    m_pSerializer->startElementNS( XML_a, XML_stretch,
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_fillRect,
            FSEND );
    m_pSerializer->endElementNS( XML_a, XML_stretch );
    m_pSerializer->endElementNS( XML_pic, XML_blipFill );

    // TODO setup the right values below
    m_pSerializer->startElementNS( XML_pic, XML_spPr,
            XML_bwMode, "auto",
            FSEND );
    m_pSerializer->startElementNS( XML_a, XML_xfrm,
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_off,
            XML_x, "0", XML_y, "0",
            FSEND );
    OString aWidth( OString::number( TwipsToEMU( rSize.Width() ) ) );
    OString aHeight( OString::number( TwipsToEMU( rSize.Height() ) ) );
    m_pSerializer->singleElementNS( XML_a, XML_ext,
            XML_cx, aWidth.getStr(),
            XML_cy, aHeight.getStr(),
            FSEND );
    m_pSerializer->endElementNS( XML_a, XML_xfrm );
    m_pSerializer->startElementNS( XML_a, XML_prstGeom,
            XML_prst, "rect",
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_avLst,
            FSEND );
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

void DocxAttributeOutput::WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rOLENode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat )
{
    if( WriteOLEChart( pSdrObj, rSize ))
        return;
    if( WriteOLEMath( pSdrObj, rOLENode, rSize ))
        return;
    if( PostponeOLE( pSdrObj, rOLENode, rSize, pFlyFrameFormat ))
        return;
    // Then we fall back to just export the object as a graphic.
    if( !m_pPostponedGraphic )
        FlyFrameGraphic( nullptr, rSize, pFlyFrameFormat, &rOLENode );
    else
        // w:drawing should not be inside w:rPr, so write it out later
        m_pPostponedGraphic->push_back(PostponedGraphic(nullptr, rSize, pFlyFrameFormat, &rOLENode, nullptr));
}

bool DocxAttributeOutput::WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize )
{
    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY );
    if (!xShape.is())
        return false;

    uno::Reference<beans::XPropertySet> const xPropSet(xShape, uno::UNO_QUERY);
    if (!xPropSet.is())
        return false;

    OUString clsid; // why is the property of type string, not sequence<byte>?
    xPropSet->getPropertyValue("CLSID") >>= clsid;
    SAL_WARN_IF(clsid.isEmpty(), "sw.ww8", "OLE without CLSID?");
    SvGlobalName aClassID;
    bool const isValid(aClassID.MakeId(clsid));
    SAL_WARN_IF(!isValid, "sw.ww8", "OLE with invalid CLSID?");

    if (!SotExchange::IsChart(aClassID))
        return false;

    m_postponedChart = pSdrObj;
    m_postponedChartSize = rSize;
    return true;
}

/*
 * Write chart hierarchy in w:drawing after end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedChart()
{
    if(m_postponedChart == nullptr)
        return;
    uno::Reference< chart2::XChartDocument > xChartDoc;
    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(m_postponedChart)->getUnoShape(), uno::UNO_QUERY );
    if( xShape.is() )
    {
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
        if( xPropSet.is() )
            xChartDoc.set( xPropSet->getPropertyValue( "Model" ), uno::UNO_QUERY );
    }

    if( xChartDoc.is() )
    {
        OSL_TRACE("DocxAttributeOutput::WriteOLE2Obj: export chart ");
        m_pSerializer->startElementNS( XML_w, XML_drawing,
            FSEND );
        m_pSerializer->startElementNS( XML_wp, XML_inline,
            XML_distT, "0", XML_distB, "0", XML_distL, "0", XML_distR, "0",
            FSEND );

        OString aWidth( OString::number( TwipsToEMU( m_postponedChartSize.Width() ) ) );
        OString aHeight( OString::number( TwipsToEMU( m_postponedChartSize.Height() ) ) );
        m_pSerializer->singleElementNS( XML_wp, XML_extent,
            XML_cx, aWidth.getStr(),
            XML_cy, aHeight.getStr(),
            FSEND );
        // TODO - the right effectExtent, extent including the effect
        m_pSerializer->singleElementNS( XML_wp, XML_effectExtent,
            XML_l, "0", XML_t, "0", XML_r, "0", XML_b, "0",
            FSEND );

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
            XML_id, I32S( m_anchorId++ ),
            XML_name, USS( sName ),
            FSEND );

        m_pSerializer->singleElementNS( XML_wp, XML_cNvGraphicFramePr,
            FSEND );

        m_pSerializer->startElementNS( XML_a, XML_graphic,
            FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
            FSEND );

        m_pSerializer->startElementNS( XML_a, XML_graphicData,
            XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/chart",
            FSEND );

        OString aRelId;
        m_nChartCount++;
        uno::Reference< frame::XModel > xModel( xChartDoc, uno::UNO_QUERY );
        aRelId = m_rExport.OutputChart( xModel, m_nChartCount, m_pSerializer );

        m_pSerializer->singleElementNS( XML_c, XML_chart,
            FSNS( XML_xmlns, XML_c ), "http://schemas.openxmlformats.org/drawingml/2006/chart",
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSNS( XML_r, XML_id ), aRelId.getStr(),
            FSEND );

        m_pSerializer->endElementNS( XML_a, XML_graphicData );
        m_pSerializer->endElementNS( XML_a, XML_graphic );
        m_pSerializer->endElementNS( XML_wp, XML_inline );
        m_pSerializer->endElementNS( XML_w, XML_drawing );

    }
    m_postponedChart = nullptr;
}

bool DocxAttributeOutput::WriteOLEMath( const SdrObject*, const SwOLENode& rOLENode, const Size& )
{
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode&>(rOLENode).GetOLEObj().GetOleRef());
    SvGlobalName aObjName(xObj->getClassID());

    if( !SotExchange::IsMath(aObjName) )
        return false;
    m_aPostponedMaths.push_back(&rOLENode);
    return true;
}

void DocxAttributeOutput::WritePostponedMath(const SwOLENode* pPostponedMath)
{
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode*>(pPostponedMath)->GetOLEObj().GetOleRef());
    if (embed::EmbedStates::LOADED == xObj->getCurrentState())
    {   // must be running so there is a Component
        xObj->changeState(embed::EmbedStates::RUNNING);
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
                oox::drawingml::DOCUMENT_DOCX);
}

void DocxAttributeOutput::WritePostponedFormControl(const SdrObject* pObject)
{
    if (!pObject || pObject->GetObjInventor() != FmFormInventor)
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

        Date aOriginalDate(Date::EMPTY);
        OUString sOriginalContent, sDateFormat, sAlias;
        OUString sLocale("en-US");
        uno::Sequence<beans::PropertyValue> aGrabBag;
        uno::Reference<beans::XPropertySet> xShapePropertySet(pFormObj->getUnoShape(), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aCharFormat;
        if (xShapePropertySet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= aGrabBag)
        {
            for (sal_Int32 i=0; i < aGrabBag.getLength(); ++i)
            {
                if (aGrabBag[i].Name == "DateFormat")
                    aGrabBag[i].Value >>= sDateFormat;
                else if (aGrabBag[i].Name == "Locale")
                    aGrabBag[i].Value >>= sLocale;
                else if (aGrabBag[i].Name == "OriginalContent")
                    aGrabBag[i].Value >>= sOriginalContent;
                else if (aGrabBag[i].Name == "OriginalDate")
                {
                    css::util::Date aUNODate;
                    aGrabBag[i].Value >>= aUNODate;
                    aOriginalDate.SetDay(aUNODate.Day);
                    aOriginalDate.SetMonth(aUNODate.Month);
                    aOriginalDate.SetYear(aUNODate.Year);
                }
                else if (aGrabBag[i].Name == "CharFormat")
                    aGrabBag[i].Value >>= aCharFormat;
                else if (aGrabBag[i].Name == "ooxml:CT_SdtPr_alias")
                    aGrabBag[i].Value >>= sAlias;
            }
        }
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

            if (aOriginalDate == aDate)
            {
                aContentText = sOriginalContent;
                // sDateFormat was extracted from the grab bag
            }
            else
            {
                aContentText = OUString::createFromAscii(DateToDDMMYYYYOString(aDate).getStr());
                sDateFormat = "dd/MM/yyyy";
            }
        }
        else
            aContentText = xPropertySet->getPropertyValue("HelpText").get<OUString>();

        // output component

        m_pSerializer->startElementNS(XML_w, XML_sdt, FSEND);
        m_pSerializer->startElementNS(XML_w, XML_sdtPr, FSEND);

        if (!sAlias.isEmpty())
            m_pSerializer->singleElementNS(XML_w, XML_alias,
                                           FSNS(XML_w, XML_val), OUStringToOString(sAlias, RTL_TEXTENCODING_UTF8),
                                           FSEND);

        if (bHasDate)
            m_pSerializer->startElementNS(XML_w, XML_date,
                                          FSNS( XML_w, XML_fullDate ), sDate.getStr(),
                                          FSEND);
        else
            m_pSerializer->startElementNS(XML_w, XML_date, FSEND);

        m_pSerializer->singleElementNS(XML_w, XML_dateFormat,
                                       FSNS(XML_w, XML_val),
                                       OUStringToOString( sDateFormat, RTL_TEXTENCODING_UTF8 ).getStr(),
                                       FSEND);
        m_pSerializer->singleElementNS(XML_w, XML_lid,
                                       FSNS(XML_w, XML_val),
                                       OUStringToOString( sLocale, RTL_TEXTENCODING_UTF8 ).getStr(),
                                       FSEND);
        m_pSerializer->singleElementNS(XML_w, XML_storeMappedDataAs,
                                       FSNS(XML_w, XML_val), "dateTime",
                                       FSEND);
        m_pSerializer->singleElementNS(XML_w, XML_calendar,
                                       FSNS(XML_w, XML_val), "gregorian",
                                       FSEND);

        m_pSerializer->endElementNS(XML_w, XML_date);
        m_pSerializer->endElementNS(XML_w, XML_sdtPr);

        m_pSerializer->startElementNS(XML_w, XML_sdtContent, FSEND);
        m_pSerializer->startElementNS(XML_w, XML_r, FSEND);

        if (aCharFormat.hasElements())
        {
            m_pTableStyleExport->SetSerializer(m_pSerializer);
            m_pTableStyleExport->CharFormat(aCharFormat);
        }

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
        uno::Sequence<OUString> aItems = xPropertySet->getPropertyValue("StringItemList").get< uno::Sequence<OUString> >();

        // output component

        m_pSerializer->startElementNS(XML_w, XML_sdt, FSEND);
        m_pSerializer->startElementNS(XML_w, XML_sdtPr, FSEND);

        m_pSerializer->startElementNS(XML_w, XML_dropDownList, FSEND);

        for (sal_Int32 i=0; i < aItems.getLength(); ++i)
        {
            m_pSerializer->singleElementNS(XML_w, XML_listItem,
                                           FSNS(XML_w, XML_displayText),
                                           OUStringToOString( aItems[i], RTL_TEXTENCODING_UTF8 ).getStr(),
                                           FSNS(XML_w, XML_value),
                                           OUStringToOString( aItems[i], RTL_TEXTENCODING_UTF8 ).getStr(),
                                           FSEND);
        }

        m_pSerializer->endElementNS(XML_w, XML_dropDownList);
        m_pSerializer->endElementNS(XML_w, XML_sdtPr);

        m_pSerializer->startElementNS(XML_w, XML_sdtContent, FSEND);
        m_pSerializer->startElementNS(XML_w, XML_r, FSEND);
        RunText(sText);
        m_pSerializer->endElementNS(XML_w, XML_r);
        m_pSerializer->endElementNS(XML_w, XML_sdtContent);

        m_pSerializer->endElementNS(XML_w, XML_sdt);
    }
}

bool DocxAttributeOutput::PostponeOLE( const SdrObject*, SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat )
{
    if( !m_pPostponedOLEs )
        //cannot be postponed, try to write now
        WriteOLE( rNode, rSize, pFlyFrameFormat );
    else
        m_pPostponedOLEs->push_back( PostponedOLE( &rNode, rSize, pFlyFrameFormat ) );
    return true;
}

/*
 * Write w:object hierarchy for embedded objects after end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedOLE()
{
    if( !m_pPostponedOLEs )
        return;

    for( std::list< PostponedOLE >::iterator it = m_pPostponedOLEs->begin();
         it != m_pPostponedOLEs->end();
         ++it )
    {
        WriteOLE( *it->object, it->size, it->frame );
    }

    // clear list of postponed objects
    m_pPostponedOLEs.reset(nullptr);
}

void DocxAttributeOutput::WriteOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* rFlyFrameFormat )
{
    // get interoperability information about embedded objects
    uno::Reference< beans::XPropertySet > xPropSet( m_rExport.m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aGrabBag, aObjectsInteropList,aObjectInteropAttributes;
    xPropSet->getPropertyValue( UNO_NAME_MISC_OBJ_INTEROPGRABBAG ) >>= aGrabBag;
    for( sal_Int32 i=0; i < aGrabBag.getLength(); ++i )
        if ( aGrabBag[i].Name == "EmbeddedObjects" )
        {
            aGrabBag[i].Value >>= aObjectsInteropList;
            break;
        }

    SwOLEObj& aObject = rNode.GetOLEObj();
    uno::Reference < embed::XEmbeddedObject > xObj( aObject.GetOleRef() );
    comphelper::EmbeddedObjectContainer* aContainer = aObject.GetObject().GetContainer();
    OUString sObjectName = aContainer->GetEmbeddedObjectName( xObj );

    // set some attributes according to the type of the embedded object
    OUString sProgID, sDrawAspect = "Content";
    for( sal_Int32 i=0; i < aObjectsInteropList.getLength(); ++i )
        if ( aObjectsInteropList[i].Name == sObjectName )
        {
            aObjectsInteropList[i].Value >>= aObjectInteropAttributes;
            break;
        }

    for( sal_Int32 i=0; i < aObjectInteropAttributes.getLength(); ++i )
    {
            if ( aObjectInteropAttributes[i].Name == "ProgID" )
            {
                aObjectInteropAttributes[i].Value >>= sProgID;
            }
            else if ( aObjectInteropAttributes[i].Name == "DrawAspect" )
            {
                aObjectInteropAttributes[i].Value >>= sDrawAspect;
            }
    }

    // write embedded file
    OString sId = m_rExport.WriteOLEObject(aObject, sProgID);

    if( sId.isEmpty() )
    {
        // the embedded file could not be saved
        // fallback: save as an image
        FlyFrameGraphic( nullptr, rSize, rFlyFrameFormat, &rNode );
        return;
    }

    // write preview image
    const Graphic* pGraphic = rNode.GetGraphic();
    m_rDrawingML.SetFS(m_pSerializer);
    OUString sImageId = m_rDrawingML.WriteImage( *pGraphic );

    m_pSerializer->startElementNS( XML_w, XML_object, FSEND );

    OStringBuffer sShapeStyle, sShapeId;
    sShapeStyle.append( "width:" ).append( double( rSize.Width() ) / 20 )
                        .append( "pt;height:" ).append( double( rSize.Height() ) / 20 )
                        .append( "pt" ); //from VMLExport::AddRectangleDimensions(), it does: value/20
    sShapeId.append( "ole_" ).append( sId );

    // shape definition
    m_pSerializer->startElementNS( XML_v, XML_shape,
                                   XML_id, sShapeId.getStr(),
                                   XML_style, sShapeStyle.getStr(),
                                   FSNS( XML_o, XML_ole ), "", //compulsory, even if it's empty
                                   FSEND );

    // shape filled with the preview image
    m_pSerializer->singleElementNS( XML_v, XML_imagedata,
                                    FSNS( XML_r, XML_id ), OUStringToOString( sImageId, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    FSNS( XML_o, XML_title ), "",
                                    FSEND );

    m_pSerializer->endElementNS( XML_v, XML_shape );

    // OLE object definition
    m_pSerializer->singleElementNS( XML_o, XML_OLEObject,
                                    XML_Type, "Embed",
                                    XML_ProgID, OUStringToOString( sProgID, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    XML_ShapeID, sShapeId.getStr(),
                                    XML_DrawAspect, OUStringToOString( sDrawAspect, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    XML_ObjectID, "_" + OString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max())),
                                    FSNS( XML_r, XML_id ), sId.getStr(),
                                    FSEND );

    m_pSerializer->endElementNS( XML_w, XML_object );
}

/*
 * Write w:pict hierarchy  end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedVMLDrawing()
{
    if (!m_pPostponedVMLDrawings)
        return;

    for( std::list< PostponedDrawing >::iterator it = m_pPostponedVMLDrawings->begin();
         it != m_pPostponedVMLDrawings->end();
         ++it )
    {
        m_rExport.SdrExporter().writeVMLDrawing(it->object, *(it->frame), *(it->point));
    }
    m_pPostponedVMLDrawings.reset(nullptr);
}

void DocxAttributeOutput::WritePostponedCustomShape()
{
    if (!m_pPostponedCustomShape)
        return;

    bool bStartedParaSdt = m_bStartedParaSdt;
    for( std::list< PostponedDrawing >::iterator it = m_pPostponedCustomShape->begin();
         it != m_pPostponedCustomShape->end();
         ++it )
    {
        if ( IsAlternateContentChoiceOpen() )
            m_rExport.SdrExporter().writeDMLDrawing(it->object, (it->frame), m_anchorId++);
        else
            m_rExport.SdrExporter().writeDMLAndVMLDrawing(it->object, *(it->frame), *(it->point), m_anchorId++);
    }
    m_bStartedParaSdt = bStartedParaSdt;
    m_pPostponedCustomShape.reset(nullptr);
}

void DocxAttributeOutput::WritePostponedDMLDrawing()
{
    if (!m_pPostponedDMLDrawings)
        return;

    // Clear the list early, this method may be called recursively.
    std::unique_ptr< std::list<PostponedDrawing> > pPostponedDMLDrawings(m_pPostponedDMLDrawings.release());
    std::unique_ptr< std::list<PostponedOLE> > pPostponedOLEs(m_pPostponedOLEs.release());

    bool bStartedParaSdt = m_bStartedParaSdt;
    for( std::list< PostponedDrawing >::iterator it = pPostponedDMLDrawings->begin();
         it != pPostponedDMLDrawings->end();
         ++it )
    {
        // Avoid w:drawing within another w:drawing.
        if ( IsAlternateContentChoiceOpen() && !( m_rExport.SdrExporter().IsDrawingOpen()) )
           m_rExport.SdrExporter().writeDMLDrawing(it->object, (it->frame), m_anchorId++);
        else
            m_rExport.SdrExporter().writeDMLAndVMLDrawing(it->object, *(it->frame), *(it->point), m_anchorId++);
    }
    m_bStartedParaSdt = bStartedParaSdt;

    m_pPostponedOLEs.reset(pPostponedOLEs.release());
}

void DocxAttributeOutput::OutputFlyFrame_Impl( const ww8::Frame &rFrame, const Point& rNdTopLeft )
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
                        m_pPostponedGraphic->push_back(PostponedGraphic(pGrfNode, rFrame.GetLayoutSize(), nullptr, nullptr, pSdrObj));
                    }
                }
            }
            break;
        case ww8::Frame::eDrawing:
            {
                const SdrObject* pSdrObj = rFrame.GetFrameFormat().FindRealSdrObject();
                if ( pSdrObj )
                {
                    if ( IsDiagram( pSdrObj ) )
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
                            bool bStartedParaSdt = m_bStartedParaSdt;
                            if ( IsAlternateContentChoiceOpen() )
                            {
                                // Do not write w:drawing inside w:drawing. Instead Postpone the Inner Drawing.
                                if( m_rExport.SdrExporter().IsDrawingOpen() )
                                    m_pPostponedCustomShape->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat()), &rNdTopLeft));
                                else
                                    m_rExport.SdrExporter().writeDMLDrawing( pSdrObj, &rFrame.GetFrameFormat(), m_anchorId++);
                            }
                            else
                                m_rExport.SdrExporter().writeDMLAndVMLDrawing( pSdrObj, rFrame.GetFrameFormat(), rNdTopLeft, m_anchorId++);
                            m_bStartedParaSdt = bStartedParaSdt;

                            m_bPostponedProcessingFly = false ;
                        }
                        // IsAlternateContentChoiceOpen(): check is to ensure that only one object is getting added. Without this check, plus one object gets added
                        // m_bParagraphFrameOpen: check if the frame is open.
                        else if (IsAlternateContentChoiceOpen() && m_bParagraphFrameOpen)
                            m_pPostponedCustomShape->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat()), &rNdTopLeft));
                        else
                        {
                            // we are writing out attributes, but w:drawing should not be inside w:rPr, so write it out later
                            m_bPostponedProcessingFly = true ;
                            m_pPostponedDMLDrawings->push_back(PostponedDrawing(pSdrObj, &(rFrame.GetFrameFormat()), &rNdTopLeft));
                        }
                    }
                }
            }
            break;
        case ww8::Frame::eTextBox:
            {
                // If this is a TextBox of a shape, then ignore: it's handled in WriteTextBox().
                if (m_rExport.SdrExporter().isTextBox(rFrame.GetFrameFormat()))
                    break;

                // The frame output is postponed to the end of the anchor paragraph
                bool bDuplicate = false;
                const OUString& rName = rFrame.GetFrameFormat().GetName();
                unsigned nSize = m_aFramesOfParagraph.size();
                for( unsigned nIndex = 0; nIndex < nSize; ++nIndex )
                {
                    const OUString& rNameExisting = m_aFramesOfParagraph[nIndex].GetFrameFormat().GetName();

                    if (!rName.isEmpty() && !rNameExisting.isEmpty())
                    {
                        if (rName == rNameExisting)
                            bDuplicate = true;
                    }
                }

                if( !bDuplicate )
                {
                    m_bPostponedProcessingFly = true ;
                    m_aFramesOfParagraph.push_back(ww8::Frame(rFrame));
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
                    WriteOLE2Obj( pSdrObj, rOLENd, rFrame.GetLayoutSize(), dynamic_cast<const SwFlyFrameFormat*>( &rFrameFormat ));
                    m_bPostponedProcessingFly = false ;
                }
            }
            break;
        case ww8::Frame::eFormControl:
            {
                const SdrObject* pObject = rFrame.GetFrameFormat().FindRealSdrObject();
                m_aPostponedFormControls.push_back(pObject);
                m_bPostponedProcessingFly = true ;
            }
            break;
        default:
            OSL_TRACE( "TODO DocxAttributeOutput::OutputFlyFrame_Impl( const ww8::Frame& rFrame, const Point& rNdTopLeft ) - frame type '%s'\n",
                    rFrame.GetWriterType() == ww8::Frame::eTextBox? "eTextBox":
                    ( rFrame.GetWriterType() == ww8::Frame::eOle? "eOle": "???" ) );
            break;
    }

    m_pSerializer->mergeTopMarks(Tag_OutputFlyFrame, sax_fastparser::MergeMarks::POSTPONE);
}

bool DocxAttributeOutput::IsDiagram( const SdrObject* sdrObject )
{
    uno::Reference< drawing::XShape > xShape( const_cast<SdrObject*>(sdrObject)->getUnoShape(), uno::UNO_QUERY );
    if ( !xShape.is() )
        return false;

    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        return false;

    // if the shape doesn't have the InteropGrabBag property, it's not a diagram
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return false;

    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        // if we find any of the diagram components, it's a diagram
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXData" || propName == "OOXLayout" || propName == "OOXStyle" ||
             propName == "OOXColor" || propName == "OOXDrawing")
            return true;
    }
    return false;
}

void DocxAttributeOutput::WriteOutliner(const OutlinerParaObject& rParaObj)
{
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( m_rExport, rEditObj, TXT_HFTXTBOX );

    sal_Int32 nPara = rEditObj.GetParagraphCount();

    m_pSerializer->startElementNS( XML_w, XML_txbxContent, FSEND );
    for (sal_Int32 n = 0; n < nPara; ++n)
    {
        if( n )
            aAttrIter.NextPara( n );

        OUString aStr( rEditObj.GetText( n ));
        sal_Int32 nAktPos = 0;
        sal_Int32 nEnd = aStr.getLength();

        StartParagraph(ww8::WW8TableNodeInfo::Pointer_t());

        // Write paragraph properties.
        StartParagraphProperties();
        aAttrIter.OutParaAttr(false);
        SfxItemSet aParagraphMarkerProperties(m_rExport.m_pDoc->GetAttrPool());
        EndParagraphProperties(aParagraphMarkerProperties, nullptr, nullptr, nullptr);

        do {
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);

            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

            // Write run properties.
            m_pSerializer->startElementNS(XML_w, XML_rPr, FSEND);
            aAttrIter.OutAttr(nAktPos);
            WriteCollectedRunProperties();
            m_pSerializer->endElementNS(XML_w, XML_rPr);

            bool bTextAtr = aAttrIter.IsTextAttr( nAktPos );
            if( !bTextAtr )
            {
                OUString aOut( aStr.copy( nAktPos, nNextAttr - nAktPos ) );
                RunText(aOut);
            }

            m_pSerializer->endElementNS( XML_w, XML_r );

            nAktPos = nNextAttr;
            aAttrIter.NextPos();
        }
        while( nAktPos < nEnd );
        // Word can't handle nested text boxes, so write them on the same level.
        ++m_nTextFrameLevel;
        EndParagraph(ww8::WW8TableNodeInfoInner::Pointer_t());
        --m_nTextFrameLevel;
    }
    m_pSerializer->endElementNS( XML_w, XML_txbxContent );
}

void DocxAttributeOutput::pushToTableExportContext(DocxTableExportContext& rContext)
{
    rContext.m_pTableInfo = m_rExport.m_pTableInfo;
    m_rExport.m_pTableInfo = ww8::WW8TableInfo::Pointer_t(new ww8::WW8TableInfo());

    rContext.m_bTableCellOpen = m_tableReference->m_bTableCellOpen;
    m_tableReference->m_bTableCellOpen = false;

    rContext.m_nTableDepth = m_tableReference->m_nTableDepth;
    m_tableReference->m_nTableDepth = 0;
}

void DocxAttributeOutput::popFromTableExportContext(DocxTableExportContext& rContext)
{
    m_rExport.m_pTableInfo = rContext.m_pTableInfo;
    m_tableReference->m_bTableCellOpen = rContext.m_bTableCellOpen;
    m_tableReference->m_nTableDepth = rContext.m_nTableDepth;
}

void DocxAttributeOutput::WriteTextBox(uno::Reference<drawing::XShape> xShape)
{
    DocxTableExportContext aTableExportContext;
    pushToTableExportContext(aTableExportContext);

    SwFrameFormat* pTextBox = SwTextBoxHelper::findTextBox(xShape);
    const SwPosition* pAnchor = pTextBox->GetAnchor().GetContentAnchor();
    ww8::Frame aFrame(*pTextBox, *pAnchor);
    m_rExport.SdrExporter().writeDMLTextFrame(&aFrame, m_anchorId++, /*bTextBoxOnly=*/true);

    popFromTableExportContext(aTableExportContext);
}

void DocxAttributeOutput::WriteVMLTextBox(uno::Reference<drawing::XShape> xShape)
{
    DocxTableExportContext aTableExportContext;
    pushToTableExportContext(aTableExportContext);

    SwFrameFormat* pTextBox = SwTextBoxHelper::findTextBox(xShape);
    const SwPosition* pAnchor = pTextBox->GetAnchor().GetContentAnchor();
    ww8::Frame aFrame(*pTextBox, *pAnchor);
    m_rExport.SdrExporter().writeVMLTextFrame(&aFrame, /*bTextBoxOnly=*/true);

    popFromTableExportContext(aTableExportContext);
}

oox::drawingml::DrawingML& DocxAttributeOutput::GetDrawingML()
{
    return m_rDrawingML;
}

/// Functor to do case-insensitive ordering of OUString instances.
struct OUStringIgnoreCase
{
    bool operator() (const OUString& lhs, const OUString& rhs) const
    {
        return lhs.compareToIgnoreAsciiCase(rhs) < 0;
    }
};

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

    static std::set<OUString, OUStringIgnoreCase> aWhitelist;
    if (aWhitelist.empty())
    {
        aWhitelist.insert("No Spacing");
        aWhitelist.insert("List Paragraph");
        aWhitelist.insert("Quote");
        aWhitelist.insert("Intense Quote");
        aWhitelist.insert("Subtle Emphasis,");
        aWhitelist.insert("Intense Emphasis");
        aWhitelist.insert("Subtle Reference");
        aWhitelist.insert("Intense Reference");
        aWhitelist.insert("Book Title");
        aWhitelist.insert("TOC Heading");
    }
    // Not custom style? Then we have a list of standard styles which should be qFormat.
    return aWhitelist.find(rName) != aWhitelist.end();
}

void DocxAttributeOutput::StartStyle( const OUString& rName, StyleType eType,
        sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId, bool bAutoUpdate )
{
    bool bQFormat = false, bUnhideWhenUsed = false, bSemiHidden = false, bLocked = false, bDefault = false, bCustomStyle = false;
    OUString aLink, aRsid, aUiPriority;
    FastAttributeList* pStyleAttributeList = FastSerializerHelper::createAttrList();
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

    for (sal_Int32 i = 0; i < rGrabBag.getLength(); ++i)
    {
        if (rGrabBag[i].Name == "uiPriority")
            aUiPriority = rGrabBag[i].Value.get<OUString>();
        else if (rGrabBag[i].Name == "qFormat")
            bQFormat = true;
        else if (rGrabBag[i].Name == "link")
            aLink = rGrabBag[i].Value.get<OUString>();
        else if (rGrabBag[i].Name == "rsid")
            aRsid = rGrabBag[i].Value.get<OUString>();
        else if (rGrabBag[i].Name == "unhideWhenUsed")
            bUnhideWhenUsed = true;
        else if (rGrabBag[i].Name == "semiHidden")
            bSemiHidden = true;
        else if (rGrabBag[i].Name == "locked")
            bLocked = true;
        else if (rGrabBag[i].Name == "default")
            bDefault = rGrabBag[i].Value.get<sal_Bool>();
        else if (rGrabBag[i].Name == "customStyle")
            bCustomStyle = rGrabBag[i].Value.get<sal_Bool>();
        else
            SAL_WARN("sw.ww8", "Unhandled style property: " << rGrabBag[i].Name);
    }

    // MSO exports English names and writerfilter only recognize them.
    const sal_Char *pEnglishName = nullptr;
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
    pStyleAttributeList->add(FSNS( XML_w, XML_styleId ), m_rExport.m_pStyles->GetStyleId(nId).getStr());
    if (bDefault)
        pStyleAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pStyleAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    XFastAttributeListRef xStyleAttributeList(pStyleAttributeList);
    m_pSerializer->startElementNS( XML_w, XML_style, xStyleAttributeList);
    m_pSerializer->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), pEnglishName ? pEnglishName : OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    if ( nBase != 0x0FFF && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_basedOn,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nBase).getStr(),
                FSEND );
    }

    if ( nNext != nId && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_next,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nNext).getStr(),
                FSEND );
    }

    if (!aLink.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_link,
                FSNS(XML_w, XML_val), OUStringToOString(aLink, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);

    if ( bAutoUpdate )
        m_pSerializer->singleElementNS( XML_w, XML_autoRedefine, FSEND );

    if (!aUiPriority.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority,
                FSNS(XML_w, XML_val), OUStringToOString(aUiPriority, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden, FSEND);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed, FSEND);

    if (bQFormat || lcl_guessQFormat(rName, nWwId))
        m_pSerializer->singleElementNS(XML_w, XML_qFormat, FSEND);
    if (bLocked)
        m_pSerializer->singleElementNS(XML_w, XML_locked, FSEND);
    if (!aRsid.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_rsid,
                FSNS(XML_w, XML_val), OUStringToOString(aRsid, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
}

void DocxAttributeOutput::EndStyle()
{
    m_pSerializer->endElementNS( XML_w, XML_style );
}

void DocxAttributeOutput::StartStyleProperties( bool bParProp, sal_uInt16 /*nStyle*/ )
{
    if ( bParProp )
    {
        m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );
        InitCollectedParagraphProperties();
    }
    else
    {
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
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

namespace
{

void lcl_OutlineLevel(sax_fastparser::FSHelperPtr pSerializer, sal_uInt16 nLevel)
{
    if (nLevel >= WW8ListManager::nMaxLevel)
        nLevel = WW8ListManager::nMaxLevel - 1;

    pSerializer->singleElementNS(XML_w, XML_outlineLvl,
            FSNS(XML_w, XML_val), OString::number(nLevel).getStr(),
            FSEND);
}

}

void DocxAttributeOutput::OutlineNumbering(sal_uInt8 const nLvl)
{
    lcl_OutlineLevel(m_pSerializer, nLvl);
}

void DocxAttributeOutput::ParaOutlineLevel(const SfxUInt16Item& rItem)
{
    if (rItem.GetValue() > 0)
        lcl_OutlineLevel(m_pSerializer, rItem.GetValue() - 1);
}

void DocxAttributeOutput::PageBreakBefore( bool bBreak )
{
    if ( bBreak )
        m_pSerializer->singleElementNS( XML_w, XML_pageBreakBefore, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_pageBreakBefore,
                FSNS( XML_w, XML_val ), "false",
                FSEND );
}

void DocxAttributeOutput::SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo )
{
    switch ( nC )
    {
        case msword::ColumnBreak:
            // The column break should be output in the next paragraph...
            m_nColBreakStatus = COLBRK_POSTPONE;
            break;
        case msword::PageBreak:
            if ( pSectionInfo )
            {
                // don't add section properties if this will be the first
                // paragraph in the document
                if ( !m_bParagraphOpened && !m_bIsFirstParagraph)
                {
                    // Create a dummy paragraph if needed
                    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );
                    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );

                    m_rExport.SectionProperties( *pSectionInfo );

                    m_pSerializer->endElementNS( XML_w, XML_pPr );
                    m_pSerializer->endElementNS( XML_w, XML_p );
                }
                else
                {
                    // postpone the output of this; it has to be done inside the
                    // paragraph properties, so remember it until then
                    m_pSectionInfo.reset( new WW8_SepInfo( *pSectionInfo ));
                }
            }
            else if ( m_bParagraphOpened )
            {
                m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
                m_pSerializer->singleElementNS( XML_w, XML_br,
                        FSNS( XML_w, XML_type ), "page", FSEND );
                m_pSerializer->endElementNS( XML_w, XML_r );
            }
            else
                m_bPostponedPageBreak = true;

            break;
        default:
            OSL_TRACE( "Unknown section break to write: %d", nC );
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
    m_pSerializer->startElementNS( XML_w, XML_sectPr, FSEND );
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
    sal_Int32 len = sizeof ( aOrder ) / sizeof( sal_Int32 );
    uno::Sequence< sal_Int32 > aSeqOrder( len );
    for ( sal_Int32 i = 0; i < len; i++ )
        aSeqOrder[i] = aOrder[i];

    m_pSerializer->mark(Tag_StartSection, aSeqOrder);
    m_bHadSectPr = true;
}

void DocxAttributeOutput::EndSection()
{
    // Write the section properties
    if ( m_pSectionSpacingAttrList.is() )
    {
        XFastAttributeListRef xAttrList( m_pSectionSpacingAttrList );
        m_pSectionSpacingAttrList.clear();

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
        m_pSerializer->singleElementNS( XML_w, XML_formProt,
                FSNS( XML_w, XML_val ), "true", FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_formProt,
                FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo )
{
    FastAttributeList* pAttr = FastSerializerHelper::createAttrList();
    pAttr->add( FSNS( XML_w, XML_countBy ), OString::number(rLnNumInfo.GetCountBy()).getStr());
    pAttr->add( FSNS( XML_w, XML_restart ), rLnNumInfo.IsRestartEachPage() ? "newPage" : "continuous" );
    if( rLnNumInfo.GetPosFromLeft())
        pAttr->add( FSNS( XML_w, XML_distance ), OString::number(rLnNumInfo.GetPosFromLeft()).getStr());
    if( nRestartNo )
        pAttr->add( FSNS( XML_w, XML_start ), OString::number( nRestartNo).getStr());
    XFastAttributeListRef xAttrs( pAttr );
    m_pSerializer->singleElementNS( XML_w, XML_lnNumType, xAttrs );
}

void DocxAttributeOutput::SectionTitlePage()
{
    m_pSerializer->singleElementNS( XML_w, XML_titlePg, FSEND );
}

void DocxAttributeOutput::SectionPageBorders( const SwFrameFormat* pFormat, const SwFrameFormat* /*pFirstPageFormat*/ )
{
    // Output the margins

    const SvxBoxItem& rBox = pFormat->GetBox( );

    const SvxBorderLine* pLeft = rBox.GetLeft( );
    const SvxBorderLine* pTop = rBox.GetTop( );
    const SvxBorderLine* pRight = rBox.GetRight( );
    const SvxBorderLine* pBottom = rBox.GetBottom( );

    if ( pBottom || pTop || pLeft || pRight )
    {
        bool bExportDistanceFromPageEdge = false;
        if ( boxHasLineLargerThan31(rBox) )
        {
            // The distance is larger than '31'. This cannot be exported as 'distance from text'.
            // Instead - it should be exported as 'distance from page edge'.
            // This is based on http://wiki.openoffice.org/wiki/Writer/MSInteroperability/PageBorder
            // Specifically 'export case #2'
            bExportDistanceFromPageEdge = true;
        }

        // All distances are relative to the text margins
        m_pSerializer->startElementNS( XML_w, XML_pgBorders,
               FSNS( XML_w, XML_display ), "allPages",
               FSNS( XML_w, XML_offsetFrom ), bExportDistanceFromPageEdge ? "page" : "text",
               FSEND );

        OutputBorderOptions aOutputBorderOptions = lcl_getBoxBorderOptions();

        // Check if the distance is larger than 31 points
        aOutputBorderOptions.bCheckDistanceSize = true;

        // Check if there is a shadow item
        const SfxPoolItem* pItem = GetExport().HasItem( RES_SHADOW );
        if ( pItem )
        {
            const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
            aOutputBorderOptions.aShadowLocation = pShadowItem->GetLocation();
        }

        // By top margin, impl_borders() means the distance between the top of the page and the header frame.
        PageMargins aMargins = m_pageMargins;
        HdFtDistanceGlue aGlue(pFormat->GetAttrSet());
        if (aGlue.HasHeader())
            aMargins.nPageMarginTop = aGlue.dyaHdrTop;
        // Ditto for bottom margin.
        if (aGlue.HasFooter())
            aMargins.nPageMarginBottom = aGlue.dyaHdrBottom;

        std::map<SvxBoxItemLine, css::table::BorderLine2> aEmptyMap; // empty styles map
        impl_borders( m_pSerializer, rBox, aOutputBorderOptions, &aMargins,
                      aEmptyMap );

        m_pSerializer->endElementNS( XML_w, XML_pgBorders );
    }
}

void DocxAttributeOutput::SectionBiDi( bool bBiDi )
{
    if ( bBiDi )
        m_pSerializer->singleElementNS( XML_w, XML_bidi, FSEND );
}

static OString impl_NumberingType( sal_uInt16 nNumberingType )
{
    OString aType;

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
        case style::NumberingType::CHARS_HEBREW: aType = "hebrew1"; break;

        default:                            aType = "none";        break;
    }

    return aType;
}

// Converting Level Numbering Format Code to string
static OString impl_LevelNFC( sal_uInt16 nNumberingType , const SfxItemSet *pOutSet)
{
    OString aType;

    switch ( nNumberingType )
    {
        case style::NumberingType::CHARS_UPPER_LETTER:
        case style::NumberingType::CHARS_UPPER_LETTER_N:
        case style::NumberingType::CHARS_LOWER_LETTER:
        case style::NumberingType::CHARS_LOWER_LETTER_N:
        case style::NumberingType::ROMAN_UPPER:
        case style::NumberingType::ROMAN_LOWER:
        case style::NumberingType::ARABIC:
        case style::NumberingType::BITMAP:
        case style::NumberingType::CHAR_SPECIAL:
        case style::NumberingType::CHARS_HEBREW:
        case style::NumberingType::NUMBER_NONE:
            return impl_NumberingType( nNumberingType );
        case style::NumberingType::FULLWIDTH_ARABIC: aType="decimalFullWidth"; break;
        case style::NumberingType::TIAN_GAN_ZH: aType="ideographTraditional"; break;
        case style::NumberingType::DI_ZI_ZH: aType="ideographZodiac"; break;
        case style::NumberingType::NUMBER_LOWER_ZH:
            aType="taiwaneseCountingThousand";
            if (pOutSet) {
                const SvxLanguageItem rLang = static_cast<const SvxLanguageItem&>( pOutSet->Get( RES_CHRATR_CJK_LANGUAGE) );
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
        case style::NumberingType::NUMBER_HANGUL_KO: aType="koreanDigital";break;
        case style::NumberingType::NUMBER_UPPER_KO: aType="koreanLegal"; break;
        case style::NumberingType::CIRCLE_NUMBER: aType="decimalEnclosedCircle"; break;
        case style::NumberingType::CHARS_ARABIC: aType="arabicAlpha"; break;
        case style::NumberingType::CHARS_THAI: aType="thaiLetters"; break;
        case style::NumberingType::CHARS_PERSIAN: aType="hindiVowels"; break;
/*
        Fallback the rest to decimal.
        case style::NumberingType::NATIVE_NUMBERING:
        case style::NumberingType::HANGUL_CIRCLED_JAMO_KO:
        case style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO:
        case style::NumberingType::CHARS_GREEK_UPPER_LETTER:
        case style::NumberingType::CHARS_GREEK_LOWER_LETTER:
        case style::NumberingType::PAGE_DESCRIPTOR:
        case style::NumberingType::TRANSLITERATION:
        case style::NumberingType::CHARS_NEPALI:
        case style::NumberingType::CHARS_KHMER:
        case style::NumberingType::CHARS_LAO:
        case style::NumberingType::CHARS_TIBETAN:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_BG:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_BG:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_BG:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_BG:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_RU:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_RU:
        case style::NumberingType::CHARS_MYANMAR:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_SR:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_SR:
        case style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_SR:
        case style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_SR:
*/
        default:
            aType = "decimal";        break;
    }
    return aType;
}


void DocxAttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber )
{
    // FIXME Not called properly with page styles like "First Page"

    FastAttributeList* pAttr = FastSerializerHelper::createAttrList();

    // boost::none means no restart: then don't output that attribute if it is negative
    if ( oPageRestartNumber )
       pAttr->add( FSNS( XML_w, XML_start ), OString::number( oPageRestartNumber.get() ) );

    // nNumType corresponds to w:fmt. See WW8Export::GetNumId() for more precisions
    OString aFormat( impl_NumberingType( nNumType ) );
    if ( !aFormat.isEmpty() )
        pAttr->add( FSNS( XML_w, XML_fmt ), aFormat.getStr() );

    XFastAttributeListRef xAttrs( pAttr );
    m_pSerializer->singleElementNS( XML_w, XML_pgNumType, xAttrs );

    // see 2.6.12 pgNumType (Page Numbering Settings)
    OSL_TRACE( "TODO DocxAttributeOutput::SectionPageNumbering()" );
}

void DocxAttributeOutput::SectionType( sal_uInt8 nBreakCode )
{
    /*  break code:   0 No break, 1 New column
        2 New page, 3 Even page, 4 Odd page
        */
    const char* pType = nullptr;
    switch ( nBreakCode )
    {
        case 1:  pType = "nextColumn"; break;
        case 2:  pType = "nextPage";   break;
        case 3:  pType = "evenPage";   break;
        case 4:  pType = "oddPage";    break;
        default: pType = "continuous"; break;
    }

    if ( pType )
        m_pSerializer->singleElementNS( XML_w, XML_type,
                FSNS( XML_w, XML_val ), pType,
                FSEND );
}

void DocxAttributeOutput::TextVerticalAdjustment( const drawing::TextVerticalAdjust nVA )
{
    switch( nVA )
    {
        case drawing::TextVerticalAdjust_CENTER:
            m_pSerializer->singleElementNS( XML_w, XML_vAlign,
                FSNS( XML_w, XML_val ), "center", FSEND );
            break;
        case drawing::TextVerticalAdjust_BOTTOM:
            m_pSerializer->singleElementNS( XML_w, XML_vAlign,
                FSNS( XML_w, XML_val ), "bottom", FSEND );
            break;
        case drawing::TextVerticalAdjust_BLOCK:  //justify
            m_pSerializer->singleElementNS( XML_w, XML_vAlign,
                FSNS( XML_w, XML_val ), "both", FSEND );
            break;
        default:
            break;
    }
}

void DocxAttributeOutput::StartFont( const OUString& rFamilyName ) const
{
    m_pSerializer->startElementNS( XML_w, XML_font,
            FSNS( XML_w, XML_name ), OUStringToOString( rFamilyName, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
}

void DocxAttributeOutput::EndFont() const
{
    m_pSerializer->endElementNS( XML_w, XML_font );
}

void DocxAttributeOutput::FontAlternateName( const OUString& rName ) const
{
    m_pSerializer->singleElementNS( XML_w, XML_altName,
            FSNS( XML_w, XML_val ), OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
}

void DocxAttributeOutput::FontCharset( sal_uInt8 nCharSet, rtl_TextEncoding nEncoding ) const
{
    FastAttributeList* pAttr = FastSerializerHelper::createAttrList();

    OString aCharSet( OString::number( nCharSet, 16 ) );
    if ( aCharSet.getLength() == 1 )
        aCharSet = OString( "0" ) + aCharSet;
    pAttr->add( FSNS( XML_w, XML_val ), aCharSet.getStr());

    if( GetExport().GetFilter().getVersion( ) != oox::core::ECMA_DIALECT )
    {
        if( const char* charset = rtl_getMimeCharsetFromTextEncoding( nEncoding ))
            pAttr->add( FSNS( XML_w, XML_characterSet ), charset );
    }

    m_pSerializer->singleElementNS( XML_w, XML_charset, XFastAttributeListRef( pAttr ));
}

void DocxAttributeOutput::FontFamilyType( FontFamily eFamily ) const
{
    const char *pFamily = nullptr;
    switch ( eFamily )
    {
        case FAMILY_ROMAN:      pFamily = "roman"; break;
        case FAMILY_SWISS:      pFamily = "swiss"; break;
        case FAMILY_MODERN:     pFamily = "modern"; break;
        case FAMILY_SCRIPT:     pFamily = "script"; break;
        case FAMILY_DECORATIVE: pFamily = "decorative"; break;
        default:                pFamily = "auto"; break; // no font family
    }

    if ( pFamily )
        m_pSerializer->singleElementNS( XML_w, XML_family,
                FSNS( XML_w, XML_val ), pFamily,
                FSEND );
}

void DocxAttributeOutput::FontPitchType( FontPitch ePitch ) const
{
    const char *pPitch = nullptr;
    switch ( ePitch )
    {
        case PITCH_VARIABLE: pPitch = "variable"; break;
        case PITCH_FIXED:    pPitch = "fixed"; break;
        default:             pPitch = "default"; break; // no info about the pitch
    }

    if ( pPitch )
        m_pSerializer->singleElementNS( XML_w, XML_pitch,
                FSNS( XML_w, XML_val ), pPitch,
                FSEND );
}

void DocxAttributeOutput::EmbedFont( const OUString& name, FontFamily family, FontPitch pitch, rtl_TextEncoding encoding )
{
    if( !m_rExport.m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        return; // no font embedding with this document
    EmbedFontStyle( name, XML_embedRegular, family, ITALIC_NONE, WEIGHT_NORMAL, pitch, encoding );
    EmbedFontStyle( name, XML_embedBold, family, ITALIC_NONE, WEIGHT_BOLD, pitch, encoding );
    EmbedFontStyle( name, XML_embedItalic, family, ITALIC_NORMAL, WEIGHT_NORMAL, pitch, encoding );
    EmbedFontStyle( name, XML_embedBoldItalic, family, ITALIC_NORMAL, WEIGHT_BOLD, pitch, encoding );
}

static inline char toHexChar( int value )
{
    return value >= 10 ? value + 'A' - 10 : value + '0';
}

void DocxAttributeOutput::EmbedFontStyle( const OUString& name, int tag, FontFamily family, FontItalic italic,
    FontWeight weight, FontPitch pitch, rtl_TextEncoding encoding )
{
    // Embed font if at least viewing is allowed (in which case the opening app must check
    // the font license rights too and open either read-only or not use the font for editing).
    OUString fontUrl = EmbeddedFontsHelper::fontFileUrl( name, family, italic, weight, pitch, encoding,
        EmbeddedFontsHelper::ViewingAllowed );
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
        char buffer[ 4096 ];
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
            xOutStream->writeBytes( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( buffer ), readSize ));
        }
        xOutStream->closeOutput();
        OString relId = OUStringToOString( GetExport().GetFilter().addRelation( m_pSerializer->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/font",
            "fonts/font" + OUString::number( m_nextFontId ) + ".odttf" ), RTL_TEXTENCODING_UTF8 );
        EmbeddedFontRef ref;
        ref.relId = relId;
        ref.fontKey = fontKeyStr;
        fontFilesMap[ fontUrl ] = ref;
        ++m_nextFontId;
    }
    m_pSerializer->singleElementNS( XML_w, tag,
        FSNS( XML_r, XML_id ), fontFilesMap[ fontUrl ].relId,
        FSNS( XML_w, XML_fontKey ), fontFilesMap[ fontUrl ].fontKey,
        FSEND );
}

OString DocxAttributeOutput::TransHighlightColor( sal_uInt8 nIco )
{
    switch (nIco)
    {
        case 1: return OString("black"); break;
        case 2: return OString("blue"); break;
        case 3: return OString("cyan"); break;
        case 4: return OString("green"); break;
        case 5: return OString("magenta"); break;
        case 6: return OString("red"); break;
        case 7: return OString("yellow"); break;
        case 8: return OString("white"); break;
        case 9: return OString("darkBlue"); break;
        case 10: return OString("darkCyan"); break;
        case 11: return OString("darkGreen"); break;
        case 12: return OString("darkMagenta"); break;
        case 13: return OString("darkRed"); break;
        case 14: return OString("darkYellow"); break;
        case 15: return OString("darkGray"); break;
        case 16: return OString("lightGray"); break;
        default: return OString(); break;
    }
}

void DocxAttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule )
{
    // nId is the same both for abstract numbering definition as well as the
    // numbering definition itself
    // TODO check that this is actually true & fix if not ;-)
    OString aId( OString::number( nId ) );

    m_pSerializer->startElementNS( XML_w, XML_num,
            FSNS( XML_w, XML_numId ), aId.getStr(),
            FSEND );

    m_pSerializer->singleElementNS( XML_w, XML_abstractNumId,
            FSNS( XML_w, XML_val ), aId.getStr(),
            FSEND );

#if OSL_DEBUG_LEVEL > 1
    // TODO ww8 version writes this, anything to do about it here?
    if ( rRule.IsContinusNum() )
        OSL_TRACE( "TODO DocxAttributeOutput::NumberingDefinition()" );
#else
    (void) rRule; // to quiet the warning...
#endif

    m_pSerializer->endElementNS( XML_w, XML_num );
}

void DocxAttributeOutput::StartAbstractNumbering( sal_uInt16 nId )
{
    const SwNumRule* pRule = (*m_rExport.m_pUsedNumTable)[nId - 1];
    m_bExportingOutline = pRule && pRule->IsOutlineRule();
    m_pSerializer->startElementNS( XML_w, XML_abstractNum,
            FSNS( XML_w, XML_abstractNumId ), OString::number( nId ).getStr(),
            FSEND );
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
    m_pSerializer->startElementNS( XML_w, XML_lvl,
            FSNS( XML_w, XML_ilvl ), OString::number( nLevel ).getStr(),
            FSEND );

    // start with the nStart value. Do not write w:start if Numbered Lists
    // starts from zero.As it's an optional parameter.
    // refer ECMA 376 Second edition Part-1
    if(!(0 == nLevel && 0 == nStart))
    {
        m_pSerializer->singleElementNS( XML_w, XML_start,
                FSNS( XML_w, XML_val ), OString::number( nStart ).getStr(),
                FSEND );
    }

    if (m_bExportingOutline)
    {
        sal_uInt16 nId = m_rExport.m_pStyles->GetHeadingParagraphStyleId( nLevel );
        if ( nId != SAL_MAX_UINT16 )
            m_pSerializer->singleElementNS( XML_w, XML_pStyle ,
                FSNS( XML_w, XML_val ), m_rExport.m_pStyles->GetStyleId(nId).getStr(),
                FSEND );
    }
    // format
    OString aFormat( impl_LevelNFC( nNumberingType ,pOutSet) );

    if ( !aFormat.isEmpty() )
        m_pSerializer->singleElementNS( XML_w, XML_numFmt,
                FSNS( XML_w, XML_val ), aFormat.getStr(),
                FSEND );

    // suffix
    const char *pSuffix = nullptr;
    switch ( nFollow )
    {
        case 1:  pSuffix = "space";   break;
        case 2:  pSuffix = "nothing"; break;
        default: /*pSuffix = "tab";*/ break;
    }
    if ( pSuffix )
        m_pSerializer->singleElementNS( XML_w, XML_suff,
                FSNS( XML_w, XML_val ), pSuffix,
                FSEND );

    // text
    OUString aText( rNumberingString );
    OUStringBuffer aBuffer( aText.getLength() + WW8ListManager::nMaxLevel );

    const sal_Unicode *pPrev = aText.getStr();
    const sal_Unicode *pIt = aText.getStr();
    while ( pIt < aText.getStr() + aText.getLength() )
    {
        // convert the level values to %NUMBER form
        // (we don't use pNumLvlPos at all)
        // FIXME so far we support the ww8 limit of levels only
        if ( *pIt < sal_Unicode( WW8ListManager::nMaxLevel ) )
        {
            aBuffer.append( pPrev, pIt - pPrev );
            aBuffer.append( '%' );
            aBuffer.append( OUString::number( sal_Int32( *pIt ) + 1 ) );

            pPrev = pIt + 1;
        }
        ++pIt;
    }
    if ( pPrev < pIt )
        aBuffer.append( pPrev, pIt - pPrev );

    // If bullet char is empty, set lvlText as empty
    if ( aText.equals ( OUString(sal_Unicode(0)) ) && nNumberingType == SVX_NUM_CHAR_SPECIAL )
    {
        m_pSerializer->singleElementNS( XML_w, XML_lvlText, FSNS( XML_w, XML_val ), "", FSEND );
    }
    else
    {
        // Writer's "zero width space" suffix is necessary, so that LabelFollowedBy shows up, but Word doesn't require that.
        OUString aLevelText = aBuffer.makeStringAndClear();
        static OUString aZeroWidthSpace(static_cast<sal_Unicode>(0x200B));
        if (aLevelText == aZeroWidthSpace)
            aLevelText.clear();
        m_pSerializer->singleElementNS(XML_w, XML_lvlText, FSNS(XML_w, XML_val), aLevelText.toUtf8(), FSEND);
    }

    // bullet
    if (nNumberingType == SVX_NUM_BITMAP && pBrush)
    {
        int nIndex = m_rExport.GetGrfIndex(*pBrush);
        if (nIndex != -1)
        {
            m_pSerializer->singleElementNS(XML_w, XML_lvlPicBulletId,
                    FSNS(XML_w, XML_val), OString::number(nIndex).getStr(),
                    FSEND);
        }
    }

    // justification
    const char *pJc;
    bool ecmaDialect = ( m_rExport.GetFilter().getVersion() == oox::core::ECMA_DIALECT );
    switch ( eAdjust )
    {
        case SVX_ADJUST_CENTER: pJc = "center"; break;
        case SVX_ADJUST_RIGHT:  pJc = !ecmaDialect ? "end" : "right";  break;
        default:                pJc = !ecmaDialect ? "start" : "left";   break;
    }
    m_pSerializer->singleElementNS( XML_w, XML_lvlJc,
            FSNS( XML_w, XML_val ), pJc,
            FSEND );

    // indentation
    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );
    if( nListTabPos != 0 )
    {
        m_pSerializer->startElementNS( XML_w, XML_tabs, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_tab,
                FSNS( XML_w, XML_val ), "num",
                FSNS( XML_w, XML_pos ), OString::number( nListTabPos ).getStr(),
                FSEND );
        m_pSerializer->endElementNS( XML_w, XML_tabs );
    }

    sal_Int32 nToken = ecmaDialect ? XML_left : XML_start;
    m_pSerializer->singleElementNS( XML_w, XML_ind,
            FSNS( XML_w, nToken ), OString::number( nIndentAt ).getStr(),
            FSNS( XML_w, XML_hanging ), OString::number( -nFirstLineIndex ).getStr(),
            FSEND );
    m_pSerializer->endElementNS( XML_w, XML_pPr );

    // font
    if ( pOutSet )
    {
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );

        if ( pFont )
        {
            GetExport().GetId( *pFont ); // ensure font info is written to fontTable.xml
            OString aFamilyName( OUStringToOString( OUString( pFont->GetFamilyName() ), RTL_TEXTENCODING_UTF8 ) );
            m_pSerializer->singleElementNS( XML_w, XML_rFonts,
                    FSNS( XML_w, XML_ascii ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_hAnsi ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_cs ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_hint ), "default",
                    FSEND );
        }
        m_rExport.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rExport.m_bExportModeRTF );

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
        case SVX_CASEMAP_KAPITAELCHEN:
            m_pSerializer->singleElementNS( XML_w, XML_smallCaps, FSEND );
            break;
        case SVX_CASEMAP_VERSALIEN:
            m_pSerializer->singleElementNS( XML_w, XML_caps, FSEND );
            break;
        default: // Something that ooxml does not support
            m_pSerializer->singleElementNS( XML_w, XML_smallCaps, FSNS( XML_w, XML_val ), "false", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_caps, FSNS( XML_w, XML_val ), "false", FSEND );
            break;
    }
}

void DocxAttributeOutput::CharColor( const SvxColorItem& rColor )
{
    const Color aColor( rColor.GetValue() );
    OString aColorString;

    aColorString = msfilter::util::ConvertColor( aColor );

    const char* pExistingValue(nullptr);
    if (m_pColorAttrList.is() && m_pColorAttrList->getAsChar(FSNS(XML_w, XML_val), pExistingValue))
    {
        assert(aColorString.equalsL(pExistingValue, rtl_str_getLength(pExistingValue)));
        return;
    }

    AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_val ), aColorString.getStr() );
}

void DocxAttributeOutput::CharContour( const SvxContourItem& rContour )
{
    if ( rContour.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_outline, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_outline, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharCrossedOut( const SvxCrossedOutItem& rCrossedOut )
{
    switch ( rCrossedOut.GetStrikeout() )
    {
        case STRIKEOUT_DOUBLE:
            m_pSerializer->singleElementNS( XML_w, XML_dstrike, FSEND );
            break;
        case STRIKEOUT_NONE:
            m_pSerializer->singleElementNS( XML_w, XML_dstrike, FSNS( XML_w, XML_val ), "false", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_strike, FSNS( XML_w, XML_val ), "false", FSEND );
            break;
        default:
            m_pSerializer->singleElementNS( XML_w, XML_strike, FSEND );
            break;
    }
}

void DocxAttributeOutput::CharEscapement( const SvxEscapementItem& rEscapement )
{
    OString sIss;
    short nEsc = rEscapement.GetEsc(), nProp = rEscapement.GetProp();
    if ( !nEsc )
    {
        sIss = OString( "baseline" );
        nEsc = 0;
        nProp = 100;
    }
    else if ( DFLT_ESC_PROP == nProp )
    {
        if ( DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            sIss = OString( "subscript" );
        else if ( DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc )
            sIss = OString( "superscript" );
    }

    if ( !sIss.isEmpty() )
        m_pSerializer->singleElementNS( XML_w, XML_vertAlign,
           FSNS( XML_w, XML_val ), sIss.getStr(), FSEND );

    const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(m_rExport.GetItem(RES_CHRATR_FONTSIZE));
    if (sIss.isEmpty() || sIss.match(OString("baseline")))
    {
        long nHeight = rItem.GetHeight();
        OString sPos = OString::number( ( nHeight * nEsc + 500 ) / 1000 );
        m_pSerializer->singleElementNS( XML_w, XML_position,
                FSNS( XML_w, XML_val ), sPos.getStr( ), FSEND );

        if( ( 100 != nProp || sIss.match( OString( "baseline" ) ) ) && !m_rExport.m_bFontSizeWritten )
        {
            OString sSize = OString::number( ( nHeight * nProp + 500 ) / 1000 );
                m_pSerializer->singleElementNS( XML_w, XML_sz,
                    FSNS( XML_w, XML_val ), sSize.getStr( ), FSEND );
        }
    }
}

void DocxAttributeOutput::CharFont( const SvxFontItem& rFont)
{
    GetExport().GetId( rFont ); // ensure font info is written to fontTable.xml
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    if (!sFontNameUtf8.isEmpty())
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
            m_pSerializer->singleElementNS( XML_w, XML_sz, FSNS( XML_w, XML_val ), fontSize.getStr(), FSEND );
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            m_pSerializer->singleElementNS( XML_w, XML_szCs, FSNS( XML_w, XML_val ), fontSize.getStr(), FSEND );
            break;
    }
}

void DocxAttributeOutput::CharKerning( const SvxKerningItem& rKerning )
{
    OString aKerning = OString::number(  rKerning.GetValue() );
    m_pSerializer->singleElementNS( XML_w, XML_spacing, FSNS(XML_w, XML_val), aKerning.getStr(), FSEND );
}

void DocxAttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    OString aLanguageCode( OUStringToOString(
                LanguageTag( rLanguage.GetLanguage()).getBcp47(),
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
        m_pSerializer->singleElementNS( XML_w, XML_i, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_i, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    if ( rShadow.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_shadow, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_shadow, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    const char *pUnderlineValue;

    switch ( rUnderline.GetLineStyle() )
    {
        case UNDERLINE_SINGLE:         pUnderlineValue = "single";          break;
        case UNDERLINE_BOLD:           pUnderlineValue = "thick";           break;
        case UNDERLINE_DOUBLE:         pUnderlineValue = "double";          break;
        case UNDERLINE_DOTTED:         pUnderlineValue = "dotted";          break;
        case UNDERLINE_DASH:           pUnderlineValue = "dash";            break;
        case UNDERLINE_DASHDOT:        pUnderlineValue = "dotDash";         break;
        case UNDERLINE_DASHDOTDOT:     pUnderlineValue = "dotDotDash";      break;
        case UNDERLINE_WAVE:           pUnderlineValue = "wave";            break;
        case UNDERLINE_BOLDDOTTED:     pUnderlineValue = "dottedHeavy";     break;
        case UNDERLINE_BOLDDASH:       pUnderlineValue = "dashedHeavy";     break;
        case UNDERLINE_LONGDASH:       pUnderlineValue = "dashLongHeavy";   break;
        case UNDERLINE_BOLDLONGDASH:   pUnderlineValue = "dashLongHeavy";   break;
        case UNDERLINE_BOLDDASHDOT:    pUnderlineValue = "dashDotHeavy";    break;
        case UNDERLINE_BOLDDASHDOTDOT: pUnderlineValue = "dashDotDotHeavy"; break;
        case UNDERLINE_BOLDWAVE:       pUnderlineValue = "wavyHeavy";       break;
        case UNDERLINE_DOUBLEWAVE:     pUnderlineValue = "wavyDouble";      break;
        case UNDERLINE_NONE:           // fall through
        default:                       pUnderlineValue = "none";            break;
    }

    Color aUnderlineColor = rUnderline.GetColor();
    bool  bUnderlineHasColor = aUnderlineColor.GetTransparency() == 0;
    if (bUnderlineHasColor)
    {
        // Underline has a color
        m_pSerializer->singleElementNS( XML_w, XML_u,
                                        FSNS( XML_w, XML_val ), pUnderlineValue,
                                        FSNS( XML_w, XML_color ), msfilter::util::ConvertColor( aUnderlineColor ).getStr(),
                                    FSEND );
    }
    else
    {
        // Underline has no color
        m_pSerializer->singleElementNS( XML_w, XML_u, FSNS( XML_w, XML_val ), pUnderlineValue, FSEND );
    }
}

void DocxAttributeOutput::CharWeight( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_b, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_b, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharAutoKern( const SvxAutoKernItem& )
{
    OSL_TRACE( "TODO DocxAttributeOutput::CharAutoKern()" );
}

void DocxAttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    if ( rBlink.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS( XML_w, XML_val ), "blinkBackground", FSEND );
    else
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS( XML_w, XML_val ), "none", FSEND );
}

#define MSWORD_CH_SHADING_FILL "FFFFFF" // The attribute w:fill of w:shd, for MS-Word's character shading,
#define MSWORD_CH_SHADING_COLOR "auto" // The attribute w:color of w:shd, for MS-Word's character shading,
#define MSWORD_CH_SHADING_VAL "pct15" // The attribute w:value of w:shd, for MS-Word's character shading,

void DocxAttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    // Check if the brush shading pattern is 'PCT15'. If so - write it back to the DOCX
    if (rBrush.GetShadingValue() == ShadingPattern::PCT15)
    {
        m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_val ), MSWORD_CH_SHADING_VAL,
            FSNS( XML_w, XML_color ), MSWORD_CH_SHADING_COLOR,
            FSNS( XML_w, XML_fill ), MSWORD_CH_SHADING_FILL,
            FSEND );
    }
    else
    {
        m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_fill ), msfilter::util::ConvertColor( rBrush.GetColor() ).getStr(),
            FSNS( XML_w, XML_val ), "clear",
            FSEND );
    }
}

void DocxAttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_eastAsia ), sFontNameUtf8.getStr() );
}

void DocxAttributeOutput::CharPostureCJK( const SvxPostureItem& rPosture )
{
    if ( rPosture.GetPosture() != ITALIC_NONE )
        m_pSerializer->singleElementNS( XML_w, XML_i, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_i, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharWeightCJK( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_b, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_b, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharFontCTL( const SvxFontItem& rFont )
{
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_cs ), sFontNameUtf8.getStr() );

}

void DocxAttributeOutput::CharPostureCTL( const SvxPostureItem& rPosture)
{
    if ( rPosture.GetPosture() != ITALIC_NONE )
        m_pSerializer->singleElementNS( XML_w, XML_iCs, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_iCs, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_bCs, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_bCs, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharBidiRTL( const SfxPoolItem& )
{
}

void DocxAttributeOutput::CharIdctHint( const SfxPoolItem& )
{
}

void DocxAttributeOutput::CharRotate( const SvxCharRotateItem& rRotate)
{
    // Not rotated or we the rotation already handled?
    if ( !rRotate.GetValue() || m_bBtLr || m_rExport.SdrExporter().getFrameBtLr())
        return;

    AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_vert ), "true" );

    if (rRotate.IsFitToLine())
        AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_vertCompress ), "true" );
}

void DocxAttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
{
    const char *pEmphasis;

    switch ( rEmphasisMark.GetValue() )
    {
    default:
    case EMPHASISMARK_NONE:
        pEmphasis = "none";
        break;
    case EMPHASISMARK_DOT | EMPHASISMARK_POS_ABOVE:
        pEmphasis = "dot";
        break;
    case EMPHASISMARK_ACCENT | EMPHASISMARK_POS_ABOVE:
        pEmphasis = "comma";
        break;
    case EMPHASISMARK_CIRCLE | EMPHASISMARK_POS_ABOVE:
        pEmphasis = "circle";
        break;
    case EMPHASISMARK_DOT|EMPHASISMARK_POS_BELOW:
        pEmphasis = "underDot";
        break;
    }

    m_pSerializer->singleElementNS( XML_w, XML_em, FSNS( XML_w, XML_val ), pEmphasis, FSEND );
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
        sBracket = const_cast<sal_Char *>("curly");
    else if ((cStart == '<') || (cEnd == '>'))
        sBracket = const_cast<sal_Char *>("angle");
    else if ((cStart == '[') || (cEnd == ']'))
        sBracket = const_cast<sal_Char *>("square");
    else
        sBracket = const_cast<sal_Char *>("round");
    AddToAttrList( m_pEastAsianLayoutAttrList, FSNS( XML_w, XML_combineBrackets ), sBracket.getStr() );
}

void DocxAttributeOutput::CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth )
{
    m_pSerializer->singleElementNS( XML_w, XML_w,
            FSNS( XML_w, XML_val ), OString::number( rScaleWidth.GetValue() ).getStr(), FSEND );
}

void DocxAttributeOutput::CharRelief( const SvxCharReliefItem& rRelief )
{
    switch ( rRelief.GetValue() )
    {
        case RELIEF_EMBOSSED:
            m_pSerializer->singleElementNS( XML_w, XML_emboss, FSEND );
            break;
        case RELIEF_ENGRAVED:
            m_pSerializer->singleElementNS( XML_w, XML_imprint, FSEND );
            break;
        default:
            m_pSerializer->singleElementNS( XML_w, XML_emboss, FSNS( XML_w, XML_val ), "false", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_imprint, FSNS( XML_w, XML_val ), "false", FSEND );
            break;
    }
}

void DocxAttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    if ( rHidden.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_vanish, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_vanish, FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::CharBorder(
    const SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow )
{
    impl_borderLine( m_pSerializer, XML_bdr, pAllBorder, nDist, bShadow );
}

void DocxAttributeOutput::CharHighlight( const SvxBrushItem& rHighlight )
{
    const OString sColor = TransHighlightColor( msfilter::util::TransColToIco(rHighlight.GetColor()) );
    if ( !sColor.isEmpty() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_highlight,
            FSNS( XML_w, XML_val ), sColor.getStr(), FSEND );
    }
}

void DocxAttributeOutput::TextINetFormat( const SwFormatINetFormat& rLink )
{
    const SwTextINetFormat* pINetFormat = rLink.GetTextINetFormat();
    const SwCharFormat* pCharFormat = pINetFormat->GetCharFormat();

    OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(pCharFormat)));

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

void DocxAttributeOutput::TextCharFormat( const SwFormatCharFormat& rCharFormat )
{
    OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(rCharFormat.GetCharFormat())));

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

void DocxAttributeOutput::RefField( const SwField&  rField, const OUString& rRef )
{
    sal_uInt16 nType = rField.GetTyp( )->Which( );
    if ( nType == RES_GETEXPFLD )
    {
        OUString sCmd = FieldString( ww::eREF );
        sCmd += "\"" + rRef + "\" ";

        m_rExport.OutputField( &rField, ww::eREF, sCmd );
    }

    // There is nothing to do here for the set fields
}

void DocxAttributeOutput::HiddenField( const SwField& /*rField*/ )
{
    OSL_TRACE( "TODO DocxAttributeOutput::HiddenField()" );
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
    m_postitFields.push_back(std::make_pair(pPostItField, nId));
}

void DocxAttributeOutput::WritePostitFieldReference()
{
    while( m_postitFieldsMaxId < m_postitFields.size())
    {
        OString idstr = OString::number(m_postitFields[m_postitFieldsMaxId].second);

        // In case this file is inside annotation marks, we want to write the
        // comment reference after the annotation mark is closed, not here.
        OString idname = OUStringToOString(m_postitFields[m_postitFieldsMaxId].first->GetName(), RTL_TEXTENCODING_UTF8);
        std::map< OString, sal_Int32 >::iterator it = m_rOpenedAnnotationMarksIds.find( idname );
        if ( it == m_rOpenedAnnotationMarksIds.end(  ) )
            m_pSerializer->singleElementNS( XML_w, XML_commentReference, FSNS( XML_w, XML_id ), idstr.getStr(), FSEND );
        ++m_postitFieldsMaxId;
    }
}

void DocxAttributeOutput::WritePostitFields()
{
    for( size_t i = 0;
         i < m_postitFields.size();
         ++i )
    {
        OString idstr = OString::number( m_postitFields[ i ].second);
        const SwPostItField* f = m_postitFields[ i ].first;
        m_pSerializer->startElementNS( XML_w, XML_comment, FSNS( XML_w, XML_id ), idstr.getStr(),
            FSNS( XML_w, XML_author ), OUStringToOString( f->GetPar1(), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSNS( XML_w, XML_date ), DateTimeToOString(f->GetDateTime()).getStr(),
            FSNS( XML_w, XML_initials ), OUStringToOString( f->GetInitials(), RTL_TEXTENCODING_UTF8 ).getStr(), FSEND );
        // Check for the text object existing, it seems that it can be NULL when saving a newly created
        // comment without giving focus back to the main document. As GetText() is empty in that case as well,
        // that is probably a bug in the Writer core.
        if( f->GetTextObject() != nullptr )
            GetExport().WriteOutliner( *f->GetTextObject(), TXT_ATN );
        m_pSerializer->endElementNS( XML_w, XML_comment );
    }
}

bool DocxAttributeOutput::DropdownField( const SwField* pField )
{
    bool bExpand = false;

    ww::eField eType = ww::eFORMDROPDOWN;
    OUString sCmd = FieldString( eType  );
    GetExport( ).OutputField( pField, eType, sCmd );

    return bExpand;
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
    m_pSerializer->startElementNS( XML_w, XML_sdt, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_sdtPr, FSEND );
    if( !pField->GetPar2().isEmpty())
        m_pSerializer->singleElementNS( XML_w, XML_alias,
            FSNS( XML_w, XML_val ), OUStringToOString( pField->GetPar2(), RTL_TEXTENCODING_UTF8 ), FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_temporary, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_showingPlcHdr, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_text, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_sdtPr );
    m_pSerializer->startElementNS( XML_w, XML_sdtContent, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
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
    OUString sCmd;
    m_rExport.OutputField( pField, ww::eUNKNOWN, sCmd );
}

void DocxAttributeOutput::WriteField_Impl( const SwField* pField, ww::eField eType, const OUString& rFieldCmd, sal_uInt8 nMode )
{
    struct FieldInfos infos;
    if (pField)
        infos.pField.reset(pField->CopyField());
    infos.sCmd = rFieldCmd;
    infos.eType = eType;
    infos.bClose = WRITEFIELD_CLOSE & nMode;
    infos.bOpen = WRITEFIELD_START & nMode;
    m_Fields.push_back( infos );

    if ( pField )
    {
        sal_uInt16 nType = pField->GetTyp( )->Which( );
        sal_uInt16 nSubType = pField->GetSubType();

        // TODO Any other field types here ?
        if ( ( nType == RES_SETEXPFLD ) && ( nSubType & nsSwGetSetExpType::GSE_STRING ) )
        {
            const SwSetExpField *pSet = static_cast<const SwSetExpField*>( pField );
            m_sFieldBkm = pSet->GetPar1( );
        }
        else if ( nType == RES_DROPDOWN )
        {
            const SwDropDownField* pDropDown = static_cast<const SwDropDownField*>( pField );
            m_sFieldBkm = pDropDown->GetName( );
        }
    }
}

void DocxAttributeOutput::WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark )
{
    if ( !m_Fields.empty() )
        m_Fields.begin()->pFieldmark = &rFieldmark;
}

void DocxAttributeOutput::WriteBookmarks_Impl( std::vector< OUString >& rStarts,
        std::vector< OUString >& rEnds )
{
    for ( std::vector< OUString >::const_iterator it = rStarts.begin(), end = rStarts.end(); it != end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rBookmarksStart.push_back( rName );
    }
    rStarts.clear();

    for ( std::vector< OUString >::const_iterator it = rEnds.begin(), end = rEnds.end(); it != end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rBookmarksEnd.push_back( rName );
    }
    rEnds.clear();
}

void DocxAttributeOutput::WriteAnnotationMarks_Impl( std::vector< OUString >& rStarts,
        std::vector< OUString >& rEnds )
{
    for ( std::vector< OUString >::const_iterator it = rStarts.begin(), end = rStarts.end(); it != end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rAnnotationMarksStart.push_back( rName );
    }
    rStarts.clear();

    for ( std::vector< OUString >::const_iterator it = rEnds.begin(), end = rEnds.end(); it != end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rAnnotationMarksEnd.push_back( rName );
    }
    rEnds.clear();
}

void DocxAttributeOutput::TextFootnote_Impl( const SwFormatFootnote& rFootnote )
{
    const SwEndNoteInfo& rInfo = rFootnote.IsEndNote()?
        m_rExport.m_pDoc->GetEndNoteInfo(): m_rExport.m_pDoc->GetFootnoteInfo();

    // footnote/endnote run properties
    const SwCharFormat* pCharFormat = rInfo.GetAnchorCharFormat( *m_rExport.m_pDoc );

    OString aStyleId(m_rExport.m_pStyles->GetStyleId(m_rExport.GetId(pCharFormat)));

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );

    // remember the footnote/endnote to
    // 1) write the footnoteReference/endnoteReference in EndRunProperties()
    // 2) be able to dump them all to footnotes.xml/endnotes.xml
    if ( !rFootnote.IsEndNote() )
        m_pFootnotesList->add( rFootnote );
    else
        m_pEndnotesList->add( rFootnote );
}

void DocxAttributeOutput::FootnoteEndnoteReference()
{
    sal_Int32 nId;
    const SwFormatFootnote *pFootnote = m_pFootnotesList->getCurrent( nId );

    // both cannot be set at the same time - if they are, it's a bug
    if ( !pFootnote )
        pFootnote = m_pEndnotesList->getCurrent( nId );

    if ( !pFootnote )
        return;

    sal_Int32 nToken = pFootnote->IsEndNote()? XML_endnoteReference: XML_footnoteReference;

    // write it
    if ( pFootnote->GetNumStr().isEmpty() )
    {
        // autonumbered
        m_pSerializer->singleElementNS( XML_w, nToken,
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(),
                FSEND );
    }
    else
    {
        // not autonumbered
        m_pSerializer->singleElementNS( XML_w, nToken,
                FSNS( XML_w, XML_customMarkFollows ), "1",
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(),
                FSEND );

        RunText( pFootnote->GetNumStr() );
    }
}

void DocxAttributeOutput::FootnotesEndnotes( bool bFootnotes )
{
    m_setFootnote = true;
    const FootnotesVector& rVector = bFootnotes? m_pFootnotesList->getVector(): m_pEndnotesList->getVector();

    sal_Int32 nBody = bFootnotes? XML_footnotes: XML_endnotes;
    sal_Int32 nItem = bFootnotes? XML_footnote:  XML_endnote;

    m_pSerializer->startElementNS( XML_w, nBody, DocxExport::MainXmlNamespaces() );

    sal_Int32 nIndex = 0;

    // separator
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::number( nIndex++ ).getStr(),
            FSNS( XML_w, XML_type ), "separator",
            FSEND );
    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

    bool bSeparator = true;
    if (bFootnotes)
    {
        const SwPageFootnoteInfo& rFootnoteInfo = m_rExport.m_pDoc->GetPageDesc(0).GetFootnoteInfo();
        // Request a separator only in case the width is larger than zero.
        bSeparator = double(rFootnoteInfo.GetWidth()) > 0;
    }

    if (bSeparator)
        m_pSerializer->singleElementNS( XML_w, XML_separator, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // separator
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::number( nIndex++ ).getStr(),
            FSNS( XML_w, XML_type ), "continuationSeparator",
            FSEND );
    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_continuationSeparator, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // if new special ones are added, update also WriteFootnoteEndnotePr()

    // footnotes/endnotes themselves
    for ( FootnotesVector::const_iterator i = rVector.begin(); i != rVector.end(); ++i, ++nIndex )
    {
        m_pSerializer->startElementNS( XML_w, nItem,
                FSNS( XML_w, XML_id ), OString::number( nIndex ).getStr(),
                FSEND );

        const SwNodeIndex* pIndex = (*i)->GetTextFootnote()->GetStartNode();
        // tag required at the start of each footnote/endnote
        m_footnoteEndnoteRefTag = bFootnotes ? XML_footnoteRef : XML_endnoteRef;

        m_rExport.WriteSpecialText( pIndex->GetIndex() + 1,
                pIndex->GetNode().EndOfSectionIndex(),
                bFootnotes? TXT_FTN: TXT_EDN );

        m_pSerializer->endElementNS( XML_w, nItem );
    }

    m_pSerializer->endElementNS( XML_w, nBody );

}

void DocxAttributeOutput::WriteFootnoteEndnotePr( ::sax_fastparser::FSHelperPtr fs, int tag,
    const SwEndNoteInfo& info, int listtag )
{
    fs->startElementNS( XML_w, tag, FSEND );
    const char* fmt = nullptr;
    switch( info.aFormat.GetNumberingType())
    {
        case SVX_NUM_CHARS_UPPER_LETTER_N: // fall through, map to upper letters
        case SVX_NUM_CHARS_UPPER_LETTER:
            fmt = "upperLetter";
            break;
        case SVX_NUM_CHARS_LOWER_LETTER_N: // fall through, map to lower letters
        case SVX_NUM_CHARS_LOWER_LETTER:
            fmt = "lowerLetter";
            break;
        case SVX_NUM_ROMAN_UPPER:
            fmt = "upperRoman";
            break;
        case SVX_NUM_ROMAN_LOWER:
            fmt = "lowerRoman";
            break;
        case SVX_NUM_ARABIC:
            fmt = "decimal";
            break;
        case SVX_NUM_NUMBER_NONE:
            fmt = "none";
            break;
        case SVX_NUM_CHAR_SPECIAL:
            fmt = "bullet";
            break;
        case SVX_NUM_PAGEDESC:
        case SVX_NUM_BITMAP:
        default:
            break; // no format
    }
    if( fmt != nullptr )
        fs->singleElementNS( XML_w, XML_numFmt, FSNS( XML_w, XML_val ), fmt, FSEND );
    if( info.nFootnoteOffset != 0 )
        fs->singleElementNS( XML_w, XML_numStart, FSNS( XML_w, XML_val ),
            OString::number( info.nFootnoteOffset + 1).getStr(), FSEND );
    if( listtag != 0 ) // we are writing to settings.xml, write also special footnote/endnote list
    { // there are currently only two hardcoded ones ( see FootnotesEndnotes())
        fs->singleElementNS( XML_w, listtag, FSNS( XML_w, XML_id ), "0", FSEND );
        fs->singleElementNS( XML_w, listtag, FSNS( XML_w, XML_id ), "1", FSEND );
    }
    fs->endElementNS( XML_w, tag );
}

void DocxAttributeOutput::SectFootnoteEndnotePr()
{
    if( HasFootnotes())
        WriteFootnoteEndnotePr( m_pSerializer, XML_footnotePr, m_rExport.m_pDoc->GetFootnoteInfo(), 0 );
    if( HasEndnotes())
        WriteFootnoteEndnotePr( m_pSerializer, XML_endnotePr, m_rExport.m_pDoc->GetEndNoteInfo(), 0 );
}

void DocxAttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    if ( nSpace < 0 )
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "exact",
                FSNS( XML_w, XML_line ), OString::number( -nSpace ).getStr() );
    }
    else if( nMulti )
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "auto",
                FSNS( XML_w, XML_line ), OString::number( nSpace ).getStr() );
    }
    else if ( nSpace > 0 )
    {
        AddToAttrList( m_pParagraphSpacingAttrList, 2,
                FSNS( XML_w, XML_lineRule ), "atLeast",
                FSNS( XML_w, XML_line ), OString::number( nSpace ).getStr() );
    }
    else
        AddToAttrList( m_pParagraphSpacingAttrList, FSNS( XML_w, XML_lineRule ), "auto" );
}

void DocxAttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    const char *pAdjustString;

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    const SfxItemSet* pItems = GetExport().GetCurItemSet();
    const SvxFrameDirectionItem* rFrameDir = pItems?
        static_cast< const SvxFrameDirectionItem* >( pItems->GetItem( RES_FRAMEDIR ) ): nullptr;

    short nDir = FRMDIR_ENVIRONMENT;
    if( rFrameDir != nullptr )
        nDir = rFrameDir->GetValue();
    if ( nDir == FRMDIR_ENVIRONMENT )
        nDir = GetExport( ).GetDefaultFrameDirection( );
    bool bRtl = ( nDir == FRMDIR_HORI_RIGHT_TOP );

    switch ( rAdjust.GetAdjust() )
    {
        case SVX_ADJUST_LEFT:
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
        case SVX_ADJUST_RIGHT:
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
        case SVX_ADJUST_BLOCKLINE:
        case SVX_ADJUST_BLOCK:
            pAdjustString = "both";
            break;
        case SVX_ADJUST_CENTER:
            pAdjustString = "center";
            break;
        default:
            return; // not supported attribute
    }
    m_pSerializer->singleElementNS( XML_w, XML_jc, FSNS( XML_w, XML_val ), pAdjustString, FSEND );
}

void DocxAttributeOutput::ParaSplit( const SvxFormatSplitItem& rSplit )
{
    if (rSplit.GetValue())
        m_pSerializer->singleElementNS( XML_w, XML_keepLines, FSNS( XML_w, XML_val ), "false", FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_keepLines, FSEND );
}

void DocxAttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
    if (rWidows.GetValue())
        m_pSerializer->singleElementNS( XML_w, XML_widowControl, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_widowControl, FSNS( XML_w, XML_val ), "false", FSEND );
}

static void impl_WriteTabElement( FSHelperPtr pSerializer,
                                  const SvxTabStop& rTab, long /* nCurrentLeft */ )
{
    FastAttributeList *pTabElementAttrList = FastSerializerHelper::createAttrList();

    switch (rTab.GetAdjustment())
    {
    case SVX_TAB_ADJUST_RIGHT:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "right" ) );
        break;
    case SVX_TAB_ADJUST_DECIMAL:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "decimal" ) );
        break;
    case SVX_TAB_ADJUST_CENTER:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "center" ) );
        break;
    case SVX_TAB_ADJUST_DEFAULT:
    case SVX_TAB_ADJUST_LEFT:
    default:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( "left" ) );
        break;
    }

    // Because GetTabPos already includes indent, we don't need to add nCurrentLeft (CurrentLeft is indentation information)
    //pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::valueOf( rTab.GetTabPos() + nCurrentLeft ) );
    pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::number( rTab.GetTabPos()                ) );

    sal_Unicode cFillChar = rTab.GetFill();

    if ('.' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "dot" ) );
    else if ( '-' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "hyphen" ) );
    else if ( sal_Unicode(0xB7) == cFillChar ) // middle dot
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "middleDot" ) );
    else if ( '_' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "underscore" ) );
    else
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( "none" ) );

    pSerializer->singleElementNS( XML_w, XML_tab, pTabElementAttrList );
}

void DocxAttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStop )
{
    const SfxPoolItem* pLR = m_rExport.HasItem( RES_LR_SPACE );
    long nCurrentLeft = pLR ? static_cast<const SvxLRSpaceItem*>(pLR)->GetTextLeft() : 0;

    sal_uInt16 nCount = rTabStop.Count();

    // <w:tabs> must contain at least one <w:tab>, so don't write it empty
    if( nCount == 0 )
        return;
    if( nCount == 1 && rTabStop[ 0 ].GetAdjustment() == SVX_TAB_ADJUST_DEFAULT )
    {
        GetExport().setDefaultTabStop( rTabStop[ 0 ].GetTabPos());
        return;
    }

    m_pSerializer->startElementNS( XML_w, XML_tabs, FSEND );

    for (sal_uInt16 i = 0; i < nCount; i++ )
    {
        if( rTabStop[i].GetAdjustment() != SVX_TAB_ADJUST_DEFAULT )
            impl_WriteTabElement( m_pSerializer, rTabStop[i], nCurrentLeft );
        else
            GetExport().setDefaultTabStop( rTabStop[i].GetTabPos());
    }

    m_pSerializer->endElementNS( XML_w, XML_tabs );
}

void DocxAttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    m_pSerializer->singleElementNS( XML_w, XML_suppressAutoHyphens,
            FSNS( XML_w, XML_val ), OString::boolean( !rHyphenZone.IsHyphen() ),
            FSEND );
}

void DocxAttributeOutput::ParaNumRule_Impl( const SwTextNode* /*pTextNd*/, sal_Int32 nLvl, sal_Int32 nNumId )
{
    if ( USHRT_MAX != nNumId )
    {
        m_pSerializer->startElementNS( XML_w, XML_numPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_ilvl, FSNS( XML_w, XML_val ), OString::number( nLvl).getStr(), FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_numId, FSNS( XML_w, XML_val ), OString::number( nNumId).getStr(), FSEND );
        m_pSerializer->endElementNS( XML_w, XML_numPr );
    }
}

void DocxAttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    m_pSerializer->singleElementNS( XML_w, XML_autoSpaceDE,
           FSNS( XML_w, XML_val ), OString::boolean( rScriptSpace.GetValue() ),
           FSEND );
}

void DocxAttributeOutput::ParaHangingPunctuation( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_overflowPunct,
           FSNS( XML_w, XML_val ), OString::boolean( rItem.GetValue() ),
           FSEND );
}

void DocxAttributeOutput::ParaForbiddenRules( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_kinsoku,
           FSNS( XML_w, XML_val ), OString::boolean( rItem.GetValue() ),
           FSEND );
}

void DocxAttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    const char *pAlignString;

    switch ( rAlign.GetValue() )
    {
        case SvxParaVertAlignItem::BASELINE:
            pAlignString = "baseline";
            break;
        case SvxParaVertAlignItem::TOP:
            pAlignString = "top";
            break;
        case SvxParaVertAlignItem::CENTER:
            pAlignString = "center";
            break;
        case SvxParaVertAlignItem::BOTTOM:
            pAlignString = "bottom";
            break;
        case SvxParaVertAlignItem::AUTOMATIC:
            pAlignString = "auto";
            break;
        default:
            return; // not supported attribute
    }
    m_pSerializer->singleElementNS( XML_w, XML_textAlignment, FSNS( XML_w, XML_val ), pAlignString, FSEND );
}

void DocxAttributeOutput::ParaSnapToGrid( const SvxParaGridItem& rGrid )
{
    m_pSerializer->singleElementNS( XML_w, XML_snapToGrid,
            FSNS( XML_w, XML_val ), OString::boolean( rGrid.GetValue() ),
            FSEND );
}

void DocxAttributeOutput::FormatFrameSize( const SwFormatFrameSize& rSize )
{
    if (m_rExport.SdrExporter().getTextFrameSyntax() && m_rExport.SdrExporter().getFlyFrameSize())
    {
        const Size* pSize = m_rExport.SdrExporter().getFlyFrameSize();
        m_rExport.SdrExporter().getTextFrameStyle().append(";width:").append(double(pSize->Width()) / 20);
        m_rExport.SdrExporter().getTextFrameStyle().append("pt;height:").append(double(pSize->Height()) / 20).append("pt");
    }
    else if (m_rExport.SdrExporter().getDMLTextFrameSyntax())
    {
    }
    else if ( m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == ATT_FIX_SIZE )
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(),
                    FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ).getStr() );

        if ( rSize.GetHeight() )
        {
            OString sRule( "exact" );
            if ( rSize.GetHeightSizeType() == ATT_MIN_SIZE )
                sRule = OString( "atLeast" );
            AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), 2,
                    FSNS( XML_w, XML_hRule ), sRule.getStr(),
                    FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ).getStr() );
        }
    }
    else if ( m_rExport.m_bOutPageDescs )
    {
        FastAttributeList *attrList = FastSerializerHelper::createAttrList( );
        if ( m_rExport.m_pAktPageDesc->GetLandscape( ) )
            attrList->add( FSNS( XML_w, XML_orient ), "landscape" );

        attrList->add( FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ) );
        attrList->add( FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ) );

        XFastAttributeListRef xAttrList( attrList );
        attrList = nullptr;

        m_pSerializer->singleElementNS( XML_w, XML_pgSz, xAttrList );
    }
}

void DocxAttributeOutput::FormatPaperBin( const SvxPaperBinItem& )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FormatPaperBin()" );
}

void DocxAttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLRSpace )
{
    bool bEcma = m_rExport.GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-left:").append(double(rLRSpace.GetLeft()) / 20).append("pt");
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-right:").append(double(rLRSpace.GetRight()) / 20).append("pt");
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
        m_pageMargins.nPageMarginLeft = 0;
        m_pageMargins.nPageMarginRight = 0;

        const SfxPoolItem* pItem = m_rExport.HasItem( RES_BOX );
        if ( pItem )
        {
            m_pageMargins.nPageMarginRight = static_cast<const SvxBoxItem*>(pItem)->CalcLineSpace( SvxBoxItemLine::LEFT );
            m_pageMargins.nPageMarginLeft = static_cast<const SvxBoxItem*>(pItem)->CalcLineSpace( SvxBoxItemLine::RIGHT );
        }
        else
            m_pageMargins.nPageMarginLeft = m_pageMargins.nPageMarginRight = 0;

        m_pageMargins.nPageMarginLeft = m_pageMargins.nPageMarginLeft + (sal_uInt16)rLRSpace.GetLeft();
        m_pageMargins.nPageMarginRight = m_pageMargins.nPageMarginRight + (sal_uInt16)rLRSpace.GetRight();

        AddToAttrList( m_pSectionSpacingAttrList, 2,
                FSNS( XML_w, XML_left ), OString::number( m_pageMargins.nPageMarginLeft ).getStr(),
                FSNS( XML_w, XML_right ), OString::number( m_pageMargins.nPageMarginRight ).getStr() );
    }
    else
    {
        FastAttributeList *pLRSpaceAttrList = FastSerializerHelper::createAttrList();
        if((0 != rLRSpace.GetTextLeft()) || (rLRSpace.IsExplicitZeroMarginValLeft()))
        {
            pLRSpaceAttrList->add( FSNS( XML_w, ( bEcma ? XML_left : XML_start ) ), OString::number(  rLRSpace.GetTextLeft() ) );
        }
        if((0 != rLRSpace.GetRight()) || (rLRSpace.IsExplicitZeroMarginValRight()))
        {
            pLRSpaceAttrList->add( FSNS( XML_w, ( bEcma ? XML_right : XML_end ) ), OString::number(  rLRSpace.GetRight() ) );
        }
        sal_Int32 nFirstLineAdjustment = rLRSpace.GetTextFirstLineOfst();
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
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-top:").append(double(rULSpace.GetUpper()) / 20).append("pt");
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-wrap-distance-bottom:").append(double(rULSpace.GetLower()) / 20).append("pt");
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

        // Page top
        m_pageMargins.nPageMarginTop = aDistances.dyaTop;

        sal_Int32 nFooter = 0;
        if ( aDistances.HasFooter() )
            nFooter = sal_Int32( aDistances.dyaHdrBottom );

        // Page Bottom
        m_pageMargins.nPageMarginBottom = aDistances.dyaBottom;

        AddToAttrList( m_pSectionSpacingAttrList, 5,
                FSNS( XML_w, XML_header ), OString::number( nHeader ).getStr(),
                FSNS( XML_w, XML_top ), OString::number( m_pageMargins.nPageMarginTop ).getStr(),
                FSNS( XML_w, XML_footer ), OString::number( nFooter ).getStr(),
                FSNS( XML_w, XML_bottom ), OString::number( m_pageMargins.nPageMarginBottom ).getStr(),
                // FIXME Page Gutter is not handled ATM, setting to 0 as it's mandatory for OOXML
                FSNS( XML_w, XML_gutter ), "0" );
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
            m_pSerializer->singleElementNS( XML_w, XML_contextualSpacing, FSEND );
    }
}

void DocxAttributeOutput::FormatSurround( const SwFormatSurround& rSurround )
{
    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        OString sType, sSide;
        switch (rSurround.GetSurround())
        {
            case SURROUND_NONE:
                sType = "topAndBottom";
                break;
            case SURROUND_PARALLEL:
                sType = "square";
                break;
            case SURROUND_IDEAL:
                sType = "square";
                sSide = "largest";
                break;
            case SURROUND_LEFT:
                sType = "square";
                sSide = "left";
                break;
            case SURROUND_RIGHT:
                sType = "square";
                sSide = "right";
                break;
            case SURROUND_THROUGHT:
                /* empty type and side means throught */
            default:
                break;
        }
        if (!sType.isEmpty() || !sSide.isEmpty())
        {
            m_rExport.SdrExporter().setFlyWrapAttrList(FastSerializerHelper::createAttrList());
            if (!sType.isEmpty())
                m_rExport.SdrExporter().getFlyWrapAttrList()->add(XML_type, sType);
            if (!sSide.isEmpty())
                m_rExport.SdrExporter().getFlyWrapAttrList()->add(XML_side, sSide);
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
            case SURROUND_NONE:
                sWrap = OString( "none" );
                break;
            case SURROUND_THROUGHT:
                sWrap = OString( "through" );
                break;
            case SURROUND_IDEAL:
            case SURROUND_PARALLEL:
            case SURROUND_LEFT:
            case SURROUND_RIGHT:
            default:
                sWrap = OString( "around" );
        }

        AddToAttrList( m_rExport.SdrExporter().getFlyAttrList(), FSNS( XML_w, XML_wrap ), sWrap.getStr() );
    }
}

void DocxAttributeOutput::FormatVertOrientation( const SwFormatVertOrient& rFlyVert )
{
    OString sAlign;
    switch( rFlyVert.GetVertOrient() )
    {
        case text::VertOrientation::NONE:
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            sAlign = OString( "center" );
            break;
        case text::VertOrientation::BOTTOM:
            sAlign = OString( "bottom" );
            break;
        case text::VertOrientation::LINE_BOTTOM:
            sAlign = OString( "outside" );
            break;
        case text::VertOrientation::TOP:
            sAlign = OString( "top" );
            break;
        case text::VertOrientation::LINE_TOP:
        default:
            sAlign = OString( "inside" );
            break;
    }
    OString sVAnchor( "page" );
    switch ( rFlyVert.GetRelationOrient( ) )
    {
        case text::RelOrientation::CHAR:
        case text::RelOrientation::PRINT_AREA:
        case text::RelOrientation::TEXT_LINE:
        case text::RelOrientation::FRAME:
            sVAnchor = OString( "text" );
            break;
        case text::RelOrientation::PAGE_LEFT:
        case text::RelOrientation::PAGE_RIGHT:
        case text::RelOrientation::FRAME_LEFT:
        case text::RelOrientation::FRAME_RIGHT:
        case text::RelOrientation::PAGE_PRINT_AREA:
            sVAnchor = OString( "margin" );
            break;
        case text::RelOrientation::PAGE_FRAME:
        default:
            break;
    }

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";margin-top:").append(double(rFlyVert.GetPos()) / 20).append("pt");
        if ( !sAlign.isEmpty() )
            m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-vertical:").append(sAlign);
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-vertical-relative:").append(sVAnchor);
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
    OString sAlign;
    switch( rFlyHori.GetHoriOrient() )
    {
        case text::HoriOrientation::NONE:
            break;
        case text::HoriOrientation::LEFT:
            sAlign = OString( rFlyHori.IsPosToggle( ) ? "inside" : "left" );
            break;
        case text::HoriOrientation::RIGHT:
            sAlign = OString( rFlyHori.IsPosToggle( ) ? "outside" : "right" );
            break;
        case text::HoriOrientation::CENTER:
        case text::HoriOrientation::FULL: // FULL only for tables
        default:
            sAlign = OString( "center" );
            break;
    }
    OString sHAnchor( "page" );
    switch ( rFlyHori.GetRelationOrient( ) )
    {
        case text::RelOrientation::CHAR:
        case text::RelOrientation::PRINT_AREA:
        case text::RelOrientation::FRAME:
            sHAnchor = OString( "text" );
            break;
        case text::RelOrientation::PAGE_LEFT:
        case text::RelOrientation::PAGE_RIGHT:
        case text::RelOrientation::FRAME_LEFT:
        case text::RelOrientation::FRAME_RIGHT:
        case text::RelOrientation::PAGE_PRINT_AREA:
            sHAnchor = OString( "margin" );
            break;
        case text::RelOrientation::PAGE_FRAME:
        default:
            break;
    }

    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        m_rExport.SdrExporter().getTextFrameStyle().append(";margin-left:").append(double(rFlyHori.GetPos()) / 20).append("pt");
        if ( !sAlign.isEmpty() )
            m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-horizontal:").append(sAlign);
        m_rExport.SdrExporter().getTextFrameStyle().append(";mso-position-horizontal-relative:").append(sHAnchor);
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

boost::optional<sal_Int32> lcl_getDmlAlpha(const SvxBrushItem& rBrush)
{
    boost::optional<sal_Int32> oRet;
    sal_Int32 nTransparency = rBrush.GetColor().GetTransparency();
    if (nTransparency)
    {
        // Convert transparency to percent
        sal_Int8 nTransparencyPercent = SvxBrushItem::TransparencyToPercent(nTransparency);

        // Calculate alpha value
        // Consider oox/source/drawingml/color.cxx : getTransparency() function.
        sal_Int32 nAlpha = (::oox::drawingml::MAX_PERCENT - ( ::oox::drawingml::PER_PERCENT * nTransparencyPercent ) );
        oRet = nAlpha;
    }
    return oRet;
}

void DocxAttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    OString sColor = msfilter::util::ConvertColor( rBrush.GetColor().GetRGBColor() );
    boost::optional<sal_Int32> oAlpha = lcl_getDmlAlpha(rBrush);
    if (m_rExport.SdrExporter().getTextFrameSyntax())
    {
        // Handle 'Opacity'
        if (oAlpha)
        {
            // Calculate opacity value
            // Consider oox/source/vml/vmlformatting.cxx : decodeColor() function.
            double fOpacity = (double)(*oAlpha) * 65535 / ::oox::drawingml::MAX_PERCENT;
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
            m_pSerializer->startElementNS(XML_a, XML_solidFill, FSEND);
            m_pSerializer->startElementNS(XML_a, XML_srgbClr,
                                          XML_val, sColor,
                                          FSEND);
            if (oAlpha)
                m_pSerializer->singleElementNS(XML_a, XML_alpha,
                                              XML_val, OString::number(*oAlpha),
                                              FSEND);
            m_pSerializer->endElementNS(XML_a, XML_srgbClr);
            m_pSerializer->endElementNS(XML_a, XML_solidFill);
        }
    }
    else if ( !m_rExport.m_bOutPageDescs )
    {
        // compare fill color with the original fill color
        OString sOriginalFill = OUStringToOString(
                m_sOriginalBackgroundColor, RTL_TEXTENCODING_UTF8 );

        if( !m_pBackgroundAttrList.is() )
        {
            m_pBackgroundAttrList = FastSerializerHelper::createAttrList();
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_fill ), sColor.getStr() );
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_val ), "clear" );
        }
        else if ( sOriginalFill != sColor )
        {
            // fill was modified during edition, theme fill attribute must be dropped
            m_pBackgroundAttrList = FastSerializerHelper::createAttrList();
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_fill ), sColor.getStr() );
            m_pBackgroundAttrList->add( FSNS( XML_w, XML_val ), "clear" );
        }
        m_sOriginalBackgroundColor.clear();
    }
}

void DocxAttributeOutput::FormatFillStyle( const XFillStyleItem& rFillStyle )
{
    if (!m_bIgnoreNextFill)
        m_oFillStyle.reset(rFillStyle.GetValue());
    else
        m_bIgnoreNextFill = false;
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
        sal_Int32 nReverseAngle = 4500 - rGradient.GetAngle();
        nReverseAngle = nReverseAngle / 10;
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
                    m_rDrawingML.WriteBlipFill( xPropertySet, "BackGraphicURL" );
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
            editeng::SvxBorderStyle eBorderStyle = pTop->GetBorderLineStyle();
            if (eBorderStyle == table::BorderLineStyle::NONE)
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
                    if( drawing::LineStyle_DASH == pTop->GetBorderLineStyle() ) // Line Style is Dash type
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
        double fDistanceLeftInch = fDistanceLeftTwips / 1440;
        double fDistanceTopInch = fDistanceTopTwips / 1440;
        double fDistanceRightInch = fDistanceRightTwips / 1440;
        double fDistanceBottomInch = fDistanceBottomTwips / 1440;

        // This code will write ONLY the non-default values. The values are in 'left','top','right','bottom' order.
        // so 'bottom' is checked if it is default and if it is non-default - all the values will be written
        // otherwise - 'right' is checked if it is default and if it is non-default - all the values except for 'bottom' will be written
        // and so on.
        OStringBuffer aInset;
        if(!aInset.isEmpty() || fDistanceBottomInch != double(0.05))
            aInset.insert(0, "," + OString::number(fDistanceBottomInch) + "in");

        if(!aInset.isEmpty() || fDistanceRightInch != double(0.1))
            aInset.insert(0, "," + OString::number(fDistanceRightInch) + "in");

        if(!aInset.isEmpty() || fDistanceTopInch != double(0.05))
            aInset.insert(0, "," + OString::number(fDistanceTopInch) + "in");

        if(!aInset.isEmpty() || fDistanceLeftInch != double(0.1))
            aInset.insert(0, OString::number(fDistanceLeftInch) + "in");

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

    if ( !m_bOpenedSectPr || GetWritingHeaderFooter())
    {
        // Not inside a section

        // Open the paragraph's borders tag
        m_pSerializer->startElementNS( XML_w, XML_pBdr, FSEND );

        std::map<SvxBoxItemLine, css::table::BorderLine2> aEmptyMap; // empty styles map
        impl_borders( m_pSerializer, rBox, aOutputBorderOptions, &m_pageMargins,
                      aEmptyMap );

        // Close the paragraph's borders tag
        m_pSerializer->endElementNS( XML_w, XML_pBdr );
    }
}

void DocxAttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol& rCol, bool bEven, SwTwips nPageSize )
{
    // Get the columns attributes
    FastAttributeList *pColsAttrList = FastSerializerHelper::createAttrList();

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
            FastAttributeList *pColAttrList = FastSerializerHelper::createAttrList();
            sal_uInt16 nWidth = rCol.CalcPrtColWidth( n, ( sal_uInt16 ) nPageSize );
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

void DocxAttributeOutput::FormatKeep( const SvxFormatKeepItem& )
{
    m_pSerializer->singleElementNS( XML_w, XML_keepNext, FSEND );
}

void DocxAttributeOutput::FormatTextGrid( const SwTextGridItem& rGrid )
{
    FastAttributeList *pGridAttrList = FastSerializerHelper::createAttrList();

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
    pGridAttrList->add( FSNS( XML_w, XML_type ), sGridType.getStr( ) );

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
        m_pSerializer->singleElementNS( XML_w, XML_suppressLineNumbers, FSEND );
}

void DocxAttributeOutput::FormatFrameDirection( const SvxFrameDirectionItem& rDirection )
{
    OString sTextFlow;
    bool bBiDi = false;
    short nDir = rDirection.GetValue();

    if ( nDir == FRMDIR_ENVIRONMENT )
        nDir = GetExport( ).GetDefaultFrameDirection( );

    switch ( nDir )
    {
        default:
        case FRMDIR_HORI_LEFT_TOP:
            sTextFlow = OString( "lrTb" );
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            sTextFlow = OString( "lrTb" );
            bBiDi = true;
            break;
        case FRMDIR_VERT_TOP_LEFT: // many things but not this one
        case FRMDIR_VERT_TOP_RIGHT:
            sTextFlow = OString( "tbRl" );
            break;
    }

    if ( m_rExport.m_bOutPageDescs )
    {
        m_pSerializer->singleElementNS( XML_w, XML_textDirection,
               FSNS( XML_w, XML_val ), sTextFlow.getStr( ),
               FSEND );
        if ( bBiDi )
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSEND );
    }
    else if ( !m_rExport.m_bOutFlyFrameAttrs )
    {
        if ( bBiDi )
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSNS( XML_w, XML_val ), "1", FSEND );
        else
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSNS( XML_w, XML_val ), "0", FSEND );
    }
}

void DocxAttributeOutput::ParaGrabBag(const SfxGrabBagItem& rItem)
{
    const std::map<OUString, css::uno::Any>& rMap = rItem.GetGrabBag();
    for (std::map<OUString, css::uno::Any>::const_iterator i = rMap.begin(); i != rMap.end(); ++i)
    {
        if (i->first == "MirrorIndents")
            m_pSerializer->singleElementNS(XML_w, XML_mirrorIndents, FSEND);
        else if (i->first == "ParaTopMarginBeforeAutoSpacing")
        {
            m_bParaBeforeAutoSpacing = true;
            // get fixed value which was set during import
            i->second >>= m_nParaBeforeSpacing;
            m_nParaBeforeSpacing = convertMm100ToTwip(m_nParaBeforeSpacing);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << i->first << " : m_nParaBeforeSpacing= " << m_nParaBeforeSpacing);
        }
        else if (i->first == "ParaBottomMarginAfterAutoSpacing")
        {
            m_bParaAfterAutoSpacing = true;
            // get fixed value which was set during import
            i->second >>= m_nParaAfterSpacing;
            m_nParaAfterSpacing = convertMm100ToTwip(m_nParaAfterSpacing);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << i->first << " : m_nParaBeforeSpacing= " << m_nParaAfterSpacing);
        }
        else if (i->first == "CharThemeFill")
        {
            uno::Sequence<beans::PropertyValue> aGrabBagSeq;
            i->second >>= aGrabBagSeq;

            for (sal_Int32 j=0; j < aGrabBagSeq.getLength(); ++j)
            {
                OString sVal = OUStringToOString(aGrabBagSeq[j].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);

                if (sVal.isEmpty())
                    continue;

                if (aGrabBagSeq[j].Name == "val")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_val), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "color")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_color), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeColor")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeColor), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeTint")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeTint), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeShade")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeShade), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "fill")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_fill), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeFill")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFill), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeFillTint")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFillTint), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "themeFillShade")
                    AddToAttrList(m_pBackgroundAttrList, FSNS(XML_w, XML_themeFillShade), sVal.getStr());
                else if (aGrabBagSeq[j].Name == "originalColor")
                    aGrabBagSeq[j].Value >>= m_sOriginalBackgroundColor;
            }
        }
        else if (i->first == "SdtPr")
        {
            uno::Sequence<beans::PropertyValue> aGrabBagSdt =
                    i->second.get< uno::Sequence<beans::PropertyValue> >();
            for (sal_Int32 k=0; k < aGrabBagSdt.getLength(); ++k)
            {
                beans::PropertyValue aPropertyValue = aGrabBagSdt[k];
                if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartObj" ||
                        aPropertyValue.Name == "ooxml:CT_SdtPr_docPartList")
                {
                    if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartObj")
                        m_nParagraphSdtPrToken = FSNS( XML_w, XML_docPartObj );
                    else if (aPropertyValue.Name == "ooxml:CT_SdtPr_docPartList")
                        m_nParagraphSdtPrToken = FSNS( XML_w, XML_docPartList );

                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (sal_Int32 j=0; j < aGrabBag.getLength(); ++j)
                    {
                        OUString sValue = aGrabBag[j].Value.get<OUString>();
                        if (aGrabBag[j].Name == "ooxml:CT_SdtDocPart_docPartGallery")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w, XML_docPartGallery ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDocPart_docPartCategory")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w, XML_docPartCategory ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDocPart_docPartUnique")
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
                    m_nParagraphSdtPrToken = FSNS(XML_w, XML_text);
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_dataBinding" && !m_pParagraphSdtPrDataBindingAttrs.is())
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (sal_Int32 j = 0; j < aGrabBag.getLength(); ++j)
                    {
                        OUString sValue = aGrabBag[j].Value.get<OUString>();
                        if (aGrabBag[j].Name == "ooxml:CT_DataBinding_prefixMappings")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_prefixMappings ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_DataBinding_xpath")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_xpath ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_DataBinding_storeItemID")
                            AddToAttrList( m_pParagraphSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_storeItemID ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
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
                    for (sal_Int32 j=0; j < aGrabBag.getLength(); ++j)
                    {
                        OUString sValue = aGrabBag[j].Value.get<OUString>();
                        if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_checked")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checked ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_checkedState")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checkedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_uncheckedState")
                            AddToAttrList( m_pParagraphSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_uncheckedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_id")
                    m_bParagraphSdtHasId = true;
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_date")
                {
                    m_nParagraphSdtPrToken = FSNS(XML_w, XML_date);
                    uno::Sequence<beans::PropertyValue> aGrabBag = aPropertyValue.Value.get< uno::Sequence<beans::PropertyValue> >();
                    for (sal_Int32 j=0; j < aGrabBag.getLength(); ++j)
                    {
                        OString sValue = OUStringToOString(aGrabBag[j].Value.get<OUString>(), RTL_TEXTENCODING_UTF8);

                        if (aGrabBag[j].Name == "ooxml:CT_SdtDate_fullDate")
                            AddToAttrList(m_pParagraphSdtPrTokenAttributes, FSNS(XML_w, XML_fullDate), sValue.getStr());
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDate_dateFormat")
                            AddToAttrList(m_pParagraphSdtPrTokenChildren, FSNS(XML_w, XML_dateFormat), sValue.getStr());
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDate_lid")
                            AddToAttrList(m_pParagraphSdtPrTokenChildren, FSNS(XML_w, XML_lid), sValue.getStr());
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDate_storeMappedDataAs")
                            AddToAttrList(m_pParagraphSdtPrTokenChildren, FSNS(XML_w, XML_storeMappedDataAs), sValue.getStr());
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtDate_calendar")
                            AddToAttrList(m_pParagraphSdtPrTokenChildren, FSNS(XML_w, XML_calendar), sValue.getStr());
                        else
                            SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled SdtPr / ooxml:CT_SdtPr_date grab bag property " << aGrabBag[j].Name);
                    }
                }
                else
                    SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled SdtPr grab bag property " << aPropertyValue.Name);
            }
        }
        else if (i->first == "ParaCnfStyle")
        {
            uno::Sequence<beans::PropertyValue> aAttributes = i->second.get< uno::Sequence<beans::PropertyValue> >();
            m_pTableStyleExport->CnfStyle(aAttributes);
        }
        else if (i->first == "ParaSdtEndBefore")
        {
            // Handled already in StartParagraph().
        }
        else
            SAL_WARN("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled grab bag property " << i->first );
    }
}

void DocxAttributeOutput::CharGrabBag( const SfxGrabBagItem& rItem )
{
    const std::map< OUString, css::uno::Any >& rMap = rItem.GetGrabBag();

    // get original values of theme-derived properties to check if they have changed during the edition
    bool bWriteCSTheme = true;
    bool bWriteAsciiTheme = true;
    bool bWriteEastAsiaTheme = true;
    bool bWriteThemeFontColor = true;
    OUString sOriginalValue;
    for ( std::map< OUString, css::uno::Any >::const_iterator i = rMap.begin(); i != rMap.end(); ++i )
    {
        if ( m_pFontsAttrList.is() && i->first == "CharThemeFontNameCs" )
        {
            if ( i->second >>= sOriginalValue )
                bWriteCSTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_cs ) ) == sOriginalValue );
        }
        else if ( m_pFontsAttrList.is() && i->first == "CharThemeFontNameAscii" )
        {
            if ( i->second >>= sOriginalValue )
                bWriteAsciiTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_ascii ) ) == sOriginalValue );
        }
        else if ( m_pFontsAttrList.is() && i->first == "CharThemeFontNameEastAsia" )
        {
            if ( i->second >>= sOriginalValue )
                bWriteEastAsiaTheme =
                        ( m_pFontsAttrList->getOptionalValue( FSNS( XML_w, XML_eastAsia ) ) == sOriginalValue );
        }
        else if ( m_pColorAttrList.is() && i->first == "CharThemeOriginalColor" )
        {
            if ( i->second >>= sOriginalValue )
                bWriteThemeFontColor =
                        ( m_pColorAttrList->getOptionalValue( FSNS( XML_w, XML_val ) ) == sOriginalValue );
        }
    }

    // save theme attributes back to the run properties
    OUString str;
    for ( std::map< OUString, css::uno::Any >::const_iterator i = rMap.begin(); i != rMap.end(); ++i )
    {
        if ( i->first == "CharThemeNameAscii" && bWriteAsciiTheme )
        {
            i->second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_asciiTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeNameCs" && bWriteCSTheme )
        {
            i->second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_cstheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeNameEastAsia" && bWriteEastAsiaTheme )
        {
            i->second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_eastAsiaTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeNameHAnsi" && bWriteAsciiTheme )
        // this is not a mistake: in LibO we don't directly support the hAnsi family
        // of attributes so we save the same value from ascii attributes instead
        {
            i->second >>= str;
            AddToAttrList( m_pFontsAttrList, FSNS( XML_w, XML_hAnsiTheme ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeColor" && bWriteThemeFontColor )
        {
            i->second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeColor ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeColorShade" )
        {
            i->second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeShade ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if ( i->first == "CharThemeColorTint" )
        {
            i->second >>= str;
            AddToAttrList( m_pColorAttrList, FSNS( XML_w, XML_themeTint ),
                    OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else if( i->first == "CharThemeFontNameCs"   ||
                i->first == "CharThemeFontNameAscii" ||
                i->first == "CharThemeFontNameEastAsia" ||
                i->first == "CharThemeOriginalColor" )
        {
            // just skip these, they were processed before
        }
        else if(i->first == "CharGlowTextEffect" ||
                i->first == "CharShadowTextEffect" ||
                i->first == "CharReflectionTextEffect" ||
                i->first == "CharTextOutlineTextEffect" ||
                i->first == "CharTextFillTextEffect" ||
                i->first == "CharScene3DTextEffect" ||
                i->first == "CharProps3DTextEffect" ||
                i->first == "CharLigaturesTextEffect" ||
                i->first == "CharNumFormTextEffect" ||
                i->first == "CharNumSpacingTextEffect" ||
                i->first == "CharStylisticSetsTextEffect" ||
                i->first == "CharCntxtAltsTextEffect")
        {
            beans::PropertyValue aPropertyValue;
            i->second >>= aPropertyValue;
            m_aTextEffectsGrabBag.push_back(aPropertyValue);
        }
        else if (i->first == "SdtEndBefore")
        {
            if (m_bStartedCharSdt)
                m_bEndCharSdt = true;
        }
        else if (i->first == "SdtPr" && FLY_NOT_PROCESSED != m_nStateOfFlyFrame )
        {
            uno::Sequence<beans::PropertyValue> aGrabBagSdt =
                    i->second.get< uno::Sequence<beans::PropertyValue> >();
            for (sal_Int32 k=0; k < aGrabBagSdt.getLength(); ++k)
            {
                beans::PropertyValue aPropertyValue = aGrabBagSdt[k];
                if (aPropertyValue.Name == "ooxml:CT_SdtPr_checkbox")
                {
                    m_nRunSdtPrToken = FSNS( XML_w14, XML_checkbox );
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (sal_Int32 j=0; j < aGrabBag.getLength(); ++j)
                    {
                        OUString sValue = aGrabBag[j].Value.get<OUString>();
                        if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_checked")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checked ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_checkedState")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_checkedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_SdtCheckbox_uncheckedState")
                            AddToAttrList( m_pRunSdtPrTokenChildren,
                                           FSNS( XML_w14, XML_uncheckedState ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
                else if (aPropertyValue.Name == "ooxml:CT_SdtPr_dataBinding" && !m_pRunSdtPrDataBindingAttrs.is())
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    aPropertyValue.Value >>= aGrabBag;
                    for (sal_Int32 j=0; j < aGrabBag.getLength(); ++j)
                    {
                        OUString sValue = aGrabBag[j].Value.get<OUString>();
                        if (aGrabBag[j].Name == "ooxml:CT_DataBinding_prefixMappings")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_prefixMappings ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_DataBinding_xpath")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_xpath ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
                        else if (aGrabBag[j].Name == "ooxml:CT_DataBinding_storeItemID")
                            AddToAttrList( m_pRunSdtPrDataBindingAttrs,
                                           FSNS( XML_w, XML_storeItemID ),
                                           OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr() );
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
            SAL_INFO("sw.ww8", "DocxAttributeOutput::CharGrabBag: unhandled grab bag property " << i->first);
    }
}

DocxAttributeOutput::DocxAttributeOutput( DocxExport &rExport, FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML )
    : m_rExport( rExport ),
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
      m_sFieldBkm( ),
      m_nNextBookmarkId( 0 ),
      m_nNextAnnotationMarkId( 0 ),
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
      m_postponedChart( nullptr ),
      pendingPlaceholder( nullptr ),
      m_postitFieldsMaxId( 0 ),
      m_anchorId( 1 ),
      m_nextFontId( 1 ),
      m_tableReference(new TableReference()),
      m_bIgnoreNextFill(false),
      m_bBtLr(false),
      m_pTableStyleExport(new DocxTableStyleExport(rExport.m_pDoc, pSerializer)),
      m_bParaBeforeAutoSpacing(false),
      m_bParaAfterAutoSpacing(false),
      m_nParaBeforeSpacing(0),
      m_nParaAfterSpacing(0),
      m_setFootnote(false)
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

void DocxAttributeOutput::SetSerializer( ::sax_fastparser::FSHelperPtr pSerializer )
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
            FSNS(XML_w, XML_numPicBulletId), OString::number(nId).getStr(),
            FSEND);

    OStringBuffer aStyle;
    // Size is in twips, we need it in points.
    aStyle.append("width:").append(double(aSize.Width()) / 20);
    aStyle.append("pt;height:").append(double(aSize.Height()) / 20).append("pt");
    m_pSerializer->startElementNS( XML_w, XML_pict, FSEND);
    m_pSerializer->startElementNS( XML_v, XML_shape,
            XML_style, aStyle.getStr(),
            FSNS(XML_o, XML_bullet), "t",
            FSEND);

    m_rDrawingML.SetFS(m_pSerializer);
    OUString aRelId = m_rDrawingML.WriteImage(rGraphic);
    m_pSerializer->singleElementNS( XML_v, XML_imagedata,
            FSNS(XML_r, XML_id), OUStringToOString(aRelId, RTL_TEXTENCODING_UTF8),
            FSNS(XML_o, XML_title), "",
            FSEND);

    m_pSerializer->endElementNS(XML_v, XML_shape);
    m_pSerializer->endElementNS(XML_w, XML_pict);

    m_pSerializer->endElementNS(XML_w, XML_numPicBullet);
}

void DocxAttributeOutput::AddToAttrList( uno::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nAttrName, const sal_Char* sAttrValue )
{
    AddToAttrList( pAttrList, 1, nAttrName, sAttrValue );
}

void DocxAttributeOutput::AddToAttrList( uno::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nAttrs, ... )
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

void DocxAttributeOutput::SetStartedParaSdt(bool bStartedParaSdt)
{
    m_bStartedParaSdt = bStartedParaSdt;
}

bool DocxAttributeOutput::IsStartedParaSdt()
{
    return m_bStartedParaSdt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
