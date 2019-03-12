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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <hyphen.h>
#include "hyphenimp.hxx"

#include <linguistic/hyphdta.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/log.hxx>

#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <svtools/strings.hrc>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>
#include <osl/file.hxx>

#include <stdio.h>
#include <string.h>

#include <cassert>
#include <vector>
#include <set>
#include <memory>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

Hyphenator::Hyphenator() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = false;
}

Hyphenator::~Hyphenator()
{
    for (auto & rInfo : mvDicts)
    {
        if (rInfo.aPtr)
            hnj_hyphen_free(rInfo.aPtr);
    }

    if (pPropHelper)
    {
        pPropHelper->RemoveAsPropListener();
    }
}

PropertyHelper_Hyphenation& Hyphenator::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper.reset( new PropertyHelper_Hyphenation (static_cast<XHyphenator *>(this), xPropSet ) );
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}

Sequence< Locale > SAL_CALL Hyphenator::getLocales()
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.
    if (mvDicts.empty())
    {
        SvtLinguConfig aLinguCfg;

        // get list of dictionaries-to-use
        // (or better speaking: the list of dictionaries using the
        // new configuration entries).
        std::vector< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( "Hyphenators",
                "org.openoffice.lingu.LibHnjHyphenator", aFormatList );
        sal_Int32 nLen = aFormatList.getLength();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            std::vector< SvtLinguConfigDictionaryEntry > aTmpDic(
                    aLinguCfg.GetActiveDictionariesByFormat( aFormatList[i] ) );
            aDics.insert( aDics.end(), aTmpDic.begin(), aTmpDic.end() );
        }

        //!! for compatibility with old dictionaries (the ones not using extensions
        //!! or new configuration entries, but still using the dictionary.lst file)
        //!! Get the list of old style spell checking dictionaries to use...
        std::vector< SvtLinguConfigDictionaryEntry > aOldStyleDics(
                GetOldStyleDics( "HYPH" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list of new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        sal_Int32 numdict = aDics.size();
        if (numdict)
        {
            // get supported locales from the dictionaries-to-use...
            sal_Int32 k = 0;
            std::set<OUString> aLocaleNamesSet;
            for (auto const& dict : aDics)
            {
                uno::Sequence< OUString > aLocaleNames( dict.aLocaleNames );
                sal_Int32 nLen2 = aLocaleNames.getLength();
                for (k = 0;  k < nLen2;  ++k)
                {
                    aLocaleNamesSet.insert( aLocaleNames[k] );
                }
            }
            // ... and add them to the resulting sequence
            aSuppLocales.realloc( aLocaleNamesSet.size() );
            k = 0;
            for (auto const& localeName :  aLocaleNamesSet)
            {
                Locale aTmp( LanguageTag::convertToLocale(localeName));
                aSuppLocales[k++] = aTmp;
            }

            //! For each dictionary and each locale we need a separate entry.
            //! If this results in more than one dictionary per locale than (for now)
            //! it is undefined which dictionary gets used.
            //! In the future the implementation should support using several dictionaries
            //! for one locale.
            numdict = 0;
            for (auto const& dict :  aDics)
                numdict = numdict + dict.aLocaleNames.getLength();

            // add dictionary information
            mvDicts.resize(numdict);

            k = 0;
            for (auto const& dict :  aDics)
            {
                if (dict.aLocaleNames.getLength() > 0 &&
                    dict.aLocations.getLength() > 0)
                {
                    uno::Sequence< OUString > aLocaleNames(dict.aLocaleNames);
                    sal_Int32 nLocales = aLocaleNames.getLength();

                    // currently only one language per dictionary is supported in the actual implementation...
                    // Thus here we work-around this by adding the same dictionary several times.
                    // Once for each of its supported locales.
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        LanguageTag aLanguageTag(dict.aLocaleNames[i]);
                        mvDicts[k].aPtr = nullptr;
                        mvDicts[k].eEnc = RTL_TEXTENCODING_DONTKNOW;
                        mvDicts[k].aLoc = aLanguageTag.getLocale();
                        mvDicts[k].apCC.reset( new CharClass( aLanguageTag ) );
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = dict.aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        mvDicts[k].aName = aLocation;

                        ++k;
                    }
                }
            }
            DBG_ASSERT( k == numdict, "index mismatch?" );
        }
        else
        {
            // no dictionary found so register no dictionaries
            numdict = 0;
            mvDicts.clear();
            aSuppLocales.realloc(0);
        }
    }

    return aSuppLocales;
}

