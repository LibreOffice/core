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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_formula.hxx"

//----------------------------------------------------------------------------
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/wall.hxx>

#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/svtreebx.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>

#include <unotools/charclass.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

#include "formdlgs.hrc"
#include "funcpage.hxx"
#include "formula/formula.hxx"
#include "formula/IFunctionDescription.hxx"
#include "formula/FormulaCompiler.hxx"
#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "formula/formdata.hxx"
#include "formula/formulahelper.hxx"
#include "structpg.hxx"
#include "parawin.hxx"
#include "ModuleHelper.hxx"
#include "ForResId.hrc"
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/bind.hpp>
#include <comphelper/processfactory.hxx>
#include <map>

#define TOKEN_OPEN  0
#define TOKEN_CLOSE 1
#define TOKEN_SEP   2
namespace formula
{
    using namespace ::com::sun::star;

    class OFormulaToken : public IFormulaToken
    {
        sal_Int32   m_nParaCount;
        bool        m_bIsFunction;

    public:
        OFormulaToken(bool _bFunction,sal_Int32 _nParaCount) : m_nParaCount(_nParaCount),m_bIsFunction(_bFunction){}

        virtual bool isFunction() const { return m_bIsFunction; }
        virtual sal_uInt32 getArgumentCount() const { return m_nParaCount; }
    };


    class FormulaDlg_Impl
    {
    public:
        ::std::pair<RefButton*,RefEdit*>
                        RefInputStartBefore( RefEdit* pEdit, RefButton* pButton );
        void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton );
        void            RefInputDoneAfter( BOOL bForced );
        BOOL            CalcValue( const String& rStrExp, String& rStrResult );
        BOOL            CalcStruct( const String& rStrExp);
        void            UpdateValues();
        void            DeleteArgs();
        xub_StrLen      GetFunctionPos(xub_StrLen nPos);
        void            ClearAllParas();

        void            MakeTree(IStructHelper* _pTree,SvLBoxEntry* pParent,FormulaToken* _pToken,long Count);
        void            fillTree(IStructHelper* _pTree);
        void            UpdateTokenArray( const String& rStrExp);
        String          RepairFormula(const String& aFormula);
        void            FillDialog(BOOL nFlag=TRUE);
        void            EditNextFunc( BOOL bForward, xub_StrLen nFStart=NOT_FOUND );
        void            EditThisFunc(xub_StrLen nFStart);
        void            EditFuncParas(xub_StrLen nEditPos);


        void            UpdateArgInput( USHORT nOffset, USHORT nInput );
        void            Update();
        void            Update(const String& _sExp);


        void            SaveArg( USHORT nEd );
        void            UpdateSelection();
        void            DoEnter( BOOL bOk );
        void            UpdateFunctionDesc();
        void            ResizeArgArr( const IFunctionDescription* pNewFunc );
        void            FillListboxes();
        void            FillControls(BOOL &rbNext, BOOL &rbPrev);

        FormulaDlgMode  SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,BOOL bMatrix,BOOL _bSelect,BOOL _bUpdate);
        void            SetMeText(const String& _sText);
        BOOL            CheckMatrix(String& aFormula /*IN/OUT*/);

        void            SetEdSelection();

        BOOL            UpdateParaWin(Selection& _rSelection);
        void            UpdateParaWin(const Selection& _rSelection,const String& _sRefStr);

        void            SetData(xub_StrLen nFStart,xub_StrLen nNextFStart,xub_StrLen nNextFEnd,xub_StrLen& PrivStart,xub_StrLen& PrivEnd);
        void            PreNotify( NotifyEvent& rNEvt );

        RefEdit*        GetCurrRefEdit();
        ULONG           FindFocusWin(Window *pWin);

        const FormulaHelper& GetFormulaHelper() const;
        uno::Reference< sheet::XFormulaOpCodeMapper > GetFormulaOpCodeMapper() const;

        DECL_LINK( ModifyHdl, ParaWin* );
        DECL_LINK( FxHdl, ParaWin* );

        DECL_LINK( MatrixHdl, CheckBox *);
        DECL_LINK( FormulaHdl, MultiLineEdit* );
        DECL_LINK( FormulaCursorHdl, EditBox*);
        DECL_LINK( BtnHdl, PushButton* );
        DECL_LINK( GetEdFocusHdl, ArgInput* );
        DECL_LINK( GetFxFocusHdl, ArgInput* );
        DECL_LINK( DblClkHdl, FuncPage* );
        DECL_LINK( FuncSelHdl, FuncPage*);
        DECL_LINK( StructSelHdl, StructPage * );
    public:
        OModuleClient                                           m_aModuleClient;
        mutable uno::Reference< sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
        uno::Sequence< sheet::FormulaToken >                    m_aTokenList;
        ::std::auto_ptr<FormulaTokenArray>                      m_pTokenArray;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aSpecialOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pSpecialOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaToken >            m_aSeparatorsOpCodes;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aFunctionOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pFunctionOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aUnaryOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pUnaryOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aBinaryOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pBinaryOpCodesEnd;
        ::std::vector< ::boost::shared_ptr<OFormulaToken> >     m_aTokens;
        ::std::map<FormulaToken*,sheet::FormulaToken>           m_aTokenMap;
        IFormulaEditorHelper*                                   m_pHelper;
        Dialog*  m_pParent;
        IControlReferenceHandler*  m_pDlg;
        TabControl      aTabCtrl;
        GroupBox        aGEdit;     //! MUST be placed before pParaWin for initializing
        ParaWin*        pParaWin;
        FixedText       aFtHeadLine;
        FixedInfo       aFtFuncName;
        FixedInfo       aFtFuncDesc;

        FixedText       aFtEditName;
        //FixedInfo     aFtEditDesc;

        FixedText       aFtResult;
        ValWnd          aWndResult;

        FixedText       aFtFormula;
        EditBox         aMEFormula;

        CheckBox        aBtnMatrix;
        HelpButton      aBtnHelp;
        CancelButton    aBtnCancel;

        PushButton      aBtnBackward;
        PushButton      aBtnForward;
        OKButton        aBtnEnd;

        RefEdit     aEdRef;
        RefButton   aRefBtn;

        FixedText       aFtFormResult;
        ValWnd          aWndFormResult;

        RefEdit*        pTheRefEdit;
        RefButton*  pTheRefButton;
        FuncPage*   pFuncPage;
        StructPage* pStructPage;
        String          aOldFormula;
        BOOL            bStructUpdate;
        MultiLineEdit*  pMEdit;
        BOOL            bUserMatrixFlag;
        Timer           aTimer;

        const String    aTitle1;
        const String    aTitle2;
        const String    aTxtEnd;
        const String    aTxtOk;     // behind aBtnEnd
        FormulaHelper
                        m_aFormulaHelper;

        SmartId         m_aSmartEditHelpId;

        ULONG           nOldHelp;
        ULONG           nOldUnique;
        ULONG           nActivWinId;
        BOOL            bIsShutDown;



        Font            aFntBold;
        Font            aFntLight;
        USHORT          nEdFocus;
    //    Selection       theCurSel;
        BOOL            bEditFlag;
        const IFunctionDescription* pFuncDesc;
        xub_StrLen      nArgs;
        ::std::vector< ::rtl::OUString > m_aArguments;
        Selection       aFuncSel;

        FormulaDlg_Impl(Dialog* pParent
                        , bool _bSupportFunctionResult
                        , bool _bSupportResult
                        , bool _bSupportMatrix
                        ,IFormulaEditorHelper* _pHelper
                        ,const IFunctionManager* _pFunctionMgr
                        ,IControlReferenceHandler* _pDlg);
        ~FormulaDlg_Impl();

    };
