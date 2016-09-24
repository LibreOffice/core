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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>

#include "nthesimp.hxx"
#include <linguistic/misc.hxx>
#include <linguistic/lngprops.hxx>
#include "nthesdta.hxx"

#include <list>
#include <set>
#include <string.h>

// XML-header to query SPELLML support
#define SPELLML_SUPPORT "<?xml?>"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

static uno::Reference< XLinguServiceManager2 > GetLngSvcMgr_Impl()
{
    uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
    uno::Reference< XLinguServiceManager2 > xRes = LinguServiceManager::create( xContext ) ;
    return xRes;
}

Thesaurus::Thesaurus() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = false;
    pPropHelper = nullptr;
    aThes = nullptr;
    aCharSetInfo = nullptr;
    aTEncs = nullptr;
    aTLocs = nullptr;
    aTNames = nullptr;
    numthes = 0;
    prevLocale = LANGUAGE_DONTKNOW;
}

Thesaurus::~Thesaurus()
{
    if (aThes)
    {
        for (int i = 0; i < numthes; i++)
        {
            if (aThes[i]) delete aThes[i];
            aThes[i] = nullptr;
        }
        delete[] aThes;
    }
    aThes = nullptr;
    if (aCharSetInfo)
    {
        for (int i = 0; i < numthes; i++)
        {
            if (aCharSetInfo[i]) delete aCharSetInfo[i];
            aCharSetInfo[i] = nullptr;
        }
        delete[] aCharSetInfo;
    }
    aCharSetInfo = nullptr;
    numthes = 0;
    if (aTEncs) delete[] aTEncs;
    aTEncs = nullptr;
    if (aTLocs) delete[] aTLocs;
    aTLocs = nullptr;
    if (aTNames) delete[] aTNames;
    aTNames = nullptr;

    if (pPropHelper)
    {
        pPropHelper->RemoveAsPropListener();
        delete pPropHelper;
    }
}

PropertyHelper_Thesaurus& Thesaurus::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Thesaurus( static_cast<XThesaurus *>(this), xPropSet );
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}

Sequence< Locale > SAL_CALL Thesaurus::getLocales()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.
    if (!numthes)
    {
        SvtLinguConfig aLinguCfg;

        // get list of dictionaries-to-use
        std::list< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( "Thesauri",
                "org.openoffice.lingu.new.Thesaurus", aFormatList );
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
                GetOldStyleDics( "THES" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list od new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        numthes = aDics.size();
        if (numthes)
        {
            // get supported locales from the dictionaries-to-use...
            sal_Int32 k = 0;
            std::set< OUString, lt_rtl_OUString > aLocaleNamesSet;
            std::list< SvtLinguConfigDictionaryEntry >::const_iterator aDictIt;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                sal_Int32 nLen2 = aLocaleNames.getLength();
                for (k = 0;  k < nLen2;  ++k)
                {
                    aLocaleNamesSet.insert( aLocaleNames[k] );
                }
            }
            // ... and add them to the resulting sequence
            aSuppLocales.realloc( aLocaleNamesSet.size() );
            std::set< OUString, lt_rtl_OUString >::const_iterator aItB;
            k = 0;
            for (aItB = aLocaleNamesSet.begin();  aItB != aLocaleNamesSet.end();  ++aItB)
            {
                Locale aTmp( LanguageTag::convertToLocale( *aItB ));
                aSuppLocales[k++] = aTmp;
            }

            //! For each dictionary and each locale we need a separate entry.
            //! If this results in more than one dictionary per locale than (for now)
            //! it is undefined which dictionary gets used.
            //! In the future the implementation should support using several dictionaries
            //! for one locale.
            numthes = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
                numthes = numthes + aDictIt->aLocaleNames.getLength();

            // add dictionary information
            aThes   = new MyThes* [numthes];
            aTEncs  = new rtl_TextEncoding [numthes];
            aTLocs  = new Locale [numthes];
            aTNames = new OUString [numthes];
            aCharSetInfo = new CharClass* [numthes];

            k = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                if (aDictIt->aLocaleNames.getLength() > 0 &&
                    aDictIt->aLocations.getLength() > 0)
                {
                    uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                    sal_Int32 nLocales = aLocaleNames.getLength();

                    // currently only one language per dictionary is supported in the actual implementation...
                    // Thus here we work-around this by adding the same dictionary several times.
                    // Once for each of its supported locales.
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        LanguageTag aLanguageTag( aDictIt->aLocaleNames[i] );
                        aThes[k]  = nullptr;
                        aTEncs[k]  = RTL_TEXTENCODING_DONTKNOW;
                        aTLocs[k]  = aLanguageTag.getLocale();
                        aCharSetInfo[k] = new CharClass( aLanguageTag );
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = aDictIt->aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        aTNames[k] = aLocation;

                        ++k;
                    }
                }
            }
            DBG_ASSERT( k == numthes, "index mismatch?" );
        }
        else
        {
            /* no dictionary found so register no dictionaries */
            numthes = 0;
            aThes  = nullptr;
            aTEncs  = nullptr;
            aTLocs  = nullptr;
            aTNames = nullptr;
            aCharSetInfo = nullptr;
            aSuppLocales.realloc(0);
        }
    }

    return aSuppLocales;
}

