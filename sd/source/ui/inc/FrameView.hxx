/*************************************************************************
 *
 *  $RCSfile: FrameView.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 11:28:51 $
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
    const BOOL HasRuler() { return bRuler; }

    void SetNoColors(const BOOL bNoCol)
                 { bNoColors = bNoCol; }
    const BOOL IsNoColors() { return bNoColors; }

    void SetNoAttribs(const BOOL bNoAttr)
                 { bNoAttribs = bNoAttr; }
    const BOOL IsNoAttribs() { return bNoAttribs; }

    void SetVisArea(const Rectangle& rVisArea)
                 { aVisArea = rVisArea; }
    const Rectangle GetVisArea() { return aVisArea; }

    void SetPageKind(PageKind eKind) { ePageKind = eKind; }
    const PageKind GetPageKind() { return ePageKind; }

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page kind that was selected while last saving this document */
    void SetPageKindOnLoad(PageKind eKind) { ePageKindOnLoad = eKind; }

    /** can be used to get the page kind that was selected on last save of this document */
    const PageKind GetPageKindOnLoad() { return ePageKindOnLoad; }

    void SetSelectedPage (USHORT nPage);
    const USHORT GetSelectedPage (void) const;

    /** is used in FrameView::ReadUserDataSequence() only to store the
        page that was selected while last saving this document */
    void SetSelectedPageOnLoad (USHORT nPage) { nSelectedPageOnLoad = nPage; }

    /** can be used to get the page that was selected on last save of this document */
    const USHORT GetSelectedPageOnLoad (void) const { return nSelectedPageOnLoad; }

    void SetViewShEditMode(EditMode eMode, PageKind eKind);
    EditMode GetViewShEditMode(PageKind eKind);

    void SetLayerMode(BOOL bMode)
                 { bLayerMode = bMode; }
    const BOOL IsLayerMode() { return bLayerMode; }

    void SetQuickEdit(BOOL bQEdit)
                 { bQuickEdit = bQEdit; }
    const BOOL IsQuickEdit() { return bQuickEdit; }

    void        SetBigHandles( BOOL bOn = TRUE ) { bBigHandles = bOn; }
    const BOOL  IsBigHandles() const { return bBigHandles; }

    void        SetDoubleClickTextEdit( BOOL bOn = TRUE ) { bDoubleClickTextEdit = bOn; }
    const BOOL  IsDoubleClickTextEdit() const { return bDoubleClickTextEdit; }

    void        SetClickChangeRotation( BOOL bOn = TRUE ) { bClickChangeRotation = bOn; }
    const BOOL  IsClickChangeRotation() const { return bClickChangeRotation; }

    void SetDragWithCopy(BOOL bCopy)
                 { bDragWithCopy = bCopy; }
    const BOOL IsDragWithCopy() { return bDragWithCopy; }

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

    void SetPresentationViewShellId(USHORT nId)
                 { nPresViewShellId = nId; }
    const USHORT GetPresentationViewShellId() { return nPresViewShellId; }

    void SetSlotId(USHORT nId) { nSlotId = nId; }
    const USHORT GetSlotId() { return nSlotId; }

    void SetSlidesPerRow(USHORT nSlides) { nSlidesPerRow = nSlides; }
    USHORT GetSlidesPerRow() const { return nSlidesPerRow; }

    void SetDrawMode(ULONG nNewDrawMode) { nDrawMode = nNewDrawMode; };
    ULONG GetDrawMode() const { return nDrawMode; };

    void SetTabCtrlPercent( double nPercent ) { nTabCtrlPercent = nPercent; }
    double GetTabCtrlPercent() const { return nTabCtrlPercent; }

    friend SvStream& operator << (SvStream& rOut, const FrameView& rView);
    friend SvStream& operator >> (SvStream& rIn, FrameView& rView);

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

private:
    USHORT          nRefCount;
    VirtualDevice*  pVDev;
    BOOL            bRuler;
    SetOfByte       aVisibleLayers;
    SetOfByte       aLockedLayers;
    SetOfByte       aPrintableLayers;
    SdrHelpLineList aStandardHelpLines;
    SdrHelpLineList aNotesHelpLines;
    SdrHelpLineList aHandoutHelpLines;
    BOOL            bNoColors;         // Gliederungsmodus
    BOOL            bNoAttribs;        // Gliederungsmodus
    Rectangle       aVisArea;          // Sichtbarer Bereich
    PageKind        ePageKind;         // Seitentyp (Standard, Notizen, Handzettel)
    USHORT          nSelectedPage;     // Selektierte Seite
    PageKind        ePageKindOnLoad;
    USHORT          nSelectedPageOnLoad;
    EditMode        eStandardEditMode; // Editmode im Zeichenmodus (Page/MasterPage)
    EditMode        eNotesEditMode;    // Editmode im Notizen-Modus (Page/MasterPage)
    EditMode        eHandoutEditMode;  // Editmode im Handzettel-Modus (Page/MasterPage)
    BOOL            bLayerMode;        // Layer an/aus
    BOOL            bQuickEdit;        // QuickEdit an/aus
    BOOL            bDragWithCopy;     // Verschieben mit Kopie (Ctrl-Taste)
    BOOL            bBigHandles;       // Grosse Handles
    BOOL            bDoubleClickTextEdit; // Textmodus nach Doppelklick
    BOOL            bClickChangeRotation; // Einfachklick wechselt Selektions-/Rotationsmodus
    USHORT          nPresViewShellId;  // ViewShell aus der die Pres. gestartet wurde
    USHORT          nSlotId;           // SlotId, welche initial ausgefuehrt wird
    USHORT          nSlidesPerRow;     // Dias pro Reihe auf dem Diatisch
    ULONG           nDrawMode;         // Drawmode fuer das normale Fenster
    double          nTabCtrlPercent;

    /** The type of the previous view shell.  The (default) value
        ViewShell::ST_NONE indicates that there was no previous view shell.
        Note that this value is used only temporarily and is not saved or
        restored.
    */
    ViewShell::ShellType mePreviousViewShellType;
};

} // end of namespace sd

#endif

