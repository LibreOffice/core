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

#include "docxexport.hxx"
#include "docxexportfilter.hxx"
#include "docxattributeoutput.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/export/chartexport.hxx>

#include <map>
#include <algorithm>

#include <IMark.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <wrtww8.hxx>
#include <fltini.hxx>
#include <fmtline.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <section.hxx>
#include <ftninfo.hxx>

#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>

#include <docary.hxx>
#include <numrule.hxx>
#include <charfmt.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>

#include "ww8par.hxx"
#include "ww8scan.hxx"

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/font.hxx>

using namespace sax_fastparser;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::oox;

using oox::vml::VMLExport;

using sw::mark::IMark;

AttributeOutputBase& DocxExport::AttrOutput() const
{
    return *m_pAttrOutput;
}

MSWordSections& DocxExport::Sections() const
{
    return *m_pSections;
}

bool DocxExport::CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich )
{
    // TODO FIXME is this actually true for docx? - this is ~copied from WW8
    if ( nScript == i18n::ScriptType::ASIAN )
    {
        // for asian in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch ( nWhich )
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                return false;
            default:
                break;
        }
    }
    else if ( nScript != i18n::ScriptType::COMPLEX )
    {
        // for western in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch ( nWhich )
        {
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
                return false;
            default:
                break;
        }
    }
    return true;
}

void DocxExport::AppendBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if ( GetBookmarks( rNode, nAktPos, nAktPos + nLen, aMarks ) )
    {
        for ( IMarkVector::const_iterator it = aMarks.begin(), end = aMarks.end();
              it != end; ++it )
        {
            IMark* pMark = (*it);

            xub_StrLen nStart = pMark->GetMarkStart().nContent.GetIndex();
            xub_StrLen nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart == nAktPos )
                aStarts.push_back( pMark->GetName() );

            if ( nEnd == nAktPos )
                aEnds.push_back( pMark->GetName() );
        }
    }

    m_pAttrOutput->WriteBookmarks_Impl( aStarts, aEnds );
}

void DocxExport::AppendBookmark( const OUString& rName, bool /*bSkip*/ )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    aStarts.push_back( rName );
    aEnds.push_back( rName );

    m_pAttrOutput->WriteBookmarks_Impl( aStarts, aEnds );
}

::rtl::OString DocxExport::AddRelation( const OUString& rType, const OUString& rTarget )
{
    OUString sId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
           rType, rTarget, true );

    return ::rtl::OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

bool DocxExport::DisallowInheritingOutlineNumbering( const SwFmt& rFmt )
{
    bool bRet( false );

    if (SFX_ITEM_SET != rFmt.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFmt *pParent = rFmt.DerivedFrom())
        {
            if (((const SwTxtFmtColl*)pParent)->IsAssignedToListLevelOfOutlineStyle())
            {
                ::sax_fastparser::FSHelperPtr pSerializer = m_pAttrOutput->GetSerializer( );
                // Level 9 disables the outline
                pSerializer->singleElementNS( XML_w, XML_outlineLvl,
                        FSNS( XML_w, XML_val ), "9" ,
                        FSEND );

                bRet = true;
            }
        }
    }

    return bRet;
}

void DocxExport::WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
        const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 /*nBreakCode*/ )
{
    // headers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_EVEN )
        WriteHeaderFooter( rLeftFmt, true, "even" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD )
        WriteHeaderFooter( rFmt, true, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_FIRST )
        WriteHeaderFooter( rFirstPageFmt, true, "first" );

    // footers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_EVEN )
        WriteHeaderFooter( rLeftFmt, false, "even" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD )
        WriteHeaderFooter( rFmt, false, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_FIRST )
        WriteHeaderFooter( rFirstPageFmt, false, "first" );

    if ( nHeadFootFlags & ( nsHdFtFlags::WW8_FOOTER_EVEN | nsHdFtFlags::WW8_HEADER_EVEN ))
        settings.evenAndOddHeaders = true;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "DocxExport::WriteHeadersFooters() - nBreakCode introduced, but ignored\n" );
