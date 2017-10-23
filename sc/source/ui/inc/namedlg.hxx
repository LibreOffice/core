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

#ifndef INCLUDED_SC_SOURCE_UI_INC_NAMEDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NAMEDLG_HXX

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include <rangenam.hxx>
#include "anyrefdg.hxx"
#include "namemgrtable.hxx"

#include <memory>
#include <stack>
#include <map>

class ScViewData;
class ScDocument;

//logic behind the manage names dialog
class ScNameDlg : public ScAnyRefDlg, public ScRangeManagerTable::InitListener
{
private:
    VclPtr<Edit>               m_pEdName;
    VclPtr<formula::RefEdit>   m_pEdAssign;
    VclPtr<formula::RefButton> m_pRbAssign;
    VclPtr<ListBox>            m_pLbScope;

    VclPtr<CheckBox>           m_pBtnPrintArea;
    VclPtr<CheckBox>           m_pBtnColHeader;
    VclPtr<CheckBox>           m_pBtnCriteria;
    VclPtr<CheckBox>           m_pBtnRowHeader;

    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnDelete;
    VclPtr<PushButton>         m_pBtnOk;
    VclPtr<PushButton>         m_pBtnCancel;

    VclPtr<FixedText>          m_pFtInfo;

    VclPtr<ScRangeManagerTable> m_pRangeManagerTable;

    const OUString maGlobalNameStr;
    const OUString maErrInvalidNameStr;
    const OUString maErrNameInUse;
    const OUString maStrMultiSelect;
    OUString maStrInfoDefault;

    ScViewData*     mpViewData;
    ScDocument*     mpDoc;
    const ScAddress maCursorPos;

    bool mbNeedUpdate;
    bool mbDataChanged;
    //ugly hack to call DefineNames from ManageNames
    bool mbCloseWithoutUndo;

    typedef std::map<OUString, std::unique_ptr<ScRangeName>> RangeNameContainer;

    RangeNameContainer m_RangeMap;

private:
    void Init();
    void UpdateChecks(const ScRangeData* pData);
    void ShowOptions(const ScRangeNameLine& rLine);

    bool IsNameValid();
    bool IsFormulaValid();
    void CheckForEmptyTable();

    ScRangeName* GetRangeName(const OUString& rScope);

    void AddPushed();
    void RemovePushed();
    void CancelPushed();
    void ScopeChanged();
    void NameModified();

    void SelectionChanged();

    // Handler:
    DECL_LINK( OkBtnHdl, Button*, void );
    DECL_LINK( CancelBtnHdl, Button*, void );
    DECL_LINK( AddBtnHdl, Button*, void );
    DECL_LINK( RemoveBtnHdl, Button*, void );
    DECL_LINK( EdModifyHdl, Edit&, void );
    DECL_LINK( EdModifyCheckBoxHdl, CheckBox&, void );
    DECL_LINK( AssignGetFocusHdl, Control&, void );
    DECL_LINK( SelectionChangedHdl_Impl, SvTreeListBox*, void );
    DECL_LINK( ScopeChangedHdl, ListBox&, void );

protected:
    virtual void    RefInputDone( bool bForced = false ) override;

public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos,
                               std::map<OUString, std::unique_ptr<ScRangeName>>* pRangeMap = nullptr);
    virtual         ~ScNameDlg() override;
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual bool    IsRefInputMode() const override;

    virtual void    SetActive() override;
    virtual bool    Close() override;

    virtual void tableInitialized() override;

    void GetRangeNames(std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap);
    void SetEntry(const OUString& rName, const OUString& rScope);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
