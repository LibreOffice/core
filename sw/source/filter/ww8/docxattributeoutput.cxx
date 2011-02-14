/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "docxattributeoutput.hxx"
#include "docxexport.hxx"
#include "docxfootnotes.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "ww8par.hxx"

#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/utils.hxx>
#include <oox/export/vmlexport.hxx>

#include <i18npool/mslangid.hxx>

#include <hintids.hxx>

#include <svl/poolitem.hxx>

#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/blnkitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>

#include <docufld.hxx>
#include <flddropdown.hxx>
#include <format.hxx>
#include <fmtclds.hxx>
#include <fmtinfmt.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmtftn.hxx>
#include <fmtrowsplt.hxx>
#include <fmtline.hxx>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <ftninfo.hxx>
#include <htmltbl.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <swmodule.hxx>
#include <swtable.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <numrule.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#include <tools/color.hxx>

#include <com/sun/star/i18n/ScriptType.hdl>

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

using namespace oox;
using namespace docx;
using namespace sax_fastparser;
using namespace nsSwDocInfoSubType;
using namespace nsFieldFlags;
using namespace sw::util;

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
    m_pSectionInfo = NULL;

    m_bParagraphOpened = true;
}

void DocxAttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    // write the paragraph properties + the run, already in the correct order
    m_pSerializer->mergeTopMarks();
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
        sal_uInt16 nLinesCount = rLines.Count( );

        if ( pInner->isEndOfCell() )
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

void DocxAttributeOutput::StartParagraphProperties( const SwTxtNode& rNode )
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
        m_rExport.OutputSectionBreaks( pTxtNode->GetpSwAttrSet(), *pTxtNode );
    }
    else if ( aNextIndex.GetNode().IsTableNode() )
    {
        const SwTableNode* pTableNode = static_cast< SwTableNode* >( &aNextIndex.GetNode() );
        const SwFrmFmt *pFmt = pTableNode->GetTable().GetFrmFmt();
        m_rExport.OutputSectionBreaks( &(pFmt->GetAttrSet()), *pTableNode );
    }

    // postpone the output so that we can later [in EndParagraphProperties()]
    // prepend the properties before the run
    m_pSerializer->mark();

    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );

    // and output the section break now (if it appeared)
    if ( m_pSectionInfo )
    {
        m_rExport.SectionProperties( *m_pSectionInfo );
        m_pSectionInfo = NULL;
    }

    InitCollectedParagraphProperties();
}

void DocxAttributeOutput::InitCollectedParagraphProperties()
{
    m_pSpacingAttrList = NULL;
}

void DocxAttributeOutput::WriteCollectedParagraphProperties()
{
    if ( m_pSpacingAttrList )
    {
        XFastAttributeListRef xAttrList( m_pSpacingAttrList );
        m_pSpacingAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_spacing, xAttrList );
    }
}

void DocxAttributeOutput::EndParagraphProperties()
{
    WriteCollectedParagraphProperties();

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

void DocxAttributeOutput::StartRun( const SwRedlineData* pRedlineData )
{
    // if there is some redlining in the document, output it
    StartRedline( pRedlineData );

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
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin(); pIt != m_Fields.end(); ++pIt )
    {
        // Add the fields starts for all but hyperlinks and TOCs
        if ( pIt->bOpen && pIt->pField )
        {
            StartField_Impl( *pIt );

            // Remove the field from the stack if only the start has to be written
            // Unknown fields sould be removed too
            if ( !pIt->bClose || ( pIt->eType == ww::eUNKNOWN ) )
            {
                m_Fields.erase( pIt );
                --pIt;
            }
        }
    }


    // write the run properties + the text, already in the correct order
    m_pSerializer->mergeTopMarks(); // merges with "postponed text", see above

    // level down, to be able to prepend the actual run start attribute (just
    // before "postponed run start")
    m_pSerializer->mark(); // let's call it "actual run start"

    // prepend the actual run start
    if ( m_pHyperlinkAttrList )
    {
        XFastAttributeListRef xAttrList ( m_pHyperlinkAttrList );

        m_pSerializer->startElementNS( XML_w, XML_hyperlink, xAttrList );
    }

    // Write the hyperlink and toc fields starts
    for ( std::vector<FieldInfos>::iterator pIt = m_Fields.begin(); pIt != m_Fields.end(); ++pIt )
    {
        // Add the fields starts for hyperlinks, TOCs and index marks
        if ( pIt->bOpen )
        {
            StartField_Impl( *pIt, sal_True );

            // Remove the field if no end needs to be written
            if ( !pIt->bClose ) {
                m_Fields.erase( pIt );
                --pIt;
            }
        }
    }

    DoWriteBookmarks( );

    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND ); // merges with "postponed run start", see above

    // write the run start + the run content
    m_pSerializer->mergeTopMarks(); // merges the "actual run start"

    // append the actual run end
    m_pSerializer->endElementNS( XML_w, XML_r );

    if ( m_pHyperlinkAttrList )
    {
        m_pSerializer->endElementNS( XML_w, XML_hyperlink );
        m_pHyperlinkAttrList = NULL;
    }

    while ( m_Fields.begin() != m_Fields.end() )
    {
        EndField_Impl( m_Fields.front( ) );
        m_Fields.erase( m_Fields.begin( ) );
    }

    // if there is some redlining in the document, output it
    EndRedline();
}

void DocxAttributeOutput::DoWriteBookmarks()
{
    // Write the start bookmarks
    for ( std::vector< OString >::const_iterator it = m_rMarksStart.begin(), end = m_rMarksStart.end();
          it < end; ++it )
    {
        const OString& rName = *it;

        // Output the bookmark
        sal_uInt16 nId = m_nNextMarkId++;
        m_rOpenedMarksIds[rName] = nId;
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkStart,
            FSNS( XML_w, XML_id ), OString::valueOf( sal_Int32( nId ) ).getStr(  ),
            FSNS( XML_w, XML_name ), rName.getStr(),
            FSEND );
    }
    m_rMarksStart.clear();

    // export the end bookmarks
    for ( std::vector< OString >::const_iterator it = m_rMarksEnd.begin(), end = m_rMarksEnd.end();
          it < end; ++it )
    {
        const OString& rName = *it;

        // Get the id of the bookmark
        std::map< OString, sal_uInt16 >::iterator pPos = m_rOpenedMarksIds.find( rName );
        if ( pPos != m_rOpenedMarksIds.end(  ) )
        {
            sal_uInt16 nId = ( *pPos ).second;
            m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
                FSNS( XML_w, XML_id ), OString::valueOf( sal_Int32( nId ) ).getStr(  ),
                FSEND );
            m_rOpenedMarksIds.erase( rName );
        }
    }
    m_rMarksEnd.clear();
}

void DocxAttributeOutput::StartField_Impl( FieldInfos& rInfos, sal_Bool bWriteRun )
{
    if ( rInfos.pField && rInfos.eType == ww::eUNKNOWN )
    {
        // Expand unsupported fields
        RunText( rInfos.pField->Expand( ) );
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

                const SwDropDownField& rFld2 = *(SwDropDownField*)rInfos.pField;
                uno::Sequence<rtl::OUString> aItems =
                    rFld2.GetItemSequence();
                GetExport().DoComboBox(rFld2.GetName(),
                           rFld2.GetHelp(),
                           rFld2.GetToolTip(),
                           rFld2.GetSelectedItem(), aItems);

                m_pSerializer->endElementNS( XML_w, XML_fldChar );

                if ( bWriteRun )
                    m_pSerializer->endElementNS( XML_w, XML_r );

        }
        else
        {
            // Write the field start
            m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin",
                FSEND );

            if ( bWriteRun )
                m_pSerializer->endElementNS( XML_w, XML_r );

            // The hyperlinks fields can't be expanded: the value is
            // normally in the text run
            if ( !rInfos.pField )
                CmdField_Impl( rInfos );
        }
    }
}

void DocxAttributeOutput::DoWriteCmd( String& rCmd )
{
    // Write the Field command
    m_pSerializer->startElementNS( XML_w, XML_instrText, FSEND );
    m_pSerializer->writeEscaped( OUString( rCmd ) );
    m_pSerializer->endElementNS( XML_w, XML_instrText );

}

