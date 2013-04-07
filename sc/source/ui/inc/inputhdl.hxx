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

#ifndef SC_INPUTHDL_HXX
#define SC_INPUTHDL_HXX

#include "global.hxx"
#include "address.hxx"
#include "typedstrdata.hxx"

#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include "tools/link.hxx"
#include "vcl/vclevent.hxx"
#include <editeng/svxenum.hxx>

#include <set>

#include <boost/noncopyable.hpp>

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

struct ESelection;

//========================================================================
//  ScInputHandler
//========================================================================

class ScInputHandler : boost::noncopyable
{
private:
    ScInputWindow*          pInputWin;

    ScEditEngineDefaulter*  pEngine;                // edited data in the sheet
    EditView*               pTableView;                 // associated active EditView
    EditView*               pTopView;                   // EditView in dthe input row

    ScTypedCaseStrSet* pColumnData;
    ScTypedCaseStrSet* pFormulaData;
    ScTypedCaseStrSet* pFormulaDataPara;
    ScTypedCaseStrSet::const_iterator miAutoPosColumn;
    ScTypedCaseStrSet::const_iterator miAutoPosFormula;

    Window*                 pTipVisibleParent;
    sal_uLong                   nTipVisible;
    Window*                 pTipVisibleSecParent;
    sal_uLong                   nTipVisibleSec;
    OUString           aManualTip;
    OUString           aAutoSearch;

    OUString           aCurrentText;

    OUString           aFormText;                  // for autopilot function
    xub_StrLen              nFormSelStart;              // Selection for autopilot function
    xub_StrLen              nFormSelEnd;

    sal_uInt16                  nAutoPar;                   // autom.parentheses than can be overwritten

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
    bool                    bCellHasPercentFormat:1;
    bool                    bLastIsSymbol:1;
    sal_uLong                   nValidation;
    SvxCellHorJustify       eAttrAdjust;

    Fraction                aScaleX;                    // for ref MapMode
    Fraction                aScaleY;

    ScTabViewShell*         pRefViewSh;
    ScTabViewShell*         pActiveViewSh;

    const ScPatternAttr*    pLastPattern;
    SfxItemSet*             pEditDefaults;

    ScInputHdlState*        pLastState;
    Timer*                  pDelayTimer;

    ScRangeFindList*        pRangeFindList;

    static bool             bAutoComplete;              // from app options
    static bool             bOptLoaded;

private:
    void            UpdateActiveView();
    void            SyncViews( EditView* pSourceView = NULL );
    /**
     * @param cTyped typed character. If 0, look at existing document content
     *               for text or number.
     * @param bInputActivated true if the cell input mode is activated (via
     *                        F2), false otherwise.
     * @return true if the new edit mode has been started.
     */
    bool            StartTable( sal_Unicode cTyped, bool bFromCommand, bool bInputActivated );
    void            RemoveSelection();
    void            UpdateFormulaMode();
    void            InvalidateAttribs();
    void            ImplCreateEditEngine();
    DECL_LINK(      DelayTimer, Timer* );
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
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( ShowHideTipVisibleParentListener, VclWindowEvent* );
    DECL_LINK( ShowHideTipVisibleSecParentListener, VclWindowEvent* );

public:
                    ScInputHandler();
    virtual         ~ScInputHandler();

    void            SetMode( ScInputMode eNewMode );
    bool            IsInputMode() const { return (eMode != SC_INPUT_NONE); }
    bool            IsEditMode() const  { return (eMode != SC_INPUT_NONE &&
                                                  eMode != SC_INPUT_TYPE); }
    bool            IsTopMode() const   { return (eMode == SC_INPUT_TOP);  }

    const OUString& GetEditString();
    const OUString& GetFormString() const { return aFormText; }

    const ScAddress& GetCursorPos() const   { return aCursorPos; }

    bool            GetTextAndFields( ScEditEngineDefaulter& rDestEngine );

    bool            KeyInput( const KeyEvent& rKEvt, bool bStartEdit = false );
    void            EnterHandler( sal_uInt8 nBlockMode = 0 );
    void            CancelHandler();
    void            SetReference( const ScRange& rRef, ScDocument* pDoc );
    void            AddRefEntry();

