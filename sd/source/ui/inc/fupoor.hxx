/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fupoor.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:43:53 $
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

#ifndef SD_FU_POOR_HXX
#define SD_FU_POOR_HXX

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
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
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _SALHELPER_SIMPLEREFERENCECOMPONENT_HXX_
#include "helper/simplereferencecomponent.hxx"
#endif

class SdDrawDocument;
class SfxRequest;
class Dialog;
class SdrObject;

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
    static const int DRGPIX = 2;                   // Drag MinMove in Pixel

    TYPEINFO();

    virtual void DoExecute( SfxRequest& rReq );

    // #95491# see member
    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    const sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    DrawDocShell* GetDocSh() { return mpDocSh; }
    SdDrawDocument* GetDoc() { return mpDoc; }

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& );
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);

    // #95491# moved from inline to *.cxx
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual BOOL Command(const CommandEvent& rCEvt);
    virtual BOOL RequestHelp(const HelpEvent& rHEvt);
    virtual void Paint(const Rectangle&, ::sd::Window* );
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(::sd::Window* pWin) { mpWindow = pWin; }
    void WriteStatus(const String& aStr);  // Statuszeile schreiben

    // #97016# II
    virtual void SelectionHasChanged();

    USHORT  GetSlotID() const { return( nSlotId ); }
    USHORT  GetSlotValue() const { return( nSlotValue ); }

    void    SetNoScrollUntilInside(BOOL bNoScroll = TRUE)
            { bNoScrollUntilInside = bNoScroll; }

    void StartDelayToScrollTimer ();

    // #97016#
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

    // #i33136#
    /** Decide if the object to be created should be created
        orthogonal. Default implementation uses nSlotID
        to decide. May be overloaded to use other criterias
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
    long diffPoint (long pos1, long pos2);

    void ImpForceQuadratic(Rectangle& rRect);

    /** Switch to another layer.  The layer to switch to is specified by an
        offset relative to the active layer.  With respect to the layer bar
        control at the lower left of the document window positive values
        move to the right and negative values move to the left.

        <p>Switching the layer is independant of the view's layer mode.  The
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

    // #95491# member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)

private:
    sal_uInt16      mnCode;

};

typedef rtl::Reference< FuPoor > FunctionReference;

} // end of namespace sd

#endif      // _SD_FUPOOR_HXX

