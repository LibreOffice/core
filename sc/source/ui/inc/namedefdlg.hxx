/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_NAMEDEFDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NAMEDEFDLG_HXX

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
    VclPtr<Edit> m_pEdName;

    VclPtr<formula::RefEdit> m_pEdRange;
    VclPtr<formula::RefButton> m_pRbRange;

    VclPtr<ListBox> m_pLbScope;

    VclPtr<CheckBox> m_pBtnRowHeader;
    VclPtr<CheckBox> m_pBtnColHeader;
    VclPtr<CheckBox> m_pBtnPrintArea;
    VclPtr<CheckBox> m_pBtnCriteria;

    VclPtr<PushButton> m_pBtnAdd;
    VclPtr<PushButton> m_pBtnCancel;
    VclPtr<FixedText> m_pFtInfo;

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

    DECL_LINK_TYPED( CancelBtnHdl, Button*, void );
    DECL_LINK_TYPED( AddBtnHdl, Button*, void );
    DECL_LINK( NameModifyHdl, void* );
    DECL_LINK_TYPED( AssignGetFocusHdl, Control&, void );

protected:
    virtual void    RefInputDone( bool bForced = false ) override;

public:
    ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                    ScViewData* pViewData, const std::map<OUString, ScRangeName*>& aRangeMap,
                    const ScAddress& aCursorPos, const bool bUndo);

    virtual ~ScNameDefDlg();
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual bool    IsRefInputMode() const override;

    virtual void    SetActive() override;
    virtual bool    Close() override;

    void GetNewData( OUString& rName, OUString& rScope );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
