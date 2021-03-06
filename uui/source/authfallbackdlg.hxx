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

class AuthFallbackDlg : public weld::GenericDialogController
{
private:
    bool m_bGoogleMode;

    std::unique_ptr<weld::Label> m_xTVInstructions;
    std::unique_ptr<weld::Entry> m_xEDUrl;
    std::unique_ptr<weld::Entry> m_xEDCode;
    std::unique_ptr<weld::Entry> m_xEDGoogleCode;
    std::unique_ptr<weld::Button> m_xBTOk;
    std::unique_ptr<weld::Button> m_xBTCancel;
    std::unique_ptr<weld::Widget> m_xGoogleBox;
    std::unique_ptr<weld::Widget> m_xOneDriveBox;

public:
    AuthFallbackDlg(weld::Window* pParent, const OUString& instructions, const OUString& url);
    virtual ~AuthFallbackDlg() override;

    OUString GetCode() const;

private:
    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);
};
