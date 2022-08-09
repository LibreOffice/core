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

#include <sal/config.h>

#include <string_view>

#include <osl/diagnose.h>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/HtmlWriter.hxx>
#include <rtl/strbuf.hxx>
#include <ndindex.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <charfmt.hxx>

#include "swhtml.hxx"
#include "wrthtml.hxx"

static sal_Int32 lcl_html_getNextPart( OUString& rPart, const OUString& rContent,
                             sal_Int32 nPos )
{
    rPart.clear();
    sal_Int32 nLen = rContent.getLength();
    if( nPos >= nLen )
    {
        nPos = -1;
    }
    else
    {
        bool bQuoted = false, bDone = false;
        for( ; nPos < nLen && !bDone; nPos++ )
        {
            sal_Unicode c = rContent[nPos];
            switch( c )
            {
            case '\\':
                if( bQuoted )
                    rPart += OUStringChar( c );
                bQuoted = !bQuoted;
                break;

            case ';':
                if( bQuoted )
                    rPart += OUStringChar( c );
                else
                    bDone = true;
                bQuoted = false;
                break;

            default:
                rPart += OUStringChar( c );
                bQuoted = false;
                break;
            }
        }
    }

    return nPos;
}

static sal_Int32 lcl_html_getEndNoteInfo( SwEndNoteInfo& rInfo,
                                    const OUString& rContent,
                                    bool bEndNote )
{
    sal_Int32 nStrPos = 0;
    for( int nPart = 0; nPart < 4; ++nPart )
    {
        OUString aPart;
        if( -1 != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 0:
            rInfo.m_aFormat.SetNumberingType( bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC );
            if( !aPart.isEmpty() )
                rInfo.m_aFormat.SetNumberingType(SwHTMLParser::GetNumType( aPart,
                                                             rInfo.m_aFormat.GetNumberingType() ));
            break;

        case 1:
            rInfo.m_nFootnoteOffset = aPart.isEmpty() ? 0 : o3tl::narrowing<sal_uInt16>(aPart.toInt32());
            break;

        case 2:
            rInfo.SetPrefix( aPart );
            break;

        case 3:
            rInfo.SetSuffix( aPart );
            break;
        }
    }

    return nStrPos;
}

void SwHTMLParser::FillEndNoteInfo( const OUString& rContent )
{
    SwEndNoteInfo aInfo( m_xDoc->GetEndNoteInfo() );
    lcl_html_getEndNoteInfo( aInfo, rContent, true );
    m_xDoc->SetEndNoteInfo( aInfo );
}

void SwHTMLParser::FillFootNoteInfo( const OUString& rContent )
{
    SwFootnoteInfo aInfo( m_xDoc->GetFootnoteInfo() );

    sal_Int32 nStrPos = lcl_html_getEndNoteInfo( aInfo, rContent, false );

    for( int nPart = 4; nPart < 8; ++nPart )
    {
        OUString aPart;
        if( -1 != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 4:
            aInfo.m_eNum = FTNNUM_DOC;
            if( !aPart.isEmpty() )
            {
                switch( aPart[0] )
                {
                case 'D': aInfo.m_eNum = FTNNUM_DOC; break;
                case 'C': aInfo.m_eNum = FTNNUM_CHAPTER; break;
                case 'P': aInfo.m_eNum = FTNNUM_PAGE; break;
                }
            }
            break;

        case 5:
            aInfo.m_ePos = FTNPOS_PAGE;
            if( !aPart.isEmpty() )
            {
                switch( aPart[0] )
                {
                case 'C': aInfo.m_ePos = FTNPOS_CHAPTER; break;
                case 'P': aInfo.m_ePos = FTNPOS_PAGE; break;
                }
            }
            break;

        case 6:
            aInfo.m_aQuoVadis = aPart;
            break;

        case 7:
            aInfo.m_aErgoSum = aPart;
            break;
        }
    }

    m_xDoc->SetFootnoteInfo( aInfo );
}

