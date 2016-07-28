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
#include <svl/languageoptions.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdglev.hxx>
#include <svx/selectioncontroller.hxx>
#include <memory>

class SdrOutliner;
class OutlinerView;
class EditStatus;
class EditFieldInfo;
class ImpSdrEditPara;
struct PasteOrDropInfos;
class SdrUndoManager;
class TextChainCursorManager;
class SfxViewShell;

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace sdr {
    class SelectionController;
}

enum SdrEndTextEditKind {SDRENDTEXTEDIT_UNCHANGED, // textobject unchanged
                         SDRENDTEXTEDIT_CHANGED,   // textobject changed
                         SDRENDTEXTEDIT_DELETED,   // textobject implicitly deleted
                         SDRENDTEXTEDIT_SHOULDBEDELETED}; // for writer: textobject should be deleted


// - general edit for objectspecific properties
// - textedit for all drawobjects, inherited from SdrTextObj
// - macromod


class SVX_DLLPUBLIC SdrObjEditView: public SdrGlueEditView
{
    friend class                SdrPageView;
    friend class                ImpSdrEditPara;

protected:
    // TextEdit
    SdrObjectWeakRef            mxTextEditObj;         // current object in TextEdit
    SdrPageView*                pTextEditPV;
    SdrOutliner*                pTextEditOutliner;     // outliner for the TextEdit
    OutlinerView*               pTextEditOutlinerView; // current view of the outliners
    VclPtr<vcl::Window>         pTextEditWin;          // matching window to pTextEditOutlinerView
    vcl::Cursor*                pTextEditCursorMerker; // to restore the cursor in each window
    ImpSdrEditPara*             pEditPara;             // trash bin for everything else to stay compatible
    SdrObject*                  pMacroObj;
    SdrPageView*                pMacroPV;
    VclPtr<vcl::Window>         pMacroWin;

    Rectangle                   aTextEditArea;
    Rectangle                   aMinTextEditArea;
    Link<EditFieldInfo*,void>   aOldCalcFieldValueLink; // for call the old handler
    Point                       aMacroDownPos;

    sal_uInt16                  nMacroTol;

    bool                        bTextEditDontDelete : 1;   // do not delete outliner and view of SdrEndTextEdit (f. spellchecking)
    bool                        bTextEditOnlyOneView : 1;  // a single OutlinerView (f. spellchecking)
    bool                        bTextEditNewObj : 1;       // current edited object was just recreated
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
    // which want to use this feature will need to override this virtual method,
    // provide their document UndoManager and derive it from SdrUndoManager.
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const;

    void ImpMoveCursorAfterChainingEvent(TextChainCursorManager *pCursorManager);
    TextChainCursorManager *ImpHandleMotionThroughBoxesKeyInput(const KeyEvent& rKEvt, vcl::Window* pWin, bool *bOutHandled);


    OutlinerView* ImpFindOutlinerView(vcl::Window* pWin) const;

    // Create a new OutlinerView at the heap and initialize all required parameters.
    // pTextEditObj, pTextEditPV and pTextEditOutliner have to be initialized
    OutlinerView* ImpMakeOutlinerView(vcl::Window* pWin, bool bNoPaint, OutlinerView* pGivenView, SfxViewShell* pViewShell = nullptr) const;
    void ImpPaintOutlinerView(OutlinerView& rOutlView, const Rectangle& rRect, OutputDevice& rTargetDevice) const;
    void ImpInvalidateOutlinerView(OutlinerView& rOutlView) const;

    // Chaining
    void ImpChainingEventHdl();
    DECL_LINK_TYPED(ImpAfterCutOrPasteChainingEventHdl, LinkParamNone*, void);


    // Check if the whole text is selected.
    // Still returns sal_True if there is no text present.
    bool ImpIsTextEditAllSelected() const;
    void ImpMakeTextCursorAreaVisible();

    // handler for AutoGrowing text with active Outliner
    DECL_LINK_TYPED(ImpOutlinerStatusEventHdl, EditStatus&, void);
    DECL_LINK_TYPED(ImpOutlinerCalcFieldValueHdl, EditFieldInfo*, void);

    // link for EndTextEditHdl
    DECL_LINK_TYPED(EndTextEditHdl, SdrUndoManager*, void);

    void ImpMacroUp(const Point& rUpPos);
    void ImpMacroDown(const Point& rDownPos);

    DECL_LINK_TYPED( BeginPasteOrDropHdl, PasteOrDropInfos*, void );
    DECL_LINK_TYPED( EndPasteOrDropHdl, PasteOrDropInfos*, void );

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut = nullptr);
    virtual ~SdrObjEditView();

