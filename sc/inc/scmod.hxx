/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scmod.hxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:15:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_SCMOD_HXX
#define SC_SCMOD_HXX

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_SCDLL_HXX
#include "scdll.hxx"
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"       // ScInputMode
#endif

#ifndef SC_MARKDATA_HXX     //@05.01.98
#include "markdata.hxx"     //ScMarkData
#endif

#ifndef SC_SHELLIDS_HXX
#include "shellids.hxx"
#endif

#include <tools/shl.hxx>


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
class ScTeamDlg;
class ScEditFunctionDlg;
class ScMessagePool;
class EditFieldInfo;
class ScNavipiCfg;
class ScAddInCfg;
class ScFormEditData;
class ScTransferObj;
class ScDrawTransferObj;
class ScSelectionTransferObj;

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


class ScModule: public SfxModule, public SfxListener
{
    Timer               aIdleTimer;
    Timer               aSpellTimer;
    ScDragData          aDragData;
    ScClipData          aClipData;
    ScSelectionTransferObj* pSelTransfer;
    ScMessagePool*      pMessagePool;
    //  globalen InputHandler gibt's nicht mehr, jede View hat einen
    ScInputHandler*     pRefInputHandler;
    ScTeamDlg*          pTeamDlg;
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
    USHORT              nCurRefDlgId;
    BOOL                bIsWaterCan;
    BOOL                bIsInEditCommand;
    BOOL                bIsInExecuteDrop;
    bool                mbIsInSharedDocLoading;
    bool                mbIsInSharedDocSaving;

public:
                    SFX_DECL_INTERFACE(SCID_APP)

                    ScModule( SfxObjectFactory* pFact );
    virtual         ~ScModule();

    virtual void        FillStatusBar(StatusBar &rBar);
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void                DeleteCfg();
    void                CloseDialogs();

                        // von der Applikation verschoben:

    DECL_LINK( IdleHandler,     Timer* );   // Timer statt idle
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

    void                SetWaterCan( BOOL bNew )    { bIsWaterCan = bNew; }
    BOOL                GetIsWaterCan() const       { return bIsWaterCan; }

    void                SetInEditCommand( BOOL bNew )   { bIsInEditCommand = bNew; }
    BOOL                IsInEditCommand() const         { return bIsInEditCommand; }

    void                SetInExecuteDrop( BOOL bNew )   { bIsInExecuteDrop = bNew; }
    BOOL                IsInExecuteDrop() const         { return bIsInExecuteDrop; }

    // Options:
    const ScViewOptions&    GetViewOptions  ();
    const ScDocOptions&     GetDocOptions   ();
SC_DLLPUBLIC    const ScAppOptions&     GetAppOptions   ();
    const ScInputOptions&   GetInputOptions ();
SC_DLLPUBLIC    const ScPrintOptions&   GetPrintOptions ();
    void                    SetViewOptions  ( const ScViewOptions& rOpt );
    void                    SetDocOptions   ( const ScDocOptions& rOpt );
SC_DLLPUBLIC    void                    SetAppOptions   ( const ScAppOptions& rOpt );
    void                    SetInputOptions ( const ScInputOptions& rOpt );
    void                    SetPrintOptions ( const ScPrintOptions& rOpt );
    void                    InsertEntryToLRUList(USHORT nFIndex);
    void                    RecentFunctionsChanged();

    static void         GetSpellSettings( USHORT& rDefLang, USHORT& rCjkLang, USHORT& rCtlLang,
                                        BOOL& rAutoSpell, BOOL& rHideAuto );
    static void         SetAutoSpellProperty( BOOL bSet );
    static void         SetHideAutoProperty( BOOL bSet );
    static BOOL         HasThesaurusLanguage( USHORT nLang );

    USHORT              GetOptDigitLanguage();      // from CTL options

    ScNavipiCfg&        GetNavipiCfg();
    ScAddInCfg&         GetAddInCfg();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions& GetAccessOptions();
    SvtCTLOptions&      GetCTLOptions();
    SvtUserOptions&     GetUserOptions();

    void                ModifyOptions( const SfxItemSet& rOptSet );

    //  InputHandler:
    BOOL                IsEditMode();   // nicht bei SC_INPUT_TYPE
    BOOL                IsInputMode();  // auch bei SC_INPUT_TYPE
    void                SetInputMode( ScInputMode eMode );
    BOOL                InputKeyEvent( const KeyEvent& rKEvt, BOOL bStartEdit = FALSE );
    SC_DLLPUBLIC void                InputEnterHandler( BYTE nBlockMode = 0 );
    void                InputCancelHandler();
    void                InputSelection( EditView* pView );
    void                InputChanged( EditView* pView );
    ScInputHandler*     GetInputHdl( ScTabViewShell* pViewSh = NULL, BOOL bUseRef = TRUE );

    void                SetRefInputHdl( ScInputHandler* pNew );
    ScInputHandler*     GetRefInputHdl();

    void                SetInputWindow( ScInputWindow* pWin );
    void                ViewShellGone(ScTabViewShell* pViewSh);
    void                ViewShellChanged();
    // Kommunikation mit Funktionsautopilot
    void                InputGetSelection( xub_StrLen& rStart, xub_StrLen& rEnd );
    void                InputSetSelection( xub_StrLen nStart, xub_StrLen nEnd );
    void                InputReplaceSelection( const String& rStr );
    String              InputGetFormulaStr();
    void                ActivateInputWindow( const String* pStr = NULL,
                                                BOOL bMatrix = FALSE );

    void                InitFormEditData();
    void                ClearFormEditData();
    ScFormEditData*     GetFormEditData()       { return pFormEditData; }

    //  Referenzeingabe:
    void                SetRefDialog( USHORT nId, BOOL bVis, SfxViewFrame* pViewFrm = NULL );
    BOOL                IsModalMode(SfxObjectShell* pDocSh = NULL);
    BOOL                IsFormulaMode();
    BOOL                IsRefDialogOpen();
    BOOL                IsTableLocked();
    void                OpenTeamDlg();
    void                SetTeamDlg( ScTeamDlg* pDlg )           { pTeamDlg = pDlg; }
    ScTeamDlg*          GetTeamDlg() const                      { return pTeamDlg; }
    void                SetReference( const ScRange& rRef, ScDocument* pDoc,
                                        const ScMarkData* pMarkData = NULL );
    void                AddRefEntry();
    void                EndReference();
    USHORT              GetCurRefDlgId() const                  { return nCurRefDlgId; }

    //virtuelle Methoden fuer den Optionendialog
    virtual SfxItemSet*  CreateItemSet( USHORT nId );
    virtual void         ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet );

    void                SetInSharedDocLoading( bool bNew )  { mbIsInSharedDocLoading = bNew; }
    bool                IsInSharedDocLoading() const        { return mbIsInSharedDocLoading; }
    void                SetInSharedDocSaving( bool bNew )   { mbIsInSharedDocSaving = bNew; }
    bool                IsInSharedDocSaving() const         { return mbIsInSharedDocSaving; }
};

#define SC_MOD() ( *(ScModule**) GetAppData(SHL_CALC) )

#endif


