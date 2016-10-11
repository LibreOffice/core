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
#include "docxsdrexport.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/export/shapes.hxx>

#include <map>
#include <algorithm>

#include <IMark.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <wrtww8.hxx>
#include <fltini.hxx>
#include <fmtline.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <section.hxx>
#include <ftninfo.hxx>
#include <pagedesc.hxx>

#include <editeng/unoprnms.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/hyphenzoneitem.hxx>

#include <docary.hxx>
#include <numrule.hxx>
#include <charfmt.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>

#include "ww8par.hxx"
#include "ww8scan.hxx"
#include <oox/token/properties.hxx>
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

DocxAttributeOutput& DocxExport::DocxAttrOutput() const
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

void DocxExport::AppendBookmarks( const SwTextNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen )
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

            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

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

void DocxExport::AppendAnnotationMarks( const SwTextNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if ( GetAnnotationMarks( rNode, nAktPos, nAktPos + nLen, aMarks ) )
    {
        for ( IMarkVector::const_iterator it = aMarks.begin(), end = aMarks.end();
              it != end; ++it )
        {
            IMark* pMark = (*it);

            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart == nAktPos )
                aStarts.push_back( pMark->GetName() );

            if ( nEnd == nAktPos )
                aEnds.push_back( pMark->GetName() );
        }
    }

    m_pAttrOutput->WriteAnnotationMarks_Impl( aStarts, aEnds );
}

void DocxExport::ExportGrfBullet(const SwTextNode&)
{
    // Just collect the bullets for now, numbering.xml is not yet started.
    CollectGrfsOfBullets();
}

OString DocxExport::AddRelation( const OUString& rType, const OUString& rTarget )
{
    OUString sId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
           rType, rTarget, true );

    return sId.toUtf8();
}

bool DocxExport::DisallowInheritingOutlineNumbering( const SwFormat& rFormat )
{
    bool bRet( false );

    if (SfxItemState::SET != rFormat.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFormat *pParent = rFormat.DerivedFrom())
        {
            if (static_cast<const SwTextFormatColl*>(pParent)->IsAssignedToListLevelOfOutlineStyle())
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
        const SwFrameFormat& rFormat, const SwFrameFormat& rLeftFormat, const SwFrameFormat& rFirstPageFormat, sal_uInt8 /*nBreakCode*/ )
{
    m_nHeadersFootersInSection = 1;
    // Turn ON flag for 'Writing Headers \ Footers'
    m_pAttrOutput->SetWritingHeaderFooter( true );

    // headers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_EVEN )
        WriteHeaderFooter( rLeftFormat, true, "even" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD )
        WriteHeaderFooter( rFormat, true, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_FIRST )
        WriteHeaderFooter( rFirstPageFormat, true, "first" );

    // footers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_EVEN )
        WriteHeaderFooter( rLeftFormat, false, "even" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD )
        WriteHeaderFooter( rFormat, false, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_FIRST )
        WriteHeaderFooter( rFirstPageFormat, false, "first" );

    if ( nHeadFootFlags & ( nsHdFtFlags::WW8_FOOTER_EVEN | nsHdFtFlags::WW8_HEADER_EVEN ))
        m_aSettings.evenAndOddHeaders = true;

    // Turn OFF flag for 'Writing Headers \ Footers'
    m_pAttrOutput->SetWritingHeaderFooter( false );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "DocxExport::WriteHeadersFooters() - nBreakCode introduced, but ignored\n" );
#endif
}

