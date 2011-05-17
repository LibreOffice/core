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

#include "scdllapi.h"
#include "scdll.hxx"
#include <vcl/timer.hxx>
#include <svl/lstner.hxx>
#include "global.hxx"       // ScInputMode
#include "markdata.hxx"     // ScMarkData
#include "shellids.hxx"
#include <unotools/options.hxx>
#include <tools/shl.hxx>

#include <map>
#include <list>
#include <algorithm>


class KeyEvent;
class SdrModel;
class SdrView;
class EditView;
class SfxErrorHandler;
class SvxErrorHandler;
class SvtAccessibilityOptions;
class SvtCTLOptions;
class SvtUserOptions;

namespace svtools { class ColorConfig; }

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
class ScEditFunctionDlg;
class ScMessagePool;
class EditFieldInfo;
class ScNavipiCfg;
class ScAddInCfg;

class ScTransferObj;
class ScDrawTransferObj;
class ScSelectionTransferObj;

class ScFormEditData;

//==================================================================

//      for internal Drag&Drop:

#define SC_DROP_NAVIGATOR       1
#define SC_DROP_TABLE           2

struct ScDragData
{
    ScTransferObj*      pCellTransfer;
    ScDrawTransferObj*  pDrawTransfer;

    String              aLinkDoc;
    String              aLinkTable;
    String              aLinkArea;
    ScDocument*         pJumpLocalDoc;
    String              aJumpTarget;
    String              aJumpText;
};

struct ScClipData
{
    ScTransferObj*      pCellClipboard;
    ScDrawTransferObj*  pDrawClipboard;
};

//==================================================================


class ScModule: public SfxModule, public SfxListener, utl::ConfigurationListener
{
    Timer               aIdleTimer;
    Timer               aSpellTimer;
    ScDragData          aDragData;
    ScClipData          aClipData;
    ScSelectionTransferObj* pSelTransfer;
    ScMessagePool*      pMessagePool;
    // there is no global InputHandler anymore, each View has it's own
    ScInputHandler*     pRefInputHandler;
    ScViewCfg*          pViewCfg;
    ScDocCfg*           pDocCfg;
    ScAppCfg*           pAppCfg;
    ScInputCfg*         pInputCfg;
    ScPrintCfg*         pPrintCfg;
    ScNavipiCfg*        pNavipiCfg;
    ScAddInCfg*         pAddInCfg;
    svtools::ColorConfig*   pColorConfig;
    SvtAccessibilityOptions* pAccessOptions;
    SvtCTLOptions*      pCTLOptions;
    SvtUserOptions*     pUserOptions;
    SfxErrorHandler*    pErrorHdl;
    SvxErrorHandler*    pSvxErrorHdl;
    ScFormEditData*     pFormEditData;
    sal_uInt16              nCurRefDlgId;
    sal_Bool                bIsWaterCan;
    sal_Bool                bIsInEditCommand;
    sal_Bool                bIsInExecuteDrop;
    bool                mbIsInSharedDocLoading;
    bool                mbIsInSharedDocSaving;

    std::map<sal_uInt16, std::list<Window*> > m_mapRefWindow;
public:
                    SFX_DECL_INTERFACE(SCID_APP)

                    ScModule( SfxObjectFactory* pFact );
    virtual         ~ScModule();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
    void                DeleteCfg();

                        // moved by the application

    DECL_LINK( IdleHandler,     Timer* );   // Timer instead of idle
    DECL_LINK( SpellTimerHdl,   Timer* );
    DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

    void                Execute( SfxRequest& rReq );
    void                GetState( SfxItemSet& rSet );
    void                HideDisabledSlots( SfxItemSet& rSet );

    void                AnythingChanged();

    //  Drag & Drop:
    const ScDragData&   GetDragData() const     { return aDragData; }
    void                SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj );
    void                ResetDragObject();
    void                SetDragLink( const String& rDoc, const String& rTab, const String& rArea );
    void                SetDragJump( ScDocument* pLocalDoc,
                                    const String& rTarget, const String& rText );

    //  clipboard:
    const ScClipData&   GetClipData() const     { return aClipData; }
    void                SetClipObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj );

    ScDocument*         GetClipDoc();       // called from document - should be removed later

    //  X selection:
    ScSelectionTransferObj* GetSelectionTransfer() const    { return pSelTransfer; }
    void                SetSelectionTransfer( ScSelectionTransferObj* pNew );

    void                SetWaterCan( sal_Bool bNew )    { bIsWaterCan = bNew; }
    sal_Bool                GetIsWaterCan() const       { return bIsWaterCan; }

    void                SetInEditCommand( sal_Bool bNew )   { bIsInEditCommand = bNew; }
    sal_Bool                IsInEditCommand() const         { return bIsInEditCommand; }

    void                SetInExecuteDrop( sal_Bool bNew )   { bIsInExecuteDrop = bNew; }
    sal_Bool                IsInExecuteDrop() const         { return bIsInExecuteDrop; }

    // Options:
    const ScViewOptions&    GetViewOptions  ();
