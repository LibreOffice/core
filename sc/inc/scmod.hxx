/*************************************************************************
 *
 *  $RCSfile: scmod.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-23 19:20:40 $
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

#ifndef SC_SCMOD_HXX
#define SC_SCMOD_HXX

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

class KeyEvent;
class SdrModel;
class SdrView;
class EditView;
class SfxErrorHandler;
class SvxErrorHandler;

class ScRange;
class ScDocument;
class ScViewCfg;
class ScDocCfg;
class ScAppCfg;
class ScInputCfg;
class ScViewOptions;
class ScDocOptions;
class ScAppOptions;
class ScInputOptions;
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
class ScTransferObj;
class ScDrawTransferObj;

//==================================================================

//      for internal Drag&Drop:

#define SC_DROP_NAVIGATOR       1
#define SC_DROP_TABLE           2

struct ScDragData
{
//  ScDocument*     pDoc;
//  ScMarkData      aMarkData;
//  USHORT          nStartX, nStartY, nTabNo, nSizeX, nSizeY;
//  USHORT          nHandleX, nHandleY;
//  USHORT          nFlags;
//  SdrModel*       pSdrModel;
//  SdrView*        pSdrView;

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


class ScModule: public ScModuleDummy, public SfxListener
{
    Timer               aIdleTimer;
    Timer               aSpellTimer;
    ScDragData          aDragData;
    ScClipData          aClipData;
    ScMessagePool*      pMessagePool;
    //  globalen InputHandler gibt's nicht mehr, jede View hat einen
    ScInputHandler*     pRefInputHandler;
    ScTeamDlg*          pTeamDlg;
    ScViewCfg*          pViewCfg;
    ScDocCfg*           pDocCfg;
    ScAppCfg*           pAppCfg;
    ScInputCfg*         pInputCfg;
    ScNavipiCfg*        pNavipiCfg;
    SfxErrorHandler*    pErrorHdl;
    SvxErrorHandler*    pSvxErrorHdl;
    ScFormEditData*     pFormEditData;
    USHORT              nCurRefDlgId;
    BOOL                bDragWasIntern;
    BOOL                bIsWaterCan;
    BOOL                bIsInEditCommand;

public:
                    SFX_DECL_INTERFACE(SCID_APP);

                    ScModule( SfxObjectFactory* pFact );
    virtual         ~ScModule();

    virtual SfxModule*  Load();
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

    void                AnythingChanged();

    //  Drag & Drop:
    const ScDragData&   GetDragData() const     { return aDragData; }
    void                SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj );
    void                ResetDragObject();
    void                SetDragObject( const ScMarkData& rMarkData, const ScRange& rRange,
                                        USHORT nHandleX, USHORT nHandleY,
                                        ScDocument* pDoc, USHORT nFlags );
    void                SetDragObject( SdrModel* pModel, SdrView* pView, USHORT nFlags );
    void                SetDragLink( const String& rDoc, const String& rTab, const String& rArea );
    void                SetDragJump( ScDocument* pLocalDoc,
                                    const String& rTarget, const String& rText );

    void                SetDragIntern(BOOL bSet=TRUE)   { bDragWasIntern = bSet; }
    BOOL                GetDragIntern() const           { return bDragWasIntern; }

    //  clipboard:
    const ScClipData&   GetClipData() const     { return aClipData; }
    void                SetClipObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj );

    ScDocument*         GetClipDoc();       // called from document - should be removed later

    void                SetWaterCan( BOOL bNew )    { bIsWaterCan = bNew; }
    BOOL                GetIsWaterCan() const       { return bIsWaterCan; }

    void                SetInEditCommand( BOOL bNew )   { bIsInEditCommand = bNew; }
    BOOL                IsInEditCommand() const         { return bIsInEditCommand; }

    // Options:
    const ScViewOptions&    GetViewOptions  ();
    const ScDocOptions&     GetDocOptions   ();
    const ScAppOptions&     GetAppOptions   ();
    const ScInputOptions&   GetInputOptions ();
    void                    SetViewOptions  ( const ScViewOptions& rOpt );
    void                    SetDocOptions   ( const ScDocOptions& rOpt );
    void                    SetAppOptions   ( const ScAppOptions& rOpt );
    void                    SetInputOptions ( const ScInputOptions& rOpt );
    void                    InsertEntryToLRUList(USHORT nFIndex);
    void                    RecentFunctionsChanged();

    static void         GetSpellSettings( USHORT& rDefLang, USHORT& rCjkLang, USHORT& rCtlLang,
                                        BOOL& rAutoSpell, BOOL& rHideAuto );
    static void         SetAutoSpellProperty( BOOL bSet );
    static void         SetHideAutoProperty( BOOL bSet );
    static BOOL         HasThesaurusLanguage( USHORT nLang );

    ScNavipiCfg&        GetNavipiCfg();

    void                ModifyOptions( const SfxItemSet& rOptSet );

    //  InputHandler:
    BOOL                IsEditMode();   // nicht bei SC_INPUT_TYPE
    BOOL                IsInputMode();  // auch bei SC_INPUT_TYPE
    void                SetInputMode( ScInputMode eMode );
    BOOL                InputKeyEvent( const KeyEvent& rKEvt, BOOL bStartEdit = FALSE );
    void                InputEnterHandler( BYTE nBlockMode = 0 );
    void                InputCancelHandler();
    void                InputSelection( EditView* pView );
    void                InputChanged( EditView* pView );
    ScInputHandler*     GetInputHdl( ScTabViewShell* pViewSh = NULL );

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
};

#define SC_MOD() ( *(ScModule**) GetAppData(SHL_CALC) )

#endif