sal_Bool SAL_CALL Thesaurus::hasLocale(const Locale& rLocale)
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!aSuppLocales.getLength())
        getLocales();
    sal_Int32 nLen = aSuppLocales.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const Locale *pLocale = aSuppLocales.getConstArray();
        if (rLocale == pLocale[i])
        {
            bRes = true;
            break;
        }
    }
    return bRes;
}

Sequence < Reference < css::linguistic2::XMeaning > > SAL_CALL Thesaurus::queryMeanings(
    const OUString& qTerm, const Locale& rLocale,
    const PropertyValues& rProperties)
    throw(IllegalArgumentException, RuntimeException, std::exception)
{
    MutexGuard      aGuard( GetLinguMutex() );

    uno::Sequence< Reference< XMeaning > > aMeanings( 1 );
    uno::Sequence< Reference< XMeaning > > noMeanings( 0 );
    uno::Reference< XLinguServiceManager2 > xLngSvcMgr( GetLngSvcMgr_Impl() );
    uno::Reference< XSpellChecker1 > xSpell;

    OUString aRTerm(qTerm);
    OUString aPTerm(qTerm);
    CapType ct = CapType::UNKNOWN;
    sal_Int32 stem = 0;
    sal_Int32 stem2 = 0;

    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );

    if (LinguIsUnspecified( nLanguage) || aRTerm.isEmpty())
        return noMeanings;

    if (!hasLocale( rLocale ))
#ifdef LINGU_EXCEPTIONS
        throw( IllegalArgumentException() );
#else
        return noMeanings;
