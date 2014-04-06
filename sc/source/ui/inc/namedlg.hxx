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

#ifndef SC_NAMEDLG_HXX
#define SC_NAMEDLG_HXX

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include "rangenam.hxx"
#include "anyrefdg.hxx"
#include "namemgrtable.hxx"

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_set.hpp>

#include <stack>
#include <map>

class ScViewData;
class ScDocument;


//logic behind the manage names dialog
class ScNameDlg : public ScAnyRefDlg
{
private:
    Edit*               m_pEdName;
    formula::RefEdit*   m_pEdAssign;
    formula::RefButton* m_pRbAssign;
    ListBox*            m_pLbScope;

    CheckBox*           m_pBtnPrintArea;
    CheckBox*           m_pBtnColHeader;
    CheckBox*           m_pBtnCriteria;
    CheckBox*           m_pBtnRowHeader;

    PushButton*         m_pBtnAdd;
    PushButton*         m_pBtnDelete;
    PushButton*         m_pBtnOk;
    PushButton*         m_pBtnCancel;

    FixedText*          m_pFtInfo;

    ScRangeManagerTable* m_pRangeManagerTable;

    const OUString maGlobalNameStr;
    const OUString maErrInvalidNameStr;
    const OUString maErrNameInUse;
    const OUString maStrMultiSelect;
    OUString maStrInfoDefault;

    ScViewData*     mpViewData;
    ScDocument*     mpDoc;
    const ScAddress maCursorPos;
    Selection       maCurSel;

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
    void UpdateNames();

    bool IsNameValid();
    bool IsFormulaValid();
    void CheckForEmptyTable();

    ScRangeName* GetRangeName(const OUString& rScope);

    bool AddPushed();
    void RemovePushed();
    void OKPushed();
    void CancelPushed();
    void NameSelected();
    void ScopeChanged();
    void NameModified();

    void SelectionChanged();

    // Handler:
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( SelectBtnHdl, void * );
    DECL_LINK( EdModifyHdl, void * );
    DECL_LINK( AssignGetFocusHdl, void * );
    DECL_LINK( SelectionChangedHdl_Impl, void* );
    DECL_LINK( ScopeChangedHdl, void* );

protected:
    virtual void    RefInputDone( bool bForced = false ) SAL_OVERRIDE;

public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos, boost::ptr_map<OUString, ScRangeName>* pRangeMap = NULL );
    virtual         ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual bool    IsRefInputMode() const SAL_OVERRIDE;

    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

    void GetRangeNames(boost::ptr_map<OUString, ScRangeName>& rRangeMap);
    void SetEntry(const OUString& rName, const OUString& rScope);

};

#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
