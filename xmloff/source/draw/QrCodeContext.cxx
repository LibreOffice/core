/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QrCodeContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
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
using namespace xmloff::token;

QrCodeContext::QrCodeContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
                             const Reference<XAttributeList>& xAttrList,
                             const Reference<XShape>& rxShape)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference<beans::XPropertySet> xPropSet(rxShape, UNO_QUERY_THROW);

    xPropSet->setPropertyValue("QrCodeShape", Any(xAttrList->getValueByName("loext:qrcode")));

    Reference<XGraphic> xGraphic;
    try
    {
        css::uno::Reference<XStorable> xStorable(GetImport().GetModel(), UNO_QUERY_THROW);
        Reference<XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, xStorable->getLocation(), ElementModes::READ);

        if (!xStorage.is())
        {
            SAL_WARN("xmloff", "No xStorage!");
            return;
        }
    }
    catch (css::uno::Exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
