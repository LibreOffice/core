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

#ifndef SC_INPUTHDL_HXX
#define SC_INPUTHDL_HXX

#include "global.hxx"
#include "address.hxx"
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <editeng/svxenum.hxx>

class ScDocument;
class ScTabView;
class ScTabViewShell;
class ScInputWindow;
class ScPatternAttr;
class EditEngine;
class ScEditEngineDefaulter;
class EditView;
class EditTextObject;
class ScInputHdlState;
class TypedScStrCollection;
class ScRangeFindList;
class Timer;
class KeyEvent;
class CommandEvent;

struct ESelection;

//========================================================================
//  ScInputHandler
//========================================================================

class ScInputHandler
{
private:
    ScInputWindow*          pInputWin;

    ScEditEngineDefaulter*  pEngine;                // editierte Daten in der Tabelle
    EditView*               pTableView;                 // aktive EditView dazu
    EditView*               pTopView;                   // EditView in der Eingabezeile

    TypedScStrCollection*       pColumnData;
    TypedScStrCollection*       pFormulaData;
    TypedScStrCollection*       pFormulaDataPara;
    Window*                 pTipVisibleParent;
    sal_uLong                   nTipVisible;
    Window*                 pTipVisibleSecParent;
    sal_uLong                   nTipVisibleSec;
    String                  aManualTip;
    String                  aAutoSearch;
    sal_uInt16                  nAutoPos;
    sal_Bool                    bUseTab;                    // Blaettern moeglich

    sal_Bool                    bTextValid;                 // Text noch nicht in Edit-Engine
    String                  aCurrentText;

    String                  aFormText;                  // fuer Funktions-Autopilot
    xub_StrLen              nFormSelStart;              // Selektion fuer Funktions-Autopilot
    xub_StrLen              nFormSelEnd;

    sal_uInt16                  nAutoPar;                   // autom.parentheses than can be overwritten

    ScAddress               aCursorPos;
    ScInputMode             eMode;
    sal_Bool                    bModified;
    sal_Bool                    bSelIsRef;
    sal_Bool                    bFormulaMode;
    sal_Bool                    bInRangeUpdate;
    sal_Bool                    bParenthesisShown;
    sal_Bool                    bCreatingFuncView;
    sal_Bool                    bInEnterHandler;
    sal_Bool                    bCommandErrorShown;
    sal_Bool                    bInOwnChange;

    sal_Bool                    bProtected;
    sal_Bool                    bCellHasPercentFormat;
    sal_uLong                   nValidation;
    SvxCellHorJustify       eAttrAdjust;

    Fraction                aScaleX;                    // fuer Ref-MapMode
    Fraction                aScaleY;

    ScTabViewShell*         pRefViewSh;
    ScTabViewShell*         pActiveViewSh;

    const ScPatternAttr*    pLastPattern;
    SfxItemSet*             pEditDefaults;
    sal_Bool                    bLastIsSymbol;

    ScInputHdlState*        pLastState;
    Timer*                  pDelayTimer;

    ScRangeFindList*        pRangeFindList;

    static sal_Bool             bAutoComplete;              // aus App-Optionen
    static sal_Bool             bOptLoaded;

#ifdef _INPUTHDL_CXX
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
    void            NextAutoEntry( sal_Bool bBack );
    void            UpdateAdjust( sal_Unicode cTyped );
    void            GetFormulaData();
    void            UseFormulaData();
    void            NextFormulaEntry( sal_Bool bBack );
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
    sal_Bool            CursorAtClosingPar();
    void            SkipClosingPar();
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( ShowHideTipVisibleParentListener, VclWindowEvent* );
    DECL_LINK( ShowHideTipVisibleSecParentListener, VclWindowEvent* );
#endif

public:
                    ScInputHandler();
    virtual         ~ScInputHandler();

    void            SetMode( ScInputMode eNewMode );
    sal_Bool            IsInputMode() const { return (eMode != SC_INPUT_NONE); }
    sal_Bool            IsEditMode() const  { return (eMode != SC_INPUT_NONE &&
                                                  eMode != SC_INPUT_TYPE); }
    sal_Bool            IsTopMode() const   { return (eMode == SC_INPUT_TOP);  }

