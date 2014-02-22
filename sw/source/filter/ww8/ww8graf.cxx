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
#include <boost/scoped_ptr.hpp>
#include <math.h>
#include <fmturl.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/whiter.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace sw::types;
using namespace sw::util;


Color WW8TransCol(SVBT32 nWC)
{
#if 1               

    
    
    
    
    static const ColorData eColA[] = {              
        COL_BLACK, COL_RED, COL_LIGHTRED,           
        COL_GREEN, COL_BROWN, COL_BLACK,            
        COL_LIGHTGREEN, COL_BLACK, COL_YELLOW,      
        COL_BLUE, COL_MAGENTA, COL_BLACK,           
        COL_CYAN, COL_LIGHTGRAY, COL_BLACK,         
        COL_BLACK, COL_BLACK, COL_BLACK,            
        COL_LIGHTBLUE, COL_BLACK, COL_LIGHTMAGENTA, 
        COL_BLACK, COL_BLACK, COL_BLACK,            
        COL_LIGHTCYAN, COL_BLACK, COL_WHITE };      

    
    
    
    
    
    

    if( !( nWC[3] & 0x1 ) &&                        
        (    ( nWC[0] == 0 ||  nWC[0]== 0x80 || nWC[0] == 0xff )    
          && ( nWC[1] == 0 ||  nWC[1]== 0x80 || nWC[1] == 0xff )    
          && ( nWC[2] == 0 ||  nWC[2]== 0x80 || nWC[2] == 0xff ) ) ){
        int nIdx = 0;       
        for (int i = 2; i >= 0; i--)
        {
            nIdx *= 3;
            if (nWC[i])
                nIdx += ((nWC[i] == 0xff) ? 2 : 1);
        }
        if (eColA[nIdx] != COL_BLACK)
            return Color(eColA[nIdx]);  
    }
#endif

    if (nWC[3] & 0x1)
    {
        
        sal_uInt8 u = (sal_uInt8)( (sal_uLong)( 200 - nWC[0] ) * 256 / 200 );
        return Color(u, u, u);
    }

    
    return Color(nWC[0], nWC[1], nWC[2]);
}

void wwFrameNamer::SetUniqueGraphName(SwFrmFmt *pFrmFmt, const OUString &rFixed)
{
    if (mbIsDisabled || rFixed.isEmpty())
        return;

    pFrmFmt->SetName(msSeed+OUString::number(++mnImportedGraphicsCount) + ": " + rFixed);
}


bool SwWW8ImplReader::ReadGrafStart(void* pData, short nDataSiz,
    WW8_DPHEAD* pHd, const WW8_DO* pDo, SfxAllItemSet &rSet)
{
    if (SVBT16ToShort(pHd->cb) < sizeof(WW8_DPHEAD) + nDataSiz)
    {
        OSL_ENSURE( !this, "+Grafik-Element: Size ?" );
        pStrm->SeekRel(SVBT16ToShort(pHd->cb) - sizeof(WW8_DPHEAD));
        return false;
    }

    bool bCouldRead = checkRead(*pStrm, pData, nDataSiz);
    OSL_ENSURE(bCouldRead, "Short Graphic header");
    if (!bCouldRead)
        return false;

    RndStdIds eAnchor = (pDo->by < 2) ? FLY_AT_PAGE : FLY_AT_PARA;
    rSet.Put(SwFmtAnchor(eAnchor));

    nDrawXOfs2 = nDrawXOfs;
    nDrawYOfs2 = nDrawYOfs;

    if (eAnchor == FLY_AT_PARA)
    {
        if( pDo->bx == 1 )       
            nDrawXOfs2 = static_cast< short >(nDrawXOfs2 - maSectionManager.GetPageLeft());
        if( nInTable )                          
            nDrawXOfs2 = nDrawXOfs2 - GetTableLeft();       
                                                
    }
    else
    {
        if( pDo->bx != 1 )
            nDrawXOfs2 = static_cast< short >(nDrawXOfs2 + maSectionManager.GetPageLeft());
    }

    return true;
}


static void SetStdAttr( SfxItemSet& rSet, WW8_DP_LINETYPE& rL,
                        WW8_DP_SHADOW& rSh )
{
    if( SVBT16ToShort( rL.lnps ) == 5 ){            
        rSet.Put( XLineStyleItem( XLINE_NONE ) );
    }else{                                          
        Color aCol( WW8TransCol( rL.lnpc ) );           
        rSet.Put( XLineColorItem( OUString(), aCol ) );
        rSet.Put( XLineWidthItem( SVBT16ToShort( rL.lnpw ) ) );
                                                    
        if( SVBT16ToShort( rL.lnps ) >= 1
            && SVBT16ToShort(rL.lnps ) <= 4 ){      
            rSet.Put( XLineStyleItem( XLINE_DASH ) );
            sal_Int16 nLen = SVBT16ToShort( rL.lnpw );
            XDash aD( XDASH_RECT, 1, 2 * nLen, 1, 5 * nLen, 5 * nLen );
            switch( SVBT16ToShort( rL.lnps ) ){
            case 1: aD.SetDots( 0 );            
                    aD.SetDashLen( 6 * nLen );
                    aD.SetDistance( 4 * nLen );
                    break;
            case 2: aD.SetDashes( 0 ); break;   
            case 3: break;                      
            case 4: aD.SetDots( 2 ); break;     
            }
            rSet.Put( XLineDashItem( OUString(), aD ) );
        }else{
            rSet.Put( XLineStyleItem( XLINE_SOLID ) );  
        }
    }
    if( SVBT16ToShort( rSh.shdwpi ) ){                  
        rSet.Put(SdrShadowItem(true));
        rSet.Put( SdrShadowXDistItem( SVBT16ToShort( rSh.xaOffset ) ) );
        rSet.Put( SdrShadowYDistItem( SVBT16ToShort( rSh.yaOffset ) ) );
    }
}








