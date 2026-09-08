/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "SignatureLineDialogBase.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <cpo/uno/Reference.hxx>

class SignSignatureLineDialog : public SignatureLineDialogBase
{
public:
    SignSignatureLineDialog(weld::Widget* pParent, cpo::uno::Reference<css::frame::XModel> xModel);

    void Apply();

private:
    std::unique_ptr<weld::Entry> m_xEditName;
    std::unique_ptr<weld::TextView> m_xEditComment;
    std::unique_ptr<weld::Button> m_xBtnLoadImage;
    std::unique_ptr<weld::Button> m_xBtnClearImage;
    std::unique_ptr<weld::Button> m_xBtnChooseCertificate;
    std::unique_ptr<weld::Button> m_xBtnSign;
    std::unique_ptr<weld::Label> m_xLabelHint;
    std::unique_ptr<weld::Label> m_xLabelHintText;
    std::unique_ptr<weld::Label> m_xLabelAddComment;

    cpo::uno::Reference<css::beans::XPropertySet> m_xShapeProperties;
    cpo::uno::Reference<css::security::XCertificate> m_xSelectedCertifate;
    cpo::uno::Reference<css::graphic::XGraphic> m_xSignatureImage;
    OUString m_aSignatureLineId;
    OUString m_aSuggestedSignerName;
    OUString m_aSuggestedSignerTitle;
    bool m_bShowSignDate;
    OUString m_sOriginalImageBtnLabel;

    void ValidateFields();
    cpo::uno::Reference<css::graphic::XGraphic> getSignedGraphic(bool bValid);

    DECL_LINK(clearImage, weld::Button&, void);
    DECL_LINK(loadImage, weld::Button&, void);
    DECL_LINK(chooseCertificate, weld::Button&, void);
    DECL_LINK(entryChanged, weld::Entry&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
