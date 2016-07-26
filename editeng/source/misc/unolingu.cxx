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


#include <editeng/unolingu.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/linguistic2/DictionaryList.hpp>
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/LinguProperties.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <linguistic/misc.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::comphelper;
using namespace ::linguistic;
using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;

static uno::Reference< XLinguServiceManager2 > GetLngSvcMgr_Impl()
{
    uno::Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    uno::Reference< XLinguServiceManager2 > xRes = LinguServiceManager::create(xContext);
    return xRes;
}

//! Dummy implementation in order to avoid loading of lingu DLL
//! when only the XSupportedLocales interface is used.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when "real" work needs to be done only.
class ThesDummy_Impl :
    public cppu::WeakImplHelper< XThesaurus >
{
    uno::Reference< XThesaurus >     xThes;      // the real one...
    Sequence< lang::Locale >         *pLocaleSeq;

    void GetCfgLocales();

    void GetThes_Impl();

public:
    ThesDummy_Impl() : pLocaleSeq(nullptr)  {}
    virtual ~ThesDummy_Impl();

    // XSupportedLocales
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL
        getLocales()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        hasLocale( const css::lang::Locale& rLocale )
            throw(css::uno::RuntimeException, std::exception) override;

    // XThesaurus
    virtual css::uno::Sequence<
            css::uno::Reference< css::linguistic2::XMeaning > > SAL_CALL
        queryMeanings( const OUString& rTerm,
                const css::lang::Locale& rLocale,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
};


ThesDummy_Impl::~ThesDummy_Impl()
{
    delete pLocaleSeq;
}


void ThesDummy_Impl::GetCfgLocales()
{
    if (!pLocaleSeq)
    {
        SvtLinguConfig aCfg;
        OUString  aNode("ServiceManager/ThesaurusList");
        Sequence < OUString > aNodeNames( aCfg.GetNodeNames( aNode ) );
        const OUString *pNodeNames = aNodeNames.getConstArray();
        sal_Int32 nLen = aNodeNames.getLength();
        pLocaleSeq = new Sequence< lang::Locale >( nLen );
        lang::Locale *pLocale = pLocaleSeq->getArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            pLocale[i] = LanguageTag::convertToLocaleWithFallback( pNodeNames[i] );
        }
    }
}


void ThesDummy_Impl::GetThes_Impl()
{
    if (!xThes.is())
    {
        uno::Reference< XLinguServiceManager2 > xLngSvcMgr( GetLngSvcMgr_Impl() );
        xThes = xLngSvcMgr->getThesaurus();

        if (xThes.is())
        {
            // no longer needed...
            delete pLocaleSeq;    pLocaleSeq = nullptr;
        }
    }
}


uno::Sequence< lang::Locale > SAL_CALL
        ThesDummy_Impl::getLocales()
            throw(uno::RuntimeException, std::exception)
{
    GetThes_Impl();
    if (xThes.is())
        return xThes->getLocales();
    else if (!pLocaleSeq)       // if not already loaded save startup time by avoiding loading them now
        GetCfgLocales();
    return *pLocaleSeq;
}


sal_Bool SAL_CALL
        ThesDummy_Impl::hasLocale( const lang::Locale& rLocale )
            throw(uno::RuntimeException, std::exception)
{
    GetThes_Impl();
    if (xThes.is())
        return xThes->hasLocale( rLocale );
    else if (!pLocaleSeq)       // if not already loaded save startup time by avoiding loading them now
        GetCfgLocales();
    bool bFound = false;
    sal_Int32 nLen = pLocaleSeq->getLength();
    const lang::Locale *pLocale = pLocaleSeq->getConstArray();
    const lang::Locale *pEnd = pLocale + nLen;
    for ( ;  pLocale < pEnd  &&  !bFound;  ++pLocale)
    {
        bFound = pLocale->Language == rLocale.Language  &&
                 pLocale->Country  == rLocale.Country   &&
                 pLocale->Variant  == rLocale.Variant;
    }
    return bFound;
}


uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL
        ThesDummy_Impl::queryMeanings(
                const OUString& rTerm,
                const lang::Locale& rLocale,
                const beans::PropertyValues& rProperties )
            throw(lang::IllegalArgumentException,
                  uno::RuntimeException, std::exception)
{
    GetThes_Impl();
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aRes;
    OSL_ENSURE( xThes.is(), "Thesaurus missing" );
    if (xThes.is())
        aRes = xThes->queryMeanings( rTerm, rLocale, rProperties );
    return aRes;
}


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
class SpellDummy_Impl :
    public cppu::WeakImplHelper< XSpellChecker1 >
{
    uno::Reference< XSpellChecker1 >     xSpell;      // the real one...

    void    GetSpell_Impl();

public:

    // XSupportedLanguages (for XSpellChecker1)
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL
        getLanguages()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        hasLanguage( sal_Int16 nLanguage )
            throw(css::uno::RuntimeException, std::exception) override;

    // XSpellChecker1 (same as XSpellChecker but sal_Int16 for language)
    virtual sal_Bool SAL_CALL
        isValid( const OUString& rWord, sal_Int16 nLanguage,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL
        spell( const OUString& rWord, sal_Int16 nLanguage,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
};


void SpellDummy_Impl::GetSpell_Impl()
{
    if (!xSpell.is())
    {
        uno::Reference< XLinguServiceManager2 > xLngSvcMgr( GetLngSvcMgr_Impl() );
        xSpell.set( xLngSvcMgr->getSpellChecker(), UNO_QUERY );
    }
}


uno::Sequence< sal_Int16 > SAL_CALL
    SpellDummy_Impl::getLanguages()
        throw(uno::RuntimeException, std::exception)
{
    GetSpell_Impl();
    if (xSpell.is())
        return xSpell->getLanguages();
    else
        return uno::Sequence< sal_Int16 >();
}


sal_Bool SAL_CALL
    SpellDummy_Impl::hasLanguage( sal_Int16 nLanguage )
        throw(uno::RuntimeException, std::exception)
{
    GetSpell_Impl();
    bool bRes = false;
    if (xSpell.is())
        bRes = xSpell->hasLanguage( nLanguage );
    return bRes;
}


sal_Bool SAL_CALL
    SpellDummy_Impl::isValid( const OUString& rWord, sal_Int16 nLanguage,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException, std::exception)
{
    GetSpell_Impl();
    bool bRes = true;
    if (xSpell.is())
        bRes = xSpell->isValid( rWord, nLanguage, rProperties );
    return bRes;
}


uno::Reference< linguistic2::XSpellAlternatives > SAL_CALL
    SpellDummy_Impl::spell( const OUString& rWord, sal_Int16 nLanguage,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException, std::exception)
{
    GetSpell_Impl();
    uno::Reference< linguistic2::XSpellAlternatives > xRes;
    if (xSpell.is())
        xRes = xSpell->spell( rWord, nLanguage, rProperties );
    return xRes;
}


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
class HyphDummy_Impl :
    public cppu::WeakImplHelper< XHyphenator >
{
    uno::Reference< XHyphenator >     xHyph;      // the real one...

    void    GetHyph_Impl();

public:

    // XSupportedLocales
    virtual css::uno::Sequence<
            css::lang::Locale > SAL_CALL
        getLocales()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        hasLocale( const css::lang::Locale& rLocale )
            throw(css::uno::RuntimeException, std::exception) override;

    // XHyphenator
    virtual css::uno::Reference<
            css::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const OUString& rWord,
                const css::lang::Locale& rLocale,
                sal_Int16 nMaxLeading,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
            css::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const OUString& rWord,
                const css::lang::Locale& rLocale,
                sal_Int16 nIndex,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
            css::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const OUString& rWord,
                const css::lang::Locale& rLocale,
                const css::beans::PropertyValues& rProperties )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException, std::exception) override;
};


void HyphDummy_Impl::GetHyph_Impl()
{
    if (!xHyph.is())
    {
        uno::Reference< XLinguServiceManager2 > xLngSvcMgr( GetLngSvcMgr_Impl() );
        xHyph = xLngSvcMgr->getHyphenator();
    }
}


uno::Sequence< lang::Locale > SAL_CALL
    HyphDummy_Impl::getLocales()
        throw(uno::RuntimeException, std::exception)
{
    GetHyph_Impl();
    if (xHyph.is())
        return xHyph->getLocales();
    else
        return uno::Sequence< lang::Locale >();
}


sal_Bool SAL_CALL
    HyphDummy_Impl::hasLocale( const lang::Locale& rLocale )
        throw(uno::RuntimeException, std::exception)
{
    GetHyph_Impl();
    bool bRes = false;
    if (xHyph.is())
        bRes = xHyph->hasLocale( rLocale );
    return bRes;
}


uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
    HyphDummy_Impl::hyphenate(
            const OUString& rWord,
            const lang::Locale& rLocale,
            sal_Int16 nMaxLeading,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException, std::exception)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XHyphenatedWord > xRes;
    if (xHyph.is())
        xRes = xHyph->hyphenate( rWord, rLocale, nMaxLeading, rProperties );
    return xRes;
}


uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
    HyphDummy_Impl::queryAlternativeSpelling(
            const OUString& rWord,
            const lang::Locale& rLocale,
            sal_Int16 nIndex,
            const PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException, std::exception)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XHyphenatedWord > xRes;
    if (xHyph.is())
        xRes = xHyph->queryAlternativeSpelling( rWord, rLocale, nIndex, rProperties );
    return xRes;
}


uno::Reference< linguistic2::XPossibleHyphens > SAL_CALL
    HyphDummy_Impl::createPossibleHyphens(
            const OUString& rWord,
            const lang::Locale& rLocale,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException, std::exception)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XPossibleHyphens > xRes;
    if (xHyph.is())
        xRes = xHyph->createPossibleHyphens( rWord, rLocale, rProperties );
    return xRes;
}

class LinguMgrExitLstnr : public cppu::WeakImplHelper<XEventListener>
{
    uno::Reference< XDesktop2 >        xDesktop;

    static void AtExit();

public:
    LinguMgrExitLstnr();
    virtual ~LinguMgrExitLstnr();

    // lang::XEventListener
    virtual void    SAL_CALL disposing(const EventObject& rSource)
            throw( RuntimeException, std::exception ) override;
};

LinguMgrExitLstnr::LinguMgrExitLstnr()
{
    // add object to frame::Desktop EventListeners in order to properly call
    // the AtExit function at application exit.

    uno::Reference< XComponentContext >  xContext = getProcessComponentContext();
    xDesktop = Desktop::create( xContext );
    xDesktop->addEventListener( this );
}

LinguMgrExitLstnr::~LinguMgrExitLstnr()
{
    if (xDesktop.is())
    {
        xDesktop->removeEventListener( this );
        xDesktop = nullptr;    //! release reference to desktop
    }
    OSL_ENSURE(!xDesktop.is(), "reference to desktop should be released");
}

void LinguMgrExitLstnr::disposing(const EventObject& rSource)
        throw( RuntimeException, std::exception )
{
    if (xDesktop.is()  &&  rSource.Source == xDesktop)
    {
        xDesktop->removeEventListener( this );
        xDesktop = nullptr;    //! release reference to desktop

        AtExit();
    }
}

void LinguMgrExitLstnr::AtExit()
{
    SolarMutexGuard g;

    // release references
    LinguMgr::xLngSvcMgr    = nullptr;
    LinguMgr::xSpell        = nullptr;
    LinguMgr::xHyph         = nullptr;
    LinguMgr::xThes         = nullptr;
    LinguMgr::xDicList      = nullptr;
    LinguMgr::xProp         = nullptr;
    LinguMgr::xIgnoreAll    = nullptr;
    LinguMgr::xChangeAll    = nullptr;

    LinguMgr::bExiting      = true;

    LinguMgr::pExitLstnr    = nullptr;
}


LinguMgrExitLstnr *             LinguMgr::pExitLstnr    = nullptr;
bool                            LinguMgr::bExiting      = false;
uno::Reference< XLinguServiceManager2 >  LinguMgr::xLngSvcMgr    = nullptr;
uno::Reference< XSpellChecker1 >    LinguMgr::xSpell        = nullptr;
uno::Reference< XHyphenator >       LinguMgr::xHyph         = nullptr;
uno::Reference< XThesaurus >        LinguMgr::xThes         = nullptr;
uno::Reference< XSearchableDictionaryList >   LinguMgr::xDicList      = nullptr;
uno::Reference< XLinguProperties >  LinguMgr::xProp         = nullptr;
uno::Reference< XDictionary >       LinguMgr::xIgnoreAll    = nullptr;
uno::Reference< XDictionary >       LinguMgr::xChangeAll    = nullptr;


uno::Reference< XLinguServiceManager2 > LinguMgr::GetLngSvcMgr()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    return xLngSvcMgr;
}


uno::Reference< XSpellChecker1 > LinguMgr::GetSpellChecker()
{
    return xSpell.is() ? xSpell : GetSpell();
}

