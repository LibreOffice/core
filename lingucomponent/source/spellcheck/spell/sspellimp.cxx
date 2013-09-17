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
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <lingutil.hxx>
#include <hunspell.hxx>
#include <dictmgr.hxx>
#include <sspellimp.hxx>

#include <linguistic/lngprops.hxx>
#include <linguistic/spelldta.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>

#include <list>
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
#define SPELLML_HEADER "<?xml?>"

///////////////////////////////////////////////////////////////////////////

SpellChecker::SpellChecker() :
    aDicts(NULL),
    aDEncs(NULL),
    aDLocs(NULL),
    aDNames(NULL),
    numdict(0),
    aEvtListeners(GetLinguMutex()),
    pPropHelper(NULL),
    bDisposing(false)
{
}

SpellChecker::~SpellChecker()
{
    if (aDicts)
    {
       for (int i = 0; i < numdict; ++i)
       {
            delete aDicts[i];
       }
       delete[] aDicts;
    }
    delete[] aDEncs;
    delete[] aDLocs;
    delete[] aDNames;
    if (pPropHelper)
    {
        pPropHelper->RemoveAsPropListener();
        delete pPropHelper;
    }
}

PropertyHelper_Spelling & SpellChecker::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Spelling( (XSpellChecker *) this, xPropSet );
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}


Sequence< Locale > SAL_CALL SpellChecker::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.

    if (!numdict)
    {
        SvtLinguConfig aLinguCfg;

        // get list of extension dictionaries-to-use
        // (or better speaking: the list of dictionaries using the
        // new configuration entries).
        std::list< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( "SpellCheckers",
                "org.openoffice.lingu.MySpellSpellChecker", aFormatList );
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
                GetOldStyleDics( "DICT" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list od new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        if (!aDics.empty())
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
            numdict = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
                numdict = numdict + aDictIt->aLocaleNames.getLength();

            // add dictionary information
            aDicts  = new Hunspell* [numdict];
            aDEncs  = new rtl_TextEncoding [numdict];
            aDLocs  = new Locale [numdict];
            aDNames = new OUString [numdict];
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
                    // Once for each of it's supported locales.
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        aDicts[k]  = NULL;
                        aDEncs[k]  = RTL_TEXTENCODING_DONTKNOW;
                        aDLocs[k]  = LanguageTag::convertToLocale( aLocaleNames[i] );
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = aDictIt->aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        aDNames[k] = aLocation;

                        ++k;
                    }
                }
            }
            DBG_ASSERT( k == numdict, "index mismatch?" );
        }
        else
        {
            /* no dictionary found so register no dictionaries */
            numdict = 0;
            delete[] aDicts;
            aDicts  = NULL;
            delete[] aDEncs;
            aDEncs = NULL;
            delete[] aDLocs;
            aDLocs  = NULL;
            delete[] aDNames;
            aDNames = NULL;
            aSuppLocales.realloc(0);
        }
    }

    return aSuppLocales;
}


sal_Bool SAL_CALL SpellChecker::hasLocale(const Locale& rLocale)
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!aSuppLocales.getLength())
        getLocales();

    const Locale *pLocale = aSuppLocales.getConstArray();
    sal_Int32 nLen = aSuppLocales.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (rLocale == pLocale[i])
        {
            bRes = sal_True;
            break;
        }
    }
    return bRes;
}


sal_Int16 SpellChecker::GetSpellFailure( const OUString &rWord, const Locale &rLocale )
{
    Hunspell * pMS = NULL;
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
            rBuf[ix] = (sal_Unicode)0x0022;
        else if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = (sal_Unicode)0x0027;

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
        for (sal_Int32 i = 0; i < numdict; ++i)
        {
            pMS = NULL;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == aDLocs[i])
            {
                if (!aDicts[i])
                {
                    OUString dicpath = aDNames[i] + ".dic";
                    OUString affpath = aDNames[i] + ".aff";
                    OUString dict;
                    OUString aff;
                    osl::FileBase::getSystemPathFromFileURL(dicpath,dict);
                    osl::FileBase::getSystemPathFromFileURL(affpath,aff);
                    OString aTmpaff(OU2ENC(aff,osl_getThreadTextEncoding()));
                    OString aTmpdict(OU2ENC(dict,osl_getThreadTextEncoding()));

#if defined(WNT)
                    // workaround for Windows specifc problem that the
                    // path length in calls to 'fopen' is limted to somewhat
                    // about 120+ characters which will usually be exceed when
                    // using dictionaries as extensions.
                    aTmpaff = Win_GetShortPathName( aff );
                    aTmpdict = Win_GetShortPathName( dict );
#endif

                    aDicts[i] = new Hunspell(aTmpaff.getStr(),aTmpdict.getStr());
                    aDEncs[i] = RTL_TEXTENCODING_DONTKNOW;
                    if (aDicts[i])
                        aDEncs[i] = getTextEncodingFromCharset(aDicts[i]->get_dic_encoding());
                }
                pMS = aDicts[i];
                eEnc = aDEncs[i];
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
                int rVal = pMS->spell((char*)aWrd.getStr());
                if (rVal != 1) {
                    if (extrachar && (eEnc != RTL_TEXTENCODING_UTF8)) {
                        OUStringBuffer mBuf(nWord);
                        n = mBuf.getLength();
                        for (sal_Int32 ix=n-1; ix >= 0; ix--)
                        {
                          switch (mBuf[ix]) {
                            case 0xFB00: mBuf.remove(ix, 1); mBuf.insert(ix, "ff"); break;
                            case 0xFB01: mBuf.remove(ix, 1); mBuf.insert(ix, "fi"); break;
                            case 0xFB02: mBuf.remove(ix, 1); mBuf.insert(ix, "fl"); break;
                            case 0xFB03: mBuf.remove(ix, 1); mBuf.insert(ix, "ffi"); break;
                            case 0xFB04: mBuf.remove(ix, 1); mBuf.insert(ix, "ffl"); break;
                            case 0x200C:
                            case 0x200D: mBuf.remove(ix, 1); break;
                          }
                        }
                        OUString mWord(mBuf.makeStringAndClear());
                        OString bWrd(OU2ENC(mWord, eEnc));
                        rVal = pMS->spell((char*)bWrd.getStr());
                        if (rVal == 1) return -1;
                    }
                    nRes = SpellFailure::SPELLING_ERROR;
                } else {
                    return -1;
                }
                pMS = NULL;
            }
        }
    }

    return nRes;
}