void DocxExport::OutputField( const SwField* pField, ww::eField eFieldType, const OUString& rFieldCmd, sal_uInt8 nMode )
{
    m_pAttrOutput->WriteField_Impl( pField, eFieldType, rFieldCmd, nMode );
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

void DocxExport::DoComboBox(const OUString& rName,
                             const OUString& rHelp,
                             const OUString& rToolTip,
                             const OUString& rSelected,
                             uno::Sequence<OUString>& rListItems)
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
            FSNS( XML_w, XML_val ), OString::number( nId ).getStr( ),
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

void DocxExport::DoFormText(const SwInputField* /*pField*/)
{
    OSL_TRACE( "TODO DocxExport::ForFormText()" );
}

OString DocxExport::OutputChart( uno::Reference< frame::XModel >& xModel, sal_Int32 nCount, ::sax_fastparser::FSHelperPtr m_pSerializer )
{
    OUString aFileName = "charts/chart" + OUString::number(nCount) + ".xml";
    OUString sId = m_pFilter->addRelation( m_pSerializer->getOutputStream(),
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart",
                    aFileName );
    aFileName = "word/charts/chart" + OUString::number(nCount) + ".xml";
    ::sax_fastparser::FSHelperPtr pChartFS =
        m_pFilter->openFragmentStreamWithSerializer( aFileName,
            "application/vnd.openxmlformats-officedocument.drawingml.chart+xml" );

    oox::drawingml::ChartExport aChartExport(XML_w, pChartFS, xModel, m_pFilter, oox::drawingml::DOCUMENT_DOCX);
    aChartExport.ExportContent();
    return OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

OString DocxExport::WriteOLEObject(SwOLEObj& rObject, OUString & io_rProgID)
{
    uno::Reference <embed::XEmbeddedObject> xObj( rObject.GetOleRef() );
    uno::Reference<uno::XComponentContext> const xContext(
        GetFilter().getComponentContext());

    OUString sMediaType;
    OUString sRelationType;
    OUString sSuffix;
    const char * pProgID(nullptr);

    uno::Reference<io::XInputStream> const xInStream =
        oox::GetOLEObjectStream(xContext, xObj, io_rProgID,
            sMediaType, sRelationType, sSuffix, pProgID);

    if (!xInStream.is())
    {
        return OString();
    }

    assert(!sMediaType.isEmpty());
    assert(!sRelationType.isEmpty());
    assert(!sSuffix.isEmpty());
    OUString sFileName = "embeddings/oleObject" + OUString::number( ++m_nOLEObjects ) + "." + sSuffix;
    uno::Reference<io::XOutputStream> const xOutStream =
        GetFilter().openFragmentStream("word/" + sFileName, sMediaType);
    assert(xOutStream.is()); // no reason why that could fail

    bool const isExported = lcl_CopyStream(xInStream, xOutStream);

    OUString sId;
    if (isExported)
    {
        sId = m_pFilter->addRelation( GetFS()->getOutputStream(),
                sRelationType, sFileName );
        if (pProgID)
        {
            io_rProgID = OUString::createFromAscii(pProgID);
        }
    }

    return OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

// function copied from embeddedobj/source/msole/oleembed.cxx
bool DocxExport::lcl_CopyStream( uno::Reference<io::XInputStream> xIn, uno::Reference<io::XOutputStream> xOut )
{
    if( !xIn.is() || !xOut.is() )
        return false;

    const sal_Int32 nChunkSize = 4096;
    uno::Sequence< sal_Int8 > aData(nChunkSize);
    sal_Int32 nTotalRead = 0;
    sal_Int32 nRead = 0;
    do
    {
        nRead = xIn->readBytes(aData, nChunkSize);
        nTotalRead += nRead;
        xOut->writeBytes(aData);
    } while (nRead == nChunkSize);
    return nTotalRead != 0;
}

void DocxExport::OutputDML(uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    sal_Int32 nNamespace = XML_wps;
    if (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        nNamespace = XML_wpg;
    else if (xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        nNamespace = XML_pic;
    oox::drawingml::ShapeExport aExport(nNamespace, m_pAttrOutput->GetSerializer(), nullptr, m_pFilter, oox::drawingml::DOCUMENT_DOCX, m_pAttrOutput);
    aExport.WriteShape(xShape);
}

void DocxExport::ExportDocument_Impl()
{
    // Set the 'Track Revisions' flag in the settings structure
    m_aSettings.trackRevisions = 0 != ( nsRedlineMode_t::REDLINE_ON & m_nOrigRedlineMode );

    InitStyles();

    // init sections
    m_pSections = new MSWordSections( *this );

    // Make sure images are counted from one, even when exporting multiple documents.
    oox::drawingml::DrawingML::ResetCounters();

    WriteMainText();

    WriteFootnotesEndnotes();

    WritePostitFields();

    WriteNumbering();

    WriteFonts();

    WriteSettings();

    WriteTheme();

    WriteGlossary();

    WriteCustomXml();

    WriteActiveX();

    WriteEmbeddings();

    m_aLinkedTextboxesHelper.clear();   //final cleanup
    delete m_pStyles;
    m_pStyles = nullptr;
    delete m_pSections;
    m_pSections = nullptr;
}

void DocxExport::AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum )
{
    AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );
    m_pSections->AppendSection( pPageDesc, pFormat, nLnNum, m_pAttrOutput->IsFirstParagraph() );
}

void DocxExport::OutputEndNode( const SwEndNode& rEndNode )
{
    MSWordExportBase::OutputEndNode( rEndNode );

    if ( TXT_MAINTEXT == m_nTextTyp && rEndNode.StartOfSectionNode()->IsSectionNode() )
    {
        // this originally comes from WW8Export::WriteText(), and looks like it
        // could have some code common with SectionNode()...

        const SwSection& rSect = rEndNode.StartOfSectionNode()->GetSectionNode()->GetSection();
        if ( m_bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
            m_bStartTOX = false;

        SwNodeIndex aIdx( rEndNode, 1 );
        const SwNode& rNd = aIdx.GetNode();
        if ( rNd.IsEndNode() && rNd.StartOfSectionNode()->IsSectionNode() )
            return;

        bool isInTable = IsInTable();
        if ( !rNd.IsSectionNode() && isInTable ) // No sections in table
        {
            const SwSectionFormat* pParentFormat = rSect.GetFormat()->GetParent();
            if( !pParentFormat )
                pParentFormat = reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1));

            sal_uLong nRstLnNum;
            if( rNd.IsContentNode() )
                nRstLnNum = rNd.GetContentNode()->GetSwAttrSet().GetLineNumber().GetStartValue();
            else
                nRstLnNum = 0;

            AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo( ) );
            m_pSections->AppendSection( m_pAktPageDesc, pParentFormat, nRstLnNum );
        }
    }
    else if (TXT_MAINTEXT == m_nTextTyp && rEndNode.StartOfSectionNode()->IsTableNode())
        // End node of a table: see if a section break should be written after the table.
        AttrOutput().SectionBreaks(rEndNode);
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
        const SwNode& rNd, const SwFormatPageDesc* pNewPgDescFormat,
        const SwPageDesc* pNewPgDesc )
{
    // tell the attribute output that we are ready to write the section
    // break [has to be output inside paragraph properties]
    AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );

    const SwSectionFormat* pFormat = GetSectionFormat( rNd );
    const sal_uLong nLnNm = GetSectionLineNo( pSet, rNd );

    OSL_ENSURE( pNewPgDescFormat || pNewPgDesc, "Neither page desc format nor page desc provided." );

    if ( pNewPgDescFormat )
    {
        m_pSections->AppendSection( *pNewPgDescFormat, rNd, pFormat, nLnNm );
    }
    else if ( pNewPgDesc )
    {
        m_pSections->AppendSection( pNewPgDesc, rNd, pFormat, nLnNm );
    }

}

