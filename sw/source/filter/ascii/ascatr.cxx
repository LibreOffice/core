/*************************************************************************
 *
 *  $RCSfile: ascatr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
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
#ifndef _WRTASC_HXX
#include <wrtasc.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif

/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des ASCII-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */

void lcl_WriteString( SvStream& rOut, const String& rStr,
                      rtl_TextEncoding eSrcSet, rtl_TextEncoding eDest );

class SwASC_AttrIter
{
    SvPtrarr aTxtAtrArr;
    SvUShorts aChrSetArr;
    SwASCWriter& rWrt;
    const SwTxtNode& rNd;
    xub_StrLen nAktSwPos;
    rtl_TextEncoding eNdChrSet;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    BOOL SetCharSet( const SwTxtAttr& rTxtAttr, BOOL bStart );

public:
    SwASC_AttrIter( SwASCWriter& rWrt, const SwTxtNode& rNd, xub_StrLen nStt );

    void NextPos() { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    xub_StrLen WhereNext() const                    { return nAktSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const     { return eNdChrSet; }

    BOOL OutAttr( xub_StrLen nSwPos, rtl_TextEncoding eChrSet );
};


SwASC_AttrIter::SwASC_AttrIter( SwASCWriter& rWr, const SwTxtNode& rTxtNd,
                                xub_StrLen nStt )
    : rWrt( rWr ), rNd( rTxtNd ), nAktSwPos( 0 ),
    aTxtAtrArr( 0, 4 ), aChrSetArr( 0, 4 )
{
    // Attributwechsel an Pos 0 wird ignoriert, da davon ausgegangen
    // wird, dass am Absatzanfang sowieso die Attribute neu ausgegeben
    // werden.
    eNdChrSet = ((SvxFontItem&)rNd.SwCntntNode::GetAttr(
                                        RES_CHRATR_FONT )).GetCharSet();
    nAktSwPos = SearchNext( nStt + 1 );
}

rtl_TextEncoding SwASC_AttrIter::GetNextCharSet() const
{
    if( aChrSetArr.Count() )
        return (rtl_TextEncoding )aChrSetArr[ aChrSetArr.Count() - 1 ];
    return eNdChrSet;
}

xub_StrLen SwASC_AttrIter::SearchNext( xub_StrLen nStartPos )
{
    register xub_StrLen nMinPos = STRING_MAXLEN;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        register USHORT i;
        register xub_StrLen nPos;
        const xub_StrLen * pPos;

// kann noch optimiert werden, wenn ausgenutzt wird, dass die TxtAttrs
// nach der Anfangsposition geordnet sind. Dann muessten
// allerdings noch 2 Indices gemerkt werden
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            nPos = *pHt->GetStart();    // gibt erstes Attr-Zeichen
            pPos = pHt->GetEnd();

            if( nPos >= nStartPos && nPos <= nMinPos &&
                ( !pPos || SetCharSet( *pHt, TRUE ) ) )
                nMinPos = nPos;

            if( pPos ? ( (nPos = *pPos) >= nStartPos && nPos <= nMinPos &&
                         SetCharSet( *pHt, FALSE ) )
                     : (( ++nPos ) >= nStartPos && nPos < nMinPos) )
                nMinPos = nPos;
        }
    }
    return nMinPos;
}

BOOL SwASC_AttrIter::SetCharSet( const SwTxtAttr& rAttr, BOOL bStart )
{
    void* p = 0;
    rtl_TextEncoding eChrSet;
    const SfxPoolItem& rItem = rAttr.GetAttr();
    switch( rItem.Which() )
    {
    case RES_CHRATR_FONT:
        p = (void*)&rAttr;
        eChrSet = ((SvxFontItem&)rItem).GetCharSet();
        break;

    case RES_TXTATR_CHARFMT:
        {
            const SfxPoolItem* pItem;
            if( ((SwFmtCharFmt&)rItem).GetCharFmt() && SFX_ITEM_SET ==
                ((SwFmtCharFmt&)rItem).GetCharFmt()->GetItemState(
                    RES_CHRATR_FONT, TRUE, &pItem ))
            {
                eChrSet = ((SvxFontItem*)pItem)->GetCharSet();
                p = (void*)&rAttr;
            }
        }
        break;
    }

    if( p )
    {
        USHORT nPos;
        if( bStart )
        {
            nPos = aChrSetArr.Count();
            aChrSetArr.Insert( eChrSet, nPos );
            aTxtAtrArr.Insert( p, nPos );
        }
        else if( USHRT_MAX != ( nPos = aTxtAtrArr.GetPos( p )) )
        {
            aTxtAtrArr.Remove( nPos );
            aChrSetArr.Remove( nPos );
        }
    }
    return 0 != p;
}

