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

// SPIF/STANAG security label dialog. Classifications and category tag sets come
// from the SPIF policy; categories are shown as a flat checkable list.
class SwSecurityLabelDlg final : public weld::GenericDialogController
{
    sw::seclabel::SpifPolicy m_aPolicy;

    // Per category row: flat index of its owning tag, and whether that tag is
    // single-selection (toggling one of its categories clears the others).
    std::vector<sal_Int32> m_aRowTag;
    std::vector<bool> m_aTagSingle;

    std::unique_ptr<weld::ComboBox> m_xClassification;
    std::unique_ptr<weld::TreeView> m_xCategories;
    std::unique_ptr<weld::Label> m_xPreview;

    DECL_LINK(ClassificationHdl, weld::ComboBox&, void);
    DECL_LINK(CategoryToggleHdl, const weld::TreeView::iter_col&, void);

    void PopulateCategories();
    void UpdatePreview();

public:
    SwSecurityLabelDlg(weld::Window* pParent);
    virtual ~SwSecurityLabelDlg() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SECLABELDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
