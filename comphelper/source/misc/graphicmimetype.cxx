/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/graphicmimetype.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::beans;
using namespace css::graphic;
using namespace css::io;
using namespace css::uno;

namespace comphelper
{
OUString GraphicMimeTypeHelper::GetMimeTypeForExtension(const OString& rExt)
{
    struct XMLGraphicMimeTypeMapper
    {
        const char* pExt;
        const char* pMimeType;
    };

    static const XMLGraphicMimeTypeMapper aMapper[]
        = { { "gif", "image/gif" },   { "png", "image/png" },     { "jpg", "image/jpeg" },
            { "tif", "image/tiff" },  { "svg", "image/svg+xml" }, { "pdf", "application/pdf" },
            { "wmf", "image/x-wmf" }, { "emf", "image/x-emf" },   { "eps", "image/x-eps" },
            { "bmp", "image/bmp" },   { "pct", "image/x-pict" },  { "svm", "image/x-svm" } };

    OUString aMimeType;

    long const nCount = SAL_N_ELEMENTS(aMapper);
    for (long i = 0; (i < nCount) && aMimeType.isEmpty(); ++i)
    {
        if (rExt == aMapper[i].pExt)
            aMimeType = OUString(aMapper[i].pMimeType, strlen(aMapper[i].pMimeType),
                                 RTL_TEXTENCODING_ASCII_US);
    }

    return aMimeType;
}

OUString GraphicMimeTypeHelper::GetMimeTypeForXGraphic(Reference<XGraphic> xGraphic)
{
    OUString aSourceMimeType;
    Reference<XPropertySet> const xGraphicPropertySet(xGraphic, UNO_QUERY);
    if (xGraphicPropertySet.is() && // it's null if it's an external link
        (xGraphicPropertySet->getPropertyValue("MimeType") >>= aSourceMimeType))
    {
        return aSourceMimeType;
    }
    return OUString("");
}

OUString GraphicMimeTypeHelper::GetMimeTypeForImageUrl(const OUString& rImageUrl)
{
    // Create the graphic to retrieve the mimetype from it
    Reference<XGraphicProvider> xProvider
        = css::graphic::GraphicProvider::create(comphelper::getProcessComponentContext());
    Sequence<PropertyValue> aMediaProperties(1);
    aMediaProperties[0].Name = "URL";
    aMediaProperties[0].Value <<= rImageUrl;
    Reference<XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));

    return GetMimeTypeForXGraphic(xGraphic);
}

OUString GraphicMimeTypeHelper::GetMimeTypeForImageStream(Reference<XInputStream> xInputStream)
{
    // Create the graphic to retrieve the mimetype from it
    Reference<XGraphicProvider> xProvider
        = css::graphic::GraphicProvider::create(comphelper::getProcessComponentContext());
    Sequence<PropertyValue> aMediaProperties(1);
    aMediaProperties[0].Name = "InputStream";
    aMediaProperties[0].Value <<= xInputStream;
    Reference<XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));

    return GetMimeTypeForXGraphic(xGraphic);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
