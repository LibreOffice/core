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

#include <comphelper/string.hxx>
#include <svl/urihelper.hxx>
#include <hintids.hxx>
#include <osl/endian.h>
#include <svx/fmglob.hxx>
#include <svx/sdtaitm.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <svx/xlineit.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdocapt.hxx>
#include <svx/sxctitm.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/svdoattr.hxx>
#include <editeng/brushitem.hxx>
#include <svx/rectenum.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/xfltrit.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <grfatr.hxx>
#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docary.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <docsh.hxx>
#include <mdiexp.hxx>
#include <fmtcnct.hxx>
#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"
#include "ww8graf.hxx"
#include <fmtinfmt.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <fmtfollowtextflow.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/editobj.hxx>
#include <math.h>
#include <fmturl.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/whiter.hxx>
#include <o3tl/enumrange.hxx>
#include <memory>
#include <filter/msfilter/escherex.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace sw::types;
using namespace sw::util;

// helper methods
Color WW8TransCol(SVBT32 nWC)
{
#if 1               // 1 = use predefined color, 0 = ignore

    // color table to convert RGB values to pre-defined colors
    // (to make the writer UI show the right color names)
    // the table is split in base 3, the greys are missing as
    // they don't fit into that system (4 values: bw, wb, 2 * grey)
    static const ColorData eColA[] = {              //  B G R  B G R  B G R
        COL_BLACK, COL_RED, COL_LIGHTRED,           //  0 0 0, 0 0 1, 0 0 2
        COL_GREEN, COL_BROWN, COL_BLACK,            //  0 1 0, 0 1 1, 0 1 2
        COL_LIGHTGREEN, COL_BLACK, COL_YELLOW,      //  0 2 0, 0 2 1, 0 2 2
        COL_BLUE, COL_MAGENTA, COL_BLACK,           //  1 0 0, 1 0 1, 1 0 2
        COL_CYAN, COL_LIGHTGRAY, COL_BLACK,         //  1 1 0, 1 1 1, 1 1 2
        COL_BLACK, COL_BLACK, COL_BLACK,            //  1 2 0, 1 2 1, 1 2 2
        COL_LIGHTBLUE, COL_BLACK, COL_LIGHTMAGENTA, //  2 0 0, 2 0 1, 2 0 2
        COL_BLACK, COL_BLACK, COL_BLACK,            //  2 1 0, 2 1 1, 2 1 2
        COL_LIGHTCYAN, COL_BLACK, COL_WHITE };      //  2 2 0, 2 2 1, 2 2 2

    // In nWC[3] is a byte that's not described in the WW documentation.
    // Its meaning appears to be the following: For 0, it's a normal color
    // whose RGB values are in nWC[0..2]. If nWC[3] is 0x1, 0x7d or 0x83,
    // it's a grey value whose black portion is given in 0.5% in nWC[0].
    // I guess that BIT(0) in nWC[3] is relevant for distinguishing RGB/Grey.

    if( !( nWC[3] & 0x1 ) &&                        // not special (grey)
        (    ( nWC[0] == 0 ||  nWC[0]== 0x80 || nWC[0] == 0xff )    // R
          && ( nWC[1] == 0 ||  nWC[1]== 0x80 || nWC[1] == 0xff )    // G
          && ( nWC[2] == 0 ||  nWC[2]== 0x80 || nWC[2] == 0xff ) ) ){// B
        int nIdx = 0;       // and now: Idx-calculation in base 3
        for (int i = 2; i >= 0; i--)
        {
            nIdx *= 3;
            if (nWC[i])
                nIdx += ((nWC[i] == 0xff) ? 2 : 1);
        }
        if (eColA[nIdx] != COL_BLACK)
            return Color(eColA[nIdx]);  // default color
    }
#endif

    if (nWC[3] & 0x1)
    {
        // Special color gray
        sal_uInt8 u = (sal_uInt8)( (sal_uLong)( 200 - nWC[0] ) * 256 / 200 );
        return Color(u, u, u);
    }

    // User-Color
    return Color(nWC[0], nWC[1], nWC[2]);
}

void wwFrameNamer::SetUniqueGraphName(SwFrameFormat *pFrameFormat, const OUString &rFixed)
{
    if (mbIsDisabled || rFixed.isEmpty())
        return;

    pFrameFormat->SetName(msSeed+OUString::number(++mnImportedGraphicsCount) + ": " + rFixed);
}

// ReadGrafStart reads object data and if necessary creates an anchor
bool SwWW8ImplReader::ReadGrafStart(void* pData, short nDataSiz,
    WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    if (SVBT16ToShort(pHd->cb) < sizeof(WW8_DPHEAD) + nDataSiz)
    {
        OSL_ENSURE( false, "+graphic element: too short?" );
        m_pStrm->SeekRel(SVBT16ToShort(pHd->cb) - sizeof(WW8_DPHEAD));
        return false;
    }

    bool bCouldRead = checkRead(*m_pStrm, pData, nDataSiz);
    OSL_ENSURE(bCouldRead, "Short Graphic header");
    if (!bCouldRead)
        return false;

    SwFormatAnchor aAnchor( FLY_AT_CHAR );
    aAnchor.SetAnchor( m_pPaM->GetPoint() );
    rSet.Put( aAnchor );

    m_nDrawXOfs2 = m_nDrawXOfs;
    m_nDrawYOfs2 = m_nDrawYOfs;

    return true;
}

// SetStdAttr() sets standard attributes
static void SetStdAttr( SfxItemSet& rSet, WW8_DP_LINETYPE& rL,
                        WW8_DP_SHADOW& rSh )
{
    if( SVBT16ToShort( rL.lnps ) == 5 ){            // invisible
        rSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    }else{                                          // visible
        Color aCol( WW8TransCol( rL.lnpc ) );           // line color
        rSet.Put( XLineColorItem( OUString(), aCol ) );
        rSet.Put( XLineWidthItem( SVBT16ToShort( rL.lnpw ) ) );
                                                    // line thickness
        if( SVBT16ToShort( rL.lnps ) >= 1
            && SVBT16ToShort(rL.lnps ) <= 4 ){      // line style
            rSet.Put( XLineStyleItem( drawing::LineStyle_DASH ) );
            sal_Int16 nLen = SVBT16ToShort( rL.lnpw );
            XDash aD( css::drawing::DashStyle_RECT, 1, 2 * nLen, 1, 5 * nLen, 5 * nLen );
            switch( SVBT16ToShort( rL.lnps ) ){
            case 1: aD.SetDots( 0 );            // Dash
                    aD.SetDashLen( 6 * nLen );
                    aD.SetDistance( 4 * nLen );
                    break;
            case 2: aD.SetDashes( 0 ); break;   // Dot
            case 3: break;                      // Dash Dot
            case 4: aD.SetDots( 2 ); break;     // Dash Dot Dot
            }
            rSet.Put( XLineDashItem( OUString(), aD ) );
        }else{
            rSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );  // needed for TextBox
        }
    }
    if( SVBT16ToShort( rSh.shdwpi ) ){                  // shadow
        rSet.Put(makeSdrShadowItem(true));
        rSet.Put( makeSdrShadowXDistItem( SVBT16ToShort( rSh.xaOffset ) ) );
        rSet.Put( makeSdrShadowYDistItem( SVBT16ToShort( rSh.yaOffset ) ) );
    }
}