FormulaDlg_Impl::FormulaDlg_Impl(Dialog* pParent
                                        , bool _bSupportFunctionResult
                                        , bool _bSupportResult
                                        , bool _bSupportMatrix
                                        ,IFormulaEditorHelper* _pHelper
                                        ,const IFunctionManager* _pFunctionMgr
                                        ,IControlReferenceHandler* _pDlg)
    :
    m_pHelper       (_pHelper),
    m_pParent       (pParent),
    m_pDlg          (_pDlg),
    aTabCtrl        ( pParent, ModuleRes( TC_FUNCTION ) ),
    aGEdit          ( pParent, ModuleRes( GB_EDIT ) ),
    aFtHeadLine     ( pParent, ModuleRes( FT_HEADLINE ) ),
    aFtFuncName     ( pParent, ModuleRes( FT_FUNCNAME ) ),
    aFtFuncDesc     ( pParent, ModuleRes( FT_FUNCDESC ) ),
    //
    aFtEditName     ( pParent, ModuleRes( FT_EDITNAME ) ),
    aFtResult       ( pParent, ModuleRes( FT_RESULT ) ),
    aWndResult      ( pParent, ModuleRes( WND_RESULT ) ),

    aFtFormula      ( pParent, ModuleRes( FT_FORMULA ) ),
    aMEFormula      ( pParent, ModuleRes( ED_FORMULA ) ),
    //
    aBtnMatrix      ( pParent, ModuleRes( BTN_MATRIX ) ),
    aBtnHelp        ( pParent, ModuleRes( BTN_HELP ) ),
    aBtnCancel      ( pParent, ModuleRes( BTN_CANCEL ) ),
    aBtnBackward    ( pParent, ModuleRes( BTN_BACKWARD ) ),
    aBtnForward     ( pParent, ModuleRes( BTN_FORWARD ) ),
    aBtnEnd         ( pParent, ModuleRes( BTN_END ) ),
    aEdRef          ( pParent, _pDlg, ModuleRes( ED_REF) ),
    aRefBtn         ( pParent, ModuleRes( RB_REF),&aEdRef,_pDlg ),
    aFtFormResult   ( pParent, ModuleRes( FT_FORMULA_RESULT)),
    aWndFormResult  ( pParent, ModuleRes( WND_FORMULA_RESULT)),
    //
    pTheRefEdit     (NULL),
    pMEdit          (NULL),
    bUserMatrixFlag (FALSE),
    //
    aTitle1         ( ModuleRes( STR_TITLE1 ) ),        // local resource
    aTitle2         ( ModuleRes( STR_TITLE2 ) ),        // local resource
    aTxtEnd         ( ModuleRes( STR_END ) ),           // local resource
    aTxtOk          ( aBtnEnd.GetText() ),
    m_aFormulaHelper(_pFunctionMgr),
    //
    nActivWinId     (0),
    bIsShutDown     (FALSE),
    nEdFocus        (0),
    pFuncDesc       (NULL),
    nArgs           (0)
{
    pParaWin = new ParaWin( pParent,_pDlg, aGEdit.GetPosPixel());
    aGEdit.Hide();
    pParaWin->Hide();
    aFtEditName.Hide();
    aEdRef.Hide();
    aRefBtn.Hide();

    pMEdit = aMEFormula.GetEdit();
    m_aSmartEditHelpId = pMEdit->GetSmartHelpId();
    pMEdit->SetSmartUniqueId(m_aSmartEditHelpId);

    bEditFlag=FALSE;
    bStructUpdate=TRUE;
    Point aPos=aGEdit.GetPosPixel();
    pParaWin->SetPosPixel(aPos);
    pParaWin->SetArgModifiedHdl(LINK( this, FormulaDlg_Impl, ModifyHdl ) );
    pParaWin->SetFxHdl(LINK( this, FormulaDlg_Impl, FxHdl ) );

    pFuncPage= new FuncPage( &aTabCtrl,_pFunctionMgr);
    pStructPage= new StructPage( &aTabCtrl);
    pFuncPage->Hide();
    pStructPage->Hide();
    aTabCtrl.SetTabPage( TP_FUNCTION, pFuncPage);
    aTabCtrl.SetTabPage( TP_STRUCT, pStructPage);

    nOldHelp = pParent->GetHelpId();                // HelpId from resource always for "Page 1"
    nOldUnique = pParent->GetUniqueId();

    aFtResult.Show( _bSupportResult );
    aWndResult.Show( _bSupportResult );

    aFtFormResult.Show( _bSupportFunctionResult );
    aWndFormResult.Show( _bSupportFunctionResult );

    if ( _bSupportMatrix )
        aBtnMatrix.SetClickHdl(LINK( this, FormulaDlg_Impl, MatrixHdl ) );
    else
        aBtnMatrix.Hide();

    aBtnCancel  .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnEnd     .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnForward .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnBackward.SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );

    pFuncPage->SetDoubleClickHdl( LINK( this, FormulaDlg_Impl, DblClkHdl ) );
    pFuncPage->SetSelectHdl( LINK( this, FormulaDlg_Impl, FuncSelHdl) );
    pStructPage->SetSelectionHdl( LINK( this, FormulaDlg_Impl, StructSelHdl ) );
    pMEdit->SetModifyHdl( LINK( this, FormulaDlg_Impl, FormulaHdl ) );
    aMEFormula.SetSelChangedHdl( LINK( this, FormulaDlg_Impl, FormulaCursorHdl ) );

    aFntLight = aFtFormula.GetFont();
    aFntLight.SetTransparent( TRUE );
    aFntBold = aFntLight;
    aFntBold.SetWeight( WEIGHT_BOLD );

    pParaWin->SetArgumentFonts(aFntBold,aFntLight);

    //  function description for choosing a function is no longer in a different color

    aFtHeadLine.SetFont(aFntBold);
    aFtFuncName.SetFont(aFntLight);
    aFtFuncDesc.SetFont(aFntLight);
}
FormulaDlg_Impl::~FormulaDlg_Impl()
{
    if(aTimer.IsActive())
    {
        aTimer.SetTimeoutHdl(Link());
        aTimer.Stop();
    }// if(aTimer.IsActive())
    bIsShutDown=TRUE;// Set it in order to PreNotify not to save GetFocus.
    FormEditData* pData = m_pHelper->getFormEditData();
    if (pData) // it won't be destroyed over Close;
    {
        pData->SetFStart((xub_StrLen)pMEdit->GetSelection().Min());
        pData->SetSelection(pMEdit->GetSelection());

        if(aTabCtrl.GetCurPageId()==TP_FUNCTION)
            pData->SetMode( (USHORT) FORMULA_FORMDLG_FORMULA );
        else
            pData->SetMode( (USHORT) FORMULA_FORMDLG_EDIT );
        pData->SetUndoStr(pMEdit->GetText());
        pData->SetMatrixFlag(aBtnMatrix.IsChecked());
    }

    aTabCtrl.RemovePage(TP_FUNCTION);
    aTabCtrl.RemovePage(TP_STRUCT);

    delete pStructPage;
    delete pFuncPage;
    delete pParaWin;
    DeleteArgs();
}
// -----------------------------------------------------------------------------
void FormulaDlg_Impl::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_GETFOCUS && !bIsShutDown)
    {
        Window* pWin=rNEvt.GetWindow();
        if(pWin!=NULL)
        {
            nActivWinId = pWin->GetUniqueId();
            if(nActivWinId==0)
            {
                Window* pParent=pWin->GetParent();
                while(pParent!=NULL)
                {
                    nActivWinId=pParent->GetUniqueId();

                    if(nActivWinId!=0) break;

                    pParent=pParent->GetParent();
                }
            }
            if(nActivWinId!=0)
            {

                FormEditData* pData = m_pHelper->getFormEditData();

                if (pData && !aTimer.IsActive()) // it won't be destroyed over Close;
                {
                    pData->SetUniqueId(nActivWinId);
                }
            }
        }
    }
}
uno::Reference< sheet::XFormulaOpCodeMapper > FormulaDlg_Impl::GetFormulaOpCodeMapper() const
{
    if ( !m_xOpCodeMapper.is() )
    {
        m_xOpCodeMapper = m_pHelper->getFormulaOpCodeMapper();
        m_aFunctionOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::FUNCTIONS);
        m_pFunctionOpCodesEnd = m_aFunctionOpCodes.getConstArray() + m_aFunctionOpCodes.getLength();

        m_aUnaryOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::UNARY_OPERATORS);
        m_pUnaryOpCodesEnd = m_aUnaryOpCodes.getConstArray() + m_aUnaryOpCodes.getLength();

        m_aBinaryOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::BINARY_OPERATORS);
        m_pBinaryOpCodesEnd = m_aBinaryOpCodes.getConstArray() + m_aBinaryOpCodes.getLength();

        uno::Sequence< ::rtl::OUString > aArgs(3);
        aArgs[TOKEN_OPEN]   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("));
        aArgs[TOKEN_CLOSE]  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
        aArgs[TOKEN_SEP]    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
        m_aSeparatorsOpCodes = m_xOpCodeMapper->getMappings(aArgs,sheet::FormulaLanguage::ODFF);

        m_aSpecialOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::SPECIAL);
        m_pSpecialOpCodesEnd = m_aSpecialOpCodes.getConstArray() + m_aSpecialOpCodes.getLength();
    } // if ( !m_xOpCodeMapper.is() )
    return m_xOpCodeMapper;
}

