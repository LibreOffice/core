/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fumark.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:28:59 $
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

#ifndef SC_FUMARK_HXX
#define SC_FUMARK_HXX

#include "fupoor.hxx"

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"         // ScRangeListRef
#endif


/*************************************************************************
|*
|* Funktion zum Aufziehen eines Rechtecks
|*
\************************************************************************/

class FuMarkRect : public FuPoor
{
 protected:
    Point           aBeginPos;
    Rectangle       aZoomRect;
    BOOL            bVisible;
    BOOL            bStartDrag;
    ScRangeListRef  aSourceRange;

 public:
    FuMarkRect(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuMarkRect();

    virtual BOOL KeyInput(const KeyEvent& rKEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt);

    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual BYTE Command(const CommandEvent& rCEvt);
};



#endif

