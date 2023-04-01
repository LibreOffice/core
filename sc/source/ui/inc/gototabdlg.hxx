/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/weld.hxx>

class ScGoToTabDlg : public weld::GenericDialogController
{
private:
    std::vector<OUString> maCacheSheetsNames;

    std::unique_ptr<weld::Frame> m_xFrameMask;
    std::unique_ptr<weld::Entry> m_xEnNameMask;
    std::unique_ptr<weld::Frame> m_xFrameSheets;
    std::unique_ptr<weld::TreeView> m_xLb;

    DECL_LINK(DblClkHdl, weld::TreeView&, bool);
    DECL_LINK(FindNameHdl, weld::Entry&, void);

public:
    ScGoToTabDlg(weld::Window* pParent);
    virtual ~ScGoToTabDlg() override;

    /** Sets dialog title, label texts and help IDs. */
    void SetDescription(const OUString& rTitle, const OUString& rEntryLabel,
                        const OUString& rListLabel, const OUString& rDlgHelpId,
                        const OUString& rEnHelpId, const OUString& rLbHelpId);

    /** Inserts a string into the weld::TreeView. */
    void Insert(const OUString& rString, bool bSelected);

    OUString GetSelectedEntry() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