void DocxExport::InitStyles()
{
    m_pStyles = new MSWordStyles( *this, /*bListStyles =*/ true );

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
    m_pStyles->OutputStylesTable();

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

        pPostitFS->startElementNS( XML_w, XML_comments, MainXmlNamespaces());
        m_pAttrOutput->SetSerializer( pPostitFS );
        m_pAttrOutput->WritePostitFields();
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
        pPostitFS->endElementNS( XML_w, XML_comments );
    }
}

void DocxExport::WriteNumbering()
{
    if ( !m_pUsedNumTable )
        return; // no numbering is used

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering",
        "numbering.xml" );

    ::sax_fastparser::FSHelperPtr pNumberingFS = m_pFilter->openFragmentStreamWithSerializer( "word/numbering.xml",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml" );

    // switch the serializer to redirect the output to word/numbering.xml
    m_pAttrOutput->SetSerializer( pNumberingFS );

    pNumberingFS->startElementNS( XML_w, XML_numbering,
            FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
            FSNS( XML_xmlns, XML_o ), "urn:schemas-microsoft-com:office:office",
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSNS( XML_xmlns, XML_v ), "urn:schemas-microsoft-com:vml",
            FSEND );

    BulletDefinitions();

    AbstractNumberingDefinitions();

    NumberingDefinitions();

    pNumberingFS->endElementNS( XML_w, XML_numbering );

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
}

void DocxExport::WriteHeaderFooter( const SwFormat& rFormat, bool bHeader, const char* pType )
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

        pFS->startElementNS( XML_w, XML_hdr, MainXmlNamespaces());
    }
    else
    {
        OUString aName( OUStringBuffer().append("footer").append( ++m_nFooters ).append(".xml").makeStringAndClear() );

        aRelId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer",
                aName );

        pFS = m_pFilter->openFragmentStreamWithSerializer( OUStringBuffer().append("word/").append( aName ).makeStringAndClear(),
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml" );

        pFS->startElementNS( XML_w, XML_ftr, MainXmlNamespaces());
    }

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pFS );
    m_pVMLExport->SetFS( pFS );
    m_pSdrExport->setSerializer(pFS);
    SetFS( pFS );
    bool bStartedParaSdt = m_pAttrOutput->IsStartedParaSdt();
    m_pAttrOutput->SetStartedParaSdt(false);

    DocxTableExportContext aTableExportContext;
    m_pAttrOutput->pushToTableExportContext(aTableExportContext);
    //When the stream changes the cache which is maintained for the graphics in case of alternate content is not cleared.
    //So clearing the alternate content graphic cache.
    m_pAttrOutput->PushRelIdCache();
    // do the work
    WriteHeaderFooterText( rFormat, bHeader );
    m_pAttrOutput->PopRelIdCache();
    m_pAttrOutput->popFromTableExportContext(aTableExportContext);
    m_pAttrOutput->EndParaSdtBlock();

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
    m_pVMLExport->SetFS( m_pDocumentFS );
    m_pSdrExport->setSerializer(m_pDocumentFS);
    SetFS( m_pDocumentFS );
    m_pAttrOutput->SetStartedParaSdt(bStartedParaSdt);

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
            FSNS( XML_r, XML_id ), aRelId.toUtf8().getStr(),
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
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSEND );

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pFS );

    // do the work
    m_aFontHelper.WriteFontTable( *m_pAttrOutput );

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );

    pFS->endElementNS( XML_w, XML_fonts );
}

