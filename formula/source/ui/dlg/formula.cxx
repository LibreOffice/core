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
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <sal/log.hxx>

#include <unotools/charclass.hxx>
#include <comphelper/diagnose_ex.hxx>

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
#include <map>

// For tab page
#define TOKEN_OPEN  0
#define TOKEN_CLOSE 1
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

    void            MakeTree(StructPage* _pTree, weld::TreeIter* pParent, const FormulaToken* pFuncToken,
                             const FormulaToken* _pToken, tools::Long Count);
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

    RefEdit*    GetCurrRefEdit();

    const FormulaHelper& GetFormulaHelper() const { return m_aFormulaHelper;}
    void InitFormulaOpCodeMapper();

    void UpdateOldSel();
    void FormulaCursor();

    DECL_LINK( ModifyHdl, ParaWin&, void );
    DECL_LINK( FxHdl, ParaWin&, void );

    DECL_LINK( MatrixHdl, weld::Toggleable&, void );
    DECL_LINK( FormulaHdl, weld::TextView&, void);
    DECL_LINK( FormulaCursorHdl, weld::TextView&, void );
    DECL_LINK( BtnHdl, weld::Button&, void );
    DECL_LINK( DblClkHdl, FuncPage&, void );
    DECL_LINK( FuncSelHdl, FuncPage&, void );
    DECL_LINK( StructSelHdl, StructPage&, void );
public:
    mutable uno::Reference< sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
    uno::Sequence< sheet::FormulaToken >                    m_aTokenList;
    ::std::unique_ptr<FormulaTokenArray>                    m_pTokenArray;
    ::std::optional<FormulaTokenArrayPlainIterator>         m_oTokenArrayIterator;
    mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aSpecialOpCodes;
    mutable uno::Sequence< sheet::FormulaToken >            m_aSeparatorsOpCodes;
    mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aFunctionOpCodes;
    mutable const sheet::FormulaOpCodeMapEntry*             m_pFunctionOpCodesEnd;
    ::std::map<const FormulaToken*, sheet::FormulaToken>    m_aTokenMap;
    IFormulaEditorHelper*                                   m_pHelper;
    weld::Dialog&           m_rDialog;

    OUString                m_aOldFormula;
    bool                    m_bStructUpdate;
    bool                    m_bUserMatrixFlag;

    const OUString          m_aTitle1;
    const OUString          m_aTitle2;
    FormulaHelper           m_aFormulaHelper;

    OUString                m_aEditHelpId;

    OUString                 m_aOldHelp;
    bool                    m_bMakingTree;  // in method of constructing tree

    bool                    m_bEditFlag;
    const IFunctionDescription* m_pFuncDesc;
    sal_Int32               m_nArgs;
    ::std::vector< OUString > m_aArguments;
    Selection               m_aFuncSel;

    sal_Int32               m_nFuncExpStart;     ///< current formula position for treeview results

    int m_nSelectionStart;
    int m_nSelectionEnd;

    RefEdit* m_pTheRefEdit;
    RefButton* m_pTheRefButton;

    std::unique_ptr<weld::Notebook> m_xTabCtrl;
    std::unique_ptr<weld::Container> m_xParaWinBox;
    std::unique_ptr<ParaWin> m_xParaWin;
    std::unique_ptr<weld::Label> m_xFtHeadLine;
    std::unique_ptr<weld::Label> m_xFtFuncName;
    std::unique_ptr<weld::Label> m_xFtFuncDesc;

    std::unique_ptr<weld::Label> m_xFtEditName;

    std::unique_ptr<weld::Label> m_xFtResult;
    std::unique_ptr<weld::Entry> m_xWndResult;

    std::unique_ptr<weld::Label> m_xFtFormula;
    std::unique_ptr<weld::TextView> m_xMEdit;

    std::unique_ptr<weld::CheckButton> m_xBtnMatrix;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::unique_ptr<weld::Button> m_xBtnBackward;
    std::unique_ptr<weld::Button> m_xBtnForward;
    std::unique_ptr<weld::Button> m_xBtnEnd;

    std::unique_ptr<weld::Label> m_xFtFormResult;
    std::unique_ptr<weld::Entry> m_xWndFormResult;

    std::unique_ptr<RefEdit> m_xEdRef;
    std::unique_ptr<RefButton> m_xRefBtn;

    std::unique_ptr<FuncPage> m_xFuncPage;
    std::unique_ptr<StructPage> m_xStructPage;

    FormulaDlg_Impl(weld::Dialog& rDialog,
                    weld::Builder& rBuilder,
                    bool _bSupportFunctionResult,
                    bool _bSupportResult,
                    bool _bSupportMatrix,
                    IFormulaEditorHelper* _pHelper,
                    const IFunctionManager* _pFunctionMgr,
                    IControlReferenceHandler* _pDlg);
    ~FormulaDlg_Impl();
};

