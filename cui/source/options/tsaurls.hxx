/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_TSAURLS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_TSAURLS_HXX

#include <vcl/weld.hxx>

class TSAURLsDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::TreeView> m_xURLListBox;
    std::unique_ptr<weld::Label> m_xEnterAUrl;

    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(DeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    // After operations in a TreeView we have nothing selected
    // Is Selected element handler for the TreeView
    DECL_LINK(SelectHdl, weld::TreeView&, void);

    std::set<OUString> m_aURLs;

    void AddTSAURL(const OUString &rURL);

public:
    explicit TSAURLsDialog(weld::Window* pParent);
    virtual ~TSAURLsDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
