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

#ifndef INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX
#define INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX

#include <svtools/simptabl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <sigstruct.hxx>


namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class HeaderBar;

class CertificateChooser : public ModalDialog
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxCtx;
    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > maCerts;

    VclPtr<SvSimpleTable>   m_pCertLB;
    VclPtr<PushButton>      m_pViewBtn;
    VclPtr<OKButton>        m_pOKBtn;
    VclPtr<Edit> m_pDescriptionED;

    bool                    mbInitialized;

    sal_uInt16              GetSelectedEntryPos() const;

    DECL_LINK(ViewButtonHdl, Button*, void);
    DECL_LINK(CertificateHighlightHdl, SvTreeListBox*, void );
    DECL_LINK(CertificateSelectHdl, SvTreeListBox*, bool );

    void ImplShowCertificateDetails();
    void ImplInitialize();

    void HandleOneUsageBit(OUString& string, int& bits, int bit, const char *name);

public:
    CertificateChooser(vcl::Window* pParent, css::uno::Reference< css::uno::XComponentContext>& rxCtx, css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment);
    virtual ~CertificateChooser() override;
    virtual void dispose() override;

    short Execute() override;

    css::uno::Reference< css::security::XCertificate > GetSelectedCertificate();
    /// Gets the description string provided when selecting the certificate.
    OUString GetDescription();

    OUString UsageInClearText(int bits);
};

#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
