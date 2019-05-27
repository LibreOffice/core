/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_AUTOREDACTDIALOG_HXX
#define INCLUDED_SFX2_AUTOREDACTDIALOG_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>

#include <vcl/idle.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace weld
{
class Button;
}
namespace weld
{
class Label;
}
namespace weld
{
class Window;
}
namespace weld
{
class TreeView;
}

struct RedactionTarget
{
    sal_uInt32 nID;
    OUString sName;
    OUString sType;
    bool bCaseSensitive;
    bool bWholeWords;
    OUString sDescription;
};

class TargetsTable
{
    std::unique_ptr<weld::TreeView> m_xControl;
    int GetRowByTargetName(const OUString& sName);

public:
    TargetsTable(std::unique_ptr<weld::TreeView> xControl);
    void InsertTarget(RedactionTarget* pTarget);
    void SelectByName(const OUString& sName);
    RedactionTarget* GetTargetByName(const OUString& sName);
    OUString GetNameProposal();

    void unselect_all() { m_xControl->unselect_all(); }
    bool has_focus() const { return m_xControl->has_focus(); }
    int n_children() const { return m_xControl->n_children(); }
    int get_selected_index() const { return m_xControl->get_selected_index(); }
    std::vector<int> get_selected_rows() const { return m_xControl->get_selected_rows(); }
    void clear() { m_xControl->clear(); }
    void remove(int nRow) { m_xControl->remove(nRow); }
    void select(int nRow) { m_xControl->select(nRow); }
    OUString get_id(int nRow) const { return m_xControl->get_id(nRow); }

    //void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xControl->connect_changed(rLink); }
    //void connect_row_activated(const Link<weld::TreeView&, void>& rLink) { m_xControl->connect_row_activated(rLink); }
};

class SFX2_DLLPUBLIC SfxAutoRedactDialog : public SfxDialogController
{
    SfxObjectShellLock m_xDocShell;
    std::vector<std::pair<TargetsTable*, OUString>> m_aTableTargets;

    std::unique_ptr<weld::Label> m_xRedactionTargetsLabel;
    std::unique_ptr<TargetsTable> m_xTargetsBox;
    std::unique_ptr<weld::Button> m_xLoadBtn;
    std::unique_ptr<weld::Button> m_xSaveBtn;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xEditBtn;
    std::unique_ptr<weld::Button> m_xDeleteBtn;

public:
    SfxAutoRedactDialog(weld::Window* pParent);
    virtual ~SfxAutoRedactDialog() override;

    /*
     * Check if the dialog has any valid redaction targets.
     */
    bool hasTargets() const;

    // TODO: Some method(s) to check emptiness/validity
    // TODO: Some method(s) to get the search params/objects
    // TODO: Some method(s) to load/save redaction target sets
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
