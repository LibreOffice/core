/*************************************************************************
 *
 *  $RCSfile: wrtw8nds.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 13:43:39 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen wg. SvxULSpaceItem
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMATR_HXX //autogen
#include <frmatr.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen wg. SwFmtINetFmt
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen wg. SwFmtRefMark
#include <fmtrfmrk.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen wg. SwFmtCharFmt
#include <fchrfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _TOX_HXX //autogen wg. SwTOXMark
#include <tox.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>           // SwPosFlyFrms
#endif
#ifndef _REFFLD_HXX //autogen wg. SwGetRefField
#include <reffld.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _TXTTXMRK_HXX
#include <txttxmrk.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif

/*  */

WW8_AttrIter::WW8_AttrIter( SwWW8Writer& rWr )
    : rWrt( rWr ), pOld( rWr.pChpIter )
{
    rWrt.pChpIter = this;
}

WW8_AttrIter::~WW8_AttrIter()
{
    rWrt.pChpIter = pOld;
}

void WW8_AttrIter::GetItems( WW8Bytes& rItems ) const
{
}


/*  */

// Die Klasse WW8_SwAttrIter ist eine Hilfe zum Aufbauen der Fkp.chpx.
// Dabei werden nur Zeichen-Attribute beachtet; Absatz-Attribute brauchen
// diese Behandlung nicht.
// Die Absatz- und Textattribute des Writers kommen rein, und es wird
// mit Where() die naechste Position geliefert, an der sich die Attribute
// aendern. IsTxtAtr() sagt, ob sich an der mit Where() gelieferten Position
// ein Attribut ohne Ende und mit \xff im Text befindet.
// Mit OutAttr() werden die Attribute an der angegebenen SwPos
// ausgegeben.
class WW8_SwAttrIter : public WW8_AttrIter
{
    const SwTxtNode& rNd;
    SvPtrarr aTxtAtrArr;
    SvUShorts aChrSetArr;
    const SwRedline* pCurRedline;
    xub_StrLen nAktSwPos;
    xub_StrLen nTmpSwPos;                   // fuer HasItem()
    USHORT nCurRedlinePos;
    rtl_TextEncoding eNdChrSet;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    void SetCharSet( const SwTxtAttr& rTxtAttr, BOOL bStart );
    void FieldVanish( const String& rTxt );

    void OutSwFmtINetFmt( const SwFmtINetFmt& rAttr, BOOL bStart );
    void OutSwFmtRefMark( const SwFmtRefMark& rAttr, BOOL bStart );
    void OutSwTOXMark( const SwTOXMark& rAttr, BOOL bStart );

public:
    WW8_SwAttrIter( SwWW8Writer& rWr, const SwTxtNode& rNd );

    BOOL IsTxtAttr( xub_StrLen nSwPos );
    BOOL IsRedlineAtEnd( xub_StrLen nPos ) const;

    void NextPos() { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( xub_StrLen nSwPos );
    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const;
    virtual void GetItems( WW8Bytes& rItems ) const;
    BOOL OutAttrWithRange( xub_StrLen nPos );
    void OutRedlines( xub_StrLen nPos );

    xub_StrLen WhereNext() const                    { return nAktSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const             { return eNdChrSet; }
};


WW8_SwAttrIter::WW8_SwAttrIter( SwWW8Writer& rWr, const SwTxtNode& rTxtNd )
    : WW8_AttrIter( rWr ), rNd( rTxtNd ), nAktSwPos( 0 ), nTmpSwPos( 0 ),
    aTxtAtrArr( 0, 4 ), aChrSetArr( 0, 4 ), nCurRedlinePos( USHRT_MAX ),
    pCurRedline( 0 )
{
    // Attributwechsel an Pos 0 wird ignoriert, da davon ausgegangen
    // wird, dass am Absatzanfang sowieso die Attribute neu ausgegeben
    // werden.
    eNdChrSet = ((SvxFontItem&)rNd.SwCntntNode::GetAttr(
                                        RES_CHRATR_FONT )).GetCharSet();
    if( rWrt.pDoc->GetRedlineTbl().Count() )
    {
        SwPosition aPos( rNd, SwIndex( (SwTxtNode*)&rNd ) );
        pCurRedline = rWrt.pDoc->GetRedline( aPos, &nCurRedlinePos );
    }
    nAktSwPos = SearchNext( 1 );
}

rtl_TextEncoding WW8_SwAttrIter::GetNextCharSet() const
{
    if( aChrSetArr.Count() )
        return (rtl_TextEncoding)aChrSetArr[ aChrSetArr.Count() - 1 ];
    return eNdChrSet;
}

xub_StrLen WW8_SwAttrIter::SearchNext( xub_StrLen nStartPos )
{
    register xub_StrLen nPos;
    register xub_StrLen nMinPos = STRING_MAXLEN;
    register xub_StrLen i;

    // first the redline, then the attributes
    if( pCurRedline )
    {
        const SwPosition* pEnd = pCurRedline->End();
        if( pEnd->nNode == rNd &&
            ( i = pEnd->nContent.GetIndex() ) >= nStartPos )
            nMinPos = i;
    }

    if( nCurRedlinePos < rWrt.pDoc->GetRedlineTbl().Count() )
    {
        // nCurRedlinePos point to the next redline
        nPos = nCurRedlinePos;
        if( pCurRedline )
            ++nPos;

        for( ; nPos < rWrt.pDoc->GetRedlineTbl().Count(); ++nPos )
        {
            const SwRedline* pRedl = rWrt.pDoc->GetRedlineTbl()[ nPos ];

            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();

            if( pStt->nNode == rNd )
            {
                if( ( i = pStt->nContent.GetIndex() ) >= nStartPos &&
                    i < nMinPos )
                    nMinPos = i;
            }
            else
                break;

            if( pEnd->nNode == rNd &&
                ( i = pEnd->nContent.GetIndex() ) < nMinPos &&
                i >= nStartPos )
                    nMinPos = i;
        }
    }

    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {

// kann noch optimiert werden, wenn ausgenutzt wird, dass die TxtAttrs
// nach der Anfangsposition geordnet sind. Dann muessten
// allerdings noch 2 Indices gemerkt werden
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            nPos = *pHt->GetStart();    // gibt erstes Attr-Zeichen
            if( nPos >= nStartPos && nPos <= nMinPos )
            {
                nMinPos = nPos;
                SetCharSet( *pHt, TRUE );
            }

            if( pHt->GetEnd() )         // Attr mit Ende
            {
                nPos = *pHt->GetEnd();      // gibt letztes Attr-Zeichen + 1
                if( nPos >= nStartPos && nPos <= nMinPos )
                {
                    nMinPos = nPos;
                    SetCharSet( *pHt, FALSE );
                }
            }
            else
            {                                   // Attr ohne Ende
                nPos = *pHt->GetStart() + 1;    // Laenge 1 wegen CH_TXTATR im Text
                if( nPos >= nStartPos && nPos <= nMinPos )
                {
                    nMinPos = nPos;
                    SetCharSet( *pHt, FALSE );
                }
            }
        }
    }
    return nMinPos;
}

void WW8_SwAttrIter::SetCharSet( const SwTxtAttr& rAttr, BOOL bStart )
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

//  case RES_TXTATR_INETFMT:
//      break;
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
}

void WW8_SwAttrIter::OutAttr( xub_StrLen nSwPos )
{
    if( rNd.GetpSwAttrSet() )
        rWrt.Out_SfxItemSet( *rNd.GetpSwAttrSet(), FALSE, TRUE );

    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        const SwModify* pOldMod = rWrt.pOutFmtNode;
        rWrt.pOutFmtNode = &rNd;

        nTmpSwPos = nSwPos;
        register xub_StrLen i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const xub_StrLen* pEnd = pHt->GetEnd();

            if( pEnd ? ( nSwPos >= *pHt->GetStart() && nSwPos < *pEnd )
                        : nSwPos == *pHt->GetStart() )
                Out( aWW8AttrFnTab, pHt->GetAttr(), rWrt );
            else if( nSwPos < *pHt->GetStart() )
                break;
        }

        nTmpSwPos = 0;      // HasTextItem nur in dem obigen Bereich erlaubt
        rWrt.pOutFmtNode = pOldMod;
    }

    OutRedlines( nSwPos );
}

