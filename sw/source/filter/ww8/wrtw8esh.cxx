/*************************************************************************
 *
 *  $RCSfile: wrtw8esh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: khz $ $Date: 2000-09-21 12:18:55 $
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


#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDTRANS_HXX
#include <svx/svdtrans.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

using namespace ::com::sun::star;

SV_DECL_VARARR_SORT( SvULongsSort, ULONG, 1, 1 )
SV_IMPL_VARARR_SORT( SvULongsSort, ULONG )

#define sEscherStream       String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "tempEsher" ))
#define sEscherPictStream   String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "EsherPicts" ))


WW8_WrPlcDrawObj::WW8_WrPlcDrawObj( BYTE nTyp )
    : nTTyp( nTyp ), aParentPos( 0, 16 )
{
}

WW8_WrPlcDrawObj::~WW8_WrPlcDrawObj()
{
    for( USHORT n = aParentPos.Count(); n; )
        delete (Point*)aParentPos[ --n ];
}

void WW8_WrPlcDrawObj::WritePlc( SwWW8Writer& rWrt ) const
{
    if( 8 != rWrt.pFib->nVersion )
        return ;        // solange kein DrawObjects in 95 exportiert wird


    ULONG nFcStart = rWrt.pTableStrm->Tell();
    USHORT nLen = aCps.Count();

    if( nLen )
    {
        // write CPs
        WW8Fib& rFib = *rWrt.pFib;
        ULONG nCpOffs = TXT_TXTBOX == nTTyp
                            ? 0 : (rWrt.pFib->ccpText + rWrt.pFib->ccpFtn);
        register USHORT i;
        for( i = 0; i < nLen; i++ )
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aCps[ i ] - nCpOffs );
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                            rFib.ccpText + rFib.ccpFtn +
                                            rFib.ccpHdr + rFib.ccpEdn +
                                            rFib.ccpTxbx + rFib.ccpHdrTxbx + 1 );

        for( i = 0; i < nLen; ++i )
        {
            // write the fspa-struct
            const SwFrmFmt& rFmt = *(SwFrmFmt*)aCntnt[ i ];
            const SdrObject* pObj = rFmt.FindRealSdrObject();

            Rectangle aRect;
            if( RES_FLYFRMFMT == rFmt.Which() )
            {
                Point aObjPos;
                SwRect aLayRect( rFmt.FindLayoutRect( FALSE, &aObjPos ));
                if( aLayRect.IsEmpty() )
                {
                    // the Object is not visible - so get the values from
                    // the format. The Position may not be correct.
                    const SwFmtVertOrient& rVOr = rFmt.GetVertOrient();
                    if( VERT_NONE == rVOr.GetVertOrient() )
                        aObjPos.Y() = rVOr.GetPos();
                    const SwFmtHoriOrient& rHOr = rFmt.GetHoriOrient();
                    if( HORI_NONE == rHOr.GetHoriOrient() )
                        aObjPos.X() = rHOr.GetPos();

                    aRect.SetSize( rFmt.GetFrmSize().GetSize() );
                    aRect.SetPos( aObjPos );
                }
                else
                    aRect = aLayRect.SVRect();
            }
            else
            {
                ASSERT( pObj, "wo ist das SDR-Object?" );
                if( pObj )
                    aRect = pObj->GetSnapRect();
            }
            aRect -= *(Point*)aParentPos[ i ];

            // spid
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aShapeIds[ i ] );

            //xaLeft/yaTop/xaRight/yaBottom - rel. to anchor
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRect.Left() );
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRect.Top() );
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRect.Right() );
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRect.Bottom() );

            //fHdr/bx/by/wr/wrk/fRcaSimple/fBelowText/fAnchorLock
            USHORT nFlags;
            if( FLY_PAGE == rFmt.GetAnchor().GetAnchorId() )
                nFlags = 0x0A;      // x-rel to top Page, y-rel to top Page
            else
                nFlags = 0x14;      // x-rel to text,  y-rel to text

            const SwFmtSurround& rSurr = rFmt.GetSurround();
            USHORT nContour = rSurr.IsContour() ? 0x80 : 0x40;
            switch( rSurr.GetSurround() )
            {
            case SURROUND_NONE:         nFlags |= 0x020;    break;
            case SURROUND_THROUGHT:     nFlags |= 0x060;    break;
            case SURROUND_PARALLEL:     nFlags |= 0x000 | nContour; break;
            case SURROUND_IDEAL:        nFlags |= 0x600 | nContour; break;
            case SURROUND_LEFT:         nFlags |= 0x200 | nContour; break;
            case SURROUND_RIGHT:        nFlags |= 0x400 | nContour; break;
            }
            if( pObj && pObj->GetLayer() == rWrt.pDoc->GetHellId() )
                nFlags |= 0x4000;

            SwWW8Writer::WriteShort( *rWrt.pTableStrm, nFlags );

            // cTxbx
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
        }


        if( TXT_TXTBOX == nTTyp )
        {
            rFib.fcPlcfspaMom = nFcStart;
            rFib.lcbPlcfspaMom = rWrt.pTableStrm->Tell() - nFcStart;
        }
        else
        {
            rFib.fcPlcfspaHdr = nFcStart;
            rFib.lcbPlcfspaHdr = rWrt.pTableStrm->Tell() - nFcStart;
        }
    }
}


BOOL WW8_WrPlcDrawObj::Append( SwWW8Writer& rWrt, WW8_CP nCp,
                                const SwFrmFmt& rFmt,
                                const Point& rNdTopLeft )
{
    BOOL bRet = FALSE;
    if( TXT_HDFT == rWrt.nTxtTyp || TXT_MAINTEXT == rWrt.nTxtTyp )
    {
        const SdrObject* pObj = rFmt.FindSdrObject(), *pInsObj = 0;
        if( RES_FLYFRMFMT == rFmt.Which() )
        {
            // check for textflyframe and if it is the first in a Chain
            const SwNodeIndex* pNdIdx = rFmt.GetCntnt().GetCntntIdx();
            if( pNdIdx )
                bRet = TRUE;
        }
        else
            bRet = TRUE;
    }

    if( bRet )
    {
        aCps.Insert( nCp, aCps.Count() );
        void* p = (void*)&rFmt;
        aCntnt.Insert( p, aCntnt.Count() );
        void* pPos = new Point( rNdTopLeft );
        aParentPos.Insert( pPos, aParentPos.Count() );
        aShapeIds.Insert( ULONG(0), aShapeIds.Count() );
    }
    return bRet;
}

void WW8_WrPlcDrawObj::SetShapeId( const SwFrmFmt& rFmt, UINT32 nId )
{
    const VoidPtr p = (void*)&rFmt;
    USHORT nPos = aCntnt.GetPos( p );
    if( USHRT_MAX != nPos )
        aShapeIds[ nPos ] = nId;
}

/*  */

void WW8_WrPlcTxtBoxes::WriteTxt( SwWW8Writer& rWrt )
{
    rWrt.bInWriteEscher = TRUE;
    long& rccp = TXT_TXTBOX == nTyp ? rWrt.pFib->ccpTxbx
                                    : rWrt.pFib->ccpHdrTxbx;
    WW8_WrPlcSubDoc::WriteTxt( rWrt, nTyp, rccp );

    WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    WW8Fib& rFib = *rWrt.pFib;
    long nOffset = rFib.ccpText + rFib.ccpFtn + rFib.ccpHdr + rFib.ccpAtn
                            + rFib.ccpEdn;
    if( TXT_TXTBOX == nTyp )
        rWrt.pFldTxtBxs->Finish( nCP, nOffset );
    else
        rWrt.pFldHFTxtBxs->Finish( nCP, nOffset + rFib.ccpTxbx );
    rWrt.bInWriteEscher = FALSE;
}