#endif
}

void DocxExport::OutputField( const SwField* pFld, ww::eField eFldType, const String& rFldCmd, sal_uInt8 nMode )
{
    m_pAttrOutput->WriteField_Impl( pFld, eFldType, rFldCmd, nMode );
}

void DocxExport::WriteFormData( const ::sw::mark::IFieldmark& rFieldmark )
{
    m_pAttrOutput->WriteFormData_Impl( rFieldmark );
}

void DocxExport::WriteHyperlinkData( const ::sw::mark::IFieldmark& /*rFieldmark*/ )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "TODO DocxExport::WriteHyperlinkData()\n" );
#endif
}

void DocxExport::DoComboBox(const rtl::OUString& rName,
                             const rtl::OUString& rHelp,
                             const rtl::OUString& rToolTip,
                             const rtl::OUString& rSelected,
                             uno::Sequence<rtl::OUString>& rListItems)
{
    m_pDocumentFS->startElementNS( XML_w, XML_ffData, FSEND );

    m_pDocumentFS->singleElementNS( XML_w, XML_name,
            FSNS( XML_w, XML_val ), OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    m_pDocumentFS->singleElementNS( XML_w, XML_enabled, FSEND );

    if ( !rHelp.isEmpty() )
        m_pDocumentFS->singleElementNS( XML_w, XML_helpText,
            FSNS( XML_w, XML_val ), OUStringToOString( rHelp, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    if ( !rToolTip.isEmpty() )
        m_pDocumentFS->singleElementNS( XML_w, XML_statusText,
            FSNS( XML_w, XML_val ), OUStringToOString( rToolTip, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );

    m_pDocumentFS->startElementNS( XML_w, XML_ddList, FSEND );

    // Output the 0-based index of the selected value
    sal_uInt32 nListItems = rListItems.getLength();
    sal_Int32 nId = 0;
    sal_uInt32 nI = 0;
    while ( ( nI < nListItems ) && ( nId == 0 ) )
    {
        if ( rListItems[nI] == rSelected )
            nId = nI;
        nI++;
    }

    m_pDocumentFS->singleElementNS( XML_w, XML_result,
            FSNS( XML_w, XML_val ), rtl::OString::valueOf( nId ).getStr( ),
            FSEND );

    // Loop over the entries

    for (sal_uInt32 i = 0; i < nListItems; i++)
    {
        m_pDocumentFS->singleElementNS( XML_w, XML_listEntry,
                FSNS( XML_w, XML_val ), OUStringToOString( rListItems[i], RTL_TEXTENCODING_UTF8 ).getStr(),
               FSEND );
    }

    m_pDocumentFS->endElementNS( XML_w, XML_ddList );

    m_pDocumentFS->endElementNS( XML_w, XML_ffData );
}

void DocxExport::DoFormText(const SwInputField* /*pFld*/)
{
    OSL_TRACE( "TODO DocxExport::ForFormText()" );
}

rtl::OString DocxExport::OutputChart( uno::Reference< frame::XModel >& xModel, sal_Int32 nCount )
{
    rtl::OUString aFileName = rtl::OUStringBuffer().append("charts/chart").append(nCount).append(".xml").makeStringAndClear();

    OUString sId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart",
                    aFileName );

    aFileName = rtl::OUStringBuffer().append("word/charts/chart").append(nCount).append(".xml").makeStringAndClear();

    ::sax_fastparser::FSHelperPtr pChartFS =
        m_pFilter->openFragmentStreamWithSerializer( aFileName,
            "application/vnd.openxmlformats-officedocument.drawingml.chart" );

    oox::drawingml::ChartExport aChartExport( XML_w, pChartFS, xModel, m_pFilter, oox::drawingml::DrawingML::DOCUMENT_DOCX );
    aChartExport.ExportContent();
    return ::rtl::OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

void DocxExport::ExportDocument_Impl()
{
    InitStyles();

    // init sections
    m_pSections = new MSWordSections( *this );

    WriteMainText();

    WriteFootnotesEndnotes();

    WritePostitFields();

    WriteNumbering();

    WriteFonts();

    WriteSettings();

    delete pStyles, pStyles = NULL;
    delete m_pSections, m_pSections = NULL;
}

void DocxExport::OutputPageSectionBreaks( const SwTxtNode& )
{
    OSL_TRACE( "TODO DocxExport::OutputPageSectionBreaks( const SwTxtNode& )" );
}


void DocxExport::AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum )
{
    AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );
    m_pSections->AppendSection( pPageDesc, pFmt, nLnNum );
}

void DocxExport::OutputEndNode( const SwEndNode& rEndNode )
{
    MSWordExportBase::OutputEndNode( rEndNode );

    if ( TXT_MAINTEXT == nTxtTyp && rEndNode.StartOfSectionNode()->IsSectionNode() )
    {
        // this originally comes from WW8Export::WriteText(), and looks like it
        // could have some code common with SectionNode()...

        const SwSection& rSect = rEndNode.StartOfSectionNode()->GetSectionNode()->GetSection();
        if ( bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
            bStartTOX = false;

        SwNodeIndex aIdx( rEndNode, 1 );
        const SwNode& rNd = aIdx.GetNode();
        if ( rNd.IsEndNode() && rNd.StartOfSectionNode()->IsSectionNode() )
            return;

        bool isInTable = IsInTable();
        if ( !rNd.IsSectionNode() && isInTable ) // No sections in table
        {
            const SwSectionFmt* pParentFmt = rSect.GetFmt()->GetParent();
            if( !pParentFmt )
                pParentFmt = (SwSectionFmt*)0xFFFFFFFF;

            sal_uLong nRstLnNum;
            if( rNd.IsCntntNode() )
                nRstLnNum = const_cast< SwCntntNode* >( rNd.GetCntntNode() )->GetSwAttrSet().GetLineNumber().GetStartValue();
            else
                nRstLnNum = 0;

            AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo( ) );
            m_pSections->AppendSection( pAktPageDesc, pParentFmt, nRstLnNum );
        }
    }
}

void DocxExport::OutputTableNode( const SwTableNode& )
{
    OSL_TRACE( "TODO DocxExport::OutputTableNode( const SwTableNode& )" );
}

void DocxExport::OutputGrfNode( const SwGrfNode& )
{
    OSL_TRACE( "TODO DocxExport::OutputGrfNode( const SwGrfNode& )" );
}

void DocxExport::OutputOLENode( const SwOLENode& )
{
    OSL_TRACE( "TODO DocxExport::OutputOLENode( const SwOLENode& )" );
}

void DocxExport::OutputLinkedOLE( const OUString& )
{
    // Nothing to implement here: WW8 only
}

sal_uLong DocxExport::ReplaceCr( sal_uInt8 )
{
    // Completely unused for Docx export... only here for code sharing
    // purpose with binary export
    return 0;
}

void DocxExport::PrepareNewPageDesc( const SfxItemSet* pSet,
        const SwNode& rNd, const SwFmtPageDesc* pNewPgDescFmt,
        const SwPageDesc* pNewPgDesc )
{
    // tell the attribute output that we are ready to write the section
    // break [has to be output inside paragraph properties]
    AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );

    const SwSectionFmt* pFmt = GetSectionFormat( rNd );
    const sal_uLong nLnNm = GetSectionLineNo( pSet, rNd );

    OSL_ENSURE( pNewPgDescFmt || pNewPgDesc, "Neither page desc format nor page desc provided." );

    if ( pNewPgDescFmt )
    {
        m_pSections->AppendSection( *pNewPgDescFmt, rNd, pFmt, nLnNm );
    }
    else if ( pNewPgDesc )
    {
        m_pSections->AppendSection( pNewPgDesc, rNd, pFmt, nLnNm );
    }

}

void DocxExport::InitStyles()
{
    pStyles = new MSWordStyles( *this );

    // setup word/styles.xml and the relations + content type
    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles",
            "styles.xml" );

    ::sax_fastparser::FSHelperPtr pStylesFS =
        m_pFilter->openFragmentStreamWithSerializer( "word/styles.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml" );

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pStylesFS );

    // do the work
    pStyles->OutputStylesTable();

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
}

