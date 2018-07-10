/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include "macspellimp.hxx"

#include <linguistic/spelldta.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

MacSpellChecker::MacSpellChecker() :
    aEvtListeners( GetLinguMutex() )
{
    aDEncs = nullptr;
    aDLocs = nullptr;
    aDNames = nullptr;
    bDisposing = false;
    pPropHelper = nullptr;
    numdict = 0;
    NSApplicationLoad();
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    macTag = [NSSpellChecker uniqueSpellDocumentTag];
    [pool release];
}


MacSpellChecker::~MacSpellChecker()
{
  numdict = 0;
  if (aDEncs) delete[] aDEncs;
  aDEncs = nullptr;
  if (aDLocs) delete[] aDLocs;
  aDLocs = nullptr;
  if (aDNames) delete[] aDNames;
  aDNames = nullptr;
  if (pPropHelper)
     pPropHelper->RemoveAsPropListener();
}


PropertyHelper_Spell & MacSpellChecker::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Spell( static_cast<XSpellChecker *>(this), xPropSet );
        xPropHelper = pPropHelper;
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}


Sequence< Locale > SAL_CALL MacSpellChecker::getLocales()
{
    MutexGuard  aGuard( GetLinguMutex() );

        // this routine should return the locales supported by the installed
        // dictionaries.  So here we need to parse both the user edited
        // dictionary list and the shared dictionary list
        // to see what dictionaries the admin/user has installed

        int numshr;          // number of shared dictionary entries
        rtl_TextEncoding aEnc = RTL_TEXTENCODING_UTF8;

        std::vector<NSString *> postspdict;

    if (!numdict) {

        // invoke a dictionary manager to get the user dictionary list
        // TODO How on Mac OS X?

        // invoke a second  dictionary manager to get the shared dictionary list
        NSArray *aSpellCheckLanguages = [[NSSpellChecker sharedSpellChecker] availableLanguages];

        for (NSUInteger i = 0; i < [aSpellCheckLanguages count]; i++)
        {
            NSString* pLangStr = static_cast<NSString*>([aSpellCheckLanguages objectAtIndex:i]);

            // Fix up generic languages (without territory code) and odd combinations that LO
            // doesn't handle.
            if ([pLangStr isEqualToString:@"da"])
            {
                postspdict.push_back( @"da_DK" );
            }
            else if ([pLangStr isEqualToString:@"de"])
            {
                const std::vector<NSString*> aDE
                    { @"AT", @"BE", @"CH", @"DE", @"LI", @"LU" };
                for (auto c: aDE)
                {
                    pLangStr = [@"de_" stringByAppendingString: c];
                    postspdict.push_back( pLangStr );
                }
            }
            else if ([pLangStr isEqualToString:@"en"])
            {
                // System has en_AU, en_CA, en_GB, and en_IN. Add the rest.
                const std::vector<NSString*> aEN
                    { @"BW", @"BZ", @"GH", @"GM", @"IE", @"JM", @"MU", @"MW", @"MY", @"NA",
                      @"NZ", @"PH", @"TT", @"US", @"ZA", @"ZW" };
                for (auto c: aEN)
                {
                    pLangStr = [@"en_" stringByAppendingString: c];
                    postspdict.push_back( pLangStr );
                }
            }
            else if ([pLangStr isEqualToString:@"en_JP"]
                     || [pLangStr isEqualToString:@"en_SG"])
            {
                // Just skip, LO doesn't have those yet in this context.
            }
            else if ([pLangStr isEqualToString:@"es"])
            {
                const std::vector<NSString*> aES
                    { @"AR", @"BO", @"CL", @"CO", @"CR", @"CU", @"DO", @"EC", @"ES", @"GT",
                      @"HN", @"MX", @"NI", @"PA", @"PE", @"PR", @"PY", @"SV", @"UY", @"VE" };
                for (auto c: aES)
                {
                    pLangStr = [@"es_" stringByAppendingString: c];
                    postspdict.push_back( pLangStr );
                }
            }
            else if ([pLangStr isEqualToString:@"fi"])
            {
                postspdict.push_back( @"fi_FI" );
            }
            else if ([pLangStr isEqualToString:@"fr"])
            {
                const std::vector<NSString*> aFR
                    { @"BE", @"BF", @"BJ", @"CA", @"CH", @"CI", @"FR", @"LU", @"MC", @"ML",
                      @"MU", @"NE", @"SN", @"TG" };
                for (auto c: aFR)
                {
                    pLangStr = [@"fr_" stringByAppendingString: c];
                    postspdict.push_back( pLangStr );
                }
            }
            else if ([pLangStr isEqualToString:@"it"])
            {
                postspdict.push_back( @"it_CH" );
                postspdict.push_back( @"it_IT" );
            }
            else if ([pLangStr isEqualToString:@"ko"])
            {
                postspdict.push_back( @"ko_KR" );
            }
            else if ([pLangStr isEqualToString:@"nl"])
            {
                postspdict.push_back( @"nl_BE" );
                postspdict.push_back( @"nl_NL" );
            }
            else if ([pLangStr isEqualToString:@"nb"])
            {
                postspdict.push_back( @"nb_NO" );
            }
            else if ([pLangStr isEqualToString:@"pl"])
            {
                postspdict.push_back( @"pl_PL" );
            }
            else if ([pLangStr isEqualToString:@"ru"])
            {
                postspdict.push_back( @"ru_RU" );
            }
            else if ([pLangStr isEqualToString:@"sv"])
            {
                postspdict.push_back( @"sv_FI" );
                postspdict.push_back( @"sv_SE" );
            }
            else if ([pLangStr isEqualToString:@"tr"])
            {
                postspdict.push_back( @"tr_TR" );
            }
            else
                postspdict.push_back( pLangStr );
        }
        // System has pt_BR and pt_PT, add pt_AO.
        postspdict.push_back( @"pt_AO" );

        numshr = postspdict.size();

        // we really should merge these and remove duplicates but since
        // users can name their dictionaries anything they want it would
        // be impossible to know if a real duplication exists unless we
        // add some unique key to each myspell dictionary
        numdict = numshr;

        if (numdict) {
            aDLocs = new Locale [numdict];
            aDEncs  = new rtl_TextEncoding [numdict];
            aDNames = new OUString [numdict];
            aSuppLocales.realloc(numdict);
            Locale * pLocale = aSuppLocales.getArray();
            int numlocs = 0;
            int newloc;
            int i,j;
            int k = 0;

            //first add the user dictionaries
            //TODO for MAC?

            // now add the shared dictionaries
            for (i = 0; i < numshr; i++) {
                NSDictionary *aLocDict = [ NSLocale componentsFromLocaleIdentifier:postspdict[i] ];
                NSString* aLang = [ aLocDict objectForKey:NSLocaleLanguageCode ];
                NSString* aCountry = [ aLocDict objectForKey:NSLocaleCountryCode ];
                OUString lang([aLang cStringUsingEncoding: NSUTF8StringEncoding], [aLang length], aEnc);
                OUString country([ aCountry cStringUsingEncoding: NSUTF8StringEncoding], [aCountry length], aEnc);
                Locale nLoc( lang, country, OUString() );
                newloc = 1;
                //eliminate duplicates (is this needed for MacOS?)
                for (j = 0; j < numlocs; j++) {
                    if (nLoc == pLocale[j]) newloc = 0;
                }
                if (newloc) {
                    pLocale[numlocs] = nLoc;
                    numlocs++;
                }
                aDLocs[k] = nLoc;
                aDEncs[k] = 0;
                k++;
            }

            aSuppLocales.realloc(numlocs);

        } else {
            /* no dictionary.lst found so register no dictionaries */
            numdict = 0;
                aDEncs  = nullptr;
                aDLocs = nullptr;
                aDNames = nullptr;
                aSuppLocales.realloc(0);
            }
        }

    return aSuppLocales;
}