void WW8_WrPlcTxtBoxes::Append( const SdrObject& rObj, UINT32 nShapeId )
{
    void* p = (void*)&rObj;
    aCntnt.Insert( p, aCntnt.Count() );
    aShapeIds.Insert( nShapeId, aShapeIds.Count() );
}

const SvULongs* WW8_WrPlcTxtBoxes::GetShapeIdArr() const
{
    return &aShapeIds;
}

/*  */

UINT32 SwWW8Writer::GetSdrOrdNum( const SwFrmFmt& rFmt ) const
{
    UINT32 nOrdNum;
    const SdrObject* pObj = rFmt.FindRealSdrObject();
    if( pObj )
        nOrdNum = pObj->GetOrdNum();
    else
    {
        // no Layout for this format, then recalc the ordnum
        SwFrmFmt* pFmt = (SwFrmFmt*)&rFmt;
        nOrdNum = pDoc->GetSpzFrmFmts()->GetPos( pFmt );

        const SdrModel* pModel = pDoc->GetDrawModel();
        if( pModel )
            nOrdNum += pModel->GetPage( 0 )->GetObjCount();
    }
    return nOrdNum;
}

void SwWW8Writer::AppendFlyInFlys( WW8_CP& rCP, const SwFrmFmt& rFrmFmt,
                                    const Point& rNdTopLeft )
{
    ASSERT( !pEscher, "der EscherStream wurde schon geschrieben!" );
    if( pEscher )
        return ;

    WW8_WrPlcDrawObj *pDrwO = TXT_HDFT == nTxtTyp ? pHFSdrObjs : pSdrObjs;
    if( pDrwO->Append( *this, rCP, rFrmFmt, rNdTopLeft ))
    {
        static BYTE __READONLY_DATA aSpec8[] = {
            0x03, 0x6a, 0, 0, 0, 0,         // sprmCObjLocation (wahrscheinlich unnoetig)
            0x55, 0x08, 1                   // sprmCFSpec
        };
                                                // fSpec-Attribut TRUE
                            // Fuer DrawObjets muss ein Spezial-Zeichen
                            // in den Text und darum ein fSpec-Attribut
        pChpPlc->AppendFkpEntry( Strm().Tell() );
        WriteChar( 0x8 );
        rCP += 1;       // to next charakter position
        pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aSpec8 ), aSpec8 );

        if( RES_FLYFRMFMT == rFrmFmt.Which() )
        {
            // search all Flys/DrawObj in Flys and put it behind this
            // text position.
            const SwNodeIndex* pNdIdx = rFrmFmt.GetCntnt().GetCntntIdx();
            ASSERT( pNdIdx, "wo ist der NodeIndex geblieben?" );
            ULONG nStart = pNdIdx->GetIndex(),
                  nEnd = pNdIdx->GetNode().EndOfSectionIndex(),
                  nIdx;

            USHORT nArrLen = pDoc->GetSpzFrmFmts()->Count();
            for( USHORT n = 0; n < nArrLen; ++n )
            {
                const SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
                const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
                const SwPosition* pAPos;
                if( ( pAnchor->GetAnchorId() == FLY_AT_CNTNT ||
                      pAnchor->GetAnchorId() == FLY_AT_FLY ||
                      pAnchor->GetAnchorId() == FLY_AUTO_CNTNT ) &&
                    0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
                    nStart <= ( nIdx = pAPos->nNode.GetIndex()) &&
                    nIdx < nEnd )
                {
                    AppendFlyInFlys( rCP, *pFmt, rNdTopLeft );
                }
            }
        }
    }
}


USHORT SwWW8Writer::TranslateToEditEngineId( USHORT nWhich )
{
    switch( nWhich )
    {
    case RES_LR_SPACE:              nWhich = EE_PARA_LRSPACE;       break;
    case RES_UL_SPACE:              nWhich = EE_PARA_ULSPACE;       break;
    case RES_PARATR_LINESPACING:    nWhich = EE_PARA_SBL;           break;
    case RES_PARATR_ADJUST:         nWhich = EE_PARA_JUST;          break;
    case RES_PARATR_TABSTOP:        nWhich = EE_PARA_TABS;          break;

    case RES_CHRATR_COLOR:          nWhich = EE_CHAR_COLOR;         break;
    case RES_CHRATR_FONT:           nWhich = EE_CHAR_FONTINFO;      break;
    case RES_CHRATR_FONTSIZE:       nWhich = EE_CHAR_FONTHEIGHT;    break;
    case RES_CHRATR_WEIGHT:         nWhich = EE_CHAR_WEIGHT;        break;
    case RES_CHRATR_UNDERLINE:      nWhich = EE_CHAR_UNDERLINE;     break;
    case RES_CHRATR_CROSSEDOUT:     nWhich = EE_CHAR_STRIKEOUT;     break;
    case RES_CHRATR_POSTURE:        nWhich = EE_CHAR_ITALIC;        break;
    case RES_CHRATR_CONTOUR:        nWhich = EE_CHAR_OUTLINE;       break;
    case RES_CHRATR_SHADOWED:       nWhich = EE_CHAR_SHADOW;        break;
    case RES_CHRATR_ESCAPEMENT:     nWhich = EE_CHAR_ESCAPEMENT;    break;
    case RES_CHRATR_AUTOKERN:       nWhich = EE_CHAR_PAIRKERNING;   break;
    case RES_CHRATR_KERNING:        nWhich = EE_CHAR_KERNING;       break;
    case RES_CHRATR_WORDLINEMODE:   nWhich = EE_CHAR_WLM;           break;
    case RES_CHRATR_CHARSETCOLOR:   nWhich = EE_FEATURE_NOTCONV;    break;


    default:
//  case EE_PARA_NUMBULLET:
//  case EE_PARA_HYPHENATE:
//  case EE_PARA_BULLETSTATE:
//  case EE_PARA_OUTLLRSPACE:
//  case EE_PARA_OUTLLEVEL:
//  case EE_PARA_BULLET:
//ITEMID_FONTWIDTH        EE_CHAR_FONTWIDTH
        nWhich = 0;
        break;
    }
    return nWhich;
}

