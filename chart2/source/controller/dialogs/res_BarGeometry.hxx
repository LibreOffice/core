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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_BARGEOMETRY_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_BARGEOMETRY_HXX

#include <vcl/weld.hxx>

namespace chart
{

class BarGeometryResources
{
public:
    explicit BarGeometryResources(weld::Builder* pParent);

    void set_visible( bool bShow );
    void set_sensitive( bool bEnable );

    sal_Int32 get_selected_index() const;
    void select(sal_Int32 nPos);

    void connect_changed(const Link<weld::TreeView&,void>& rLink);

private:
    std::unique_ptr<weld::Label> m_xFT_Geometry;
    std::unique_ptr<weld::TreeView> m_xLB_Geometry;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
