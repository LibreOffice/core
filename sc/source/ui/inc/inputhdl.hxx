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

#ifndef INCLUDED_SC_SOURCE_UI_INC_INPUTHDL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_INPUTHDL_HXX

#include <global.hxx>
#include <address.hxx>
#include <tools/solar.h>
#include <typedstrdata.hxx>

#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <editeng/svxenum.hxx>
#include "viewdata.hxx"

#include <set>
#include <memory>
#include <vector>

class ScDocument;
class ScTabViewShell;
class ScInputWindow;
class ScPatternAttr;
class ScEditEngineDefaulter;
class EditView;
class EditTextObject;
class ScInputHdlState;
class ScRangeFindList;
class Timer;
class KeyEvent;
class CommandEvent;
class VclWindowEvent;
namespace vcl { class Window; }
struct ReferenceMark;
struct ESelection;

//  ScInputHandler

class ScInputHandler final
{
private:
    VclPtr<ScInputWindow>          pInputWin;

    std::unique_ptr<ScEditEngineDefaulter> mpEditEngine;     ///< Edited data in the sheet (when the user clicks into the sheet, and starts writing there).
    EditView*               pTableView;                 // associated active EditView
    EditView*               pTopView;                   // EditView in the input row

    std::unique_ptr<ScTypedCaseStrSet> pColumnData;
    std::unique_ptr<ScTypedCaseStrSet> pFormulaData;
    std::unique_ptr<ScTypedCaseStrSet> pFormulaDataPara;
    ScTypedCaseStrSet::const_iterator miAutoPosColumn;
    ScTypedCaseStrSet::const_iterator miAutoPosFormula;

    VclPtr<vcl::Window>     pTipVisibleParent;
    void*                   nTipVisible;
    VclPtr<vcl::Window>     pTipVisibleSecParent;
    void*                   nTipVisibleSec;
    OUString                aManualTip;
    OUString                aAutoSearch;

    OUString                aCurrentText;

    OUString                aFormText;                  // for autopilot function
    sal_Int32               nFormSelStart;              // Selection for autopilot function
    sal_Int32               nFormSelEnd;

    sal_Unicode             nCellPercentFormatDecSep;   // 0:= no percent format, else which decimal separator

    sal_uInt16              nAutoPar;                   // autom.parentheses than can be overwritten

    ScAddress               aCursorPos;
    ScInputMode             eMode;
    bool                    bUseTab:1;                    // Scrolling possible
    bool                    bTextValid:1;                 // Text is not in edit engine
    bool                    bModified:1;
    bool                    bSelIsRef:1;
    bool                    bFormulaMode:1;
    bool                    bInRangeUpdate:1;
    bool                    bParenthesisShown:1;
    bool                    bCreatingFuncView:1;
    bool                    bInEnterHandler:1;
    bool                    bCommandErrorShown:1;
    bool                    bInOwnChange:1;

    bool                    bProtected:1;
    bool                    bLastIsSymbol:1;
    bool                    mbDocumentDisposing:1;
    sal_uLong                   nValidation;
    SvxCellHorJustify       eAttrAdjust;

    Fraction                aScaleX;                    // for ref MapMode
    Fraction                aScaleY;

    ScTabViewShell*         pRefViewSh;
    ScTabViewShell*         pActiveViewSh;

    const ScPatternAttr*    pLastPattern;
    std::unique_ptr<SfxItemSet>
                            pEditDefaults;

    std::unique_ptr<ScInputHdlState>
                            pLastState;
    std::unique_ptr<Timer>  pDelayTimer;

    std::unique_ptr<ScRangeFindList>
                            pRangeFindList;