sal_Bool SAL_CALL Hyphenator::hasLocale(const Locale& rLocale)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!aSuppLocales.getLength())
        getLocales();

    const Locale *pLocale = aSuppLocales.getConstArray();
    sal_Int32 nLen = aSuppLocales.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (rLocale == pLocale[i])
        {
            bRes = true;
            break;
        }
    }
    return bRes;
}

namespace {
bool LoadDictionary(HDInfo& rDict)
{
    OUString DictFN = rDict.aName + ".dic";
    OUString dictpath;

    osl::FileBase::getSystemPathFromFileURL(DictFN, dictpath);

#if defined(_WIN32)
    // hnj_hyphen_load expects UTF-8 encoded paths with \\?\ long path prefix.
    OString sTmp = Win_AddLongPathPrefix(OUStringToOString(dictpath, RTL_TEXTENCODING_UTF8));
#else
    OString sTmp(OU2ENC(dictpath, osl_getThreadTextEncoding()));
#endif
    HyphenDict *dict = nullptr;
    if ((dict = hnj_hyphen_load(sTmp.getStr())) == nullptr)
    {
        SAL_WARN(
            "lingucomponent",
            "Couldn't find file " << OU2ENC(dictpath, osl_getThreadTextEncoding()));
        return false;
    }
    rDict.aPtr = dict;
    rDict.eEnc = getTextEncodingFromCharset(dict->cset);
    return true;
}
}

