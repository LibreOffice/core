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
#include "docxhelper.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/export/shapes.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/relationship.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/ole/olehelper.hxx>

#include <map>
#include <algorithm>

#include <IMark.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include "wrtww8.hxx"
#include <fltini.hxx>
#include <fmtline.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <section.hxx>
#include <ftninfo.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <swdbdata.hxx>

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
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/storagehelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <vcl/font.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/diagnose_ex.h>

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

void DocxExport::AppendBookmarks( const SwTextNode& rNode, sal_Int32 nCurrentPos, sal_Int32 nLen )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if ( GetBookmarks( rNode, nCurrentPos, nCurrentPos + nLen, aMarks ) )
    {
        for ( IMark* pMark : aMarks )
        {
            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart == nCurrentPos )
                aStarts.push_back( pMark->GetName() );

            if ( nEnd == nCurrentPos )
                aEnds.push_back( pMark->GetName() );
        }
    }

    const OUString& aStr( rNode.GetText() );
    const sal_Int32 nEnd = aStr.getLength();

    if ( nCurrentPos == nEnd )
        m_pAttrOutput->WriteFinalBookmarks_Impl( aStarts, aEnds );
    else
        m_pAttrOutput->WriteBookmarks_Impl( aStarts, aEnds );
}

void DocxExport::AppendBookmark( const OUString& rName )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    aStarts.push_back( rName );
    aEnds.push_back( rName );

    m_pAttrOutput->WriteBookmarks_Impl( aStarts, aEnds );
}

void DocxExport::AppendAnnotationMarks( const SwWW8AttrIter& rAttrs, sal_Int32 nCurrentPos, sal_Int32 nLen )
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if (GetAnnotationMarks(rAttrs, nCurrentPos, nCurrentPos + nLen, aMarks))
    {
        for ( IMark* pMark : aMarks )
        {
            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart == nCurrentPos )
                aStarts.push_back( pMark->GetName() );

            if ( nEnd == nCurrentPos )
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
                pSerializer->singleElementNS(XML_w, XML_outlineLvl, FSNS(XML_w, XML_val), "9");

                bRet = true;
            }
        }
    }

    return bRet;
}

void DocxExport::WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
        const SwFrameFormat& rFormat, const SwFrameFormat& rLeftFormat, const SwFrameFormat& rFirstPageFormat, sal_uInt8 nBreakCode )
{
    m_nHeadersFootersInSection = 1;

    // document setting indicating the requirement of EVEN and ODD for both headers and footers
    if ( nHeadFootFlags & ( nsHdFtFlags::WW8_FOOTER_EVEN | nsHdFtFlags::WW8_HEADER_EVEN ))
        m_aSettings.evenAndOddHeaders = true;

    // Turn ON flag for 'Writing Headers \ Footers'
    m_pAttrOutput->SetWritingHeaderFooter( true );

    // headers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_EVEN )
        WriteHeaderFooter( &rLeftFormat, true, "even" );
    else if ( m_aSettings.evenAndOddHeaders )
    {
        if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD )
            WriteHeaderFooter( &rFormat, true, "even" );
        else if ( m_bHasHdr && nBreakCode == 2 )
            WriteHeaderFooter( nullptr, true, "even" );
    }

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD )
        WriteHeaderFooter( &rFormat, true, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_FIRST )
        WriteHeaderFooter( &rFirstPageFormat, true, "first" );

    if( (nHeadFootFlags & (nsHdFtFlags::WW8_HEADER_EVEN
                         | nsHdFtFlags::WW8_HEADER_ODD
                         | nsHdFtFlags::WW8_HEADER_FIRST)) == 0
            && m_bHasHdr && nBreakCode == 2 ) // 2: nexPage
        WriteHeaderFooter( nullptr, true, "default" );


    // footers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_EVEN )
        WriteHeaderFooter( &rLeftFormat, false, "even" );
    else if ( m_aSettings.evenAndOddHeaders )
    {
        if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD )
           WriteHeaderFooter( &rFormat, false, "even" );
        else if ( m_bHasFtr && nBreakCode == 2 )
            WriteHeaderFooter( nullptr, false, "even");
    }

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD )
        WriteHeaderFooter( &rFormat, false, "default" );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_FIRST )
        WriteHeaderFooter( &rFirstPageFormat, false, "first" );

    if( (nHeadFootFlags & (nsHdFtFlags::WW8_FOOTER_EVEN
                         | nsHdFtFlags::WW8_FOOTER_ODD
                         | nsHdFtFlags::WW8_FOOTER_FIRST)) == 0
            && m_bHasFtr && nBreakCode == 2 ) // 2: nexPage
        WriteHeaderFooter( nullptr, false, "default");

    // Turn OFF flag for 'Writing Headers \ Footers'
    m_pAttrOutput->SetWritingHeaderFooter( false );
}

void DocxExport::OutputField( const SwField* pField, ww::eField eFieldType, const OUString& rFieldCmd, FieldFlags nMode )
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
                             const uno::Sequence<OUString>& rListItems)
{
    m_pDocumentFS->startElementNS(XML_w, XML_ffData);

    m_pDocumentFS->singleElementNS(XML_w, XML_name, FSNS(XML_w, XML_val), rName.toUtf8());

    m_pDocumentFS->singleElementNS(XML_w, XML_enabled);

    if ( !rHelp.isEmpty() )
        m_pDocumentFS->singleElementNS(XML_w, XML_helpText, FSNS(XML_w, XML_val), rHelp.toUtf8());

    if ( !rToolTip.isEmpty() )
        m_pDocumentFS->singleElementNS( XML_w, XML_statusText,
            FSNS( XML_w, XML_val ), rToolTip.toUtf8() );

    m_pDocumentFS->startElementNS(XML_w, XML_ddList);

    // Output the 0-based index of the selected value
    sal_Int32 nId = comphelper::findValue(rListItems, rSelected);
    if (nId == -1)
        nId = 0;

    m_pDocumentFS->singleElementNS(XML_w, XML_result, FSNS(XML_w, XML_val), OString::number(nId));

    // unfortunately Word 2013 refuses to load DOCX with more than 25 listEntry
    SAL_WARN_IF(25 < rListItems.getLength(), "sw.ww8", "DocxExport::DoComboBox data loss with more than 25 entries");
    auto const nSize(std::min(sal_Int32(25), rListItems.getLength()));
    for (auto i = 0; i < nSize; ++i)
    {
        m_pDocumentFS->singleElementNS( XML_w, XML_listEntry,
                FSNS(XML_w, XML_val), rListItems[i].toUtf8() );
    }

    m_pDocumentFS->endElementNS( XML_w, XML_ddList );

    m_pDocumentFS->endElementNS( XML_w, XML_ffData );
}