FormulaDlg_Impl::FormulaDlg_Impl(weld::Dialog& rDialog,
                                 weld::Builder& rBuilder,
                                 bool _bSupportFunctionResult,
                                 bool _bSupportResult,
                                 bool _bSupportMatrix,
                                 IFormulaEditorHelper* _pHelper,
                                 const IFunctionManager* _pFunctionMgr,
                                 IControlReferenceHandler* _pDlg)
    : m_pFunctionOpCodesEnd(nullptr)
    , m_pHelper(_pHelper)
    , m_rDialog(rDialog)
    , m_bUserMatrixFlag(false)
    , m_aTitle1( ForResId( STR_TITLE1 ) )
    , m_aTitle2( ForResId( STR_TITLE2 ) )
    , m_aFormulaHelper(_pFunctionMgr)
    , m_bMakingTree(false)
    , m_pFuncDesc(nullptr)
    , m_nArgs(0)
    , m_nFuncExpStart(0)
    , m_nSelectionStart(-1)
    , m_nSelectionEnd(-1)
    , m_pTheRefEdit(nullptr)
    , m_pTheRefButton(nullptr)
    , m_xTabCtrl(rBuilder.weld_notebook(u"tabcontrol"_ustr))
    , m_xParaWinBox(rBuilder.weld_container(u"BOX"_ustr))
    , m_xFtHeadLine(rBuilder.weld_label(u"headline"_ustr))
    , m_xFtFuncName(rBuilder.weld_label(u"funcname"_ustr))
    , m_xFtFuncDesc(rBuilder.weld_label(u"funcdesc"_ustr))
    , m_xFtEditName(rBuilder.weld_label(u"editname"_ustr))
    , m_xFtResult(rBuilder.weld_label(u"label2"_ustr))
    , m_xWndResult(rBuilder.weld_entry(u"result"_ustr))
    , m_xFtFormula(rBuilder.weld_label(u"formula"_ustr))
    , m_xMEdit(rBuilder.weld_text_view(u"ed_formula"_ustr))
    , m_xBtnMatrix(rBuilder.weld_check_button(u"array"_ustr))
    , m_xBtnCancel(rBuilder.weld_button(u"cancel"_ustr))
    , m_xBtnBackward(rBuilder.weld_button(u"back"_ustr))
    , m_xBtnForward(rBuilder.weld_button(u"next"_ustr))
    , m_xBtnEnd(rBuilder.weld_button(u"ok"_ustr))
    , m_xFtFormResult(rBuilder.weld_label(u"label1"_ustr))
    , m_xWndFormResult(rBuilder.weld_entry(u"formula_result"_ustr))
    , m_xEdRef(new RefEdit(rBuilder.weld_entry(u"ED_REF"_ustr)))
    , m_xRefBtn(new RefButton(rBuilder.weld_button(u"RB_REF"_ustr)))
{
    auto nWidth = m_xMEdit->get_approximate_digit_width() * 62;

    //Space for two lines of text
    m_xFtHeadLine->set_label(u"X\nX\n"_ustr);
    auto nHeight = m_xFtHeadLine->get_preferred_size().Height();
    m_xFtHeadLine->set_size_request(nWidth, nHeight);
    m_xFtHeadLine->set_label(u""_ustr);

    m_xFtFuncName->set_label(u"X\nX\n"_ustr);
    nHeight = m_xFtFuncName->get_preferred_size().Height();
    m_xFtFuncName->set_size_request(nWidth, nHeight);
    m_xFtFuncDesc->set_size_request(nWidth, nHeight);
    m_xFtFuncName->set_label(u""_ustr);

    m_xMEdit->set_size_request(nWidth,
                               m_xMEdit->get_height_rows(5));

    m_xEdRef->SetReferences(_pDlg, m_xFtEditName.get());
    m_xRefBtn->SetReferences(_pDlg, m_xEdRef.get());

    m_xParaWin.reset(new ParaWin(m_xParaWinBox.get(), _pDlg));
    m_xParaWin->Show();
    m_xParaWinBox->hide();
    m_xFtEditName->hide();
    m_xEdRef->GetWidget()->hide();
    m_xRefBtn->GetWidget()->hide();

    m_xMEdit->set_accessible_name(m_xFtFormula->get_label());

    m_aEditHelpId = m_xMEdit->get_help_id();

    m_bEditFlag =false;
    m_bStructUpdate =true;
    m_xParaWin->SetArgModifiedHdl( LINK( this, FormulaDlg_Impl, ModifyHdl ) );
    m_xParaWin->SetFxHdl( LINK( this, FormulaDlg_Impl, FxHdl ) );

    m_xFuncPage.reset(new FuncPage(m_xTabCtrl->get_page(u"functiontab"_ustr), _pFunctionMgr));
    m_xStructPage.reset(new StructPage(m_xTabCtrl->get_page(u"structtab"_ustr)));
    m_xTabCtrl->set_current_page(u"functiontab"_ustr);

    m_aOldHelp = m_rDialog.get_help_id();                // HelpId from resource always for "Page 1"

    m_xFtResult->set_visible( _bSupportResult );
    m_xWndResult->set_visible( _bSupportResult );

    m_xFtFormResult->set_visible( _bSupportFunctionResult );
    m_xWndFormResult->set_visible( _bSupportFunctionResult );

    if ( _bSupportMatrix )
        m_xBtnMatrix->connect_toggled( LINK( this, FormulaDlg_Impl, MatrixHdl ) );
    else
        m_xBtnMatrix->hide();

    m_xBtnCancel->connect_clicked( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_xBtnEnd->connect_clicked( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_xBtnForward->connect_clicked( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    m_xBtnBackward->connect_clicked( LINK( this, FormulaDlg_Impl, BtnHdl ) );

    m_xFuncPage->SetDoubleClickHdl( LINK( this, FormulaDlg_Impl, DblClkHdl ) );
    m_xFuncPage->SetSelectHdl( LINK( this, FormulaDlg_Impl, FuncSelHdl) );
    m_xStructPage->SetSelectionHdl( LINK( this, FormulaDlg_Impl, StructSelHdl ) );
    m_xMEdit->connect_changed( LINK( this, FormulaDlg_Impl, FormulaHdl ) );
    m_xMEdit->connect_cursor_position( LINK( this, FormulaDlg_Impl, FormulaCursorHdl ) );

    vcl::Font aFntLight = m_xFtFormula->get_font();
    vcl::Font aFntBold = aFntLight;
    aFntBold.SetWeight( WEIGHT_BOLD );

    m_xParaWin->SetArgumentFonts( aFntBold, aFntLight);
}

FormulaDlg_Impl::~FormulaDlg_Impl()
{
    m_xTabCtrl->remove_page(u"functiontab"_ustr);
    m_xTabCtrl->remove_page(u"structtab"_ustr);

    DeleteArgs();
}

void FormulaDlg_Impl::StoreFormEditData(FormEditData* pData)
{
    if (!pData) // it won't be destroyed via Close
        return;

    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    pData->SetFStart(nStartPos);
    pData->SetSelection(Selection(nStartPos, nEndPos));

    if (m_xTabCtrl->get_current_page_ident() == "functiontab")
        pData->SetMode( FormulaDlgMode::Formula );
    else
        pData->SetMode( FormulaDlgMode::Edit );
    pData->SetUndoStr(m_xMEdit->get_text());
    pData->SetMatrixFlag(m_xBtnMatrix->get_active());
}

void FormulaDlg_Impl::InitFormulaOpCodeMapper()
{
    if ( m_xOpCodeMapper.is() )
        return;

    m_xOpCodeMapper = m_pHelper->getFormulaOpCodeMapper();
    m_aFunctionOpCodes = m_xOpCodeMapper->getAvailableMappings( sheet::FormulaLanguage::ODFF, sheet::FormulaMapGroup::FUNCTIONS);
    m_pFunctionOpCodesEnd = m_aFunctionOpCodes.getConstArray() + m_aFunctionOpCodes.getLength();

    // 0:TOKEN_OPEN, 1:TOKEN_CLOSE, 2:TOKEN_SEP
    uno::Sequence< OUString > aArgs { u"("_ustr, u")"_ustr, u";"_ustr };
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
    OUString  aFormString = m_aFormulaHelper.GetCharClass().uppercase(m_xMEdit->get_text());

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
        const sal_Int32 nOpPush = m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::PUSH].Token.OpCode;
        const sal_Int32 nOpSpaces = m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode;
        const sal_Int32 nOpWhitespace = m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::WHITESPACE].Token.OpCode;
        while ( pIter != pEnd )
        {
            const sal_Int32 eOp = pIter->OpCode;
            uno::Sequence<sheet::FormulaToken> aArgs { *pIter };
            const OUString aString = xParser->printFormula( aArgs, aRefPos);
            const sheet::FormulaToken* pNextToken = pIter + 1;

            if ( !m_bUserMatrixFlag && FormulaCompiler::IsMatrixFunction(static_cast<OpCode>(eOp)) )
            {
                m_xBtnMatrix->set_active(true);
            }

            if (eOp == nOpPush || eOp == nOpSpaces || eOp == nOpWhitespace)
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
                    aArgs.getArray()[0] = *pNextToken;
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

            if ( bIsFunction && nOpSpaces != eOp && nOpWhitespace != eOp )
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
                        nFuncPos = m_xMEdit->get_text().getLength();
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
        TOOLS_WARN_EXCEPTION("formula.ui", "FormulaDlg_Impl::GetFunctionPos");
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
            bResult = m_pHelper->calculateValue( rStrExp, rStrResult, bForceMatrixFormula || m_xBtnMatrix->get_active());
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
    if (m_xBtnMatrix->get_visible() && !m_xBtnMatrix->get_active())
    {
        std::unique_ptr<FormulaCompiler> pCompiler(m_pHelper->createCompiler(*m_pTokenArray));
        // In the case of the reportdesign dialog there is no currently active
        // OpCode symbol mapping that could be used to create strings from
        // tokens, it's all dreaded API mapping. However, in that case there's
        // no array/matrix support anyway, but ensure checking.
        if (pCompiler->GetCurrentOpCodeMap())
        {
            const sal_Int32 nPos = m_aFuncSel.Min();
            assert( 0 <= nPos && nPos < m_pHelper->getCurrentFormula().getLength());
            OUStringBuffer aBuf;
            const FormulaToken* pToken = nullptr;
            for (pToken = m_oTokenArrayIterator->First(); pToken; pToken = m_oTokenArrayIterator->Next())
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
        m_xWndResult->set_text( aStrResult );

    if (m_bMakingTree)
        return;

    aStrResult.clear();
    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_xWndFormResult->set_text( aStrResult );
    else
    {
        aStrResult.clear();
        m_xWndFormResult->set_text( aStrResult );
    }
    CalcStruct( m_xMEdit->get_text(), bForceRecalcStruct);
}

void FormulaDlg_Impl::CalcStruct( const OUString& rStrExp, bool bForceRecalcStruct )
{
    sal_Int32 nLength = rStrExp.getLength();

    if ( !(!rStrExp.isEmpty() && (bForceRecalcStruct || m_aOldFormula != rStrExp) && m_bStructUpdate))
        return;

    m_xStructPage->ClearStruct();

    OUString aString = rStrExp;
    if (rStrExp[nLength-1] == '(')
    {
        aString = aString.copy( 0, nLength-1);
    }

    aString = aString.replaceAll( "\n", "");
    OUString aStrResult;

    if ( CalcValue( aString, aStrResult ) )
        m_xWndFormResult->set_text(aStrResult);

    UpdateTokenArray(aString);
    fillTree(m_xStructPage.get());

    m_aOldFormula = rStrExp;
    if (rStrExp[nLength-1] == '(')
        UpdateTokenArray(rStrExp);
}

void FormulaDlg_Impl::MakeTree(StructPage* _pTree, weld::TreeIter* pParent, const FormulaToken* pFuncToken,
                               const FormulaToken* _pToken, tools::Long Count)
{
    if ( _pToken == nullptr || Count <= 0 )
        return;

    tools::Long nParas = _pToken->GetParamCount();
    OpCode eOp = _pToken->GetOpCode();

    // #i101512# for output, the original token is needed
    const FormulaToken* pOrigToken = (_pToken->GetType() == svFAP) ? _pToken->GetFAPOrigToken() : _pToken;
    ::std::map<const FormulaToken*, sheet::FormulaToken>::const_iterator itr = m_aTokenMap.find(pOrigToken);
    if (itr == m_aTokenMap.end())
        return;

    uno::Sequence<sheet::FormulaToken> aArgs { itr->second };
    try
    {
        const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
        const OUString aResult = m_pHelper->getFormulaParser()->printFormula( aArgs, aRefPos);

        if ( nParas > 0 || (nParas == 0 && _pToken->IsFunction()) )
        {
            std::unique_ptr<weld::TreeIter> xEntry;
            weld::TreeIter* pEntry;

            bool bCalcSubformula = false;
            OUString aTest = _pTree->GetEntryText(pParent);

            if (aTest == aResult && (eOp == ocAdd || eOp == ocMul || eOp == ocAmpersand))
            {
                pEntry = pParent;
            }
            else
            {
                xEntry = m_xStructPage->GetTlbStruct().make_iterator();

                if (eOp == ocBad)
                {
                    _pTree->InsertEntry(aResult, pParent, STRUCT_ERROR, 0, _pToken, *xEntry);
                }
                else if (!((SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP) ||
                            (SC_OPCODE_START_UN_OP <= eOp && eOp < SC_OPCODE_STOP_UN_OP)))
                {
                    // Not a binary or unary operator.
                    bCalcSubformula = true;
                    _pTree->InsertEntry(aResult, pParent, STRUCT_FOLDER, 0, _pToken, *xEntry);
                }
                else
                {
                    /* TODO: question remains, why not sub calculate operators? */
                    _pTree->InsertEntry(aResult, pParent, STRUCT_FOLDER, 0, _pToken, *xEntry);
                }

                pEntry = xEntry.get();
            }

            MakeTree(_pTree, pEntry, _pToken, m_oTokenArrayIterator->PrevRPN(), nParas);

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
                    m_xWndResult->set_text( aStr );
                aStr = m_xWndResult->get_text();
                m_xStructPage->GetTlbStruct().set_text(*pEntry, aResult + " = " + aStr);
            }

            --Count;
            m_oTokenArrayIterator->NextRPN();   /* TODO: what's this to be? ThisRPN()? */
            MakeTree( _pTree, pParent, _pToken, m_oTokenArrayIterator->PrevRPN(), Count);
        }
        else
        {
            std::unique_ptr<weld::TreeIter> xEntry(m_xStructPage->GetTlbStruct().make_iterator());
            if (eOp == ocBad)
            {
                _pTree->InsertEntry( aResult, pParent, STRUCT_ERROR, 0, _pToken, *xEntry);
            }
            else if (eOp == ocPush)
            {
                // Interpret range reference in matrix context to resolve
                // as array elements. Depending on parameter classification
                // a scalar value (non-array context) is calculated first.
                OUString aUnforcedResult;
                bool bForceMatrix = (!m_xBtnMatrix->get_active() &&
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
                            pParent, STRUCT_END, 0, _pToken, *xEntry);
                }
                else
                    _pTree->InsertEntry(aResult, pParent, STRUCT_END, 0, _pToken, *xEntry);
            }
            else
            {
                _pTree->InsertEntry(aResult, pParent, STRUCT_END, 0, _pToken, *xEntry);
            }
            --Count;
            MakeTree( _pTree, pParent, _pToken, m_oTokenArrayIterator->PrevRPN(), Count);
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("formula.ui");
    }
}

void FormulaDlg_Impl::fillTree(StructPage* _pTree)
{
    InitFormulaOpCodeMapper();
    FormulaToken* pToken = m_oTokenArrayIterator->LastRPN();

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
    m_oTokenArrayIterator.emplace(*m_pTokenArray);
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
        m_xBtnBackward->set_sensitive(bPrev);
        m_xBtnForward->set_sensitive(bNext);
    }

    OUString aStrResult;

    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_xWndFormResult->set_text( aStrResult );
    else
    {
        aStrResult.clear();
        m_xWndFormResult->set_text( aStrResult );
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
        // Category listbox holds additional entries for Last Used and All, so
        // the offset should be two but hard coded numbers are ugly...
        const sal_Int32 nCategoryOffset = m_xFuncPage->GetCategoryEntryCount() - m_aFormulaHelper.GetCategoryCount();
        if ( m_xFuncPage->GetCategory() != static_cast<sal_Int32>(m_pFuncDesc->getCategory()->getNumber() + nCategoryOffset) )
            m_xFuncPage->SetCategory(m_pFuncDesc->getCategory()->getNumber() + nCategoryOffset);

        sal_Int32 nPos = m_xFuncPage->GetFuncPos(m_pFuncDesc);

        m_xFuncPage->SetFunction(nPos);
    }
    else if ( pData )
    {
        // tdf#104487 - remember last used function category
        m_xFuncPage->SetCategory(FuncPage::GetRememeberdFunctionCategory());
        m_xFuncPage->SetFunction( -1 );
    }
    FuncSelHdl(*m_xFuncPage);

    m_pHelper->setDispatcherLock( true );   // Activate Modal-Mode

    //  HelpId for 1. page is the one from the resource
    m_rDialog.set_help_id( m_aOldHelp );
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
            m_xFtHeadLine->hide();
            m_xFtFuncName->hide();
            m_xFtFuncDesc->hide();
            m_xParaWin->SetFunctionDesc(m_pFuncDesc);
            m_xFtEditName->set_label( m_pFuncDesc->getFunctionName() );
            m_xFtEditName->show();
            m_xParaWinBox->show();
            const OUString aHelpId = m_pFuncDesc->getHelpId();
            if ( !aHelpId.isEmpty() )
                m_xMEdit->set_help_id(aHelpId);
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
            m_xMEdit->set_text(m_pHelper->getCurrentFormula());
        sal_Int32 PrivStart, PrivEnd;
        m_pHelper->getSelection( PrivStart, PrivEnd);
        if (!m_bEditFlag)
            m_xMEdit->select_region(PrivStart, PrivEnd);

        m_nArgs = m_pFuncDesc->getSuppressedArgumentCount();
        sal_uInt16 nOffset = pData->GetOffset();

        //  Concatenate the Edit's for Focus-Control

        if (bTestFlag)
            m_xParaWin->SetArgumentOffset(nOffset);
        sal_uInt16 nActiv = 0;
        sal_Int32   nArgPos  = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );

        int nStartPos, nEndPos;
        m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
        if (nStartPos > nEndPos)
            std::swap(nStartPos, nEndPos);

        sal_Int32 nEditPos = nStartPos;
        bool    bFlag    = false;

        for (sal_Int32 i = 0; i < m_nArgs; i++)
        {
            sal_Int32 nLength = m_aArguments[i].getLength()+1;
            m_xParaWin->SetArgument( i, m_aArguments[i]);
            if (nArgPos <= nEditPos && nEditPos < nArgPos+nLength)
            {
                nActiv = i;
                bFlag = true;
            }
            nArgPos = nArgPos + nLength;
        }
        m_xParaWin->UpdateParas();

        if (bFlag)
        {
            m_xParaWin->SetActiveLine(nActiv);
        }

        UpdateValues();
    }
    else
    {
        m_xFtEditName->set_label(u""_ustr);
        m_xMEdit->set_help_id(m_aEditHelpId);
    }
        //  test if before/after are anymore functions

    sal_Int32 nTempStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
    rbNext = m_aFormulaHelper.GetNextFunc( aFormula, false, nTempStart );

    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    nTempStart = nStartPos;
    pData->SetFStart(nTempStart);
    rbPrev = m_aFormulaHelper.GetNextFunc( aFormula, true, nTempStart );
}