// SetFill() sets fill attributes such as fore- and background color and
// pattern by reducing to a color
// SetFill() doesn't yet set a pattern, because Sdr can't easily do that
// and the Sdr hatching (XDash) isn't finished yet.
// Instead, a mixed color will be picked that's between the selected ones.
static void SetFill( SfxItemSet& rSet, WW8_DP_FILL& rFill )
{
    static const sal_uInt8 nPatA[] =
    {
             0,  0,  5, 10, 20, 25, 30, 40, 50, 60, 70, 75, 80,
            90, 50, 50, 50, 50, 50, 50, 33, 33, 33, 33, 33, 33
    };
    sal_uInt16 nPat = SVBT16ToShort(rFill.flpp);

    if (nPat == 0) // transparent
        rSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
    else
    {
        rSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));  // necessary for textbox
        if (nPat <= 1 || (SAL_N_ELEMENTS(nPatA) <= nPat))
        {
            // Solid background or unknown
            rSet.Put(XFillColorItem(OUString(), WW8TransCol(rFill.dlpcBg)));
        }
        else
        {                                      // Brush -> color mix
            Color aB( WW8TransCol( rFill.dlpcBg ) );
            Color aF( WW8TransCol( rFill.dlpcFg ) );
            aB.SetRed( (sal_uInt8)( ( (sal_uLong)aF.GetRed() * nPatA[nPat]
                        + (sal_uLong)aB.GetRed() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetGreen( (sal_uInt8)( ( (sal_uLong)aF.GetGreen() * nPatA[nPat]
                        + (sal_uLong)aB.GetGreen() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetBlue( (sal_uInt8)( ( (sal_uLong)aF.GetBlue() * nPatA[nPat]
                        + (sal_uLong)aB.GetBlue() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            rSet.Put( XFillColorItem( OUString(), aB ) );
        }
    }
}

static void SetLineEndAttr( SfxItemSet& rSet, WW8_DP_LINEEND& rLe,
                            WW8_DP_LINETYPE& rLt )
{
    sal_uInt16 aSB = SVBT16ToShort( rLe.aStartBits );
    if( aSB & 0x3 )
    {
        ::basegfx::B2DPolygon aPolygon;
        aPolygon.append(::basegfx::B2DPoint(0.0, 330.0));
        aPolygon.append(::basegfx::B2DPoint(100.0, 0.0));
        aPolygon.append(::basegfx::B2DPoint(200.0, 330.0));
        aPolygon.setClosed(true);
        rSet.Put( XLineEndItem( OUString(), ::basegfx::B2DPolyPolygon(aPolygon) ) );
        sal_uInt16 nSiz = SVBT16ToShort( rLt.lnpw )
                        * ( ( aSB >> 2 & 0x3 ) + ( aSB >> 4 & 0x3 ) );
        if( nSiz < 220 ) nSiz = 220;
        rSet.Put(XLineEndWidthItem(nSiz));
        rSet.Put(XLineEndCenterItem(false));
    }

    sal_uInt16 aEB = SVBT16ToShort( rLe.aEndBits );
    if( aEB & 0x3 ){
        ::basegfx::B2DPolygon aPolygon;
        aPolygon.append(::basegfx::B2DPoint(0.0, 330.0));
        aPolygon.append(::basegfx::B2DPoint(100.0, 0.0));
        aPolygon.append(::basegfx::B2DPoint(200.0, 330.0));
        aPolygon.setClosed(true);
        rSet.Put( XLineStartItem( OUString(), ::basegfx::B2DPolyPolygon(aPolygon) ) );
        sal_uInt16 nSiz = SVBT16ToShort( rLt.lnpw )
                        * ( ( aEB >> 2 & 0x3 ) + ( aEB >> 4 & 0x3 ) );
        if( nSiz < 220 ) nSiz = 220;
        rSet.Put(XLineStartWidthItem(nSiz));
        rSet.Put(XLineStartCenterItem(false));
    }
}

// start of routines for the different objects
SdrObject* SwWW8ImplReader::ReadLine(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    WW8_DP_LINE aLine;

    if( !ReadGrafStart( static_cast<void*>(&aLine), sizeof( aLine ), pHd, rSet ) )
        return nullptr;

    Point aP[2];
    {
        Point& rP0 = aP[0];
        Point& rP1 = aP[1];

        rP0.X() = (sal_Int16)SVBT16ToShort( pHd->xa ) + m_nDrawXOfs2;
        rP0.Y() = (sal_Int16)SVBT16ToShort( pHd->ya ) + m_nDrawYOfs2;
        rP1 = rP0;
        rP0.X() += (sal_Int16)SVBT16ToShort( aLine.xaStart );
        rP0.Y() += (sal_Int16)SVBT16ToShort( aLine.yaStart );
        rP1.X() += (sal_Int16)SVBT16ToShort( aLine.xaEnd );
        rP1.Y() += (sal_Int16)SVBT16ToShort( aLine.yaEnd );
    }

    ::basegfx::B2DPolygon aPolygon;
    aPolygon.append(::basegfx::B2DPoint(aP[0].X(), aP[0].Y()));
    aPolygon.append(::basegfx::B2DPoint(aP[1].X(), aP[1].Y()));
    SdrObject* pObj = new SdrPathObj(OBJ_LINE, ::basegfx::B2DPolyPolygon(aPolygon));

    SetStdAttr( rSet, aLine.aLnt, aLine.aShd );
    SetLineEndAttr( rSet, aLine.aEpp, aLine.aLnt );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadRect(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    WW8_DP_RECT aRect;

    if( !ReadGrafStart( static_cast<void*>(&aRect), sizeof( aRect ), pHd, rSet ) )
        return nullptr;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + m_nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + m_nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrRectObj( Rectangle( aP0, aP1 ) );

    SetStdAttr( rSet, aRect.aLnt, aRect.aShd );
    SetFill( rSet, aRect.aFill );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadElipse(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    WW8_DP_ELIPSE aElipse;

    if( !ReadGrafStart( static_cast<void*>(&aElipse), sizeof( aElipse ), pHd, rSet ) )
        return nullptr;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + m_nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + m_nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrCircObj( OBJ_CIRC, Rectangle( aP0, aP1 ) );

    SetStdAttr( rSet, aElipse.aLnt, aElipse.aShd );
    SetFill( rSet, aElipse.aFill );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadArc(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    WW8_DP_ARC aArc;

    if( !ReadGrafStart( static_cast<void*>(&aArc), sizeof( aArc ), pHd, rSet ) )
        return nullptr;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + m_nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + m_nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa ) * 2;
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya ) * 2;

    short nA[] = { 2, 3, 1, 0 };
    short nW = nA[ ( ( aArc.fLeft & 1 ) << 1 ) + ( aArc.fUp & 1 ) ];
    if( !aArc.fLeft ){
        aP0.Y() -= (sal_Int16)SVBT16ToShort( pHd->dya );
        aP1.Y() -= (sal_Int16)SVBT16ToShort( pHd->dya );
    }
    if( aArc.fUp ){
        aP0.X() -= (sal_Int16)SVBT16ToShort( pHd->dxa );
        aP1.X() -= (sal_Int16)SVBT16ToShort( pHd->dxa );
    }

    SdrObject* pObj = new SdrCircObj( OBJ_SECT, Rectangle( aP0, aP1 ),
                               nW * 9000, ( ( nW + 1 ) & 3 ) * 9000 );

    SetStdAttr( rSet, aArc.aLnt, aArc.aShd );
    SetFill( rSet, aArc.aFill );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadPolyLine(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    WW8_DP_POLYLINE aPoly;

    if( !ReadGrafStart( static_cast<void*>(&aPoly), sizeof( aPoly ), pHd, rSet ) )
        return nullptr;

    sal_uInt16 nCount = SVBT16ToShort( aPoly.aBits1 ) >> 1 & 0x7fff;
    std::unique_ptr<SVBT16[]> xP(new SVBT16[nCount * 2]);

    bool bCouldRead = checkRead(*m_pStrm, xP.get(), nCount * 4);      // read points
    OSL_ENSURE(bCouldRead, "Short PolyLine header");
    if (!bCouldRead)
        return nullptr;

    tools::Polygon aP( nCount );
    Point aPt;
    for (sal_uInt16 i=0; i<nCount; ++i)
    {
        aPt.X() = SVBT16ToShort( xP[i << 1] ) + m_nDrawXOfs2
                  + (sal_Int16)SVBT16ToShort( pHd->xa );
        aPt.Y() = SVBT16ToShort( xP[( i << 1 ) + 1] ) + m_nDrawYOfs2
                  + (sal_Int16)SVBT16ToShort( pHd->ya );
        aP[i] = aPt;
    }
    xP.reset();

    SdrObject* pObj = new SdrPathObj(( SVBT16ToShort( aPoly.aBits1 ) & 0x1 ) ? OBJ_POLY : OBJ_PLIN, ::basegfx::B2DPolyPolygon(aP.getB2DPolygon()));
    SetStdAttr( rSet, aPoly.aLnt, aPoly.aShd );
    SetFill( rSet, aPoly.aFill );

    return pObj;
}

ESelection GetESelection(EditEngine &rDrawEditEngine, long nCpStart, long nCpEnd)
{
    sal_Int32 nPCnt = rDrawEditEngine.GetParagraphCount();
    sal_Int32 nSP = 0;
    sal_Int32 nEP = 0;
    while(      (nSP < nPCnt)
            &&  (nCpStart >= rDrawEditEngine.GetTextLen( nSP ) + 1) )
    {
        nCpStart -= rDrawEditEngine.GetTextLen( nSP ) + 1;
        nSP++;
    }
        // at the end, switch to the new line only 1 character later as
        // otherwise line attributes reach one line too far
    while(      (nEP < nPCnt)
            &&  (nCpEnd > rDrawEditEngine.GetTextLen( nEP ) + 1) )
    {
        nCpEnd -= rDrawEditEngine.GetTextLen( nEP ) + 1;
        nEP++;
    }
    return ESelection( nSP, nCpStart, nEP, nCpEnd );
}

// InsertTxbxStyAttrs() sets style attributes into the passed ItemSet.
// SW styles are used since import-WW-styles are alreaday destroyed.
// SW styles are examined in depth first search order (with parent styles)
// for the attributes given in aSrcTab. They're cloned, and the clones'
// Which-IDs are changed according to the aDstTab table so that the
// EditEngine will not ignore them.
// Both Paragraph and character attributes are stuffed into the ItemSet.
void SwWW8ImplReader::InsertTxbxStyAttrs( SfxItemSet& rS, sal_uInt16 nColl )
{
    SwWW8StyInf * pStyInf = GetStyle(nColl);
    if( pStyInf != nullptr && pStyInf->m_pFormat && pStyInf->m_bColl )
    {
        const SfxPoolItem* pItem;
        for( sal_uInt16 i = POOLATTR_BEGIN; i < POOLATTR_END; i++ )
        {
            // If we are set in the source and not set in the destination
            // then add it in.
            if ( SfxItemState::SET == pStyInf->m_pFormat->GetItemState(
                i, true, &pItem ) )
            {
                SfxItemPool *pEditPool = rS.GetPool();
                sal_uInt16 nWhich = i;
                sal_uInt16 nSlotId = m_rDoc.GetAttrPool().GetSlotId(nWhich);
                if (
                    nSlotId && nWhich != nSlotId &&
                    0 != (nWhich = pEditPool->GetWhich(nSlotId)) &&
                    nWhich != nSlotId &&
                    ( SfxItemState::SET != rS.GetItemState(nWhich, false) )
                   )
                {
                    SfxPoolItem* pCopy = pItem->Clone();
                    pCopy->SetWhich( nWhich );
                    rS.Put( *pCopy );
                    delete pCopy;
                }
            }
        }
    }

}

static void lcl_StripFields(OUString &rString, long &rNewStartCp)
{
    sal_Int32 nStartPos = 0;
    for (;;)
    {
        nStartPos = rString.indexOf(0x13, nStartPos);
        if (nStartPos<0)
            return;

        const sal_Unicode cStops[] = {0x14, 0x15, 0};
        const sal_Int32 nStopPos = comphelper::string::indexOfAny(rString, cStops, nStartPos);
        if (nStopPos<0)
        {
            rNewStartCp += rString.getLength()-nStartPos;
            rString = rString.copy(0, nStartPos);
            return;
        }

        const bool was0x14 = rString[nStopPos]==0x14;
        rString = rString.replaceAt(nStartPos, nStopPos+1-nStartPos, "");
        rNewStartCp += nStopPos-nStartPos;

        if (was0x14)
        {
            ++rNewStartCp;
            nStartPos = rString.indexOf(0x15, nStartPos);
            if (nStartPos<0)
                return;
            rString = rString.replaceAt(nStartPos, 1, "");
        }
    }
}

class Chunk
{
private:
    OUString msURL;
    long mnStartPos; // 0x13
    long mnEndPos;   // 0x15
public:
    explicit Chunk(long nStart, const OUString &rURL)
        : msURL(rURL), mnStartPos(nStart), mnEndPos(0)  {}

    void SetEndPos(long nEnd) { mnEndPos = nEnd; }
    long GetStartPos() const {return mnStartPos;}
    long GetEndPos() const {return mnEndPos;}
    const OUString &GetURL() const {return msURL;}
    void Adjust(sal_Int32 nAdjust)
    {
        mnStartPos-=nAdjust;
        mnEndPos-=nAdjust;
    }
};

// InsertAttrsAsDrawingAttrs() sets attributes between StartCp and EndCp.
// Style attributes are set as hard, paragraph and character attributes.
void SwWW8ImplReader::InsertAttrsAsDrawingAttrs(long nStartCp, long nEndCp,
    ManTypes eType, bool bONLYnPicLocFc)
{
    /*
     Save and create new plcxman for this drawing object, of the type that
     will include the para end mark inside a paragraph property range, as
     drawing boxes have real paragraph marks as part of their text, while
     normal writer has separate nodes for each paragraph and so has no actual
     paragraph mark as part of the paragraph text.
    */
    WW8ReaderSave aSave(this);
    m_pPlcxMan = new WW8PLCFMan(m_pSBase, eType, nStartCp, true);

    WW8_CP nStart = m_pPlcxMan->Where();
    WW8_CP nNext, nStartReplace=0;

    bool bDoingSymbol = false;
    sal_Unicode cReplaceSymbol = m_cSymbol;

    SfxItemSet *pS = new SfxItemSet(m_pDrawEditEngine->GetEmptyItemSet());
    WW8PLCFManResult aRes;

    std::deque<Chunk> aChunks;

    // Here store stack location
    size_t nCurrentCount = m_pCtrlStck->size();
    while (nStart < nEndCp)
    {
        // nStart is the beginning of the attributes for this range, and
        // may be before the text itself. So watch out for that
        WW8_CP nTextStart = nStart;
        if (nTextStart < nStartCp)
            nTextStart = nStartCp;

        // get position of next SPRM
        bool bStartAttr = m_pPlcxMan->Get(&aRes);
        m_nAktColl = m_pPlcxMan->GetColl();
        if (aRes.nSprmId)
        {
            if( bONLYnPicLocFc )
            {
                if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
                {
                    Read_PicLoc(aRes.nSprmId, aRes.pMemPos +
                        m_pSprmParser->DistanceToData(aRes.nSprmId), 4);
                     // Ok, that's what we were looking for.  Now let's get
                     // out of here!
                    break;
                }
            }
            else if ( aRes.nSprmId && (
                (eFTN >  aRes.nSprmId) || (0x0800 <= aRes.nSprmId) ) )
            {
                // Here place them onto our usual stack and we will pop them
                // off and convert them later
                if (bStartAttr)
                {
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
                    if (!bDoingSymbol && m_bSymbol)
                    {
                        bDoingSymbol = true;
                        nStartReplace = nTextStart;
                        cReplaceSymbol = m_cSymbol;
                    }
                }
                else
                {
                    EndSprm( aRes.nSprmId );
                    if (!m_bSymbol && bDoingSymbol)
                    {
                        bDoingSymbol = false;
                        OUStringBuffer sTemp;
                        comphelper::string::padToLength(sTemp,
                            nTextStart - nStartReplace, cReplaceSymbol);
                        m_pDrawEditEngine->QuickInsertText(sTemp.makeStringAndClear(),
                            GetESelection(*m_pDrawEditEngine, nStartReplace - nStartCp,
                            nTextStart - nStartCp ) );
                    }
                }
            }
            else if (aRes.nSprmId == eFLD)
            {
                if (bStartAttr)
                {
                    size_t nCount = m_pCtrlStck->size();
                    if (m_aFieldStack.empty() && Read_Field(&aRes))
                    {
                        OUString sURL;
                        for (size_t nI = m_pCtrlStck->size(); nI > nCount; --nI)
                        {
                            const SfxPoolItem *pItem = ((*m_pCtrlStck)[nI-1]).pAttr.get();
                            sal_uInt16 nWhich = pItem->Which();
                            if (nWhich == RES_TXTATR_INETFMT)
                            {
                                const SwFormatINetFormat *pURL =
                                    static_cast<const SwFormatINetFormat *>(pItem);
                                sURL = pURL->GetValue();
                            }
                            m_pCtrlStck->DeleteAndDestroy(nI-1);
                        }
                        aChunks.push_back(Chunk(nStart, sURL));
                    }
                }
                else
                {
                    if (!m_aFieldStack.empty() && End_Field())
                        aChunks.back().SetEndPos(nStart+1);
                }
            }
        }

        m_pPlcxMan->advance();
        nNext = m_pPlcxMan->Where();

        const WW8_CP nEnd = ( nNext < nEndCp ) ? nNext : nEndCp;
        if (!bONLYnPicLocFc && nNext != nStart && nEnd >= nStartCp)
        {
            SfxItemPool *pEditPool = pS->GetPool();

            // Here read current properties and convert them into pS
            // and put those attrs into the draw box if they can be converted
            // to draw attributes
            if (m_pCtrlStck->size() - nCurrentCount)
            {
                for (size_t i = nCurrentCount; i < m_pCtrlStck->size(); ++i)
                {
                    const SfxPoolItem *pItem = ((*m_pCtrlStck)[i]).pAttr.get();
                    sal_uInt16 nWhich = pItem->Which();
                    if( nWhich < RES_FLTRATTR_BEGIN ||
                        nWhich >= RES_FLTRATTR_END )
                    {
                        sal_uInt16 nSlotId = m_rDoc.GetAttrPool().GetSlotId(nWhich);
                        if (
                            nSlotId && nWhich != nSlotId &&
                            0 != (nWhich = pEditPool->GetWhich(nSlotId)) &&
                            nWhich != nSlotId
                        )
                        {
                            SfxPoolItem* pCopy = pItem->Clone();
                            pCopy->SetWhich( nWhich );
                            pS->Put( *pCopy );
                            delete pCopy;
                        }
                    }
                }
            }
            // Fill in the remainder from the style
            InsertTxbxStyAttrs(*pS, m_nAktColl);

            if( pS->Count() )
            {
                m_pDrawEditEngine->QuickSetAttribs( *pS,
                    GetESelection(*m_pDrawEditEngine, nTextStart - nStartCp, nEnd - nStartCp ) );
                delete pS;
                pS = new SfxItemSet(m_pDrawEditEngine->GetEmptyItemSet());
            }
        }
        nStart = nNext;
    }
    delete pS;

    // pop off as far as recorded location just in case there were some left
    // unclosed
    for (size_t nI = m_pCtrlStck->size(); nI > nCurrentCount; --nI)
        m_pCtrlStck->DeleteAndDestroy(nI-1);

    typedef std::deque<Chunk>::iterator myIter;
    myIter aEnd = aChunks.end();
    for (myIter aIter = aChunks.begin(); aIter != aEnd; ++aIter)
    {
        ESelection aSel(GetESelection(*m_pDrawEditEngine, aIter->GetStartPos()-nStartCp,
            aIter->GetEndPos()-nStartCp));
        OUString aString(m_pDrawEditEngine->GetText(aSel));
        const sal_Int32 nOrigLen = aString.getLength();
        long nDummy(0);
        lcl_StripFields(aString, nDummy);

        sal_Int32 nChanged;
        if (!aIter->GetURL().isEmpty())
        {
            SvxURLField aURL(aIter->GetURL(), aString,
                SVXURLFORMAT_APPDEFAULT);
            m_pDrawEditEngine->QuickInsertField(SvxFieldItem(aURL, EE_FEATURE_FIELD), aSel);
            nChanged = nOrigLen - 1;
        }
        else
        {
            m_pDrawEditEngine->QuickInsertText(aString, aSel);
            nChanged = nOrigLen - aString.getLength();
        }
        for (myIter aIter2 = aIter+1; aIter2 != aEnd; ++aIter2)
            aIter2->Adjust(nChanged);
    }

    /*
     Don't worry about the new pPlcxMan, the restorer removes it when
     replacing the current one with the old one.
    */
    aSave.Restore(this);
}

bool SwWW8ImplReader::GetTxbxTextSttEndCp(WW8_CP& rStartCp, WW8_CP& rEndCp,
    sal_uInt16 nTxBxS, sal_uInt16 nSequence)
{
    // grab the TextBox-PLCF quickly
    WW8PLCFspecial* pT = m_pPlcxMan ? m_pPlcxMan->GetTxbx() : nullptr;
    if( !pT )
    {
        OSL_ENSURE( false, "+where's the text graphic (1)?" );
        return false;
    }

    // ggfs. zuerst die richtige TextBox-Story finden
    bool bCheckTextBoxStory = ( nTxBxS && pT->GetIMax() >= nTxBxS );
    if(  bCheckTextBoxStory )
        pT->SetIdx( nTxBxS-1 );

    // then determine start and end
    void* pT0;
    if( !pT->Get( rStartCp, pT0 ) )
    {
        OSL_ENSURE( false, "+where's the text graphic (2)?" );
        return false;
    }

    if( bCheckTextBoxStory )
    {
        bool bReusable = (0 != SVBT16ToShort( static_cast<WW8_TXBXS*>(pT0)->fReusable ));
        while( bReusable )
        {
            pT->advance();
            if( !pT->Get( rStartCp, pT0 ) )
            {
                OSL_ENSURE( false, "+where's the text graphic (2a)?" );
                return false;
            }
            bReusable = (0 != SVBT16ToShort( static_cast<WW8_TXBXS*>(pT0)->fReusable ));
        }
    }
    pT->advance();
    if( !pT->Get( rEndCp, pT0 ) )
    {
        OSL_ENSURE( false, "+where's the text graphic (3)?" );
        return false;
    }

    // find the right page in the break table (if necessary)
    if( bCheckTextBoxStory )
    {
        // special case: entire chain should be determined - done!
        if( USHRT_MAX > nSequence )
        {
            long nMinStartCp = rStartCp;
            long nMaxEndCp   = rEndCp;
            // rasch den TextBox-Break-Deskriptor-PLCF greifen
            pT = m_pPlcxMan->GetTxbxBkd();
            if (!pT) // It can occur on occasion, Caolan
                return false;

            // find first entry for this TextBox story
            if( !pT->SeekPos( rStartCp ) )
            {
                OSL_ENSURE( false, "+where's the text graphic (4)" );
                return false;
            }
            // if needed skip the appropriate number of entries
            for (sal_uInt16 iSequence = 0; iSequence < nSequence; ++iSequence)
                pT->advance();
            // and determine actual start and end
            if(    (!pT->Get( rStartCp, pT0 ))
                || ( nMinStartCp > rStartCp  ) )
            {
                OSL_ENSURE( false, "+where's the text graphic (5)?" );
                return false;
            }
            if( rStartCp >= nMaxEndCp )
                rEndCp = rStartCp;  // not an error: empty string
            else
            {
                pT->advance();
                if ( (!pT->Get(rEndCp, pT0)) || (nMaxEndCp < rEndCp-1) )
                {
                    OSL_ENSURE( false, "+where's the text graphic (6)?" );
                    return false;
                }
                rEndCp -= 1;
            }
        }
        else
            rEndCp -= 1;
    }
    else
        rEndCp -= 1;
    return true;
}

// TxbxText() grabs the text from the WW file and returns that along with
// the StartCp and the corrected (by -2, or -1 for version 8) EndCp.
sal_Int32 SwWW8ImplReader::GetRangeAsDrawingString(OUString& rString, long nStartCp, long nEndCp, ManTypes eType)
{
    WW8_CP nOffset = 0;
    m_pWwFib->GetBaseCp(eType, &nOffset); //TODO: check return value

    OSL_ENSURE(nStartCp <= nEndCp, "+Wo ist der Grafik-Text (7) ?");
    if (nStartCp == nEndCp)
        rString.clear();      // empty string: entirely possible
    else if (nStartCp < nEndCp)
    {
        // read the text: can be split into multiple pieces
        const sal_Int32 nLen = m_pSBase->WW8ReadString(*m_pStrm, rString,
            nStartCp + nOffset, nEndCp - nStartCp, GetCurrentCharSet());
        OSL_ENSURE(nLen, "+where's the text graphic (8)?");
        if (nLen>0)
        {
            if( rString[nLen-1]==0x0d )
                rString = rString.copy(0, nLen-1);

            rString = rString.replace( 0xb, 0xa );
            return nLen;
        }
    }
    return 0;
}

//EditEngine::InsertText will replace dos lines resulting in a shorter
//string than is passed in, so inserting attributes based on the original
//string len can fail. So here replace the dos line ends similar to
//how EditEngine does it, but preserve the length and replace the extra
//chars with placeholders, record the position of the placeholders and
//remove those extra chars after attributes have been inserted
std::vector<sal_Int32> replaceDosLineEndsButPreserveLength(OUString &rIn)
{
    OUStringBuffer aNewData(rIn);
    std::vector<sal_Int32> aDosLineEndDummies;
    sal_Int32 i = 0;
    sal_Int32 nStrLen = rIn.getLength();
    while (i < nStrLen)
    {
        // \r or \n causes linebreak
        if (rIn[i] == '\r' || rIn[i] == '\n')
        {
            // skip char if \r\n or \n\r
            if ( (i+1) < nStrLen && ((rIn[i+1] == '\r') || (rIn[i+1] == '\n')) &&
                 (rIn[i] != rIn[i+1]) )
            {
                ++i;
                aDosLineEndDummies.push_back(i);
                aNewData[i] = 0;
            }
        }
        ++i;
    }
    rIn = aNewData.makeStringAndClear();
    return aDosLineEndDummies;
}

void removePositions(EditEngine &rDrawEditEngine, const std::vector<sal_Int32>& rDosLineEndDummies)
{
    for (auto aIter = rDosLineEndDummies.rbegin(); aIter != rDosLineEndDummies.rend(); ++aIter)
    {
        sal_Int32 nCharPos(*aIter);
        rDrawEditEngine.QuickDelete(GetESelection(rDrawEditEngine, nCharPos, nCharPos+1));
    }
}

OutlinerParaObject* SwWW8ImplReader::ImportAsOutliner(OUString &rString, WW8_CP nStartCp, WW8_CP nEndCp, ManTypes eType)
{
    OutlinerParaObject* pRet = nullptr;

    sal_Int32 nLen = GetRangeAsDrawingString(rString, nStartCp, nEndCp, eType);
    if (nLen > 0)
    {
        if (!m_pDrawEditEngine)
            m_pDrawEditEngine = new EditEngine(nullptr);

        //replace dos line endings with editeng ones, replace any extra chars with
        //placeholders to keep the inserted string len in sync with the attribute cps
        //and record in aDosLineEnds the superfluous positions
        OUString sEEString(rString);
        std::vector<sal_Int32> aDosLineEnds(replaceDosLineEndsButPreserveLength(sEEString));
        m_pDrawEditEngine->SetText(sEEString);
        InsertAttrsAsDrawingAttrs(nStartCp, nStartCp+nLen, eType);
        //remove any superfluous placeholders of replaceDosLineEndsButPreserveLength
        //after attributes have been inserted
        removePositions(*m_pDrawEditEngine, aDosLineEnds);

        // Annotations typically begin with a (useless) 0x5
        if ((eType == MAN_AND) && m_pDrawEditEngine->GetTextLen())
        {
            ESelection aFirstChar(0, 0, 0, 1);
            if (m_pDrawEditEngine->GetText( aFirstChar ) == "\x05")
                m_pDrawEditEngine->QuickDelete(aFirstChar);
        }

        EditTextObject* pTemporaryText = m_pDrawEditEngine->CreateTextObject();
        pRet = new OutlinerParaObject(*pTemporaryText);
        pRet->SetOutlinerMode( OutlinerMode::TextObject );
        delete pTemporaryText;

        m_pDrawEditEngine->SetText( OUString() );
        m_pDrawEditEngine->SetParaAttribs(0, m_pDrawEditEngine->GetEmptyItemSet());

        // Strip out fields, leaving the result
        long nDummy(0);
        lcl_StripFields(rString, nDummy);
        // Strip out word's special characters for the simple string
        rString = rString.replaceAll("\x01", "");
        rString = rString.replaceAll("\x05", "");
        rString = rString.replaceAll("\x08", "");
        rString = rString.replaceAll("\007\007", "\007\012");
        rString = rString.replace(0x7, ' ');
    }

    return pRet;
}

// InsertTxbxText() fuegt fuer TextBoxen und CaptionBoxen den Text
// und die Attribute ein
void SwWW8ImplReader::InsertTxbxText(SdrTextObj* pTextObj,
    Size* pObjSiz, sal_uInt16 nTxBxS, sal_uInt16 nSequence, long nPosCp,
    SwFrameFormat* pOldFlyFormat, bool bMakeSdrGrafObj, bool& rbEraseTextObj,
    bool* pbTestTxbxContainsText, long* pnStartCp, long* pnEndCp,
    bool* pbContainsGraphics, SvxMSDffImportRec* pRecord)
{
    SwFrameFormat* pFlyFormat = nullptr;
    sal_uLong nOld = m_pStrm->Tell();

    ManTypes eType = m_pPlcxMan->GetManType() == MAN_HDFT ? MAN_TXBX_HDFT : MAN_TXBX;

    rbEraseTextObj = false;

    OUString aString;
    WW8_CP nStartCp, nEndCp;
    bool bContainsGraphics = false;
    bool bTextWasRead = GetTxbxTextSttEndCp(nStartCp, nEndCp, nTxBxS, nSequence) &&
                        GetRangeAsDrawingString(aString, nStartCp, nEndCp, eType) > 0;

    if (!m_pDrawEditEngine)
        m_pDrawEditEngine = new EditEngine(nullptr);
    if( pObjSiz )
        m_pDrawEditEngine->SetPaperSize( *pObjSiz );

    const OUString aOrigString(aString);
    if( bTextWasRead )
    {
        long nNewStartCp = nStartCp;
        lcl_StripFields(aString, nNewStartCp);

        if (aString.getLength()!=1)
        {
            bContainsGraphics = aString.indexOf(0x1)<0 || aString.indexOf(0x8)<0;
        }
        else        // May be a single graphic or object
        {
            bool bDone = true;
            switch( aString[0] )
            {
                case 0x1:
                    if (!pbTestTxbxContainsText)
                    {
                        WW8ReaderSave aSave(this, nNewStartCp -1);
                        bool bOldEmbeddObj = m_bEmbeddObj;
                        // bEmbedd Ordinarily would have been set by field
                        // parse, but this is impossible here so...
                        m_bEmbeddObj = true;

                        // 1st look for OLE- or Graph-Indicator Sprms
                        WW8PLCFx_Cp_FKP* pChp = m_pPlcxMan->GetChpPLCF();
                        WW8PLCFxDesc aDesc;
                        pChp->GetSprms( &aDesc );
                        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen,
                            *m_pSprmParser);

                        for( int nLoop = 0; nLoop < 2; ++nLoop )
                        {
                            while (aSprmIter.GetSprms())
                            {
                                const sal_uInt8 *const pParams(aSprmIter.GetAktParams());
                                if (nullptr == pParams)
                                    break;
                                sal_uInt16 nAktId = aSprmIter.GetAktId();
                                switch( nAktId )
                                {
                                    case     75:
                                    case    118:
                                    case 0x080A:
                                    case 0x0856:
                                            Read_Obj(nAktId, pParams, 1);
                                        break;
                                    case     68:  // Read_Pic()
                                    case 0x6A03:
                                    case 0x680E:
                                            Read_PicLoc(nAktId, pParams, 1);
                                        break;
                                }
                                aSprmIter.advance();
                            }

                            if( !nLoop )
                            {
                                pChp->GetPCDSprms(  aDesc );
                                aSprmIter.SetSprms( aDesc.pMemPos,
                                    aDesc.nSprmsLen );
                            }
                        }
                        aSave.Restore(this);
                        m_bEmbeddObj=bOldEmbeddObj;

                        // then import either an OLE of a Graphic
                        if( m_bObj )
                        {
                            if( bMakeSdrGrafObj && pTextObj &&
                                pTextObj->GetUpGroup() )
                            {
                                // SdrOleObj/SdrGrafObj anstatt des
                                // SdrTextObj in dessen Gruppe einsetzen

                                Graphic aGraph;
                                SdrObject* pNew = ImportOleBase(aGraph);

                                if( !pNew )
                                {
                                    pNew = new SdrGrafObj;
                                    static_cast<SdrGrafObj*>(pNew)->SetGraphic(aGraph);
                                }

                                GrafikCtor();

                                pNew->SetModel( m_pDrawModel );
                                pNew->SetLogicRect( pTextObj->GetCurrentBoundRect() );
                                pNew->SetLayer( pTextObj->GetLayer() );

                                pTextObj->GetUpGroup()->GetSubList()->
                                    ReplaceObject(pNew, pTextObj->GetOrdNum());
                            }
                            else
                                pFlyFormat = ImportOle();
                            m_bObj = false;
                        }
                        else
                        {
                            InsertAttrsAsDrawingAttrs(nNewStartCp, nNewStartCp+1,
                                eType, true);
                            pFlyFormat = ImportGraf(bMakeSdrGrafObj ? pTextObj : nullptr,
                                pOldFlyFormat);
                        }
                    }
                    break;
                case 0x8:
                    if ( (!pbTestTxbxContainsText) && (!m_bObj) )
                        pFlyFormat = Read_GrafLayer( nPosCp );
                    break;
                default:
                    bDone = false;
                    break;
            }

            if( bDone )
            {
                if( pFlyFormat )
                {
                    if( pRecord )
                    {
                        SfxItemSet aFlySet( m_rDoc.GetAttrPool(),
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1 );

                        Rectangle aInnerDist(   pRecord->nDxTextLeft,
                                                pRecord->nDyTextTop,
                                                pRecord->nDxTextRight,
                                                pRecord->nDyTextBottom  );
                        MatchSdrItemsIntoFlySet( pTextObj,
                                                 aFlySet,
                                                 pRecord->eLineStyle,
                                                 pRecord->eLineDashing,
                                                 pRecord->eShapeType,
                                                 aInnerDist );

                        pFlyFormat->SetFormatAttr( aFlySet );

                        MapWrapIntoFlyFormat(pRecord, pFlyFormat);
                    }
                }
                aString.clear();
                rbEraseTextObj = (nullptr != pFlyFormat);
            }
        }
    }

    if( pnStartCp )
        *pnStartCp = nStartCp;
    if( pnEndCp )
        *pnEndCp = nEndCp;

    if( pbTestTxbxContainsText )
        *pbTestTxbxContainsText = bTextWasRead && ! rbEraseTextObj;
    else if( !rbEraseTextObj )
    {
        if( bTextWasRead )
        {
            m_pDrawEditEngine->SetText(aOrigString);
            InsertAttrsAsDrawingAttrs(nStartCp, nEndCp, eType);
        }

        bool bVertical = pTextObj->IsVerticalWriting();
        EditTextObject* pTemporaryText = m_pDrawEditEngine->CreateTextObject();
        OutlinerParaObject* pOp = new OutlinerParaObject(*pTemporaryText);
        pOp->SetOutlinerMode( OutlinerMode::TextObject );
        pOp->SetVertical( bVertical );
        delete pTemporaryText;
        pTextObj->NbcSetOutlinerParaObject( pOp );
        pTextObj->SetVerticalWriting(bVertical);

        // Fuer die naechste Textbox noch die alten Absatz-Attribute
        // und Styles entfernen, sonst startet die naechste Box
        // mit falschen Attributen.
        // Vorgehen: Text loeschen = auf 1 Absatz reduzieren
        // und an diesem Absatz die Absatzattribute und Styles loeschen
        // (Empfehlung JOE)
        m_pDrawEditEngine->SetText( OUString() );
        m_pDrawEditEngine->SetParaAttribs(0, m_pDrawEditEngine->GetEmptyItemSet());
    }

    m_pStrm->Seek( nOld );
    if (pbContainsGraphics)
        *pbContainsGraphics = bContainsGraphics;
}

bool SwWW8ImplReader::TxbxChainContainsRealText(sal_uInt16 nTxBxS, long& rStartCp,
    long&  rEndCp)
{
    bool bErase, bContainsText;
    InsertTxbxText( nullptr,nullptr,nTxBxS,USHRT_MAX,0,nullptr,false, bErase, &bContainsText,
        &rStartCp, &rEndCp );
    return bContainsText;
}

// TextBoxes only for Ver67 !!
SdrObject* SwWW8ImplReader::ReadTextBox(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    bool bDummy;
    WW8_DP_TXTBOX aTextB;

    if( !ReadGrafStart( static_cast<void*>(&aTextB), sizeof( aTextB ), pHd, rSet ) )
        return nullptr;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + m_nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + m_nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrRectObj( OBJ_TEXT, Rectangle( aP0, aP1 ) );
    pObj->SetModel( m_pDrawModel );
    pObj->NbcSetSnapRect(Rectangle(aP0, aP1));
    Size aSize( (sal_Int16)SVBT16ToShort( pHd->dxa ) ,
        (sal_Int16)SVBT16ToShort( pHd->dya ) );

    long nStartCpFly,nEndCpFly;
    bool bContainsGraphics;
    InsertTxbxText(dynamic_cast<SdrTextObj*>(pObj), &aSize, 0, 0, 0, nullptr, false,
        bDummy,nullptr,&nStartCpFly,&nEndCpFly,&bContainsGraphics);

    SetStdAttr( rSet, aTextB.aLnt, aTextB.aShd );
    SetFill( rSet, aTextB.aFill );

    rSet.Put( SdrTextFitToSizeTypeItem( SdrFitToSizeType::NONE ) );
    rSet.Put( makeSdrTextAutoGrowWidthItem(false));
    rSet.Put( makeSdrTextAutoGrowHeightItem(false));
    rSet.Put( makeSdrTextLeftDistItem(  MIN_BORDER_DIST*2 ) );
    rSet.Put( makeSdrTextRightDistItem( MIN_BORDER_DIST*2 ) );
    rSet.Put( makeSdrTextUpperDistItem( MIN_BORDER_DIST ) );
    rSet.Put( makeSdrTextLowerDistItem( MIN_BORDER_DIST ) );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadCaptionBox(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    static const SdrCaptionType aCaptA[] = { SdrCaptionType::Type1, SdrCaptionType::Type2,
                                       SdrCaptionType::Type3, SdrCaptionType::Type4 };

    WW8_DP_CALLOUT_TXTBOX aCallB;

    if( !ReadGrafStart( static_cast<void*>(&aCallB), sizeof( aCallB ), pHd, rSet ) )
        return nullptr;

    sal_uInt16 nCount = SVBT16ToShort( aCallB.dpPolyLine.aBits1 ) >> 1 & 0x7fff;
    std::unique_ptr<SVBT16[]> xP(new SVBT16[nCount * 2]);

    bool bCouldRead = checkRead(*m_pStrm, xP.get(), nCount * 4);      // Punkte einlesen
    OSL_ENSURE(bCouldRead, "Short CaptionBox header");
    if (!bCouldRead)
        return nullptr;

    sal_uInt8 nTyp = (sal_uInt8)nCount - 1;
    if( nTyp == 1 && SVBT16ToShort( xP[0] ) == SVBT16ToShort( xP[2] ) )
        nTyp = 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) +
               (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.xa ) + m_nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya )
               + (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.ya ) + m_nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dya );
    Point aP2( (sal_Int16)SVBT16ToShort( pHd->xa )
                + (sal_Int16)SVBT16ToShort( aCallB.dpheadPolyLine.xa )
                + m_nDrawXOfs2 + (sal_Int16)SVBT16ToShort( xP[0] ),
               (sal_Int16)SVBT16ToShort( pHd->ya )
               + (sal_Int16)SVBT16ToShort( aCallB.dpheadPolyLine.ya )
               + m_nDrawYOfs2 + (sal_Int16)SVBT16ToShort( xP[1] ) );
    xP.reset();

    SdrCaptionObj* pObj = new SdrCaptionObj( Rectangle( aP0, aP1 ), aP2 );
    pObj->SetModel( m_pDrawModel );
    pObj->NbcSetSnapRect(Rectangle(aP0, aP1));
    Size aSize( (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dxa ),
                           (sal_Int16)SVBT16ToShort(  aCallB.dpheadTxbx.dya ) );
    bool bEraseThisObject;

    InsertTxbxText(pObj, &aSize, 0, 0, 0, nullptr, false, bEraseThisObject );

    if( SVBT16ToShort( aCallB.dptxbx.aLnt.lnps ) != 5 ) // Umrandung sichtbar ?
        SetStdAttr( rSet, aCallB.dptxbx.aLnt, aCallB.dptxbx.aShd );
    else                                                // nein -> Nimm Linie
        SetStdAttr( rSet, aCallB.dpPolyLine.aLnt, aCallB.dptxbx.aShd );
    SetFill( rSet, aCallB.dptxbx.aFill );
    rSet.Put( SdrCaptionTypeItem( aCaptA[nTyp] ) );

    return pObj;
}

SdrObject *SwWW8ImplReader::ReadGroup(WW8_DPHEAD* pHd, SfxAllItemSet &rSet)
{
    sal_Int16 nGrouped;

    if( !ReadGrafStart( static_cast<void*>(&nGrouped), sizeof( nGrouped ), pHd, rSet ) )
        return nullptr;

#ifdef OSL_BIGENDIAN
    nGrouped = (sal_Int16)OSL_SWAPWORD( nGrouped );
#endif

    m_nDrawXOfs = m_nDrawXOfs + (sal_Int16)SVBT16ToShort( pHd->xa );
    m_nDrawYOfs = m_nDrawYOfs + (sal_Int16)SVBT16ToShort( pHd->ya );

    SdrObject* pObj = new SdrObjGroup;

    short nLeft = (sal_Int16)SVBT16ToShort( pHd->cb ) - sizeof( WW8_DPHEAD );
    for (int i = 0; i < nGrouped; i++)
    {
        SfxAllItemSet aSet(m_pDrawModel->GetItemPool());
        if (SdrObject *pObject = ReadGrafPrimitive(nLeft, aSet))
        {
            // first add and then set ItemSet
            SdrObjList *pSubGroup = pObj->GetSubList();
            OSL_ENSURE(pSubGroup, "Why no sublist available?");
            if (pSubGroup)
                pSubGroup->InsertObject(pObject, 0);
            pObject->SetMergedItemSetAndBroadcast(aSet);
        }
    }

    m_nDrawXOfs = m_nDrawXOfs - (sal_Int16)SVBT16ToShort( pHd->xa );
    m_nDrawYOfs = m_nDrawYOfs - (sal_Int16)SVBT16ToShort( pHd->ya );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadGrafPrimitive(short& rLeft, SfxAllItemSet &rSet)
{
    // This whole archaic word 6 graphic import can probably be refactored
    // into an object hierarachy with a little effort.
    SdrObject *pRet=nullptr;
    WW8_DPHEAD aHd;                         // Lese Draw-Primitive-Header
    bool bCouldRead = checkRead(*m_pStrm, &aHd, sizeof(WW8_DPHEAD));
    OSL_ENSURE(bCouldRead, "Graphic Primitive header short read" );
    if (!bCouldRead)
    {
        rLeft=0;
        return pRet;
    }

    if( rLeft >= SVBT16ToShort(aHd.cb) )    // Vorsichtsmassmahme
    {
        rSet.Put(SwFormatSurround(SURROUND_THROUGHT));
        switch (SVBT16ToShort(aHd.dpk) & 0xff )
        {
            case 0:
                pRet = ReadGroup(&aHd, rSet);
                break;
            case 1:
                pRet = ReadLine(&aHd, rSet);
                break;
            case 2:
                pRet = ReadTextBox(&aHd, rSet);
                break;
            case 3:
                pRet = ReadRect(&aHd, rSet);
                break;
            case 4:
                pRet = ReadElipse(&aHd, rSet);
                break;
            case 5:
                pRet = ReadArc(&aHd, rSet);
                break;
            case 6:
                pRet = ReadPolyLine(&aHd, rSet);
                break;
            case 7:
                pRet = ReadCaptionBox(&aHd, rSet);
                break;
            default:    // unknown
                m_pStrm->SeekRel(SVBT16ToShort(aHd.cb) - sizeof(WW8_DPHEAD));
                break;
        }
    }
    else
    {
        OSL_ENSURE( false, "+Grafik-Overlap" );
    }
    rLeft = rLeft - SVBT16ToShort( aHd.cb );
    return pRet;
}

void SwWW8ImplReader::ReadGrafLayer1( WW8PLCFspecial* pPF, long nGrafAnchorCp )
{
    pPF->SeekPos( nGrafAnchorCp );
    WW8_FC nStartFc;
    void* pF0;
    if( !pPF->Get( nStartFc, pF0 ) )
    {
        OSL_ENSURE( false, "+Wo ist die Grafik (2) ?" );
        return;
    }
    WW8_FDOA* pF = static_cast<WW8_FDOA*>(pF0);
    if( !SVBT32ToUInt32( pF->fc ) )
    {
        OSL_ENSURE( false, "+Wo ist die Grafik (3) ?" );
        return;
    }

    bool bCouldSeek = checkSeek(*m_pStrm, SVBT32ToUInt32(pF->fc));
    OSL_ENSURE(bCouldSeek, "Invalid Graphic offset");
    if (!bCouldSeek)
        return;

    // Lese Draw-Header
    WW8_DO aDo;
    bool bCouldRead = checkRead(*m_pStrm, &aDo, sizeof(WW8_DO));
    OSL_ENSURE(bCouldRead, "Short Graphic header");
    if (!bCouldRead)
        return;

    short nLeft = SVBT16ToShort( aDo.cb ) - sizeof( WW8_DO );
    while (nLeft > static_cast<short>(sizeof(WW8_DPHEAD)))
    {
        SfxAllItemSet aSet( m_pDrawModel->GetItemPool() );
        if (SdrObject *pObject = ReadGrafPrimitive(nLeft, aSet))
        {
            m_pWWZOrder->InsertDrawingObject(pObject, SVBT16ToShort(aDo.dhgt));

            Rectangle aRect(pObject->GetSnapRect());

            const sal_uInt32 nCntRelTo = 3;

            // Adjustment is horizontally relative to...
            static const sal_Int16 aHoriRelOriTab[nCntRelTo] =
            {
                text::RelOrientation::PAGE_PRINT_AREA, // 0 is page textarea margin
                text::RelOrientation::PAGE_FRAME,   // 1 is page margin
                text::RelOrientation::FRAME,          // 2 is relative to paragraph
            };

            // Adjustment is vertically relative to...
            static const sal_Int16 aVertRelOriTab[nCntRelTo] =
            {
                text::RelOrientation::PAGE_PRINT_AREA, // 0 is page textarea margin
                text::RelOrientation::PAGE_FRAME,   // 1 is page margin
                text::RelOrientation::FRAME,          // 2 is relative to paragraph
            };

            const int nXAlign = aDo.bx < nCntRelTo ? aDo.bx : 0;
            const int nYAlign = aDo.by < nCntRelTo ? aDo.by : 0;

            aSet.Put(SwFormatHoriOrient(aRect.Left(), text::HoriOrientation::NONE,
                aHoriRelOriTab[ nXAlign ]));
            aSet.Put(SwFormatVertOrient(aRect.Top(), text::VertOrientation::NONE,
                aVertRelOriTab[ nYAlign ]));

            SwFrameFormat *pFrame = m_rDoc.getIDocumentContentOperations().InsertDrawObj( *m_pPaM, *pObject, aSet );
            pObject->SetMergedItemSet(aSet);

            if (dynamic_cast< const SwDrawFrameFormat *>( pFrame ) !=  nullptr)
            {
                static_cast<SwDrawFrameFormat*>(pFrame)->PosAttrSet();
            }

            AddAutoAnchor(pFrame);
        }
    }
}

sal_Int32 SwMSDffManager::GetEscherLineMatch(MSO_LineStyle eStyle,
    MSO_SPT eShapeType, sal_Int32 &rThick)
{
    sal_Int32 nOutsideThick = 0;
    /*
    Beachte: im Gegensatz zu den Winword-ueblichen Tabellen- und
    Rahmen-Randbreiten-Angaben, bei denen jeweils aus der Staerke *einer*
    Linie die Gesamt-Randbreite zu errechnen ist, liegen die aus dem ESCHER
    stammenden Daten bereits als Gesamt-Breite [twips] vor!

    Der Winword default ist 15 tw. Wir nehmen hierfuer unsere 20 tw Linie.  (
    0.75 pt uns 1.0 pt sehen sich auf dem Ausdruck naemlich aehnlicher als
    etwas 0.75 pt und unsere 0.05 pt Haarlinie. ) Die Haarlinie setzen wir nur
    bei Winword-Staerken bis zu maximal 0.5 pt ein.
    */
    switch( eStyle )
    {
    case mso_lineTriple:
    case mso_lineSimple:
        nOutsideThick = eShapeType != mso_sptTextBox ? rThick : rThick/2;
        break;
    case mso_lineDouble:
        if (eShapeType == mso_sptTextBox)
        {
            nOutsideThick = rThick/6;
            rThick = rThick*2/3;
        }
        else
            nOutsideThick = rThick*2/3;
        break;
    case mso_lineThickThin:
        if (eShapeType == mso_sptTextBox)
        {
            nOutsideThick = rThick*3/10;
            rThick = rThick*4/5;
        }
        else
            nOutsideThick = rThick*4/5;
        break;
    case mso_lineThinThick:
        {
        if (eShapeType == mso_sptTextBox)
        {
            nOutsideThick = rThick/10;
            rThick = rThick*3/5;
        }
        else
            nOutsideThick = rThick*3/5;
        }
        break;
    default:
        break;
    }
    return nOutsideThick;
}

// Returns the thickness of the line outside the frame, the logic of
// words positioning of borders around floating objects is that of a
// disturbed mind.
sal_Int32 SwWW8ImplReader::MatchSdrBoxIntoFlyBoxItem(const Color& rLineColor,
    MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType, sal_Int32 &rLineThick,
    SvxBoxItem& rBox )
{
    sal_Int32 nOutsideThick = 0;
    if( !rLineThick )
        return nOutsideThick;

    ::editeng::SvxBorderStyle nIdx = table::BorderLineStyle::NONE;

    sal_Int32 nLineThick=rLineThick;
    nOutsideThick = SwMSDffManager::GetEscherLineMatch(eLineStyle,
        eShapeType, rLineThick);

    /*
    Beachte: im Gegensatz zu den Winword-ueblichen Tabellen- und
    Rahmen-Randbreiten-Angaben, bei denen jeweils aus der Staerke *einer*
    Linie die Gesamt-Randbreite zu errechnen ist, liegen die aus dem ESCHER
    stammenden Daten bereits als Gesamt-Breite [twips] vor!

    Der Winword default ist 15 tw. Wir nehmen hierfuer unsere 20 tw Linie.  (
    0.75 pt uns 1.0 pt sehen sich auf dem Ausdruck naemlich aehnlicher als
    etwas 0.75 pt und unsere 0.05 pt Haarlinie. ) Die Haarlinie setzen wir nur
    bei Winword-Staerken bis zu maximal 0.5 pt ein.
    */
    switch( +eLineStyle )
    {
    // zuerst die Einzel-Linien
    case mso_lineSimple:
        nIdx = table::BorderLineStyle::SOLID;
    break;
    // dann die Doppel-Linien, fuer die wir feine Entsprechungen haben :-)))
    case mso_lineDouble:
        nIdx = table::BorderLineStyle::DOUBLE;
    break;
    case mso_lineThickThin:
        nIdx = table::BorderLineStyle::THICKTHIN_SMALLGAP;
    break;
    case mso_lineThinThick:
        nIdx = table::BorderLineStyle::THINTHICK_SMALLGAP;
    break;
    // We have no triple border, use double instead.
    case mso_lineTriple:
        nIdx = table::BorderLineStyle::DOUBLE;
    break;
    // no line style is set
    case (MSO_LineStyle)USHRT_MAX:
        break;
    // erroneously not implemented line style is set
    default:
        OSL_ENSURE(false, "eLineStyle is not (yet) implemented!");
        break;
    }

    switch( eDashing )
    {
        case mso_lineDashGEL:
            nIdx = table::BorderLineStyle::DASHED;
            break;
        case mso_lineDotGEL:
            nIdx = table::BorderLineStyle::DOTTED;
            break;
        default:
            break;
    }

    if (table::BorderLineStyle::NONE != nIdx)
    {
        SvxBorderLine aLine;
        aLine.SetColor( rLineColor );

        aLine.SetWidth( nLineThick ); // No conversion here, nLineThick is already in twips
        aLine.SetBorderLineStyle(nIdx);

        for(SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>())
        {
            // aLine is cloned by SetLine
            rBox.SetLine(&aLine, nLine);
        }
    }

    return nOutsideThick;
}

#define WW8ITEMVALUE(ItemSet,Id,Cast)  static_cast<const Cast&>((ItemSet).Get(Id)).GetValue()

void SwWW8ImplReader::MatchSdrItemsIntoFlySet( SdrObject* pSdrObj,
    SfxItemSet& rFlySet, MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType,
    Rectangle& rInnerDist )
{
/*
    am Rahmen zu setzende Frame-Attribute
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SwFormatFrameSize            falls noch nicht gesetzt, hier setzen
    SvxLRSpaceItem          hier setzen
    SvxULSpaceItem          hier setzen
    SvxOpaqueItem           (Derzeit bei Rahmen nicht moeglich! khz 10.2.1999)
    SwFormatSurround           bereits gesetzt
    SwFormatVertOrient         bereits gesetzt
    SwFormatHoriOrient         bereits gesetzt
    SwFormatAnchor             bereits gesetzt
    SvxBoxItem              hier setzen
    SvxBrushItem            hier setzen
    SvxShadowItem           hier setzen
*/

    // 1. GrafikObjekt des Docs?
    GrafikCtor();

    const SfxItemSet& rOldSet = pSdrObj->GetMergedItemSet();

    // einige Items koennen direkt so uebernommen werden
    const sal_uInt16 nDirectMatch = 2;
    static RES_FRMATR const aDirectMatch[ nDirectMatch ] =
    {
        RES_LR_SPACE,   // Aussenabstand links/rechts: SvxLRSpaceItem
        RES_UL_SPACE    // Aussenabstand Oben/unten:   SvxULSpaceItem
    };
    const SfxPoolItem* pPoolItem;
    for(RES_FRMATR i : aDirectMatch)
        if( SfxItemState::SET == rOldSet.GetItemState(
                    static_cast< sal_uInt16 >(i), false, &pPoolItem) )
        {
            rFlySet.Put( *pPoolItem );
        }

    // jetzt die Umrandung berechnen und die Box bauen: Das Mass wird fuer die
    // Rahmen-GROESSE benoetigt!
    SvxBoxItem aBox(sw::util::ItemGet<SvxBoxItem>(rFlySet, RES_BOX));
    // dashed oder solid wird zu solid
    // WW-default: 0.75 pt = 15 twips
    sal_Int32 nLineThick = 15, nOutside=0;

    // check if LineStyle is *really* set!
    const SfxPoolItem* pItem;

    SfxItemState eState = rOldSet.GetItemState(XATTR_LINESTYLE,true,&pItem);
    if( eState == SfxItemState::SET )
    {
        // Now, that we know there is a line style we will make use the
        // parameter given to us when calling the method...  :-)
        const Color aLineColor = static_cast< XLineColorItem const & >(
            rOldSet.Get(XATTR_LINECOLOR)).GetColorValue();
        nLineThick = WW8ITEMVALUE(rOldSet, XATTR_LINEWIDTH, XLineWidthItem);

        if( !nLineThick )
            nLineThick = 1; // for Writer, zero is "no border", so set a minimal value

        nOutside = MatchSdrBoxIntoFlyBoxItem(aLineColor, eLineStyle,
            eDashing, eShapeType, nLineThick, aBox);
    }

    rInnerDist.Left()+=nLineThick;
    rInnerDist.Top()+=nLineThick;
    rInnerDist.Right()+=nLineThick;
    rInnerDist.Bottom()+=nLineThick;

    rInnerDist.Left()   -= aBox.CalcLineWidth( SvxBoxItemLine::LEFT );
    rInnerDist.Top()    -= aBox.CalcLineWidth( SvxBoxItemLine::TOP );
    rInnerDist.Right()  -= aBox.CalcLineWidth( SvxBoxItemLine::RIGHT );
    rInnerDist.Bottom() -= aBox.CalcLineWidth( SvxBoxItemLine::BOTTOM );

    // set distances from box's border to text contained within the box
    if( 0 < rInnerDist.Left() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Left(), SvxBoxItemLine::LEFT );
    if( 0 < rInnerDist.Top() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Top(), SvxBoxItemLine::TOP );
    if( 0 < rInnerDist.Right() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Right(), SvxBoxItemLine::RIGHT );
    if( 0 < rInnerDist.Bottom() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Bottom(), SvxBoxItemLine::BOTTOM );

    bool bFixSize = !(WW8ITEMVALUE(rOldSet, SDRATTR_TEXT_AUTOGROWHEIGHT,
        SdrOnOffItem));

    // Size: SwFormatFrameSize
    if( SfxItemState::SET != rFlySet.GetItemState(RES_FRM_SIZE, false) )
    {
        const Rectangle& rSnapRect = pSdrObj->GetSnapRect();
        // if necessary adapt width and position of the framework: The
        // recorded interior is to remain equally large despite thick edges.
        rFlySet.Put( SwFormatFrameSize(bFixSize ? ATT_FIX_SIZE : ATT_VAR_SIZE,
            rSnapRect.GetWidth()  + 2*nOutside,
            rSnapRect.GetHeight() + 2*nOutside) );
    }
    else // If a size is set, adjust it to consider border thickness
    {
        SwFormatFrameSize aSize = static_cast<const SwFormatFrameSize &>(rFlySet.Get(RES_FRM_SIZE));

        SwFormatFrameSize aNewSize = SwFormatFrameSize(bFixSize ? ATT_FIX_SIZE : ATT_VAR_SIZE,
            aSize.GetWidth()  + 2*nOutside,
            aSize.GetHeight() + 2*nOutside);
        aNewSize.SetWidthSizeType(aSize.GetWidthSizeType());
        rFlySet.Put( aNewSize );
    }

    // Sadly word puts escher borders outside the graphic, but orients the
    // graphic in relation to the top left inside the border. We don't
    if (nOutside)
    {
        SwFormatHoriOrient aHori = static_cast<const SwFormatHoriOrient &>(rFlySet.Get(
            RES_HORI_ORIENT));
        aHori.SetPos(MakeSafePositioningValue(aHori.GetPos()-nOutside));
        rFlySet.Put(aHori);

        SwFormatVertOrient aVert = static_cast<const SwFormatVertOrient &>(rFlySet.Get(
            RES_VERT_ORIENT));
        aVert.SetPos(aVert.GetPos()-nOutside);
        rFlySet.Put(aVert);
    }

    // jetzt die Umrandung setzen
    rFlySet.Put( aBox );

    // Schattenwurf der Box: SvxShadowItem
    if( WW8ITEMVALUE(rOldSet, SDRATTR_SHADOW, SdrOnOffItem) )
    {
        SvxShadowItem aShadow( RES_SHADOW );

        const Color aShdColor = static_cast< XColorItem const & >(
            rOldSet.Get(SDRATTR_SHADOWCOLOR)).GetColorValue();
        const sal_Int32 nShdDistX = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWXDIST,
            SdrMetricItem);
        const sal_Int32 nShdDistY = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWYDIST,
            SdrMetricItem);

        aShadow.SetColor( Color( aShdColor ) );

        aShadow.SetWidth(writer_cast<sal_uInt16>((std::abs( nShdDistX) +
            std::abs( nShdDistY )) / 2 ));

        SvxShadowLocation eShdPosi;
        if( 0 <= nShdDistX )
        {
            if( 0 <= nShdDistY )
                eShdPosi = SVX_SHADOW_BOTTOMRIGHT;
            else
                eShdPosi = SVX_SHADOW_TOPRIGHT;
        }
        else
        {
            if( 0 <= nShdDistY )
                eShdPosi = SVX_SHADOW_BOTTOMLEFT;
            else
                eShdPosi = SVX_SHADOW_TOPLEFT;
        }
        aShadow.SetLocation( eShdPosi );

        rFlySet.Put( aShadow );
    }
    Color Temp(COL_WHITE);
    SvxBrushItem aBrushItem(Temp, RES_BACKGROUND);
    bool bBrushItemOk = false;
    sal_uInt8 nTrans = 0;

    // Separate transparency
    eState = rOldSet.GetItemState(XATTR_FILLTRANSPARENCE, true, &pItem);
    if (eState == SfxItemState::SET)
    {
        sal_uInt16 nRes = WW8ITEMVALUE(rOldSet, XATTR_FILLTRANSPARENCE,
            XFillTransparenceItem);
        nTrans = sal_uInt8((nRes * 0xFE) / 100);
        aBrushItem.GetColor().SetTransparency(nTrans);
        bBrushItemOk = true;
    }

    // Hintergrund: SvxBrushItem
    eState = rOldSet.GetItemState(XATTR_FILLSTYLE, true, &pItem);
    if (eState == SfxItemState::SET)
    {
        const drawing::FillStyle eFill = static_cast<const XFillStyleItem*>(pItem)->GetValue();

        switch (eFill)
        {
            default:
            case drawing::FillStyle_NONE:
                // Writer graphics don't have it yet
                if (eShapeType != mso_sptPictureFrame)
                {
                    aBrushItem.GetColor().SetTransparency(0xFE);
                    bBrushItemOk = true;
                }
            break;
            case drawing::FillStyle_SOLID:
            case drawing::FillStyle_GRADIENT:
                {
                    const Color aColor = static_cast< XFillColorItem const & >(
                        rOldSet.Get(XATTR_FILLCOLOR)).GetColorValue();
                    aBrushItem.SetColor(aColor);

                    if (bBrushItemOk) // has trans
                        aBrushItem.GetColor().SetTransparency(nTrans);

                    bBrushItemOk = true;
                }
            break;
            case drawing::FillStyle_HATCH:
            break;
            case drawing::FillStyle_BITMAP:
                {
                    GraphicObject aGrfObj(static_cast< XFillBitmapItem const & >(rOldSet.Get(XATTR_FILLBITMAP)).GetGraphicObject());
                    const bool bTile(WW8ITEMVALUE(rOldSet, XATTR_FILLBMP_TILE, SfxBoolItem));

                    if(bBrushItemOk) // has trans
                    {
                        GraphicAttr aAttr(aGrfObj.GetAttr());

                        aAttr.SetTransparency(nTrans);
                        aGrfObj.SetAttr(aAttr);
                    }

                    aBrushItem.SetGraphicObject(aGrfObj);
                    aBrushItem.SetGraphicPos(bTile ? GPOS_TILED : GPOS_AREA);
                    bBrushItemOk = true;
                }
            break;
        }
    }

    if (bBrushItemOk)
        rFlySet.Put(aBrushItem);
}

void SwWW8ImplReader::AdjustLRWrapForWordMargins(
    const SvxMSDffImportRec &rRecord, SvxLRSpaceItem &rLR)
{
    sal_uInt32 nXRelTo = SvxMSDffImportRec::RELTO_DEFAULT;
    if ( rRecord.pXRelTo )
    {
        nXRelTo = *(rRecord.pXRelTo);
    }

    // Left adjustments - if horizontally aligned to left of
    // margin or column then remove the left wrapping
    if (rRecord.nXAlign == 1)
    {
        if ((nXRelTo == 0) || (nXRelTo == 2))
            rLR.SetLeft((sal_uInt16)0);
    }

    // Right adjustments - if horizontally aligned to right of
    // margin or column then remove the right wrapping
    if (rRecord.nXAlign == 3)
    {
        if ((nXRelTo == 0) || (nXRelTo == 2))
            rLR.SetRight((sal_uInt16)0);
    }

    // Inside margin, remove left wrapping
    if ((rRecord.nXAlign == 4) && (nXRelTo == 0))
    {
        rLR.SetLeft((sal_uInt16)0);
    }

    // Outside margin, remove left wrapping
    if ((rRecord.nXAlign == 5) && (nXRelTo == 0))
    {
        rLR.SetRight((sal_uInt16)0);
    }
}

void SwWW8ImplReader::AdjustULWrapForWordMargins(
    const SvxMSDffImportRec &rRecord, SvxULSpaceItem &rUL)
{
    sal_uInt32 nYRelTo = SvxMSDffImportRec::RELTO_DEFAULT;
    if ( rRecord.pYRelTo )
    {
        nYRelTo = *(rRecord.pYRelTo);
    }

    // Top adjustment - remove upper wrapping if aligned to page
    // printable area or to page
    if (rRecord.nYAlign == 1)
    {
        if ((nYRelTo == 0) || (nYRelTo == 1))
            rUL.SetUpper((sal_uInt16)0);
    }

    // Bottom adjustment - remove bottom wrapping if aligned to page or
    // printable area or to page
    if (rRecord.nYAlign == 3)
    {
        if ((nYRelTo == 0) || (nYRelTo == 1))
            rUL.SetLower((sal_uInt16)0);
    }

    // Remove top margin if aligned vertically inside margin
    if ((rRecord.nYAlign == 4) && (nYRelTo == 0))
        rUL.SetUpper((sal_uInt16)0);
}

void SwWW8ImplReader::MapWrapIntoFlyFormat(SvxMSDffImportRec* pRecord,
    SwFrameFormat* pFlyFormat)
{
    if (!pRecord || !pFlyFormat)
        return;

    if (pRecord->nDxWrapDistLeft || pRecord->nDxWrapDistRight)
    {
        SvxLRSpaceItem aLR(writer_cast<sal_uInt16>(pRecord->nDxWrapDistLeft),
            writer_cast<sal_uInt16>(pRecord->nDxWrapDistRight), 0, 0, RES_LR_SPACE);
        AdjustLRWrapForWordMargins(*pRecord, aLR);
        pFlyFormat->SetFormatAttr(aLR);
    }
    if (pRecord->nDyWrapDistTop || pRecord->nDyWrapDistBottom)
    {
        SvxULSpaceItem aUL(writer_cast<sal_uInt16>(pRecord->nDyWrapDistTop),
            writer_cast<sal_uInt16>(pRecord->nDyWrapDistBottom), RES_UL_SPACE);
        AdjustULWrapForWordMargins(*pRecord, aUL);
        pFlyFormat->SetFormatAttr(aUL);
    }

    // If we are contoured and have a custom polygon...
    if (pRecord->pWrapPolygon && pFlyFormat->GetSurround().IsContour())
    {
        if (SwNoTextNode *pNd = GetNoTextNodeFromSwFrameFormat(*pFlyFormat))
        {

            /*
             Gather round children and hear of a tale that will raise the
             hairs on the back of your neck this dark halloween night.

             There is a polygon in word that describes the wraping around
             the graphic.

             Here are some sample values for the simplest case of a square
             around some solid coloured graphics

                                X       Y       Pixel size of graphic
                TopLeft         -54     21600   400x400
                Bottom Right    0       21546

                TopLeft         -108    21600   200x200
                Bottom Right    0       21492

                TopLeft         -216    21600   100x100
                Bottom Right    0       21384

                TopLeft         -432    21600   50x50
                Bottom Right    0       21168

                TopLeft         -76     21600   283x212
                Bottom Right    0       21498

             So given that the size of the values remains pretty much the
             same despite the size of the graphic, we can tell that the
             polygon is measured in units that are independent of the
             graphic. But why does the left corner move a different value
             to the left each time, and why does the bottom move upwards
             each time, when the right and top remain at the same value ?

             I have no idea, but clearly once we calculate the values out
             we see that the left margin is always a fixed realworld
             distance from the true left and the polygon bottom is the same
             fixed value from the bottom. i.e. 15twips.

             So here we take our word provided polygon, shift it to the
             right by 15twips and rescale it widthwise to shrink the width
             a little to fit the now moved right margin back to where it
             was, and stretch the height a little to make the bottom move
             down the missing 15twips then we get a polygon that matches
             what I actually see in word
            */

            tools::PolyPolygon aPoly(*pRecord->pWrapPolygon);
            const Size &rSize = pNd->GetTwipSize();
            /*
             Move to the left by 15twips, and rescale to
             a) shrink right bound back to orig position
             b) stretch bottom bound to where I think it should have been
             in the first place
            */
            Fraction aMoveHack(ww::nWrap100Percent, rSize.Width());
            aMoveHack *= Fraction(15, 1);
            long nMove(aMoveHack);
            aPoly.Move(nMove, 0);

            Fraction aHackX(ww::nWrap100Percent, ww::nWrap100Percent + nMove);
            Fraction aHackY(ww::nWrap100Percent, ww::nWrap100Percent - nMove);
            aPoly.Scale(aHackX, aHackY);

            // Turn polygon back into units that match the graphic's
            const Size &rOrigSize = pNd->GetGraphic().GetPrefSize();
            Fraction aMapPolyX(rOrigSize.Width(), ww::nWrap100Percent);
            Fraction aMapPolyY(rOrigSize.Height(), ww::nWrap100Percent);
            aPoly.Scale(aMapPolyX, aMapPolyY);

            // #i47277# - contour is already in unit of the
            // graphic preferred unit. Thus, call method <SetContour(..)>
            pNd->SetContour(&aPoly);
        }
    }
}

static sal_Int32 lcl_ConvertCrop(sal_uInt32 const nCrop, sal_Int32 const nSize)
{
    // cast to sal_Int32 to handle negative crop properly
    sal_Int32 const nIntegral(static_cast<sal_Int32>(nCrop) >> 16);
    // fdo#77454: heuristic to detect mangled values written by old OOo/LO
    if (abs(nIntegral) >= 50) // FIXME: what's a good cut-off?
    {
        SAL_INFO("sw.ww8", "ignoring suspiciously large crop: " << nIntegral);
        return 0;
    }
    return (nIntegral * nSize) + (((nCrop & 0xffff) * nSize) >> 16);
}

void
SwWW8ImplReader::SetAttributesAtGrfNode(SvxMSDffImportRec const*const pRecord,
    SwFrameFormat *pFlyFormat, WW8_FSPA *pF )
{
    const SwNodeIndex* pIdx = pFlyFormat->GetContent(false).GetContentIdx();
    SwGrfNode *const pGrfNd(
        pIdx ? m_rDoc.GetNodes()[pIdx->GetIndex() + 1]->GetGrfNode() : nullptr);
    if (pGrfNd)
    {
        Size aSz(pGrfNd->GetTwipSize());
        // use type <sal_uInt64> instead of sal_uLong to get correct results
        // in the following calculations.
        sal_uInt64 rHeight = aSz.Height();
        sal_uInt64 rWidth  = aSz.Width();
        if( !rWidth && pF)
            rWidth  = pF->nXaRight  - pF->nXaLeft;
        else if( !rHeight && pF)
            rHeight = pF->nYaBottom - pF->nYaTop;

        if( pRecord->nCropFromTop || pRecord->nCropFromBottom ||
            pRecord->nCropFromLeft || pRecord->nCropFromRight )
        {
            SwCropGrf aCrop;            // Cropping is stored in 'fixed floats'
                                        // 16.16 (fraction times total
            if( pRecord->nCropFromTop ) //        image width or height resp.)
            {
                aCrop.SetTop(lcl_ConvertCrop(pRecord->nCropFromTop, rHeight));
            }
            if( pRecord->nCropFromBottom )
            {
                aCrop.SetBottom(lcl_ConvertCrop(pRecord->nCropFromBottom, rHeight));
            }
            if( pRecord->nCropFromLeft )
            {
                aCrop.SetLeft(lcl_ConvertCrop(pRecord->nCropFromLeft, rWidth));
            }
            if( pRecord->nCropFromRight )
            {
                aCrop.SetRight(lcl_ConvertCrop(pRecord->nCropFromRight,rWidth));
            }

            pGrfNd->SetAttr( aCrop );
        }

        bool bFlipH = pRecord->nFlags & SHAPEFLAG_FLIPH;
        bool bFlipV = pRecord->nFlags & SHAPEFLAG_FLIPV;
        if ( bFlipH || bFlipV )
        {
            SwMirrorGrf aMirror = pGrfNd->GetSwAttrSet().GetMirrorGrf();
            if( bFlipH )
            {
                if( bFlipV )
                    aMirror.SetValue(RES_MIRROR_GRAPH_BOTH);
                else
                    aMirror.SetValue(RES_MIRROR_GRAPH_VERT);
            }
            else
                aMirror.SetValue(RES_MIRROR_GRAPH_HOR);

            pGrfNd->SetAttr( aMirror );
        }

        if (pRecord->pObj)
        {
            const SfxItemSet& rOldSet = pRecord->pObj->GetMergedItemSet();
            // contrast
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFCONTRAST,
                SdrGrafContrastItem))
            {
                SwContrastGrf aContrast(
                    WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFCONTRAST, SdrGrafContrastItem));
                pGrfNd->SetAttr( aContrast );
            }

            // luminance
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFLUMINANCE,
                SdrGrafLuminanceItem))
            {
                SwLuminanceGrf aLuminance(WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFLUMINANCE, SdrGrafLuminanceItem));
                pGrfNd->SetAttr( aLuminance );
            }
            // gamma
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFGAMMA, SdrGrafGamma100Item))
            {
                double fVal = WW8ITEMVALUE(rOldSet, SDRATTR_GRAFGAMMA,
                    SdrGrafGamma100Item);
                pGrfNd->SetAttr(SwGammaGrf(fVal/100.));
            }

            // drawmode
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFMODE, SdrGrafModeItem))
            {
                SwDrawModeGrf aDrawMode( static_cast< sal_uInt16 >(WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFMODE, SdrGrafModeItem)) );
                pGrfNd->SetAttr( aDrawMode );
            }
        }
    }
}