void DocxAttributeOutput::CmdField_Impl( FieldInfos& rInfos )
{
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    xub_StrLen nNbToken = rInfos.sCmd.GetTokenCount( '\t' );

    for ( xub_StrLen i = 0; i < nNbToken; i++ )
    {
        String sToken = rInfos.sCmd.GetToken( i, '\t' );
        // Write the Field command
        DoWriteCmd( sToken );

        // Replace tabs by </instrText><tab/><instrText>
        if ( i < ( nNbToken - 1 ) )
            RunText( String::CreateFromAscii( "\t" ) );
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
    if ( aBkmName.getLength( ) > 0 )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkStart,
               FSNS( XML_w, XML_id ), OString::valueOf( sal_Int32( m_nNextMarkId ) ).getStr( ),
               FSNS( XML_w, XML_name ), OUStringToOString( aBkmName, RTL_TEXTENCODING_UTF8 ).getStr( ),
               FSEND );
    }

    if (rInfos.pField ) // For hyperlinks and TOX
    {
        // Write the Field latest value
        m_pSerializer->startElementNS( XML_w, XML_r, FSEND );

        // Find another way for hyperlinks
        RunText( rInfos.pField->Expand( ) );
        m_pSerializer->endElementNS( XML_w, XML_r );
    }

    // Write the bookmark end if any
    if ( aBkmName.getLength( ) > 0 )
    {
        m_pSerializer->singleElementNS( XML_w, XML_bookmarkEnd,
               FSNS( XML_w, XML_id ), OString::valueOf( sal_Int32( m_nNextMarkId ) ).getStr( ),
               FSEND );

        m_nNextMarkId++;
    }

    // Write the Field end
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_fldChar,
          FSNS( XML_w, XML_fldCharType ), "end",
          FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );


    // Write the ref field if a bookmark had to be set and the field
    // should be visible
    if ( rInfos.pField )
    {
        sal_uInt16 nSubType = rInfos.pField->GetSubType( );
        bool bIsSetField = rInfos.pField->GetTyp( )->Which( ) == RES_SETEXPFLD;
        bool bShowRef = ( !bIsSetField || ( nSubType & nsSwExtendedSubType::SUB_INVISIBLE ) ) ? false : true;

        if ( ( m_sFieldBkm.Len( ) > 0 ) && bShowRef )
        {
            // Write the field beginning
            m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_fldChar,
                FSNS( XML_w, XML_fldCharType ), "begin",
                FSEND );
            m_pSerializer->endElementNS( XML_w, XML_r );

            rInfos.sCmd = FieldString( ww::eREF );
            rInfos.sCmd.APPEND_CONST_ASC( "\"" );
            rInfos.sCmd += m_sFieldBkm;
            rInfos.sCmd.APPEND_CONST_ASC( "\" " );

            // Clean the field bookmark data to avoid infinite loop
            m_sFieldBkm = String( );

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
}

void DocxAttributeOutput::InitCollectedRunProperties()
{
    m_pFontsAttrList = NULL;
    m_pEastAsianLayoutAttrList = NULL;
    m_pCharLangAttrList = NULL;
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
}

void DocxAttributeOutput::EndRunProperties( const SwRedlineData* /*pRedlineData*/ )
{
    WriteCollectedRunProperties();

    m_pSerializer->endElementNS( XML_w, XML_rPr );

    // write footnotes/endnotes if we have any
    FootnoteEndnoteReference();

    // merge the properties _before_ the run text (strictly speaking, just
    // after the start of the run)
    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND );
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
    if ( *pBegin == sal_Unicode( ' ' ) || *( pEnd - 1 ) == sal_Unicode( ' ' ) )
    {
        pSerializer->startElementNS( XML_w, nTextToken, FSNS( XML_xml, XML_space ), "preserve", FSEND );
    }
    else
        pSerializer->startElementNS( XML_w, nTextToken, FSEND );

    pSerializer->writeEscaped( OUString( pBegin, pEnd - pBegin ) );

    pSerializer->endElementNS( XML_w, nTextToken );
}

void DocxAttributeOutput::RunText( const String& rText, rtl_TextEncoding /*eCharSet*/ )
{
    OUString aText( rText );

    // one text can be split into more <w:t>blah</w:t>'s by line breaks etc.
    const sal_Unicode *pBegin = aText.getStr();
    const sal_Unicode *pEnd = pBegin + aText.getLength();

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

void DocxAttributeOutput::RawText( const String& /*rText*/, bool /*bForceUnicode*/, rtl_TextEncoding /*eCharSet*/ )
{
    OSL_TRACE("TODO DocxAttributeOutput::RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet )\n" );
}

void DocxAttributeOutput::StartRuby( const SwTxtNode& /*rNode*/, const SwFmtRuby& /*rRuby*/ )
{
    OSL_TRACE("TODO DocxAttributeOutput::StartRuby( const SwTxtNode& rNode, const SwFmtRuby& rRuby )\n" );
}

void DocxAttributeOutput::EndRuby()
{
    OSL_TRACE( "TODO DocxAttributeOutput::EndRuby()\n" );
}

bool DocxAttributeOutput::AnalyzeURL( const String& rUrl, const String& rTarget, String* pLinkURL, String* pMark )
{
    bool bBookMarkOnly = AttributeOutputBase::AnalyzeURL( rUrl, rTarget, pLinkURL, pMark );

    String sURL = *pLinkURL;
    String sMark = *pMark;

    bool bOutputField = sMark.Len();

    if ( bOutputField )
    {
        if ( bBookMarkOnly )
            sURL = FieldString( ww::eHYPERLINK );
        else
        {
            String sFld( FieldString( ww::eHYPERLINK ) );
            sFld.APPEND_CONST_ASC( "\"" );
            sURL.Insert( sFld, 0 );
            sURL += '\"';
        }

        if ( sMark.Len() )
            ( ( sURL.APPEND_CONST_ASC( " \\l \"" ) ) += sMark ) += '\"';

        if ( rTarget.Len() )
            ( sURL.APPEND_CONST_ASC( " \\n " ) ) += rTarget;
    }

    *pLinkURL = sURL;
    *pMark = sMark;

    return bBookMarkOnly;
}

bool DocxAttributeOutput::StartURL( const String& rUrl, const String& rTarget )
{
    String sMark;
    String sUrl;

    bool bBookmarkOnly = AnalyzeURL( rUrl, rTarget, &sUrl, &sMark );

    if ( sMark.Len() && !bBookmarkOnly )
    {
        m_rExport.OutputField( NULL, ww::eHYPERLINK, sUrl );
    }
    else
    {
        // Output a hyperlink XML element

        m_pHyperlinkAttrList = m_pSerializer->createAttrList();
        if ( !bBookmarkOnly )
        {
            OUString osUrl( sUrl );

            ::rtl::OString sId = m_rExport.AddRelation(
                S( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" ),
                osUrl, S("External") );
            m_pHyperlinkAttrList->add( FSNS( XML_r, XML_id), sId.getStr());
        }
        else
            m_pHyperlinkAttrList->add( FSNS( XML_w, XML_anchor ),
                    OUStringToOString( OUString( sMark ), RTL_TEXTENCODING_UTF8 ).getStr( ) );

        OUString sTarget( rTarget );
        if ( sTarget.getLength( ) > 0 )
        {
            OString soTarget = OUStringToOString( sTarget, RTL_TEXTENCODING_UTF8 );
            m_pHyperlinkAttrList->add(FSNS( XML_w, XML_tgtFrame ), soTarget.getStr());
        }
    }

    return true;
}

bool DocxAttributeOutput::EndURL()
{
    return true;
}

void DocxAttributeOutput::FieldVanish( const String& rTxt, ww::eField eType )
{
    WriteField_Impl( NULL, eType, rTxt, WRITEFIELD_ALL );
}

void DocxAttributeOutput::Redline( const SwRedlineData* /*pRedline*/ )
{
    OSL_TRACE( "TODO DocxAttributeOutput::Redline( const SwRedlineData* pRedline )\n" );
}

/// Append the number as 2-digit when less than 10.
static void impl_AppendTwoDigits( OStringBuffer &rBuffer, sal_Int32 nNum )
{
    if ( nNum < 0 || nNum > 99 )
    {
        rBuffer.append( "00" );
        return;
    }

    if ( nNum < 10 )
        rBuffer.append( '0' );

    rBuffer.append( nNum );
}

/** Convert DateTime to xsd::dateTime string.

I guess there must be an implementation of this somewhere in OOo, but I failed
to find it, unfortunately :-(
*/
static OString impl_DateTimeToOString( const DateTime& rDateTime )
{
    DateTime aInUTC( rDateTime );
    aInUTC.ConvertToUTC();

    OStringBuffer aBuffer( 25 );
    aBuffer.append( sal_Int32( aInUTC.GetYear() ) );
    aBuffer.append( '-' );

    impl_AppendTwoDigits( aBuffer, aInUTC.GetMonth() );
    aBuffer.append( '-' );

    impl_AppendTwoDigits( aBuffer, aInUTC.GetDay() );
    aBuffer.append( 'T' );

    impl_AppendTwoDigits( aBuffer, aInUTC.GetHour() );
    aBuffer.append( ':' );

    impl_AppendTwoDigits( aBuffer, aInUTC.GetMin() );
    aBuffer.append( ':' );

    impl_AppendTwoDigits( aBuffer, aInUTC.GetSec() );
    aBuffer.append( 'Z' ); // we are in UTC

    return aBuffer.makeStringAndClear();
}

void DocxAttributeOutput::StartRedline( const SwRedlineData* pRedlineData )
{
    m_pRedlineData = pRedlineData;

    if ( !m_pRedlineData )
        return;

    // FIXME check if it's necessary to travel over the Next()'s in pRedlineData

    OString aId( OString::valueOf( m_nRedlineId++ ) );

    const String &rAuthor( SW_MOD()->GetRedlineAuthor( pRedlineData->GetAuthor() ) );
    OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );

    OString aDate( impl_DateTimeToOString( pRedlineData->GetTimeStamp() ) );

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
            OSL_TRACE( "TODO DocxAttributeOutput::StartRedline()\n" );
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
            OSL_TRACE( "TODO DocxAttributeOutput::EndRedline()\n" );
            break;
        default:
            break;
    }

    m_pRedlineData = NULL;
}