void DocxExport::DoFormText(const SwInputField* pField)
{
    assert(pField);
    const OUString sStr = FieldString(ww::eFILLIN) + "\"" + pField->GetPar2() + "\"";
    OutputField(pField, ww::eFILLIN, sStr);
}

OString DocxExport::OutputChart( uno::Reference< frame::XModel > const & xModel, sal_Int32 nCount, ::sax_fastparser::FSHelperPtr const & m_pSerializer )
{
    OUString aFileName = "charts/chart" + OUString::number(nCount) + ".xml";
    OUString sId = m_pFilter->addRelation( m_pSerializer->getOutputStream(),
                    oox::getRelationship(Relationship::CHART),
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

    try
    {
        ::comphelper::OStorageHelper::CopyInputToOutput(xInStream, xOutStream);
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("sw.ww8", "DocxExport::WriteOLEObject");
        return OString();
    }

    OUString const sId = m_pFilter->addRelation( GetFS()->getOutputStream(),
                sRelationType, sFileName );
    if (pProgID)
    {
        io_rProgID = OUString::createFromAscii(pProgID);
    }

    return OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

std::pair<OString, OString> DocxExport::WriteActiveXObject(const uno::Reference<drawing::XShape>& rxShape,
                                                           const uno::Reference<awt::XControlModel>& rxControlModel)
{
    ++m_nActiveXControls;

    // Write out ActiveX binary
    const OUString sBinaryFileName = "word/activeX/activeX" + OUString::number(m_nActiveXControls) + ".bin";

    OString sGUID;
    OString sName;
    uno::Reference<io::XStream> xOutStorage(m_pFilter->openFragmentStream(sBinaryFileName, "application/vnd.ms-office.activeX"), uno::UNO_QUERY);
    if(xOutStorage.is())
    {
        oox::ole::OleStorage aOleStorage(m_pFilter->getComponentContext(), xOutStorage, false);
        uno::Reference<io::XOutputStream> xOutputStream(aOleStorage.openOutputStream("contents"), uno::UNO_SET_THROW);
        uno::Reference< css::frame::XModel > xModel( m_pDoc->GetDocShell() ? m_pDoc->GetDocShell()->GetModel() : nullptr );
        oox::ole::OleFormCtrlExportHelper exportHelper(comphelper::getProcessComponentContext(), xModel, rxControlModel);
        if ( !exportHelper.isValid() )
            return std::make_pair<OString, OString>(OString(), OString());
        sGUID = OUStringToOString(exportHelper.getGUID(), RTL_TEXTENCODING_UTF8);
        sName = OUStringToOString(exportHelper.getName(), RTL_TEXTENCODING_UTF8);
        exportHelper.exportControl(xOutputStream, rxShape->getSize(), true);
        aOleStorage.commit();
    }

    // Write out ActiveX fragment
    const OUString sXMLFileName = "word/activeX/activeX" + OUString::number( m_nActiveXControls ) + ".xml";
    ::sax_fastparser::FSHelperPtr pActiveXFS = m_pFilter->openFragmentStreamWithSerializer(sXMLFileName, "application/vnd.ms-office.activeX+xml" );

    const OUString sBinaryId = m_pFilter->addRelation( pActiveXFS->getOutputStream(),
                                                       oox::getRelationship(Relationship::ACTIVEXCONTROLBINARY),
                                                       sBinaryFileName.copy(sBinaryFileName.lastIndexOf("/") + 1) );

    pActiveXFS->singleElementNS(XML_ax, XML_ocx,
                                FSNS(XML_xmlns, XML_ax), m_pFilter->getNamespaceURL(OOX_NS(ax)).toUtf8(),
                                FSNS(XML_xmlns, XML_r), m_pFilter->getNamespaceURL(OOX_NS(officeRel)).toUtf8(),
                                FSNS(XML_ax, XML_classid), "{" + sGUID + "}",
                                FSNS(XML_ax, XML_persistence), "persistStorage",
                                FSNS(XML_r, XML_id), sBinaryId.toUtf8());

    OString sXMLId = OUStringToOString(m_pFilter->addRelation(m_pDocumentFS->getOutputStream(),
                                                              oox::getRelationship(Relationship::CONTROL),
                                                              sXMLFileName.copy(sBinaryFileName.indexOf("/") + 1)),
                                       RTL_TEXTENCODING_UTF8);

    return std::pair<OString, OString>(sXMLId, sName);
}

void DocxExport::OutputDML(uno::Reference<drawing::XShape> const & xShape)
{
    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    sal_Int32 nNamespace = XML_wps;
    if (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        nNamespace = XML_wpg;
    else if (xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        nNamespace = XML_pic;
    oox::drawingml::ShapeExport aExport(nNamespace, m_pAttrOutput->GetSerializer(), nullptr, m_pFilter, oox::drawingml::DOCUMENT_DOCX, m_pAttrOutput.get());
    aExport.WriteShape(xShape);
}

ErrCode DocxExport::ExportDocument_Impl()
{
    // Set the 'Reviewing' flags in the settings structure
    m_aSettings.revisionView = m_bOrigShowChanges;
    m_aSettings.trackRevisions = bool( RedlineFlags::On & m_nOrigRedlineFlags );

    InitStyles();

    // init sections
    m_pSections.reset(new MSWordSections( *this ));

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

    WriteEmbeddings();

    WriteVBA();

    m_aLinkedTextboxesHelper.clear();   //final cleanup
    m_pStyles.reset();
    m_pSections.reset();

    return ERRCODE_NONE;
}

void DocxExport::AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum )
{
    AttrOutput().SectionBreak( msword::PageBreak, false, m_pSections->CurrentSectionInfo() );
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

            AppendSection( m_pCurrentPageDesc, pParentFormat, nRstLnNum );
        }
        else
        {
            AttrOutput().SectionBreaks( rEndNode );
        }
    }
    else if (TXT_MAINTEXT == m_nTextTyp && rEndNode.StartOfSectionNode()->IsTableNode())
        // End node of a table: see if a section break should be written after the table.
        AttrOutput().SectionBreaks(rEndNode);
}

void DocxExport::OutputGrfNode( const SwGrfNode& )
{
    SAL_INFO("sw.ww8", "TODO DocxExport::OutputGrfNode( const SwGrfNode& )" );
}

void DocxExport::OutputOLENode( const SwOLENode& )
{
    SAL_INFO("sw.ww8", "TODO DocxExport::OutputOLENode( const SwOLENode& )" );
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
    AttrOutput().SectionBreak( msword::PageBreak, false, m_pSections->CurrentSectionInfo() );

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
    m_pStyles.reset(new MSWordStyles( *this, /*bListStyles =*/ true ));

    // setup word/styles.xml and the relations + content type
    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            oox::getRelationship(Relationship::STYLES),
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
                oox::getRelationship(Relationship::FOOTNOTES),
                "footnotes.xml" );

        ::sax_fastparser::FSHelperPtr pFootnotesFS =
            m_pFilter->openFragmentStreamWithSerializer( "word/footnotes.xml",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml" );

        // switch the serializer to redirect the output to word/footnotes.xml
        m_pAttrOutput->SetSerializer( pFootnotesFS );
        // tdf#99227
        m_pSdrExport->setSerializer( pFootnotesFS );
        // tdf#107969
        m_pVMLExport->SetFS(pFootnotesFS);

        // do the work
        m_pAttrOutput->FootnotesEndnotes( true );

        // switch the serializer back
        m_pVMLExport->SetFS(m_pDocumentFS);
        m_pSdrExport->setSerializer( m_pDocumentFS );
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
    }

    if ( m_pAttrOutput->HasEndnotes() )
    {
        // setup word/styles.xml and the relations + content type
        m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                oox::getRelationship(Relationship::ENDNOTES),
                "endnotes.xml" );

        ::sax_fastparser::FSHelperPtr pEndnotesFS =
            m_pFilter->openFragmentStreamWithSerializer( "word/endnotes.xml",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml" );

        // switch the serializer to redirect the output to word/endnotes.xml
        m_pAttrOutput->SetSerializer( pEndnotesFS );
        // tdf#99227
        m_pSdrExport->setSerializer( pEndnotesFS );
        // tdf#107969
        m_pVMLExport->SetFS(pEndnotesFS);

        // do the work
        m_pAttrOutput->FootnotesEndnotes( false );

        // switch the serializer back
        m_pVMLExport->SetFS(m_pDocumentFS);
        m_pSdrExport->setSerializer( m_pDocumentFS );
        m_pAttrOutput->SetSerializer( m_pDocumentFS );
    }
}

