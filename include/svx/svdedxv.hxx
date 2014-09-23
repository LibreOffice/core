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

#ifndef INCLUDED_SVX_SVDEDXV_HXX
#define INCLUDED_SVX_SVDEDXV_HXX

#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdglev.hxx>

#include <svx/selectioncontroller.hxx>

#include <boost/shared_ptr.hpp>

class SdrOutliner;
class OutlinerView;
class EditStatus;
class EditFieldInfo;
class ImpSdrEditPara;
struct PasteOrDropInfos;
class SdrUndoManager;

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace sdr {
    class SelectionController;
}

enum SdrEndTextEditKind {SDRENDTEXTEDIT_UNCHANGED, // Textobjekt unveraendert
                         SDRENDTEXTEDIT_CHANGED,   // Textobjekt wurde geaendert
                         SDRENDTEXTEDIT_DELETED,   // Textobjekt implizit geloescht
                         SDRENDTEXTEDIT_SHOULDBEDELETED}; // Fuer Writer: Textobjekt sollte geloescht werden


// - Allgemeines Edit fuer objektspeziefische Eigenschaften
// - Textedit fuer alle vom SdrTextObj abgeleiteten Zeichenobjekte
// - Macromodus


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
    vcl::Window*                     pTextEditWin;          // passendes Win zu pTextEditOutlinerView
    vcl::Cursor*                pTextEditCursorMerker; // Zum Restaurieren des Cursors am jeweiligen Win
    ImpSdrEditPara*             pEditPara; // Da hau' ich erstmal alles rein um kompatibel zu bleiben...
    SdrObject*                  pMacroObj;
    SdrPageView*                pMacroPV;
    vcl::Window*                     pMacroWin;

    Rectangle                   aTextEditArea;
    Rectangle                   aMinTextEditArea;
    Link                        aOldCalcFieldValueLink; // Zum rufen des alten Handlers
    Point                       aMacroDownPos;

    sal_uInt16                  nMacroTol;

    bool                        bTextEditDontDelete : 1;   // Outliner und View bei SdrEndTextEdit nicht deleten (f. Rechtschreibpruefung)
    bool                        bTextEditOnlyOneView : 1;  // Nur eine OutlinerView (f. Rechtschreibpruefung)
    bool                        bTextEditNewObj : 1;       // Aktuell editiertes Objekt wurde gerade neu erzeugt
    bool                        bQuickTextEditMode : 1;    // persistent(->CrtV). Default=TRUE
    bool                        bMacroMode : 1;            // persistent(->CrtV). Default=TRUE
    bool                        bMacroDown : 1;

    rtl::Reference< sdr::SelectionController > mxSelectionController;
    rtl::Reference< sdr::SelectionController > mxLastSelectionController;

private:
    ::svl::IUndoManager* mpOldTextEditUndoManager;

    SVX_DLLPRIVATE void ImpClearVars();

