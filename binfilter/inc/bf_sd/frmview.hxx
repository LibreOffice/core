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

#ifndef _SD_FRMVIEW_HXX
#define _SD_FRMVIEW_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <bf_svx/svdview.hxx>

#include "pres.hxx"
namespace binfilter {

class SdDrawDocument;
class SdOptions;


/*************************************************************************
|*
|* View fuer den MDIFrame
|*
\************************************************************************/
class FrameView : public SdrView
{
private:
    USHORT			nRefCount;
    VirtualDevice*	pVDev;
    BOOL			bRuler;
    SetOfByte		aVisibleLayers;
    SetOfByte		aLockedLayers;
    SetOfByte		aPrintableLayers;
    SdrHelpLineList aStandardHelpLines;
    SdrHelpLineList aNotesHelpLines;
    SdrHelpLineList aHandoutHelpLines;
    BOOL			bNoColors;		   // Gliederungsmodus
    BOOL			bNoAttribs; 	   // Gliederungsmodus
    Rectangle		aVisArea;		   // Sichtbarer Bereich
    PageKind		ePageKind;		   // Seitentyp (Standard, Notizen, Handzettel)
    USHORT			nSelectedPage;	   // Selektierte Seite
    EditMode        eStandardEditMode; // Editmode im Zeichenmodus (Page/MasterPage)
    EditMode        eNotesEditMode;    // Editmode im Notizen-Modus (Page/MasterPage)
    EditMode        eHandoutEditMode;  // Editmode im Handzettel-Modus (Page/MasterPage)
    BOOL			bLayerMode; 	   // Layer an/aus
    BOOL			bQuickEdit; 	   // QuickEdit an/aus
    BOOL			bDragWithCopy;	   // Verschieben mit Kopie (Ctrl-Taste)
    BOOL			bBigHandles;	   // Grosse Handles
    BOOL			bDoubleClickTextEdit; // Textmodus nach Doppelklick
    BOOL			bClickChangeRotation; // Einfachklick wechselt Selektions-/Rotationsmodus
    USHORT			nPresViewShellId;  // ViewShell aus der die Pres. gestartet wurde
    USHORT			nSlotId;		   // SlotId, welche initial ausgefuehrt wird
    USHORT          nSlidesPerRow;     // Dias pro Reihe auf dem Diatisch
    ULONG			nDrawMode;		   // Drawmode fuer das normale Fenster
    ULONG			nPreviewDrawMode;  // Drawmode fuer das preview Fenster
    BOOL            bShowPreviewInPageMode;       // Preview in EM_PAGE automatisch anzeigen
    BOOL            bShowPreviewInMasterPageMode; // Preview in EM_MASTERPAGE automatisch anzeigen
    BOOL            bShowPreviewInOutlineMode;    // Preview im Gliederungsmodus automatisch anzeigen
    double          nTabCtrlPercent;

public:
    FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView = NULL );
    FrameView(const FrameView& rFrameView);
    virtual ~FrameView();

    void Connect();
    void Disconnect();

    void Update(SdOptions* pOptions);

    void  SetStandardHelpLines(const SdrHelpLineList& rHelpLines)
                             { aStandardHelpLines = rHelpLines; }
    const SdrHelpLineList& GetStandardHelpLines() { return aStandardHelpLines; }
    void  SetNotesHelpLines(const SdrHelpLineList& rHelpLines)
                             { aNotesHelpLines = rHelpLines; }
    const SdrHelpLineList& GetNotesHelpLines() { return aNotesHelpLines; }
    void  SetHandoutHelpLines(const SdrHelpLineList& rHelpLines)
                             { aHandoutHelpLines = rHelpLines; }
    const SdrHelpLineList& GetHandoutHelpLines() { return aHandoutHelpLines; }

    void SetVisibleLayers(const SetOfByte& rVisibleLayers)
                         { aVisibleLayers = rVisibleLayers; }
    const SetOfByte& GetVisibleLayers() { return aVisibleLayers; }

    void SetLockedLayers(const SetOfByte& rLockedLayers)
                        { aLockedLayers = rLockedLayers; }
    const SetOfByte& GetLockedLayers() { return aLockedLayers; }

