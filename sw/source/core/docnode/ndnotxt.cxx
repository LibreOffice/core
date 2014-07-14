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
#include <hints.hxx>            // fuer SwFmtChg
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>

// --> OD 2009-07-13 #i73249#
#include <frmfmt.hxx>
// <--

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
    // soll eine Harte-Attributierung gesetzt werden?
    if( pAutoAttr )
        SetAttr( *pAutoAttr );
}


SwNoTxtNode::~SwNoTxtNode()
{
    delete pContour;
}


// erzeugt fuer alle Ableitungen einen AttrSet mit Bereichen
// fuer Frame- und Grafik-Attributen
void SwNoTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    ASSERT( !mpAttrSet.get(), "AttrSet ist doch gesetzt" );
    SwAttrSet aNewAttrSet( rPool, aNoTxtNodeSetRange );

    // put names of parent style and conditional style:
    const SwFmtColl* pFmtColl = GetFmtColl();
    String sVal;
    SwStyleNameMapper::FillProgName( pFmtColl->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
    SfxStringItem aFmtColl( RES_FRMATR_STYLE_NAME, sVal );
    aNewAttrSet.Put( aFmtColl );

    aNewAttrSet.SetParent( &GetFmtColl()->GetAttrSet() );
    mpAttrSet = GetDoc()->GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_NOTXT );
}

// Dummies fuer das Laden/Speichern von persistenten Daten
// bei Grafiken und OLE-Objekten


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
    ASSERT( !pContour, "Contour available." );
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
        sal_Bool bPixelGrf = aGrfMap.GetMapUnit() == MAP_PIXEL;
        const MapMode aContourMap( bPixelGrf ? MAP_PIXEL : MAP_100TH_MM );
        if( bPixelGrf ? !bPixelContour : aGrfMap != aContourMap )
        {
            // --> OD #i102238#
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
            // <--
            ASSERT( !bPixelGrf || aGrfMap == aContourMap,
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
                        // --> OD #i102238#
                        if ( nGrfDPIx != 0 && nGrfDPIy != 0 )
                        {
                            rPoly[i] = Point( rPoly[i].X() * pOutDev->ImplGetDPIX() / nGrfDPIx,
                                              rPoly[i].Y() * pOutDev->ImplGetDPIY() / nGrfDPIy );
                        }
                        // <--
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
    ASSERT( pContour, "Contour not available." );
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
        ASSERT( aGrfMap.GetMapUnit() != MAP_PIXEL ||
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
        ASSERT( GetOLENode(), "new type of Node?" );
        aRet = *((SwOLENode*)this)->SwOLENode::GetGraphic();
    }
    return aRet;
}

void SwNoTxtNode::SetTitle( const String& rTitle, bool bBroadcast )
{
    // Title attribute of <SdrObject> replaces own AlternateText attribute
    SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(GetFlyFmt());
    ASSERT( pFlyFmt,
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
    ASSERT( pFlyFmt,
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
    ASSERT( pFlyFmt,
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
    ASSERT( pFlyFmt,
            "<SwNoTxtNode::GetDescription(..)> - missing <SwFlyFrmFmt> instance" );
    if ( !pFlyFmt )
    {
        return aEmptyStr;
    }

    return pFlyFmt->GetObjDescription();
}
// <--
