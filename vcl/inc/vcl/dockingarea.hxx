/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dockingarea.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:51:47 $
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

#ifndef _SV_DOCKINGAREA_HXX
#define _SV_DOCKINGAREA_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

// ------------------------------------------
//  A simple container for docked toolbars
//  - its main purpose is theming support
// ------------------------------------------

class VCL_DLLPUBLIC DockingAreaWindow : public Window
{
    class ImplData;

private:
    ImplData*       mpImplData;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  DockingAreaWindow (const DockingAreaWindow &);
    SAL_DLLPRIVATE  DockingAreaWindow & operator= (const DockingAreaWindow &);

public:
                    DockingAreaWindow( Window *pParent );
                    ~DockingAreaWindow();

    void            SetAlign( WindowAlign eNewAlign );
    WindowAlign     GetAlign() const;
    BOOL            IsHorizontal() const;

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nType );

};

#endif // _SV_SYSWIN_HXX
