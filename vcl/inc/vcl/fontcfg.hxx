/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontcfg.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _VCL_FONTCFG_HXX
#define _VCL_FONTCFG_HXX

#include "dllapi.h"
#include <tools/string.hxx>
#ifndef _VCL_ENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <hash_map>
#include <hash_set>
#include <vector>

namespace com {
namespace sun {
namespace star {
namespace lang {

// equality operator needed for hash_map;
// (-> why does this need to be in the namespace of Locale ? g++ fails to compile else)
inline bool operator==( const com::sun::star::lang::Locale& rLeft, const com::sun::star::lang::Locale& rRight )
{
    return
        rLeft.Language.equals( rRight.Language ) &&
        rLeft.Country.equals( rRight.Country )  &&
        rLeft.Variant.equals( rRight.Variant )
        ;
}
}}}}

namespace vcl
{

struct LocaleHash
{
    size_t operator()( const com::sun::star::lang::Locale& rLocale ) const
    {
        return
            (size_t)rLocale.Language.hashCode() ^
            (size_t)rLocale.Country.hashCode()  ^
            (size_t)rLocale.Variant.hashCode();
    }
};

class DefaultFontConfiguration
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
            m_xConfigProvider;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
            m_xConfigAccess;

    struct LocaleAccess
    {
        // the real string used in the configuration
        // used to get rid of upper/lower case problems
        rtl::OUString aConfigLocaleString;
        // xAccess is mutable to be able to be filled on demand
        mutable com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xAccess;
    };

    std::hash_map< com::sun::star::lang::Locale,
                   LocaleAccess,
                   vcl::LocaleHash >
            m_aConfig;

    rtl::OUString tryLocale( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rType ) const;

    DefaultFontConfiguration();
    public:
    ~DefaultFontConfiguration();

    static DefaultFontConfiguration* get();

    rtl::OUString getDefaultFont( const com::sun::star::lang::Locale& rLocale, int nType ) const;
    rtl::OUString getUserInterfaceFont( const com::sun::star::lang::Locale& rLocale ) const;
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
#define IMPL_FONT_ATTR_DEFAULT       ((ULONG)0x00000001)
#define IMPL_FONT_ATTR_STANDARD      ((ULONG)0x00000002)
#define IMPL_FONT_ATTR_NORMAL        ((ULONG)0x00000004)
#define IMPL_FONT_ATTR_SYMBOL        ((ULONG)0x00000008)
#define IMPL_FONT_ATTR_FIXED         ((ULONG)0x00000010)
#define IMPL_FONT_ATTR_SANSSERIF     ((ULONG)0x00000020)
#define IMPL_FONT_ATTR_SERIF         ((ULONG)0x00000040)
#define IMPL_FONT_ATTR_DECORATIVE    ((ULONG)0x00000080)
#define IMPL_FONT_ATTR_SPECIAL       ((ULONG)0x00000100)
#define IMPL_FONT_ATTR_ITALIC        ((ULONG)0x00000200)
#define IMPL_FONT_ATTR_TITLING       ((ULONG)0x00000400)
#define IMPL_FONT_ATTR_CAPITALS      ((ULONG)0x00000800)
#define IMPL_FONT_ATTR_CJK           ((ULONG)0x00001000)
#define IMPL_FONT_ATTR_CJK_JP        ((ULONG)0x00002000)
#define IMPL_FONT_ATTR_CJK_SC        ((ULONG)0x00004000)
#define IMPL_FONT_ATTR_CJK_TC        ((ULONG)0x00008000)
#define IMPL_FONT_ATTR_CJK_KR        ((ULONG)0x00010000)
#define IMPL_FONT_ATTR_CTL           ((ULONG)0x00020000)
#define IMPL_FONT_ATTR_NONELATIN     ((ULONG)0x00040000)
#define IMPL_FONT_ATTR_FULL          ((ULONG)0x00080000)
#define IMPL_FONT_ATTR_OUTLINE       ((ULONG)0x00100000)
#define IMPL_FONT_ATTR_SHADOW        ((ULONG)0x00200000)
#define IMPL_FONT_ATTR_ROUNDED       ((ULONG)0x00400000)
#define IMPL_FONT_ATTR_TYPEWRITER    ((ULONG)0x00800000)
#define IMPL_FONT_ATTR_SCRIPT        ((ULONG)0x01000000)
#define IMPL_FONT_ATTR_HANDWRITING   ((ULONG)0x02000000)
#define IMPL_FONT_ATTR_CHANCERY      ((ULONG)0x04000000)
#define IMPL_FONT_ATTR_COMIC         ((ULONG)0x08000000)
#define IMPL_FONT_ATTR_BRUSHSCRIPT   ((ULONG)0x10000000)
#define IMPL_FONT_ATTR_GOTHIC        ((ULONG)0x20000000)
#define IMPL_FONT_ATTR_SCHOOLBOOK    ((ULONG)0x40000000)
#define IMPL_FONT_ATTR_OTHERSTYLE    ((ULONG)0x80000000)

#define IMPL_FONT_ATTR_CJK_ALLLANG   (IMPL_FONT_ATTR_CJK_JP | IMPL_FONT_ATTR_CJK_SC | IMPL_FONT_ATTR_CJK_TC | IMPL_FONT_ATTR_CJK_KR)
#define IMPL_FONT_ATTR_ALLSCRIPT     (IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSUBSCRIPT  (IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSERIFSTYLE (IMPL_FONT_ATTR_ALLSCRIPT |\
                                      IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_SERIF |\
                                      IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_ITALIC |\
                                      IMPL_FONT_ATTR_GOTHIC | IMPL_FONT_ATTR_SCHOOLBOOK |\
                                      IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_OUTLINE)