USHORT SwWW8Writer::TranslateFromEditEngineId( USHORT nWhich )
{
    switch( nWhich )
    {
    case EE_PARA_LRSPACE:       nWhich = RES_LR_SPACE;              break;
    case EE_PARA_ULSPACE:       nWhich = RES_UL_SPACE;              break;
    case EE_PARA_SBL:           nWhich = RES_PARATR_LINESPACING;    break;
    case EE_PARA_JUST:          nWhich = RES_PARATR_ADJUST;         break;
    case EE_PARA_TABS:          nWhich = RES_PARATR_TABSTOP;        break;

    case EE_CHAR_COLOR:         nWhich = RES_CHRATR_COLOR;          break;
    case EE_CHAR_FONTINFO:      nWhich = RES_CHRATR_FONT;           break;
    case EE_CHAR_FONTHEIGHT:    nWhich = RES_CHRATR_FONTSIZE;       break;
    case EE_CHAR_WEIGHT:        nWhich = RES_CHRATR_WEIGHT;         break;
    case EE_CHAR_UNDERLINE:     nWhich = RES_CHRATR_UNDERLINE;      break;
    case EE_CHAR_STRIKEOUT:     nWhich = RES_CHRATR_CROSSEDOUT;     break;
    case EE_CHAR_ITALIC:        nWhich = RES_CHRATR_POSTURE;        break;
    case EE_CHAR_OUTLINE:       nWhich = RES_CHRATR_CONTOUR;        break;
    case EE_CHAR_SHADOW:        nWhich = RES_CHRATR_SHADOWED;       break;
    case EE_CHAR_ESCAPEMENT:    nWhich = RES_CHRATR_ESCAPEMENT;     break;
    case EE_CHAR_PAIRKERNING:   nWhich = RES_CHRATR_AUTOKERN;       break;
    case EE_CHAR_KERNING:       nWhich = RES_CHRATR_KERNING;        break;
    case EE_CHAR_WLM:           nWhich = RES_CHRATR_WORDLINEMODE;   break;
    case EE_FEATURE_NOTCONV:    nWhich = RES_CHRATR_CHARSETCOLOR;   break;

    default:
//  case EE_PARA_NUMBULLET:
//  case EE_PARA_HYPHENATE:
//  case EE_PARA_BULLETSTATE:
//  case EE_PARA_OUTLLRSPACE:
//  case EE_PARA_OUTLLEVEL:
//  case EE_PARA_BULLET:
//ITEMID_FONTWIDTH        EE_CHAR_FONTWIDTH
        nWhich = 0;
        break;
    }
    return nWhich;
}

class WW8_SdrAttrIter : public WW8_AttrIter
{
    const EditTextObject* pEditObj;
    EECharAttribArray aTxtAtrArr;
    SvPtrarr aChrTxtAtrArr;
    SvUShorts aChrSetArr;
    USHORT nPara;
    xub_StrLen nAktSwPos;
    xub_StrLen nTmpSwPos;                   // fuer HasItem()
    rtl_TextEncoding eNdChrSet;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    void SetCharSet( const EECharAttrib& rTxtAttr, BOOL bStart );

public:
    WW8_SdrAttrIter( SwWW8Writer& rWr, const EditTextObject& rEditObj );
    void NextPara( USHORT nPar );
    void OutParaAttr( BOOL bCharAttr );

    BOOL IsTxtAttr( xub_StrLen nSwPos );