void FormulaDlg_Impl::DeleteArgs()
{
    ::std::vector< ::rtl::OUString>().swap(m_aArguments);
    nArgs = 0;
}
namespace
{
    // comparing two property instances
    struct OpCodeCompare : public ::std::binary_function< sheet::FormulaOpCodeMapEntry, sal_Int32 , bool >
    {
        bool operator() (const sheet::FormulaOpCodeMapEntry& x, sal_Int32 y) const
        {
            return x.Token.OpCode == y;
        }
    };
}
// -----------------------------------------------------------------------------
xub_StrLen FormulaDlg_Impl::GetFunctionPos(xub_StrLen nPos)
{
    const sal_Unicode sep = m_pHelper->getFunctionManager()->getSingleToken(IFunctionManager::eSep);

    xub_StrLen nTokPos=1;
    xub_StrLen nOldTokPos=1;
    xub_StrLen nFuncPos=STRING_NOTFOUND;    //@ Testwise
    xub_StrLen nPrevFuncPos=1;
    short  nBracketCount=0;
    BOOL   bFlag=FALSE;
    String aFormString = pMEdit->GetText();
    m_aFormulaHelper.GetCharClass()->toUpper( aFormString );

    if ( m_aTokenList.getLength() )
    {
        const uno::Reference< sheet::XFormulaParser > xParser(m_pHelper->getFormulaParser());
        const table::CellAddress aRefPos(m_pHelper->getReferencePosition());

        const sheet::FormulaToken* pIter = m_aTokenList.getConstArray();
        const sheet::FormulaToken* pEnd = pIter + m_aTokenList.getLength();
        try
        {
            while ( pIter != pEnd )
            {
                const sal_Int32 eOp = pIter->OpCode;
                uno::Sequence<sheet::FormulaToken> aArgs(1);
                aArgs[0] = *pIter;
                const String aString = xParser->printFormula(aArgs, aRefPos);
                const sheet::FormulaToken* pNextToken = pIter + 1;

                if(!bUserMatrixFlag && FormulaCompiler::IsMatrixFunction((OpCode)eOp) )
                {
                    aBtnMatrix.Check();
                }

                if ( eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::PUSH].Token.OpCode || eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode )
                {
                    const xub_StrLen n1=aFormString.Search(sep, nTokPos);
                    const xub_StrLen n2=aFormString.Search(')',nTokPos);
                    xub_StrLen nXXX=nTokPos;
                    if(n1<n2)
                    {
                        nTokPos=n1;
                    }
                    else
                    {
                        nTokPos=n2;
                    }
                    if ( pNextToken != pEnd )
                    {
                        aArgs[0] = *pNextToken;
                        const String a2String = xParser->printFormula(aArgs, aRefPos);
                        const xub_StrLen n3 = aFormString.Search(a2String,nXXX);
                        if ( n3 < nTokPos )
                            nTokPos = n3;
                    }
                }
                else
                {
                    nTokPos = sal::static_int_cast<xub_StrLen>( nTokPos + aString.Len() );
                }

                if ( eOp == m_aSeparatorsOpCodes[TOKEN_OPEN].OpCode )
                {
                    nBracketCount++;
                    bFlag=TRUE;
                }
                else if ( eOp == m_aSeparatorsOpCodes[TOKEN_CLOSE].OpCode )
                {
                    nBracketCount--;
                    bFlag=FALSE;
                    nFuncPos=nPrevFuncPos;
                }
                bool bIsFunction = ::std::find_if(m_aFunctionOpCodes.getConstArray(),m_pFunctionOpCodesEnd,::std::bind2nd(OpCodeCompare(),boost::cref(eOp))) != m_pFunctionOpCodesEnd;

                if ( bIsFunction && m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode != eOp )
                {
                    nPrevFuncPos = nFuncPos;
                    nFuncPos = nOldTokPos;
                }

                if ( nOldTokPos <= nPos && nPos < nTokPos )
                {
                    if ( !bIsFunction )
                    {
                        if ( nBracketCount < 1 )
                        {
                            nFuncPos= pMEdit->GetText().Len();
                        }
                        else if ( !bFlag )
                        {
                            nFuncPos=nPrevFuncPos;
                        }
                    }
                    break;
                }

                pIter = pNextToken;
                nOldTokPos = nTokPos;
            } // while ( pIter != pEnd )
        }
        catch(const uno::Exception& )
        {
            OSL_FAIL("Exception caught!");
        }
    }

    return nFuncPos;
}
// -----------------------------------------------------------------------------
BOOL FormulaDlg_Impl::CalcValue( const String& rStrExp, String& rStrResult )
{
    BOOL bResult = TRUE;

    if ( rStrExp.Len() > 0 )
    {
        // Only calculate the value when there isn't any more keyboard input:

        if ( !Application::AnyInput( INPUT_KEYBOARD ) )
        {
            bResult = m_pHelper->calculateValue(rStrExp,rStrResult);
        }
        else
            bResult = FALSE;
    }

    return bResult;
}

void FormulaDlg_Impl::UpdateValues()
{
    String aStrResult;

    if ( CalcValue( pFuncDesc->getFormula( m_aArguments ), aStrResult ) )
        aWndResult.SetValue( aStrResult );

    aStrResult.Erase();
    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(pMEdit->GetText());
}

BOOL FormulaDlg_Impl::CalcStruct( const String& rStrExp)
{
    BOOL bResult = TRUE;
    xub_StrLen nLength=rStrExp.Len();

    if ( rStrExp.Len() > 0 && aOldFormula!=rStrExp && bStructUpdate)
    {
        // Only calculate the value when there isn't any more keyboard input:

        if ( !Application::AnyInput( INPUT_KEYBOARD ) )
        {
            pStructPage->ClearStruct();

            String aString=rStrExp;
            if(rStrExp.GetChar(nLength-1)=='(')
            {
                aString.Erase((xub_StrLen)(nLength-1));
            }

            aString.EraseAllChars('\n');
            String aStrResult;

            if ( CalcValue(aString, aStrResult ) )
                aWndFormResult.SetValue( aStrResult );

            UpdateTokenArray(aString);
            fillTree(pStructPage);

            aOldFormula=rStrExp;
            if(rStrExp.GetChar(nLength-1)=='(')
                UpdateTokenArray(rStrExp);
        }
        else
            bResult = FALSE;
    }
    return bResult;
}