sal_Bool SAL_CALL MacSpellChecker::hasLocale(const Locale& rLocale)
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


sal_Int16 MacSpellChecker::GetSpellFailure( const OUString &rWord, const Locale &rLocale )
{
    rtl_TextEncoding aEnc;

    // initialize a myspell object for each dictionary once
        // (note: mutex is held higher up in isValid)


    sal_Int16 nRes = -1;

        // first handle smart quotes both single and double
    OUStringBuffer rBuf(rWord);
        sal_Int32 n = rBuf.getLength();
        sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++) {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D)) rBuf[ix] = u'"';
        if ((c == 0x2018) || (c == 0x2019)) rBuf[ix] = u'\'';
        }
        OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        aEnc = 0;
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSString* aNSStr = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(nWord.getStr()) length: nWord.getLength()]autorelease];
        NSString* aLang = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rLocale.Language.getStr()) length: rLocale.Language.getLength()]autorelease];
        if(rLocale.Country.getLength()>0)
        {
            NSString* aCountry = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rLocale.Country.getStr()) length: rLocale.Country.getLength()]autorelease];
            NSString* aTaggedCountry = [@"_" stringByAppendingString:aCountry];
            aLang = [aLang  stringByAppendingString:aTaggedCountry];
        }

        NSInteger aCount;
        NSRange range = [[NSSpellChecker sharedSpellChecker] checkSpellingOfString:aNSStr startingAt:0 language:aLang wrap:false inSpellDocumentWithTag:macTag wordCount:&aCount];
        int rVal = 0;
        if(range.length>0)
        {
            rVal = -1;
        }
        else
        {
            rVal = 1;
        }
        [pool release];
        if (rVal != 1)
        {
            nRes = SpellFailure::SPELLING_ERROR;
        } else {
            return -1;
        }
    }
    return nRes;
}



