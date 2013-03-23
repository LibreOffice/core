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
#pragma once
#if 1

#include <vcl/mapmod.hxx>
#include <svx/IAccessibleViewForwarder.hxx>

class Window;

namespace chart
{

class AccessibleChartView;

class AccessibleViewForwarder : public ::accessibility::IAccessibleViewForwarder
{
public:
    AccessibleViewForwarder( AccessibleChartView* pAccChartView, Window* pWindow );
    virtual ~AccessibleViewForwarder();

    // ________ IAccessibleViewforwarder ________
    virtual sal_Bool IsValid() const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel( const Point& rPoint ) const;
    virtual Size LogicToPixel( const Size& rSize ) const;
    virtual Point PixelToLogic( const Point& rPoint ) const;
    virtual Size PixelToLogic( const Size& rSize ) const;

private:
    AccessibleViewForwarder( AccessibleViewForwarder& );
    AccessibleViewForwarder& operator=( AccessibleViewForwarder& );

    AccessibleChartView* m_pAccChartView;
    Window* m_pWindow;
    MapMode m_aMapMode;
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
