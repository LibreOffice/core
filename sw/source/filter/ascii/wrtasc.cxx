/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
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

#include <statstr.hrc>          // ResId fuer Statusleiste

//-----------------------------------------------------------------

SwASCWriter::SwASCWriter( const String& rFltNm )
{
    SwAsciiOptions aNewOpts;

    switch( 5 <= rFltNm.Len() ? rFltNm.GetChar( 4 ) : 0 )
    {
    case 'D':
#if !defined(PM2)
                aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                if( 5 < rFltNm.Len() )
                    switch( rFltNm.Copy( 5 ).ToInt32() )
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
        if( rFltNm.Copy( 4 ).EqualsAscii( "_DLG" ))
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
    sal_Char cLineEnd[ 3 ];
    sal_Char* pCEnd = cLineEnd;
    if( bASCII_ParaAsCR )           // falls vorgegeben ist.
        *pCEnd++ = '\015';
    else if( bASCII_ParaAsBlanc )
        *pCEnd++ = ' ';
    else
        switch( GetAsciiOptions().GetParaFlags() )
        {
        case LINEEND_CR:    *pCEnd++ = '\015'; break;
        case LINEEND_LF:    *pCEnd++ = '\012'; break;
        case LINEEND_CRLF:  *pCEnd++ = '\015', *pCEnd++ = '\012'; break;
        }
    *pCEnd = 0;

    sLineEnd.AssignAscii( cLineEnd );

    long nMaxNode = pDoc->GetNodes().Count();

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

    SwPaM* pPam = pOrigPam;

    sal_Bool bWriteSttTag = bUCS2_WithStartChar &&
        (RTL_TEXTENCODING_UCS2 == GetAsciiOptions().GetCharSet() ||
        RTL_TEXTENCODING_UTF8 == GetAsciiOptions().GetCharSet());

    rtl_TextEncoding eOld = Strm().GetStreamCharSet();
    Strm().SetStreamCharSet( GetAsciiOptions().GetCharSet() );

    // gebe alle Bereich des Pams in das ASC-File aus.
    do {
        sal_Bool bTstFly = sal_True;
        while( pCurPam->GetPoint()->nNode.GetIndex() < pCurPam->GetMark()->nNode.GetIndex() ||
              (pCurPam->GetPoint()->nNode.GetIndex() == pCurPam->GetMark()->nNode.GetIndex() &&
               pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
        {
            SwTxtNode* pNd = pCurPam->GetPoint()->nNode.GetNode().GetTxtNode();
            if( pNd )
            {
                // sollten nur Rahmen vorhanden sein?
                // (Moeglich, wenn Rahmen-Selektion ins Clipboard
                // gestellt wurde)
                if( bTstFly && bWriteAll &&
                    // keine Laenge
                    !pNd->GetTxt().Len() &&
                    // Rahmen vorhanden
                    pDoc->GetSpzFrmFmts()->Count() &&
                    // nur ein Node im Array
                    pDoc->GetNodes().GetEndOfExtras().GetIndex() + 3 ==
                    pDoc->GetNodes().GetEndOfContent().GetIndex() &&
                    // und genau der ist selektiert
                    pDoc->GetNodes().GetEndOfContent().GetIndex() - 1 ==
                    pCurPam->GetPoint()->nNode.GetIndex() )
                {
                    // dann den Inhalt vom Rahmen ausgeben.
                    // dieser steht immer an Position 0 !!
                    SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[ 0 ];
                    const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                    if( pIdx )
                    {
                        delete pCurPam;
                        pCurPam = NewSwPaM( *pDoc, pIdx->GetIndex(),
                                    pIdx->GetNode().EndOfSectionIndex() );
                        pCurPam->Exchange();
                        continue;       // while-Schleife neu aufsetzen !!
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
                        bWriteSttTag = sal_False;
                    }
                    Out( aASCNodeFnTab, *pNd, *this );
                }
                bTstFly = sal_False;        // eimal Testen reicht
            }

            if( !pCurPam->Move( fnMoveForward, fnGoNode ) )
                break;

            if( bShowProgress )
                ::SetProgressState( pCurPam->GetPoint()->nNode.GetIndex(),
                                    pDoc->GetDocShell() );   // Wie weit ?

        }
    } while( CopyNextPam( &pPam ) );        // bis alle Pam bearbeitet

    Strm().SetStreamCharSet( eOld );

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

    return 0;
}


void GetASCWriter( const String& rFltNm, const String& /*rBaseURL*/, WriterRef& xRet )
{
  xRet = new SwASCWriter( rFltNm );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
