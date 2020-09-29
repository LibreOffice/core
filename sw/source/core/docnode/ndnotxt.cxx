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
#include <doc.hxx>
#include <fmtcol.hxx>
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <ndindex.hxx>
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>

#include <frmfmt.hxx>

SwNoTextNode::SwNoTextNode( const SwNodeIndex & rWhere,
                  const SwNodeType nNdType,
                  SwGrfFormatColl *pGrfColl,
                  SwAttrSet const * pAutoAttr ) :
    SwContentNode( rWhere, nNdType, pGrfColl ),
    m_bAutomaticContour( false ),
    m_bContourMapModeValid( true ),
    m_bPixelContour( false )
{
    // Should this set a hard attribute?
    if( pAutoAttr )
        SetAttr( *pAutoAttr );
}

SwNoTextNode::~SwNoTextNode()
{
}

/// Creates an AttrSet for all derivations with ranges for frame-
/// and graphics-attributes.
void SwNoTextNode::NewAttrSet( SwAttrPool& rPool )
{
    OSL_ENSURE( !mpAttrSet, "AttrSet is already set" );
    SwAttrSet aNewAttrSet( rPool, aNoTextNodeSetRange );

    // put names of parent style and conditional style:
    const SwFormatColl* pFormatColl = GetFormatColl();
    OUString sVal;
    SwStyleNameMapper::FillProgName( pFormatColl->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
    SfxStringItem aFormatColl( RES_FRMATR_STYLE_NAME, sVal );
    aNewAttrSet.Put( aFormatColl );

    aNewAttrSet.SetParent( &GetFormatColl()->GetAttrSet() );
    mpAttrSet = GetDoc().GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_NOTXT );
}

/// Dummies for loading/saving of persistent data
/// when working with graphics and OLE objects
bool SwNoTextNode::RestorePersistentData()
{
    return true;
}

bool SwNoTextNode::SavePersistentData()
{
    return true;
}

void SwNoTextNode::SetContour( const tools::PolyPolygon *pPoly, bool bAutomatic )
{
    if ( pPoly )
        m_pContour.reset( new tools::PolyPolygon( *pPoly ) );
    else
        m_pContour.reset();
    m_bAutomaticContour = bAutomatic;
    m_bContourMapModeValid = true;
    m_bPixelContour = false;
}

void SwNoTextNode::CreateContour()
{
    OSL_ENSURE( !m_pContour, "Contour available." );
    m_pContour.reset( new tools::PolyPolygon(SvxContourDlg::CreateAutoContour(GetGraphic())) );
    m_bAutomaticContour = true;
    m_bContourMapModeValid = true;
    m_bPixelContour = false;
}

const tools::PolyPolygon *SwNoTextNode::HasContour() const
{
    if( !m_bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        bool bPixelGrf = aGrfMap.GetMapUnit() == MapUnit::MapPixel;
        const MapMode aContourMap( bPixelGrf ? MapUnit::MapPixel : MapUnit::Map100thMM );
        if( bPixelGrf ? !m_bPixelContour : aGrfMap != aContourMap )
        {
            double nGrfDPIx = 0.0;
            double nGrfDPIy = 0.0;
            {
                if ( !bPixelGrf && m_bPixelContour )
                {
                    basegfx::B2DSize aDPI = GetGraphic().GetPPI();
                    nGrfDPIx = aDPI.getX();
                    nGrfDPIy = aDPI.getY();
                }
            }
            OSL_ENSURE( !bPixelGrf || aGrfMap == aContourMap,
                        "scale factor for pixel unsupported" );
            OutputDevice* pOutDev =
                (bPixelGrf || m_bPixelContour) ? Application::GetDefaultDevice()
                                             : nullptr;
            sal_uInt16 nPolyCount = m_pContour->Count();
            for( sal_uInt16 j=0; j<nPolyCount; j++ )
            {
                tools::Polygon& rPoly = (*m_pContour)[j];

                sal_uInt16 nCount = rPoly.GetSize();
                for( sal_uInt16 i=0 ; i<nCount; i++ )
                {
                    if( bPixelGrf )
                        rPoly[i] = pOutDev->LogicToPixel( rPoly[i],
                                                          aContourMap );
                    else if( m_bPixelContour )
                    {
                        rPoly[i] = pOutDev->PixelToLogic( rPoly[i], aGrfMap );

                        if ( nGrfDPIx != 0 && nGrfDPIy != 0 )
                        {
                            rPoly[i] = Point( rPoly[i].getX() * pOutDev->GetDPIX() / nGrfDPIx,
                                              rPoly[i].getY() * pOutDev->GetDPIY() / nGrfDPIy );
                        }
                    }
                    else
                        rPoly[i] = OutputDevice::LogicToLogic( rPoly[i],
                                                                 aContourMap,
                                                                 aGrfMap );
                }
            }
        }
        const_cast<SwNoTextNode *>(this)->m_bContourMapModeValid = true;
        const_cast<SwNoTextNode *>(this)->m_bPixelContour = false;
    }

    return m_pContour.get();
}

