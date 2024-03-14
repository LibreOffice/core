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

#include <osl/endian.h>
#include <tools/stream.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>
#include "wrtasc.hxx"
#include <frameformats.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <o3tl/string_view.hxx>

#include <strings.hrc>

SwASCWriter::SwASCWriter( std::u16string_view rFltNm )
{
    SwAsciiOptions aNewOpts;

    switch( 5 <= rFltNm.size() ? rFltNm[4] : 0 )
    {
    case 'D':
                aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
                if( 5 < rFltNm.size() )
                {
                    std::u16string_view aFilterNum = rFltNm.substr( 5 );
                    switch( o3tl::toInt32(aFilterNum) )
                    {
                    case 437: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_437 );  break;
                    case 850: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );  break;
                    case 860: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_860 );  break;
                    case 861: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_861 );  break;
                    case 863: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_863 );  break;
                    case 865: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_865 );  break;
                    }
                }
                break;

    case 'A':
#ifndef _WIN32
                aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                break;

    case 'M':
                aNewOpts.SetCharSet( RTL_TEXTENCODING_APPLE_ROMAN );
                aNewOpts.SetParaFlags( LINEEND_CR );
                break;

    case 'X':
#ifdef _WIN32
                aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aNewOpts.SetParaFlags( LINEEND_LF );
#endif
                break;

    default:
        if( rFltNm.size() >= 4 && rFltNm.substr( 4 )==u"_DLG" )
        {
            // use the options
            aNewOpts = GetAsciiOptions();
        }
    }
    SetAsciiOptions( aNewOpts );
}

SwASCWriter::~SwASCWriter() {}

ErrCode SwASCWriter::WriteStream()
{
    static constexpr OUString STR_CR = u"\015"_ustr;
    static constexpr OUStringLiteral STR_LF = u"\012";
    static constexpr OUStringLiteral STR_CRLF = u"\015\012";
    static constexpr OUStringLiteral STR_BLANK = u" ";
    bool bIncludeBOM = GetAsciiOptions().GetIncludeBOM();
    bool bIncludeHidden = GetAsciiOptions().GetIncludeHidden();

    if( m_bASCII_ParaAsCR )           // If predefined
        m_sLineEnd = STR_CR;
    else if( m_bASCII_ParaAsBlank )
        m_sLineEnd = STR_BLANK;
    else
        switch( GetAsciiOptions().GetParaFlags() )
        {
        case LINEEND_CR:    m_sLineEnd = STR_CR; break;
        case LINEEND_LF:    m_sLineEnd = STR_LF; break;
        case LINEEND_CRLF:  m_sLineEnd = STR_CRLF; break;
        }

    SwNodeOffset nMaxNode = m_pDoc->GetNodes().Count();

    if( m_bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, sal_Int32(nMaxNode), m_pDoc->GetDocShell() );

    SwPaM* pPam = m_pOrigPam;

    bool bWriteSttTag = m_bUCS2_WithStartChar &&
        (RTL_TEXTENCODING_UCS2 == GetAsciiOptions().GetCharSet() ||
        RTL_TEXTENCODING_UTF8 == GetAsciiOptions().GetCharSet());

    rtl_TextEncoding eOld = Strm().GetStreamCharSet();
    Strm().SetStreamCharSet( GetAsciiOptions().GetCharSet() );

    // Output all areas of the pam into the ASC file
    do {
        bool bTstFly = true;
        while( m_pCurrentPam->GetPoint()->GetNodeIndex() < m_pCurrentPam->GetMark()->GetNodeIndex() ||
              (m_pCurrentPam->GetPoint()->GetNodeIndex() == m_pCurrentPam->GetMark()->GetNodeIndex() &&
               m_pCurrentPam->GetPoint()->GetContentIndex() <= m_pCurrentPam->GetMark()->GetContentIndex()) )
        {
            SwTextNode* pNd = m_pCurrentPam->GetPoint()->GetNode().GetTextNode();
            if( pNd )
            {
                // Should we have frames only?
                // That's possible, if we put a frame selection into the clipboard
                if( bTstFly && m_bWriteAll &&
                    pNd->GetText().isEmpty() &&
                    // Frame exists
                    !m_pDoc->GetSpzFrameFormats()->empty() &&
                    // Only one node in the array
                    m_pDoc->GetNodes().GetEndOfExtras().GetIndex() + 3 ==
                    m_pDoc->GetNodes().GetEndOfContent().GetIndex() &&
                    // And exactly this one is selected
                    m_pDoc->GetNodes().GetEndOfContent().GetIndex() - 1 ==
                    m_pCurrentPam->GetPoint()->GetNodeIndex() )
                {
                    // Print the frame's content.
                    // It is always at position 0!
                    const SwFrameFormat* pFormat = (*m_pDoc->GetSpzFrameFormats())[ 0 ];
                    const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                    if( pIdx )
                    {
                        m_pCurrentPam = NewUnoCursor(*m_pDoc, pIdx->GetIndex(),
                                    pIdx->GetNode().EndOfSectionIndex() );
                        m_pCurrentPam->Exchange();
                        continue;       // reset while loop!
                    }
                }
                else if (!pNd->IsHidden() || bIncludeHidden)
                {
                    if (bWriteSttTag)
                    {
                        switch(GetAsciiOptions().GetCharSet())
                        {
                            case RTL_TEXTENCODING_UTF8:
                                if( bIncludeBOM )
                                {
                                    Strm().WriteUChar( 0xEF ).WriteUChar( 0xBB ).WriteUChar( 0xBF );
                                }

                                break;
                            case RTL_TEXTENCODING_UCS2:
#ifdef OSL_LITENDIAN
                                Strm().SetEndian(SvStreamEndian::LITTLE);
#else
                                Strm().SetEndian(SvStreamEndian::BIG);
#endif
                                if( bIncludeBOM )
                                {
                                    Strm().StartWritingUnicodeText();
                                }
                                break;

                        }
                        bWriteSttTag = false;
                    }

                    SwTableNode* pTableNd = pNd->FindTableNode();

                    // Handle a table
                    if (pTableNd && m_bWriteAll)
                        WriteTable(pTableNd, pNd);
                    else
                        Out( aASCNodeFnTab, *pNd, *this );
                }
                bTstFly = false;        // Testing once is enough
            }

            if( !m_pCurrentPam->Move( fnMoveForward, GoInNode ) )
                break;

            if( m_bShowProgress )
                ::SetProgressState( sal_Int32(m_pCurrentPam->GetPoint()->GetNodeIndex()),
                                    m_pDoc->GetDocShell() );   // How far?

        }
    } while( CopyNextPam( &pPam ) ); // Until all pams are processed

    Strm().SetStreamCharSet( eOld );

    if( m_bShowProgress )
        ::EndProgress( m_pDoc->GetDocShell() );

    return ERRCODE_NONE;
}