public:

    // used to call the old ImpPaintOutlinerView. Will be replaced when the
    // outliner will be displayed on the overlay in edit mode.
    void TextEditDrawing(SdrPaintWindow& rPaintWindow) const;

    // Actionhandling for macromod
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BrkAction() override;
    virtual void BckAction() override;
    virtual void TakeActionRect(Rectangle& rRect) const override;

    SdrPageView* ShowSdrPage(SdrPage* pPage) override;
    void HideSdrPage() override;
    /// Get access to the view shell owning this draw view, if any.
    virtual SfxViewShell* GetSfxViewShell() const;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    virtual void ModelHasChanged() override;


    // TextEdit over an outliner

    // QuickTextEditMode = edit the text straight after selection. Default=TRUE. Persistent.
    void SetQuickTextEditMode(bool bOn) { bQuickTextEditMode=bOn; }
    bool IsQuickTextEditMode() const { return bQuickTextEditMode; }

    // Start the TextEditMode. If pWin==NULL, use the first window, which is logged at the View.
    // The cursor of the currently edited window is stored with SdrBeginTextEdit()
    // and restored with SdrEndTextEdit().
    // The app has to ensure, that the BegEdit of the window logged cursor is still valid,
    // when SdrEndTextEdit is called.
    // With the parameter pEditOutliner, the app has the possibility to specify his own outliner,
    // which is used for editing. After the SdrBeginTextEdit call, the outliner belongs to
    // SdrObjEditView, and is also later destroyed by this via delete (if bDontDeleteOutliner=sal_False).
    // Afterwards the SdrObjEditView sets the modflag (EditEngine/Outliner) at this instance and also the
    // StatusEventHdl.
    // Similarly a specific OutlinerView can be specified.

    virtual bool SdrBeginTextEdit(SdrObject* pObj, SdrPageView* pPV = nullptr, vcl::Window* pWin = nullptr, bool bIsNewObj = false,
        SdrOutliner* pGivenOutliner = nullptr, OutlinerView* pGivenOutlinerView = nullptr,
        bool bDontDeleteOutliner = false, bool bOnlyOneView = false, bool bGrabFocus = true);
    // bDontDeleteReally is a special parameter for writer
    // If this flag is set, then a maybe empty textobject is not deleted.
    // Instead you get a return code SDRENDTEXTEDIT_SHOULDBEDELETED
    // (in place of SDRENDTEXTEDIT_BEDELETED), which says, the obj should be
    // deleted.
    virtual SdrEndTextEditKind SdrEndTextEdit(bool bDontDeleteReally = false);
    virtual bool IsTextEdit() const override;

    // This method returns sal_True, if the point rHit is inside the
    // objectspace or the OutlinerView.
    bool IsTextEditHit(const Point& rHit) const;

    // This method returns sal_True, if the point rHit is inside the
    // handle-thick frame, which surrounds the OutlinerView at TextFrames.
    bool IsTextEditFrameHit(const Point& rHit) const;

    // At active selection, between MouseButtonDown and
    // MouseButtonUp, this method always returns TRUE.
    bool IsTextEditInSelectionMode() const;

    // If sb needs the object out of the TextEdit:
    SdrObject* GetTextEditObject() const { return mxTextEditObj.get(); }

    // info about TextEditPageView. Default is 0L.
    virtual SdrPageView* GetTextEditPageView() const override;

    // Current window of the outliners.
    void SetTextEditWin(vcl::Window* pWin);

    // Now at this outliner, events can be send, attributes can be set,
    // call Cut/Copy/Paste, call Undo/Redo, and so on...
    const SdrOutliner* GetTextEditOutliner() const { return pTextEditOutliner; }
    SdrOutliner* GetTextEditOutliner() { return pTextEditOutliner; }
    const OutlinerView* GetTextEditOutlinerView() const { return pTextEditOutlinerView; }
    OutlinerView* GetTextEditOutlinerView() { return pTextEditOutlinerView; }

    virtual bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool Command(const CommandEvent& rCEvt, vcl::Window* pWin) override;

    // #97766# make virtual to change implementation e.g. for SdOutlineView
    virtual SvtScriptType GetScriptType() const;

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    // Intern: at mounting new OutlinerView...
    virtual void AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window* pWindow) override;
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin) override;

    sal_uInt16 GetSelectionLevel() const;


    // Object-MacroModus (e.g. rect as button or sth. like that):

    // Persistent. Default TRUE. SvDraw evaluates the flag e.g. at SdrView::GetPreferredPointer().
    // Has only effect, if the document has draw-objects with macrofunctionality (SdrObject::HasMacro()==sal_True).
    bool IsMacroMode() const { return bMacroMode; }
    bool BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin);
    bool BegMacroObj(const Point& rPnt, SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin) { return BegMacroObj(rPnt,-2,pObj,pPV,pWin); }
    void MovMacroObj(const Point& rPnt);
    void BrkMacroObj();
    bool EndMacroObj();
    bool IsMacroObj() const { return pMacroObj!=nullptr; }

    /** fills the given any with a XTextCursor for the current text selection.
        Leaves the any untouched if there currently is no text selected */
    void getTextSelection( css::uno::Any& rSelection );

    virtual void MarkListHasChanged() override;

    rtl::Reference< sdr::SelectionController > getSelectionController() const { return mxSelectionController; }

    /** returns true if the shape identified by its inventor and identifier supports format paint brush operation */
    static bool SupportsFormatPaintbrush( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier );

    /** returns a format paint brush set from the current selection */
    bool TakeFormatPaintBrush( std::shared_ptr< SfxItemSet >& rFormatSet  );

    /** applies a format paint brush set from the current selection.
        if bNoCharacterFormats is true, no character attributes are changed.
        if bNoParagraphFormats is true, no paragraph attributes are changed.
    */
    void ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats );

    /** helper function for selections with multiple SdrText for one SdrTextObj (f.e. tables ) */
    static void ApplyFormatPaintBrushToText( SfxItemSet& rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats );

protected:
    virtual void OnBeginPasteOrDrop( PasteOrDropInfos* pInfo );
    virtual void OnEndPasteOrDrop( PasteOrDropInfos* pInfo );

};

#endif // INCLUDED_SVX_SVDEDXV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
