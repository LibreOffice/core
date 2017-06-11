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

#include <vcl/msgbox.hxx>

#include "strings.hrc"
#include "unknownauthdlg.hxx"
#include <comphelper/processfactory.hxx>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

using namespace css;


IMPL_LINK_NOARG(UnknownAuthDialog, OKHdl_Impl, Button*, void)
{
    if ( m_pOptionButtonAccept->IsChecked() )
    {
        EndDialog( RET_OK );
    } else
    {
        EndDialog();
    }
}


IMPL_LINK_NOARG(UnknownAuthDialog, ViewCertHdl_Impl, Button*, void)
{
    uno::Reference< css::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures(
        css::security::DocumentDigitalSignatures::createDefault(m_xContext) );

    xDocumentDigitalSignatures.get()->showCertificate(m_rXCert);
}


UnknownAuthDialog::UnknownAuthDialog(vcl::Window* pParent,
    const css::uno::Reference< css::security::XCertificate >& rXCert,
    const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : MessageDialog(pParent, "UnknownAuthDialog",
        "uui/ui/unknownauthdialog.ui")
    , m_xContext(xContext)
    , m_rXCert(rXCert)
{
    get(m_pOptionButtonAccept, "accept");
    get(m_pOptionButtonDontAccept, "reject");
    get(m_pCommandButtonOK, "ok");
    get(m_pView_Certificate, "examine");

    m_pView_Certificate->SetClickHdl(LINK(this, UnknownAuthDialog, ViewCertHdl_Impl));
    m_pCommandButtonOK->SetClickHdl(LINK(this, UnknownAuthDialog, OKHdl_Impl));
}

UnknownAuthDialog::~UnknownAuthDialog()
{
    disposeOnce();
}

void UnknownAuthDialog::dispose()
{
    m_pCommandButtonOK.clear();
    m_pView_Certificate.clear();
    m_pOptionButtonAccept.clear();
    m_pOptionButtonDontAccept.clear();
    MessageDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