SdrObject* SwWW8ImplReader::CreateContactObject(SwFrameFormat* pFormat)
{
    if(pFormat)
    {
        SdrObject* pNewObject = m_bNewDoc ? nullptr : pFormat->FindRealSdrObject();
        if (!pNewObject)
            pNewObject = pFormat->FindSdrObject();
        SwFlyFrameFormat* pFlyFormat(nullptr);
        if (!pNewObject && (pFlyFormat = dynamic_cast<SwFlyFrameFormat*>(pFormat)))
        {
            pFlyFormat->InitContact(m_pDrawModel);
            pNewObject = pFlyFormat->GetContact()->GetMaster();
        }
        return pNewObject;
    }
    return nullptr;
}

// Miserable miserable hack to fudge word's graphic layout in RTL mode to ours.
bool SwWW8ImplReader::MiserableRTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel)
{
    if (!IsRightToLeft())
        return false;
    return RTLGraphicsHack(rLeft, nWidth, eHoriOri, eHoriRel,
            m_aSectionManager.GetPageLeft(),
            m_aSectionManager.GetPageRight(),
            m_aSectionManager.GetPageWidth());
}

RndStdIds SwWW8ImplReader::ProcessEscherAlign(SvxMSDffImportRec* pRecord,
    WW8_FSPA *pFSPA, SfxItemSet &rFlySet, bool /*bOrgObjectWasReplace*/)
{
    OSL_ENSURE(pRecord || pFSPA, "give me something! to work with for anchoring");
    if (!pRecord && !pFSPA)
        return FLY_AT_PAGE;
    bool bCurSectionVertical = m_aSectionManager.CurrentSectionIsVertical();

    SvxMSDffImportRec aRecordFromFSPA;
    if (!pRecord)
        pRecord = &aRecordFromFSPA;
    if (!(pRecord->pXRelTo) && pFSPA)
    {
        pRecord->pXRelTo = new sal_uInt32;
        *(pRecord->pXRelTo) = pFSPA->nbx;
    }
    if (!(pRecord->pYRelTo) && pFSPA)
    {
        pRecord->pYRelTo = new sal_uInt32;
        *(pRecord->pYRelTo) = pFSPA->nby;
    }

    // nXAlign - abs. Position, Left,  Centered,  Right,  Inside, Outside
    // nYAlign - abs. Position, Top,   Centered,  Bottom, Inside, Outside

    // nXRelTo - Page printable area, Page,  Column,    Character
    // nYRelTo - Page printable area, Page,  Paragraph, Line

    const sal_uInt32 nCntXAlign = 6;
    const sal_uInt32 nCntYAlign = 6;

    const sal_uInt32 nCntRelTo  = 4;

    sal_uInt32 nXAlign = nCntXAlign > pRecord->nXAlign ? pRecord->nXAlign : 1;
    sal_uInt32 nYAlign = nCntYAlign > pRecord->nYAlign ? pRecord->nYAlign : 1;

    if (pFSPA)
    {
        // #i52565# - try to handle special case for objects in tables regarding its X Rel

        // if X and Y Rel values are on default take it as a hint, that they have not been set
        // by <SwMSDffManager::ProcessObj(..)>
        const bool bXYRelHaveDefaultValues = *(pRecord->pXRelTo) == 2 && *(pRecord->pYRelTo) == 2;
        if ( bXYRelHaveDefaultValues
             && m_nInTable > 0
             && !bCurSectionVertical )
        {
            if ( pFSPA->nby != *(pRecord->pYRelTo) )
            {
                *(pRecord->pYRelTo) = pFSPA->nby;
            }
        }
    }

    sal_uInt32 nXRelTo = (pRecord->pXRelTo && nCntRelTo > *(pRecord->pXRelTo)) ? *(pRecord->pXRelTo) : 1;
    sal_uInt32 nYRelTo = (pRecord->pYRelTo && nCntRelTo > *(pRecord->pYRelTo)) ? *(pRecord->pYRelTo) : 1;

    RndStdIds eAnchor = IsInlineEscherHack() ? FLY_AS_CHAR : FLY_AT_CHAR; // #i43718#

    SwFormatAnchor aAnchor( eAnchor );
    aAnchor.SetAnchor( m_pPaM->GetPoint() );
    rFlySet.Put( aAnchor );

    if (pFSPA)
    {
        // #i18732#
        // Given new layout where everything is changed to be anchored to
        // character the following 4 tables may need to be changed.

        // horizontal Adjustment
        static const sal_Int16 aHoriOriTab[ nCntXAlign ] =
        {
            text::HoriOrientation::NONE,     // From left position
            text::HoriOrientation::LEFT,     // left
            text::HoriOrientation::CENTER,   // centered
            text::HoriOrientation::RIGHT,    // right
            // #i36649#
            // - inside -> text::HoriOrientation::LEFT and outside -> text::HoriOrientation::RIGHT
            text::HoriOrientation::LEFT,   // inside
            text::HoriOrientation::RIGHT   // outside
        };

        // generic vertical Adjustment
        static const sal_Int16 aVertOriTab[ nCntYAlign ] =
        {
            text::VertOrientation::NONE,         // From Top position
            text::VertOrientation::TOP,          // top
            text::VertOrientation::CENTER,       // centered
            text::VertOrientation::BOTTOM,       // bottom
            text::VertOrientation::LINE_TOP,     // inside (obscure)
            text::VertOrientation::LINE_BOTTOM   // outside (obscure)
        };

        // #i22673# - to-line vertical alignment
        static const sal_Int16 aToLineVertOriTab[ nCntYAlign ] =
        {
            text::VertOrientation::NONE,         // below
            text::VertOrientation::LINE_BOTTOM,  // top
            text::VertOrientation::LINE_CENTER,  // centered
            text::VertOrientation::LINE_TOP,     // bottom
            text::VertOrientation::LINE_BOTTOM,  // inside (obscure)
            text::VertOrientation::LINE_TOP      // outside (obscure)
        };

        // Adjustment is horizontally relative to...
        static const sal_Int16 aHoriRelOriTab[nCntRelTo] =
        {
            text::RelOrientation::PAGE_PRINT_AREA,    // 0 is page textarea margin
            text::RelOrientation::PAGE_FRAME,  // 1 is page margin
            text::RelOrientation::FRAME,         // 2 is relative to column
            text::RelOrientation::CHAR       // 3 is relative to character
        };

        // Adjustment is vertically relative to...
        // #i22673# - adjustment for new vertical alignment at top of line.
        static const sal_Int16 aVertRelOriTab[nCntRelTo] =
        {
            text::RelOrientation::PAGE_PRINT_AREA, // 0 is page textarea margin
            text::RelOrientation::PAGE_FRAME,   // 1 is page margin
            text::RelOrientation::FRAME,          // 2 is relative to paragraph
            text::RelOrientation::TEXT_LINE   // 3 is relative to line
        };

        sal_Int16 eHoriOri = aHoriOriTab[ nXAlign ];
        sal_Int16 eHoriRel = aHoriRelOriTab[  nXRelTo ];

        // #i36649# - adjustments for certain alignments
        if ( eHoriOri == text::HoriOrientation::LEFT && eHoriRel == text::RelOrientation::PAGE_FRAME )
        {
            // convert 'left to page' to 'from left -<width> to page text area'
            eHoriOri = text::HoriOrientation::NONE;
            eHoriRel = text::RelOrientation::PAGE_PRINT_AREA;
            const long nWidth = pFSPA->nXaRight - pFSPA->nXaLeft;
            pFSPA->nXaLeft = -nWidth;
            pFSPA->nXaRight = 0;
        }
        else if ( eHoriOri == text::HoriOrientation::RIGHT && eHoriRel == text::RelOrientation::PAGE_FRAME )
        {
            // convert 'right to page' to 'from left 0 to right page border'
            eHoriOri = text::HoriOrientation::NONE;
            eHoriRel = text::RelOrientation::PAGE_RIGHT;
            const long nWidth = pFSPA->nXaRight - pFSPA->nXaLeft;
            pFSPA->nXaLeft = 0;
            pFSPA->nXaRight = nWidth;
        }

        // #i24255# - position of floating screen objects in
        // R2L layout are given in L2R layout, thus convert them of all
        // floating screen objects, which are imported.
        {
            // Miserable miserable hack.
            SwTwips nWidth = (pFSPA->nXaRight - pFSPA->nXaLeft);
            SwTwips nLeft = pFSPA->nXaLeft;
            if (MiserableRTLGraphicsHack(nLeft, nWidth, eHoriOri,
                eHoriRel))
            {
                pFSPA->nXaLeft = nLeft;
                pFSPA->nXaRight = pFSPA->nXaLeft + nWidth;
            }
        }

        // if the object is anchored inside a table cell, is horizontal aligned
        // at frame|character and has wrap through, but its attribute
        // 'layout in table cell' isn't set, convert its horizontal alignment to page text area.
        // #i84783# - use new method <IsObjectLayoutInTableCell()>
        if ( m_nInTable &&
             ( eHoriRel == text::RelOrientation::FRAME || eHoriRel == text::RelOrientation::CHAR ) &&
             pFSPA->nwr == 3 &&
             !IsObjectLayoutInTableCell( pRecord->nLayoutInTableCell ) )
        {
            eHoriRel = text::RelOrientation::PAGE_PRINT_AREA;
        }

        // Writer honours this wrap distance when aligned as "left" or "right",
        // Word doesn't. Writer doesn't honour it when its "from left".
        if (eHoriOri == text::HoriOrientation::LEFT)
            pRecord->nDxWrapDistLeft=0;
        else if (eHoriOri == text::HoriOrientation::RIGHT)
            pRecord->nDxWrapDistRight=0;

        sal_Int16 eVertRel;

        eVertRel = aVertRelOriTab[  nYRelTo ]; // #i18732#
        if ( bCurSectionVertical && nYRelTo == 2 )
            eVertRel = text::RelOrientation::PAGE_PRINT_AREA;
        // #i22673# - fill <eVertOri> in dependence of <eVertRel>
        sal_Int16 eVertOri;
        if ( eVertRel == text::RelOrientation::TEXT_LINE )
        {
            eVertOri = aToLineVertOriTab[ nYAlign ];
        }
        else
        {
            eVertOri = aVertOriTab[ nYAlign ];
        }

        // Below line in word is a positive value, while in writer its
        // negative
        long nYPos = pFSPA->nYaTop;
        // #i22673#
        if ((eVertRel == text::RelOrientation::TEXT_LINE) && (eVertOri == text::VertOrientation::NONE))
            nYPos = -nYPos;

        SwFormatHoriOrient aHoriOri(MakeSafePositioningValue(  bCurSectionVertical ? nYPos : pFSPA->nXaLeft ),
                                                            bCurSectionVertical ? eVertOri : eHoriOri,
                                                            bCurSectionVertical ? eVertRel : eHoriRel);
        if( 4 <= nXAlign )
            aHoriOri.SetPosToggle(true);
        rFlySet.Put( aHoriOri );

        rFlySet.Put(SwFormatVertOrient(MakeSafePositioningValue( !bCurSectionVertical ? nYPos : -pFSPA->nXaRight ),
                                                                !bCurSectionVertical ? eVertOri : eHoriOri,
                                                                !bCurSectionVertical ? eVertRel : eHoriRel ));
    }

    return eAnchor;
}

