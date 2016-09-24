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

namespace chart
{

BarGeometryResources::BarGeometryResources(VclBuilderContainer* pWindow)
{
    pWindow->get(m_pFT_Geometry, "shapeft");
    pWindow->get(m_pLB_Geometry, "shape");
}

void BarGeometryResources::SetSelectHdl( const Link<ListBox&,void>& rLink )
{
    m_pLB_Geometry->SetSelectHdl( rLink );
}

void BarGeometryResources::Show( bool bShow )
{
    m_pFT_Geometry->Show( bShow );
    m_pLB_Geometry->Show( bShow );
}
void BarGeometryResources::Enable( bool bEnable )
{
    m_pFT_Geometry->Enable( bEnable );
    m_pLB_Geometry->Enable( bEnable );
}

sal_Int32 BarGeometryResources::GetSelectEntryCount() const
{
    return m_pLB_Geometry->GetSelectEntryCount();
}

sal_Int32 BarGeometryResources::GetSelectEntryPos() const
{
    return m_pLB_Geometry->GetSelectEntryPos();
}

void BarGeometryResources::SelectEntryPos(sal_Int32 nPos)
{
    if( nPos < m_pLB_Geometry->GetEntryCount() )
        m_pLB_Geometry->SelectEntryPos( nPos );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