    static bool             bAutoComplete;              // from app options
    static bool             bOptLoaded;
    ::std::set< sal_Unicode >    maFormulaChar;  //fdo 75264

private:
    void            UpdateActiveView();
    void            SyncViews( const EditView* pSourceView = nullptr );
    /**
     * @param cTyped typed character. If 0, look at existing document content
     *               for text or number.
     * @param bInputActivated true if the cell input mode is activated (via
     *                        F2), false otherwise.
     * @param pTopEngine top window input line EditEngine. If not nullptr then
     *                   some default attributes are merged to it from the
     *                   table EditEngine.
     * @return true if the new edit mode has been started.
     */
    bool            StartTable( sal_Unicode cTyped, bool bFromCommand, bool bInputActivated,
                                ScEditEngineDefaulter* pTopEngine );
    void            RemoveSelection();
    void            UpdateFormulaMode();
    static void     InvalidateAttribs();
    void            ImplCreateEditEngine();
    DECL_LINK( DelayTimer, Timer*, void );
    void            GetColData();
    void            UseColData();
    void            NextAutoEntry( bool bBack );
    void            UpdateAdjust( sal_Unicode cTyped );
    void            GetFormulaData();
    void            UseFormulaData();
    void            NextFormulaEntry( bool bBack );
    void            PasteFunctionData();
    void            PasteManualTip();
    EditView*       GetFuncEditView();
    void            RemoveAdjust();
    void            RemoveRangeFinder();
    void            DeleteRangeFinder();
    void            UpdateParenthesis();
    void            UpdateAutoCorrFlag();
    void            ResetAutoPar();
    void            AutoParAdded();
    bool            CursorAtClosingPar();
    void            SkipClosingPar();
    bool            GetFuncName( OUString& aStart, OUString& aResult );  // fdo75264
    void            ShowArgumentsTip( OUString& rSelText );
    DECL_LINK( ModifyHdl, LinkParamNone*, void );
    DECL_LINK( ShowHideTipVisibleParentListener, VclWindowEvent&, void );
    DECL_LINK( ShowHideTipVisibleSecParentListener, VclWindowEvent&, void );

public:
    ScInputHandler(const ScInputHandler&) = delete;
    const ScInputHandler& operator=(const ScInputHandler&) = delete;

                    ScInputHandler();
                    ~ScInputHandler();

    void            SetMode( ScInputMode eNewMode, const OUString* pInitText = nullptr,
                             ScEditEngineDefaulter* pTopEngine = nullptr );
    bool            IsInputMode() const { return (eMode != SC_INPUT_NONE); }
    bool            IsEditMode() const  { return (eMode != SC_INPUT_NONE &&
                                                  eMode != SC_INPUT_TYPE); }
    bool            IsTopMode() const   { return (eMode == SC_INPUT_TOP);  }

    const OUString& GetEditString();
    const OUString& GetFormString() const { return aFormText; }

    const ScAddress& GetCursorPos() const   { return aCursorPos; }

    bool            GetTextAndFields( ScEditEngineDefaulter& rDestEngine );
    void            MergeLanguageAttributes( ScEditEngineDefaulter& rDestEngine ) const;

    bool            KeyInput( const KeyEvent& rKEvt, bool bStartEdit );
    void            EnterHandler( ScEnterMode nBlockMode = ScEnterMode::NORMAL );
    void            CancelHandler();
    void            SetReference( const ScRange& rRef, const ScDocument& rDoc );
    void            AddRefEntry();

    void            InputCommand( const CommandEvent& rCEvt );

    void            InsertFunction( const OUString& rFuncName, bool bAddPar = true );
    void            ClearText();

    void            InputSelection( const EditView* pView );
    void            InputChanged( const EditView* pView, bool bFromNotify );

    void            ViewShellGone(const ScTabViewShell* pViewSh);
    void            SetRefViewShell(ScTabViewShell* pRefVsh) {pRefViewSh=pRefVsh;}

    void            NotifyChange( const ScInputHdlState* pState, bool bForce = false,
                                  ScTabViewShell* pSourceSh = nullptr,
                                  bool bStopEditing = true);
    void            UpdateCellAdjust( SvxCellHorJustify eJust );

    void            ResetDelayTimer(); //BugId 54702