void FormulaDlg_Impl::ClearAllParas()
{
    DeleteArgs();
    m_pFuncDesc = nullptr;
    m_xParaWin->ClearAll();
    m_xWndResult->set_text(OUString());
    m_xFtFuncName->set_label(OUString());
    FuncSelHdl(*m_xFuncPage);

    if (m_xFuncPage->IsVisible())
    {
        m_xFtEditName->hide();
        m_xParaWinBox->hide();

        m_xBtnForward->set_sensitive(true); //@new
        m_xFtHeadLine->show();
        m_xFtFuncName->show();
        m_xFtFuncDesc->show();
    }
}

OUString FormulaDlg_Impl::RepairFormula(const OUString& aFormula)
{
    OUString aResult('=');
    try
    {
        UpdateTokenArray(aFormula);

        if ( m_aTokenList.hasElements() )
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
        TOOLS_WARN_EXCEPTION("formula.ui", "FormulaDlg_Impl::RepairFormula");
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
        OUString  aString = RepairFormula(m_xMEdit->get_text());
        m_pHelper->setSelection( 0, aInputFormula.getLength());
        m_pHelper->setCurrentFormula(aString);
    }

    m_pHelper->switchBack();

    m_pHelper->dispatch( bOk, m_xBtnMatrix->get_active());
    //  Clear data
    m_pHelper->deleteFormData();

    //  Close dialog
    m_pHelper->doClose(bOk);
}


