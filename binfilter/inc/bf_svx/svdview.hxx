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

#ifndef _SVDVIEW_HXX
#define _SVDVIEW_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#ifndef _SVDHDL_HXX
#include <bf_svx/svdhdl.hxx>
#endif

#include <bf_svx/svdcrtv.hxx>
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Klassenhierarchie der View:
//         SfxListener
//         SdrPaintView    PntV   Action            ModChg   Attr   Notify
//         SdrSnapView     SnpV   Action
//
//         SdrMarkView     MrkV   Action   MrkChg   ModChg          Notify
//
//         SdrEditView     EdtV            MrkChg   ModChg   Attr
//         SdrPolyEditView PoEV
//         SdrGlueEditView GlEV
//         SdrObjEditView  EdxV   Action            ModChg   Attr   Notify
//
//         SdrExchangeView XcgV
//         SdrDragView     DrgV   Action
//
//         SdrCreateView   CrtV   Action
//         SdrView         View
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SvxURLField;

//************************************************************
//   Defines
//************************************************************

enum SdrViewContext {SDRCONTEXT_STANDARD,
                     SDRCONTEXT_POINTEDIT,
                     SDRCONTEXT_GLUEPOINTEDIT,
                     SDRCONTEXT_TEXTEDIT,
                     SDRCONTEXT_GRAPHIC};

enum SdrEventKind  {SDREVENT_NONE,
                    SDREVENT_TEXTEDIT,
                    SDREVENT_MOVACTION,
                    SDREVENT_ENDACTION,
                    SDREVENT_BCKACTION,
                    SDREVENT_BRKACTION,
                    SDREVENT_ENDCREATE,
                    SDREVENT_ENDDRAG,
                    SDREVENT_MARKOBJ,
                    SDREVENT_MARKPOINT,
                    SDREVENT_MARKGLUEPOINT,
                    SDREVENT_BEGMARK,
                    SDREVENT_BEGINSOBJPOINT,
                    SDREVENT_ENDINSOBJPOINT,
                    SDREVENT_BEGINSGLUEPOINT,
                    SDREVENT_BEGDRAGHELPLINE,
                    SDREVENT_BEGDRAGOBJ,
                    SDREVENT_BEGCREATEOBJ,
                    SDREVENT_BEGMACROOBJ,
                    SDREVENT_BEGTEXTEDIT,
                    SDREVENT_ENDMARK,
                    SDREVENT_BRKMARK,
                    SDREVENT_EXECUTEURL};