void DocxExport::WritePostitFields()
{
    if ( m_pAttrOutput->HasPostitFields() )
    {
        m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                oox::getRelationship(Relationship::COMMENTS),
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
        oox::getRelationship(Relationship::NUMBERING),
        "numbering.xml" );

    ::sax_fastparser::FSHelperPtr pNumberingFS = m_pFilter->openFragmentStreamWithSerializer( "word/numbering.xml",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml" );

    // switch the serializer to redirect the output to word/numbering.xml
    m_pAttrOutput->SetSerializer( pNumberingFS );
    m_pDrawingML->SetFS( pNumberingFS );

    pNumberingFS->startElementNS( XML_w, XML_numbering,
            FSNS( XML_xmlns, XML_w ), m_pFilter->getNamespaceURL(OOX_NS(doc)).toUtf8(),
            FSNS( XML_xmlns, XML_o ), m_pFilter->getNamespaceURL(OOX_NS(vmlOffice)).toUtf8(),
            FSNS( XML_xmlns, XML_r ), m_pFilter->getNamespaceURL(OOX_NS(officeRel)).toUtf8(),
            FSNS( XML_xmlns, XML_v ), m_pFilter->getNamespaceURL(OOX_NS(vml)).toUtf8() );

    BulletDefinitions();

    AbstractNumberingDefinitions();

    NumberingDefinitions();

    pNumberingFS->endElementNS( XML_w, XML_numbering );

    // switch the serializer back
    m_pDrawingML->SetFS( m_pDocumentFS );
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
}

void DocxExport::WriteHeaderFooter( const SwFormat* pFormat, bool bHeader, const char* pType )
{
    // setup the xml stream
    OUString aRelId;
    ::sax_fastparser::FSHelperPtr pFS;
    if ( bHeader )
    {
        OUString aName( "header" + OUString::number( ++m_nHeaders ) + ".xml" );

        aRelId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                oox::getRelationship(Relationship::HEADER),
                aName );

        pFS = m_pFilter->openFragmentStreamWithSerializer( "word/" + aName,
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml" );

        pFS->startElementNS( XML_w, XML_hdr, MainXmlNamespaces());
    }
    else
    {
        OUString aName( "footer" + OUString::number( ++m_nFooters ) + ".xml" );

        aRelId = m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                oox::getRelationship(Relationship::FOOTER),
                aName );

        pFS = m_pFilter->openFragmentStreamWithSerializer( "word/" + aName,
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml" );

        pFS->startElementNS( XML_w, XML_ftr, MainXmlNamespaces());
    }

    // switch the serializer to redirect the output to word/styles.xml
    m_pAttrOutput->SetSerializer( pFS );
    m_pVMLExport->SetFS( pFS );
    m_pSdrExport->setSerializer(pFS);
    SetFS( pFS );
    {
        DocxTableExportContext aTableExportContext(*m_pAttrOutput);
        //When the stream changes the cache which is maintained for the graphics in case of alternate content is not cleared.
        //So clearing the alternate content graphic cache.
        m_pAttrOutput->PushRelIdCache();
        // do the work
        if (pFormat == nullptr)
            AttrOutput().EmptyParagraph();
        else
            WriteHeaderFooterText(*pFormat, bHeader);
        m_pAttrOutput->PopRelIdCache();
        m_pAttrOutput->EndParaSdtBlock();
    }

    // switch the serializer back
    m_pAttrOutput->SetSerializer( m_pDocumentFS );
    m_pVMLExport->SetFS( m_pDocumentFS );
    m_pSdrExport->setSerializer(m_pDocumentFS);
    SetFS( m_pDocumentFS );

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
            FSNS( XML_r, XML_id ), aRelId.toUtf8() );
}