void DocxExport::WriteFootnotesEndnotes()
{
    if ( m_pAttrOutput->HasFootnotes() )
    {
        // setup word/styles.xml and the relations + content type
        m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes",
                "footnotes.xml" );

        ::sax_fastparser::FSHelperPtr pFootnotesFS =
            m_pFilter->openFragmentStreamWithSerializer( "word/footnotes.xml",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml" );

        // switch the serializer to redirect the output to word/footnotes.xml
        m_pAttrOutput->SetSerializer( pFootnotesFS );

        // do the work
        m_pAttrOutput->FootnotesEndnotes( true );

        // switch the serializer back
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
    }

    if ( m_pAttrOutput->HasEndnotes() )
    {
        // setup word/styles.xml and the relations + content type
        m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes",
                "endnotes.xml" );

        ::sax_fastparser::FSHelperPtr pEndnotesFS =
            m_pFilter->openFragmentStreamWithSerializer( "word/endnotes.xml",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml" );

        // switch the serializer to redirect the output to word/endnotes.xml
        m_pAttrOutput->SetSerializer( pEndnotesFS );

        // do the work
        m_pAttrOutput->FootnotesEndnotes( false );

        // switch the serializer back
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
    }
}

void DocxExport::WritePostitFields()
{
    if ( m_pAttrOutput->HasPostitFields() )
    {
        m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments",
                "comments.xml" );

        ::sax_fastparser::FSHelperPtr pPostitFS =
            m_pFilter->openFragmentStreamWithSerializer( "word/comments.xml",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml" );

        pPostitFS->startElementNS( XML_w, XML_comments, MainXmlNamespaces( pPostitFS ));
        m_pAttrOutput->SetSerializer( pPostitFS );
        m_pAttrOutput->WritePostitFields();
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
        pPostitFS->endElementNS( XML_w, XML_comments );
    }
}

