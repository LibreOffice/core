/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_chart2.hxx"

#include "res_BarGeometry.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "ResId.hxx"

#include <svtools/controldims.hrc>

//.............................................................................
namespace chart
{
//.............................................................................

BarGeometryResources::BarGeometryResources( Window* pWindow )
    : m_aFT_Geometry( pWindow, pWindow->GetStyle() )
    , m_aLB_Geometry( pWindow, SchResId( LB_BAR_GEOMETRY ) )
{
    m_aFT_Geometry.SetText( String( SchResId( STR_BAR_GEOMETRY )) );
    m_aFT_Geometry.SetSizePixel( m_aFT_Geometry.CalcMinimumSize() );
    m_aLB_Geometry.SetAccessibleName(m_aFT_Geometry.GetText());
    m_aLB_Geometry.SetAccessibleRelationLabeledBy(&m_aFT_Geometry);
}
void BarGeometryResources::SetPosPixel( const Point& rPosition )
{
    Window* pWindow( m_aFT_Geometry.GetParent() );

    Size aDistanceSize( 2,2 );
    if( pWindow )
        aDistanceSize = Size( pWindow->LogicToPixel( Size(0,RSC_SP_CTRL_DESC_Y), MapMode(MAP_APPFONT) ) );

    m_aFT_Geometry.SetPosPixel( rPosition );
    m_aLB_Geometry.SetPosPixel( Point( rPosition.X()+aDistanceSize.Width(), rPosition.Y()+m_aFT_Geometry.GetSizePixel().Height()+aDistanceSize.Height()) );
}
Size BarGeometryResources::GetSizePixel() const
{
    long nHeight = m_aLB_Geometry.GetPosPixel().Y()
        - m_aFT_Geometry.GetPosPixel().Y();
    nHeight += m_aLB_Geometry.GetSizePixel().Height();

    long nWidth = m_aLB_Geometry.GetSizePixel().Width();
    if( nWidth < m_aFT_Geometry.GetSizePixel().Width() )
        nWidth = m_aFT_Geometry.GetSizePixel().Width();

    return Size( nHeight, nWidth );
}
BarGeometryResources::~BarGeometryResources()
{
}

void BarGeometryResources::SetSelectHdl( const Link& rLink )
{
    m_aLB_Geometry.SetSelectHdl( rLink );
}

void BarGeometryResources::Show( bool bShow )
{
    m_aFT_Geometry.Show( bShow );
    m_aLB_Geometry.Show( bShow );
}
void BarGeometryResources::Enable( bool bEnable )
{
    m_aFT_Geometry.Enable( bEnable );
    m_aLB_Geometry.Enable( bEnable );
}

sal_uInt16 BarGeometryResources::GetSelectEntryCount() const
{
    return m_aLB_Geometry.GetSelectEntryCount();
}
sal_uInt16 BarGeometryResources::GetSelectEntryPos() const
{
    return m_aLB_Geometry.GetSelectEntryPos();
}
void BarGeometryResources::SelectEntryPos( sal_uInt16 nPos )
{
    if( nPos < m_aLB_Geometry.GetEntryCount() )
        m_aLB_Geometry.SelectEntryPos( nPos );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