void DocxAttributeOutput::FormatDrop( const SwTxtNode& /*rNode*/, const SwFmtDrop& /*rSwFmtDrop*/, sal_uInt16 /*nStyle*/, ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/, ww8::WW8TableNodeInfoInner::Pointer_t )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle )\n" );
}

void DocxAttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OString aStyleId( "style" );
    aStyleId += OString::valueOf( sal_Int32( nStyle ) );

    m_pSerializer->singleElementNS( XML_w, XML_pStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

#if 0
void DocxAttributeOutput::InTable()
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::InTable()\n" );
#endif
}


void DocxAttributeOutput::TableRowProperties( bool /*bHeader*/, long /*nCellHeight*/, bool /*bCannotSplit*/, bool /*bRightToLeft*/ )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::TableRowProperties()\n" );
#endif
}
#endif

static OString impl_ConvertColor( const Color &rColor )
{
    OString color( "auto" );
    if ( rColor.GetColor() != COL_AUTO )
    {
        const char pHexDigits[] = "0123456789ABCDEF";
        char pBuffer[] = "000000";

        pBuffer[0] = pHexDigits[ ( rColor.GetRed()   >> 4 ) & 0x0F ];
        pBuffer[1] = pHexDigits[   rColor.GetRed()          & 0x0F ];
        pBuffer[2] = pHexDigits[ ( rColor.GetGreen() >> 4 ) & 0x0F ];
        pBuffer[3] = pHexDigits[   rColor.GetGreen()        & 0x0F ];
        pBuffer[4] = pHexDigits[ ( rColor.GetBlue()  >> 4 ) & 0x0F ];
        pBuffer[5] = pHexDigits[   rColor.GetBlue()         & 0x0F ];

        color = OString( pBuffer );
    }
    return color;
}

static void impl_borderLine( FSHelperPtr pSerializer, sal_Int32 elementToken, const SvxBorderLine* pBorderLine, sal_uInt16 nDist )
{
    FastAttributeList* pAttr = pSerializer->createAttrList();

    sal_uInt16 inW = pBorderLine->GetInWidth();
    sal_uInt16 outW = pBorderLine->GetOutWidth();
    sal_uInt16 nWidth  = inW + outW;

    // Compute val attribute value
    // Can be one of:
    //      single, double,
    //      basicWideOutline, basicWideInline
    // OOXml also supports those types of borders, but we'll try to play with the first ones.
    //      thickThinMediumGap, thickThinLargeGap, thickThinSmallGap
    //      thinThickLargeGap, thinThickMediumGap, thinThickSmallGap
    const char* pVal = "single";
    if ( pBorderLine->isDouble() )
    {
        if ( inW == outW )
            pVal = ( sal_Char* )"double";
        else if ( inW > outW )
        {
            pVal = ( sal_Char* )"thinThickMediumGap";
        }
        else if ( inW < outW )
        {
            pVal = ( sal_Char* )"thickThinMediumGap";
        }
    }

    pAttr->add( FSNS( XML_w, XML_val ), OString( pVal ) );

    // Compute the sz attribute

    // The unit is the 8th of point
    nWidth = sal_Int32( nWidth / 2.5 );
    sal_uInt16 nMinWidth = 2;
    sal_uInt16 nMaxWidth = 96;

    if ( nWidth > nMaxWidth )
        nWidth = nMaxWidth;
    else if ( nWidth < nMinWidth )
        nWidth = nMinWidth;

    pAttr->add( FSNS( XML_w, XML_sz ), OString::valueOf( sal_Int32( nWidth ) ) );

    // Get the distance (in pt)
    pAttr->add( FSNS( XML_w, XML_space ), OString::valueOf( sal_Int32( nDist / 20 ) ) );

    // Get the color code as an RRGGBB hex value
    OString sColor( impl_ConvertColor( pBorderLine->GetColor( ) ) );
    pAttr->add( FSNS( XML_w, XML_color ), sColor );

    XFastAttributeListRef xAttrs( pAttr );
    pSerializer->singleElementNS( XML_w, elementToken, xAttrs );
}

static void impl_pageBorders( FSHelperPtr pSerializer, const SvxBoxItem& rBox )
{
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    static const sal_uInt16 aXmlElements[] =
    {
        XML_top, XML_left, XML_bottom, XML_right
    };
    const sal_uInt16* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        if ( pLn )
            impl_borderLine( pSerializer, aXmlElements[i], pLn, 0 );
    }
}

void DocxAttributeOutput::TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    m_pSerializer->startElementNS( XML_w, XML_tcPr, FSEND );

    const SwTableBox *pTblBox = pTableTextNodeInfoInner->getTableBox( );

    // The cell borders
    m_pSerializer->startElementNS( XML_w, XML_tcBorders, FSEND );
    SwFrmFmt *pFmt = pTblBox->GetFrmFmt( );
    impl_pageBorders( m_pSerializer, pFmt->GetBox( ) );
    m_pSerializer->endElementNS( XML_w, XML_tcBorders );

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

    // Horizontal spans
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    SwWriteTableCell *pCell = pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];

    sal_uInt16 nColSpan = pCell->GetColSpan();
    if ( nColSpan > 1 )
        m_pSerializer->singleElementNS( XML_w, XML_gridSpan,
                FSNS( XML_w, XML_val ), OString::valueOf( sal_Int32( nColSpan ) ).getStr(),
                FSEND );

    TableBackgrounds( pTableTextNodeInfoInner );

    // Cell prefered width
    SwTwips nWidth = GetGridCols( pTableTextNodeInfoInner )[ pTableTextNodeInfoInner->getCell() ];
    m_pSerializer->singleElementNS( XML_w, XML_tcW,
           FSNS( XML_w, XML_w ), OString::valueOf( sal_Int32( nWidth ) ).getStr( ),
           FSNS( XML_w, XML_type ), "dxa",
           FSEND );

    // Cell margins
    m_pSerializer->startElementNS( XML_w, XML_tcMar, FSEND );
    const SvxBoxItem& rBox = pFmt->GetBox( );
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    static const sal_uInt16 aXmlElements[] =
    {
        XML_top, XML_left, XML_bottom, XML_right
    };
    const sal_uInt16* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        sal_Int32 nDist = sal_Int32( rBox.GetDistance( *pBrd ) );
        m_pSerializer->singleElementNS( XML_w, aXmlElements[i],
               FSNS( XML_w, XML_w ), OString::valueOf( nDist ).getStr( ),
               FSNS( XML_w, XML_type ), "dxa",
               FSEND );
    }

    m_pSerializer->endElementNS( XML_w, XML_tcMar );

    TableVerticalCell( pTableTextNodeInfoInner );

    m_pSerializer->endElementNS( XML_w, XML_tcPr );
}

void DocxAttributeOutput::InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans (and maybe other infos)
    GetTablePageSize( pTableTextNodeInfoInner, nPageSize, bRelBoxSize );

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

    InitTableHelper( pTableTextNodeInfoInner );
    TableDefinition( pTableTextNodeInfoInner );
}

void DocxAttributeOutput::EndTable()
{
    m_pSerializer->endElementNS( XML_w, XML_tbl );

    if ( m_nTableDepth > 0 )
        --m_nTableDepth;

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
    // Write the table properties
    m_pSerializer->startElementNS( XML_w, XML_tblPr, FSEND );

    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans (and maybe other infos)
    GetTablePageSize( pTableTextNodeInfoInner, nPageSize, bRelBoxSize );

    // Output the table prefered width
    if ( nPageSize != 0 )
        m_pSerializer->singleElementNS( XML_w, XML_tblW,
                FSNS( XML_w, XML_w ), OString::valueOf( sal_Int32( nPageSize ) ).getStr( ),
                FSNS( XML_w, XML_type ), "dxa",
                FSEND );

    // Output the table borders
    TableDefaultBorders( pTableTextNodeInfoInner );
    TableBidi( pTableTextNodeInfoInner );

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
            pJcVal = "right";
            break;
        default:
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
        {
            pJcVal = "left";
            nIndent = sal_Int32( pTblFmt->GetLRSpace( ).GetLeft( ) );
            break;
        }
    }
    m_pSerializer->singleElementNS( XML_w, XML_jc,
            FSNS( XML_w, XML_val ), pJcVal,
            FSEND );

    // Table indent
    if ( nIndent != 0 )
        m_pSerializer->singleElementNS( XML_w, XML_tblInd,
                FSNS( XML_w, XML_w ), OString::valueOf( nIndent ).getStr( ),
                FSNS( XML_w, XML_type ), "dxa",
                FSEND );

    m_pSerializer->endElementNS( XML_w, XML_tblPr );


    // Write the table grid infos
    m_pSerializer->startElementNS( XML_w, XML_tblGrid, FSEND );

    std::vector<SwTwips> gridCols = GetGridCols( pTableTextNodeInfoInner );
    for ( std::vector<SwTwips>::const_iterator it = gridCols.begin(); it != gridCols.end(); ++it )
        m_pSerializer->singleElementNS( XML_w, XML_gridCol,
               FSNS( XML_w, XML_w ), OString::valueOf( sal_Int32( *it ) ).getStr( ),
               FSEND );

    m_pSerializer->endElementNS( XML_w, XML_tblGrid );
}

void DocxAttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrmFmt * pFrmFmt = pTabBox->GetFrmFmt();

    // the defaults of the table are taken from the top-left cell
    m_pSerializer->startElementNS( XML_w, XML_tblBorders, FSEND );
    impl_pageBorders( m_pSerializer, pFrmFmt->GetBox( ) );
    m_pSerializer->endElementNS( XML_w, XML_tblBorders );
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

        OString sColor = impl_ConvertColor( aColor );
        m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor.getStr( ),
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
                    FSNS( XML_w, XML_val ), OString::valueOf( nHeight ).getStr( ),
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
    const char* pCantSplit = ( !rSplittable.GetValue( ) ) ? "on" : "off";

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
                FSNS( XML_w, XML_val ), "on",
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
}

void DocxAttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t /*pNodeInfo*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo )\n" );
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
    OSL_TRACE( "TODO: DocxAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )\n" );
}

void DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
#if OSL_DEBUG_LEVEL > 0
    fprintf( stderr, "TODO: DocxAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )\n" );
#endif
}

void DocxAttributeOutput::TableRowEnd( sal_uInt32 /*nDepth*/ )
{
    OSL_TRACE( "TODO: DocxAttributeOutput::TableRowEnd( sal_uInt32 nDepth = 1 )\n" );
}

void DocxAttributeOutput::StartStyles()
{
    m_pSerializer->startElementNS( XML_w, XML_styles,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );
}

void DocxAttributeOutput::EndStyles( sal_uInt16 /*nNumberOfStyles*/ )
{
    m_pSerializer->endElementNS( XML_w, XML_styles );
}

void DocxAttributeOutput::DefaultStyle( sal_uInt16 nStyle )
{
    // are these the values of enum ww::sti (see ../inc/wwstyles.hxx)?
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::DefaultStyle( sal_uInt16 nStyle )- %d\n", nStyle );
#else
    (void) nStyle; // to quiet the warning
#endif
}

void DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode& rGrfNode, const Size& rSize )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FlyFrameGraphic( const SwGrfNode& rGrfNode, const Size& rSize ) - some stuff still missing\n" );
#endif
    // create the relation ID
    OString aRelId;
    sal_Int32 nImageType;
    if ( rGrfNode.IsLinkedFile() )
    {
        // linked image, just create the relation
        String aFileName;
        rGrfNode.GetFileFilterNms( &aFileName, 0 );

        // TODO Convert the file name to relative for better interoperability

        aRelId = m_rExport.AddRelation(
                    S( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" ),
                    OUString( aFileName ),
                    S( "External" ) );

        nImageType = XML_link;
    }
    else
    {
        // inline, we also have to write the image itself
        Graphic& rGraphic = const_cast< Graphic& >( rGrfNode.GetGrf() );

        m_rDrawingML.SetFS( m_pSerializer ); // to be sure that we write to the right stream
        OUString aImageId = m_rDrawingML.WriteImage( rGraphic );

        aRelId = OUStringToOString( aImageId, RTL_TEXTENCODING_UTF8 );

        nImageType = XML_embed;
    }

    if ( aRelId.getLength() == 0 )
        return;

    m_pSerializer->startElementNS( XML_w, XML_drawing,
            FSEND );
    m_pSerializer->startElementNS( XML_wp, XML_inline,
            XML_distT, "0", XML_distB, "0", XML_distL, "0", XML_distR, "0",
            FSEND );

    // extent of the image
    OString aWidth( OString::valueOf( TwipsToEMU( rSize.Width() ) ) );
    OString aHeight( OString::valueOf( TwipsToEMU( rSize.Height() ) ) );
    m_pSerializer->singleElementNS( XML_wp, XML_extent,
            XML_cx, aWidth.getStr(),
            XML_cy, aHeight.getStr(),
            FSEND );
    // TODO - the right effectExtent, extent including the effect
    m_pSerializer->singleElementNS( XML_wp, XML_effectExtent,
            XML_l, "0", XML_t, "0", XML_r, "0", XML_b, "0",
            FSEND );

    // picture description
    // TODO the right image description
    m_pSerializer->startElementNS( XML_wp, XML_docPr,
            XML_id, "1",
            XML_name, "Picture",
            XML_descr, "A description...",
            FSEND );
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
    // TODO the right image description
    m_pSerializer->startElementNS( XML_pic, XML_cNvPr,
            XML_id, "0",
            XML_name, "Picture",
            XML_descr, "A description...",
            FSEND );
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
    m_pSerializer->singleElementNS( XML_a, XML_srcRect,
            FSEND );
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
    m_pSerializer->endElementNS( XML_pic, XML_spPr );

    m_pSerializer->endElementNS( XML_pic, XML_pic );

    m_pSerializer->endElementNS( XML_a, XML_graphicData );
    m_pSerializer->endElementNS( XML_a, XML_graphic );
    m_pSerializer->endElementNS( XML_wp, XML_inline );

    m_pSerializer->endElementNS( XML_w, XML_drawing );
}

void DocxAttributeOutput::OutputFlyFrame_Impl( const sw::Frame &rFrame, const Point& /*rNdTopLeft*/ )
{
    m_pSerializer->mark();

    switch ( rFrame.GetWriterType() )
    {
        case sw::Frame::eGraphic:
            {
                const SwNode *pNode = rFrame.GetContent();
                const SwGrfNode *pGrfNode = pNode ? pNode->GetGrfNode() : 0;
                if ( pGrfNode )
                    FlyFrameGraphic( *pGrfNode, rFrame.GetLayoutSize() );
            }
            break;
        case sw::Frame::eDrawing:
            {
                const SdrObject* pSdrObj = rFrame.GetFrmFmt().FindRealSdrObject();
                if ( pSdrObj )
                {
                    bool bSwapInPage = false;
                    if ( !pSdrObj->GetPage() )
                    {
                        if ( SdrModel* pModel = m_rExport.pDoc->GetDrawModel() )
                        {
                            if ( SdrPage *pPage = pModel->GetPage( 0 ) )
                            {
                                bSwapInPage = true;
                                const_cast< SdrObject* >( pSdrObj )->SetPage( pPage );
                            }
                        }
                    }

                    m_pSerializer->startElementNS( XML_w, XML_pict,
                            FSEND );

                    m_rExport.VMLExporter().AddSdrObject( *pSdrObj );

                    m_pSerializer->endElementNS( XML_w, XML_pict );

                    if ( bSwapInPage )
                        const_cast< SdrObject* >( pSdrObj )->SetPage( 0 );
                }
            }
            break;
        default:
#if OSL_DEBUG_LEVEL > 0
            OSL_TRACE( "TODO DocxAttributeOutput::OutputFlyFrame_Impl( const sw::Frame& rFrame, const Point& rNdTopLeft ) - frame type '%s'\n",
                    rFrame.GetWriterType() == sw::Frame::eTxtBox? "eTxtBox":
                    ( rFrame.GetWriterType() == sw::Frame::eOle? "eOle":
                      ( rFrame.GetWriterType() == sw::Frame::eFormControl? "eFormControl": "???" ) ) );
#endif
            break;
    }

    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_POSTPONE );
}

void DocxAttributeOutput::StartStyle( const String& rName, bool bPapFmt,
        sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 /*nWwId*/, sal_uInt16 nId )
{
    OString aStyle( "style" );

    m_pSerializer->startElementNS( XML_w, XML_style,
            FSNS( XML_w, XML_type ), bPapFmt? "paragraph": "character", // FIXME is this correct?
            FSNS( XML_w, XML_styleId ), ( aStyle + OString::valueOf( sal_Int32( nId ) ) ).getStr(),
            FSEND );

    m_pSerializer->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), OUStringToOString( OUString( rName ), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    if ( nBase != 0x0FFF )
    {
        m_pSerializer->singleElementNS( XML_w, XML_basedOn,
                FSNS( XML_w, XML_val ), ( aStyle + OString::valueOf( sal_Int32( nBase ) ) ).getStr(),
                FSEND );
    }

    m_pSerializer->singleElementNS( XML_w, XML_next,
            FSNS( XML_w, XML_val ), ( aStyle + OString::valueOf( sal_Int32( nNext ) ) ).getStr(),
            FSEND );
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

void DocxAttributeOutput::OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt& /*rNFmt*/, const SwFmt& /*rFmt*/ )
{
    if ( nLvl >= WW8ListManager::nMaxLevel )
        nLvl = WW8ListManager::nMaxLevel - 1;

    m_pSerializer->singleElementNS( XML_w, XML_outlineLvl,
            FSNS( XML_w, XML_val ), OString::valueOf( sal_Int32( nLvl ) ).getStr( ),
            FSEND );
}

