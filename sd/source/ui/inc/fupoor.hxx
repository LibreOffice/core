/*************************************************************************
 *
 *  $RCSfile: fupoor.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_FUPOOR_HXX
#define _SD_FUPOOR_HXX

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SV_EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif

class SdView;
class SdViewShell;
class SdWindow;
class SdDrawDocument;
class SdDrawDocShell;
class SfxRequest;
class Dialog;

#define HITPIX    2                    // Hit-Toleranz in Pixel
#define DRGPIX    2                    // Drag MinMove in Pixel


/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuPoor
{
 protected:
    SdView*         pView;
    SdViewShell*    pViewShell;
    SdWindow*       pWindow;
    SdDrawDocShell* pDocSh;
    SdDrawDocument* pDoc;

    USHORT          nSlotId;
    USHORT          nSlotValue;

    Dialog*         pDialog;

    Timer           aScrollTimer;           // fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // fuer Drag&Drop
    DECL_LINK( DragHdl, Timer * );
    BOOL            bIsInDragMode;
    Point           aMDPos;                 // Position von MouseButtonDown

    // Flag, um AutoScrolling zu verhindern, bis von ausserhalb in das
    // Fenster hinein gedragt wurde
    BOOL            bNoScrollUntilInside;

    // Timer um das scrolling zu verzoegern, wenn aus dem fenster
    // herausgedraggt wird (ca. 1 sec.)
    Timer           aDelayToScrollTimer;    // fuer Verzoegerung bis scroll
    BOOL            bScrollable;
    BOOL            bDelayActive;
    BOOL            bFirstMouseMove;

    DECL_LINK( DelayHdl, Timer * );
    long diffPoint (long pos1, long pos2);

 public:
    TYPEINFO();

    FuPoor(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
           SdDrawDocument* pDoc, SfxRequest& rReq);
    virtual ~FuPoor();

    SdDrawDocShell* GetDocSh() { return pDocSh; }
    SdDrawDocument* GetDoc() { return pDoc; }

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL Command(const CommandEvent& rCEvt);
    virtual BOOL RequestHelp(const HelpEvent& rHEvt);
    virtual void Paint(const Rectangle& rRect, SdWindow* pWin) {}
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(SdWindow* pWin) { pWindow = pWin; }
    void WriteStatus(const String& aStr);  // Statuszeile schreiben

    virtual void SelectionHasChanged() {}

    USHORT  GetSlotID() const { return( nSlotId ); }
    USHORT  GetSlotValue() const { return( nSlotValue ); }

    void    SetNoScrollUntilInside(BOOL bNoScroll = TRUE)
            { bNoScrollUntilInside = bNoScroll; }

    void StartDelayToScrollTimer ();
};



#endif      // _SD_FUPOOR_HXX

