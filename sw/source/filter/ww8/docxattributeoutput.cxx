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
#include "docxexportfilter.hxx"
#include "docxfootnotes.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include "fmtcntnt.hxx"
#include "fmtftn.hxx"
#include "fchrfmt.hxx"
#include "tgrditem.hxx"
#include "fmtruby.hxx"
#include "breakit.hxx"

#include <comphelper/string.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <oox/mathml/export.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

#include <i18nlangtag/languagetag.hxx>

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
#include <editeng/opaqitem.hxx>
#include <editeng/editobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svl/grabbagitem.hxx>
#include <sfx2/sfxbasemodel.hxx>

#include <anchoredobject.hxx>
#include <docufld.hxx>
#include <flddropdown.hxx>
#include <fmtanchr.hxx>
#include <fmtclds.hxx>
#include <fmtinfmt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtline.hxx>
#include <frmatr.hxx>
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

#include <osl/file.hxx>
#include <rtl/tencinfo.h>
#include <vcl/embeddedfontshelper.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/XAttr.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using ::editeng::SvxBorderLine;

using namespace oox;
using namespace docx;
using namespace sax_fastparser;
using namespace nsSwDocInfoSubType;
using namespace nsFieldFlags;
using namespace sw::util;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;


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
    FFDataWriterHelper( const ::sax_fastparser::FSHelperPtr pSerializer ) : m_pSerializer( pSerializer ){}
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
    FieldMarkParamsHelper( const sw::mark::IFieldmark& rFieldmark ) : mrFieldmark( rFieldmark ) {}
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

void DocxAttributeOutput::StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo )
{
    if ( m_nColBreakStatus == COLBRK_POSTPONE )
        m_nColBreakStatus = COLBRK_WRITE;

    // Output table/table row/table cell starts if needed
    if ( pTextNodeInfo.get() )
    {
        sal_uInt32 nRow = pTextNodeInfo->getRow();
        sal_uInt32 nCell = pTextNodeInfo->getCell();

        // New cell/row?
        if ( m_nTableDepth > 0 && !m_bTableCellOpen )
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pDeepInner( pTextNodeInfo->getInnerForDepth( m_nTableDepth ) );
            if ( pDeepInner->getCell() == 0 )
                StartTableRow( pDeepInner );

            StartTableCell( pDeepInner );
        }

        if ( nRow == 0 && nCell == 0 )
        {
            // Do we have to start the table?
            // [If we are at the rigth depth already, it means that we
            // continue the table cell]
            sal_uInt32 nCurrentDepth = pTextNodeInfo->getDepth();

            if ( nCurrentDepth > m_nTableDepth )
            {
                // Start all the tables that begin here
                for ( sal_uInt32 nDepth = m_nTableDepth + 1; nDepth <= pTextNodeInfo->getDepth(); ++nDepth )
                {
                    ww8::WW8TableNodeInfoInner::Pointer_t pInner( pTextNodeInfo->getInnerForDepth( nDepth ) );

                    StartTable( pInner );
                    StartTableRow( pInner );
                    StartTableCell( pInner );
                }

                m_nTableDepth = nCurrentDepth;
            }
        }
    }

    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );

    // postpone the output of the run (we get it before the paragraph
    // properties, but must write it after them)
    m_pSerializer->mark();

    // no section break in this paragraph yet; can be set in SectionBreak()
    m_pSectionInfo.reset();

    m_bParagraphOpened = true;
}

void lcl_TextFrameShadow(FSHelperPtr pSerializer, const SwFrmFmt& rFrmFmt)
{
    SvxShadowItem aShadowItem = rFrmFmt.GetShadow();
    if (aShadowItem.GetLocation() == SVX_SHADOW_NONE)
        return;

    OString aShadowWidth( OString::number( double( aShadowItem.GetWidth() ) / 20) + "pt");
    OString aOffset;
    switch (aShadowItem.GetLocation())
    {
        case SVX_SHADOW_TOPLEFT: aOffset = "-" + aShadowWidth + ",-" + aShadowWidth; break;
        case SVX_SHADOW_TOPRIGHT: aOffset = aShadowWidth + ",-" + aShadowWidth; break;
        case SVX_SHADOW_BOTTOMLEFT: aOffset = "-" + aShadowWidth + "," + aShadowWidth; break;
        case SVX_SHADOW_BOTTOMRIGHT: aOffset = aShadowWidth + "," + aShadowWidth; break;
        case SVX_SHADOW_NONE:
        case SVX_SHADOW_END:
            break;
    }
    if (aOffset.isEmpty())
        return;

    OString aShadowColor = msfilter::util::ConvertColor(aShadowItem.GetColor());
    pSerializer->singleElementNS(XML_v, XML_shadow,
            XML_on, "t",
            XML_color, "#" + aShadowColor,
            XML_offset, aOffset,
            FSEND);
}

class ExportDataSaveRestore
{
private:
    DocxExport& m_rExport;
public:
    ExportDataSaveRestore(DocxExport& rExport, sal_uLong nStt, sal_uLong nEnd, sw::Frame *pParentFrame)
        : m_rExport(rExport)
    {
        m_rExport.SaveData(nStt, nEnd);
        m_rExport.mpParentFrame = pParentFrame;
    }
    ~ExportDataSaveRestore()
    {
        m_rExport.RestoreData();
    }
};

// Undo the text direction mangling done by the frame btLr handler in writerfilter::dmapper::DomainMapper::lcl_startCharacterGroup()
bool lcl_checkFrameBtlr(SwNode* pStartNode, sax_fastparser::FastAttributeList* pTextboxAttrList)
{
    if (!pStartNode->IsTxtNode())
        return false;

    SwTxtNode* pTxtNode = static_cast<SwTxtNode*>(pStartNode);
    if (!pTxtNode->HasHints())
        return false;

    SwTxtAttr* pTxtAttr = pTxtNode->GetTxtAttrAt(0, RES_TXTATR_AUTOFMT);

    if (!pTxtAttr || pTxtAttr->Which() != RES_TXTATR_AUTOFMT)
        return false;

    boost::shared_ptr<SfxItemSet> pItemSet = pTxtAttr->GetAutoFmt().GetStyleHandle();
    const SfxPoolItem* pItem;
    if (pItemSet->GetItemState(RES_CHRATR_ROTATE, true, &pItem) == SFX_ITEM_SET)
    {
        const SvxCharRotateItem& rCharRotate = static_cast<const SvxCharRotateItem&>(*pItem);
        if (rCharRotate.GetValue() == 900)
        {
            pTextboxAttrList->add(XML_style, "mso-layout-flow-alt:bottom-to-top");
            return true;
        }
    }
    return false;
}

void DocxAttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    // write the paragraph properties + the run, already in the correct order
    m_pSerializer->mergeTopMarks();

    // Write the anchored frame if any
    // Make a copy and clear the original early, as this method is called
    // recursively for in-frame paragraphs
    std::vector<sw::Frame> aParentFrames = m_aParentFrames;
    m_aParentFrames.clear();
    for (size_t i = 0; i < aParentFrames.size(); ++i)
    {
        sw::Frame* pParentFrame = &aParentFrames[i];

        const SwFrmFmt& rFrmFmt = pParentFrame->GetFrmFmt( );
        const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

        sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
        sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

        //Save data here and restore when out of scope
        ExportDataSaveRestore aDataGuard(m_rExport, nStt, nEnd, pParentFrame);

        // When a frame has some low height, but automatically expanded due
        // to lots of contents, this size contains the real size.
        const Size aSize = pParentFrame->GetSize();
        m_pFlyFrameSize = &aSize;

        m_bTextFrameSyntax = true;
        m_pFlyAttrList = m_pSerializer->createAttrList( );
        m_pTextboxAttrList = m_pSerializer->createAttrList();
        m_aTextFrameStyle = "position:absolute";
        m_rExport.OutputFormat( pParentFrame->GetFrmFmt(), false, false, true );
        m_pFlyAttrList->add(XML_style, m_aTextFrameStyle.makeStringAndClear());
        XFastAttributeListRef xFlyAttrList( m_pFlyAttrList );
        m_pFlyAttrList = NULL;
        m_bFrameBtLr = lcl_checkFrameBtlr(m_rExport.pDoc->GetNodes()[nStt], m_pTextboxAttrList);
        XFastAttributeListRef xTextboxAttrList(m_pTextboxAttrList);
        m_pTextboxAttrList = NULL;
        m_bTextFrameSyntax = false;
        m_pFlyFrameSize = 0;
        m_rExport.mpParentFrame = NULL;

        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->startElementNS( XML_w, XML_pict, FSEND );
        m_pSerializer->startElementNS( XML_v, XML_rect, xFlyAttrList );
        lcl_TextFrameShadow(m_pSerializer, rFrmFmt);
        if (m_pFlyFillAttrList)
        {
            XFastAttributeListRef xFlyFillAttrList(m_pFlyFillAttrList);
            m_pFlyFillAttrList = NULL;
            m_pSerializer->singleElementNS(XML_v, XML_fill, xFlyFillAttrList);
        }
        m_pSerializer->startElementNS( XML_v, XML_textbox, xTextboxAttrList );
        m_pSerializer->startElementNS( XML_w, XML_txbxContent, FSEND );
        m_rExport.WriteText( );
        m_pSerializer->endElementNS( XML_w, XML_txbxContent );
        m_pSerializer->endElementNS( XML_v, XML_textbox );

        if (m_pFlyWrapAttrList)
        {
            XFastAttributeListRef xFlyWrapAttrList(m_pFlyWrapAttrList);
            m_pFlyWrapAttrList = NULL;
            m_pSerializer->singleElementNS(XML_w10, XML_wrap, xFlyWrapAttrList);
        }

        m_pSerializer->endElementNS( XML_v, XML_rect );
        m_pSerializer->endElementNS( XML_w, XML_pict );
        m_pSerializer->endElementNS( XML_w, XML_r );
        m_bFrameBtLr = false;
    }

    m_pSerializer->endElementNS( XML_w, XML_p );

    // Check for end of cell, rows, tables here
    FinishTableRowCell( pTextNodeInfoInner );

    m_bParagraphOpened = false;

}

void DocxAttributeOutput::FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool bForceEmptyParagraph )
{
    if ( pInner.get() )
    {
        // Where are we in the table
        sal_uInt32 nRow = pInner->getRow( );

        const SwTable *pTable = pInner->getTable( );
        const SwTableLines& rLines = pTable->GetTabLines( );
        sal_uInt16 nLinesCount = rLines.size( );
        // HACK
        // msoffice seems to have an internal limitation of 63 columns for tables
        // and refuses to load .docx with more, even though the spec seems to allow that;
        // so simply if there are more columns, don't close the last one msoffice will handle
        // and merge the contents of the remaining ones into it (since we don't close the cell
        // here, following ones will not be opened)
        bool limitWorkaround = ( pInner->getCell() >= 62 && !pInner->isEndOfLine());

        if ( pInner->isEndOfCell() && !limitWorkaround )
        {
            if ( bForceEmptyParagraph )
                m_pSerializer->singleElementNS( XML_w, XML_p, FSEND );

            EndTableCell();
        }

        // This is a line end
        if ( pInner->isEndOfLine() )
            EndTableRow();

        // This is the end of the table
        if ( pInner->isEndOfLine( ) && ( nRow + 1 ) == nLinesCount )
            EndTable();
    }
}

void DocxAttributeOutput::EmptyParagraph()
{
    m_pSerializer->singleElementNS( XML_w, XML_p, FSEND );
}

void DocxAttributeOutput::SectionBreaks(const SwTxtNode& rNode)
{
    // output page/section breaks
    // Writer can have them at the beginning of a paragraph, or at the end, but
    // in docx, we have to output them in the paragraph properties of the last
    // paragraph in a section.  To get it right, we have to switch to the next
    // paragraph, and detect the section breaks there.
    SwNodeIndex aNextIndex( rNode, 1 );
    if ( aNextIndex.GetNode().IsTxtNode() )
    {
        const SwTxtNode* pTxtNode = static_cast< SwTxtNode* >( &aNextIndex.GetNode() );
        // If next node has no string - it is an empty node, so no need to output the section break
        if (!pTxtNode->GetTxt().isEmpty())
            m_rExport.OutputSectionBreaks( pTxtNode->GetpSwAttrSet(), *pTxtNode );
    }
    else if ( aNextIndex.GetNode().IsTableNode() )
    {
        const SwTableNode* pTableNode = static_cast< SwTableNode* >( &aNextIndex.GetNode() );
        const SwFrmFmt *pFmt = pTableNode->GetTable().GetFrmFmt();
        m_rExport.OutputSectionBreaks( &(pFmt->GetAttrSet()), *pTableNode );
    }
}

void DocxAttributeOutput::StartParagraphProperties()
{
    m_pSerializer->mark( );

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
    m_pParagraphSpacingAttrList = NULL;

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

    m_pSerializer->mark( aSeqOrder );
}

void DocxAttributeOutput::WriteCollectedParagraphProperties()
{
    if ( m_pFlyAttrList )
    {
        XFastAttributeListRef xAttrList( m_pFlyAttrList );
        m_pFlyAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_framePr, xAttrList );
    }

    if ( m_pParagraphSpacingAttrList )
    {
        XFastAttributeListRef xAttrList( m_pParagraphSpacingAttrList );
        m_pParagraphSpacingAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_spacing, xAttrList );
    }

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks( );
}

void DocxAttributeOutput::EndParagraphProperties()
{
    WriteCollectedParagraphProperties();

    // insert copy of <rPr>
    m_pSerializer->copyTopMarkPop();

    m_pSerializer->endElementNS( XML_w, XML_pPr );

    if ( m_nColBreakStatus == COLBRK_WRITE )
    {
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_br,
                FSNS( XML_w, XML_type ), "column", FSEND );
        m_pSerializer->endElementNS( XML_w, XML_r );

        m_nColBreakStatus = COLBRK_NONE;
    }

    // merge the properties _before_ the run (strictly speaking, just
    // after the start of the paragraph)
    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND );
}

void DocxAttributeOutput::StartRun( const SwRedlineData* pRedlineData, bool /*bSingleEmptyRun*/ )
{
    // Don't start redline data here, possibly there is a hyperlink later, and
    // that has to be started first.
    m_pRedlineData = pRedlineData;

    // postpone the output of the start of a run (there are elements that need
    // to be written before the start of the run, but we learn which they are
    // _inside_ of the run)
    m_pSerializer->mark(); // let's call it "postponed run start"

    // postpone the output of the text (we get it before the run properties,
    // but must write it after them)
    m_pSerializer->mark(); // let's call it "postponed text"
}

void DocxAttributeOutput::EndRun()
{
    // Write field starts
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin(); pIt != m_Fields.end(); )
    {
        // Add the fields starts for all but hyperlinks and TOCs
        if ( pIt->bOpen && pIt->pField )
        {
            StartField_Impl( *pIt );

            // Remove the field from the stack if only the start has to be written
            // Unknown fields sould be removed too
            if ( !pIt->bClose || ( pIt->eType == ww::eUNKNOWN ) )
            {
                if (pIt->pField)
                    delete pIt->pField;
                pIt = m_Fields.erase( pIt );
                continue;
            }
        }
        ++pIt;
    }

    // write the run properties + the text, already in the correct order
    m_pSerializer->mergeTopMarks(); // merges with "postponed text", see above

    // level down, to be able to prepend the actual run start attribute (just
    // before "postponed run start")
    m_pSerializer->mark(); // let's call it "actual run start"

    if ( m_closeHyperlinkInPreviousRun )
    {
        if ( m_startedHyperlink )
        {
            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
            m_startedHyperlink = false;
        }
        m_closeHyperlinkInPreviousRun = false;
    }

    // Write the hyperlink and toc fields starts
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin(); pIt != m_Fields.end(); )
    {
        // Add the fields starts for hyperlinks, TOCs and index marks
        if ( pIt->bOpen && !pIt->pField )
        {
            StartField_Impl( *pIt, sal_True );

            // Remove the field if no end needs to be written
            if ( !pIt->bClose ) {
                if (pIt->pField)
                    delete pIt->pField;
                pIt = m_Fields.erase( pIt );
                continue;
            }
        }
        ++pIt;
    }

    // Start the hyperlink after the fields separators or we would generate invalid file
    if ( m_pHyperlinkAttrList )
    {
        XFastAttributeListRef xAttrList ( m_pHyperlinkAttrList );

        m_pSerializer->startElementNS( XML_w, XML_hyperlink, xAttrList );
        m_pHyperlinkAttrList = NULL;
        m_startedHyperlink = true;
    }

    // if there is some redlining in the document, output it
    StartRedline();

    DoWriteBookmarks( );
    WriteCommentRanges();

    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND ); // merges with "postponed run start", see above

    // write the run start + the run content
    m_pSerializer->mergeTopMarks(); // merges the "actual run start"

    // append the actual run end
    m_pSerializer->endElementNS( XML_w, XML_r );

    WritePostponedMath();
    WritePendingPlaceholder();

    // if there is some redlining in the document, output it
    EndRedline();

    if ( m_closeHyperlinkInThisRun )
    {
        if ( m_startedHyperlink )
        {
            m_pSerializer->endElementNS( XML_w, XML_hyperlink );
            m_startedHyperlink = false;
        }
        m_closeHyperlinkInThisRun = false;
    }

    while ( m_Fields.begin() != m_Fields.end() )
    {
        EndField_Impl( m_Fields.front( ) );
        if (m_Fields.front().pField)
            delete m_Fields.front().pField;
        m_Fields.erase( m_Fields.begin( ) );
    }
}

