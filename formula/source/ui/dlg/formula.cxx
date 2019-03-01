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

#include <memory>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/wall.hxx>
#include <vcl/layout.hxx>
#include <vcl/idle.hxx>

#include <svtools/svmedit.hxx>
#include <vcl/treelistbox.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>
#include <sal/log.hxx>

#include <unotools/charclass.hxx>
#include <tools/diagnose_ex.h>

#include "funcpage.hxx"
#include <formula/formula.hxx>
#include <formula/IFunctionDescription.hxx>
#include <formula/FormulaCompiler.hxx>
#include <formula/token.hxx>
#include <formula/tokenarray.hxx>
#include <formula/formdata.hxx>
#include <formula/formulahelper.hxx>
#include "structpg.hxx"
#include "parawin.hxx"
#include <strings.hrc>
#include <core_resource.hxx>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <map>

// For tab page
#define TP_FUNCTION         1
#define TP_STRUCT           2

#define TOKEN_OPEN  0
#define TOKEN_CLOSE 1
#define TOKEN_SEP   2
namespace formula
{

using namespace ::com::sun::star;

class FormulaDlg_Impl
{
public:
    ::std::pair<RefButton*, RefEdit*>
        RefInputStartBefore( RefEdit* pEdit, RefButton* pButton );
    void            RefInputStartAfter();
    void            RefInputDoneAfter( bool bForced );
    bool            CalcValue( const OUString& rStrExp, OUString& rStrResult, bool bForceMatrixFormula = false );
    void            CalcStruct( const OUString& rStrExp, bool bForceRecalcStruct = false );
    void            UpdateValues( bool bForceRecalcStruct = false );
    void            DeleteArgs();
    sal_Int32       GetFunctionPos(sal_Int32 nPos);
    void            ClearAllParas();

    void            MakeTree( StructPage* _pTree, SvTreeListEntry* pParent, const FormulaToken* pFuncToken,
                              const FormulaToken* _pToken, long Count );
    void            fillTree(StructPage* _pTree);
    void            UpdateTokenArray( const OUString& rStrExp);
    OUString        RepairFormula(const OUString& aFormula);
    void            FillDialog(bool bFlag = true);
    bool            EditNextFunc( bool bForward, sal_Int32 nFStart = NOT_FOUND );
    void            EditThisFunc(sal_Int32 nFStart);

    OUString        GetPrevFuncExpression( bool bStartFromEnd );

    void            StoreFormEditData(FormEditData* pEditData);

    void            Update();
    void            Update(const OUString& _sExp);

    void            SaveArg( sal_uInt16 nEd );
    void            UpdateSelection();
    void            DoEnter( bool bOk );
    void            FillListboxes();
    void            FillControls( bool &rbNext, bool &rbPrev);

    FormulaDlgMode  SetMeText( const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate);
    void            SetMeText(const OUString& _sText);
    bool            CheckMatrix(OUString& aFormula /*IN/OUT*/);

    void            SetEdSelection();

    bool            UpdateParaWin(Selection& _rSelection);
    void            UpdateParaWin( const Selection& _rSelection, const OUString& _sRefStr);

    void            SetData( sal_Int32 nFStart, sal_Int32 nNextFStart, sal_Int32 nNextFEnd, sal_Int32& PrivStart, sal_Int32& PrivEnd);
    void            PreNotify( NotifyEvent const & rNEvt );

    RefEdit*        GetCurrRefEdit();

    const FormulaHelper& GetFormulaHelper() const { return m_aFormulaHelper;}
    void InitFormulaOpCodeMapper();

    DECL_LINK( ModifyHdl, ParaWin&, void );
    DECL_LINK( FxHdl, ParaWin&, void );

    DECL_LINK( MatrixHdl, Button*, void );
    DECL_LINK( FormulaHdl, Edit&, void);
    DECL_LINK( FormulaCursorHdl, EditBox&, void );
    DECL_LINK( BtnHdl, Button*, void );
    DECL_LINK( DblClkHdl, FuncPage&, void );
    DECL_LINK( FuncSelHdl, FuncPage&, void );
    DECL_LINK( StructSelHdl, StructPage&, void );
public:
    mutable uno::Reference< sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
    uno::Sequence< sheet::FormulaToken >                    m_aTokenList;
    ::std::unique_ptr<FormulaTokenArray>                    m_pTokenArray;
    ::std::unique_ptr<FormulaTokenArrayPlainIterator>       m_pTokenArrayIterator;
    mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aSpecialOpCodes;
    mutable uno::Sequence< sheet::FormulaToken >            m_aSeparatorsOpCodes;
    mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aFunctionOpCodes;
    mutable const sheet::FormulaOpCodeMapEntry*             m_pFunctionOpCodesEnd;
    ::std::map<const FormulaToken*, sheet::FormulaToken>    m_aTokenMap;
    IFormulaEditorHelper*                                   m_pHelper;
    VclPtr<Dialog>          m_pParent;
    VclPtr<TabControl>      m_pTabCtrl;
    VclPtr<VclVBox>         m_pParaWinBox;
    VclPtr<ParaWin>         m_pParaWin;
    VclPtr<FixedText>       m_pFtHeadLine;
    VclPtr<FixedText>       m_pFtFuncName;
    VclPtr<FixedText>       m_pFtFuncDesc;

    VclPtr<FixedText>       m_pFtEditName;

    VclPtr<FixedText>       m_pFtResult;
    VclPtr<Edit>            m_pWndResult;

    VclPtr<FixedText>       m_pFtFormula;
    VclPtr<EditBox>         m_pMEFormula;

    VclPtr<CheckBox>        m_pBtnMatrix;
    VclPtr<CancelButton>    m_pBtnCancel;

    VclPtr<PushButton>      m_pBtnBackward;
    VclPtr<PushButton>      m_pBtnForward;
    VclPtr<OKButton>        m_pBtnEnd;

    VclPtr<RefEdit>         m_pEdRef;
    VclPtr<RefButton>       m_pRefBtn;

    VclPtr<FixedText>       m_pFtFormResult;
    VclPtr<Edit>            m_pWndFormResult;

    VclPtr<RefEdit>         m_pTheRefEdit;
    VclPtr<RefButton>       m_pTheRefButton;
    VclPtr<FuncPage>        m_pFuncPage;
    VclPtr<StructPage>      m_pStructPage;
    OUString                m_aOldFormula;
    bool                    m_bStructUpdate;
    VclPtr<MultiLineEdit>   m_pMEdit;
    bool                    m_bUserMatrixFlag;
    Idle                    m_aIdle;

    const OUString          m_aTitle1;
    const OUString          m_aTitle2;
    FormulaHelper           m_aFormulaHelper;

    OString                 m_aEditHelpId;

    OString                 m_aOldHelp;
    bool                    m_bIsShutDown;
    bool                    m_bMakingTree;  // in method of constructing tree

    bool                    m_bEditFlag;
    const IFunctionDescription* m_pFuncDesc;
    sal_Int32               m_nArgs;
    ::std::vector< OUString > m_aArguments;
    Selection               m_aFuncSel;

    sal_Int32               m_nFuncExpStart;     ///< current formula position for treeview results