IMPL_LINK(FormulaDlg_Impl, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnCancel.get())
    {
        DoEnter(false);                 // closes the Dialog
    }
    else if (&rBtn == m_xBtnEnd.get())
    {
        DoEnter(true);                  // closes the Dialog
    }
    else if (&rBtn == m_xBtnForward.get())
    {
        const IFunctionDescription* pDesc;
        sal_Int32 nSelFunc = m_xFuncPage->GetFunction();
        if (nSelFunc != -1)
            pDesc = m_xFuncPage->GetFuncDesc( nSelFunc );
        else
        {
            // Do not overwrite the selected formula expression, just edit the
            // unlisted function.
            m_pFuncDesc = pDesc = nullptr;
        }

        if (pDesc == m_pFuncDesc || !m_xFuncPage->IsVisible())
            EditNextFunc( true );
        else
        {
            DblClkHdl(*m_xFuncPage);      //new
            m_xBtnForward->set_sensitive(false); //new
        }
    }
    else if (&rBtn == m_xBtnBackward.get())
    {
        m_bEditFlag = false;
        m_xBtnForward->set_sensitive(true);
        EditNextFunc( false );
    }
}

//                          Functions for 1. Page

// Handler for Listboxes

IMPL_LINK_NOARG( FormulaDlg_Impl, DblClkHdl, FuncPage&, void)
{
    sal_Int32 nFunc = m_xFuncPage->GetFunction();

    //  ex-UpdateLRUList
    const IFunctionDescription* pDesc = m_xFuncPage->GetFuncDesc(nFunc);
    m_pHelper->insertEntryToLRUList(pDesc);

    OUString aFuncName = m_xFuncPage->GetSelFunctionName() + "()";
    m_pHelper->setCurrentFormula(aFuncName);
    m_xMEdit->replace_selection(aFuncName);

    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    nEndPos = nEndPos - 1;
    m_xMEdit->select_region(nStartPos, nEndPos);

    FormulaHdl(*m_xMEdit);

    nStartPos = nEndPos;
    m_xMEdit->select_region(nStartPos, nEndPos);

    if (m_nArgs == 0)
    {
        BtnHdl(*m_xBtnBackward);
    }

    m_xParaWin->SetEdFocus();
    m_xBtnForward->set_sensitive(false); //@New
}