void DocxAttributeOutput::WriteCommentRanges()
{
    if (m_bPostitStart)
    {
        m_bPostitStart = false;
        OString idstr = OString::number( m_postitFieldsMaxId);
        m_pSerializer->singleElementNS( XML_w, XML_commentRangeStart, FSNS( XML_w, XML_id ), idstr.getStr(), FSEND );
    }
    if (m_bPostitEnd)
    {
        m_bPostitEnd = false;
        OString idstr = OString::number( m_postitFieldsMaxId);
        m_pSerializer->singleElementNS( XML_w, XML_commentRangeEnd, FSNS( XML_w, XML_id ), idstr.getStr(), FSEND );
    }
}

void DocxAttributeOutput::DoWriteBookmarks()
{
    // Write the start bookmarks
    for ( std::vector< OString >::const_iterator it = m_rMarksStart.begin(), end = m_rMarksStart.end();
          it != end; ++it )
    {
        const OString& rName = *it;

        // Output the bookmark
        sal_uInt16 nId = m_nNextMarkId++;
        m_rOpenedMarksIds[rName] = nId;
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkStart,
            FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
            FSNS( XML_w, XML_name ), rName.getStr(),
            FSEND );
        m_sLastOpenedMark = rName;
    }
    m_rMarksStart.clear();

    // export the end bookmarks
    for ( std::vector< OString >::const_iterator it = m_rMarksEnd.begin(), end = m_rMarksEnd.end();
          it != end; ++it )
    {
        const OString& rName = *it;

        // Get the id of the bookmark
        std::map< OString, sal_uInt16 >::iterator pPos = m_rOpenedMarksIds.find( rName );
        if ( pPos != m_rOpenedMarksIds.end(  ) )
        {
            sal_uInt16 nId = ( *pPos ).second;
            m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
                FSNS( XML_w, XML_id ), OString::number( nId ).getStr(  ),
                FSEND );
            m_rOpenedMarksIds.erase( rName );
        }
    }
    m_rMarksEnd.clear();
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
                    const SwDropDownField& rFld2 = *(SwDropDownField*)rInfos.pField;
                    uno::Sequence<OUString> aItems =
                        rFld2.GetItemSequence();
                    GetExport().DoComboBox(rFld2.GetName(),
                               rFld2.GetHelp(),
                               rFld2.GetToolTip(),
                               rFld2.GetSelectedItem(), aItems);
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
            m_pSerializer->startElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin",
                FSEND );

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
        m_aSeqMarksNames[sSeqName].push_back(m_sLastOpenedMark);
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
            RunText( OUString( "\t" ) );
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
               FSNS( XML_w, XML_id ), OString::number( m_nNextMarkId ).getStr( ),
               FSNS( XML_w, XML_name ), OUStringToOString( aBkmName, RTL_TEXTENCODING_UTF8 ).getStr( ),
               FSEND );
    }

    if (rInfos.pField ) // For hyperlinks and TOX
    {
        // Write the Field latest value
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

        OUString sExpand( rInfos.pField->ExpandField( true ) );
        // newlines embedded in fields are 0x0B in MSO and 0x0A for us
        RunText(sExpand.replace(0x0A, 0x0B));

        m_pSerializer->endElementNS( XML_w, XML_r );
    }

    // Write the bookmark end if any
    if ( !aBkmName.isEmpty() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
               FSNS( XML_w, XML_id ), OString::number( m_nNextMarkId ).getStr( ),
               FSEND );

        m_nNextMarkId++;
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
        bool bShowRef = ( !bIsSetField || ( nSubType & nsSwExtendedSubType::SUB_INVISIBLE ) ) ? false : true;

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
    m_pSerializer->mark();

    m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );

    InitCollectedRunProperties();

    OSL_ASSERT( m_postponedGraphic == NULL );
    m_postponedGraphic = new std::list< PostponedGraphic >;

    OSL_ASSERT( m_postponedDiagram == NULL );
    m_postponedDiagram = new std::list< PostponedDiagram >;

    OSL_ASSERT( m_postponedVMLDrawing == NULL );
    m_postponedVMLDrawing = new std::list< PostponedVMLDrawing >;
}

void DocxAttributeOutput::InitCollectedRunProperties()
{
    m_pFontsAttrList = NULL;
    m_pEastAsianLayoutAttrList = NULL;
    m_pCharLangAttrList = NULL;

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
        FSNS( XML_w, XML_rPrChange )
    };

    // postpone the output so that we can later [in EndParagraphProperties()]
    // prepend the properties before the run
    sal_Int32 len = sizeof ( aOrder ) / sizeof( sal_Int32 );
    uno::Sequence< sal_Int32 > aSeqOrder( len );
    for ( sal_Int32 i = 0; i < len; i++ )
        aSeqOrder[i] = aOrder[i];

    m_pSerializer->mark( aSeqOrder );

}

void DocxAttributeOutput::WriteCollectedRunProperties()
{
    // Write all differed properties
    if ( m_pFontsAttrList )
    {
        XFastAttributeListRef xAttrList( m_pFontsAttrList );
        m_pFontsAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_rFonts, xAttrList );
    }

    if ( m_pEastAsianLayoutAttrList )
    {
        XFastAttributeListRef xAttrList( m_pEastAsianLayoutAttrList );
        m_pEastAsianLayoutAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_eastAsianLayout, xAttrList );
    }

    if ( m_pCharLangAttrList )
    {
        XFastAttributeListRef xAttrList( m_pCharLangAttrList );
        m_pCharLangAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_lang, xAttrList );
    }

    // Merge the marks for the ordered elements
    m_pSerializer->mergeTopMarks();
}

void DocxAttributeOutput::EndRunProperties( const SwRedlineData* pRedlineData )
{
    // Call the 'Redline' function. This will add redline (change-tracking) information that regards to run properties.
    // This includes changes like 'Bold', 'Underline', 'Strikethrough' etc.
    Redline( pRedlineData );

    WriteCollectedRunProperties();

    m_pSerializer->endElementNS( XML_w, XML_rPr );

    // Clone <rPr>...</rPr> for later re-use, in pPr
    m_pSerializer->copyTopMarkPush();

    // write footnotes/endnotes if we have any
    FootnoteEndnoteReference();

    WritePostponedGraphic();

    WritePostponedDiagram();
    //We need to write w:drawing tag after the w:rPr.
    WritePostponedChart();

    //We need to write w:pict tag after the w:rPr.
    WritePostponedVMLDrawing();

    // merge the properties _before_ the run text (strictly speaking, just
    // after the start of the run)
    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND );
}

void DocxAttributeOutput::WritePostponedGraphic()
{
    for( std::list< PostponedGraphic >::const_iterator it = m_postponedGraphic->begin();
         it != m_postponedGraphic->end();
         ++it )
        FlyFrameGraphic( it->grfNode, it->size, 0, 0, it->pSdrObj );
    delete m_postponedGraphic;
    m_postponedGraphic = NULL;
}

void DocxAttributeOutput::WritePostponedDiagram()
{
    for( std::list< PostponedDiagram >::const_iterator it = m_postponedDiagram->begin();
         it != m_postponedDiagram->end();
         ++it )
        WriteDiagram( it->object, it->size );
    delete m_postponedDiagram;
    m_postponedDiagram = NULL;
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
static void impl_WriteRunText( FSHelperPtr pSerializer, sal_Int32 nTextToken,
        const sal_Unicode* &rBegin, const sal_Unicode* pEnd, bool bMove = true )
{
    const sal_Unicode *pBegin = rBegin;

    // skip one character after the end
    if ( bMove )
        rBegin = pEnd + 1;

    if ( pBegin >= pEnd )
        return; // we want to write at least one character

    // we have to add 'preserve' when starting/ending with space
    if ( *pBegin == ' ' || *( pEnd - 1 ) == ' ' )
    {
        pSerializer->startElementNS( XML_w, nTextToken, FSNS( XML_xml, XML_space ), "preserve", FSEND );
    }
    else
        pSerializer->startElementNS( XML_w, nTextToken, FSEND );

    pSerializer->writeEscaped( OUString( pBegin, pEnd - pBegin ) );

    pSerializer->endElementNS( XML_w, nTextToken );
}

void DocxAttributeOutput::RunText( const OUString& rText, rtl_TextEncoding /*eCharSet*/ )
{
    if( m_closeHyperlinkInThisRun )
    {
        m_closeHyperlinkInPreviousRun = true;
    }

    // one text can be split into more <w:t>blah</w:t>'s by line breaks etc.
    const sal_Unicode *pBegin = rText.getStr();
    const sal_Unicode *pEnd = pBegin + rText.getLength();

    // the text run is usually XML_t, with the exception of the deleted text
    sal_Int32 nTextToken = XML_t;
    if ( m_pRedlineData && m_pRedlineData->GetType() == nsRedlineType_t::REDLINE_DELETE )
        nTextToken = XML_delText;

    for ( const sal_Unicode *pIt = pBegin; pIt < pEnd; ++pIt )
    {
        switch ( *pIt )
        {
            case 0x09: // tab
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_tab, FSEND );
                break;
            case 0x0b: // line break
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_br, FSEND );
                break;
            case 0x1E: //non-breaking hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_noBreakHyphen, FSEND );
                break;
            case 0x1F: //soft (on demand) hyphen
                impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                m_pSerializer->singleElementNS( XML_w, XML_softHyphen, FSEND );
                break;
            default:
                if ( *pIt < 0x0020 ) // filter out the control codes
                {
                    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pIt );
                    OSL_TRACE( "Ignored control code %x in a text run.", *pIt );
                }
                break;
        }
    }

    impl_WriteRunText( m_pSerializer, nTextToken, pBegin, pEnd, false );
}

void DocxAttributeOutput::RawText( const OUString& /*rText*/, bool /*bForceUnicode*/, rtl_TextEncoding /*eCharSet*/ )
{
    OSL_TRACE("TODO DocxAttributeOutput::RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet )" );
}

void DocxAttributeOutput::StartRuby( const SwTxtNode& rNode, xub_StrLen nPos, const SwFmtRuby& rRuby )
{
    OSL_TRACE("TODO DocxAttributeOutput::StartRuby( const SwTxtNode& rNode, const SwFmtRuby& rRuby )" );
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
    StartRun( NULL );
    StartRunProperties( );
    SwWW8AttrIter aAttrIt( m_rExport, rNode );
    aAttrIt.OutAttr( nPos, true );

    sal_uInt16 nStyle = m_rExport.GetId( *rRuby.GetTxtRuby()->GetCharFmt() );
    OString aStyleId(m_rExport.pStyles->GetStyleId(nStyle));
    m_pSerializer->singleElementNS( XML_w, XML_rStyle,
            FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );

    EndRunProperties( NULL );
    RunText( rRuby.GetText( ) );
    EndRun( );
    m_pSerializer->endElementNS( XML_w, XML_rt );

    m_pSerializer->startElementNS( XML_w, XML_rubyBase, FSEND );
    StartRun( NULL );
}

void DocxAttributeOutput::EndRuby()
{
    OSL_TRACE( "TODO DocxAttributeOutput::EndRuby()" );
    EndRun( );
    m_pSerializer->endElementNS( XML_w, XML_rubyBase );
    m_pSerializer->endElementNS( XML_w, XML_ruby );
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

    if ( !sMark.isEmpty() && !bBookmarkOnly )
    {
        m_rExport.OutputField( NULL, ww::eHYPERLINK, sUrl );
    }
    else
    {
        // Output a hyperlink XML element
        m_pHyperlinkAttrList = m_pSerializer->createAttrList();

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
                    OUString aSequenceName = OUString('"') + sMark.copy(0, nPos) + OUString('"');
                    // Extract <index>.
                    sal_uInt32 nIndex = sMark.copy(nPos + 1, sMark.getLength() - nPos - sizeof("|sequence")).toInt32();
                    std::map<OUString, std::vector<OString> >::iterator it = m_aSeqMarksNames.find(aSequenceName);
                    if (it != m_aSeqMarksNames.end())
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
    return true;
}

void DocxAttributeOutput::FieldVanish( const OUString& rTxt, ww::eField eType )
{
    WriteField_Impl( NULL, eType, rTxt, WRITEFIELD_ALL );
}

// The difference between 'Redline' and 'StartRedline'+'EndRedline' is that:
// 'Redline' is used for tracked changes of formatting information of a run like Bold, Underline. (the '<w:rPrChange>' is inside the 'run' node)
// 'StartRedline' is used to output tracked changes of run insertion and deletion (the run is inside the '<w:ins>' node)
void DocxAttributeOutput::Redline( const SwRedlineData* pRedline)
{
    if ( !pRedline )
        return;

    OString aId( OString::number( pRedline->GetSeqNo() ) );
    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedline->GetAuthor() ) );
    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );
    OString aDate( msfilter::util::DateTimeToOString( pRedline->GetTimeStamp() ) );

    OUString sVal;
    OString sOVal;

    switch( pRedline->GetType() )
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

        if ( m_pCharLangAttrList )
        {
            if (m_pCharLangAttrList->hasAttribute(FSNS(XML_w, XML_val)))
            {
                m_pSerializer->mark();
                m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
                sVal = m_pCharLangAttrList->getValue(FSNS(XML_w, XML_val));
                sOVal = OUStringToOString(sVal, RTL_TEXTENCODING_UTF8);
                m_pSerializer->startElementNS(XML_w, XML_lang,
                    FSNS(XML_w, XML_val), sOVal.getStr(),
                    FSEND);
                m_pSerializer->endElementNS(XML_w, XML_lang);
                m_pSerializer->endElementNS( XML_w, XML_rPr );
                m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND );
            }
        }

        m_pSerializer->endElementNS( XML_w, XML_rPrChange );
        break;
    default:
        SAL_WARN("sw.ww8", "Unhandled redline type for export " << pRedline->GetType());
        break;
    }
}

// The difference between 'Redline' and 'StartRedline'+'EndRedline' is that:
// 'Redline' is used for tracked changes of formatting information of a run like Bold, Underline. (the '<w:rPrChange>' is inside the 'run' node)
// 'StartRedline' is used to output tracked changes of run insertion and deletion (the run is inside the '<w:ins>' node)
void DocxAttributeOutput::StartRedline()
{
    if ( !m_pRedlineData )
        return;
    const SwRedlineData* pRedlineData = m_pRedlineData;

    // FIXME check if it's necessary to travel over the Next()'s in pRedlineData

    OString aId( OString::number( m_nRedlineId++ ) );

    const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );

    OString aDate( msfilter::util::DateTimeToOString( pRedlineData->GetTimeStamp() ) );

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

void DocxAttributeOutput::EndRedline()
{
    if ( !m_pRedlineData )
        return;

    switch ( m_pRedlineData->GetType() )
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

    m_pRedlineData = NULL;
}

void DocxAttributeOutput::FormatDrop( const SwTxtNode& /*rNode*/, const SwFmtDrop& /*rSwFmtDrop*/, sal_uInt16 /*nStyle*/, ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/, ww8::WW8TableNodeInfoInner::Pointer_t )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle )" );
}

void DocxAttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OString aStyleId(m_rExport.pStyles->GetStyleId(nStyle));

    m_pSerializer->singleElementNS( XML_w, XML_pStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

static void impl_borderLine( FSHelperPtr pSerializer, sal_Int32 elementToken, const SvxBorderLine* pBorderLine, sal_uInt16 nDist, bool bWriteShadow = false )
{
    FastAttributeList* pAttr = pSerializer->createAttrList();


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
                pVal = ( sal_Char* )"single";
                break;
            case table::BorderLineStyle::DOTTED:
                pVal = ( sal_Char* )"dotted";
                break;
            case table::BorderLineStyle::DASHED:
                pVal = ( sal_Char* )"dashed";
                break;
            case table::BorderLineStyle::DOUBLE:
                pVal = ( sal_Char* )"double";
                break;
            case table::BorderLineStyle::THINTHICK_SMALLGAP:
                pVal = ( sal_Char* )"thinThickSmallGap";
                break;
            case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
                pVal = ( sal_Char* )"thinThickMediumGap";
                break;
            case table::BorderLineStyle::THINTHICK_LARGEGAP:
                pVal = ( sal_Char* )"thinThickLargeGap";
                break;
            case table::BorderLineStyle::THICKTHIN_SMALLGAP:
                pVal = ( sal_Char* )"thickThinSmallGap";
                break;
            case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
                pVal = ( sal_Char* )"thickThinMediumGap";
                break;
            case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                pVal = ( sal_Char* )"thickThinLargeGap";
                break;
            case table::BorderLineStyle::EMBOSSED:
                pVal = ( sal_Char* )"threeDEmboss";
                break;
            case table::BorderLineStyle::ENGRAVED:
                pVal = ( sal_Char* )"threeDEngrave";
                break;
            case table::BorderLineStyle::OUTSET:
                pVal = ( sal_Char* )"outset";
                break;
            case table::BorderLineStyle::INSET:
                pVal = ( sal_Char* )"inset";
                break;
            case table::BorderLineStyle::FINE_DASHED:
                pVal = ( sal_Char* )"dashSmallGap";
                break;
            case table::BorderLineStyle::NONE:
            default:
                break;
        }
    }

    pAttr->add( FSNS( XML_w, XML_val ), OString( pVal ) );

    if ( pBorderLine && !pBorderLine->isEmpty() )
    {
        // Compute the sz attribute

        double const fConverted( ::editeng::ConvertBorderWidthToWord(
                pBorderLine->GetBorderLineStyle(), pBorderLine->GetWidth()));
        // The unit is the 8th of point
        sal_Int32 nWidth = sal_Int32( fConverted / 2.5 );
        sal_uInt16 nMinWidth = 2;
        sal_uInt16 nMaxWidth = 96;

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
                ( rBox.GetDistance( BOX_LINE_TOP ) / 20 ) > 31 ||
                ( rBox.GetDistance( BOX_LINE_LEFT ) / 20 ) > 31 ||
                ( rBox.GetDistance( BOX_LINE_BOTTOM ) / 20 ) > 31 ||
                ( rBox.GetDistance( BOX_LINE_RIGHT ) / 20 ) > 31
            );
}

static void impl_borders( FSHelperPtr pSerializer, const SvxBoxItem& rBox, const OutputBorderOptions& rOptions, PageMargins* pageMargins)
{
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    const sal_Int32 aXmlElements[] =
    {
        XML_top,
        rOptions.bUseStartEnd ? XML_start : XML_left,
        XML_bottom,
        rOptions.bUseStartEnd ? XML_end : XML_right
    };
    bool tagWritten = false;
    const sal_uInt16* pBrd = aBorders;

    bool bExportDistanceFromPageEdge = false;
    if ( rOptions.bCheckDistanceSize == true && boxHasLineLargerThan31(rBox) == true )
    {
        // The distance is larger than '31'. This cannot be exported as 'distance from text'.
        // Instead - it should be exported as 'distance from page edge'.
        // This is based on http://wiki.openoffice.org/wiki/Writer/MSInteroperability/PageBorder
        // Specifically 'export case #2'
        bExportDistanceFromPageEdge = true;
    }

    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );

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
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPLEFT     || rOptions.aShadowLocation == SVX_SHADOW_TOPRIGHT      )    &&  *pBrd == BOX_LINE_TOP   )  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPLEFT     || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMLEFT    )    &&  *pBrd == BOX_LINE_LEFT  )  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_BOTTOMLEFT  || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMRIGHT   )    &&  *pBrd == BOX_LINE_BOTTOM)  ||
                    ( ( rOptions.aShadowLocation == SVX_SHADOW_TOPRIGHT    || rOptions.aShadowLocation == SVX_SHADOW_BOTTOMRIGHT   )    &&  *pBrd == BOX_LINE_RIGHT )
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
                if ( *pBrd == BOX_LINE_TOP)
                    nDist = pageMargins->nPageMarginTop - rBox.GetDistance( *pBrd );
                else if ( *pBrd == BOX_LINE_LEFT)
                    nDist = pageMargins->nPageMarginLeft - rBox.GetDistance( *pBrd );
                else if ( *pBrd == BOX_LINE_BOTTOM)
                    nDist = pageMargins->nPageMarginBottom - rBox.GetDistance( *pBrd );
                else if ( *pBrd == BOX_LINE_RIGHT)
                    nDist = pageMargins->nPageMarginRight - rBox.GetDistance( *pBrd );
            }
            else
            {
                // Export 'Distance from text'
                nDist = rBox.GetDistance( *pBrd );
            }
        }

        impl_borderLine( pSerializer, aXmlElements[i], pLn, nDist, bWriteShadow );

        // When exporting default borders, we need to export these 2 attr
        if ( rOptions.bWriteInsideHV) {
            if ( i == 2 )
                impl_borderLine( pSerializer, XML_insideH, pLn, 0 );
            else if ( i == 3 )
                impl_borderLine( pSerializer, XML_insideV, pLn, 0 );
        }
    }
    if (tagWritten && rOptions.bWriteTag) {
        pSerializer->endElementNS( XML_w, rOptions.tag );
    }
}

static void impl_cellMargins( FSHelperPtr pSerializer, const SvxBoxItem& rBox, sal_Int32 tag, bool bUseStartEnd = false, const SvxBoxItem* pDefaultMargins = 0)
{
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    const sal_Int32 aXmlElements[] =
    {
        XML_top,
        bUseStartEnd ? XML_start : XML_left,
        XML_bottom,
        bUseStartEnd ? XML_end : XML_right
    };
    bool tagWritten = false;
    const sal_uInt16* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        sal_Int32 nDist = sal_Int32( rBox.GetDistance( *pBrd ) );

        if ( aBorders[i] == BOX_LINE_LEFT ) {
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

void DocxAttributeOutput::TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    m_pSerializer->startElementNS( XML_w, XML_tcPr, FSEND );

    const SwTableBox *pTblBox = pTableTextNodeInfoInner->getTableBox( );

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // Cell prefered width
    SwTwips nWidth = GetGridCols( pTableTextNodeInfoInner )->at( pTableTextNodeInfoInner->getCell() );
    if ( pTableTextNodeInfoInner->getCell() )
        nWidth = nWidth - GetGridCols( pTableTextNodeInfoInner )->at( pTableTextNodeInfoInner->getCell() - 1 );
    m_pSerializer->singleElementNS( XML_w, XML_tcW,
           FSNS( XML_w, XML_w ), OString::number( nWidth ).getStr( ),
           FSNS( XML_w, XML_type ), "dxa",
           FSEND );

    // Horizontal spans
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    const SwWriteTableCell *pCell = &pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];

    sal_uInt16 nColSpan = pCell->GetColSpan();
    if ( nColSpan > 1 )
        m_pSerializer->singleElementNS( XML_w, XML_gridSpan,
                FSNS( XML_w, XML_val ), OString::number( nColSpan ).getStr(),
                FSEND );

    // Vertical merges
    long vSpan = pTblBox->getRowSpan( );
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

    const SvxBoxItem& rBox = pTblBox->GetFrmFmt( )->GetBox( );
    const SvxBoxItem& rDefaultBox = (*tableFirstCells.rbegin())->getTableBox( )->GetFrmFmt( )->GetBox( );
    {
        // The cell borders
        impl_borders( m_pSerializer, rBox, lcl_getTableCellBorderOptions(bEcma), NULL );
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
    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans (and maybe other infos)
    GetTablePageSize( pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize );

    const SwTable* pTable = pTableTextNodeInfoInner->getTable( );
    const SwFrmFmt *pFmt = pTable->GetFrmFmt( );
    SwTwips nTblSz = pFmt->GetFrmSize( ).GetWidth( );

    const SwHTMLTableLayout *pLayout = pTable->GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        m_pTableWrt = new SwWriteTable( pLayout );
    else
        m_pTableWrt = new SwWriteTable( pTable->GetTabLines(), (sal_uInt16)nPageSize,
                (sal_uInt16)nTblSz, false);
}

void DocxAttributeOutput::StartTable( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    m_pSerializer->startElementNS( XML_w, XML_tbl, FSEND );

    tableFirstCells.push_back(pTableTextNodeInfoInner);

    InitTableHelper( pTableTextNodeInfoInner );
    TableDefinition( pTableTextNodeInfoInner );
}

void DocxAttributeOutput::EndTable()
{
    m_pSerializer->endElementNS( XML_w, XML_tbl );

    if ( m_nTableDepth > 0 )
        --m_nTableDepth;

    tableFirstCells.pop_back();

    // We closed the table; if it is a nested table, the cell that contains it
    // still continues
    m_bTableCellOpen = true;

    // Cleans the table helper
    delete m_pTableWrt, m_pTableWrt = NULL;
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

    TableHeight( pTableTextNodeInfoInner );
    TableCanSplit( pTableTextNodeInfoInner );

    m_pSerializer->endElementNS( XML_w, XML_trPr );
}

void DocxAttributeOutput::EndTableRow( )
{
    m_pSerializer->endElementNS( XML_w, XML_tr );
}

void DocxAttributeOutput::StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    if ( !m_pTableWrt )
        InitTableHelper( pTableTextNodeInfoInner );

    m_pSerializer->startElementNS( XML_w, XML_tc, FSEND );

    // Write the cell properties here
    TableCellProperties( pTableTextNodeInfoInner );

    m_bTableCellOpen = true;
}

void DocxAttributeOutput::EndTableCell( )
{
    m_pSerializer->endElementNS( XML_w, XML_tc );

    m_bBtLr = false;
    m_bTableCellOpen = false;
}

void DocxAttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
}

void DocxAttributeOutput::TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfo*/ )
{
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

    m_pSerializer->mark( aSeqOrder );

    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans (and maybe other infos)
    GetTablePageSize( pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize );

    // Output the table prefered width
    if ( nPageSize != 0 )
        m_pSerializer->singleElementNS( XML_w, XML_tblW,
                FSNS( XML_w, XML_w ), OString::number( nPageSize ).getStr( ),
                FSNS( XML_w, XML_type ), "dxa",
                FSEND );

    // Output the table alignement
    const SwTable *pTable = pTableTextNodeInfoInner->getTable();
    SwFrmFmt *pTblFmt = pTable->GetFrmFmt( );
    const char* pJcVal;
    sal_Int32 nIndent = 0;
    switch ( pTblFmt->GetHoriOrient( ).GetHoriOrient( ) )
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
            nIndent = sal_Int32( pTblFmt->GetLRSpace( ).GetLeft( ) );
            // Table indentation has different meaning in Word, depending if the table is nested or not.
            // If nested, tblInd is added to parent table's left spacing and defines left edge position
            // If not nested, text position of left-most cell must be at absolute X = tblInd
            // so, table_spacing + table_spacing_to_content = tblInd
            if (m_nTableDepth == 0)
            {
                const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
                const SwFrmFmt * pFrmFmt = pTabBox->GetFrmFmt();
                nIndent += sal_Int32( pFrmFmt->GetBox( ).GetDistance( BOX_LINE_LEFT ) );
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
    m_pSerializer->mergeTopMarks( );

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
    const SwFrmFmt * pFrmFmt = pTabBox->GetFrmFmt();

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    // the defaults of the table are taken from the top-left cell
    impl_borders( m_pSerializer, pFrmFmt->GetBox( ), lcl_getTableDefaultBorderOptions(bEcma), NULL );
}

void DocxAttributeOutput::TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrmFmt * pFrmFmt = pTabBox->GetFrmFmt();
    const SvxBoxItem& rBox = pFrmFmt->GetBox( );
    const bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    impl_cellMargins(m_pSerializer, rBox, XML_tblCellMar, !bEcma);
}

void DocxAttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox *pTblBox = pTableTextNodeInfoInner->getTableBox( );
    const SwFrmFmt *pFmt = pTblBox->GetFrmFmt( );
    const SfxPoolItem *pI = NULL;

    Color aColor;
    if ( SFX_ITEM_ON == pFmt->GetAttrSet().GetItemState( RES_BACKGROUND, false, &pI ) )
        aColor = dynamic_cast<const SvxBrushItem *>(pI)->GetColor();
    else
        aColor = COL_AUTO;

    OString sColor = msfilter::util::ConvertColor( aColor );
    m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_fill ), sColor.getStr( ),
            FSNS( XML_w, XML_val ), "clear",
            FSEND );
}

void DocxAttributeOutput::TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();

    const SwFmtFrmSize& rLSz = pLineFmt->GetFrmSize();
    if ( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
    {
        sal_Int32 nHeight = rLSz.GetHeight();
        const char *pRule = NULL;

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
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();

    const SwFmtRowSplit& rSplittable = pLineFmt->GetRowSplit( );
    const char* pCantSplit = ( !rSplittable.GetValue( ) ) ? "true" : "false";

    m_pSerializer->singleElementNS( XML_w, XML_cantSplit,
           FSNS( XML_w, XML_val ), pCantSplit,
           FSEND );
}

void DocxAttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrmFmt * pFrmFmt = pTable->GetFrmFmt();

    if ( m_rExport.TrueFrameDirection( *pFrmFmt ) == FRMDIR_HORI_RIGHT_TOP )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bidiVisual,
                FSNS( XML_w, XML_val ), "true",
                FSEND );
    }
}

void DocxAttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrmFmt *pFrmFmt = pTabBox->GetFrmFmt( );

    if ( FRMDIR_VERT_TOP_RIGHT == m_rExport.TrueFrameDirection( *pFrmFmt ) )
        m_pSerializer->singleElementNS( XML_w, XML_textDirection,
               FSNS( XML_w, XML_val ), "tbRl",
               FSEND );
    else if ( FRMDIR_HORI_LEFT_TOP == m_rExport.TrueFrameDirection( *pFrmFmt ) )
    {
        // Undo the text direction mangling done by the btLr handler in writerfilter::dmapper::DomainMapperTableManager::sprm()
        SwPaM aPam(*pTabBox->GetSttNd(), 0);
        aPam.GetPoint()->nNode++;
        if (aPam.GetPoint()->nNode.GetNode().IsTxtNode())
        {
            const SwTxtNode& rTxtNode = (const SwTxtNode&)aPam.GetPoint()->nNode.GetNode();
            if( const SwAttrSet* pAttrSet = rTxtNode.GetpSwAttrSet())
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

    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    const SwWriteTableCell *pCell = &pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];
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

void DocxAttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t /*pNodeInfo*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo )" );
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
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "TODO: DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )\n" );
#endif
}

void DocxAttributeOutput::TableRowEnd( sal_uInt32 /*nDepth*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableRowEnd( sal_uInt32 nDepth = 1 )" );
}

void DocxAttributeOutput::StartStyles()
{
    m_pSerializer->startElementNS( XML_w, XML_styles,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );

    DocDefaults();
    LatentStyles();
}

sal_Int32 DocxStringGetToken(DocxStringTokenMap const * pMap, OUString aName)
{
    OString sName = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
    while (pMap->pToken)
    {
        if (sName == pMap->pToken)
            return pMap->nToken;
        ++pMap;
    }
    return 0;
}

DocxStringTokenMap const aDefaultTokens[] = {
    {"defQFormat", XML_defQFormat},
    {"defUnhideWhenUsed", XML_defUnhideWhenUsed},
    {"defSemiHidden", XML_defSemiHidden},
    {"count", XML_count},
    {"defUIPriority", XML_defUIPriority},
    {"defLockedState", XML_defLockedState},
    {0, 0}
};

DocxStringTokenMap const aExceptionTokens[] = {
    {"name", XML_name},
    {"locked", XML_locked},
    {"uiPriority", XML_uiPriority},
    {"semiHidden", XML_semiHidden},
    {"unhideWhenUsed", XML_unhideWhenUsed},
    {"qFormat", XML_qFormat},
    {0, 0}
};

void DocxAttributeOutput::LatentStyles()
{
    // Do we have latent styles available?
    uno::Reference<beans::XPropertySet> xPropertySet(m_rExport.pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
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
    sax_fastparser::FastAttributeList* pAttributeList = m_pSerializer->createAttrList();
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
    pAttributeList = 0;

    // Then handle the exceptions.
    for (sal_Int32 i = 0; i < aLsdExceptions.getLength(); ++i)
    {
        pAttributeList = m_pSerializer->createAttrList();

        uno::Sequence<beans::PropertyValue> aAttributes;
        aLsdExceptions[i].Value >>= aAttributes;
        for (sal_Int32 j = 0; j < aAttributes.getLength(); ++j)
            if (sal_Int32 nToken = DocxStringGetToken(aExceptionTokens, aAttributes[j].Name))
                pAttributeList->add(FSNS(XML_w, nToken), OUStringToOString(aAttributes[j].Value.get<OUString>(), RTL_TEXTENCODING_UTF8));

        xAttributeList = pAttributeList;
        m_pSerializer->singleElementNS(XML_w, XML_lsdException, xAttributeList);
        pAttributeList = 0;
    }

    m_pSerializer->endElementNS(XML_w, XML_latentStyles);
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
                              rBrushItem.GetShadingValue() != sal_uInt32(ShadingPattern::CLEAR) ||
                              rBrushItem.GetGraphic() != NULL ||
                              rBrushItem.GetGraphicObject() != NULL);
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
                              rBrushItem.GetShadingValue() != sal_uInt32(ShadingPattern::CLEAR) ||
                              rBrushItem.GetGraphic() != NULL ||
                              rBrushItem.GetGraphicObject() != NULL);
            }
            break;

        case RES_PARATR_LINESPACING:
            bMustWrite = static_cast< const SvxLineSpacingItem& >(rHt).GetInterLineSpaceRule() != SVX_INTER_LINE_SPACE_OFF;
            break;
        case RES_PARATR_ADJUST:
            bMustWrite = static_cast< const SvxAdjustItem& >(rHt).GetAdjust() != SVX_ADJUST_LEFT;
            break;
        case RES_PARATR_SPLIT:
            bMustWrite = !static_cast< const SvxFmtSplitItem& >(rHt).GetValue();
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
        OutputDefaultItem(m_rExport.pDoc->GetDefault(i));

    EndStyleProperties(false);

    m_pSerializer->endElementNS(XML_w, XML_rPrDefault);

    // Output the default paragraph properties
    m_pSerializer->startElementNS(XML_w, XML_pPrDefault, FSEND);

    StartStyleProperties(true, 0);

    for (int i = int(RES_PARATR_BEGIN); i < int(RES_PARATR_END); ++i)
        OutputDefaultItem(m_rExport.pDoc->GetDefault(i));

    EndStyleProperties(true);

    m_pSerializer->endElementNS(XML_w, XML_pPrDefault);

    m_pSerializer->endElementNS(XML_w, XML_docDefaults);
}

void DocxAttributeOutput::EndStyles( sal_uInt16 /*nNumberOfStyles*/ )
{
    m_pTableStyleExport->TableStyles();
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

// Converts ARGB transparency (0..255) to drawingml alpha (opposite, and 0..100000)
OString lcl_ConvertTransparency(const Color& rColor)
{
    if (rColor.GetTransparency() > 0)
    {
        sal_Int32 nTransparencyPercent = 100 - float(rColor.GetTransparency()) / 2.55;
        return OString::number(nTransparencyPercent * oox::drawingml::PER_PERCENT);
    }
    else
        return OString("");
}

/* Writes <a:srcRect> tag back to document.xml if a file conatins a cropped image.
*  NOTE : Tested on images of type JPEG,EMF/WMF,BMP, PNG and GIF.
*/
void DocxAttributeOutput::WriteSrcRect(const SdrObject* pSdrObj )
{
    uno::Reference< drawing::XShape > xShape( ((SdrObject*)pSdrObj)->getUnoShape(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );

    OUString sUrl;
    xPropSet->getPropertyValue("GraphicURL") >>= sUrl;
    Size aOriginalSize( GraphicObject::CreateGraphicObjectFromURL( sUrl ).GetPrefSize() );

    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;
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

void DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrmFmt* pOLEFrmFmt, SwOLENode* pOLENode, const SdrObject* pSdrObj )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrmFmt* pOLEFrmFmt, SwOLENode* pOLENode, const SdrObject* pSdrObj  ) - some stuff still missing" );
    // detect mis-use of the API
    assert(pGrfNode || (pOLEFrmFmt && pOLENode));
    const SwFrmFmt* pFrmFmt = pGrfNode ? pGrfNode->GetFlyFmt() : pOLEFrmFmt;
    // create the relation ID
    OString aRelId;
    sal_Int32 nImageType;
    if ( pGrfNode && pGrfNode->IsLinkedFile() )
    {
        // linked image, just create the relation
        OUString aFileName;
        pGrfNode->GetFileFilterNms( &aFileName, 0 );

        // TODO Convert the file name to relative for better interoperability

        aRelId = m_rExport.AddRelation(
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image",
                    aFileName );

        nImageType = XML_link;
    }
    else
    {
        // inline, we also have to write the image itself
        const Graphic* pGraphic = 0;
        if (pGrfNode)
            pGraphic = &pGrfNode->GetGrf();
        else
            pGraphic = pOLENode->GetGraphic();

        m_rDrawingML.SetFS( m_pSerializer ); // to be sure that we write to the right stream

        bool bSwapped = pGraphic->IsSwapOut();
        if (bSwapped)
        {
            if (pGrfNode)
            {
                // always swapin via the Node
                const_cast<SwGrfNode*>(pGrfNode)->SwapIn();
            }
            else
                const_cast<Graphic*>(pGraphic)->SwapIn();
        }

        OUString aImageId = m_rDrawingML.WriteImage( *pGraphic );

        if (bSwapped)
            const_cast<Graphic*>(pGraphic)->SwapOut();

        aRelId = OUStringToOString( aImageId, RTL_TEXTENCODING_UTF8 );

        nImageType = XML_embed;
    }

    if ( aRelId.isEmpty() )
        return;

    m_pSerializer->startElementNS( XML_w, XML_drawing, FSEND );

    const SvxLRSpaceItem pLRSpaceItem = pFrmFmt->GetLRSpace(false);
    const SvxULSpaceItem pULSpaceItem = pFrmFmt->GetULSpace(false);

    bool isAnchor = pFrmFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR;
    if( isAnchor )
    {
        ::sax_fastparser::FastAttributeList* attrList = m_pSerializer->createAttrList();
        attrList->add( XML_behindDoc, pFrmFmt->GetOpaque().GetValue() ? "0" : "1" );
        attrList->add( XML_distT, OString::number( TwipsToEMU( pULSpaceItem.GetUpper() ) ).getStr( ) );
        attrList->add( XML_distB, OString::number( TwipsToEMU( pULSpaceItem.GetLower() ) ).getStr( ) );
        attrList->add( XML_distL, OString::number( TwipsToEMU( pLRSpaceItem.GetLeft() ) ).getStr( ) );
        attrList->add( XML_distR, OString::number( TwipsToEMU( pLRSpaceItem.GetRight() ) ).getStr( ) );
        attrList->add( XML_simplePos, "0" );
        attrList->add( XML_locked, "0" );
        attrList->add( XML_layoutInCell, "1" );
        attrList->add( XML_allowOverlap, "1" ); // TODO
        if( const SdrObject* pObj = pFrmFmt->FindRealSdrObject())
            attrList->add( XML_relativeHeight, OString::number( pObj->GetOrdNum()));
        m_pSerializer->startElementNS( XML_wp, XML_anchor, XFastAttributeListRef( attrList ));
        m_pSerializer->singleElementNS( XML_wp, XML_simplePos, XML_x, "0", XML_y, "0", FSEND ); // required, unused
        const char* relativeFromH;
        const char* relativeFromV;
        const char* alignH = NULL;
        const char* alignV = NULL;
        switch (pFrmFmt->GetVertOrient().GetRelationOrient() )
        {
            case text::RelOrientation::PAGE_PRINT_AREA:
                relativeFromV = "margin";
                break;
            case text::RelOrientation::PAGE_FRAME:
                relativeFromV = "page";
                break;
            case text::RelOrientation::FRAME:
                relativeFromV = "paragraph";
                break;
            case text::RelOrientation::TEXT_LINE:
            default:
                relativeFromV = "line";
                break;
        }
        switch (pFrmFmt->GetVertOrient().GetVertOrient() )
        {
            case text::VertOrientation::TOP:
            case text::VertOrientation::CHAR_TOP:
            case text::VertOrientation::LINE_TOP:
                if( pFrmFmt->GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                    alignV = "bottom";
                else
                    alignV = "top";
                break;
            case text::VertOrientation::BOTTOM:
            case text::VertOrientation::CHAR_BOTTOM:
            case text::VertOrientation::LINE_BOTTOM:
                if( pFrmFmt->GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                    alignV = "top";
                else
                    alignV = "bottom";
                break;
            case text::VertOrientation::CENTER:
            case text::VertOrientation::CHAR_CENTER:
            case text::VertOrientation::LINE_CENTER:
                alignV = "center";
                break;
            default:
                break;
        }
        switch (pFrmFmt->GetHoriOrient().GetRelationOrient() )
        {
            case text::RelOrientation::PAGE_PRINT_AREA:
                relativeFromH = "margin";
                break;
            case text::RelOrientation::PAGE_FRAME:
                relativeFromH = "page";
                break;
            case text::RelOrientation::CHAR:
                relativeFromH = "character";
                break;
            case text::RelOrientation::FRAME:
            default:
                relativeFromH = "column";
                break;
        }
        switch (pFrmFmt->GetHoriOrient().GetHoriOrient() )
        {
            case text::HoriOrientation::LEFT:
                alignH = "left";
                break;
            case text::HoriOrientation::RIGHT:
                alignH = "right";
                break;
            case text::HoriOrientation::CENTER:
                alignH = "center";
                break;
            case text::HoriOrientation::INSIDE:
                alignH = "inside";
                break;
            case text::HoriOrientation::OUTSIDE:
                alignH = "outside";
                break;
            default:
                break;
        }
        m_pSerializer->startElementNS( XML_wp, XML_positionH, XML_relativeFrom, relativeFromH, FSEND );
        if( alignH != NULL )
        {
            m_pSerializer->startElementNS( XML_wp, XML_align, FSEND );
            m_pSerializer->write( alignH );
            m_pSerializer->endElementNS( XML_wp, XML_align );
        }
        else
        {
            m_pSerializer->startElementNS( XML_wp, XML_posOffset, FSEND );
            m_pSerializer->write( TwipsToEMU( pFrmFmt->GetHoriOrient().GetPos()));
            m_pSerializer->endElementNS( XML_wp, XML_posOffset );
        }
        m_pSerializer->endElementNS( XML_wp, XML_positionH );
        m_pSerializer->startElementNS( XML_wp, XML_positionV, XML_relativeFrom, relativeFromV, FSEND );
        if( alignV != NULL )
        {
            m_pSerializer->startElementNS( XML_wp, XML_align, FSEND );
            m_pSerializer->write( alignV );
            m_pSerializer->endElementNS( XML_wp, XML_align );
        }
        else
        {
            m_pSerializer->startElementNS( XML_wp, XML_posOffset, FSEND );
            m_pSerializer->write( TwipsToEMU( pFrmFmt->GetVertOrient().GetPos()));
            m_pSerializer->endElementNS( XML_wp, XML_posOffset );
        }
        m_pSerializer->endElementNS( XML_wp, XML_positionV );
    }
    else
    {
        m_pSerializer->startElementNS( XML_wp, XML_inline,
                XML_distT, OString::number( TwipsToEMU( pULSpaceItem.GetUpper() ) ).getStr( ),
                XML_distB, OString::number( TwipsToEMU( pULSpaceItem.GetLower() ) ).getStr( ),
                XML_distL, OString::number( TwipsToEMU( pLRSpaceItem.GetLeft() ) ).getStr( ),
                XML_distR, OString::number( TwipsToEMU( pLRSpaceItem.GetRight() ) ).getStr( ),
                FSEND );
    }
    // now the common parts
    // extent of the image
    OString aWidth( OString::number( TwipsToEMU( rSize.Width() ) ) );
    OString aHeight( OString::number( TwipsToEMU( rSize.Height() ) ) );
    m_pSerializer->singleElementNS( XML_wp, XML_extent,
            XML_cx, aWidth.getStr(),
            XML_cy, aHeight.getStr(),
            FSEND );

    // effectExtent, extent including the effect (shadow only for now)
    SvxShadowItem aShadowItem = pFrmFmt->GetShadow();
    OString aLeftExt("0"), aRightExt("0"), aTopExt("0"), aBottomExt("0");
    if ( aShadowItem.GetLocation() != SVX_SHADOW_NONE )
    {
        OString aShadowWidth( OString::number( TwipsToEMU( aShadowItem.GetWidth() ) ) );
        switch ( aShadowItem.GetLocation() )
        {
            case SVX_SHADOW_TOPLEFT:
                aTopExt = aLeftExt = aShadowWidth;
                break;
            case SVX_SHADOW_TOPRIGHT:
                aTopExt = aRightExt = aShadowWidth;
                break;
            case SVX_SHADOW_BOTTOMLEFT:
                aBottomExt = aLeftExt = aShadowWidth;
                break;
            case SVX_SHADOW_BOTTOMRIGHT:
                aBottomExt = aRightExt = aShadowWidth;
                break;
            case SVX_SHADOW_NONE:
            case SVX_SHADOW_END:
                break;
        }
    }

    m_pSerializer->singleElementNS( XML_wp, XML_effectExtent,
            XML_l, aLeftExt, XML_t, aTopExt, XML_r, aRightExt, XML_b, aBottomExt,
            FSEND );

    if( isAnchor )
    {
        switch( pFrmFmt->GetSurround().GetValue())
        {
            case SURROUND_NONE:
                m_pSerializer->singleElementNS( XML_wp, XML_wrapTopAndBottom, FSEND );
                break;
            case SURROUND_THROUGHT:
                m_pSerializer->singleElementNS( XML_wp, XML_wrapNone, FSEND );
                break;
            case SURROUND_PARALLEL:
                m_pSerializer->singleElementNS( XML_wp, XML_wrapSquare,
                    XML_wrapText, "bothSides", FSEND );
                break;
            case SURROUND_IDEAL:
            default:
                m_pSerializer->singleElementNS( XML_wp, XML_wrapSquare,
                    XML_wrapText, "largest", FSEND );
                break;
        }
    }
    // picture description (used for pic:cNvPr later too)
    ::sax_fastparser::FastAttributeList* docPrattrList = m_pSerializer->createAttrList();
    docPrattrList->add( XML_id, OString::number( m_anchorId++).getStr());
    docPrattrList->add( XML_name, "Picture" );
    docPrattrList->add( XML_descr, OUStringToOString( pGrfNode ? pGrfNode->GetDescription() : pOLEFrmFmt->GetObjDescription(), RTL_TEXTENCODING_UTF8 ).getStr());
    if( GetExport().GetFilter().getVersion( ) != oox::core::ECMA_DIALECT )
        docPrattrList->add( XML_title, OUStringToOString( pGrfNode ? pGrfNode->GetTitle() : pOLEFrmFmt->GetObjTitle(), RTL_TEXTENCODING_UTF8 ).getStr());
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
    m_pSerializer->singleElementNS( XML_a, XML_blip,
            FSNS( XML_r, nImageType ), aRelId.getStr(),
            FSEND );

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
    m_pSerializer->singleElementNS( XML_a, XML_noFill,
            FSEND );
    m_pSerializer->startElementNS( XML_a, XML_ln,
            XML_w, "9525",
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_noFill,
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_miter,
            XML_lim, "800000",
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_headEnd,
            FSEND );
    m_pSerializer->singleElementNS( XML_a, XML_tailEnd,
            FSEND );
    m_pSerializer->endElementNS( XML_a, XML_ln );

    // Output effects
    if ( aShadowItem.GetLocation() != SVX_SHADOW_NONE )
    {
        // Distance is measured diagonally from corner
        double nShadowDist = sqrt((aShadowItem.GetWidth()*aShadowItem.GetWidth())*2.0);
        OString aShadowDist( OString::number( TwipsToEMU( nShadowDist ) ) );
        OString aShadowColor = msfilter::util::ConvertColor( aShadowItem.GetColor() );
        OString aShadowAlpha = lcl_ConvertTransparency(aShadowItem.GetColor());
        sal_uInt32 nShadowDir = 0;
        switch ( aShadowItem.GetLocation() )
        {
            case SVX_SHADOW_TOPLEFT: nShadowDir = 13500000; break;
            case SVX_SHADOW_TOPRIGHT: nShadowDir = 18900000; break;
            case SVX_SHADOW_BOTTOMLEFT: nShadowDir = 8100000; break;
            case SVX_SHADOW_BOTTOMRIGHT: nShadowDir = 2700000; break;
            case SVX_SHADOW_NONE:
            case SVX_SHADOW_END:
                break;
        }
        OString aShadowDir( OString::number( nShadowDir ) );

        m_pSerializer->startElementNS( XML_a, XML_effectLst, FSEND );
        m_pSerializer->startElementNS( XML_a, XML_outerShdw,
                                       XML_dist, aShadowDist.getStr(),
                                       XML_dir, aShadowDir.getStr(), FSEND );
        if (aShadowAlpha.isEmpty())
            m_pSerializer->singleElementNS( XML_a, XML_srgbClr,
                                            XML_val, aShadowColor.getStr(), FSEND );
        else
        {
            m_pSerializer->startElementNS(XML_a, XML_srgbClr, XML_val, aShadowColor.getStr(), FSEND);
            m_pSerializer->singleElementNS(XML_a, XML_alpha, XML_val, aShadowAlpha.getStr(), FSEND);
            m_pSerializer->endElementNS(XML_a, XML_srgbClr);
        }
        m_pSerializer->endElementNS( XML_a, XML_outerShdw );
        m_pSerializer->endElementNS( XML_a, XML_effectLst );
    }

    m_pSerializer->endElementNS( XML_pic, XML_spPr );

    m_pSerializer->endElementNS( XML_pic, XML_pic );

    m_pSerializer->endElementNS( XML_a, XML_graphicData );
    m_pSerializer->endElementNS( XML_a, XML_graphic );
    m_pSerializer->endElementNS( XML_wp, isAnchor ? XML_anchor : XML_inline );

    m_pSerializer->endElementNS( XML_w, XML_drawing );
}

void DocxAttributeOutput::WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rOLENode, const Size& rSize, const SwFlyFrmFmt* pFlyFrmFmt )
{
    if( WriteOLEChart( pSdrObj, rSize ))
        return;
    if( WriteOLEMath( pSdrObj, rOLENode, rSize ))
        return;
    // Then we fall back to just export the object as a graphic.
    FlyFrameGraphic( 0, rSize, pFlyFrmFmt, &rOLENode );
}

bool DocxAttributeOutput::WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize )
{
    uno::Reference< chart2::XChartDocument > xChartDoc;
    uno::Reference< drawing::XShape > xShape( ((SdrObject*)pSdrObj)->getUnoShape(), uno::UNO_QUERY );
    if( xShape.is() )
    {
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
        if( xPropSet.is() )
            xChartDoc.set( xPropSet->getPropertyValue( "Model" ), uno::UNO_QUERY );
    }

    if( xChartDoc.is() )
    {
        m_postponedChart = pSdrObj;
        m_postponedChartSize = rSize;
        return true;
    }
    return false;
}

