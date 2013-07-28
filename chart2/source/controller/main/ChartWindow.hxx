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
#ifndef _CHARTWINDOW_HXX
#define _CHARTWINDOW_HXX

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
    virtual void PrePaint();
    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual void MouseMove( const MouseEvent& rMEvt );
    virtual void Tracking( const TrackingEvent& rTEvt );
    virtual void MouseButtonUp( const MouseEvent& rMEvt );
    virtual void Resize();
    virtual void Activate();
    virtual void Deactivate();
    virtual void GetFocus();
    virtual void LoseFocus();
    virtual void Command( const CommandEvent& rCEvt );
    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    virtual void RequestHelp( const HelpEvent& rHEvt );

    void ForceInvalidate();
    virtual void Invalidate( sal_uInt16 nFlags = 0 );
    virtual void Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    virtual void Invalidate( const Region& rRegion, sal_uInt16 nFlags = 0 );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

private:
    WindowController*    m_pWindowController;
    bool m_bInPaint;

    void adjustHighContrastMode();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
