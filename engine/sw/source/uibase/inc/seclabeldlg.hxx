/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SECLABELDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SECLABELDLG_HXX

#include <vcl/weld.hxx>

#include <SpifPolicy.hxx>

class SwWrtShell;

// SPIF/STANAG security label dialog. The provisioned policies populate the policy
// selector; the chosen policy's classifications and category tag sets drive the
// classification dropdown and the flat checkable category list.
class SwSecurityLabelDlg final : public weld::GenericDialogController
{
    sw::seclabel::SpifPolicySet m_aPolicySet;
    // The policy currently driving the editor (an entry of m_aPolicySet), or null
    // when no policy is provisioned.
    const sw::seclabel::SpifPolicy* m_pPolicy = nullptr;
    SwWrtShell& m_rSh;

    // Per category row: flat index of its owning tag, and whether that tag is
    // single-selection (toggling one of its categories clears the others).
    std::vector<sal_Int32> m_aRowTag;
    std::vector<bool> m_aTagSingle;

    // The document carries a label whose policy this dialog cannot edit (its OID
    // does not match the provisioned policy); the dialog shows it read-only and
    // offers re-labeling. Cleared once the user chooses to re-label.
    bool m_bForeignPolicy = false;

    std::unique_ptr<weld::Widget> m_xEditBox;
    std::unique_ptr<weld::ComboBox> m_xPolicy;
    std::unique_ptr<weld::ComboBox> m_xClassification;
    std::unique_ptr<weld::TreeView> m_xCategories;
    std::unique_ptr<weld::Label> m_xPreview;
    std::unique_ptr<weld::Label> m_xWarning;
    std::unique_ptr<weld::Button> m_xOkBtn;
    std::unique_ptr<weld::Button> m_xRelabelBtn;

    DECL_LINK(PolicyHdl, weld::ComboBox&, void);
    DECL_LINK(ClassificationHdl, weld::ComboBox&, void);
    DECL_LINK(CategoryToggleHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(RelabelHdl, weld::Button&, void);

    std::vector<bool> collectSelection() const;
    void applyLabel(const OUString& rClassification, const std::vector<bool>& rSelected);
    void PopulatePolicies();
    void PopulateClassifications();
    void PopulateCategories();
    void UpdatePreview();
    // Make m_aPolicySet.aPolicies[nIndex] the active policy and rebuild the editor.
    void setActivePolicy(int nIndex);

    // Pre-select policy, classification and categories from a label already in the
    // document, or enter the read-only foreign-policy view if its policy is not ours.
    void initFromExistingLabel();
    // Show rLabel read-only and offer re-labeling (foreign/un-provisioned policy).
    void enterForeignMode(const sw::seclabel::StanagLabel& rLabel);

public:
    SwSecurityLabelDlg(weld::Window* pParent, SwWrtShell& rSh);
    virtual ~SwSecurityLabelDlg() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SECLABELDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
