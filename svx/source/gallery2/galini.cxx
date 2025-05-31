/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * The world's quickest and lamest .desktop / .ini file parser.
 * Ideally the .thm file would move to a .desktop file in
 * future.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <galleryfilestorageentry.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <o3tl/string_view.hxx>
#include <memory>

OUString GalleryFileStorageEntry::ReadStrFromIni(std::string_view aKeyName) const
{
    std::unique_ptr<SvStream> pStrm(::utl::UcbStreamHelper::CreateStream(
        GetStrURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                StreamMode::READ ));

    OUString aResult;

    if( pStrm )
    {
        const LanguageTag& rLangTag = Application::GetSettings().GetUILanguageTag();
        const std::vector<OUString> aFallbacks = rLangTag.getFallbackStrings(true);
        size_t nRank = aFallbacks.size();

        // Suppose aKeyName is "a"
        //         ini has " a [ en ] = Foo
        //                   a [ en_US ] = Bar "
        //         and aFallbacks is { "en-US", "en" }
        // Then we must return "Bar", because its locale has higher fallback rank.

        OStringBuffer aLineBuf;
        while (pStrm->ReadLine(aLineBuf))
        {
            std::string_view aLine(aLineBuf);

            // comments
            if (aLine.starts_with("#"))
                continue;

            size_t n = aLine.find('=');
            if (n == std::string_view::npos)
                continue;

            std::string_view aKey(o3tl::trim(aLine.substr(0, n)));
            std::string_view aValue(o3tl::trim(aLine.substr(n + 1)));

            n = aKey.find('[');
            if (n == std::string_view::npos || n < 1)
                continue;

            std::string_view aLocale = o3tl::trim(aKey.substr(n + 1, aKey.find(']', n + 2)));
            aKey = o3tl::trim(aKey.substr(0, n));

            SAL_INFO("svx", "ini file has '" << aKey << "' [ '" << aLocale << "' ] = '" << aValue << "'");

            if (aKey != aKeyName)
                continue;

            // grisly language matching, is this not available somewhere else?
            OUString aLang
                = OStringToOUString(aLocale, RTL_TEXTENCODING_ASCII_US).replace('_', '-');
            for (n = 0; n < nRank; ++n)
            {
                auto& rFallback = aFallbacks[n];
                SAL_INFO( "svx", "compare '" << aLang << "' with '" << rFallback << "' rank " << nRank << " vs. " << n );
                if (rFallback == aLang)
                {
                    nRank = n; // try to get the most accurate match
                    aResult = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);
                }
            }
            if (nRank == 0)
                break;
        }
    }

    SAL_INFO( "svx", "readStrFromIni returns '" << aResult << "'");
    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