//                          Functions for right Page

void FormulaDlg_Impl::SetData( sal_Int32 nFStart, sal_Int32 nNextFStart, sal_Int32 nNextFEnd, sal_Int32& PrivStart, sal_Int32& PrivEnd)
{
    sal_Int32 nFEnd;

    // Notice and set new selection
    m_pHelper->getSelection( nFStart, nFEnd );
    m_pHelper->setSelection( nNextFStart, nNextFEnd );
    if (!m_bEditFlag)
        m_xMEdit->set_text(m_pHelper->getCurrentFormula());


    m_pHelper->getSelection( PrivStart, PrivEnd);
    if (!m_bEditFlag)
    {
        m_xMEdit->select_region(PrivStart, PrivEnd);
        UpdateOldSel();
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
    if (nEd >= m_nArgs)
        return;

    for (sal_uInt16 i = 0; i <= nEd; i++)
    {
        if ( m_aArguments[i].isEmpty() )
            m_aArguments[i] = " ";
    }
    if (!m_xParaWin->GetArgument(nEd).isEmpty())
        m_aArguments[nEd] = m_xParaWin->GetArgument(nEd);

    sal_uInt16 nClearPos = nEd+1;
    for (sal_Int32 i = nEd+1; i < m_nArgs; i++)
    {
        if ( !m_xParaWin->GetArgument(i).isEmpty() )
        {
            nClearPos = i+1;
        }
    }

    for (sal_Int32 i = nClearPos; i < m_nArgs; i++)
    {
        m_aArguments[i].clear();
    }
}

IMPL_LINK( FormulaDlg_Impl, FxHdl, ParaWin&, rPtr, void )
{
    if (&rPtr != m_xParaWin.get())
        return;

    m_xBtnForward->set_sensitive(true); //@ In order to be able to input another function.
    m_xTabCtrl->set_current_page(u"functiontab"_ustr);

    OUString aUndoStr = m_pHelper->getCurrentFormula();       // it will be added before a ";"
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    sal_uInt16 nArgNo = m_xParaWin->GetActiveLine();
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
    m_xFuncPage->SetFocus(); //There Parawin is not visible anymore
}

IMPL_LINK( FormulaDlg_Impl, ModifyHdl, ParaWin&, rPtr, void )
{
    if (&rPtr == m_xParaWin.get())
    {
        SaveArg(m_xParaWin->GetActiveLine());
        UpdateValues();

        UpdateSelection();
        CalcStruct(m_xMEdit->get_text());
    }
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FormulaHdl, weld::TextView&, void)
{

    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    m_bEditFlag = true;
    OUString    aInputFormula = m_pHelper->getCurrentFormula();
    OUString    aString = m_xMEdit->get_text();

    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    if (aString.isEmpty())      // in case everything was cleared
    {
        aString += "=";
        m_xMEdit->set_text(aString);
        nStartPos = 1;
        nEndPos = 1;
        m_xMEdit->select_region(nStartPos, nEndPos);
    }
    else if (aString[0]!='=')   // in case it's replaced
    {
        aString = "=" + aString;
        m_xMEdit->set_text(aString);
        nStartPos += 1;
        nEndPos += 1;
        m_xMEdit->select_region(nStartPos, nEndPos);
    }

    m_pHelper->setSelection( 0, aInputFormula.getLength());
    m_pHelper->setCurrentFormula(aString);
    m_pHelper->setSelection(nStartPos, nEndPos);

    sal_Int32 nPos = nStartPos - 1;

    OUString aStrResult;

    if ( CalcValue( m_pHelper->getCurrentFormula(), aStrResult ) )
        m_xWndFormResult->set_text( aStrResult );
    else
    {
        aStrResult.clear();
        m_xWndFormResult->set_text( aStrResult );
    }
    CalcStruct(aString);

    nPos = GetFunctionPos(nPos);

    if (nPos < nStartPos - 1)
    {
        sal_Int32 nPos1 = aString.indexOf( '(', nPos);
        EditNextFunc( false, nPos1);
    }
    else
    {
        ClearAllParas();
    }

    m_pHelper->setSelection(nStartPos, nEndPos);
    m_bEditFlag = false;
}

void FormulaDlg_Impl::FormulaCursor()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    m_bEditFlag = true;

    OUString    aString = m_xMEdit->get_text();

    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    m_pHelper->setSelection(nStartPos, nEndPos);

    if (nStartPos == 0)
    {
        nStartPos = 1;
        m_xMEdit->select_region(nStartPos, nEndPos);
    }
    if (nStartPos != aString.getLength())
    {
        sal_Int32 nPos = nStartPos;

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
    m_pHelper->setSelection(nStartPos, nEndPos);

    m_bEditFlag = false;
}

void FormulaDlg_Impl::UpdateOldSel()
{
    m_xMEdit->get_selection_bounds(m_nSelectionStart, m_nSelectionEnd);
    if (m_nSelectionStart > m_nSelectionEnd)
        std::swap(m_nSelectionStart, m_nSelectionEnd);
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FormulaCursorHdl, weld::TextView&, void)
{
    int nStartPos, nEndPos;
    m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
    if (nStartPos > nEndPos)
        std::swap(nStartPos, nEndPos);

    if (nStartPos != m_nSelectionStart || nEndPos != m_nSelectionEnd)
    {
        m_nSelectionStart = nStartPos;
        m_nSelectionEnd = nEndPos;
        FormulaCursor();
    }
}

void FormulaDlg_Impl::UpdateSelection()
{
    m_pHelper->setSelection( m_aFuncSel.Min(), m_aFuncSel.Max());
    if (m_pFuncDesc)
    {
        m_pHelper->setCurrentFormula( m_pFuncDesc->getFormula( m_aArguments ) );
        m_nArgs = m_pFuncDesc->getSuppressedArgumentCount();
    }
    else
    {
        m_pHelper->setCurrentFormula(u""_ustr);
        m_nArgs = 0;
    }

    m_xMEdit->set_text(m_pHelper->getCurrentFormula());
    sal_Int32 PrivStart, PrivEnd;
    m_pHelper->getSelection( PrivStart, PrivEnd);
    m_aFuncSel.Min() = PrivStart;
    m_aFuncSel.Max() = PrivEnd;

    OUString aFormula = m_xMEdit->get_text();
    sal_Int32 nArgPos = m_aFormulaHelper.GetArgStart( aFormula, PrivStart, 0);

    sal_uInt16 nPos = m_xParaWin->GetActiveLine();
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

    m_pHelper->setSelection(nArgPos, nArgPos + nLength);
    m_xMEdit->select_region(nArgPos, nArgPos + nLength);
    UpdateOldSel();
}

::std::pair<RefButton*, RefEdit*> FormulaDlg_Impl::RefInputStartBefore(RefEdit* pEdit, RefButton* pButton)
{
    m_pTheRefEdit = pEdit;
    m_pTheRefButton = pButton;

    Selection aOrigSelection;
    if (m_pTheRefEdit)
    {
        // grab selection before showing next widget in case the selection is blown away
        // by it appearing
        aOrigSelection = m_pTheRefEdit->GetSelection();
    }

    // because its initially hidden, give it its optimal size so clicking the
    // refbutton has an initial size to work when retro-fitting this to .ui
    m_xEdRef->GetWidget()->set_size_request(m_xEdRef->GetWidget()->get_preferred_size().Width(), -1);
    m_xEdRef->GetWidget()->show();

    if ( m_pTheRefEdit )
    {
        m_xEdRef->SetRefString(m_pTheRefEdit->GetText());
        m_xEdRef->SetSelection(aOrigSelection);
        m_xEdRef->GetWidget()->set_help_id(m_pTheRefEdit->GetWidget()->get_help_id());
    }

    m_xRefBtn->GetWidget()->set_visible(pButton != nullptr);

    ::std::pair<RefButton*, RefEdit*> aPair;
    aPair.first = pButton ? m_xRefBtn.get() : nullptr;
    aPair.second = m_xEdRef.get();
    return aPair;
}

void FormulaDlg_Impl::RefInputStartAfter()
{
    m_xRefBtn->SetEndImage();

    if (!m_pTheRefEdit)
        return;

    OUString aStr = m_aTitle2 + " " + m_xFtEditName->get_label() + "( ";

    if ( m_xParaWin->GetActiveLine() > 0 )
        aStr += "...; ";
    aStr += m_xParaWin->GetActiveArgName();
    if ( m_xParaWin->GetActiveLine() + 1 < m_nArgs )
        aStr += "; ...";
    aStr += " )";

    m_rDialog.set_title(m_rDialog.strip_mnemonic(aStr));
}

void FormulaDlg_Impl::RefInputDoneAfter( bool bForced )
{
    m_xRefBtn->SetStartImage();
    if (!bForced && m_xRefBtn->GetWidget()->get_visible())
        return;

    m_xEdRef->GetWidget()->hide();
    m_xRefBtn->GetWidget()->hide();
    if ( m_pTheRefEdit )
    {
        m_pTheRefEdit->SetRefString( m_xEdRef->GetText() );
        m_pTheRefEdit->GrabFocus();

        if ( m_pTheRefButton )
            m_pTheRefButton->SetStartImage();

        sal_uInt16 nPrivActiv = m_xParaWin->GetActiveLine();
        m_xParaWin->SetArgument( nPrivActiv, m_xEdRef->GetText() );
        ModifyHdl( *m_xParaWin );
        m_pTheRefEdit = nullptr;
    }
    m_rDialog.set_title(m_aTitle1);
}

RefEdit* FormulaDlg_Impl::GetCurrRefEdit()
{
    return m_xEdRef->GetWidget()->get_visible() ? m_xEdRef.get() : m_xParaWin->GetActiveEdit();
}

void FormulaDlg_Impl::Update()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    const OUString sExpression = m_xMEdit->get_text();
    m_aOldFormula.clear();
    UpdateTokenArray(sExpression);
    FormulaCursor();
    CalcStruct(sExpression);
    if (pData->GetMode() == FormulaDlgMode::Formula)
        m_xTabCtrl->set_current_page(u"functiontab"_ustr);
    else
        m_xTabCtrl->set_current_page(u"structtab"_ustr);
    m_xBtnMatrix->set_active(pData->GetMatrixFlag());
}

