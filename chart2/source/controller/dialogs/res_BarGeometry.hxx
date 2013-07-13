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
#ifndef _CHART2_RES_BARGEOMETRY_HXX
#define _CHART2_RES_BARGEOMETRY_HXX

// header for class FixedText
#include <vcl/fixed.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>

namespace chart
{

class BarGeometryResources
{
public:
    BarGeometryResources( Window* pParent );
    virtual ~BarGeometryResources();

    void  SetPosPixel( const Point& rPosition );
    Size  GetSizePixel() const;

    void Show( bool bShow );
    void Enable( bool bEnable );

    sal_uInt16 GetSelectEntryCount() const;
    sal_uInt16 GetSelectEntryPos() const;
    void SelectEntryPos( sal_uInt16 nPos );

    void SetSelectHdl( const Link& rLink );

private:
    FixedText   m_aFT_Geometry;
    ListBox     m_aLB_Geometry;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