BOOL WW8_SwAttrIter::IsTxtAttr( xub_StrLen nSwPos )
{
    // search for attrs without end position
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        register USHORT i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if( !pHt->GetEnd() && *pHt->GetStart() == nSwPos )
                return TRUE;
        }
    }
    return FALSE;
}

// HasItem ist fuer die Zusammenfassung des Doppel-Attributes Underline
// und WordLineMode als TextItems. OutAttr() ruft die Ausgabefunktion,
// die dann ueber HasItem() nach anderen Items an der
// Attribut-Anfangposition fragen kann.
// Es koennen nur Attribute mit Ende abgefragt werden.
// Es wird mit bDeep gesucht
const SfxPoolItem* WW8_SwAttrIter::HasTextItem( USHORT nWhich ) const
{
    const SfxPoolItem* pRet = 0;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( !nTmpSwPos && !pTxtAttrs )
    {
        ASSERT( !this, "HasTextItem nicht aus OutAttr gerufen !" );
    }
    else if( pTxtAttrs )
    {
        register USHORT i;
        for( i = 0; i < pTxtAttrs->Count(); ++i )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            const xub_StrLen* pAtrEnd = 0;
            if( 0 != ( pAtrEnd = pHt->GetEnd() ) &&     // nur Attr mit Ende
                nWhich == pItem->Which() &&             //
                nTmpSwPos >= *pHt->GetStart() && nTmpSwPos < *pAtrEnd )
            {
                pRet = pItem;       // gefunden
                break;
            }
            else if( nTmpSwPos < *pHt->GetStart() )
                break;              // dann kommt da nichts mehr
        }
    }
    return pRet;
}

void WW8_SwAttrIter::GetItems( WW8Bytes& rItems ) const
{
    WW8Bytes* pO_Sav = rWrt.pO;
    rWrt.pO = &rItems;

    if( rNd.GetpSwAttrSet() )
        rWrt.Out_SfxItemSet( *rNd.GetpSwAttrSet(), FALSE, TRUE );

    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        const SwModify* pOldMod = rWrt.pOutFmtNode;
        rWrt.pOutFmtNode = &rNd;

        register USHORT i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const xub_StrLen* pEnd = pHt->GetEnd();

            if( pEnd && nTmpSwPos >= *pHt->GetStart() && nTmpSwPos < *pEnd )
                Out( aWW8AttrFnTab, pHt->GetAttr(), rWrt );
            else if( nTmpSwPos < *pHt->GetStart() )
                break;
        }

        rWrt.pOutFmtNode = pOldMod;
    }

//  OutRedlines( nTmpSwPos );
    rWrt.pO = pO_Sav;
}

const SfxPoolItem& WW8_SwAttrIter::GetItem( USHORT nWhich ) const
{
    const SfxPoolItem* pRet = HasTextItem( nWhich );
    if( !pRet )
        pRet = &rNd.SwCntntNode::GetAttr( nWhich );
    return *pRet;
}

void WW8_SwAttrIter::OutSwFmtINetFmt( const SwFmtINetFmt& rINet, BOOL bStart )
{
    if( bStart )
    {
        INetURLObject aURL( rINet.GetValue() );
        String sURL( aURL.GetURLNoMark(INetURLObject::DECODE_UNAMBIGUOUS) );
        String sMark( aURL.GetMark(INetURLObject::DECODE_UNAMBIGUOUS) );

        sURL = INetURLObject::AbsToRel( sURL, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS);
        sURL.Insert( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "HYPERLINK \"" )), 0 );
        sURL += '\"';

        if( sMark.Len() )
            (( sURL.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " \\l \"" )))
                    += sMark ) += '\"';

        if( rINet.GetTargetFrame().Len() )
            ( sURL.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " \\n " )))
                    += rINet.GetTargetFrame();

        rWrt.OutField( 0, 88, sURL,
                    WRITEFIELD_START | WRITEFIELD_CMD_START );

        // write the refence to the "picture" structure
        ULONG nDataStt = rWrt.pDataStrm->Tell();
        rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell() );

//I'm leaving this as a define for now for easy removal of this code
//C.
#define WRITE_HYPERLINK_IN_DATA_STREAM

#ifdef WRITE_HYPERLINK_IN_DATA_STREAM
//WinWord 2000 doesn't write this - so its a temp solution by W97
        rWrt.WriteChar( 0x01 );

        static BYTE aArr1[] = {
            0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

            0x06, 0x08, 0x01,       // sprmCFData
            0x55, 0x08, 0x01,       // sprmCFSpec
            0x02, 0x08, 0x01        // sprmCFFldVanish
        };
        BYTE* pDataAdr = aArr1 + 2;
        Set_UInt32( pDataAdr, nDataStt );

        rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                    sizeof( aArr1 ), aArr1 );
#endif

        rWrt.OutField( 0, 88, sURL, WRITEFIELD_CMD_END );


#ifdef WRITE_HYPERLINK_IN_DATA_STREAM
        // now write the picture structur
        sURL = aURL.GetURLNoMark();

        BOOL bAbsolute = TRUE;  //all links end up in the data stream as
                                //absolute references.
        INetProtocol aProto = aURL.GetProtocol();

        static BYTE __READONLY_DATA aURLData1[] = {
            0,0,0,0,        // len of struct
            0x44,0,         // the start of "next" data
            0,0,0,0,0,0,0,0,0,0,                // PIC-Structure!
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
            0,0,0,0,                            // /
        };
        static BYTE __READONLY_DATA MAGIC_A[] = {
            // start of "next" data
            0xD0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
            0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
        };

        rWrt.pDataStrm->Write( aURLData1, sizeof(aURLData1) );
        BYTE nAnchor=0x00;
        if( sMark.Len() )
            nAnchor=0x08;
        rWrt.pDataStrm->Write( &nAnchor, 1 );
        rWrt.pDataStrm->Write( MAGIC_A, sizeof(MAGIC_A) );
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, 0x00000002);
        UINT32 nFlag=0x01;
        if (bAbsolute) nFlag |= 0x02;
        if( sMark.Len() ) nFlag |= 0x08;
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, nFlag );

        if (aProto == INET_PROT_FILE)
        {
// version 1 (for a document)

            static BYTE __READONLY_DATA MAGIC_C[] = {
                0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
                0x00, 0x00
            };

            static BYTE __READONLY_DATA MAGIC_D[] = {
                0xFF, 0xFF, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            sURL = aURL.PathToFileName();

            rWrt.pDataStrm->Write( MAGIC_C, sizeof(MAGIC_C) );
            SwWW8Writer::WriteLong( *rWrt.pDataStrm, sURL.Len()+1 );
            SwWW8Writer::WriteString8( *rWrt.pDataStrm, sURL, TRUE,
                                        RTL_TEXTENCODING_MS_1252 );
            rWrt.pDataStrm->Write( MAGIC_D, sizeof( MAGIC_D) );

            SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2*sURL.Len()+6);
            SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2*sURL.Len());
            SwWW8Writer::WriteShort( *rWrt.pDataStrm, 3 );
            SwWW8Writer::WriteString16( *rWrt.pDataStrm, sURL, FALSE );
        }
        else
        {
            // version 2 (simple url)
            // an write some data to the data stream, but dont ask
            // what the data mean, except for the URL.
            // The First piece is the WW8_PIC structure.
            //
            static BYTE __READONLY_DATA MAGIC_B[] = {
                0xE0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
                0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
            };

            rWrt.pDataStrm->Write( MAGIC_B, sizeof(MAGIC_B) );
            SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2 * (sURL.Len()+1) );
            SwWW8Writer::WriteString16( *rWrt.pDataStrm, sURL, TRUE );
        }

        if( sMark.Len() )
        {
            SwWW8Writer::WriteLong( *rWrt.pDataStrm, sMark.Len()+1 );
            SwWW8Writer::WriteString16( *rWrt.pDataStrm, sMark, TRUE );
        }
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, nDataStt,
            rWrt.pDataStrm->Tell() - nDataStt );