void SwASCWriter::SetupFilterOptions(SfxMedium& rMedium)
{
    if( const SfxStringItem* pItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
    {
        SwAsciiOptions aOpt;
        OUString sItemOpt;
        sItemOpt = pItem->GetValue();
        aOpt.ReadUserData(sItemOpt);
        SetAsciiOptions(aOpt);
    }
}

void SwASCWriter::WriteTable(SwTableNode* pTableNd, SwTextNode* pNd)
{
    OUString sPreLineEnd = this->m_sLineEnd;
    m_sLineEnd = u""_ustr;

    const SwTableLine* pEndTabLine = pTableNd->GetTable().GetTabLines().back();
    const SwTableBox* pEndTabBox = pEndTabLine->GetTabBoxes().back();

    for( const SwTableLine* pLine : pTableNd->GetTable().GetTabLines() )
    {
        for( const SwTableBox* pBox : pLine->GetTabBoxes() )
        {
            Out( aASCNodeFnTab, *pNd, *this );

            Point aPrevBoxPoint = pNd->GetTableBox()->GetCoordinates();
            m_pCurrentPam->Move(fnMoveForward, GoInNode);
            pNd = m_pCurrentPam->GetPoint()->GetNode().GetTextNode();

            // Line break in a box
            // Each line is a new SwTextNode so we
            // need to parse inside the current box
            while (pNd->GetTableBox() && (pNd->GetTableBox()->GetCoordinates() == aPrevBoxPoint))
            {
                Strm().WriteUnicodeOrByteText(sPreLineEnd);
                Out(aASCNodeFnTab, *pNd, *this);

                m_pCurrentPam->Move(fnMoveForward, GoInNode);
                pNd = m_pCurrentPam->GetPoint()->GetNode().GetTextNode();
            }
            if (pBox != pLine->GetTabBoxes().back())
                Strm().WriteUChar( 0x9 );

            if (pBox == pEndTabBox)
                this->m_sLineEnd = sPreLineEnd;

        }// end for each Box

        if (pLine == pEndTabLine)
        {
            m_pCurrentPam->Move(fnMoveBackward, GoInNode);
            pNd = m_pCurrentPam->GetPoint()->GetNode().GetTextNode();
            Strm().WriteUnicodeOrByteText( sPreLineEnd );
        }
        if (pLine != pEndTabLine)
            Strm().WriteUnicodeOrByteText( sPreLineEnd );

    }// end For each row
    this->m_sLineEnd = sPreLineEnd;
}

void GetASCWriter(
    std::u16string_view rFltNm, [[maybe_unused]] const OUString& /*rBaseURL*/, WriterRef& xRet )
{
  xRet = new SwASCWriter( rFltNm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
