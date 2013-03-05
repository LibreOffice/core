/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "anyrefdg.hxx"
#include "namedefdlg.hrc"
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

    std::map<rtl::OUString, ScRangeName*> maRangeMap;

    void CancelPushed();
    void AddPushed();

    bool IsNameValid();
    bool IsFormulaValid();

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( NameModifyHdl, void* );
    DECL_LINK( AssignGetFocusHdl, void * );

protected:
    virtual void    RefInputDone( sal_Bool bForced = sal_False );

public:
    ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                    ScViewData* pViewData, std::map<rtl::OUString, ScRangeName*> aRangeMap,
                    const ScAddress& aCursorPos, const bool bUndo);

    virtual ~ScNameDefDlg() {};

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

    void GetNewData( rtl::OUString& rName, rtl::OUString& rScope );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