    FormulaDlg_Impl(Dialog* pParent
            , bool _bSupportFunctionResult
            , bool _bSupportResult
            , bool _bSupportMatrix
            , IFormulaEditorHelper* _pHelper
            , const IFunctionManager* _pFunctionMgr
            , IControlReferenceHandler* _pDlg);
    ~FormulaDlg_Impl();

};

FormulaDlg_Impl::FormulaDlg_Impl(Dialog* pParent
                                        , bool _bSupportFunctionResult
                                        , bool _bSupportResult
                                        , bool _bSupportMatrix
                                        , IFormulaEditorHelper* _pHelper
                                        , const IFunctionManager* _pFunctionMgr
                                        , IControlReferenceHandler* _pDlg)
    :
    m_pFunctionOpCodesEnd(nullptr),
    m_pHelper       (_pHelper),
    m_pParent       (pParent),
    m_pTheRefEdit   (nullptr),
    m_pTheRefButton (nullptr),
    m_pMEdit        (nullptr),
    m_bUserMatrixFlag(false),
    m_aTitle1       ( ForResId( STR_TITLE1 ) ),
    m_aTitle2       ( ForResId( STR_TITLE2 ) ),
    m_aFormulaHelper(_pFunctionMgr),
    m_bIsShutDown   (false),
    m_bMakingTree   (false),
    m_pFuncDesc     (nullptr),
    m_nArgs         (0),
    m_nFuncExpStart (0)
{
    pParent->get(m_pParaWinBox, "BOX");
    pParent->get(m_pTabCtrl, "tabs");
    pParent->get(m_pFtHeadLine, "headline");
    pParent->get(m_pFtFuncName, "funcname");
    pParent->get(m_pFtFuncDesc, "funcdesc");
    pParent->get(m_pFtEditName, "editname");
    pParent->get(m_pFtResult, "label2");
    pParent->get(m_pWndResult, "result");
    pParent->get(m_pFtFormula, "formula");

    //Space for two lines of text
    m_pFtHeadLine->SetText("X\nX\n");
    long nHeight = m_pFtHeadLine->GetOptimalSize().Height();
    m_pFtHeadLine->set_height_request(nHeight);
    m_pFtHeadLine->SetText("");

    m_pFtFuncName->SetText("X\nX\n");
    nHeight = m_pFtFuncName->GetOptimalSize().Height();
    m_pFtFuncName->set_height_request(nHeight);
    m_pFtFuncDesc->set_height_request(nHeight);
    m_pFtFuncName->SetText("");

    pParent->get(m_pMEFormula, "ed_formula");
    Size aSize(pParent->LogicToPixel(Size(203, 43), MapMode(MapUnit::MapAppFont)));
    m_pMEFormula->set_height_request(aSize.Height());
    m_pMEFormula->set_width_request(aSize.Width());
    pParent->get(m_pBtnMatrix, "array");
    pParent->get(m_pBtnCancel, "cancel");
    pParent->get(m_pBtnBackward, "back");
    pParent->get(m_pBtnForward, "next");
    pParent->get(m_pBtnEnd, "ok");
    pParent->get(m_pFtFormResult, "label1");
    pParent->get(m_pWndFormResult, "formula_result");
    pParent->get(m_pEdRef, "ED_REF");
    m_pEdRef->SetReferences(_pDlg, m_pFtEditName);
    pParent->get(m_pRefBtn, "RB_REF");
    m_pRefBtn->SetReferences(_pDlg, m_pEdRef);

    m_pParaWin = VclPtr<ParaWin>::Create(m_pParaWinBox, _pDlg);
    m_pParaWin->Show();
    m_pParaWinBox->Hide();
    m_pFtEditName->Hide();
    m_pEdRef->Hide();
    m_pRefBtn->Hide();

    m_pMEdit = m_pMEFormula->GetEdit();

    m_pMEdit->SetAccessibleName(m_pFtFormula->GetText());

    m_aEditHelpId = m_pMEdit->GetHelpId();

    m_bEditFlag =false;
    m_bStructUpdate =true;
    m_pParaWin->SetArgModifiedHdl( LINK( this, FormulaDlg_Impl, ModifyHdl ) );
    m_pParaWin->SetFxHdl( LINK( this, FormulaDlg_Impl, FxHdl ) );

    m_pFuncPage = VclPtr<FuncPage>::Create( m_pTabCtrl, _pFunctionMgr);
    m_pStructPage = VclPtr<StructPage>::Create( m_pTabCtrl);
    m_pFuncPage->Hide();
    m_pStructPage->Hide();
    m_pTabCtrl->SetTabPage( TP_FUNCTION, m_pFuncPage);
    m_pTabCtrl->SetTabPage( TP_STRUCT, m_pStructPage);

    m_aOldHelp = pParent->GetHelpId();                // HelpId from resource always for "Page 1"

    m_pFtResult->Show( _bSupportResult );
    m_pWndResult->Show( _bSupportResult );

    m_pFtFormResult->Show( _bSupportFunctionResult );
    m_pWndFormResult->Show( _bSupportFunctionResult );

    if ( _bSupportMatrix )
        m_pBtnMatrix->SetClickHdl( LINK( this, FormulaDlg_Impl, MatrixHdl ) );
    else
        m_pBtnMatrix->Hide();

    m_pBtnCancel  ->SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_pBtnEnd     ->SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_pBtnForward ->SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_pBtnBackward->SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );

    m_pFuncPage->SetDoubleClickHdl( LINK( this, FormulaDlg_Impl, DblClkHdl ) );
    m_pFuncPage->SetSelectHdl( LINK( this, FormulaDlg_Impl, FuncSelHdl) );
    m_pStructPage->SetSelectionHdl( LINK( this, FormulaDlg_Impl, StructSelHdl ) );
    m_pMEdit->SetModifyHdl( LINK( this, FormulaDlg_Impl, FormulaHdl ) );
    m_pMEFormula->SetSelChangedHdl( LINK( this, FormulaDlg_Impl, FormulaCursorHdl ) );

    vcl::Font aFntLight = m_pFtFormula->GetFont();
    aFntLight.SetTransparent( true );
    vcl::Font aFntBold = aFntLight;
    aFntBold.SetWeight( WEIGHT_BOLD );

    m_pParaWin->SetArgumentFonts( aFntBold, aFntLight);

    //  function description for choosing a function is no longer in a different color

    m_pFtHeadLine->SetFont(aFntBold);
    m_pFtFuncName->SetFont(aFntLight);
    m_pFtFuncDesc->SetFont(aFntLight);
}

FormulaDlg_Impl::~FormulaDlg_Impl()
{
    if (m_aIdle.IsActive())
    {
        m_aIdle.ClearInvokeHandler();
        m_aIdle.Stop();
    }
    m_bIsShutDown = true; // Set it in order to PreNotify not to save GetFocus.

    m_pTabCtrl->RemovePage(TP_FUNCTION);
    m_pTabCtrl->RemovePage(TP_STRUCT);

    m_pStructPage.disposeAndClear();
    m_pFuncPage.disposeAndClear();
    m_pParaWin.disposeAndClear();
    DeleteArgs();
}

void FormulaDlg_Impl::StoreFormEditData(FormEditData* pData)
{
    if (pData) // it won't be destroyed via Close
    {
        pData->SetFStart(m_pMEdit->GetSelection().Min());
        pData->SetSelection(m_pMEdit->GetSelection());

        if (m_pTabCtrl->GetCurPageId() == TP_FUNCTION)
            pData->SetMode( FormulaDlgMode::Formula );
        else
            pData->SetMode( FormulaDlgMode::Edit );
        pData->SetUndoStr(m_pMEdit->GetText());
        pData->SetMatrixFlag(m_pBtnMatrix->IsChecked());
    }
}


void FormulaDlg_Impl::PreNotify( NotifyEvent const & rNEvt )
{
    if (m_bIsShutDown)
        return;
    MouseNotifyEvent nSwitch = rNEvt.GetType();
    if (nSwitch != MouseNotifyEvent::GETFOCUS)
        return;
    vcl::Window* pWin = rNEvt.GetWindow();
    if (!pWin)
        return;
    if (m_aIdle.IsActive()) // will be destroyed via Close
        return;
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;
    pData->SetFocusWindow(pWin);
}

void FormulaDlg_Impl::InitFormulaOpCodeMapper()
{
    if ( m_xOpCodeMapper.is() )
        return;

    m_xOpCodeMapper = m_pHelper->getFormulaOpCodeMapper();
    m_aFunctionOpCodes = m_xOpCodeMapper->getAvailableMappings( sheet::FormulaLanguage::ODFF, sheet::FormulaMapGroup::FUNCTIONS);
    m_pFunctionOpCodesEnd = m_aFunctionOpCodes.getConstArray() + m_aFunctionOpCodes.getLength();

    uno::Sequence< OUString > aArgs(3);
    aArgs[TOKEN_OPEN]   = "(";
    aArgs[TOKEN_CLOSE]  = ")";
    aArgs[TOKEN_SEP]    = ";";
    m_aSeparatorsOpCodes = m_xOpCodeMapper->getMappings( aArgs, sheet::FormulaLanguage::ODFF);

    m_aSpecialOpCodes = m_xOpCodeMapper->getAvailableMappings( sheet::FormulaLanguage::ODFF, sheet::FormulaMapGroup::SPECIAL);
}

void FormulaDlg_Impl::DeleteArgs()
{
    ::std::vector< OUString>().swap(m_aArguments);
    m_nArgs = 0;
}

