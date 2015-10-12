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
#ifndef INCLUDED_UUI_SOURCE_UNKNOWNAUTHDLG_HXX
#define INCLUDED_UUI_SOURCE_UNKNOWNAUTHDLG_HXX

#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


//= Https_UADialog

class UnknownAuthDialog : public MessageDialog
{
private:
    VclPtr<PushButton> m_pCommandButtonOK;
    VclPtr<PushButton> m_pView_Certificate;
    VclPtr<RadioButton> m_pOptionButtonAccept;
    VclPtr<RadioButton> m_pOptionButtonDontAccept;

    const css::uno::Reference< css::uno::XComponentContext >& m_xContext;
    const css::uno::Reference< css::security::XCertificate >& m_rXCert;

    DECL_LINK_TYPED(OKHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ViewCertHdl_Impl, Button*, void);

public:
    UnknownAuthDialog(vcl::Window* pParent,
        const css::uno::Reference< css::security::XCertificate >& rXCert,
        const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~UnknownAuthDialog();
    virtual void dispose() override;

    css::uno::Reference< css::security::XCertificate > getCert()
    {
        return m_rXCert;
    }

    void setDescriptionText(const OUString &rText)
    {
        set_primary_text(rText);
    }
};

#endif // INCLUDED_UUI_SOURCE_UNKNOWNAUTHDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
