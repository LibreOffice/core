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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTWINDOW_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTWINDOW_HXX

#include <vcl/window.hxx>

namespace chart
{

class WindowController;

/** The ChartWindow collects events from the window and forwards them the to the controller
thus the controller can perform appropriate actions
*/

class ChartWindow : public Window
{
public:
    ChartWindow( WindowController* pWindowController, Window* pParent, WinBits nStyle );
    virtual ~ChartWindow();

    void clear();

    //from base class Window:
    virtual void PrePaint() SAL_OVERRIDE;
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
    virtual void Activate() SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;
    virtual void GetFocus() SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;
    virtual void Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

    void ForceInvalidate();
    virtual void Invalidate( sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;
    virtual void Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;
    virtual void Invalidate( const Region& rRegion, sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

private:
    WindowController*    m_pWindowController;
    bool m_bInPaint;

    void adjustHighContrastMode();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