/*
 * Write chart hierarchy in w:drawing after end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedChart()
{
       if(m_postponedChart == NULL)
                return;
       uno::Reference< chart2::XChartDocument > xChartDoc;
       uno::Reference< drawing::XShape > xShape( ((SdrObject*)m_postponedChart)->getUnoShape(), uno::UNO_QUERY );
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

        m_pSerializer->singleElementNS( XML_wp, XML_docPr,
            XML_id, I32S( ++m_docPrID ),
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
        static sal_Int32 nChartCount = 0;
        nChartCount++;
        uno::Reference< frame::XModel > xModel( xChartDoc, uno::UNO_QUERY );
        aRelId = m_rExport.OutputChart( xModel, nChartCount );

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
        m_postponedChart = NULL;
    return;
}

bool DocxAttributeOutput::WriteOLEMath( const SdrObject*, const SwOLENode& rOLENode, const Size& )
{
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode&>(rOLENode).GetOLEObj().GetOleRef());
    sal_Int64 nAspect = rOLENode.GetAspect();
    svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
    SvGlobalName aObjName(aObjRef->getClassID());

    if( !SotExchange::IsMath(aObjName) )
        return false;
    assert( m_postponedMath == NULL ); // make it a list if there can be more inside one run
    m_postponedMath = &rOLENode;
    return true;
}

void DocxAttributeOutput::WritePostponedMath()
{
    if( m_postponedMath == NULL )
        return;
    uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode*>(m_postponedMath)->GetOLEObj().GetOleRef());
    uno::Reference< uno::XInterface > xInterface( xObj->getComponent(), uno::UNO_QUERY );
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaExportBase* formulaexport = dynamic_cast<oox::FormulaExportBase*>(dynamic_cast<SfxBaseModel*>(xInterface.get()));
    assert( formulaexport != NULL );
    formulaexport->writeFormulaOoxml( m_pSerializer, GetExport().GetFilter().getVersion());
    m_postponedMath = NULL;
}

/*
 * Write w:pict hierarchy  end element of w:rPr tag.
 */
void DocxAttributeOutput::WritePostponedVMLDrawing()
{
    if(m_postponedVMLDrawing == NULL)
        return;

    for( std::list< PostponedVMLDrawing >::iterator it = m_postponedVMLDrawing->begin();
         it != m_postponedVMLDrawing->end();
         ++it )
    {
        WriteVMLDrawing(it->object, *(it->frame), *(it->vpt));
    }
    delete m_postponedVMLDrawing;
    m_postponedVMLDrawing = NULL;
}

void DocxAttributeOutput::WriteVMLDrawing( const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft )
{
   bool bSwapInPage = false;
   if ( !(sdrObj)->GetPage() )
   {
       if ( SdrModel* pModel = m_rExport.pDoc->GetDrawModel() )
       {
           if ( SdrPage *pPage = pModel->GetPage( 0 ) )
           {
               bSwapInPage = true;
               const_cast< SdrObject* >( sdrObj )->SetPage( pPage );
           }
       }
   }

   m_pSerializer->startElementNS( XML_w, XML_pict, FSEND );
   m_rDrawingML.SetFS(m_pSerializer);
   // See WinwordAnchoring::SetAnchoring(), these are not part of the SdrObject, have to be passed around manually.

   SwFmtHoriOrient rHoriOri = (rFrmFmt).GetHoriOrient();
   SwFmtVertOrient rVertOri = (rFrmFmt).GetVertOrient();
   m_rExport.VMLExporter().AddSdrObject( *(sdrObj),
        rHoriOri.GetHoriOrient(), rVertOri.GetVertOrient(),
        rHoriOri.GetRelationOrient(),
        rVertOri.GetRelationOrient(), (&rNdTopLeft) );
   m_pSerializer->endElementNS( XML_w, XML_pict );

   if ( bSwapInPage )
       const_cast< SdrObject* >( sdrObj )->SetPage( 0 );
}

void DocxAttributeOutput::OutputFlyFrame_Impl( const sw::Frame &rFrame, const Point& rNdTopLeft )
{
    m_pSerializer->mark();

    switch ( rFrame.GetWriterType() )
    {
        case sw::Frame::eGraphic:
            {
                const SdrObject* pSdrObj = rFrame.GetFrmFmt().FindRealSdrObject();
                const SwNode *pNode = rFrame.GetContent();
                const SwGrfNode *pGrfNode = pNode ? pNode->GetGrfNode() : 0;
                if ( pGrfNode )
                {
                    if( m_postponedGraphic == NULL )
                        FlyFrameGraphic( pGrfNode, rFrame.GetLayoutSize(), 0, 0, pSdrObj);
                    else // we are writing out attributes, but w:drawing should not be inside w:rPr,
                    {    // so write it out later
                        m_postponedGraphic->push_back( PostponedGraphic( pGrfNode, rFrame.GetLayoutSize(), pSdrObj));
                    }
                }
            }
            break;
        case sw::Frame::eDrawing:
            {
                const SdrObject* pSdrObj = rFrame.GetFrmFmt().FindRealSdrObject();
                if ( pSdrObj )
                {
                    if ( IsDiagram( pSdrObj ) )
                    {
                        if ( m_postponedDiagram == NULL )
                            WriteDiagram( pSdrObj, rFrame.GetLayoutSize() );
                        else // we are writing out attributes, but w:drawing should not be inside w:rPr,
                        {    // so write it out later
                            m_postponedDiagram->push_back( PostponedDiagram( pSdrObj, rFrame.GetSize() ) );
                        }
                    }
                    else
                    {
                        if ( m_postponedVMLDrawing == NULL )
                            WriteVMLDrawing( pSdrObj, rFrame.GetFrmFmt(), rNdTopLeft);
                        else // we are writing out attributes, but w:pict should not be inside w:rPr,
                        {    // so write it out later
                             m_postponedVMLDrawing->push_back( PostponedVMLDrawing( pSdrObj, &(rFrame.GetFrmFmt()), &rNdTopLeft ) );
                        }
                    }
                }
            }
            break;
        case sw::Frame::eTxtBox:
            {
                // The frame output is postponed to the end of the anchor paragraph
                m_aParentFrames.push_back(sw::Frame(rFrame));
            }
            break;
        case sw::Frame::eOle:
            {
                const SwFrmFmt &rFrmFmt = rFrame.GetFrmFmt();
                const SdrObject *pSdrObj = rFrmFmt.FindRealSdrObject();
                if ( pSdrObj )
                {
                    SwNodeIndex aIdx(*rFrmFmt.GetCntnt().GetCntntIdx(), 1);
                    SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
                    WriteOLE2Obj( pSdrObj, rOLENd, rFrame.GetLayoutSize(), dynamic_cast<const SwFlyFrmFmt*>( &rFrmFmt ));
                }
            }
            break;
        default:
            OSL_TRACE( "TODO DocxAttributeOutput::OutputFlyFrame_Impl( const sw::Frame& rFrame, const Point& rNdTopLeft ) - frame type '%s'\n",
                    rFrame.GetWriterType() == sw::Frame::eTxtBox? "eTxtBox":
                    ( rFrame.GetWriterType() == sw::Frame::eOle? "eOle":
                      ( rFrame.GetWriterType() == sw::Frame::eFormControl? "eFormControl": "???" ) ) );
            break;
    }

    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_POSTPONE );
}

bool DocxAttributeOutput::IsDiagram( const SdrObject* sdrObject )
{
    uno::Reference< drawing::XShape > xShape( ((SdrObject*)sdrObject)->getUnoShape(), uno::UNO_QUERY );
    if ( !xShape.is() )
        return false;

    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        return false;

    // if the shape doesn't have the InteropGrabBag property, it's not a diagram
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString pName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( pName ) )
        return false;

    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( pName ) >>= propList;
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