BOOL SwASC_AttrIter::OutAttr( xub_StrLen nSwPos, rtl_TextEncoding eChrSet )
{
    BOOL bRet = FALSE;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        register USHORT i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const xub_StrLen * pEnd = pHt->GetEnd();
            if( !pEnd && nSwPos == *pHt->GetStart() )
            {
                bRet = TRUE;
                String sOut;
                switch( pHt->Which() )
                {
                case RES_TXTATR_FIELD:
                    sOut = ((SwTxtFld*)pHt)->GetFld().GetFld()->Expand();
                    break;

                case RES_TXTATR_HARDBLANK:
                    sOut = ((SwTxtHardBlank*)pHt)->GetChar();
                    break;

                case RES_TXTATR_FTN:
                    {
                        const SwFmtFtn& rFtn = pHt->GetFtn();
                        if( rFtn.GetNumStr().Len() )
                            sOut = rFtn.GetNumStr();
                        else if( rFtn.IsEndNote() )
                            sOut = rWrt.pDoc->GetEndNoteInfo().aFmt.
                                            GetNumStr( rFtn.GetNumber() );
                        else
                            sOut = rWrt.pDoc->GetFtnInfo().aFmt.
                                            GetNumStr( rFtn.GetNumber() );
                    }
                    break;
                }
                if( sOut.Len() )
                    ::lcl_WriteString( rWrt.Strm(), sOut, eChrSet,
                                        rWrt.GetAsciiOptions().GetCharSet() );
            }
            else if( nSwPos < *pHt->GetStart() )
                break;
        }
    }
    return bRet;
}


//------------------------
/* Ausgabe der Nodes */
//------------------------

static Writer& OutASC_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    const SwTxtNode& rNd = (SwTxtNode&)rNode;

    xub_StrLen nStrPos = rWrt.pCurPam->GetPoint()->nContent.GetIndex();
    xub_StrLen nNodeEnde = rNd.Len(), nEnde = nNodeEnde;
    BOOL bLastNd =  rWrt.pCurPam->GetPoint()->nNode == rWrt.pCurPam->GetMark()->nNode;
    if( bLastNd )
        nEnde = rWrt.pCurPam->GetMark()->nContent.GetIndex();

    SwASC_AttrIter aAttrIter( (SwASCWriter&)rWrt, rNd, nStrPos );
    rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();
    rtl_TextEncoding eSet = rWrt.GetAsciiOptions().GetCharSet();

    if( !nStrPos )
        ::lcl_WriteString( rWrt.Strm(), rNd.GetNumString(), eChrSet, eSet );

    String aStr( rNd.GetTxt() );
    if( rWrt.bASCII_ParaAsBlanc )
        aStr.SearchAndReplaceAll( 0x0A, ' ' );

    do {
        xub_StrLen nNextAttr = aAttrIter.WhereNext();
        rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

        if( nNextAttr > nEnde )
            nNextAttr = nEnde;

        if( !aAttrIter.OutAttr( nStrPos, eChrSet ))
            ::lcl_WriteString( rWrt.Strm(),
                                aStr.Copy( nStrPos, nNextAttr - nStrPos ),
                                  eChrSet, eSet );
        nStrPos = nNextAttr;
        eChrSet = eNextChrSet;
        aAttrIter.NextPos();
    } while( nStrPos < nEnde );

    if( !bLastNd ||
        ( !rWrt.bWriteClipboardDoc && !rWrt.bASCII_NoLastLineEnd )
            && !nStrPos && nEnde == nNodeEnde )
        ::lcl_WriteString( rWrt.Strm(), ((SwASCWriter&)rWrt).GetLineEnd(),
                              RTL_TEXTENCODING_MS_1252, eSet );

    return rWrt;
}