sal_Int32 FormulaDlg_Impl::GetFunctionPos(sal_Int32 nPos)
{
    if ( !m_aTokenList.hasElements() )
        return SAL_MAX_INT32;

    const sal_Unicode sep = m_pHelper->getFunctionManager()->getSingleToken(IFunctionManager::eSep);

    sal_Int32 nFuncPos = SAL_MAX_INT32;
    OUString  aFormString = m_aFormulaHelper.GetCharClass()->uppercase(m_pMEdit->GetText());

    const uno::Reference< sheet::XFormulaParser > xParser(m_pHelper->getFormulaParser());
    const table::CellAddress aRefPos(m_pHelper->getReferencePosition());

    const sheet::FormulaToken* pIter = m_aTokenList.getConstArray();
    const sheet::FormulaToken* pEnd = pIter + m_aTokenList.getLength();
    try
    {
        bool  bFlag = false;
        sal_Int32 nTokPos = 1;
        sal_Int32 nOldTokPos = 1;
        sal_Int32 nPrevFuncPos = 1;
        short nBracketCount = 0;
        while ( pIter != pEnd )
        {
            const sal_Int32 eOp = pIter->OpCode;
            uno::Sequence<sheet::FormulaToken> aArgs(1);
            aArgs[0] = *pIter;
            const OUString aString = xParser->printFormula( aArgs, aRefPos);
            const sheet::FormulaToken* pNextToken = pIter + 1;

            if ( !m_bUserMatrixFlag && FormulaCompiler::IsMatrixFunction(static_cast<OpCode>(eOp)) )
            {
                m_pBtnMatrix->Check();
            }

            if (eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::PUSH].Token.OpCode ||
                eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode)
            {
                const sal_Int32 n1 = nTokPos < 0 ? -1 : aFormString.indexOf( sep, nTokPos);
                const sal_Int32 n2 = nTokPos < 0 ? -1 : aFormString.indexOf( ')', nTokPos);
                sal_Int32 nXXX = nTokPos;
                if ( n1 < n2 && n1 != -1 )
                {
                    nTokPos = n1;
                }
                else
                {
                    nTokPos = n2;
                }
                if ( pNextToken != pEnd )
                {
                    aArgs[0] = *pNextToken;
                    const OUString a2String = xParser->printFormula( aArgs, aRefPos);
                    const sal_Int32 n3 = nXXX < 0 ? -1 : aFormString.indexOf( a2String, nXXX);
                    if (n3 < nTokPos && n3 != -1)
                        nTokPos = n3;
                }
            }
            else
            {
                nTokPos = nTokPos + aString.getLength();
            }

            if ( eOp == m_aSeparatorsOpCodes[TOKEN_OPEN].OpCode )
            {
                nBracketCount++;
                bFlag = true;
            }
            else if ( eOp == m_aSeparatorsOpCodes[TOKEN_CLOSE].OpCode )
            {
                nBracketCount--;
                bFlag = false;
                nFuncPos = nPrevFuncPos;
            }
            bool bIsFunction = std::any_of( m_aFunctionOpCodes.getConstArray(),
                    m_pFunctionOpCodesEnd,
                    [&eOp](const sheet::FormulaOpCodeMapEntry& aEntry) { return aEntry.Token.OpCode == eOp; });

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
                        nFuncPos = m_pMEdit->GetText().getLength();
                    }
                    else if ( !bFlag )
                    {
                        nFuncPos = nPrevFuncPos;
                    }
                }
                break;
            }

            pIter = pNextToken;
            nOldTokPos = nTokPos;
        } // while ( pIter != pEnd )
    }
    catch ( const uno::Exception& )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("formula.ui", "FormulaDlg_Impl::GetFunctionPos exception! " << exceptionToString(ex));
    }

    return nFuncPos;
}

bool FormulaDlg_Impl::CalcValue( const OUString& rStrExp, OUString& rStrResult, bool bForceMatrixFormula )
{
    bool bResult = true;

    if ( !rStrExp.isEmpty() )
    {
        // Only calculate the value when there isn't any more keyboard input:

        // Make this debuggable by assigning to a variable that can be changed
        // from within the debugger.
        bool bInput = Application::AnyInput( VclInputFlags::KEYBOARD );
        if ( !bInput )
        {
            bResult = m_pHelper->calculateValue( rStrExp, rStrResult, bForceMatrixFormula || m_pBtnMatrix->IsChecked());
        }
        else
            bResult = false;
    }

    return bResult;
}

void FormulaDlg_Impl::UpdateValues( bool bForceRecalcStruct )
{
    // Take a force-array context into account. RPN creation propagated those
    // to tokens that are ref-counted so also available in the token array.
    bool bForceArray = false;
    // Only necessary if it's not a matrix formula anyway and matrix evaluation
    // is supported, i.e. the button is visible.
    if (m_pBtnMatrix->IsVisible() && !m_pBtnMatrix->IsChecked())
    {
        std::unique_ptr<FormulaCompiler> pCompiler(m_pHelper->createCompiler(*m_pTokenArray));
        // In the case of the reportdesign dialog there is no currently active
        // OpCode symbol mapping that could be used to create strings from
        // tokens, it's all dreaded API mapping. However, in that case there's
        // no array/matrix support anyway, but ensure checking.
        if (pCompiler->GetCurrentOpCodeMap().get())
        {
            const sal_Int32 nPos = m_aFuncSel.Min();
            assert( 0 <= nPos && nPos < m_pHelper->getCurrentFormula().getLength());
            OUStringBuffer aBuf;
            const FormulaToken* pToken = nullptr;
            for (pToken = m_pTokenArrayIterator->First(); pToken; pToken = m_pTokenArrayIterator->Next())
            {
                pCompiler->CreateStringFromToken( aBuf, pToken);
                if (nPos < aBuf.getLength())
                    break;
            }
            if (pToken && nPos < aBuf.getLength())
                bForceArray = pToken->IsInForceArray();
        }
    }

    OUString aStrResult;
    if (m_pFuncDesc && CalcValue( m_pFuncDesc->getFormula( m_aArguments), aStrResult, bForceArray))
        m_pWndResult->SetText( aStrResult );

    if (m_bMakingTree)
        return;

    aStrResult.clear();
    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_pWndFormResult->SetText( aStrResult );
    else
    {
        aStrResult.clear();
        m_pWndFormResult->SetText( aStrResult );
    }
    CalcStruct( m_pMEdit->GetText(), bForceRecalcStruct);
}

void FormulaDlg_Impl::CalcStruct( const OUString& rStrExp, bool bForceRecalcStruct )
{
    sal_Int32 nLength = rStrExp.getLength();

    if ( !rStrExp.isEmpty() && (bForceRecalcStruct || m_aOldFormula != rStrExp) && m_bStructUpdate)
    {
        m_pStructPage->ClearStruct();

        OUString aString = rStrExp;
        if (rStrExp[nLength-1] == '(')
        {
            aString = aString.copy( 0, nLength-1);
        }

        aString = aString.replaceAll( "\n", "");
        OUString aStrResult;

        if ( CalcValue( aString, aStrResult ) )
            m_pWndFormResult->SetText( aStrResult );

        UpdateTokenArray(aString);
        fillTree(m_pStructPage);

        m_aOldFormula = rStrExp;
        if (rStrExp[nLength-1] == '(')
            UpdateTokenArray(rStrExp);
    }
}