void DocxAttributeOutput::WriteDiagram( const SdrObject* sdrObject, const Size& size )
{
    uno::Reference< drawing::XShape > xShape( ((SdrObject*)sdrObject)->getUnoShape(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );

    uno::Reference<xml::dom::XDocument> dataDom;
    uno::Reference<xml::dom::XDocument> layoutDom;
    uno::Reference<xml::dom::XDocument> styleDom;
    uno::Reference<xml::dom::XDocument> colorDom;
    uno::Reference<xml::dom::XDocument> drawingDom;

    // retrieve the doms from the GrabBag
    OUString pName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( pName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXData" )
             propList[nProp].Value >>= dataDom;
        else if ( propName == "OOXLayout" )
             propList[nProp].Value >>= layoutDom;
        else if ( propName == "OOXStyle" )
             propList[nProp].Value >>= styleDom;
        else if ( propName == "OOXColor" )
             propList[nProp].Value >>= colorDom;
        else if ( propName == "OOXDrawing" )
             propList[nProp].Value >>= drawingDom;
    }

    // check that we have the 4 mandatory XDocuments
    // if not, there was an error importing and we won't output anything
    if ( !dataDom.is() || !layoutDom.is() || !styleDom.is() || !colorDom.is() )
        return;

    // write necessary tags to document.xml
    m_pSerializer->startElementNS( XML_w, XML_drawing,
        FSEND );
    m_pSerializer->startElementNS( XML_wp, XML_inline,
        XML_distT, "0", XML_distB, "0", XML_distL, "0", XML_distR, "0",
        FSEND );

    OString aWidth( OString::number( TwipsToEMU( size.Width() ) ) );
    OString aHeight( OString::number( TwipsToEMU( size.Height() ) ) );
    m_pSerializer->singleElementNS( XML_wp, XML_extent,
        XML_cx, aWidth.getStr(),
        XML_cy, aHeight.getStr(),
        FSEND );
    // TODO - the right effectExtent, extent including the effect
    m_pSerializer->singleElementNS( XML_wp, XML_effectExtent,
        XML_l, "0", XML_t, "0", XML_r, "0", XML_b, "0",
        FSEND );

    // generate an unique id
    static sal_Int32 diagramCount = 0;
    diagramCount++;
    OUString sName = "Diagram" + OUString::number( diagramCount );

    m_pSerializer->singleElementNS( XML_wp, XML_docPr,
        XML_id, I32S( diagramCount ),
        XML_name, USS( sName ),
        FSEND );

    m_pSerializer->singleElementNS( XML_wp, XML_cNvGraphicFramePr,
        FSEND );

    m_pSerializer->startElementNS( XML_a, XML_graphic,
        FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
        FSEND );

    m_pSerializer->startElementNS( XML_a, XML_graphicData,
        XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/diagram",
        FSEND );

    // add data relation
    OUString dataFileName = "diagrams/data" + OUString::number( diagramCount ) + ".xml";
    OString dataRelId = OUStringToOString(m_rExport.GetFilter().addRelation( m_pSerializer->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData",
        dataFileName, false ), RTL_TEXTENCODING_UTF8 );

    // add layout relation
    OUString layoutFileName = "diagrams/layout" + OUString::number( diagramCount ) + ".xml";
    OString layoutRelId = OUStringToOString(m_rExport.GetFilter().addRelation( m_pSerializer->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout",
        layoutFileName, false ), RTL_TEXTENCODING_UTF8 );

    // add style relation
    OUString styleFileName = "diagrams/quickStyle" + OUString::number( diagramCount ) + ".xml";
    OString styleRelId = OUStringToOString(m_rExport.GetFilter().addRelation( m_pSerializer->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle",
        styleFileName , false), RTL_TEXTENCODING_UTF8 );

    // add color relation
    OUString colorFileName = "diagrams/colors" + OUString::number( diagramCount ) + ".xml";
    OString colorRelId = OUStringToOString(m_rExport.GetFilter().addRelation( m_pSerializer->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors",
        colorFileName, false ), RTL_TEXTENCODING_UTF8 );

    OUString drawingFileName;
    if ( drawingDom.is() )
    {
        // add drawing relation
        drawingFileName = "diagrams/drawing" + OUString::number( diagramCount ) + ".xml";
        OUString drawingRelId = m_rExport.GetFilter().addRelation( m_pSerializer->getOutputStream(),
            "http://schemas.microsoft.com/office/2007/relationships/diagramDrawing",
            drawingFileName , false);

        // the data dom contains a reference to the drawing relation. We need to update it with the new generated
        // relation value before writing the dom to a file

        // Get the dsp:damaModelExt node from the dom
        uno::Reference< xml::dom::XNodeList > nodeList =
            dataDom->getElementsByTagNameNS( "http://schemas.microsoft.com/office/drawing/2008/diagram", "dataModelExt" );

        // There must be one element only so get it
        uno::Reference< xml::dom::XNode > node = nodeList->item( 0 );

        // Get the list of attributes of the node
        uno::Reference< xml::dom::XNamedNodeMap > nodeMap = node->getAttributes();

        // Get the node with the relId attribute and set its new value
        uno::Reference< xml::dom::XNode > relIdNode = nodeMap->getNamedItem( "relId" );
        relIdNode->setNodeValue( drawingRelId );
    }

    m_pSerializer->singleElementNS ( XML_dgm, XML_relIds,
                                     FSNS( XML_xmlns, XML_dgm ), "http://schemas.openxmlformats.org/drawingml/2006/diagram",
                                     FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
                                     FSNS( XML_r, XML_dm ), dataRelId.getStr(),
                                     FSNS( XML_r, XML_lo ), layoutRelId.getStr(),
                                     FSNS( XML_r, XML_qs ), styleRelId.getStr(),
                                     FSNS( XML_r, XML_cs ), colorRelId.getStr(),
                                     FSEND );

    m_pSerializer->endElementNS( XML_a, XML_graphicData );
    m_pSerializer->endElementNS( XML_a, XML_graphic );
    m_pSerializer->endElementNS( XML_wp, XML_inline );
    m_pSerializer->endElementNS( XML_w, XML_drawing );

    uno::Reference< xml::sax::XSAXSerializable > serializer;
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );

    // write data file
    serializer.set( dataDom, uno::UNO_QUERY );
    writer->setOutputStream( m_rExport.GetFilter().openFragmentStream( "word/" + dataFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml" ) );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    // write layout file
    serializer.set( layoutDom, uno::UNO_QUERY );
    writer->setOutputStream( m_rExport.GetFilter().openFragmentStream( "word/" + layoutFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml" ) );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    // write style file
    serializer.set( styleDom, uno::UNO_QUERY );
    writer->setOutputStream( m_rExport.GetFilter().openFragmentStream( "word/" + styleFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml" ) );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    // write color file
    serializer.set( colorDom, uno::UNO_QUERY );
    writer->setOutputStream( m_rExport.GetFilter().openFragmentStream( "word/" + colorFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml" ) );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    // write drawing file
    if ( drawingDom.is() )
    {
        serializer.set( drawingDom, uno::UNO_QUERY );
        writer->setOutputStream( m_rExport.GetFilter().openFragmentStream( "word/" + drawingFileName,
            "application/vnd.openxmlformats-officedocument.drawingml.diagramDrawing+xml" ) );
        serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
            uno::Sequence< beans::StringPair >() );
    }
}

void DocxAttributeOutput::WriteOutliner(const OutlinerParaObject& rParaObj)
{
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( m_rExport, rEditObj, TXT_HFTXTBOX );

    sal_Int32 nPara = rEditObj.GetParagraphCount();

    m_pSerializer->startElementNS( XML_v, XML_textbox, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_txbxContent, FSEND );
    for (sal_Int32 n = 0; n < nPara; ++n)
    {
        if( n )
            aAttrIter.NextPara( n );

        OUString aStr( rEditObj.GetText( n ));
        xub_StrLen nAktPos = 0;
        sal_Int32 nEnd = aStr.getLength();

        m_pSerializer->startElementNS( XML_w, XML_p, FSEND );

        // Write paragraph properties.
        m_pSerializer->startElementNS(XML_w, XML_pPr, FSEND);
        aAttrIter.OutParaAttr(false);
        m_pSerializer->endElementNS(XML_w, XML_pPr);

        do {
            xub_StrLen nNextAttr = aAttrIter.WhereNext();
            if( nNextAttr > nEnd )
                nNextAttr = nEnd;

            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

            // Write run properties.
            m_pSerializer->startElementNS(XML_w, XML_rPr, FSEND);
            aAttrIter.OutAttr(nAktPos);
            m_pSerializer->endElementNS(XML_w, XML_rPr);

            bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
            if( !bTxtAtr )
            {
                OUString aOut( aStr.copy( nAktPos, nNextAttr - nAktPos ) );
                RunText(aOut);
            }

            m_pSerializer->endElementNS( XML_w, XML_r );

            nAktPos = nNextAttr;
            aAttrIter.NextPos();
        }
        while( nAktPos < nEnd );
        m_pSerializer->endElementNS( XML_w, XML_p );
    }
    m_pSerializer->endElementNS( XML_w, XML_txbxContent );
    m_pSerializer->endElementNS( XML_v, XML_textbox );
}

oox::drawingml::DrawingML& DocxAttributeOutput::GetDrawingML()
{
    return m_rDrawingML;
}

void DocxAttributeOutput::StartStyle( const OUString& rName, StyleType eType,
        sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 /*nWwId*/, sal_uInt16 nId, bool bAutoUpdate )
{
    bool bQFormat = false, bUnhideWhenUsed = false, bSemiHidden = false, bLocked = false, bDefault = false, bCustomStyle = false;
    OUString aLink, aRsid, aUiPriority;
    FastAttributeList* pStyleAttributeList = m_pSerializer->createAttrList();
    uno::Any aAny;
    if (eType == STYLE_TYPE_PARA || eType == STYLE_TYPE_CHAR)
    {
        const SwFmt* pFmt = m_rExport.pStyles->GetSwFmt(nId);
        pFmt->GetGrabBagItem(aAny);
    }
    else
    {
        const SwNumRule* pRule = m_rExport.pStyles->GetSwNumRule(nId);
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

    const char* pType = 0;
    switch (eType)
    {
        case STYLE_TYPE_PARA: pType = "paragraph"; break;
        case STYLE_TYPE_CHAR: pType = "character"; break;
        case STYLE_TYPE_LIST: pType = "numbering"; break;
    }
    pStyleAttributeList->add(FSNS( XML_w, XML_type ), pType);
    pStyleAttributeList->add(FSNS( XML_w, XML_styleId ), m_rExport.pStyles->GetStyleId(nId).getStr());
    if (bDefault)
        pStyleAttributeList->add(FSNS(XML_w, XML_default), "1");
    if (bCustomStyle)
        pStyleAttributeList->add(FSNS(XML_w, XML_customStyle), "1");
    XFastAttributeListRef xStyleAttributeList(pStyleAttributeList);
    m_pSerializer->startElementNS( XML_w, XML_style, xStyleAttributeList);

    m_pSerializer->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), OUStringToOString( OUString( rName ), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    // Output properties from grab-bag.
    if (!aUiPriority.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_uiPriority,
                FSNS(XML_w, XML_val), OUStringToOString(aUiPriority, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (bQFormat)
        m_pSerializer->singleElementNS(XML_w, XML_qFormat, FSEND);
    if (bSemiHidden)
        m_pSerializer->singleElementNS(XML_w, XML_semiHidden, FSEND);
    if (bUnhideWhenUsed)
        m_pSerializer->singleElementNS(XML_w, XML_unhideWhenUsed, FSEND);
    if (!aLink.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_link,
                FSNS(XML_w, XML_val), OUStringToOString(aLink, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);
    if (bLocked)
        m_pSerializer->singleElementNS(XML_w, XML_locked, FSEND);
    if (!aRsid.isEmpty())
        m_pSerializer->singleElementNS(XML_w, XML_rsid,
                FSNS(XML_w, XML_val), OUStringToOString(aRsid, RTL_TEXTENCODING_UTF8).getStr(),
                FSEND);

    if ( nBase != 0x0FFF && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_basedOn,
                FSNS( XML_w, XML_val ), m_rExport.pStyles->GetStyleId(nBase).getStr(),
                FSEND );
    }

    if ( nNext != nId && eType != STYLE_TYPE_LIST)
    {
        m_pSerializer->singleElementNS( XML_w, XML_next,
                FSNS( XML_w, XML_val ), m_rExport.pStyles->GetStyleId(nNext).getStr(),
                FSEND );
    }

    if ( bAutoUpdate )
        m_pSerializer->singleElementNS( XML_w, XML_autoRedefine, FSEND );
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
        m_pSerializer->endElementNS( XML_w, XML_pPr );
    }
    else
    {
        WriteCollectedRunProperties();
        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }
}

void lcl_OutlineLevel(sax_fastparser::FSHelperPtr pSerializer, sal_uInt16 nLevel)
{
    if (nLevel >= WW8ListManager::nMaxLevel)
        nLevel = WW8ListManager::nMaxLevel - 1;

    pSerializer->singleElementNS(XML_w, XML_outlineLvl,
            FSNS(XML_w, XML_val), OString::number(nLevel).getStr(),
            FSEND);
}

void DocxAttributeOutput::OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt& /*rNFmt*/, const SwFmt& /*rFmt*/ )
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
                if ( !m_bParagraphOpened )
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
            else
            {
                m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
                m_pSerializer->singleElementNS( XML_w, XML_br,
                        FSNS( XML_w, XML_type ), "page", FSEND );
                m_pSerializer->endElementNS( XML_w, XML_r );
            }
            break;
        default:
            OSL_TRACE( "Unknown section break to write: %d", nC );
            break;
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

    m_pSerializer->mark( aSeqOrder );
}

void DocxAttributeOutput::EndSection()
{
    // Write the section properties
    if ( m_pSectionSpacingAttrList )
    {
        XFastAttributeListRef xAttrList( m_pSectionSpacingAttrList );
        m_pSectionSpacingAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_pgMar, xAttrList );
    }

    // Order the elements
    m_pSerializer->mergeTopMarks( );

    m_pSerializer->endElementNS( XML_w, XML_sectPr );
    m_bOpenedSectPr = false;
}

void DocxAttributeOutput::SectionFormProtection( bool bProtected )
{
    if ( bProtected )
        m_pSerializer->singleElementNS( XML_w, XML_formProt, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_formProt,
                FSNS( XML_w, XML_val ), "false", FSEND );
}

void DocxAttributeOutput::SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo )
{
    FastAttributeList* pAttr = m_pSerializer->createAttrList();
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

void DocxAttributeOutput::SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* /*pFirstPageFmt*/ )
{
    // Output the margins

    const SvxBoxItem& rBox = pFmt->GetBox( );

    const SvxBorderLine* pLeft = rBox.GetLeft( );
    const SvxBorderLine* pTop = rBox.GetTop( );
    const SvxBorderLine* pRight = rBox.GetRight( );
    const SvxBorderLine* pBottom = rBox.GetBottom( );

    if ( pBottom || pTop || pLeft || pRight )
    {
        bool bExportDistanceFromPageEdge = false;
        if ( boxHasLineLargerThan31(rBox) == true )
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
            const SvxShadowItem* pShadowItem = (const SvxShadowItem*)pItem;
            aOutputBorderOptions.aShadowLocation = pShadowItem->GetLocation();
        }

        impl_borders( m_pSerializer, rBox, aOutputBorderOptions, &m_pageMargins );

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

        default:                            aType = "none";        break;
    }

    return aType;
}

void DocxAttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, ::boost::optional<sal_uInt16> oPageRestartNumber )
{
    // FIXME Not called properly with page styles like "First Page"

    FastAttributeList* pAttr = m_pSerializer->createAttrList();

    // boost::none means no restart: then don't output that attribute if it is negative
    if ( oPageRestartNumber )
       pAttr->add( FSNS( XML_w, XML_start ), OString::number( oPageRestartNumber.get() ) );

    // nNumType corresponds to w:fmt. See WW8Export::GetNumId() for more precisions
    OString aFmt( impl_NumberingType( nNumType ) );
    if ( !aFmt.isEmpty() )
        pAttr->add( FSNS( XML_w, XML_fmt ), aFmt.getStr() );

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
    const char* pType = NULL;
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
    FastAttributeList* pAttr = m_pSerializer->createAttrList();

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
    const char *pFamily = NULL;
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
    const char *pPitch = NULL;
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
    if( !m_rExport.pDoc->get( IDocumentSettingAccess::EMBED_FONTS ))
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
        uno::Reference< com::sun::star::io::XOutputStream > xOutStream = m_rExport.GetFilter().openFragmentStream(
            OUString( "word/fonts/font" ) + OUString::number(m_nextFontId) + ".odttf",
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
            OUString( "fonts/font" ) + OUString::number( m_nextFontId ) + ".odttf" ), RTL_TEXTENCODING_UTF8 );
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

OString DocxAttributeOutput::TransHighlightColor( const Color& rColor )
{
    switch (rColor.GetColor())
    {
        case 0x000000: return OString("black"); break;
        case 0x0000ff: return OString("blue"); break;
        case 0x00ffff: return OString("cyan"); break;
        case 0x00ff00: return OString("green"); break;
        case 0xff00ff: return OString("magenta"); break;
        case 0xff0000: return OString("red"); break;
        case 0xffff00: return OString("yellow"); break;
        case 0xffffff: return OString("white"); break;
        case 0x000080: return OString("darkBlue"); break;
        case 0x008080: return OString("darkCyan"); break;
        case 0x008000: return OString("darkGreen"); break;
        case 0x800080: return OString("darkMagenta"); break;
        case 0x800000: return OString("darkRed"); break;
        case 0x808000: return OString("darkYellow"); break;
        case 0x808080: return OString("darkGray"); break;
        case 0xC0C0C0: return OString("lightGray"); break;
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

    // start with the nStart value
    m_pSerializer->singleElementNS( XML_w, XML_start,
            FSNS( XML_w, XML_val ), OString::number( nStart ).getStr(),
            FSEND );

    // format
    OString aFmt( impl_NumberingType( nNumberingType ) );

    if ( !aFmt.isEmpty() )
        m_pSerializer->singleElementNS( XML_w, XML_numFmt,
                FSNS( XML_w, XML_val ), aFmt.getStr(),
                FSEND );

    // suffix
    const char *pSuffix = NULL;
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
    if ( aText.equals ( OUString(sal_Unicode(0)) ) )
    {
        m_pSerializer->singleElementNS( XML_w, XML_lvlText, FSNS( XML_w, XML_val ), "", FSEND );
    }
    else
    {
        m_pSerializer->singleElementNS( XML_w, XML_lvlText,FSNS( XML_w, XML_val ), OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr(), FSEND );
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
        m_rExport.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rExport.mbExportModeRTF );

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

    m_pSerializer->singleElementNS( XML_w, XML_color,
            FSNS( XML_w, XML_val ), aColorString.getStr(), FSEND );
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
    if (&rItem != NULL && (sIss.isEmpty() || sIss.match(OString("baseline"))))
    {
        long nHeight = rItem.GetHeight();
        OString sPos = OString::number( ( nHeight * nEsc + 500 ) / 1000 );
        m_pSerializer->singleElementNS( XML_w, XML_position,
                FSNS( XML_w, XML_val ), sPos.getStr( ), FSEND );

        if( 100 != nProp || sIss.match( OString( "baseline" ) ) )
        {
            OString sSize = OString::number( ( nHeight * nProp + 500 ) / 1000 );
                m_pSerializer->singleElementNS( XML_w, XML_sz,
                    FSNS( XML_w, XML_val ), sSize.getStr( ), FSEND );
        }
    }
}

void DocxAttributeOutput::CharFont( const SvxFontItem& rFont)
{
    if (!m_pFontsAttrList)
        m_pFontsAttrList = m_pSerializer->createAttrList();
    GetExport().GetId( rFont ); // ensure font info is written to fontTable.xml
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_ascii), sFontNameUtf8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_hAnsi), sFontNameUtf8);
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
    if (!m_pCharLangAttrList)
        m_pCharLangAttrList = m_pSerializer->createAttrList();

    OString aLanguageCode( OUStringToOString(
                LanguageTag( rLanguage.GetLanguage()).getBcp47(),
                RTL_TEXTENCODING_UTF8));

    switch ( rLanguage.Which() )
    {
        case RES_CHRATR_LANGUAGE:
            m_pCharLangAttrList->add(FSNS(XML_w, XML_val), aLanguageCode);
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            m_pCharLangAttrList->add(FSNS(XML_w, XML_eastAsia), aLanguageCode);
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            m_pCharLangAttrList->add(FSNS(XML_w, XML_bidi), aLanguageCode);
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
    if (rBrush.GetShadingValue() == +ShadingPattern::PCT15)
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
    if (!m_pFontsAttrList)
        m_pFontsAttrList = m_pSerializer->createAttrList();
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_eastAsia), sFontNameUtf8);
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
    if (!m_pFontsAttrList)
        m_pFontsAttrList = m_pSerializer->createAttrList();
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_cs), sFontNameUtf8);

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

void DocxAttributeOutput::CharRotate( const SvxCharRotateItem& rRotate)
{
    // Not rorated or we the rotation already handled?
    if ( !rRotate.GetValue() || m_bBtLr || m_bFrameBtLr)
        return;

    if (!m_pEastAsianLayoutAttrList)
        m_pEastAsianLayoutAttrList = m_pSerializer->createAttrList();
    OString sTrue((sal_Char *)"true");
    m_pEastAsianLayoutAttrList->add(FSNS(XML_w, XML_vert), sTrue);

    if (rRotate.IsFitToLine())
        m_pEastAsianLayoutAttrList->add(FSNS(XML_w, XML_vertCompress), sTrue);
}

void DocxAttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
{
    const char *pEmphasis;

    switch ( rEmphasisMark.GetValue() )
    {
        case EMPHASISMARK_NONE:         pEmphasis = "none";     break;
        case EMPHASISMARK_SIDE_DOTS:    pEmphasis = "dot";      break;
        case EMPHASISMARK_CIRCLE_ABOVE: pEmphasis = "circle";   break;
        case EMPHASISMARK_DOTS_BELOW:   pEmphasis = "underDot"; break;
        default:                        pEmphasis = "comma";    break;
    }

    m_pSerializer->singleElementNS( XML_w, XML_em, FSNS( XML_w, XML_val ), pEmphasis, FSEND );
}

void DocxAttributeOutput::CharTwoLines( const SvxTwoLinesItem& rTwoLines )
{
    if ( !rTwoLines.GetValue() )
        return;

    if (!m_pEastAsianLayoutAttrList)
        m_pEastAsianLayoutAttrList = m_pSerializer->createAttrList();
    OString sTrue((sal_Char *)"true");
    m_pEastAsianLayoutAttrList->add(FSNS(XML_w, XML_combine), sTrue);

    sal_Unicode cStart = rTwoLines.GetStartBracket();
    sal_Unicode cEnd = rTwoLines.GetEndBracket();

    if (!cStart && !cEnd)
        return;

    OString sBracket;
    if ((cStart == '{') || (cEnd == '}'))
        sBracket = (sal_Char *)"curly";
    else if ((cStart == '<') || (cEnd == '>'))
        sBracket = (sal_Char *)"angle";
    else if ((cStart == '[') || (cEnd == ']'))
        sBracket = (sal_Char *)"square";
    else
        sBracket = (sal_Char *)"round";
    m_pEastAsianLayoutAttrList->add(FSNS(XML_w, XML_combineBrackets), sBracket);
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
    const OString sColor = TransHighlightColor( rHighlight.GetColor() );
    if ( !sColor.isEmpty() )
    {
        m_pSerializer->singleElementNS( XML_w, XML_highlight,
            FSNS( XML_w, XML_val ), sColor.getStr(), FSEND );
    }
}

