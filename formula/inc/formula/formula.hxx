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

#ifndef FORMULA_FORMULA_HXX
#define FORMULA_FORMULA_HXX

#include <sfx2/basedlgs.hxx>
#include <memory>
#include "formula/formuladllapi.h"
#include "formula/omoduleclient.hxx"
#include <formula/IFunctionDescription.hxx>

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

class FormulaDlg_Impl;
class IControlReferenceHandler;
class IFunctionDescription;
class IFunctionManager;
class FormulaHelper;
class RefEdit;
class RefButton;
class FormEditData;
//============================================================================
class FORMULA_DLLPUBLIC FormulaModalDialog :   public ModalDialog, public formula::IFormulaEditorHelper
{
    friend class FormulaDlg_Impl;
public:
                    FormulaModalDialog( Window* pParent
                                            , bool _bSupportFunctionResult
                                            , bool _bSupportResult
                                            , bool _bSupportMatrix
                                            ,IFunctionManager* _pFunctionMgr
                                            ,IControlReferenceHandler* _pDlg = NULL );
                    virtual ~FormulaModalDialog();
private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<FormulaDlg_Impl> m_pImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputDoneAfter( sal_Bool bForced = sal_False );
    void            SetFocusWin(Window *pWin,const rtl::OString& nUniqueId);

    void            SetMeText(const String& _sText);
    void            Update();
    sal_Bool            CheckMatrix(String& aFormula /*IN/OUT*/);
    void            Update(const String& _sExp);

    void            StoreFormEditData(FormEditData* pData);
};

class FORMULA_DLLPUBLIC FormulaDlg:
    private OModuleClient, public SfxModelessDialog, public IFormulaEditorHelper
        // order of base classes is important, as OModuleClient controls the
        // lifecycle of the ResMgr passed into SfxModelessDialog (via
        // formula::ModuleRes), and at least with DBG_UTIL calling TestRes in
        // ~Resource, the ResMgr must outlive the Resource (from which
        // SfxModelessDialog ultimately derives)
{
    friend class FormulaDlg_Impl;
public:
                    FormulaDlg( SfxBindings* pB
                                    , SfxChildWindow* pCW
                                    , Window* pParent
                                    , bool _bSupportFunctionResult
                                    , bool _bSupportResult
                                    , bool _bSupportMatrix
                                    , IFunctionManager* _pFunctionMgr
                                    , IControlReferenceHandler* _pDlg = NULL );
                    virtual ~FormulaDlg();
private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<FormulaDlg_Impl> m_pImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    DECL_LINK( UpdateFocusHdl, void*);
protected:
    void            disableOk();

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton = NULL );
    void            RefInputDoneAfter( sal_Bool bForced = sal_False );
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

    void            StoreFormEditData(FormEditData* pData);

    const FormulaHelper& GetFormulaHelper() const;
};

} // formula

#endif // FORMULA_FORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