#endif

    if (prevTerm == qTerm && prevLocale == nLanguage)
        return prevMeanings;

    mentry * pmean = nullptr;
    sal_Int32 nmean = 0;

    PropertyHelper_Thesaurus &rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    MyThes * pTH = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;
    CharClass * pCC = nullptr;

    // find the first thesaurus that matches the locale
    for (int i =0; i < numthes; i++)
    {
        if (rLocale == aTLocs[i])
        {
            // open up and intialize this thesaurus if need be
            if (!aThes[i])
            {
                OUString datpath = aTNames[i] + ".dat";
                OUString idxpath = aTNames[i] + ".idx";
                OUString ndat;
                OUString nidx;
                osl::FileBase::getSystemPathFromFileURL(datpath,ndat);
                osl::FileBase::getSystemPathFromFileURL(idxpath,nidx);

#if defined(_WIN32)
                // MyThes waits UTF-8 encoded paths with \\?\ long path prefix.
                OString aTmpidx = Win_AddLongPathPrefix(OUStringToOString(nidx, RTL_TEXTENCODING_UTF8));
                OString aTmpdat = Win_AddLongPathPrefix(OUStringToOString(ndat, RTL_TEXTENCODING_UTF8));
#else
                OString aTmpidx(OU2ENC(nidx,osl_getThreadTextEncoding()));
                OString aTmpdat(OU2ENC(ndat,osl_getThreadTextEncoding()));
#endif

                aThes[i] = new MyThes(aTmpidx.getStr(),aTmpdat.getStr());
                if (aThes[i])
                    aTEncs[i] = getTextEncodingFromCharset(aThes[i]->get_th_encoding());
            }
            pTH = aThes[i];
            eEnc = aTEncs[i];
            pCC = aCharSetInfo[i];

            if (pTH)
                break;
        }
    }

    // we don't want to work with a default text encoding since following incorrect
    // results may occur only for specific text and thus may be hard to notice.
    // Thus better always make a clean exit here if the text encoding is in question.
    // Hopefully something not working at all will raise proper attention quickly. ;-)
    DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
    if (eEnc == RTL_TEXTENCODING_DONTKNOW)
        return noMeanings;

    while (pTH)
    {
        // convert word to all lower case for searching
        if (!stem)
            ct = capitalType(aRTerm, pCC);
        OUString nTerm(makeLowerCase(aRTerm, pCC));
        OString aTmp( OU2ENC(nTerm, eEnc) );
        nmean = pTH->Lookup(aTmp.getStr(),aTmp.getLength(),&pmean);

        if (nmean)
            aMeanings.realloc( nmean );

        mentry * pe = pmean;
        OUString codeTerm = qTerm;
        Reference< XSpellAlternatives > xTmpRes2;

        if (stem)
        {
            xTmpRes2 = xSpell->spell( "<?xml?><query type='analyze'><word>" +
                                      aPTerm + "</word></query>", nLanguage, rProperties );
            if (xTmpRes2.is())
            {
                Sequence<OUString>seq = xTmpRes2->getAlternatives();
                if (seq.getLength() > 0)
                {
                    codeTerm = seq[0];
                    stem2 = 1;
                }
            }
        }

        for (int j = 0; j < nmean; j++)
        {
            int count = pe->count;
            if (count)
            {
                Sequence< OUString > aStr( count );
                OUString *pStr = aStr.getArray();

                for (int i=0; i < count; i++)
                {
                    OUString sTerm(pe->psyns[i],strlen(pe->psyns[i]),eEnc );
                    sal_Int32 catpos = sTerm.indexOf('(');
                    OUString catst;
                    if (catpos > 2)
                    {
                        // remove category name for affixation and casing
                        catst = " " + sTerm.copy(catpos);
                        sTerm = sTerm.copy(0, catpos);
                        sTerm = sTerm.trim();
                    }
                    // generate synonyms with affixes
                    if (stem && stem2)
                    {
                        Reference< XSpellAlternatives > xTmpRes;
                        xTmpRes = xSpell->spell( "<?xml?><query type='generate'><word>" +
                        sTerm + "</word>" + codeTerm + "</query>", nLanguage, rProperties );
                        if (xTmpRes.is())
                        {
                            Sequence<OUString>seq = xTmpRes->getAlternatives();
                            if (seq.getLength() > 0)
                                sTerm = seq[0];
                        }
                    }

                    CapType ct1 = capitalType(sTerm, pCC);
                    if (CapType::MIXED == ct1)
                        ct = ct1;
                    OUString cTerm;
                    switch (ct)
                    {
                        case CapType::ALLCAP:
                            cTerm = makeUpperCase(sTerm, pCC);
                            break;
                        case CapType::INITCAP:
                            cTerm = makeInitCap(sTerm, pCC);
                            break;
                        default:
                            cTerm = sTerm;
                            break;
                    }
                    OUString aAlt( cTerm + catst);
                    pStr[i] = aAlt;
                }
                Meaning * pMn = new Meaning(aRTerm);
                OUString dTerm(pe->defn,strlen(pe->defn),eEnc );
                pMn->SetMeaning(dTerm);
                pMn->SetSynonyms(aStr);
                Reference<XMeaning>* pMeaning = aMeanings.getArray();
                pMeaning[j] = pMn;
            }
            pe++;
        }
        pTH->CleanUpAfterLookup(&pmean,nmean);

        if (nmean)
        {
            prevTerm = qTerm;
            prevMeanings = aMeanings;
            prevLocale = nLanguage;
            return aMeanings;
        }

        if (stem || !xLngSvcMgr.is())
            return noMeanings;
        stem = 1;

        xSpell.set( xLngSvcMgr->getSpellChecker(), UNO_QUERY );
        if (!xSpell.is() || !xSpell->isValid( SPELLML_SUPPORT, nLanguage, rProperties ))
            return noMeanings;
        Reference< XSpellAlternatives > xTmpRes;
        xTmpRes = xSpell->spell( "<?xml?><query type='stem'><word>" +
            aRTerm + "</word></query>", nLanguage, rProperties );
        if (xTmpRes.is())
        {
            Sequence<OUString>seq = xTmpRes->getAlternatives();
            if (seq.getLength() > 0)
            {
                aRTerm = seq[0];  // XXX Use only the first stem
                continue;
            }
        }

        // stem the last word of the synonym (for categories after affixation)
        aRTerm = aRTerm.trim();
        sal_Int32 pos = aRTerm.lastIndexOf(' ');
        if (!pos)
            return noMeanings;
        xTmpRes = xSpell->spell( "<?xml?><query type='stem'><word>" +
            aRTerm.copy(pos + 1) + "</word></query>", nLanguage, rProperties );
        if (xTmpRes.is())
        {
            Sequence<OUString>seq = xTmpRes->getAlternatives();
            if (seq.getLength() > 0)
            {
                aPTerm = aRTerm.copy(pos + 1);
                aRTerm = aRTerm.copy(0, pos + 1) + seq[0];
#if  0
                for (int i = 0; i < seq.getLength(); i++)
                {
                    OString o = OUStringToOString(seq[i], RTL_TEXTENCODING_UTF8);
                    fprintf(stderr, "%d: %s\n", i + 1, o.pData->buffer);
                }
#endif
                continue;
            }
        }
        break;
    }
    return noMeanings;
}

