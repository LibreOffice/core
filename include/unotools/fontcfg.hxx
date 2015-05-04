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
#ifndef INCLUDED_UNOTOOLS_FONTCFG_HXX
#define INCLUDED_UNOTOOLS_FONTCFG_HXX

#include <i18nlangtag/languagetag.hxx>
#include <unotools/unotoolsdllapi.h>
#include <tools/solar.h>
#include <tools/fontenum.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <o3tl/typed_flags_set.hxx>

#include <unordered_map>
#include <unordered_set>
#include <vector>

// DEFAULT       - Default-Font like Andale Sans UI, Palace Script, Albany, Thorndale, Cumberland, ...
// STANDARD      - Standard-Font like Arial, Times, Courier, ...
// NORMAL        - normal Font for writing text like Arial, Verdana, Arial Narrow, Trebuchet, Times, Courier, ...
// SYMBOL        - Font with symbols
// DECORATIVE    - Readable and normally used for drawings
// SPECIAL       - very special design
// TITLING       - only uppercase characters
// FONT_ATTR_FULL          - Font with normally all characters
// CAPITALS     - only uppercase characters, but lowercase characters smaller as the uppercase characters
// TYPEWRITER    - like a typewriter: Courier, ...
// SCRIPT        - Handwriting or Script
// HANDWRITING   - More Handwriting with normal letters
// CHANCERY      - Like Zapf Chancery
// COMIC         - Like Comic Sans MS
// BRUSHSCRIPT   - More Script
// OTHERSTYLE    - OldStyle, ... so negativ points
enum class ImplFontAttrs : sal_uLong
{
    None          = 0x00000000,
    Default       = 0x00000001,
    Standard      = 0x00000002,
    Normal        = 0x00000004,
    Symbol        = 0x00000008,
    Fixed         = 0x00000010,
    SansSerif     = 0x00000020,
    Serif         = 0x00000040,
    Decorative    = 0x00000080,
    Special       = 0x00000100,
    Italic        = 0x00000200,
    Titling       = 0x00000400,
    Capitals      = 0x00000800,
    CJK           = 0x00001000,
    CJK_JP        = 0x00002000,
    CJK_SC        = 0x00004000,
    CJK_TC        = 0x00008000,
    CJK_KR        = 0x00010000,
    CTL           = 0x00020000,
    NoneLatin     = 0x00040000,
    Full          = 0x00080000,
    Outline       = 0x00100000,
    Shadow        = 0x00200000,
    Rounded       = 0x00400000,
    Typewriter    = 0x00800000,
    Script        = 0x01000000,
    Handwriting   = 0x02000000,
    Chancery      = 0x04000000,
    Comic         = 0x08000000,
    BrushScript   = 0x10000000,
    Gothic        = 0x20000000,
    Schoolbook    = 0x40000000,
    OtherStyle    = 0x80000000,
    CJK_AllLang   = CJK_JP | CJK_SC | CJK_TC | CJK_KR,
    AllScript     = Script | Handwriting | Chancery | Comic | BrushScript,
    AllSubscript  = Handwriting | Chancery | Comic | BrushScript,
    AllSerifStyle = AllScript | SansSerif | Serif | Fixed | Italic | Gothic | Schoolbook | Shadow | Outline,
};
namespace o3tl
{
    template<> struct typed_flags<ImplFontAttrs> : is_typed_flags<ImplFontAttrs, 0xffffffff> {};
}


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

    std::unordered_map< OUString, LocaleAccess, OUStringHash > m_aConfig;

    OUString tryLocale( const OUString& rBcp47, const OUString& rType ) const;

    public:
    DefaultFontConfiguration();
    ~DefaultFontConfiguration();

    static DefaultFontConfiguration& get();

    OUString getDefaultFont( const LanguageTag& rLanguageTag, int nType ) const;
    OUString getUserInterfaceFont( const LanguageTag& rLanguageTag ) const;
};

struct UNOTOOLS_DLLPUBLIC FontNameAttr
{
    OUString                            Name;
    ::std::vector< OUString >           Substitutions;
    ::std::vector< OUString >           MSSubstitutions;
    ::std::vector< OUString >           PSSubstitutions;
    ::std::vector< OUString >           HTMLSubstitutions;
    FontWeight                          Weight;
    FontWidth                           Width;
    ImplFontAttrs                       Type;
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
    std::unordered_map< OUString, LocaleSubst, OUStringHash > m_aSubst;
    typedef std::unordered_set< OUString, OUStringHash > UniqueSubstHash;
    mutable UniqueSubstHash maSubstHash;

    void fillSubstVector( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rFont,
                          const OUString& rType,
                          std::vector< OUString >& rSubstVector ) const;
    FontWeight getSubstWeight( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rFont,
                          const OUString& rType ) const;
    FontWidth getSubstWidth( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rFont,
                             const OUString& rType ) const;
    ImplFontAttrs getSubstType( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rFont,
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
    static void getMapName( const OUString& rOrgName, OUString& rShortName, OUString& rFamilyName, FontWeight& rWeight, FontWidth& rWidth, ImplFontAttrs& rType );
};

} // namespace utl

#endif // INCLUDED_UNOTOOLS_FONTCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