Reference< XHyphenatedWord > SAL_CALL Hyphenator::hyphenate( const OUString& aWord,
       const css::lang::Locale& aLocale,
       sal_Int16 nMaxLeading,
       const css::uno::Sequence< css::beans::PropertyValue >& aProperties )
{
    PropertyHelper_Hyphenation& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    HyphenDict *dict = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    Reference< XHyphenatedWord > xRes;

    int k = -1;
    for (size_t j = 0; j < mvDicts.size(); ++j)
    {
        if (aLocale == mvDicts[j].aLoc)
            k = j;
    }

    // if we have a hyphenation dictionary matching this locale
    if (k != -1)
    {
        int nHyphenationPos = -1;
        int nHyphenationPosAlt = -1;
        int nHyphenationPosAltHyph = -1;

        // if this dictionary has not been loaded yet do that
        if (!mvDicts[k].aPtr)
        {
            if (!LoadDictionary(mvDicts[k]))
                return nullptr;
        }

        // other wise hyphenate the word with that dictionary
        dict = mvDicts[k].aPtr;
        eEnc = mvDicts[k].eEnc;
        CharClass * pCC =  mvDicts[k].apCC.get();

        // we don't want to work with a default text encoding since following incorrect
        // results may occur only for specific text and thus may be hard to notice.
        // Thus better always make a clean exit here if the text encoding is in question.
        // Hopefully something not working at all will raise proper attention quickly. ;-)
        DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
        if (eEnc == RTL_TEXTENCODING_DONTKNOW)
            return nullptr;

        CapType ct = capitalType(aWord, pCC);

        // first convert any smart quotes or apostrophes to normal ones
        OUStringBuffer rBuf(aWord);
        sal_Int32 nc = rBuf.getLength();
        sal_Unicode ch;
        for (sal_Int32 ix=0; ix < nc; ix++)
        {
            ch = rBuf[ix];
            if ((ch == 0x201C) || (ch == 0x201D))
                rBuf[ix] = u'"';
            if ((ch == 0x2018) || (ch == 0x2019))
                rBuf[ix] = u'\'';
        }
        OUString nWord(rBuf.makeStringAndClear());

        // now convert word to all lowercase for pattern recognition
        OUString nTerm(makeLowerCase(nWord, pCC));

        // now convert word to needed encoding
        OString encWord(OU2ENC(nTerm,eEnc));

        int wordlen = encWord.getLength();
        std::unique_ptr<char[]> lcword(new char[wordlen + 1]);
        std::unique_ptr<char[]> hyphens(new char[wordlen + 5]);

        char ** rep = nullptr; // replacements of discretionary hyphenation
        int * pos = nullptr; // array of [hyphenation point] minus [deletion position]
        int * cut = nullptr; // length of deletions in original word

        // copy converted word into simple char buffer
        strcpy(lcword.get(),encWord.getStr());

        // now strip off any ending periods
        int n = wordlen-1;
        while((n >=0) && (lcword[n] == '.'))
            n--;
        n++;
        if (n > 0)
        {
            const bool bFailed = 0 != hnj_hyphen_hyphenate3( dict, lcword.get(), n, hyphens.get(), nullptr,
                    &rep, &pos, &cut, minLead, minTrail,
                    std::max<sal_Int16>(dict->clhmin, std::max<sal_Int16>(dict->clhmin, 2) + std::max(0, minLead  - std::max<sal_Int16>(dict->lhmin, 2))),
                    std::max<sal_Int16>(dict->crhmin, std::max<sal_Int16>(dict->crhmin, 2) + std::max(0, minTrail - std::max<sal_Int16>(dict->rhmin, 2))) );
            if (bFailed)
            {
                // whoops something did not work
                if (rep)
                {
                    for(int j = 0; j < n; j++)
                    {
                        if (rep[j]) free(rep[j]);
                    }
                    free(rep);
                }
                if (pos) free(pos);
                if (cut) free(cut);
                return nullptr;
            }
        }

        // now backfill hyphens[] for any removed trailing periods
        for (int c = n; c < wordlen; c++) hyphens[c] = '0';
        hyphens[wordlen] = '\0';

        sal_Int32 Leading =  GetPosInWordToCheck( aWord, nMaxLeading );

        for (sal_Int32 i = 0; i < n; i++)
        {
            int leftrep = 0;
            bool hit = (n >= minLen);
            if (!rep || !rep[i])
            {
                hit = hit && (hyphens[i]&1) && (i < Leading);
                hit = hit && (i >= (minLead-1) );
                hit = hit && ((n - i - 1) >= minTrail);
            }
            else
            {
                // calculate change character length before hyphenation point signed with '='
                for (char * c = rep[i]; *c && (*c != '='); c++)
                {
                    if (eEnc == RTL_TEXTENCODING_UTF8)
                    {
                        if (static_cast<unsigned char>(*c) >> 6 != 2)
                            leftrep++;
                    }
                    else
                        leftrep++;
                }
                hit = hit && (hyphens[i]&1) && ((i + leftrep - pos[i]) < Leading);
                hit = hit && ((i + leftrep - pos[i]) >= (minLead-1) );
                hit = hit && ((n - i - 1 + sal::static_int_cast< sal_sSize >(strlen(rep[i])) - leftrep - 1) >= minTrail);
            }
            if (hit)
            {
                nHyphenationPos = i;
                if (rep && rep[i])
                {
                    nHyphenationPosAlt = i - pos[i];
                    nHyphenationPosAltHyph = i + leftrep - pos[i];
                }
            }
        }

        if (nHyphenationPos  == -1)
        {
            xRes = nullptr;
        }
        else
        {
            if (rep && rep[nHyphenationPos])
            {
                // remove equal sign
                char * s = rep[nHyphenationPos];
                int eq = 0;
                for (; *s; s++)
                {
                    if (*s == '=') eq = 1;
                    if (eq) *s = *(s + 1);
                }
                OUString repHyphlow(rep[nHyphenationPos], strlen(rep[nHyphenationPos]), eEnc);
                OUString repHyph;
                switch (ct)
                {
                    case CapType::ALLCAP:
                    {
                        repHyph = makeUpperCase(repHyphlow, pCC);
                        break;
                    }
                    case CapType::INITCAP:
                    {
                        if (nHyphenationPosAlt == -1)
                            repHyph = makeInitCap(repHyphlow, pCC);
                        else
                             repHyph = repHyphlow;
                        break;
                    }
                    default:
                    {
                        repHyph = repHyphlow;
                        break;
                    }
                }

                // handle shortening
                sal_Int16 nPos = static_cast<sal_Int16>((nHyphenationPosAltHyph < nHyphenationPos) ?
                nHyphenationPosAltHyph : nHyphenationPos);
                // discretionary hyphenation
                xRes = HyphenatedWord::CreateHyphenatedWord( aWord, LinguLocaleToLanguage( aLocale ), nPos,
                    aWord.replaceAt(nHyphenationPosAlt + 1, cut[nHyphenationPos], repHyph),
                    static_cast<sal_Int16>(nHyphenationPosAltHyph));
            }
            else
            {
                xRes = HyphenatedWord::CreateHyphenatedWord( aWord, LinguLocaleToLanguage( aLocale ),
                    static_cast<sal_Int16>(nHyphenationPos), aWord, static_cast<sal_Int16>(nHyphenationPos));
            }
        }

        if (rep)
        {
            for(int j = 0; j < n; j++)
            {
                if (rep[j]) free(rep[j]);
            }
            free(rep);
        }
        if (pos) free(pos);
        if (cut) free(cut);
        return xRes;
    }
    return nullptr;
}

