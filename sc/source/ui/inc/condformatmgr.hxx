/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATMGR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATMGR_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/simptabl.hxx>

#include <map>

class ScDocument;
class ScConditionalFormat;
class ScConditionalFormatList;

class ScCondFormatManagerWindow : public SvSimpleTable
{
private:
    void Init();
    OUString createEntryString(const ScConditionalFormat& rFormat);
    void setColSizes();

    ScDocument* const mpDoc;
    ScConditionalFormatList* mpFormatList;
    std::map<SvTreeListEntry*, sal_Int32> maMapLBoxEntryToCondIndex;

public:
    ScCondFormatManagerWindow(SvSimpleTableContainer& rParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList);

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
    virtual void Resize() override;
};

class ScCondFormatManagerDlg : public ModalDialog
{
public:
    ScCondFormatManagerDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList);
    virtual ~ScCondFormatManagerDlg() override;
    virtual void dispose() override;

    std::unique_ptr<ScConditionalFormatList> GetConditionalFormatList();

    bool CondFormatsChanged() const;
    void SetModified();

    ScConditionalFormat* GetCondFormatSelected();

private:
    VclPtr<PushButton> m_pBtnAdd;
    VclPtr<PushButton> m_pBtnRemove;
    VclPtr<PushButton> m_pBtnEdit;
    std::unique_ptr<ScConditionalFormatList> mpFormatList;
    VclPtr<ScCondFormatManagerWindow> m_pCtrlManager;
    void UpdateButtonSensitivity();

    DECL_LINK(RemoveBtnHdl, Button*, void);
    DECL_LINK(EditBtnClickHdl, Button*, void);
    DECL_LINK(AddBtnHdl, Button*, void);
    DECL_LINK(EditBtnHdl, SvTreeListBox*, bool);

    bool mbModified;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
