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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#define INCLUDED_UNOTOOLS_TRANSLITERATIONWRAPPER_HXX

#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

namespace utl
{

class UNOTOOLS_DLLPUBLIC TransliterationWrapper
{
    css::uno::Reference< css::i18n::XExtendedTransliteration > xTrans;
    LanguageTag aLanguageTag;
    sal_uInt32 nType;
    mutable bool bFirstCall;

    TransliterationWrapper( const TransliterationWrapper& ) = delete;
    TransliterationWrapper& operator=( const TransliterationWrapper& ) = delete;

    void loadModuleImpl() const;
    void setLanguageLocaleImpl( sal_uInt16 nLang );

public:
    TransliterationWrapper( const css::uno::Reference< css::uno::XComponentContext > & rxContext,
                    sal_uInt32 nType );

    ~TransliterationWrapper();

    sal_uInt32 getType() const { return nType; }

    bool needLanguageForTheMode() const;

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
    void loadModuleByImplName( const OUString& rModuleName, sal_uInt16 nLang );

    /** This transliteration method corresponds with the loadModuleByImplName()
        method. It relies on a module being loaded and does not try load one.
        If for any reason the string can't be transliterated the original
        string is returned.  */
    OUString transliterate( const OUString& rStr,
                        sal_Int32 nStart, sal_Int32 nLen,
                        css::uno::Sequence <sal_Int32>* pOffset ) const;

    // Wrapper implementations of class Transliteration
    OUString transliterate( const OUString& rStr, sal_uInt16 nLanguage,
                        sal_Int32 nStart, sal_Int32 nLen,
                        css::uno::Sequence <sal_Int32>* pOffset );

    /** If two strings are equal per this transliteration.
        Returns the number of matched code points in any case, even if strings
        are not equal, for example:
        equals( "a", 0, 1, nMatch1, "aaa", 0, 3, nMatch2 )
        returns false and nMatch:=1 and nMatch2:=1
        equals( "aab", 0, 3, nMatch1, "aaa", 0, 3, nMatch2 )
        returns false and nMatch:=2 and nMatch2:=2
     */
    bool equals(
        const OUString& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const OUString& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) const;

    sal_Int32 compareString( const OUString& rStr1, const OUString& rStr2 ) const;

    // helpers

    /** If two strings are really equal as per this translation, and not just
        one string is matching the start of the other. Use this method instead
        of compareString()==0 because it is much faster.
     */
    bool isEqual( const OUString& rStr1, const OUString& rStr2 ) const;

    /** If string rStr1 matches the start of string rStr2, i.e. "a" in "aaa"
     */
    bool isMatch( const OUString& rStr1, const OUString& rStr2 ) const;

};

}       // namespace utl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
