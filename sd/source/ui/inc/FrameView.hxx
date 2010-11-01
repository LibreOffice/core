/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_FRAME_VIEW_HXX
#define SD_FRAME_VIEW_HXX

#include "ViewShell.hxx"
#include <svx/svdview.hxx>
#include "pres.hxx"

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
    SD_DLLPUBLIC FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView = NULL );
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
    BOOL HasRuler() const { return mbRuler; }

    void SetNoColors(const BOOL bNoCol)
                 { mbNoColors = bNoCol; }
    BOOL IsNoColors() const { return mbNoColors; }

    void SetNoAttribs(const BOOL bNoAttr)
                 { mbNoAttribs = bNoAttr; }
    BOOL IsNoAttribs() const { return mbNoAttribs; }

    void SetVisArea(const Rectangle& rVisArea)
                 { maVisArea = rVisArea; }
    const Rectangle GetVisArea() { return maVisArea; }

    void SetPageKind(PageKind eKind) { mePageKind = eKind; }
    PageKind GetPageKind() const { return mePageKind; }

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page kind that was selected while last saving this document */
    void SetPageKindOnLoad(PageKind eKind) { mePageKindOnLoad = eKind; }

    /** can be used to get the page kind that was selected on last save of this document */
    PageKind GetPageKindOnLoad() const { return mePageKindOnLoad; }

    SD_DLLPUBLIC void SetSelectedPage (USHORT nPage);
    USHORT GetSelectedPage () const;

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page that was selected while last saving this document */
    void SetSelectedPageOnLoad (USHORT nPage) { mnSelectedPageOnLoad = nPage; }

    /** can be used to get the page that was selected on last save of this document */
    USHORT GetSelectedPageOnLoad () const { return mnSelectedPageOnLoad; }

    SD_DLLPUBLIC void SetViewShEditMode(EditMode eMode, PageKind eKind);
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
    BOOL IsLayerMode() const { return mbLayerMode; }

    void SetQuickEdit(BOOL bQEdit)
                 { mbQuickEdit = bQEdit; }
    BOOL IsQuickEdit() const { return mbQuickEdit; }

    void        SetBigHandles( BOOL bOn = TRUE ) { mbBigHandles = bOn; }
    BOOL    IsBigHandles() const { return mbBigHandles; }

    void        SetDoubleClickTextEdit( BOOL bOn = TRUE ) { mbDoubleClickTextEdit = bOn; }
    BOOL    IsDoubleClickTextEdit() const { return mbDoubleClickTextEdit; }

    void        SetClickChangeRotation( BOOL bOn = TRUE ) { mbClickChangeRotation = bOn; }
    BOOL    IsClickChangeRotation() const { return mbClickChangeRotation; }

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
    USHORT GetPresentationViewShellId() const { return mnPresViewShellId; }

    void SetSlotId(USHORT nId) { mnSlotId = nId; }
    USHORT GetSlotId() const { return mnSlotId; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
