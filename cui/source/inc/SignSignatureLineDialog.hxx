/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_CUI_INC_SIGNSIGNATURELINEDIALOG_HXX
#define INCLUDED_CUI_INC_SIGNSIGNATURELINEDIALOG_HXX

#include <vcl/weld.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/uno/Reference.hxx>

class SignSignatureLineDialog : public weld::GenericDialogController
{
public:
    SignSignatureLineDialog(weld::Widget* pParent, css::uno::Reference<css::frame::XModel> xModel);

    short execute();

private:
    std::unique_ptr<weld::Entry> m_xEditName;
    std::unique_ptr<weld::TextView> m_xEditComment;
    std::unique_ptr<weld::Button> m_xBtnChooseCertificate;
    std::unique_ptr<weld::Button> m_xBtnSign;

    css::uno::Reference<css::frame::XModel> m_xModel;
    css::uno::Reference<css::beans::XPropertySet> m_xShapeProperties;
    css::uno::Reference<css::security::XCertificate> m_xSelectedCertifate;
    OUString m_aSignatureLineId;
    OUString m_aSuggestedSignerName;
    OUString m_aSuggestedSignerTitle;
    bool m_bShowSignDate;

    void ValidateFields();
    void Apply();
    OUString getCDataString(const OUString& rString);
    OUString getSignatureImage();

    DECL_LINK(chooseCertificate, weld::Button&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
