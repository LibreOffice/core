/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SD_FRAME_VIEW_HXX
#define SD_FRAME_VIEW_HXX

#include "ViewShell.hxx"
#include <svx/svdview.hxx>
#include "pres.hxx"

class SdDrawDocument;
class SdOptions;

namespace sd {

/**
 * View for MDIFrame
 */
class SD_DLLPUBLIC FrameView
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

    void SetRuler(const sal_Bool bRulerOn)
                 { mbRuler = bRulerOn; }
    sal_Bool HasRuler() const { return mbRuler; }

    void SetNoColors(const sal_Bool bNoCol)
                 { mbNoColors = bNoCol; }
    sal_Bool IsNoColors() const { return mbNoColors; }

    void SetNoAttribs(const sal_Bool bNoAttr)
                 { mbNoAttribs = bNoAttr; }
    sal_Bool IsNoAttribs() const { return mbNoAttribs; }

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

    void SetSelectedPage (sal_uInt16 nPage);
    sal_uInt16 GetSelectedPage () const;

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page that was selected while last saving this document */
    void SetSelectedPageOnLoad (sal_uInt16 nPage) { mnSelectedPageOnLoad = nPage; }

    /** can be used to get the page that was selected on last save of this document */
    sal_uInt16 GetSelectedPageOnLoad () const { return mnSelectedPageOnLoad; }

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

    void SetLayerMode(sal_Bool bMode)
                 { mbLayerMode = bMode; }
    sal_Bool IsLayerMode() const { return mbLayerMode; }

    void SetQuickEdit(sal_Bool bQEdit)
                 { mbQuickEdit = bQEdit; }
    sal_Bool IsQuickEdit() const { return mbQuickEdit; }

    void        SetDoubleClickTextEdit( sal_Bool bOn = sal_True ) { mbDoubleClickTextEdit = bOn; }
    sal_Bool    IsDoubleClickTextEdit() const { return mbDoubleClickTextEdit; }

    void        SetClickChangeRotation( sal_Bool bOn = sal_True ) { mbClickChangeRotation = bOn; }
    sal_Bool    IsClickChangeRotation() const { return mbClickChangeRotation; }

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

    void SetPresentationViewShellId(sal_uInt16 nId)
                 { mnPresViewShellId = nId; }
    sal_uInt16 GetPresentationViewShellId() const { return mnPresViewShellId; }

    void SetSlotId(sal_uInt16 nId) { mnSlotId = nId; }
    sal_uInt16 GetSlotId() const { return mnSlotId; }

    void SetSlidesPerRow(sal_uInt16 nSlides) { mnSlidesPerRow = nSlides; }
    sal_uInt16 GetSlidesPerRow() const { return mnSlidesPerRow; }

    void SetDrawMode(sal_uLong nNewDrawMode) { mnDrawMode = nNewDrawMode; };
    sal_uLong GetDrawMode() const { return mnDrawMode; };

    void SetTabCtrlPercent( double nPercent ) { mnTabCtrlPercent = nPercent; }
    double GetTabCtrlPercent() const { return mnTabCtrlPercent; }

    void SetIsNavigatorShowingAllShapes (const bool bIsNavigatorShowingAllShapes);
    bool IsNavigatorShowingAllShapes (void) const;

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

private:
    sal_uInt16          mnRefCount;
    VirtualDevice*  mpVDev;
    sal_Bool            mbRuler;
    SetOfByte       maVisibleLayers;
    SetOfByte       maLockedLayers;
    SetOfByte       maPrintableLayers;
    SdrHelpLineList maStandardHelpLines;
    SdrHelpLineList maNotesHelpLines;
    SdrHelpLineList maHandoutHelpLines;
    sal_Bool            mbNoColors;        ///< structuring mode
    sal_Bool            mbNoAttribs;       ///< structuring mode
    Rectangle           maVisArea;         ///< visible area
    PageKind            mePageKind;        ///< kind of page (standard, notes, handout)
    sal_uInt16          mnSelectedPage;
    PageKind            mePageKindOnLoad;
    sal_uInt16          mnSelectedPageOnLoad;
    EditMode            meStandardEditMode; ///< edit mode in drawing mode (Page/MasterPage)
    EditMode            meNotesEditMode;    ///< edit mode in notes mode (Page/MasterPage)
    EditMode            meHandoutEditMode;  ///< edit mode in handout mode (Page/MasterPage)
    EditMode            meEditModeOnLoad;
    sal_Bool            mbLayerMode;        ///< layer on/off
    sal_Bool            mbQuickEdit;        ///< QuickEdit on/off
    sal_Bool            mbDoubleClickTextEdit; ///< text mode after double click
    sal_Bool            mbClickChangeRotation; ///< single click switches between selection/rotation mode
    sal_uInt16          mnPresViewShellId;  ///< ViewShell from which the presentation was started
    sal_uInt16          mnSlotId;           ///< SlotId, which was initial mentioned
    sal_uInt16          mnSlidesPerRow;     ///< slides per row on the slide-desk
    sal_uLong           mnDrawMode;         ///< draw mode for the normal window
    double              mnTabCtrlPercent;
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
