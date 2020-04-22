/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SignatureLineContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>

using namespace css;
using namespace css::xml::sax;
using namespace css::uno;
using namespace css::drawing;
using namespace css::embed;
using namespace css::frame;
using namespace css::io;
using namespace css::graphic;
using namespace css::security;
using namespace xmloff::token;

SignatureLineContext::SignatureLineContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
                                           const OUString& rLocalName,
                                           const Reference<XAttributeList>& xAttrList,
                                           const Reference<XShape>& rxShape)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference<beans::XPropertySet> xPropSet(rxShape, UNO_QUERY_THROW);

    xPropSet->setPropertyValue("IsSignatureLine", Any(true));

    // FIXME prefix abuse
    // TODO 1.4 what namespace?
    xPropSet->setPropertyValue("SignatureLineId", Any(xAttrList->getValueByName("loext:id")));
    xPropSet->setPropertyValue("SignatureLineSuggestedSignerName",
                               Any(xAttrList->getValueByName("loext:suggested-signer-name")));
    xPropSet->setPropertyValue("SignatureLineSuggestedSignerTitle",
                               Any(xAttrList->getValueByName("loext:suggested-signer-title")));
    xPropSet->setPropertyValue("SignatureLineSuggestedSignerEmail",
                               Any(xAttrList->getValueByName("loext:suggested-signer-email")));
    xPropSet->setPropertyValue("SignatureLineSigningInstructions",
                               Any(xAttrList->getValueByName("loext:signing-instructions")));

    bool bShowSignDate = xAttrList->getValueByName("loext:show-sign-date") == GetXMLToken(XML_TRUE);
    bool bCanAddComment
        = xAttrList->getValueByName("loext:can-add-comment") == GetXMLToken(XML_TRUE);
    xPropSet->setPropertyValue("SignatureLineShowSignDate", Any(bShowSignDate));
    xPropSet->setPropertyValue("SignatureLineCanAddComment", Any(bCanAddComment));

    // Save unsigned graphic (need it when exporting)
    Reference<XGraphic> xUnsignedGraphic;
    xPropSet->getPropertyValue("Graphic") >>= xUnsignedGraphic;
    if (xUnsignedGraphic.is())
        xPropSet->setPropertyValue("SignatureLineUnsignedImage", Any(xUnsignedGraphic));

    Reference<XGraphic> xGraphic;
    bool bIsSigned(false);
    try
    {
        // Get the document signatures
        css::uno::Reference<XStorable> xStorable(GetImport().GetModel(), UNO_QUERY_THROW);
        Reference<XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, xStorable->getLocation(), ElementModes::READ);

        if (!xStorage.is())
        {
            SAL_WARN("xmloff", "No xStorage!");
            return;
        }

        OUString const aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(xStorage));
        Reference<XDocumentDigitalSignatures> xSignatures(
            security::DocumentDigitalSignatures::createWithVersion(
                comphelper::getProcessComponentContext(), aODFVersion));

        Sequence<DocumentSignatureInformation> xSignatureInfo
            = xSignatures->verifyDocumentContentSignatures(xStorage, Reference<XInputStream>());

        // Try to find matching signature line image - if none exists that is fine,
        // then the signature line is not digitally signed.
        auto pSignatureInfo = std::find_if(
            xSignatureInfo.begin(), xSignatureInfo.end(),
            [&xAttrList](const DocumentSignatureInformation& rSignatureInfo) {
                // FIXME prefix abuse
                // TODO 1.4 what namespace?
                return rSignatureInfo.SignatureLineId == xAttrList->getValueByName("loext:id");
            });
        if (pSignatureInfo != xSignatureInfo.end())
        {
            bIsSigned = true;
            if (pSignatureInfo->SignatureIsValid)
            {
                // Signature is valid, use the 'valid' image
                SAL_WARN_IF(!pSignatureInfo->ValidSignatureLineImage.is(), "xmloff",
                            "No ValidSignatureLineImage!");
                xGraphic = pSignatureInfo->ValidSignatureLineImage;
            }
            else
            {
                // Signature is invalid, use the 'invalid' image
                SAL_WARN_IF(!pSignatureInfo->InvalidSignatureLineImage.is(), "xmloff",
                            "No InvalidSignatureLineImage!");
                xGraphic = pSignatureInfo->InvalidSignatureLineImage;
            }

            xPropSet->setPropertyValue("Graphic", Any(xGraphic));
        }
        xPropSet->setPropertyValue("SignatureLineIsSigned", Any(bIsSigned));
    }
    catch (css::uno::Exception&)
    {
        // DocumentDigitalSignatures service not available.
        // We render the "unsigned" shape instead.
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
