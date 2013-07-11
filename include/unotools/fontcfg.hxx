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
#ifndef _UNOTOOLS_FONTCFG_HXX
#define _UNOTOOLS_FONTCFG_HXX

#include <i18nlangtag/languagetag.hxx>
#include <unotools/unotoolsdllapi.h>
#include <tools/solar.h>
#include <tools/fontenum.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <vector>

namespace utl
{

class UNOTOOLS_DLLPUBLIC DefaultFontConfiguration
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
            m_xConfigProvider;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
            m_xConfigAccess;

    struct LocaleAccess
    {
        // the real string used in the configuration
        // used to get rid of upper/lower case problems
        OUString aConfigLocaleString;
        // xAccess is mutable to be able to be filled on demand
        mutable com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xAccess;
    };

    boost::unordered_map< OUString, LocaleAccess, OUStringHash > m_aConfig;

    OUString tryLocale( const OUString& rBcp47, const OUString& rType ) const;

    public:
    DefaultFontConfiguration();
    ~DefaultFontConfiguration();

    static DefaultFontConfiguration& get();

    OUString getDefaultFont( const LanguageTag& rLanguageTag, int nType ) const;
    OUString getUserInterfaceFont( const LanguageTag& rLanguageTag ) const;
};

// IMPL_FONT_ATTR_DEFAULT       - Default-Font like Andale Sans UI, Palace Script, Albany, Thorndale, Cumberland, ...
// IMPL_FONT_ATTR_STANDARD      - Standard-Font like Arial, Times, Courier, ...
// IMPL_FONT_ATTR_NORMAL        - normal Font for writing text like Arial, Verdana, Arial Narrow, Trebuchet, Times, Courier, ...
// IMPL_FONT_ATTR_SYMBOL        - Font with symbols
// IMPL_FONT_ATTR_DECORATIVE    - Readable and normally used for drawings
// IMPL_FONT_ATTR_SPECIAL       - very special design
// IMPL_FONT_ATTR_TITLING       - only uppercase characters
// IMPL_FONT_ATTR_FULL          - Font with normally all characters
// IMPL_FONT_ATTR_CAPITALS     - only uppercase characters, but lowercase characters smaller as the uppercase characters
// IMPL_FONT_ATTR_TYPEWRITER    - like a typewriter: Courier, ...
// IMPL_FONT_ATTR_SCRIPT        - Handwriting or Script
// IMPL_FONT_ATTR_HANDWRITING   - More Handwriting with normal letters
// IMPL_FONT_ATTR_CHANCERY      - Like Zapf Chancery
// IMPL_FONT_ATTR_COMIC         - Like Comic Sans MS
// IMPL_FONT_ATTR_BRUSHSCRIPT   - More Script
// IMPL_FONT_ATTR_OTHERSTYLE    - OldStyle, ... so negativ points
#define IMPL_FONT_ATTR_DEFAULT       ((sal_uLong)0x00000001)
#define IMPL_FONT_ATTR_STANDARD      ((sal_uLong)0x00000002)
#define IMPL_FONT_ATTR_NORMAL        ((sal_uLong)0x00000004)
#define IMPL_FONT_ATTR_SYMBOL        ((sal_uLong)0x00000008)
#define IMPL_FONT_ATTR_FIXED         ((sal_uLong)0x00000010)
#define IMPL_FONT_ATTR_SANSSERIF     ((sal_uLong)0x00000020)
#define IMPL_FONT_ATTR_SERIF         ((sal_uLong)0x00000040)
#define IMPL_FONT_ATTR_DECORATIVE    ((sal_uLong)0x00000080)
#define IMPL_FONT_ATTR_SPECIAL       ((sal_uLong)0x00000100)
#define IMPL_FONT_ATTR_ITALIC        ((sal_uLong)0x00000200)
#define IMPL_FONT_ATTR_TITLING       ((sal_uLong)0x00000400)
#define IMPL_FONT_ATTR_CAPITALS      ((sal_uLong)0x00000800)
#define IMPL_FONT_ATTR_CJK           ((sal_uLong)0x00001000)
#define IMPL_FONT_ATTR_CJK_JP        ((sal_uLong)0x00002000)
#define IMPL_FONT_ATTR_CJK_SC        ((sal_uLong)0x00004000)
#define IMPL_FONT_ATTR_CJK_TC        ((sal_uLong)0x00008000)
#define IMPL_FONT_ATTR_CJK_KR        ((sal_uLong)0x00010000)
#define IMPL_FONT_ATTR_CTL           ((sal_uLong)0x00020000)
#define IMPL_FONT_ATTR_NONELATIN     ((sal_uLong)0x00040000)
#define IMPL_FONT_ATTR_FULL          ((sal_uLong)0x00080000)
#define IMPL_FONT_ATTR_OUTLINE       ((sal_uLong)0x00100000)
#define IMPL_FONT_ATTR_SHADOW        ((sal_uLong)0x00200000)
#define IMPL_FONT_ATTR_ROUNDED       ((sal_uLong)0x00400000)
#define IMPL_FONT_ATTR_TYPEWRITER    ((sal_uLong)0x00800000)
#define IMPL_FONT_ATTR_SCRIPT        ((sal_uLong)0x01000000)
#define IMPL_FONT_ATTR_HANDWRITING   ((sal_uLong)0x02000000)
#define IMPL_FONT_ATTR_CHANCERY      ((sal_uLong)0x04000000)
#define IMPL_FONT_ATTR_COMIC         ((sal_uLong)0x08000000)
#define IMPL_FONT_ATTR_BRUSHSCRIPT   ((sal_uLong)0x10000000)
#define IMPL_FONT_ATTR_GOTHIC        ((sal_uLong)0x20000000)
#define IMPL_FONT_ATTR_SCHOOLBOOK    ((sal_uLong)0x40000000)
#define IMPL_FONT_ATTR_OTHERSTYLE    ((sal_uLong)0x80000000)