void FormulaDlg_Impl::MakeTree( StructPage* _pTree, SvTreeListEntry* pParent, const FormulaToken* pFuncToken,
        const FormulaToken* _pToken, long Count )
{
    if ( _pToken != nullptr && Count > 0 )
    {
        long nParas = _pToken->GetParamCount();
        OpCode eOp = _pToken->GetOpCode();

        // #i101512# for output, the original token is needed
        const FormulaToken* pOrigToken = (_pToken->GetType() == svFAP) ? _pToken->GetFAPOrigToken() : _pToken;
        uno::Sequence<sheet::FormulaToken> aArgs(1);
        ::std::map<const FormulaToken*, sheet::FormulaToken>::const_iterator itr = m_aTokenMap.find(pOrigToken);
        if (itr == m_aTokenMap.end())
            return;

        aArgs[0] = itr->second;
        try
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const OUString aResult = m_pHelper->getFormulaParser()->printFormula( aArgs, aRefPos);

            if ( nParas > 0 )
            {
                SvTreeListEntry* pEntry;

                bool bCalcSubformula = false;
                OUString aTest = _pTree->GetEntryText(pParent);

                if (aTest == aResult && (eOp == ocAdd || eOp == ocMul || eOp == ocAmpersand))
                {
                    pEntry = pParent;
                }
                else
                {
                    if (eOp == ocBad)
                    {
                        pEntry = _pTree->InsertEntry( aResult, pParent, STRUCT_ERROR, 0, _pToken);
                    }
                    else if (!((SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP) ||
                                (SC_OPCODE_START_UN_OP <= eOp && eOp < SC_OPCODE_STOP_UN_OP)))
                    {
                        // Not a binary or unary operator.
                        bCalcSubformula = true;
                        pEntry = _pTree->InsertEntry( aResult, pParent, STRUCT_FOLDER, 0, _pToken);
                    }
                    else
                    {
                        /* TODO: question remains, why not sub calculate operators? */
                        pEntry = _pTree->InsertEntry( aResult, pParent, STRUCT_FOLDER, 0, _pToken);
                    }
                }

                MakeTree( _pTree, pEntry, _pToken, m_pTokenArrayIterator->PrevRPN(), nParas);

                if (bCalcSubformula)
                {
                    OUString aFormula;

                    if (!m_bMakingTree)
                    {
                        // gets the last subformula result
                        m_bMakingTree = true;
                        aFormula = GetPrevFuncExpression( true);
                    }
                    else
                    {
                        // gets subsequent subformula results (from the back)
                        aFormula = GetPrevFuncExpression( false);
                    }

                    OUString aStr;
                    if (CalcValue( aFormula, aStr, _pToken->IsInForceArray()))
                        m_pWndResult->SetText( aStr );
                    aStr = m_pWndResult->GetText();
                    m_pStructPage->GetTlbStruct()->SetEntryText( pEntry, aResult + " = " + aStr);
                }

                --Count;
                m_pTokenArrayIterator->NextRPN();   /* TODO: what's this to be? ThisRPN()? */
                MakeTree( _pTree, pParent, _pToken, m_pTokenArrayIterator->PrevRPN(), Count);
            }
            else
            {
                if (eOp == ocBad)
                {
                    _pTree->InsertEntry( aResult, pParent, STRUCT_ERROR, 0, _pToken);
                }
                else if (eOp == ocPush)
                {
                    // Interpret range reference in matrix context to resolve
                    // as array elements. Depending on parameter classification
                    // a scalar value (non-array context) is calculated first.
                    OUString aUnforcedResult;
                    bool bForceMatrix = (!m_pBtnMatrix->IsChecked() &&
                            (_pToken->GetType() == svDoubleRef || _pToken->GetType() == svExternalDoubleRef));
                    if (bForceMatrix && pFuncToken)
                    {
                        formula::ParamClass eParamClass = ParamClass::Reference;
                        if (pFuncToken->IsInForceArray())
                            eParamClass = ParamClass::ForceArray;
                        else
                        {
                            std::shared_ptr<FormulaCompiler> pCompiler = m_pHelper->getCompiler();
                            if (pCompiler)
                                eParamClass = pCompiler->GetForceArrayParameter( pFuncToken, Count - 1);
                        }
                        switch (eParamClass)
                        {
                            case ParamClass::Unknown:
                            case ParamClass::Bounds:
                            case ParamClass::Value:
                                if (CalcValue( "=" + aResult, aUnforcedResult, false) && aUnforcedResult != aResult)
                                    aUnforcedResult += "  ";
                                else
                                    aUnforcedResult.clear();
                            break;
                            case ParamClass::Reference:
                            case ParamClass::ReferenceOrRefArray:
                            case ParamClass::Array:
                            case ParamClass::ForceArray:
                            case ParamClass::ReferenceOrForceArray:
                            case ParamClass::SuppressedReferenceOrForceArray:
                            case ParamClass::ForceArrayReturn:
                                ;   // nothing, only as array/matrix
                            // no default to get compiler warning
                        }
                    }
                    OUString aCellResult;
                    if (CalcValue( "=" + aResult, aCellResult, bForceMatrix) && aCellResult != aResult)
                    {
                        // Cell is a formula, print subformula.
                        // With scalar values prints "A1:A3 = 2 {1;2;3}"
                        _pTree->InsertEntry( aResult + " = " + aUnforcedResult + aCellResult,
                                pParent, STRUCT_END, 0, _pToken);
                    }
                    else
                        _pTree->InsertEntry( aResult, pParent, STRUCT_END, 0, _pToken);
                }
                else
                {
                    _pTree->InsertEntry( aResult, pParent, STRUCT_END, 0, _pToken);
                }
                --Count;
                MakeTree( _pTree, pParent, _pToken, m_pTokenArrayIterator->PrevRPN(), Count);
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("formula.ui");
        }
    }
}

void FormulaDlg_Impl::fillTree(StructPage* _pTree)
{
    InitFormulaOpCodeMapper();
    FormulaToken* pToken = m_pTokenArrayIterator->LastRPN();

    if ( pToken != nullptr)
    {
        MakeTree( _pTree, nullptr, nullptr, pToken, 1);
        m_bMakingTree = false;
    }
}

void FormulaDlg_Impl::UpdateTokenArray( const OUString& rStrExp)
{
    m_aTokenMap.clear();
    m_aTokenList.realloc(0);
    try
    {
        const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
        m_aTokenList = m_pHelper->getFormulaParser()->parseFormula( rStrExp, aRefPos);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("formula.ui");
    }
    InitFormulaOpCodeMapper();
    m_pTokenArray = m_pHelper->convertToTokenArray(m_aTokenList);
    m_pTokenArrayIterator.reset(new FormulaTokenArrayPlainIterator(*m_pTokenArray));
    const sal_Int32 nLen = static_cast<sal_Int32>(m_pTokenArray->GetLen());
    FormulaToken** pTokens = m_pTokenArray->GetArray();
    if ( pTokens && nLen == m_aTokenList.getLength() )
    {
        for (sal_Int32 nPos = 0; nPos < nLen; nPos++)
        {
            m_aTokenMap.emplace( pTokens[nPos], m_aTokenList[nPos] );
        }
    } // if ( pTokens && nLen == m_aTokenList.getLength() )

    std::unique_ptr<FormulaCompiler> pCompiler(m_pHelper->createCompiler(*m_pTokenArray));
    // #i101512# Disable special handling of jump commands.
    pCompiler->EnableJumpCommandReorder(false);
    pCompiler->EnableStopOnError(false);
    pCompiler->SetComputeIIFlag(true);
    pCompiler->SetMatrixFlag(m_bUserMatrixFlag);
    pCompiler->CompileTokenArray();
}

void FormulaDlg_Impl::FillDialog(bool bFlag)
{
    bool bNext = true, bPrev = true;
    if (bFlag)
        FillControls( bNext, bPrev);
    FillListboxes();
    if (bFlag)
    {
        m_pBtnBackward->Enable(bPrev);
        m_pBtnForward->Enable(bNext);
    }

    OUString aStrResult;

    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_pWndFormResult->SetText( aStrResult );
    else
    {
        aStrResult.clear();
        m_pWndFormResult->SetText( aStrResult );
    }
}


void FormulaDlg_Impl::FillListboxes()
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    //  1. Page: select function
    if ( m_pFuncDesc && m_pFuncDesc->getCategory() )
    {
        // We'll never have more than int32 max categories so this is safe ...
        if ( m_pFuncPage->GetCategory() != static_cast<sal_Int32>(m_pFuncDesc->getCategory()->getNumber() + 1) )
            m_pFuncPage->SetCategory(m_pFuncDesc->getCategory()->getNumber() + 1);

        sal_Int32 nPos = m_pFuncPage->GetFuncPos(m_pFuncDesc);

        m_pFuncPage->SetFunction(nPos);
    }
    else if ( pData )
    {
        m_pFuncPage->SetCategory( 1 );
        m_pFuncPage->SetFunction( LISTBOX_ENTRY_NOTFOUND );
    }
    FuncSelHdl(*m_pFuncPage);

    m_pHelper->setDispatcherLock( true );   // Activate Modal-Mode

    //  HelpId for 1. page is the one from the resource
    m_pParent->SetHelpId( m_aOldHelp );
}