void DocxExport::WriteFonts()
{
    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            oox::getRelationship(Relationship::FONTTABLE),
            "fontTable.xml" );

    ::sax_fastparser::FSHelperPtr pFS = m_pFilter->openFragmentStreamWithSerializer(
            "word/fontTable.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml" );

    pFS->startElementNS( XML_w, XML_fonts,
            FSNS( XML_xmlns, XML_w ), m_pFilter->getNamespaceURL(OOX_NS(doc)).toUtf8(),
            FSNS( XML_xmlns, XML_r ), m_pFilter->getNamespaceURL(OOX_NS(officeRel)).toUtf8() );

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
    bool bSecurityOptOpenReadOnly = false;
    if ( pDocShell )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
               pDocShell->GetModel( ), uno::UNO_QUERY );
        xDocProps = xDPS->getDocumentProperties();
        bSecurityOptOpenReadOnly = pDocShell->IsSecurityOptOpenReadOnly();
    }

    m_pFilter->exportDocumentProperties( xDocProps, bSecurityOptOpenReadOnly );
}

static auto
WriteCompat(SwDoc const& rDoc, ::sax_fastparser::FSHelperPtr const& rpFS,
        sal_Int32 & rTargetCompatibilityMode) -> void
{
    if (!rDoc.getIDocumentSettingAccess().get(DocumentSettingId::ADD_EXT_LEADING))
    {
        rpFS->singleElementNS(XML_w, XML_noLeading);
        if (rTargetCompatibilityMode > 14)
        {   // Word ignores noLeading in compatibilityMode 15
            rTargetCompatibilityMode = 14;
        }
    }
    // Do not justify lines with manual break
    if (rDoc.getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK))
    {
        rpFS->singleElementNS(XML_w, XML_doNotExpandShiftReturn);
    }
}