void DocxExport::WriteProperties( )
{
    // Write the core properties
    SwDocShell* pDocShell( m_pDoc->GetDocShell( ) );
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
    SwViewShell *pViewShell(m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell());
    if( !pViewShell && !m_aSettings.hasData() && !m_pAttrOutput->HasFootnotes() && !m_pAttrOutput->HasEndnotes())
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
    if (pViewShell)
    {
        OString aZoom(OString::number(pViewShell->GetViewOptions()->GetZoom()));
        pFS->singleElementNS(XML_w, XML_zoom, FSNS(XML_w, XML_percent), aZoom.getStr(), FSEND);
    }

    // Display Background Shape
    if (boost::optional<SvxBrushItem> oBrush = getBackground())
    {
        // Turn on the 'displayBackgroundShape'
        pFS->singleElementNS( XML_w, XML_displayBackgroundShape, FSEND );
    }

    // Track Changes
    if ( m_aSettings.trackRevisions )
        pFS->singleElementNS( XML_w, XML_trackRevisions, FSEND );

    // Mirror Margins
    if(isMirroredMargin())
        pFS->singleElementNS( XML_w, XML_mirrorMargins, FSEND );

    // Embed Fonts
    if( m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        pFS->singleElementNS( XML_w, XML_embedTrueTypeFonts, FSEND );

    // Embed System Fonts
    if( m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_SYSTEM_FONTS ))
        pFS->singleElementNS( XML_w, XML_embedSystemFonts, FSEND );

    // Default Tab Stop
    if( m_aSettings.defaultTabStop != 0 )
        pFS->singleElementNS( XML_w, XML_defaultTabStop, FSNS( XML_w, XML_val ),
            OString::number( m_aSettings.defaultTabStop).getStr(), FSEND );

    // Protect form
    if( m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ))
    {
        pFS->singleElementNS( XML_w, XML_documentProtection, FSNS(XML_w, XML_edit), "forms", FSNS(XML_w, XML_enforcement), "1",  FSEND );
    }

    // Automatic hyphenation: it's a global setting in Word, it's a paragraph setting in Writer.
    // Use the setting from the default style.
    SwTextFormatColl* pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, /*bRegardLanguage=*/false);
    const SfxPoolItem* pItem;
    if (pColl && SfxItemState::SET == pColl->GetItemState(RES_PARATR_HYPHENZONE, false, &pItem))
    {
        pFS->singleElementNS(XML_w, XML_autoHyphenation,
                             FSNS(XML_w, XML_val), OString::boolean(static_cast<const SvxHyphenZoneItem*>(pItem)->IsHyphen()),
                             FSEND);
    }

    // Even and Odd Headers
    if( m_aSettings.evenAndOddHeaders )
        pFS->singleElementNS( XML_w, XML_evenAndOddHeaders, FSEND );

    // Has Footnotes
    if( m_pAttrOutput->HasFootnotes())
        DocxAttributeOutput::WriteFootnoteEndnotePr( pFS, XML_footnotePr, m_pDoc->GetFootnoteInfo(), XML_footnote );

    // Has Endnotes
    if( m_pAttrOutput->HasEndnotes())
        DocxAttributeOutput::WriteFootnoteEndnotePr( pFS, XML_endnotePr, m_pDoc->GetEndNoteInfo(), XML_endnote );

    // Has themeFontLang information
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aGrabBagName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( xPropSetInfo->hasPropertyByName( aGrabBagName ) )
    {
        uno::Sequence< beans::PropertyValue > propList;
        xPropSet->getPropertyValue( aGrabBagName ) >>= propList;
        for( sal_Int32 i=0; i < propList.getLength(); ++i )
        {
            if ( propList[i].Name == "ThemeFontLangProps" )
            {
                uno::Sequence< beans::PropertyValue > themeFontLangProps;
                propList[i].Value >>= themeFontLangProps;
                OUString aValues[3];
                for( sal_Int32 j=0; j < themeFontLangProps.getLength(); ++j )
                {
                    if( themeFontLangProps[j].Name == "val" )
                        themeFontLangProps[j].Value >>= aValues[0];
                    else if( themeFontLangProps[j].Name == "eastAsia" )
                        themeFontLangProps[j].Value >>= aValues[1];
                    else if( themeFontLangProps[j].Name == "bidi" )
                        themeFontLangProps[j].Value >>= aValues[2];
                }
                pFS->singleElementNS( XML_w, XML_themeFontLang,
                                      FSNS( XML_w, XML_val ), OUStringToOString( aValues[0], RTL_TEXTENCODING_UTF8 ).getStr(),
                                      FSNS( XML_w, XML_eastAsia ), OUStringToOString( aValues[1], RTL_TEXTENCODING_UTF8 ).getStr(),
                                      FSNS( XML_w, XML_bidi ), OUStringToOString( aValues[2], RTL_TEXTENCODING_UTF8 ).getStr(),
                                      FSEND );
            }
            else if ( propList[i].Name == "CompatSettings" )
            {
                pFS->startElementNS( XML_w, XML_compat, FSEND );

                uno::Sequence< beans::PropertyValue > aCompatSettingsSequence;
                propList[i].Value >>= aCompatSettingsSequence;
                for(sal_Int32 j=0; j < aCompatSettingsSequence.getLength(); ++j)
                {
                    uno::Sequence< beans::PropertyValue > aCompatSetting;
                    aCompatSettingsSequence[j].Value >>= aCompatSetting;
                    OUString aName;
                    OUString aUri;
                    OUString aValue;

                    for(sal_Int32 k=0; k < aCompatSetting.getLength(); ++k)
                    {
                        if( aCompatSetting[k].Name == "name" )
                            aCompatSetting[k].Value >>= aName;
                        else if( aCompatSetting[k].Name == "uri" )
                            aCompatSetting[k].Value >>= aUri;
                        else if( aCompatSetting[k].Name == "val" )
                            aCompatSetting[k].Value >>= aValue;
                    }
                    pFS->singleElementNS( XML_w, XML_compatSetting,
                        FSNS( XML_w, XML_name ), OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr(),
                        FSNS( XML_w, XML_uri ),  OUStringToOString(aUri, RTL_TEXTENCODING_UTF8).getStr(),
                        FSNS( XML_w, XML_val ),  OUStringToOString(aValue, RTL_TEXTENCODING_UTF8).getStr(),
                        FSEND);
                }

                pFS->endElementNS( XML_w, XML_compat );
            }
        }
    }

    // Section-specific write protection
    if ( m_pSections->DocumentIsProtected() )
    {
        pFS->singleElementNS( XML_w, XML_documentProtection,
                              FSNS( XML_w, XML_enforcement ), "true",
                              FSNS( XML_w, XML_edit ), "forms",
                              FSEND );
    }

    pFS->endElementNS( XML_w, XML_settings );
}