void lcl_WriteString( SvStream& rStream, const String& rStr,
                      rtl_TextEncoding eSrcSet, rtl_TextEncoding eDestSet )
{
    if( rStr.Len() )
    {
        if( RTL_TEXTENCODING_UCS2 == eDestSet )
            rStream.Write( rStr.GetBuffer(), rStr.Len() * sizeof( sal_Unicode ));
        else if( RTL_TEXTENCODING_UCS4 == eDestSet )
        {
            // ????
        }
        else
        {
            ByteString sOut( rStr, eDestSet );
            rStream << sOut.GetBuffer();
        }
    }
}

/*
 * lege hier jetzt die Tabellen fuer die ASCII-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der ASCII-DLL
 * bekannt sein muessen.
 */

SwNodeFnTab aASCNodeFnTab = {
/* RES_TXTNODE  */                   OutASC_SwTxtNode,
/* RES_GRFNODE  */                   0,
/* RES_OLENODE  */                   0
};

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ascii/ascatr.cxx,v 1.1.1.1 2000-09-18 17:14:53 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.59  2000/09/18 16:04:38  willem.vandorp
      OpenOffice header added.

      Revision 1.58  2000/05/08 17:36:58  jp
      Changes for Unicode

      Revision 1.57  2000/02/11 14:36:19  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.56  1999/11/01 17:49:58  jp
      own convert from char to UNICODE char

      Revision 1.55  1999/11/01 11:11:14  jp
      call string-convert with the CharSet of the font

      Revision 1.54  1999/08/26 18:34:10  JP
      load and save Text with more options (charset/language/lineend/font)


      Rev 1.53   26 Aug 1999 20:34:10   JP
   load and save Text with more options (charset/language/lineend/font)

      Rev 1.52   27 Jun 1998 16:01:50   JP
   Writer mit neuen Flags; fuer den ASCII-Writer, etwas aufgeraeumt

      Rev 1.51   27 Jan 1998 19:37:12   HR
   Ueberfluessige Klammern weg, HPUX mag sie nicht

      Rev 1.50   25 Nov 1997 18:55:18   MA
   includes

      Rev 1.49   03 Nov 1997 14:03:46   MA
   precomp entfernt

      Rev 1.48   10 Oct 1997 12:19:46   JP
   beim Clipboard das letzte CR/LF nicht schreiben

      Rev 1.47   14 Jan 1997 08:43:32   MA
   includes

      Rev 1.46   23 Oct 1996 19:49:00   JP
   String Umstellung: [] -> GetChar()

      Rev 1.45   28 Jun 1996 13:59:08   MA
   includes

      Rev 1.44   03 Apr 1996 12:32:26   JP
   OutSwTxtNode: nur im letzten Node entscheiden, ob CRLF geschrieben wird

      Rev 1.43   26 Mar 1996 15:24:08   JP
   Bugfix: unbeannte Zeichen richtig schreiben

      Rev 1.42   13 Feb 1996 19:41:30   JP
   Bug #25428#: LineEnd von SV holen und entsp. auswerten

      Rev 1.41   31 Jan 1996 14:27:36   mk
   SINIX

      Rev 1.40   24 Nov 1995 17:24:46   OM
   PCH->PRECOMPILED

      Rev 1.39   30 Oct 1995 13:03:38   MA
   chg: ConvertUnknown direkt eingebaut.

      Rev 1.38   02 Oct 1995 13:10:48   JP
   Umstellung auf schreiben verschiedener CodeSets

      Rev 1.37   25 Oct 1994 15:10:22   MA
   PreHdr.

      Rev 1.36   21 Jul 1994 09:29:52   JP
   OutASC_SwTxtNode: fuer den ZeilenUmbruch im Translate die Laenge vom Node
       benutzen und nicht die vom String. Stehen Felder/Nummern/usw. im String
       kann es nie passen!

      Rev 1.35   04 Mar 1994 10:58:28   JP
   Translate: auf const char* casten.

      Rev 1.34   25 Feb 1994 12:54:46   JP
   schreibe mit expandierten Feldern und Numerierung

      Rev 1.33   15 Feb 1994 08:24:10   MI
   Filter ab jetzt handsegmentiert

      Rev 1.32   28 Jan 1994 11:28:36   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.31   13 Jan 1994 08:28:28   MI
   Segmentierung per #define ermoeglicht

      Rev 1.30   12 Jan 1994 11:22:46   MI
   Segmentierungs-Probleme

*************************************************************************/
