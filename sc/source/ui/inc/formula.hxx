/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formula.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:26:26 $
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

#ifndef SC_FORMULA_HXX
#define SC_FORMULA_HXX

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef SC_FUNCUTL_HXX
#include "funcutl.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"       // ScAddress
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#endif

#ifndef _SV_TABCTRL_HXX //autogen
#include <vcl/tabctrl.hxx>
#endif

#ifndef SC_PARAWIN_HXX
#include "parawin.hxx"
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif

#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif

#ifndef SC_FUNCPAGE_HXX
#include "funcpage.hxx"
#endif

#ifndef SC_STRUCTPG_HXX
#include "structpg.hxx"
#endif

class ScViewData;
class ScDocument;
class ScFuncDesc;
class ScInputHandler;
class ScDocShell;

//============================================================================

enum ScFormulaDlgMode { SC_FORMDLG_FORMULA, SC_FORMDLG_ARGS, SC_FORMDLG_EDIT };

//============================================================================

typedef ScTabViewShell* PtrTabViewShell;

//============================================================================

//============================================================================

class ScFormulaDlg : public ScAnyRefDlg
{
public:
                    ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    Window* pParent, ScViewData* pViewData );
                    ~ScFormulaDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pRefDoc );
    virtual BOOL    IsRefInputMode() const;
    virtual BOOL    IsDocAllowed(SfxObjectShell* pDocSh) const;
    virtual void    SetActive();
    virtual BOOL    Close();

private:

    TabControl      aTabCtrl;
    GroupBox        aGbEdit;        //! MUST be placed before aScParaWin for initializing
    ScParaWin       aScParaWin;
    FixedText       aFtHeadLine;
    FixedInfo       aFtFuncName;
    FixedInfo       aFtFuncDesc;

    FixedText       aFtEditName;
    //FixedInfo     aFtEditDesc;

    FixedText       aFtResult;
    ValWnd          aWndResult;

    FixedText       aFtFormula;
    ScEditBox       aMEFormula;

    CheckBox        aBtnMatrix;
    HelpButton      aBtnHelp;
    CancelButton    aBtnCancel;

    PushButton      aBtnBackward;
    PushButton      aBtnForward;
    OKButton        aBtnEnd;

    ScRefEdit       aEdRef;
    ScRefButton     aRefBtn;

    FixedText       aFtFormResult;
    ValWnd          aWndFormResult;

    ScRefEdit*      pTheRefEdit;
    ScRefButton*    pTheRefButton;
    ScFuncPage*     pScFuncPage;
    ScStructPage*   pScStructPage;
    ScFormulaCell*  pCell;
    ScCompiler*     pComp;
    ScTokenArray*   pScTokA;
    String          aOldFormula;
    BOOL            bStructUpdate;
    MultiLineEdit*  pMEdit;
    BOOL            bUserMatrixFlag;
    Timer           aTimer;

    const String    aTitle1;
    const String    aTitle2;
    const String    aTxtEnd;
    const String    aTxtOk;     // hinter aBtnEnd

    ULONG           nOldHelp;
    ULONG           nOldUnique;
    ULONG           nActivWinId;
    BOOL            bIsShutDown;



    Font            aFntBold;
    Font            aFntLight;
    USHORT          nEdFocus;
//    Selection       theCurSel;
    BOOL            bEditFlag;
    const ScFuncDesc*   pFuncDesc;
    USHORT          nArgs;
    String**        pArgArr;
    Selection       aFuncSel;

    static ScDocument*  pDoc;
    static ScAddress    aCursorPos;

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    RefInputStart( ScRefEdit* pEdit, ScRefButton* pButton = NULL );
    virtual void    RefInputDone( BOOL bForced = FALSE );
    ULONG           FindFocusWin(Window *pWin);
    void            SetFocusWin(Window *pWin,ULONG nUniqueId);
    String          RepairFormula(const String& aFormula);
    void            SaveLRUEntry(const ScFuncDesc*  pFuncDesc);
    void            HighlightFunctionParas(const String& aFormula);

private:

    BOOL            IsInputHdl(ScInputHandler* pHdl);
    ScInputHandler* GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh=NULL);

    void            MakeTree(SvLBoxEntry* pParent,ScToken* pScToken,long Count,
                                    ScTokenArray* pScTokA,ScCompiler*   pComp);

    void            ClearAllParas();
    void            DeleteArgs();
    void            FillDialog(BOOL nFlag=TRUE);
    void            EditNextFunc( BOOL bForward, xub_StrLen nFStart=NOT_FOUND );
    void            EditThisFunc(xub_StrLen nFStart);
    void            EditFuncParas(xub_StrLen nEditPos);


    void            UpdateArgInput( USHORT nOffset, USHORT nInput );
    BOOL            CalcValue( const String& rStrExp, String& rStrResult );
    BOOL            CalcStruct( const String& rStrExp);

    void            UpdateValues();
    void            SaveArg( USHORT nEd );
    void            UpdateSelection();
    void            DoEnter( BOOL bOk );
    void            UpdateFunctionDesc();
    void            ResizeArgArr( const ScFuncDesc* pNewFunc );
    void            FillListboxes();
    void            FillControls();

    xub_StrLen      GetFunctionPos(xub_StrLen nPos);
    void            UpdateTokenArray( const String& rStrExp);

    ScRefEdit*      GetCurrRefEdit();

    DECL_LINK( ScrollHdl, ScParaWin* );
    DECL_LINK( ModifyHdl, ScParaWin* );
    DECL_LINK( FxHdl, ScParaWin* );

    DECL_LINK( MatrixHdl, CheckBox *);
    DECL_LINK( FormulaHdl, MultiLineEdit* );
    DECL_LINK( FormulaCursorHdl, ScEditBox*);
    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( GetEdFocusHdl, ArgInput* );
    DECL_LINK( GetFxFocusHdl, ArgInput* );
    DECL_LINK( DblClkHdl, ScFuncPage* );
    DECL_LINK( FuncSelHdl, ScFuncPage*);
    DECL_LINK( StructSelHdl, ScStructPage * );
    DECL_LINK( UpdateFocusHdl, Timer*);
};



#endif // SC_CRNRDLG_HXX

