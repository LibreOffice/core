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

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include <lingutil.hxx>
#include <hunspell.hxx>
#include "sspellimp.hxx"

#include <linguistic/lngprops.hxx>
#include <linguistic/spelldta.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/strings.hrc>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/log.hxx>

#include <utility>
#include <vector>
#include <set>
#include <string.h>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

// XML-header of SPELLML queries
#if !defined SPELL_XML
#define SPELL_XML "<?xml?>"
#endif

// only available in hunspell >= 1.5
#if !defined MAXWORDLEN
#define MAXWORDLEN 176
#endif

SpellChecker::SpellChecker() :
    m_aEvtListeners(GetLinguMutex()),
    m_bDisposing(false)
{
}

SpellChecker::DictItem::DictItem(OUString i_DName, Locale i_DLoc, rtl_TextEncoding i_DEnc)
    : m_aDName(std::move(i_DName))
    , m_aDLoc(std::move(i_DLoc))
    , m_aDEnc(i_DEnc)
{
}

SpellChecker::~SpellChecker()
{
    if (m_pPropHelper)
    {
        m_pPropHelper->RemoveAsPropListener();
    }
}

PropertyHelper_Spelling & SpellChecker::GetPropHelper_Impl()
{
    if (!m_pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        m_pPropHelper.reset( new PropertyHelper_Spelling( static_cast<XSpellChecker *>(this), xPropSet ) );
        m_pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *m_pPropHelper;
}

Sequence< Locale > SAL_CALL SpellChecker::getLocales()
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.
    if (m_DictItems.empty())
    {
        SvtLinguConfig aLinguCfg;

        // get list of extension dictionaries-to-use
        // (or better speaking: the list of dictionaries using the
        // new configuration entries).
        std::vector< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( "SpellCheckers",
                "org.openoffice.lingu.MySpellSpellChecker", aFormatList );
        for (auto const& format : aFormatList)
        {
            std::vector< SvtLinguConfigDictionaryEntry > aTmpDic(
                    aLinguCfg.GetActiveDictionariesByFormat(format) );
            aDics.insert( aDics.end(), aTmpDic.begin(), aTmpDic.end() );
        }

        //!! for compatibility with old dictionaries (the ones not using extensions
        //!! or new configuration entries, but still using the dictionary.lst file)
        //!! Get the list of old style spell checking dictionaries to use...
        std::vector< SvtLinguConfigDictionaryEntry > aOldStyleDics(
                GetOldStyleDics( "DICT" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list od new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        if (!aDics.empty())
        {
            uno::Reference< lang::XMultiServiceFactory > xServiceFactory(comphelper::getProcessServiceFactory());
            uno::Reference< ucb::XSimpleFileAccess > xAccess(xServiceFactory->createInstance("com.sun.star.ucb.SimpleFileAccess"), uno::UNO_QUERY);
            // get supported locales from the dictionaries-to-use...
            std::set<OUString> aLocaleNamesSet;
            for (auto const& dict : aDics)
            {
                uno::Sequence< OUString > aLocaleNames( dict.aLocaleNames );
                uno::Sequence< OUString > aLocations( dict.aLocations );
                SAL_WARN_IF(
                    aLocaleNames.hasElements() && !aLocations.hasElements(),
                    "lingucomponent", "no locations");
                if (aLocations.hasElements())
                {
                    if (xAccess.is() && xAccess->exists(aLocations[0]))
                    {
                        for (auto const& locale : aLocaleNames)
                        {
                            if (!comphelper::LibreOfficeKit::isWhitelistedLanguage(locale))
                                continue;

                            aLocaleNamesSet.insert(locale);
                        }
                    }
                    else
                    {
                        SAL_WARN(
                            "lingucomponent",
                            "missing <" << aLocations[0] << ">");
                    }
                }
            }
            // ... and add them to the resulting sequence
            m_aSuppLocales.realloc( aLocaleNamesSet.size() );
            sal_Int32 k = 0;
            for (auto const& localeName : aLocaleNamesSet)
            {
                Locale aTmp( LanguageTag::convertToLocale(localeName));
                m_aSuppLocales[k++] = aTmp;
            }

            //! For each dictionary and each locale we need a separate entry.
            //! If this results in more than one dictionary per locale than (for now)
            //! it is undefined which dictionary gets used.
            //! In the future the implementation should support using several dictionaries
            //! for one locale.
            sal_uInt32 nDictSize = 0;
            for (auto const& dict : aDics)
                nDictSize += dict.aLocaleNames.getLength();

            // add dictionary information
            m_DictItems.reserve(nDictSize);
            for (auto const& dict : aDics)
            {
                if (dict.aLocaleNames.getLength() > 0 &&
                    dict.aLocations.getLength() > 0)
                {
                    uno::Sequence< OUString > aLocaleNames( dict.aLocaleNames );

                    // currently only one language per dictionary is supported in the actual implementation...
                    // Thus here we work-around this by adding the same dictionary several times.
                    // Once for each of its supported locales.
                    for (auto const& localeName : aLocaleNames)
                    {
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = dict.aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );

                        m_DictItems.emplace_back(aLocation, LanguageTag::convertToLocale(localeName), RTL_TEXTENCODING_DONTKNOW);
                    }
                }
            }
            DBG_ASSERT( nDictSize == m_DictItems.size(), "index mismatch?" );
        }
        else
        {
            // no dictionary found so register no dictionaries
            m_aSuppLocales.realloc(0);
        }
    }

    return m_aSuppLocales;
}

sal_Bool SAL_CALL SpellChecker::hasLocale(const Locale& rLocale)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_aSuppLocales.getLength())
        getLocales();

    for (auto const& suppLocale : m_aSuppLocales)
    {
        if (rLocale == suppLocale)
        {
            bRes = true;
            break;
        }
    }
    return bRes;
}

