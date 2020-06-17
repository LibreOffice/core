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

#include <svx/gallerybinaryengine.hxx>
#include <svx/galmisc.hxx>
#include <galobj.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>

static bool FileExists(const INetURLObject& rURL, const OUString& rExt)
{
    INetURLObject aURL(rURL);
    aURL.setExtension(rExt);
    return FileExists(aURL);
}

INetURLObject GalleryBinaryEngine::ImplGetURLIgnoreCase(const INetURLObject& rURL)
{
    INetURLObject aURL(rURL);

    // check original file name
    if (!FileExists(aURL))
    {
        // check upper case file name
        aURL.setName(aURL.getName().toAsciiUpperCase());

        if (!FileExists(aURL))
        {
            // check lower case file name
            aURL.setName(aURL.getName().toAsciiLowerCase());
        }
    }

    return aURL;
}

void GalleryBinaryEngine::CreateUniqueURL(const INetURLObject& rBaseURL, INetURLObject& aURL)
{
    INetURLObject aBaseNoCase(ImplGetURLIgnoreCase(rBaseURL));
    aURL = aBaseNoCase;
    static sal_Int32 nIdx = 0;
    while (FileExists(aURL, "thm"))
    { // create new URLs
        nIdx++;
        aURL = aBaseNoCase;
        aURL.setName(aURL.getName() + OUString::number(nIdx));
    }
}

void GalleryBinaryEngine::SetThmExtension(INetURLObject aURL)
{
    aURL.setExtension("thm");
    aThmURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetSdgExtension(INetURLObject aURL)
{
    aURL.setExtension("sdg");
    aSdgURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetSdvExtension(INetURLObject aURL)
{
    aURL.setExtension("sdv");
    aSdvURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetStrExtension(INetURLObject aURL)
{
    aURL.setExtension("str");
    aStrURL = ImplGetURLIgnoreCase(aURL);
}

bool GalleryBinaryEngine::ImplWriteSgaObject(
    const SgaObject& rObj, sal_uInt32 nPos, GalleryObject* pExistentEntry, OUString& aDestDir,
    ::std::vector<std::unique_ptr<GalleryObject>>& aObjectList)
{
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
        GetSdgURL().GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE));
    bool bRet = false;

    if (pOStm)
    {
        const sal_uInt32 nOffset = pOStm->Seek(STREAM_SEEK_TO_END);

        rObj.WriteData(*pOStm, aDestDir);

        if (!pOStm->GetError())
        {
            GalleryObject* pEntry;

            if (!pExistentEntry)
            {
                pEntry = new GalleryObject;
                if (nPos < aObjectList.size())
                {
                    aObjectList.emplace(aObjectList.begin() + nPos, pEntry);
                }
                else
                    aObjectList.emplace_back(pEntry);
            }
            else
                pEntry = pExistentEntry;

            pEntry->aURL = rObj.GetURL();
            pEntry->nOffset = nOffset;
            pEntry->eObjKind = rObj.GetObjKind();
            bRet = true;
        }
    }

    return bRet;
}