void FormulaDlg_Impl::FillControls( bool &rbNext, bool &rbPrev)
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData )
        return;

    //  2. Page or Edit: show selected function

    sal_Int32  nFStart     = pData->GetFStart();
    OUString   aFormula    = m_pHelper->getCurrentFormula() + " )";
    sal_Int32  nNextFStart = nFStart;
    sal_Int32  nNextFEnd   = 0;

    DeleteArgs();
    const IFunctionDescription* pOldFuncDesc = m_pFuncDesc;

    if ( m_aFormulaHelper.GetNextFunc( aFormula, false,
                                     nNextFStart, &nNextFEnd, &m_pFuncDesc, &m_aArguments ) )
    {
        const bool bTestFlag = (pOldFuncDesc != m_pFuncDesc);
        if (bTestFlag)
        {
            m_pFtHeadLine->Hide();
            m_pFtFuncName->Hide();
            m_pFtFuncDesc->Hide();
            m_pParaWin->SetFunctionDesc(m_pFuncDesc);
            m_pFtEditName->SetText( m_pFuncDesc->getFunctionName() );
            m_pFtEditName->Show();
            m_pParaWinBox->Show();
            const OString aHelpId = m_pFuncDesc->getHelpId();
            if ( !aHelpId.isEmpty() )
                m_pMEdit->SetHelpId(aHelpId);
        }

        sal_Int32 nOldStart, nOldEnd;
        m_pHelper->getSelection( nOldStart, nOldEnd );
        if ( nOldStart != nNextFStart || nOldEnd != nNextFEnd )
        {
            m_pHelper->setSelection( nNextFStart, nNextFEnd );
        }
        m_aFuncSel.Min() = nNextFStart;
        m_aFuncSel.Max() = nNextFEnd;

        if (!m_bEditFlag)
            m_pMEdit->SetText(m_pHelper->getCurrentFormula());
        sal_Int32 PrivStart, PrivEnd;
        m_pHelper->getSelection( PrivStart, PrivEnd);
        if (!m_bEditFlag)
            m_pMEdit->SetSelection( Selection( PrivStart, PrivEnd));

        m_nArgs = m_pFuncDesc->getSuppressedArgumentCount();
        sal_uInt16 nOffset = pData->GetOffset();

        //  Concatenate the Edit's for Focus-Control

        if (bTestFlag)
            m_pParaWin->SetArgumentOffset(nOffset);
        sal_uInt16 nActiv = 0;
        sal_Int32   nArgPos  = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        sal_Int32   nEditPos = m_pMEdit->GetSelection().Min();
        bool    bFlag    = false;

        for (sal_Int32 i = 0; i < m_nArgs; i++)
        {
            sal_Int32 nLength = m_aArguments[i].getLength()+1;
            m_pParaWin->SetArgument( i, m_aArguments[i]);
            if (nArgPos <= nEditPos && nEditPos < nArgPos+nLength)
            {
                nActiv = i;
                bFlag = true;
            }
            nArgPos = nArgPos + nLength;
        }
        m_pParaWin->UpdateParas();

        if (bFlag)
        {
            m_pParaWin->SetActiveLine(nActiv);
        }

        UpdateValues();
    }
    else
    {
        m_pFtEditName->SetText("");
        m_pMEdit->SetHelpId( m_aEditHelpId );
    }
        //  test if before/after are anymore functions

    sal_Int32 nTempStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
    rbNext = m_aFormulaHelper.GetNextFunc( aFormula, false, nTempStart );
    nTempStart = m_pMEdit->GetSelection().Min();
    pData->SetFStart(nTempStart);
    rbPrev = m_aFormulaHelper.GetNextFunc( aFormula, true, nTempStart );
}


void FormulaDlg_Impl::ClearAllParas()
{
    DeleteArgs();
    m_pFuncDesc = nullptr;
    m_pParaWin->ClearAll();
    m_pWndResult->SetText(OUString());
    m_pFtFuncName->SetText(OUString());
    FuncSelHdl(*m_pFuncPage);

    if (m_pFuncPage->IsVisible())
    {
        m_pFtEditName->Hide();
        m_pParaWinBox->Hide();

        m_pBtnForward->Enable(); //@new
        m_pFtHeadLine->Show();
        m_pFtFuncName->Show();
        m_pFtFuncDesc->Show();
    }
}

OUString FormulaDlg_Impl::RepairFormula(const OUString& aFormula)
{
    OUString aResult('=');
    try
    {
        UpdateTokenArray(aFormula);

        if ( m_aTokenList.getLength() )
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const OUString sFormula( m_pHelper->getFormulaParser()->printFormula( m_aTokenList, aRefPos));
            if ( sFormula.isEmpty() || sFormula[0] != '=' )
                aResult += sFormula;
            else
                aResult = sFormula;

        }
    }
    catch ( const uno::Exception& )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("formula.ui", "FormulaDlg_Impl::RepairFormula exception! " << exceptionToString(ex));
    }
    return aResult;
}

void FormulaDlg_Impl::DoEnter(bool bOk)
{
    //  Accept input to the document or cancel
    if ( bOk)
    {
        //  remove dummy arguments
        OUString  aInputFormula = m_pHelper->getCurrentFormula();
        OUString  aString = RepairFormula(m_pMEdit->GetText());
        m_pHelper->setSelection( 0, aInputFormula.getLength());
        m_pHelper->setCurrentFormula(aString);
    }

    m_pHelper->switchBack();

    m_pHelper->dispatch( bOk, m_pBtnMatrix->IsChecked());
    //  Clear data
    m_pHelper->deleteFormData();

    //  Close dialog
    m_pHelper->doClose(bOk);
}


IMPL_LINK( FormulaDlg_Impl, BtnHdl, Button*, pBtn, void )
{
    if ( pBtn == m_pBtnCancel )
    {
        DoEnter(false);                 // closes the Dialog
    }
    else if ( pBtn == m_pBtnEnd )
    {
        DoEnter(true);                  // closes the Dialog
    }
    else if ( pBtn == m_pBtnForward )
    {
        const IFunctionDescription* pDesc;
        sal_Int32 nSelFunc = m_pFuncPage->GetFunction();
        if (nSelFunc != LISTBOX_ENTRY_NOTFOUND)
            pDesc = m_pFuncPage->GetFuncDesc( nSelFunc );
        else
        {
            // Do not overwrite the selected formula expression, just edit the
            // unlisted function.
            m_pFuncDesc = pDesc = nullptr;
        }

        if (pDesc == m_pFuncDesc || !m_pFuncPage->IsVisible())
            EditNextFunc( true );
        else
        {
            DblClkHdl(*m_pFuncPage);      //new
            m_pBtnForward->Enable(false); //new
        }
    }
    else if ( pBtn == m_pBtnBackward )
    {
        m_bEditFlag = false;
        m_pBtnForward->Enable();
        EditNextFunc( false );
        m_pMEFormula->Invalidate();
        m_pMEFormula->Update();
    }
}


//                          Functions for 1. Page


// Handler for Listboxes

IMPL_LINK_NOARG( FormulaDlg_Impl, DblClkHdl, FuncPage&, void)
{
    sal_Int32 nFunc = m_pFuncPage->GetFunction();

    //  ex-UpdateLRUList
    const IFunctionDescription* pDesc = m_pFuncPage->GetFuncDesc(nFunc);
    m_pHelper->insertEntryToLRUList(pDesc);

    OUString aFuncName = m_pFuncPage->GetSelFunctionName() + "()";
    m_pHelper->setCurrentFormula(aFuncName);
    m_pMEdit->ReplaceSelected(aFuncName);

    Selection aSel = m_pMEdit->GetSelection();
    aSel.Max() = aSel.Max()-1;
    m_pMEdit->SetSelection(aSel);

    FormulaHdl(*m_pMEdit);

    aSel.Min() = aSel.Max();
    m_pMEdit->SetSelection(aSel);

    if (m_nArgs == 0)
    {
        BtnHdl(m_pBtnBackward);
    }

    m_pParaWin->SetEdFocus();
    m_pBtnForward->Enable(false); //@New
}


//                          Functions for right Page

void FormulaDlg_Impl::SetData( sal_Int32 nFStart, sal_Int32 nNextFStart, sal_Int32 nNextFEnd, sal_Int32& PrivStart, sal_Int32& PrivEnd)
{
    sal_Int32 nFEnd;

    // Notice and set new selection
    m_pHelper->getSelection( nFStart, nFEnd );
    m_pHelper->setSelection( nNextFStart, nNextFEnd );
    if (!m_bEditFlag)
        m_pMEdit->SetText(m_pHelper->getCurrentFormula());


    m_pHelper->getSelection( PrivStart, PrivEnd);
    if (!m_bEditFlag)
    {
        m_pMEdit->SetSelection( Selection( PrivStart, PrivEnd));
        m_pMEFormula->UpdateOldSel();
    }

    FormEditData* pData = m_pHelper->getFormEditData();
    pData->SetFStart( nNextFStart );
    pData->SetOffset( 0 );

    FillDialog();
}