void FormulaDlg_Impl::Update(const OUString& _sExp)
{
    CalcStruct(_sExp);
    FillDialog();
    FuncSelHdl(*m_xFuncPage);
}

void FormulaDlg_Impl::SetMeText(const OUString& _sText)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    m_xMEdit->set_text(_sText);
    auto aSelection = pData->GetSelection();
    m_xMEdit->select_region(aSelection.Min(), aSelection.Max());
    UpdateOldSel();
}

FormulaDlgMode FormulaDlg_Impl::SetMeText( const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate)
{
    FormulaDlgMode eMode = FormulaDlgMode::Formula;
    if (!m_bEditFlag)
        m_xMEdit->set_text(_sText);

    if ( _bSelect || !m_bEditFlag )
        m_xMEdit->select_region(PrivStart, PrivEnd);
    if ( _bUpdate )
    {
        UpdateOldSel();
        int nStartPos, nEndPos;
        m_xMEdit->get_selection_bounds(nStartPos, nEndPos);
        if (nStartPos > nEndPos)
            std::swap(nStartPos, nEndPos);
        m_pHelper->showReference(m_xMEdit->get_text().copy(nStartPos, nEndPos - nStartPos));
        eMode = FormulaDlgMode::Edit;

        m_xBtnMatrix->set_active( bMatrix );
    } // if ( _bUpdate )
    return eMode;
}