Reference< XInterface > SAL_CALL Thesaurus_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new Thesaurus);
    return xService;
}

OUString SAL_CALL Thesaurus::getServiceDisplayName( const Locale& /*rLocale*/ )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return OUString( "Mythes Thesaurus" );
}

void SAL_CALL Thesaurus::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (1 == nLen)
        {
            Reference< XLinguProperties >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Thesaurus( static_cast<XThesaurus *>(this), xPropSet );
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            OSL_FAIL( "wrong number of arguments in sequence" );
    }
}

OUString SAL_CALL Thesaurus::makeLowerCase(const OUString& aTerm, CharClass * pCC)
{
    if (pCC)
        return pCC->lowercase(aTerm);
    return aTerm;
}

OUString SAL_CALL Thesaurus::makeUpperCase(const OUString& aTerm, CharClass * pCC)
{
    if (pCC)
        return pCC->uppercase(aTerm);
    return aTerm;
}

OUString SAL_CALL Thesaurus::makeInitCap(const OUString& aTerm, CharClass * pCC)
{
    sal_Int32 tlen = aTerm.getLength();
    if ((pCC) && (tlen))
    {
        OUString bTemp = aTerm.copy(0,1);
        if (tlen > 1)
        {
            return ( pCC->uppercase(bTemp, 0, 1)
                     + pCC->lowercase(aTerm,1,(tlen-1)) );
        }

        return pCC->uppercase(bTemp, 0, 1);
    }
    return aTerm;
}

void SAL_CALL Thesaurus::dispose()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( static_cast<XThesaurus *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
        if (pPropHelper)
        {
            pPropHelper->RemoveAsPropListener();
            delete pPropHelper;
            pPropHelper = nullptr;
        }
    }
}

void SAL_CALL Thesaurus::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL Thesaurus::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL Thesaurus::getImplementationName()
        throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL Thesaurus::supportsService( const OUString& ServiceName )
        throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL Thesaurus::getSupportedServiceNames()
        throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

Sequence< OUString > Thesaurus::getSupportedServiceNames_Static()
        throw()
{
    Sequence< OUString > aSNS { SN_THESAURUS };
    return aSNS;
}

void * SAL_CALL Thesaurus_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = nullptr;
    if ( Thesaurus::getImplementationName_Static().equalsAscii( pImplName ) )
    {

        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                Thesaurus::getImplementationName_Static(),
                Thesaurus_CreateInstance,
                Thesaurus::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