void DocxExport::WriteNumbering()
{
    if ( !pUsedNumTbl )
        return; // no numbering is used

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering",
        "numbering.xml" );

    ::sax_fastparser::FSHelperPtr pNumberingFS = m_pFilter->openFragmentStreamWithSerializer( "word/numbering.xml",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml" );

    // switch the serializer to redirect the output to word/nubering.xml
    m_pAttrOutput->SetSerializer( pNumberingFS );

    pNumberingFS->startElementNS( XML_w, XML_numbering,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );

    AbstractNumberingDefinitions();

    NumberingDefinitions();

    pNumberingFS->endElementNS( XML_w, XML_numbering );

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
}

void DocxExport::WriteHeaderFooter( const SwFmt& rFmt, bool bHeader, const char* pType )
{
    // setup the xml stream
    OUString aRelId;
    ::sax_fastparser::FSHelperPtr pFS;
    if ( bHeader )
    {
        OUString aName( OUStringBuffer().append("header").append( ++m_nHeaders ).append(".xml").makeStringAndClear() );

        aRelId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header",
                aName );

        pFS = m_pFilter->openFragmentStreamWithSerializer( OUStringBuffer().append("word/").append( aName ).makeStringAndClear(),
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml" );

        pFS->startElementNS( XML_w, XML_hdr, MainXmlNamespaces( pFS ));
    }
    else
    {
        OUString aName( OUStringBuffer().append("footer").append( ++m_nFooters ).append(".xml").makeStringAndClear() );

        aRelId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer",
                aName );

        pFS = m_pFilter->openFragmentStreamWithSerializer( OUStringBuffer().append("word/").append( aName ).makeStringAndClear(),
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml" );

        pFS->startElementNS( XML_w, XML_ftr, MainXmlNamespaces( pFS ));
    }

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pFS );

    // do the work
    WriteHeaderFooterText( rFmt, bHeader );

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );

    // close the tag
    sal_Int32 nReference;
    if ( bHeader )
    {
        pFS->endElementNS( XML_w, XML_hdr );
        nReference = XML_headerReference;
    }
    else
    {
        pFS->endElementNS( XML_w, XML_ftr );
        nReference = XML_footerReference;
    }

    // and write the reference
    m_pDocumentFS->singleElementNS( XML_w, nReference,
            FSNS( XML_w, XML_type ), pType,
            FSNS( XML_r, XML_id ), rtl::OUStringToOString( aRelId, RTL_TEXTENCODING_UTF8 ).getStr(),
            FSEND );
}