void DocxExport::WriteTheme()
{
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Reference<xml::dom::XDocument> themeDom;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXTheme" )
        {
             propList[nProp].Value >>= themeDom;
             break;
        }
    }

    // no theme dom to write
    if ( !themeDom.is() )
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme",
            "theme/theme1.xml" );

    uno::Reference< xml::sax::XSAXSerializable > serializer( themeDom, uno::UNO_QUERY );
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
    writer->setOutputStream( GetFilter().openFragmentStream( "word/theme/theme1.xml",
        "application/vnd.openxmlformats-officedocument.theme+xml" ) );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );
}

void DocxExport::WriteGlossary()
{
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Reference<xml::dom::XDocument> glossaryDocDom;
    uno::Sequence< uno::Sequence< uno::Any> > glossaryDomList;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    sal_Int32 collectedProperties = 0;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXGlossary" )
        {
             propList[nProp].Value >>= glossaryDocDom;
             collectedProperties++;
        }
        if (propName == "OOXGlossaryDom")
        {
            propList[nProp].Value >>= glossaryDomList;
            collectedProperties++;
        }
        if (collectedProperties == 2)
            break;
    }

    // no glossary dom to write
    if ( !glossaryDocDom.is() )
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument",
            "glossary/document.xml" );

    uno::Reference< io::XOutputStream > xOutputStream = GetFilter().openFragmentStream( "word/glossary/document.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document.glossary+xml" );

    uno::Reference< xml::sax::XSAXSerializable > serializer( glossaryDocDom, uno::UNO_QUERY );
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
    writer->setOutputStream( xOutputStream );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    sal_Int32 length = glossaryDomList.getLength();
    for ( int i =0; i < length; i++)
    {
        uno::Sequence< uno::Any> glossaryElement = glossaryDomList[i];
        OUString gTarget, gType, gId, contentType;
        uno::Reference<xml::dom::XDocument> xDom;
        glossaryElement[0] >>= xDom;
        glossaryElement[1] >>= gId;
        glossaryElement[2] >>= gType;
        glossaryElement[3] >>= gTarget;
        glossaryElement[4] >>= contentType;
        gId = gId.copy(3); //"rId" only save the numeric value

        PropertySet aProps(xOutputStream);
        aProps.setAnyProperty( PROP_RelId, uno::makeAny( sal_Int32( gId.toInt32() )));
        m_pFilter->addRelation( xOutputStream, gType, gTarget);
        uno::Reference< xml::sax::XSAXSerializable > gserializer( xDom, uno::UNO_QUERY );
        writer->setOutputStream(GetFilter().openFragmentStream( "word/glossary/" + gTarget, contentType ) );
        gserializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
               uno::Sequence< beans::StringPair >() );
    }
}