bool FormulaDlg_Impl::CheckMatrix(OUString& aFormula)
{
    m_xMEdit->grab_focus();
    sal_Int32 nLen = aFormula.getLength();
    bool bMatrix =  nLen > 3                    // Matrix-Formula
            && aFormula[0] == '{'
            && aFormula[1] == '='
            && aFormula[nLen-1] == '}';
    if ( bMatrix )
    {
        aFormula = aFormula.copy( 1, aFormula.getLength()-2 );
        m_xBtnMatrix->set_active( bMatrix );
        m_xBtnMatrix->set_sensitive(false);
    } // if ( bMatrix )

    m_xTabCtrl->set_current_page(u"structtab"_ustr);
    return bMatrix;
}

IMPL_LINK_NOARG( FormulaDlg_Impl, StructSelHdl, StructPage&, void)
{
    m_bStructUpdate = false;
    if (m_xStructPage->IsVisible())
        m_xBtnForward->set_sensitive(false); //@New
    m_bStructUpdate = true;
}

IMPL_LINK_NOARG( FormulaDlg_Impl, MatrixHdl, weld::Toggleable&, void)
{
    m_bUserMatrixFlag = true;
    UpdateValues(true);
}

IMPL_LINK_NOARG( FormulaDlg_Impl, FuncSelHdl, FuncPage&, void)
{
    if (   (m_xFuncPage->GetFunctionEntryCount() > 0)
        && (m_xFuncPage->GetFunction() != -1) )
    {
        const IFunctionDescription* pDesc = m_xFuncPage->GetFuncDesc( m_xFuncPage->GetFunction() );

        if (pDesc != m_pFuncDesc)
            m_xBtnForward->set_sensitive(true); //new

        if (pDesc)
        {
            pDesc->initArgumentInfo();      // full argument info is needed

            OUString aSig = pDesc->getSignature();
            m_xFtHeadLine->set_label( pDesc->getFunctionName() );
            m_xFtFuncName->set_label( aSig );
            m_xFtFuncDesc->set_label( pDesc->getDescription() );
        }
    }
    else
    {
        m_xFtHeadLine->set_label( OUString() );
        m_xFtFuncName->set_label( OUString() );
        m_xFtFuncDesc->set_label( OUString() );
    }
}

