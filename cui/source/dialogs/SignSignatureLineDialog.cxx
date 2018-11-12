/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SignSignatureLineDialog.hxx>

#include <sal/types.h>
#include <sal/log.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>

#include <comphelper/processfactory.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <tools/stream.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/weld.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace comphelper;
using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::frame;
using namespace css::io;
using namespace css::lang;
using namespace css::frame;
using namespace css::text;
using namespace css::drawing;
using namespace css::graphic;
using namespace css::security;

SignSignatureLineDialog::SignSignatureLineDialog(weld::Widget* pParent, Reference<XModel> xModel)
    : SignatureLineDialogBase(pParent, std::move(xModel), "cui/ui/signsignatureline.ui",
                              "SignSignatureLineDialog")
    , m_xEditName(m_xBuilder->weld_entry("edit_name"))
    , m_xEditComment(m_xBuilder->weld_text_view("edit_comment"))
    , m_xBtnChooseCertificate(m_xBuilder->weld_button("btn_select_certificate"))
    , m_xBtnSign(m_xBuilder->weld_button("ok"))
    , m_xLabelHint(m_xBuilder->weld_label("label_hint"))
    , m_xLabelHintText(m_xBuilder->weld_label("label_hint_text"))
    , m_xLabelAddComment(m_xBuilder->weld_label("label_add_comment"))
    , m_bShowSignDate(false)
{
    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    m_xShapeProperties.set(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    bool bIsSignatureLine(false);
    m_xShapeProperties->getPropertyValue("IsSignatureLine") >>= bIsSignatureLine;
    if (!bIsSignatureLine)
    {
        SAL_WARN("cui.dialogs", "No signature line selected!");
        return;
    }

    m_xBtnChooseCertificate->connect_clicked(
        LINK(this, SignSignatureLineDialog, chooseCertificate));
    m_xEditName->connect_changed(LINK(this, SignSignatureLineDialog, entryChanged));

    // Read properties from selected signature line
    m_xShapeProperties->getPropertyValue("SignatureLineId") >>= m_aSignatureLineId;
    m_xShapeProperties->getPropertyValue("SignatureLineSuggestedSignerName")
        >>= m_aSuggestedSignerName;
    m_xShapeProperties->getPropertyValue("SignatureLineSuggestedSignerTitle")
        >>= m_aSuggestedSignerTitle;
    OUString aSigningInstructions;
    m_xShapeProperties->getPropertyValue("SignatureLineSigningInstructions")
        >>= aSigningInstructions;
    m_xShapeProperties->getPropertyValue("SignatureLineShowSignDate") >>= m_bShowSignDate;
    bool bCanAddComment(false);
    m_xShapeProperties->getPropertyValue("SignatureLineCanAddComment") >>= bCanAddComment;

    if (aSigningInstructions.isEmpty())
    {
        m_xLabelHint->hide();
        m_xLabelHintText->hide();
    }
    else
    {
        m_xLabelHintText->set_label(aSigningInstructions);
    }

    if (bCanAddComment)
    {
        m_xEditComment->set_size_request(m_xEditComment->get_approximate_digit_width() * 48,
                                         m_xEditComment->get_text_height() * 5);
    }
    else
    {
        m_xLabelAddComment->hide();
        m_xEditComment->hide();
        m_xEditComment->set_size_request(0, 0);
    }

    ValidateFields();
}

IMPL_LINK_NOARG(SignSignatureLineDialog, chooseCertificate, weld::Button&, void)
{
    // Document needs to be saved before selecting a certificate
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if (!pShell->PrepareForSigning(m_xDialog.get()))
        return;

    Reference<XDocumentDigitalSignatures> xSigner(DocumentDigitalSignatures::createWithVersion(
        comphelper::getProcessComponentContext(), "1.2"));
    OUString aDescription;
    CertificateKind certificateKind = CertificateKind_NONE;
    // When signing ooxml, we only want X.509 certificates
    if (pShell->GetMedium()->GetFilter()->IsAlienFormat())
        certificateKind = CertificateKind_X509;
    Reference<XCertificate> xSignCertificate
        = xSigner->selectSigningCertificateWithType(certificateKind, aDescription);

    if (xSignCertificate.is())
    {
        m_xSelectedCertifate = xSignCertificate;
        m_xBtnChooseCertificate->set_label(
            xmlsec::GetContentPart(xSignCertificate->getSubjectName()));
    }
    ValidateFields();
}

IMPL_LINK_NOARG(SignSignatureLineDialog, entryChanged, weld::Entry&, void) { ValidateFields(); }

void SignSignatureLineDialog::ValidateFields()
{
    bool bEnable = m_xSelectedCertifate.is() && !m_xEditName->get_text().isEmpty();
    m_xBtnSign->set_sensitive(bEnable);
}

void SignSignatureLineDialog::Apply()
{
    if (!m_xSelectedCertifate.is())
    {
        SAL_WARN("cui.dialogs", "No certificate selected!");
        return;
    }

    SfxObjectShell* pShell = SfxObjectShell::Current();
    Reference<XGraphic> xValidGraphic = getSignedGraphic(true);
    Reference<XGraphic> xInvalidGraphic = getSignedGraphic(false);
    pShell->SignSignatureLine(m_xDialog.get(), m_aSignatureLineId, m_xSelectedCertifate,
                              xValidGraphic, xInvalidGraphic, m_xEditComment->get_text());
}

const css::uno::Reference<css::graphic::XGraphic>
SignSignatureLineDialog::getSignedGraphic(bool bValid)
{
    // Read svg and replace placeholder texts
    OUString aSvgImage(getSignatureImage());
    aSvgImage = aSvgImage.replaceAll("[SIGNER_NAME]", getCDataString(m_aSuggestedSignerName));
    aSvgImage = aSvgImage.replaceAll("[SIGNER_TITLE]", getCDataString(m_aSuggestedSignerTitle));

    aSvgImage = aSvgImage.replaceAll("[SIGNATURE]", getCDataString(m_xEditName->get_text()));
    OUString aIssuerLine
        = CuiResId(RID_SVXSTR_SIGNATURELINE_SIGNED_BY)
              .replaceFirst("%1", xmlsec::GetContentPart(m_xSelectedCertifate->getSubjectName()));
    aSvgImage = aSvgImage.replaceAll("[SIGNED_BY]", getCDataString(aIssuerLine));
    if (bValid)
        aSvgImage = aSvgImage.replaceAll("[INVALID_SIGNATURE]", "");

    OUString aDate;
    if (m_bShowSignDate && bValid)
    {
        const SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
        Date aDateTime(Date::SYSTEM);
        aDate = rLocaleData.getDate(aDateTime);
    }
    aSvgImage = aSvgImage.replaceAll("[DATE]", aDate);

    // Create graphic
    SvMemoryStream aSvgStream(4096, 4096);
    aSvgStream.WriteOString(OUStringToOString(aSvgImage, RTL_TEXTENCODING_UTF8));
    Reference<XInputStream> xInputStream(new utl::OSeekableInputStreamWrapper(aSvgStream));
    Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<XGraphicProvider> xProvider = css::graphic::GraphicProvider::create(xContext);

    Sequence<PropertyValue> aMediaProperties(1);
    aMediaProperties[0].Name = "InputStream";
    aMediaProperties[0].Value <<= xInputStream;
    Reference<XGraphic> xGraphic = xProvider->queryGraphic(aMediaProperties);
    return xGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