#endif

    }
    else
        rWrt.OutField( 0, 0, aEmptyStr, WRITEFIELD_CLOSE );
}


void WW8_SwAttrIter::OutSwFmtRefMark( const SwFmtRefMark& rAttr, BOOL bStart )
{
    if( rWrt.HasRefToObject( REF_SETREFATTR, &rAttr.GetRefName(), 0 ))
        rWrt.AppendBookmark( rWrt.GetBookmarkName( REF_SETREFATTR,
                                            &rAttr.GetRefName(), 0 ));
}

void WW8_SwAttrIter::FieldVanish( const String& rTxt )
{
    WW8Bytes aItems;
    rWrt.pChpIter->GetItems( aItems );
    USHORT nStt_TxtAtt = aItems.Count();

    // sprmCFFldVanish
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x802 );
    else
        aItems.Insert( 67, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );

    USHORT nStt_sprmCFSpec = aItems.Count();

    // sprmCFSpec --  fSpec-Attribut TRUE
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x855 );
    else
        aItems.Insert( 117, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );


//  rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), nStt_TxtAtt,
//                                  aItems.GetData() );
    rWrt.WriteChar( '\x13' );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );
    rWrt.OutSwString( rTxt, 0, rTxt.Len(), rWrt.IsUnicode(),
                        RTL_TEXTENCODING_MS_1252 );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), nStt_sprmCFSpec,
                                    aItems.GetData() );
    rWrt.WriteChar( '\x15' );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );
}

void WW8_SwAttrIter::OutSwTOXMark( const SwTOXMark& rAttr, BOOL bStart )
{
    // its a field; so get the Text form the Node and build the field
    ASSERT( !bStart, "calls only with the endposition!" );
    String sTxt;

    const SwTxtTOXMark& rTxtTOXMark = *rAttr.GetTxtTOXMark();
    const xub_StrLen* pTxtEnd = rTxtTOXMark.GetEnd();
    if( pTxtEnd )       // has range?
        sTxt = rNd.GetExpandTxt( *rTxtTOXMark.GetStart(),
                                 *pTxtEnd  - *rTxtTOXMark.GetStart() );
    else
        sTxt = rAttr.GetAlternativeText();

    switch( rAttr.GetTOXType()->GetType() )
    {
    case TOX_INDEX:
        if( rAttr.GetPrimaryKey().Len() )
        {
            sTxt.Insert( ':', 0 );
            sTxt.Insert( rAttr.GetPrimaryKey(), 0 );
        }
        sTxt.InsertAscii( " XE \"", 0 );
        sTxt.InsertAscii( "\" " );
        break;

    case TOX_USER:
        ( sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\" \\f \"" )))
                += (sal_Char)( 'A' + rWrt.GetId( *rAttr.GetTOXType() ));
        // kein break;
    case TOX_CONTENT:
        {
            sTxt.InsertAscii( " INHALT \"", 0 );
            USHORT nLvl = rAttr.GetLevel();
            if( nLvl > nWW8MaxListLevel )
                nLvl = nWW8MaxListLevel;

            ((sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\" \\l " )))
                += String::CreateFromInt32( nLvl )) += ' ';
        }
        break;
    }

    if( sTxt.Len() )
        FieldVanish( sTxt );
}

BOOL WW8_SwAttrIter::OutAttrWithRange( xub_StrLen nPos )
{
    BOOL bRet = FALSE;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        nTmpSwPos = nPos;
        const xub_StrLen* pEnd;
        register USHORT i;
        for( i = 0; i < pTxtAttrs->Count(); ++i )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            switch( pItem->Which() )
            {
            case RES_TXTATR_INETFMT:
                if( rWrt.bWrtWW8 )  // nur WW8 kann Hyperlinks
                {
                    if( nPos == *pHt->GetStart() )
                    {
                        OutSwFmtINetFmt( (SwFmtINetFmt&)*pItem, TRUE );
                        bRet = TRUE;
                    }
                    else if( nPos == *pHt->GetEnd() )
                        OutSwFmtINetFmt( (SwFmtINetFmt&)*pItem, FALSE );
                }
                break;

            case RES_TXTATR_REFMARK:
                if( nPos == *pHt->GetStart() )
                    OutSwFmtRefMark( (SwFmtRefMark&)*pItem, TRUE );
                if( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                    OutSwFmtRefMark( (SwFmtRefMark&)*pItem, FALSE );
                break;

            case RES_TXTATR_TOXMARK:
                if( 0 != ( pEnd = pHt->GetEnd() ) ? nPos == *pEnd
                                                  : nPos == *pHt->GetStart())
                    OutSwTOXMark( (SwTOXMark&)*pItem, FALSE );
                break;
            }
        }
        nTmpSwPos = 0;      // HasTextItem nur in dem obigen Bereich erlaubt
    }
    return bRet;
}

BOOL WW8_SwAttrIter::IsRedlineAtEnd( xub_StrLen nEnd ) const
{
    BOOL bRet = FALSE;
    // search next Redline
    for( USHORT nPos = nCurRedlinePos;
        nPos < rWrt.pDoc->GetRedlineTbl().Count(); ++nPos )
    {
        const SwPosition* pEnd = rWrt.pDoc->GetRedlineTbl()[ nPos ]->End();
        if( pEnd->nNode == rNd )
        {
            if( pEnd->nContent.GetIndex() == nEnd )
            {
                bRet = TRUE;
                break;
            }
        }
        else
            break;
    }
    return bRet;
}

void WW8_SwAttrIter::OutRedlines( xub_StrLen nPos )
{
    if( pCurRedline )
    {
        const SwPosition* pEnd = pCurRedline->End();
        if( pEnd->nNode == rNd &&
            pEnd->nContent.GetIndex() <= nPos )
        {
            pCurRedline = 0;
            ++nCurRedlinePos;
        }
        else
            // write data of current redline
            rWrt.OutRedline( pCurRedline->GetRedlineData() );
    }

    if( !pCurRedline )
    {
        // search next Redline
        for( ; nCurRedlinePos < rWrt.pDoc->GetRedlineTbl().Count();
                ++nCurRedlinePos )
        {
            const SwRedline* pRedl = rWrt.pDoc->GetRedlineTbl()[ nCurRedlinePos ];

            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();

            if( pStt->nNode == rNd )
            {
                if( pStt->nContent.GetIndex() >= nPos )
                {
                    if( pStt->nContent.GetIndex() == nPos )
                    {
                        // write data of this redline
                        pCurRedline = pRedl;
                        rWrt.OutRedline( pCurRedline->GetRedlineData() );
                    }
                    break;
                }
            }
            else
                break;

            if( pEnd->nNode == rNd &&
                pEnd->nContent.GetIndex() < nPos )
            {
                pCurRedline = pRedl;
                break;
            }
        }
    }
}

/*  */

