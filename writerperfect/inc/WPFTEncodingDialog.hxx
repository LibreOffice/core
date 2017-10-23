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

#ifndef INCLUDED_WRITERPERFECT_WPFTENCODINGDIALOG_HXX
#define INCLUDED_WRITERPERFECT_WPFTENCODINGDIALOG_HXX

#include <sal/types.h>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>

#include "writerperfectdllapi.h"

namespace writerperfect
{

class WRITERPERFECT_DLLPUBLIC WPFTEncodingDialog : public ModalDialog
{
public:
    WPFTEncodingDialog(const OUString &title, const OUString &defEncoding);

    virtual ~WPFTEncodingDialog() override;

    OUString GetEncoding() const;
    bool hasUserCalledCancel() const
    {
        return m_userHasCancelled;
    }
private:
    VclPtr<ListBox> m_pLbCharset;
    VclPtr<OKButton>           m_pBtnOk;
    VclPtr<CancelButton>           m_pBtnCancel;

    bool m_userHasCancelled;
private:
    DECL_LINK(DoubleClickHdl, ListBox &, void);
    DECL_LINK(CancelHdl, Button *, void);

    void dispose() override;

    WPFTEncodingDialog(WPFTEncodingDialog const &) = delete;
    WPFTEncodingDialog &operator=(WPFTEncodingDialog const &) = delete;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