struct FontNameAttr
{
    String                              Name;
    ::std::vector< String >             Substitutions;
    ::std::vector< String >             MSSubstitutions;
    ::std::vector< String >             PSSubstitutions;
    ::std::vector< String >             HTMLSubstitutions;
    FontWeight                          Weight;
    FontWidth                           Width;
    unsigned long                       Type; // bitfield of IMPL_FONT_ATTR_*
};

class VCL_DLLPUBLIC FontSubstConfiguration
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
            m_xConfigProvider;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
            m_xConfigAccess;
    struct LocaleSubst
    {
        rtl::OUString                           aConfigLocaleString;
        mutable bool                            bConfigRead;
        // note: aSubstAttributes must be sorted alphabetically by Name
        // searches on the substitutes are done with Name as key, where
        // a minimal match is sufficient (that is e.g. "Thorndale" will match
        // "Thorndale BlaBlub"). Also names must be lower case.
        mutable std::vector< FontNameAttr >     aSubstAttributes;

        LocaleSubst() : bConfigRead( false ) {}
    };
    std::hash_map< com::sun::star::lang::Locale, LocaleSubst, vcl::LocaleHash > m_aSubst;
    typedef std::hash_set< rtl::OUString, rtl::OUStringHash > UniqueSubstHash;
    mutable UniqueSubstHash maSubstHash;


    void fillSubstVector( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                          const rtl::OUString& rType,
                          std::vector< String >& rSubstVector ) const;
    FontWeight getSubstWeight( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                          const rtl::OUString& rType ) const;
    FontWidth getSubstWidth( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                             const rtl::OUString& rType ) const;
    unsigned long getSubstType( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xFont,
                                const rtl::OUString& rType ) const;
    void readLocaleSubst( const com::sun::star::lang::Locale& rLocale ) const;
    FontSubstConfiguration();
public:
    ~FontSubstConfiguration();

    static FontSubstConfiguration* get();

    const FontNameAttr* getSubstInfo(
                                     const String& rFontName,
                                     const com::sun::star::lang::Locale& rLocale =
                                     com::sun::star::lang::Locale( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) ),
                                                                   rtl::OUString(),
                                                                   rtl::OUString() )
                                     ) const;
    static void getMapName( const String& rOrgName, String& rShortName, String& rFamilyName, FontWeight& rWeight, FontWidth& rWidth, ULONG& rType );
};

} // namespace vcl

#endif // _VCL_FONTCFG_HXX