sal_Int16 SpellChecker::GetSpellFailure(const OUString &rWord, const Locale &rLocale)
{
    if (rWord.getLength() > MAXWORDLEN)
        return -1;

    Hunspell * pMS = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // initialize a myspell object for each dictionary once
    // (note: mutex is held higher up in isValid)

    sal_Int16 nRes = -1;

    // first handle smart quotes both single and double
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    sal_Int32 extrachar = 0;

    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = u'"';
        else if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = u'\'';

        // recognize words with Unicode ligatures and ZWNJ/ZWJ characters (only
        // with 8-bit encoded dictionaries. For UTF-8 encoded dictionaries
        // set ICONV and IGNORE aff file options, if needed.)
        else if ((c == 0x200C) || (c == 0x200D) ||
            ((c >= 0xFB00) && (c <= 0xFB04)))
                extrachar = 1;
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        for (auto& currDict : m_DictItems)
        {
            pMS = nullptr;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == currDict.m_aDLoc)
            {
                if (!currDict.m_pDict)
                {
                    OUString dicpath = currDict.m_aDName + ".dic";
                    OUString affpath = currDict.m_aDName + ".aff";
                    OUString dict;
                    OUString aff;
                    osl::FileBase::getSystemPathFromFileURL(dicpath,dict);
                    osl::FileBase::getSystemPathFromFileURL(affpath,aff);
#if defined(_WIN32)
                    // workaround for Windows specific problem that the
                    // path length in calls to 'fopen' is limited to somewhat
                    // about 120+ characters which will usually be exceed when
                    // using dictionaries as extensions. (Hunspell waits UTF-8 encoded
                    // path with \\?\ long path prefix.)
                    OString aTmpaff = Win_AddLongPathPrefix(OUStringToOString(aff, RTL_TEXTENCODING_UTF8));
                    OString aTmpdict = Win_AddLongPathPrefix(OUStringToOString(dict, RTL_TEXTENCODING_UTF8));
#else
                    OString aTmpaff(OU2ENC(aff,osl_getThreadTextEncoding()));
                    OString aTmpdict(OU2ENC(dict,osl_getThreadTextEncoding()));
#endif

                    currDict.m_pDict = std::make_unique<Hunspell>(aTmpaff.getStr(),aTmpdict.getStr());
#if defined(H_DEPRECATED)
                    currDict.m_aDEnc = getTextEncodingFromCharset(currDict.m_pDict->get_dict_encoding().c_str());
#else
                    currDict.m_aDEnc = getTextEncodingFromCharset(currDict.m_pDict->get_dic_encoding());
#endif
                }
                pMS  = currDict.m_pDict.get();
                eEnc = currDict.m_aDEnc;
            }

            if (pMS)
            {
                // we don't want to work with a default text encoding since following incorrect
                // results may occur only for specific text and thus may be hard to notice.
                // Thus better always make a clean exit here if the text encoding is in question.
                // Hopefully something not working at all will raise proper attention quickly. ;-)
                DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
                if (eEnc == RTL_TEXTENCODING_DONTKNOW)
                    return -1;

                OString aWrd(OU2ENC(nWord,eEnc));
#if defined(H_DEPRECATED)
                bool bVal = pMS->spell(std::string(aWrd.getStr()));
#else
                bool bVal = pMS->spell(aWrd.getStr()) != 0;
#endif
                if (!bVal) {
                    if (extrachar && (eEnc != RTL_TEXTENCODING_UTF8)) {
                        OUStringBuffer aBuf(nWord);
                        n = aBuf.getLength();
                        for (sal_Int32 ix=n-1; ix >= 0; ix--)
                        {
                          switch (aBuf[ix]) {
                            case 0xFB00: aBuf.remove(ix, 1); aBuf.insert(ix, "ff"); break;
                            case 0xFB01: aBuf.remove(ix, 1); aBuf.insert(ix, "fi"); break;
                            case 0xFB02: aBuf.remove(ix, 1); aBuf.insert(ix, "fl"); break;
                            case 0xFB03: aBuf.remove(ix, 1); aBuf.insert(ix, "ffi"); break;
                            case 0xFB04: aBuf.remove(ix, 1); aBuf.insert(ix, "ffl"); break;
                            case 0x200C:
                            case 0x200D: aBuf.remove(ix, 1); break;
                          }
                        }
                        OUString aWord(aBuf.makeStringAndClear());
                        OString bWrd(OU2ENC(aWord, eEnc));
#if defined(H_DEPRECATED)
                        bVal = pMS->spell(std::string(bWrd.getStr()));
#else
                        bVal = pMS->spell(bWrd.getStr()) != 0;
#endif
                        if (bVal) return -1;
                    }
                    nRes = SpellFailure::SPELLING_ERROR;
                } else {
                    return -1;
                }
                pMS = nullptr;
            }
        }
    }

    return nRes;
}