    const String&   GetEditString();
    const String&   GetFormString() const   { return aFormText; }

    const ScAddress& GetCursorPos() const   { return aCursorPos; }

    sal_Bool            GetTextAndFields( ScEditEngineDefaulter& rDestEngine );

    sal_Bool            KeyInput( const KeyEvent& rKEvt, sal_Bool bStartEdit = false );
    void            EnterHandler( sal_uInt8 nBlockMode = 0 );
    void            CancelHandler();
    void            SetReference( const ScRange& rRef, ScDocument* pDoc );
    void            AddRefEntry();

    sal_Bool            InputCommand( const CommandEvent& rCEvt, sal_Bool bForce );

    void            InsertFunction( const String& rFuncName, sal_Bool bAddPar = sal_True );
    void            ClearText();

    void            InputSelection( EditView* pView );
    void            InputChanged( EditView* pView, sal_Bool bFromNotify = false );

    void            ViewShellGone(ScTabViewShell* pViewSh);
    void            SetRefViewShell(ScTabViewShell* pRefVsh) {pRefViewSh=pRefVsh;}

    void            NotifyChange( const ScInputHdlState* pState, sal_Bool bForce = false,
                                    ScTabViewShell* pSourceSh = NULL,
                                    sal_Bool bStopEditing = sal_True);
    void            UpdateCellAdjust( SvxCellHorJustify eJust );

    void            ResetDelayTimer(); //BugId 54702

    void            HideTip();
    void            HideTipBelow();
    void            ShowTipCursor();
    void            ShowTip( const String& rText );     // am Cursor
    void            ShowTipBelow( const String& rText );

    void            SetRefScale( const Fraction& rX, const Fraction& rY );
    void            UpdateRefDevice();

    EditView*       GetActiveView();
    EditView*       GetTableView()      { return pTableView; }
    EditView*       GetTopView()        { return pTopView; }

    sal_Bool            DataChanging( sal_Unicode cTyped = 0, sal_Bool bFromCommand = false );
    void            DataChanged( sal_Bool bFromTopNotify = false );

    sal_Bool            TakesReturn() const     { return ( nTipVisible != 0 ); }

    void            SetModified()       { bModified = sal_True; }

    sal_Bool            GetSelIsRef() const     { return bSelIsRef; }
    void            SetSelIsRef(sal_Bool bSet)  { bSelIsRef = bSet; }

    void            ShowRefFrame();

    ScRangeFindList* GetRangeFindList()     { return pRangeFindList; }

    void            UpdateRange( sal_uInt16 nIndex, const ScRange& rNew );

    // Kommunikation mit Funktionsautopilot
    void            InputGetSelection       ( xub_StrLen& rStart, xub_StrLen& rEnd );
    void            InputSetSelection       ( xub_StrLen nStart, xub_StrLen nEnd );
    void            InputReplaceSelection   ( const String& rStr );
    String          InputGetFormulaStr      ();

    sal_Bool            IsFormulaMode() const                   { return bFormulaMode; }
    ScInputWindow*  GetInputWindow()                        { return pInputWin; }
    void            SetInputWindow( ScInputWindow* pNew )   { pInputWin = pNew; }
    void            StopInputWinEngine( sal_Bool bAll );

    sal_Bool            IsInEnterHandler() const                { return bInEnterHandler; }
    sal_Bool            IsInOwnChange() const                   { return bInOwnChange; }

    sal_Bool            IsModalMode( SfxObjectShell* pDocSh );

    void            ForgetLastPattern();

    void            UpdateSpellSettings( sal_Bool bFromStartTab = false );

    void            FormulaPreview();

    Size            GetTextSize();      // in 1/100mm

                    // eigentlich private, fuer SID_INPUT_SUM public
    void            InitRangeFinder( const String& rFormula );

    static void     SetAutoComplete(sal_Bool bSet)  { bAutoComplete = bSet; }
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
