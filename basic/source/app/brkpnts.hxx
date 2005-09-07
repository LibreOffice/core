/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brkpnts.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:14:33 $
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

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#define MARKER_NOMARKER 0xFFFF


class SbModule;
class BreakpointListe;
struct Breakpoint;
class ImageList;

DECLARE_LIST( BreakpointList, Breakpoint* );

class BreakpointWindow : public Window, public BreakpointList
{
public:
    BreakpointWindow( Window *pParent );
//  ~BreakpointWindow();

    void        Reset();

    void        SetModule( SbModule *pMod );
    void        SetBPsInModule();

    void        InsertBreakpoint( USHORT nLine );
    void        ToggleBreakpoint( USHORT nLine );
    void        AdjustBreakpoints( ULONG nLine, BOOL bInserted );

    void        LoadBreakpoints( String aFilename );
    void        SaveBreakpoints( String aFilename );

protected:
    Breakpoint* FindBreakpoint( ULONG nLine );

private:
    BreakpointListe *pBreakpoints;

private:
    long            nCurYOffset;
    USHORT          nMarkerPos;
    SbModule*       pModule;
    BOOL            bErrorMarker;
    static ImageList *pImages;

protected:
    virtual void    Paint( const Rectangle& );
    Breakpoint*     FindBreakpoint( const Point& rMousePos );
    void            ShowMarker( BOOL bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

public:

//  void            SetModulWindow( ModulWindow* pWin )
//                      { pModulWindow = pWin; }

    void            SetMarkerPos( USHORT nLine, BOOL bErrorMarker = FALSE );

    void            Scroll( long nHorzScroll, long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
};