void FormulaDlg_Impl::UpdateParaWin( const Selection& _rSelection, const OUString& _sRefStr)
{
    Selection theSel = _rSelection;
    m_xEdRef->GetWidget()->replace_selection(_sRefStr);
    theSel.Max() = theSel.Min() + _sRefStr.getLength();
    m_xEdRef->SetSelection( theSel );


    // Manual Update of the results' fields:

    sal_uInt16 nPrivActiv = m_xParaWin->GetActiveLine();
    m_xParaWin->SetArgument( nPrivActiv, m_xEdRef->GetText());
    m_xParaWin->UpdateParas();

    RefEdit* pEd = GetCurrRefEdit();
    if (pEd)
        pEd->SetSelection( theSel );
}

bool FormulaDlg_Impl::UpdateParaWin(Selection& _rSelection)
{
    OUString      aStrEd;
    RefEdit* pEd = GetCurrRefEdit();
    if (pEd && !m_pTheRefEdit)
    {
        _rSelection = pEd->GetSelection();
        _rSelection.Normalize();
        aStrEd = pEd->GetText();
        m_xEdRef->SetRefString(aStrEd);
        m_xEdRef->SetSelection( _rSelection );
    }
    else
    {
        _rSelection = m_xEdRef->GetSelection();
        _rSelection.Normalize();
        aStrEd = m_xEdRef->GetText();
    }
    return m_pTheRefEdit == nullptr;
}

void FormulaDlg_Impl::SetEdSelection()
{
    RefEdit* pEd = GetCurrRefEdit()/*aScParaWin.GetActiveEdit()*/;
    if (pEd)
    {
        Selection theSel = m_xEdRef->GetSelection();
        //  Edit may have the focus -> call ModifyHdl in addition
        //  to what's happening in GetFocus
        pEd->GetModifyHdl().Call(*pEd);
        pEd->GrabFocus();
        pEd->SetSelection(theSel);
    } // if ( pEd )
}

FormulaModalDialog::FormulaModalDialog(weld::Window* pParent,
                                       IFunctionManager const * _pFunctionMgr,
                                       IControlReferenceHandler* _pDlg)
    : GenericDialogController(pParent, u"formula/ui/formuladialog.ui"_ustr, u"FormulaDialog"_ustr)
    , m_pImpl(new FormulaDlg_Impl(*m_xDialog, *m_xBuilder, false/*_bSupportFunctionResult*/,
                                  false/*_bSupportResult*/, false/*_bSupportMatrix*/,
                                  this, _pFunctionMgr, _pDlg))
{
    m_xDialog->set_title(m_pImpl->m_aTitle1);
}

FormulaModalDialog::~FormulaModalDialog() { }

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

void FormulaModalDialog::StoreFormEditData(FormEditData* pData)
{
    m_pImpl->StoreFormEditData(pData);
}

//      Initialisation / General functions  for Dialog
FormulaDlg::FormulaDlg(SfxBindings* pB, SfxChildWindow* pCW,
                       weld::Window* pParent,
                       IFunctionManager const * _pFunctionMgr, IControlReferenceHandler* _pDlg)
    : SfxModelessDialogController( pB, pCW, pParent, u"formula/ui/formuladialog.ui"_ustr, u"FormulaDialog"_ustr)
    , m_pImpl(new FormulaDlg_Impl(*m_xDialog, *m_xBuilder, true/*_bSupportFunctionResult*/
                                             , true/*_bSupportResult*/
                                             , true/*_bSupportMatrix*/
                                             , this, _pFunctionMgr, _pDlg))
{
    m_xDialog->set_title(m_pImpl->m_aTitle1);
}

FormulaDlg::~FormulaDlg()
{
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
    return m_pImpl->m_xMEdit->get_text();
}

void FormulaDlg::Update()
{
    m_pImpl->Update();
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

void FormulaDlg::disableOk()
{
    m_pImpl->m_xBtnEnd->set_sensitive(false);
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

RefEdit* FormulaDlg::GetActiveEdit()
{
    return m_pImpl->m_xParaWin->GetActiveEdit();
}

const FormulaHelper& FormulaDlg::GetFormulaHelper() const
{
    return m_pImpl->GetFormulaHelper();
}

void FormulaDlg::SetEdSelection()
{
    m_pImpl->SetEdSelection();
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