// -----------------------------------------------------------------------------
void FormulaDlg_Impl::MakeTree(IStructHelper* _pTree,SvLBoxEntry* pParent,FormulaToken* _pToken,long Count)
{
    if( _pToken != NULL && Count > 0 )
    {
        long nParas = _pToken->GetParamCount();
        OpCode eOp = _pToken->GetOpCode();

        // #i101512# for output, the original token is needed
        FormulaToken* pOrigToken = (_pToken->GetType() == svFAP) ? _pToken->GetFAPOrigToken() : _pToken;
        uno::Sequence<sheet::FormulaToken> aArgs(1);
        aArgs[0] = m_aTokenMap.find(pOrigToken)->second;
        try
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const String aResult = m_pHelper->getFormulaParser()->printFormula(aArgs, aRefPos);

            if ( nParas > 0 )
            {
                SvLBoxEntry* pEntry;

                String aTest=_pTree->GetEntryText(pParent);

                if(aTest==aResult &&
                    (eOp==ocAdd || eOp==ocMul ||
                     eOp==ocAmpersand))
                {
                    pEntry=pParent;
                }
                else
                {
                    if(eOp==ocBad)
                    {
                        pEntry=_pTree->InsertEntry(aResult,pParent,STRUCT_ERROR,0,_pToken);
                    }
                    else
                    {
                        pEntry=_pTree->InsertEntry(aResult,pParent,STRUCT_FOLDER,0,_pToken);
                    }
                }

                MakeTree(_pTree,pEntry,m_pTokenArray->PrevRPN(),nParas);
                --Count;
                m_pTokenArray->NextRPN();
                MakeTree(_pTree,pParent,m_pTokenArray->PrevRPN(),Count);
            }
            else
            {
                if(eOp==ocBad)
                {
                    _pTree->InsertEntry(aResult,pParent,STRUCT_ERROR,0,_pToken);
                }
                else
                {
                    _pTree->InsertEntry(aResult,pParent,STRUCT_END,0,_pToken);
                }
                --Count;
                MakeTree(_pTree,pParent,m_pTokenArray->PrevRPN(),Count);
            }
        }
        catch(uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void FormulaDlg_Impl::fillTree(IStructHelper* _pTree)
{
    GetFormulaOpCodeMapper();
    FormulaToken* pToken = m_pTokenArray->LastRPN();

    if( pToken != NULL)
    {
        MakeTree(_pTree,NULL,pToken,1);
    }
}
void FormulaDlg_Impl::UpdateTokenArray( const String& rStrExp)
{
    m_aTokenMap.clear();
    m_aTokenList.realloc(0);
    try
    {
        const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
        m_aTokenList = m_pHelper->getFormulaParser()->parseFormula(rStrExp, aRefPos);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    GetFormulaOpCodeMapper(); // just to get it initialized
    m_pTokenArray = m_pHelper->convertToTokenArray(m_aTokenList);
    const sal_Int32 nLen = static_cast<sal_Int32>(m_pTokenArray->GetLen());
    FormulaToken** pTokens = m_pTokenArray->GetArray();
    if ( pTokens && nLen == m_aTokenList.getLength() )
    {
        for (sal_Int32 nPos=0; nPos<nLen; nPos++)
        {
            m_aTokenMap.insert(::std::map<FormulaToken*,sheet::FormulaToken>::value_type(pTokens[nPos],m_aTokenList[nPos]));
        }
    } // if ( pTokens && nLen == m_aTokenList.getLength() )

    FormulaCompiler aCompiler(*m_pTokenArray.get());
    aCompiler.SetCompileForFAP(TRUE);   // #i101512# special handling is needed
    aCompiler.CompileTokenArray();
}

void FormulaDlg_Impl::FillDialog(BOOL nFlag)
{
    BOOL bNext=TRUE, bPrev=TRUE;
    if(nFlag)
        FillControls(bNext, bPrev);
    FillListboxes();
    if(nFlag)
    {
        aBtnBackward.Enable(bPrev);
        aBtnForward.Enable(bNext);
    }

    String aStrResult;

    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }
}

// -----------------------------------------------------------------------------
void FormulaDlg_Impl::FillListboxes()
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    String aNewTitle;
    //  1. Page: select function
    if ( pFuncDesc && pFuncDesc->getCategory() )
    {
        if( pFuncPage->GetCategory() != pFuncDesc->getCategory()->getNumber() + 1 )
            pFuncPage->SetCategory(static_cast<USHORT>(pFuncDesc->getCategory()->getNumber() + 1));

        USHORT nPos=pFuncPage->GetFuncPos(pFuncDesc);

        pFuncPage->SetFunction(nPos);
    }
    else if ( pData )
    {
        pFuncPage->SetCategory( pData->GetCatSel() );
        pFuncPage->SetFunction( pData->GetFuncSel() );
    }
    FuncSelHdl(NULL);

    //  ResizeArgArr is now already in UpdateFunctionDesc


    m_pHelper->setDispatcherLock( TRUE);// Activate Modal-Mode

    aNewTitle = aTitle1;

    //  HelpId for 1. page is the one from the resource
    m_pParent->SetHelpId( nOldHelp );
    m_pParent->SetUniqueId( nOldUnique );
}
// -----------------------------------------------------------------------------
void FormulaDlg_Impl::FillControls(BOOL &rbNext, BOOL &rbPrev)
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData )
        return;

    String aNewTitle;
    //  2. Page or Edit: show selected function

    xub_StrLen nFStart     = pData->GetFStart();
    String aFormula        = m_pHelper->getCurrentFormula();
    xub_StrLen nNextFStart = nFStart;
    xub_StrLen nNextFEnd   = 0;

    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " )" ));
    DeleteArgs();
    const IFunctionDescription* pOldFuncDesc = pFuncDesc;
    BOOL bTestFlag = FALSE;

    if ( m_aFormulaHelper.GetNextFunc( aFormula, FALSE,
                                     nNextFStart, &nNextFEnd, &pFuncDesc, &m_aArguments ) )
    {
        bTestFlag = (pOldFuncDesc != pFuncDesc);
        if(bTestFlag)
        {
            aFtHeadLine.Hide();
            aFtFuncName.Hide();
            aFtFuncDesc.Hide();
            pParaWin->SetFunctionDesc(pFuncDesc);
            aFtEditName.SetText( pFuncDesc->getFunctionName() );
            aFtEditName.Show();
            pParaWin->Show();
            const long nHelpId = pFuncDesc->getHelpId();
            if ( nHelpId )
                pMEdit->SetSmartHelpId(SmartId(nHelpId));
        }

        xub_StrLen nOldStart, nOldEnd;
        m_pHelper->getSelection( nOldStart, nOldEnd );
        if ( nOldStart != nNextFStart || nOldEnd != nNextFEnd )
        {
            m_pHelper->setSelection( nNextFStart, nNextFEnd );
        }
        aFuncSel.Min() = nNextFStart;
        aFuncSel.Max() = nNextFEnd;

        if(!bEditFlag)
            pMEdit->SetText(m_pHelper->getCurrentFormula());
        xub_StrLen PrivStart, PrivEnd;
        m_pHelper->getSelection( PrivStart, PrivEnd);
        if(!bEditFlag)
            pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

        nArgs = pFuncDesc->getSuppressedArgumentCount();
        USHORT nOffset = pData->GetOffset();
        nEdFocus = pData->GetEdFocus();

        //  Concatenate the Edit's for Focus-Control

        if(bTestFlag)
            pParaWin->SetArgumentOffset(nOffset);
        USHORT nActiv=0;
        xub_StrLen nArgPos= m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        xub_StrLen nEditPos=(xub_StrLen) pMEdit->GetSelection().Min();
        BOOL    bFlag=FALSE;

        for(USHORT i=0;i<nArgs;i++)
        {
            sal_Int32 nLength = m_aArguments[i].getLength()+1;
            pParaWin->SetArgument(i,m_aArguments[i]);
            if(nArgPos<=nEditPos && nEditPos<nArgPos+nLength)
            {
                nActiv=i;
                bFlag=TRUE;
            }
            nArgPos = sal::static_int_cast<xub_StrLen>( nArgPos + nLength );
        }
        pParaWin->UpdateParas();

        if(bFlag)
        {
            pParaWin->SetActiveLine(nActiv);
        }

        //pParaWin->SetEdFocus( nEdFocus );
        UpdateValues();
    }
    else
    {
        aFtEditName.SetText(String());
        pMEdit->SetSmartHelpId(m_aSmartEditHelpId);
    }
        //  Test, ob vorne/hinten noch mehr Funktionen sind

    xub_StrLen nTempStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
    rbNext = m_aFormulaHelper.GetNextFunc( aFormula, FALSE, nTempStart );
    nTempStart=(xub_StrLen)pMEdit->GetSelection().Min();
    pData->SetFStart(nTempStart);
    rbPrev = m_aFormulaHelper.GetNextFunc( aFormula, TRUE, nTempStart );
}
// -----------------------------------------------------------------------------

void FormulaDlg_Impl::ClearAllParas()
{
    DeleteArgs();
    pFuncDesc = NULL;
    pParaWin->ClearAll();
    aWndResult.SetValue(String());
    aFtFuncName.SetText(String());
    FuncSelHdl(NULL);

    if(pFuncPage->IsVisible())
    {
        aFtEditName.Hide();
        pParaWin->Hide();

        aBtnForward.Enable(TRUE); //@new
        aFtHeadLine.Show();
        aFtFuncName.Show();
        aFtFuncDesc.Show();
    }
}
String FormulaDlg_Impl::RepairFormula(const String& aFormula)
{
    String aResult('=');
    try
    {
        UpdateTokenArray(aFormula);

        if ( m_aTokenList.getLength() )
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const String sFormula(m_pHelper->getFormulaParser()->printFormula(m_aTokenList, aRefPos));
            if ( !sFormula.Len() || sFormula.GetChar(0) != '=' )
                aResult += sFormula;
            else
                aResult = sFormula;

        }
    }
    catch(const uno::Exception& )
    {
        OSL_FAIL("Exception caught!");
    }
    return aResult;
}

void FormulaDlg_Impl::DoEnter(BOOL bOk)
{
    //  Accept input to the document or cancel
    if ( bOk)
    {
        //  remove dummy arguments
        String  aInputFormula = m_pHelper->getCurrentFormula();
        String  aString = RepairFormula(pMEdit->GetText());
        m_pHelper->setSelection(0, aInputFormula.Len());
        m_pHelper->setCurrentFormula(aString);
    }

    m_pHelper->switchBack();

    m_pHelper->dispatch(bOk,aBtnMatrix.IsChecked());
    //  Clear data
    m_pHelper->deleteFormData();

    //  Close dialog
    m_pHelper->doClose(bOk);
}
// -----------------------------------------------------------------------------

IMPL_LINK( FormulaDlg_Impl, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnCancel )
    {
        DoEnter(FALSE);                 // closes the Dialog
    }
    else if ( pBtn == &aBtnEnd )
    {
        DoEnter(TRUE);                  // closes the Dialog
    }
    else if ( pBtn == &aBtnForward )
    {
        //@pMEdit->GrabFocus();         // In order to show the selection too
        const IFunctionDescription* pDesc =pFuncPage->GetFuncDesc( pFuncPage->GetFunction() );

        if(pDesc==pFuncDesc || !pFuncPage->IsVisible())
            EditNextFunc( TRUE );
        else
        {
            DblClkHdl(pFuncPage);      //new
            aBtnForward.Enable(FALSE); //new
        }
        //@EditNextFunc( TRUE );
    }
    else if ( pBtn == &aBtnBackward )
    {
        bEditFlag=FALSE;
        aBtnForward.Enable(TRUE);
        EditNextFunc( FALSE );
        aMEFormula.Invalidate();
        aMEFormula.Update();
    }
    //...

    return 0;
}
// -----------------------------------------------------------------------------


//  --------------------------------------------------------------------------
//                          Functions for 1. Page
//  --------------------------------------------------------------------------

void FormulaDlg_Impl::ResizeArgArr( const IFunctionDescription* pNewFunc )
{
    if ( pFuncDesc != pNewFunc )
    {
        DeleteArgs();

        if ( pNewFunc )
            nArgs = pNewFunc->getSuppressedArgumentCount();

        pFuncDesc = pNewFunc;
    }
}
// -----------------------------------------------------------------------------

