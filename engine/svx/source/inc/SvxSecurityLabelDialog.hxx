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

#pragma once

#include <vcl/weld.hxx>

#include <svx/seclabel/SecurityLabelTarget.hxx>
#include <svx/seclabel/SpifPolicy.hxx>

#include <map>
#include <memory>
#include <vector>

// SPIF/STANAG security label dialog. The provisioned policies populate the policy
// selector; the chosen policy's classifications and category tag sets drive the
// classification dropdown and the grouped checkable category list.
class SvxSecurityLabelDialog final : public weld::GenericDialogController
{
    // The category area is a static pool of group slots (see seclabeldialog.ui): each
    // slot is a bold header label plus a box of checkboxes. Runtime relabels/shows one
    // group per securityCategoryTag and one checkbox per selectable tagCategory, and
    // hides the rest. Static widgets stay interactive in JSDialog (runtime-created ones
    // do not), so the pool is fixed; a policy needing more warns and is truncated.
    static constexpr int MAX_GROUPS = 10;
    static constexpr int MAX_CATS = 100;
    svx::seclabel::SpifPolicySet m_aPolicySet;
    // The policy currently driving the editor (an entry of m_aPolicySet), or null
    // when no policy is provisioned.
    const svx::seclabel::SpifPolicy* m_pPolicy = nullptr;
    // App-specific marking placement + banner push, and the model access.
    std::unique_ptr<svx::seclabel::SecurityLabelTarget> m_pTarget;

    // The used group slots, in policy tag-set/tag order. nCats is how many checkboxes
    // of the slot are in use; bSingle is single-selection (checking one clears the
    // others of that group). collectSelection walks these in order, which matches
    // buildLabel's selectable-category indexing.
    struct GroupSlot
    {
        bool bSingle = false;
        int nCats = 0;
        // Selectable categories the tag really has. Larger than nCats when the tag
        // outgrew the checkbox pool; collectSelection pads the difference.
        int nSelectable = 0;
    };
    std::vector<GroupSlot> m_aGroups;
    // Maps a pooled checkbox to its used-group index, for the single-selection handler.
    std::map<const weld::Toggleable*, size_t> m_aCheckGroup;

    // The document carries a label whose policy this dialog cannot edit (its OID
    // does not match the provisioned policy); the dialog shows it read-only and
    // offers re-labeling. Cleared once the user chooses to re-label.
    bool m_bForeignPolicy = false;

    // The document already carries a label (matching or foreign); enables Remove.
    bool m_bHasLabel = false;

    // The label read from the document (valid when m_bHasLabel). Re-applied whenever
    // its own policy becomes active again, so switching policy away and back does not
    // lose the stored classification and selections.
    svx::seclabel::StanagLabel m_aLabel;

    std::unique_ptr<weld::Widget> m_xEditBox;
    std::unique_ptr<weld::ComboBox> m_xPolicy;
    std::unique_ptr<weld::ComboBox> m_xClassification;
    // The scrolled viewport around the category pool; height-capped so only it scrolls
    // (never the whole dialog).
    std::unique_ptr<weld::ScrolledWindow> m_xCategoriesWin;
    // The static category pool: MAX_GROUPS group boxes, each with a header label and
    // MAX_CATS checkboxes (m_xChecks[group][cat]).
    std::vector<std::unique_ptr<weld::Widget>> m_xGroupBoxes;
    std::vector<std::unique_ptr<weld::Label>> m_xGroupLabels;
    // Italic selection-count note beside each group header (e.g. "select at least 1").
    std::vector<std::unique_ptr<weld::Label>> m_xGroupHints;
    std::vector<std::vector<std::unique_ptr<weld::CheckButton>>> m_xChecks;
    // The marking banner atop the dialog: a title-background strip, hidden while
    // there is no marking to show.
    std::unique_ptr<weld::Label> m_xPreview;
    std::unique_ptr<weld::Label> m_xWarning;
    std::unique_ptr<weld::Button> m_xOkBtn;
    std::unique_ptr<weld::Button> m_xRelabelBtn;
    std::unique_ptr<weld::Button> m_xRemoveBtn;

    DECL_LINK(PolicyHdl, weld::ComboBox&, void);
    DECL_LINK(ClassificationHdl, weld::ComboBox&, void);
    DECL_LINK(CategoryToggleHdl, weld::Toggleable&, void);
    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(RelabelHdl, weld::Button&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);

    std::vector<bool> collectSelection() const;
    void applyLabel(const OUString& rClassification, const std::vector<bool>& rSelected);
    void PopulatePolicies();
    void PopulateClassifications();
    void PopulateCategories();
    void UpdatePreview();
    // Put rMarking in the banner atop the dialog; an empty marking leaves no banner.
    void setPreview(const OUString& rMarking);
    // Make m_aPolicySet.aPolicies[nIndex] the active policy and rebuild the editor.
    void setActivePolicy(int nIndex);

    // Pre-select policy, classification and categories from a label already in the
    // document, or enter the read-only foreign-policy view if its policy is not ours.
    void initFromExistingLabel();
    // Set the classification and category checks from m_aLabel (the active policy must
    // be the label's). Rebuilds the categories, so callers need not populate first.
    void restoreFromLabel();
    // Show rLabel read-only and offer re-labeling (foreign/un-provisioned policy).
    void enterForeignMode(const svx::seclabel::StanagLabel& rLabel);

public:
    SvxSecurityLabelDialog(weld::Window* pParent,
                           std::unique_ptr<svx::seclabel::SecurityLabelTarget> pTarget);
    virtual ~SvxSecurityLabelDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