void DocxExport::WriteSettings()
{
    SwViewShell *pViewShell(m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell());
    if( !pViewShell && !m_aSettings.hasData() && !m_pAttrOutput->HasFootnotes() && !m_pAttrOutput->HasEndnotes())
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            oox::getRelationship(Relationship::SETTINGS),
            "settings.xml" );

    ::sax_fastparser::FSHelperPtr pFS = m_pFilter->openFragmentStreamWithSerializer(
            "word/settings.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml" );

    pFS->startElementNS( XML_w, XML_settings,
            FSNS( XML_xmlns, XML_w ), m_pFilter->getNamespaceURL(OOX_NS(doc)).toUtf8() );

    // View
    if (pViewShell && pViewShell->GetViewOptions()->getBrowseMode())
    {
        pFS->singleElementNS(XML_w, XML_view, FSNS(XML_w, XML_val), "web");
    }

    // Zoom
    if (pViewShell)
    {
        rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(
            sax_fastparser::FastSerializerHelper::createAttrList());

        switch (pViewShell->GetViewOptions()->GetZoomType())
        {
            case SvxZoomType::WHOLEPAGE:
                pAttributeList->add(FSNS(XML_w, XML_val), "fullPage");
                break;
            case SvxZoomType::PAGEWIDTH:
                pAttributeList->add(FSNS(XML_w, XML_val), "bestFit");
                break;
            case SvxZoomType::OPTIMAL:
                pAttributeList->add(FSNS(XML_w, XML_val), "textFit");
                break;
            default:
                break;
        }

        OString aZoom(OString::number(pViewShell->GetViewOptions()->GetZoom()));
        pAttributeList->add(FSNS(XML_w, XML_percent), aZoom);
        sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList.get());
        pFS->singleElementNS(XML_w, XML_zoom, xAttributeList);
    }

    // Display Background Shape
    if (std::shared_ptr<SvxBrushItem> oBrush = getBackground(); oBrush)
    {
        // Turn on the 'displayBackgroundShape'
        pFS->singleElementNS(XML_w, XML_displayBackgroundShape);
    }

    // Track Changes
    if ( !m_aSettings.revisionView )
        pFS->singleElementNS( XML_w, XML_revisionView,
            FSNS( XML_w, XML_insDel ), "0",
            FSNS( XML_w, XML_formatting ), "0" );

    if ( m_aSettings.trackRevisions )
        pFS->singleElementNS(XML_w, XML_trackRevisions);

    // Mirror Margins
    if(isMirroredMargin())
        pFS->singleElementNS(XML_w, XML_mirrorMargins);

    // Embed Fonts
    if( m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        pFS->singleElementNS(XML_w, XML_embedTrueTypeFonts);

    // Embed System Fonts
    if( m_pDoc->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_SYSTEM_FONTS ))
        pFS->singleElementNS(XML_w, XML_embedSystemFonts);

    // Default Tab Stop
    if( m_aSettings.defaultTabStop != 0 )
        pFS->singleElementNS( XML_w, XML_defaultTabStop, FSNS( XML_w, XML_val ),
            OString::number(m_aSettings.defaultTabStop) );

    // export current mail merge database and table names
    SwDBData aData = m_pDoc->GetDBData();
    if ( !aData.sDataSource.isEmpty() && aData.nCommandType == css::sdb::CommandType::TABLE && !aData.sCommand.isEmpty() )
    {
        OUString sDataSource =
            "SELECT * FROM " +
            aData.sDataSource + // current database
            ".dbo." + // default database owner
            aData.sCommand + // sheet name
            "$"; // sheet identifier
        pFS->startElementNS( XML_w, XML_mailMerge );
        pFS->singleElementNS(XML_w, XML_mainDocumentType,
            FSNS( XML_w, XML_val ), "formLetters" );
        pFS->singleElementNS(XML_w, XML_dataType,
            FSNS( XML_w, XML_val ), "textFile" );
        pFS->singleElementNS( XML_w, XML_query,
            FSNS( XML_w, XML_val ), OUStringToOString( sDataSource, RTL_TEXTENCODING_UTF8 ).getStr() );
        pFS->endElementNS( XML_w, XML_mailMerge );
    }

    // Automatic hyphenation: it's a global setting in Word, it's a paragraph setting in Writer.
    // Set it's value to "auto" and disable on paragraph level, if no hyphenation is used there.
    pFS->singleElementNS(XML_w, XML_autoHyphenation, FSNS(XML_w, XML_val), "true");

    // Hyphenation details set depending on default style
    SwTextFormatColl* pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, /*bRegardLanguage=*/false);
    const SfxPoolItem* pItem;
    if (pColl && SfxItemState::SET == pColl->GetItemState(RES_PARATR_HYPHENZONE, false, &pItem))
    {
        if (static_cast<const SvxHyphenZoneItem*>(pItem)->IsNoCapsHyphenation())
            pFS->singleElementNS(XML_w, XML_doNotHyphenateCaps);
    }

    // Even and Odd Headers
    if( m_aSettings.evenAndOddHeaders )
        pFS->singleElementNS(XML_w, XML_evenAndOddHeaders);

    // Has Footnotes
    if( m_pAttrOutput->HasFootnotes())
        DocxAttributeOutput::WriteFootnoteEndnotePr( pFS, XML_footnotePr, m_pDoc->GetFootnoteInfo(), XML_footnote );

    // Has Endnotes
    if( m_pAttrOutput->HasEndnotes())
        DocxAttributeOutput::WriteFootnoteEndnotePr( pFS, XML_endnotePr, m_pDoc->GetEndNoteInfo(), XML_endnote );

    // Has themeFontLang information
    uno::Reference< beans::XPropertySet > xPropSet( m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW );

    bool hasProtectionProperties = false;
    bool bHasRedlineProtectionKey = false;
    bool bHasDummyRedlineProtectionKey = false;
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
    if ( xPropSetInfo->hasPropertyByName( "RedlineProtectionKey" ) )
    {
        uno::Sequence<sal_Int8> aKey;
        xPropSet->getPropertyValue( "RedlineProtectionKey" ) >>= aKey;
        bHasRedlineProtectionKey = aKey.hasElements();
        bHasDummyRedlineProtectionKey = aKey.getLength() == 1 && aKey[0] == 1;
    }

    /* Compatibility Mode (tdf#131304)
     * 11:  .doc level    [Word 97-2003]
     * 12:  .docx default [Word 2007]  [LO < 7.0]
     * 14:                [Word 2010]
     * 15:                [Word 2013/2016/2019]  [LO >= 7.0]
     *
     * The PRIMARY purpose of compatibility mode does not seem to be related to layout etc.
     * Its focus is on sharing files between multiple users, tracking the lowest supported mode in the group.
     * It is to BENEFIT older programs by not using certain new features that they don't understand.
     *
     * The next time the compat mode needs to be changed, I forsee the following steps:
     * 1.) Accept the new mode: Start round-tripping the new value, indicating we understand that format.
     * 2.) Many years later, change the TargetCompatilityMode for new documents, when we no longer care
     *     about working with perfect compatibility with older versions of MS Word.
     */
    sal_Int32 nTargetCompatibilityMode = 15; //older versions might not open our files well
    bool bHasCompatibilityMode = false;
    const OUString aGrabBagName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( xPropSetInfo->hasPropertyByName( aGrabBagName ) )
    {
        uno::Sequence< beans::PropertyValue > propList;
        xPropSet->getPropertyValue( aGrabBagName ) >>= propList;

        for( const auto& rProp : std::as_const(propList) )
        {
            if ( rProp.Name == "ThemeFontLangProps" )
            {
                uno::Sequence< beans::PropertyValue > themeFontLangProps;
                rProp.Value >>= themeFontLangProps;
                OUString aValues[3];
                for( const auto& rThemeFontLangProp : std::as_const(themeFontLangProps) )
                {
                    if( rThemeFontLangProp.Name == "val" )
                        rThemeFontLangProp.Value >>= aValues[0];
                    else if( rThemeFontLangProp.Name == "eastAsia" )
                        rThemeFontLangProp.Value >>= aValues[1];
                    else if( rThemeFontLangProp.Name == "bidi" )
                        rThemeFontLangProp.Value >>= aValues[2];
                }
                pFS->singleElementNS( XML_w, XML_themeFontLang,
                                      FSNS( XML_w, XML_val ), aValues[0].toUtf8(),
                                      FSNS( XML_w, XML_eastAsia ), aValues[1].toUtf8(),
                                      FSNS( XML_w, XML_bidi ), aValues[2].toUtf8() );
            }
            else if ( rProp.Name == "CompatSettings" )
            {
                pFS->startElementNS(XML_w, XML_compat);

                WriteCompat(*m_pDoc, pFS, nTargetCompatibilityMode);

                uno::Sequence< beans::PropertyValue > aCompatSettingsSequence;
                rProp.Value >>= aCompatSettingsSequence;

                for(const auto& rCompatSetting : std::as_const(aCompatSettingsSequence))
                {
                    uno::Sequence< beans::PropertyValue > aCompatSetting;
                    rCompatSetting.Value >>= aCompatSetting;
                    OUString aName;
                    OUString aUri;
                    OUString aValue;

                    for(const auto& rPropVal : std::as_const(aCompatSetting))
                    {
                        if( rPropVal.Name == "name" )
                            rPropVal.Value >>= aName;
                        else if( rPropVal.Name == "uri" )
                            rPropVal.Value >>= aUri;
                        else if( rPropVal.Name == "val" )
                            rPropVal.Value >>= aValue;
                    }
                    if ( aName == "compatibilityMode" )
                    {
                        bHasCompatibilityMode = true;
                        // Among the group of programs sharing this document, the lowest mode is retained.
                        // Reduce this number if we are not comfortable with the new/unknown mode yet.
                        // Step 1 in accepting a new mode would be to comment out the following clause
                        // and roundtrip the new value instead of overwriting with the older number.
                        // There are no newer modes at the time this code was written.
                        if ( aValue.toInt32() > nTargetCompatibilityMode )
                            aValue = OUString::number(nTargetCompatibilityMode);
                    }

                    pFS->singleElementNS( XML_w, XML_compatSetting,
                        FSNS( XML_w, XML_name ), aName.toUtf8(),
                        FSNS( XML_w, XML_uri ),  aUri.toUtf8(),
                        FSNS( XML_w, XML_val ),  aValue.toUtf8());
                }

                if ( !bHasCompatibilityMode )
                {
                    pFS->singleElementNS( XML_w, XML_compatSetting,
                        FSNS( XML_w, XML_name ), "compatibilityMode",
                        FSNS( XML_w, XML_uri ),  "http://schemas.microsoft.com/office/word",
                        FSNS( XML_w, XML_val ),  OString::number(nTargetCompatibilityMode));
                    bHasCompatibilityMode = true;
                }

                pFS->endElementNS( XML_w, XML_compat );
            }
            else if (rProp.Name == "DocumentProtection")
            {

                uno::Sequence< beans::PropertyValue > rAttributeList;
                rProp.Value >>= rAttributeList;

                if (rAttributeList.hasElements())
                {
                    sax_fastparser::FastAttributeList* pAttributeList = sax_fastparser::FastSerializerHelper::createAttrList();
                    bool bIsProtectionTrackChanges = false;
                    for (const auto& rAttribute : std::as_const(rAttributeList))
                    {
                        static DocxStringTokenMap const aTokens[] =
                        {
                            { "edit",                XML_edit },
                            { "enforcement",         XML_enforcement },
                            { "formatting",          XML_formatting },
                            { "cryptProviderType",   XML_cryptProviderType },
                            { "cryptAlgorithmClass", XML_cryptAlgorithmClass },
                            { "cryptAlgorithmType",  XML_cryptAlgorithmType },
                            { "cryptAlgorithmSid",   XML_cryptAlgorithmSid },
                            { "cryptSpinCount",      XML_cryptSpinCount },
                            { "hash",                XML_hash },
                            { "salt",                XML_salt },
                            { nullptr, 0 }
                        };

                        if (sal_Int32 nToken = DocxStringGetToken(aTokens, rAttribute.Name))
                        {
                            OUString sValue = rAttribute.Value.get<OUString>();
                            pAttributeList->add(FSNS(XML_w, nToken), sValue.toUtf8());
                            if ( nToken == XML_edit && sValue == "trackedChanges" )
                                bIsProtectionTrackChanges = true;
                        }
                    }

                    // we have document protection from input DOCX file
                    // and in the case of change tracking protection, we didn't modify it
                    hasProtectionProperties = !bIsProtectionTrackChanges || bHasDummyRedlineProtectionKey;
                    if ( hasProtectionProperties )
                    {
                        sax_fastparser::XFastAttributeListRef xAttributeList(pAttributeList);
                        pFS->singleElementNS(XML_w, XML_documentProtection, xAttributeList);
                    }

                }
            }
            else if (rProp.Name == "HyphenationZone")
            {
                sal_Int16 nHyphenationZone;
                rProp.Value >>= nHyphenationZone;
                if (nHyphenationZone > 0)
                    pFS->singleElementNS(XML_w, XML_hyphenationZone, FSNS(XML_w, XML_val),
                                         OString::number(nHyphenationZone));
            }
        }
    }
    if ( !bHasCompatibilityMode )
    {
        pFS->startElementNS(XML_w, XML_compat);

        WriteCompat(*m_pDoc, pFS, nTargetCompatibilityMode);

        pFS->singleElementNS( XML_w, XML_compatSetting,
            FSNS( XML_w, XML_name ), "compatibilityMode",
            FSNS( XML_w, XML_uri ),  "http://schemas.microsoft.com/office/word",
            FSNS( XML_w, XML_val ),  OString::number(nTargetCompatibilityMode));
        pFS->endElementNS( XML_w, XML_compat );
    }

    if (! hasProtectionProperties)
    {
        // Protect form - highest priority
        // Section-specific write protection
        if (m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_FORM) ||
            m_pSections->DocumentIsProtected())
        {
            // we have form protection from Writer or from input ODT file

            pFS->singleElementNS(XML_w, XML_documentProtection,
                FSNS(XML_w, XML_edit), "forms",
                FSNS(XML_w, XML_enforcement), "true");
        }
        // Protect Change Tracking - next priority
        else if ( bHasRedlineProtectionKey && !bHasDummyRedlineProtectionKey )
        {
            // we have change tracking protection from Writer or from input ODT file

            pFS->singleElementNS(XML_w, XML_documentProtection,
                FSNS(XML_w, XML_edit), "trackedChanges",
                FSNS(XML_w, XML_enforcement), "1");
        }
    }

    // finish settings.xml
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
    auto pProp = std::find_if(propList.begin(), propList.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "OOXTheme"; });
    if (pProp != propList.end())
        pProp->Value >>= themeDom;

    // no theme dom to write
    if ( !themeDom.is() )
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            oox::getRelationship(Relationship::THEME),
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
    for ( const auto& rProp : std::as_const(propList) )
    {
        OUString propName = rProp.Name;
        if ( propName == "OOXGlossary" )
        {
             rProp.Value >>= glossaryDocDom;
             collectedProperties++;
        }
        if (propName == "OOXGlossaryDom")
        {
            rProp.Value >>= glossaryDomList;
            collectedProperties++;
        }
        if (collectedProperties == 2)
            break;
    }

    // no glossary dom to write
    if ( !glossaryDocDom.is() )
        return;

    m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
            oox::getRelationship(Relationship::GLOSSARYDOCUMENT),
            "glossary/document.xml" );

    uno::Reference< io::XOutputStream > xOutputStream = GetFilter().openFragmentStream( "word/glossary/document.xml",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document.glossary+xml" );

    uno::Reference< xml::sax::XSAXSerializable > serializer( glossaryDocDom, uno::UNO_QUERY );
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
    writer->setOutputStream( xOutputStream );
    serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
        uno::Sequence< beans::StringPair >() );

    for ( const uno::Sequence< uno::Any>& glossaryElement : std::as_const(glossaryDomList))
    {
        OUString gTarget, gType, gId, contentType;
        uno::Reference<xml::dom::XDocument> xDom;
        glossaryElement[0] >>= xDom;
        glossaryElement[1] >>= gId;
        glossaryElement[2] >>= gType;
        glossaryElement[3] >>= gTarget;
        glossaryElement[4] >>= contentType;
        gId = gId.copy(3); //"rId" only save the numeric value

        PropertySet aProps(xOutputStream);
        aProps.setAnyProperty( PROP_RelId, uno::makeAny( gId.toInt32() ));
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
    static const OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if ( !xPropSetInfo->hasPropertyByName( aName ) )
        return;

    uno::Sequence<uno::Reference<xml::dom::XDocument> > customXmlDomlist;
    uno::Sequence<uno::Reference<xml::dom::XDocument> > customXmlDomPropslist;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue( aName ) >>= propList;
    auto pProp = std::find_if(propList.begin(), propList.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "OOXCustomXml"; });
    if (pProp != propList.end())
        pProp->Value >>= customXmlDomlist;

    pProp = std::find_if(propList.begin(), propList.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "OOXCustomXmlProps"; });
    if (pProp != propList.end())
        pProp->Value >>= customXmlDomPropslist;

    for (sal_Int32 j = 0; j < customXmlDomlist.getLength(); j++)
    {
        uno::Reference<xml::dom::XDocument> customXmlDom = customXmlDomlist[j];
        uno::Reference<xml::dom::XDocument> customXmlDomProps = customXmlDomPropslist[j];
        if (customXmlDom.is())
        {
            m_pFilter->addRelation( m_pDocumentFS->getOutputStream(),
                    oox::getRelationship(Relationship::CUSTOMXML),
                    "../customXml/item"+OUString::number((j+1))+".xml" );

            uno::Reference< xml::sax::XSAXSerializable > serializer( customXmlDom, uno::UNO_QUERY );
            uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create( comphelper::getProcessComponentContext() );
            writer->setOutputStream( GetFilter().openFragmentStream( "customXml/item"+OUString::number((j+1))+".xml",
                "application/xml" ) );
            serializer->serialize( uno::Reference< xml::sax::XDocumentHandler >( writer, uno::UNO_QUERY_THROW ),
                uno::Sequence< beans::StringPair >() );
        }

        if (customXmlDomProps.is())
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
                    oox::getRelationship(Relationship::CUSTOMXMLPROPS),
                    "itemProps"+OUString::number((j+1))+".xml" );
        }
    }
}

