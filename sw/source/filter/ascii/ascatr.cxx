/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <hintids.hxx>
#include <tools/stream.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <editeng/fontitem.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <wrtasc.hxx>
#include <txatbase.hxx>
#include <fchrfmt.hxx>
#include <txtfld.hxx>
#include <txtatr.hxx>
#include <fmtftn.hxx>
#include <charfmt.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <ftninfo.hxx>

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

    void NextPos()
    {
        nAktSwPos = SearchNext( nAktSwPos + 1 );
    }

    xub_StrLen WhereNext() const
    {
        return nAktSwPos;
    }

    sal_Bool OutAttr( xub_StrLen nSwPos );
};


SwASC_AttrIter::SwASC_AttrIter(
    SwASCWriter& rWr,
    const SwTxtNode& rTxtNd,
    xub_StrLen nStt )
    : rWrt( rWr )
    , rNd( rTxtNd )
    , nAktSwPos( 0 )
{
    nAktSwPos = SearchNext( nStt + 1 );
}


xub_StrLen SwASC_AttrIter::SearchNext( xub_StrLen nStartPos )
{
    xub_StrLen nMinPos = STRING_MAXLEN;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        for ( sal_uInt16 i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( pHt->HasDummyChar() )
            {
                xub_StrLen nPos = *pHt->GetStart();

                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;

                if( ( ++nPos ) >= nStartPos && nPos < nMinPos )
                    nMinPos = nPos;
            }
            else if ( pHt->HasContent() )
            {
                const xub_StrLen nHintStart = *pHt->GetStart();
                if ( nHintStart >= nStartPos && nHintStart <= nMinPos )
                {
                    nMinPos = nHintStart;
                }

                const xub_StrLen nHintEnd = pHt->End() ? *pHt->End() : STRING_MAXLEN;
                if ( nHintEnd >= nStartPos && nHintEnd < nMinPos )
                {
                    nMinPos = nHintEnd;
                }
            }
        }
    }
    return nMinPos;
}


sal_Bool SwASC_AttrIter::OutAttr( xub_StrLen nSwPos )
{
    sal_Bool bRet = sal_False;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        sal_uInt16 i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( ( pHt->HasDummyChar()
                   || pHt->HasContent() )
                 && nSwPos == *pHt->GetStart() )
            {
                bRet = sal_True;
                String sOut;
                switch( pHt->Which() )
                {
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                case RES_TXTATR_INPUTFIELD:
                    sOut = static_cast<SwTxtFld const*>(pHt)->GetFmtFld().GetField()->ExpandField(true);
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
    sal_Bool bLastNd =  rWrt.pCurPam->GetPoint()->nNode == rWrt.pCurPam->GetMark()->nNode;
    if( bLastNd )
        nEnde = rWrt.pCurPam->GetMark()->nContent.GetIndex();

    SwASC_AttrIter aAttrIter( (SwASCWriter&)rWrt, rNd, nStrPos );

    if( !nStrPos && rWrt.bExportPargraphNumbering )
    {
        String numString( rNd.GetNumString() );
        if (numString.Len())
        {
            numString.Append(' ');
            rWrt.Strm().WriteUnicodeOrByteText(numString);
        }
    }

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
        ( ( !rWrt.bWriteClipboardDoc && !rWrt.bASCII_NoLastLineEnd )
            && !nStrPos && nEnde == nNodeEnde ) )
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

