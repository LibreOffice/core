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

#ifndef SC_SCMOD_HXX
#define SC_SCMOD_HXX

#include "scdll.hxx"

#include <vcl/timer.hxx>

#include <bf_svtools/lstner.hxx>

#include "global.hxx"		// ScInputMode

#include "markdata.hxx"		//ScMarkData

#include "shellids.hxx"
class KeyEvent;
class SfxErrorHandler;
namespace binfilter {
class SvtCTLOptions;
class ColorConfig;
class SdrModel;
class SdrView;
class EditView;
class SvxErrorHandler;

class ScRange;
class ScDocument;
class ScViewCfg;
class ScDocCfg;
class ScAppCfg;
class ScInputCfg;
class ScPrintCfg;
class ScViewOptions;
class ScDocOptions;
class ScAppOptions;
class ScInputOptions;
class ScPrintOptions;
class ScInputHandler;
class ScInputWindow;
class ScTabViewShell;
class ScFunctionDlg;
class ScArgDlgBase;
class ScTeamDlg;
class ScEditFunctionDlg;
class ScMessagePool;
class EditFieldInfo;
class ScNavipiCfg;
class ScFormEditData;

//==================================================================

//		for internal Drag&Drop:

#define SC_DROP_NAVIGATOR		1
#define SC_DROP_TABLE			2

struct ScDragData
{
    String				aLinkDoc;
    String				aLinkTable;
    String				aLinkArea;
    ScDocument*			pJumpLocalDoc;
    String				aJumpTarget;
    String				aJumpText;
};

//==================================================================


class ScModule: public ScModuleDummy, public SfxListener
{
    Timer				aIdleTimer;
    Timer				aSpellTimer;
    ScDragData			aDragData;
    ScMessagePool*      pMessagePool;
    //	globalen InputHandler gibt's nicht mehr, jede View hat einen
    ScInputHandler*     pRefInputHandler;
    ScTeamDlg*			pTeamDlg;
    ScViewCfg*			pViewCfg;
    ScDocCfg*			pDocCfg;
    ScAppCfg*           pAppCfg;
    ScInputCfg*			pInputCfg;
    ScPrintCfg*			pPrintCfg;
    ScNavipiCfg*		pNavipiCfg;
    ColorConfig*		pColorConfig;
    SvtCTLOptions*		pCTLOptions;
    SfxErrorHandler*	pErrorHdl;
    SvxErrorHandler* 	pSvxErrorHdl;
    ScFormEditData*		pFormEditData;
    USHORT				nCurRefDlgId;
    BOOL				bIsWaterCan;
    BOOL				bIsInEditCommand;

public:
                    ScModule( SfxObjectFactory* pFact );
    virtual			~ScModule();

    virtual SfxModule*	Load();
    virtual void    	FillStatusBar(StatusBar &rBar);

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void				DeleteCfg();

                        // von der Applikation verschoben:

    DECL_LINK( IdleHandler,		Timer* );	// Timer statt idle
    DECL_LINK( SpellTimerHdl,	Timer* );
    DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

    void				AnythingChanged();

    //	Drag & Drop:
    const ScDragData&	GetDragData() const		{ return aDragData; }
    void				ResetDragObject();

    void				SetWaterCan( BOOL bNew )	{ bIsWaterCan = bNew; }
    BOOL				GetIsWaterCan() const 		{ return bIsWaterCan; }

    void				SetInEditCommand( BOOL bNew )	{ bIsInEditCommand = bNew; }
    BOOL				IsInEditCommand() const 		{ return bIsInEditCommand; }

    // Options:
    const ScViewOptions&	GetViewOptions	();
    const ScDocOptions&		GetDocOptions	();
    const ScAppOptions&		GetAppOptions	();
    const ScInputOptions&	GetInputOptions	();
    const ScPrintOptions&	GetPrintOptions	();
    void					SetAppOptions	( const ScAppOptions& rOpt );
    void					SetInputOptions	( const ScInputOptions& rOpt );
    void					SetPrintOptions	( const ScPrintOptions& rOpt );
    void					RecentFunctionsChanged();

    static void			GetSpellSettings( USHORT& rDefLang, USHORT& rCjkLang, USHORT& rCtlLang,
                                        BOOL& rAutoSpell, BOOL& rHideAuto );

    USHORT				GetOptDigitLanguage();		// from CTL options

    ColorConfig&		GetColorConfig();
    SvtCTLOptions&		GetCTLOptions();


    //	InputHandler:
    void                InputEnterHandler( BYTE nBlockMode = 0 );
    ScInputHandler*		GetInputHdl( ScTabViewShell* pViewSh = NULL, BOOL bUseRef = TRUE );

    ScInputHandler*		GetRefInputHdl();

    // Kommunikation mit Funktionsautopilot

    ScFormEditData*		GetFormEditData()		{ return pFormEditData; }

    void				SetTeamDlg( ScTeamDlg* pDlg )			{ pTeamDlg = pDlg; }
    ScTeamDlg*			GetTeamDlg() const						{ return pTeamDlg; }
    USHORT				GetCurRefDlgId() const					{ return nCurRefDlgId; }

    //virtuelle Methoden fuer den Optionendialog
};

#define SC_MOD() ( *(ScModule**) GetAppData(BF_SHL_CALC) )

} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