Writer& OutWW8_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    SwTxtNode* pNd = &((SwTxtNode&)rNode);

    BOOL bFlyInTable = rWW8Wrt.pFlyFmt && rWW8Wrt.bIsInTable;

    // akt. Style
    if( !bFlyInTable )
        rWW8Wrt.nStyleBeforeFly
            = rWW8Wrt.GetId( (SwTxtFmtColl&)pNd->GetAnyFmtColl() );

    SVBT16 nSty;
    ShortToSVBT16( rWW8Wrt.nStyleBeforeFly, nSty );

    WW8Bytes* pO = rWW8Wrt.pO;

    if( rWW8Wrt.bStartTOX )
    {
        // ignore TOX header section
        const SwSectionNode* pSectNd = rNode.FindSectionNode();
        if( TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
            rWW8Wrt.StartTOX( pSectNd->GetSection() );
    }

    const SwSection* pTOXSect = 0;
    if( rWW8Wrt.bInWriteTOX )
    {
        // check for end of TOX
        SwNodeIndex aIdx( *pNd, 1 );
        if( !aIdx.GetNode().IsTxtNode() )
        {
            const SwSectionNode* pTOXSectNd = rNode.FindSectionNode();
            pTOXSect = &pTOXSectNd->GetSection();

            const SwNode* pNxt = rNode.GetNodes().GoNext( &aIdx );
            if( pNxt && pNxt->FindSectionNode() == pTOXSectNd )
                pTOXSect = 0;
        }
    }

    WW8_SwAttrIter aAttrIter( rWW8Wrt, *pNd );
    rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();

    ASSERT( !pO->Count(), " pO ist am Zeilenanfang nicht leer" );

    String aStr( pNd->GetTxt() );

    // 0x0a     ( Hard Line Break ) -> 0x0b
    // 0xad     ( soft hyphen )     -> 0x1f
    // 0x2011   ( hard hyphen )     -> 0x1e
    if( aStr.Len() )
    {
        aStr.SearchAndReplaceAll( 0x0A, 0x0B );
        aStr.SearchAndReplaceAll( CHAR_HARDHYPHEN, 0x1e );
        aStr.SearchAndReplaceAll( CHAR_SOFTHYPHEN, 0x1f );
    }
    xub_StrLen nAktPos = 0;
    xub_StrLen nEnd = aStr.Len();
    BOOL bUnicode = rWW8Wrt.bWrtWW8, bRedlineAtEnd = FALSE;
    do {
/*
        if( !bUseAlwaysUnicode )
        {
            switch( eChrSet )
            {
//          case CHARSET_DONTKNOW:       ????
            case CHARSET_SYMBOL:
//          case CHARSET_WIN_SYMBOL:
//          case CHARSET_WIN_WINGDINGS:
//          case CHARSET_MAC_DINGBATS:
//          case CHARSET_MAC_SYMBOL:
//          case CHARSET_ADOBE_SYMBOL:
//          case CHARSET_ADOBE_DINGBATS:
//          case CHARSET_STAR_STARBATS:
//          case CHARSET_STAR_STARMATH:
                bUnicode = rWW8Wrt.bWrtWW8 ? TRUE : FALSE;
                break;
            default:
                bUnicode = FALSE;
                break;
            }
        }
*/
        xub_StrLen nNextAttr = aAttrIter.WhereNext();
        rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

        if( nNextAttr > nEnd )
            nNextAttr = nEnd;

        rWW8Wrt.AppendBookmarks( *pNd, nAktPos, nNextAttr - nAktPos );
        BOOL bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
        BOOL bAttrWithRange = aAttrIter.OutAttrWithRange( nAktPos );

        if( !bTxtAtr )
            rWW8Wrt.OutSwString( aStr, nAktPos, nNextAttr - nAktPos,
                                    bUnicode, eChrSet );

                    // Am Zeilenende werden die Attribute bis ueber das CR
                    // aufgezogen. Ausnahme: Fussnoten am Zeilenende
        if( nNextAttr == nEnd )
        {
            if( !bTxtAtr && !bAttrWithRange )
            {
                if( aAttrIter.IsRedlineAtEnd( nEnd ))
                    bRedlineAtEnd = TRUE;
                else
                {
                    if( pTOXSect )
                        rWW8Wrt.EndTOX( *pTOXSect );
                    rWW8Wrt.WriteCR();              // CR danach
                }
            }
        }
                                        // Ausgabe der Zeichenattribute
        aAttrIter.OutAttr( nAktPos );   // nAktPos - 1 ??
        rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                            pO->Count(), pO->GetData() );
        pO->Remove( 0, pO->Count() );                   // leeren

                    // Ausnahme: Fussnoten am Zeilenende
        if( nNextAttr == nEnd )
        {
            aAttrIter.OutAttrWithRange( nEnd );
            if( pO->Count() )
            {
                rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                            pO->Count(), pO->GetData() );
                pO->Remove( 0, pO->Count() );                   // leeren
            }

            if( bTxtAtr || bAttrWithRange || bRedlineAtEnd )
            {
                if( pTOXSect )
                    rWW8Wrt.EndTOX( *pTOXSect );

                rWW8Wrt.WriteCR();              // CR danach

                if( bRedlineAtEnd )
                {
                    aAttrIter.OutRedlines( nEnd );
                    if( pO->Count() )
                    {
                        rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                                    pO->Count(), pO->GetData() );
                        pO->Remove( 0, pO->Count() );                   // leeren
                    }
                }
            }
        }
        nAktPos = nNextAttr;
        eChrSet = eNextChrSet;
        aAttrIter.NextPos();
    }
    while( nAktPos < nEnd );

    ASSERT( !pO->Count(), " pO ist am ZeilenEnde nicht leer" );

//  // gibt es harte Absatz-Attributierung oder Fly- oder Table-Attrs ?
//  if( pNd->GetpSwAttrSet() || rWW8Wrt.pFlyFmt || rWW8Wrt.bOutTable ){
        pO->Insert( (BYTE*)&nSty, 2, pO->Count() );     // Style #

        if( rWW8Wrt.pFlyFmt && !rWW8Wrt.bIsInTable )    // Fly-Attrs
            rWW8Wrt.Out_SwFmt( *rWW8Wrt.pFlyFmt, FALSE, FALSE, TRUE );

        if( rWW8Wrt.bOutTable )
        {                                               // Tab-Attr
            // sprmPFInTable
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( *pO, 0x2416 );
            else
                pO->Insert( 24, pO->Count() );
            pO->Insert( 1, pO->Count() );
        }

        if( !bFlyInTable )
        {
            SfxItemSet* pTmpSet = 0;
            const BYTE nPrvNxtNd =
                ( WWFL_ULSPACE_LIKE_SWG & rWW8Wrt.GetIniFlags())
                        ? (ND_HAS_PREV_LAYNODE|ND_HAS_NEXT_LAYNODE)
                        : pNd->HasPrevNextLayNode();

            if( (ND_HAS_PREV_LAYNODE|ND_HAS_NEXT_LAYNODE ) != nPrvNxtNd )
            {
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == pNd->GetSwAttrSet().GetItemState(
                        RES_UL_SPACE, TRUE, &pItem ) &&
                    ( ( !( ND_HAS_PREV_LAYNODE & nPrvNxtNd ) &&
                       ((SvxULSpaceItem*)pItem)->GetUpper()) ||
                      ( !( ND_HAS_NEXT_LAYNODE & nPrvNxtNd ) &&
                       ((SvxULSpaceItem*)pItem)->GetLower()) ))
                {
                    pTmpSet = new SfxItemSet( pNd->GetSwAttrSet() );
                    SvxULSpaceItem aUL( *(SvxULSpaceItem*)pItem );
                    if( !(ND_HAS_PREV_LAYNODE & nPrvNxtNd ))
                        aUL.SetUpper( 0 );
                    if( !(ND_HAS_NEXT_LAYNODE & nPrvNxtNd ))
                        aUL.SetLower( 0 );
                    pTmpSet->Put( aUL );
                }
            }

            if( !pTmpSet )
                pTmpSet = pNd->GetpSwAttrSet();

            const SwNumRule* pRule;
            const SwNodeNum* pNum;
            if( (( 0 != ( pNum = pNd->GetNum() ) &&
                    0 != ( pRule = pNd->GetNumRule() )) ||
                    ( 0 != ( pNum = pNd->GetOutlineNum() ) &&
                    0 != ( pRule = rWrt.pDoc->GetOutlineNumRule() ) ) ) &&
                    pNum->GetLevel() < NO_NUM )
            {
                BYTE nLvl = GetRealLevel( pNum->GetLevel() );
                const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
                if( !pFmt )
                    pFmt = &pRule->Get( nLvl );

                if( pTmpSet == pNd->GetpSwAttrSet() )
                    pTmpSet = new SfxItemSet( pNd->GetSwAttrSet() );

                SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pTmpSet->Get( RES_LR_SPACE ) );
                aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );

                if( MAXLEVEL > pNum->GetLevel() )
                {
                    aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
                    if( pNum == pNd->GetNum() && SFX_ITEM_SET !=
                        pTmpSet->GetItemState( RES_PARATR_NUMRULE ) )
                    {
                        // NumRule from a template - then put it into the itemset
                        pTmpSet->Put( SwNumRuleItem( pRule->GetName() ));
                    }
                }
                else
                    pTmpSet->ClearItem( RES_PARATR_NUMRULE );

                pTmpSet->Put( aLR );
                SwWW8Writer::CorrTabStopInSet( *pTmpSet, pFmt->GetAbsLSpace() );
            }

            if( pTmpSet )
            {                                               // Para-Attrs
                rWW8Wrt.pStyAttr = &pNd->GetAnyFmtColl().GetAttrSet();

                const SwModify* pOldMod = rWW8Wrt.pOutFmtNode;
                rWW8Wrt.pOutFmtNode = pNd;

                rWW8Wrt.Out_SfxItemSet( *pTmpSet, TRUE, FALSE );        // Pap-Attrs

                rWW8Wrt.pStyAttr = 0;
                rWW8Wrt.pOutFmtNode = pOldMod;

                if( pTmpSet != pNd->GetpSwAttrSet() )
                    delete pTmpSet;
            }
        }
        rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                        pO->Count(), pO->GetData() );
        pO->Remove( 0, pO->Count() );                       // leeren
