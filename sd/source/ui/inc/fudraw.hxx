/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fudraw.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:32:42 $
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

#ifndef SD_FU_DRAW_HXX
#define SD_FU_DRAW_HXX

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

struct SdrViewEvent;
class SdrObject;

namespace sd {

/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

class FuDraw
    : public FuPoor
{
public:
    TYPEINFO();

    FuDraw (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuDraw();

    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual BOOL RequestHelp(const HelpEvent& rHEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt = NULL);

    virtual void DoubleClick(const MouseEvent& rMEvt);

    BOOL    SetPointer(SdrObject* pObj, const Point& rPos);
    BOOL    SetHelpText(SdrObject* pObj, const Point& rPos, const SdrViewEvent& rVEvt);

    void    SetPermanent(BOOL bSet) { bPermanent = bSet; }

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

protected:
    Pointer aNewPointer;
    Pointer aOldPointer;
    BOOL    bMBDown;
    BOOL    bDragHelpLine;
    USHORT  nHelpLine;
    BOOL    bPermanent;
};

} // end of namespace sd

#endif      // _SD_FUDRAW_HXX
