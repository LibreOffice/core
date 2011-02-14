/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHARTWINDOW_HXX
#define _CHARTWINDOW_HXX

#include <vcl/window.hxx>

//.............................................................................
namespace chart
{
//.............................................................................


class WindowController;

//-----------------------------------------------------------------------------
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

//.............................................................................
} //namespace chart
//.............................................................................

#endif