//  }else{
//      rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(), sizeof( nSty ), nSty );
//  }
    return rWrt;
}


/*  */

//---------------------------------------------------------------------------
//       Tabellen
//---------------------------------------------------------------------------

Writer& OutWW8_SwTblNode( Writer& rWrt, SwTableNode & rNode )
{
    SwWW8Writer & rWW8Wrt = (SwWW8Writer&)rWrt;
    SwTable& rTbl = rNode.GetTable();
    rWW8Wrt.Out_SfxBreakItems( rTbl.GetFrmFmt()->GetAttrSet(), rNode );

    SwTableLines& rLns = rTbl.GetTabLines();
    SwTwips nPageSize = 0, nTblOffset = 0;

    {
        Point aPt;
        SwRect aRect( rTbl.GetFrmFmt()->FindLayoutRect( FALSE, &aPt ));
        if( aRect.IsEmpty() )
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pFmt = rWW8Wrt.pFlyFmt ? rWW8Wrt.pFlyFmt :
                rWrt.pDoc->GetPageDesc(0).GetPageFmtOfNode( rNode, FALSE );

            aRect = pFmt->FindLayoutRect( TRUE );
            if( 0 == ( nPageSize = aRect.Width() ))
            {
                const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
                nPageSize = pFmt->GetFrmSize().GetWidth() -
                                rLR.GetLeft() - rLR.GetRight();
            }
        }
        else
            nPageSize = aRect.Width();
    }

    SwTwips nTblSz = rTbl.GetFrmFmt()->GetFrmSize().GetWidth();
    WW8Bytes aAt( 128, 128 );   // Attribute fuer's Tabellen-Zeilenende

    static BYTE __READONLY_DATA aNullBytes[] = { 0, 0, 0, 0 };

    // sprmPDxaFromText10
    if( rWW8Wrt.bWrtWW8 )
    {
        static BYTE __READONLY_DATA  aTabLineAttr[] = {
                0, 0,               // Sty # 0
                0x16, 0x24, 1,      // sprmPFInTable
                0x17, 0x24, 1 };    // sprmPFTtp
        aAt.Insert( aTabLineAttr, sizeof( aTabLineAttr ), aAt.Count() );
    }
    else
    {
        static BYTE __READONLY_DATA  aTabLineAttr[] = {
                0, 0,               // Sty # 0
                24, 1,              // sprmPFInTable
                25, 1 };            // sprmPFTtp
        aAt.Insert( aTabLineAttr, sizeof( aTabLineAttr ), aAt.Count() );
    }

    SwHoriOrient eHOri = rTbl.GetFrmFmt()->GetHoriOrient().GetHoriOrient();
    switch( eHOri )
    {
    case HORI_CENTER:
    case HORI_RIGHT:
        if( rWW8Wrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( aAt, 0x5400 );
        else
            aAt.Insert( 182, aAt.Count() );
        SwWW8Writer::InsUInt16( aAt, (HORI_RIGHT == eHOri ? 2 : 1 ));
        break;

    case HORI_NONE:
    case HORI_LEFT_AND_WIDTH:
        {
            const SvxLRSpaceItem& rLRSp = rTbl.GetFrmFmt()->GetLRSpace();
            nTblOffset = rLRSp.GetLeft();
            nPageSize -= nTblOffset + rLRSp.GetRight();
        }
        break;
//  case FLY_HORI_FULL:
    }

    BOOL bRelBoxSize = TRUE /*ALWAYS relativ (nPageSize + ( nPageSize / 10 )) < nTblSz*/;
    USHORT nStdAtLen = aAt.Count();

    SwWriteTable* pTableWrt;
    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        pTableWrt = new SwWriteTable( pLayout );
    else
        pTableWrt = new SwWriteTable( rTbl.GetTabLines(), (USHORT)nPageSize,
                                          (USHORT)nTblSz, FALSE );

    // WW6 / 8 can not have more then 31 / 64 cells
    const USHORT nMaxCols = rWW8Wrt.bWrtWW8 ? 64 : 31;
    // rCols are the array of all cols of the table
    const SwWriteTableCols& rCols = pTableWrt->GetCols();
    USHORT nColCnt = rCols.Count();
    SwWriteTableCellPtr* pBoxArr = new SwWriteTableCellPtr[ nColCnt ];
    USHORT* pRowSpans = new USHORT[ nColCnt ];
    memset( pBoxArr, 0, sizeof( pBoxArr[0] ) * nColCnt );
    memset( pRowSpans, 0, sizeof( pRowSpans[0] ) * nColCnt );
    long nLastHeight = 0;
    const SwWriteTableRows& rRows = pTableWrt->GetRows();
    for( USHORT nLine = 0; nLine < rRows.Count(); ++nLine )
    {
        USHORT nBox, nRealBox;

        const SwWriteTableRow *pRow = rRows[ nLine ];
        const SwWriteTableCells& rCells = pRow->GetCells();

        BOOL bFixRowHeight = FALSE;
        USHORT nRealColCnt = 0;
        for( nColCnt = 0, nBox = 0; nBox < rCells.Count(); ++nColCnt )
        {
            if( !pRowSpans[ nColCnt ] )
            {
                // set new BoxPtr
                SwWriteTableCell* pCell = rCells[ nBox++ ];
                pBoxArr[ nColCnt ] = pCell;
                pRowSpans[ nColCnt ] = pCell->GetRowSpan();
                for( USHORT nCellSpan = pCell->GetColSpan(), nCS = 1;
                        nCS < nCellSpan; ++nCS, ++nColCnt )
                {
                    pBoxArr[ nColCnt+1 ] = pBoxArr[ nColCnt ];
                    pRowSpans[ nColCnt+1 ] = pRowSpans[ nColCnt ];
                }
                ++nRealColCnt;
            }
            else if( !nColCnt || pBoxArr[ nColCnt-1 ] != pBoxArr[ nColCnt ] )
                ++nRealColCnt;

            if( 1 != pRowSpans[ nColCnt ] )
                bFixRowHeight = TRUE;
        }

        for( ; nColCnt < rCols.Count() && pRowSpans[ nColCnt ]; ++nColCnt )
        {
            if( !nColCnt || pBoxArr[ nColCnt-1 ] != pBoxArr[ nColCnt ] )
                ++nRealColCnt;
            bFixRowHeight = TRUE;
        }

        USHORT nWWColMax = nRealColCnt > nMaxCols ? nMaxCols : nRealColCnt;

        // 1.Zeile eine Headline?  sprmTTableHeader
        if( !nLine && rTbl.IsHeadlineRepeat() )
        {
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0x3404 );
            else
                aAt.Insert( 186, aAt.Count() );
            aAt.Insert( 1, aAt.Count() );
        }

        // Zeilenhoehe ausgeben   sprmTDyaRowHeight
        long nHeight = 0;
        if( bFixRowHeight )
        {
            nHeight = -pRow->GetPos();      //neg. => abs. height!
            if( nLine )
                nHeight += rRows[ nLine - 1 ]->GetPos();
        }
        else
        {
            const SwTableLine* pLine = pBoxArr[ 0 ]->GetBox()->GetUpper();
            const SwFmtFrmSize& rLSz = pLine->GetFrmFmt()->GetFrmSize();
            if( ATT_VAR_SIZE != rLSz.GetSizeType() && rLSz.GetHeight() )
                nHeight = ATT_MIN_SIZE == rLSz.GetSizeType()
                                                ? rLSz.GetHeight()
                                                : -rLSz.GetHeight();
        }
        if( nHeight )
        {
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0x9407 );
            else
                aAt.Insert( 189, aAt.Count() );
            SwWW8Writer::InsUInt16( aAt, (USHORT)nHeight );
        }

        // sprmTFCantSplit - unsere Tabellen duerfen nie in der Zeile
        // gesplittet werden.
        if( rWW8Wrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( aAt, 0x3403 );
        else
            aAt.Insert( 185, aAt.Count() );
        aAt.Insert( (BYTE)0, aAt.Count() );

        // Inhalt der Boxen ausgeben
        for( nBox = 0, nRealBox = 0; nBox < nColCnt; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            if( pBoxArr[ nBox ]->GetRowSpan() == pRowSpans[ nBox ] )
            {
                // new Box
                const SwStartNode* pSttNd = pBoxArr[ nBox ]->GetBox()->GetSttNd();
                WW8SaveData aSaveData( rWW8Wrt, pSttNd->GetIndex()+1,
                                        pSttNd->EndOfSectionIndex() );
                rWW8Wrt.bOutTable = TRUE;
                rWW8Wrt.bIsInTable= TRUE;
                rWW8Wrt.WriteText();
            }
            else
            {
                rWW8Wrt.bOutTable = TRUE;
                rWW8Wrt.WriteStringAsPara( aEmptyStr );
                rWW8Wrt.bOutTable = FALSE;
            }

            if( nWWColMax < nRealColCnt )
            {
                if( nRealBox+1 < nWWColMax || nRealBox+1 == nRealColCnt )
                    rWW8Wrt.ReplaceCr( (char)0x07 );    // SpaltenEnde
            }
            else if( nRealBox < nWWColMax )
                rWW8Wrt.ReplaceCr( (char)0x07 );    // SpaltenEnde
            ++nRealBox;
        }

        // das wars mit der Line
        rWW8Wrt.WriteChar( (char)0x7 ); // TabellenZeilen-Ende

        if( rWW8Wrt.bWrtWW8 )
        {
            // SprmTDefTable
            // 0+1 - OpCode, 2+3 - Laenge = Cells * (sizeof TC + uint16)
            // 4 - Cells (max 32!)
            SwWW8Writer::InsUInt16( aAt, 0xD608 );
            SwWW8Writer::InsUInt16( aAt, 2 + ( nWWColMax + 1 ) * 2 +
                                                ( nWWColMax * 20 ));
            aAt.Insert( nWWColMax, aAt.Count() );               //
        }
        else
        {
            // SprmTDefTable
            // 0 - OpCode, 1,2 - Laenge = Cells * (sizeof TC + uint16)
            // 3 - Cells (max 32!)
            aAt.Insert( 190, aAt.Count() );
            SwWW8Writer::InsUInt16( aAt, nWWColMax * 12 + 4 );  // Sprm-Laenge
            aAt.Insert( nWWColMax, aAt.Count() );               //
        }

        SwTwips nSz = 0, nCalc;
        SwWW8Writer::InsUInt16( aAt, (USHORT)nTblOffset );

        for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            const SwFrmFmt* pBoxFmt = pBoxArr[ nBox ]->GetBox()->GetFrmFmt();
            const SwFmtFrmSize& rLSz = pBoxFmt->GetFrmSize();
            nSz += rLSz.GetWidth();
            nCalc = nSz;
            if( bRelBoxSize )
            {
                nCalc *= nPageSize;
                nCalc /= nTblSz;
            }
            SwWW8Writer::InsUInt16( aAt, (USHORT)(nTblOffset + nCalc ));
            ++nRealBox;
        }

        for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; nBox++ )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;
            // rgf, erstmal alles 0 - WW8: 4, sonst 2 Byte
            // write vertikal alignment
            const SwFrmFmt& rFmt = *pBoxArr[ nBox ]->GetBox()->GetFrmFmt();
            if( rWW8Wrt.bWrtWW8 )
            {
                USHORT nFlags = pBoxArr[ nBox ]->GetRowSpan();
                if( 1 < nFlags )
                {
                    if( nFlags == pRowSpans[ nBox ] )
                        nFlags = 0x60;                  // start a new vert. merge
                    else
                        nFlags = 0x20;                  // continue a vert. merge
                }
                else
                    nFlags = 0;                     // no vert. merge

                switch( rFmt.GetVertOrient().GetVertOrient() )
                {
                case VERT_CENTER:   nFlags |= 0x080;    break;
                case VERT_BOTTOM:   nFlags |= 0x100;    break;
                }
                SwWW8Writer::InsUInt16( aAt, nFlags );
            }
            ++nRealBox;
            aAt.Insert( aNullBytes, 2, aAt.Count() );   // dummy
            rWW8Wrt.Out_SwFmtTableBox( aAt, rFmt.GetBox() ); // 8/16 Byte
        }

        // Background
        USHORT nBackg;
        if( pRow->GetBackground() )
            // over all boxes!
            nBackg = nWWColMax;
        else
        {
            nBackg = 0;
            for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; ++nBox )
            {
                if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                    continue;

                SwWriteTableCell* pCell = pBoxArr[ nBox ];
                if( pCell->GetBackground() ||
                    SFX_ITEM_ON == pCell->GetBox()->GetFrmFmt()->
                    GetItemState( RES_BACKGROUND, FALSE ) )
                    nBackg = nRealBox + 1;
                ++nRealBox;
            }
        }

        if( nBackg )
        {
            // TableShade, 0(+1) - OpCode, 1(2) - Count * sizeof( SHD ),
            // 2..(3..) - SHD
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0xD609 );
            else
                aAt.Insert( (BYTE)191, aAt.Count() );
            aAt.Insert( (BYTE)(nBackg * 2), aAt.Count() );  // Len

            const SfxPoolItem* pI;
            for( nBox = 0, nRealBox = 0; nRealBox < nBackg; ++nBox )
            {
                if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                    continue;

                USHORT nValue;
                SwWriteTableCell* pCell = pBoxArr[ nBox ];
                if( SFX_ITEM_ON == pCell->GetBox()->GetFrmFmt()->
                    GetAttrSet().GetItemState( RES_BACKGROUND, FALSE, &pI )
                    || 0 != ( pI = pCell->GetBackground() )
                    || 0 != ( pI = pRow->GetBackground() ) )
                {
                    WW8_SHD aShd;
                    rWW8Wrt.TransBrush( ((const SvxBrushItem*)pI)->
                                        GetColor(), aShd );
                    nValue = aShd.GetValue();
                }
                else
                    nValue = 0;
                SwWW8Writer::InsUInt16( aAt, nValue );
                ++nRealBox;
            }
        }

        rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                   aAt.Count(), aAt.GetData() );
        if( aAt.Count() > nStdAtLen )
            aAt.Remove( nStdAtLen, aAt.Count() - nStdAtLen );

        for( nBox = 0; nBox < nColCnt; ++nBox )
            --pRowSpans[ nBox ];
    }

    delete pTableWrt;
    delete pBoxArr;
    delete pRowSpans;

    // Pam hinter die Tabelle verschieben
    rWW8Wrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

    return rWrt;
}


