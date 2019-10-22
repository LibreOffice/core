/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

class CertPathDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Button> m_xManualButton;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::TreeView> m_xCertPathList;
    OUString m_sAddDialogText;
    OUString m_sManualLabel;
    OUString m_sManualPath;

    DECL_LINK(CheckHdl_Impl, const weld::TreeView::iter_col&, void);
    DECL_LINK(ManualHdl_Impl, weld::Button&, void);
    DECL_LINK(OKHdl_Impl, weld::Button&, void);

    void HandleEntryChecked(int nRow);
    void AddCertPath(const OUString& rProfile, const OUString& rPath, bool bSelect = true);
    void AddManualCertPath(const OUString& sUserSetCertPath, bool bSelect = true);

public:
    explicit CertPathDialog(weld::Window* pParent);
    virtual ~CertPathDialog() override;

    void Init();

    // returns true, if the service currently uses the selected path or is not initialized
    // yet and therefore has no active NSS path.
    bool isActiveServicePath() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
