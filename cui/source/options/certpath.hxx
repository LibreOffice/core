/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_CERTPATH_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_CERTPATH_HXX

#include <vcl/weld.hxx>

class CertPathDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::TreeView> m_xCertPathList;
    std::unique_ptr<weld::Label> m_xAddDialogLabel;
    std::unique_ptr<weld::Label> m_xManualLabel;
    OUString m_sAddDialogText;
    OUString m_sManual;

    typedef std::pair<int, int> row_col;
    DECL_LINK(CheckHdl_Impl, const row_col&, void);
    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(OKHdl_Impl, weld::Button&, void);

    void HandleEntryChecked(int nRow);
    void AddCertPath(const OUString &rProfile, const OUString &rPath);
public:
    explicit CertPathDialog(weld::Window* pParent);
    virtual ~CertPathDialog() override;

    OUString getDirectory() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
