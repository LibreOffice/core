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
    virtual void dispose() SAL_OVERRIDE;

    virtual bool IsRefInputMode() const SAL_OVERRIDE;
    virtual void SetReference(const ScRange& rRange, ScDocument* pDoc) SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;

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
    DECL_LINK(RefModifiedHdl, void*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
