/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_AUTHFALLBACKDLG_HXX
#define INCLUDED_SVTOOLS_AUTHFALLBACKDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/vclmedit.hxx>


class AuthFallbackDlg : public ModalDialog
{
private:
    VclPtr<VclMultiLineEdit> m_pTVInstructions;
    VclPtr<Edit> m_pEDUrl;
    VclPtr<Edit> m_pEDCode;
    VclPtr<PushButton> m_pBTOk;
    VclPtr<PushButton> m_pBTCancel;

public:
    AuthFallbackDlg(Window* pParent, const OUString& instructions,
                    const OUString& url);
    virtual ~AuthFallbackDlg();
    virtual void dispose() override;

    OUString GetCode() const { return m_pEDCode->GetText(); }

private:

    DECL_LINK_TYPED ( OKHdl, Button *, void );
    DECL_LINK_TYPED ( CancelHdl, Button *, void );
};

#endif // INCLUDED_SVTOOLS_AUTHFALLBACKDLG_HXX
