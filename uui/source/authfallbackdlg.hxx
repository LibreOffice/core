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
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

class AuthFallbackDlg : public ModalDialog
{
private:
    VclPtr<VclMultiLineEdit> m_pTVInstructions;
    VclPtr<Edit> m_pEDUrl;
    VclPtr<Edit> m_pEDCode;
    VclPtr<Edit> m_pEDGoogleCode;
    VclPtr<PushButton> m_pBTOk;
    VclPtr<PushButton> m_pBTCancel;
    VclPtr<VclVBox> m_pGoogleBox;
    VclPtr<VclVBox> m_pOneDriveBox;
    bool m_bGoogleMode;

public:
    AuthFallbackDlg(Window* pParent, const OUString& instructions,
                    const OUString& url);
    virtual ~AuthFallbackDlg() override;
    virtual void dispose() override;

    OUString GetCode() const;

private:

    DECL_LINK ( OKHdl, Button *, void );
    DECL_LINK ( CancelHdl, Button *, void );
};

#endif // INCLUDED_SVTOOLS_AUTHFALLBACKDLG_HXX
