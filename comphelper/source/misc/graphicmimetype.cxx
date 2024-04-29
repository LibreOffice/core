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
#include <comphelper/mediamimetype.hxx>

#include <map>
#include <set>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

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

    size_t const nCount = std::size(aMapper);
    for (size_t i = 0; (i < nCount) && aMimeType.isEmpty(); ++i)
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
        (xGraphicPropertySet->getPropertyValue(u"MimeType"_ustr) >>= aSourceMimeType))
    {
        return aSourceMimeType;
    }
    return u""_ustr;
}

OUString
GraphicMimeTypeHelper::GetMimeTypeForImageStream(const Reference<XInputStream>& xInputStream)
{
    // Create the graphic to retrieve the mimetype from it
    Reference<XGraphicProvider> xProvider
        = css::graphic::GraphicProvider::create(comphelper::getProcessComponentContext());
    Sequence<PropertyValue> aMediaProperties{ comphelper::makePropertyValue(u"InputStream"_ustr,
                                                                            xInputStream) };
    Reference<XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));

    return GetMimeTypeForXGraphic(xGraphic);
}

OUString GraphicMimeTypeHelper::GetMimeTypeForConvertDataFormat(ConvertDataFormat convertDataFormat)
{
    switch (convertDataFormat)
    {
        case ConvertDataFormat::BMP:
            return u"image/bmp"_ustr;
        case ConvertDataFormat::GIF:
            return u"image/gif"_ustr;
        case ConvertDataFormat::JPG:
            return u"image/jpeg"_ustr;
        case ConvertDataFormat::PCT:
            return u"image/x-pict"_ustr;
        case ConvertDataFormat::PNG:
            return u"image/png"_ustr;
        case ConvertDataFormat::SVM:
            return u"image/x-svm"_ustr;
        case ConvertDataFormat::TIF:
            return u"image/tiff"_ustr;
        case ConvertDataFormat::WMF:
            return u"image/x-wmf"_ustr;
        case ConvertDataFormat::EMF:
            return u"image/x-emf"_ustr;
        case ConvertDataFormat::SVG:
            return u"image/svg+xml"_ustr;
        case ConvertDataFormat::MET: // What is this?
        case ConvertDataFormat::Unknown:
        default:
            return u""_ustr;
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

static auto GetMediaMimes() -> std::map<OString, OString> const&
{
    static std::map<OString, OString> const mimes = {
        { "mp4", "video/mp4" },
        { "ts", "video/MP2T" },
        { "mpeg", "video/mpeg" },
        { "mpg", "video/mpeg" },
        { "mkv", "video/x-matroska" },
        { "webm", "video/webm" },
        { "ogv", "video/ogg" },
        { "mov", "video/quicktime" },
        { "wmv", "video/x-ms-wmv" },
        { "avi", "video/x-msvideo" },
        { "m4a", "audio/mp4" },
        { "aac", "audio/aac" },
        { "mp3", "audio/mpeg" }, // https://bugs.chromium.org/p/chromium/issues/detail?id=227004
        { "ogg", "audio/ogg" },
        { "oga", "audio/ogg" },
        { "opus", "audio/ogg" },
        { "flac", "audio/flac" }, // missing at IANA?
        // note there is RFC 2631 but i got the impression that vnd.wave
        // requires specifying the codec in the container; also this page
        // says "Historic" whatever that means:
        // https://www.iana.org/assignments/wave-avi-codec-registry/wave-avi-codec-registry.xhtml
        { "wav", "audio/x-wav" },
    };
    return mimes;
}

auto IsMediaMimeType(::std::string_view const rMimeType) -> bool
{
    return IsMediaMimeType(OStringToOUString(rMimeType, RTL_TEXTENCODING_UTF8));
}

auto IsMediaMimeType(OUString const& rMimeType) -> bool
{
    static std::set<OUString> mimes;
    if (mimes.empty())
    {
        auto const& rMap(GetMediaMimes());
        for (auto const& it : rMap)
        {
            mimes.insert(OStringToOUString(it.second, RTL_TEXTENCODING_UTF8));
        }
    }
    return rMimeType == AVMEDIA_MIMETYPE_COMMON || mimes.find(rMimeType) != mimes.end();
}

auto GuessMediaMimeType(::std::u16string_view rFileName) -> OUString
{
    if (auto const i = rFileName.rfind('.'); i != ::std::string_view::npos)
    {
        OString const ext(OUStringToOString(rFileName.substr(i + 1), RTL_TEXTENCODING_UTF8));
        auto const& rMap(GetMediaMimes());
        auto const it(rMap.find(ext));
        if (it != rMap.end())
        {
            return OStringToOUString(it->second, RTL_TEXTENCODING_ASCII_US);
        }
    }
    return OUString();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