void DocxExport::WriteFonts()
{
    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable",
            "fontTable.xml" );

    ::sax_fastparser::FSHelperPtr pFS = m_pFilter->openFragmentStreamWithSerializer(
            "word/fontTable.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml" );

    pFS->startElementNS( XML_w, XML_fonts,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pFS );

    // do the work
    maFontHelper.WriteFontTable( *m_pAttrOutput );

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );

    pFS->endElementNS( XML_w, XML_fonts );
}


void DocxExport::WriteProperties( )
{
    // Write the core properties
    SwDocShell* pDocShell( pDoc->GetDocShell( ) );
    uno::Reference<document::XDocumentProperties> xDocProps;
    if ( pDocShell )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
               pDocShell->GetModel( ), uno::UNO_QUERY );
        xDocProps = xDPS->getDocumentProperties();
    }

    m_pFilter->exportDocumentProperties( xDocProps );
}

void DocxExport::WriteSettings()
{
    ViewShell *pViewShell(pDoc->GetCurrentViewShell());
    if( !pViewShell && !settings.hasData() && !m_pAttrOutput->HasFootnotes() && !m_pAttrOutput->HasEndnotes())
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings",
            "settings.xml" );

    ::sax_fastparser::FSHelperPtr pFS = m_pFilter->openFragmentStreamWithSerializer(
            "word/settings.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml" );

    pFS->startElementNS( XML_w, XML_settings,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSEND );

    // Zoom
    rtl::OString aZoom(rtl::OString::valueOf(sal_Int32(pViewShell->GetViewOptions()->GetZoom())));
    pFS->singleElementNS(XML_w, XML_zoom, FSNS(XML_w, XML_percent), aZoom.getStr(), FSEND);

    if( settings.defaultTabStop != 0 )
        pFS->singleElementNS( XML_w, XML_defaultTabStop, FSNS( XML_w, XML_val ),
            rtl::OString::valueOf( sal_Int32( settings.defaultTabStop )).getStr(), FSEND );
    if( settings.evenAndOddHeaders )
        pFS->singleElementNS( XML_w, XML_evenAndOddHeaders, FSEND );

    if( m_pAttrOutput->HasFootnotes())
        m_pAttrOutput->WriteFootnoteEndnotePr( pFS, XML_footnotePr, pDoc->GetFtnInfo(), XML_footnote );
    if( m_pAttrOutput->HasEndnotes())
        m_pAttrOutput->WriteFootnoteEndnotePr( pFS, XML_endnotePr, pDoc->GetEndNoteInfo(), XML_endnote );

    pFS->endElementNS( XML_w, XML_settings );
}

VMLExport& DocxExport::VMLExporter()
{
    return *m_pVMLExport;
}

void DocxExport::WriteMainText()
{
    // setup the namespaces
    m_pDocumentFS->startElementNS( XML_w, XML_document, MainXmlNamespaces( m_pDocumentFS ));

    // body
    m_pDocumentFS->startElementNS( XML_w, XML_body, FSEND );

    pCurPam->GetPoint()->nNode = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();

    // the text
    WriteText();

    // the last section info
    const WW8_SepInfo *pSectionInfo = m_pSections? m_pSections->CurrentSectionInfo(): NULL;
    if ( pSectionInfo )
        SectionProperties( *pSectionInfo );

    // finish body and document
    m_pDocumentFS->endElementNS( XML_w, XML_body );
    m_pDocumentFS->endElementNS( XML_w, XML_document );
}