    void NextPos() { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( xub_StrLen nSwPos );
    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const;
    BOOL OutAttrWithRange( xub_StrLen nPos );
    xub_StrLen WhereNext() const                { return nAktSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const     { return eNdChrSet; }
};


WW8_SdrAttrIter::WW8_SdrAttrIter( SwWW8Writer& rWr,
                                    const EditTextObject& rEditObj )
    : WW8_AttrIter( rWr ), pEditObj( &rEditObj ),
    aTxtAtrArr( 0, 4 ), aChrTxtAtrArr( 0, 4 ), aChrSetArr( 0, 4 )
{
    NextPara( 0 );
}

void WW8_SdrAttrIter::NextPara( USHORT nPar )
{
    nPara = nPar;
    // Attributwechsel an Pos 0 wird ignoriert, da davon ausgegangen
    // wird, dass am Absatzanfang sowieso die Attribute neu ausgegeben
    // werden.
    aChrTxtAtrArr.Remove( 0, aChrTxtAtrArr.Count() );
    aChrSetArr.Remove( 0, aChrSetArr.Count() );
    nAktSwPos = nTmpSwPos = 0;

    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
    eNdChrSet = ((SvxFontItem&)aSet.Get( EE_CHAR_FONTINFO )).GetCharSet();
    pEditObj->GetCharAttribs( nPara, aTxtAtrArr );
    nAktSwPos = SearchNext( 1 );
}

rtl_TextEncoding WW8_SdrAttrIter::GetNextCharSet() const
{
    if( aChrSetArr.Count() )
        return (rtl_TextEncoding)aChrSetArr[ aChrSetArr.Count() - 1 ];
    return eNdChrSet;
}

// der erste Parameter in SearchNext() liefert zurueck, ob es ein TxtAtr ist.
xub_StrLen WW8_SdrAttrIter::SearchNext( xub_StrLen nStartPos )
{
    register xub_StrLen nPos;
    register xub_StrLen nMinPos = STRING_MAXLEN;
    register xub_StrLen i;

    for( i = 0; i < aTxtAtrArr.Count(); i++ )
    {
        const EECharAttrib& rHt = aTxtAtrArr[ i ];
        nPos = rHt.nStart;  // gibt erstes Attr-Zeichen
        if( nPos >= nStartPos && nPos <= nMinPos )
        {
            nMinPos = nPos;
            SetCharSet( rHt, TRUE );
        }

//??        if( pHt->GetEnd() )         // Attr mit Ende
        {
            nPos = rHt.nEnd;        // gibt letztes Attr-Zeichen + 1
            if( nPos >= nStartPos && nPos < nMinPos )
            {
                nMinPos = nPos;
                SetCharSet( rHt, FALSE );
            }
        }
/*      else
        {                                   // Attr ohne Ende
            nPos = rHt.nStart + 1;  // Laenge 1 wegen CH_TXTATR im Text
            if( nPos >= nStartPos && nPos < nMinPos )
            {
                nMinPos = nPos;
                SetCharSet( rHt, FALSE );
            }
        }
*/
    }
    return nMinPos;
}

void WW8_SdrAttrIter::SetCharSet( const EECharAttrib& rAttr, BOOL bStart )
{
    void* p = 0;
    rtl_TextEncoding eChrSet;
    const SfxPoolItem& rItem = *rAttr.pAttr;
    switch( rItem.Which() )
    {
    case EE_CHAR_FONTINFO:
        p = (void*)&rAttr;
        eChrSet = ((SvxFontItem&)rItem).GetCharSet();
        break;
    }

    if( p )
    {
        USHORT nPos;
        if( bStart )
        {
            nPos = aChrSetArr.Count();
            aChrSetArr.Insert( eChrSet, nPos );
            aChrTxtAtrArr.Insert( p, nPos );
        }
        else if( USHRT_MAX != ( nPos = aChrTxtAtrArr.GetPos( p )) )
        {
            aChrTxtAtrArr.Remove( nPos );
            aChrSetArr.Remove( nPos );
        }
    }
}

void WW8_SdrAttrIter::OutAttr( xub_StrLen nSwPos )
{
    OutParaAttr( TRUE );

    if( aTxtAtrArr.Count() )
    {
        const SwModify* pOldMod = rWrt.pOutFmtNode;
        rWrt.pOutFmtNode = 0;

        nTmpSwPos = nSwPos;
        register USHORT i, nWhich;
        FnAttrOut pOut;
        for( i = 0; i < aTxtAtrArr.Count(); i++ )
        {
            const EECharAttrib& rHt = aTxtAtrArr[ i ];
            if( nSwPos >= rHt.nStart && nSwPos < rHt.nEnd &&
                0 != ( nWhich = rWrt.TranslateFromEditEngineId(
                                                rHt.pAttr->Which() )) &&
                0 != ( pOut = aWW8AttrFnTab[ nWhich - RES_CHRATR_BEGIN ] ) )

                    (*pOut)( rWrt, *rHt.pAttr );

            else if( nSwPos < rHt.nStart )
                break;
        }

        nTmpSwPos = 0;      // HasTextItem nur in dem obigen Bereich erlaubt
        rWrt.pOutFmtNode = pOldMod;
    }
}

BOOL WW8_SdrAttrIter::IsTxtAttr( xub_StrLen nSwPos )
{
// solange die EditEngine keine Attribute hat, die nur eine Position haben
return  FALSE;

    // search for attrs without end position
    register USHORT i;
    for( i = 0; i < aTxtAtrArr.Count(); i++ )
        if( aTxtAtrArr[ i ].nStart == nSwPos )
            return TRUE;
    return FALSE;
}

// HasItem ist fuer die Zusammenfassung des Doppel-Attributes Underline
// und WordLineMode als TextItems. OutAttr() ruft die Ausgabefunktion,
// die dann ueber HasItem() nach anderen Items an der
// Attribut-Anfangposition fragen kann.
// Es koennen nur Attribute mit Ende abgefragt werden.
// Es wird mit bDeep gesucht
const SfxPoolItem* WW8_SdrAttrIter::HasTextItem( USHORT nWhich ) const
{
    const SfxPoolItem* pRet = 0;
    if( 0 != ( nWhich = rWrt.TranslateToEditEngineId( nWhich )) )
    {
        register USHORT i;
        for( i = 0; i < aTxtAtrArr.Count(); ++i )
        {
            const EECharAttrib& rHt = aTxtAtrArr[ i ];
            if( nWhich == rHt.pAttr->Which() &&
                nTmpSwPos >= rHt.nStart && nTmpSwPos < rHt.nEnd )
            {
                pRet = rHt.pAttr;       // gefunden
                break;
            }
            else if( nTmpSwPos < rHt.nStart )
                break;              // dann kommt da nichts mehr
        }
    }
    return pRet;
}

const SfxPoolItem& WW8_SdrAttrIter::GetItem( USHORT nWhich ) const
{
    const SfxPoolItem* pRet = HasTextItem( nWhich );
    if( !pRet )
    {
        SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
        pRet = &aSet.Get( rWrt.TranslateToEditEngineId( nWhich ) );
    }
    return *pRet;
}

void WW8_SdrAttrIter::OutParaAttr( BOOL bCharAttr )
{
    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
    if( aSet.Count() )
    {
        const SfxItemSet* pOldSet = rWrt.GetCurItemSet();
        rWrt.SetCurItemSet( &aSet );

        SfxItemIter aIter( aSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        FnAttrOut pOut;

        do {
            USHORT nWhich = rWrt.TranslateFromEditEngineId( pItem->Which() );
            if( nWhich && 0 != ( pOut =
                aWW8AttrFnTab[ nWhich - RES_CHRATR_BEGIN ] )
                && ( bCharAttr ? ( nWhich >= RES_CHRATR_BEGIN
                                  && nWhich < RES_TXTATR_END)
                               : (nWhich >= RES_PARATR_BEGIN
                                  && nWhich < RES_FRMATR_END) ) )
                (*pOut)( rWrt, *pItem );

        } while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
        rWrt.SetCurItemSet( pOldSet );
    }
}

void SwWW8Writer::WriteSdrTextObj( const SdrObject& rObj )
{
    const SdrTextObj* pTxtObj = PTR_CAST( SdrTextObj, &rObj );
    ASSERT( pTxtObj, "das ist gar kein SdrTextObj!" );

    BOOL bAnyWrite = FALSE;
    const OutlinerParaObject* pParaObj = pTxtObj->GetOutlinerParaObject();
    if( pParaObj )
    {
        const EditTextObject& rEditObj = pParaObj->GetTextObject();
        WW8_SdrAttrIter aAttrIter( *this, rEditObj );

        USHORT nPara = rEditObj.GetParagraphCount();
        BYTE bNul = 0;
        for( USHORT n = 0; n < nPara; ++n )
        {
            if( n )
                aAttrIter.NextPara( n );

            rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();

            ASSERT( !pO->Count(), " pO ist am Zeilenanfang nicht leer" );

            String aStr( rEditObj.GetText( n ));
            xub_StrLen nAktPos = 0;
            xub_StrLen nEnd = aStr.Len();
            BOOL bUnicode = TRUE;
            do {
/*
                switch( eChrSet )
                {
        //      case CHARSET_DONTKNOW:       ????
                case CHARSET_SYMBOL:
        //      case CHARSET_WIN_SYMBOL:
        //      case CHARSET_WIN_WINGDINGS:
        //      case CHARSET_MAC_DINGBATS:
        //      case CHARSET_MAC_SYMBOL:
        //      case CHARSET_ADOBE_SYMBOL:
        //      case CHARSET_ADOBE_DINGBATS:
        //      case CHARSET_STAR_STARBATS:
        //      case CHARSET_STAR_STARMATH:
                    bUnicode = bWrtWW8 ? TRUE : FALSE;
                    break;
                default:
                    bUnicode = FALSE;
                    break;
                }
*/
                xub_StrLen nNextAttr = aAttrIter.WhereNext();
                rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

                if( nNextAttr > nEnd )
                    nNextAttr = nEnd;

                BOOL bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
                if( !bTxtAtr )
                    OutSwString( aStr, nAktPos, nNextAttr - nAktPos,
                                    bUnicode, eChrSet );

                            // Am Zeilenende werden die Attribute bis ueber das CR
                            // aufgezogen. Ausnahme: Fussnoten am Zeilenende
                if( nNextAttr == nEnd && !bTxtAtr )
                    WriteCR();              // CR danach

                                                // Ausgabe der Zeichenattribute
                aAttrIter.OutAttr( nAktPos );   // nAktPos - 1 ??
                pChpPlc->AppendFkpEntry( Strm().Tell(),
                                                pO->Count(), pO->GetData() );
                pO->Remove( 0, pO->Count() );                   // leeren

                            // Ausnahme: Fussnoten am Zeilenende
                if( nNextAttr == nEnd && bTxtAtr )
                    WriteCR();              // CR danach

                nAktPos = nNextAttr;
                eChrSet = eNextChrSet;
                aAttrIter.NextPos();
            }
            while( nAktPos < nEnd );

            ASSERT( !pO->Count(), " pO ist am ZeilenEnde nicht leer" );

            pO->Insert( bNul, pO->Count() );        // Style # as short
            pO->Insert( bNul, pO->Count() );

            aAttrIter.OutParaAttr( FALSE );

            ULONG nPos = Strm().Tell();
            pPapPlc->AppendFkpEntry( Strm().Tell(),
                                            pO->Count(), pO->GetData() );
            pO->Remove( 0, pO->Count() );                       // leeren
            pChpPlc->AppendFkpEntry( nPos );
        }
        bAnyWrite = 0 != nPara;
    }
    if( !bAnyWrite )
        WriteStringAsPara( aEmptyStr );
}

/*  */

class SwEscherEx : public  EscherEx
{
    // private members
    SvULongs aFollowShpIds;
    SvPtrarr aSortFmts;
    SwWW8Writer& rWrt;
    WW8_WrPlcTxtBoxes *pTxtBxs;
    SvStream* pEscherStrm, *pPictStrm;
    long nEmuMul, nEmuDiv;

    // private methods
    void Init();

    UINT32 GetFlyShapeId( const SwFrmFmt& rFmt );
    void MakeZOrderArrAndFollowIds( const SvPtrarr& rSrcArr );

    INT32 ToFract16( INT32 nVal, UINT32 nMax ) const
        {
            if( nMax )
            {
                INT32 nMSVal = ( nVal / 65536) * nMax;
                nMSVal += ( nVal * 65536 ) / nMax;
                return nMSVal;
            }
            return 0;
        }
    INT32 DrawModelToEmu( INT32 nVal ) const
        { return BigMulDiv( nVal, nEmuMul, nEmuDiv ); }

    void WriteTxtFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId,
                                UINT32 nTxtBox );
    void WriteGrfFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId );
    void WriteOLEFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId );
    void WriteFlyFrameAttr( const SwFrmFmt& rFmt );
    void WriteGrfAttr( const SwNoTxtNode& rNd );

