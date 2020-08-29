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

#include <svx/gallerybinaryengineentry.hxx>
#include <svx/galmisc.hxx>
#include <svx/unomodel.hxx>
#include <svx/fmmodel.hxx>
#include <svx/gallery1.hxx>
#include "codec.hxx"
#include "gallerydrawmodel.hxx"

#include <sal/log.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>

#include <com/sun/star/ucb/ContentCreationException.hpp>

static bool FileExists(const INetURLObject& rURL, const OUString& rExt)
{
    INetURLObject aURL(rURL);
    aURL.setExtension(rExt);
    return FileExists(aURL);
}

GalleryBinaryEngineEntry::GalleryBinaryEngineEntry()
{
    mpGalleryStorageLocations = std::make_unique<GalleryBinaryStorageLocations>();
}

void GalleryBinaryEngineEntry::setStorageLocations(INetURLObject & rURL)
{
    mpGalleryStorageLocations->SetStorageLocations(rURL);
}

std::unique_ptr<GalleryBinaryEngine> GalleryBinaryEngineEntry::createGalleryStorageEngine(
    GalleryObjectCollection& mrGalleryObjectCollection, bool& bReadOnly)
{
    return std::make_unique<GalleryBinaryEngine>(*mpGalleryStorageLocations,
                                                 mrGalleryObjectCollection, bReadOnly);
}

void GalleryBinaryEngineEntry::CreateUniqueURL(const INetURLObject& rBaseURL, INetURLObject& aURL)
{
    INetURLObject aBaseNoCase(GalleryBinaryStorageLocations::ImplGetURLIgnoreCase(rBaseURL));
    aURL = aBaseNoCase;
    static sal_Int32 nIdx = 0;
    while (FileExists(aURL, "thm"))
    { // create new URLs
        nIdx++;
        aURL = aBaseNoCase;
        aURL.setName(aURL.getName() + OUString::number(nIdx));
    }
}

GalleryThemeEntry* GalleryBinaryEngineEntry::CreateThemeEntry(const INetURLObject& rURL,
                                                              bool bReadOnly)
{
    DBG_ASSERT(rURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    GalleryThemeEntry* pRet = nullptr;

    if (FileExists(rURL))
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
            rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

        if (pIStm)
        {
            OUString aThemeName;
            sal_uInt16 nVersion;

            pIStm->ReadUInt16(nVersion);

            if (nVersion <= 0x00ff)
            {
                bool bThemeNameFromResource = false;
                sal_uInt32 nThemeId = 0;

                OString aTmpStr = read_uInt16_lenPrefixed_uInt8s_ToOString(*pIStm);
                aThemeName = OStringToOUString(aTmpStr, RTL_TEXTENCODING_UTF8);

                // execute a character conversion
                if (nVersion >= 0x0004)
                {
                    sal_uInt32 nCount;
                    sal_uInt16 nTemp16;

                    pIStm->ReadUInt32(nCount).ReadUInt16(nTemp16);
                    pIStm->Seek(STREAM_SEEK_TO_END);

                    // check whether there is a newer version;
                    // therefore jump back by 520Bytes (8 bytes ID + 512Bytes reserve buffer)
                    // if this is at all possible.
                    if (pIStm->Tell() >= 520)
                    {
                        sal_uInt32 nId1, nId2;

                        pIStm->SeekRel(-520);
                        pIStm->ReadUInt32(nId1).ReadUInt32(nId2);

                        if (nId1 == COMPAT_FORMAT('G', 'A', 'L', 'R')
                            && nId2 == COMPAT_FORMAT('E', 'S', 'R', 'V'))
                        {
                            VersionCompat aCompat(*pIStm, StreamMode::READ);

                            pIStm->ReadUInt32(nThemeId);

                            if (aCompat.GetVersion() >= 2)
                            {
                                pIStm->ReadCharAsBool(bThemeNameFromResource);
                            }
                        }
                    }
                }

                pRet = new GalleryThemeEntry(false, rURL, aThemeName, bReadOnly, false, nThemeId,
                                             bThemeNameFromResource);
            }
        }
    }

    return pRet;
}

void GalleryBinaryEngineEntry::removeTheme()
{
    INetURLObject aThmURL(GetThmURL());
    INetURLObject aSdgURL(GetSdgURL());
    INetURLObject aSdvURL(GetSdvURL());
    INetURLObject aStrURL(GetStrURL());

    KillFile(aThmURL);
    KillFile(aSdgURL);
    KillFile(aSdvURL);
    KillFile(aStrURL);
}

std::unique_ptr<GalleryTheme>&
GalleryBinaryEngineEntry::getCachedTheme(std::unique_ptr<GalleryTheme>& pNewTheme)
{
    INetURLObject aURL = GetThmURL();

    DBG_ASSERT(aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    if (FileExists(aURL))
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
            aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

        if (pIStm)
        {
            try
            {
                ReadGalleryTheme(*pIStm, *pNewTheme);

                if (pIStm->GetError())
                    pNewTheme.reset();
            }
            catch (const css::ucb::ContentCreationException&)
            {
            }
        }
    }
    return pNewTheme;
}

SvStream& ReadGalleryTheme(SvStream& rIn, GalleryTheme& rTheme) { return rTheme.ReadData(rIn); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