uno::Reference< XHyphenator > LinguMgr::GetHyphenator()
{
    return xHyph.is() ? xHyph : GetHyph();
}

uno::Reference< XThesaurus > LinguMgr::GetThesaurus()
{
    return xThes.is() ? xThes : GetThes();
}

uno::Reference< XSearchableDictionaryList > LinguMgr::GetDictionaryList()
{
    return xDicList.is() ? xDicList : GetDicList();
}

uno::Reference< linguistic2::XLinguProperties > LinguMgr::GetLinguPropertySet()
{
    return xProp.is() ? xProp : GetProp();
}

uno::Reference< XDictionary > LinguMgr::GetStandardDic()
{
    //! don't hold reference to this
    //! (it may be removed from dictionary list and needs to be
    //! created empty if accessed again)
    return GetStandard();
}

uno::Reference< XDictionary > LinguMgr::GetIgnoreAllList()
{
    return xIgnoreAll.is() ? xIgnoreAll : GetIgnoreAll();
}

uno::Reference< XDictionary > LinguMgr::GetChangeAllList()
{
    return xChangeAll.is() ? xChangeAll : GetChangeAll();
}

uno::Reference< XSpellChecker1 > LinguMgr::GetSpell()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    xSpell = new SpellDummy_Impl;
    return xSpell;
}

uno::Reference< XHyphenator > LinguMgr::GetHyph()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    xHyph = new HyphDummy_Impl;
    return xHyph;
}

uno::Reference< XThesaurus > LinguMgr::GetThes()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    //! when only the XSupportedLocales interface is used.
    //! The dummy accesses the real implementation (and thus loading the DLL)
    //! when "real" work needs to be done only.
    xThes = new ThesDummy_Impl;
    return xThes;
}

uno::Reference< XSearchableDictionaryList > LinguMgr::GetDicList()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    xDicList = linguistic2::DictionaryList::create( getProcessComponentContext() );
    return xDicList;
}

uno::Reference< linguistic2::XLinguProperties > LinguMgr::GetProp()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    xProp = linguistic2::LinguProperties::create( getProcessComponentContext()  );
    return xProp;
}

uno::Reference< XDictionary > LinguMgr::GetIgnoreAll()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XSearchableDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (xTmpDicList.is())
    {
        xIgnoreAll.set( xTmpDicList->getDictionaryByName( "IgnoreAllList" ), UNO_QUERY );
    }
    return xIgnoreAll;
}

uno::Reference< XDictionary > LinguMgr::GetChangeAll()
{
    if (bExiting)
        return nullptr;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XSearchableDictionaryList > _xDicList( GetDictionaryList() , UNO_QUERY );
    if (_xDicList.is())
    {
        xChangeAll.set( _xDicList->createDictionary(
                            "ChangeAllList",
                            LanguageTag::convertToLocale( LANGUAGE_NONE ),
                            DictionaryType_NEGATIVE, OUString() ),
                        UNO_QUERY );
    }
    return xChangeAll;
}

uno::Reference< XDictionary > LinguMgr::GetStandard()
{
    // Tries to return a dictionary which may hold positive entries is
    // persistent and not read-only.

    if (bExiting)
        return nullptr;

    uno::Reference< XSearchableDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (!xTmpDicList.is())
        return nullptr;

    const OUString aDicName( "standard.dic" );
    uno::Reference< XDictionary >   xDic( xTmpDicList->getDictionaryByName( aDicName ),
                                      UNO_QUERY );
    if (!xDic.is())
    {
        // try to create standard dictionary
        uno::Reference< XDictionary >    xTmp;
        try
        {
            xTmp =  xTmpDicList->createDictionary( aDicName,
                        LanguageTag::convertToLocale( LANGUAGE_NONE ),
                        DictionaryType_POSITIVE,
                        linguistic::GetWritableDictionaryURL( aDicName ) );
        }
        catch(const css::uno::Exception &)
        {
        }

        // add new dictionary to list
        if (xTmp.is())
        {
            xTmpDicList->addDictionary( xTmp );
            xTmp->setActive( true );
        }
        xDic.set( xTmp, UNO_QUERY );
    }
#if OSL_DEBUG_LEVEL > 1
    uno::Reference< XStorable >      xStor( xDic, UNO_QUERY );
    OSL_ENSURE( xDic.is() && xDic->getDictionaryType() == DictionaryType_POSITIVE,
            "wrong dictionary type");
    OSL_ENSURE( xDic.is() && LanguageTag( xDic->getLocale() ).getLanguageType() == LANGUAGE_NONE,
            "wrong dictionary language");
    OSL_ENSURE( !xStor.is() || (xStor->hasLocation() && !xStor->isReadonly()),
            "dictionary not editable" );
#endif

    return xDic;
}

