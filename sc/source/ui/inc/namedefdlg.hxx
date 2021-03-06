/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "anyrefdg.hxx"

#include <map>

class ScRangeName;
class ScDocument;
class ScDocShell;
class ScViewData;

class ScNameDefDlg : public ScAnyRefDlgController
{
private:
    bool mbUndo; //if true we need to add an undo action after creating a range name
    ScDocument& mrDoc;
    ScDocShell* mpDocShell;

    ScAddress maCursorPos;
    OUString maStrInfoDefault;
    const OUString maGlobalNameStr;
    const OUString maErrInvalidNameStr;
    const OUString maErrInvalidNameCellRefStr;
    const OUString maErrNameInUse;

    //hack to call this dialog from Manage Names
    OUString maName;
    OUString maScope;

    std::map<OUString, ScRangeName*> maRangeMap;

    std::unique_ptr<weld::Entry> m_xEdName;

    std::unique_ptr<formula::RefEdit> m_xEdRange;
    std::unique_ptr<formula::RefButton> m_xRbRange;

    std::unique_ptr<weld::ComboBox> m_xLbScope;

    std::unique_ptr<weld::CheckButton> m_xBtnRowHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnColHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnPrintArea;
    std::unique_ptr<weld::CheckButton> m_xBtnCriteria;

    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Label> m_xFtInfo;
    std::unique_ptr<weld::Expander> m_xExpander;
    std::unique_ptr<weld::Label> m_xFtRange;

    void CancelPushed();
    void AddPushed();

    bool IsNameValid();
    bool IsFormulaValid();

    DECL_LINK(CancelBtnHdl, weld::Button&, void);
    DECL_LINK(AddBtnHdl, weld::Button&, void);
    DECL_LINK(NameModifyHdl, weld::Entry&, void);
    DECL_LINK(AssignGetFocusHdl, formula::RefEdit&, void);

protected:
    virtual void RefInputDone(bool bForced = false) override;

public:
    ScNameDefDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                 const ScViewData& rViewData, const std::map<OUString, ScRangeName*>& aRangeMap,
                 const ScAddress& aCursorPos, const bool bUndo);

    virtual ~ScNameDefDlg() override;

    virtual void SetReference(const ScRange& rRef, ScDocument& rDoc) override;
    virtual bool IsRefInputMode() const override;

    virtual void SetActive() override;
    virtual void Close() override;

    void GetNewData(OUString& rName, OUString& rScope);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