void SwHTMLParser::InsertFootEndNote( const OUString& rName, bool bEndNote,
                                      bool bFixed )
{
    if( !m_pFootEndNoteImpl )
        m_pFootEndNoteImpl.reset(new SwHTMLFootEndNote_Impl);

    m_pFootEndNoteImpl->sName = rName;
    if( m_pFootEndNoteImpl->sName.getLength() > 3 )
        m_pFootEndNoteImpl->sName = m_pFootEndNoteImpl->sName.copy( 0, m_pFootEndNoteImpl->sName.getLength() - 3 );
    m_pFootEndNoteImpl->sName = m_pFootEndNoteImpl->sName.toAsciiUpperCase();
    m_pFootEndNoteImpl->bEndNote = bEndNote;
    m_pFootEndNoteImpl->bFixed = bFixed;
    m_pFootEndNoteImpl->sContent.clear();
}

void SwHTMLParser::FinishFootEndNote()
{
    if( !m_pFootEndNoteImpl )
        return;

    SwFormatFootnote aFootnote( m_pFootEndNoteImpl->bEndNote );
    if( m_pFootEndNoteImpl->bFixed )
        aFootnote.SetNumStr( m_pFootEndNoteImpl->sContent );

    m_xDoc->getIDocumentContentOperations().InsertPoolItem( *m_pPam, aFootnote );
    SwTextFootnote * const pTextFootnote = static_cast<SwTextFootnote *>(
        m_pPam->GetNode().GetTextNode()->GetTextAttrForCharAt(
            m_pPam->GetPoint()->GetContentIndex() - 1, RES_TXTATR_FTN ) );
    // In header and footer no footnotes can be inserted.
    if (pTextFootnote)
        m_pFootEndNoteImpl->aTextFootnotes.push_back(SwHTMLTextFootnote(m_pFootEndNoteImpl->sName,pTextFootnote));
    m_pFootEndNoteImpl->sName.clear();
    m_pFootEndNoteImpl->sContent.clear();
    m_pFootEndNoteImpl->bFixed = false;
}

void SwHTMLParser::InsertFootEndNoteText()
{
    if( m_pFootEndNoteImpl && m_pFootEndNoteImpl->bFixed )
        m_pFootEndNoteImpl->sContent += aToken;
}

SwNodeIndex *SwHTMLParser::GetFootEndNoteSection( const OUString& rName )
{
    SwNodeIndex *pStartNodeIdx = nullptr;

    if (m_pFootEndNoteImpl)
    {
        OUString aName(rName.toAsciiUpperCase());

        size_t nCount = m_pFootEndNoteImpl->aTextFootnotes.size();
        for(size_t i = 0; i < nCount; ++i)
        {
            if (m_pFootEndNoteImpl->aTextFootnotes[i].sName == aName)
            {
                pStartNodeIdx = const_cast<SwNodeIndex*>(m_pFootEndNoteImpl->aTextFootnotes[i].pTextFootnote->GetStartNode());
                m_pFootEndNoteImpl->aTextFootnotes.erase( m_pFootEndNoteImpl->aTextFootnotes.begin() + i );
                if (m_pFootEndNoteImpl->aTextFootnotes.empty())
                {
                    m_pFootEndNoteImpl.reset();
                }

                break;
            }
        }
    }

    return pStartNodeIdx;
}

Writer& OutHTML_SwFormatLineBreak(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    const auto& rLineBreak = static_cast<const SwFormatLineBreak&>(rHt);

    HtmlWriter aWriter(rHTMLWrt.Strm(), rHTMLWrt.maNamespace);
    aWriter.start(OOO_STRING_SVTOOLS_HTML_linebreak);
    switch (rLineBreak.GetValue())
    {
        case SwLineBreakClear::NONE:
            aWriter.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, "none");
            break;
        case SwLineBreakClear::LEFT:
            aWriter.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, "left");
            break;
        case SwLineBreakClear::RIGHT:
            aWriter.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, "right");
            break;
        case SwLineBreakClear::ALL:
            aWriter.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, "all");
            break;
    }
    aWriter.end();
    return rWrt;
}

