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

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <sax/tools/converter.hxx>

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
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i = 0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);

        switch (nPrefix)
        {
            case XML_NAMESPACE_LO_EXT:
                if (IsXMLToken(aLocalName, XML_QRCODE_ERROR_CORRECTION))
                {
                    OUString aErrorCorrValue = sValue;

                    if (aErrorCorrValue == "low")
                        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::LOW;
                    else if (aErrorCorrValue == "medium")
                        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::MEDIUM;
                    else if (aErrorCorrValue == "quartile")
                        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::QUARTILE;
                    else
                        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::HIGH;
                }

                if (IsXMLToken(aLocalName, XML_QRCODE_BORDER))
                {
                    sal_Int32 nAttrVal;
                    if (sax::Converter::convertNumber(nAttrVal, sValue, 0))
                        aQRCode.Border = nAttrVal;
                }
                break;

            case XML_NAMESPACE_OFFICE:
                if (IsXMLToken(aLocalName, XML_STRING_VALUE))
                {
                    aQRCode.Payload = sValue;
                }
        }
    }
    xPropSet->setPropertyValue("QRCodeProperties", Any(aQRCode));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
