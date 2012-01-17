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


//==================================================================

//logic behind the manage names dialog
class ScNameDlg : public ScAnyRefDlg
{
private:
    Edit            maEdName;
    formula::RefEdit        maEdAssign;
    formula::RefButton      aRbAssign;
    ListBox         maLbScope;

    DisclosureButton maBtnMore;
    CheckBox        maBtnPrintArea;
    CheckBox        maBtnColHeader;
    CheckBox        maBtnCriteria;
    CheckBox        maBtnRowHeader;

    PushButton      maBtnAdd;
    PushButton      maBtnDelete;
    HelpButton      maBtnHelp;
    PushButton      maBtnOk;
    PushButton      maBtnCancel;

    FixedText       maFtScope;
    FixedText       maFtRange;
    FixedText       maFtName;
    FixedLine       maFlDiv;
    FixedText       maFtInfo;

    ScRangeManagerCtrl maNameMgrCtrl;
    ScRangeManagerTable* mpRangeManagerTable;

    const rtl::OUString maGlobalNameStr;
    const rtl::OUString maErrInvalidNameStr;
    const rtl::OUString maErrNameInUse;
    const rtl::OUString maStrInfoDefault;
    const rtl::OUString maStrMultiSelect;

    ScViewData*     mpViewData;
    ScDocument*     mpDoc;
    const ScAddress maCursorPos;
    Selection       maCurSel;

    bool mbNeedUpdate;
    bool mbDataChanged;
    //ugly hack to call DefineNames from ManageNames
    bool mbCloseWithoutUndo;

    typedef boost::ptr_map<rtl::OUString, ScRangeName> RangeNameContainer;

    RangeNameContainer maRangeMap;

private:
    void Init();
    void UpdateChecks(ScRangeData* pData);
    void ShowOptions(const ScRangeNameLine& rLine);
    void UpdateNames();

    bool IsNameValid();
    bool IsFormulaValid();
    void CheckForEmptyTable();

    ScRangeName* GetRangeName(const rtl::OUString& rScope);

    bool AddPushed();
    void RemovePushed();
    void OKPushed();
    void CancelPushed();
    void NameSelected();
    void ScopeChanged();
    void NameModified();
    void MorePushed();

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
    DECL_LINK( MoreBtnHdl, void* );

protected:
    virtual void    RefInputDone( sal_Bool bForced = sal_False );

public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos, boost::ptr_map<rtl::OUString, ScRangeName>* pRangeMap = NULL );
    virtual         ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

    void GetRangeNames(boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap);
    void SetEntry(const rtl::OUString& rName, const rtl::OUString& rScope);

};



#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
