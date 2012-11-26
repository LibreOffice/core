/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDEDXV_HXX
#define _SVDEDXV_HXX

#include <rtl/ref.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdglev.hxx>
#include <svx/selectioncontroller.hxx>
#include <boost/shared_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrOutliner;
class OutlinerView;
class EditStatus;
class EditFieldInfo;
class ImpSdrEditPara;
struct PasteOrDropInfos;
class SdrUndoManager;

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace sdr { class SelectionController; }

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines

enum SdrEndTextEditKind {SDRENDTEXTEDIT_UNCHANGED, // Textobjekt unveraendert
                         SDRENDTEXTEDIT_CHANGED,   // Textobjekt wurde geaendert
                         SDRENDTEXTEDIT_DELETED,   // Textobjekt implizit geloescht
                         SDRENDTEXTEDIT_SHOULDBEDELETED}; // Fuer Writer: Textobjekt sollte geloescht werden

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjEditView: public SdrGlueEditView
{
protected:
    // TextEdit
    SdrObjectWeakRef            mxTextEditObj;          // Aktuell im TextEdit befindliches Obj

    SdrOutliner*                mpTextEditOutliner;     // Na eben der Outliner fuers TextEdit
    OutlinerView*               mpTextEditOutlinerView; // die aktuelle View des Outliners
    Window*                     mpTextEditWin;          // passendes Win zu mpTextEditOutlinerView
    Cursor*                     mpTextEditCursorMerker; // Zum Restaurieren des Cursors am jeweiligen Win

    SdrObject*                  mpMacroObj;
    Window*                     mpMacroWin;

    basegfx::B2DRange           maTextEditArea;
    basegfx::B2DRange           maMinTextEditArea;
    Link                        maOldCalcFieldValueLink; // Zum rufen des alten Handlers
    basegfx::B2DPoint           maMacroDownPos;
    sal_uInt16                  mnMacroTol;

    sal_uInt16                  nMacroTol;

    /// bitfield
    bool                        mbTextEditDontDelete : 1;   // Outliner und View bei SdrEndTextEdit nicht deleten (f. Rechtschreibpruefung)
    bool                        mbTextEditOnlyOneView : 1;  // Nur eine OutlinerView (f. Rechtschreibpruefung)
    bool                        mbTextEditNewObj : 1;       // Aktuell editiertes Objekt wurde gerade neu erzeugt
    bool                        mbQuickTextEditMode : 1;    // persistent(->CrtV). Default=true
    bool                        mbMacroDown : 1;

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

    OutlinerView* ImpFindOutlinerView(Window* pWin) const;

    // Eine neue OutlinerView auf dem Heap anlegen und alle erforderlichen Parameter setzen.
    // pTextEditObj und mpTextEditOutliner muessen initiallisiert sein.
    OutlinerView* ImpMakeOutlinerView(Window* pWin, bool bNoPaint, OutlinerView* pGivenView) const;
    void ImpPaintOutlinerView(OutlinerView& rOutlView, const basegfx::B2DRange& rRange, OutputDevice& rTargetDevice) const;
    void ImpInvalidateOutlinerView(OutlinerView& rOutlView) const;

    // Feststellen, ob der gesamte Text markiert ist. Liefert auch true wenn
    // kein Text vorhanden ist.
    bool ImpIsTextEditAllSelected() const;
    void ImpMakeTextCursorAreaVisible();

    // Handler fuer AutoGrowing Text bei aktivem Outliner
    DECL_LINK(ImpOutlinerStatusEventHdl,EditStatus*);
    DECL_LINK(ImpOutlinerCalcFieldValueHdl,EditFieldInfo*);

    void ImpMacroUp(const basegfx::B2DPoint& rUpPos);
    void ImpMacroDown(const basegfx::B2DPoint& rDownPos);

    // link for EndTextEditHdl
    DECL_LINK(EndTextEditHdl, SdrUndoManager*);
       DECL_LINK( BeginPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK( EndPasteOrDropHdl, PasteOrDropInfos* );

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrObjEditView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrObjEditView();

    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfos );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfos );

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used to call the old ImpPaintOutlinerView. Will be replaced when the
    // outliner will be displayed on the overlay in edit mode.
    void TextEditDrawing(SdrPaintWindow& rPaintWindow) const;

    // Actionhandling fuer Macromodus
    virtual bool IsAction() const;
    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    virtual void EndAction();
    virtual void BrkAction();
    virtual void BckAction();
    virtual basegfx::B2DRange TakeActionRange() const;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
    virtual void LazyReactOnObjectChanges();

    //************************************************************************
    // TextEdit ueber einen Outliner
    //************************************************************************
    // QuickTextEditMode bedeutet, dass Objekte mit Text sofort beim Anklicken
    // editiert werden sollen. Default=true. Persistent.
    void SetQuickTextEditMode(bool bOn) { if(mbQuickTextEditMode != bOn) mbQuickTextEditMode = bOn; }
    bool IsQuickTextEditMode() const { return mbQuickTextEditMode; }

    // Starten des TextEditMode. Ist pWin==NULL, wird das erste an der View
    // angemeldete Win verwendet.
    // Der Cursor des Fensters an dem Editiert wird wird bei
    // SdrBeginTextEdit() gemerkt und bei SdrEndTextEdit() wieder restauriert.
    // Die App muss sicherstellen, das die zum Zeitpunkt des BegEdit am
    // Windows angemeldete Cursorinstanz beim SdrEndTextEdit noch gueltig ist.
    // Ueber den Parameter pEditOutliner kann die Applikation einen eigenen
    // Outliner vorgeben, der zum Editieren verwendet wird. Dieser gehoert
    // nach Aufruf von SdrBeginTextEdit der SdrObjEditView und wird von dieser
    // spaeter via delete zerstoert (falls bDontDeleteOutliner=false). Die
    // SdrObjEditView setzt dann das Modusflag (EditEngine/Outliner) an
    // dieser Instanz und ausserdem auch den StatusEventHdl.
    // Ebenso kann eine spezifische OutlinerView vorgegeben werden.

    virtual bool SdrBeginTextEdit(SdrObject* pObj, ::Window* pWin = 0, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = 0, OutlinerView* pGivenOutlinerView = 0,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true);
    // bDontDeleteReally ist ein Spezialparameter fuer den Writer.
    // Ist dieses Flag gesetzt, dann wird ein evtl. leeres Textobjekt
    // nicht geloescht. Stattdessen gibt es dann einen Returncode
    // SDRENDTEXTEDIT_SHOULDBEDELETED (anstelle von SDRENDTEXTEDIT_BEDELETED)
    // der besagt, dass das Objekt geloescht werden sollte.
    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false);
    virtual bool IsTextEdit() const;

    // true=Es wird ein Textrahmen (OBJ_TEXT,OBJ_OUTLINETEXT,...) editiert
    // ansonsten handelt es sich um ein beschriftetes Zeichenobjekt, an dem
    // der Text ja bekanntlich hor. und vert. zentriert wird.
    bool IsTextEditFrame() const;

    // Diese Methode liefert true, wenn der Punkt rHit innerhalb der
    // des Objektbereichs oder der OutlinerView liegt.
    bool IsTextEditHit(const basegfx::B2DPoint& rHit, double fTol) const;

    // Diese Methode liefert true, wenn der Punkt rHit innerhalb des
    // Handle-dicken Rahmens liegt, der die OutlinerView bei TextFrames
    // umschliesst.
    bool IsTextEditFrameHit(const basegfx::B2DPoint& rHit) const;

    // Bei aktiver Selektion, also zwischen MouseButtonDown und
    // MouseButtonUp liefert diese Methode immer true.
    bool IsTextEditInSelectionMode() const;

    // Folgende Methode addiert einen passenden Offset zum MouseEvent
    // um diesen an den Outliner weiterzureichen.
    void AddTextEditOfs(MouseEvent& rMEvt) const;

    // Wer das z.Zt. im TextEdit befindliche Objekt braucht:
    SdrObject* GetTextEditObject() const { return mxTextEditObj.get(); }

    // Das aktuelle Win des Outliners
    Window* GetTextEditWin() const { return mpTextEditWin; }
    void SetTextEditWin(Window* pWin);

    // An den hier abgeholten Outliner kann man schliesslich
    // Events versenden, Attribute setzen, Cut/Copy/Paste rufen,
    // Undo/Redo rufen, etc.
    const SdrOutliner* GetTextEditOutliner() const { return mpTextEditOutliner; }
    SdrOutliner* GetTextEditOutliner() { return mpTextEditOutliner; }
    const OutlinerView* GetTextEditOutlinerView() const { return mpTextEditOutlinerView; }
    OutlinerView* GetTextEditOutlinerView() { return mpTextEditOutlinerView; }

    virtual bool KeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
    virtual bool Command(const CommandEvent& rCEvt, Window* pWin);
    bool Cut(sal_uInt32 nFormat=SDR_ANYFORMAT);
    bool Yank(sal_uInt32 nFormat=SDR_ANYFORMAT);
    bool Paste(Window* pWin=NULL, sal_uInt32 nFormat=SDR_ANYFORMAT);

    // #97766# make virtual to change implementation e.g. for SdOutlineView
    virtual sal_uInt16 GetScriptType() const;

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    // Intern: Beim Splitteraufziehen neue OutlinerView...
    virtual void AddWindowToPaintView(OutputDevice* pNewWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    //************************************************************************
    // Object-MacroModus (z.B. Rect als Button oder sowas):
    //************************************************************************
    // Persistent. Default true. SvDraw wertet das Flag u.a. bei
    // SdrView::GetPreferedPointer() aus. Hat nur Wirkung, wenn das Dokument
    // Draw-Objekte mit Macrofunktionalitaet hat (SdrObject::HasMacro()==true).
    bool BegMacroObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, Window* pWin);
    void MovMacroObj(const basegfx::B2DPoint& rPnt);
    void BrkMacroObj();
    bool EndMacroObj();
    bool IsMacroObj() const { return (0 != mpMacroObj); }
    bool IsMacroObjDown() const { return mbMacroDown; }

    /** fills the given any with a XTextCursor for the current text selection.
        Leaves the any untouched if there currently is no text selected */
    void getTextSelection( ::com::sun::star::uno::Any& rSelection );

    virtual void handleSelectionChange();

    rtl::Reference< sdr::SelectionController > getSelectionController() const { return mxSelectionController; }

    /** returns true if the shape supports format paint brush operation */
    virtual bool SupportsFormatPaintbrush(const SdrObject& rSdrObject) const;

    /** returns a format paint brush set from the current selection */
    virtual bool TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  );

    /** applies a format paint brush set from the current selection.
        if bNoCharacterFormats is true, no character attributes are changed.
        if bNoParagraphFormats is true, no paragraph attributes are changed.
    */
    virtual void ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats );

    /** helper function for selections with multiple SdrText for one SdrTextObj (f.e. tables ) */
    void ApplyFormatPaintBrushToText( SfxItemSet& rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats );
};

#endif //_SVDEDXV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
