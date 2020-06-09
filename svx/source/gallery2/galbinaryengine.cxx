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

#include <svx/galbinaryengine.hxx>

#include <memory>

GalleryBinaryEngine::GalleryBinaryEngine(bool bCreateUniqueURL, const INetURLObject& rBaseURL)
{
    INetURLObject aURL(rBaseURL);
    DBG_ASSERT(aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    if (bCreateUniqueURL)
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

    aURL.setExtension("thm");
    aThmURL = ImplGetURLIgnoreCase(aURL);

    aURL.setExtension("sdg");
    aSdgURL = ImplGetURLIgnoreCase(aURL);

    aURL.setExtension("sdv");
    aSdvURL = ImplGetURLIgnoreCase(aURL);

    aURL.setExtension("str");
    aStrURL = ImplGetURLIgnoreCase(aURL);
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

OUString GalleryBinaryEngine::ReadStrFromIni(const OUString& aKeyName)
{
    std::unique_ptr<SvStream> pStrm(::utl::UcbStreamHelper::CreateStream(
        GetStrURL().GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

    const LanguageTag& rLangTag = Application::GetSettings().GetUILanguageTag();

    ::std::vector<OUString> aFallbacks = rLangTag.getFallbackStrings(true);

    OUString aResult;
    sal_Int32 nRank = 42;

    if (pStrm)
    {
        OString aLine;
        while (pStrm->ReadLine(aLine))
        {
            OUString aKey;
            OUString aLocale;
            OUString aValue;
            sal_Int32 n;

            // comments
            if (aLine.startsWith("#"))
                continue;

            // a[en_US] = Bob
            if ((n = aLine.indexOf('=')) >= 1)
            {
                aKey = OStringToOUString(aLine.copy(0, n).trim(), RTL_TEXTENCODING_ASCII_US);
                aValue = OStringToOUString(aLine.copy(n + 1).trim(), RTL_TEXTENCODING_UTF8);

                if ((n = aKey.indexOf('[')) >= 1)
                {
                    aLocale = aKey.copy(n + 1).trim();
                    aKey = aKey.copy(0, n).trim();
                    if ((n = aLocale.indexOf(']')) >= 1)
                        aLocale = aLocale.copy(0, n).trim();
                }
            }
            SAL_INFO("svx",
                     "ini file has '" << aKey << "' [ '" << aLocale << "' ] = '" << aValue << "'");

            // grisly language matching, is this not available somewhere else?
            if (aKey == aKeyName)
            {
                /* FIXME-BCP47: what is this supposed to do? */
                n = 0;
                OUString aLang = aLocale.replace('_', '-');
                for (const auto& rFallback : aFallbacks)
                {
                    SAL_INFO("svx", "compare '" << aLang << "' with '" << rFallback << "' rank "
                                                << nRank << " vs. " << n);
                    if (rFallback == aLang && n < nRank)
                    {
                        nRank = n; // try to get the most accurate match
                        aResult = aValue;
                    }
                    ++n;
                }
            }
        }
    }

    SAL_INFO("svx", "readStrFromIni returns '" << aResult << "'");
    return aResult;
}