void FormulaDlg_Impl::EditThisFunc(sal_Int32 nFStart)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    OUString aFormula = m_pHelper->getCurrentFormula();

    if (nFStart == NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    sal_Int32 nNextFStart  = nFStart;
    sal_Int32 nNextFEnd    = 0;

    bool bFound;

    bFound = m_aFormulaHelper.GetNextFunc( aFormula, false, nNextFStart, &nNextFEnd);
    if ( bFound )
    {
        sal_Int32 PrivStart, PrivEnd;
        SetData( nFStart, nNextFStart, nNextFEnd, PrivStart, PrivEnd);
        m_pHelper->showReference( aFormula.copy( PrivStart, PrivEnd-PrivStart));
    }
    else
    {
        ClearAllParas();
    }
}

bool FormulaDlg_Impl::EditNextFunc( bool bForward, sal_Int32 nFStart )
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return false;

    OUString aFormula = m_pHelper->getCurrentFormula();

    if (nFStart == NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    sal_Int32 nNextFStart  = 0;
    sal_Int32 nNextFEnd    = 0;

    bool bFound;
    if ( bForward )
    {
        nNextFStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, false, nNextFStart, &nNextFEnd);
    }
    else
    {
        nNextFStart = nFStart;
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, true, nNextFStart, &nNextFEnd);
    }

    if ( bFound )
    {
        sal_Int32 PrivStart, PrivEnd;
        SetData( nFStart, nNextFStart, nNextFEnd, PrivStart, PrivEnd);
    }

    return bFound;
}

OUString FormulaDlg_Impl::GetPrevFuncExpression( bool bStartFromEnd )
{
    OUString aExpression;

    OUString aFormula( m_pHelper->getCurrentFormula());
    if (aFormula.isEmpty())
        return aExpression;

    if (bStartFromEnd || m_nFuncExpStart >= aFormula.getLength())
        m_nFuncExpStart = aFormula.getLength() - 1;

    sal_Int32 nFStart = m_nFuncExpStart;
    sal_Int32 nFEnd   = 0;
    if (m_aFormulaHelper.GetNextFunc( aFormula, true, nFStart, &nFEnd))
    {
        aExpression = aFormula.copy( nFStart, nFEnd - nFStart); // nFEnd is exclusive
        m_nFuncExpStart = nFStart;
    }

    return aExpression;
}

void FormulaDlg_Impl::SaveArg( sal_uInt16 nEd )
{
    if (nEd < m_nArgs)
    {
        for (sal_uInt16 i = 0; i <= nEd; i++)
        {
            if ( m_aArguments[i].isEmpty() )
                m_aArguments[i] = " ";
        }
        if (!m_pParaWin->GetArgument(nEd).isEmpty())
            m_aArguments[nEd] = m_pParaWin->GetArgument(nEd);

        sal_uInt16 nClearPos = nEd+1;
        for (sal_Int32 i = nEd+1; i < m_nArgs; i++)
        {
            if ( !m_pParaWin->GetArgument(i).isEmpty() )
            {
                nClearPos = i+1;
            }
        }

        for (sal_Int32 i = nClearPos; i < m_nArgs; i++)
        {
            m_aArguments[i].clear();
        }
    }
}

IMPL_LINK( FormulaDlg_Impl, FxHdl, ParaWin&, rPtr, void )
{
    if (&rPtr == m_pParaWin)
    {
        m_pBtnForward->Enable(); //@ In order to be able to input another function.
        m_pTabCtrl->SetCurPageId(TP_FUNCTION);

        OUString aUndoStr = m_pHelper->getCurrentFormula();       // it will be added before a ";"
        FormEditData* pData = m_pHelper->getFormEditData();
        if (!pData)
            return;

        sal_uInt16 nArgNo = m_pParaWin->GetActiveLine();
        sal_uInt16 nEdFocus = nArgNo;

        SaveArg(nArgNo);
        UpdateSelection();

        sal_Int32 nFormulaStrPos = pData->GetFStart();

        OUString aFormula = m_pHelper->getCurrentFormula();
        sal_Int32 n1 = m_aFormulaHelper.GetArgStart( aFormula, nFormulaStrPos, nEdFocus + pData->GetOffset() );

        pData->SaveValues();
        pData->SetMode( FormulaDlgMode::Formula );
        pData->SetFStart( n1 );
        pData->SetUndoStr( aUndoStr );
        ClearAllParas();

        FillDialog(false);
        m_pFuncPage->SetFocus(); //There Parawin is not visible anymore
    }
}

