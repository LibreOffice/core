/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <rtl/ustring.hxx>
#include <xmlmultiimagehelper.hxx>

#include <comphelper/graphicmimetype.hxx>

using namespace ::com::sun::star;

namespace
{
    OUString getMimeTypeForURL(const OUString& rString)
    {
        OUString sMimeType;
        if (rString.startsWith("vnd.sun.star.Package"))
        {
            OString aExtension = OUStringToOString(rString.subView(rString.lastIndexOf(".") + 1), RTL_TEXTENCODING_ASCII_US);
            sMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension(aExtension);
        }
        return sMimeType;
    }

    sal_uInt32 getQualityIndex(std::u16string_view rMimeType)
    {
        // pixel formats first
        if (rMimeType == u"image/bmp")
        {
            return 10;
        }
        if (rMimeType == u"image/gif")
        {
            return 20;
        }
        if (rMimeType == u"image/jpeg")
        {
            return 30;
        }
        if (rMimeType == u"image/png")
        {
            return 40;
        }

        // vector formats, prefer always
        if (rMimeType == u"image/x-vclgraphic") // MIMETYPE_VCLGRAPHIC
        {
            return 990;
        }
        if (rMimeType == u"image/x-svm")
        {
            return 1000;
        }
        if (rMimeType == u"image/x-wmf")
        {
            return 1010;
        }
        if (rMimeType == u"image/x-emf")
        {
            return 1020;
        }
        if (rMimeType == u"image/x-eps")
        {
            return 1025;
        }
        if (rMimeType == u"application/pdf")
        {
            return 1030;
        }
        if (rMimeType == u"image/svg+xml")
        {
            return 1040;
        }

        return 0;
    }
}

MultiImageImportHelper::MultiImageImportHelper()
:   maImplContextVector(),
    mbSupportsMultipleContents(false)
{
}

MultiImageImportHelper::~MultiImageImportHelper()
{
}

SvXMLImportContextRef MultiImageImportHelper::solveMultipleImages()
{
    SvXMLImportContextRef pContext;
    if(maImplContextVector.size() > 1)
    {
        // multiple child contexts were imported, decide which is the most valuable one
        // and remove the rest
        std::vector<SvXMLImportContextRef>::size_type nIndexOfPreferred(maImplContextVector.size());
        sal_uInt32 nBestQuality(0);

        for(std::vector<SvXMLImportContextRef>::size_type a = 0; a < maImplContextVector.size(); a++)
        {
            const SvXMLImportContext& rContext = *maImplContextVector[a];


            OUString sMimeType;

            const OUString aStreamURL(getGraphicPackageURLFromImportContext(rContext));
            if (!aStreamURL.isEmpty())
            {
                sMimeType = getMimeTypeForURL(aStreamURL);
            }
            else
            {
                uno::Reference<graphic::XGraphic> xGraphic(getGraphicFromImportContext(rContext));
                if (xGraphic.is())
                    sMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForXGraphic(xGraphic);
            }

            sal_uInt32 nNewQuality = getQualityIndex(sMimeType);
            if (nNewQuality > nBestQuality)
            {
                nBestQuality = nNewQuality;
                nIndexOfPreferred = a;
            }
        }

        // correct if needed, default is to use the last entry
        if(nIndexOfPreferred >= maImplContextVector.size())
        {
            nIndexOfPreferred = maImplContextVector.size() - 1;
        }

        // Take out the most valuable one
        const std::vector< SvXMLImportContextRef >::iterator aRemove(maImplContextVector.begin() + nIndexOfPreferred);
        pContext = *aRemove;
        maImplContextVector.erase(aRemove);

        // remove the rest from parent
        for(std::vector<SvXMLImportContextRef>::size_type a = 0; a < maImplContextVector.size(); a++)
        {
            SvXMLImportContext& rCandidate = *maImplContextVector[a];

            removeGraphicFromImportContext(rCandidate);
        }
        // re-insert it so that solveMultipleImages is idempotent
        maImplContextVector.clear();
        maImplContextVector.push_back(pContext);
    }
    else if (maImplContextVector.size() == 1)
    {
        pContext = maImplContextVector.front();
    }

    return pContext;
}

void MultiImageImportHelper::addContent(const SvXMLImportContext& rSvXMLImportContext)
{
    maImplContextVector.emplace_back(const_cast< SvXMLImportContext* >(&rSvXMLImportContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
