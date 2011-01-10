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

#include <rtl/ref.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdglev.hxx>

#include <svx/selectioncontroller.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

#include <boost/shared_ptr.hpp>

class SdrOutliner;
class OutlinerView;
class EditStatus;
class EditFieldInfo;
class ImpSdrEditPara;
struct PasteOrDropInfos;

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace sdr {
    class SelectionController;
}

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

class SVX_DLLPUBLIC SdrObjEditView: public SdrGlueEditView
{
    friend class                SdrPageView;
    friend class                ImpSdrEditPara;

protected:
    // TextEdit
    SdrObjectWeakRef            mxTextEditObj;          // Aktuell im TextEdit befindliches Obj
    SdrPageView*                pTextEditPV;
    SdrOutliner*                pTextEditOutliner;     // Na eben der Outliner fuers TextEdit
    OutlinerView*               pTextEditOutlinerView; // die aktuelle View des Outliners
    Window*                     pTextEditWin;          // passendes Win zu pTextEditOutlinerView
    Cursor*                     pTextEditCursorMerker; // Zum Restaurieren des Cursors am jeweiligen Win
    ImpSdrEditPara*             pEditPara; // Da hau' ich erstmal alles rein um kompatibel zu bleiben...
    SdrObject*                  pMacroObj;
    SdrPageView*                pMacroPV;
    Window*                     pMacroWin;

    Rectangle                   aTextEditArea;
    Rectangle                   aMinTextEditArea;
    Link                        aOldCalcFieldValueLink; // Zum rufen des alten Handlers
    Point                       aMacroDownPos;

    sal_uInt16                      nMacroTol;

    unsigned                    bTextEditDontDelete : 1;   // Outliner und View bei SdrEndTextEdit nicht deleten (f. Rechtschreibpruefung)
    unsigned                    bTextEditOnlyOneView : 1;  // Nur eine OutlinerView (f. Rechtschreibpruefung)
    unsigned                    bTextEditNewObj : 1;       // Aktuell editiertes Objekt wurde gerade neu erzeugt
    unsigned                    bQuickTextEditMode : 1;    // persistent(->CrtV). Default=TRUE
    unsigned                    bMacroMode : 1;            // persistent(->CrtV). Default=TRUE
    unsigned                    bMacroDown : 1;

    rtl::Reference< sdr::SelectionController > mxSelectionController;
    rtl::Reference< sdr::SelectionController > mxLastSelectionController;

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    OutlinerView* ImpFindOutlinerView(Window* pWin) const;

    // Eine neue OutlinerView auf dem Heap anlegen und alle erforderlichen Parameter setzen.
    // pTextEditObj, pTextEditPV und pTextEditOutliner muessen initiallisiert sein.
    OutlinerView* ImpMakeOutlinerView(Window* pWin, sal_Bool bNoPaint, OutlinerView* pGivenView) const;
    void ImpPaintOutlinerView(OutlinerView& rOutlView, const Rectangle& rRect) const;
    void ImpInvalidateOutlinerView(OutlinerView& rOutlView) const;

    // Feststellen, ob der gesamte Text markiert ist. Liefert auch sal_True wenn
    // kein Text vorhanden ist.
    sal_Bool ImpIsTextEditAllSelected() const;
    void ImpMakeTextCursorAreaVisible();

    // Handler fuer AutoGrowing Text bei aktivem Outliner
    DECL_LINK(ImpOutlinerStatusEventHdl,EditStatus*);
    DECL_LINK(ImpOutlinerCalcFieldValueHdl,EditFieldInfo*);

    void ImpMacroUp(const Point& rUpPos);
    void ImpMacroDown(const Point& rDownPos);

       DECL_LINK( BeginPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK( EndPasteOrDropHdl, PasteOrDropInfos* );

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrObjEditView();

public:
    // Actionhandling fuer Macromodus
    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BrkAction();
    virtual void BckAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
    virtual void ModelHasChanged();

    //************************************************************************
    // TextEdit ueber einen Outliner
    //************************************************************************
    // QuickTextEditMode bedeutet, dass Objekte mit Text sofort beim Anklicken
    // editiert werden sollen. Default=TRUE. Persistent.
    void SetQuickTextEditMode(sal_Bool bOn) { bQuickTextEditMode=bOn; }
    sal_Bool IsQuickTextEditMode() const { return bQuickTextEditMode; }

    // Starten des TextEditMode. Ist pWin==NULL, wird das erste an der View
    // angemeldete Win verwendet.
    // Der Cursor des Fensters an dem Editiert wird wird bei
    // SdrBeginTextEdit() gemerkt und bei SdrEndTextEdit() wieder restauriert.
    // Die App muss sicherstellen, das die zum Zeitpunkt des BegEdit am
    // Windows angemeldete Cursorinstanz beim SdrEndTextEdit noch gueltig ist.
    // Ueber den Parameter pEditOutliner kann die Applikation einen eigenen
    // Outliner vorgeben, der zum Editieren verwendet wird. Dieser gehoert
    // nach Aufruf von SdrBeginTextEdit der SdrObjEditView und wird von dieser
    // spaeter via delete zerstoert (falls bDontDeleteOutliner=sal_False). Die
    // SdrObjEditView setzt dann das Modusflag (EditEngine/Outliner) an
    // dieser Instanz und ausserdem auch den StatusEventHdl.
    // Ebenso kann eine spezifische OutlinerView vorgegeben werden.

    virtual sal_Bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = 0L, ::Window* pWin = 0L, sal_Bool bIsNewObj = sal_False,
        SdrOutliner* pGivenOutliner = 0L, OutlinerView* pGivenOutlinerView = 0L,
        sal_Bool bDontDeleteOutliner = sal_False, sal_Bool bOnlyOneView = sal_False, sal_Bool bGrabFocus = sal_True);
    // bDontDeleteReally ist ein Spezialparameter fuer den Writer.
    // Ist dieses Flag gesetzt, dann wird ein evtl. leeres Textobjekt
    // nicht geloescht. Stattdessen gibt es dann einen Returncode
    // SDRENDTEXTEDIT_SHOULDBEDELETED (anstelle von SDRENDTEXTEDIT_BEDELETED)
    // der besagt, dass das Objekt geloescht werden sollte.
    virtual SdrEndTextEditKind SdrEndTextEdit(sal_Bool bDontDeleteReally = sal_False);
    virtual bool IsTextEdit() const;