    UINT32 WriteFlyFrm( const SwFrmFmt& rFmt );

    virtual SvStream* QueryPicStream();
    virtual UINT32 QueryTextID( const uno::Reference< drawing::XShape>& , UINT32 );

    SwEscherEx( const SwEscherEx& );
public:
    SwEscherEx( SvStream* pStrm, SwWW8Writer& rWW8Wrt );
    virtual ~SwEscherEx();

    void FinishEscher();
    void WritePictures();
};


/*  */

void SwWW8Writer::CreateEscher()
{
    if(pHFSdrObjs->GetCntntArr().Count() || pSdrObjs->GetCntntArr().Count())
    {
        ASSERT( !pEscher, "wer hat den Pointer nicht geloescht?" );
        SvStream* pEscherStrm = pStg->OpenStream( sEscherStream,
                                STREAM_READWRITE | STREAM_SHARE_DENYALL );
        pEscherStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        pEscher = new SwEscherEx( pEscherStrm, *this );
    }
}

void SwWW8Writer::WriteEscher()
{
    if( pEscher )
    {
        ULONG nStart = pTableStrm->Tell();

        pEscher->WritePictures();
        pEscher->FinishEscher();

        pFib->fcDggInfo = nStart;
        pFib->lcbDggInfo = pTableStrm->Tell() - nStart;
        delete pEscher, pEscher = 0;
    }
}


/*  */

// Output- Routines for Escher Export

SwEscherEx::SwEscherEx( SvStream* pStrm, SwWW8Writer& rWW8Wrt )
/*  : SvxEscherEx( *pStrm,
                   rWW8Wrt.pHFSdrObjs->GetCntntArr().Count() ? 2 : 1,
                   *rWW8Wrt.pDoc->GetDrawModel(),
                   1440 ),
*/
    : EscherEx( *pStrm, rWW8Wrt.pHFSdrObjs->GetCntntArr().Count() ? 2 : 1 ),
    rWrt( rWW8Wrt ), pTxtBxs( 0 ), pEscherStrm( pStrm ), pPictStrm( 0 )
{
    Init();
    OpenContainer( ESCHER_DggContainer );
        BeginCount();
        AddColor( 0x08000004 );
        AddColor( 0x08000001 );
        AddColor( 0x08000002 );
        AddColor( 0x100000f7 );
        EndCount( ESCHER_SplitMenuColors );
    CloseContainer();   // ESCHER_DggContainer

    BYTE i = 2;     // for header/footer and the other
    WW8_WrPlcDrawObj *pSdrObjs = rWrt.pHFSdrObjs;
    pTxtBxs = rWrt.pHFTxtBxs;

    if( !pSdrObjs->GetCntntArr().Count() )      // if no header/footer -> skip over
    {
        --i;
        pSdrObjs = rWrt.pSdrObjs;
        pTxtBxs = rWrt.pTxtBxs;
    }

    for( ; i--; pSdrObjs = rWrt.pSdrObjs, pTxtBxs = rWrt.pTxtBxs )
    {
        // "dummy char" (or any Count ?) - why? This knows only M$
        GetStream() << (sal_Char)i;

        OpenContainer( ESCHER_DgContainer );

            EnterGroup( 0 );

                ULONG nSecondShapeId = pSdrObjs == rWrt.pSdrObjs
                                    ? GetShapeID() : 0;

                // write now all Writer-/DrawObjects
                MakeZOrderArrAndFollowIds( pSdrObjs->GetCntntArr() );

                ULONG nShapeId;
                for( USHORT n = 0; n < aSortFmts.Count(); ++n )
                {
                    const SwFrmFmt& rFmt = *(SwFrmFmt*)aSortFmts[ n ];
                    if( RES_FLYFRMFMT == rFmt.Which() )
                        nShapeId = WriteFlyFrm( rFmt );
                    else
                    {
                        const SdrObject* pObj = rFmt.FindRealSdrObject();
                        if( pObj )
                        {
                            nShapeId = AddSdrObject( *pObj );
                        }
#ifndef PRODUCT
                        else
                            ASSERT( !this, "wo ist das SDR-Object?" );
#endif
                    }

                    if( !nShapeId )
                    {
                        /*!!!*/ const SdrObject* pObj = 0;
                        nShapeId = AddDummyShape( *pObj );
                    }

                    pSdrObjs->SetShapeId( rFmt, nShapeId );
                }

            EndSdrObjectPage();         // ???? Bugfix for 74724

            LeaveGroup();

            if( nSecondShapeId )
            {
                OpenContainer( ESCHER_SpContainer );

                    AddShape( ESCHER_ShpInst_Rectangle, 0xe00, nSecondShapeId );

                    BeginCount();
                    // default Fuellfarbe ist das StarOffice blau7
                    // ----> von DrawingLayer besorgen !!
                    AddOpt( ESCHER_Prop_fillColor, 0xffb800 );
                    AddOpt( ESCHER_Prop_fillBackColor, 0 );
                    AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00100010 );
                    AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
                    AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
                    AddOpt( ESCHER_Prop_lineWidth, 0 );

// winword defaults!
//                  AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
//                  AddOpt( ESCHER_Prop_lineWidth, 0 );
//                  AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
//                  AddOpt( ESCHER_Prop_bWMode, 0x9 );
//                  AddOpt( ESCHER_Prop_fBackground, 0x10001 );

                    EndCount( ESCHER_OPT, 3 );

                    AddAtom( 4, ESCHER_ClientData );
                    GetStream() << 1L;

                CloseContainer();   // ESCHER_SpContainer
            }
        CloseContainer();   // ESCHER_DgContainer
    }
}

SwEscherEx::~SwEscherEx()
{
}

