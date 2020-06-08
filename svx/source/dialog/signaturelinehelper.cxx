/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/signaturelinehelper.hxx>

#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>
#include <tools/stream.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/weld.hxx>

using namespace com::sun::star;

namespace svx::SignatureLineHelper
{
OUString getSignatureImage(const OUString& rType)
{
    OUString aType = rType;
    if (aType.isEmpty())
    {
        aType = "signature-line.svg";
    }
    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/" + aType);
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, StreamMode::READ);
    if (aStream.GetError() != ERRCODE_NONE)
    {
        SAL_WARN("cui.dialogs", "failed to open " << aType);
    }

    OString const svg = read_uInt8s_ToOString(aStream, aStream.remainingSize());
    return OUString::fromUtf8(svg);
}

uno::Reference<security::XCertificate> getSignatureCertificate(SfxObjectShell* pShell,
                                                               weld::Window* pParent)
{
    if (!pShell)
    {
        return uno::Reference<security::XCertificate>();
    }

    if (!pParent)
    {
        return uno::Reference<security::XCertificate>();
    }

    uno::Reference<security::XDocumentDigitalSignatures> xSigner;
    if (pShell->GetMedium()->GetFilter()->IsAlienFormat())
    {
        xSigner = security::DocumentDigitalSignatures::createDefault(
            comphelper::getProcessComponentContext());
    }
    else
    {
        OUString const aODFVersion(
            comphelper::OStorageHelper::GetODFVersionFromStorage(pShell->GetStorage()));
        xSigner = security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), aODFVersion);
    }
    xSigner->setParentWindow(pParent->GetXWindow());
    OUString aDescription;
    security::CertificateKind certificateKind = security::CertificateKind_NONE;
    // When signing ooxml, we only want X.509 certificates
    if (pShell->GetMedium()->GetFilter()->IsAlienFormat())
    {
        certificateKind = security::CertificateKind_X509;
    }
    uno::Reference<security::XCertificate> xSignCertificate
        = xSigner->selectSigningCertificateWithType(certificateKind, aDescription);
    return xSignCertificate;
}

OUString getSignerName(const css::uno::Reference<css::security::XCertificate>& xCertificate)
{
    return comphelper::xmlsec::GetContentPart(xCertificate->getSubjectName(),
                                              xCertificate->getCertificateKind());
}

OUString getLocalizedDate()
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    Date aDateTime(Date::SYSTEM);
    return rLocaleData.getDate(aDateTime);
}

uno::Reference<graphic::XGraphic> importSVG(const OUString& rSVG)
{
    SvMemoryStream aSvgStream(4096, 4096);
    aSvgStream.WriteOString(OUStringToOString(rSVG, RTL_TEXTENCODING_UTF8));
    uno::Reference<io::XInputStream> xInputStream(new utl::OSeekableInputStreamWrapper(aSvgStream));
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<graphic::XGraphicProvider> xProvider
        = graphic::GraphicProvider::create(xContext);

    uno::Sequence<beans::PropertyValue> aMediaProperties(1);
    aMediaProperties[0].Name = "InputStream";
    aMediaProperties[0].Value <<= xInputStream;
    uno::Reference<graphic::XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));
    return xGraphic;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
