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

#ifndef INCLUDED_FORMULA_FORMULA_HXX
#define INCLUDED_FORMULA_FORMULA_HXX

#include <config_options.h>
#include <memory>
#include <utility>

#include <formula/formuladllapi.h>
#include <formula/IFunctionDescription.hxx>
#include <o3tl/deleter.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sfx2/basedlgs.hxx>
#include <tools/gen.hxx>

class NotifyEvent;
class SfxBindings;
class SfxChildWindow;

namespace formula
{

#define STRUCT_END    1
#define STRUCT_FOLDER 2
#define STRUCT_ERROR  3

enum class FormulaDlgMode { Formula, Edit };


class FormulaDlg_Impl;
class IControlReferenceHandler;
class FormulaHelper;
class RefEdit;
class RefButton;
class FormEditData;

class UNLESS_MERGELIBS_MORE(FORMULA_DLLPUBLIC) FormulaModalDialog
    : public weld::GenericDialogController, public formula::IFormulaEditorHelper
{
    friend class FormulaDlg_Impl;
public:
    FormulaModalDialog(weld::Window* pParent, IFunctionManager const * _pFunctionMgr,
                       IControlReferenceHandler* _pDlg);
    virtual ~FormulaModalDialog() override;

private:
    std::unique_ptr<FormulaDlg_Impl, o3tl::default_delete<FormulaDlg_Impl>> m_pImpl;

protected:

    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton );
    void            RefInputStartAfter();
    void            RefInputDoneAfter();

    void            SetMeText(const OUString& _sText);
    void            Update();
    void            CheckMatrix(OUString& aFormula /*IN/OUT*/);
    void            Update(const OUString& _sExp);

    void            StoreFormEditData(FormEditData* pData);
};

class FORMULA_DLLPUBLIC FormulaDlg:
    public SfxModelessDialogController, public IFormulaEditorHelper
{
    friend class FormulaDlg_Impl;
public:
    FormulaDlg(SfxBindings* pB, SfxChildWindow* pCW,
               weld::Window* pParent,
               IFunctionManager const * _pFunctionMgr,
               IControlReferenceHandler* _pDlg);
    virtual ~FormulaDlg() override;
private:
    std::unique_ptr<FormulaDlg_Impl, o3tl::default_delete<FormulaDlg_Impl>> m_pImpl;

protected:
    void            disableOk();

protected:

    ::std::pair<RefButton*,RefEdit*> RefInputStartBefore( RefEdit* pEdit, RefButton* pButton );
    void            RefInputStartAfter();
    void            RefInputDoneAfter( bool bForced );

    void            SetMeText(const OUString& _sText);
    FormulaDlgMode SetMeText(const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate);
    void            Update();
    bool            CheckMatrix(OUString& aFormula /*IN/OUT*/);
    OUString        GetMeText() const;
    void            Update(const OUString& _sExp);
    void            DoEnter();
    const IFunctionDescription* getCurrentFunctionDescription() const;
    bool            UpdateParaWin(Selection& _rSelection);
    void            UpdateParaWin(const Selection& _rSelection, const OUString& _sRefStr);
    RefEdit*    GetActiveEdit();
    void            SetEdSelection();

    void            StoreFormEditData(FormEditData* pData);

    const FormulaHelper& GetFormulaHelper() const;
};

} // formula

#endif // INCLUDED_FORMULA_FORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
