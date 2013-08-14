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

#include <hintids.hxx>
#include <osl/endian.h>
#include <tools/stream.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>
#include <wrtasc.hxx>

#include <statstr.hrc>          // ResId for status bar

//-----------------------------------------------------------------

SwASCWriter::SwASCWriter( const OUString& rFltNm )
{
    SwAsciiOptions aNewOpts;

    switch( 5 <= rFltNm.getLength() ? rFltNm[4] : 0 )
    {
    case 'D':
                aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
                if( 5 < rFltNm.getLength() )
                    switch( rFltNm.copy( 5 ).toInt32() )
                    {
                    case 437: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_437 );  break;
                    case 850: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );  break;
                    case 860: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_860 );  break;
                    case 861: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_861 );  break;
                    case 863: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_863 );  break;
                    case 865: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_865 );  break;
                    }
                break;

    case 'A':
#if !defined(WNT)
                aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                break;

    case 'M':
                aNewOpts.SetCharSet( RTL_TEXTENCODING_APPLE_ROMAN );
                aNewOpts.SetParaFlags( LINEEND_CR );
                break;

    case 'X':
#if !defined(UNX)
                aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aNewOpts.SetParaFlags( LINEEND_LF );
#endif
                break;

    default:
        if( rFltNm.copy( 4 )=="_DLG" )
        {
            // use the options
            aNewOpts = GetAsciiOptions();
        }
    }
    SetAsciiOptions( aNewOpts );
}

SwASCWriter::~SwASCWriter() {}

sal_uLong SwASCWriter::WriteStream()
{
    if( bASCII_ParaAsCR )           // If predefined
        m_sLineEnd = "\015";
    else if( bASCII_ParaAsBlanc )
        m_sLineEnd = " ";
    else
        switch( GetAsciiOptions().GetParaFlags() )
        {
        case LINEEND_CR:    m_sLineEnd = "\015"; break;
        case LINEEND_LF:    m_sLineEnd = "\012"; break;
        case LINEEND_CRLF:  m_sLineEnd = "\015\012"; break;
        }

    long nMaxNode = pDoc->GetNodes().Count();

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

    SwPaM* pPam = pOrigPam;

    bool bWriteSttTag = bUCS2_WithStartChar &&
        (RTL_TEXTENCODING_UCS2 == GetAsciiOptions().GetCharSet() ||
        RTL_TEXTENCODING_UTF8 == GetAsciiOptions().GetCharSet());

    rtl_TextEncoding eOld = Strm().GetStreamCharSet();
    Strm().SetStreamCharSet( GetAsciiOptions().GetCharSet() );

    // Output all areas of the pam into the ASC file
    do {
        bool bTstFly = true;
        while( pCurPam->GetPoint()->nNode.GetIndex() < pCurPam->GetMark()->nNode.GetIndex() ||
              (pCurPam->GetPoint()->nNode.GetIndex() == pCurPam->GetMark()->nNode.GetIndex() &&
               pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
        {
            SwTxtNode* pNd = pCurPam->GetPoint()->nNode.GetNode().GetTxtNode();
            if( pNd )
            {
                // Should we have frames only?
                // That's possible, if we put a frame selection into the clipboard
                if( bTstFly && bWriteAll &&
                    pNd->GetTxt().isEmpty() &&
                    // Frame exists
                    pDoc->GetSpzFrmFmts()->size() &&
                    // Only one node in the array
                    pDoc->GetNodes().GetEndOfExtras().GetIndex() + 3 ==
                    pDoc->GetNodes().GetEndOfContent().GetIndex() &&
                    // And exactly this one is selected
                    pDoc->GetNodes().GetEndOfContent().GetIndex() - 1 ==
                    pCurPam->GetPoint()->nNode.GetIndex() )
                {
                    // Print the frame's content.
                    // It is always at position 0!
                    SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[ 0 ];
                    const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                    if( pIdx )
                    {
                        delete pCurPam;
                        pCurPam = NewSwPaM( *pDoc, pIdx->GetIndex(),
                                    pIdx->GetNode().EndOfSectionIndex() );
                        pCurPam->Exchange();
                        continue;       // reset while loop!
                    }
                }
                else
                {
                    if (bWriteSttTag)
                    {
                        switch(GetAsciiOptions().GetCharSet())
                        {
                            case RTL_TEXTENCODING_UTF8:
                                Strm() << sal_uInt8(0xEF) << sal_uInt8(0xBB) <<
                                    sal_uInt8(0xBF);
                                break;
                            case RTL_TEXTENCODING_UCS2:
                                Strm().SetEndianSwap(sal_False);
#ifdef OSL_LITENDIAN
                                Strm() << sal_uInt8(0xFF) << sal_uInt8(0xFE);
#else
                                Strm() << sal_uInt8(0xFE) << sal_uInt8(0xFF);
#endif
                                break;

                        }
                        bWriteSttTag = false;
                    }
                    Out( aASCNodeFnTab, *pNd, *this );
                }
                bTstFly = false;        // Testing once is enough
            }

            if( !pCurPam->Move( fnMoveForward, fnGoNode ) )
                break;

            if( bShowProgress )
                ::SetProgressState( pCurPam->GetPoint()->nNode.GetIndex(),
                                    pDoc->GetDocShell() );   // How far?

        }
    } while( CopyNextPam( &pPam ) ); // Until all pams are processed

    Strm().SetStreamCharSet( eOld );

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

    return 0;
}


void GetASCWriter( const OUString& rFltNm, const OUString& /*rBaseURL*/, WriterRef& xRet )
{
  xRet = new SwASCWriter( rFltNm );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