IMPL_LINK( FormulaDlg_Impl, ModifyHdl, ParaWin&, rPtr, void )
{
    if (&rPtr == m_pParaWin)
    {
        SaveArg(m_pParaWin->GetActiveLine());
        UpdateValues();

        UpdateSelection();
        CalcStruct(m_pMEdit->GetText());
    }
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FormulaHdl, Edit&, void)
{

    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    m_bEditFlag = true;
    OUString    aInputFormula = m_pHelper->getCurrentFormula();
    OUString    aString = m_pMEdit->GetText();

    Selection   aSel  = m_pMEdit->GetSelection();

    if (aString.isEmpty())      // in case everything was cleared
    {
        aString += "=";
        m_pMEdit->SetText(aString);
        aSel .Min() = 1;
        aSel .Max() = 1;
        m_pMEdit->SetSelection(aSel);
    }
    else if (aString[0]!='=')   // in case it's replaced
    {
        aString = "=" + aString;
        m_pMEdit->SetText(aString);
        aSel .Min() += 1;
        aSel .Max() += 1;
        m_pMEdit->SetSelection(aSel);
    }


    m_pHelper->setSelection( 0, aInputFormula.getLength());
    m_pHelper->setCurrentFormula(aString);
    m_pHelper->setSelection( aSel.Min(), aSel.Max());

    sal_Int32 nPos = aSel.Min()-1;

    OUString aStrResult;

    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_pWndFormResult->SetText( aStrResult );
    else
    {
        aStrResult.clear();
        m_pWndFormResult->SetText( aStrResult );
    }
    CalcStruct(aString);

    nPos = GetFunctionPos(nPos);

    if (nPos < aSel.Min()-1)
    {
        sal_Int32 nPos1 = aString.indexOf( '(', nPos);
        EditNextFunc( false, nPos1);
    }
    else
    {
        ClearAllParas();
    }

    m_pHelper->setSelection( aSel.Min(), aSel.Max());
    m_bEditFlag = false;
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FormulaCursorHdl, EditBox&, void)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    m_bEditFlag = true;

    OUString    aString = m_pMEdit->GetText();

    Selection   aSel = m_pMEdit->GetSelection();
    m_pHelper->setSelection( aSel.Min(), aSel.Max());

    if (aSel.Min() == 0)
    {
        aSel.Min() = 1;
        m_pMEdit->SetSelection(aSel);
    }

    if (aSel.Min() != aString.getLength())
    {
        sal_Int32 nPos = aSel.Min();

        sal_Int32 nFStart = GetFunctionPos(nPos - 1);

        if (nFStart < nPos)
        {
            sal_Int32 nPos1 = m_aFormulaHelper.GetFunctionEnd( aString, nFStart);

            if (nPos1 > nPos)
            {
                EditThisFunc(nFStart);
            }
            else
            {
                sal_Int32 n = nPos;
                short nCount = 1;
                while(n > 0)
                {
                   if (aString[n]==')')
                       nCount++;
                   else if (aString[n]=='(')
                       nCount--;
                   if (nCount == 0)
                       break;
                   n--;
                }
                if (nCount == 0)
                {
                    nFStart = m_aFormulaHelper.GetFunctionStart( aString, n, true);
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
    m_pHelper->setSelection( aSel.Min(), aSel.Max());

    m_bEditFlag = false;
}

void FormulaDlg_Impl::UpdateSelection()
{
    m_pHelper->setSelection( m_aFuncSel.Min(), m_aFuncSel.Max());
    m_pHelper->setCurrentFormula( m_pFuncDesc->getFormula( m_aArguments ) );
    m_pMEdit->SetText(m_pHelper->getCurrentFormula());
    sal_Int32 PrivStart, PrivEnd;
    m_pHelper->getSelection( PrivStart, PrivEnd);
    m_aFuncSel.Min() = PrivStart;
    m_aFuncSel.Max() = PrivEnd;

    m_nArgs = m_pFuncDesc->getSuppressedArgumentCount();

    OUString aFormula = m_pMEdit->GetText();
    sal_Int32 nArgPos = m_aFormulaHelper.GetArgStart( aFormula, PrivStart, 0);

    sal_uInt16 nPos = m_pParaWin->GetActiveLine();
    if (nPos >= m_aArguments.size())
    {
        SAL_WARN("formula.ui","FormulaDlg_Impl::UpdateSelection - shot in foot: nPos " <<
                nPos << " >= m_aArguments.size() " << m_aArguments.size() <<
                " for aFormula '" << aFormula << "'");
        nPos = m_aArguments.size();
        if (nPos)
            --nPos;
    }

    for (sal_uInt16 i = 0; i < nPos; i++)
    {
        nArgPos += (m_aArguments[i].getLength() + 1);
    }
    sal_Int32 nLength = (nPos < m_aArguments.size()) ? m_aArguments[nPos].getLength() : 0;

    Selection aSel( nArgPos, nArgPos+nLength);
    m_pHelper->setSelection( static_cast<sal_uInt16>(nArgPos),static_cast<sal_uInt16>(nArgPos+nLength));
    m_pMEdit->SetSelection(aSel);
    m_pMEFormula->UpdateOldSel();
}

::std::pair<RefButton*, RefEdit*> FormulaDlg_Impl::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    //because its initially hidden, give it its optimal
    //size so clicking the refbutton has an initial
    //size to work when retro-fitting this to .ui
    m_pEdRef->SetSizePixel(m_pEdRef->GetOptimalSize());
    m_pEdRef->Show();
    m_pTheRefEdit = pEdit;
    m_pTheRefButton = pButton;

    if ( m_pTheRefEdit )
    {
        m_pEdRef->SetRefString( m_pTheRefEdit->GetText() );
        m_pEdRef->SetSelection( m_pTheRefEdit->GetSelection() );
        m_pEdRef->SetHelpId( m_pTheRefEdit->GetHelpId() );
    }

    m_pRefBtn->Show( pButton != nullptr );

    ::std::pair<RefButton*, RefEdit*> aPair;
    aPair.first = pButton ? m_pRefBtn.get() : nullptr;
    aPair.second = m_pEdRef;
    return aPair;
}

void FormulaDlg_Impl::RefInputStartAfter()
{
    m_pRefBtn->SetEndImage();

    if ( m_pTheRefEdit )
    {
        OUString aStr = m_aTitle2 + " " + m_pFtEditName->GetText() + "( ";

        if ( m_pParaWin->GetActiveLine() > 0 )
            aStr += "...; ";
        aStr += m_pParaWin->GetActiveArgName();
        if ( m_pParaWin->GetActiveLine() + 1 < m_nArgs )
            aStr += "; ...";
        aStr += " )";

        m_pParent->SetText( MnemonicGenerator::EraseAllMnemonicChars( aStr ) );
    }
}

void FormulaDlg_Impl::RefInputDoneAfter( bool bForced )
{
    m_pRefBtn->SetStartImage();
    if ( bForced || !m_pRefBtn->IsVisible() )
    {
        m_pEdRef->Hide();
        m_pRefBtn->Hide();
        if ( m_pTheRefEdit )
        {
            m_pTheRefEdit->SetRefString( m_pEdRef->GetText() );
            m_pTheRefEdit->GrabFocus();

            if ( m_pTheRefButton )
                m_pTheRefButton->SetStartImage();

            sal_uInt16 nPrivActiv = m_pParaWin->GetActiveLine();
            m_pParaWin->SetArgument( nPrivActiv, m_pEdRef->GetText() );
            ModifyHdl( *m_pParaWin );
            m_pTheRefEdit = nullptr;
        }
        m_pParent->SetText( m_aTitle1 );
    }
}

RefEdit* FormulaDlg_Impl::GetCurrRefEdit()
{
    return m_pEdRef->IsVisible() ? m_pEdRef.get() : m_pParaWin->GetActiveEdit();
}

void FormulaDlg_Impl::Update()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    const OUString sExpression = m_pMEdit->GetText();
    m_aOldFormula.clear();
    UpdateTokenArray(sExpression);
    FormulaCursorHdl(*m_pMEFormula);
    CalcStruct(sExpression);
    if (pData->GetMode() == FormulaDlgMode::Formula)
        m_pTabCtrl->SetCurPageId(TP_FUNCTION);
    else
        m_pTabCtrl->SetCurPageId(TP_STRUCT);
    m_pBtnMatrix->Check(pData->GetMatrixFlag());
}

void FormulaDlg_Impl::Update(const OUString& _sExp)
{
    CalcStruct(_sExp);
    FillDialog();
    FuncSelHdl(*m_pFuncPage);
}

void FormulaDlg_Impl::SetMeText(const OUString& _sText)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    m_pMEdit->SetText(_sText);
    m_pMEdit->SetSelection( pData->GetSelection());
    m_pMEFormula->UpdateOldSel();
}

FormulaDlgMode FormulaDlg_Impl::SetMeText( const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate)
{
    FormulaDlgMode eMode = FormulaDlgMode::Formula;
    if (!m_bEditFlag)
        m_pMEdit->SetText(_sText);

    if ( _bSelect || !m_bEditFlag )
        m_pMEdit->SetSelection( Selection( PrivStart, PrivEnd));
    if ( _bUpdate )
    {
        m_pMEFormula->UpdateOldSel();
        m_pMEdit->Invalidate();
        m_pHelper->showReference(m_pMEdit->GetSelected());
        eMode = FormulaDlgMode::Edit;

        m_pBtnMatrix->Check( bMatrix );
    } // if ( _bUpdate )
    return eMode;
}

bool FormulaDlg_Impl::CheckMatrix(OUString& aFormula)
{
    m_pMEdit->GrabFocus();
    sal_Int32 nLen = aFormula.getLength();
    bool bMatrix =  nLen > 3                    // Matrix-Formula
            && aFormula[0] == '{'
            && aFormula[1] == '='
            && aFormula[nLen-1] == '}';
    if ( bMatrix )
    {
        aFormula = aFormula.copy( 1, aFormula.getLength()-2 );
        m_pBtnMatrix->Check( bMatrix );
        m_pBtnMatrix->Disable();
    } // if ( bMatrix )

    m_pTabCtrl->SetCurPageId(TP_STRUCT);
    return bMatrix;
}

IMPL_LINK_NOARG( FormulaDlg_Impl, StructSelHdl, StructPage&, void)
{
    m_bStructUpdate = false;
    if (m_pStructPage->IsVisible())
        m_pBtnForward->Enable(false); //@New
    m_bStructUpdate = true;
}

