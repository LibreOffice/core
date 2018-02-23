/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/svapp.hxx>

#include <strings.hrc>
#include "unknownauthdlg.hxx"
#include <comphelper/processfactory.hxx>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

using namespace css;


IMPL_LINK_NOARG(UnknownAuthDialog, OKHdl_Impl, weld::Button&, void)
{
    if (m_xOptionButtonAccept->get_active())
    {
        m_xDialog->response(RET_OK);
    }
    else
    {
        m_xDialog->response(RET_CANCEL);
    }
}

IMPL_LINK_NOARG(UnknownAuthDialog, ViewCertHdl_Impl, weld::Button&, void)
{
    uno::Reference< css::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures(
        css::security::DocumentDigitalSignatures::createDefault(m_xContext) );

    xDocumentDigitalSignatures.get()->showCertificate(m_rXCert);
}

UnknownAuthDialog::UnknownAuthDialog(weld::Window* pParent,
    const css::uno::Reference< css::security::XCertificate >& rXCert,
    const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xBuilder(Application::CreateBuilder(pParent, "uui/ui/unknownauthdialog.ui"))
    , m_xDialog(m_xBuilder->weld_message_dialog("UnknownAuthDialog"))
    , m_xCommandButtonOK(m_xBuilder->weld_button("ok"))
    , m_xView_Certificate(m_xBuilder->weld_button("examine"))
    , m_xOptionButtonAccept(m_xBuilder->weld_radio_button("accept"))
    , m_xOptionButtonDontAccept(m_xBuilder->weld_radio_button("reject"))
    , m_xContext(xContext)
    , m_rXCert(rXCert)
{
    m_xView_Certificate->connect_clicked(LINK(this, UnknownAuthDialog, ViewCertHdl_Impl));
    m_xCommandButtonOK->connect_clicked(LINK(this, UnknownAuthDialog, OKHdl_Impl));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
