/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FrameView.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 13:37:48 $
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

#ifndef SD_FRAME_VIEW_HXX
#define SD_FRAME_VIEW_HXX

#include "ViewShell.hxx"

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

class SdDrawDocument;
class SdOptions;

namespace sd {

/*************************************************************************
|*
|* View fuer den MDIFrame
|*
\************************************************************************/
class FrameView
    : public SdrView
{
public:
    FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView = NULL );
    FrameView(const FrameView& rFrameView);
    virtual ~FrameView();

    void Connect();
    void Disconnect();

    void Update(SdOptions* pOptions);

    void  SetStandardHelpLines(const SdrHelpLineList& rHelpLines)
                             { maStandardHelpLines = rHelpLines; }
    const SdrHelpLineList& GetStandardHelpLines() { return maStandardHelpLines; }
    void  SetNotesHelpLines(const SdrHelpLineList& rHelpLines)
                             { maNotesHelpLines = rHelpLines; }
    const SdrHelpLineList& GetNotesHelpLines() { return maNotesHelpLines; }
    void  SetHandoutHelpLines(const SdrHelpLineList& rHelpLines)
                             { maHandoutHelpLines = rHelpLines; }
    const SdrHelpLineList& GetHandoutHelpLines() { return maHandoutHelpLines; }

    void SetVisibleLayers(const SetOfByte& rVisibleLayers)
                         { maVisibleLayers = rVisibleLayers; }
    const SetOfByte& GetVisibleLayers() { return maVisibleLayers; }

    void SetLockedLayers(const SetOfByte& rLockedLayers)
                        { maLockedLayers = rLockedLayers; }
    const SetOfByte& GetLockedLayers() { return maLockedLayers; }

    void SetPrintableLayers(const SetOfByte& rPrintableLayers)
                         { maPrintableLayers = rPrintableLayers; }
    const SetOfByte& GetPrintableLayers() { return maPrintableLayers; }

    void SetRuler(const BOOL bRulerOn)
                 { mbRuler = bRulerOn; }
    const BOOL HasRuler() { return mbRuler; }

    void SetNoColors(const BOOL bNoCol)
                 { mbNoColors = bNoCol; }
    const BOOL IsNoColors() { return mbNoColors; }

    void SetNoAttribs(const BOOL bNoAttr)
                 { mbNoAttribs = bNoAttr; }
    const BOOL IsNoAttribs() { return mbNoAttribs; }

    void SetVisArea(const Rectangle& rVisArea)
                 { maVisArea = rVisArea; }
    const Rectangle GetVisArea() { return maVisArea; }

    void SetPageKind(PageKind eKind) { mePageKind = eKind; }
    const PageKind GetPageKind() { return mePageKind; }

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page kind that was selected while last saving this document */
    void SetPageKindOnLoad(PageKind eKind) { mePageKindOnLoad = eKind; }

    /** can be used to get the page kind that was selected on last save of this document */
    const PageKind GetPageKindOnLoad() { return mePageKindOnLoad; }

    void SetSelectedPage (USHORT nPage);
    const USHORT GetSelectedPage (void) const;

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page that was selected while last saving this document */
    void SetSelectedPageOnLoad (USHORT nPage) { mnSelectedPageOnLoad = nPage; }

    /** can be used to get the page that was selected on last save of this document */
    const USHORT GetSelectedPageOnLoad (void) const { return mnSelectedPageOnLoad; }

    void SetViewShEditMode(EditMode eMode, PageKind eKind);
    EditMode GetViewShEditMode (PageKind eKind);

    /** Remember the edit mode of the main view shell at the time when the
        document is loaded.
    */
    void SetViewShEditModeOnLoad (const EditMode eMode);

    /** Return the value of the edit mode as it was when the document was
        loaded.
    */
    EditMode GetViewShEditModeOnLoad (void) const;

    void SetLayerMode(BOOL bMode)
                 { mbLayerMode = bMode; }
    const BOOL IsLayerMode() { return mbLayerMode; }

    void SetQuickEdit(BOOL bQEdit)
                 { mbQuickEdit = bQEdit; }
    const BOOL IsQuickEdit() { return mbQuickEdit; }

    void        SetBigHandles( BOOL bOn = TRUE ) { mbBigHandles = bOn; }
    const BOOL  IsBigHandles() const { return mbBigHandles; }

    void        SetDoubleClickTextEdit( BOOL bOn = TRUE ) { mbDoubleClickTextEdit = bOn; }
    const BOOL  IsDoubleClickTextEdit() const { return mbDoubleClickTextEdit; }