void DocxExport::WriteVBA()
{
    uno::Reference<document::XStorageBasedDocument> xStorageBasedDocument(m_pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    if (!xStorageBasedDocument.is())
        return;

    uno::Reference<embed::XStorage> xDocumentStorage = xStorageBasedDocument->getDocumentStorage();
    OUString aMacrosName("_MS_VBA_Macros");
    if (!xDocumentStorage.is() || !xDocumentStorage->hasByName(aMacrosName))
        return;

    const sal_Int32 nOpenMode = embed::ElementModes::READ;
    uno::Reference<io::XStream> xMacrosStream = xDocumentStorage->openStreamElement(aMacrosName, nOpenMode);
    uno::Reference<io::XOutputStream> xProjectStream;
    if (xMacrosStream.is())
    {
        // First handle the project stream, this sets xProjectStream.
        std::unique_ptr<SvStream> pIn(utl::UcbStreamHelper::CreateStream(xMacrosStream));

        xProjectStream = GetFilter().openFragmentStream("word/vbaProject.bin", "application/vnd.ms-office.vbaProject");
        uno::Reference<io::XStream> xOutputStream(xProjectStream, uno::UNO_QUERY);
        if (!xOutputStream.is())
            return;
        std::unique_ptr<SvStream> pOut(utl::UcbStreamHelper::CreateStream(xOutputStream));

        // Write the stream.
        pOut->WriteStream(*pIn);

        // Write the relationship.
        m_pFilter->addRelation(m_pDocumentFS->getOutputStream(), oox::getRelationship(Relationship::VBAPROJECT), "vbaProject.bin");
    }

    OUString aDataName("_MS_VBA_Macros_XML");
    if (!xDocumentStorage.is() || !xDocumentStorage->hasByName(aDataName))
        return;

    uno::Reference<io::XStream> xDataStream = xDocumentStorage->openStreamElement(aDataName, nOpenMode);
    if (xDataStream.is())
    {
        // Then the data stream, which wants to work with an already set
        // xProjectStream.
        std::unique_ptr<SvStream> pIn(utl::UcbStreamHelper::CreateStream(xDataStream));

        uno::Reference<io::XStream> xOutputStream(GetFilter().openFragmentStream("word/vbaData.xml", "application/vnd.ms-word.vbaData+xml"), uno::UNO_QUERY);
        if (!xOutputStream.is())
            return;
        std::unique_ptr<SvStream> pOut(utl::UcbStreamHelper::CreateStream(xOutputStream));

        // Write the stream.
        pOut->WriteStream(*pIn);

        // Write the relationship.
        if (!xProjectStream.is())
            return;

        m_pFilter->addRelation(xProjectStream, oox::getRelationship(Relationship::WORDVBADATA), "vbaData.xml");
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
    auto pProp = std::find_if(propList.begin(), propList.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "OOXEmbeddings"; });
    if (pProp != propList.end())
        pProp->Value >>= embeddingsList;
    for (const auto& rEmbedding : std::as_const(embeddingsList))
    {
        OUString embeddingPath = rEmbedding.Name;
        uno::Reference<io::XInputStream> embeddingsStream;
        rEmbedding.Value >>= embeddingsStream;

        OUString contentType = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        // FIXME: this .xlsm hack is silly - if anything the mime-type for an existing embedded object should be read from [Content_Types].xml
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
                TOOLS_WARN_EXCEPTION("sw.ww8", "WriteEmbeddings() ::Failed to copy Inputstream to outputstream exception caught");
            }
            xOutStream->closeOutput();
        }
    }
}