void FormulaDlg_Impl::UpdateFunctionDesc()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;
    USHORT nCat = pFuncPage->GetCategory();
    if ( nCat == LISTBOX_ENTRY_NOTFOUND )
        nCat = 0;
    pData->SetCatSel( nCat );
    USHORT nFunc = pFuncPage->GetFunction();
    if ( nFunc == LISTBOX_ENTRY_NOTFOUND )
        nFunc = 0;
    pData->SetFuncSel( nFunc );

    if (   (pFuncPage->GetFunctionEntryCount() > 0)
        && (pFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        const IFunctionDescription* pDesc = pFuncPage->GetFuncDesc(pFuncPage->GetFunction() );
        if (pDesc)
        {
            pDesc->initArgumentInfo();      // full argument info is needed

            String aSig = pDesc->getSignature();

            aFtFuncName.SetText( aSig );
            aFtFuncDesc.SetText( pDesc->getDescription() );
            ResizeArgArr( pDesc );

            if ( !m_aArguments.empty() )        // still arguments there?
                aSig = pDesc->getFormula( m_aArguments );           // for input line
            //@ m_pHelper->setCurrentFormula( aSig );
        }
    }
    else
    {
        aFtFuncName.SetText( String() );
        aFtFuncDesc.SetText( String() );

        //ResizeArgArr( NULL );
        m_pHelper->setCurrentFormula( String() );
    }
}
// -----------------------------------------------------------------------------

// Handler for Listboxes

IMPL_LINK( FormulaDlg_Impl, DblClkHdl, FuncPage*, EMPTYARG )
{
    USHORT nFunc = pFuncPage->GetFunction();

    //  ex-UpdateLRUList
    const IFunctionDescription* pDesc = pFuncPage->GetFuncDesc(nFunc);
    m_pHelper->insertEntryToLRUList(pDesc);

    String aFuncName = pFuncPage->GetSelFunctionName();
    aFuncName.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
    m_pHelper->setCurrentFormula(aFuncName);
    pMEdit->ReplaceSelected(aFuncName);

    Selection aSel=pMEdit->GetSelection();
    aSel.Max()=aSel.Max()-1;
    pMEdit->SetSelection(aSel);

    FormulaHdl(pMEdit);

    aSel.Min()=aSel.Max();
    pMEdit->SetSelection(aSel);

    if(nArgs==0)
    {
        BtnHdl(&aBtnBackward);
    }

    pParaWin->SetEdFocus(0);
    aBtnForward.Enable(FALSE); //@New

    return 0;
}
// -----------------------------------------------------------------------------

//  --------------------------------------------------------------------------
//                          Functions for right Page
//  --------------------------------------------------------------------------
void FormulaDlg_Impl::SetData(xub_StrLen nFStart,xub_StrLen nNextFStart,xub_StrLen nNextFEnd,xub_StrLen& PrivStart,xub_StrLen& PrivEnd)
{
    xub_StrLen nFEnd;

    // Notice and set new selection
    m_pHelper->getSelection( nFStart, nFEnd );
    m_pHelper->setSelection( nNextFStart, nNextFEnd );
    if(!bEditFlag)
        pMEdit->SetText(m_pHelper->getCurrentFormula());


    m_pHelper->getSelection( PrivStart, PrivEnd);
    if(!bEditFlag)
    {
        pMEdit->SetSelection( Selection(PrivStart, PrivEnd));
        aMEFormula.UpdateOldSel();
    }

    FormEditData* pData = m_pHelper->getFormEditData();
    pData->SetFStart( nNextFStart );
    pData->SetOffset( 0 );
    pData->SetEdFocus( 0 );

    FillDialog();
}
// -----------------------------------------------------------------------------
void FormulaDlg_Impl::EditThisFunc(xub_StrLen nFStart)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return;

    String aFormula = m_pHelper->getCurrentFormula();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    xub_StrLen nNextFStart  = nFStart;
    xub_StrLen nNextFEnd    = 0;

    BOOL bFound;

    //@bFound = m_pHelper->getNextFunction( aFormula, FALSE, nNextFStart, &nNextFEnd, &pFuncDesc );

    bFound = m_aFormulaHelper.GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd);
    if ( bFound )
    {
        xub_StrLen PrivStart, PrivEnd;
        SetData(nFStart,nNextFStart,nNextFEnd,PrivStart, PrivEnd);
        m_pHelper->showReference(aFormula.Copy(PrivStart, PrivEnd-PrivStart));
    }
    else
    {
        ClearAllParas();
    }
}

void FormulaDlg_Impl::EditNextFunc( BOOL bForward, xub_StrLen nFStart )
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    String aFormula = m_pHelper->getCurrentFormula();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    xub_StrLen nNextFStart  = 0;
    xub_StrLen nNextFEnd    = 0;

    BOOL bFound;
    if ( bForward )
    {
        nNextFStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        //@bFound = m_pHelper->getNextFunction( aFormula, FALSE, nNextFStart, &nNextFEnd, &pFuncDesc );
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd);
    }
    else
    {
        nNextFStart = nFStart;
        //@bFound = m_pHelper->getNextFunction( aFormula, TRUE, nNextFStart, &nNextFEnd, &pFuncDesc );
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, TRUE, nNextFStart, &nNextFEnd);
    }

    if ( bFound )
    {
        xub_StrLen PrivStart, PrivEnd;
        SetData(nFStart,nNextFStart,nNextFEnd,PrivStart, PrivEnd);
    }
}

void FormulaDlg_Impl::EditFuncParas(xub_StrLen nEditPos)
{
    if(pFuncDesc!=NULL)
    {
        FormEditData* pData = m_pHelper->getFormEditData();
        if (!pData) return;

        String aFormula = m_pHelper->getCurrentFormula();
        aFormula +=')';
        xub_StrLen nFStart = pData->GetFStart();

        DeleteArgs();

        nArgs = pFuncDesc->getSuppressedArgumentCount();

        sal_Int32 nArgPos=m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        m_aFormulaHelper.GetArgStrings(m_aArguments,aFormula, nFStart, nArgs );
//      m_aArguments = ScFormulaUtil::GetArgStrings( aFormula, nFStart, nArgs );

        USHORT nActiv=pParaWin->GetSliderPos();
        BOOL    bFlag=FALSE;
        ::std::vector< ::rtl::OUString >::iterator aIter = m_aArguments.begin();
        ::std::vector< ::rtl::OUString >::iterator aEnd = m_aArguments.end();
        for(USHORT i=0;aIter != aEnd;i++,++aIter)
        {
            sal_Int32 nLength=(*aIter).getLength();
            pParaWin->SetArgument(i,(*aIter));
            if(nArgPos<=nEditPos && nEditPos<nArgPos+nLength)
            {
                nActiv=i;
                bFlag=TRUE;
            }
            nArgPos+=nLength+1;
        }

        if(bFlag)
        {
            pParaWin->SetSliderPos(nActiv);
        }

        pParaWin->UpdateParas();
        UpdateValues();
    }

}

void FormulaDlg_Impl::SaveArg( USHORT nEd )
{
    if (nEd<nArgs)
    {
        USHORT i;
        for(i=0;i<=nEd;i++)
        {
            if ( m_aArguments[i].getLength() == 0 )
                m_aArguments[i] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
        }
        if(pParaWin->GetArgument(nEd).Len()!=0)
            m_aArguments[nEd] = pParaWin->GetArgument(nEd);

        USHORT nClearPos=nEd+1;
        for(i=nEd+1;i<nArgs;i++)
        {
            if(pParaWin->GetArgument(i).Len()!=0)
            {
                nClearPos=i+1;
            }
        }

        for(i=nClearPos;i<nArgs;i++)
        {
            m_aArguments[i] = ::rtl::OUString();
        }
    }
}

IMPL_LINK( FormulaDlg_Impl, FxHdl, ParaWin*, pPtr )
{
    if(pPtr==pParaWin)
    {
        aBtnForward.Enable(TRUE); //@ In order to be able to input another function.
        aTabCtrl.SetCurPageId(TP_FUNCTION);

        String aUndoStr = m_pHelper->getCurrentFormula();       // it will be added before a ";"
        FormEditData* pData = m_pHelper->getFormEditData();
        if (!pData) return 0;

        USHORT nArgNo = pParaWin->GetActiveLine();
        nEdFocus=nArgNo;

        SaveArg(nArgNo);
        UpdateSelection();

        xub_StrLen nFormulaStrPos = pData->GetFStart();

        String aFormula = m_pHelper->getCurrentFormula();
        xub_StrLen n1 = m_aFormulaHelper.GetArgStart( aFormula, nFormulaStrPos, nEdFocus+pData->GetOffset() );

        pData->SetEdFocus( nEdFocus );
        pData->SaveValues();
        pData->SetMode( (USHORT) FORMULA_FORMDLG_FORMULA );
        pData->SetFStart( n1 );
        pData->SetUndoStr( aUndoStr );
        ClearAllParas();

        FillDialog(FALSE);
        pFuncPage->SetFocus(); //There Parawin is not visible anymore
    }
    return 0;
}

