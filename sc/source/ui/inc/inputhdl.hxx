/*************************************************************************
 *
 *  $RCSfile: inputhdl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
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

#ifndef SC_INPUTHDL_HXX
#define SC_INPUTHDL_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

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
class TypedStrCollection;
class ScRangeFindList;
class Timer;
class KeyEvent;

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

    TypedStrCollection*     pColumnData;
    TypedStrCollection*     pFormulaData;
    ULONG                   nTipVisible;
    String                  aManualTip;
    String                  aAutoSearch;
    USHORT                  nAutoPos;
    BOOL                    bUseTab;                    // Blaettern moeglich

    BOOL                    bTextValid;                 // Text noch nicht in Edit-Engine
    String                  aCurrentText;

    String                  aFormText;                  // fuer Funktions-Autopilot
    xub_StrLen              nFormSelStart;              // Selektion fuer Funktions-Autopilot
    xub_StrLen              nFormSelEnd;

    USHORT                  nAutoPar;                   // autom.parentheses than can be overwritten

    ScAddress               aCursorPos;
    ScInputMode             eMode;
    BOOL                    bModified;
    BOOL                    bSelIsRef;
    BOOL                    bFormulaMode;
    BOOL                    bInRangeUpdate;
    BOOL                    bParenthesisShown;
    BOOL                    bCreatingFuncView;
    BOOL                    bInEnterHandler;

    BOOL                    bProtected;
    BOOL                    bCellHasPercentFormat;
    ULONG                   nValidation;
    USHORT                  nAttrAdjust;                // enum SvxCellHorJustify

    Fraction                aScaleX;                    // fuer Ref-MapMode
    Fraction                aScaleY;

    ScTabViewShell*         pRefViewSh;
    ScTabViewShell*         pActiveViewSh;

    const ScPatternAttr*    pLastPattern;
    SfxItemSet*             pEditDefaults;
    BOOL                    bLastIsSymbol;

    ScInputHdlState*        pLastState;
    Timer*                  pDelayTimer;

    ScRangeFindList*        pRangeFindList;

    static BOOL             bAutoComplete;              // aus App-Optionen
    static BOOL             bOptLoaded;

#ifdef _INPUTHDL_CXX
private:
    void            UpdateActiveView();
    void            SetAllUpdateMode( BOOL bUpdate );
    void            SyncViews( EditView* pSourceView = NULL );
    BOOL            StartTable( sal_Unicode cTyped );
    void            RemoveSelection();
    void            UpdateFormulaMode();
    void            InvalidateAttribs();
    void            ImplCreateEditEngine();
    DECL_LINK(      DelayTimer, Timer* );
    void            GetColData();
    void            UseColData();
    void            NextAutoEntry( BOOL bBack );
    void            UpdateAdjust( sal_Unicode cTyped );
    void            GetFormulaData();
    void            UseFormulaData();
    void            NextFormulaEntry( BOOL bBack );
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
    BOOL            CursorAtClosingPar();
    void            SkipClosingPar();
#endif

public:
                    ScInputHandler();
    virtual         ~ScInputHandler();

    void            SetMode( ScInputMode eNewMode );
    BOOL            IsInputMode() const { return (eMode != SC_INPUT_NONE); }
    BOOL            IsEditMode() const  { return (eMode != SC_INPUT_NONE &&
                                                  eMode != SC_INPUT_TYPE); }
    BOOL            IsTopMode() const   { return (eMode == SC_INPUT_TOP);  }

    const String&   GetEditString();
    const String&   GetFormString() const   { return aFormText; }

    BOOL            GetTextAndFields( ScEditEngineDefaulter& rDestEngine );

    BOOL            KeyInput( const KeyEvent& rKEvt, BOOL bStartEdit = FALSE );
    void            EnterHandler( BYTE nBlockMode = 0 );
    void            CancelHandler();
    void            SetReference( const ScRange& rRef, ScDocument* pDoc );
    void            AddRefEntry();

    void            InsertFunction( const String& rFuncName, BOOL bAddPar = TRUE );
    void            ClearText();

    void            InputSelection( EditView* pView );
    void            InputChanged( EditView* pView );

    void            ViewShellGone(ScTabViewShell* pViewSh);
    void            SetRefViewShell(ScTabViewShell* pRefVsh) {pRefViewSh=pRefVsh;}


    void            NotifyChange( const ScInputHdlState* pState, BOOL bForce = FALSE );

    void            ResetDelayTimer(); //BugId 54702

    void            HideTip();
    void            ShowTip( const String& rText );     // am Cursor

    void            SetRefScale( const Fraction& rX, const Fraction& rY );

    EditView*       GetActiveView();
    EditView*       GetTableView()      { return pTableView; }
    EditView*       GetTopView()        { return pTopView; }

    BOOL            DataChanging( sal_Unicode cTyped = 0 );
    void            DataChanged();

    BOOL            TakesReturn() const     { return ( nTipVisible != 0 ); }

    void            SetModified()       { bModified = TRUE; }

    BOOL            GetSelIsRef() const     { return bSelIsRef; }
    void            SetSelIsRef(BOOL bSet)  { bSelIsRef = bSet; }

    void            ShowRefFrame();

    ScRangeFindList* GetRangeFindList()     { return pRangeFindList; }

    void            UpdateRange( USHORT nIndex, const ScRange& rNew );

    // Kommunikation mit Funktionsautopilot
    void            InputGetSelection       ( xub_StrLen& rStart, xub_StrLen& rEnd );
    void            InputSetSelection       ( xub_StrLen nStart, xub_StrLen nEnd );
    void            InputReplaceSelection   ( const String& rStr );
    String          InputGetFormulaStr      ();

    BOOL            IsFormulaMode() const                   { return bFormulaMode; }
    ScInputWindow*  GetInputWindow()                        { return pInputWin; }
    void            SetInputWindow( ScInputWindow* pNew )   { pInputWin = pNew; }

    BOOL            IsModalMode( SfxObjectShell* pDocSh );

    void            ActivateInputWindow( const String&     rText,
                                         const ESelection& rSel );

    void            ForgetLastPattern();

    void            UpdateSpellSettings( BOOL bFromStartTab = FALSE );

    void            FormulaPreview();

    Size            GetTextSize();      // in 1/100mm

                    // eigentlich private, fuer SID_INPUT_SUM public
    void            InitRangeFinder( const String& rFormula );

    static void     SetAutoComplete(BOOL bSet)  { bAutoComplete = bSet; }
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