bool DocxExport::isMirroredMargin()
{
    bool bMirroredMargins = false;
    if ( UseOnPage::Mirror == (UseOnPage::Mirror & m_pDoc->GetPageDesc(0).ReadUseOn()) )
    {
        bMirroredMargins = true;
    }
    return bMirroredMargins;
}

void DocxExport::WriteMainText()
{
    // setup the namespaces
    m_pDocumentFS->startElementNS( XML_w, XML_document, MainXmlNamespaces());

    if ( getenv("SW_DEBUG_DOM") )
    {
        m_pDoc->dumpAsXml();
    }

    // reset the incrementing linked-textboxes chain ID before re-saving.
    m_nLinkedTextboxesChainId=0;
    m_aLinkedTextboxesHelper.clear();

    // Write background page color
    if (std::shared_ptr<SvxBrushItem> oBrush = getBackground(); oBrush)
    {
        Color backgroundColor = oBrush->GetColor();
        OString aBackgroundColorStr = msfilter::util::ConvertColor(backgroundColor);

        m_pDocumentFS->singleElementNS(XML_w, XML_background, FSNS(XML_w, XML_color),
                                       aBackgroundColorStr);
    }

    // body
    m_pDocumentFS->startElementNS(XML_w, XML_body);

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
    pAttr->add( FSNS( XML_xmlns, XML_o ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(vmlOffice)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_r ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(officeRel)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_v ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(vml)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_w ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(doc)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_w10 ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(vmlWord)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_wp ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(dmlWordDr)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_wps ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(wps)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_wpg ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(wpg)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_mc ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(mce)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_wp14 ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(wp14)), RTL_TEXTENCODING_UTF8).getStr() );
    pAttr->add( FSNS( XML_xmlns, XML_w14 ), OUStringToOString(m_pFilter->getNamespaceURL(OOX_NS(w14)), RTL_TEXTENCODING_UTF8).getStr() );
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
        sal_Int32 nCurrentPos = 0;
        const sal_Int32 nEnd = aStr.getLength();
        do {
            AttrOutput().StartRun( nullptr, 0 );
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            bool bTextAtr = aAttrIter.IsTextAttr( nCurrentPos );
            if( !bTextAtr )
            {
                if( nCurrentPos == 0 && nNextAttr - nCurrentPos == aStr.getLength())
                    AttrOutput().RunText( aStr, eChrSet );
                else
                {
                    OUString tmp( aStr.copy( nCurrentPos, nNextAttr - nCurrentPos ));
                    AttrOutput().RunText( tmp, eChrSet );
                }
            }
            AttrOutput().StartRunProperties();
            aAttrIter.OutAttr( nCurrentPos );
            AttrOutput().EndRunProperties( nullptr );

            nCurrentPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();

            AttrOutput().EndRun( nullptr, 0 );

        } while( nCurrentPos < nEnd );
