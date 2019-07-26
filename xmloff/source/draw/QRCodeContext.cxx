/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QRCodeContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/QRCode.hpp>
#include <com/sun/star/drawing/QRCodeErrorCorrection.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <sax/tools/converter.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::xml::sax;
using namespace css::uno;
using namespace css::drawing;
using namespace css::embed;
using namespace css::frame;
using namespace css::io;
using namespace css::graphic;
using namespace xmloff::token;

QRCodeContext::QRCodeContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
                             const Reference<XAttributeList>& xAttrList,
                             const Reference<XShape>& rxShape)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference<beans::XPropertySet> xPropSet(rxShape, UNO_QUERY_THROW);

    css::drawing::QRCode aQRCode;
    aQRCode.Payload = xAttrList->getValueByName("office:string-value");

    //ErrorCorrection Import as per Schema
    OUString aErrorCorrValue = xAttrList->getValueByName("loext:qrcode-errorcorrection");
    if (aErrorCorrValue == "low")
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::LOW;
    else if (aErrorCorrValue == "medium")
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::MEDIUM;
    else if (aErrorCorrValue == "quartile")
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::QUARTILE;
    else
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::HIGH;

    bool bBorderConverted = sax::Converter::convertNumber(
        aQRCode.Border, xAttrList->getValueByName("loext:qrcode-border"), 0);

    if (bBorderConverted)
    {
        xPropSet->setPropertyValue("QRCodeProperties", Any(aQRCode));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