sal_Bool SAL_CALL SpellChecker::isValid( const OUString& rWord, const Locale& rLocale,
            const css::uno::Sequence< css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rLocale == Locale()  ||  rWord.isEmpty())
        return true;

    if (!hasLocale( rLocale ))
        return true;

    // return sal_False to process SPELLML requests (they are longer than the header)
    if (rWord.match(SPELL_XML, 0) && (rWord.getLength() > 10)) return false;

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simpler solution than the provided
    // one using the PropertyHelper_Spell.
    PropertyHelper_Spelling& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1 && !rWord.match(SPELL_XML, 0))
    {
        LanguageType nLang = LinguLocaleToLanguage( rLocale );
        // postprocess result for errors that should be ignored
        const bool bIgnoreError =
                (!rHelper.IsSpellUpperCase()  && IsUpper( rWord, nLang )) ||
                (!rHelper.IsSpellWithDigits() && HasDigits( rWord )) ||
                (!rHelper.IsSpellCapitalization()  &&  nFailure == SpellFailure::CAPTION_ERROR);
        if (bIgnoreError)
            nFailure = -1;
    }

    return (nFailure == -1);
}

Reference< XSpellAlternatives >
    SpellChecker::GetProposals( const OUString &rWord, const Locale &rLocale )
{
    // Retrieves the return values for the 'spell' function call in case
    // of a misspelled word.
    // Especially it may give a list of suggested (correct) words:
    Reference< XSpellAlternatives > xRes;
    // note: mutex is held by higher up by spell which covers both

    Hunspell* pMS = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // first handle smart quotes (single and double)
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = u'"';
        if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = u'\'';
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        LanguageType nLang = LinguLocaleToLanguage( rLocale );
        int numsug = 0;

        Sequence< OUString > aStr( 0 );
        for (const auto& currDict : m_DictItems)
        {
            pMS = nullptr;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == currDict.m_aDLoc)
            {
                pMS  = currDict.m_pDict.get();
                eEnc = currDict.m_aDEnc;
            }

            if (pMS)
            {
                OString aWrd(OU2ENC(nWord,eEnc));
#if defined(H_DEPRECATED)
                std::vector<std::string> suglst = pMS->suggest(std::string(aWrd.getStr()));
                if (!suglst.empty())
                {
                    aStr.realloc(numsug + suglst.size());
                    OUString *pStr = aStr.getArray();
                    for (size_t ii = 0; ii < suglst.size(); ++ii)
                    {
                        OUString cvtwrd(suglst[ii].c_str(), suglst[ii].size(), eEnc);
                        pStr[numsug + ii] = cvtwrd;
                    }
                    numsug += suglst.size();
                }
#else
                char ** suglst = nullptr;
                int count = pMS->suggest(&suglst, aWrd.getStr());
                if (count)
                {
                    aStr.realloc( numsug + count );
                    OUString *pStr = aStr.getArray();
                    for (int ii=0; ii < count; ++ii)
                    {
                        OUString cvtwrd(suglst[ii],strlen(suglst[ii]),eEnc);
                        pStr[numsug + ii] = cvtwrd;
                    }
                    numsug += count;
                }
                pMS->free_list(&suglst, count);
#endif
            }
        }

        // now return an empty alternative for no suggestions or the list of alternatives if some found
        xRes = SpellAlternatives::CreateSpellAlternatives( rWord, nLang, SpellFailure::SPELLING_ERROR, aStr );
        return xRes;
    }
    return xRes;
}

