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

#ifndef INCLUDED_EDITENG_UNOLINGU_HXX
#define INCLUDED_EDITENG_UNOLINGU_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <editeng/editengdllapi.h>

class LinguMgrExitLstnr;

namespace weld { class Window; }
namespace linguistic { enum class DictionaryError; }


class EDITENG_DLLPUBLIC LinguMgr
{
    friend class LinguMgrExitLstnr;

    static css::uno::Reference< css::linguistic2::XLinguServiceManager2 >     xLngSvcMgr;
    static css::uno::Reference< css::linguistic2::XSpellChecker1 >            xSpell;
    static css::uno::Reference< css::linguistic2::XHyphenator >               xHyph;
    static css::uno::Reference< css::linguistic2::XThesaurus >                xThes;
    static css::uno::Reference< css::linguistic2::XSearchableDictionaryList > xDicList;
    static css::uno::Reference< css::linguistic2::XLinguProperties >          xProp;

    static css::uno::Reference< css::linguistic2::XDictionary >               xIgnoreAll;
    static css::uno::Reference< css::linguistic2::XDictionary >               xChangeAll;

    static LinguMgrExitLstnr                           *pExitLstnr;
    static bool                                        bExiting;

    static css::uno::Reference< css::linguistic2::XSpellChecker1 >            GetSpell();
    static css::uno::Reference< css::linguistic2::XHyphenator >               GetHyph();
    static css::uno::Reference< css::linguistic2::XThesaurus >                GetThes();
    static css::uno::Reference< css::linguistic2::XSearchableDictionaryList > GetDicList();
    static css::uno::Reference< css::linguistic2::XLinguProperties >          GetProp();
    static css::uno::Reference< css::linguistic2::XDictionary >               GetStandard();
    static css::uno::Reference< css::linguistic2::XDictionary >               GetIgnoreAll();
    static css::uno::Reference< css::linguistic2::XDictionary >               GetChangeAll();

    LinguMgr(const LinguMgr &) = delete;
    LinguMgr & operator = (const LinguMgr &) = delete;

public:

    static css::uno::Reference< css::linguistic2::XSpellChecker1 >            GetSpellChecker();
    static css::uno::Reference< css::linguistic2::XHyphenator >               GetHyphenator();
    static css::uno::Reference< css::linguistic2::XThesaurus >                GetThesaurus();
    static css::uno::Reference< css::linguistic2::XSearchableDictionaryList > GetDictionaryList();
    static css::uno::Reference< css::linguistic2::XLinguProperties >          GetLinguPropertySet();

    static css::uno::Reference< css::linguistic2::XLinguServiceManager2 >     GetLngSvcMgr();

    static css::uno::Reference< css::linguistic2::XDictionary >               GetStandardDic();
    static css::uno::Reference< css::linguistic2::XDictionary >               GetIgnoreAllList();
    static css::uno::Reference< css::linguistic2::XDictionary >               GetChangeAllList();
};


namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XHyphenatedWord;
}}}}


struct SvxAlternativeSpelling
{
    OUString         aReplacement;
    sal_Int16       nChangedPos,
                    nChangedLength;
    bool            bIsAltSpelling;

    inline SvxAlternativeSpelling();
};

inline SvxAlternativeSpelling::SvxAlternativeSpelling() :
    nChangedPos(-1), nChangedLength(-1), bIsAltSpelling(false)
{
}


EDITENG_DLLPUBLIC SvxAlternativeSpelling SvxGetAltSpelling(
        const css::uno::Reference< css::linguistic2::XHyphenatedWord > & rHyphWord );


class EDITENG_DLLPUBLIC SvxDicListChgClamp
{
private:
    css::uno::Reference<
        css::linguistic2::XSearchableDictionaryList >    xDicList;

    SvxDicListChgClamp(const SvxDicListChgClamp &) = delete;
    SvxDicListChgClamp & operator = (const SvxDicListChgClamp &) = delete;

public:
    SvxDicListChgClamp( css::uno::Reference<
        css::linguistic2::XSearchableDictionaryList > const &rxDicList );
    ~SvxDicListChgClamp();
};

EDITENG_DLLPUBLIC short SvxDicError(weld::Window *pParent, linguistic::DictionaryError nError);


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
