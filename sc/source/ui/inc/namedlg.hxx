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
#include <vcl/ctrl.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include "rangenam.hxx"
#include "anyrefdg.hxx"

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_set.hpp>

#include <stack>
#include <map>

class ScViewData;
class ScDocument;
struct ScNameDlgImpl;

struct ScRangeNameLine
{
    rtl::OUString aName;
    rtl::OUString aExpression;
    rtl::OUString aScope;
};

class ScRangeManagerTable : public SvTabListBox
{
private:
    HeaderBar maHeaderBar;
    ScRangeName* mpGlobalRangeName;
    std::map<rtl::OUString, ScRangeName*> maTabRangeNames;
    rtl::OUString maGlobalString;
public:
    ScRangeManagerTable( Window* pParent, ScRangeName* pGlobalRangeName, std::map<rtl::OUString, ScRangeName*> aTabRangeNames );
    ~ScRangeManagerTable() {};

    void addEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    void UpdateEntries();
};

class ScRangeManagerCtrl : public Control
{
public:
    ScRangeManagerCtrl(Window* pParent, const ScResId& rResId):
        Control( pParent, rResId) {}
};

//Undo Stack
class ScNameManagerUndo
{
protected:
public:
    virtual ~ScNameManagerUndo();
    virtual void Undo();
};

class ScNameManagerUndoAdd : public ScNameManagerUndo
{
    ScRangeData* mpData;
    ScRangeName* mpRangeName;
public:
    ScNameManagerUndoAdd(ScRangeName* pRangeName, ScRangeData* pData):
        mpData(pData), mpRangeName(pRangeName) {}
    virtual ~ScNameManagerUndoAdd();
    virtual void Undo();
};

class ScNameManagerUndoDelete : public ScNameManagerUndo
{
    ScRangeData* mpData;
    ScRangeName* mpRangeName;
public:
    ScNameManagerUndoDelete(ScRangeName* pRangeName, ScRangeData* pData):
        mpData(pData), mpRangeName(pRangeName) {}
    virtual ~ScNameManagerUndoDelete();
    virtual void Undo();
};

class ScNameManagerUndoModify : public ScNameManagerUndo
{
    ScRangeData* mpOldData;
    ScRangeData* mpNewData;
    ScRangeName* mpOldRangeName;
    ScRangeName* mpNewRangeName;
public:
    ScNameManagerUndoModify(ScRangeName* pOldRangeName, ScRangeData* pOldData, ScRangeName* pNewRangeName, ScRangeData* pNewData):
        mpOldData(pOldData), mpNewData(pNewData), mpOldRangeName(pOldRangeName), mpNewRangeName(pNewRangeName) {}
    virtual ~ScNameManagerUndoModify();
    virtual void Undo();
};

//==================================================================

class ScNameDlg : public ScAnyRefDlg
{
private:
    FixedText       maFtScope;
    FixedText       maFtRange;
    FixedText       maFtName;
    ListBox         maLbScope;

    formula::RefEdit        maEdAssign;
    Edit            maEdName;
    formula::RefButton      aRbAssign;

    FixedLine       maFlDiv;
    CheckBox        maBtnPrintArea;
    CheckBox        maBtnColHeader;
    CheckBox        maBtnCriteria;
    CheckBox        maBtnRowHeader;

    ScRangeManagerCtrl maNameMgrCtrl;
    ScRangeManagerTable* mpRangeManagerTable;

    HelpButton      maBtnHelp;
    PushButton      maBtnAdd;
    PushButton      maBtnModify;
    PushButton      maBtnBack;
    PushButton      maBtnDelete;
    PushButton      maBtnClose;
    MoreButton      maBtnMore;

    const String    mErrMsgInvalidSym;
    const rtl::OUString maErrMsgModifiedFailed;
    const ::rtl::OUString maGlobalNameStr;

    ScViewData*     mpViewData;
    ScDocument*     mpDoc;
    const ScAddress maCursorPos;
    Selection       maCurSel;

    std::stack<ScNameManagerUndo*> maUndoStack;

private:
    void Init();
    void UpdateChecks(ScRangeData* pData);
    void ShowOptions(const ScRangeNameLine& rLine);
    void UpdateNames();
    void CalcCurTableAssign( String& aAssign, ScRangeData* pRangeData );


    bool AddPushed();
    void RemovePushed();
    void OKPushed();
    void ModifiedPushed();
    void NameSelected();
    void ScopeChanged();
    void NameModified();
    void BackPushed();

    void SelectionChanged();

    // Handler:
    DECL_LINK( CloseBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( ModifyBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( EdModifyHdl, void * );
    DECL_LINK( NameSelectHdl, void * );
    DECL_LINK( AssignGetFocusHdl, void * );
    DECL_LINK( SelectionChangedHdl_Impl, void* );
    DECL_LINK( BackBtnHdl, void * );
    DECL_LINK( ScopeChangedHdl, void* );

protected:
    virtual void    RefInputDone( sal_Bool bForced = sal_False );

public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos );
                    ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

};



#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