void SwEscherEx::Init()
{
    SdrModel& rModel = *rWrt.pDoc->GetDrawModel();
    // PPT arbeitet nur mit Einheiten zu 576DPI
    // WW hingegen verwendet twips, dh. 1440DPI.
    MapUnit eMap = rModel.GetScaleUnit();

//  Fraction aFact( nAppScale );
//  aFact /= GetMapFactor( MAP_INCH, eMap ).X();
    // create little values
//  aFact = Fraction( aFact.GetNumerator(), aFact.GetDenominator() );

    // Bei 100TH_MM -> 2540/576=635/144
    // Bei Twip     -> 1440/576=5/2
//  nMapMul = aFact.GetNumerator();
//  nMapDiv = aFact.GetDenominator();
//  bNeedMap = nMapMul != nMapDiv;

    // MS-DFF-Properties sind grossteils in EMU (English Metric Units) angegeben
    // 1mm=36000emu, 1twip=635emu
    Fraction aFact( 360, 1 );
//  aFact = Fraction( 360, 1 );
    aFact /= GetMapFactor( MAP_100TH_MM, eMap ).X();
    // create little values
    aFact = Fraction( aFact.GetNumerator(), aFact.GetDenominator() );
    nEmuMul = aFact.GetNumerator();
    nEmuDiv = aFact.GetDenominator();

    // Und noch was fuer typografische Points
//  aFact = Fraction( 1, 1 );
//  aFact /= GetMapFactor(MAP_POINT,eMap).X();
//  nPntMul = aFact.GetNumerator();
//  nPntDiv = aFact.GetDenominator();

    SetHellLayerId( rWrt.pDoc->GetHellId() );
}

void SwEscherEx::WritePictures()
{
    if( pPictStrm )
    {
        // set the blip - entries to the correct stream pos
        INT32 nEndPos = rWrt.Strm().Tell();
        SetNewBlipStreamOffset( nEndPos );

        pPictStrm->Seek( 0 );
        rWrt.Strm() << *pPictStrm;

        delete pPictStrm, pPictStrm = 0;
        rWrt.GetStorage().Remove( sEscherPictStream );
    }
    Flush();
}

void SwEscherEx::FinishEscher()
{
    pEscherStrm->Seek( 0 );
    *rWrt.pTableStrm << *pEscherStrm;

    delete pEscherStrm, pEscherStrm = 0;
    rWrt.GetStorage().Remove( sEscherStream );
}


UINT32 SwEscherEx::WriteFlyFrm( const SwFrmFmt& rFmt )
{
    // check for textflyframe and if it is the first in a Chain
    UINT32 nShapeId = 0;
    const SwNodeIndex* pNdIdx = rFmt.GetCntnt().GetCntntIdx();
    if( pNdIdx )
    {
        SwNodeIndex aIdx( *pNdIdx, 1 );
        switch( aIdx.GetNode().GetNodeType() )
        {
        case ND_GRFNODE:
            WriteGrfFlyFrame( rFmt, nShapeId =GetShapeID() );
            break;

        case ND_OLENODE:
            WriteOLEFlyFrame( rFmt, nShapeId =GetShapeID() );
            break;

        default:
            {
                const SdrObject* pObj = rFmt.FindRealSdrObject();
                if( pObj )
                {
                    // check for the first in a Chain
                    UINT32 nTxtId;
                    USHORT nOff = 0;
                    const SwFrmFmt* pFmt = &rFmt, *pPrev;
                    while( 0 != ( pPrev = pFmt->GetChain().GetPrev() ))
                    {
                        ++nOff;
                        pFmt = pPrev;
                    }

                    nShapeId = GetFlyShapeId( rFmt );
                    if( !nOff )
                    {
                        void* p = (void*)pObj;
                        nTxtId = pTxtBxs->GetPos( p );
                        if( USHRT_MAX == nTxtId )
                        {
                            pTxtBxs->Append( *pObj, nShapeId );
                            nTxtId = pTxtBxs->Count();
                        }
                        else
                            ++nTxtId;
                    }
                    else
                    {
                        const SdrObject* pPrevObj = pFmt->FindRealSdrObject();
                        void* p = (void*)pPrevObj;
                        nTxtId = pTxtBxs->GetPos( p );
                        if( USHRT_MAX == nTxtId )
                        {
                            UINT32 nPrevShapeId = GetFlyShapeId( *pFmt );
                            pTxtBxs->Append( *pPrevObj, nPrevShapeId );
                            nTxtId = pTxtBxs->Count();
                        }
                        else
                            ++nTxtId;
                    }
                    nTxtId *= 0x10000;
                    nTxtId += nOff;

                    WriteTxtFlyFrame( rFmt, nShapeId, nTxtId );
                }
            }
        }
    }
    return nShapeId;
}

void SwEscherEx::WriteTxtFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId,
                                    UINT32 nTxtBox )
{
    OpenContainer( ESCHER_SpContainer );

    AddShape( ESCHER_ShpInst_TextBox, 0xa00, nShapeId );

    BeginCount();

    AddOpt( ESCHER_Prop_lTxid, nTxtBox );

    const VoidPtr pNext = rFmt.GetChain().GetNext();
    if( pNext )
    {
        USHORT nPos = aSortFmts.GetPos( pNext );
        if( USHRT_MAX != nPos && aFollowShpIds[ nPos ] )
            AddOpt( ESCHER_Prop_hspNext, aFollowShpIds[ nPos ] );
    }

    WriteFlyFrameAttr( rFmt );

    EndCount( ESCHER_OPT, 3 );

    AddAtom( 4, ESCHER_ClientAnchor );  GetStream() << 0L;
    AddAtom( 4, ESCHER_ClientData );    GetStream() << 1L;
    AddAtom( 4, ESCHER_ClientTextbox ); GetStream() << nTxtBox;

    CloseContainer();   // ESCHER_SpContainer
}

void SwEscherEx::WriteGrfFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId )
{
    OpenContainer( ESCHER_SpContainer );

    AddShape( ESCHER_ShpInst_PictureFrame, 0xa00, nShapeId );

    BeginCount();

    UINT32 nFlags = ESCHER_BlipFlagDefault;
    SwNodeIndex aIdx( *rFmt.GetCntnt().GetCntntIdx(), 1 );
    SwGrfNode& rGrfNd = *aIdx.GetNode().GetGrfNode();
    if( rGrfNd.IsLinkedFile() )
    {
        String sURL;
        rGrfNd.GetFileFilterNms( &sURL, 0 );

        WW8Bytes aBuf;
        SwWW8Writer::InsAsString16( aBuf, sURL );
        SwWW8Writer::InsUInt16( aBuf, 0 );

        USHORT nArrLen = aBuf.Count();
        BYTE* pArr = new BYTE[ nArrLen ];
        memcpy( pArr, aBuf.GetData(), nArrLen );

        AddOpt( ESCHER_Prop_pibName, TRUE, nArrLen, pArr, nArrLen );
        nFlags = ESCHER_BlipFlagLinkToFile | ESCHER_BlipFlagURL |
                    ESCHER_BlipFlagDoNotSave;
    }
    else
    {
        rGrfNd.SwapIn( TRUE );
        AddOpt( ESCHER_Prop_pib, AddGraphic( *QueryPicStream(),
                                            rGrfNd.GetGrf() ), TRUE );
    }
    AddOpt( ESCHER_Prop_pibFlags, nFlags );

    WriteFlyFrameAttr( rFmt );
    WriteGrfAttr( rGrfNd );

    EndCount( ESCHER_OPT, 3 );

    AddAtom( 4, ESCHER_ClientAnchor );      GetStream() << 0L;
    AddAtom( 4, ESCHER_ClientData );        GetStream() << 1L;

    CloseContainer();   // ESCHER_SpContainer
}

