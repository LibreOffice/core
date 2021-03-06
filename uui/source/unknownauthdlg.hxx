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
#pragma once

#include <vcl/weld.hxx>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


//= Https_UADialog

class UnknownAuthDialog : public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::Button> m_xCommandButtonOK;
    std::unique_ptr<weld::Button> m_xView_Certificate;
    std::unique_ptr<weld::RadioButton> m_xOptionButtonAccept;
    std::unique_ptr<weld::RadioButton> m_xOptionButtonDontAccept;

    const css::uno::Reference< css::uno::XComponentContext >& m_xContext;
    const css::uno::Reference< css::security::XCertificate >& m_rXCert;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(ViewCertHdl_Impl, weld::Button&, void);

public:
    UnknownAuthDialog(weld::Window* pParent,
        const css::uno::Reference< css::security::XCertificate >& rXCert,
        const css::uno::Reference< css::uno::XComponentContext >& xContext);

    void setDescriptionText(const OUString &rText)
    {
        m_xDialog->set_primary_text(rText);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
