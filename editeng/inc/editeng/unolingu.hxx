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

#ifndef _UNO_LINGU_HXX
#define _UNO_LINGU_HXX

#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "editeng/editengdllapi.h"

class LinguMgrExitLstnr;

class Window;

///////////////////////////////////////////////////////////////////////////

class EDITENG_DLLPUBLIC LinguMgr
{
    friend class LinguMgrExitLstnr;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceManager2 > xLngSvcMgr;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpell;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyph;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     xThes;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > xDicList;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         xProp;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xIgnoreAll;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xChangeAll;

    static LinguMgrExitLstnr                           *pExitLstnr;
    static sal_Bool                                     bExiting;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > GetSpell();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    GetHyph();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     GetThes();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > GetDicList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetProp();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetStandard();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetIgnoreAll();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetChangeAll();

    // disallow access to copy-constructor and assignment-operator
    LinguMgr(const LinguMgr &);
    LinguMgr & operator = (const LinguMgr &);

public:

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > GetSpellChecker();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    GetHyphenator();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     GetThesaurus();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > GetDictionaryList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetLinguPropertySet();

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceManager2 > GetLngSvcMgr();

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetStandardDic();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetIgnoreAllList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetChangeAllList();
};

///////////////////////////////////////////////////////////////////////////

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XHyphenatedWord;
}}}}


struct SvxAlternativeSpelling
{
    String      aReplacement;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >    xHyphWord;
    sal_Int16       nChangedPos,
                 nChangedLength;
    sal_Bool        bIsAltSpelling;

    inline SvxAlternativeSpelling();
};

inline SvxAlternativeSpelling::SvxAlternativeSpelling() :
    nChangedPos(-1), nChangedLength(-1), bIsAltSpelling(sal_False)
{
}


EDITENG_DLLPUBLIC SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord );


///////////////////////////////////////////////////////////////////////////

class EDITENG_DLLPUBLIC SvxDicListChgClamp
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >    xDicList;

    // disallow access to copy-constructor and assignment-operator
    EDITENG_DLLPRIVATE SvxDicListChgClamp(const SvxDicListChgClamp &);
    EDITENG_DLLPRIVATE SvxDicListChgClamp & operator = (const SvxDicListChgClamp &);

public:
    SvxDicListChgClamp( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >  &rxDicList );
    ~SvxDicListChgClamp();
};

///////////////////////////////////////////////////////////////////////////

//TODO: remove those functions or make them inline
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellChecker1 > SvxGetSpellChecker();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenator >    SvxGetHyphenator();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XThesaurus >     SvxGetThesaurus();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionaryList > SvxGetDictionaryList();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >         SvxGetLinguPropertySet();
//TODO: remove argument or provide SvxGetIgnoreAllList with the same one
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetOrCreatePosDic(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryList >  xDicList );
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetIgnoreAllList();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetChangeAllList();

///////////////////////////////////////////////////////////////////////////
// misc functions
//

EDITENG_DLLPUBLIC LanguageType                      SvxLocaleToLanguage(
        const ::com::sun::star::lang::Locale& rLocale );
EDITENG_DLLPUBLIC ::com::sun::star::lang::Locale&   SvxLanguageToLocale(
        ::com::sun::star::lang::Locale& rLocale, LanguageType eLang );
EDITENG_DLLPUBLIC ::com::sun::star::lang::Locale    SvxCreateLocale( LanguageType eLang );


EDITENG_DLLPUBLIC short SvxDicError( Window *pParent, sal_Int16 nError );


#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
