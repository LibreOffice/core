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

#include <hintids.hxx>
#include <tools/poly.hxx>
#include <svl/stritem.hxx>
#include <svx/contdlg.hxx>
#include <vcl/svapp.hxx>
#include <docary.hxx>
#include <doc.hxx>
#include <fmtcol.hxx>
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <ndindex.hxx>
#include <hints.hxx>
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>

#include <frmfmt.hxx> // #i73249#

SwNoTxtNode::SwNoTxtNode( const SwNodeIndex & rWhere,
                  const sal_uInt8 nNdType,
                  SwGrfFmtColl *pGrfColl,
                  SwAttrSet* pAutoAttr ) :
    SwCntntNode( rWhere, nNdType, pGrfColl ),
    pContour( 0 ),
    bAutomaticContour( sal_False ),
    bContourMapModeValid( sal_True ),
    bPixelContour( sal_False )
{
    // Should this set a hard attribute?
    if( pAutoAttr )
        SetAttr( *pAutoAttr );
}

SwNoTxtNode::~SwNoTxtNode()
{
    delete pContour;
}

/// Creates an AttrSet for all derivations with ranges for frame-
/// and graphics-attributes.
void SwNoTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    OSL_ENSURE( !mpAttrSet.get(), "AttrSet is already set" );
    SwAttrSet aNewAttrSet( rPool, aNoTxtNodeSetRange );

    // put names of parent style and conditional style:
    const SwFmtColl* pFmtColl = GetFmtColl();
    OUString sVal;
    SwStyleNameMapper::FillProgName( pFmtColl->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
    SfxStringItem aFmtColl( RES_FRMATR_STYLE_NAME, sVal );
    aNewAttrSet.Put( aFmtColl );

    aNewAttrSet.SetParent( &GetFmtColl()->GetAttrSet() );
    mpAttrSet = GetDoc()->GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_NOTXT );
}

/// Dummies for loading/saving of persistent data
/// when working with graphics and OLE objects
sal_Bool SwNoTxtNode::RestorePersistentData()
{
    return sal_True;
}

sal_Bool SwNoTxtNode::SavePersistentData()
{
    return sal_True;
}

void SwNoTxtNode::SetContour( const PolyPolygon *pPoly, sal_Bool bAutomatic )
{
    delete pContour;
    if ( pPoly )
        pContour = new PolyPolygon( *pPoly );
    else
        pContour = 0;
    bAutomaticContour = bAutomatic;
    bContourMapModeValid = sal_True;
    bPixelContour = sal_False;
}

void SwNoTxtNode::CreateContour()
{
    OSL_ENSURE( !pContour, "Contour available." );
    pContour = new PolyPolygon(SvxContourDlg::CreateAutoContour(GetGraphic()));
    bAutomaticContour = sal_True;
    bContourMapModeValid = sal_True;
    bPixelContour = sal_False;
}

const PolyPolygon *SwNoTxtNode::HasContour() const
{
    if( !bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        bool bPixelGrf = aGrfMap.GetMapUnit() == MAP_PIXEL;
        const MapMode aContourMap( bPixelGrf ? MAP_PIXEL : MAP_100TH_MM );
        if( bPixelGrf ? !bPixelContour : aGrfMap != aContourMap )
        {
            // #i102238#
            double nGrfDPIx = 0.0;
            double nGrfDPIy = 0.0;
            {
                if ( !bPixelGrf && bPixelContour )
                {
                    const Size aGrfPixelSize( GetGraphic().GetSizePixel() );
                    const Size aGrfPrefMapModeSize( GetGraphic().GetPrefSize() );
                    if ( aGrfMap.GetMapUnit() == MAP_INCH )
                    {
                        nGrfDPIx = aGrfPixelSize.Width() / ( (double)aGrfMap.GetScaleX() * aGrfPrefMapModeSize.Width() );
                        nGrfDPIy = aGrfPixelSize.Height() / ( (double)aGrfMap.GetScaleY() * aGrfPrefMapModeSize.Height() );
                    }
                    else
                    {
                        const Size aGrf1000thInchSize =
                            OutputDevice::LogicToLogic( aGrfPrefMapModeSize,
                                                        aGrfMap, MAP_1000TH_INCH );
                        nGrfDPIx = 1000.0 * aGrfPixelSize.Width() / aGrf1000thInchSize.Width();
                        nGrfDPIy = 1000.0 * aGrfPixelSize.Height() / aGrf1000thInchSize.Height();
                    }
                }
            }
            OSL_ENSURE( !bPixelGrf || aGrfMap == aContourMap,
                        "scale factor for pixel unsupported" );
            OutputDevice* pOutDev =
                (bPixelGrf || bPixelContour) ? Application::GetDefaultDevice()
                                             : 0;
            sal_uInt16 nPolyCount = pContour->Count();
            for( sal_uInt16 j=0; j<nPolyCount; j++ )
            {
                Polygon& rPoly = (*pContour)[j];

                sal_uInt16 nCount = rPoly.GetSize();
                for( sal_uInt16 i=0 ; i<nCount; i++ )
                {
                    if( bPixelGrf )
                        rPoly[i] = pOutDev->LogicToPixel( rPoly[i],
                                                          aContourMap );
                    else if( bPixelContour )
                    {
                        rPoly[i] = pOutDev->PixelToLogic( rPoly[i], aGrfMap );
                        // #i102238#
                        if ( nGrfDPIx != 0 && nGrfDPIy != 0 )
                        {
                            rPoly[i] = Point( rPoly[i].getX() * pOutDev->ImplGetDPIX() / nGrfDPIx,
                                              rPoly[i].getY() * pOutDev->ImplGetDPIY() / nGrfDPIy );
                        }
                    }
                    else
                        rPoly[i] = OutputDevice::LogicToLogic( rPoly[i],
                                                                 aContourMap,
                                                                 aGrfMap );
                }
            }
        }
        ((SwNoTxtNode *)this)->bContourMapModeValid = sal_True;
        ((SwNoTxtNode *)this)->bPixelContour = sal_False;
    }

    return pContour;
}

