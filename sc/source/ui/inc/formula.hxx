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

#ifndef SC_FORMULA_HXX
#define SC_FORMULA_HXX

#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include "compiler.hxx"
#include "cell.hxx"

#include "formula/formula.hxx"
#include "IAnyRefDialog.hxx"
#include "anyrefdg.hxx"
#include <formula/IFunctionDescription.hxx>

class ScViewData;
class ScDocument;
class ScFuncDesc;
class ScInputHandler;
class ScDocShell;
class SvLBoxEntry;

//============================================================================
typedef ScTabViewShell* PtrTabViewShell;
//============================================================================

class ScFormulaDlg : public formula::FormulaDlg,
                     public IAnyRefDialog,
                     public formula::IFormulaEditorHelper
{
    ScFormulaReferenceHelper m_aHelper;
    ScFormulaCell*  pCell;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser>          m_xParser;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;

    static ScDocument*  pDoc;
    static ScAddress    aCursorPos;
public:
                    ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                        Window* pParent, ScViewData* pViewData ,formula::IFunctionManager* _pFunctionMgr);
                    ~ScFormulaDlg();

    // IFormulaEditorHelper
    virtual void notifyChange();
    virtual void fill();
    virtual bool calculateValue(const String& _sExpression,String& _rResult);
    virtual void doClose(sal_Bool _bOk);
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc);
    virtual void showReference(const String& _sFormula);
    virtual void dispatch(sal_Bool _bOK,sal_Bool _bMartixChecked);
    virtual void setDispatcherLock( sal_Bool bLock );
    virtual void setReferenceInput(const formula::FormEditData* _pData);
    virtual void deleteFormData();
    virtual void clear();
    virtual void switchBack();
    virtual formula::FormEditData* getFormEditData() const;
    virtual void setCurrentFormula(const String& _sReplacement);
    virtual void setSelection(xub_StrLen _nStart,xub_StrLen _nEnd);
    virtual void getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const;
    virtual String getCurrentFormula() const;

    virtual formula::IFunctionManager* getFunctionManager();
    virtual ::std::auto_ptr<formula::FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const;
    virtual ::com::sun::star::table::CellAddress getReferencePosition() const;

    virtual sal_Bool    Close();

    // sc::IAnyRefDialog
    virtual void ShowReference(const String& _sRef);
    virtual void HideReference( sal_Bool bDoneRefMode = sal_True );
    virtual void SetReference( const ScRange& rRef, ScDocument* pD );

    virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void RefInputDone( sal_Bool bForced = false );
    virtual sal_Bool IsTableLocked() const;
    virtual sal_Bool IsRefInputMode() const;

    virtual sal_Bool IsDocAllowed( SfxObjectShell* pDocSh ) const;
    virtual void AddRefEntry();
    virtual void SetActive();
    virtual void ViewShellChanged( ScTabViewShell* pScViewShell );
protected:

    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    sal_uLong        FindFocusWin(Window *pWin);
    void         SaveLRUEntry(const ScFuncDesc* pFuncDesc);
    void         HighlightFunctionParas(const String& aFormula);

    sal_Bool        IsInputHdl(ScInputHandler* pHdl);
    ScInputHandler* GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh);
};



#endif // SC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