protected:
    // central method to get an SdrUndoManager for enhanced TextEdit. Default will
    // try to return a dynamic_casted GetModel()->GetSdrUndoManager(). Applications
    // which want to use this feature will need to overload this virtual method,
    // provide their document UndoManager and derive it from SdrUndoManager.
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const;

    OutlinerView* ImpFindOutlinerView(vcl::Window* pWin) const;

    // Eine neue OutlinerView auf dem Heap anlegen und alle erforderlichen Parameter setzen.
    // pTextEditObj, pTextEditPV und pTextEditOutliner muessen initiallisiert sein.
    OutlinerView* ImpMakeOutlinerView(vcl::Window* pWin, bool bNoPaint, OutlinerView* pGivenView) const;
    void ImpPaintOutlinerView(OutlinerView& rOutlView, const Rectangle& rRect, OutputDevice& rTargetDevice) const;
    void ImpInvalidateOutlinerView(OutlinerView& rOutlView) const;

    // Feststellen, ob der gesamte Text markiert ist. Liefert auch sal_True wenn
    // kein Text vorhanden ist.
    bool ImpIsTextEditAllSelected() const;
    void ImpMakeTextCursorAreaVisible();

    // Handler fuer AutoGrowing Text bei aktivem Outliner
    DECL_LINK(ImpOutlinerStatusEventHdl,EditStatus*);
    DECL_LINK(ImpOutlinerCalcFieldValueHdl,EditFieldInfo*);

    // link for EndTextEditHdl
    DECL_LINK(EndTextEditHdl, SdrUndoManager*);

    void ImpMacroUp(const Point& rUpPos);
    void ImpMacroDown(const Point& rDownPos);

       DECL_LINK( BeginPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK( EndPasteOrDropHdl, PasteOrDropInfos* );

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrObjEditView();

public:

    // used to call the old ImpPaintOutlinerView. Will be replaced when the
    // outliner will be displayed on the overlay in edit mode.
    void TextEditDrawing(SdrPaintWindow& rPaintWindow) const;

    // Actionhandling fuer Macromodus
    virtual bool IsAction() const SAL_OVERRIDE;
    virtual void MovAction(const Point& rPnt) SAL_OVERRIDE;
    virtual void EndAction() SAL_OVERRIDE;
    virtual void BrkAction() SAL_OVERRIDE;
    virtual void BckAction() SAL_OVERRIDE;
    virtual void TakeActionRect(Rectangle& rRect) const SAL_OVERRIDE;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;
    virtual void ModelHasChanged() SAL_OVERRIDE;


    // TextEdit ueber einen Outliner

    // QuickTextEditMode bedeutet, dass Objekte mit Text sofort beim Anklicken
    // editiert werden sollen. Default=TRUE. Persistent.
    void SetQuickTextEditMode(bool bOn) { bQuickTextEditMode=bOn; }
    bool IsQuickTextEditMode() const { return bQuickTextEditMode; }

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

    virtual bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = 0L, vcl::Window* pWin = 0L, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = 0L, OutlinerView* pGivenOutlinerView = 0L,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true);
    // bDontDeleteReally ist ein Spezialparameter fuer den Writer.
    // Ist dieses Flag gesetzt, dann wird ein evtl. leeres Textobjekt
    // nicht geloescht. Stattdessen gibt es dann einen Returncode
    // SDRENDTEXTEDIT_SHOULDBEDELETED (anstelle von SDRENDTEXTEDIT_BEDELETED)
    // der besagt, dass das Objekt geloescht werden sollte.
    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false);
    virtual bool IsTextEdit() const SAL_OVERRIDE;

    // Diese Methode liefert sal_True, wenn der Punkt rHit innerhalb der
    // des Objektbereichs oder der OutlinerView liegt.
    bool IsTextEditHit(const Point& rHit, short nTol) const;

    // Diese Methode liefert sal_True, wenn der Punkt rHit innerhalb des
    // Handle-dicken Rahmens liegt, der die OutlinerView bei TextFrames
    // umschliesst.
    bool IsTextEditFrameHit(const Point& rHit) const;

    // Bei aktiver Selektion, also zwischen MouseButtonDown und
    // MouseButtonUp liefert diese Methode immer TRUE.
    bool IsTextEditInSelectionMode() const;

    // Wer das z.Zt. im TextEdit befindliche Objekt braucht:
    SdrObject* GetTextEditObject() const { return mxTextEditObj.get(); }

    // info about TextEditPageView. Default is 0L.
    virtual SdrPageView* GetTextEditPageView() const SAL_OVERRIDE;

    // Das aktuelle Win des Outliners
    vcl::Window* GetTextEditWin() const { return pTextEditWin; }
    void SetTextEditWin(vcl::Window* pWin);

    // An den hier abgeholten Outliner kann man schliesslich
    // Events versenden, Attribute setzen, Cut/Copy/Paste rufen,
    // Undo/Redo rufen, etc.
    const SdrOutliner* GetTextEditOutliner() const { return pTextEditOutliner; }
    SdrOutliner* GetTextEditOutliner() { return pTextEditOutliner; }
    const OutlinerView* GetTextEditOutlinerView() const { return pTextEditOutlinerView; }
    OutlinerView* GetTextEditOutlinerView() { return pTextEditOutlinerView; }

    virtual bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool Command(const CommandEvent& rCEvt, vcl::Window* pWin) SAL_OVERRIDE;

    // #97766# make virtual to change implementation e.g. for SdOutlineView
    virtual sal_uInt16 GetScriptType() const;

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    // Intern: Beim Splitteraufziehen neue OutlinerView...
    virtual void AddWindowToPaintView(OutputDevice* pNewWin) SAL_OVERRIDE;
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin) SAL_OVERRIDE;

    sal_uInt16 GetSelectionLevel() const;



    // Object-MacroModus (z.B. Rect als Button oder sowas):

    // Persistent. Default TRUE. SvDraw wertet das Flag u.a. bei
    // SdrView::GetPreferredPointer() aus. Hat nur Wirkung, wenn das Dokument
    // Draw-Objekte mit Macrofunktionalitaet hat (SdrObject::HasMacro()==sal_True).
    void SetMacroMode(bool bOn) { bMacroMode=bOn; }
    bool IsMacroMode() const { return bMacroMode; }
    bool BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin);
    bool BegMacroObj(const Point& rPnt, SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin) { return BegMacroObj(rPnt,-2,pObj,pPV,pWin); }
    void MovMacroObj(const Point& rPnt);
    void BrkMacroObj();
    bool EndMacroObj();
    bool IsMacroObj() const { return pMacroObj!=NULL; }
    bool IsMacroObjDown() const { return bMacroDown; }

    /** fills the given any with a XTextCursor for the current text selection.
        Leaves the any untouched if there currently is no text selected */
    void getTextSelection( ::com::sun::star::uno::Any& rSelection );

    virtual void MarkListHasChanged() SAL_OVERRIDE;

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

#endif // INCLUDED_SVX_SVDEDXV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
