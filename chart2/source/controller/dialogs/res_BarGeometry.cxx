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