uno::Reference< XSpellChecker1 >  SvxGetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

uno::Reference< XHyphenator >  SvxGetHyphenator()
{
    return LinguMgr::GetHyphenator();
}

uno::Reference< XThesaurus >  SvxGetThesaurus()
{
    return LinguMgr::GetThesaurus();
}

uno::Reference< XSearchableDictionaryList >  SvxGetDictionaryList()
{
    return LinguMgr::GetDictionaryList();
}

uno::Reference< XLinguProperties >  SvxGetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

//TODO: remove argument or provide SvxGetIgnoreAllList with the same one
uno::Reference< XDictionary >  SvxGetOrCreatePosDic(
        uno::Reference< XSearchableDictionaryList >  /* xDicList */ )
{
    return LinguMgr::GetStandardDic();
}

uno::Reference< XDictionary >  SvxGetIgnoreAllList()
{
    return LinguMgr::GetIgnoreAllList();
}

uno::Reference< XDictionary >  SvxGetChangeAllList()
{
    return LinguMgr::GetChangeAllList();
}

#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

SvxAlternativeSpelling SvxGetAltSpelling(
        const css::uno::Reference< css::linguistic2::XHyphenatedWord > & rHyphWord )
{
    SvxAlternativeSpelling aRes;
    if (rHyphWord.is() && rHyphWord->isAlternativeSpelling())
    {
        OUString aWord( rHyphWord->getWord() ),
                 aAltWord( rHyphWord->getHyphenatedWord() );
        sal_Int16   nHyphenationPos     = rHyphWord->getHyphenationPos(),
                nHyphenPos          = rHyphWord->getHyphenPos();
        sal_Int16   nLen    = (sal_Int16)aWord.getLength();
        sal_Int16   nAltLen = (sal_Int16)aAltWord.getLength();
        const sal_Unicode *pWord    = aWord.getStr(),
                          *pAltWord = aAltWord.getStr();

        // count number of chars from the left to the
        // hyphenation pos / hyphen pos that are equal
        sal_Int16 nL = 0;
        while (nL <= nHyphenationPos && nL <= nHyphenPos
               && pWord[ nL ] == pAltWord[ nL ])
            ++nL;
        // count number of chars from the right to the
        // hyphenation pos / hyphen pos that are equal
        sal_Int16 nR = 0;
        sal_Int32 nIdx    = nLen - 1;
        sal_Int32 nAltIdx = nAltLen - 1;
        while (nIdx > nHyphenationPos && nAltIdx > nHyphenPos
               && pWord[ nIdx-- ] == pAltWord[ nAltIdx-- ])
            ++nR;

        aRes.aReplacement       = aAltWord.copy( nL, nAltLen - nL - nR );
        aRes.nChangedPos        = (sal_Int16) nL;
        aRes.nChangedLength     = nLen - nL - nR;
        aRes.bIsAltSpelling     = true;
        aRes.xHyphWord          = rHyphWord;
    }
    return aRes;
}


SvxDicListChgClamp::SvxDicListChgClamp( uno::Reference< XSearchableDictionaryList >  &rxDicList ) :
    xDicList    ( rxDicList )
{
    if (xDicList.is())
    {
        xDicList->beginCollectEvents();
    }
}

SvxDicListChgClamp::~SvxDicListChgClamp()
{
    if (xDicList.is())
    {
        xDicList->endCollectEvents();
    }
}

short SvxDicError( vcl::Window *pParent, linguistic::DictionaryError nError )
{
    short nRes = 0;
    if (linguistic::DictionaryError::NONE != nError)
    {
        int nRid;
        switch (nError)
        {
            case linguistic::DictionaryError::FULL     : nRid = RID_SVXSTR_DIC_ERR_FULL;  break;
            case linguistic::DictionaryError::READONLY : nRid = RID_SVXSTR_DIC_ERR_READONLY;  break;
            default:
                nRid = RID_SVXSTR_DIC_ERR_UNKNOWN;
                SAL_WARN("editeng", "unexpected case");
        }
        nRes = ScopedVclPtrInstance<InfoBox>(pParent, EE_RESSTR(nRid))->Execute();
    }
    return nRes;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
