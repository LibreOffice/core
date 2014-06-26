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
private :

    VclMultiLineEdit* m_pTVInstructions;
    Edit* m_pEDUrl;
    Edit* m_pEDCode;
    PushButton* m_pBTOk;
    PushButton* m_pBTCancel;

public :

     AuthFallbackDlg( Window* pParent);
     AuthFallbackDlg(Window* pParent, const OUString& instructions,
             const OUString& url );
     virtual ~AuthFallbackDlg();

    OUString GetCode() { return m_pEDCode->GetText(); }

private:

    DECL_LINK ( OKHdl, Button * );
    DECL_LINK ( CancelHdl, Button * );
};

#endif // INCLUDED_SVTOOLS_AUTHFALLBACKDLG_HXX