#define SDRMOUSEBUTTONDOWN 1
#define SDRMOUSEMOVE       2
#define SDRMOUSEBUTTONUP   3

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@ @@ @@ @@@@@ @@   @@
//  @@ @@ @@ @@    @@   @@
//  @@ @@ @@ @@    @@ @ @@
//  @@@@@ @@ @@@@  @@@@@@@
//   @@@  @@ @@    @@@@@@@
//   @@@  @@ @@    @@@ @@@
//    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrView: public SdrCreateView
{
    friend class				SdrPageView;

    unsigned					bNoExtendedMouseDispatcher : 1;
    unsigned					bNoExtendedKeyDispatcher : 1;
    unsigned					bNoExtendedCommandDispatcher : 1;
    unsigned					bTextEditOnObjectsWithoutTextIfTextTool : 1;

protected:
    // Abgeleitete Klassen ueberladen lediglich die Methoden ReadData() und
    // WriteData(). Diese werden von den Streamoperatoren der View gerufen.
    // Wichtig ist, dass die ueberladen Methoden der abgeleiteten Klasse
    // als erstes die Methode der Basisklasse rufen, da sonst der CharSet
    // am Stream nicht korrekt gesetzt wird, ...
    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

public:
    TYPEINFO();
    SdrView(SdrModel* pModel1, OutputDevice* pOut);
    SdrView(SdrModel* pModel1);
    virtual ~SdrView();

    // Default sind alle Dispatcher aktiviert. Will die App z.B. fuer
    // Sonderbehandlungen im MouseDispatcher eingreifen, so muss sie
    // den erweiterten MouseDispather mit unten stehender Methode deaktivieren
    // und selbst nachimplementieren. Beispiel fuer MouseButtonDown:
    //      SdrHitKind eHit=pSdrView->PickAnything(rMEvt,SDRMOUSEBUTTONDOWN,aVEvt);
    //      ... hier Applikationsspezifischer Eingriff ...
    //      pSdrView->DoMouseEvent(aVEvt);
    //      SetPointer(GetPreferedPointer(...))
    //      CaptureMouse(...)
    void EnableExtendedMouseEventDispatcher(BOOL bOn) { bNoExtendedMouseDispatcher = !bOn; }
    BOOL IsExtendedMouseEventDispatcherEnabled() const { return bNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(BOOL bOn) { bNoExtendedKeyDispatcher=!bOn; }
    BOOL IsExtendedKeyInputDispatcherEnabled() const { return bNoExtendedKeyDispatcher; }

    void EnableExtendedCommandEventDispatcher(BOOL bOn) { bNoExtendedCommandDispatcher=!bOn; }
    BOOL IsExtendedCommandEventDispatcherEnabled() const { return bNoExtendedCommandDispatcher; }

    void EnableTextEditOnObjectsWithoutTextIfTextTool(BOOL bOn) { bTextEditOnObjectsWithoutTextIfTextTool=bOn; }
    BOOL IsEnableTextEditOnObjectsWithoutTextIfTextToolEnabled() const { return bTextEditOnObjectsWithoutTextIfTextTool; }


    // Die Methoden beruecksichtigen den jeweiligen Kontex:
    // - Einfaches Zeichnen
    // - Punktbearbeitungs-Mode
    // - Klebepunkt-Editmode
    // - TextEdit
    // - ... to be continued
    BOOL HasMarked() const { return HasMarkedObj(); }
    void UnmarkAll();


    virtual void DeleteMarked();

    // Strings werden beim rausstreamen in den am Stream eingestellten
    // StreamCharSet konvertiert.
    // Abgeleitete Klassen mit eigenen persistenten Membern ueberladen
    // die virtuellen Methoden ReadData() und WriteData().
    friend SvStream& operator<<(SvStream& rOut, const SdrView& rView);

    // Wenn das Model im Stream in einem fremden CharSet vorliegt
    // wird beim einstreamen implizit auf den SystemCharSet konvertiert.
    // Abgeleitete Klassen mit eigenen persistenten Membern ueberladen
    // die virtuellen Methoden ReadData() und WriteData().
    friend SvStream& operator>>(SvStream& rIn, SdrView& rView);


    virtual void onAccessibilityOptionsChanged();
};

}//end of namespace binfilter
#endif //_SVDVIEW_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Die App macht sich zunaechst ein SdrModel.
// Anschliessend oeffnet sie ein Win und erzeugt dann eine SdrView.
// An der SdrView meldet sie dann mit der Methode ShowPage() eine Seite an.
// Eine SdrView kann in beliebig vielen Fenstern gleichzeitig angezeigt werden.
// Intern:
// Eine SdrView kann beliebig viele Seiten gleichzeitig anzeigen. Seiten
// werden an- und abgemeldet mit ShowPage()/HidePage(). Fuer jede angemeldete
// Seite wird eine SdrPageView-Instanz im Container aPages angelegt. Bei
// gleichzeitiger Anzeige mehrerer Seiten ist darauf zu achten, dass der Offset-
// Parameter von ShowPage() der Seitengroesse angepasst ist, da sich sonst die
// Seiten ueberlappen koennten.
//
// Elementare Methoden:
// ~~~~~~~~~~~~~~~~~~~~
//   Einfache Events:
//   ~~~~~~~~~~~~~~~~
//     BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);
//     BOOL MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
//     BOOL MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
//     BOOL MouseMove(const MouseEvent& rMEvt, Window* pWin);
//     BOOL Command(const CommandEvent& rCEvt, Window* pWin);
//
//   Exchange (Clipboard derzeit noch ohne SdrPrivateData):
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     BOOL Cut(ULONG nFormat=SDR_ANYFORMAT);
//     BOOL Yank(ULONG nFormat=SDR_ANYFORMAT);
//     BOOL Paste(Window* pWin=NULL, ULONG nFormat=SDR_ANYFORMAT);
//
//   SfxItems:
//   ~~~~~~~~~
//     BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;
//     BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll);
//     SfxStyleSheet* GetStyleSheet() const;
//     BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
//
///////////////////////////////////////////////////////////////////////////////////////////////// */

