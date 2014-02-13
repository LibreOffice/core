/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_NAMEDEFDLG_HXX
#define SC_NAMEDEFDLG_HXX

#include "anyrefdg.hxx"
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

#include <map>

class ScRangeName;
class ScDocument;
class ScDocShell;
class ScViewData;

class ScNameDefDlg : public ScAnyRefDlg
{
private:
    Edit* m_pEdName;

    formula::RefEdit* m_pEdRange;
    formula::RefButton* m_pRbRange;

    ListBox* m_pLbScope;

    CheckBox* m_pBtnRowHeader;
    CheckBox* m_pBtnColHeader;
    CheckBox* m_pBtnPrintArea;
    CheckBox* m_pBtnCriteria;

    PushButton* m_pBtnAdd;
    PushButton* m_pBtnCancel;
    FixedText* m_pFtInfo;

    bool mbUndo; //if true we need to add an undo action after creating a range name
    ScDocument* mpDoc;
    ScDocShell* mpDocShell;

    ScAddress maCursorPos;
    OUString maStrInfoDefault;
    const OUString maGlobalNameStr;
    const OUString maErrInvalidNameStr;
    const OUString maErrNameInUse;

    //hack to call this dialog from Manage Names
    OUString maName;
    OUString maScope;

    std::map<OUString, ScRangeName*> maRangeMap;

    void CancelPushed();
    void AddPushed();

    bool IsNameValid();
    bool IsFormulaValid();

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( NameModifyHdl, void* );
    DECL_LINK( AssignGetFocusHdl, void * );

protected:
    virtual void    RefInputDone( bool bForced = false );

public:
    ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                    ScViewData* pViewData, std::map<OUString, ScRangeName*> aRangeMap,
                    const ScAddress& aCursorPos, const bool bUndo);

    virtual ~ScNameDefDlg() {};

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

    void GetNewData( OUString& rName, OUString& rScope );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
