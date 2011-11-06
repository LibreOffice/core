/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_FORMULA_HXX
#define SC_FORMULA_HXX

#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
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
    virtual void RefInputDone( sal_Bool bForced = sal_False );
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