/*  */

Writer& OutWW8_SwSectionNode( Writer& rWrt, SwSectionNode& rSectionNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwSection& rSection = rSectionNode.GetSection();

    SwNodeIndex aIdx( rSectionNode, 1 );
    const SwNode& rNd = aIdx.GetNode();
    if( !rNd.IsSectionNode()  )
    {
        // Bug 74245 - if the first Node inside the section has an own
        //              PageDesc or PageBreak attribut, then dont write
        //              here the section break
        ULONG nRstLnNum = 0;
        const SfxItemSet* pSet;
        if( rNd.IsTableNode() )
            pSet = &rNd.GetTableNode()->GetTable().GetFrmFmt()->GetAttrSet();
        else if( rNd.IsCntntNode() )
        {
            pSet = &rNd.GetCntntNode()->GetSwAttrSet();
            nRstLnNum = ((SwFmtLineNumber&)pSet->Get(
                            RES_LINENUMBER )).GetStartValue();
        }
        else
            pSet = 0;

        const SfxPoolItem* pI;
        if( pSet && (
            ( SFX_ITEM_ON != pSet->GetItemState( RES_PAGEDESC, TRUE, &pI )
                || 0 == ((SwFmtPageDesc*)pI)->GetPageDesc() ) ||
            ( SFX_ITEM_ON != pSet->GetItemState( RES_BREAK, TRUE, &pI )
                || SVX_BREAK_NONE == ((SvxFmtBreakItem*)pI)->GetBreak() )))
            pSet = 0;

        if( !pSet )
        {
            // new Section with no own PageDesc/-Break
            //  -> write follow section break;
            const SwSectionFmt& rFmt = *rSection.GetFmt();
            rWW8Wrt.ReplaceCr( (char)0xc ); // Indikator fuer Page/Section-Break

            rWW8Wrt.pSepx->AppendSep( rWW8Wrt.Fc2Cp( rWrt.Strm().Tell() ),
                                        rWW8Wrt.pAktPageDesc,
                                        &rFmt, nRstLnNum );
        }
    }
    if( TOX_CONTENT_SECTION == rSection.GetType() )
        rWW8Wrt.bStartTOX = TRUE;
    return rWrt;
}

