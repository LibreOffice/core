/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#define _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <tools/string.hxx>
#include <tools/solar.h>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace utl
{

class UNOTOOLS_DLLPUBLIC TransliterationWrapper
{
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::i18n::XExtendedTransliteration > xTrans;
    ::com::sun::star::lang::Locale aLocale;
    sal_uInt32 nType;
    sal_uInt16 nLanguage;
    mutable sal_Bool bFirstCall;

                                // not implemented, prevent usage
    TransliterationWrapper( const TransliterationWrapper& );
    TransliterationWrapper& operator=( const TransliterationWrapper& );

    void loadModuleImpl() const;
    void setLanguageLocaleImpl( sal_uInt16 nLang );

public:
    TransliterationWrapper( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                    sal_uInt32 nType );

    ~TransliterationWrapper();

    // get current Locale / Language
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale;}
    sal_uInt16 getLanguage() const { return nLanguage; }

    sal_uInt32 getType() const { return nType; }

    sal_Bool needLanguageForTheMode() const;

    /** set a new language and load the corresponding transliteration module if
        needed for the mode set with nType in the ctor */
    void loadModuleIfNeeded( sal_uInt16 nLang );

    /** Load the transliteration module specified by rModuleName, which has to
        be the UNO service implementation name that is expanded to the full UNO
        service implementation name, for example, "NumToCharKanjiShort_ja_JP"
        expands to
        "com.sun.star.i18n.Transliteration.NumToCharKanjiShort_ja_JP".
        @ATTENTION!
        This method ignores the mode type set with the constructor and
        interferes with the loadModuleIfNeeded() method and the transliterate()
        method that gets a LanguageType passed as parameter.  Using one of
        those may load a different module and overwrite this setting. Only the
        transliterate() method that takes no LanguageType parameter may be used
        for a specific module loaded with this method.  */
    void loadModuleByImplName( const String& rModuleName, sal_uInt16 nLang );

    /** This transliteration method corresponds with the loadModuleByImplName()
        method. It relies on a module being loaded and does not try load one.
        If for any reason the string can't be transliterated the original
        string is returned.  */
    String transliterate( const String& rStr,
                        xub_StrLen nStart, xub_StrLen nLen,
                        ::com::sun::star::uno::Sequence <sal_Int32>* pOffset ) const;

    // Wrapper implementations of class Transliteration
    String transliterate( const String& rStr, sal_uInt16 nLanguage,
                        xub_StrLen nStart, xub_StrLen nLen,
                        ::com::sun::star::uno::Sequence <sal_Int32>* pOffset );

    /** If two strings are equal per this transliteration.
        Returns the number of matched code points in any case, even if strings
        are not equal, for example:
        equals( "a", 0, 1, nMatch1, "aaa", 0, 3, nMatch2 )
        returns false and nMatch:=1 and nMatch2:=1
        equals( "aab", 0, 3, nMatch1, "aaa", 0, 3, nMatch2 )
        returns false and nMatch:=2 and nMatch2:=2
     */
    sal_Bool equals(
        const String& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const String& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) const;

    sal_Int32 compareSubstring(
        const String& rStr1, sal_Int32 nOff1, sal_Int32 nLen1,
        const String& rStr2, sal_Int32 nOff2, sal_Int32 nLen2 ) const;

    sal_Int32 compareString( const String& rStr1, const String& rStr2 ) const;


    // helpers

    /** If two strings are really equal as per this translation, and not just
        one string is matching the start of the other. Use this method instead
        of compareString()==0 because it is much faster.
     */
    sal_Bool isEqual( const String& rStr1, const String& rStr2 ) const;

    /** If string rStr1 matches the start of string rStr2, i.e. "a" in "aaa"
     */
    sal_Bool isMatch( const String& rStr1, const String& rStr2 ) const;

};

// ............................................................................
}       // namespace utl
// ............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
