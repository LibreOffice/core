/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ascatr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:43:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

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

class SwASC_AttrIter
{
    SwASCWriter& rWrt;
    const SwTxtNode& rNd;
    xub_StrLen nAktSwPos;

    xub_StrLen SearchNext( xub_StrLen nStartPos );

public:
    SwASC_AttrIter( SwASCWriter& rWrt, const SwTxtNode& rNd, xub_StrLen nStt );

    void NextPos()      { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    xub_StrLen WhereNext() const        { return nAktSwPos; }
    BOOL OutAttr( xub_StrLen nSwPos );
};


SwASC_AttrIter::SwASC_AttrIter( SwASCWriter& rWr, const SwTxtNode& rTxtNd,
                                xub_StrLen nStt )
    : rWrt( rWr ), rNd( rTxtNd ), nAktSwPos( 0 )
{
    nAktSwPos = SearchNext( nStt + 1 );
}


xub_StrLen SwASC_AttrIter::SearchNext( xub_StrLen nStartPos )
{
    xub_StrLen nMinPos = STRING_MAXLEN;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        USHORT i;
        xub_StrLen nPos;
        const xub_StrLen * pPos;

// kann noch optimiert werden, wenn ausgenutzt wird, dass die TxtAttrs
// nach der Anfangsposition geordnet sind. Dann muessten
// allerdings noch 2 Indices gemerkt werden
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            nPos = *pHt->GetStart();    // gibt erstes Attr-Zeichen
            pPos = pHt->GetEnd();
            if( !pPos )
            {
                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;

                if( ( ++nPos ) >= nStartPos && nPos < nMinPos )
                    nMinPos = nPos;
            }
        }
    }
    return nMinPos;
}


BOOL SwASC_AttrIter::OutAttr( xub_StrLen nSwPos )
{
    BOOL bRet = FALSE;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        USHORT i;
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
                    rWrt.Strm().WriteUnicodeOrByteText( sOut );
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

    if( !nStrPos )
        rWrt.Strm().WriteUnicodeOrByteText( rNd.GetNumString() );

    String aStr( rNd.GetTxt() );
    if( rWrt.bASCII_ParaAsBlanc )
        aStr.SearchAndReplaceAll( 0x0A, ' ' );

    const bool bExportSoftHyphens = RTL_TEXTENCODING_UCS2 == rWrt.GetAsciiOptions().GetCharSet() ||
                                    RTL_TEXTENCODING_UTF8 == rWrt.GetAsciiOptions().GetCharSet();

    do {
        xub_StrLen nNextAttr = aAttrIter.WhereNext();

        if( nNextAttr > nEnde )
            nNextAttr = nEnde;

        if( !aAttrIter.OutAttr( nStrPos ))
        {
            String aOutStr( aStr.Copy( nStrPos, nNextAttr - nStrPos ) );
            if ( !bExportSoftHyphens )
                aOutStr.EraseAllChars( CHAR_SOFTHYPHEN );

            rWrt.Strm().WriteUnicodeOrByteText( aOutStr );
        }
        nStrPos = nNextAttr;
        aAttrIter.NextPos();
    } while( nStrPos < nEnde );

    if( !bLastNd ||
        ( !rWrt.bWriteClipboardDoc && !rWrt.bASCII_NoLastLineEnd )
            && !nStrPos && nEnde == nNodeEnde )
        rWrt.Strm().WriteUnicodeOrByteText( ((SwASCWriter&)rWrt).GetLineEnd());

    return rWrt;
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

