/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconstr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:27:42 $
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

#ifndef SC_FUCONSTR_HXX
#define SC_FUCONSTR_HXX

#ifndef _SV_HXX
#endif

#ifndef SC_FUDRAW_HXX
#include "fudraw.hxx"
#endif


/************************************************************************/

#define MIN_FREEHAND_DISTANCE   10


/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class FuConstruct : public FuDraw
{
 public:
    FuConstruct(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq);

    virtual ~FuConstruct();
                                       // Mouse- & Key-Events
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual BYTE Command(const CommandEvent& rCEvt);

            BOOL SimpleMouseButtonUp(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};



#endif      // _SD_FUCONSTR_HXX