//        aAttrIter.OutParaAttr(false);
        AttrOutput().EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t());
    }
}

void DocxExport::SetFS( ::sax_fastparser::FSHelperPtr const & pFS )
{
    mpFS = pFS;
}

DocxExport::DocxExport(DocxExportFilter* pFilter, SwDoc* pDocument,
        std::shared_ptr<SwUnoCursor> & pCurrentPam,
                       SwPaM* pOriginalPam, bool bDocm, bool bTemplate)
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      m_pFilter( pFilter ),
      m_nHeaders( 0 ),
      m_nFooters( 0 ),
      m_nOLEObjects( 0 ),
      m_nActiveXControls( 0 ),
      m_nHeadersFootersInSection(0),
      m_bDocm(bDocm),
      m_bTemplate(bTemplate)
{
    // Write the document properties
    WriteProperties( );

    // relations for the document
    m_pFilter->addRelation( oox::getRelationship(Relationship::OFFICEDOCUMENT),
            "word/document.xml" );

    // Set media type depending of document type
    OUString aMediaType;
    if (m_bDocm)
    {
        if (m_bTemplate)
        {
            aMediaType = "application/vnd.ms-word.template.macroEnabledTemplate.main+xml";
        }
        else
        {
            aMediaType = "application/vnd.ms-word.document.macroEnabled.main+xml";
        }
    }
    else
    {
        if (m_bTemplate)
        {
            aMediaType = "application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml";
        }
        else
        {
            aMediaType = "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml";
        }
    }


    // the actual document
    m_pDocumentFS = m_pFilter->openFragmentStreamWithSerializer( "word/document.xml", aMediaType );

    SetFS(m_pDocumentFS);

    // the DrawingML access
    m_pDrawingML.reset(new oox::drawingml::DrawingML(m_pDocumentFS, m_pFilter, oox::drawingml::DOCUMENT_DOCX));

    // the attribute output for the document
    m_pAttrOutput.reset(new DocxAttributeOutput( *this, m_pDocumentFS, m_pDrawingML.get() ));

    // the related VMLExport
    m_pVMLExport.reset(new VMLExport( m_pDocumentFS, m_pAttrOutput.get() ));

    // the related drawing export
    m_pSdrExport.reset(new DocxSdrExport( *this, m_pDocumentFS, m_pDrawingML.get() ));
}

DocxExport::~DocxExport()
{
}

DocxSettingsData::DocxSettingsData()
: evenAndOddHeaders( false )
, defaultTabStop( 0 )
, revisionView( true )
, trackRevisions( false )
{
}

bool DocxSettingsData::hasData() const
{
    if( evenAndOddHeaders )
        return true;
    if( defaultTabStop != 0 )
        return true;
    if ( !revisionView )
        return true;
    if ( trackRevisions )
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