/*  */

//---------------------------------------------------------------------------
//       Flys
//---------------------------------------------------------------------------

void SwWW8Writer::OutWW8FlyFrmsInCntnt( const SwTxtNode& rNd )
{
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        for( USHORT n=0; n < pTxtAttrs->Count(); ++n )
        {
            const SwTxtAttr* pAttr = (*pTxtAttrs)[ n ];
            if( RES_TXTATR_FLYCNT == pAttr->Which() )
            {
                // zeichengebundenes Attribut
                const SwFmtFlyCnt& rFlyCntnt = pAttr->GetFlyCnt();
                const SwFlyFrmFmt& rFlyFrmFmt = *(SwFlyFrmFmt*)rFlyCntnt.GetFrmFmt();
                const SwNodeIndex* pNodeIndex = rFlyFrmFmt.GetCntnt().GetCntntIdx();

                if( pNodeIndex )
                {
                    ULONG nStt = pNodeIndex->GetIndex()+1,
                          nEnd = pNodeIndex->GetNode().EndOfSectionIndex();

                    if( (nStt < nEnd) && !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                    {
                        Point aOffset;
                        // Rechtecke des Flys und des Absatzes besorgen
                        SwRect aParentRect( rNd.FindLayoutRect( FALSE, &aOffset ) ),
                               aFlyRect( rFlyFrmFmt.FindLayoutRect( FALSE, &aOffset ) );

                        aOffset = aFlyRect.Pos() - aParentRect.Pos();

                        // PaM umsetzen: auf Inhalt des Fly-Frameformats
                        WW8SaveData aSaveData( *this, nStt, nEnd );

                        // wird in Out_SwFmt() ausgewertet
                        pFlyOffset = &aOffset;
                        pFlyFmt = (SwFlyFrmFmt*)&rFlyFrmFmt;
                        // Ok, rausschreiben:
                        WriteText();
                    }
                }
            }
        }
    }
}