    void            HideTip();
    void            HideTipBelow();
    void            ShowTipCursor();
    void            ShowTip( const OUString& rText );     // at Cursor
    void            ShowTipBelow( const OUString& rText );
    void            ShowFuncList( const ::std::vector< OUString > & rFuncStrVec );

    void            SetRefScale( const Fraction& rX, const Fraction& rY );
    void            UpdateRefDevice();

    EditView*       GetActiveView();
    EditView*       GetTableView()      { return pTableView; }
    EditView*       GetTopView()        { return pTopView; }

    bool            DataChanging( sal_Unicode cTyped = 0, bool bFromCommand = false );
    void            DataChanged( bool bFromTopNotify = false, bool bSetModified = true );

    bool            TakesReturn() const     { return ( nTipVisible != nullptr ); }

    void            SetModified()       { bModified = true; }

    bool            GetSelIsRef() const     { return bSelIsRef; }
    void            SetSelIsRef(bool bSet)  { bSelIsRef = bSet; }

    void            ShowRefFrame();

    ScRangeFindList* GetRangeFindList()     { return pRangeFindList.get(); }

    void            UpdateRange( sal_uInt16 nIndex, const ScRange& rNew );

    // Communication with the autopilot function
    void            InputGetSelection       ( sal_Int32& rStart, sal_Int32& rEnd );
    void            InputSetSelection       ( sal_Int32 nStart, sal_Int32 nEnd );
    void            InputReplaceSelection   ( const OUString& rStr );
    void            InputTurnOffWinEngine();

    bool            IsFormulaMode() const                   { return bFormulaMode; }
    ScInputWindow*  GetInputWindow()                        { return pInputWin; }
    void            SetInputWindow( ScInputWindow* pNew );
    void            StopInputWinEngine( bool bAll );

    bool            IsInEnterHandler() const                { return bInEnterHandler; }
    bool            IsInOwnChange() const                   { return bInOwnChange; }

    bool            IsModalMode( const SfxObjectShell* pDocSh );

    void            ForgetLastPattern();

    void            UpdateSpellSettings( bool bFromStartTab = false );

    void            FormulaPreview();

    Size            GetTextSize();      // in 1/100mm

                    // actually private, public for SID_INPUT_SUM
    void            InitRangeFinder(const OUString& rFormula);

    void            UpdateLokReferenceMarks();
    static void     SendReferenceMarks( const SfxViewShell* pViewShell,
                         const std::vector<ReferenceMark>& rReferenceMarks );

    void SetDocumentDisposing( bool b );

    static void     SetAutoComplete(bool bSet)  { bAutoComplete = bSet; }

    static ReferenceMark GetReferenceMark( ScViewData& rViewData, ScDocShell* pDocSh,
                                    long nX1, long nX2, long nY1, long nY2,
                                    long nTab, const Color& rColor );

    void            LOKPasteFunctionData(const OUString& rFunctionName);
};

//  ScInputHdlState

class ScInputHdlState
{
    friend class ScInputHandler;

public:
        ScInputHdlState( const ScAddress& rCurPos,
                         const ScAddress& rStartPos,
                         const ScAddress& rEndPos,
                         const OUString& rString,
                         const EditTextObject* pData );
        ScInputHdlState( const ScInputHdlState& rCpy );
        ~ScInputHdlState();

    ScInputHdlState&    operator= ( const ScInputHdlState& r );
    bool                operator==( const ScInputHdlState& r ) const;

    const ScAddress&        GetPos() const          { return aCursorPos; }
    const ScAddress&        GetStartPos() const     { return aStartPos; }
    const ScAddress&        GetEndPos() const       { return aEndPos; }
    const OUString&         GetString() const       { return aString; }
    const EditTextObject*   GetEditData() const     { return pEditData.get(); }

private:
    ScAddress       aCursorPos;
    ScAddress       aStartPos;
    ScAddress       aEndPos;
    OUString        aString;
    std::unique_ptr<EditTextObject> pEditData;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