Reference < XHyphenatedWord > SAL_CALL Hyphenator::queryAlternativeSpelling(
        const OUString& aWord,
        const css::lang::Locale& aLocale,
        sal_Int16 nIndex,
        const css::uno::Sequence< css::beans::PropertyValue >& aProperties )
{
    // Firstly we allow only one plus character before the hyphen to avoid to miss the right break point:
    for (int extrachar = 1; extrachar <= 2; extrachar++)
    {
        Reference< XHyphenatedWord > xRes = hyphenate(aWord, aLocale, nIndex + 1 + extrachar, aProperties);
        if (xRes.is() && xRes->isAlternativeSpelling() && xRes->getHyphenationPos() == nIndex)
            return xRes;
    }
    return nullptr;
}

Reference< XPossibleHyphens > SAL_CALL Hyphenator::createPossibleHyphens( const OUString& aWord,
        const css::lang::Locale& aLocale,
        const css::uno::Sequence< css::beans::PropertyValue >& aProperties )
{
    PropertyHelper_Hyphenation& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    // Resolves: fdo#41083 honour MinWordLength in "createPossibleHyphens" as
    // well as "hyphenate"
    if (aWord.getLength() < minLen)
    {
        return PossibleHyphens::CreatePossibleHyphens( aWord, LinguLocaleToLanguage( aLocale ),
                      aWord, Sequence< sal_Int16 >() );
    }

    int k = -1;
    for (size_t j = 0; j < mvDicts.size(); ++j)
    {
        if (aLocale == mvDicts[j].aLoc)
            k = j;
    }

    // if we have a hyphenation dictionary matching this locale
    if (k != -1)
    {
        HyphenDict *dict = nullptr;
        // if this dictionary has not been loaded yet do that
        if (!mvDicts[k].aPtr)
        {
            if (!LoadDictionary(mvDicts[k]))
                return nullptr;
        }

        // other wise hyphenate the word with that dictionary
        dict = mvDicts[k].aPtr;
        rtl_TextEncoding eEnc = mvDicts[k].eEnc;
        CharClass* pCC = mvDicts[k].apCC.get();

        // we don't want to work with a default text encoding since following incorrect
        // results may occur only for specific text and thus may be hard to notice.
        // Thus better always make a clean exit here if the text encoding is in question.
        // Hopefully something not working at all will raise proper attention quickly. ;-)
        DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
        if (eEnc == RTL_TEXTENCODING_DONTKNOW)
            return nullptr;

        // first handle smart quotes both single and double
        OUStringBuffer rBuf(aWord);
        sal_Int32 nc = rBuf.getLength();
        sal_Unicode ch;
        for (sal_Int32 ix=0; ix < nc; ix++)
        {
            ch = rBuf[ix];
            if ((ch == 0x201C) || (ch == 0x201D))
                rBuf[ix] = u'"';
            if ((ch == 0x2018) || (ch == 0x2019))
                rBuf[ix] = u'\'';
        }
        OUString nWord(rBuf.makeStringAndClear());

        // now convert word to all lowercase for pattern recognition
        OUString nTerm(makeLowerCase(nWord, pCC));

        // now convert word to needed encoding
        OString encWord(OU2ENC(nTerm,eEnc));

        sal_Int32 wordlen = encWord.getLength();
        std::unique_ptr<char[]> lcword(new char[wordlen+1]);
        std::unique_ptr<char[]> hyphens(new char[wordlen+5]);
        char ** rep = nullptr; // replacements of discretionary hyphenation
        int * pos = nullptr; // array of [hyphenation point] minus [deletion position]
        int * cut = nullptr; // length of deletions in original word

        // copy converted word into simple char buffer
        strcpy(lcword.get(),encWord.getStr());

        // first remove any trailing periods
        sal_Int32 n = wordlen-1;
        while((n >=0) && (lcword[n] == '.'))
            n--;
        n++;
        if (n > 0)
        {
            const bool bFailed = 0 != hnj_hyphen_hyphenate3(dict, lcword.get(), n, hyphens.get(), nullptr,
                    &rep, &pos, &cut, minLead, minTrail,
                    std::max<sal_Int16>(dict->clhmin, std::max<sal_Int16>(dict->clhmin, 2) + std::max(0, minLead - std::max<sal_Int16>(dict->lhmin, 2))),
                    std::max<sal_Int16>(dict->crhmin, std::max<sal_Int16>(dict->crhmin, 2) + std::max(0, minTrail - std::max<sal_Int16>(dict->rhmin, 2))) );
            if (bFailed)
            {
                if (rep)
                {
                    for(int j = 0; j < n; j++)
                    {
                        if (rep[j]) free(rep[j]);
                    }
                    free(rep);
                }
                if (pos) free(pos);
                if (cut) free(cut);

                return nullptr;
            }
        }
        // now backfill hyphens[] for any removed periods
        for (sal_Int32 c = n; c < wordlen; c++)
            hyphens[c] = '0';
        hyphens[wordlen] = '\0';

        sal_Int32 nHyphCount = 0;

        for ( sal_Int32 i = 0; i < encWord.getLength(); i++)
        {
            if (hyphens[i]&1)
                nHyphCount++;
        }

        Sequence< sal_Int16 > aHyphPos(nHyphCount);
        sal_Int16 *pPos = aHyphPos.getArray();
        OUStringBuffer hyphenatedWordBuffer;
        nHyphCount = 0;

        for (sal_Int32 i = 0; i < nWord.getLength(); i++)
        {
            hyphenatedWordBuffer.append(aWord[i]);
            // hyphenation position
            if (hyphens[i]&1)
            {
                // linguistic::PossibleHyphens is stuck with
                // css::uno::Sequence<sal_Int16> because of
                // css.linguistic2.XPossibleHyphens.getHyphenationPositions, so
                // any further positions need to be ignored:
                assert(i >= SAL_MIN_INT16);
                if (i > SAL_MAX_INT16)
                {
                    SAL_WARN(
                        "lingucomponent",
                        "hyphen pos " << i << " > SAL_MAX_INT16 in \"" << aWord
                            << "\"");
                    continue;
                }
                pPos[nHyphCount] = i;
                hyphenatedWordBuffer.append('=');
                nHyphCount++;
            }
        }

        OUString hyphenatedWord = hyphenatedWordBuffer.makeStringAndClear();

        Reference< XPossibleHyphens > xRes = PossibleHyphens::CreatePossibleHyphens(
            aWord, LinguLocaleToLanguage( aLocale ), hyphenatedWord, aHyphPos);

        if (rep)
        {
            for(int j = 0; j < n; j++)
            {
                if (rep[j]) free(rep[j]);
            }
            free(rep);
        }
        if (pos) free(pos);
        if (cut) free(cut);

        return xRes;
    }

    return nullptr;
}

