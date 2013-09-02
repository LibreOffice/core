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

#include <xmloff/xmlmultiimagehelper.hxx>
#include <rtl/ustring.hxx>

using namespace ::com::sun::star;

namespace
{
    sal_uInt32 getQualityIndex(const OUString& rString)
    {
        sal_uInt32 nRetval(0);

        // pixel formats first
        if(rString.endsWithAsciiL(".bmp", 4))
        {
            return 10;
        }
        if(rString.endsWithAsciiL(".gif", 4))
        {
            return 20;
        }
        if(rString.endsWithAsciiL(".jpg", 4))
        {
            return 30;
        }
        if(rString.endsWithAsciiL(".png", 4))
        {
            return 40;
        }

        // vector formats, prefer always
        if(rString.endsWithAsciiL(".svm", 4))
        {
            return 1000;
        }
        if(rString.endsWithAsciiL(".wmf", 4))
        {
            return 1010;
        }
        if(rString.endsWithAsciiL(".emf", 4))
        {
            return 1020;
        }
        else if(rString.endsWithAsciiL(".svg", 4))
        {
            return 1030;
        }

        return nRetval;
    }
}

MultiImageImportHelper::MultiImageImportHelper()
:   maImplContextVector(),
    mbSupportsMultipleContents(false)
{
}

MultiImageImportHelper::~MultiImageImportHelper()
{
    while(!maImplContextVector.empty())
    {
        delete *(maImplContextVector.end() - 1);
        maImplContextVector.pop_back();
    }
}

SvXMLImportContextRef MultiImageImportHelper::solveMultipleImages()
{
    SvXMLImportContextRef pContext;
    if(maImplContextVector.size() > 1)
    {
        // multiple child contexts were imported, decide which is the most valuable one
        // and remove the rest
        sal_uInt32 nIndexOfPreferred(maImplContextVector.size());
        sal_uInt32 nBestQuality(0), a(0);

        for(a = 0; a < maImplContextVector.size(); a++)
        {
            const OUString aStreamURL(getGraphicURLFromImportContext(**maImplContextVector[a]));
            const sal_uInt32 nNewQuality(getQualityIndex(aStreamURL));

            if(nNewQuality > nBestQuality)
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
        const std::vector< SvXMLImportContextRef* >::iterator aRemove(maImplContextVector.begin() + nIndexOfPreferred);
        pContext = **aRemove;
        delete *aRemove;
        maImplContextVector.erase(aRemove);

        // remove the rest from parent
        for(a = 0; a < maImplContextVector.size(); a++)
        {
            removeGraphicFromImportContext(**maImplContextVector[a]);
        }
    }
    else if (maImplContextVector.size() == 1)
    {
        pContext = *maImplContextVector.front();
    }

    return pContext;
}

void MultiImageImportHelper::addContent(const SvXMLImportContext& rSvXMLImportContext)
{
    if(dynamic_cast< const SvXMLImportContext* >(&rSvXMLImportContext))
    {
        maImplContextVector.push_back(new SvXMLImportContextRef(const_cast< SvXMLImportContext* >(&rSvXMLImportContext)));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