    void        SetClickChangeRotation( BOOL bOn = TRUE ) { mbClickChangeRotation = bOn; }
    const BOOL  IsClickChangeRotation() const { return mbClickChangeRotation; }

    /** Remember the type of the view shell that was (or soon will be)
        previously associated with this frame view.
        @param eType
            The type of the previous view shell or ViewShell::ST_NONE to
            indicate that there is no previous view shell.
    */
    void SetPreviousViewShellType (ViewShell::ShellType eType);

    /** Return the type of the view shell previously associated with this
        frame view.
    */
    ViewShell::ShellType GetPreviousViewShellType (void) const;

    /** Remember the type of the view shell at the time when the document is
        loaded or, rather, when the ViewShellBase is constructed.
    */
    void SetViewShellTypeOnLoad (ViewShell::ShellType eType);

    ViewShell::ShellType GetViewShellTypeOnLoad (void) const;

    void SetPresentationViewShellId(USHORT nId)
                 { mnPresViewShellId = nId; }
    const USHORT GetPresentationViewShellId() { return mnPresViewShellId; }

    void SetSlotId(USHORT nId) { mnSlotId = nId; }
    const USHORT GetSlotId() { return mnSlotId; }

    void SetSlidesPerRow(USHORT nSlides) { mnSlidesPerRow = nSlides; }
    USHORT GetSlidesPerRow() const { return mnSlidesPerRow; }

    void SetDrawMode(ULONG nNewDrawMode) { mnDrawMode = nNewDrawMode; };
    ULONG GetDrawMode() const { return mnDrawMode; };

    void SetTabCtrlPercent( double nPercent ) { mnTabCtrlPercent = nPercent; }
    double GetTabCtrlPercent() const { return mnTabCtrlPercent; }

    void SetIsNavigatorShowingAllShapes (const bool bIsNavigatorShowingAllShapes);
    bool IsNavigatorShowingAllShapes (void) const;

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

private:
    USHORT          mnRefCount;
    VirtualDevice*  mpVDev;
    BOOL            mbRuler;
    SetOfByte       maVisibleLayers;
    SetOfByte       maLockedLayers;
    SetOfByte       maPrintableLayers;
    SdrHelpLineList maStandardHelpLines;
    SdrHelpLineList maNotesHelpLines;
    SdrHelpLineList maHandoutHelpLines;
    BOOL            mbNoColors;        // Gliederungsmodus
    BOOL            mbNoAttribs;       // Gliederungsmodus
    Rectangle       maVisArea;         // Sichtbarer Bereich
    PageKind        mePageKind;        // Seitentyp (Standard, Notizen, Handzettel)
    USHORT          mnSelectedPage;    // Selektierte Seite
    PageKind        mePageKindOnLoad;
    USHORT          mnSelectedPageOnLoad;
    EditMode        meStandardEditMode; // Editmode im Zeichenmodus (Page/MasterPage)
    EditMode        meNotesEditMode;    // Editmode im Notizen-Modus (Page/MasterPage)
    EditMode        meHandoutEditMode;  // Editmode im Handzettel-Modus (Page/MasterPage)
    EditMode        meEditModeOnLoad;
    BOOL            mbLayerMode;       // Layer an/aus
    BOOL            mbQuickEdit;       // QuickEdit an/aus
    BOOL            mbBigHandles;      // Grosse Handles
    BOOL            mbDoubleClickTextEdit; // Textmodus nach Doppelklick
    BOOL            mbClickChangeRotation; // Einfachklick wechselt Selektions-/Rotationsmodus
    USHORT          mnPresViewShellId;  // ViewShell aus der die Pres. gestartet wurde
    USHORT          mnSlotId;          // SlotId, welche initial ausgefuehrt wird
    USHORT          mnSlidesPerRow;     // Dias pro Reihe auf dem Diatisch
    ULONG           mnDrawMode;        // Drawmode fuer das normale Fenster
    double          mnTabCtrlPercent;
    /** Remember whether the navigator shows all shapes (<TRUE/>) or only
        the names ones (<FALSE/>).  Not persistent.
    */
    bool            mbIsNavigatorShowingAllShapes;

    /** The type of the previous view shell.  The (default) value
        ViewShell::ST_NONE indicates that there was no previous view shell.
        Note that this value is used only temporarily and is not saved or
        restored.
    */
    ViewShell::ShellType mePreviousViewShellType;

    ViewShell::ShellType meViewShellTypeOnLoad;
};

} // end of namespace sd

#endif