#define IMPL_FONT_ATTR_CJK_ALLLANG   (IMPL_FONT_ATTR_CJK_JP | IMPL_FONT_ATTR_CJK_SC | IMPL_FONT_ATTR_CJK_TC | IMPL_FONT_ATTR_CJK_KR)
#define IMPL_FONT_ATTR_ALLSCRIPT     (IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSUBSCRIPT  (IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSERIFSTYLE (IMPL_FONT_ATTR_ALLSCRIPT |\
                                      IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_SERIF |\
                                      IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_ITALIC |\
                                      IMPL_FONT_ATTR_GOTHIC | IMPL_FONT_ATTR_SCHOOLBOOK |\
                                      IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_OUTLINE)

struct UNOTOOLS_DLLPUBLIC FontNameAttr
{
    OUString                            Name;
    ::std::vector< OUString >             Substitutions;
    ::std::vector< OUString >             MSSubstitutions;
    ::std::vector< OUString >             PSSubstitutions;
    ::std::vector< OUString >             HTMLSubstitutions;
    FontWeight                          Weight;
    FontWidth                           Width;
    unsigned long                       Type; // bitfield of IMPL_FONT_ATTR_*
};

class UNOTOOLS_DLLPUBLIC FontSubstConfiguration
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
            m_xConfigProvider;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
            m_xConfigAccess;
    struct LocaleSubst
    {
        OUString                           aConfigLocaleString;
        mutable bool                            bConfigRead;
        // note: aSubstAttributes must be sorted alphabetically by Name
        // searches on the substitutes are done with Name as key, where
        // a minimal match is sufficient (that is e.g. "Thorndale" will match
        // "Thorndale BlaBlub"). Also names must be lower case.
        mutable std::vector< FontNameAttr >     aSubstAttributes;

        LocaleSubst() : bConfigRead( false ) {}
    };
    boost::unordered_map< OUString, LocaleSubst, OUStringHash > m_aSubst;
    typedef boost::unordered_set< OUString, OUStringHash > UniqueSubstHash;
    mutable UniqueSubstHash maSubstHash;


    void fillSubstVector( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                          const OUString& rType,
                          std::vector< OUString >& rSubstVector ) const;
    FontWeight getSubstWeight( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                          const OUString& rType ) const;
    FontWidth getSubstWidth( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                             const OUString& rType ) const;
    unsigned long getSubstType( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                                const OUString& rType ) const;
    void readLocaleSubst( const OUString& rBcp47 ) const;
public:
    FontSubstConfiguration();
    ~FontSubstConfiguration();

    static FontSubstConfiguration& get();

    const FontNameAttr* getSubstInfo(
                                     const OUString& rFontName,
                                     const LanguageTag& rLanguageTag = LanguageTag( OUString( "en"))
                                     ) const;
    static void getMapName( const OUString& rOrgName, OUString& rShortName, OUString& rFamilyName, FontWeight& rWeight, FontWidth& rWidth, sal_uLong& rType );
};

} // namespace utl

#endif // _UNOTOOLS_FONTCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
