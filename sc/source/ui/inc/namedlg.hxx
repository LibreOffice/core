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

#include "rangenam.hxx"
#include "anyrefdg.hxx"
#include "namemgrtable.hxx"

#include <boost/ptr_container/ptr_map.hpp>

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

    typedef boost::ptr_map<OUString, ScRangeName> RangeNameContainer;

    RangeNameContainer maRangeMap;

private:
    void Init();
    void UpdateChecks(ScRangeData* pData);
    void ShowOptions(const ScRangeNameLine& rLine);

    bool IsNameValid();
    bool IsFormulaValid();
    void CheckForEmptyTable();

    ScRangeName* GetRangeName(const OUString& rScope);

    bool AddPushed();
    void RemovePushed();
    void CancelPushed();
    void ScopeChanged();
    void NameModified();

    void SelectionChanged();

    // Handler:
    DECL_LINK_TYPED( OkBtnHdl, Button*, void );
    DECL_LINK_TYPED( CancelBtnHdl, Button*, void );
    DECL_LINK_TYPED( AddBtnHdl, Button*, void );
    DECL_LINK_TYPED( RemoveBtnHdl, Button*, void );
    DECL_LINK( EdModifyHdl, void * );
    DECL_LINK_TYPED( EdModifyCheckBoxHdl, CheckBox&, void );
    DECL_LINK_TYPED( AssignGetFocusHdl, Control&, void );
    DECL_LINK_TYPED( SelectionChangedHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( ScopeChangedHdl, ListBox&, void );

protected:
    virtual void    RefInputDone( bool bForced = false ) SAL_OVERRIDE;

public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos, boost::ptr_map<OUString, ScRangeName>* pRangeMap = NULL );
    virtual         ~ScNameDlg();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual bool    IsRefInputMode() const SAL_OVERRIDE;

    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

    virtual void tableInitialized() SAL_OVERRIDE;

    void GetRangeNames(boost::ptr_map<OUString, ScRangeName>& rRangeMap);
    void SetEntry(const OUString& rName, const OUString& rScope);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
