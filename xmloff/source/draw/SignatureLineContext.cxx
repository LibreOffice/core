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
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <xmloff/xmltoken.hxx>

using namespace css;
using namespace css::xml::sax;
using namespace css::uno;
using namespace css::drawing;
using namespace xmloff::token;

SignatureLineContext::SignatureLineContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
                                           const OUString& rLocalName,
                                           const Reference<XAttributeList>& xAttrList,
                                           const Reference<XShape>& rxShape)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference<beans::XPropertySet> xPropSet(rxShape, UNO_QUERY_THROW);

    xPropSet->setPropertyValue("IsSignatureLine", Any(true));

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
