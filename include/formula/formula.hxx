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
    void            SetFocusWin(Window *pWin,const OString& nUniqueId);

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
    void            SetFocusWin(Window *pWin,const OString& nUniqueId);
    void            HighlightFunctionParas(const String& aFormula);

    void            SetMeText(const String& _sText);
    FormulaDlgMode SetMeText(const String& _sText,xub_StrLen PrivStart, xub_StrLen PrivEnd,sal_Bool bMatrix,sal_Bool _bSelect,sal_Bool _bUpdate);
    void            Update();
    sal_Bool            CheckMatrix(String& aFormula /*IN/OUT*/);
    String          GetMeText() const;
    void            Update(const String& _sExp);
    void            CheckMatrix();
    void            DoEnter(sal_Bool _bOk);
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