void DocxAttributeOutput::TextINetFormat( const SwFmtINetFmt& rLink )
{
    const SwTxtINetFmt* pINetFmt = rLink.GetTxtINetFmt();
    const SwCharFmt* pCharFmt = pINetFmt->GetCharFmt();

    OString aStyleId(m_rExport.pStyles->GetStyleId(m_rExport.GetId(*pCharFmt)));

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

void DocxAttributeOutput::TextCharFormat( const SwFmtCharFmt& rCharFmt )
{
    OString aStyleId(m_rExport.pStyles->GetStyleId(m_rExport.GetId(*rCharFmt.GetCharFmt())));

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

void DocxAttributeOutput::RefField( const SwField&  rFld, const OUString& rRef )
{
    sal_uInt16 nType = rFld.GetTyp( )->Which( );
    if ( nType == RES_GETEXPFLD )
    {
        OUString sCmd = FieldString( ww::eREF );
        sCmd += "\"" + rRef + "\" ";

        m_rExport.OutputField( &rFld, ww::eREF, sCmd );
    }

    // There is nothing to do here for the set fields
}

void DocxAttributeOutput::HiddenField( const SwField& /*rFld*/ )
{
    OSL_TRACE( "TODO DocxAttributeOutput::HiddenField()" );
}

void DocxAttributeOutput::PostitField( const SwField* pFld )
{
    assert( dynamic_cast< const SwPostItField* >( pFld ));
    m_postitFields.push_back( static_cast< const SwPostItField* >( pFld ));
}

void DocxAttributeOutput::WritePostitFieldReference()
{
    while( m_postitFieldsMaxId < m_postitFields.size())
    {
        OString idstr = OString::number( m_postitFieldsMaxId);
        m_pSerializer->singleElementNS( XML_w, XML_commentReference, FSNS( XML_w, XML_id ), idstr.getStr(), FSEND );
        ++m_postitFieldsMaxId;
    }
}

void DocxAttributeOutput::WritePostitFieldStart()
{
    m_bPostitStart = true;
}

void DocxAttributeOutput::WritePostitFieldEnd()
{
    m_bPostitEnd = true;
}

void DocxAttributeOutput::WritePostitFields()
{
    for( unsigned int i = 0;
         i < m_postitFields.size();
         ++i )
    {
        OString idstr = OString::number( i);
        const SwPostItField* f = m_postitFields[ i ];
        m_pSerializer->startElementNS( XML_w, XML_comment, FSNS( XML_w, XML_id ), idstr.getStr(),
            FSNS( XML_w, XML_author ), OUStringToOString( f->GetPar1(), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSNS( XML_w, XML_date ), msfilter::util::DateTimeToOString(f->GetDateTime()).getStr(),
            FSNS( XML_w, XML_initials ), OUStringToOString( f->GetInitials(), RTL_TEXTENCODING_UTF8 ).getStr(), FSEND );
        // Check for the text object existing, it seems that it can be NULL when saving a newly created
        // comment without giving focus back to the main document. As GetTxt() is empty in that case as well,
        // that is probably a bug in the Writer core.
        if( f->GetTextObject() != NULL )
            GetExport().WriteOutliner( *f->GetTextObject(), TXT_ATN );
        m_pSerializer->endElementNS( XML_w, XML_comment );
    }
}

bool DocxAttributeOutput::DropdownField( const SwField* pFld )
{
    bool bExpand = false;

    ww::eField eType = ww::eFORMDROPDOWN;
    OUString sCmd = FieldString( eType  );
    GetExport( ).OutputField( pFld, eType, sCmd );

    return bExpand;
}

bool DocxAttributeOutput::PlaceholderField( const SwField* pFld )
{
    assert( pendingPlaceholder == NULL );
    pendingPlaceholder = pFld;
    return false; // do not expand
}

void DocxAttributeOutput::WritePendingPlaceholder()
{
    if( pendingPlaceholder == NULL )
        return;
    const SwField* pFld = pendingPlaceholder;
    pendingPlaceholder = NULL;
    m_pSerializer->startElementNS( XML_w, XML_sdt, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_sdtPr, FSEND );
    if( !pFld->GetPar2().isEmpty())
        m_pSerializer->singleElementNS( XML_w, XML_alias,
            FSNS( XML_w, XML_val ), OUStringToOString( pFld->GetPar2(), RTL_TEXTENCODING_UTF8 ), FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_temporary, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_showingPlcHdr, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_text, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_sdtPr );
    m_pSerializer->startElementNS( XML_w, XML_sdtContent, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    RunText( pFld->GetPar1());
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_sdtContent );
    m_pSerializer->endElementNS( XML_w, XML_sdt );
}

void DocxAttributeOutput::SetField( const SwField& rFld, ww::eField eType, const OUString& rCmd )
{
    // field bookmarks are handled in the EndRun method
    GetExport().OutputField(&rFld, eType, rCmd );
}

void DocxAttributeOutput::WriteExpand( const SwField* pFld )
{
    // Will be written in the next End Run
    OUString sCmd;
    m_rExport.OutputField( pFld, ww::eUNKNOWN, sCmd );
}

void DocxAttributeOutput::WriteField_Impl( const SwField* pFld, ww::eField eType, const OUString& rFldCmd, sal_uInt8 nMode )
{
    struct FieldInfos infos;
    if (pFld)
        infos.pField = pFld->CopyField();
    infos.sCmd = rFldCmd;
    infos.eType = eType;
    infos.bClose = WRITEFIELD_CLOSE & nMode;
    infos.bOpen = WRITEFIELD_START & nMode;
    m_Fields.push_back( infos );

    if ( pFld )
    {
        sal_uInt16 nType = pFld->GetTyp( )->Which( );
        sal_uInt16 nSubType = pFld->GetSubType();

        // TODO Any other field types here ?
        if ( ( nType == RES_SETEXPFLD ) && ( nSubType & nsSwGetSetExpType::GSE_STRING ) )
        {
            const SwSetExpField *pSet = ( const SwSetExpField* )( pFld );
            m_sFieldBkm = pSet->GetPar1( );
        }
        else if ( nType == RES_DROPDOWN )
        {
            const SwDropDownField* pDropDown = ( const SwDropDownField* )( pFld );
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
        m_rMarksStart.push_back( rName );
    }
    rStarts.clear();

    for ( std::vector< OUString >::const_iterator it = rEnds.begin(), end = rEnds.end(); it != end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rMarksEnd.push_back( rName );
    }
    rEnds.clear();
}

void DocxAttributeOutput::TextFootnote_Impl( const SwFmtFtn& rFootnote )
{
    const SwEndNoteInfo& rInfo = rFootnote.IsEndNote()?
        m_rExport.pDoc->GetEndNoteInfo(): m_rExport.pDoc->GetFtnInfo();

    // footnote/endnote run properties
    const SwCharFmt* pCharFmt = rInfo.GetAnchorCharFmt( *m_rExport.pDoc );

    OString aStyleId(m_rExport.pStyles->GetStyleId(m_rExport.GetId(*pCharFmt)));

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
    const SwFmtFtn *pFootnote = m_pFootnotesList->getCurrent( nId );

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

    m_pSerializer->startElementNS( XML_w, nBody,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSEND );

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
        const SwPageFtnInfo& rFtnInfo = m_rExport.pDoc->GetPageDesc(0).GetFtnInfo();
        // Request a separator only in case the width is larger than zero.
        bSeparator = double(rFtnInfo.GetWidth()) > 0;
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

        const SwNodeIndex* pIndex = (*i)->GetTxtFtn()->GetStartNode();
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
    const char* fmt = NULL;
    switch( info.aFmt.GetNumberingType())
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
    if( fmt != NULL )
        fs->singleElementNS( XML_w, XML_numFmt, FSNS( XML_w, XML_val ), fmt, FSEND );
    if( info.nFtnOffset != 0 )
        fs->singleElementNS( XML_w, XML_numStart, FSNS( XML_w, XML_val ),
            OString::number( info.nFtnOffset + 1).getStr(), FSEND );
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
        WriteFootnoteEndnotePr( m_pSerializer, XML_footnotePr, m_rExport.pDoc->GetFtnInfo(), 0 );
    if( HasEndnotes())
        WriteFootnoteEndnotePr( m_pSerializer, XML_endnotePr, m_rExport.pDoc->GetEndNoteInfo(), 0 );
}

void DocxAttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    if ( !m_pParagraphSpacingAttrList )
        m_pParagraphSpacingAttrList = m_pSerializer->createAttrList();

    if ( nSpace < 0 )
    {
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "exact" );
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_line ), OString::number( -nSpace ) );
    }
    else if( nMulti )
    {
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "auto" );
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_line ), OString::number( nSpace ) );
    }
    else if ( nSpace > 0 )
    {
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "atLeast" );
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_line ), OString::number( nSpace ) );
    }
    else
        m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "auto" );
}

void DocxAttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    const char *pAdjustString;

    bool bEcma = GetExport().GetFilter().getVersion( ) == oox::core::ECMA_DIALECT;

    const SfxItemSet* pItems = GetExport().GetCurItemSet();
    const SvxFrameDirectionItem* rFrameDir = pItems?
        static_cast< const SvxFrameDirectionItem* >( pItems->GetItem( RES_FRAMEDIR ) ): NULL;

    short nDir = FRMDIR_ENVIRONMENT;
    if( rFrameDir != NULL )
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

void DocxAttributeOutput::ParaSplit( const SvxFmtSplitItem& rSplit )
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
    FastAttributeList *pTabElementAttrList = pSerializer->createAttrList();

    switch (rTab.GetAdjustment())
    {
    case SVX_TAB_ADJUST_RIGHT:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( (sal_Char *)"right") );
        break;
    case SVX_TAB_ADJUST_DECIMAL:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( (sal_Char *)"decimal") );
        break;
    case SVX_TAB_ADJUST_CENTER:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( (sal_Char *)"center") );
        break;
    case SVX_TAB_ADJUST_DEFAULT:
    case SVX_TAB_ADJUST_LEFT:
    default:
        pTabElementAttrList->add( FSNS( XML_w, XML_val ), OString( (sal_Char *)"left") );
        break;
    }

    // Because GetTabPos already includes indent, we don't need to add nCurrentLeft (CurrentLeft is indentation information)
    //pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::valueOf( rTab.GetTabPos() + nCurrentLeft ) );
    pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::number( rTab.GetTabPos()                ) );

    sal_Unicode cFillChar = rTab.GetFill();

    if ('.' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "dot" ) );
    else if ( '-' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "hyphen" ) );
    else if ( sal_Unicode(0xB7) == cFillChar ) // middle dot
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "middleDot" ) );
    else if ( '_' == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "underscore" ) );
    else
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "none" ) );

    pSerializer->singleElementNS( XML_w, XML_tab, pTabElementAttrList );
}

void DocxAttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStop )
{
    const SfxPoolItem* pLR = m_rExport.HasItem( RES_LR_SPACE );
    long nCurrentLeft = pLR ? ((const SvxLRSpaceItem*)pLR)->GetTxtLeft() : 0;

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
            FSNS( XML_w, XML_val ), rHyphenZone.IsHyphen( ) ? "false" : "true" ,
            FSEND );
}

void DocxAttributeOutput::ParaNumRule_Impl( const SwTxtNode* /*pTxtNd*/, sal_Int32 nLvl, sal_Int32 nNumId )
{
    if ( USHRT_MAX != nNumId && 0 != nNumId )
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
           FSNS( XML_w, XML_val ), rScriptSpace.GetValue( ) ? "true": "false",
           FSEND );
}

void DocxAttributeOutput::ParaHangingPunctuation( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_overflowPunct,
           FSNS( XML_w, XML_val ), rItem.GetValue( ) ? "true": "false",
           FSEND );
}

void DocxAttributeOutput::ParaForbiddenRules( const SfxBoolItem& rItem )
{
    m_pSerializer->singleElementNS( XML_w, XML_kinsoku,
           FSNS( XML_w, XML_val ), rItem.GetValue( ) ? "true": "false",
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
            FSNS( XML_w, XML_val ), rGrid.GetValue( ) ? "true": "false",
            FSEND );
}

void DocxAttributeOutput::FormatFrameSize( const SwFmtFrmSize& rSize )
{
    if (m_bTextFrameSyntax && m_pFlyFrameSize)
    {
        m_aTextFrameStyle.append(";width:").append(double(m_pFlyFrameSize->Width()) / 20);
        m_aTextFrameStyle.append("pt;height:").append(double(m_pFlyFrameSize->Height()) / 20).append("pt");
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList( );

        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == ATT_FIX_SIZE )
        {
            m_pFlyAttrList->add( FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ) );
        }

        if ( rSize.GetHeight() )
        {
            OString sRule( "exact" );
            if ( rSize.GetHeightSizeType() == ATT_MIN_SIZE )
                sRule = OString( "atLeast" );
            m_pFlyAttrList->add( FSNS( XML_w, XML_hRule ), sRule );
            m_pFlyAttrList->add( FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ) );
        }
    }
    else if ( m_rExport.bOutPageDescs )
    {
        FastAttributeList *attrList = m_pSerializer->createAttrList( );
        if ( m_rExport.pAktPageDesc->GetLandscape( ) )
            attrList->add( FSNS( XML_w, XML_orient ), "landscape" );

        attrList->add( FSNS( XML_w, XML_w ), OString::number( rSize.GetWidth( ) ) );
        attrList->add( FSNS( XML_w, XML_h ), OString::number( rSize.GetHeight( ) ) );

        XFastAttributeListRef xAttrList( attrList );
        attrList = NULL;

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

    if (m_bTextFrameSyntax)
    {
        m_aTextFrameStyle.append(";mso-wrap-distance-left:").append(double(rLRSpace.GetLeft()) / 20).append("pt");
        m_aTextFrameStyle.append(";mso-wrap-distance-right:").append(double(rLRSpace.GetRight()) / 20).append("pt");
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList();

        m_pFlyAttrList->add( FSNS( XML_w, XML_hSpace ),
                OString::number(
                    ( rLRSpace.GetLeft() + rLRSpace.GetRight() ) / 2 ) );
    }
    else if ( m_rExport.bOutPageDescs )
    {
        if ( !m_pSectionSpacingAttrList )
            m_pSectionSpacingAttrList = m_pSerializer->createAttrList();

        m_pageMargins.nPageMarginLeft = 0;
        m_pageMargins.nPageMarginRight = 0;

        const SfxPoolItem* pItem = m_rExport.HasItem( RES_BOX );
        if ( pItem )
        {
            m_pageMargins.nPageMarginRight = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_LEFT );
            m_pageMargins.nPageMarginLeft = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_RIGHT );
        }
        else
            m_pageMargins.nPageMarginLeft = m_pageMargins.nPageMarginRight = 0;

        m_pageMargins.nPageMarginLeft = m_pageMargins.nPageMarginLeft + (sal_uInt16)rLRSpace.GetLeft();
        m_pageMargins.nPageMarginRight = m_pageMargins.nPageMarginRight + (sal_uInt16)rLRSpace.GetRight();

        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_left ), OString::number( m_pageMargins.nPageMarginLeft ) );
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_right ), OString::number( m_pageMargins.nPageMarginRight ) );
    }
    else
    {
        FastAttributeList *pLRSpaceAttrList = m_pSerializer->createAttrList();

        pLRSpaceAttrList->add( FSNS( XML_w, ( bEcma ? XML_left : XML_start ) ), OString::number(  rLRSpace.GetTxtLeft() ) );
        pLRSpaceAttrList->add( FSNS( XML_w, ( bEcma ? XML_right : XML_end ) ), OString::number(  rLRSpace.GetRight() ) );

        sal_Int32 nFirstLineAdjustment = rLRSpace.GetTxtFirstLineOfst();
        if (nFirstLineAdjustment > 0)
            pLRSpaceAttrList->add( FSNS( XML_w, XML_firstLine ), OString::number( nFirstLineAdjustment ) );
        else
            pLRSpaceAttrList->add( FSNS( XML_w, XML_hanging ), OString::number( - nFirstLineAdjustment ) );
        m_pSerializer->singleElementNS( XML_w, XML_ind, pLRSpaceAttrList );
    }
}

void DocxAttributeOutput::FormatULSpace( const SvxULSpaceItem& rULSpace )
{

    if (m_bTextFrameSyntax)
    {
        m_aTextFrameStyle.append(";mso-wrap-distance-top:").append(double(rULSpace.GetUpper()) / 20).append("pt");
        m_aTextFrameStyle.append(";mso-wrap-distance-bottom:").append(double(rULSpace.GetLower()) / 20).append("pt");
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList();

        m_pFlyAttrList->add( FSNS( XML_w, XML_vSpace ),
                OString::number(
                    ( rULSpace.GetLower() + rULSpace.GetUpper() ) / 2 ) );
    }
    else if (m_rExport.bOutPageDescs )
    {
        OSL_ENSURE( m_rExport.GetCurItemSet(), "Impossible" );
        if ( !m_rExport.GetCurItemSet() )
            return;

        if ( !m_pSectionSpacingAttrList )
            m_pSectionSpacingAttrList = m_pSerializer->createAttrList();

        HdFtDistanceGlue aDistances( *m_rExport.GetCurItemSet() );

        sal_Int32 nHeader = 0;
        if ( aDistances.HasHeader() )
            nHeader = sal_Int32( aDistances.dyaHdrTop );
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_header ), OString::number( nHeader ) );

        // Page top
        m_pageMargins.nPageMarginTop = aDistances.dyaTop;
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_top ),
                OString::number( m_pageMargins.nPageMarginTop ) );

        sal_Int32 nFooter = 0;
        if ( aDistances.HasFooter() )
            nFooter = sal_Int32( aDistances.dyaHdrBottom );
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_footer ), OString::number( nFooter ) );

        // Page Bottom
        m_pageMargins.nPageMarginBottom = aDistances.dyaBottom;
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_bottom ),
                OString::number( m_pageMargins.nPageMarginBottom ) );

        // FIXME Page Gutter is not handled ATM, setting to 0 as it's mandatory for OOXML
        m_pSectionSpacingAttrList->add( FSNS( XML_w, XML_gutter ),
                OString::number( 0 ) );
    }
    else
    {
        if ( !m_pParagraphSpacingAttrList )
            m_pParagraphSpacingAttrList = m_pSerializer->createAttrList();
        SAL_INFO("sw.ww8", "DocxAttributeOutput::FormatULSpace: setting spacing" << rULSpace.GetUpper() );
        // check if before auto spacing was set during import and spacing we get from actual object is same
        // that we set in import. If yes just write beforeAutoSpacing tag.
        if (m_bParaBeforeAutoSpacing && m_iParaBeforeSpacing == rULSpace.GetUpper())
        {
            m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_beforeAutospacing ),
                    "1" );
        }
        else
        {
            m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_before ),
                    OString::number( rULSpace.GetUpper() ) );
        }
        m_bParaBeforeAutoSpacing = false;
        // check if after auto spacing was set during import and spacing we get from actual object is same
        // that we set in import. If yes just write afterAutoSpacing tag.
        if (m_bParaAfterAutoSpacing && m_iParaAfterSpacing == rULSpace.GetLower())
        {
            m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_afterAutospacing ),
                    "1" );
        }
        else
        {
            m_pParagraphSpacingAttrList->add( FSNS( XML_w, XML_after ),
                    OString::number( rULSpace.GetLower()) );
        }
        m_bParaAfterAutoSpacing = false;

        if (rULSpace.GetContext())
            m_pSerializer->singleElementNS( XML_w, XML_contextualSpacing, FSEND );
    }
}