// #i84783#
bool SwWW8ImplReader::IsObjectLayoutInTableCell( const sal_uInt32 nLayoutInTableCell ) const
{
    bool bIsObjectLayoutInTableCell = false;

    if ( m_bVer8 )
    {
        const sal_uInt16 nWWVersion = m_pWwFib->m_nProduct & 0xE000;
        switch ( nWWVersion )
        {
            case 0x0000: // version 8 aka Microsoft Word 97
            {
                bIsObjectLayoutInTableCell = false;
                OSL_ENSURE( nLayoutInTableCell == 0xFFFFFFFF,
                        "no explicit object attribute layout in table cell expected." );
            }
            break;
            case 0x2000: // version 9 aka Microsoft Word 2000
            case 0x4000: // version 10 aka Microsoft Word 2002
            case 0x6000: // version 11 aka Microsoft Word 2003
            case 0x8000: // version 12 aka Microsoft Word 2007
            case 0xC000: // version 14 aka Microsoft Word 2010
            {
                // #i98037#
                // adjustment of conditions needed after deeper analysis of
                // certain test cases.
                if ( nLayoutInTableCell == 0xFFFFFFFF || // no explicit attribute value given
                     nLayoutInTableCell == 0x80008000 ||
                     ( nLayoutInTableCell & 0x02000000 &&
                       !(nLayoutInTableCell & 0x80000000 ) ) )
                {
                    bIsObjectLayoutInTableCell = true;
                }
                else
                {
                    bIsObjectLayoutInTableCell = false;
                }
            }
            break;
            default:
            {
                OSL_FAIL( "unknown version." );
            }
        }
    }

    return bIsObjectLayoutInTableCell;
}

