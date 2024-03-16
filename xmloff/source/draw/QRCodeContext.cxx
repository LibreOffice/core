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
#include <com/sun/star/drawing/BarCode.hpp>
#include <com/sun/star/drawing/BarCodeErrorCorrection.hpp>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>

using namespace css;
using namespace css::xml::sax;
using namespace css::uno;
using namespace css::drawing;
using namespace xmloff::token;

QRCodeContext::QRCodeContext(SvXMLImport& rImport, sal_Int32 /*nElement*/,
                             const Reference<XFastAttributeList>& xAttrList,
                             const Reference<XShape>& rxShape)
    : SvXMLImportContext(rImport)
{
    Reference<beans::XPropertySet> xPropSet(rxShape, UNO_QUERY_THROW);

    css::drawing::BarCode aBarCode;

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_QRCODE_ERROR_CORRECTION):
            {
                OUString aErrorCorrValue = aIter.toString();

                if (aErrorCorrValue == "low")
                    aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::LOW;
                else if (aErrorCorrValue == "medium")
                    aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::MEDIUM;
                else if (aErrorCorrValue == "quartile")
                    aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::QUARTILE;
                else
                    aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::HIGH;
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_QRCODE_BORDER):
            {
                sal_Int32 nAttrVal;
                if (sax::Converter::convertNumber(nAttrVal, aIter.toView(), 0))
                    aBarCode.Border = nAttrVal;
                break;
            }
            case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            {
                aBarCode.Payload = aIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_QRCODE_TYPE):
            {
                sal_Int32 nAttrVal;
                if (sax::Converter::convertNumber(nAttrVal, aIter.toView(), 0))
                    aBarCode.Type = nAttrVal;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    xPropSet->setPropertyValue("BarCodeProperties", Any(aBarCode));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
