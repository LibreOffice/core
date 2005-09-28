/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fudraw.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 12:12:10 $
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

#ifndef SC_FUDRAW_HXX
#define SC_FUDRAW_HXX

#ifndef _SC_FUPOOR_HXX
#include "fupoor.hxx"
#endif

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

class FuDraw : public FuPoor
{
 protected:
    Pointer aNewPointer;
    Pointer aOldPointer;

 public:
    FuDraw(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuDraw();

    virtual BOOL KeyInput(const KeyEvent& rKEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt);

    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    // #97016# II
    virtual void SelectionHasChanged();

    BOOL IsSizingOrMovingNote( const MouseEvent& rMEvt ) const;
    void CheckVisibleNote() const;

 private:
    void    DoModifiers(const MouseEvent& rMEvt);
    void    ResetModifiers();
};



#endif      // _SD_FUDRAW_HXX