void DocxExport::WriteCustomXml()
{
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Sequence<uno::Reference<xml::dom::XDocument> > customXmlDomlist;
    uno::Sequence<uno::Reference<xml::dom::XDocument> > customXmlDomPropslist;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXCustomXml" )
        {
             propList[nProp].Value >>= customXmlDomlist;
             break;
        }
    }

    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXCustomXmlProps" )
        {
             propList[nProp].Value >>= customXmlDomPropslist;
             break;
        }
    }

    for (sal_Int32 j = 0; j < customXmlDomlist.getLength(); j++) {

        uno::Reference<xml::dom::XDocument> customXmlDom = customXmlDomlist[j];
        uno::Reference<xml::dom::XDocument> customXmlDomProps = customXmlDomPropslist[j];
        if ( customXmlDom.is() )
        {
            m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml",
                    "../customXml/item"+OUString::number((j+1))+".xml" );

            uno::Reference< xml::sax::XSAXSerializable > serializer( customXmlDom, uno::UNO_QUERY );
            uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
            writer->setOutputStream( GetFilter().openFragmentStream( "customXml/item"+OUString::number((j+1))+".xml",
                "application/xml" ) );
            serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
                uno::Sequence< beans::StringPair >() );
        }

        if ( customXmlDomProps.is() )
        {

            uno::Reference< xml::sax::XSAXSerializable > serializer( customXmlDomProps, uno::UNO_QUERY );
            uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
            writer->setOutputStream( GetFilter().openFragmentStream( "customXml/itemProps"+OUString::number((j+1))+".xml",
                "application/vnd.openxmlformats-officedocument.customXmlProperties+xml" ) );
            serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
                uno::Sequence< beans::StringPair >() );

            // Adding itemprops's relationship entry to item.xml.rels file
            m_pFilter->addRelation( GetFilter().openFragmentStream( "customXml/item"+OUString::number((j+1))+".xml",
                    "application/xml" ) ,
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXmlProps",
                    "itemProps"+OUString::number((j+1))+".xml" );

        }
    }
}

void DocxExport::WriteActiveX()
{
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Sequence<uno::Reference<xml::dom::XDocument> > activeXDomlist;
    uno::Sequence<uno::Reference<io::XInputStream> > activeXBinList;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXActiveX" )
        {
             propList[nProp].Value >>= activeXDomlist;
             break;
        }
    }

    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXActiveXBin" )
        {
            propList[nProp].Value >>= activeXBinList;
            break;
        }
    }

    for (sal_Int32 j = 0; j < activeXDomlist.getLength(); j++)
    {
        uno::Reference<xml::dom::XDocument> activeXDom = activeXDomlist[j];
        uno::Reference<io::XInputStream> activeXBin = activeXBinList[j];

        if ( activeXDom.is() )
        {
            m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/control",
                    "activeX/activeX"+OUString::number((j+1))+".xml" );

            uno::Reference< xml::sax::XSAXSerializable > serializer( activeXDom, uno::UNO_QUERY );
            uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
            writer->setOutputStream( GetFilter().openFragmentStream( "word/activeX/activeX"+OUString::number((j+1))+".xml",
                "application/vnd.ms-office.activeX+xml" ) );
            serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
                uno::Sequence< beans::StringPair >() );
        }

        if ( activeXBin.is() )
        {
            uno::Reference< io::XOutputStream > xOutStream = GetFilter().openFragmentStream("word/activeX/activeX"+OUString::number((j+1))+".bin",
                    "application/vnd.ms-office.activeX");

            try
            {
                sal_Int32 nBufferSize = 512;
                uno::Sequence< sal_Int8 > aDataBuffer(nBufferSize);
                sal_Int32 nRead;
                do
                {
                    nRead = activeXBin->readBytes( aDataBuffer, nBufferSize );
                    if( nRead )
                    {
                        if( nRead < nBufferSize )
                        {
                            nBufferSize = nRead;
                            aDataBuffer.realloc(nRead);
                        }
                        xOutStream->writeBytes( aDataBuffer );
                    }
                }
                while( nRead );
                xOutStream->flush();
            }
            catch(const uno::Exception&)
            {
                SAL_WARN("sw.ww8", "WriteActiveX() ::Failed to copy Inputstream to outputstream exception catched!");
            }

            xOutStream->closeOutput();
            // Adding itemprops's relationship entry to item.xml.rels file
            m_pFilter->addRelation( GetFilter().openFragmentStream( "/word/activeX/activeX"+OUString::number((j+1))+".xml",
                    "application/vnd.ms-office.activeX+xml" ) ,
                    "http://schemas.microsoft.com/office/2006/relationships/activeXControlBinary",
                    "activeX"+OUString::number((j+1))+".bin" );

        }
     }
}