SwFrameFormat* SwWW8ImplReader::Read_GrafLayer( long nGrafAnchorCp )
{
    if( m_nIniFlags & WW8FL_NO_GRAFLAYER )
        return nullptr;

    ::SetProgressState(m_nProgress, m_pDocShell);     // Update

    m_nDrawCpO = 0;
    m_bDrawCpOValid = m_pWwFib->GetBaseCp(m_pPlcxMan->GetManType() == MAN_HDFT ? MAN_TXBX_HDFT : MAN_TXBX, &m_nDrawCpO);

    GrafikCtor();

    WW8PLCFspecial* pPF = m_pPlcxMan->GetFdoa();
    if( !pPF )
    {
        OSL_ENSURE( false, "Where is the graphic (1) ?" );
        return nullptr;
    }

    if( m_bVer67 )
    {
        long nOldPos = m_pStrm->Tell();

        m_nDrawXOfs = m_nDrawYOfs = 0;
        ReadGrafLayer1( pPF, nGrafAnchorCp );

        m_pStrm->Seek( nOldPos );
        return nullptr;
    }

    // Normal case of Word 8+ version stuff
    pPF->SeekPos( nGrafAnchorCp );

    WW8_FC nStartFc;
    void* pF0;
    if( !pPF->Get( nStartFc, pF0 ) ){
        OSL_ENSURE( false, "+Wo ist die Grafik (2) ?" );
        return nullptr;
    }

    WW8_FSPA_SHADOW* pFS = static_cast<WW8_FSPA_SHADOW*>(pF0);
    WW8_FSPA*        pF;
    WW8_FSPA aFSFA;
    pF = &aFSFA;
    WW8FSPAShadowToReal( pFS, pF );
    if( !pF->nSpId )
    {
        OSL_ENSURE( false, "+Wo ist die Grafik (3) ?" );
        return nullptr;
    }

    if (!m_pMSDffManager->GetModel())
         m_pMSDffManager->SetModel(m_pDrawModel, 1440);

    Rectangle aRect(pF->nXaLeft,  pF->nYaTop, pF->nXaRight, pF->nYaBottom);
    SvxMSDffImportData aData( aRect );

    /*
    #i20540#
    The SdrOle2Obj will try and manage any ole objects it finds, causing all
    sorts of trouble later on
    */
    SwDocShell* pPersist = m_rDoc.GetDocShell();
    m_rDoc.SetDocShell(nullptr);         // #i20540# Persist guard

    SdrObject* pObject = nullptr;
    bool bOk = (m_pMSDffManager->GetShape(pF->nSpId, pObject, aData) && pObject);

    m_rDoc.SetDocShell(pPersist);  // #i20540# Persist guard

    if (!bOk)
    {
        OSL_ENSURE( false, "Where is the Shape ?" );
        return nullptr;
    }

    bool bDone = false;
    SdrObject* pOurNewObject = nullptr;
    bool bReplaceable = false;

    switch (SdrObjKind(pObject->GetObjIdentifier()))
    {
        case OBJ_GRAF:
            bReplaceable = true;
            bDone = true;
            break;
        case OBJ_OLE2:
            bReplaceable = true;
            break;
        default:
            break;

    }

    // when in a header or footer word appears to treat all elements as wrap through

    // Umfluss-Modus ermitteln
    SfxItemSet aFlySet(m_rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SwSurround eSurround = SURROUND_PARALLEL;
    bool bContour = false;
    switch (pF->nwr)
    {
        case 0: // 0 like 2, but doesn't require absolute object
        case 2: // 2 wrap around absolute object
            eSurround = SURROUND_PARALLEL;
            break;
        case 1: // 1 no text next to shape
            eSurround = SURROUND_NONE;
            break;
        case 3: // 3 wrap as if no object present
            eSurround = SURROUND_THROUGHT;
            break;
        case 4: // 4 wrap tightly around object
        case 5: // 5 wrap tightly, but allow holes
            eSurround = SURROUND_PARALLEL;
            bContour = true;
            break;
    }

    // bei Modus 2 oder 4 auch den Zusatzparameter beruecksichtigen
    if ( (2 == pF->nwr) || (4 == pF->nwr) )
    {
        switch( pF->nwrk )
        {
            // 0 wrap both sides
            case 0:
                eSurround = SURROUND_PARALLEL;
                break;
            // 1 wrap only on left
            case 1:
                eSurround = SURROUND_LEFT;
                break;
            // 2 wrap only on right
            case 2:
                eSurround = SURROUND_RIGHT;
                break;
            // 3 wrap only on largest side
            case 3:
                eSurround = SURROUND_IDEAL;
                break;
        }
    }

    SwFormatSurround aSur( eSurround );
    aSur.SetContour( bContour );
    aSur.SetOutside(true); // Winword kann nur Aussen-Konturen
    aFlySet.Put( aSur );

    // eingelesenes Objekt (kann eine ganze Gruppe sein) jetzt korrekt
    // positionieren usw.

    OSL_ENSURE(!((aData.size() != 1) && bReplaceable),
        "Replaceable drawing with > 1 entries ?");

    if (aData.size() != 1)
        bReplaceable = false;

    SvxMSDffImportRec* pRecord = nullptr;
    /*
        Get the record for top level object, so we can get the word anchoring
        and wrapping information for it.
    */
    for (MSDffImportRecords::const_iterator it = aData.begin();
            it != aData.end(); ++it) // MSVC2008 wants const_iterator here???
    {
        if ((*it)->pObj == pObject)
        {
            pRecord = it->get();
            break;
        }
    }

    OSL_ENSURE(pRecord, "how did that happen?");
    if (!pRecord)
        return nullptr;

    const bool bLayoutInTableCell =
        m_nInTable && IsObjectLayoutInTableCell( pRecord->nLayoutInTableCell );

    // #i18732# - Switch on 'follow text flow', if object is laid out
    // inside table cell and its wrapping isn't 'SURROUND_THROUGH'
    if (bLayoutInTableCell && eSurround != SURROUND_THROUGHT)
    {
        SwFormatFollowTextFlow aFollowTextFlow( true );
        aFlySet.Put( aFollowTextFlow );
    }

    // #i21847#
    // Some shapes are set to *hidden*, don't import those ones.
    if (pRecord->bHidden)
        return nullptr;

    sal_uInt16 nCount = pObject->GetUserDataCount();
    if(nCount)
    {
        OUString lnName, aObjName, aTarFrame;
        for (sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObject->GetUserData( i );
            if( pData && pData->GetInventor() == SdrInventor::ScOrSwDraw
                    && pData->GetId() == SW_UD_IMAPDATA)
            {
                SwMacroInfo* macInf = dynamic_cast<SwMacroInfo*>(pData);
                if( macInf && macInf->GetShapeId() == pF->nSpId )
                {
                    lnName = macInf->GetHlink();
                    aObjName = macInf->GetName();
                    aTarFrame = macInf->GetTarFrame();
                    break;
                }
            }
        }
        SwFormatURL* pFormatURL = new SwFormatURL();
        pFormatURL->SetURL( lnName, false );
        if (!aObjName.isEmpty())
            pFormatURL->SetName(aObjName);
        if (!aTarFrame.isEmpty())
            pFormatURL->SetTargetFrameName(aTarFrame);
        pFormatURL->SetMap(nullptr);
        aFlySet.Put(*pFormatURL);
    }

    // If we are to be "below text" then we are not to be opaque
    // #i14045# MM If we are in a header or footer then make the object transparent
    // Not exactly like word but close enough for now

    // both flags <bBelowText> and <bDrawHell> have to be set to move object into the background.
    // #i46794# - it reveals that value of flag <bBelowText> can be neglected.
    const bool bMoveToBackgrd = pRecord->bDrawHell ||
                                ( ( m_bIsHeader || m_bIsFooter ) && pF->nwr == 3 );
    if ( bMoveToBackgrd )
        aFlySet.Put(SvxOpaqueItem(RES_OPAQUE,false));

    OUString aObjName = pObject->GetName();

    SwFrameFormat* pRetFrameFormat = nullptr;
    if (bReplaceable)
    {
        // Single graphics or ole objects
        pRetFrameFormat = ImportReplaceableDrawables(pObject, pOurNewObject, pRecord,
            pF, aFlySet);
    }
    else
    {
        // Drawing objects, (e.g. ovals or drawing groups)
        if (pF->bRcaSimple)
        {
            pF->nbx = WW8_FSPA::RelPageBorder;
            pF->nby = WW8_FSPA::RelPageBorder;
        }

        RndStdIds eAnchor = ProcessEscherAlign(pRecord, pF, aFlySet,
            bReplaceable);

        // Should we, and is it possible to make this into a writer textbox
        if ((!(m_nIniFlags1 & WW8FL_NO_FLY_FOR_TXBX)) && pRecord->bReplaceByFly)
        {
            pRetFrameFormat = ConvertDrawTextToFly(pObject, pOurNewObject, pRecord,
                eAnchor, pF, aFlySet);
            if (pRetFrameFormat)
                bDone = true;
        }

        if (!bDone)
        {
            sw::util::SetLayer aSetLayer(m_rDoc);
            if ( bMoveToBackgrd )
                aSetLayer.SendObjectToHell(*pObject);
            else
                aSetLayer.SendObjectToHeaven(*pObject);

            if (!IsInlineEscherHack())
            {
                /* Need to make sure that the correct layer ordering is applied. */
                //  pass information, if object is in page header|footer to method.
                m_pWWZOrder->InsertEscherObject( pObject, pF->nSpId,
                                               m_bIsHeader || m_bIsFooter );
            }
            else
            {
                m_pWWZOrder->InsertTextLayerObject(pObject);
            }

            pRetFrameFormat = m_rDoc.getIDocumentContentOperations().InsertDrawObj(*m_pPaM, *pObject, aFlySet );

            OSL_ENSURE(pRetFrameFormat->GetAnchor().GetAnchorId() ==
                eAnchor, "Not the anchor type requested!");

            /*
                Insert text if necessary into textboxes contained in groups.
            */
            if (!aData.empty())
            {
                for (MSDffImportRecords::const_iterator it = aData.begin();
                        it != aData.end(); ++it)
                {
                    pRecord = it->get();
                    if (pRecord->pObj && pRecord->aTextId.nTxBxS)
                    { // #i52825# pRetFrameFormat can be NULL
                        pRetFrameFormat = MungeTextIntoDrawBox(pRecord->pObj,
                            pRecord, nGrafAnchorCp, pRetFrameFormat);
                    }
                }
            }
        }
    }

    // #i44344#, #i44681# - positioning attributes already set
    if ( pRetFrameFormat /*#i52825# */ && dynamic_cast< const SwDrawFrameFormat *>( pRetFrameFormat ) !=  nullptr )
    {
        static_cast<SwDrawFrameFormat*>(pRetFrameFormat)->PosAttrSet();
    }
    if (!IsInlineEscherHack())
        MapWrapIntoFlyFormat(pRecord, pRetFrameFormat);

    // Set frame name with object name
    if( pRetFrameFormat /*#i52825# */ && !aObjName.isEmpty() )
        pRetFrameFormat->SetName( aObjName );
    return AddAutoAnchor(pRetFrameFormat);
}

SwFrameFormat *SwWW8ImplReader::AddAutoAnchor(SwFrameFormat *pFormat)
{
    /*
     * anchored to character at the current position will move along the
     * paragraph as text is added because we are at the insertion point.
     *
     * Leave to later and set the correct location then.
     */
    if ((pFormat) && (pFormat->GetAnchor().GetAnchorId() != FLY_AS_CHAR))
    {
        m_pAnchorStck->AddAnchor(*m_pPaM->GetPoint(), pFormat);
    }
    return pFormat;
}

SwFrameFormat* SwWW8ImplReader::MungeTextIntoDrawBox(SdrObject* pTrueObject,
    SvxMSDffImportRec *pRecord, long nGrafAnchorCp, SwFrameFormat* pRetFrameFormat)
{
    SdrTextObj* pSdrTextObj;

    // Pruefen, ob Gruppenobjekt (z.B. zwei Klammern) vorliegt
    if (SdrObjGroup* pThisGroup = dynamic_cast<SdrObjGroup*>( pRecord->pObj) )
    {
        // Gruppenobjekte haben keinen Text. Fuege ein Textobjekt in die
        // Gruppe ein, um den Text zu halten.
        pSdrTextObj = new SdrRectObj( OBJ_TEXT, pThisGroup->GetCurrentBoundRect());

        SfxItemSet aSet(m_pDrawModel->GetItemPool());
        aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
        aSet.Put(SdrTextFitToSizeTypeItem( SdrFitToSizeType::NONE ));
        aSet.Put(makeSdrTextAutoGrowHeightItem(false));
        aSet.Put(makeSdrTextAutoGrowWidthItem(false));
        pSdrTextObj->SetMergedItemSet(aSet);
        pSdrTextObj->NbcSetLayer( pThisGroup->GetLayer() );
        pThisGroup->GetSubList()->NbcInsertObject(pSdrTextObj);
    }
    else
        pSdrTextObj = dynamic_cast<SdrTextObj*>( pRecord->pObj );

    if( pSdrTextObj )
    {
        Size aObjSize(pSdrTextObj->GetSnapRect().GetWidth(),
            pSdrTextObj->GetSnapRect().GetHeight());

        // Objekt ist Bestandteil einer Gruppe?
        SdrObject* pGroupObject = pSdrTextObj->GetUpGroup();

        const size_t nOrdNum = pSdrTextObj->GetOrdNum();
        bool bEraseThisObject;
        InsertTxbxText( pSdrTextObj, &aObjSize, pRecord->aTextId.nTxBxS,
            pRecord->aTextId.nSequence, nGrafAnchorCp, pRetFrameFormat,
            (pSdrTextObj != pTrueObject) || (nullptr != pGroupObject),
            bEraseThisObject, nullptr, nullptr, nullptr, nullptr, pRecord);

        // wurde dieses Objekt ersetzt ??
        if (bEraseThisObject)
        {
            if( pGroupObject || (pSdrTextObj != pTrueObject) )
            {
                // Objekt wurde bereits (in der Gruppe und) der Drawing-Page
                // durch ein neues SdrGrafObj ersetzt.

                SdrObject* pNewObj = pGroupObject ?
                    pGroupObject->GetSubList()->GetObj(nOrdNum) : pTrueObject;
                if (pSdrTextObj != pNewObj)
                {
                    // Objekt in der Z-Order-Liste ersetzen
                    m_pMSDffManager->ExchangeInShapeOrder(pSdrTextObj, 0, pNewObj);
                    // Objekt jetzt noch loeschen
                    SdrObject::Free( pRecord->pObj );
                    // und das neue Objekt merken.
                    pRecord->pObj = pNewObj;
                }
            }
            else
            {
                // Objekt aus der Z-Order-Liste loeschen
                m_pMSDffManager->RemoveFromShapeOrder( pSdrTextObj );
                // Objekt aus der Drawing-Page rausnehmen
                if( pSdrTextObj->GetPage() )
                    m_pDrawPg->RemoveObject( pSdrTextObj->GetOrdNum() );
                // und FrameFormat entfernen, da durch Grafik ersetzt (dies
                // loescht auch das Objekt)
                m_rDoc.DelFrameFormat( pRetFrameFormat );
                pRetFrameFormat = nullptr;
                // auch den Objektmerker loeschen
                pRecord->pObj = nullptr;
            }
        }
        else
        {
            // ww8-default Randabstand einsetzen
            SfxItemSet aItemSet(m_pDrawModel->GetItemPool(),
                SDRATTR_TEXT_LEFTDIST, SDRATTR_TEXT_LOWERDIST);
            aItemSet.Put( makeSdrTextLeftDistItem( pRecord->nDxTextLeft ) );
            aItemSet.Put( makeSdrTextRightDistItem( pRecord->nDxTextRight  ) );
            aItemSet.Put( makeSdrTextUpperDistItem( pRecord->nDyTextTop    ) );
            aItemSet.Put( makeSdrTextLowerDistItem( pRecord->nDyTextBottom ) );
            pSdrTextObj->SetMergedItemSetAndBroadcast(aItemSet);
        }
    }
    return pRetFrameFormat;
}

SwFlyFrameFormat* SwWW8ImplReader::ConvertDrawTextToFly(SdrObject* &rpObject,
    SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, RndStdIds eAnchor,
    WW8_FSPA *pF, SfxItemSet &rFlySet)
{
    SwFlyFrameFormat* pRetFrameFormat = nullptr;
    long nStartCp;
    long nEndCp;

    // Check if this textbox chain contains text as conversion of an empty
    // chain would not make sense.
    if ( TxbxChainContainsRealText(pRecord->aTextId.nTxBxS,nStartCp,nEndCp) )
    {
        // The Text is not read into SdrTextObj!  Rather insert a frame and
        // insert the text from nStartCp to nEndCp.

        // More attributes can be used in a frame compared to the
        // Edit-Engine, and it can contain field, OLEs or graphics...
        Rectangle aInnerDist(pRecord->nDxTextLeft, pRecord->nDyTextTop,
            pRecord->nDxTextRight, pRecord->nDyTextBottom);

        SwFormatFrameSize aFrameSize(ATT_FIX_SIZE, pF->nXaRight - pF->nXaLeft, pF->nYaBottom - pF->nYaTop);
        aFrameSize.SetWidthSizeType(pRecord->bAutoWidth ? ATT_VAR_SIZE : ATT_FIX_SIZE);
        rFlySet.Put(aFrameSize);

        MatchSdrItemsIntoFlySet( rpObject, rFlySet, pRecord->eLineStyle,
            pRecord->eLineDashing, pRecord->eShapeType, aInnerDist );

        SdrTextObj *pSdrTextObj = dynamic_cast<SdrTextObj*>(rpObject);
        if (pSdrTextObj && pSdrTextObj->IsVerticalWriting())
            rFlySet.Put(SvxFrameDirectionItem(FRMDIR_VERT_TOP_RIGHT, RES_FRAMEDIR));

        pRetFrameFormat = m_rDoc.MakeFlySection(eAnchor, m_pPaM->GetPoint(), &rFlySet);
        OSL_ENSURE(pRetFrameFormat->GetAnchor().GetAnchorId() == eAnchor,
            "Not the anchor type requested!");

        // if everything is OK, find pointer on new object and correct
        // Z-order list (oder delete entry)
        rpOurNewObject = CreateContactObject(pRetFrameFormat);

        // remove old object from the Z-Order list
        m_pMSDffManager->RemoveFromShapeOrder( rpObject );

        // and delete the object
        SdrObject::Free( rpObject );
        /*
            NB: only query pOrgShapeObject starting here!
        */

        if (rpOurNewObject)
        {
            /*
            We do not store our rpOutNewObject in the ShapeOrder because we
            have a FrameFormat from which we can regenerate the contact object when
            we need it. Because, we can have frames anchored to paragraphs in
            header/footers and we can copy header/footers, if we do copy a
            header/footer with a nonpage anchored frame in it then the contact
            objects are invalidated. Under this condition the FrameFormat will be
            updated to reflect this change and can be used to get a new
            contact object, while a raw rpOutNewObject stored here becomes
            deleted and useless.
            */
            m_pMSDffManager->StoreShapeOrder(pF->nSpId,
                (((sal_uLong)pRecord->aTextId.nTxBxS) << 16) +
                pRecord->aTextId.nSequence, nullptr, pRetFrameFormat);

            // The Contact object has to be inserted into the draw page, so
            // SwWW8ImplReader::LoadDoc1() can determine the z-order.
            if (!rpOurNewObject->IsInserted())
            {
                // pass information, if object is in page header|footer to method.
                m_pWWZOrder->InsertEscherObject( rpOurNewObject, pF->nSpId,
                                               m_bIsHeader || m_bIsFooter );
            }
        }

        // Box-0 receives the text for the whole chain!
        if( !pRecord->aTextId.nSequence )
        {
            // save flags etc and reset them
            WW8ReaderSave aSave( this );

            MoveInsideFly(pRetFrameFormat);

            SwNodeIndex aStart(m_pPaM->GetPoint()->nNode);

            m_pWWZOrder->InsideEscher(pF->nSpId);

            // read in the text
            m_bTxbxFlySection = true;
            bool bJoined = ReadText(nStartCp, (nEndCp-nStartCp),
                MAN_MAINTEXT == m_pPlcxMan->GetManType() ?
                        MAN_TXBX : MAN_TXBX_HDFT);

            m_pWWZOrder->OutsideEscher();

            MoveOutsideFly(pRetFrameFormat, aSave.GetStartPos(),!bJoined);

            aSave.Restore( this );

            StripNegativeAfterIndent(pRetFrameFormat);
        }

    }
    return pRetFrameFormat;
}

void MatchEscherMirrorIntoFlySet(const SvxMSDffImportRec &rRecord,
    SfxItemSet &rFlySet)
{
    if (rRecord.bVFlip || rRecord.bHFlip)
    {
        MirrorGraph eType(RES_MIRROR_GRAPH_DONT);
        if (rRecord.bVFlip && rRecord.bHFlip)
            eType = RES_MIRROR_GRAPH_BOTH;
        else if (rRecord.bVFlip)
            eType = RES_MIRROR_GRAPH_HOR;
        else
            eType = RES_MIRROR_GRAPH_VERT;
        rFlySet.Put( SwMirrorGrf(eType) );
    }
}

SwFlyFrameFormat* SwWW8ImplReader::ImportReplaceableDrawables( SdrObject* &rpObject,
    SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, WW8_FSPA *pF,
    SfxItemSet &rFlySet )
{
    SwFlyFrameFormat* pRetFrameFormat = nullptr;
    long nWidthTw  = pF->nXaRight - pF->nXaLeft;
    if (0 > nWidthTw)
        nWidthTw = 0;
    long nHeightTw = pF->nYaBottom - pF->nYaTop;
    if (0 > nHeightTw)
        nHeightTw = 0;

    ProcessEscherAlign(pRecord, pF, rFlySet, true);

    rFlySet.Put(SwFormatFrameSize(ATT_FIX_SIZE, nWidthTw, nHeightTw));

    SfxItemSet aGrSet(m_rDoc.GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);

    if (pRecord)
    {
        // Note that the escher inner distance only seems to be honoured in
        // word for textboxes, not for graphics and ole objects.
        Rectangle aInnerDist(0, 0, 0, 0);

        MatchSdrItemsIntoFlySet(rpObject, rFlySet, pRecord->eLineStyle,
            pRecord->eLineDashing, pRecord->eShapeType, aInnerDist);

        MatchEscherMirrorIntoFlySet(*pRecord, aGrSet);
    }

    OUString aObjectName(rpObject->GetName());
    if (OBJ_OLE2 == SdrObjKind(rpObject->GetObjIdentifier()))
        pRetFrameFormat = InsertOle(*static_cast<SdrOle2Obj*>(rpObject), rFlySet, &aGrSet);
    else
    {
        const SdrGrafObj *pGrf = static_cast<const SdrGrafObj*>(rpObject);
        bool bDone = false;
        if (pGrf->IsLinkedGraphic() && !pGrf->GetFileName().isEmpty())
        {
            GraphicType eType = pGrf->GetGraphicType();
            OUString aGrfName(
                URIHelper::SmartRel2Abs(
                    INetURLObject(m_sBaseURL), pGrf->GetFileName(),
                    URIHelper::GetMaybeFileHdl()));
            // correction of fix for issue #i10939#:
            // One of the two conditions have to be true to insert the graphic
            // as a linked graphic -
            if (GraphicType::NONE == eType || CanUseRemoteLink(aGrfName))
            {
                pRetFrameFormat = m_rDoc.getIDocumentContentOperations().Insert(*m_pPaM, aGrfName, OUString(), nullptr,
                    &rFlySet, &aGrSet, nullptr);
                bDone = true;
            }
        }
        if (!bDone)
        {
            const Graphic& rGraph = pGrf->GetGraphic();
            pRetFrameFormat = m_rDoc.getIDocumentContentOperations().Insert(*m_pPaM, OUString(), OUString(), &rGraph,
                &rFlySet, &aGrSet, nullptr);
        }
    }

    if (pRetFrameFormat)
    {
        if( pRecord )
        {
            if( OBJ_OLE2 != SdrObjKind(rpObject->GetObjIdentifier()) )
                SetAttributesAtGrfNode( pRecord, pRetFrameFormat, pF );
        }
        // mehrfaches Auftreten gleicher Grafik-Namen vermeiden
        m_aGrfNameGenerator.SetUniqueGraphName(pRetFrameFormat, aObjectName);
    }
    // falls alles Ok, Zeiger auf neues Objekt ermitteln und Z-Order-Liste
    // entsprechend korrigieren (oder Eintrag loeschen)
    rpOurNewObject = CreateContactObject(pRetFrameFormat);

    // altes Objekt aus der Z-Order-Liste entfernen
    m_pMSDffManager->RemoveFromShapeOrder( rpObject );
    // aus der Drawing-Page rausnehmen
    if( rpObject->GetPage() )
        m_pDrawPg->RemoveObject( rpObject->GetOrdNum() );

    // und das Objekt loeschen
    SdrObject::Free( rpObject );
    /*
        Achtung: ab jetzt nur noch pOrgShapeObject abfragen!
    */

    // Kontakt-Objekt in die Z-Order-Liste und die Page aufnehmen
    if (rpOurNewObject)
    {
        if (!m_bHdFtFootnoteEdn)
            m_pMSDffManager->StoreShapeOrder(pF->nSpId, 0, rpOurNewObject );

        // Das Kontakt-Objekt MUSS in die Draw-Page gesetzt werden, damit in
        // SwWW8ImplReader::LoadDoc1() die Z-Order festgelegt werden kann !!!
        if (!rpOurNewObject->IsInserted())
        {
            // pass information, if object is in page header|footer to method.
            m_pWWZOrder->InsertEscherObject( rpOurNewObject, pF->nSpId,
                                           m_bIsHeader || m_bIsFooter );
        }
    }
    return pRetFrameFormat;
}

void SwWW8ImplReader::GrafikCtor()  // Fuer SVDraw und VCControls und Escher
{
    if (!m_pDrawModel)
    {
        m_rDoc.getIDocumentDrawModelAccess().GetOrCreateDrawModel(); // #i52858# - method name changed
        m_pDrawModel  = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
        OSL_ENSURE(m_pDrawModel, "Kann DrawModel nicht anlegen");
        m_pDrawPg = m_pDrawModel->GetPage(0);

        m_pMSDffManager = new SwMSDffManager(*this, m_bSkipImages);
        m_pMSDffManager->SetModel(m_pDrawModel, 1440);
        /*
         Now the dff manager always needs a controls converter as well, but a
         control converter may still exist without a dffmanager.
        */
        m_pFormImpl = new SwMSConvertControls(m_pDocShell, m_pPaM);

        m_pWWZOrder = new wwZOrderer(sw::util::SetLayer(m_rDoc), m_pDrawPg,
            m_pMSDffManager->GetShapeOrders());
    }
}

void SwWW8ImplReader::GrafikDtor()
{
    DELETEZ(m_pDrawEditEngine); // evtl. von Grafik angelegt
    DELETEZ(m_pWWZOrder);       // dito
}

void SwWW8FltAnchorStack::AddAnchor(const SwPosition& rPos, SwFrameFormat *pFormat)
{
    OSL_ENSURE(pFormat->GetAnchor().GetAnchorId() != FLY_AS_CHAR,
        "Don't use fltanchors with inline frames, slap!");
    NewAttr(rPos, SwFltAnchor(pFormat));
}

void SwWW8FltAnchorStack::Flush()
{
    size_t nCnt = size();
    for (size_t i=0; i < nCnt; ++i)
    {
        SwFltStackEntry &rEntry = (*this)[i];
        SwPosition aDummy(rEntry.m_aMkPos.m_nNode);
        SetAttrInDoc(aDummy, rEntry);
        DeleteAndDestroy(i--);
        --nCnt;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