void SwEscherEx::WriteOLEFlyFrame( const SwFrmFmt& rFmt, UINT32 nShapeId )
{
    SwNodeIndex aIdx( *rFmt.GetCntnt().GetCntntIdx(), 1 );
    SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
    const SvInPlaceObjectRef xObj( rOLENd.GetOLEObj().GetOleRef() );
    SvData aData( FORMAT_GDIMETAFILE );
    GDIMetaFile* pMtf;

    const SdrObject* pSdrObj = rFmt.FindRealSdrObject();
    if( pSdrObj && xObj->GetData( &aData ) &&
        aData.GetData( &pMtf, TRANSFER_MOVE ) )
    {
        OpenContainer( ESCHER_SpContainer );

        AddShape( ESCHER_ShpInst_PictureFrame, 0xa10, nShapeId );

        BeginCount();

        {
            SvMemoryStream aGrfStrm;
            WriteWindowMetafile( aGrfStrm, *pMtf );
            const BYTE* pMem = (BYTE*)aGrfStrm.GetData();
            UINT32 nLen = aGrfStrm.GetSize();
            Size aSz( rOLENd.GetTwipSize() );
            aSz.Width() = DrawModelToEmu( aSz.Width() );
            aSz.Height() = DrawModelToEmu( aSz.Height() );
            Rectangle aRect( Point(0,0), aSz );
            AddOpt( ESCHER_Prop_pib, AddWMF( *QueryPicStream(),
                                    pMem + 22, nLen - 22, aRect ), TRUE );
        }
        pTxtBxs->Append( *pSdrObj, nShapeId );
        UINT32 nPicId = pTxtBxs->Count();
        nPicId *= 0x10000;
        AddOpt( ESCHER_Prop_pictureId, nPicId );
        AddOpt( ESCHER_Prop_pictureActive, 0x10000 );

        WriteFlyFrameAttr( rFmt );
        WriteGrfAttr( rOLENd );

        EndCount( ESCHER_OPT, 3 );

        AddAtom( 4, ESCHER_ClientAnchor );      GetStream() << 0L;
        AddAtom( 4, ESCHER_ClientData );        GetStream() << 1L;

        CloseContainer();   // ESCHER_SpContainer
    }
}


void SwEscherEx::WriteGrfAttr( const SwNoTxtNode& rNd )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState( RES_GRFATR_CROPGRF,
                                                        TRUE, &pItem ))
    {
        const Size aSz( rNd.GetTwipSize() );
        INT32 nVal;
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetLeft() ) )
            AddOpt( ESCHER_Prop_cropFromLeft, ToFract16( nVal, aSz.Width()) );
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetRight() ) )
            AddOpt( ESCHER_Prop_cropFromRight, ToFract16( nVal, aSz.Width()));
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetTop() ) )
            AddOpt( ESCHER_Prop_cropFromTop, ToFract16( nVal, aSz.Height()));
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetBottom() ) )
            AddOpt( ESCHER_Prop_cropFromBottom, ToFract16( nVal, aSz.Height()));
    }
    // mirror ??
}

void SwEscherEx::WriteFlyFrameAttr( const SwFrmFmt& rFmt )
{
    const SfxPoolItem* pItem;
    BOOL bFirstLine = TRUE;
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_BOX, TRUE, &pItem ) )
    {
        static UINT16 __READONLY_DATA aExhperProp[ 4 ] = {
            ESCHER_Prop_dyTextTop,  ESCHER_Prop_dyTextBottom,
            ESCHER_Prop_dxTextLeft, ESCHER_Prop_dxTextRight
        };
        const SvxBorderLine* pLine;

        for( int n = 0; n < 4; ++n )
            if( 0 != ( pLine = ((SvxBoxItem*)pItem)->GetLine( n )) )
            {
                if( bFirstLine )
                {
                    UINT32 nLineColor = GetColor( pLine->GetColor(), FALSE );
                    AddOpt( ESCHER_Prop_lineColor, nLineColor );
                    AddOpt( ESCHER_Prop_lineBackColor, nLineColor ^ 0xffffff );

                    UINT32 nLineWidth;
                    ESCHER_LineStyle eStyle;
                    if( pLine->GetInWidth() )
                    {
                        // double line
                        nLineWidth = pLine->GetInWidth() +
                                       pLine->GetOutWidth() +
                                       pLine->GetDistance();
                        if( pLine->GetInWidth() == pLine->GetOutWidth() )
                            eStyle = ESCHER_LineDouble;
                        else if( pLine->GetInWidth() < pLine->GetOutWidth() )
                            eStyle = ESCHER_LineThickThin;
                        else
                            eStyle = ESCHER_LineThinThick;
                    }
                    else
                    {
                        // simple line
                        eStyle = ESCHER_LineSimple;
                        nLineWidth = pLine->GetOutWidth();
                    }

                    AddOpt( ESCHER_Prop_lineStyle, eStyle );
                    AddOpt( ESCHER_Prop_lineWidth, DrawModelToEmu( nLineWidth ));

                    bFirstLine = FALSE;
                }

                AddOpt( aExhperProp[ n ], DrawModelToEmu(
                                ((SvxBoxItem*)pItem)->GetDistance( n ) ));
            }
    }
    if( bFirstLine )                // no valid line found
    {
        AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
        AddOpt( ESCHER_Prop_dyTextTop, 0 );
        AddOpt( ESCHER_Prop_dyTextBottom, 0 );
        AddOpt( ESCHER_Prop_dxTextLeft, 0 );
        AddOpt( ESCHER_Prop_dxTextRight, 0 );
    }

    // write shadow
/*
    WinWord dont know any shadow at textboxes
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_SHADOW, TRUE, &pItem ) )
    {
            mpEscherEx->AddOpt( ESCHER_Prop_fshadowObscured, 0x20002 );
            if( rObj.ImplGetPropertyValue(  C2U("ShadowColor") ) )
                mpEscherEx->AddOpt( ESCHER_Prop_shadowColor,
                    mpEscherEx->GetColor(
                            *((UINT32*)rObj.GetUsrAny().get()) ) );


            if ( rObj.ImplGetPropertyValue( C2U("Shadow") ) )
            {
                if ( rObj.GetUsrAny().getBOOL() )
                {
                    if( rObj.ImplGetPropertyValue( C2U("ShadowXDistance") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOffsetX,
                                    *((INT32*)rObj.GetUsrAny().get()) * 360 );
                    if( rObj.ImplGetPropertyValue( C2U("ShadowYDistance") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOffsetY,
                                    *((INT32*)rObj.GetUsrAny().get()) * 360 );
                    if( rObj.ImplGetPropertyValue( C2U("ShadowTransparence") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOpacity,
                            ( ( 100 - (*((UINT16*)rObj.GetUsrAny().get()) )
                                        << 16 ) / 100 ) );
                }
            }
    }
*/

    if( SFX_ITEM_SET == rFmt.GetItemState( RES_BACKGROUND, TRUE, &pItem ) )
    {
        if( /* ((SvxBrushItem*)pItem)->GetGraphicLink() || */
                ((SvxBrushItem*)pItem)->GetGraphic() )
        {
/*          if( ((SvxBrushItem*)pItem)->GetGraphicLink() )
            {
                AddOpt( ESCHER_Prop_fillBlipName, TRUE,
                // wie jetzt ??
                AddOpt( UINT16 nPropertyID, BOOL bBlib, UINT32 nPropValue,
                        BYTE* pProp, UINT32 nPropSize );
            }
            else
*/
            {
                AddOpt( ESCHER_Prop_fillBlip, AddGraphic( *QueryPicStream(),
                                    *((SvxBrushItem*)pItem)->GetGraphic() ),
                                    TRUE );
            }
            AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
            AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
            AddOpt( ESCHER_Prop_fillBackColor, 0 );
        }
        else if( !((SvxBrushItem*)pItem)->GetColor().GetTransparency() )
        {
            UINT32 nFillColor = GetColor( ((SvxBrushItem*)pItem)->
                                                    GetColor(), FALSE );
            AddOpt( ESCHER_Prop_fillColor, nFillColor );
            AddOpt( ESCHER_Prop_fillBackColor, nFillColor ^ 0xffffff );
            AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );
        }
        else
        {
            AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
        }
    }

    if( SFX_ITEM_SET == rFmt.GetItemState( RES_LR_SPACE, TRUE, &pItem ))
    {
        AddOpt( ESCHER_Prop_dxWrapDistLeft,
                DrawModelToEmu( ((SvxLRSpaceItem*)pItem)->GetLeft() ) );
        AddOpt( ESCHER_Prop_dxWrapDistRight,
                DrawModelToEmu( ((SvxLRSpaceItem*)pItem)->GetRight() ) );
    }
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_UL_SPACE, TRUE, &pItem ))
    {
        AddOpt( ESCHER_Prop_dyWrapDistTop,
                DrawModelToEmu( ((SvxULSpaceItem*)pItem)->GetUpper() ) );
        AddOpt( ESCHER_Prop_dyWrapDistBottom,
                DrawModelToEmu( ((SvxULSpaceItem*)pItem)->GetLower() ) );
    }

    const SdrObject* pObj = rFmt.FindRealSdrObject();
    if( pObj && pObj->GetLayer() == GetHellLayerId() )
        AddOpt( ESCHER_Prop_fPrint, 0x200020 );
}

