/*************************************************************************
 *
 *  $RCSfile: wrtasc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:48:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _WRTASC_HXX
#include <wrtasc.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif

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
#if !defined(WIN) && !defined(WNT)
                aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                break;

    case 'M':
#if !defined(MAC)
                aNewOpts.SetCharSet( RTL_TEXTENCODING_APPLE_ROMAN );
                aNewOpts.SetParaFlags( LINEEND_CR );
#endif
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

ULONG SwASCWriter::WriteStream()
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

    BOOL bWriteSttTag = bUCS2_WithStartChar &&
        (RTL_TEXTENCODING_UCS2 == GetAsciiOptions().GetCharSet() ||
        RTL_TEXTENCODING_UTF8 == GetAsciiOptions().GetCharSet());

    rtl_TextEncoding eOld = Strm().GetStreamCharSet();
    Strm().SetStreamCharSet( GetAsciiOptions().GetCharSet() );

    // gebe alle Bereich des Pams in das ASC-File aus.
    do {
        BOOL bTstFly = TRUE;
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
                                Strm() << BYTE(0xEF) << BYTE(0xBB) <<
                                    BYTE(0xBF);
                                break;
                            case RTL_TEXTENCODING_UCS2:
                                //Strm().StartWritingUnicodeText();
                                Strm().SetEndianSwap(FALSE);
#ifdef __LITTLEENDIAN
                                Strm() << BYTE(0xFF) << BYTE(0xFE);
#else
                                Strm() << BYTE(0xFE) << BYTE(0xFF);
#endif
                                break;

                        }
                        bWriteSttTag = FALSE;
                    }
                    Out( aASCNodeFnTab, *pNd, *this );
                }
                bTstFly = FALSE;        // eimal Testen reicht
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


void GetASCWriter( const String& rFltNm, WriterRef& xRet )
{
  xRet = new SwASCWriter( rFltNm );
}