void SwNoTextNode::GetContour( tools::PolyPolygon &rPoly ) const
{
    OSL_ENSURE( m_pContour, "Contour not available." );
    rPoly = *HasContour();
}

void SwNoTextNode::SetContourAPI( const tools::PolyPolygon *pPoly )
{
    if ( pPoly )
        m_pContour.reset( new tools::PolyPolygon( *pPoly ) );
    else
        m_pContour.reset();
    m_bContourMapModeValid = false;
}

bool SwNoTextNode::GetContourAPI( tools::PolyPolygon &rContour ) const
{
    if( !m_pContour )
        return false;

    rContour = *m_pContour;
    if( m_bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        const MapMode aContourMap( MapUnit::Map100thMM );
        OSL_ENSURE( aGrfMap.GetMapUnit() != MapUnit::MapPixel ||
                aGrfMap == MapMode( MapUnit::MapPixel ),
                    "scale factor for pixel unsupported" );
        if( aGrfMap.GetMapUnit() != MapUnit::MapPixel &&
            aGrfMap != aContourMap )
        {
            sal_uInt16 nPolyCount = rContour.Count();
            for( sal_uInt16 j=0; j<nPolyCount; j++ )
            {
                tools::Polygon& rPoly = rContour[j];

                sal_uInt16 nCount = rPoly.GetSize();
                for( sal_uInt16 i=0 ; i<nCount; i++ )
                {
                    rPoly[i] = OutputDevice::LogicToLogic( rPoly[i], aGrfMap,
                                                           aContourMap );
                }
            }
        }
    }

    return true;
}

bool SwNoTextNode::IsPixelContour() const
{
    bool bRet;
    if( m_bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        bRet = aGrfMap.GetMapUnit() == MapUnit::MapPixel;
    }
    else
    {
        bRet = m_bPixelContour;
    }

    return bRet;
}

Graphic SwNoTextNode::GetGraphic() const
{
    Graphic aRet;
    if ( GetGrfNode() )
    {
        aRet = static_cast<const SwGrfNode*>(this)->GetGrf(true);
    }
    else
    {
        OSL_ENSURE( GetOLENode(), "new type of Node?" );
        aRet = *const_cast<SwOLENode*>(static_cast<const SwOLENode*>(this))->SwOLENode::GetGraphic();
    }
    return aRet;
}

// #i73249#
void SwNoTextNode::SetTitle( const OUString& rTitle )
{
    // Title attribute of <SdrObject> replaces own AlternateText attribute
    SwFlyFrameFormat* pFlyFormat = dynamic_cast<SwFlyFrameFormat*>(GetFlyFormat());
    OSL_ENSURE( pFlyFormat, "<SwNoTextNode::SetTitle(..)> - missing <SwFlyFrameFormat> instance" );
    if ( !pFlyFormat )
    {
        return;
    }

    pFlyFormat->SetObjTitle( rTitle );
}

OUString SwNoTextNode::GetTitle() const
{
    const SwFlyFrameFormat* pFlyFormat = dynamic_cast<const SwFlyFrameFormat*>(GetFlyFormat());
    OSL_ENSURE( pFlyFormat, "<SwNoTextNode::GetTitle(..)> - missing <SwFlyFrameFormat> instance" );
    if ( !pFlyFormat )
    {
        return OUString();
    }

    return pFlyFormat->GetObjTitle();
}

void SwNoTextNode::SetDescription( const OUString& rDescription )
{
    SwFlyFrameFormat* pFlyFormat = dynamic_cast<SwFlyFrameFormat*>(GetFlyFormat());
    OSL_ENSURE( pFlyFormat, "<SwNoTextNode::SetDescription(..)> - missing <SwFlyFrameFormat> instance" );
    if ( !pFlyFormat )
    {
        return;
    }

    pFlyFormat->SetObjDescription( rDescription );
}

OUString SwNoTextNode::GetDescription() const
{
    const SwFlyFrameFormat* pFlyFormat = dynamic_cast<const SwFlyFrameFormat*>(GetFlyFormat());
    OSL_ENSURE( pFlyFormat, "<SwNoTextNode::GetDescription(..)> - missing <SwFlyFrameFormat> instance" );
    if ( !pFlyFormat )
    {
        return OUString();
    }

    return pFlyFormat->GetObjDescription();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