static void SetFill( SfxItemSet& rSet, WW8_DP_FILL& rFill )
{
    static const sal_uInt8 nPatA[] =
    {
             0,  0,  5, 10, 20, 25, 30, 40, 50, 60, 70, 75, 80,
            90, 50, 50, 50, 50, 50, 50, 33, 33, 33, 33, 33, 33
    };
    sal_uInt16 nPat = SVBT16ToShort(rFill.flpp);

    if (nPat == 0) 
        rSet.Put(XFillStyleItem(XFILL_NONE));
    else
    {
        rSet.Put(XFillStyleItem(XFILL_SOLID));  
        if (nPat <= 1 || ((sizeof(nPatA)/sizeof(nPatA[0])) <= nPat))
        {
            
            rSet.Put(XFillColorItem(OUString(), WW8TransCol(rFill.dlpcBg)));
        }
        else
        {                                      
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


SdrObject* SwWW8ImplReader::ReadLine( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    WW8_DP_LINE aLine;

    if( !ReadGrafStart( (void*)&aLine, sizeof( aLine ), pHd, pDo, rSet ) )
        return 0;

    Point aP[2];
    {
        Point& rP0 = aP[0];
        Point& rP1 = aP[1];

        rP0.X() = (sal_Int16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2;
        rP0.Y() = (sal_Int16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2;
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

SdrObject* SwWW8ImplReader::ReadRect( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    WW8_DP_RECT aRect;

    if( !ReadGrafStart( (void*)&aRect, sizeof( aRect ), pHd, pDo, rSet ) )
        return 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrRectObj( Rectangle( aP0, aP1 ) );

    SetStdAttr( rSet, aRect.aLnt, aRect.aShd );
    SetFill( rSet, aRect.aFill );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadElipse( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    WW8_DP_ELIPSE aElipse;

    if( !ReadGrafStart( (void*)&aElipse, sizeof( aElipse ), pHd, pDo, rSet ) )
        return 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrCircObj( OBJ_CIRC, Rectangle( aP0, aP1 ) );

    SetStdAttr( rSet, aElipse.aLnt, aElipse.aShd );
    SetFill( rSet, aElipse.aFill );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadArc( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    WW8_DP_ARC aArc;

    if( !ReadGrafStart( (void*)&aArc, sizeof( aArc ), pHd, pDo, rSet ) )
        return 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
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

SdrObject* SwWW8ImplReader::ReadPolyLine( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    WW8_DP_POLYLINE aPoly;

    if( !ReadGrafStart( (void*)&aPoly, sizeof( aPoly ), pHd, pDo, rSet ) )
        return 0;

    sal_uInt16 nCount = SVBT16ToShort( aPoly.aBits1 ) >> 1 & 0x7fff;
    boost::scoped_array<SVBT16> xP(new SVBT16[nCount * 2]);

    bool bCouldRead = checkRead(*pStrm, xP.get(), nCount * 4);      
    OSL_ENSURE(bCouldRead, "Short PolyLine header");
    if (!bCouldRead)
        return 0;

    Polygon aP( nCount );
    Point aPt;
    for (sal_uInt16 i=0; i<nCount; ++i)
    {
        aPt.X() = SVBT16ToShort( xP[i << 1] ) + nDrawXOfs2
                  + (sal_Int16)SVBT16ToShort( pHd->xa );
        aPt.Y() = SVBT16ToShort( xP[( i << 1 ) + 1] ) + nDrawYOfs2
                  + (sal_Int16)SVBT16ToShort( pHd->ya );
        aP[i] = aPt;
    }
    xP.reset();

    SdrObject* pObj = new SdrPathObj(( SVBT16ToShort( aPoly.aBits1 ) & 0x1 ) ? OBJ_POLY : OBJ_PLIN, ::basegfx::B2DPolyPolygon(aP.getB2DPolygon()));
    SetStdAttr( rSet, aPoly.aLnt, aPoly.aShd );
    SetFill( rSet, aPoly.aFill );

    return pObj;
}

ESelection SwWW8ImplReader::GetESelection( long nCpStart, long nCpEnd )
{
    sal_Int32 nPCnt = mpDrawEditEngine->GetParagraphCount();
    sal_Int32 nSP = 0;
    sal_Int32 nEP = 0;
    while(      (nSP < nPCnt)
            &&  (nCpStart >= mpDrawEditEngine->GetTextLen( nSP ) + 1) )
    {
        nCpStart -= mpDrawEditEngine->GetTextLen( nSP ) + 1;
        nSP++;
    }
        
        
    while(      (nEP < nPCnt)
            &&  (nCpEnd > mpDrawEditEngine->GetTextLen( nEP ) + 1) )
    {
        nCpEnd -= mpDrawEditEngine->GetTextLen( nEP ) + 1;
        nEP++;
    }
    return ESelection( nSP, nCpStart, nEP, nCpEnd );
}











void SwWW8ImplReader::InsertTxbxStyAttrs( SfxItemSet& rS, sal_uInt16 nColl )
{
    SwWW8StyInf * pStyInf = GetStyle(nColl);
    if( pStyInf != NULL && pStyInf->pFmt && pStyInf->bColl )
    {
        const SfxPoolItem* pItem;
        for( sal_uInt16 i = POOLATTR_BEGIN; i < POOLATTR_END; i++ )
        {
            
            
            if ( SFX_ITEM_SET == pStyInf->pFmt->GetItemState(
                i, true, &pItem ) )
            {
                SfxItemPool *pEditPool = rS.GetPool();
                sal_uInt16 nWhich = i;
                sal_uInt16 nSlotId = rDoc.GetAttrPool().GetSlotId(nWhich);
                if (
                    nSlotId && nWhich != nSlotId &&
                    0 != (nWhich = pEditPool->GetWhich(nSlotId)) &&
                    nWhich != nSlotId &&
                    ( SFX_ITEM_SET != rS.GetItemState(nWhich, false) )
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
    long mnStartPos; 
    long mnEndPos;   
public:
    explicit Chunk(long nStart, const OUString &rURL)
        : msURL(rURL), mnStartPos(nStart), mnEndPos(0)  {}
    Chunk(const Chunk &rChunk)
        : msURL(rChunk.msURL), mnStartPos(rChunk.mnStartPos),
        mnEndPos(rChunk.mnEndPos) {}
    Chunk& operator=(const Chunk &rChunk)
    {
        msURL = rChunk.msURL;
        mnStartPos = rChunk.mnStartPos;
        mnEndPos = rChunk.mnEndPos;
        return *this;
    }
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
    pPlcxMan = new WW8PLCFMan(pSBase, eType, nStartCp, true);

    WW8_CP nStart = pPlcxMan->Where();
    WW8_CP nNext, nEnd, nStartReplace=0;

    bool bDoingSymbol = false;
    sal_Unicode cReplaceSymbol = cSymbol;

    SfxItemSet *pS = new SfxItemSet(mpDrawEditEngine->GetEmptyItemSet());
    WW8PLCFManResult aRes;

    std::deque<Chunk> aChunks;

    
    size_t nCurrentCount = pCtrlStck->size();
    while (nStart < nEndCp)
    {
        
        
        WW8_CP nTxtStart = nStart;
        if (nTxtStart < nStartCp)
            nTxtStart = nStartCp;
        
        bool bStartAttr = pPlcxMan->Get(&aRes);
        nAktColl = pPlcxMan->GetColl();
        if (aRes.nSprmId)
        {
            if( bONLYnPicLocFc )
            {
                if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
                {
                    Read_PicLoc(aRes.nSprmId, aRes.pMemPos +
                        mpSprmParser->DistanceToData(aRes.nSprmId), 4);
                     
                     
                    break;
                }
            }
            else if ( aRes.nSprmId && (
                (eFTN >  aRes.nSprmId) || (0x0800 <= aRes.nSprmId) ) )
            {
                
                
                if (bStartAttr)
                {
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
                    if (!bDoingSymbol && bSymbol == true)
                    {
                        bDoingSymbol = true;
                        nStartReplace = nTxtStart;
                        cReplaceSymbol = cSymbol;
                    }
                }
                else
                {
                    EndSprm( aRes.nSprmId );
                    if (bSymbol == false && bDoingSymbol)
                    {
                        bDoingSymbol = false;
                        OUStringBuffer sTemp;
                        comphelper::string::padToLength(sTemp,
                            nTxtStart - nStartReplace, cReplaceSymbol);
                        mpDrawEditEngine->QuickInsertText(sTemp.makeStringAndClear(),
                            GetESelection(nStartReplace - nStartCp,
                            nTxtStart - nStartCp ) );
                    }
                }
            }
            else if (aRes.nSprmId == eFLD)
            {
                if (bStartAttr)
                {
                    size_t nCount = pCtrlStck->size();
                    if (maFieldStack.empty() && Read_Field(&aRes))
                    {
                        OUString sURL;
                        for (size_t nI = pCtrlStck->size(); nI > nCount; --nI)
                        {
                            const SfxPoolItem *pItem = ((*pCtrlStck)[nI-1]).pAttr;
                            sal_uInt16 nWhich = pItem->Which();
                            if (nWhich == RES_TXTATR_INETFMT)
                            {
                                const SwFmtINetFmt *pURL =
                                    (const SwFmtINetFmt *)pItem;
                                sURL = pURL->GetValue();
                            }
                            pCtrlStck->DeleteAndDestroy(nI-1);
                        }
                        aChunks.push_back(Chunk(nStart, sURL));
                    }
                }
                else
                {
                    if (!maFieldStack.empty() && End_Field())
                        aChunks.back().SetEndPos(nStart+1);
                }
            }
        }

        pPlcxMan->advance();
        nNext = pPlcxMan->Where();

        if( (nNext != nStart) && !bONLYnPicLocFc )
        {
            nEnd = ( nNext < nEndCp ) ? nNext : nEndCp;
            SfxItemPool *pEditPool = pS->GetPool();

            
            
            
            if (pCtrlStck->size() - nCurrentCount)
            {
                for (size_t i = nCurrentCount; i < pCtrlStck->size(); ++i)
                {
                    const SfxPoolItem *pItem = ((*pCtrlStck)[i]).pAttr;
                    sal_uInt16 nWhich = pItem->Which();
                    if( nWhich < RES_FLTRATTR_BEGIN ||
                        nWhich >= RES_FLTRATTR_END )
                    {
                        sal_uInt16 nSlotId = rDoc.GetAttrPool().GetSlotId(nWhich);
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
            
            InsertTxbxStyAttrs(*pS, nAktColl);

            if( pS->Count() )
            {
                mpDrawEditEngine->QuickSetAttribs( *pS,
                    GetESelection( nTxtStart - nStartCp, nEnd - nStartCp ) );
                delete pS;
                pS = new SfxItemSet(mpDrawEditEngine->GetEmptyItemSet());
            }
        }
        nStart = nNext;
    }
    delete pS;

    
    
    for (size_t nI = pCtrlStck->size(); nI > nCurrentCount; --nI)
        pCtrlStck->DeleteAndDestroy(nI-1);

    typedef std::deque<Chunk>::iterator myIter;
    myIter aEnd = aChunks.end();
    for (myIter aIter = aChunks.begin(); aIter != aEnd; ++aIter)
    {
        ESelection aSel(GetESelection(aIter->GetStartPos()-nStartCp,
            aIter->GetEndPos()-nStartCp));
        OUString aString(mpDrawEditEngine->GetText(aSel));
        const sal_Int32 nOrigLen = aString.getLength();
        long nDummy(0);
        lcl_StripFields(aString, nDummy);

        sal_Int32 nChanged;
        if (!aIter->GetURL().isEmpty())
        {
            SvxURLField aURL(aIter->GetURL(), aString,
                SVXURLFORMAT_APPDEFAULT);
            mpDrawEditEngine->QuickInsertField(SvxFieldItem(aURL, EE_FEATURE_FIELD), aSel);
            nChanged = nOrigLen - 1;
        }
        else
        {
            mpDrawEditEngine->QuickInsertText(aString, aSel);
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
    
    WW8PLCFspecial* pT = pPlcxMan ? pPlcxMan->GetTxbx() : 0;
    if( !pT )
    {
        OSL_ENSURE( !this, "+Wo ist der Grafik-Text (1) ?" );
        return false;
    }

    
    bool bCheckTextBoxStory = ( nTxBxS && pT->GetIMax() >= nTxBxS );
    if(  bCheckTextBoxStory )
        pT->SetIdx( nTxBxS-1 );

    
    void* pT0;
    if( !pT->Get( rStartCp, pT0 ) )
    {
        OSL_ENSURE( !this, "+Wo ist der Grafik-Text (2) ?" );
        return false;
    }

    if( bCheckTextBoxStory )
    {
        bool bReusable = (0 != SVBT16ToShort( ((WW8_TXBXS*)pT0)->fReusable ));
        while( bReusable )
        {
            pT->advance();
            if( !pT->Get( rStartCp, pT0 ) )
            {
                OSL_ENSURE( !this, "+Wo ist der Grafik-Text (2-a) ?" );
                return false;
            }
            bReusable = (0 != SVBT16ToShort( ((WW8_TXBXS*)pT0)->fReusable ));
        }
    }
    pT->advance();
    if( !pT->Get( rEndCp, pT0 ) )
    {
        OSL_ENSURE( !this, "+Wo ist der Grafik-Text (3) ?" );
        return false;
    }

    
    if( bCheckTextBoxStory )
    {
        
        
        if( USHRT_MAX > nSequence )
        {
            long nMinStartCp = rStartCp;
            long nMaxEndCp   = rEndCp;
            
            pT = pPlcxMan->GetTxbxBkd();
            if (!pT) 
                return false;

            
            if( !pT->SeekPos( rStartCp ) )
            {
                OSL_ENSURE( !this, "+Wo ist der Grafik-Text (4) ?" );
                return false;
            }
            
            for (sal_uInt16 iSequence = 0; iSequence < nSequence; ++iSequence)
                pT->advance();
            
            if(    (!pT->Get( rStartCp, pT0 ))
                || ( nMinStartCp > rStartCp  ) )
            {
                OSL_ENSURE( !this, "+Wo ist der Grafik-Text (5) ?" );
                return false;
            }
            if( rStartCp >= nMaxEndCp )
                rEndCp = rStartCp;  
            else
            {
                pT->advance();
                if ( (!pT->Get(rEndCp, pT0)) || (nMaxEndCp < rEndCp-1) )
                {
                    OSL_ENSURE( !this, "+Wo ist der Grafik-Text (6) ?" );
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



bool SwWW8ImplReader::GetRangeAsDrawingString(OUString& rString, long nStartCp, long nEndCp, ManTypes eType)
{
    WW8_CP nOffset = pWwFib->GetBaseCp(eType);

    OSL_ENSURE(nStartCp <= nEndCp, "+Wo ist der Grafik-Text (7) ?");
    if (nStartCp == nEndCp)
        rString = OUString();      
    else if (nStartCp < nEndCp)
    {
        
        const sal_Int32 nLen = pSBase->WW8ReadString(*pStrm, rString,
            nStartCp + nOffset, nEndCp - nStartCp, GetCurrentCharSet());
        OSL_ENSURE(nLen, "+Wo ist der Grafik-Text (8) ?");
        if (nLen>0)
        {
            if( rString[nLen-1]==0x0d )
                rString = rString.copy(0, nLen-1);

            rString = rString.replace( 0xb, 0xa );
            return true;
        }
    }
    return false;
}

OutlinerParaObject* SwWW8ImplReader::ImportAsOutliner(OUString &rString, WW8_CP nStartCp, WW8_CP nEndCp, ManTypes eType)
{
    OutlinerParaObject* pRet = 0;

    if (GetRangeAsDrawingString( rString, nStartCp, nEndCp, eType ))
    {
        if (!mpDrawEditEngine)
            mpDrawEditEngine = new EditEngine(0);

        mpDrawEditEngine->SetText(rString);
        InsertAttrsAsDrawingAttrs(nStartCp, nEndCp, eType);

        
        if ((eType == MAN_AND) && mpDrawEditEngine->GetTextLen())
        {
            ESelection aFirstChar(0, 0, 0, 1);
            if (comphelper::string::equals(mpDrawEditEngine->GetText( aFirstChar ), 0x5))
                mpDrawEditEngine->QuickDelete(aFirstChar);
        }

        EditTextObject* pTemporaryText = mpDrawEditEngine->CreateTextObject();
        pRet = new OutlinerParaObject(*pTemporaryText);
        pRet->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
        delete pTemporaryText;

        mpDrawEditEngine->SetText( OUString() );
        mpDrawEditEngine->SetParaAttribs(0, mpDrawEditEngine->GetEmptyItemSet());

        
        long nDummy(0);
        lcl_StripFields(rString, nDummy);
        
        rString = rString.replaceAll(OUString(0x1), "");
        rString = rString.replaceAll(OUString(0x5), "");
        rString = rString.replaceAll(OUString(0x8), "");
        rString = rString.replaceAll("\007\007", "\007\012");
        rString = rString.replace(0x7, ' ');
    }

    return pRet;
}



SwFrmFmt* SwWW8ImplReader::InsertTxbxText(SdrTextObj* pTextObj,
    Size* pObjSiz, sal_uInt16 nTxBxS, sal_uInt16 nSequence, long nPosCp,
    SwFrmFmt* pOldFlyFmt, bool bMakeSdrGrafObj, bool& rbEraseTextObj,
    bool* pbTestTxbxContainsText, long* pnStartCp, long* pnEndCp,
    bool* pbContainsGraphics, SvxMSDffImportRec* pRecord)
{
    SwFrmFmt* pFlyFmt = 0;
    sal_uLong nOld = pStrm->Tell();

    ManTypes eType = pPlcxMan->GetManType() == MAN_HDFT ? MAN_TXBX_HDFT : MAN_TXBX;

    rbEraseTextObj = false;

    OUString aString;
    WW8_CP nStartCp, nEndCp;
    bool bContainsGraphics = false;
    bool bTextWasRead = GetTxbxTextSttEndCp( nStartCp, nEndCp, nTxBxS,
        nSequence ) && GetRangeAsDrawingString( aString, nStartCp, nEndCp, eType );

    if (!mpDrawEditEngine)
        mpDrawEditEngine = new EditEngine(0);
    if( pObjSiz )
        mpDrawEditEngine->SetPaperSize( *pObjSiz );

    const OUString aOrigString(aString);
    if( bTextWasRead )
    {
        long nNewStartCp = nStartCp;
        lcl_StripFields(aString, nNewStartCp);

        if (aString.getLength()!=1)
        {
            bContainsGraphics = aString.indexOf(0x1)<0 || aString.indexOf(0x8)<0;
        }
        else        
        {
            bool bDone = true;
            switch( aString[0] )
            {
                case 0x1:
                    if (!pbTestTxbxContainsText)
                    {
                        WW8ReaderSave aSave(this, nNewStartCp -1);
                        bool bOldEmbeddObj = bEmbeddObj;
                        
                        
                        bEmbeddObj = true;

                        
                        WW8PLCFx_Cp_FKP* pChp = pPlcxMan->GetChpPLCF();
                        WW8PLCFxDesc aDesc;
                        pChp->GetSprms( &aDesc );
                        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen,
                            *mpSprmParser);

                        for( int nLoop = 0; nLoop < 2; ++nLoop )
                        {
                            const sal_uInt8* pParams;
                            while( aSprmIter.GetSprms()
                                && (0 != (pParams = aSprmIter.GetAktParams())) )
                            {
                                sal_uInt16 nAktId = aSprmIter.GetAktId();
                                switch( nAktId )
                                {
                                    case     75:
                                    case    118:
                                    case 0x080A:
                                    case 0x0856:
                                            Read_Obj(nAktId, pParams, 1);
                                        break;
                                    case     68:  
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
                        bEmbeddObj=bOldEmbeddObj;

                        
                        if( bObj )
                        {
                            if( bMakeSdrGrafObj && pTextObj &&
                                pTextObj->GetUpGroup() )
                            {
                                
                                

                                Graphic aGraph;
                                SdrObject* pNew = ImportOleBase(aGraph);

                                if( !pNew )
                                {
                                    pNew = new SdrGrafObj;
                                    ((SdrGrafObj*)pNew)->SetGraphic(aGraph);
                                }

                                GrafikCtor();

                                pNew->SetModel( pDrawModel );
                                pNew->SetLogicRect( pTextObj->GetCurrentBoundRect() );
                                pNew->SetLayer( pTextObj->GetLayer() );

                                pTextObj->GetUpGroup()->GetSubList()->
                                    ReplaceObject(pNew, pTextObj->GetOrdNum());
                            }
                            else
                                pFlyFmt = ImportOle();
                            bObj = false;
                        }
                        else
                        {
                            InsertAttrsAsDrawingAttrs(nNewStartCp, nNewStartCp+1,
                                eType, true);
                            pFlyFmt = ImportGraf(bMakeSdrGrafObj ? pTextObj : 0,
                                pOldFlyFmt);
                        }
                    }
                    break;
                case 0x8:
                    if ( (!pbTestTxbxContainsText) && (!bObj) )
                        pFlyFmt = Read_GrafLayer( nPosCp );
                    break;
                default:
                    bDone = false;
                    break;
            }

            if( bDone )
            {
                if( pFlyFmt )
                {
                    if( pRecord )
                    {
                        SfxItemSet aFlySet( rDoc.GetAttrPool(),
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

                        pFlyFmt->SetFmtAttr( aFlySet );

                        MapWrapIntoFlyFmt(pRecord, pFlyFmt);
                    }
                }
                aString = OUString();
                rbEraseTextObj = (0 != pFlyFmt);
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
            mpDrawEditEngine->SetText(aOrigString);
            InsertAttrsAsDrawingAttrs(nStartCp, nEndCp, eType);
        }

        bool bVertical = pTextObj->IsVerticalWriting() ? true : false;
        EditTextObject* pTemporaryText = mpDrawEditEngine->CreateTextObject();
        OutlinerParaObject* pOp = new OutlinerParaObject(*pTemporaryText);
        pOp->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
        pOp->SetVertical( bVertical );
        delete pTemporaryText;
        pTextObj->NbcSetOutlinerParaObject( pOp );
        pTextObj->SetVerticalWriting(bVertical);

        
        
        
        
        
        
        mpDrawEditEngine->SetText( OUString() );
        mpDrawEditEngine->SetParaAttribs(0, mpDrawEditEngine->GetEmptyItemSet());
    }

    pStrm->Seek( nOld );
    if (pbContainsGraphics)
        *pbContainsGraphics = bContainsGraphics;
    return pFlyFmt;
}


bool SwWW8ImplReader::TxbxChainContainsRealText(sal_uInt16 nTxBxS, long& rStartCp,
    long&  rEndCp)
{
    bool bErase, bContainsText;
    InsertTxbxText( 0,0,nTxBxS,USHRT_MAX,0,0,false, bErase, &bContainsText,
        &rStartCp, &rEndCp );
    return bContainsText;
}



SdrObject* SwWW8ImplReader::ReadTxtBox( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    bool bDummy;
    WW8_DP_TXTBOX aTxtB;

    if( !ReadGrafStart( (void*)&aTxtB, sizeof( aTxtB ), pHd, pDo, rSet ) )
        return 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrRectObj( OBJ_TEXT, Rectangle( aP0, aP1 ) );
    pObj->SetModel( pDrawModel );
    pObj->NbcSetSnapRect(Rectangle(aP0, aP1));
    Size aSize( (sal_Int16)SVBT16ToShort( pHd->dxa ) ,
        (sal_Int16)SVBT16ToShort( pHd->dya ) );

    long nStartCpFly,nEndCpFly;
    bool bContainsGraphics;
    InsertTxbxText(PTR_CAST(SdrTextObj,pObj), &aSize, 0, 0, 0, 0, false,
        bDummy,0,&nStartCpFly,&nEndCpFly,&bContainsGraphics);

    SetStdAttr( rSet, aTxtB.aLnt, aTxtB.aShd );
    SetFill( rSet, aTxtB.aFill );

    rSet.Put( SdrTextFitToSizeTypeItem( SDRTEXTFIT_NONE ) );
    rSet.Put( SdrTextAutoGrowWidthItem(false));
    rSet.Put( SdrTextAutoGrowHeightItem(false));
    rSet.Put( SdrTextLeftDistItem(  MIN_BORDER_DIST*2 ) );
    rSet.Put( SdrTextRightDistItem( MIN_BORDER_DIST*2 ) );
    rSet.Put( SdrTextUpperDistItem( MIN_BORDER_DIST ) );
    rSet.Put( SdrTextLowerDistItem( MIN_BORDER_DIST ) );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadCaptionBox( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    static const SdrCaptionType aCaptA[] = { SDRCAPT_TYPE1, SDRCAPT_TYPE2,
                                       SDRCAPT_TYPE3, SDRCAPT_TYPE4 };

    WW8_DP_CALLOUT_TXTBOX aCallB;

    if( !ReadGrafStart( (void*)&aCallB, sizeof( aCallB ), pHd, pDo, rSet ) )
        return 0;

    sal_uInt16 nCount = SVBT16ToShort( aCallB.dpPolyLine.aBits1 ) >> 1 & 0x7fff;
    boost::scoped_array<SVBT16> xP(new SVBT16[nCount * 2]);

    bool bCouldRead = checkRead(*pStrm, xP.get(), nCount * 4);      
    OSL_ENSURE(bCouldRead, "Short CaptionBox header");
    if (!bCouldRead)
        return 0;

    sal_uInt8 nTyp = (sal_uInt8)nCount - 1;
    if( nTyp == 1 && SVBT16ToShort( xP[0] ) == SVBT16ToShort( xP[2] ) )
        nTyp = 0;

    Point aP0( (sal_Int16)SVBT16ToShort( pHd->xa ) +
               (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.xa ) + nDrawXOfs2,
               (sal_Int16)SVBT16ToShort( pHd->ya )
               + (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dxa );
    aP1.Y() += (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dya );
    Point aP2( (sal_Int16)SVBT16ToShort( pHd->xa )
                + (sal_Int16)SVBT16ToShort( aCallB.dpheadPolyLine.xa )
                + nDrawXOfs2 + (sal_Int16)SVBT16ToShort( xP[0] ),
               (sal_Int16)SVBT16ToShort( pHd->ya )
               + (sal_Int16)SVBT16ToShort( aCallB.dpheadPolyLine.ya )
               + nDrawYOfs2 + (sal_Int16)SVBT16ToShort( xP[1] ) );
    xP.reset();

    SdrCaptionObj* pObj = new SdrCaptionObj( Rectangle( aP0, aP1 ), aP2 );
    pObj->SetModel( pDrawModel );
    pObj->NbcSetSnapRect(Rectangle(aP0, aP1));
    Size aSize( (sal_Int16)SVBT16ToShort( aCallB.dpheadTxbx.dxa ),
                           (sal_Int16)SVBT16ToShort(  aCallB.dpheadTxbx.dya ) );
    bool bEraseThisObject;

    InsertTxbxText(pObj, &aSize, 0, 0, 0, 0, false, bEraseThisObject );

    if( SVBT16ToShort( aCallB.dptxbx.aLnt.lnps ) != 5 ) 
        SetStdAttr( rSet, aCallB.dptxbx.aLnt, aCallB.dptxbx.aShd );
    else                                                
        SetStdAttr( rSet, aCallB.dpPolyLine.aLnt, aCallB.dptxbx.aShd );
    SetFill( rSet, aCallB.dptxbx.aFill );
    rSet.Put( SdrCaptionTypeItem( aCaptA[nTyp] ) );

    return pObj;
}


SdrObject *SwWW8ImplReader::ReadGroup( WW8_DPHEAD* pHd, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    sal_Int16 nGrouped;

    if( !ReadGrafStart( (void*)&nGrouped, sizeof( nGrouped ), pHd, pDo, rSet ) )
        return 0;

#ifdef OSL_BIGENDIAN
    nGrouped = (sal_Int16)OSL_SWAPWORD( nGrouped );
#endif

    nDrawXOfs = nDrawXOfs + (sal_Int16)SVBT16ToShort( pHd->xa );
    nDrawYOfs = nDrawYOfs + (sal_Int16)SVBT16ToShort( pHd->ya );

    SdrObject* pObj = new SdrObjGroup;

    short nLeft = (sal_Int16)SVBT16ToShort( pHd->cb ) - sizeof( WW8_DPHEAD );
    for (int i = 0; i < nGrouped; i++)
    {
        SfxAllItemSet aSet(pDrawModel->GetItemPool());
        if (SdrObject *pObject = ReadGrafPrimitive(nLeft, pDo, aSet))
        {
            
            SdrObjList *pSubGroup = pObj->GetSubList();
            OSL_ENSURE(pSubGroup, "Why no sublist available?");
            if (pSubGroup)
                pSubGroup->InsertObject(pObject, 0);
            pObject->SetMergedItemSetAndBroadcast(aSet);
        }
    }

    nDrawXOfs = nDrawXOfs - (sal_Int16)SVBT16ToShort( pHd->xa );
    nDrawYOfs = nDrawYOfs - (sal_Int16)SVBT16ToShort( pHd->ya );

    return pObj;
}

SdrObject* SwWW8ImplReader::ReadGrafPrimitive( short& rLeft, const WW8_DO* pDo,
    SfxAllItemSet &rSet)
{
    
    
    SdrObject *pRet=0;
    WW8_DPHEAD aHd;                         
    bool bCouldRead = checkRead(*pStrm, &aHd, sizeof(WW8_DPHEAD));
    OSL_ENSURE(bCouldRead, "Graphic Primitive header short read" );
    if (!bCouldRead)
    {
        rLeft=0;
        return pRet;
    }

    if( rLeft >= SVBT16ToShort(aHd.cb) )    
    {
        rSet.Put(SwFmtSurround(SURROUND_THROUGHT));
        switch (SVBT16ToShort(aHd.dpk) & 0xff )
        {
            case 0:
                pRet = ReadGroup( &aHd, pDo, rSet );
                break;
            case 1:
                pRet = ReadLine( &aHd, pDo, rSet );
                break;
            case 2:
                pRet = ReadTxtBox( &aHd, pDo, rSet );
                break;
            case 3:
                pRet = ReadRect( &aHd, pDo, rSet );
                break;
            case 4:
                pRet = ReadElipse( &aHd, pDo, rSet );
                break;
            case 5:
                pRet = ReadArc( &aHd, pDo, rSet );
                break;
            case 6:
                pRet = ReadPolyLine( &aHd, pDo, rSet );
                break;
            case 7:
                pRet = ReadCaptionBox( &aHd, pDo, rSet );
                break;
            default:    
                pStrm->SeekRel(SVBT16ToShort(aHd.cb) - sizeof(WW8_DPHEAD));
                break;
        }
    }
    else
    {
        OSL_ENSURE( !this, "+Grafik-Overlap" );
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
        OSL_ENSURE( !this, "+Wo ist die Grafik (2) ?" );
        return;
    }
    WW8_FDOA* pF = (WW8_FDOA*)pF0;
    if( !SVBT32ToUInt32( pF->fc ) )
    {
        OSL_ENSURE( !this, "+Wo ist die Grafik (3) ?" );
        return;
    }

    bool bCouldSeek = checkSeek(*pStrm, SVBT32ToUInt32(pF->fc));
    OSL_ENSURE(bCouldSeek, "Invalid Graphic offset");
    if (!bCouldSeek)
        return;

    
    WW8_DO aDo;
    bool bCouldRead = checkRead(*pStrm, &aDo, sizeof(WW8_DO));
    OSL_ENSURE(bCouldRead, "Short Graphic header");
    if (!bCouldRead)
        return;

    short nLeft = SVBT16ToShort( aDo.cb ) - sizeof( WW8_DO );
    while (nLeft > static_cast<short>(sizeof(WW8_DPHEAD)))
    {
        SfxAllItemSet aSet( pDrawModel->GetItemPool() );
        if (SdrObject *pObject = ReadGrafPrimitive( nLeft, &aDo, aSet ))
        {
            pWWZOrder->InsertDrawingObject(pObject, SVBT16ToShort(aDo.dhgt));
            SwFrmFmt *pFrm = rDoc.InsertDrawObj( *pPaM, *pObject, aSet );
            pObject->SetMergedItemSet(aSet);
            pAnchorStck->AddAnchor(*pPaM->GetPoint(), pFrm);
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
    
    case mso_lineSimple:
        nIdx = table::BorderLineStyle::SOLID;
    break;
    
    case mso_lineDouble:
        nIdx = table::BorderLineStyle::DOUBLE;
    break;
    case mso_lineThickThin:
        nIdx = table::BorderLineStyle::THICKTHIN_SMALLGAP;
    break;
    case mso_lineThinThick:
        nIdx = table::BorderLineStyle::THINTHICK_SMALLGAP;
    break;
    
    case mso_lineTriple:
        nIdx = table::BorderLineStyle::DOUBLE;
    break;
    
    case (MSO_LineStyle)USHRT_MAX:
        break;
    
    default:
        OSL_ENSURE(!this, "eLineStyle is not (yet) implemented!");
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

        aLine.SetWidth( nLineThick ); 
        aLine.SetBorderLineStyle(nIdx);

        for(sal_uInt16 nLine = 0; nLine < 4; ++nLine)
        {
            
            rBox.SetLine(&aLine, nLine);
        }
    }

    return nOutsideThick;
}

#define WW8ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

void SwWW8ImplReader::MatchSdrItemsIntoFlySet( SdrObject* pSdrObj,
    SfxItemSet& rFlySet, MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType,
    Rectangle& rInnerDist )
{
/*
    am Rahmen zu setzende Frame-Attribute
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SwFmtFrmSize            falls noch nicht gesetzt, hier setzen
    SvxLRSpaceItem          hier setzen
    SvxULSpaceItem          hier setzen
    SvxOpaqueItem           (Derzeit bei Rahmen nicht moeglich! khz 10.2.1999)
    SwFmtSurround           bereits gesetzt
    SwFmtVertOrient         bereits gesetzt
    SwFmtHoriOrient         bereits gesetzt
    SwFmtAnchor             bereits gesetzt
    SvxBoxItem              hier setzen
    SvxBrushItem            hier setzen
    SvxShadowItem           hier setzen
*/

    
    GrafikCtor();

    const SfxItemSet& rOldSet = pSdrObj->GetMergedItemSet();

    
    const sal_uInt16 nDirectMatch = 2;
    static RES_FRMATR const aDirectMatch[ nDirectMatch ] =
    {
        RES_LR_SPACE,   
        RES_UL_SPACE    
    };
    const SfxPoolItem* pPoolItem;
    for(sal_uInt16 nItem = 0; nItem < nDirectMatch; ++nItem)
        if( SFX_ITEM_SET == rOldSet.GetItemState(
                    static_cast< sal_uInt16 >(aDirectMatch[ nItem ]), false, &pPoolItem) )
        {
            rFlySet.Put( *pPoolItem );
        }


    
    
    SvxBoxItem aBox(sw::util::ItemGet<SvxBoxItem>(rFlySet, RES_BOX));
    
    
    sal_Int32 nLineThick = 15, nOutside=0;

    
    const SfxPoolItem* pItem;

    SfxItemState eState = rOldSet.GetItemState(XATTR_LINESTYLE,true,&pItem);
    if( eState == SFX_ITEM_SET )
    {
        
        
        const Color aLineColor = static_cast< XLineColorItem const & >(
            rOldSet.Get(XATTR_LINECOLOR)).GetColorValue();
        nLineThick = WW8ITEMVALUE(rOldSet, XATTR_LINEWIDTH, XLineWidthItem);

        if( !nLineThick )
            nLineThick = 1; 

        nOutside = MatchSdrBoxIntoFlyBoxItem(aLineColor, eLineStyle,
            eDashing, eShapeType, nLineThick, aBox);
    }

    rInnerDist.Left()+=nLineThick;
    rInnerDist.Top()+=nLineThick;
    rInnerDist.Right()+=nLineThick;
    rInnerDist.Bottom()+=nLineThick;

    const SvxBorderLine *pLine;
    if (0 != (pLine = aBox.GetLine(BOX_LINE_LEFT)))
    {
        rInnerDist.Left() -= (pLine->GetScaledWidth());
    }

    if (0 != (pLine = aBox.GetLine(BOX_LINE_TOP)))
    {
        rInnerDist.Top() -= (pLine->GetScaledWidth());
    }

    if (0 != (pLine = aBox.GetLine(BOX_LINE_RIGHT)))
    {
        rInnerDist.Right() -= (pLine->GetScaledWidth());
    }

    if (0 != (pLine = aBox.GetLine(BOX_LINE_BOTTOM)))
    {
        rInnerDist.Bottom() -= (pLine->GetScaledWidth());
    }

    
    if( 0 < rInnerDist.Left() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Left(), BOX_LINE_LEFT );
    if( 0 < rInnerDist.Top() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Top(), BOX_LINE_TOP );
    if( 0 < rInnerDist.Right() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Right(), BOX_LINE_RIGHT );
    if( 0 < rInnerDist.Bottom() )
        aBox.SetDistance( (sal_uInt16)rInnerDist.Bottom(), BOX_LINE_BOTTOM );

    bool bFixSize = !(WW8ITEMVALUE(rOldSet, SDRATTR_TEXT_AUTOGROWHEIGHT,
        SdrTextAutoGrowHeightItem));

    
    if( SFX_ITEM_SET != rFlySet.GetItemState(RES_FRM_SIZE, false) )
    {
        const Rectangle& rSnapRect = pSdrObj->GetSnapRect();
        
        
        rFlySet.Put( SwFmtFrmSize(bFixSize ? ATT_FIX_SIZE : ATT_VAR_SIZE,
            rSnapRect.GetWidth()  + 2*nOutside,
            rSnapRect.GetHeight() + 2*nOutside) );
    }
    else 
    {
        SwFmtFrmSize aSize = (const SwFmtFrmSize &)(rFlySet.Get(RES_FRM_SIZE));

        SwFmtFrmSize aNewSize = SwFmtFrmSize(bFixSize ? ATT_FIX_SIZE : ATT_VAR_SIZE,
            aSize.GetWidth()  + 2*nOutside,
            aSize.GetHeight() + 2*nOutside);
        aNewSize.SetWidthSizeType(aSize.GetWidthSizeType());
        rFlySet.Put( aNewSize );
    }

    
    
    if (nOutside)
    {
        SwFmtHoriOrient aHori = (const SwFmtHoriOrient &)(rFlySet.Get(
            RES_HORI_ORIENT));
        aHori.SetPos(MakeSafePositioningValue(aHori.GetPos()-nOutside));
        rFlySet.Put(aHori);

        SwFmtVertOrient aVert = (const SwFmtVertOrient &)(rFlySet.Get(
            RES_VERT_ORIENT));
        aVert.SetPos(aVert.GetPos()-nOutside);
        rFlySet.Put(aVert);
    }

    
    rFlySet.Put( aBox );

    
    if( WW8ITEMVALUE(rOldSet, SDRATTR_SHADOW, SdrShadowItem) )
    {
        SvxShadowItem aShadow( RES_SHADOW );

        const Color aShdColor = static_cast< SdrShadowColorItem const & >(
            rOldSet.Get(SDRATTR_SHADOWCOLOR)).GetColorValue();
        const sal_Int32 nShdDistX = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWXDIST,
            SdrShadowXDistItem);
        const sal_Int32 nShdDistY = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWYDIST,
            SdrShadowYDistItem);

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

    
    eState = rOldSet.GetItemState(XATTR_FILLTRANSPARENCE, true, &pItem);
    if (eState == SFX_ITEM_SET)
    {
        sal_uInt16 nRes = WW8ITEMVALUE(rOldSet, XATTR_FILLTRANSPARENCE,
            XFillTransparenceItem);
        nTrans = sal_uInt8((nRes * 0xFE) / 100);
        aBrushItem.GetColor().SetTransparency(nTrans);
        bBrushItemOk = true;
    }

    
    eState = rOldSet.GetItemState(XATTR_FILLSTYLE, true, &pItem);
    if (eState == SFX_ITEM_SET)
    {
        const XFillStyle eFill = ((const XFillStyleItem*)pItem)->GetValue();

        switch (eFill)
        {
            case XFILL_NONE:
                
                if (eShapeType != mso_sptPictureFrame)
                {
                    aBrushItem.GetColor().SetTransparency(0xFE);
                    bBrushItemOk = true;
                }
            break;
            case XFILL_SOLID:
            case XFILL_GRADIENT:
                {
                    const Color aColor = static_cast< XFillColorItem const & >(
                        rOldSet.Get(XATTR_FILLCOLOR)).GetColorValue();
                    aBrushItem.SetColor(aColor);

                    if (bBrushItemOk) 
                        aBrushItem.GetColor().SetTransparency(nTrans);

                    bBrushItemOk = true;
                }
            break;
            
            
            case XFILL_HATCH:
            break;
            case XFILL_BITMAP:
                {
                    GraphicObject aGrfObj(static_cast< XFillBitmapItem const & >(rOldSet.Get(XATTR_FILLBITMAP)).GetGraphicObject());
                    const bool bTile(WW8ITEMVALUE(rOldSet, XATTR_FILLBMP_TILE, SfxBoolItem) ? true: false);

                    if(bBrushItemOk) 
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
        rFlySet.Put(aBrushItem, RES_BACKGROUND);
}

void SwWW8ImplReader::AdjustLRWrapForWordMargins(
    const SvxMSDffImportRec &rRecord, SvxLRSpaceItem &rLR)
{
    sal_uInt32 nXRelTo = SvxMSDffImportRec::RELTO_DEFAULT;
    if ( rRecord.pXRelTo )
    {
        nXRelTo = *(rRecord.pXRelTo);
    }

    
    
    if (rRecord.nXAlign == 1)
    {
        if ((nXRelTo == 0) || (nXRelTo == 2))
            rLR.SetLeft((sal_uInt16)0);
    }

    
    
    if (rRecord.nXAlign == 3)
    {
        if ((nXRelTo == 0) || (nXRelTo == 2))
            rLR.SetRight((sal_uInt16)0);
    }

    
    if ((rRecord.nXAlign == 4) && (nXRelTo == 0))
    {
        rLR.SetLeft((sal_uInt16)0);
    }

    
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

    
    
    if (rRecord.nYAlign == 1)
    {
        if ((nYRelTo == 0) || (nYRelTo == 1))
            rUL.SetUpper((sal_uInt16)0);
    }

    
    
    if (rRecord.nYAlign == 3)
    {
        if ((nYRelTo == 0) || (nYRelTo == 1))
            rUL.SetLower((sal_uInt16)0);
    }

    
    if ((rRecord.nYAlign == 4) && (nYRelTo == 0))
        rUL.SetUpper((sal_uInt16)0);
}

void SwWW8ImplReader::MapWrapIntoFlyFmt(SvxMSDffImportRec* pRecord,
    SwFrmFmt* pFlyFmt)
{
    if (!pRecord || !pFlyFmt)
        return;

    if (pRecord->nDxWrapDistLeft || pRecord->nDxWrapDistRight)
    {
        SvxLRSpaceItem aLR(writer_cast<sal_uInt16>(pRecord->nDxWrapDistLeft),
            writer_cast<sal_uInt16>(pRecord->nDxWrapDistRight), 0, 0, RES_LR_SPACE);
        AdjustLRWrapForWordMargins(*pRecord, aLR);
        pFlyFmt->SetFmtAttr(aLR);
    }
    if (pRecord->nDyWrapDistTop || pRecord->nDyWrapDistBottom)
    {
        SvxULSpaceItem aUL(writer_cast<sal_uInt16>(pRecord->nDyWrapDistTop),
            writer_cast<sal_uInt16>(pRecord->nDyWrapDistBottom), RES_UL_SPACE);
        AdjustULWrapForWordMargins(*pRecord, aUL);
        pFlyFmt->SetFmtAttr(aUL);
    }

    
    if (pRecord->pWrapPolygon && pFlyFmt->GetSurround().IsContour())
    {
        if (SwNoTxtNode *pNd = GetNoTxtNodeFromSwFrmFmt(*pFlyFmt))
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

            PolyPolygon aPoly(*pRecord->pWrapPolygon);
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

            
            const Size &rOrigSize = pNd->GetGraphic().GetPrefSize();
            Fraction aMapPolyX(rOrigSize.Width(), ww::nWrap100Percent);
            Fraction aMapPolyY(rOrigSize.Height(), ww::nWrap100Percent);
            aPoly.Scale(aMapPolyX, aMapPolyY);

            
            
            pNd->SetContour(&aPoly);
        }
    }
}

void
SwWW8ImplReader::SetAttributesAtGrfNode(SvxMSDffImportRec const*const pRecord,
    SwFrmFmt *pFlyFmt, WW8_FSPA *pF )
{
    const SwNodeIndex* pIdx = pFlyFmt->GetCntnt(false).GetCntntIdx();
    SwGrfNode* pGrfNd;
    if( pIdx && 0 != (pGrfNd = rDoc.GetNodes()[pIdx->GetIndex() + 1]->GetGrfNode() ))
    {
        Size aSz(pGrfNd->GetTwipSize());
        
        
        sal_uInt64 rHeight = aSz.Height();
        sal_uInt64 rWidth  = aSz.Width();
        if( !rWidth && pF)
            rWidth  = pF->nXaRight  - pF->nXaLeft;
        else if( !rHeight && pF)
            rHeight = pF->nYaBottom - pF->nYaTop;

        if( pRecord->nCropFromTop || pRecord->nCropFromBottom ||
            pRecord->nCropFromLeft || pRecord->nCropFromRight )
        {
            SwCropGrf aCrop;            
                                        
            if( pRecord->nCropFromTop ) 
                aCrop.SetTop( static_cast< sal_Int32 >(
                (   ( (pRecord->nCropFromTop    >> 16   ) * rHeight )
                  + (((pRecord->nCropFromTop    & 0xffff) * rHeight ) >> 16) )));
            if( pRecord->nCropFromBottom )
                aCrop.SetBottom( static_cast< sal_Int32 >(
                (   ( (pRecord->nCropFromBottom >> 16   ) * rHeight )
                  + (((pRecord->nCropFromBottom & 0xffff) * rHeight ) >> 16) )));
            if( pRecord->nCropFromLeft )
                aCrop.SetLeft( static_cast< sal_Int32 >(
                (   ( (pRecord->nCropFromLeft   >> 16   ) * rWidth  )
                  + (((pRecord->nCropFromLeft   & 0xffff) * rWidth  ) >> 16) )));
            if( pRecord->nCropFromRight )
                aCrop.SetRight( static_cast< sal_Int32 >(
                (   ( (pRecord->nCropFromRight  >> 16   ) * rWidth  )
                  + (((pRecord->nCropFromRight  & 0xffff) * rWidth  ) >> 16) )));

            pGrfNd->SetAttr( aCrop );
        }

        if (pRecord->pObj)
        {
            const SfxItemSet& rOldSet = pRecord->pObj->GetMergedItemSet();
            
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFCONTRAST,
                SdrGrafContrastItem))
            {
                SwContrastGrf aContrast(
                    WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFCONTRAST, SdrGrafContrastItem));
                pGrfNd->SetAttr( aContrast );
            }

            
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFLUMINANCE,
                SdrGrafLuminanceItem))
            {
                SwLuminanceGrf aLuminance(WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFLUMINANCE, SdrGrafLuminanceItem));
                pGrfNd->SetAttr( aLuminance );
            }
            
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFGAMMA, SdrGrafGamma100Item))
            {
                double fVal = WW8ITEMVALUE(rOldSet, SDRATTR_GRAFGAMMA,
                    SdrGrafGamma100Item);
                pGrfNd->SetAttr(SwGammaGrf(fVal/100.));
            }

            
            if (WW8ITEMVALUE(rOldSet, SDRATTR_GRAFMODE, SdrGrafModeItem))
            {
                SwDrawModeGrf aDrawMode( static_cast< sal_uInt16 >(WW8ITEMVALUE(rOldSet,
                    SDRATTR_GRAFMODE, SdrGrafModeItem)) );
                pGrfNd->SetAttr( aDrawMode );
            }
        }
    }
}

SdrObject* SwWW8ImplReader::CreateContactObject(SwFrmFmt* pFlyFmt)
{
    if (pFlyFmt)
    {
        SdrObject* pNewObject = mbNewDoc ? 0 : pFlyFmt->FindRealSdrObject();
        if (!pNewObject)
            pNewObject = pFlyFmt->FindSdrObject();
        if (!pNewObject && pFlyFmt->ISA(SwFlyFrmFmt))
        {
            SwFlyDrawContact* pContactObject
                = new SwFlyDrawContact(static_cast<SwFlyFrmFmt*>(pFlyFmt),
                pDrawModel);
            pNewObject = pContactObject->GetMaster();
        }
        return pNewObject;
    }
    return 0;
}


bool SwWW8ImplReader::MiserableRTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel)
{
    if (!IsRightToLeft())
        return false;
    return RTLGraphicsHack(rLeft, nWidth, eHoriOri, eHoriRel,
            maSectionManager.GetPageLeft(),
            maSectionManager.GetPageRight(),
            maSectionManager.GetPageWidth());
}

RndStdIds SwWW8ImplReader::ProcessEscherAlign(SvxMSDffImportRec* pRecord,
    WW8_FSPA *pFSPA, SfxItemSet &rFlySet, bool /*bOrgObjectWasReplace*/)
{
    OSL_ENSURE(pRecord || pFSPA, "give me something! to work with for anchoring");
    if (!pRecord && !pFSPA)
        return FLY_AT_PAGE;
    sal_Bool bCurSectionVertical = maSectionManager.CurrentSectionIsVertical();

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

    
    

    
    

    const sal_uInt32 nCntXAlign = 6;
    const sal_uInt32 nCntYAlign = 6;

    const sal_uInt32 nCntRelTo  = 4;

    sal_uInt32 nXAlign = nCntXAlign > pRecord->nXAlign ? pRecord->nXAlign : 1;
    sal_uInt32 nYAlign = nCntYAlign > pRecord->nYAlign ? pRecord->nYAlign : 1;

    if (pFSPA)
    {
        /*
        #i15718# #i19008#
        Strangely in this case the FSPA value seems to be considered before
        the newer escher nXRelTo record.
        */
        
        
        
        
        

        if ( *(pRecord->pXRelTo) == 2 && *(pRecord->pYRelTo) == 2 && !bCurSectionVertical)
        {
            
            if ( pFSPA->nby != *(pRecord->pYRelTo) )
            {
                *(pRecord->pYRelTo) = pFSPA->nby;
            }
        }
    }

    sal_uInt32 nXRelTo = nCntRelTo > *(pRecord->pXRelTo) ? *(pRecord->pXRelTo) : 1;
    sal_uInt32 nYRelTo = nCntRelTo > *(pRecord->pYRelTo) ? *(pRecord->pYRelTo) : 1;

    RndStdIds eAnchor = IsInlineEscherHack() ? FLY_AS_CHAR : FLY_AT_CHAR; 

    SwFmtAnchor aAnchor( eAnchor );
    aAnchor.SetAnchor( pPaM->GetPoint() );
    rFlySet.Put( aAnchor );

    if (pFSPA)
    {
        
        
        

        
        static const sal_Int16 aHoriOriTab[ nCntXAlign ] =
        {
            text::HoriOrientation::NONE,     
            text::HoriOrientation::LEFT,     
            text::HoriOrientation::CENTER,   
            text::HoriOrientation::RIGHT,    
            
            
            text::HoriOrientation::LEFT,   
            text::HoriOrientation::RIGHT   
        };


        
        static const sal_Int16 aVertOriTab[ nCntYAlign ] =
        {
            text::VertOrientation::NONE,         
            text::VertOrientation::TOP,          
            text::VertOrientation::CENTER,       
            text::VertOrientation::BOTTOM,       
            text::VertOrientation::LINE_TOP,     
            text::VertOrientation::LINE_BOTTOM   
        };

        
        static const sal_Int16 aToLineVertOriTab[ nCntYAlign ] =
        {
            text::VertOrientation::NONE,         
            text::VertOrientation::LINE_BOTTOM,  
            text::VertOrientation::LINE_CENTER,  
            text::VertOrientation::LINE_TOP,     
            text::VertOrientation::LINE_BOTTOM,  
            text::VertOrientation::LINE_TOP      
        };

        
        static const sal_Int16 aHoriRelOriTab[nCntRelTo] =
        {
            text::RelOrientation::PAGE_PRINT_AREA,    
            text::RelOrientation::PAGE_FRAME,  
            text::RelOrientation::FRAME,         
            text::RelOrientation::CHAR       
        };

        
        
        static const sal_Int16 aVertRelOriTab[nCntRelTo] =
        {
            text::RelOrientation::PAGE_PRINT_AREA, 
            text::RelOrientation::PAGE_FRAME,   
            text::RelOrientation::FRAME,          
            text::RelOrientation::TEXT_LINE   
        };

        sal_Int16 eHoriOri = aHoriOriTab[ nXAlign ];
        sal_Int16 eHoriRel = aHoriRelOriTab[  nXRelTo ];

        
        if ( eHoriOri == text::HoriOrientation::LEFT && eHoriRel == text::RelOrientation::PAGE_FRAME )
        {
            
            eHoriOri = text::HoriOrientation::NONE;
            eHoriRel = text::RelOrientation::PAGE_PRINT_AREA;
            const long nWidth = pFSPA->nXaRight - pFSPA->nXaLeft;
            pFSPA->nXaLeft = -nWidth;
            pFSPA->nXaRight = 0;
        }
        else if ( eHoriOri == text::HoriOrientation::RIGHT && eHoriRel == text::RelOrientation::PAGE_FRAME )
        {
            
            eHoriOri = text::HoriOrientation::NONE;
            eHoriRel = text::RelOrientation::PAGE_RIGHT;
            const long nWidth = pFSPA->nXaRight - pFSPA->nXaLeft;
            pFSPA->nXaLeft = 0;
            pFSPA->nXaRight = nWidth;
        }

        
        
        
        {
            
            SwTwips nWidth = (pFSPA->nXaRight - pFSPA->nXaLeft);
            SwTwips nLeft = pFSPA->nXaLeft;
            if (MiserableRTLGraphicsHack(nLeft, nWidth, eHoriOri,
                eHoriRel))
            {
                pFSPA->nXaLeft = nLeft;
                pFSPA->nXaRight = pFSPA->nXaLeft + nWidth;
            }
        }

        
        
        
        
        if ( nInTable &&
             ( eHoriRel == text::RelOrientation::FRAME || eHoriRel == text::RelOrientation::CHAR ) &&
             pFSPA->nwr == 3 &&
             !IsObjectLayoutInTableCell( pRecord->nLayoutInTableCell ) )
        {
            eHoriRel = text::RelOrientation::PAGE_PRINT_AREA;
        }


        
        
        if (eHoriOri == text::HoriOrientation::LEFT)
            pRecord->nDxWrapDistLeft=0;
        else if (eHoriOri == text::HoriOrientation::RIGHT)
            pRecord->nDxWrapDistRight=0;

        sal_Int16 eVertRel;

        eVertRel = aVertRelOriTab[  nYRelTo ]; 
        if ( bCurSectionVertical && nYRelTo == 2 )
            eVertRel = text::RelOrientation::PAGE_PRINT_AREA;
        
        sal_Int16 eVertOri;
        if ( eVertRel == text::RelOrientation::TEXT_LINE )
        {
            eVertOri = aToLineVertOriTab[ nYAlign ];
        }
        else
        {
            eVertOri = aVertOriTab[ nYAlign ];
        }

        
        
        long nYPos = pFSPA->nYaTop;
        
        if ((eVertRel == text::RelOrientation::TEXT_LINE) && (eVertOri == text::VertOrientation::NONE))
            nYPos = -nYPos;

        SwFmtHoriOrient aHoriOri(MakeSafePositioningValue(  bCurSectionVertical ? nYPos : pFSPA->nXaLeft ),
                                                            bCurSectionVertical ? eVertOri : eHoriOri,
                                                            bCurSectionVertical ? eVertRel : eHoriRel);
        if( 4 <= nXAlign )
            aHoriOri.SetPosToggle(true);
        rFlySet.Put( aHoriOri );

        rFlySet.Put(SwFmtVertOrient(MakeSafePositioningValue( !bCurSectionVertical ? nYPos : -pFSPA->nXaRight ),
                                                                !bCurSectionVertical ? eVertOri : eHoriOri,
                                                                !bCurSectionVertical ? eVertRel : eHoriRel ));
    }

    return eAnchor;
}


bool SwWW8ImplReader::IsObjectLayoutInTableCell( const sal_uInt32 nLayoutInTableCell ) const
{
    bool bIsObjectLayoutInTableCell = false;

    if ( bVer8 )
    {
        const sal_uInt16 nWWVersion = pWwFib->nProduct & 0xE000;
        switch ( nWWVersion )
        {
            case 0x0000: 
            {
                bIsObjectLayoutInTableCell = false;
                OSL_ENSURE( nLayoutInTableCell == 0xFFFFFFFF,
                        "no explicit object attribute layout in table cell expected." );
            }
            break;
            case 0x2000: 
            case 0x4000: 
            case 0x6000: 
            case 0x8000: 
            case 0xC000: 
            {
                
                
                
                if ( nLayoutInTableCell == 0xFFFFFFFF || 
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

SwFrmFmt* SwWW8ImplReader::Read_GrafLayer( long nGrafAnchorCp )
{
    if( nIniFlags & WW8FL_NO_GRAFLAYER )
        return 0;

    ::SetProgressState(nProgress, mpDocShell);     

    nDrawCpO = pWwFib->GetBaseCp(pPlcxMan->GetManType() == MAN_HDFT ? MAN_TXBX_HDFT : MAN_TXBX);

    GrafikCtor();

    WW8PLCFspecial* pPF = pPlcxMan->GetFdoa();
    if( !pPF )
    {
        OSL_ENSURE( !this, "Where is the grapic (1) ?" );
        return 0;
    }

    if( bVer67 )
    {
        long nOldPos = pStrm->Tell();

        nDrawXOfs = nDrawYOfs = 0;
        ReadGrafLayer1( pPF, nGrafAnchorCp );

        pStrm->Seek( nOldPos );
        return 0;
    }

    
    pPF->SeekPos( nGrafAnchorCp );

    WW8_FC nStartFc;
    void* pF0;
    if( !pPF->Get( nStartFc, pF0 ) ){
        OSL_ENSURE( !this, "+Wo ist die Grafik (2) ?" );
        return 0;
    }

    WW8_FSPA_SHADOW* pFS = (WW8_FSPA_SHADOW*)pF0;
    WW8_FSPA*        pF;
    WW8_FSPA aFSFA;
    pF = &aFSFA;
    WW8FSPAShadowToReal( pFS, pF );
    if( !pF->nSpId )
    {
        OSL_ENSURE( !this, "+Wo ist die Grafik (3) ?" );
        return 0;
    }

    if (!pMSDffManager->GetModel())
         pMSDffManager->SetModel(pDrawModel, 1440);


    Rectangle aRect(pF->nXaLeft,  pF->nYaTop, pF->nXaRight, pF->nYaBottom);
    SvxMSDffImportData aData( aRect );

    /*
    #i20540#
    The SdrOle2Obj will try and manage any ole objects it finds, causing all
    sorts of trouble later on
    */
    SwDocShell* pPersist = rDoc.GetDocShell();
    rDoc.SetDocShell(0);         

    SdrObject* pObject = 0;
    bool bOk = (pMSDffManager->GetShape(pF->nSpId, pObject, aData) && pObject);

    rDoc.SetDocShell(pPersist);  

    if (!bOk)
    {
        OSL_ENSURE( !this, "Where is the Shape ?" );
        return 0;
    }

    bool bDone = false;
    SdrObject* pOurNewObject = 0;
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

    

    
    SfxItemSet aFlySet(rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
    SwSurround eSurround = SURROUND_PARALLEL;
    bool bContour = false;
    switch (pF->nwr)
    {
        case 0: 
        case 2: 
            eSurround = SURROUND_PARALLEL;
            break;
        case 1: 
            eSurround = SURROUND_NONE;
            break;
        case 3: 
            eSurround = SURROUND_THROUGHT;
            break;
        case 4: 
        case 5: 
            eSurround = SURROUND_PARALLEL;
            bContour = true;
            break;
    }

    
    if ( (2 == pF->nwr) || (4 == pF->nwr) )
    {
        switch( pF->nwrk )
        {
            
            case 0:
                eSurround = SURROUND_PARALLEL;
                break;
            
            case 1:
                eSurround = SURROUND_LEFT;
                break;
            
            case 2:
                eSurround = SURROUND_RIGHT;
                break;
            
            case 3:
                eSurround = SURROUND_IDEAL;
                break;
        }
    }

    SwFmtSurround aSur( eSurround );
    aSur.SetContour( bContour );
    aSur.SetOutside(true); 
    aFlySet.Put( aSur );

    
    

    OSL_ENSURE(!((aData.size() != 1) && bReplaceable),
        "Replaceable drawing with > 1 entries ?");

    if (aData.size() != 1)
        bReplaceable = false;

    SvxMSDffImportRec* pRecord = 0;
    /*
        Get the record for top level object, so we can get the word anchoring
        and wrapping information for it.
    */
    for (MSDffImportRecords::const_iterator it = aData.begin();
            it != aData.end(); ++it) 
    {
        if (it->pObj == pObject)
        {
            pRecord = &const_cast<SvxMSDffImportRec&>(*it);
            break;
        }
    }

    OSL_ENSURE(pRecord, "how did that happen?");
    if (!pRecord)
        return 0;

    const bool bLayoutInTableCell =
        nInTable && IsObjectLayoutInTableCell( pRecord->nLayoutInTableCell );

    
    
    if (bLayoutInTableCell && eSurround != SURROUND_THROUGHT)
    {
        SwFmtFollowTextFlow aFollowTextFlow( sal_True );
        aFlySet.Put( aFollowTextFlow );
    }


    
    
    if (pRecord->bHidden)
        return 0;

    sal_uInt16 nCount = pObject->GetUserDataCount();
    if(nCount)
    {
        OUString lnName, aObjName, aTarFrm;
        for (sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObject->GetUserData( i );
            if( pData && pData->GetInventor() == SW_DRAWLAYER
                    && pData->GetId() == SW_UD_IMAPDATA)
            {
                SwMacroInfo* macInf = dynamic_cast<SwMacroInfo*>(pData);
                if( macInf )
                {
                    sal_Int32 nShapeId = macInf->GetShapeId();
                    if ( nShapeId ==  pF->nSpId )
                    {
                        lnName = macInf->GetHlink();
                        aObjName = macInf->GetName();
                        aTarFrm = macInf->GetTarFrm();
                        break;
                    }
                }
            }
        }
        SwFmtURL* pFmtURL = new SwFmtURL();
        pFmtURL->SetURL( lnName, false );
        if (!aObjName.isEmpty())
            pFmtURL->SetName(aObjName);
        if (!aTarFrm.isEmpty())
            pFmtURL->SetTargetFrameName(aTarFrm);
        pFmtURL->SetMap(0);
        aFlySet.Put(*pFmtURL);
    }

    
    
    

    
    
    const bool bMoveToBackgrd = pRecord->bDrawHell ||
                                ( ( bIsHeader || bIsFooter ) && pF->nwr == 3 );
    if ( bMoveToBackgrd )
        aFlySet.Put(SvxOpaqueItem(RES_OPAQUE,false));

    OUString aObjName = pObject->GetName();

    SwFrmFmt* pRetFrmFmt = 0;
    if (bReplaceable)
    {
        
        pRetFrmFmt = ImportReplaceableDrawables(pObject, pOurNewObject, pRecord,
            pF, aFlySet);
    }
    else
    {
        
        if (pF->bRcaSimple)
        {
            pF->nbx = WW8_FSPA::RelPageBorder;
            pF->nby = WW8_FSPA::RelPageBorder;
        }

        RndStdIds eAnchor = ProcessEscherAlign(pRecord, pF, aFlySet,
            bReplaceable);

        
        if ((!(nIniFlags1 & WW8FL_NO_FLY_FOR_TXBX)) && pRecord->bReplaceByFly)
        {
            pRetFrmFmt = ConvertDrawTextToFly(pObject, pOurNewObject, pRecord,
                eAnchor, pF, aFlySet);
            if (pRetFrmFmt)
                bDone = true;
        }

        if (!bDone)
        {
            sw::util::SetLayer aSetLayer(rDoc);
            if ( bMoveToBackgrd )
                aSetLayer.SendObjectToHell(*pObject);
            else
                aSetLayer.SendObjectToHeaven(*pObject);

            if (!IsInlineEscherHack())
            {
                /* Need to make sure that the correct layer ordering is applied. */
                
                pWWZOrder->InsertEscherObject( pObject, pF->nSpId,
                                               bIsHeader || bIsFooter );
            }
            else
            {
                pWWZOrder->InsertTextLayerObject(pObject);
            }

            pRetFrmFmt = rDoc.InsertDrawObj(*pPaM, *pObject, aFlySet );

            OSL_ENSURE(pRetFrmFmt->GetAnchor().GetAnchorId() ==
                eAnchor, "Not the anchor type requested!");

            /*
                Insert text if necessary into textboxes contained in groups.
            */
            if (!aData.empty())
            {
                for (MSDffImportRecords::const_iterator it = aData.begin();
                        it != aData.end(); ++it)
                {
                    pRecord = &const_cast<SvxMSDffImportRec&>(*it);
                    if (pRecord->pObj && pRecord->aTextId.nTxBxS)
                    { 
                        pRetFrmFmt = MungeTextIntoDrawBox(pRecord->pObj,
                            pRecord, nGrafAnchorCp, pRetFrmFmt);
                    }
                }
            }
        }
    }

    
    if ( pRetFrmFmt /*#i52825# */ && pRetFrmFmt->ISA(SwDrawFrmFmt) )
    {
        static_cast<SwDrawFrmFmt*>(pRetFrmFmt)->PosAttrSet();
    }
    if (!IsInlineEscherHack())
        MapWrapIntoFlyFmt(pRecord, pRetFrmFmt);

    
    if( pRetFrmFmt /*#i52825# */ && !aObjName.isEmpty() )
        pRetFrmFmt->SetName( aObjName );
    return AddAutoAnchor(pRetFrmFmt);
}

SwFrmFmt *SwWW8ImplReader::AddAutoAnchor(SwFrmFmt *pFmt)
{
    /*
     * anchored to character at the current position will move along the
     * paragraph as text is added because we are at the insertion point.
     *
     * Leave to later and set the correct location then.
     */
    if ((pFmt) && (pFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR))
    {
        pAnchorStck->AddAnchor(*pPaM->GetPoint(), pFmt);
    }
    return pFmt;
}

SwFrmFmt* SwWW8ImplReader::MungeTextIntoDrawBox(SdrObject* pTrueObject,
    SvxMSDffImportRec *pRecord, long nGrafAnchorCp, SwFrmFmt* pRetFrmFmt)
{
    SdrTextObj* pSdrTextObj;

    
    if (SdrObjGroup* pThisGroup = PTR_CAST(SdrObjGroup, pRecord->pObj))
    {
        
        
        pSdrTextObj = new SdrRectObj( OBJ_TEXT, pThisGroup->GetCurrentBoundRect());

        SfxItemSet aSet(pDrawModel->GetItemPool());
        aSet.Put(XFillStyleItem(XFILL_NONE));
        aSet.Put(XLineStyleItem(XLINE_NONE));
        aSet.Put(SdrTextFitToSizeTypeItem( SDRTEXTFIT_NONE ));
        aSet.Put(SdrTextAutoGrowHeightItem(false));
        aSet.Put(SdrTextAutoGrowWidthItem(false));
        pSdrTextObj->SetMergedItemSet(aSet);

        long nAngle = pRecord->nTextRotationAngle;
        if ( nAngle )
        {
            double a = nAngle*nPi180;
            pSdrTextObj->NbcRotate(pSdrTextObj->GetCurrentBoundRect().Center(), nAngle,
                sin(a), cos(a) );
        }

        pSdrTextObj->NbcSetLayer( pThisGroup->GetLayer() );
        pThisGroup->GetSubList()->NbcInsertObject(pSdrTextObj);
    }
    else
        pSdrTextObj = PTR_CAST(SdrTextObj, pRecord->pObj);

    if( pSdrTextObj )
    {
        Size aObjSize(pSdrTextObj->GetSnapRect().GetWidth(),
            pSdrTextObj->GetSnapRect().GetHeight());

        
        SdrObject* pGroupObject = pSdrTextObj->GetUpGroup();

        sal_uInt32 nOrdNum = pSdrTextObj->GetOrdNum();
        bool bEraseThisObject;
        InsertTxbxText( pSdrTextObj, &aObjSize, pRecord->aTextId.nTxBxS,
            pRecord->aTextId.nSequence, nGrafAnchorCp, pRetFrmFmt,
            (pSdrTextObj != pTrueObject) || (0 != pGroupObject),
            bEraseThisObject, 0, 0, 0, 0, pRecord);

        
        if (bEraseThisObject)
        {
            if( pGroupObject || (pSdrTextObj != pTrueObject) )
            {
                
                

                SdrObject* pNewObj = pGroupObject ?
                    pGroupObject->GetSubList()->GetObj(nOrdNum) : pTrueObject;
                if (pSdrTextObj != pNewObj)
                {
                    
                    pMSDffManager->ExchangeInShapeOrder(pSdrTextObj, 0,0, pNewObj);
                    
                    SdrObject::Free( pRecord->pObj );
                    
                    pRecord->pObj = pNewObj;
                }
            }
            else
            {
                
                pMSDffManager->RemoveFromShapeOrder( pSdrTextObj );
                
                if( pSdrTextObj->GetPage() )
                    pDrawPg->RemoveObject( pSdrTextObj->GetOrdNum() );
                
                
                rDoc.DelFrmFmt( pRetFrmFmt );
                pRetFrmFmt = 0;
                
                pRecord->pObj = 0;
            }
        }
        else
        {
            
            SfxItemSet aItemSet(pDrawModel->GetItemPool(),
                SDRATTR_TEXT_LEFTDIST, SDRATTR_TEXT_LOWERDIST);
            aItemSet.Put( SdrTextLeftDistItem( pRecord->nDxTextLeft ) );
            aItemSet.Put( SdrTextRightDistItem( pRecord->nDxTextRight  ) );
            aItemSet.Put( SdrTextUpperDistItem( pRecord->nDyTextTop    ) );
            aItemSet.Put( SdrTextLowerDistItem( pRecord->nDyTextBottom ) );
            pSdrTextObj->SetMergedItemSetAndBroadcast(aItemSet);
        }
    }
    return pRetFrmFmt;
}

SwFlyFrmFmt* SwWW8ImplReader::ConvertDrawTextToFly(SdrObject* &rpObject,
    SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, RndStdIds eAnchor,
    WW8_FSPA *pF, SfxItemSet &rFlySet)
{
    SwFlyFrmFmt* pRetFrmFmt = 0;
    long nStartCp;
    long nEndCp;

    
    
    if ( TxbxChainContainsRealText(pRecord->aTextId.nTxBxS,nStartCp,nEndCp) )
    {
        
        
        // 
        
        
        Rectangle aInnerDist(pRecord->nDxTextLeft, pRecord->nDyTextTop,
            pRecord->nDxTextRight, pRecord->nDyTextBottom);

        SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, pF->nXaRight - pF->nXaLeft, pF->nYaBottom - pF->nYaTop);
        aFrmSize.SetWidthSizeType(pRecord->bAutoWidth ? ATT_VAR_SIZE : ATT_FIX_SIZE);
        rFlySet.Put(aFrmSize);

        MatchSdrItemsIntoFlySet( rpObject, rFlySet, pRecord->eLineStyle,
            pRecord->eLineDashing, pRecord->eShapeType, aInnerDist );


        SdrTextObj *pSdrTextObj = PTR_CAST(SdrTextObj, rpObject);
        if (pSdrTextObj && pSdrTextObj->IsVerticalWriting())
            rFlySet.Put(SvxFrameDirectionItem(FRMDIR_VERT_TOP_RIGHT, RES_FRAMEDIR));

        pRetFrmFmt = rDoc.MakeFlySection(eAnchor, pPaM->GetPoint(), &rFlySet);
        OSL_ENSURE(pRetFrmFmt->GetAnchor().GetAnchorId() == eAnchor,
            "Not the anchor type requested!");

        
        
        rpOurNewObject = CreateContactObject(pRetFrmFmt);

        
        pMSDffManager->RemoveFromShapeOrder( rpObject );

        
        SdrObject::Free( rpObject );
        /*
            NB: only query pOrgShapeObject starting here!
        */

        if (rpOurNewObject)
        {
            /*
            We do not store our rpOutNewObject in the ShapeOrder because we
            have a FrmFmt from which we can regenerate the contact object when
            we need it. Because, we can have frames anchored to paragraphs in
            header/footers and we can copy header/footers, if we do copy a
            header/footer with a nonpage anchored frame in it then the contact
            objects are invalidated. Under this condition the FrmFmt will be
            updated to reflect this change and can be used to get a new
            contact object, while a raw rpOutNewObject stored here becomes
            deleted and useless.
            */
            pMSDffManager->StoreShapeOrder(pF->nSpId,
                (((sal_uLong)pRecord->aTextId.nTxBxS) << 16) +
                pRecord->aTextId.nSequence, 0, pRetFrmFmt);

            
            
            if (!rpOurNewObject->IsInserted())
            {
                
                pWWZOrder->InsertEscherObject( rpOurNewObject, pF->nSpId,
                                               bIsHeader || bIsFooter );
            }
        }

        
        if( !pRecord->aTextId.nSequence )
        {
            
            WW8ReaderSave aSave( this );

            MoveInsideFly(pRetFrmFmt);

            SwNodeIndex aStart(pPaM->GetPoint()->nNode);

            pWWZOrder->InsideEscher(pF->nSpId);

            
            bTxbxFlySection = true;
            bool bJoined = ReadText(nStartCp, (nEndCp-nStartCp),
                MAN_MAINTEXT == pPlcxMan->GetManType() ?
                        MAN_TXBX : MAN_TXBX_HDFT);

            pWWZOrder->OutsideEscher();

            MoveOutsideFly(pRetFrmFmt, aSave.GetStartPos(),!bJoined);

            aSave.Restore( this );
        }
    }
    return pRetFrmFmt;
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

SwFlyFrmFmt* SwWW8ImplReader::ImportReplaceableDrawables( SdrObject* &rpObject,
    SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, WW8_FSPA *pF,
    SfxItemSet &rFlySet )
{
    SwFlyFrmFmt* pRetFrmFmt = 0;
    long nWidthTw  = pF->nXaRight - pF->nXaLeft;
    if (0 > nWidthTw)
        nWidthTw = 0;
    long nHeightTw = pF->nYaBottom - pF->nYaTop;
    if (0 > nHeightTw)
        nHeightTw = 0;

    ProcessEscherAlign(pRecord, pF, rFlySet, true);

    rFlySet.Put(SwFmtFrmSize(ATT_FIX_SIZE, nWidthTw, nHeightTw));

    SfxItemSet aGrSet(rDoc.GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);

    if (pRecord)
    {
        
        
        Rectangle aInnerDist(0, 0, 0, 0);

        MatchSdrItemsIntoFlySet(rpObject, rFlySet, pRecord->eLineStyle,
            pRecord->eLineDashing, pRecord->eShapeType, aInnerDist);

        MatchEscherMirrorIntoFlySet(*pRecord, aGrSet);
    }

    OUString aObjectName(rpObject->GetName());
    if (OBJ_OLE2 == SdrObjKind(rpObject->GetObjIdentifier()))
        pRetFrmFmt = InsertOle(*((SdrOle2Obj*)rpObject), rFlySet, aGrSet);
    else
    {
        const SdrGrafObj *pGrf= (const SdrGrafObj*)rpObject;
        bool bDone = false;
        if (pGrf->IsLinkedGraphic() && !pGrf->GetFileName().isEmpty())
        {
            GraphicType eType = pGrf->GetGraphicType();
            OUString aGrfName(
                URIHelper::SmartRel2Abs(
                    INetURLObject(sBaseURL), pGrf->GetFileName(),
                    URIHelper::GetMaybeFileHdl()));
            
            
            
            if (GRAPHIC_NONE == eType || CanUseRemoteLink(aGrfName))
            {
                pRetFrmFmt = rDoc.Insert(*pPaM, aGrfName, OUString(), 0,
                    &rFlySet, &aGrSet, NULL);
                bDone = true;
            }
        }
        if (!bDone)
        {
            const Graphic& rGraph = pGrf->GetGraphic();
            pRetFrmFmt = rDoc.Insert(*pPaM, OUString(), OUString(), &rGraph,
                &rFlySet, &aGrSet, NULL);
        }
    }

    if (pRetFrmFmt)
    {
        if( pRecord )
        {
            if( OBJ_OLE2 != SdrObjKind(rpObject->GetObjIdentifier()) )
                SetAttributesAtGrfNode( pRecord, pRetFrmFmt, pF );
        }
        
        maGrfNameGenerator.SetUniqueGraphName(pRetFrmFmt, aObjectName);
    }
    
    
    rpOurNewObject = CreateContactObject(pRetFrmFmt);

    
    pMSDffManager->RemoveFromShapeOrder( rpObject );
    
    if( rpObject->GetPage() )
        pDrawPg->RemoveObject( rpObject->GetOrdNum() );

    
    SdrObject::Free( rpObject );
    /*
        Achtung: ab jetzt nur noch pOrgShapeObject abfragen!
    */

    
    if (rpOurNewObject)
    {
        if (!bHdFtFtnEdn)
            pMSDffManager->StoreShapeOrder(pF->nSpId, 0, rpOurNewObject, 0 );

        
        
        if (!rpOurNewObject->IsInserted())
        {
            
            pWWZOrder->InsertEscherObject( rpOurNewObject, pF->nSpId,
                                           bIsHeader || bIsFooter );
        }
    }
    return pRetFrmFmt;
}

void SwWW8ImplReader::GrafikCtor()  
{
    if (!pDrawModel)
    {
        rDoc.GetOrCreateDrawModel(); 
        pDrawModel  = rDoc.GetDrawModel();
        OSL_ENSURE(pDrawModel, "Kann DrawModel nicht anlegen");
        pDrawPg = pDrawModel->GetPage(0);

        pMSDffManager = new SwMSDffManager(*this);
        pMSDffManager->SetModel(pDrawModel, 1440);
        /*
         Now the dff manager always needs a controls 
         control converter may still exist 
        */
        pFormImpl = new SwMSConvertControls(mpDocShell, pPaM);

        pWWZOrder = new wwZOrderer(sw::util::SetLayer(rDoc), pDrawPg,
            pMSDffManager->GetShapeOrders());
    }
}

void SwWW8ImplReader::GrafikDtor()
{
    DELETEZ(mpDrawEditEngine); 
    DELETEZ(pWWZOrder);       
}

void SwWW8FltAnchorStack::AddAnchor(const SwPosition& rPos, SwFrmFmt *pFmt)
{
    OSL_ENSURE(pFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR,
        "Don't use fltanchors with inline frames, slap!");
    NewAttr(rPos, SwFltAnchor(pFmt));
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
