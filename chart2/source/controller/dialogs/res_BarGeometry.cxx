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
#include <ResId.hxx>
#include <chart.hrc>

namespace chart
{

BarGeometryResources::BarGeometryResources(weld::Builder* pBuilder)
    : m_xFT_Geometry(pBuilder->weld_label("shapeft"))
    , m_xLB_Geometry(pBuilder->weld_tree_view("shape"))
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(CHART_TYPE); ++i)
        m_xLB_Geometry->append_text(SchResId(CHART_TYPE[i]));
    m_xLB_Geometry->set_size_request(-1, m_xLB_Geometry->get_height_rows(4));
}

void BarGeometryResources::connect_changed(const Link<weld::TreeView&,void>& rLink)
{
    m_xLB_Geometry->connect_changed(rLink);
}

void BarGeometryResources::set_visible( bool bShow )
{
    m_xFT_Geometry->set_visible( bShow );
    m_xLB_Geometry->set_visible( bShow );
}

void BarGeometryResources::set_sensitive( bool bEnable )
{
    m_xFT_Geometry->set_sensitive( bEnable );
    m_xLB_Geometry->set_sensitive( bEnable );
}

sal_Int32 BarGeometryResources::get_selected_index() const
{
    return m_xLB_Geometry->get_selected_index();
}

void BarGeometryResources::select(sal_Int32 nPos)
{
    if (nPos < m_xLB_Geometry->n_children())
        m_xLB_Geometry->select(nPos);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