    void SetPrintableLayers(const SetOfByte& rPrintableLayers)
                         { aPrintableLayers = rPrintableLayers; }
    const SetOfByte& GetPrintableLayers() { return aPrintableLayers; }

    void SetRuler(const BOOL bRulerOn)
                 { bRuler = bRulerOn; }
    BOOL HasRuler() const { return bRuler; }

    void SetNoColors(const BOOL bNoCol)
                 { bNoColors = bNoCol; }
    BOOL IsNoColors() const { return bNoColors; }

    void SetNoAttribs(const BOOL bNoAttr)
                 { bNoAttribs = bNoAttr; }
    BOOL IsNoAttribs() const { return bNoAttribs; }

    void SetVisArea(const Rectangle& rVisArea)
                 { aVisArea = rVisArea; }
    const Rectangle GetVisArea() { return aVisArea; }

    void SetPageKind(PageKind eKind)
                 { ePageKind = eKind; }
    PageKind GetPageKind() const { return ePageKind; }

    void SetSelectedPage(USHORT nPage)
                 { nSelectedPage = nPage; }
    USHORT GetSelectedPage() const { return nSelectedPage; }

    void SetViewShEditMode(EditMode eMode, PageKind eKind);
    EditMode GetViewShEditMode(PageKind eKind);

    void SetLayerMode(BOOL bMode)
                 { bLayerMode = bMode; }
    BOOL IsLayerMode() const { return bLayerMode; }

    void SetQuickEdit(BOOL bQEdit)
                 { bQuickEdit = bQEdit; }
    BOOL IsQuickEdit() const { return bQuickEdit; }

    void		SetBigHandles( BOOL bOn = TRUE ) { bBigHandles = bOn; }
    BOOL	IsBigHandles() const { return bBigHandles; }

    void		SetDoubleClickTextEdit( BOOL bOn = TRUE ) { bDoubleClickTextEdit = bOn; }
    BOOL	IsDoubleClickTextEdit() const { return bDoubleClickTextEdit; }

    void		SetClickChangeRotation( BOOL bOn = TRUE ) { bClickChangeRotation = bOn; }
    BOOL	IsClickChangeRotation() const { return bClickChangeRotation; }

    void SetDragWithCopy(BOOL bCopy)
                 { bDragWithCopy = bCopy; }
    BOOL IsDragWithCopy() const { return bDragWithCopy; }

    void SetPresentationViewShellId(USHORT nId)
                 { nPresViewShellId = nId; }
    USHORT GetPresentationViewShellId() const { return nPresViewShellId; }

    void SetSlotId(USHORT nId) { nSlotId = nId; }
    USHORT GetSlotId() const { return nSlotId; }

    void SetSlidesPerRow(USHORT nSlides) { nSlidesPerRow = nSlides; }
    USHORT GetSlidesPerRow() const { return nSlidesPerRow; }

    void SetDrawMode(ULONG nNewDrawMode) { nDrawMode = nNewDrawMode; };
    ULONG GetDrawMode() const { return nDrawMode; };

    void SetPreviewDrawMode(ULONG nNewPrvDrawMode) { nPreviewDrawMode = nNewPrvDrawMode; };
    ULONG GetPreviewDrawMode() const { return nPreviewDrawMode; };

    void SetShowPreviewInPageMode(BOOL bShowPreview) { bShowPreviewInPageMode = bShowPreview; };
    ULONG IsShowPreviewInPageMode() const { return bShowPreviewInPageMode; };

    void SetShowPreviewInMasterPageMode(BOOL bShowPreview) { bShowPreviewInMasterPageMode = bShowPreview; };
    ULONG IsShowPreviewInMasterPageMode() const { return bShowPreviewInMasterPageMode; };

    void SetShowPreviewInOutlineMode(BOOL bShowPreview) { bShowPreviewInOutlineMode = bShowPreview; };
    ULONG IsShowPreviewInOutlineMode() const { return bShowPreviewInOutlineMode; };
    
    void SetTabCtrlPercent( double nPercent ) { nTabCtrlPercent = nPercent; }
    double GetTabCtrlPercent() const { return nTabCtrlPercent; }

    friend SvStream& operator << (SvStream& rOut, const FrameView& rView);
    friend SvStream& operator >> (SvStream& rIn, FrameView& rView);

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
};




} //namespace binfilter
#endif		// _SD_FRMVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
