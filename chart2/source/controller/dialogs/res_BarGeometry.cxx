/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_BarGeometry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:39:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "res_BarGeometry.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "ResId.hxx"

#ifndef _SVT_CONTROLDIMS_HRC_
#include <svtools/controldims.hrc>
#endif

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
Point BarGeometryResources::GetPosPixel() const
{
    return m_aFT_Geometry.GetPosPixel();
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

USHORT BarGeometryResources::GetSelectEntryCount() const
{
    return m_aLB_Geometry.GetSelectEntryCount();
}
USHORT BarGeometryResources::GetSelectEntryPos() const
{
    return m_aLB_Geometry.GetSelectEntryPos();
}
void BarGeometryResources::SelectEntryPos( USHORT nPos )
{
    if( nPos < m_aLB_Geometry.GetEntryCount() )
        m_aLB_Geometry.SelectEntryPos( nPos );
}

//.............................................................................
} //namespace chart
//.............................................................................

