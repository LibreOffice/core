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



#ifndef SD_FU_POOR_HXX
#define SD_FU_POOR_HXX

#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <rtl/ref.hxx>
#include <basegfx/point/b2dpoint.hxx>

#ifndef _SALHELPER_SIMPLEREFERENCECOMPONENT_HXX_
#include "helper/simplereferencecomponent.hxx"
#endif

class SdDrawDocument;
class SfxRequest;
class Dialog;
class SdrObject;
namespace basegfx { class B2DRange; }

namespace sd {

class DrawDocShell;
class View;
class ViewShell;
class Window;

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuPoor : public SimpleReferenceComponent
{
public:
    static const int HITPIX = 2;                   // Hit-Toleranz in Pixel

    virtual void DoExecute( SfxRequest& rReq );

    // #95491# see member
    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    DrawDocShell* GetDocSh() { return mpDocSh; }
    SdDrawDocument* GetDoc() { return mpDoc; }

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=true: Event wurde bearbeitet
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& );
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);

    // #95491# moved from inline to *.cxx
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual bool Command(const CommandEvent& rCEvt);
    virtual bool RequestHelp(const HelpEvent& rHEvt);
    virtual void Paint(const Rectangle&, ::sd::Window* );
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(::sd::Window* pWin) { mpWindow = pWin; }
    virtual void SelectionHasChanged();

    sal_uInt16  GetSlotID() const { return( nSlotId ); }
    sal_uInt16  GetSlotValue() const { return( nSlotValue ); }

    void    SetNoScrollUntilInside(bool bNoScroll = true)
            { bNoScrollUntilInside = bNoScroll; }

    void StartDelayToScrollTimer ();

    // #97016#
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

    // #i33136#
    /** Decide if the object to be created should be created
        orthogonal. Default implementation uses nSlotID
        to decide. May be overloaded to use other criteria
        for this decision

        @returns true if the to be created object should be orthogonal.
    */
    virtual bool doConstructOrthogonal() const;

protected:
    /**
        @param pViewSh
            May be NULL.
    */
    FuPoor (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuPoor (void);

    DECL_LINK( DelayHdl, Timer * );

    void ImpForceQuadratic(basegfx::B2DRange& rRange);

    /** Switch to another layer.  The layer to switch to is specified by an
        offset relative to the active layer.  With respect to the layer bar
        control at the lower left of the document window positive values
        move to the right and negative values move to the left.

        <p>Switching the layer is independent of the view's layer mode.  The
        layers are switched even when the layer mode is turned off and the
        layer control is not visible.</p>
        @param nOffset
           If the offset is positive skip that many layers in selecting the
           next layer.  If it is negative then select a previous one.  An
           offset or zero does not change the current layer.  If the
           resulting index lies outside the valid range of indices then it
           is set to either the minimal or maximal valid index, whitchever
           is nearer.
    */
    void SwitchLayer (sal_Int32 nOffset);

    ::sd::View* mpView;
    ViewShell* mpViewShell;
    ::sd::Window* mpWindow;
    DrawDocShell* mpDocSh;
    SdDrawDocument* mpDoc;

    sal_uInt16          nSlotId;
    sal_uInt16          nSlotValue;

    Dialog*         pDialog;

    Timer           aScrollTimer;           // fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );
    void ForceScroll(const basegfx::B2DPoint& aPixPos);

    Timer           aDragTimer;             // fuer Drag&Drop
    DECL_LINK( DragHdl, Timer * );
    bool            bIsInDragMode;
    basegfx::B2DPoint   aMDPos;                 // Position von MouseButtonDown

    // Flag, um AutoScrolling zu verhindern, bis von ausserhalb in das
    // Fenster hinein gedragt wurde
    bool            bNoScrollUntilInside;

    // Timer um das scrolling zu verzoegern, wenn aus dem fenster
    // herausgedraggt wird (ca. 1 sec.)
    Timer           aDelayToScrollTimer;    // fuer Verzoegerung bis scroll
    bool            bScrollable;
    bool            bDelayActive;
    bool            bFirstMouseMove;

    // #95491# member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)

private:
    sal_uInt16      mnCode;

};

typedef rtl::Reference< FuPoor > FunctionReference;

} // end of namespace sd

#endif      // _SD_FUPOOR_HXX

