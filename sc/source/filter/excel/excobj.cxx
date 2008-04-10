/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: excobj.cxx,v $
 * $Revision: 1.46 $
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
#include "precompiled_sc.hxx"

#include "scitems.hxx"
#include <svx/eeitem.hxx>

#include <svx/colritem.hxx>
#include <svx/svddef.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/xcolit.hxx>
#include <svx/xfillit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xsetit.hxx>
#include <svx/outlobj.hxx>
#include <svx/outliner.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/app.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <sot/clsids.hxx>
#include <svx/svdograf.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "editutil.hxx"
#include "xltracer.hxx"
#include "xilink.hxx"
#include "xiescher.hxx"

#include "imp_op.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

void ImportExcel::Obj()
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();

    UINT32 nObj;
    UINT16 nType, nId, nFlags;
    UINT16 nMacroLen;
    XclEscherAnchor aAnchor( nScTab );

    aIn >> nObj >> nType >> nId >> nFlags >> aAnchor >> nMacroLen;
    bool bBiff5 = GetBiff() == EXC_BIFF5;
    aIn.Ignore( bBiff5 ? 6 : 2 );

    if( bBiff5 && (nType == EXC_OBJ_CMO_CHART) )
    {
        maStrm.Seek( EXC_REC_SEEK_TO_BEGIN );
        GetObjectManager().ReadObj5( maStrm );
    }
    else
    {
        Rectangle aRect( aAnchor.GetRect( rDoc, MAP_100TH_MM ) );
        Point aTL( aRect.TopLeft() );
        Point aBR( aRect.BottomRight() );

        SfxItemSet aSet( rDoc.GetDrawLayer()->GetItemPool(), SDRATTR_START, SDRATTR_END );
        SdrObject* pObj = NULL;
        switch( nType )
        {
            case EXC_OBJ_CMO_LINE:      pObj = LineObj( aSet, aTL, aBR );   break;
            case EXC_OBJ_CMO_RECTANGLE: pObj = RectObj( aSet, aTL, aBR );   break;
            default: pExcRoot->pIR->GetTracer().TraceUnsupportedObjects();  break;
        }

        if( pObj )
        {
            pObj->NbcSetSnapRect( aRect );
            pObj->NbcSetLayer( SC_LAYER_FRONT );

            pObj->ClearMergedItem();
            pObj->SetMergedItemSetAndBroadcast( aSet );

            if( SdrPage* pSdrPage = GetSdrPage( nScTab ) )
                pSdrPage->InsertObject( pObj );
            if( bBiff5 && aIn.GetRecLeft() )
            {
                sal_uInt8 nNameLen;
                aIn >> nNameLen;
                aIn.Ignore( nNameLen + nMacroLen );
            }
        }
    }
}


void ImportExcel::SetLineStyle( SfxItemSet& rSet, sal_uInt16 nColor, sal_uInt16 nStyle, sal_uInt16 nWidth )
{
    rSet.Put( XLineColorItem( String(), GetPalette().GetColor( nColor ) ) );
    XLineStyle eStyle = (nStyle == 255) ? XLINE_NONE : (((nStyle == 1) || (nStyle == 2)) ? XLINE_DASH : XLINE_SOLID);
    rSet.Put( XLineStyleItem( eStyle ) );
    rSet.Put( XLineWidthItem( nWidth * 40 ) );
}


void ImportExcel::SetFillStyle( SfxItemSet& rSet, sal_uInt16 nBackIdx, sal_uInt16 nPattIdx, sal_uInt16 nPattern )
{
    if( nPattern == EXC_PATT_NONE )
    {
        rSet.Put( XFillStyleItem( XFILL_NONE ) );
    }
    else
    {
        rSet.Put( XFillStyleItem( XFILL_SOLID ) );
        Color aPattColor = GetPalette().GetColor( nPattIdx );
        Color aBackColor = GetPalette().GetColor( nBackIdx );
        Color aColor = XclTools::GetPatternColor( aPattColor, aBackColor, nPattern );
        rSet.Put( XFillColorItem( String(), aColor ) );
    }
}


SdrObject* ImportExcel::LineObj( SfxItemSet& rSet, const Point& rUL, const Point& rLR )
{
    BYTE        nLc, nStyle, nWeight, nAuto;
    UINT16      nEndStyles;
    BYTE        nQuadrant;
    aIn >> nLc >> nStyle >> nWeight >> nAuto >> nEndStyles >> nQuadrant;
    aIn.Ignore( 1 );

    basegfx::B2DPolygon aB2DPolygon;
    aB2DPolygon.append(basegfx::B2DPoint(rUL.X(), rUL.Y()));
    aB2DPolygon.append(basegfx::B2DPoint(rLR.X(), rLR.Y()));
    SdrPathObj* pObj = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aB2DPolygon));
    SetLineStyle( rSet, nLc, nStyle, nWeight );
    return pObj;
}


SdrObject* ImportExcel::RectObj( SfxItemSet& rSet, const Point& rUL, const Point& rLR )
{
    BYTE        nBg, nFg, nPat, fAuto1, nLc, nLStyle, nLWeight, fAuto2;
    UINT16      nFRS;
    aIn >> nBg >> nFg >> nPat >> fAuto1 >> nLc >> nLStyle
        >> nLWeight >> fAuto2 >> nFRS;

    SdrRectObj* pObj = new SdrRectObj( Rectangle( rUL, rLR ) );
    SetLineStyle( rSet, nLc, nLStyle, nLWeight );
    SetFillStyle( rSet, nBg, nFg, nPat );
    return pObj;
}