void SwWW8Writer::OutWW8FlyFrm( const SwFrmFmt& rFrmFmt,
                                const Point& rNdTopLeft )
{
    if( !bWrtWW8 )
    {
        if( RES_DRAWFRMFMT == rFrmFmt.Which() )
        {
            ASSERT( !this, "OutWW8FlyFrm: DrawInCnt-Baustelle " );
            return ;
        }

        BOOL bDone = FALSE;

        // Hole vom Node und vom letzten Node die Position in der Section
        /*
        const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();

        ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
        ULONG nEnd = pDoc->GetNodes()[ nStt - 1 ]->EndOfSectionIndex();
        */
        const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

        ULONG nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
        ULONG nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

        if( nStt >= nEnd )      // kein Bereich, also kein gueltiger Node
            return;

        const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();

        if( !bIsInTable && (FLY_IN_CNTNT == rAnch.GetAnchorId()) )
        {
            // ein zeichen(!)gebundener Rahmen liegt vor
            SwTxtNode* pParTxtNode = rAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();

            if( pParTxtNode && !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                bDone = TRUE;
        }
        if( !bDone )
        {
            // ein NICHT zeichengebundener Rahmen liegt vor

            const SwFmtFrmSize& rS = rFrmFmt.GetFrmSize();
            nFlyWidth  = rS.GetWidth();  // Fuer Anpassung Graphic-Groesse
            nFlyHeight = rS.GetHeight();

            ASSERT( !pFlyFmt, "+pFlyFmt ist vor einem Rahmen nicht 0 ( Rahmen in Rahmen ? )" );
            {
                WW8SaveData aSaveData( *this, nStt, nEnd );
                pFlyFmt = (SwFlyFrmFmt*)&rFrmFmt;
                if( pFlyFmt )
                {
                    if( bIsInTable && (FLY_PAGE != rAnch.GetAnchorId()) )
                    {
                        // Beachten: Flag  bOutTable  wieder setzen,
                        //           denn wir geben ja ganz normalen Content der
                        //           Tabelenzelle aus und keinen Rahmen
                        //           (Flag wurde oben in  aSaveData()  geloescht)
                        bOutTable = TRUE;
                        const String& rName = pFlyFmt->GetName();
                        StartCommentOutput( rName );
                        if( !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                            WriteText();
                        EndCommentOutput( rName );
                    }
                    else
                        WriteText();
                }
                else
                    ASSERT( !this, "+Fly-Ausgabe ohne FlyFmt" );
            }
            // ASSERT( !pFlyFmt, " pFlyFmt ist hinter einem Rahmen nicht 0" );
        }
    }
    else
    {
        // write as escher
        WW8_CP nCP = Fc2Cp( Strm().Tell() );
        AppendFlyInFlys( nCP, rFrmFmt, rNdTopLeft );
    }
}


void SwWW8Writer::OutFlyFrms( const SwCntntNode& rNode )
{
    if( !pFlyPos )
        return;

    // gib alle freifliegenden Rahmen die sich auf den akt. Absatz
    // und evt. auf das aktuelle Zeichen beziehen, aus.
    ULONG nCurPos = rNode.GetIndex();

    // suche nach dem Anfang der FlyFrames
    for( USHORT n = 0; n < pFlyPos->Count() &&
            (*pFlyPos)[n]->GetNdIndex().GetIndex() <
                nCurPos; ++n )
        ;

    Point aNdPos, aPgPos;
    Point* pLayPos;
    BOOL bValidNdPos = FALSE, bValidPgPos = FALSE;

    if( n < pFlyPos->Count() )
    {
        while(     ( n < pFlyPos->Count() )
                && (   nCurPos
                    == (*pFlyPos)[n]->GetNdIndex().GetIndex() ) )
        {
            const SwFrmFmt& rFmt = (*pFlyPos)[ n ]->GetFmt();
            if( FLY_PAGE == rFmt.GetAnchor().GetAnchorId() )
            {
                // get the Layout Node-Position.
                if( !bValidPgPos )
                {
                    aPgPos = rNode.FindPageFrmRect( FALSE, &aPgPos ).Pos();
                    bValidPgPos = TRUE;
                }
                pLayPos = &aPgPos;
            }
            else
            {
                // get the Layout Node-Position.
                if( !bValidNdPos )
                {
                    aNdPos = rNode.FindLayoutRect( FALSE, &aNdPos ).Pos();
                    bValidNdPos = TRUE;
                }
                pLayPos = &aNdPos;
            }

            OutWW8FlyFrm( rFmt, *pLayPos );
            ++n;
         }
    }
}

/*  */

// write data of any redline
void SwWW8Writer::OutRedline( const SwRedlineData& rRedline )
{
    if( rRedline.Next() )
        OutRedline( *rRedline.Next() );

    static USHORT __READONLY_DATA aSprmIds[ 2 * 2 * 3 ] =
    {
        // Ids for insert
            0x0801, 0x4804, 0x6805,         // for WW8
            0x0042, 0x0045, 0x0046,         // for WW6
        // Ids for delete
            0x0800, 0x4863, 0x6864,         // for WW8
            0x0041, 0x0045, 0x0046          // for WW6
    };

    const USHORT* pSprmIds = 0;
    switch( rRedline.GetType() )
    {
    case REDLINE_INSERT:
        pSprmIds = aSprmIds;
        break;

    case REDLINE_DELETE:
        pSprmIds = aSprmIds + (2 * 3);
        break;

    case REDLINE_FORMAT:
        if( bWrtWW8 )
        {
            InsUInt16( 0xca57 );
            pO->Insert( 7, pO->Count() );       // len
            pO->Insert( 1, pO->Count() );
            InsUInt16( AddRedlineAuthor( rRedline.GetAuthor() ) );
            InsUInt32( SwWW8Writer::GetDTTM( rRedline.GetTimeStamp() ));
        }
        break;
    }

    if( pSprmIds )
    {
        if( !bWrtWW8 )
            pSprmIds += 3;

        if( bWrtWW8 )
            InsUInt16( pSprmIds[0] );
        else
            pO->Insert( pSprmIds[0], pO->Count() );
        pO->Insert( 1, pO->Count() );

        if( bWrtWW8 )
            InsUInt16( pSprmIds[1] );
        else
            pO->Insert( pSprmIds[1], pO->Count() );
        InsUInt16( AddRedlineAuthor( rRedline.GetAuthor() ) );

        if( bWrtWW8 )
            InsUInt16( pSprmIds[2] );
        else
            pO->Insert( pSprmIds[2], pO->Count() );
        InsUInt32( SwWW8Writer::GetDTTM( rRedline.GetTimeStamp() ));
    }
}

/*  */

SwNodeFnTab aWW8NodeFnTab = {
/* RES_TXTNODE  */                   OutWW8_SwTxtNode,
/* RES_GRFNODE  */                   OutWW8_SwGrfNode,
/* RES_OLENODE  */                   OutWW8_SwOleNode,
};


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/wrtw8nds.cxx,v 1.3 2000-10-20 13:43:39 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/09 17:58:48  jp
      Bug #78395#: don't write UniCode by a W95 export

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.42  2000/09/18 16:04:57  willem.vandorp
      OpenOffice header added.

      Revision 1.41  2000/08/21 10:13:58  khz
      Export Line Numbering (restart on new section)

      Revision 1.40  2000/08/18 13:02:21  jp
      don't export escaped URLs

      Revision 1.39  2000/08/04 10:57:10  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.38  2000/07/25 18:51:22  jp
      Bug #76835#: OutWW8_SwTblNode - count realboxes correct

      Revision 1.37  2000/05/12 16:13:55  jp
      Changes for Unicode

      Revision 1.36  2000/04/19 18:03:31  jp
      Bug #75223#: SearchNext - new min pos must be greater than the startpos

      Revision 1.35  2000/04/17 10:33:46  jp
      Bug #74947#: OutRedlines - set curredline only if the startpos. is the cur.outpos.

      Revision 1.34  2000/03/21 16:22:40  jp
      Bug #74360#: OutSwTableNode - dont count realboxes only by W97 export

      Revision 1.33  2000/03/21 15:02:52  os
      StarView remainders removed

      Revision 1.32  2000/03/17 12:33:46  jp
      Bug #74245#: OutSectionNode - dont write setionbreak if the next node has his own pagebreak

      Revision 1.31  2000/03/15 13:46:31  jp
      Bug #74070#: WriteTable - look for colspans

      Revision 1.30  2000/03/03 15:20:01  os
      StarView remainders removed

      Revision 1.29  2000/02/25 09:55:06  jp
      Bug #73098#: read & write list entries without number

      Revision 1.28  2000/02/21 12:32:50  cmc
      #69372# Hyperlinks are exported with the extra WW97 data in the Data Stream to activate links in the Word UI

      Revision 1.27  2000/01/25 20:08:49  jp
      Bug #72146#: OutSwTxtNode - write UNICode if some character can not be converted

      Revision 1.26  1999/12/20 17:41:29  jp
      Bug #70749#: Date/Time-Numberformats in german format from the formatter

      Revision 1.25  1999/12/14 22:11:57  jp
      Bug #70846#: OutTxtNode - if numbered Node and no NumRuleItem in Itemset, then put it new

      Revision 1.24  1999/12/13 16:12:35  jp
      Bug #70789#: TOXMarks may have a position or a range

      Revision 1.23  1999/11/25 21:22:25  jp
      Bug #70131#: OutFields: append the charattributes of the node to the sprms

      Revision 1.22  1999/11/22 18:26:14  jp
      export of TOX and TOX entries

      Revision 1.21  1999/11/12 13:08:01  jp
      OutSwTable: replace CR to 0x7 on the last Box if the SW Table has to much columns

      Revision 1.20  1999/10/29 17:40:14  jp
      export of 'complex'(vert. merged) tables

      Revision 1.19  1999/10/21 17:53:13  jp
      Bug #69377#: write redline sprm of formatting correct

      Revision 1.18  1999/10/14 21:06:01  jp
      Bug #68617#,#68705# and other little things

      Revision 1.17  1999/10/14 11:09:26  jp
      export Redlines into W95/W97

      Revision 1.16  1999/10/13 18:09:37  jp
      write redlines

      Revision 1.15  1999/10/11 11:00:10  jp
      some changes for export EditEngine text in WW-Format

      Revision 1.14  1999/09/10 16:29:58  jp
      SwTable: get from Layout the upper size, Bug #67922#: dont W95 loop if fly in tablecells

      Revision 1.13  1999/09/09 17:39:52  jp
      Bug #68634#: write TableLineHeight as int16 and not as int32

      Revision 1.12  1999/08/30 16:55:30  JP
      write vertical alignment of tablecells


      Rev 1.11   30 Aug 1999 18:55:30   JP
   write vertical alignment of tablecells

      Rev 1.10   23 Jul 1999 11:18:38   KZ
   rem. CHARSET Symbols

      Rev 1.9   21 Jul 1999 18:28:22   JP
   W97 Export: experimental escher export

      Rev 1.8   06 Jul 1999 14:53:58   JP
   W97-Export corrections

      Rev 1.7   30 Jun 1999 19:56:40   JP
   W97-Export: Fields/PostIts/References

      Rev 1.6   25 Jun 1999 18:43:08   JP
   W97-Export: hyperlinks and fields, Bug #67174#

      Rev 1.5   24 Jun 1999 22:52:18   JP
   Export of Bookmarks and Hyperlinks

      Rev 1.4   23 Jun 1999 20:14:42   JP
   W97-Export: write Bookmarks

      Rev 1.3   23 Jun 1999 12:46:14   JP
   Export of numbers und outlinenumbers

      Rev 1.2   16 Jun 1999 19:54:54   JP
   Change interface of base class Writer, Export of W97 NumRules

      Rev 1.1   08 Jun 1999 09:22:36   JP
   WW97-Export: 1.Teil fuer Tabellen (W95-Funktionalitaet)

      Rev 1.0   04 Jun 1999 17:49:00   JP
   WW97-Export: Code aufgeteilt

*************************************************************************/

