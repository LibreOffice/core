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
    m_aFT_Geometry.SetText( SCH_RESSTR( STR_BAR_GEOMETRY ) );
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

    return Size( nWidth, nHeight );
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
