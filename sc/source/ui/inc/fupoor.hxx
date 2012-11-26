/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SC_FUPOOR_HXX
#define _SC_FUPOOR_HXX

#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <sfx2/request.hxx>
#include <basegfx/point/b2dpoint.hxx>

class ScDrawView;
class ScTabViewShell;
class Window;
class SdrModel;
class Dialog;
class SdrObject;
namespace basegfx { class B2DRange; }

//  Return-Werte fuer Command
#define SC_CMD_NONE     0
#define SC_CMD_USED     1
#define SC_CMD_IGNORE   2

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuPoor
{
protected:
    ScDrawView*     pView;
    ScTabViewShell* pViewShell;
    Window*         pWindow;
    SdrModel*       pDrDoc;

    SfxRequest      aSfxRequest;
    Dialog*         pDialog;

    Timer           aScrollTimer;           // fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // fuer Drag&Drop
    DECL_LINK( DragTimerHdl, Timer * );
    DECL_LINK( DragHdl, void * );
    sal_Bool            bIsInDragMode;
    basegfx::B2DPoint   aMDPos;                 // Position von MouseButtonDown

    // #95491# member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)
private:
    sal_uInt16      mnCode;

public:
    FuPoor(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuPoor();

    // #95491# see member
    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    virtual void Paint(const Rectangle&, Window*) {}

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent&) { return sal_False; }

    // #95491# moved from inline to *.cxx
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt); // { return sal_False; }

    // #95491# moved from inline to *.cxx
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt); // { return sal_False; }

    virtual sal_uInt8 Command(const CommandEvent& rCEvt);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(Window* pWin) { pWindow = pWin; }

    sal_uInt16 GetSlotID() const { return( aSfxRequest.GetSlot() ); }

    bool IsDetectiveHit( const basegfx::B2DPoint& rLogicPos );

    void    StopDragTimer();

    // #98185# Create default drawing objects via keyboard
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange);

protected:
    void ImpForceQuadratic(basegfx::B2DRange& rRange);

public:
    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SD_FUPOOR_HXX