sal_Bool SAL_CALL SpellChecker::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  rWord.isEmpty())
        return sal_True;

    if (!hasLocale( rLocale ))
        return sal_True;

    // return sal_False to process SPELLML requests (they are longer than the header)
    if (rWord.match(SPELLML_HEADER, 0) && (rWord.getLength() > 10)) return sal_False;

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simplier solution than the provided
    // one using the PropertyHelper_Spell.

    PropertyHelper_Spelling& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1 && !rWord.match(SPELLML_HEADER, 0))
    {
        sal_Int16 nLang = LinguLocaleToLanguage( rLocale );
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

    Hunspell* pMS = NULL;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // first handle smart quotes (single and double)
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = (sal_Unicode)0x0022;
        if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = (sal_Unicode)0x0027;
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        sal_Int16 nLang = LinguLocaleToLanguage( rLocale );
        int numsug = 0;

        Sequence< OUString > aStr( 0 );
        for (int i = 0; i < numdict; i++)
        {
            pMS = NULL;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == aDLocs[i])
            {
                pMS = aDicts[i];
                eEnc = aDEncs[i];
            }

            if (pMS)
            {
                char ** suglst = NULL;
                OString aWrd(OU2ENC(nWord,eEnc));
                int count = pMS->suggest(&suglst, (const char *) aWrd.getStr());

                if (count)
                {
                    aStr.realloc( numsug + count );
                    OUString *pStr = aStr.getArray();
                    for (int ii=0; ii < count; ++ii)
                    {
                        OUString cvtwrd(suglst[ii],strlen(suglst[ii]),eEnc);
                        pStr[numsug + ii] = cvtwrd;
                    }
                    pMS->free_list(&suglst, count);
                    numsug += count;
                }
            }
        }

        // now return an empty alternative for no suggestions or the list of alternatives if some found
        OUString aTmp(rWord);
        xRes = SpellAlternatives::CreateSpellAlternatives( aTmp, nLang, SpellFailure::SPELLING_ERROR, aStr );
        return xRes;
    }
    return xRes;
}


Reference< XSpellAlternatives > SAL_CALL SpellChecker::spell(
        const OUString& rWord, const Locale& rLocale,
        const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  rWord.isEmpty())
        return NULL;

    if (!hasLocale( rLocale ))
        return NULL;

    Reference< XSpellAlternatives > xAlt;
    if (!isValid( rWord, rLocale, rProperties ))
    {
        xAlt =  GetProposals( rWord, rLocale );
    }
    return xAlt;
}


Reference< XInterface > SAL_CALL SpellChecker_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{

    Reference< XInterface > xService = (cppu::OWeakObject*) new SpellChecker;
    return xService;
}


sal_Bool SAL_CALL SpellChecker::addLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}


sal_Bool SAL_CALL SpellChecker::removeLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}


OUString SAL_CALL SpellChecker::getServiceDisplayName( const Locale& /*rLocale*/ )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return OUString( "Hunspell SpellChecker" );
}


void SAL_CALL SpellChecker::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference< XLinguProperties >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;
            //rArguments.getConstArray()[1] >>= xDicList;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Spelling( (XSpellChecker *) this, xPropSet );
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}


void SAL_CALL SpellChecker::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( (XSpellChecker *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        if (pPropHelper)
        {
            pPropHelper->RemoveAsPropListener();
            delete pPropHelper;
            pPropHelper = NULL;
        }
    }
}


void SAL_CALL SpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL SpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

OUString SAL_CALL SpellChecker::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getImplementationName_Static();
}


sal_Bool SAL_CALL SpellChecker::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}


Sequence< OUString > SAL_CALL SpellChecker::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getSupportedServiceNames_Static();
}


Sequence< OUString > SpellChecker::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aSNS( 1 ); // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = SN_SPELLCHECKER;
    return aSNS;
}

void * SAL_CALL SpellChecker_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !SpellChecker::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                SpellChecker::getImplementationName_Static(),
                SpellChecker_CreateInstance,
                SpellChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


///////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
