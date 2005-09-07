/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartWindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:34:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHARTWINDOW_HXX
#define _CHARTWINDOW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

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

    //from base class Window:
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

private:
    WindowController*    m_pWindowController;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