void DocxExport::WriteEmbeddings()
{
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Sequence< beans::PropertyValue > embeddingsList;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    for ( sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp )
    {
        OUString propName = propList[nProp].Name;
        if ( propName == "OOXEmbeddings" )
        {
             propList[nProp].Value >>= embeddingsList;
             break;
        }
    }
    for (sal_Int32 j = 0; j < embeddingsList.getLength(); j++)
    {
        OUString embeddingPath = embeddingsList[j].Name;
        uno::Reference<io::XInputStream> embeddingsStream;
        embeddingsList[j].Value >>= embeddingsStream;

        OUString contentType = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        // FIXME: this .xlsm hack is silly - if anything the mime-type for an existing embedded object shoud be read from [Content_Types].xml
        if (embeddingPath.endsWith(".xlsm"))
            contentType = "application/vnd.ms-excel.sheet.macroEnabled.12";
        else if (embeddingPath.endsWith(".bin"))
            contentType = "application/vnd.openxmlformats-officedocument.oleObject";

        if ( embeddingsStream.is() )
        {
            uno::Reference< io::XOutputStream > xOutStream = GetFilter().openFragmentStream(embeddingPath,
                                    contentType);
            try
            {
                sal_Int32 nBufferSize = 512;
                uno::Sequence< sal_Int8 > aDataBuffer(nBufferSize);
                sal_Int32 nRead;
                do
                {
                    nRead = embeddingsStream->readBytes( aDataBuffer, nBufferSize );
                    if( nRead )
                    {
                        if( nRead < nBufferSize )
                        {
                            nBufferSize = nRead;
                            aDataBuffer.realloc(nRead);
                        }
                        xOutStream->writeBytes( aDataBuffer );
                    }
                }
                while( nRead );
                xOutStream->flush();
            }
            catch(const uno::Exception&)
            {
                SAL_WARN("sw.ww8", "WriteEmbeddings() ::Failed to copy Inputstream to outputstream exception catched!");
            }
            xOutStream->closeOutput();
        }
    }
}

bool DocxExport::isMirroredMargin()
{
    bool bMirroredMargins = false;
    if ( nsUseOnPage::PD_MIRROR == (nsUseOnPage::PD_MIRROR & m_pDoc->GetPageDesc(0).ReadUseOn()) )
    {
        bMirroredMargins = true;
    }
    return bMirroredMargins;
}

void DocxExport::WriteMainText()
{
    // setup the namespaces
    m_pDocumentFS->startElementNS( XML_w, XML_document, MainXmlNamespaces());

    // reset the incrementing linked-textboxes chain ID before re-saving.
    m_nLinkedTextboxesChainId=0;
    m_aLinkedTextboxesHelper.clear();

    // Write background page color
    if (boost::optional<SvxBrushItem> oBrush = getBackground())
    {
        Color backgroundColor = oBrush->GetColor();
        OString aBackgroundColorStr = msfilter::util::ConvertColor(backgroundColor);

        m_pDocumentFS->singleElementNS( XML_w, XML_background, FSNS( XML_w, XML_color ), aBackgroundColorStr, FSEND );
    }

    // body
    m_pDocumentFS->startElementNS( XML_w, XML_body, FSEND );

    m_pCurPam->GetPoint()->nNode = m_pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();

    // the text
    WriteText();

    // clear linked textboxes since old ones can't be linked to frames in a different section (correct?)
    m_aLinkedTextboxesHelper.clear();

    // the last section info
    m_pAttrOutput->EndParaSdtBlock();
    const WW8_SepInfo *pSectionInfo = m_pSections? m_pSections->CurrentSectionInfo(): nullptr;
    if ( pSectionInfo )
        SectionProperties( *pSectionInfo );

    // finish body and document
    m_pDocumentFS->endElementNS( XML_w, XML_body );
    m_pDocumentFS->endElementNS( XML_w, XML_document );
}