sal_Bool SAL_CALL
    MacSpellChecker::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  !rWord.getLength())
        return true;

    if (!hasLocale( rLocale ))
        return true;

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simpler solution than the provided
    // one using the PropertyHelper_Spell.

    PropertyHelper_Spell &rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1)
    {
        LanguageType nLang = LinguLocaleToLanguage( rLocale );
        // postprocess result for errors that should be ignored
        if (   (!rHelper.IsSpellUpperCase()  && IsUpper( rWord, nLang ))
            || (!rHelper.IsSpellWithDigits() && HasDigits( rWord ))
            || (!rHelper.IsSpellCapitalization()
                &&  nFailure == SpellFailure::CAPTION_ERROR)
        )
            nFailure = -1;
    }

    return (nFailure == -1);
}


Reference< XSpellAlternatives >
    MacSpellChecker::GetProposals( const OUString &rWord, const Locale &rLocale )
{
    // Retrieves the return values for the 'spell' function call in case
    // of a misspelled word.
    // Especially it may give a list of suggested (correct) words:

    Reference< XSpellAlternatives > xRes;
        // note: mutex is held by higher up by spell which covers both

    LanguageType nLang = LinguLocaleToLanguage( rLocale );
    int count;
    Sequence< OUString > aStr( 0 );

        // first handle smart quotes (single and double)
    OUStringBuffer rBuf(rWord);
        sal_Int32 n = rBuf.getLength();
        sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++) {
         c = rBuf[ix];
         if ((c == 0x201C) || (c == 0x201D)) rBuf[ix] = u'"';
         if ((c == 0x2018) || (c == 0x2019)) rBuf[ix] = u'\'';
        }
        OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSString* aNSStr = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(nWord.getStr()) length: nWord.getLength()]autorelease];
        NSString* aLang = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rLocale.Language.getStr()) length: rLocale.Language.getLength()]autorelease];
        if(rLocale.Country.getLength()>0)
        {
            NSString* aCountry = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rLocale.Country.getStr()) length: rLocale.Country.getLength()]autorelease];
            NSString* aTaggedCountry = [@"_" stringByAppendingString:aCountry];
            aLang = [aLang  stringByAppendingString:aTaggedCountry];
        }
        [[NSSpellChecker sharedSpellChecker] setLanguage:aLang];
        NSArray *guesses = [[NSSpellChecker sharedSpellChecker] guessesForWordRange:NSMakeRange(0, [aNSStr length]) inString:aNSStr language:aLang inSpellDocumentWithTag:0];
        count = [guesses count];
        if (count)
        {
           aStr.realloc( count );
           OUString *pStr = aStr.getArray();
               for (int ii=0; ii < count; ii++)
               {
                  // if needed add: if (suglst[ii] == NULL) continue;
                  NSString* guess = [guesses objectAtIndex:ii];
                  OUString cvtwrd(reinterpret_cast<const sal_Unicode*>([guess cStringUsingEncoding:NSUnicodeStringEncoding]), (sal_Int32)[guess length]);
                  pStr[ii] = cvtwrd;
               }
        }
       [pool release];
    }

            // now return an empty alternative for no suggestions or the list of alternatives if some found
        SpellAlternatives *pAlt = new SpellAlternatives;
        pAlt->SetWordLanguage( rWord, nLang );
        pAlt->SetFailureType( SpellFailure::SPELLING_ERROR );
        pAlt->SetAlternatives( aStr );
        xRes = pAlt;
        return xRes;

}




Reference< XSpellAlternatives > SAL_CALL
    MacSpellChecker::spell( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  !rWord.getLength())
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
Reference< XInterface > SAL_CALL MacSpellChecker_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
{

    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new MacSpellChecker);
    return xService;
}


sal_Bool SAL_CALL
    MacSpellChecker::addLinguServiceEventListener(
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


sal_Bool SAL_CALL
    MacSpellChecker::removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing && rxLstnr.is())
    {
        DBG_ASSERT( xPropHelper.is(), "xPropHelper non existent" );
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}


OUString SAL_CALL
    MacSpellChecker::getServiceDisplayName( const Locale& /*rLocale*/ )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return OUString( "Mac OS X Spell Checker" );
}


void SAL_CALL
    MacSpellChecker::initialize( const Sequence< Any >& rArguments )
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
            //! when the object is no longer used.
            pPropHelper = new PropertyHelper_Spell( static_cast<XSpellChecker *>(this), xPropSet );
            xPropHelper = pPropHelper;
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            OSL_FAIL( "wrong number of arguments in sequence" );

    }
}


void SAL_CALL
    MacSpellChecker::dispose()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( static_cast<XSpellChecker *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    MacSpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL
    MacSpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL MacSpellChecker::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MacSpellChecker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL MacSpellChecker::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Sequence< OUString > MacSpellChecker::getSupportedServiceNames_Static()
        throw()
{
    Sequence< OUString > aSNS { SN_SPELLCHECKER };
    return aSNS;
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL MacOSXSpell_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    if ( MacSpellChecker::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                MacSpellChecker::getImplementationName_Static(),
                MacSpellChecker_CreateInstance,
                MacSpellChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
