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
OUString GraphicMimeTypeHelper::GetMimeTypeForExtension(std::string_view rExt)
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

    tools::Long const nCount = SAL_N_ELEMENTS(aMapper);
    for (tools::Long i = 0; (i < nCount) && aMimeType.isEmpty(); ++i)
    {
        if (rExt == aMapper[i].pExt)
            aMimeType = OUString(aMapper[i].pMimeType, strlen(aMapper[i].pMimeType),
                                 RTL_TEXTENCODING_ASCII_US);
    }

    return aMimeType;
}

OUString GraphicMimeTypeHelper::GetMimeTypeForXGraphic(const Reference<XGraphic>& xGraphic)
{
    OUString aSourceMimeType;
    Reference<XPropertySet> const xGraphicPropertySet(xGraphic, UNO_QUERY);
    if (xGraphicPropertySet.is() && // it's null if it's an external link
        (xGraphicPropertySet->getPropertyValue("MimeType") >>= aSourceMimeType))
    {
        return aSourceMimeType;
    }
    return "";
}

OUString
GraphicMimeTypeHelper::GetMimeTypeForImageStream(const Reference<XInputStream>& xInputStream)
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

OUString GraphicMimeTypeHelper::GetMimeTypeForConvertDataFormat(ConvertDataFormat convertDataFormat)
{
    switch (convertDataFormat)
    {
        case ConvertDataFormat::BMP:
            return "image/bmp";
        case ConvertDataFormat::GIF:
            return "image/gif";
        case ConvertDataFormat::JPG:
            return "image/jpeg";
        case ConvertDataFormat::PCT:
            return "image/x-pict";
        case ConvertDataFormat::PNG:
            return "image/png";
        case ConvertDataFormat::SVM:
            return "image/x-svm";
        case ConvertDataFormat::TIF:
            return "image/tiff";
        case ConvertDataFormat::WMF:
            return "image/x-wmf";
        case ConvertDataFormat::EMF:
            return "image/x-emf";
        case ConvertDataFormat::SVG:
            return "image/svg+xml";
        case ConvertDataFormat::MET: // What is this?
        case ConvertDataFormat::Unknown:
        default:
            return "";
    }
}

char const* GraphicMimeTypeHelper::GetExtensionForConvertDataFormat(ConvertDataFormat nFormat)
{
    char const* pExt = nullptr;
    // create extension
    if (nFormat != ConvertDataFormat::Unknown)
    {
        switch (nFormat)
        {
            case ConvertDataFormat::BMP:
                pExt = ".bmp";
                break;
            case ConvertDataFormat::GIF:
                pExt = ".gif";
                break;
            case ConvertDataFormat::JPG:
                pExt = ".jpg";
                break;
            case ConvertDataFormat::MET:
                pExt = ".met";
                break;
            case ConvertDataFormat::PCT:
                pExt = ".pct";
                break;
            case ConvertDataFormat::PNG:
                pExt = ".png";
                break;
            case ConvertDataFormat::SVM:
                pExt = ".svm";
                break;
            case ConvertDataFormat::TIF:
                pExt = ".tif";
                break;
            case ConvertDataFormat::WMF:
                pExt = ".wmf";
                break;
            case ConvertDataFormat::EMF:
                pExt = ".emf";
                break;

            default:
                pExt = ".grf";
                break;
        }
    }
    return pExt;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
