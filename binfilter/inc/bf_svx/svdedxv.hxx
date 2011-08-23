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

#ifndef _SVDEDXV_HXX
#define _SVDEDXV_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <bf_svx/svdhdl.hxx>

#include <bf_svx/svdglev.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrOutliner;
class OutlinerView;
class EditStatus;
class EditFieldInfo;
class ImpSdrEditPara;

//************************************************************
//   Defines
//************************************************************

enum SdrEndTextEditKind {SDRENDTEXTEDIT_UNCHANGED, // Textobjekt unveraendert
                         SDRENDTEXTEDIT_CHANGED,   // Textobjekt wurde geaendert
                         SDRENDTEXTEDIT_DELETED,   // Textobjekt implizit geloescht
                         SDRENDTEXTEDIT_SHOULDBEDELETED}; // Fuer Writer: Textobjekt sollte geloescht werden

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@  @@ @@@@@      @@  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@  @@  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//   @@@@  @@@@@   @@@@   @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
// - Allgemeines Edit fuer objektspeziefische Eigenschaften
// - Textedit fuer alle vom SdrTextObj abgeleiteten Zeichenobjekte
// - Macromodus
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrObjEditView: public SdrGlueEditView
{
    friend class				SdrPageView;
    friend class				ImpSdrEditPara;

protected:
    // TextEdit
    SdrObject*					pTextEditObj;          // Aktuell im TextEdit befindliches Obj
    SdrPageView*				pTextEditPV;
    SdrOutliner*				pTextEditOutliner;     // Na eben der Outliner fuers TextEdit
    OutlinerView*				pTextEditOutlinerView; // die aktuelle View des Outliners
    Window*						pTextEditWin;          // passendes Win zu pTextEditOutlinerView
    Cursor*						pTextEditCursorMerker; // Zum Restaurieren des Cursors am jeweiligen Win
    ImpSdrEditPara*				pEditPara; // Da hau' ich erstmal alles rein um kompatibel zu bleiben...
    SdrObject*					pMacroObj;
    SdrPageView*				pMacroPV;
    Window*						pMacroWin;

    Rectangle					aTextEditArea;
    Rectangle					aMinTextEditArea;
    Link						aOldCalcFieldValueLink; // Zum rufen des alten Handlers
    Point						aMacroDownPos;

    USHORT						nMacroTol;

    unsigned					bTextEditDontDelete : 1;   // Outliner und View bei EndTextEdit nicht deleten (f. Rechtschreibpruefung)
    unsigned					bTextEditOnlyOneView : 1;  // Nur eine OutlinerView (f. Rechtschreibpruefung)
    unsigned					bTextEditNewObj : 1;       // Aktuell editiertes Objekt wurde gerade neu erzeugt
    unsigned					bQuickTextEditMode : 1;    // persistent(->CrtV). Default=TRUE
    unsigned					bMacroMode : 1;            // persistent(->CrtV). Default=TRUE
    unsigned					bMacroDown : 1;

private:
    void ImpClearVars();

protected:

    // Eine neue OutlinerView auf dem Heap anlegen und alle erforderlichen Parameter setzen.
    // pTextEditObj, pTextEditPV und pTextEditOutliner muessen initiallisiert sein.

    // Hintergrundfarbe fuer die Outlinerviews bestimmen

    // Feststellen, ob der gesamte Text markiert ist. Liefert auch TRUE wenn
    // kein Text vorhanden ist.

    // Handler fuer AutoGrowing Text bei aktivem Outliner


public:
    SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrObjEditView();

    // Actionhandling fuer Macromodus
    virtual BOOL IsAction() const;
    virtual void BrkAction();

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void ModelHasChanged();

    //************************************************************************
    // TextEdit ueber einen Outliner
    //************************************************************************
    // QuickTextEditMode bedeutet, dass Objekte mit Text sofort beim Anklicken
    // editiert werden sollen. Default=TRUE. Persistent.
    void SetQuickTextEditMode(BOOL bOn) { bQuickTextEditMode=bOn; }
    BOOL IsQuickTextEditMode() const { return bQuickTextEditMode; }

    // Starten des TextEditMode. Ist pWin==NULL, wird das erste an der View
    // angemeldete Win verwendet.
    // Der Cursor des Fensters an dem Editiert wird wird bei
    // BegTextEdit() gemerkt und bei EndTextEdit() wieder restauriert.
    // Die App muss sicherstellen, das die zum Zeitpunkt des BegEdit am
    // Windows angemeldete Cursorinstanz beim EndTextEdit noch gueltig ist.
    // Ueber den Parameter pEditOutliner kann die Applikation einen eigenen
    // Outliner vorgeben, der zum Editieren verwendet wird. Dieser gehoert
    // nach Aufruf von BegTextEdit der SdrObjEditView und wird von dieser
    // spaeter via delete zerstoert (falls bDontDeleteOutliner=FALSE). Die
    // SdrObjEditView setzt dann das Modusflag (EditEngine/Outliner) an
    // dieser Instanz und ausserdem auch den StatusEventHdl.
    // Ebenso kann eine spezifische OutlinerView vorgegeben werden.
    BOOL BegTextEdit(SdrObject* pObj, SdrPageView* pPV=NULL, Window* pWin=NULL,
        SdrOutliner* pGivenOutliner=NULL, OutlinerView* pGivenOutlinerView=NULL,
        BOOL bDontDeleteOutliner=FALSE, BOOL bOnlyOneView=FALSE);
    // bDontDeleteReally ist ein Spezialparameter fuer den Writer.
    // Ist dieses Flag gesetzt, dann wird ein evtl. leeres Textobjekt
    // nicht geloescht. Stattdessen gibt es dann einen Returncode
    // SDRENDTEXTEDIT_SHOULDBEDELETED (anstelle von SDRENDTEXTEDIT_BEDELETED)
    // der besagt, dass das Objekt geloescht werden sollte.
    SdrEndTextEditKind EndTextEdit(BOOL bDontDeleteReally=FALSE);
    BOOL IsTextEdit() const { return pTextEditObj!=NULL; }

    // Folgende Methode addiert einen passenden Offset zum MouseEvent
    // um diesen an den Outliner weiterzureichen.

    // Wer das z.Zt. im TextEdit befindliche Objekt braucht:
    SdrObject* GetTextEditObject() const { return pTextEditObj; }
    SdrPageView* GetTextEditPageView() const { return pTextEditPV; }

    // Das aktuelle Win des Outliners
    Window* GetTextEditWin() const { return pTextEditWin; }

    // An den hier abgeholten Outliner kann man schliesslich
    // Events versenden, Attribute setzen, Cut/Copy/Paste rufen,
    // Undo/Redo rufen, etc.
    const SdrOutliner* GetTextEditOutliner() const { return pTextEditOutliner; }
    SdrOutliner* GetTextEditOutliner() { return pTextEditOutliner; }
    const OutlinerView* GetTextEditOutlinerView() const { return pTextEditOutlinerView; }
    OutlinerView* GetTextEditOutlinerView() { return pTextEditOutlinerView; }

    // Intern: Beim Splitteraufziehen neue OutlinerView...
    virtual void DelWin(OutputDevice* pWin1);

    //************************************************************************
    // Object-MacroModus (z.B. Rect als Button oder sowas):
    //************************************************************************
    // Persistent. Default TRUE. SvDraw wertet das Flag u.a. bei
    // SdrView::GetPreferedPointer() aus. Hat nur Wirkung, wenn das Dokument
    // Draw-Objekte mit Macrofunktionalitaet hat (SdrObject::HasMacro()==TRUE).
    void SetMacroMode(BOOL bOn) { bMacroMode=bOn; }
    BOOL IsMacroMode() const { return bMacroMode; }
    void BrkMacroObj();
    BOOL IsMacroObj() const { return pMacroObj!=NULL; }
    BOOL IsMacroObjDown() const { return bMacroDown; }
};

}//end of namespace binfilter
#endif //_SVDEDXV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
