/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuzoom.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:35:50 $
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

#ifndef SD_FU_ZOOM_HXX
#define SD_FU_ZOOM_HXX

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

namespace sd {

extern USHORT SidArrayZoom[];

class FuZoom
    : public FuPoor
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

protected:
    virtual ~FuZoom (void);

    Point       aBeginPosPix;
    Point       aBeginPos;
    Point       aEndPos;
    Rectangle   aZoomRect;
    BOOL        bVisible;
    BOOL        bStartDrag;
    Pointer     aPtr;

private:
    FuZoom (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
};

} // end of namespace sd

#endif