IMPL_LINK( FormulaDlg_Impl, ModifyHdl, ParaWin*, pPtr )
{
    if(pPtr==pParaWin)
    {
        SaveArg(pParaWin->GetActiveLine());
        UpdateValues();

        UpdateSelection();
        CalcStruct(pMEdit->GetText());
    }
    return 0;
}

IMPL_LINK( FormulaDlg_Impl, FormulaHdl, MultiLineEdit*, EMPTYARG )
{

    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return 0;

    bEditFlag=TRUE;
    String      aInputFormula=m_pHelper->getCurrentFormula();
    String      aString=pMEdit->GetText();

    Selection   aSel =pMEdit->GetSelection();
    xub_StrLen nTest=0;

    if(aString.Len()==0) //in case everything was cleared
    {
        aString +='=';
        pMEdit->SetText(aString);
        aSel .Min()=1;
        aSel .Max()=1;
        pMEdit->SetSelection(aSel);
    }
    else if(aString.GetChar(nTest)!='=') //in case it's replaced;
    {
        aString.Insert( (sal_Unicode)'=', 0 );
        pMEdit->SetText(aString);
        aSel .Min()+=1;
        aSel .Max()+=1;
        pMEdit->SetSelection(aSel);
    }


    m_pHelper->setSelection(0, aInputFormula.Len());
    m_pHelper->setCurrentFormula(aString);
    m_pHelper->setSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());

    xub_StrLen nPos=(xub_StrLen)aSel.Min()-1;

    String aStrResult;

    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(aString);

    nPos=GetFunctionPos(nPos);

    if(nPos<aSel.Min()-1)
    {
        xub_StrLen nPos1=aString.Search('(',nPos);
        EditNextFunc( FALSE, nPos1);
    }
    else
    {
        ClearAllParas();
    }

    m_pHelper->setSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());
    bEditFlag=FALSE;
    return 0;
}

IMPL_LINK( FormulaDlg_Impl, FormulaCursorHdl, EditBox*, EMPTYARG )
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return 0;
    xub_StrLen nFStart = pData->GetFStart();

    bEditFlag=TRUE;

    String      aInputFormula=m_pHelper->getCurrentFormula();
    String      aString=pMEdit->GetText();

    Selection   aSel =pMEdit->GetSelection();
    m_pHelper->setSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());

    if(aSel.Min()==0)
    {
        aSel.Min()=1;
        pMEdit->SetSelection(aSel);
    }

    if(aSel.Min()!=aString.Len())
    {
        xub_StrLen nPos=(xub_StrLen)aSel.Min();

        nFStart=GetFunctionPos(nPos - 1);

        if(nFStart<nPos)
        {
            xub_StrLen nPos1=m_aFormulaHelper.GetFunctionEnd(aString,nFStart);

            if(nPos1>nPos || nPos1==STRING_NOTFOUND)
            {
                EditThisFunc(nFStart);
            }
            else
            {
                xub_StrLen n=nPos;
                short nCount=1;
                while(n>0)
                {
                   if(aString.GetChar(n)==')')
                       nCount++;
                   else if(aString.GetChar(n)=='(')
                       nCount--;
                   if(nCount==0) break;
                   n--;
                }
                if(nCount==0)
                {
                    nFStart=m_aFormulaHelper.GetFunctionStart(aString,n,TRUE);
                    EditThisFunc(nFStart);
                }
                else
                {
                    ClearAllParas();
                }
            }
        }
        else
        {
            ClearAllParas();
        }
    }
    m_pHelper->setSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());

    bEditFlag=FALSE;
    return 0;
}

void FormulaDlg_Impl::UpdateSelection()
{
    m_pHelper->setSelection((xub_StrLen)aFuncSel.Min(),(xub_StrLen)aFuncSel.Max());
    m_pHelper->setCurrentFormula( pFuncDesc->getFormula( m_aArguments ) );
    pMEdit->SetText(m_pHelper->getCurrentFormula());
    xub_StrLen PrivStart, PrivEnd;
    m_pHelper->getSelection( PrivStart, PrivEnd);
    aFuncSel.Min()=PrivStart;
    aFuncSel.Max()=PrivEnd;

    nArgs = pFuncDesc->getSuppressedArgumentCount();

    String aFormula=pMEdit->GetText();
    sal_Int32 nArgPos=m_aFormulaHelper.GetArgStart( aFormula,PrivStart,0);

    USHORT nPos=pParaWin->GetActiveLine();

    for(USHORT i=0;i<nPos;i++)
    {
        nArgPos += (m_aArguments[i].getLength() + 1);
    }
    sal_Int32 nLength= m_aArguments[nPos].getLength();

    Selection aSel(nArgPos,nArgPos+nLength);
    m_pHelper->setSelection((USHORT)nArgPos,(USHORT)(nArgPos+nLength));
    pMEdit->SetSelection(aSel);
    aMEFormula.UpdateOldSel();
}
::std::pair<RefButton*,RefEdit*> FormulaDlg_Impl::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    aEdRef.Show();
    pTheRefEdit = pEdit;
    pTheRefButton = pButton;

    if( pTheRefEdit )
    {
        aEdRef.SetRefString( pTheRefEdit->GetText() );
        aEdRef.SetSelection( pTheRefEdit->GetSelection() );
        aEdRef.SetHelpId( pTheRefEdit->GetHelpId() );
        aEdRef.SetUniqueId( pTheRefEdit->GetUniqueId() );
    }

    aRefBtn.Show( pButton != NULL );

    //m_pHelper->RefInputStart( &aEdRef, pButton ? &aRefBtn : NULL );
    ::std::pair<RefButton*,RefEdit*> aPair;
    aPair.first = pButton ? &aRefBtn : NULL;
    aPair.second = &aEdRef;
    return aPair;
}
void FormulaDlg_Impl::RefInputStartAfter( RefEdit* /*pEdit*/, RefButton* /*pButton*/ )
{
    aRefBtn.SetEndImage();

    if( pTheRefEdit )
    {
        String aStr = aTitle2;
        aStr += ' ';
        aStr += aFtEditName.GetText();
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "( " ) );
        if( pParaWin->GetActiveLine() > 0 )
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "...; " ) );
        aStr += pParaWin->GetActiveArgName();
        if( pParaWin->GetActiveLine() + 1 < nArgs )
            aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "; ..." ));
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " )" ) );

        m_pParent->SetText( MnemonicGenerator::EraseAllMnemonicChars( aStr ) );
    }
}
void FormulaDlg_Impl::RefInputDoneAfter( BOOL bForced )
{
    aRefBtn.SetStartImage();
    if( bForced || !aRefBtn.IsVisible() )
    {
        aEdRef.Hide();
        aRefBtn.Hide();
        if( pTheRefEdit )
        {
            pTheRefEdit->SetRefString( aEdRef.GetText() );
            pTheRefEdit->GrabFocus();

            if( pTheRefButton )
                pTheRefButton->SetStartImage();

            USHORT nPrivActiv = pParaWin->GetActiveLine();
            pParaWin->SetArgument( nPrivActiv, aEdRef.GetText() );
            ModifyHdl( pParaWin );
            pTheRefEdit = NULL;
        }
        m_pParent->SetText( aTitle1 );
    }
}
RefEdit* FormulaDlg_Impl::GetCurrRefEdit()
{
    return aEdRef.IsVisible() ? &aEdRef : pParaWin->GetActiveEdit();
}
void FormulaDlg_Impl::Update()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    const String sExpression = pMEdit->GetText();
    aOldFormula = String();
    UpdateTokenArray(sExpression);
    FormulaCursorHdl(&aMEFormula);
    CalcStruct(sExpression);
    if(pData->GetMode() == FORMULA_FORMDLG_FORMULA)
        aTabCtrl.SetCurPageId(TP_FUNCTION);
    else
        aTabCtrl.SetCurPageId(TP_STRUCT);
    aBtnMatrix.Check(pData->GetMatrixFlag());
    /*aTimer.SetTimeout(200);
    aTimer.SetTimeoutHdl(LINK( this, FormulaDlg_Impl, UpdateFocusHdl));
    aTimer.Start();*/
}
void FormulaDlg_Impl::Update(const String& _sExp)
{
    CalcStruct(_sExp);
    FillDialog();
    //aBtnForward.Enable(TRUE); //@New
    FuncSelHdl(NULL);
}
void FormulaDlg_Impl::SetMeText(const String& _sText)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    pMEdit->SetText(_sText);
    pMEdit->SetSelection( pData->GetSelection());
    aMEFormula.UpdateOldSel();
}
FormulaDlgMode FormulaDlg_Impl::SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,BOOL bMatrix,BOOL _bSelect,BOOL _bUpdate)
{
    FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;
    if(!bEditFlag)
        pMEdit->SetText(_sText);

    if ( _bSelect || !bEditFlag )
        pMEdit->SetSelection( Selection(PrivStart, PrivEnd));
    if ( _bUpdate )
    {
        aMEFormula.UpdateOldSel();
        pMEdit->Invalidate();
        m_pHelper->showReference(pMEdit->GetSelected());
        eMode = FORMULA_FORMDLG_EDIT;

        aBtnMatrix.Check( bMatrix );
    } // if ( _bUpdate )
    return eMode;
}
BOOL FormulaDlg_Impl::CheckMatrix(String& aFormula)
{
    pMEdit->GrabFocus();
    xub_StrLen nLen = aFormula.Len();
    BOOL bMatrix =  nLen > 3                    // Matrix-Formula ?
            && aFormula.GetChar(0) == '{'
            && aFormula.GetChar(1) == '='
            && aFormula.GetChar(nLen-1) == '}';
    if ( bMatrix )
    {
        aFormula.Erase( 0, 1 );
        aFormula.Erase( aFormula.Len()-1, 1);
        aBtnMatrix.Check( bMatrix );
        aBtnMatrix.Disable();
    } // if ( bMatrix )

    aTabCtrl.SetCurPageId(TP_STRUCT);
    return bMatrix;
}
IMPL_LINK( FormulaDlg_Impl, StructSelHdl, StructPage*, EMPTYARG )
{
    bStructUpdate=FALSE;
    if(pStructPage->IsVisible())    aBtnForward.Enable(FALSE); //@New

    bStructUpdate=TRUE;
    return 0;
}
IMPL_LINK( FormulaDlg_Impl, MatrixHdl, CheckBox *, EMPTYARG )
{
    bUserMatrixFlag=TRUE;
    return 0;
}

