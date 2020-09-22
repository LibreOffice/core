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

#include <vcl/weld.hxx>

#include <map>

class ScDocument;
class ScConditionalFormat;
class ScConditionalFormatList;

class ScCondFormatManagerWindow
{
private:
    void Init();
    void setColSizes();

    weld::TreeView& mrTreeView;
    ScDocument&     mrDoc;
    ScConditionalFormatList* mpFormatList;

public:
    ScCondFormatManagerWindow(weld::TreeView& rTreeView, ScDocument& rDoc, ScConditionalFormatList* pFormatList);

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
};

class ScCondFormatManagerDlg : public weld::GenericDialogController
{
public:
    ScCondFormatManagerDlg(weld::Window* pParent, ScDocument& rDoc, const ScConditionalFormatList* pFormatList);
    virtual ~ScCondFormatManagerDlg() override;

    std::unique_ptr<ScConditionalFormatList> GetConditionalFormatList();

    bool CondFormatsChanged() const;
    void SetModified();

    ScConditionalFormat* GetCondFormatSelected();

private:
    bool m_bModified;
    std::unique_ptr<ScConditionalFormatList> m_xFormatList;

    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;
    std::unique_ptr<weld::Button> m_xBtnEdit;
    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<ScCondFormatManagerWindow> m_xCtrlManager;

    void UpdateButtonSensitivity();

    DECL_LINK(RemoveBtnHdl, weld::Button&, void);
    DECL_LINK(EditBtnClickHdl, weld::Button&, void);
    DECL_LINK(AddBtnHdl, weld::Button&, void);
    DECL_LINK(EditBtnHdl, weld::TreeView&, bool);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