XFastAttributeListRef DocxExport::MainXmlNamespaces()
{
    FastAttributeList* pAttr = FastSerializerHelper::createAttrList();
    pAttr->add( FSNS( XML_xmlns, XML_o ), "urn:schemas-microsoft-com:office:office" );
    pAttr->add( FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships" );
    pAttr->add( FSNS( XML_xmlns, XML_v ), "urn:schemas-microsoft-com:vml" );
    pAttr->add( FSNS( XML_xmlns, XML_w ), "http://schemas.openxmlformats.org/wordprocessingml/2006/main" );
    pAttr->add( FSNS( XML_xmlns, XML_w10 ), "urn:schemas-microsoft-com:office:word" );
    pAttr->add( FSNS( XML_xmlns, XML_wp ), "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" );
    pAttr->add( FSNS( XML_xmlns, XML_wps ), "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" );
    pAttr->add( FSNS( XML_xmlns, XML_wpg ), "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" );
    pAttr->add( FSNS( XML_xmlns, XML_mc ), "http://schemas.openxmlformats.org/markup-compatibility/2006" );
    pAttr->add( FSNS( XML_xmlns, XML_wp14 ), "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" );
    pAttr->add( FSNS( XML_xmlns, XML_w14 ), "http://schemas.microsoft.com/office/word/2010/wordml" );
    pAttr->add( FSNS( XML_mc, XML_Ignorable ), "w14 wp14" );
    return XFastAttributeListRef( pAttr );
}

bool DocxExport::ignoreAttributeForStyleDefaults( sal_uInt16 nWhich ) const
{
    if( nWhich == RES_TEXTGRID )
        return true; // w:docGrid is written only to document.xml, not to styles.xml
    if (nWhich == RES_PARATR_HYPHENZONE)
        return true; // w:suppressAutoHyphens is only a formatting exception, not a default
    return MSWordExportBase::ignoreAttributeForStyleDefaults( nWhich );
}

void DocxExport::WriteOutliner(const OutlinerParaObject& rParaObj, sal_uInt8 nTyp)
{
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( *this, rEditObj, nTyp );

    sal_Int32 nPara = rEditObj.GetParagraphCount();
    for( sal_Int32 n = 0; n < nPara; ++n )
    {
        if( n )
            aAttrIter.NextPara( n );

        AttrOutput().StartParagraph( ww8::WW8TableNodeInfo::Pointer_t());
        rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();
        OUString aStr( rEditObj.GetText( n ));
        sal_Int32 nAktPos = 0;
        const sal_Int32 nEnd = aStr.getLength();
        do {
            AttrOutput().StartRun( nullptr );
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            bool bTextAtr = aAttrIter.IsTextAttr( nAktPos );
            if( !bTextAtr )
            {
                if( nAktPos == 0 && nNextAttr - nAktPos == aStr.getLength())
                    AttrOutput().RunText( aStr, eChrSet );
                else
                {
                    OUString tmp( aStr.copy( nAktPos, nNextAttr - nAktPos ));
                    AttrOutput().RunText( tmp, eChrSet );
                }
            }
            AttrOutput().StartRunProperties();
            aAttrIter.OutAttr( nAktPos );
            AttrOutput().EndRunProperties( nullptr );

            nAktPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();
            AttrOutput().EndRun();
        } while( nAktPos < nEnd );
//        aAttrIter.OutParaAttr(false);
        AttrOutput().EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t());
    }
}

void DocxExport::SetFS( ::sax_fastparser::FSHelperPtr pFS )
{
    mpFS = pFS;
}

DocxExport::DocxExport( DocxExportFilter *pFilter, SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      m_pFilter( pFilter ),
      m_pAttrOutput( nullptr ),
      m_pSections( nullptr ),
      m_nHeaders( 0 ),
      m_nFooters( 0 ),
      m_nOLEObjects( 0 ),
      m_nHeadersFootersInSection(0),
      m_pVMLExport( nullptr ),
      m_pSdrExport( nullptr )
{
    // Write the document properies
    WriteProperties( );

    // relations for the document
    m_pFilter->addRelation( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
            "word/document.xml" );

    // the actual document
    m_pDocumentFS = m_pFilter->openFragmentStreamWithSerializer( "word/document.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" );

    SetFS(m_pDocumentFS);

    // the DrawingML access
    m_pDrawingML = new oox::drawingml::DrawingML(m_pDocumentFS, m_pFilter, oox::drawingml::DOCUMENT_DOCX);

    // the attribute output for the document
    m_pAttrOutput = new DocxAttributeOutput( *this, m_pDocumentFS, m_pDrawingML );

    // the related VMLExport
    m_pVMLExport = new VMLExport( m_pDocumentFS, m_pAttrOutput );

    // the related drawing export
    m_pSdrExport = new DocxSdrExport( *this, m_pDocumentFS, m_pDrawingML );
}

DocxExport::~DocxExport()
{
    delete m_pSdrExport;
    m_pSdrExport = nullptr;
    delete m_pVMLExport;
    m_pVMLExport = nullptr;
    delete m_pAttrOutput;
    m_pAttrOutput = nullptr;
    delete m_pDrawingML;
    m_pDrawingML = nullptr;
}

DocxSettingsData::DocxSettingsData()
: evenAndOddHeaders( false )
, defaultTabStop( 0 )
, trackRevisions( false )
{
}

bool DocxSettingsData::hasData() const
{
    if( evenAndOddHeaders )
        return true;
    if( defaultTabStop != 0 )
        return true;
    if ( trackRevisions )
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