Writer& OutHTML_SwFormatFootnote( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    SwFormatFootnote& rFormatFootnote = const_cast<SwFormatFootnote&>(static_cast<const SwFormatFootnote&>(rHt));
    SwTextFootnote *pTextFootnote = rFormatFootnote.GetTextFootnote();
    if( !pTextFootnote )
        return rWrt;

    OUString sFootnoteName, sClass;
    size_t nPos;
    if( rFormatFootnote.IsEndNote() )
    {
        nPos = rHTMLWrt.m_xFootEndNotes ? rHTMLWrt.m_xFootEndNotes->size() : 0;
        OSL_ENSURE( nPos == static_cast<size_t>(rHTMLWrt.m_nFootNote + rHTMLWrt.m_nEndNote),
                "OutHTML_SwFormatFootnote: wrong position" );
        sClass = OOO_STRING_SVTOOLS_HTML_sdendnote_anc;
        sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdendnote + OUString::number( static_cast<sal_Int32>(++rHTMLWrt.m_nEndNote) );
    }
    else
    {
        nPos = rHTMLWrt.m_nFootNote;
        sClass = OOO_STRING_SVTOOLS_HTML_sdfootnote_anc;
        sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdfootnote + OUString::number( static_cast<sal_Int32>(++rHTMLWrt.m_nFootNote));
    }

    if( !rHTMLWrt.m_xFootEndNotes )
        rHTMLWrt.m_xFootEndNotes.emplace();
    rHTMLWrt.m_xFootEndNotes->insert( rHTMLWrt.m_xFootEndNotes->begin() + nPos, pTextFootnote );

    OStringBuffer sOut;
    OString aTag = rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor;
    sOut.append("<" + aTag + " " OOO_STRING_SVTOOLS_HTML_O_class "=\"");
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( rWrt.Strm(), sClass );
    sOut.append("\" " OOO_STRING_SVTOOLS_HTML_O_name "=\"");
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFootnoteName );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_anchor "\" "
                OOO_STRING_SVTOOLS_HTML_O_href "=\"#");
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFootnoteName );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_symbol "\"");
    if( !rFormatFootnote.GetNumStr().isEmpty() )
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_sdfixed);
    sOut.append(">");
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OStringConcatenation(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_superscript ));

    HTMLOutFuncs::Out_String( rWrt.Strm(), rFormatFootnote.GetViewNumStr(*rWrt.m_pDoc, nullptr) );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OStringConcatenation(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_superscript), false );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OStringConcatenation(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor), false );

    return rWrt;
}

