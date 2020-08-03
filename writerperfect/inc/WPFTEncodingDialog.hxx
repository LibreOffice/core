/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

#include "writerperfectdllapi.h"

namespace writerperfect
{
class WRITERPERFECT_DLLPUBLIC WPFTEncodingDialog final : public weld::GenericDialogController
{
public:
    WPFTEncodingDialog(weld::Window* pParent, const OUString& title, const OUString& defEncoding);

    virtual ~WPFTEncodingDialog() override;

    OUString GetEncoding() const;
    bool hasUserCalledCancel() const { return m_userHasCancelled; }

private:
    bool m_userHasCancelled;

    std::unique_ptr<weld::ComboBox> m_xLbCharset;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

private:
    DECL_LINK(CancelHdl, weld::Button&, void);

    WPFTEncodingDialog(WPFTEncodingDialog const&) = delete;
    WPFTEncodingDialog& operator=(WPFTEncodingDialog const&) = delete;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
