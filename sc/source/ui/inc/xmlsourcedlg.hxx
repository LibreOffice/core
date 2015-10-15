/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_XMLSOURCEDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_XMLSOURCEDLG_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <svtools/treelistbox.hxx>

#include "anyrefdg.hxx"
#include "orcusxml.hxx"

#include <set>
#include <memory>
#include <vector>

class ScDocument;
class ScRange;
class ScOrcusXMLContext;

class ScXMLSourceDlg : public ScAnyRefDlg
{
    VclPtr<PushButton> mpBtnSelectSource;
    VclPtr<FixedText> mpFtSourceFile;

    VclPtr<VclContainer> mpMapGrid;

    VclPtr<SvTreeListBox> mpLbTree;
    VclPtr<formula::RefEdit> mpRefEdit;
    VclPtr<formula::RefButton> mpRefBtn;

    VclPtr<PushButton> mpBtnOk;
    VclPtr<CancelButton> mpBtnCancel;

    OUString maSrcPath;

    ScOrcusXMLTreeParam maXMLParam;
    std::set<const SvTreeListEntry*> maCellLinks;
    std::set<const SvTreeListEntry*> maRangeLinks;
    std::vector<SvTreeListEntry*> maHighlightedEntries;
    SvTreeListEntry* mpCurRefEntry;
    std::unique_ptr<ScOrcusXMLContext> mpXMLContext;

    ScDocument* mpDoc;

    VclPtr<formula::RefEdit> mpActiveEdit;
    bool mbDlgLostFocus;

public:
    ScXMLSourceDlg(
        SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent, ScDocument* pDoc);
    virtual ~ScXMLSourceDlg();
    virtual void dispose() override;

    virtual bool IsRefInputMode() const override;
    virtual void SetReference(const ScRange& rRange, ScDocument* pDoc) override;
    virtual void Deactivate() override;
    virtual void SetActive() override;
    virtual bool Close() override;

private:

    void SelectSourceFile();
    void LoadSourceFileStructure(const OUString& rPath);
    void HandleGetFocus(Control* pCtrl);
    void TreeItemSelected();
    void DefaultElementSelected(SvTreeListEntry& rEntry);
    void RepeatElementSelected(SvTreeListEntry& rEntry);
    void AttributeSelected(SvTreeListEntry& rEntry);

    void SetNonLinkable();
    void SetSingleLinkable();
    void SetRangeLinkable();
    void SelectAllChildEntries(SvTreeListEntry& rEntry);

    /**
     * Check if any of its parents is linked or repeated.  The passed entry is
     * not checked; its parent is the first one to be checked, then all its
     * parents get checked all the way to the root.
     */
    bool IsParentDirty(SvTreeListEntry* pEntry) const;

    bool IsChildrenDirty(SvTreeListEntry* pEntry) const;

    void OkPressed();
    void CancelPressed();
    void RefEditModified();

    DECL_LINK_TYPED(GetFocusHdl, Control&, void);
    DECL_LINK_TYPED(BtnPressedHdl, Button*, void);
    DECL_LINK_TYPED(TreeItemSelectHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED(RefModifiedHdl, Edit&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
