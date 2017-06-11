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

#include "ids.hxx"
#include "sslwarndlg.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

using namespace css;

void SSLWarnDialog::ViewCert()
{
    uno::Reference< css::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures;

    xDocumentDigitalSignatures = css::security::DocumentDigitalSignatures::createDefault( m_xContext );

    xDocumentDigitalSignatures.get()->showCertificate(m_rXCert);
}


SSLWarnDialog::SSLWarnDialog(vcl::Window* pParent,
    const css::uno::Reference< css::security::XCertificate >& rXCert,
    const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : MessageDialog(pParent, "SSLWarnDialog", "uui/ui/sslwarndialog.ui")
    , m_xContext(xContext)
    , m_rXCert(rXCert)
{
}

void SSLWarnDialog::response(short nResponseId)
{
    switch (nResponseId)
    {
        case -1:
            ViewCert();
            break;
        default:
            MessageDialog::response(nResponseId);
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
