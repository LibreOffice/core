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

#include "anyrefdg.hxx"
#include "namemgrtable.hxx"

#include <memory>
#include <map>

class ScViewData;
class ScDocument;

//logic behind the manage names dialog
class ScNameDlg : public ScAnyRefDlgController
{
private:
    const OUString maGlobalNameStr;
    const OUString maErrInvalidNameStr;
    const OUString maErrNameInUse;
    const OUString maStrMultiSelect;
    OUString maStrInfoDefault;

    ScViewData*     mpViewData;
    ScDocument&     mrDoc;
    const ScAddress maCursorPos;

    bool mbDataChanged;
    //ugly hack to call DefineNames from ManageNames
    bool mbCloseWithoutUndo;

    typedef std::map<OUString, std::unique_ptr<ScRangeName>> RangeNameContainer;

    RangeNameContainer m_RangeMap;

    std::unique_ptr<weld::Entry> m_xEdName;
    std::unique_ptr<weld::Label> m_xFtAssign;
    std::unique_ptr<formula::RefEdit>   m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;
    std::unique_ptr<weld::ComboBox> m_xLbScope;

    std::unique_ptr<weld::CheckButton> m_xBtnPrintArea;
    std::unique_ptr<weld::CheckButton> m_xBtnColHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnCriteria;
    std::unique_ptr<weld::CheckButton> m_xBtnRowHeader;

    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnDelete;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::unique_ptr<weld::Label> m_xFtInfo;

    std::unique_ptr<weld::Expander> m_xExpander;

    std::unique_ptr<ScRangeManagerTable> m_xRangeManagerTable;

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
    void ScopeChanged();
    void NameModified();

    void SelectionChanged();

    // Handler:
    DECL_LINK( OkBtnHdl, weld::Button&, void );
    DECL_LINK( CancelBtnHdl, weld::Button&, void );
    DECL_LINK( AddBtnHdl, weld::Button&, void );
    DECL_LINK( RemoveBtnHdl, weld::Button&, void );
    DECL_LINK( EdModifyHdl, weld::Entry&, void );
    DECL_LINK( RefEdModifyHdl, formula::RefEdit&, void );
    DECL_LINK( EdModifyCheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK( AssignGetFocusHdl, formula::RefEdit&, void );
    DECL_LINK( SelectionChangedHdl_Impl, weld::TreeView&, void );
    DECL_LINK( ScopeChangedHdl, weld::ComboBox&, void );

protected:
    virtual void    RefInputDone( bool bForced = false ) override;

public:
    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
               ScViewData*      ptrViewData,
               const ScAddress& aCursorPos,
               std::map<OUString, std::unique_ptr<ScRangeName>>* pRangeMap = nullptr);
    virtual         ~ScNameDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual bool    IsRefInputMode() const override;

    virtual void    SetActive() override;
    virtual void    Close() override;

    void GetRangeNames(std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap);
    void SetEntry(const OUString& rName, const OUString& rScope);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