void SwEscherEx::MakeZOrderArrAndFollowIds( const SvPtrarr& rSrcArr )
{
    if( aSortFmts.Count() )
        aSortFmts.Remove( 0, aSortFmts.Count() );

    USHORT n, nPos, nCnt = rSrcArr.Count();
    SvULongsSort aSort( 255 < nCnt ? 255 : nCnt, 255 );
    for( n = 0; n < nCnt; ++n )
    {
        void* p = rSrcArr[ n ];
        ULONG nOrdNum = rWrt.GetSdrOrdNum( *(SwFrmFmt*)p );
        aSort.Insert( nOrdNum, nPos );
        aSortFmts.Insert( p, nPos );
    }

    if( aFollowShpIds.Count() )
        aFollowShpIds.Remove( 0, aFollowShpIds.Count() );

    ULONG nShapeId;
    const SwFmtChain* pChain;
    for( n = 0; n < nCnt; ++n )
    {
        const SwFrmFmt* pFmt = (SwFrmFmt*)aSortFmts[ n ];

        if( RES_FLYFRMFMT == pFmt->Which() &&
            ( ( pChain = &pFmt->GetChain())->GetPrev() ||
                pChain->GetNext() ) )
        {
            // the format needs a shapeid
            nShapeId = GetShapeID();
        }
        else
            nShapeId = 0;
        aFollowShpIds.Insert( nShapeId, n );
    }
}


UINT32 SwEscherEx::GetFlyShapeId( const SwFrmFmt& rFmt )
{
    const VoidPtr pFmt = (void*)&rFmt;
    USHORT nPos = aSortFmts.GetPos( pFmt );
    UINT32 nShapeId;
    if( USHRT_MAX != nPos )
    {
        if( 0 == ( nShapeId = aFollowShpIds[ nPos ] ))
        {
            nShapeId = GetShapeID();
            aFollowShpIds[ nPos ] = nShapeId;
        }
    }
    else
        nShapeId = GetShapeID();
    return nShapeId;
}

UINT32 SwEscherEx::QueryTextID( const uno::Reference< drawing::XShape>& xXShapeRef, UINT32 nShapeId )
{
    UINT32 nId = 0;
    uno::Reference< lang::XUnoTunnel > xTunnel(xXShapeRef, uno::UNO_QUERY);
    SvxShape* pSvxShape = xTunnel.is() ?
        (SvxShape*)xTunnel->getSomething(SvxShape::getUnoTunnelId()) : 0;
    if( pSvxShape )
    {
        SdrObject* pObj = pSvxShape->GetSdrObject();
        if( pObj )
        {
            pTxtBxs->Append( *pObj, nShapeId );
            nId = pTxtBxs->Count();
            nId *= 0x10000;
        }
    }
    return nId;
}

SvStream* SwEscherEx::QueryPicStream()
{
    if( !pPictStrm )
    {
        pPictStrm = rWrt.GetStorage().OpenStream( sEscherPictStream,
                                STREAM_READWRITE | STREAM_SHARE_DENYALL );
        pPictStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }
    return pPictStrm;
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/wrtw8esh.cxx,v 1.2 2000-09-21 12:18:55 khz Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.23  2000/09/18 16:04:57  willem.vandorp
      OpenOffice header added.

      Revision 1.22  2000/08/25 12:27:31  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.21  2000/07/17 20:28:23  jp
      WriteGrfFlyFrame: alloc buffer for the linked graphics

      Revision 1.20  2000/05/16 17:39:17  jp
      Changes for Unicode

      Revision 1.19  2000/05/12 16:13:51  jp
      Changes for Unicode

      Revision 1.18  2000/04/11 17:35:37  jp
      Bug #74724#: CTOR SwEscherEx - call the end outproc for the SdrPage

      Revision 1.17  2000/03/24 13:55:29  jp
      Bug #74452#: after writing the escher delete the escher object

      Revision 1.16  2000/03/21 15:04:57  os
      UNOIII

      Revision 1.15  2000/03/03 15:20:01  os
      StarView remainders removed

      Revision 1.14  2000/02/23 18:40:03  jp
      Task #73176#: AddWmf will have EMU values

      Revision 1.13  2000/02/11 14:39:51  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.12  1999/11/24 14:55:55  hr
      #65293#: added header

      Revision 1.11  1999/10/14 21:05:57  jp
      Bug #68617#,#68705# and other little things

      Revision 1.10  1999/10/13 09:08:41  jp
      write character attributs of SdrObjects

      Revision 1.9  1999/10/11 10:57:30  jp
      some bugfixes #68745#/#69033#/#68681#

      Revision 1.8  1999/09/10 16:27:59  jp
      write FlyFrms too if no SdrObject exist

      Revision 1.7  1999/09/08 16:29:18  jp
      Bug #68618#: recognize the write of escher stream

      Revision 1.6  1999/09/01 17:32:38  JP
      new: write the text of SdrTextObjects


      Rev 1.5   01 Sep 1999 19:32:38   JP
   new: write the text of SdrTextObjects

      Rev 1.4   31 Aug 1999 09:51:36   JP
   set NumberFormatInt at streams to littleendian

      Rev 1.3   24 Aug 1999 20:15:48   JP
   Escher Export

      Rev 1.2   16 Aug 1999 17:16:30   JP
   W97 Export: experimental escher export (1)

      Rev 1.1   16 Aug 1999 13:05:42   JP
   write EscherObjects - only a testversion!

      Rev 1.0   21 Jul 1999 18:28:56   JP
   W97 Export: experimental escher export

*************************************************************************/