void SwNoTxtNode::GetContour( PolyPolygon &rPoly ) const
{
    OSL_ENSURE( pContour, "Contour not available." );
    rPoly = *HasContour();
}

void SwNoTxtNode::SetContourAPI( const PolyPolygon *pPoly )
{
    delete pContour;
    if ( pPoly )
        pContour = new PolyPolygon( *pPoly );
    else
        pContour = 0;
    bContourMapModeValid = sal_False;
}

sal_Bool SwNoTxtNode::GetContourAPI( PolyPolygon &rContour ) const
{
    if( !pContour )
        return sal_False;

    rContour = *pContour;
    if( bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        const MapMode aContourMap( MAP_100TH_MM );
        OSL_ENSURE( aGrfMap.GetMapUnit() != MAP_PIXEL ||
                aGrfMap == MapMode( MAP_PIXEL ),
                    "scale factor for pixel unsupported" );
        if( aGrfMap.GetMapUnit() != MAP_PIXEL &&
            aGrfMap != aContourMap )
        {
            sal_uInt16 nPolyCount = rContour.Count();
            for( sal_uInt16 j=0; j<nPolyCount; j++ )
            {
                // --> OD #i102238# - use the right <PolyPolygon> instance
                Polygon& rPoly = rContour[j];
                // <--

                sal_uInt16 nCount = rPoly.GetSize();
                for( sal_uInt16 i=0 ; i<nCount; i++ )
                {
                    rPoly[i] = OutputDevice::LogicToLogic( rPoly[i], aGrfMap,
                                                           aContourMap );
                }
            }
        }
    }

    return sal_True;
}

sal_Bool SwNoTxtNode::IsPixelContour() const
{
    sal_Bool bRet;
    if( bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        bRet = aGrfMap.GetMapUnit() == MAP_PIXEL;
    }
    else
    {
        bRet = bPixelContour;
    }

    return bRet;
}

Graphic SwNoTxtNode::GetGraphic() const
{
    Graphic aRet;
    if ( GetGrfNode() )
    {
        ((SwGrfNode*)this)->SwapIn( sal_True );
        aRet = ((SwGrfNode*)this)->GetGrf();
    }
    else
    {
        OSL_ENSURE( GetOLENode(), "new type of Node?" );
        aRet = *((SwOLENode*)this)->SwOLENode::GetGraphic();
    }
    return aRet;
}

// #i73249#
void SwNoTxtNode::SetTitle( const String& rTitle, bool bBroadcast )
{
    // Title attribute of <SdrObject> replaces own AlternateText attribute
    SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(GetFlyFmt());
    OSL_ENSURE( pFlyFmt,
            "<SwNoTxtNode::SetTitle(..)> - missing <SwFlyFrmFmt> instance" );
    if ( !pFlyFmt )
    {
        return;
    }

    pFlyFmt->SetObjTitle( rTitle, bBroadcast );
}

const String SwNoTxtNode::GetTitle() const
{
    const SwFlyFrmFmt* pFlyFmt = dynamic_cast<const SwFlyFrmFmt*>(GetFlyFmt());
    OSL_ENSURE( pFlyFmt,
            "<SwNoTxtNode::GetTitle(..)> - missing <SwFlyFrmFmt> instance" );
    if ( !pFlyFmt )
    {
        return aEmptyStr;
    }

    return pFlyFmt->GetObjTitle();
}

void SwNoTxtNode::SetDescription( const String& rDescription, bool bBroadcast )
{
    SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(GetFlyFmt());
    OSL_ENSURE( pFlyFmt,
            "<SwNoTxtNode::SetDescription(..)> - missing <SwFlyFrmFmt> instance" );
    if ( !pFlyFmt )
    {
        return;
    }

    pFlyFmt->SetObjDescription( rDescription, bBroadcast );
}

const String SwNoTxtNode::GetDescription() const
{
    const SwFlyFrmFmt* pFlyFmt = dynamic_cast<const SwFlyFrmFmt*>(GetFlyFmt());
    OSL_ENSURE( pFlyFmt,
            "<SwNoTxtNode::GetDescription(..)> - missing <SwFlyFrmFmt> instance" );
    if ( !pFlyFmt )
    {
        return aEmptyStr;
    }

    return pFlyFmt->GetObjDescription();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