SC_DLLPUBLIC    const ScDocOptions&     GetDocOptions   ();
SC_DLLPUBLIC    const ScAppOptions&     GetAppOptions   ();
    const ScInputOptions&   GetInputOptions ();
SC_DLLPUBLIC    const ScPrintOptions&   GetPrintOptions ();
    void                    SetViewOptions  ( const ScViewOptions& rOpt );
SC_DLLPUBLIC    void                    SetDocOptions   ( const ScDocOptions& rOpt );
SC_DLLPUBLIC    void                    SetAppOptions   ( const ScAppOptions& rOpt );
    void                    SetInputOptions ( const ScInputOptions& rOpt );
    void                    SetPrintOptions ( const ScPrintOptions& rOpt );
    void                    InsertEntryToLRUList(sal_uInt16 nFIndex);
    void                    RecentFunctionsChanged();

    static void         GetSpellSettings( sal_uInt16& rDefLang, sal_uInt16& rCjkLang, sal_uInt16& rCtlLang,
                                        sal_Bool& rAutoSpell );
    static void         SetAutoSpellProperty( sal_Bool bSet );
    static sal_Bool         HasThesaurusLanguage( sal_uInt16 nLang );

    sal_uInt16              GetOptDigitLanguage();      // from CTL options

    ScNavipiCfg&        GetNavipiCfg();
    ScAddInCfg&         GetAddInCfg();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions& GetAccessOptions();
    SvtCTLOptions&      GetCTLOptions();
    SvtUserOptions&     GetUserOptions();

    void                ModifyOptions( const SfxItemSet& rOptSet );

    // InputHandler:
    sal_Bool                IsEditMode();   // not for SC_INPUT_TYPE
    sal_Bool                IsInputMode();  // also for SC_INPUT_TYPE
    void                SetInputMode( ScInputMode eMode );
    sal_Bool                InputKeyEvent( const KeyEvent& rKEvt, sal_Bool bStartEdit = false );
    SC_DLLPUBLIC void                InputEnterHandler( sal_uInt8 nBlockMode = 0 );
    void                InputCancelHandler();
    void                InputSelection( EditView* pView );
    void                InputChanged( EditView* pView );
    ScInputHandler*     GetInputHdl( ScTabViewShell* pViewSh = NULL, sal_Bool bUseRef = sal_True );

    void                SetRefInputHdl( ScInputHandler* pNew );
    ScInputHandler*     GetRefInputHdl();

    void                ViewShellGone(ScTabViewShell* pViewSh);
    void                ViewShellChanged();
    // communication with function-autopilot
    void                InputGetSelection( xub_StrLen& rStart, xub_StrLen& rEnd );
    void                InputSetSelection( xub_StrLen nStart, xub_StrLen nEnd );
    void                InputReplaceSelection( const String& rStr );
    String              InputGetFormulaStr();
    void                ActivateInputWindow( const String* pStr = NULL,
                                                sal_Bool bMatrix = false );

    void                InitFormEditData();
    void                ClearFormEditData();
    ScFormEditData*     GetFormEditData()       { return pFormEditData; }

    // input of reference:
    SC_DLLPUBLIC void               SetRefDialog( sal_uInt16 nId, sal_Bool bVis, SfxViewFrame* pViewFrm = NULL );
    sal_Bool                IsModalMode(SfxObjectShell* pDocSh = NULL);
    sal_Bool                IsFormulaMode();
    sal_Bool                IsRefDialogOpen();
    sal_Bool                IsTableLocked();
    void                SetReference( const ScRange& rRef, ScDocument* pDoc,
                                        const ScMarkData* pMarkData = NULL );
    void                AddRefEntry();
    void                EndReference();
    sal_uInt16              GetCurRefDlgId() const                  { return nCurRefDlgId; }

    // virtual methods for the options dialog
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );

    void                SetInSharedDocLoading( bool bNew )  { mbIsInSharedDocLoading = bNew; }
    bool                IsInSharedDocLoading() const        { return mbIsInSharedDocLoading; }
    void                SetInSharedDocSaving( bool bNew )   { mbIsInSharedDocSaving = bNew; }
    bool                IsInSharedDocSaving() const         { return mbIsInSharedDocSaving; }

    SC_DLLPUBLIC sal_Bool   RegisterRefWindow( sal_uInt16 nSlotId, Window *pWnd );
    SC_DLLPUBLIC sal_Bool   UnregisterRefWindow( sal_uInt16 nSlotId, Window *pWnd );
    SC_DLLPUBLIC sal_Bool   IsAliveRefDlg( sal_uInt16 nSlotId, Window *pWnd );
    SC_DLLPUBLIC Window * Find1RefWindow( sal_uInt16 nSlotId, Window *pWndAncestor );
    SC_DLLPUBLIC Window * Find1RefWindow( Window *pWndAncestor );
};

#define SC_MOD() ( *(ScModule**) GetAppData(SHL_CALC) )

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
