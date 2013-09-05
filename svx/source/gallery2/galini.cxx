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

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <svx/gallery1.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/svapp.hxx>

OUString GalleryThemeEntry::ReadStrFromIni(const OUString &aKeyName )
{
    SvStream *pStrm = ::utl::UcbStreamHelper::CreateStream(
                                GetStrURL().GetMainURL( INetURLObject::NO_DECODE ),
                                STREAM_READ );

    const LanguageTag &rLangTag = Application::GetSettings().GetUILanguageTag();

    ::std::vector< OUString > aFallbacks = rLangTag.getFallbackStrings( true);

    OUString aResult;
    sal_Int32 nRank = 42;

    if( pStrm )
    {
        OString aLine;
        while( pStrm->ReadLine( aLine ) )
        {
            OUString aKey;
            OUString aLocale;
            OUString aValue;
            sal_Int32 n;

            // comments
            if( aLine.indexOf( '#' ) == 0 )
                continue;

            // a[en_US] = Bob
            if( ( n = aLine.indexOf( '=' ) ) >= 1)
            {
                aKey = OStringToOUString(
                    aLine.copy( 0, n ).trim(), RTL_TEXTENCODING_ASCII_US );
                aValue = OStringToOUString(
                    aLine.copy( n + 1 ).trim(), RTL_TEXTENCODING_UTF8 );

                if( ( n = aKey.indexOf( '[' ) ) >= 1 )
                {
                    aLocale = aKey.copy( n + 1 ).trim();
                    aKey = aKey.copy( 0, n ).trim();
                    if( (n = aLocale.indexOf( ']' ) ) >= 1 )
                        aLocale = aLocale.copy( 0, n ).trim();
                }
            }
            SAL_INFO("svx", "ini file has '" << aKey << "' [ '" << aLocale << "' ] = '" << aValue << "'");

            // grisly language matching, is this not available somewhere else?
            if( aKey == aKeyName )
            {
                /* FIXME-BCP47: what is this supposed to do? */
                n = 0;
                OUString aLang = aLocale.replace('_','-');
                for( std::vector< OUString >::const_iterator i = aFallbacks.begin();
                     i != aFallbacks.end(); ++i, ++n )
                {
                    SAL_INFO( "svx", "compare '" << aLang << "' with '" << *i << "' rank " << nRank << " vs. " << n );
                    if( *i == aLang && n < nRank ) {
                        nRank = n; // try to get the most accurate match
                        aResult = aValue;
                    }
                }
            }
        }
        delete pStrm;
    }

    SAL_INFO( "svx", "readStrFromIni returns '" << aResult << "'");
    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
