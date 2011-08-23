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

#include <tools/fract.hxx>

#include <tools/gen.hxx>
class Timer;
class KeyEvent;
namespace binfilter {

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
class CommandEvent;

struct ESelection;

//========================================================================
//	ScInputHandler
//========================================================================

class ScInputHandler
{
private:
    ScInputWindow*			pInputWin;

    ScEditEngineDefaulter*	pEngine;   				// editierte Daten in der Tabelle
    EditView*				pTableView;					// aktive EditView dazu
    EditView*				pTopView;					// EditView in der Eingabezeile

    TypedStrCollection*		pColumnData;
    TypedStrCollection*		pFormulaData;
    ULONG					nTipVisible;
    String					aManualTip;
    String					aAutoSearch;
    USHORT					nAutoPos;
    BOOL					bUseTab;					// Blaettern moeglich

    BOOL					bTextValid;					// Text noch nicht in Edit-Engine
    String					aCurrentText;

    String					aFormText;					// fuer Funktions-Autopilot
    xub_StrLen				nFormSelStart;				// Selektion fuer Funktions-Autopilot
    xub_StrLen				nFormSelEnd;

    USHORT					nAutoPar;					// autom.parentheses than can be overwritten

    ScAddress				aCursorPos;
    ScInputMode				eMode;
    BOOL					bModified;
    BOOL					bSelIsRef;
    BOOL					bFormulaMode;
    BOOL					bInRangeUpdate;
    BOOL					bParenthesisShown;
    BOOL					bCreatingFuncView;
    BOOL					bInEnterHandler;
    BOOL					bCommandErrorShown;
    BOOL					bInOwnChange;

    BOOL					bProtected;
    BOOL					bCellHasPercentFormat;
    ULONG					nValidation;
    USHORT					nAttrAdjust;				// enum SvxCellHorJustify

    Fraction				aScaleX;					// fuer Ref-MapMode
    Fraction				aScaleY;

    ScTabViewShell*			pRefViewSh;
    ScTabViewShell*			pActiveViewSh;

    const ScPatternAttr*	pLastPattern;
    SfxItemSet*			 	pEditDefaults;
    BOOL					bLastIsSymbol;

    ScInputHdlState*		pLastState;
    Timer*					pDelayTimer;

    ScRangeFindList*		pRangeFindList;

    static BOOL				bAutoComplete;				// aus App-Optionen
    static BOOL				bOptLoaded;

#ifdef _INPUTHDL_CXX
private:
    void			ImplCreateEditEngine();
    void			DeleteRangeFinder();
    void			ResetAutoPar();
    void			StopInputWinEngine( BOOL bAll );
    DECL_LINK( ModifyHdl, void* );
#endif

public:
                    ScInputHandler();
    virtual			~ScInputHandler();

    BOOL			IsInputMode() const	{ return (eMode != SC_INPUT_NONE); }
    BOOL			IsEditMode() const	{ return (eMode != SC_INPUT_NONE &&
                                                  eMode != SC_INPUT_TYPE); }
    BOOL			IsTopMode() const	{ return (eMode == SC_INPUT_TOP);  }

    const String&	GetFormString() const	{ return aFormText; }


    void			EnterHandler( BYTE nBlockMode = 0 );




    void			ViewShellGone(ScTabViewShell* pViewSh){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void			ViewShellGone(ScTabViewShell* pViewSh);
    void			SetRefViewShell(ScTabViewShell*	pRefVsh) {pRefViewSh=pRefVsh;}


    void			NotifyChange( const ScInputHdlState* pState, BOOL bForce = FALSE,
                                    ScTabViewShell* pSourceSh = NULL,
                                    BOOL bStopEditing = TRUE);


    void			HideTip();

    void			SetRefScale( const Fraction& rX, const Fraction& rY );
    void			UpdateRefDevice();

    EditView*		GetTableView()		{ return pTableView; }
    EditView*		GetTopView()		{ return pTopView; }


    BOOL			TakesReturn() const		{ return ( nTipVisible != 0 ); }

    void			SetModified()		{ bModified = TRUE; }

    BOOL			GetSelIsRef() const		{ return bSelIsRef; }
    void			SetSelIsRef(BOOL bSet)	{ bSelIsRef = bSet; }


    ScRangeFindList* GetRangeFindList()		{ return pRangeFindList; }


    // Kommunikation mit Funktionsautopilot

    BOOL			IsFormulaMode() const					{ return bFormulaMode; }
    ScInputWindow*	GetInputWindow()						{ return pInputWin; }
    void			SetInputWindow( ScInputWindow* pNew )	{ pInputWin = pNew; }

    BOOL			IsInEnterHandler() const				{ return bInEnterHandler; }
    BOOL			IsInOwnChange() const					{ return bInOwnChange; }







                    // eigentlich private, fuer SID_INPUT_SUM public

    static void		SetAutoComplete(BOOL bSet)	{ bAutoComplete = bSet; }
};

//========================================================================
//	ScInputHdlState
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

    ScInputHdlState&	operator= ( const ScInputHdlState& r );

    const ScAddress&		GetPos() const 			{ return aCursorPos; }
    const ScAddress&		GetStartPos() const 	{ return aStartPos; }
    const ScAddress& 		GetEndPos() const 		{ return aEndPos; }
    const String&			GetString() const		{ return aString; }
    const EditTextObject*	GetEditData() const		{ return pEditData; }

private:
    ScAddress		aCursorPos;
     ScAddress		aStartPos;
     ScAddress		aEndPos;
     String			aString;
     EditTextObject* pEditData;
};



} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