XFastAttributeListRef DocxExport::MainXmlNamespaces( FSHelperPtr serializer )
{
    FastAttributeList* pAttr = serializer->createAttrList();
    pAttr->add( FSNS( XML_xmlns, XML_o ), "urn:schemas-microsoft-com:office:office" );
    pAttr->add( FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships" );
    pAttr->add( FSNS( XML_xmlns, XML_v ), "urn:schemas-microsoft-com:vml" );
    pAttr->add( FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main" );
    pAttr->add( FSNS( XML_xmlns, XML_w10 ), "urn:schemas-microsoft-com:office:word" );
    pAttr->add( FSNS( XML_xmlns, XML_wp ), "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" );
    return XFastAttributeListRef( pAttr );
}

bool DocxExport::ignoreAttributeForStyles( sal_uInt16 nWhich ) const
{
    if( nWhich == RES_TEXTGRID )
        return true; // w:docGrid is written only to document.xml, not to styles.xml
    return MSWordExportBase::ignoreAttributeForStyles( nWhich );
}

void DocxExport::WriteOutliner(const OutlinerParaObject& rParaObj, sal_uInt8 nTyp)
{
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( *this, rEditObj, nTyp );

    sal_uInt16 nPara = rEditObj.GetParagraphCount();
    for( sal_uInt16 n = 0; n < nPara; ++n )
    {
        if( n )
            aAttrIter.NextPara( n );

        AttrOutput().StartParagraph( ww8::WW8TableNodeInfo::Pointer_t());
        rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();
        String aStr( rEditObj.GetText( n ));
        xub_StrLen nAktPos = 0;
        xub_StrLen nEnd = aStr.Len();
        do {
            AttrOutput().StartRun( NULL );
            xub_StrLen nNextAttr = aAttrIter.WhereNext();
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            if( nNextAttr > nEnd )
                nNextAttr = nEnd;

            bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
            if( !bTxtAtr )
            {
                if( nAktPos == 0 && nNextAttr - nAktPos == aStr.Len())
                    AttrOutput().RunText( aStr, eChrSet );
                else
                {
                    String tmp( aStr.Copy( nAktPos, nNextAttr - nAktPos ));
                    AttrOutput().RunText( tmp, eChrSet );
                }
            }
            AttrOutput().StartRunProperties();
            aAttrIter.OutAttr( nAktPos );
            AttrOutput().EndRunProperties( NULL );

            nAktPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();
            AttrOutput().EndRun();
        } while( nAktPos < nEnd );
//        aAttrIter.OutParaAttr(false);
        AttrOutput().EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t());
    }
}

DocxExport::DocxExport( DocxExportFilter *pFilter, SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      m_pFilter( pFilter ),
      m_pAttrOutput( NULL ),
      m_pSections( NULL ),
      m_nHeaders( 0 ),
      m_nFooters( 0 ),
      m_pVMLExport( NULL )
{
    // Write the document properies
    WriteProperties( );

    // relations for the document
    m_pFilter->addRelation( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
            "word/document.xml" );

    // the actual document
    m_pDocumentFS = m_pFilter->openFragmentStreamWithSerializer( "word/document.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" );

    // the DrawingML access
    m_pDrawingML = new oox::drawingml::DrawingML( m_pDocumentFS, m_pFilter, oox::drawingml::DrawingML::DOCUMENT_DOCX );

    // the attribute output for the document
    m_pAttrOutput = new DocxAttributeOutput( *this, m_pDocumentFS, m_pDrawingML );

    // the related VMLExport
    m_pVMLExport = new VMLExport( m_pDocumentFS, m_pAttrOutput );
}

DocxExport::~DocxExport()
{
    delete m_pVMLExport, m_pVMLExport = NULL;
    delete m_pAttrOutput, m_pAttrOutput = NULL;
    delete m_pDrawingML, m_pDrawingML = NULL;
}

DocxSettingsData::DocxSettingsData()
: evenAndOddHeaders( false )
, defaultTabStop( 0 )
{
}

bool DocxSettingsData::hasData() const
{
    if( evenAndOddHeaders )
        return true;
    if( defaultTabStop != 0 )
        return true;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
