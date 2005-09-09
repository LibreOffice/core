/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuslid.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:39:52 $
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

#ifndef SD_FU_SLIDE_HXX
#define SD_FU_SLIDE_HXX

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

class SdDrawDocument;

namespace sd {

class SlideView;
class SlideViewShell;
class Window;


/*************************************************************************
|*
|* Basisklasse der Funktionen des Diamodus
|*
\************************************************************************/

class FuSlide
    : public FuPoor
{
public:
    TYPEINFO();

    FuSlide (
        SlideViewShell* pViewSh,
        ::sd::Window* pWin,
        SlideView* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuSlide (void);

    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt) { return FALSE; }

    virtual void Activate();
    virtual void Deactivate();

    virtual void ScrollStart();
    virtual void ScrollEnd();

protected:
    SlideViewShell* pSlViewShell;
    SlideView*    pSlView;
};

} // end of namespace sd

#endif