void SwHTMLWriter::OutFootEndNotes()
{
    OSL_ENSURE( m_xFootEndNotes,
            "SwHTMLWriter::OutFootEndNotes(): unnecessary call" );
    if( !m_xFootEndNotes )
        return;

#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 nFootnote = m_nFootNote, nEn = m_nEndNote;
#endif
    m_nFootNote = 0;
    m_nEndNote = 0;

    for( auto *pTextFootnote : *m_xFootEndNotes )
    {
        m_pFormatFootnote = &pTextFootnote->GetFootnote();

        OUString sFootnoteName;
        if( m_pFormatFootnote->IsEndNote() )
        {
            sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdendnote + OUString::number(static_cast<sal_Int32>(++m_nEndNote));
        }
        else
        {
            sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdfootnote + OUString::number(static_cast<sal_Int32>(++m_nFootNote));
        }

        if( m_bLFPossible )
            OutNewLine();
        OString sOut =
            "<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_division
            " " OOO_STRING_SVTOOLS_HTML_O_id "=\"";
        Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( Strm(), sFootnoteName );
        Strm().WriteCharPtr( "\">" );

        m_bLFPossible = true;
        IncIndentLevel();   // indent content of <DIV>

        OSL_ENSURE( pTextFootnote, "SwHTMLWriter::OutFootEndNotes: SwTextFootnote is missing" );
        const SwNodeIndex *pSttNdIdx = pTextFootnote->GetStartNode();
        OSL_ENSURE( pSttNdIdx,
                "SwHTMLWriter::OutFootEndNotes: StartNode-Index is missing" );
        if( pSttNdIdx )
        {
            HTMLSaveData aSaveData( *this, pSttNdIdx->GetIndex()+1,
                pSttNdIdx->GetNode().EndOfSectionIndex(), false );
            Out_SwDoc( m_pCurrentPam.get() );
        }

        DecIndentLevel();   // indent content of <DIV>
        if( m_bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OStringConcatenation(GetNamespace() + OOO_STRING_SVTOOLS_HTML_division), false );
        m_bLFPossible = true;

        OSL_ENSURE( !m_pFormatFootnote,
                "SwHTMLWriter::OutFootEndNotes: Footnote was not output" );
        if( m_pFormatFootnote )
        {
            if( m_pFormatFootnote->IsEndNote() )
                m_nEndNote++;
            else
                m_nFootNote++;

            m_pFormatFootnote = nullptr;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( nFootnote == m_nFootNote,
            "SwHTMLWriter::OutFootEndNotes: Number of footnotes does not match" );
    OSL_ENSURE( nEn == m_nEndNote,
            "SwHTMLWriter::OutFootEndNotes: Number of endnotes does not match" );
#endif

    m_xFootEndNotes.reset();
    m_nFootNote = m_nEndNote = 0;
}

OUString SwHTMLWriter::GetFootEndNoteSym( const SwFormatFootnote& rFormatFootnote )
{
    const SwEndNoteInfo * pInfo = nullptr;
    if( rFormatFootnote.GetNumStr().isEmpty() )
        pInfo = rFormatFootnote.IsEndNote() ? &m_pDoc->GetEndNoteInfo()
                                    : &m_pDoc->GetFootnoteInfo();

    OUString sRet;
    if( pInfo )
        sRet = pInfo->GetPrefix();
    sRet += rFormatFootnote.GetViewNumStr(*m_pDoc, nullptr);
    if( pInfo )
        sRet += pInfo->GetSuffix();

    return sRet;
}

void SwHTMLWriter::OutFootEndNoteSym( const SwFormatFootnote& rFormatFootnote,
                                         const OUString& rNum,
                                         sal_uInt16 nScript )
{
    const SwEndNoteInfo *pInfo;

    OUString sFootnoteName, sClass;
    if( rFormatFootnote.IsEndNote() )
    {
        sClass = OOO_STRING_SVTOOLS_HTML_sdendnote_sym;
        sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdendnote +
            OUString::number(static_cast<sal_Int32>(m_nEndNote));
        pInfo = &m_pDoc->GetEndNoteInfo();
    }
    else
    {
        sClass = OOO_STRING_SVTOOLS_HTML_sdfootnote_sym;
        sFootnoteName = OOO_STRING_SVTOOLS_HTML_sdfootnote +
            OUString::number(static_cast<sal_Int32>(m_nFootNote));
        pInfo = &m_pDoc->GetFootnoteInfo();
    }

    const SwCharFormat *pSymCharFormat = pInfo->GetCharFormat( *m_pDoc );
    if( pSymCharFormat && 0 != m_aScriptTextStyles.count( pSymCharFormat->GetName() ) )
    {
        switch( nScript )
        {
        case CSS1_OUTMODE_WESTERN:
            sClass += "-western";
            break;
        case CSS1_OUTMODE_CJK:
            sClass += "-cjk";
            break;
        case CSS1_OUTMODE_CTL:
            sClass += "-ctl";
            break;
        }
    }

    OStringBuffer sOut;
    sOut.append("<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor " "
            OOO_STRING_SVTOOLS_HTML_O_class "=\"");
    Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( Strm(), sClass );
    sOut.append("\" " OOO_STRING_SVTOOLS_HTML_O_name "=\"");
    Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( Strm(), sFootnoteName );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_symbol "\" "
            OOO_STRING_SVTOOLS_HTML_O_href "=\"#");
    Strm().WriteOString( sOut );
    sOut.setLength(0);
    HTMLOutFuncs::Out_String( Strm(), sFootnoteName );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_anchor "\">");
    Strm().WriteOString( sOut );
    sOut.setLength(0);

    HTMLOutFuncs::Out_String( Strm(), rNum );
    HTMLOutFuncs::Out_AsciiTag( Strm(), OStringConcatenation(GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor), false );
}