OUString Hyphenator::makeLowerCase(const OUString& aTerm, CharClass const * pCC)
{
    if (pCC)
        return pCC->lowercase(aTerm);
    return aTerm;
}

OUString Hyphenator::makeUpperCase(const OUString& aTerm, CharClass const * pCC)
{
    if (pCC)
        return pCC->uppercase(aTerm);
    return aTerm;
}

OUString Hyphenator::makeInitCap(const OUString& aTerm, CharClass const * pCC)
{
    sal_Int32 tlen = aTerm.getLength();
    if (pCC && tlen)
    {
        OUString bTemp = aTerm.copy(0,1);
        if (tlen > 1)
            return ( pCC->uppercase(bTemp, 0, 1) + pCC->lowercase(aTerm,1,(tlen-1)) );

        return pCC->uppercase(bTemp, 0, 1);
    }
    return aTerm;
}

/// @throws Exception
static Reference< XInterface > Hyphenator_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
{
    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new Hyphenator);
    return xService;
}

sal_Bool SAL_CALL Hyphenator::addLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

sal_Bool SAL_CALL Hyphenator::removeLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

OUString SAL_CALL Hyphenator::getServiceDisplayName(const Locale& rLocale)
{
    std::locale loc(Translate::Create("svt", LanguageTag(rLocale)));
    return Translate::get(STR_DESCRIPTION_LIBHYPHEN, loc);
}

void SAL_CALL Hyphenator::initialize( const Sequence< Any >& rArguments )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
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
            pPropHelper.reset( new PropertyHelper_Hyphenation( static_cast<XHyphenator *>(this), xPropSet ) );
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}

void SAL_CALL Hyphenator::dispose()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( static_cast<XHyphenator *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
        if (pPropHelper)
        {
            pPropHelper->RemoveAsPropListener();
            pPropHelper.reset();
        }
    }
}

void SAL_CALL Hyphenator::addEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL Hyphenator::removeEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL Hyphenator::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL Hyphenator::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL Hyphenator::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Sequence< OUString > Hyphenator::getSupportedServiceNames_Static()
        throw()
{
    Sequence< OUString > aSNS { SN_HYPHENATOR };
    return aSNS;
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * hyphen_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    if ( Hyphenator::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                Hyphenator::getImplementationName_Static(),
                Hyphenator_CreateInstance,
                Hyphenator::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