IMPL_LINK( FormulaDlg_Impl, FuncSelHdl, FuncPage*, EMPTYARG )
{
    USHORT nCat = pFuncPage->GetCategory();
    if ( nCat == LISTBOX_ENTRY_NOTFOUND ) nCat = 0;
    USHORT nFunc = pFuncPage->GetFunction();
    if ( nFunc == LISTBOX_ENTRY_NOTFOUND ) nFunc = 0;

    if (   (pFuncPage->GetFunctionEntryCount() > 0)
        && (pFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        const IFunctionDescription* pDesc =pFuncPage->GetFuncDesc( pFuncPage->GetFunction() );

        if(pDesc!=pFuncDesc) aBtnForward.Enable(TRUE); //new

        if (pDesc)
        {
            pDesc->initArgumentInfo();      // full argument info is needed

            String aSig = pDesc->getSignature();
            aFtHeadLine.SetText( pDesc->getFunctionName() );
            aFtFuncName.SetText( aSig );
            aFtFuncDesc.SetText( pDesc->getDescription() );
        }
    }
    else
    {
        aFtHeadLine.SetText( String() );
        aFtFuncName.SetText( String() );
        aFtFuncDesc.SetText( String() );
    }
    return 0;
}

void FormulaDlg_Impl::UpdateParaWin(const Selection& _rSelection,const String& _sRefStr)
{
    Selection theSel = _rSelection;
    aEdRef.ReplaceSelected( _sRefStr );
    theSel.Max() = theSel.Min() + _sRefStr.Len();
    aEdRef.SetSelection( theSel );

    //-------------------------------------
    // Manual Update of the results' fields:
    //-------------------------------------
    USHORT nPrivActiv = pParaWin->GetActiveLine();
    pParaWin->SetArgument(nPrivActiv,aEdRef.GetText());
    pParaWin->UpdateParas();

    Edit* pEd = GetCurrRefEdit();
    if( pEd != NULL )
        pEd->SetSelection( theSel );

    pParaWin->SetRefMode(FALSE);
}
BOOL FormulaDlg_Impl::UpdateParaWin(Selection& _rSelection)
{
    pParaWin->SetRefMode(TRUE);

    String      aStrEd;
    Edit* pEd = GetCurrRefEdit();
    if(pEd!=NULL && pTheRefEdit==NULL)
    {
        _rSelection=pEd->GetSelection();
        _rSelection.Justify();
        aStrEd=pEd->GetText();
        aEdRef.SetRefString(aStrEd);
        aEdRef.SetSelection( _rSelection );
    }
    else
    {
        _rSelection=aEdRef.GetSelection();
        _rSelection.Justify();
        aStrEd= aEdRef.GetText();
    }
    return pTheRefEdit == NULL;
}
ULONG FormulaDlg_Impl::FindFocusWin(Window *pWin)
{
    ULONG nUniqueId=0;
    if(pWin->HasFocus())
    {
        nUniqueId=pWin->GetUniqueId();
        if(nUniqueId==0)
        {
            Window* pParent=pWin->GetParent();
            while(pParent!=NULL)
            {
                nUniqueId=pParent->GetUniqueId();

                if(nUniqueId!=0) break;

                pParent=pParent->GetParent();
            }
        }
    }
    else
    {
        USHORT nCount=pWin->GetChildCount();

        for(USHORT i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            nUniqueId=FindFocusWin(pChild);
            if(nUniqueId>0) break;
        }
    }
    return nUniqueId;
}

void FormulaDlg_Impl::SetEdSelection()
{
    Edit* pEd = GetCurrRefEdit()/*aScParaWin.GetActiveEdit()*/;
    if( pEd )
    {
        Selection theSel = aEdRef.GetSelection();
        //  Edit may have the focus -> call ModifyHdl in addition
        //  to what's happening in GetFocus
        pEd->GetModifyHdl().Call(pEd);
        pEd->GrabFocus();
        pEd->SetSelection(theSel);
    } // if( pEd )
}
// -----------------------------------------------------------------------------
const FormulaHelper& FormulaDlg_Impl::GetFormulaHelper()  const
{
    return m_aFormulaHelper;
}
//============================================================================
FormulaModalDialog::FormulaModalDialog( Window* pParent
                                            , bool _bSupportFunctionResult
                                            , bool _bSupportResult
                                            , bool _bSupportMatrix
                                            , IFormulaEditorHelper* _pHelper
                                            , IFunctionManager* _pFunctionMgr
                                            , IControlReferenceHandler* _pDlg ) :
        ModalDialog( pParent, ModuleRes(RID_FORMULADLG_FORMULA_MODAL) ),
        m_pImpl( new FormulaDlg_Impl(this,_bSupportFunctionResult
                                            , _bSupportResult
                                            , _bSupportMatrix
                                            ,_pHelper,_pFunctionMgr,_pDlg))
{
    FreeResource();
    SetText(m_pImpl->aTitle1);
}
FormulaModalDialog::~FormulaModalDialog()
{
}
// -----------------------------------------------------------------------------
void FormulaModalDialog::Update(const String& _sExp)
{
    m_pImpl->Update(_sExp);
}

// -----------------------------------------------------------------------------
void FormulaModalDialog::SetMeText(const String& _sText)
{
    m_pImpl->SetMeText(_sText);
}

// -----------------------------------------------------------------------------
FormulaDlgMode FormulaModalDialog::SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,BOOL bMatrix,BOOL _bSelect,BOOL _bUpdate)
{
    return m_pImpl->SetMeText(_sText,PrivStart, PrivEnd,bMatrix,_bSelect,_bUpdate);
}
// -----------------------------------------------------------------------------
void FormulaModalDialog::CheckMatrix()
{
    m_pImpl->aBtnMatrix.Check();
}
// -----------------------------------------------------------------------------
BOOL FormulaModalDialog::CheckMatrix(String& aFormula)
{
    return m_pImpl->CheckMatrix(aFormula);
}
// -----------------------------------------------------------------------------
String FormulaModalDialog::GetMeText() const
{
    return m_pImpl->pMEdit->GetText();
}
// -----------------------------------------------------------------------------
void FormulaModalDialog::Update()
{
    m_pImpl->Update();
}
// -----------------------------------------------------------------------------
const FormulaHelper& FormulaModalDialog::GetFormulaHelper() const
{
    return m_pImpl->GetFormulaHelper();
}
// -----------------------------------------------------------------------------
BOOL FormulaModalDialog::isUserMatrix() const
{
    return m_pImpl->bUserMatrixFlag;
}
void FormulaModalDialog::DoEnter(BOOL _bOk)
{
    m_pImpl->DoEnter(_bOk);
}
::std::pair<RefButton*,RefEdit*> FormulaModalDialog::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}
void FormulaModalDialog::RefInputStartAfter( RefEdit* pEdit, RefButton* pButton )
{
    m_pImpl->RefInputStartAfter( pEdit, pButton );
}
void FormulaModalDialog::RefInputDoneAfter( BOOL bForced )
{
    m_pImpl->RefInputDoneAfter( bForced );
}

ULONG FormulaModalDialog::FindFocusWin(Window *pWin)
{
    return m_pImpl->FindFocusWin( pWin );
}

void FormulaModalDialog::SetFocusWin(Window *pWin,ULONG nUniqueId)
{
    if(pWin->GetUniqueId()==nUniqueId)
    {
        pWin->GrabFocus();
    }
    else
    {
        USHORT nCount=pWin->GetChildCount();

        for(USHORT i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            SetFocusWin(pChild,nUniqueId);
        }
    }
}


long FormulaModalDialog::PreNotify( NotifyEvent& rNEvt )
{
    m_pImpl->PreNotify( rNEvt );

    return ModalDialog::PreNotify(rNEvt);
}