    bool            InputCommand( const CommandEvent& rCEvt, bool bForce );

    void            InsertFunction( const String& rFuncName, bool bAddPar = true );
    void            ClearText();

    void            InputSelection( EditView* pView );
    void            InputChanged( EditView* pView, bool bFromNotify = false );

    void            ViewShellGone(ScTabViewShell* pViewSh);
    void            SetRefViewShell(ScTabViewShell* pRefVsh) {pRefViewSh=pRefVsh;}

    void            NotifyChange( const ScInputHdlState* pState, bool bForce = false,
                                  ScTabViewShell* pSourceSh = NULL,
                                  bool bStopEditing = true);
    void            UpdateCellAdjust( SvxCellHorJustify eJust );

    void            ResetDelayTimer(); //BugId 54702

    void            HideTip();
    void            HideTipBelow();
    void            ShowTipCursor();
    void            ShowTip( const String& rText );     // at Cursor
    void            ShowTipBelow( const String& rText );

    void            SetRefScale( const Fraction& rX, const Fraction& rY );
    void            UpdateRefDevice();

    EditView*       GetActiveView();
    EditView*       GetTableView()      { return pTableView; }
    EditView*       GetTopView()        { return pTopView; }

    bool            DataChanging( sal_Unicode cTyped = 0, bool bFromCommand = false );
    void            DataChanged( bool bFromTopNotify = false, bool bSetModified = true );

    bool            TakesReturn() const     { return ( nTipVisible != 0 ); }

    void            SetModified()       { bModified = true; }

    bool            GetSelIsRef() const     { return bSelIsRef; }
    void            SetSelIsRef(bool bSet)  { bSelIsRef = bSet; }

    void            ShowRefFrame();

    ScRangeFindList* GetRangeFindList()     { return pRangeFindList; }

    void            UpdateRange( sal_uInt16 nIndex, const ScRange& rNew );

    // Communication with the autopilot function
    void            InputGetSelection       ( xub_StrLen& rStart, xub_StrLen& rEnd );
    void            InputSetSelection       ( xub_StrLen nStart, xub_StrLen nEnd );
    void            InputReplaceSelection   ( const OUString& rStr );

    bool            IsFormulaMode() const                   { return bFormulaMode; }
    ScInputWindow*  GetInputWindow()                        { return pInputWin; }
    void            SetInputWindow( ScInputWindow* pNew )   { pInputWin = pNew; }
    void            StopInputWinEngine( bool bAll );

    bool            IsInEnterHandler() const                { return bInEnterHandler; }
    bool            IsInOwnChange() const                   { return bInOwnChange; }

    bool            IsModalMode( SfxObjectShell* pDocSh );

    void            ForgetLastPattern();

    void            UpdateSpellSettings( bool bFromStartTab = false );

    void            FormulaPreview();

    Size            GetTextSize();      // in 1/100mm

                    // actually private, public for SID_INPUT_SUM
    void            InitRangeFinder( const String& rFormula );

    static void     SetAutoComplete(bool bSet)  { bAutoComplete = bSet; }
};

//========================================================================
//  ScInputHdlState
//========================================================================
class ScInputHdlState
{
    friend class ScInputHandler;

public:
        ScInputHdlState( const ScAddress& rCurPos,
                         const ScAddress& rStartPos,
                         const ScAddress& rEndPos,
                         const String& rString,
                         const EditTextObject* pData );
        ScInputHdlState( const ScInputHdlState& rCpy );
        ~ScInputHdlState();

    ScInputHdlState&    operator= ( const ScInputHdlState& r );
    int                 operator==( const ScInputHdlState& r ) const;
    int                 operator!=( const ScInputHdlState& r ) const
                            { return !operator==( r ); }

    const ScAddress&        GetPos() const          { return aCursorPos; }
    const ScAddress&        GetStartPos() const     { return aStartPos; }
    const ScAddress&        GetEndPos() const       { return aEndPos; }
    const String&           GetString() const       { return aString; }
    const EditTextObject*   GetEditData() const     { return pEditData; }

private:
    ScAddress       aCursorPos;
    ScAddress       aStartPos;
    ScAddress       aEndPos;
    String          aString;
    EditTextObject* pEditData;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
