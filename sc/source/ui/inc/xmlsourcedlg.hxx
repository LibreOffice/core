/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#if 1

#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "svtools/treelistbox.hxx"

#include "expftext.hxx"
#include "anyrefdg.hxx"
#include "orcusxml.hxx"

#include <set>
#include <vector>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScRange;
class ScOrcusXMLContext;

class ScXMLSourceTree : public SvTreeListBox
{
public:
    ScXMLSourceTree(Window* pParent, const ResId& rResId);
};

class ScXMLSourceDlg : public ScAnyRefDlg
{
    FixedLine maFlSourceFile;
    ImageButton maBtnSelectSource;
    ScExpandedFixedText maFtSourceFile;

    FixedLine maFtMapXmlDoc;

    FixedText maFtMappedCellTitle;

    ScXMLSourceTree maLbTree;
    formula::RefEdit   maRefEdit;
    formula::RefButton maRefBtn;

    OKButton maBtnOk;
    CancelButton maBtnCancel;

    Image maImgFileOpen;

    rtl::OUString maSrcPath;

    ScOrcusXMLTreeParam maXMLParam;
    std::set<const SvTreeListEntry*> maCellLinks;
    std::set<const SvTreeListEntry*> maRangeLinks;
    std::vector<SvTreeListEntry*> maHighlightedEntries;
    SvTreeListEntry* mpCurRefEntry;
    boost::scoped_ptr<ScOrcusXMLContext> mpXMLContext;

    ScDocument* mpDoc;

    formula::RefEdit* mpActiveEdit;
    bool mbDlgLostFocus;

public:
    ScXMLSourceDlg(
        SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, ScDocument* pDoc);
    virtual ~ScXMLSourceDlg();

    virtual sal_Bool IsRefInputMode() const;
    virtual void SetReference(const ScRange& rRange, ScDocument* pDoc);
    virtual void Deactivate();
    virtual void SetActive();
    virtual sal_Bool Close();

private:

    void SelectSourceFile();
    void LoadSourceFileStructure(const OUString& rPath);
    void HandleGetFocus(Control* pCtrl);
    void HandleLoseFocus(Control* pCtrl);
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

    DECL_LINK(GetFocusHdl, Control*);
    DECL_LINK(LoseFocusHdl, Control*);
    DECL_LINK(BtnPressedHdl, Button*);
    DECL_LINK(TreeItemSelectHdl, void*);
    DECL_LINK(RefModifiedHdl, void*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