Reference< XSpellAlternatives > SAL_CALL SpellChecker::spell(
        const OUString& rWord, const Locale& rLocale,
        const css::uno::Sequence< css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rLocale == Locale()  ||  rWord.isEmpty())
        return nullptr;

    if (!hasLocale( rLocale ))
        return nullptr;

    Reference< XSpellAlternatives > xAlt;
    if (!isValid( rWord, rLocale, rProperties ))
    {
        xAlt =  GetProposals( rWord, rLocale );
    }
    return xAlt;
}

/// @throws Exception
static Reference< XInterface > SpellChecker_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
{

    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new SpellChecker);
    return xService;
}

sal_Bool SAL_CALL SpellChecker::addLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

sal_Bool SAL_CALL SpellChecker::removeLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

OUString SAL_CALL SpellChecker::getServiceDisplayName(const Locale& rLocale)
{
    std::locale loc(Translate::Create("svt", LanguageTag(rLocale)));
    return Translate::get(STR_DESCRIPTION_HUNSPELL, loc);
}

void SAL_CALL SpellChecker::initialize( const Sequence< Any >& rArguments )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference< XLinguProperties >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;
            // rArguments.getConstArray()[1] >>= xDicList;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is no longer used.
            m_pPropHelper.reset( new PropertyHelper_Spelling( static_cast<XSpellChecker *>(this), xPropSet ) );
            m_pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}

void SAL_CALL SpellChecker::dispose()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing)
    {
        m_bDisposing = true;
        EventObject aEvtObj( static_cast<XSpellChecker *>(this) );
        m_aEvtListeners.disposeAndClear( aEvtObj );
        if (m_pPropHelper)
        {
            m_pPropHelper->RemoveAsPropListener();
            m_pPropHelper.reset();
        }
    }
}

void SAL_CALL SpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL SpellChecker::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL SpellChecker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SpellChecker::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Sequence< OUString > SpellChecker::getSupportedServiceNames_Static()
        throw()
{
    Sequence< OUString > aSNS { SN_SPELLCHECKER };
    return aSNS;
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * spell_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    if ( SpellChecker::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                SpellChecker::getImplementationName_Static(),
                SpellChecker_CreateInstance,
                SpellChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
