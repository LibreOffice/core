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
#include <orcusxml.hxx>

#include <set>
#include <memory>

class ScDocument;
class ScRange;
class ScOrcusXMLContext;

struct CustomCompare
{
    weld::TreeView& mrLbTree;
    CustomCompare(weld::TreeView& rLbTree)
        : mrLbTree(rLbTree)
    {
    }
    bool operator()(const std::unique_ptr<weld::TreeIter>& lhs,
                    const std::unique_ptr<weld::TreeIter>& rhs) const
    {
        return mrLbTree.iter_compare(*lhs, *rhs) == -1;
    }
};

class ScXMLSourceDlg : public ScAnyRefDlgController
{
    OUString maSrcPath;

    ScOrcusXMLTreeParam maXMLParam;
    std::unique_ptr<weld::TreeIter> mxCurRefEntry;
    std::unique_ptr<ScOrcusXMLContext> mpXMLContext;

    ScDocument* mpDoc;
    bool mbDlgLostFocus;

    formula::RefEdit* mpActiveEdit;
    std::unique_ptr<weld::Button> mxBtnSelectSource;
    std::unique_ptr<weld::Label> mxFtSourceFile;

    std::unique_ptr<weld::Container> mxMapGrid;

    std::unique_ptr<weld::TreeView> mxLbTree;
    std::unique_ptr<formula::RefEdit> mxRefEdit;
    std::unique_ptr<formula::RefButton> mxRefBtn;

    std::unique_ptr<weld::Button> mxBtnOk;
    std::unique_ptr<weld::Button> mxBtnCancel;

    CustomCompare maCustomCompare;

    std::set<std::unique_ptr<weld::TreeIter>, CustomCompare> maCellLinks;
    std::set<std::unique_ptr<weld::TreeIter>, CustomCompare> maRangeLinks;

public:
    ScXMLSourceDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent, ScDocument* pDoc);
    virtual ~ScXMLSourceDlg() override;

    virtual bool IsRefInputMode() const override;
    virtual void SetReference(const ScRange& rRange, ScDocument& rDoc) override;
    virtual void Deactivate() override;
    virtual void SetActive() override;
    virtual void Close() override;

private:
    void SelectSourceFile();
    void LoadSourceFileStructure(const OUString& rPath);
    void TreeItemSelected();
    void DefaultElementSelected(weld::TreeIter& rEntry);
    void RepeatElementSelected(weld::TreeIter& rEntry);
    void AttributeSelected(weld::TreeIter& rEntry);

    void SetNonLinkable();
    void SetSingleLinkable();
    void SetRangeLinkable();
    void SelectAllChildEntries(weld::TreeIter& rEntry);

    /**
     * Check if any of its parents is linked or repeated.  The passed entry is
     * not checked; its parent is the first one to be checked, then all its
     * parents get checked all the way to the root.
     */
    bool IsParentDirty(const weld::TreeIter* pEntry) const;

    bool IsChildrenDirty(const weld::TreeIter* pEntry) const;

    void OkPressed();
    void CancelPressed();
    void RefEditModified();

    DECL_LINK(BtnPressedHdl, weld::Button&, void);
    DECL_LINK(TreeItemSelectHdl, weld::TreeView&, void);
    DECL_LINK(RefModifiedHdl, formula::RefEdit&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