void DocxAttributeOutput::PageBreakBefore( bool bBreak )
{
    if ( bBreak )
        m_pSerializer->singleElementNS( XML_w, XML_pageBreakBefore, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_pageBreakBefore,
                FSNS( XML_w, XML_val ), "off",
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
                    m_pSectionInfo = pSectionInfo;
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
#if OSL_DEBUG_LEVEL > 0
            OSL_TRACE( "Unknown section break to write: %d\n", nC );
#endif
            break;
    }
}

void DocxAttributeOutput::StartSection()
{
    m_pSerializer->startElementNS( XML_w, XML_sectPr, FSEND );
    m_bOpenedSectPr = true;
}

void DocxAttributeOutput::EndSection()
{
    // Write the section properties
    if ( m_pSpacingAttrList )
    {
        XFastAttributeListRef xAttrList( m_pSpacingAttrList );
        m_pSpacingAttrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_pgMar, xAttrList );
    }

    m_pSerializer->endElementNS( XML_w, XML_sectPr );
    m_bOpenedSectPr = false;
}

void DocxAttributeOutput::SectionFormProtection( bool bProtected )
{
    if ( bProtected )
        m_pSerializer->singleElementNS( XML_w, XML_formProt, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_formProt,
                FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::SectionLineNumbering( sal_uLong /*nRestartNo*/, const SwLineNumberInfo& /*rLnNumInfo*/ )
{
    // see 2.6.8 lnNumType (Line Numbering Settings)
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::SectionLineNumbering()\n" );
#endif
}

void DocxAttributeOutput::SectionTitlePage()
{
    m_pSerializer->singleElementNS( XML_w, XML_titlePg, FSEND );
}

void DocxAttributeOutput::SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* /*pFirstPageFmt*/ )
{
    // Output the margins


    const SvxBoxItem& rBox = pFmt->GetBox( );

    const SvxBorderLine* pBottom = rBox.GetBottom( );
    const SvxBorderLine* pTop = rBox.GetTop( );
    const SvxBorderLine* pLeft = rBox.GetLeft( );
    const SvxBorderLine* pRight = rBox.GetRight( );

    if ( pBottom || pTop || pLeft || pRight )
    {
        // All distances are relative to the text margins
        m_pSerializer->startElementNS( XML_w, XML_pgBorders,
               FSNS( XML_w, XML_display ), "allPages",
               FSNS( XML_w, XML_offsetFrom ), "text",
               FSEND );

        m_pSerializer->mark();

        m_pSerializer->endElementNS( XML_w, XML_pgBorders );
        m_pSerializer->mark();
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

void DocxAttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, sal_uInt16 nPageRestartNumber )
{
    // FIXME Not called properly with page styles like "First Page"

    FastAttributeList* pAttr = m_pSerializer->createAttrList();

    // 0 means no restart: then don't output that attribute if 0
    if ( nPageRestartNumber > 0 )
       pAttr->add( FSNS( XML_w, XML_start ), OString::valueOf( sal_Int32( nPageRestartNumber ) ) );

    // nNumType corresponds to w:fmt. See WW8Export::GetNumId() for more precisions
    OString aFmt( impl_NumberingType( nNumType ) );
    if ( aFmt.getLength() )
        pAttr->add( FSNS( XML_w, XML_fmt ), aFmt.getStr() );

    XFastAttributeListRef xAttrs( pAttr );
    m_pSerializer->singleElementNS( XML_w, XML_pgNumType, xAttrs );

    // see 2.6.12 pgNumType (Page Numbering Settings)
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::SectionPageNumbering()\n" );
#endif
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

void DocxAttributeOutput::StartFont( const String& rFamilyName ) const
{
    m_pSerializer->startElementNS( XML_w, XML_font,
            FSNS( XML_w, XML_name ), OUStringToOString( OUString( rFamilyName ), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
}

void DocxAttributeOutput::EndFont() const
{
    m_pSerializer->endElementNS( XML_w, XML_font );
}

void DocxAttributeOutput::FontAlternateName( const String& rName ) const
{
    m_pSerializer->singleElementNS( XML_w, XML_altName,
            FSNS( XML_w, XML_val ), OUStringToOString( OUString( rName ), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
}

void DocxAttributeOutput::FontCharset( sal_uInt8 nCharSet ) const
{
    OString aCharSet( OString::valueOf( sal_Int32( nCharSet ), 16 ) );
    if ( aCharSet.getLength() == 1 )
        aCharSet = OString( "0" ) + aCharSet;

    m_pSerializer->singleElementNS( XML_w, XML_charset,
            FSNS( XML_w, XML_val ), aCharSet.getStr(),
            FSEND );
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

void DocxAttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule )
{
    // nId is the same both for abstract numbering definition as well as the
    // numbering definition itself
    // TODO check that this is actually true & fix if not ;-)
    OString aId( OString::valueOf( sal_Int32( nId ) ) );

    m_pSerializer->startElementNS( XML_w, XML_num,
            FSNS( XML_w, XML_numId ), aId.getStr(),
            FSEND );

    m_pSerializer->singleElementNS( XML_w, XML_abstractNumId,
            FSNS( XML_w, XML_val ), aId.getStr(),
            FSEND );

#if OSL_DEBUG_LEVEL > 0
    // TODO ww8 version writes this, anything to do about it here?
    if ( rRule.IsContinusNum() )
        OSL_TRACE( "TODO DocxAttributeOutput::NumberingDefinition()\n" );
#else
    (void) rRule; // to quiet the warning...
#endif

    m_pSerializer->endElementNS( XML_w, XML_num );
}

void DocxAttributeOutput::StartAbstractNumbering( sal_uInt16 nId )
{
    m_pSerializer->startElementNS( XML_w, XML_abstractNum,
            FSNS( XML_w, XML_abstractNumId ), OString::valueOf( sal_Int32( nId ) ).getStr(),
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
        sal_Int16 /*nListTabPos*/,
        const String &rNumberingString )
{
    m_pSerializer->startElementNS( XML_w, XML_lvl,
            FSNS( XML_w, XML_ilvl ), OString::valueOf( sal_Int32( nLevel ) ).getStr(),
            FSEND );

    // start with the nStart value
    m_pSerializer->singleElementNS( XML_w, XML_start,
            FSNS( XML_w, XML_val ), OString::valueOf( sal_Int32( nStart ) ).getStr(),
            FSEND );

    // format
    OString aFmt( impl_NumberingType( nNumberingType ) );

    if ( aFmt.getLength() )
        m_pSerializer->singleElementNS( XML_w, XML_numFmt,
                FSNS( XML_w, XML_val ), aFmt.getStr(),
                FSEND );

    // justification
    const char *pJc;
    switch ( eAdjust )
    {
        case SVX_ADJUST_CENTER: pJc = "center"; break;
        case SVX_ADJUST_RIGHT:  pJc = "right";  break;
        default:                pJc = "left";   break;
    }
    m_pSerializer->singleElementNS( XML_w, XML_lvlJc,
            FSNS( XML_w, XML_val ), pJc,
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
            aBuffer.appendAscii( "%" );
            aBuffer.append( OUString::valueOf( sal_Int32( *pIt ) + 1 ) );

            pPrev = pIt + 1;
        }
        ++pIt;
    }
    if ( pPrev < pIt )
        aBuffer.append( pPrev, pIt - pPrev );

    m_pSerializer->singleElementNS( XML_w, XML_lvlText,
            FSNS( XML_w, XML_val ), OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    // indentation
    m_pSerializer->startElementNS( XML_w, XML_pPr, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_ind,
            FSNS( XML_w, XML_left ), OString::valueOf( sal_Int32( nIndentAt ) ).getStr(),
            FSNS( XML_w, XML_hanging ), OString::valueOf( sal_Int32( -nFirstLineIndex ) ).getStr(),
            FSEND );
    m_pSerializer->endElementNS( XML_w, XML_pPr );

    // font
    if ( pOutSet )
    {
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );

        if ( pFont )
        {
            OString aFamilyName( OUStringToOString( OUString( pFont->GetFamilyName() ), RTL_TEXTENCODING_UTF8 ) );
            m_pSerializer->singleElementNS( XML_w, XML_rFonts,
                    FSNS( XML_w, XML_ascii ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_hAnsi ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_cs ), aFamilyName.getStr(),
                    FSNS( XML_w, XML_hint ), "default",
                    FSEND );
        }
        m_rExport.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN );

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
            m_pSerializer->singleElementNS( XML_w, XML_smallCaps, FSNS( XML_w, XML_val ), "off", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_caps, FSNS( XML_w, XML_val ), "off", FSEND );
            break;
    }
}

void DocxAttributeOutput::CharColor( const SvxColorItem& rColor )
{
    const Color aColor( rColor.GetValue() );
    OString aColorString;

    aColorString = impl_ConvertColor( aColor );

    m_pSerializer->singleElementNS( XML_w, XML_color,
            FSNS( XML_w, XML_val ), aColorString.getStr(), FSEND );
}

void DocxAttributeOutput::CharContour( const SvxContourItem& rContour )
{
    if ( rContour.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_outline, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_outline, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharCrossedOut( const SvxCrossedOutItem& rCrossedOut )
{
    switch ( rCrossedOut.GetStrikeout() )
    {
        case STRIKEOUT_DOUBLE:
            m_pSerializer->singleElementNS( XML_w, XML_dstrike, FSEND );
            break;
        case STRIKEOUT_NONE:
            m_pSerializer->singleElementNS( XML_w, XML_dstrike, FSNS( XML_w, XML_val ), "off", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_strike, FSNS( XML_w, XML_val ), "off", FSEND );
            break;
        default:
            m_pSerializer->singleElementNS( XML_w, XML_strike, FSEND );
            break;
    }
}

void DocxAttributeOutput::CharEscapement( const SvxEscapementItem& /*rEscapement*/ )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::CharEscapement()\n" );
#endif
}

void DocxAttributeOutput::CharFont( const SvxFontItem& rFont)
{
    if (!m_pFontsAttrList)
        m_pFontsAttrList = m_pSerializer->createAttrList();
    OUString sFontName(rFont.GetFamilyName());
    OString sFontNameUtf8 = OUStringToOString(sFontName, RTL_TEXTENCODING_UTF8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_ascii), sFontNameUtf8);
    m_pFontsAttrList->add(FSNS(XML_w, XML_hAnsi), sFontNameUtf8);
}

void DocxAttributeOutput::CharFontSize( const SvxFontHeightItem& rFontSize)
{
    OString fontSize = OString::valueOf( sal_Int32( ( rFontSize.GetHeight() + 5 ) / 10 ) );

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
    OString aKerning = OString::valueOf( ( sal_Int32 ) rKerning.GetValue() );
    m_pSerializer->singleElementNS( XML_w, XML_kern, FSNS(XML_w, XML_val), aKerning.getStr(), FSEND );
}

void DocxAttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    if (!m_pCharLangAttrList)
        m_pCharLangAttrList = m_pSerializer->createAttrList();

    ::com::sun::star::lang::Locale xLocale= MsLangId::convertLanguageToLocale( rLanguage.GetLanguage( ) );
    OString sLanguage = OUStringToOString(xLocale.Language, RTL_TEXTENCODING_UTF8);
    OString sCountry = OUStringToOString(xLocale.Country, RTL_TEXTENCODING_UTF8);

    OString aLanguageCode = sLanguage + "-" + sCountry;

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
        m_pSerializer->singleElementNS( XML_w, XML_i, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    if ( rShadow.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_shadow, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_shadow, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    const char *pUnderline;

    switch ( rUnderline.GetLineStyle() )
    {
        case UNDERLINE_SINGLE:         pUnderline = "single";          break;
        case UNDERLINE_BOLD:           pUnderline = "thick";           break;
        case UNDERLINE_DOUBLE:         pUnderline = "double";          break;
        case UNDERLINE_DOTTED:         pUnderline = "dotted";          break;
        case UNDERLINE_DASH:           pUnderline = "dash";            break;
        case UNDERLINE_DASHDOT:        pUnderline = "dotDash";         break;
        case UNDERLINE_DASHDOTDOT:     pUnderline = "dotDotDash";      break;
        case UNDERLINE_WAVE:           pUnderline = "wave";            break;
        case UNDERLINE_BOLDDOTTED:     pUnderline = "dottedHeavy";     break;
        case UNDERLINE_BOLDDASH:       pUnderline = "dashedHeavy";     break;
        case UNDERLINE_LONGDASH:       pUnderline = "dashLongHeavy";   break;
        case UNDERLINE_BOLDLONGDASH:   pUnderline = "dashLongHeavy";   break;
        case UNDERLINE_BOLDDASHDOT:    pUnderline = "dashDotHeavy";    break;
        case UNDERLINE_BOLDDASHDOTDOT: pUnderline = "dashDotDotHeavy"; break;
        case UNDERLINE_BOLDWAVE:       pUnderline = "wavyHeavy";       break;
        case UNDERLINE_DOUBLEWAVE:     pUnderline = "wavyDouble";      break;
        case UNDERLINE_NONE:           // fall through
        default:                       pUnderline = "none";            break;
    }

    m_pSerializer->singleElementNS( XML_w, XML_u, FSNS( XML_w, XML_val ), pUnderline, FSEND );
}

void DocxAttributeOutput::CharWeight( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_b, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_b, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharAutoKern( const SvxAutoKernItem& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::CharAutoKern()\n" );
#endif
}

void DocxAttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    if ( rBlink.GetValue() )
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS( XML_w, XML_val ), "blinkBackground", FSEND );
    else
        m_pSerializer->singleElementNS(XML_w, XML_effect, FSNS( XML_w, XML_val ), "none", FSEND );
}

void DocxAttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    m_pSerializer->singleElementNS( XML_w, XML_shd,
            FSNS( XML_w, XML_fill ), impl_ConvertColor( rBrush.GetColor() ).getStr(), FSEND );
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
        m_pSerializer->singleElementNS( XML_w, XML_i, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharWeightCJK( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_b, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_b, FSNS( XML_w, XML_val ), "off", FSEND );
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
        m_pSerializer->singleElementNS( XML_w, XML_iCs, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    if ( rWeight.GetWeight() == WEIGHT_BOLD )
        m_pSerializer->singleElementNS( XML_w, XML_bCs, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_bCs, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::CharRotate( const SvxCharRotateItem& rRotate)
{
    if ( !rRotate.GetValue() )
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
            FSNS( XML_w, XML_val ), rtl::OString::valueOf( sal_Int32( rScaleWidth.GetValue() ) ).getStr(), FSEND );
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
            m_pSerializer->singleElementNS( XML_w, XML_emboss, FSNS( XML_w, XML_val ), "off", FSEND );
            m_pSerializer->singleElementNS( XML_w, XML_imprint, FSNS( XML_w, XML_val ), "off", FSEND );
            break;
    }
}

void DocxAttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    if ( rHidden.GetValue() )
        m_pSerializer->singleElementNS( XML_w, XML_vanish, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_vanish, FSNS( XML_w, XML_val ), "off", FSEND );
}

void DocxAttributeOutput::TextINetFormat( const SwFmtINetFmt& rLink )
{
    const SwTxtINetFmt* pINetFmt = rLink.GetTxtINetFmt();
    const SwCharFmt* pCharFmt = pINetFmt->GetCharFmt();

    OString aStyleId( "style" );
    aStyleId += OString::valueOf( sal_Int32( m_rExport.GetId( *pCharFmt ) ) );

    m_pSerializer->singleElementNS( XML_w, XML_rStyle, FSNS( XML_w, XML_val ), aStyleId.getStr(), FSEND );
}

void DocxAttributeOutput::TextCharFormat( const SwFmtCharFmt& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::TextCharFormat()\n" );
#endif
}

void DocxAttributeOutput::RefField( const SwField&  rFld, const String& rRef )
{
    sal_uInt16 nType = rFld.GetTyp( )->Which( );
    if ( nType == RES_GETEXPFLD )
    {
        String sCmd = FieldString( ww::eREF );
        sCmd.APPEND_CONST_ASC( "\"" );
        sCmd += rRef;
        sCmd.APPEND_CONST_ASC( "\" " );

        m_rExport.OutputField( &rFld, ww::eREF, sCmd );
    }

    // There is nothing to do here for the set fields
}

void DocxAttributeOutput::HiddenField( const SwField& /*rFld*/ )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::HiddenField()\n" );
#endif
}

void DocxAttributeOutput::PostitField( const SwField* /* pFld*/ )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::PostitField()\n" );
#endif
}

bool DocxAttributeOutput::DropdownField( const SwField* pFld )
{
    bool bExpand = false;

    ww::eField eType = ww::eFORMDROPDOWN;
    String sCmd = FieldString( eType  );
    GetExport( ).OutputField( pFld, eType, sCmd );

    return bExpand;
}

void DocxAttributeOutput::SetField( const SwField& rFld, ww::eField eType, const String& rCmd )
{
    // field bookmarks are handled in the EndRun method
    GetExport().OutputField(&rFld, eType, rCmd );
}

void DocxAttributeOutput::WriteExpand( const SwField* pFld )
{
    // Will be written in the next End Run
    String sCmd;
    m_rExport.OutputField( pFld, ww::eUNKNOWN, sCmd );
}

void DocxAttributeOutput::WriteField_Impl( const SwField* pFld, ww::eField eType, const String& rFldCmd, sal_uInt8 nMode )
{
    struct FieldInfos infos;
    infos.pField = pFld;
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

void DocxAttributeOutput::WriteBookmarks_Impl( std::vector< OUString >& rStarts,
        std::vector< OUString >& rEnds )
{
    for ( std::vector< OUString >::const_iterator it = rStarts.begin(), end = rStarts.end(); it < end; ++it )
    {
        OString rName = OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr( );
        m_rMarksStart.push_back( rName );
    }
    rStarts.clear();

    for ( std::vector< OUString >::const_iterator it = rEnds.begin(), end = rEnds.end(); it < end; ++it )
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

    OString aStyleId( "style" );
    aStyleId += OString::valueOf( sal_Int32( m_rExport.GetId( *pCharFmt ) ) );

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
    if ( pFootnote->GetNumStr().Len() == 0 )
    {
        // autonumbered
        m_pSerializer->singleElementNS( XML_w, nToken,
                FSNS( XML_w, XML_id ), ::rtl::OString::valueOf( nId ).getStr(),
                FSEND );
    }
    else
    {
        // not autonumbered
        m_pSerializer->singleElementNS( XML_w, nToken,
                FSNS( XML_w, XML_customMarkFollows ), "1",
                FSNS( XML_w, XML_id ), ::rtl::OString::valueOf( nId ).getStr(),
                FSEND );

        RunText( pFootnote->GetNumStr() );
    }
}

void DocxAttributeOutput::FootnotesEndnotes( bool bFootnotes )
{
    const FootnotesVector& rVector = bFootnotes? m_pFootnotesList->getVector(): m_pEndnotesList->getVector();

    sal_Int32 nBody = bFootnotes? XML_footnotes: XML_endnotes;
    sal_Int32 nItem = bFootnotes? XML_footnote:  XML_endnote;

    m_pSerializer->startElementNS( XML_w, nBody,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );

    sal_Int32 nIndex = 0;

    // separator
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::valueOf( nIndex++ ).getStr(),
            FSNS( XML_w, XML_type ), "separator",
            FSEND );
    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_separator, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // separator
    m_pSerializer->startElementNS( XML_w, nItem,
            FSNS( XML_w, XML_id ), OString::valueOf( nIndex++ ).getStr(),
            FSNS( XML_w, XML_type ), "continuationSeparator",
            FSEND );
    m_pSerializer->startElementNS( XML_w, XML_p, FSEND );
    m_pSerializer->startElementNS( XML_w, XML_r, FSEND );
    m_pSerializer->singleElementNS( XML_w, XML_continuationSeparator, FSEND );
    m_pSerializer->endElementNS( XML_w, XML_r );
    m_pSerializer->endElementNS( XML_w, XML_p );
    m_pSerializer->endElementNS( XML_w, nItem );

    // footnotes/endnotes themselves
    for ( FootnotesVector::const_iterator i = rVector.begin(); i != rVector.end(); ++i, ++nIndex )
    {
        m_pSerializer->startElementNS( XML_w, nItem,
                FSNS( XML_w, XML_id ), OString::valueOf( nIndex ).getStr(),
                FSEND );

        const SwNodeIndex* pIndex = (*i)->GetTxtFtn()->GetStartNode();

        m_rExport.WriteSpecialText( pIndex->GetIndex() + 1,
                pIndex->GetNode().EndOfSectionIndex(),
                bFootnotes? TXT_FTN: TXT_EDN );

        m_pSerializer->endElementNS( XML_w, nItem );
    }

    m_pSerializer->endElementNS( XML_w, nBody );

}

void DocxAttributeOutput::ParaLineSpacing_Impl( short nSpace, short /*nMulti*/ )
{
    if ( !m_pSpacingAttrList )
        m_pSpacingAttrList = m_pSerializer->createAttrList();

    if ( nSpace < 0 )
    {
        m_pSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "exact" );
        m_pSpacingAttrList->add( FSNS( XML_w, XML_line ), OString::valueOf( sal_Int32( -nSpace ) ) );
    }
    else if ( nSpace > 0 )
    {
        m_pSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "atLeast" );
        m_pSpacingAttrList->add( FSNS( XML_w, XML_line ), OString::valueOf( sal_Int32( nSpace ) ) );
    }
    else
        m_pSpacingAttrList->add( FSNS( XML_w, XML_lineRule ), "auto" );
}

void DocxAttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    const char *pAdjustString;

    switch ( rAdjust.GetAdjust() )
    {
        case SVX_ADJUST_LEFT:
            pAdjustString = "left";
            break;
        case SVX_ADJUST_RIGHT:
            pAdjustString = "right";
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
        m_pSerializer->singleElementNS( XML_w, XML_keepLines, FSNS( XML_w, XML_val ), "off", FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_keepLines, FSEND );
}

void DocxAttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
    if (rWidows.GetValue())
        m_pSerializer->singleElementNS( XML_w, XML_widowControl, FSEND );
    else
        m_pSerializer->singleElementNS( XML_w, XML_widowControl, FSNS( XML_w, XML_val ), "off", FSEND );
}

static void impl_WriteTabElement( FSHelperPtr pSerializer,
        const SvxTabStop& rTab, long nCurrentLeft )
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

    pTabElementAttrList->add( FSNS( XML_w, XML_pos ), OString::valueOf( rTab.GetTabPos() + nCurrentLeft ) );

    sal_Unicode cFillChar = rTab.GetFill();

    if (sal_Unicode('.') == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "dot" ) );
    else if ( sal_Unicode('-') == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "hyphen" ) );
    else if ( sal_Unicode(0xB7) == cFillChar ) // middle dot
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "middleDot" ) );
    else if ( sal_Unicode('_') == cFillChar )
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "underscore" ) );
    else
        pTabElementAttrList->add( FSNS( XML_w, XML_leader ), OString( (sal_Char *) "none" ) );

    pSerializer->singleElementNS( XML_w, XML_tab, pTabElementAttrList );
}

void DocxAttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStop )
{
    const SfxPoolItem* pLR = m_rExport.HasItem( RES_LR_SPACE );
    long nCurrentLeft = pLR ? ((const SvxLRSpaceItem*)pLR)->GetTxtLeft() : 0;

    m_pSerializer->startElementNS( XML_w, XML_tabs, FSEND );

    sal_uInt16 nCount = rTabStop.Count();
    for (sal_uInt16 i = 0; i < nCount; i++ )
        impl_WriteTabElement( m_pSerializer, rTabStop[i], nCurrentLeft );

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
        m_pSerializer->singleElementNS( XML_w, XML_ilvl, FSNS( XML_w, XML_val ), OString::valueOf( sal_Int32( nLvl )).getStr(), FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_numId, FSNS( XML_w, XML_val ), OString::valueOf( sal_Int32( nNumId )).getStr(), FSEND );
        m_pSerializer->endElementNS( XML_w, XML_numPr );
    }
}

void DocxAttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    sal_uInt16 nXmlElement = 0;

    switch ( rScriptSpace.Which( ) )
    {
        case RES_PARATR_SCRIPTSPACE:
            nXmlElement = XML_autoSpaceDE;
            break;
        case RES_PARATR_HANGINGPUNCTUATION:
            nXmlElement = XML_overflowPunct;
            break;
        case RES_PARATR_FORBIDDEN_RULES:
            nXmlElement = XML_kinsoku;
            break;
    }

    if ( nXmlElement )
    {
        m_pSerializer->singleElementNS( XML_w, nXmlElement,
               FSNS( XML_w, XML_val ), rScriptSpace.GetValue( ) ? "true": "false",
               FSEND );
    }
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
    if ( m_rExport.bOutFlyFrmAttrs )
    {
 #if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatFrameSize() - Fly frames\n" );
 #endif
    }
    else if ( m_rExport.bOutPageDescs )
    {
        FastAttributeList *attrList = m_pSerializer->createAttrList( );
        if ( m_rExport.pAktPageDesc->GetLandscape( ) )
            attrList->add( FSNS( XML_w, XML_orient ), "landscape" );


        attrList->add( FSNS( XML_w, XML_w ), OString::valueOf( rSize.GetWidth( ) ) );
        attrList->add( FSNS( XML_w, XML_h ), OString::valueOf( rSize.GetHeight( ) ) );

        XFastAttributeListRef xAttrList( attrList );
        attrList = NULL;

        m_pSerializer->singleElementNS( XML_w, XML_pgSz, xAttrList );
    }
}