    // sal_True=Es wird ein Textrahmen (OBJ_TEXT,OBJ_OUTLINETEXT,...) editiert
    // ansonsten handelt es sich um ein beschriftetes Zeichenobjekt, an dem
    // der Text ja bekanntlich hor. und vert. zentriert wird.
    sal_Bool IsTextEditFrame() const;

    // Diese Methode liefert sal_True, wenn der Punkt rHit innerhalb der
    // des Objektbereichs oder der OutlinerView liegt.
    sal_Bool IsTextEditHit(const Point& rHit, short nTol) const;

    // Diese Methode liefert sal_True, wenn der Punkt rHit innerhalb des
    // Handle-dicken Rahmens liegt, der die OutlinerView bei TextFrames
    // umschliesst.
    sal_Bool IsTextEditFrameHit(const Point& rHit) const;

    // Bei aktiver Selektion, also zwischen MouseButtonDown und
    // MouseButtonUp liefert diese Methode immer TRUE.
    sal_Bool IsTextEditInSelectionMode() const;

    // Folgende Methode addiert einen passenden Offset zum MouseEvent
    // um diesen an den Outliner weiterzureichen.
    void AddTextEditOfs(MouseEvent& rMEvt) const;

    // Wer das z.Zt. im TextEdit befindliche Objekt braucht:
    SdrObject* GetTextEditObject() const { return mxTextEditObj.get(); }

    // info about TextEditPageView. Default is 0L.
    virtual SdrPageView* GetTextEditPageView() const;

    // Das aktuelle Win des Outliners
    Window* GetTextEditWin() const { return pTextEditWin; }
    void SetTextEditWin(Window* pWin);

    // An den hier abgeholten Outliner kann man schliesslich
    // Events versenden, Attribute setzen, Cut/Copy/Paste rufen,
    // Undo/Redo rufen, etc.
    const SdrOutliner* GetTextEditOutliner() const { return pTextEditOutliner; }
    SdrOutliner* GetTextEditOutliner() { return pTextEditOutliner; }
    const OutlinerView* GetTextEditOutlinerView() const { return pTextEditOutlinerView; }
    OutlinerView* GetTextEditOutlinerView() { return pTextEditOutlinerView; }

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool Command(const CommandEvent& rCEvt, Window* pWin);
    sal_Bool Cut(sal_uIntPtr nFormat=SDR_ANYFORMAT);
    sal_Bool Yank(sal_uIntPtr nFormat=SDR_ANYFORMAT);
    sal_Bool Paste(Window* pWin=NULL, sal_uIntPtr nFormat=SDR_ANYFORMAT);

    // #97766# make virtual to change implementation e.g. for SdOutlineView
    virtual sal_uInt16 GetScriptType() const;

    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(sal_Bool& rOk) const;
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);

    // Intern: Beim Splitteraufziehen neue OutlinerView...
    virtual void AddWindowToPaintView(OutputDevice* pNewWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    //************************************************************************
    // Object-MacroModus (z.B. Rect als Button oder sowas):
    //************************************************************************
    // Persistent. Default TRUE. SvDraw wertet das Flag u.a. bei
    // SdrView::GetPreferedPointer() aus. Hat nur Wirkung, wenn das Dokument
    // Draw-Objekte mit Macrofunktionalitaet hat (SdrObject::HasMacro()==sal_True).
    void SetMacroMode(sal_Bool bOn) { bMacroMode=bOn; }
    sal_Bool IsMacroMode() const { return bMacroMode; }
    sal_Bool BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, Window* pWin);
    sal_Bool BegMacroObj(const Point& rPnt, SdrObject* pObj, SdrPageView* pPV, Window* pWin) { return BegMacroObj(rPnt,-2,pObj,pPV,pWin); }
    void MovMacroObj(const Point& rPnt);
    void BrkMacroObj();
    sal_Bool EndMacroObj();
    sal_Bool IsMacroObj() const { return pMacroObj!=NULL; }
    sal_Bool IsMacroObjDown() const { return bMacroDown; }

    /** fills the given any with a XTextCursor for the current text selection.
        Leaves the any untouched if there currently is no text selected */
    void getTextSelection( ::com::sun::star::uno::Any& rSelection );

    virtual void MarkListHasChanged();

    rtl::Reference< sdr::SelectionController > getSelectionController() const { return mxSelectionController; }

    /** returns true if the shape identified by its inventor and identifier supports format paint brush operation */
    virtual bool SupportsFormatPaintbrush( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const;

    /** returns a format paint brush set from the current selection */
    virtual bool TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  );

    /** applies a format paint brush set from the current selection.
        if bNoCharacterFormats is true, no character attributes are changed.
        if bNoParagraphFormats is true, no paragraph attributes are changed.
    */
    virtual void ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats );

    /** helper function for selections with multiple SdrText for one SdrTextObj (f.e. tables ) */
    void ApplyFormatPaintBrushToText( SfxItemSet& rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats );

protected:
    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos );

};

#endif //_SVDEDXV_HXX