void FormulaModalDialog::HighlightFunctionParas(const String& aFormula)
{
    m_pImpl->m_pHelper->showReference(aFormula);
}

void FormulaModalDialog::disableOk()
{
    m_pImpl->aBtnEnd.Disable();
}
// -----------------------------------------------------------------------------
const IFunctionDescription* FormulaModalDialog::getCurrentFunctionDescription() const
{
    OSL_VERIFY(!m_pImpl->pFuncDesc || m_pImpl->pFuncDesc->getSuppressedArgumentCount() == m_pImpl->nArgs);
    return m_pImpl->pFuncDesc;
}
// -----------------------------------------------------------------------------
void FormulaModalDialog::UpdateParaWin(const Selection& _rSelection,const String& _sRefStr)
{
    m_pImpl->UpdateParaWin(_rSelection,_sRefStr);
}
BOOL FormulaModalDialog::UpdateParaWin(Selection& _rSelection)
{
    return m_pImpl->UpdateParaWin(_rSelection);
}
// -----------------------------------------------------------------------------
RefEdit*    FormulaModalDialog::GetActiveEdit()
{
    return m_pImpl->pParaWin->GetActiveEdit();
}
// -----------------------------------------------------------------------------
void FormulaModalDialog::SetEdSelection()
{
    m_pImpl->SetEdSelection();
}

//  --------------------------------------------------------------------------
//      Initialisation / General functions  for Dialog
//  --------------------------------------------------------------------------
FormulaDlg::FormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                             Window* pParent
                            , bool _bSupportFunctionResult
                            , bool _bSupportResult
                            , bool _bSupportMatrix
                            , IFormulaEditorHelper* _pHelper,IFunctionManager* _pFunctionMgr,IControlReferenceHandler* _pDlg ) :
        SfxModelessDialog( pB, pCW, pParent, ModuleRes(RID_FORMULADLG_FORMULA) ),
        m_pImpl( new FormulaDlg_Impl(this, _bSupportFunctionResult
                                            , _bSupportResult
                                            , _bSupportMatrix
                                            ,_pHelper,_pFunctionMgr,_pDlg))
{
    FreeResource();
    if(GetHelpId()==0)              //Hack which hides the HelpId for a model Dialog in SfxModelessDialog
        SetHelpId(GetUniqueId());   //and will be changed in a UniqueId,
                                    //at this point we reverse it.
    SetText(m_pImpl->aTitle1);
}

FormulaDlg::~FormulaDlg()
{
}
// -----------------------------------------------------------------------------
void FormulaDlg::Update(const String& _sExp)
{
    m_pImpl->Update(_sExp);
}

// -----------------------------------------------------------------------------
void FormulaDlg::SetMeText(const String& _sText)
{
    m_pImpl->SetMeText(_sText);
}

// -----------------------------------------------------------------------------
FormulaDlgMode FormulaDlg::SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,BOOL bMatrix,BOOL _bSelect,BOOL _bUpdate)
{
    return m_pImpl->SetMeText(_sText,PrivStart, PrivEnd,bMatrix,_bSelect,_bUpdate);
}
// -----------------------------------------------------------------------------
void FormulaDlg::CheckMatrix()
{
    m_pImpl->aBtnMatrix.Check();
}
// -----------------------------------------------------------------------------
BOOL FormulaDlg::CheckMatrix(String& aFormula)
{
    return m_pImpl->CheckMatrix(aFormula);
}
// -----------------------------------------------------------------------------
String FormulaDlg::GetMeText() const
{
    return m_pImpl->pMEdit->GetText();
}
// -----------------------------------------------------------------------------
void FormulaDlg::Update()
{
    m_pImpl->Update();
    m_pImpl->aTimer.SetTimeout(200);
    m_pImpl->aTimer.SetTimeoutHdl(LINK( this, FormulaDlg, UpdateFocusHdl));
    m_pImpl->aTimer.Start();
}

// -----------------------------------------------------------------------------
BOOL FormulaDlg::isUserMatrix() const
{
    return m_pImpl->bUserMatrixFlag;
}
void FormulaDlg::DoEnter(BOOL _bOk)
{
    m_pImpl->DoEnter(_bOk);
}
::std::pair<RefButton*,RefEdit*> FormulaDlg::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}
void FormulaDlg::RefInputStartAfter( RefEdit* pEdit, RefButton* pButton )
{
    m_pImpl->RefInputStartAfter( pEdit, pButton );
}
void FormulaDlg::RefInputDoneAfter( BOOL bForced )
{
    m_pImpl->RefInputDoneAfter( bForced );
}

ULONG FormulaDlg::FindFocusWin(Window *pWin)
{
    return m_pImpl->FindFocusWin( pWin );
}

void FormulaDlg::SetFocusWin(Window *pWin,ULONG nUniqueId)
{
    if(pWin->GetUniqueId()==nUniqueId)
    {
        pWin->GrabFocus();
    }
    else
    {
        USHORT nCount=pWin->GetChildCount();

        for(USHORT i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            SetFocusWin(pChild,nUniqueId);
        }
    }
}


long FormulaDlg::PreNotify( NotifyEvent& rNEvt )
{
    m_pImpl->PreNotify( rNEvt );
    return SfxModelessDialog::PreNotify(rNEvt);
}

void FormulaDlg::HighlightFunctionParas(const String& aFormula)
{
    m_pImpl->m_pHelper->showReference(aFormula);
}

void FormulaDlg::disableOk()
{
    m_pImpl->aBtnEnd.Disable();
}
// -----------------------------------------------------------------------------
const IFunctionDescription* FormulaDlg::getCurrentFunctionDescription() const
{
    OSL_VERIFY(!m_pImpl->pFuncDesc || m_pImpl->pFuncDesc->getSuppressedArgumentCount() == m_pImpl->nArgs);
    return m_pImpl->pFuncDesc;
}
// -----------------------------------------------------------------------------
void FormulaDlg::UpdateParaWin(const Selection& _rSelection,const String& _sRefStr)
{
    m_pImpl->UpdateParaWin(_rSelection,_sRefStr);
}
BOOL FormulaDlg::UpdateParaWin(Selection& _rSelection)
{
    return m_pImpl->UpdateParaWin(_rSelection);
}
// -----------------------------------------------------------------------------
RefEdit*    FormulaDlg::GetActiveEdit()
{
    return m_pImpl->pParaWin->GetActiveEdit();
}
// -----------------------------------------------------------------------------
const FormulaHelper& FormulaDlg::GetFormulaHelper() const
{
    return m_pImpl->GetFormulaHelper();
}
// -----------------------------------------------------------------------------
void FormulaDlg::SetEdSelection()
{
    m_pImpl->SetEdSelection();
}
IMPL_LINK( FormulaDlg, UpdateFocusHdl, Timer*, EMPTYARG )
{
    FormEditData* pData = m_pImpl->m_pHelper->getFormEditData();

    if (pData) // won't be destroyed over Close;
    {
        m_pImpl->m_pHelper->setReferenceInput(pData);
        ULONG nUniqueId=pData->GetUniqueId();
        SetFocusWin(this,nUniqueId);
    }
    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FormEditData::SaveValues()
{
    FormEditData* pTemp = new FormEditData(*this);

    Reset();
    pParent = pTemp;
}
// -----------------------------------------------------------------------------
void FormEditData::Reset()
{
    pParent = NULL;
    nMode = 0;
    nFStart = 0;
    nCatSel = 1;        //! oder 0 (zuletzt benutzte)
    nFuncSel = 0;
    nOffset = 0;
    nEdFocus = 0;
    bMatrix =FALSE;
    nUniqueId=0;
    aSelection.Min()=0;
    aSelection.Max()=0;
    aUndoStr.Erase();
}
// -----------------------------------------------------------------------------
void FormEditData::RestoreValues()
{
    FormEditData* pTemp = pParent;
    DBG_ASSERT(pTemp,"RestoreValues ohne Parent");
    if (pTemp)
    {
        *this = *pTemp;
        pTemp->pParent = NULL;      // otherwise it would be cleared too!
        delete pTemp;
    }
}
// -----------------------------------------------------------------------------
const FormEditData& FormEditData::operator=( const FormEditData& r )
{
    pParent         = r.pParent;
    nMode           = r.nMode;
    nFStart         = r.nFStart;
    nCatSel         = r.nCatSel;
    nFuncSel        = r.nFuncSel;
    nOffset         = r.nOffset;
    nEdFocus        = r.nEdFocus;
    aUndoStr        = r.aUndoStr;
    bMatrix         = r.bMatrix ;
    nUniqueId       = r.nUniqueId;
    aSelection      = r.aSelection;
    return *this;
}
// -----------------------------------------------------------------------------
FormEditData::FormEditData()
{
    Reset();
}

FormEditData::~FormEditData()
{
    delete pParent;
}

FormEditData::FormEditData( const FormEditData& r )
{
    *this = r;
}

// -----------------------------------------------------------------------------
} // formula
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