void DocxAttributeOutput::FormatPaperBin( const SvxPaperBinItem& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatPaperBin()\n" );
#endif
}

void DocxAttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLRSpace )
{
    if ( m_rExport.bOutFlyFrmAttrs )
    {
#if OSL_DEBUG_LEVEL > 0
        OSL_TRACE( "DocxAttributeOutput::FormatLRSpace() - Fly frames\n" );
#endif
    }
    else if ( m_rExport.bOutPageDescs )
    {
        if ( !m_pSpacingAttrList )
            m_pSpacingAttrList = m_pSerializer->createAttrList();


        sal_uInt16 nLDist, nRDist;
        const SfxPoolItem* pItem = m_rExport.HasItem( RES_BOX );
        if ( pItem )
        {
            nRDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_LEFT );
            nLDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_RIGHT );
        }
        else
            nLDist = nRDist = 0;
        nLDist = nLDist + (sal_uInt16)rLRSpace.GetLeft();
        nRDist = nRDist + (sal_uInt16)rLRSpace.GetRight();

        m_pSpacingAttrList->add( FSNS( XML_w, XML_left ), OString::valueOf( sal_Int32( nLDist ) ) );
        m_pSpacingAttrList->add( FSNS( XML_w, XML_right ), OString::valueOf( sal_Int32( nRDist ) ) );
    }
    else
    {
        FastAttributeList *pLRSpaceAttrList = m_pSerializer->createAttrList();

        pLRSpaceAttrList->add( FSNS( XML_w, XML_left ), OString::valueOf( (sal_Int32) rLRSpace.GetTxtLeft() ) );
        pLRSpaceAttrList->add( FSNS( XML_w, XML_right ), OString::valueOf( (sal_Int32) rLRSpace.GetRight() ) );

        sal_Int32 nFirstLineAdjustment = rLRSpace.GetTxtFirstLineOfst();
        if (nFirstLineAdjustment > 0)
            pLRSpaceAttrList->add( FSNS( XML_w, XML_firstLine ), OString::valueOf( nFirstLineAdjustment ) );
        else
            pLRSpaceAttrList->add( FSNS( XML_w, XML_hanging ), OString::valueOf( - nFirstLineAdjustment ) );
        m_pSerializer->singleElementNS( XML_w, XML_ind, pLRSpaceAttrList );
    }
}