void DocxAttributeOutput::FormatSurround( const SwFmtSurround& rSurround )
{
    if (m_bTextFrameSyntax)
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
            m_pFlyWrapAttrList = m_pSerializer->createAttrList();
            if (!sType.isEmpty())
                m_pFlyWrapAttrList->add(XML_type, sType);
            if (!sSide.isEmpty())
                m_pFlyWrapAttrList->add(XML_side, sSide);
        }
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList();

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

        m_pFlyAttrList->add( FSNS( XML_w, XML_wrap ), sWrap );
    }
}

void DocxAttributeOutput::FormatVertOrientation( const SwFmtVertOrient& rFlyVert )
{
    if (m_bTextFrameSyntax)
    {
        m_aTextFrameStyle.append(";margin-top:").append(double(rFlyVert.GetPos()) / 20).append("pt");
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList();

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
            case text::VertOrientation::LINE_BOTTOM:
                sAlign = OString( "bottom" );
                break;
            case text::VertOrientation::TOP:
            case text::VertOrientation::LINE_TOP:
            default:
                sAlign = OString( "top" );
                break;
        }

        if ( !sAlign.isEmpty() )
            m_pFlyAttrList->add( FSNS( XML_w, XML_yAlign ), sAlign );
        else
            m_pFlyAttrList->add( FSNS( XML_w, XML_y ),
                OString::number( rFlyVert.GetPos() ) );

        OString sVAnchor( "page" );
        switch ( rFlyVert.GetRelationOrient( ) )
        {
            case text::RelOrientation::CHAR:
            case text::RelOrientation::PRINT_AREA:
            case text::RelOrientation::TEXT_LINE:
                sVAnchor = OString( "column" );
                break;
            case text::RelOrientation::FRAME:
            case text::RelOrientation::PAGE_LEFT:
            case text::RelOrientation::PAGE_RIGHT:
            case text::RelOrientation::FRAME_LEFT:
            case text::RelOrientation::FRAME_RIGHT:
                sVAnchor = OString( "margin" );
                break;
            case text::RelOrientation::PAGE_FRAME:
            case text::RelOrientation::PAGE_PRINT_AREA:
            default:
                break;
        }

        m_pFlyAttrList->add( FSNS( XML_w, XML_vAnchor ), sVAnchor );
    }
}

void DocxAttributeOutput::FormatHorizOrientation( const SwFmtHoriOrient& rFlyHori )
{
    if (m_bTextFrameSyntax)
    {
        m_aTextFrameStyle.append(";margin-left:").append(double(rFlyHori.GetPos()) / 20).append("pt");
    }
    else if ( m_rExport.bOutFlyFrmAttrs )
    {
        if ( !m_pFlyAttrList )
            m_pFlyAttrList = m_pSerializer->createAttrList();

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

        if ( !sAlign.isEmpty() )
            m_pFlyAttrList->add( FSNS( XML_w, XML_xAlign ), sAlign );
        else
            m_pFlyAttrList->add( FSNS( XML_w, XML_x ),
                OString::number( rFlyHori.GetPos() ) );

        OString sHAnchor( "page" );
        switch ( rFlyHori.GetRelationOrient( ) )
        {
            case text::RelOrientation::CHAR:
            case text::RelOrientation::PRINT_AREA:
                sHAnchor = OString( "text" );
                break;
            case text::RelOrientation::FRAME:
            case text::RelOrientation::PAGE_LEFT:
            case text::RelOrientation::PAGE_RIGHT:
            case text::RelOrientation::FRAME_LEFT:
            case text::RelOrientation::FRAME_RIGHT:
                sHAnchor = OString( "margin" );
                break;
            case text::RelOrientation::PAGE_FRAME:
            case text::RelOrientation::PAGE_PRINT_AREA:
            default:
                break;
        }

        m_pFlyAttrList->add( FSNS( XML_w, XML_hAnchor ), sHAnchor );
    }
}

void DocxAttributeOutput::FormatAnchor( const SwFmtAnchor& )
{
    // Fly frames: anchors here aren't matching the anchors in docx
}

void DocxAttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    OString sColor = msfilter::util::ConvertColor( rBrush.GetColor( ) );
    if (m_bTextFrameSyntax)
    {
        // Handle 'Opacity'
        sal_Int32 nTransparency = rBrush.GetColor().GetTransparency();
        if (nTransparency)
        {
            // Convert transparency to percent
            sal_Int8 nTransparencyPercent = SvxBrushItem::TransparencyToPercent(nTransparency);

            // Calculate alpha value
            // Consider oox/source/drawingml/color.cxx : getTransparency() function.
            sal_Int32 nAlpha = (::oox::drawingml::MAX_PERCENT - ( ::oox::drawingml::PER_PERCENT * nTransparencyPercent ) );

            // Calculate opacity value
            // Consider oox/source/vml/vmlformatting.cxx : decodeColor() function.
            double fOpacity = (double)nAlpha * 65535 / ::oox::drawingml::MAX_PERCENT;
            OUString sOpacity = OUString::number(fOpacity);

            if ( !m_pFlyFillAttrList )
                m_pFlyFillAttrList = m_pSerializer->createAttrList();

            m_pFlyFillAttrList->add(XML_opacity, OUStringToOString(sOpacity, RTL_TEXTENCODING_UTF8) + "f");
        }

        m_pFlyAttrList->add(XML_fillcolor, "#" + sColor);
    }
    else if ( !m_rExport.bOutPageDescs )
    {
        m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor.getStr( ),
                FSNS( XML_w, XML_val ), "clear",
                FSEND );
    }
}

void DocxAttributeOutput::FormatFillStyle( const XFillStyleItem& rFillStyle )
{
    m_oFillStyle.reset(rFillStyle.GetValue());
}

void DocxAttributeOutput::FormatFillGradient( const XFillGradientItem& rFillGradient )
{
    if (*m_oFillStyle == XFILL_GRADIENT)
    {
        if ( !m_pFlyFillAttrList )
            m_pFlyFillAttrList = m_pSerializer->createAttrList();

        m_pFlyFillAttrList->add(XML_type, "gradient");

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
            m_pFlyFillAttrList->add(XML_angle, OString::number(nReverseAngle));

        OString sColor1 = sStartColor;
        OString sColor2 = sEndColor;

        switch (rGradient.GetGradientStyle())
        {
            case XGRAD_AXIAL:
                m_pFlyFillAttrList->add(XML_focus, "50%");
                // If it is an 'axial' gradient - swap the colors
                // (because in the import process they were imported swapped)
                sColor1 = sEndColor;
                sColor2 = sStartColor;
                break;
            case XGRAD_LINEAR: break;
            case XGRAD_RADIAL: break;
            case XGRAD_ELLIPTICAL: break;
            case XGRAD_SQUARE: break;
            case XGRAD_RECT: break;
        }

        m_pFlyAttrList->add(XML_fillcolor , "#" + sColor1);
        m_pFlyFillAttrList->add(XML_color2, "#" + sColor2);
    }
    m_oFillStyle.reset();
}

void DocxAttributeOutput::FormatBox( const SvxBoxItem& rBox )
{
    if (m_bTextFrameSyntax)
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
                m_pFlyAttrList->add(XML_stroked, "f");
                m_pFlyAttrList->add(XML_strokeweight, "0pt");
            }
            else
            {
                OString sColor("#" + msfilter::util::ConvertColor(pTop->GetColor()));
                m_pFlyAttrList->add(XML_strokecolor, sColor);

                double const fConverted(editeng::ConvertBorderWidthToWord(pTop->GetBorderLineStyle(), pTop->GetWidth()));
                sal_Int32 nWidth = sal_Int32(fConverted / 20);
                m_pFlyAttrList->add(XML_strokeweight, OString::number(nWidth) + "pt");
            }
        }

        // v:textbox's inset attribute: inner margin values for textbox text - write only non-default values
        double fDistanceLeftTwips = double(rBox.GetDistance(BOX_LINE_LEFT));
        double fDistanceTopTwips = double(rBox.GetDistance(BOX_LINE_TOP));
        double fDistanceRightTwips = double(rBox.GetDistance(BOX_LINE_RIGHT));
        double fDistanceBottomTwips = double(rBox.GetDistance(BOX_LINE_BOTTOM));

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
            m_pTextboxAttrList->add(XML_inset, aInset.makeStringAndClear());

        return;
    }


    OutputBorderOptions aOutputBorderOptions = lcl_getBoxBorderOptions();
    // Check if there is a shadow item
    const SfxPoolItem* pItem = GetExport().HasItem( RES_SHADOW );
    if ( pItem )
    {
        const SvxShadowItem* pShadowItem = (const SvxShadowItem*)pItem;
        aOutputBorderOptions.aShadowLocation = pShadowItem->GetLocation();
    }

    if ( !m_bOpenedSectPr || GetWritingHeaderFooter())
    {
        // Not inside a section

        // Open the paragraph's borders tag
        m_pSerializer->startElementNS( XML_w, XML_pBdr, FSEND );

        impl_borders( m_pSerializer, rBox, aOutputBorderOptions, &m_pageMargins );

        // Close the paragraph's borders tag
        m_pSerializer->endElementNS( XML_w, XML_pBdr );
    }
}

void DocxAttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol& rCol, bool bEven, SwTwips nPageSize )
{
    // Get the columns attributes
    FastAttributeList *pColsAttrList = m_pSerializer->createAttrList();

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

    pColsAttrList->add( FSNS( XML_w, XML_sep ), bHasSep ? "true" : "false" );

    // Write the element
    m_pSerializer->startElementNS( XML_w, XML_cols, pColsAttrList );

    // Write the columns width if non-equals
    const SwColumns & rColumns = rCol.GetColumns(  );
    if ( !bEven )
    {
        for ( sal_uInt16 n = 0; n < nCols; ++n )
        {
            FastAttributeList *pColAttrList = m_pSerializer->createAttrList();
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

void DocxAttributeOutput::FormatKeep( const SvxFmtKeepItem& )
{
    m_pSerializer->singleElementNS( XML_w, XML_keepNext, FSEND );
}

void DocxAttributeOutput::FormatTextGrid( const SwTextGridItem& rGrid )
{
    FastAttributeList *pGridAttrList = m_pSerializer->createAttrList();

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

void DocxAttributeOutput::FormatLineNumbering( const SwFmtLineNumber& rNumbering )
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

    if ( m_rExport.bOutPageDescs )
    {
        m_pSerializer->singleElementNS( XML_w, XML_textDirection,
               FSNS( XML_w, XML_val ), sTextFlow.getStr( ),
               FSEND );
        if ( bBiDi )
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSEND );
    }
    else if ( !m_rExport.bOutFlyFrmAttrs )
    {
        if ( bBiDi )
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSNS( XML_w, XML_val ), "1", FSEND );
        else
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSNS( XML_w, XML_val ), "0", FSEND );
    }
}

void DocxAttributeOutput::ParaGrabBag(const SfxGrabBagItem& rItem)
{
    const std::map<OUString, com::sun::star::uno::Any>& rMap = rItem.GetGrabBag();
    for (std::map<OUString, com::sun::star::uno::Any>::const_iterator i = rMap.begin(); i != rMap.end(); ++i)
    {
        if (i->first == "MirrorIndents")
            m_pSerializer->singleElementNS(XML_w, XML_mirrorIndents, FSEND);
        else if (i->first == "ParaTopMarginBeforeAutoSpacing")
        {
            m_bParaBeforeAutoSpacing = true;
            // get fixed value which was set during import
            i->second >>= m_iParaBeforeSpacing;
            m_iParaBeforeSpacing = MM100_TO_TWIP(m_iParaBeforeSpacing);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << i->first << " : m_iParaBeforeSpacing= " << m_iParaBeforeSpacing);
        }
        else if (i->first == "ParaBottomMarginAfterAutoSpacing")
        {
            m_bParaAfterAutoSpacing = true;
            // get fixed value which was set during import
            i->second >>= m_iParaAfterSpacing;
            m_iParaAfterSpacing = MM100_TO_TWIP(m_iParaAfterSpacing);
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: property =" << i->first << " : m_iParaBeforeSpacing= " << m_iParaAfterSpacing);
        }
        else
            SAL_INFO("sw.ww8", "DocxAttributeOutput::ParaGrabBag: unhandled grab bag property " << i->first );
    }
}

DocxAttributeOutput::DocxAttributeOutput( DocxExport &rExport, FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML )
    : m_rExport( rExport ),
      m_pSerializer( pSerializer ),
      m_rDrawingML( *pDrawingML ),
      m_pFontsAttrList( NULL ),
      m_pEastAsianLayoutAttrList( NULL ),
      m_pCharLangAttrList( NULL ),
      m_pSectionSpacingAttrList( NULL ),
      m_pParagraphSpacingAttrList( NULL ),
      m_pHyperlinkAttrList( NULL ),
      m_pFlyAttrList( NULL ),
      m_pFlyFillAttrList( NULL ),
      m_pFlyWrapAttrList( NULL ),
      m_pTextboxAttrList( NULL ),
      m_pFlyFrameSize(0),
      m_pFootnotesList( new ::docx::FootnotesList() ),
      m_pEndnotesList( new ::docx::FootnotesList() ),
      m_footnoteEndnoteRefTag( 0 ),
      m_docPrID(0),
      m_pSectionInfo( NULL ),
      m_pRedlineData( NULL ),
      m_nRedlineId( 0 ),
      m_bOpenedSectPr( false ),
      m_bWritingHeaderFooter( false ),
      m_sFieldBkm( ),
      m_nNextMarkId( 0 ),
      m_bPostitStart(false),
      m_bPostitEnd(false),
      m_pTableWrt( NULL ),
      m_bTableCellOpen( false ),
      m_nTableDepth( 0 ),
      m_bParagraphOpened( false ),
      m_nColBreakStatus( COLBRK_NONE ),
      m_bTextFrameSyntax( false ),
      m_closeHyperlinkInThisRun( false ),
      m_closeHyperlinkInPreviousRun( false ),
      m_startedHyperlink( false ),
      m_postponedGraphic( NULL ),
      m_postponedDiagram( NULL ),
      m_postponedVMLDrawing(NULL),
      m_postponedMath( NULL ),
      m_postponedChart( NULL ),
      pendingPlaceholder( NULL ),
      m_postitFieldsMaxId( 0 ),
      m_anchorId( 0 ),
      m_nextFontId( 1 ),
      m_bBtLr(false),
      m_bFrameBtLr(false),
      m_pTableStyleExport(new DocxTableStyleExport(rExport.pDoc, pSerializer)),
      m_bParaBeforeAutoSpacing(false),
      m_bParaAfterAutoSpacing(false),
      m_iParaBeforeSpacing(0),
      m_iParaAfterSpacing(0),
      m_setFootnote(false)
{
}

DocxAttributeOutput::~DocxAttributeOutput()
{
    delete m_pFontsAttrList, m_pFontsAttrList = NULL;
    delete m_pEastAsianLayoutAttrList, m_pEastAsianLayoutAttrList = NULL;
    delete m_pCharLangAttrList, m_pCharLangAttrList = NULL;
    delete m_pSectionSpacingAttrList, m_pSectionSpacingAttrList = NULL;
    delete m_pParagraphSpacingAttrList, m_pParagraphSpacingAttrList = NULL;
    delete m_pHyperlinkAttrList, m_pHyperlinkAttrList = NULL;
    delete m_pFlyAttrList, m_pFlyAttrList = NULL;
    delete m_pTextboxAttrList, m_pTextboxAttrList = NULL;

    delete m_pFootnotesList, m_pFootnotesList = NULL;
    delete m_pEndnotesList, m_pEndnotesList = NULL;

    delete m_pTableWrt, m_pTableWrt = NULL;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
