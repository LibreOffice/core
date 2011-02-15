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

#ifndef FORMULA_FORMULA_HXX
#define FORMULA_FORMULA_HXX

#include <sfx2/basedlgs.hxx>
// #include "formula/funcutl.hxx"
#include <memory>
#include "formula/formuladllapi.h"

namespace formula
{
//============================================================================
#define STRUCT_END    1
#define STRUCT_FOLDER 2
#define STRUCT_ERROR  3

#define STRUCT_ERR_C1 1
#define STRUCT_ERR_C2 2

enum FormulaDlgMode { FORMULA_FORMDLG_FORMULA, FORMULA_FORMDLG_ARGS, FORMULA_FORMDLG_EDIT };

//============================================================================

class IFormulaEditorHelper;
class FormulaDlg_Impl;
class IControlReferenceHandler;
class IFunctionDescription;
class IFunctionManager;
class FormulaHelper;
class RefEdit;
class RefButton;
//============================================================================
class FORMULA_DLLPUBLIC FormulaModalDialog :   public ModalDialog
{
    friend class FormulaDlg_Impl;
public:
                    FormulaModalDialog( Window* pParent
                                            , bool _bSupportFunctionResult
                                            , bool _bSupportResult
                                            , bool _bSupportMatrix
                                            ,IFormulaEditorHelper* _pHelper
                                            ,IFunctionManager* _pFunctionMgr
                                            ,IControlReferenceHandler* _pDlg = NULL );
                    virtual ~FormulaModalDialog();
private:
    ::std::auto_ptr<FormulaDlg_Impl> m_pImpl;

protected:
    void            disableOk();

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputDoneAfter( sal_Bool bForced = sal_False );
    rtl::OString    FindFocusWin(Window *pWin);
    void            SetFocusWin(Window *pWin,const rtl::OString& nUniqueId);
    void            HighlightFunctionParas(const String& aFormula);

    void            SetMeText(const String& _sText);
    FormulaDlgMode SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,sal_Bool bMatrix,sal_Bool _bSelect,sal_Bool _bUpdate);
    void            Update();
    sal_Bool            CheckMatrix(String& aFormula /*IN/OUT*/);
    String          GetMeText() const;
    void            Update(const String& _sExp);
    void            CheckMatrix();
    void            DoEnter(sal_Bool _bOk);
    sal_Bool            isUserMatrix() const;
    const IFunctionDescription* getCurrentFunctionDescription() const;
    sal_Bool            UpdateParaWin(Selection& _rSelection);
    void            UpdateParaWin(const Selection& _rSelection,const String& _sRefStr);
    RefEdit*        GetActiveEdit();
    void            SetEdSelection();

    const FormulaHelper& GetFormulaHelper() const;
};

class FORMULA_DLLPUBLIC FormulaDlg :   public SfxModelessDialog
{
    friend class FormulaDlg_Impl;
public:
                    FormulaDlg( SfxBindings* pB
                                    , SfxChildWindow* pCW
                                    , Window* pParent
                                    , bool _bSupportFunctionResult
                                    , bool _bSupportResult
                                    , bool _bSupportMatrix
                                    , IFormulaEditorHelper* _pHelper
                                    , IFunctionManager* _pFunctionMgr
                                    , IControlReferenceHandler* _pDlg = NULL );
                    virtual ~FormulaDlg();
private:
    ::std::auto_ptr<FormulaDlg_Impl> m_pImpl;

    DECL_LINK( UpdateFocusHdl, Timer*);
protected:
    void            disableOk();

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputDoneAfter( sal_Bool bForced = sal_False );
    rtl::OString    FindFocusWin(Window *pWin);
    void            SetFocusWin(Window *pWin,const rtl::OString& nUniqueId);
    void            HighlightFunctionParas(const String& aFormula);

    void            SetMeText(const String& _sText);
    FormulaDlgMode SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,sal_Bool bMatrix,sal_Bool _bSelect,sal_Bool _bUpdate);
    void            Update();
    sal_Bool            CheckMatrix(String& aFormula /*IN/OUT*/);
    String          GetMeText() const;
    void            Update(const String& _sExp);
    void            CheckMatrix();
    void            DoEnter(sal_Bool _bOk);
    sal_Bool            isUserMatrix() const;
    const IFunctionDescription* getCurrentFunctionDescription() const;
    sal_Bool            UpdateParaWin(Selection& _rSelection);
    void            UpdateParaWin(const Selection& _rSelection,const String& _sRefStr);
    RefEdit*        GetActiveEdit();
    void            SetEdSelection();

    const FormulaHelper& GetFormulaHelper() const;
};

} // formula

#endif // FORMULA_FORMULA_HXX