IMPL_LINK_NOARG( FormulaDlg_Impl, MatrixHdl, Button*, void)
{
    m_bUserMatrixFlag = true;
    UpdateValues(true);
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FuncSelHdl, FuncPage&, void)
{
    if (   (m_pFuncPage->GetFunctionEntryCount() > 0)
        && (m_pFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        const IFunctionDescription* pDesc = m_pFuncPage->GetFuncDesc( m_pFuncPage->GetFunction() );

        if (pDesc != m_pFuncDesc)
            m_pBtnForward->Enable(); //new

        if (pDesc)
        {
            pDesc->initArgumentInfo();      // full argument info is needed

            OUString aSig = pDesc->getSignature();
            m_pFtHeadLine->SetText( pDesc->getFunctionName() );
            m_pFtFuncName->SetText( aSig );
            m_pFtFuncDesc->SetText( pDesc->getDescription() );
        }
    }
    else
    {
        m_pFtHeadLine->SetText( OUString() );
        m_pFtFuncName->SetText( OUString() );
        m_pFtFuncDesc->SetText( OUString() );
    }
}

void FormulaDlg_Impl::UpdateParaWin( const Selection& _rSelection, const OUString& _sRefStr)
{
    Selection theSel = _rSelection;
    m_pEdRef->ReplaceSelected( _sRefStr );
    theSel.Max() = theSel.Min() + _sRefStr.getLength();
    m_pEdRef->SetSelection( theSel );


    // Manual Update of the results' fields:

    sal_uInt16 nPrivActiv = m_pParaWin->GetActiveLine();
    m_pParaWin->SetArgument( nPrivActiv, m_pEdRef->GetText());
    m_pParaWin->UpdateParas();

    Edit* pEd = GetCurrRefEdit();
    if ( pEd != nullptr )
        pEd->SetSelection( theSel );
}

bool FormulaDlg_Impl::UpdateParaWin(Selection& _rSelection)
{
    OUString      aStrEd;
    Edit* pEd = GetCurrRefEdit();
    if (pEd != nullptr && m_pTheRefEdit == nullptr)
    {
        _rSelection = pEd->GetSelection();
        _rSelection.Justify();
        aStrEd = pEd->GetText();
        m_pEdRef->SetRefString(aStrEd);
        m_pEdRef->SetSelection( _rSelection );
    }
    else
    {
        _rSelection = m_pEdRef->GetSelection();
        _rSelection.Justify();
        aStrEd = m_pEdRef->GetText();
    }
    return m_pTheRefEdit == nullptr;
}

void FormulaDlg_Impl::SetEdSelection()
{
    Edit* pEd = GetCurrRefEdit()/*aScParaWin.GetActiveEdit()*/;
    if ( pEd )
    {
        Selection theSel = m_pEdRef->GetSelection();
        //  Edit may have the focus -> call ModifyHdl in addition
        //  to what's happening in GetFocus
        pEd->GetModifyHdl().Call(*pEd);
        pEd->GrabFocus();
        pEd->SetSelection(theSel);
    } // if ( pEd )
}

FormulaModalDialog::FormulaModalDialog(   vcl::Window* pParent
                                        , IFunctionManager const * _pFunctionMgr
                                        , IControlReferenceHandler* _pDlg )
    : ModalDialog(pParent, "FormulaDialog", "formula/ui/formuladialog.ui")
    , m_pImpl(new FormulaDlg_Impl(this, false/*_bSupportFunctionResult*/,
                                  false/*_bSupportResult*/, false/*_bSupportMatrix*/,
                                  this, _pFunctionMgr, _pDlg))
{
    SetText(m_pImpl->m_aTitle1);
}

FormulaModalDialog::~FormulaModalDialog() { disposeOnce(); }

void FormulaModalDialog::dispose()
{
    m_pImpl.reset();
    ModalDialog::dispose();
}

void FormulaModalDialog::Update(const OUString& _sExp)
{
    m_pImpl->Update(_sExp);
}

void FormulaModalDialog::SetMeText(const OUString& _sText)
{
    m_pImpl->SetMeText(_sText);
}

void FormulaModalDialog::CheckMatrix(OUString& aFormula)
{
    m_pImpl->CheckMatrix(aFormula);
}

void FormulaModalDialog::Update()
{
    m_pImpl->Update();
}

::std::pair<RefButton*, RefEdit*> FormulaModalDialog::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}

void FormulaModalDialog::RefInputStartAfter()
{
    m_pImpl->RefInputStartAfter();
}

void FormulaModalDialog::RefInputDoneAfter()
{
    m_pImpl->RefInputDoneAfter( true/*bForced*/ );
}

bool FormulaModalDialog::PreNotify( NotifyEvent& rNEvt )
{
    if (m_pImpl)
        m_pImpl->PreNotify( rNEvt );

    return ModalDialog::PreNotify(rNEvt);
}

void FormulaModalDialog::StoreFormEditData(FormEditData* pData)
{
    m_pImpl->StoreFormEditData(pData);
}


//      Initialisation / General functions  for Dialog

FormulaDlg::FormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                             vcl::Window* pParent
                            , IFunctionManager const * _pFunctionMgr, IControlReferenceHandler* _pDlg ) :
        SfxModelessDialog( pB, pCW, pParent, "FormulaDialog", "formula/ui/formuladialog.ui" ),
        m_pImpl( new FormulaDlg_Impl(this, true/*_bSupportFunctionResult*/
                                            , true/*_bSupportResult*/
                                            , true/*_bSupportMatrix*/
                                            , this, _pFunctionMgr, _pDlg))
{
    SetText(m_pImpl->m_aTitle1);
}

FormulaDlg::~FormulaDlg() {disposeOnce();}

void FormulaDlg::dispose()
{
    m_pImpl.reset();
    SfxModelessDialog::dispose();
}

void FormulaDlg::Update(const OUString& _sExp)
{
    m_pImpl->Update(_sExp);
}


void FormulaDlg::SetMeText(const OUString& _sText)
{
    m_pImpl->SetMeText(_sText);
}

FormulaDlgMode FormulaDlg::SetMeText( const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate)
{
    return m_pImpl->SetMeText( _sText, PrivStart, PrivEnd, bMatrix, _bSelect, _bUpdate);
}

bool FormulaDlg::CheckMatrix(OUString& aFormula)
{
    return m_pImpl->CheckMatrix(aFormula);
}

OUString FormulaDlg::GetMeText() const
{
    return m_pImpl->m_pMEdit->GetText();
}

void FormulaDlg::Update()
{
    m_pImpl->Update();
    m_pImpl->m_aIdle.SetInvokeHandler( LINK( this, FormulaDlg, UpdateFocusHdl));
    m_pImpl->m_aIdle.Start();
}

void FormulaDlg::DoEnter()
{
    m_pImpl->DoEnter(false);
}

::std::pair<RefButton*, RefEdit*> FormulaDlg::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}

void FormulaDlg::RefInputStartAfter()
{
    m_pImpl->RefInputStartAfter();
}

void FormulaDlg::RefInputDoneAfter( bool bForced )
{
    m_pImpl->RefInputDoneAfter( bForced );
}

bool FormulaDlg::PreNotify( NotifyEvent& rNEvt )
{
    if (m_pImpl)
        m_pImpl->PreNotify( rNEvt );
    return SfxModelessDialog::PreNotify(rNEvt);
}

void FormulaDlg::disableOk()
{
    m_pImpl->m_pBtnEnd->Disable();
}

void FormulaDlg::StoreFormEditData(FormEditData* pData)
{
    m_pImpl->StoreFormEditData(pData);
}

const IFunctionDescription* FormulaDlg::getCurrentFunctionDescription() const
{
    SAL_WARN_IF( (m_pImpl->m_pFuncDesc && m_pImpl->m_pFuncDesc->getSuppressedArgumentCount() != m_pImpl->m_nArgs),
            "formula.ui", "FormulaDlg::getCurrentFunctionDescription: getSuppressedArgumentCount " <<
            m_pImpl->m_pFuncDesc->getSuppressedArgumentCount() << " != m_nArgs " << m_pImpl->m_nArgs << " for " <<
            m_pImpl->m_pFuncDesc->getFunctionName());
    return m_pImpl->m_pFuncDesc;
}

void FormulaDlg::UpdateParaWin( const Selection& _rSelection, const OUString& _sRefStr)
{
    m_pImpl->UpdateParaWin( _rSelection, _sRefStr);
}

bool FormulaDlg::UpdateParaWin(Selection& _rSelection)
{
    return m_pImpl->UpdateParaWin(_rSelection);
}

RefEdit*    FormulaDlg::GetActiveEdit()
{
    return m_pImpl->m_pParaWin->GetActiveEdit();
}

const FormulaHelper& FormulaDlg::GetFormulaHelper() const
{
    return m_pImpl->GetFormulaHelper();
}

void FormulaDlg::SetEdSelection()
{
    m_pImpl->SetEdSelection();
}

IMPL_LINK_NOARG( FormulaDlg, UpdateFocusHdl, Timer *, void)
{
    FormEditData* pData = m_pImpl->m_pHelper->getFormEditData();
    if (!pData)
        return;
    // won't be destroyed via Close
    VclPtr<vcl::Window> xWin(pData->GetFocusWindow());
    if (xWin && !xWin->IsDisposed())
        xWin->GrabFocus();
}

void FormEditData::SaveValues()
{
    Reset();
}

void FormEditData::Reset()
{
    nMode = FormulaDlgMode::Formula;
    nFStart = 0;
    nOffset = 0;
    bMatrix = false;
    xFocusWin.clear();
    aSelection.Min() = 0;
    aSelection.Max() = 0;
    aUndoStr.clear();
}

FormEditData& FormEditData::operator=( const FormEditData& r )
{
    nMode           = r.nMode;
    nFStart         = r.nFStart;
    nOffset         = r.nOffset;
    aUndoStr        = r.aUndoStr;
    bMatrix         = r.bMatrix ;
    xFocusWin       = r.xFocusWin;
    aSelection      = r.aSelection;
    return *this;
}

FormEditData::FormEditData()
{
    Reset();
}

FormEditData::~FormEditData()
{
}

FormEditData::FormEditData( const FormEditData& r )
{
    *this = r;
}


} // formula


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