void DocxAttributeOutput::FormatULSpace( const SvxULSpaceItem& rULSpace )
{
    if (!m_pSpacingAttrList)
        m_pSpacingAttrList = m_pSerializer->createAttrList();

    if ( m_rExport.bOutFlyFrmAttrs )
    {
    }
    else if (m_rExport.bOutPageDescs )
    {
        ASSERT( m_rExport.GetCurItemSet(), "Impossible" );
        if ( !m_rExport.GetCurItemSet() )
            return;

        HdFtDistanceGlue aDistances( *m_rExport.GetCurItemSet() );

        if ( aDistances.HasHeader() )
        {
            // Header top
            m_pSpacingAttrList->add( FSNS( XML_w, XML_header ),
                    OString::valueOf( sal_Int32( aDistances.dyaHdrTop ) ) );
        }

        // Page top
        m_pSpacingAttrList->add( FSNS( XML_w, XML_top ),
                OString::valueOf( sal_Int32( aDistances.dyaTop ) ) );

        if ( aDistances.HasFooter() )
        {
            // Footer bottom
            m_pSpacingAttrList->add( FSNS( XML_w, XML_footer ),
                    OString::valueOf( sal_Int32( aDistances.dyaHdrBottom ) ) );
        }

        // Page Bottom
        m_pSpacingAttrList->add( FSNS( XML_w, XML_bottom ),
                OString::valueOf( sal_Int32( aDistances.dyaBottom ) ) );

    }
    else
    {
        m_pSpacingAttrList->add( FSNS( XML_w, XML_before ),
                OString::valueOf( (sal_Int32)rULSpace.GetUpper() ) );
        m_pSpacingAttrList->add( FSNS( XML_w, XML_after ),
                OString::valueOf( (sal_Int32)rULSpace.GetLower() ) );
    }
}

void DocxAttributeOutput::FormatSurround( const SwFmtSurround& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatSurround()\n" );
#endif
}

void DocxAttributeOutput::FormatVertOrientation( const SwFmtVertOrient& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatVertOrientation()\n" );
#endif
}

void DocxAttributeOutput::FormatHorizOrientation( const SwFmtHoriOrient& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatHorizOrientation()\n" );
#endif
}

void DocxAttributeOutput::FormatAnchor( const SwFmtAnchor& )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatAnchor()\n" );
#endif
}

void DocxAttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    if ( !m_rExport.bOutPageDescs )
    {
        OString sColor = impl_ConvertColor( rBrush.GetColor( ) );
        m_pSerializer->singleElementNS( XML_w, XML_shd,
                FSNS( XML_w, XML_fill ), sColor.getStr( ),
                FSEND );
    }

#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "TODO DocxAttributeOutput::FormatBackground()\n" );
#endif
}

void DocxAttributeOutput::FormatBox( const SvxBoxItem& rBox )
{

    if ( !m_bOpenedSectPr )
    {
        // Normally open the borders tag for paragraphs
        m_pSerializer->startElementNS( XML_w, XML_pBdr, FSEND );
    }

    impl_pageBorders( m_pSerializer, rBox );

    if ( m_bOpenedSectPr )
    {
        // Special handling for pgBorder
        m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_PREPEND );
        m_pSerializer->mergeTopMarks( );
    }
    else
    {
        // Normally close the borders tag for paragraphs
        m_pSerializer->endElementNS( XML_w, XML_pBdr );
    }
}

void DocxAttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol& rCol, bool bEven, SwTwips nPageSize )
{
    // Get the columns attributes
    FastAttributeList *pColsAttrList = m_pSerializer->createAttrList();

    pColsAttrList->add( FSNS( XML_w, XML_num ),
            OString::valueOf( sal_Int32( nCols ) ). getStr( ) );

    const char* pEquals = "false";
    if ( bEven )
    {
        sal_uInt16 nWidth = rCol.GetGutterWidth( true );
        pColsAttrList->add( FSNS( XML_w, XML_space ),
               OString::valueOf( sal_Int32( nWidth ) ).getStr( ) );

        pEquals = "true";
    }

    pColsAttrList->add( FSNS( XML_w, XML_equalWidth ), pEquals );

    bool bHasSep = COLADJ_NONE == rCol.GetLineAdj( );
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
                    OString::valueOf( sal_Int32( nWidth ) ).getStr( ) );

            if ( n + 1 != nCols )
            {
                sal_uInt16 nSpacing = rColumns[n]->GetRight( ) + rColumns[n + 1]->GetLeft( );
                pColAttrList->add( FSNS( XML_w, XML_space ),
                    OString::valueOf( sal_Int32( nSpacing ) ).getStr( ) );
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

void DocxAttributeOutput::FormatTextGrid( const SwTextGridItem& )
{
    OSL_TRACE( "TODO DocxAttributeOutput::FormatTextGrid()\n" );
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
            m_pSerializer->singleElementNS( XML_w, XML_bidi, FSEND );
    }
}

DocxAttributeOutput::DocxAttributeOutput( DocxExport &rExport, FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML )
    : m_rExport( rExport ),
      m_pSerializer( pSerializer ),
      m_rDrawingML( *pDrawingML ),
      m_pFontsAttrList( NULL ),
      m_pEastAsianLayoutAttrList( NULL ),
      m_pCharLangAttrList( NULL ),
      m_pSpacingAttrList( NULL ),
      m_pHyperlinkAttrList( NULL ),
      m_pFootnotesList( new ::docx::FootnotesList() ),
      m_pEndnotesList( new ::docx::FootnotesList() ),
      m_pSectionInfo( NULL ),
      m_pRedlineData( NULL ),
      m_nRedlineId( 0 ),
      m_bOpenedSectPr( false ),
      m_sFieldBkm( ),
      m_nNextMarkId( 0 ),
      m_pTableWrt( NULL ),
      m_bTableCellOpen( false ),
      m_nTableDepth( 0 ),
      m_bParagraphOpened( false ),
      m_nColBreakStatus( COLBRK_NONE )
{
}

DocxAttributeOutput::~DocxAttributeOutput()
{
    delete m_pFontsAttrList, m_pFontsAttrList = NULL;
    delete m_pEastAsianLayoutAttrList, m_pEastAsianLayoutAttrList = NULL;
    delete m_pCharLangAttrList, m_pCharLangAttrList = NULL;
    delete m_pSpacingAttrList, m_pSpacingAttrList = NULL;
    delete m_pHyperlinkAttrList, m_pHyperlinkAttrList = NULL;

    delete m_pFootnotesList, m_pFootnotesList = NULL;
    delete m_pEndnotesList, m_pEndnotesList = NULL;

    delete m_pTableWrt, m_pTableWrt = NULL;
}

MSWordExportBase& DocxAttributeOutput::GetExport()
{
    return m_rExport;
}

bool DocxAttributeOutput::HasFootnotes()
{
    return !m_pFootnotesList->isEmpty();
}

bool DocxAttributeOutput::HasEndnotes()
{
    return !m_pEndnotesList->isEmpty();
}