static int lcl_html_fillEndNoteInfo( const SwEndNoteInfo& rInfo,
                                 OUString *pParts,
                                 bool bEndNote  )
{
    int nParts = 0;
    sal_Int16 eFormat = rInfo.m_aFormat.GetNumberingType();
    if( (bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC) != eFormat )
    {
        const char *pStr = SwHTMLWriter::GetNumFormat( eFormat );
        if( pStr )
        {
            pParts[0] = OUString::createFromAscii( pStr );
            nParts = 1;
        }
    }
    if( rInfo.m_nFootnoteOffset > 0 )
    {
        pParts[1] = OUString::number(rInfo.m_nFootnoteOffset);
        nParts = 2;
    }
    if( !rInfo.GetPrefix().isEmpty() )
    {
        pParts[2] = rInfo.GetPrefix();
        nParts = 3;
    }
    if( !rInfo.GetSuffix().isEmpty() )
    {
        pParts[3] = rInfo.GetSuffix();
        nParts = 4;
    }

    return nParts;
}

static void lcl_html_outFootEndNoteInfo( Writer& rWrt, OUString const *pParts,
                                  int nParts, const char *pName )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    OUStringBuffer aContent;
    for( int i=0; i<nParts; ++i )
    {
        OUString aTmp( pParts[i] );
        aTmp = aTmp.replaceAll( "\\", "\\\\" );
        aTmp = aTmp.replaceAll( ";", "\\;" );
        if( i > 0 )
            aContent.append(";");
        aContent.append(aTmp);
    }

    rHTMLWrt.OutNewLine();
    OString sOut =
        "<" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_meta " "
        OOO_STRING_SVTOOLS_HTML_O_name "=\"" + pName +
        "\" " OOO_STRING_SVTOOLS_HTML_O_content "=\"";
    rWrt.Strm().WriteOString( sOut );
    HTMLOutFuncs::Out_String( rWrt.Strm(), aContent.makeStringAndClear() );
    rWrt.Strm().WriteCharPtr( "\">" );
}

void SwHTMLWriter::OutFootEndNoteInfo()
{
    // Number type (1 or i)
    // Offset (0)
    // Before it
    // Behind it
    // Doc/Page/Chap (D)
    // Position (S)
    // Next page
    // Beginning

    {
        const SwFootnoteInfo& rInfo = m_pDoc->GetFootnoteInfo();
        OUString aParts[8];
        int nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, false );
        if( rInfo.m_eNum != FTNNUM_DOC )
        {
            aParts[4] = rInfo.m_eNum == FTNNUM_CHAPTER ? std::u16string_view( u"C" ) : std::u16string_view( u"P" );
            nParts = 5;
        }
        if( rInfo.m_ePos != FTNPOS_PAGE)
        {
            aParts[5] = "C";
            nParts = 6;
        }
        if( !rInfo.m_aQuoVadis.isEmpty() )
        {
            aParts[6] = rInfo.m_aQuoVadis;
            nParts = 7;
        }
        if( !rInfo.m_aErgoSum.isEmpty() )
        {
            aParts[7] = rInfo.m_aErgoSum;
            nParts = 8;
        }
        if( nParts > 0 )
            lcl_html_outFootEndNoteInfo( *this, aParts, nParts,
                                         OOO_STRING_SVTOOLS_HTML_META_sdfootnote );
    }

    {
        const SwEndNoteInfo& rInfo = m_pDoc->GetEndNoteInfo();
        OUString aParts[4];
        const int nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, true );
        if( nParts > 0 )
            lcl_html_outFootEndNoteInfo( *this, aParts, nParts,
                                         OOO_STRING_SVTOOLS_HTML_META_sdendnote );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
