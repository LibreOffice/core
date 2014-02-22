/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <hyphen.h>
#include <hyphenimp.hxx>

#include <linguistic/hyphdta.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>

#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/lingucfg.hxx>
#include <osl/file.hxx>

#include <stdio.h>
#include <string.h>

#include <list>
#include <set>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;



#define Max(a,b) (a > b ? a : b)




Hyphenator::Hyphenator() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = false;
    pPropHelper = NULL;
    aDicts = NULL;
    numdict = 0;
}

Hyphenator::~Hyphenator()
{
    if (numdict && aDicts)
    {
        for (int i=0; i < numdict; ++i)
        {
            delete aDicts[i].apCC;
            if (aDicts[i].aPtr)
                hnj_hyphen_free(aDicts[i].aPtr);
        }
    }
    delete[] aDicts;

    if (pPropHelper)
    {
        pPropHelper->RemoveAsPropListener();
        delete pPropHelper;
    }
}

PropertyHelper_Hyphenation& Hyphenator::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Hyphenation ((XHyphenator *) this, xPropSet );
        pPropHelper->AddAsPropListener();   
    }
    return *pPropHelper;
}


Sequence< Locale > SAL_CALL Hyphenator::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    
    

    if (!numdict)
    {
        SvtLinguConfig aLinguCfg;

        
        
        
        std::list< SvtLinguConfigDictionaryEntry > aDics;
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

        
        
        
        std::vector< SvtLinguConfigDictionaryEntry > aOldStyleDics(
                GetOldStyleDics( "HYPH" ) );

        
        
        
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        numdict = aDics.size();
        if (numdict)
        {
            
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
            
            aSuppLocales.realloc( aLocaleNamesSet.size() );
            std::set< OUString, lt_rtl_OUString >::const_iterator aItB;
            k = 0;
            for (aItB = aLocaleNamesSet.begin();  aItB != aLocaleNamesSet.end();  ++aItB)
            {
                Locale aTmp( LanguageTag::convertToLocale( *aItB ));
                aSuppLocales[k++] = aTmp;
            }

            
            
            
            
            
            numdict = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
                numdict = numdict + aDictIt->aLocaleNames.getLength();

            
            aDicts = new HDInfo[numdict];

            k = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                if (aDictIt->aLocaleNames.getLength() > 0 &&
                    aDictIt->aLocations.getLength() > 0)
                {
                    uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                    sal_Int32 nLocales = aLocaleNames.getLength();

                    
                    
                    
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        LanguageTag aLanguageTag( aDictIt->aLocaleNames[i] );
                        aDicts[k].aPtr = NULL;
                        aDicts[k].eEnc = RTL_TEXTENCODING_DONTKNOW;
                        aDicts[k].aLoc = aLanguageTag.getLocale();
                        aDicts[k].apCC = new CharClass( aLanguageTag );
                        
                        
                        
                        OUString aLocation = aDictIt->aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        aDicts[k].aName = aLocation;

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
            aDicts = NULL;
            aSuppLocales.realloc(0);
        }
    }

    return aSuppLocales;
}



sal_Bool SAL_CALL Hyphenator::hasLocale(const Locale& rLocale)
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


Reference< XHyphenatedWord > SAL_CALL Hyphenator::hyphenate( const OUString& aWord,
       const ::com::sun::star::lang::Locale& aLocale,
       sal_Int16 nMaxLeading,
       const ::com::sun::star::beans::PropertyValues& aProperties )
       throw (com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException)
{
    int nHyphenationPos = -1;
    int nHyphenationPosAlt = -1;
    int nHyphenationPosAltHyph = -1;
    int wordlen;
    char *hyphens;
    char *lcword;
    int k = 0;

    PropertyHelper_Hyphenation& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    HyphenDict *dict = NULL;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;
    CharClass * pCC = NULL;

    Reference< XHyphenatedWord > xRes;

    k = -1;
    for (int j = 0; j < numdict; j++)
    {
        if (aLocale == aDicts[j].aLoc)
            k = j;
    }

    
    if (k != -1)
    {
        
        if (!aDicts[k].aPtr)
        {
            OUString DictFN = aDicts[k].aName + ".dic";
            OUString dictpath;

            osl::FileBase::getSystemPathFromFileURL( DictFN, dictpath );
            OString sTmp( OU2ENC( dictpath, osl_getThreadTextEncoding() ) );

#if defined(WNT)
            
            
            
            
            sTmp = Win_GetShortPathName( dictpath );
#endif

            if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
            {
               fprintf(stderr, "Couldn't find file %s\n", OU2ENC(dictpath, osl_getThreadTextEncoding()) );
               return NULL;
            }
            aDicts[k].aPtr = dict;
            aDicts[k].eEnc = getTextEncodingFromCharset(dict->cset);
        }

        
        dict = aDicts[k].aPtr;
        eEnc = aDicts[k].eEnc;
        pCC =  aDicts[k].apCC;

        
        
        
        
        DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
        if (eEnc == RTL_TEXTENCODING_DONTKNOW)
            return NULL;

        sal_uInt16 ct = capitalType(aWord, pCC);

        
        OUStringBuffer rBuf(aWord);
        sal_Int32 nc = rBuf.getLength();
        sal_Unicode ch;
        for (sal_Int32 ix=0; ix < nc; ix++)
        {
            ch = rBuf[ix];
            if ((ch == 0x201C) || (ch == 0x201D))
                rBuf[ix] = (sal_Unicode)0x0022;
            if ((ch == 0x2018) || (ch == 0x2019))
                rBuf[ix] = (sal_Unicode)0x0027;
        }
        OUString nWord(rBuf.makeStringAndClear());

        
        OUString nTerm(makeLowerCase(nWord, pCC));

        
        OString encWord(OU2ENC(nTerm,eEnc));

        wordlen = encWord.getLength();
        lcword = new char[wordlen + 1];
        hyphens = new char[wordlen + 5];

        char ** rep = NULL; 
        int * pos = NULL; 
        int * cut = NULL; 

        
        strcpy(lcword,encWord.getStr());

        
        int n = wordlen-1;
        while((n >=0) && (lcword[n] == '.'))
            n--;
        n++;
        if (n > 0)
        {
            const bool bFailed = 0 != hnj_hyphen_hyphenate3( dict, lcword, n, hyphens, NULL,
                    &rep, &pos, &cut, minLead, minTrail,
                    Max(dict->clhmin, Max(dict->clhmin, 2) + Max(0, minLead  - Max(dict->lhmin, 2))),
                    Max(dict->crhmin, Max(dict->crhmin, 2) + Max(0, minTrail - Max(dict->rhmin, 2))) );
            if (bFailed)
            {
                
                delete[] hyphens;
                delete[] lcword;
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
                return NULL;
            }
        }

        
        for (int c = n; c < wordlen; c++) hyphens[c] = '0';
        hyphens[wordlen] = '\0';

        sal_Int32 Leading =  GetPosInWordToCheck( aWord, nMaxLeading );

        for (sal_Int32 i = 0; i < n; i++)
        {
            int leftrep = 0;
            sal_Bool hit = (n >= minLen);
            if (!rep || !rep[i] || (i >= n))
            {
                hit = hit && (hyphens[i]&1) && (i < Leading);
                hit = hit && (i >= (minLead-1) );
                hit = hit && ((n - i - 1) >= minTrail);
            }
            else
            {
                
                for (char * c = rep[i]; *c && (*c != '='); c++)
                {
                    if (eEnc == RTL_TEXTENCODING_UTF8)
                    {
                        if (((unsigned char) *c) >> 6 != 2)
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
                if (rep && (i < n) && rep[i])
                {
                    nHyphenationPosAlt = i - pos[i];
                    nHyphenationPosAltHyph = i + leftrep - pos[i];
                }
            }
        }

        if (nHyphenationPos  == -1)
        {
            xRes = NULL;
        }
        else
        {
            if (rep && rep[nHyphenationPos])
            {
                
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
                    case CAPTYPE_ALLCAP:
                    {
                        repHyph = makeUpperCase(repHyphlow, pCC);
                        break;
                    }
                    case CAPTYPE_INITCAP:
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

                
                sal_Int16 nPos = (sal_Int16) ((nHyphenationPosAltHyph < nHyphenationPos) ?
                nHyphenationPosAltHyph : nHyphenationPos);
                
                xRes = HyphenatedWord::CreateHyphenatedWord( aWord, LinguLocaleToLanguage( aLocale ), nPos,
                    aWord.replaceAt(nHyphenationPosAlt + 1, cut[nHyphenationPos], repHyph),
                    (sal_Int16) nHyphenationPosAltHyph);
            }
            else
            {
                xRes = HyphenatedWord::CreateHyphenatedWord( aWord, LinguLocaleToLanguage( aLocale ),
                    (sal_Int16)nHyphenationPos, aWord, (sal_Int16) nHyphenationPos);
            }
        }

        delete[] lcword;
        delete[] hyphens;
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
    return NULL;
}


Reference < XHyphenatedWord > SAL_CALL Hyphenator::queryAlternativeSpelling(
        const OUString& aWord,
        const ::com::sun::star::lang::Locale& aLocale,
        sal_Int16 nIndex,
        const ::com::sun::star::beans::PropertyValues& aProperties )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    
    for (int extrachar = 1; extrachar <= 2; extrachar++)
    {
        Reference< XHyphenatedWord > xRes = hyphenate(aWord, aLocale, nIndex + 1 + extrachar, aProperties);
        if (xRes.is() && xRes->isAlternativeSpelling() && xRes->getHyphenationPos() == nIndex)
            return xRes;
    }
    return NULL;
}

Reference< XPossibleHyphens > SAL_CALL Hyphenator::createPossibleHyphens( const OUString& aWord,
        const ::com::sun::star::lang::Locale& aLocale,
        const ::com::sun::star::beans::PropertyValues& aProperties )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    PropertyHelper_Hyphenation& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    
    
    if (aWord.getLength() < minLen)
    {
        return PossibleHyphens::CreatePossibleHyphens( aWord, LinguLocaleToLanguage( aLocale ),
                      aWord, Sequence< sal_Int16 >() );
    }

    int k = -1;
    for (int j = 0; j < numdict; j++)
    {
        if (aLocale == aDicts[j].aLoc) k = j;
    }

    
    if (k != -1)
    {
        HyphenDict *dict = NULL;
        
        if (!aDicts[k].aPtr)
        {
            OUString DictFN = aDicts[k].aName + ".dic";
            OUString dictpath;

            osl::FileBase::getSystemPathFromFileURL( DictFN, dictpath );
            OString sTmp( OU2ENC( dictpath, osl_getThreadTextEncoding() ) );

#if defined(WNT)
            
            
            
            
            sTmp = Win_GetShortPathName( dictpath );
#endif

            if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
            {
               fprintf(stderr, "Couldn't find file %s and %s\n", sTmp.getStr(), OU2ENC(dictpath, osl_getThreadTextEncoding()) );
               return NULL;
            }
            aDicts[k].aPtr = dict;
            aDicts[k].eEnc = getTextEncodingFromCharset(dict->cset);
        }

        
        dict = aDicts[k].aPtr;
        rtl_TextEncoding eEnc = aDicts[k].eEnc;
        CharClass* pCC = aDicts[k].apCC;

        
        
        
        
        DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
        if (eEnc == RTL_TEXTENCODING_DONTKNOW)
            return NULL;

        
        OUStringBuffer rBuf(aWord);
        sal_Int32 nc = rBuf.getLength();
        sal_Unicode ch;
        for (sal_Int32 ix=0; ix < nc; ix++)
        {
            ch = rBuf[ix];
            if ((ch == 0x201C) || (ch == 0x201D))
                rBuf[ix] = (sal_Unicode)0x0022;
            if ((ch == 0x2018) || (ch == 0x2019))
                rBuf[ix] = (sal_Unicode)0x0027;
        }
        OUString nWord(rBuf.makeStringAndClear());

        
        OUString nTerm(makeLowerCase(nWord, pCC));

        
        OString encWord(OU2ENC(nTerm,eEnc));

        int wordlen = encWord.getLength();
        char *lcword = new char[wordlen+1];
        char *hyphens = new char[wordlen+5];
        char ** rep = NULL; 
        int * pos = NULL; 
        int * cut = NULL; 

        
        strcpy(lcword,encWord.getStr());

        
        int n = wordlen-1;
        while((n >=0) && (lcword[n] == '.'))
            n--;
        n++;
        if (n > 0)
        {
            const bool bFailed = 0 != hnj_hyphen_hyphenate3(dict, lcword, n, hyphens, NULL,
                    &rep, &pos, &cut, minLead, minTrail,
                    Max(dict->clhmin, Max(dict->clhmin, 2) + Max(0, minLead - Max(dict->lhmin, 2))),
                    Max(dict->crhmin, Max(dict->crhmin, 2) + Max(0, minTrail - Max(dict->rhmin, 2))) );
            if (bFailed)
            {
                delete[] hyphens;
                delete[] lcword;

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

                return NULL;
            }
        }
        
        for (int c = n; c < wordlen; c++)
            hyphens[c] = '0';
        hyphens[wordlen] = '\0';

        sal_Int16 nHyphCount = 0;
        sal_Int16 i;

        for ( i = 0; i < encWord.getLength(); i++)
        {
            if (hyphens[i]&1)
                nHyphCount++;
        }

        Sequence< sal_Int16 > aHyphPos(nHyphCount);
        sal_Int16 *pPos = aHyphPos.getArray();
        OUStringBuffer hyphenatedWordBuffer;
        nHyphCount = 0;

        for (i = 0; i < nWord.getLength(); i++)
        {
            hyphenatedWordBuffer.append(aWord[i]);
            
            if (hyphens[i]&1)
            {
                pPos[nHyphCount] = i;
                hyphenatedWordBuffer.append('=');
                nHyphCount++;
            }
        }

        OUString hyphenatedWord = hyphenatedWordBuffer.makeStringAndClear();

        Reference< XPossibleHyphens > xRes = PossibleHyphens::CreatePossibleHyphens(
            aWord, LinguLocaleToLanguage( aLocale ), hyphenatedWord, aHyphPos);

        delete[] hyphens;
        delete[] lcword;

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

    return NULL;
}

OUString SAL_CALL Hyphenator::makeLowerCase(const OUString& aTerm, CharClass * pCC)
{
    if (pCC)
        return pCC->lowercase(aTerm);
    return aTerm;
}

OUString SAL_CALL Hyphenator::makeUpperCase(const OUString& aTerm, CharClass * pCC)
{
    if (pCC)
        return pCC->uppercase(aTerm);
    return aTerm;
}


OUString SAL_CALL Hyphenator::makeInitCap(const OUString& aTerm, CharClass * pCC)
{
    sal_Int32 tlen = aTerm.getLength();
    if ((pCC) && (tlen))
    {
        OUString bTemp = aTerm.copy(0,1);
        if (tlen > 1)
            return ( pCC->uppercase(bTemp, 0, 1) + pCC->lowercase(aTerm,1,(tlen-1)) );

        return pCC->uppercase(bTemp, 0, 1);
    }
    return aTerm;
}


Reference< XInterface > SAL_CALL Hyphenator_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*) new Hyphenator;
    return xService;
}


sal_Bool SAL_CALL Hyphenator::addLinguServiceEventListener(
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


sal_Bool SAL_CALL Hyphenator::removeLinguServiceEventListener(
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


OUString SAL_CALL Hyphenator::getServiceDisplayName( const Locale& /*rLocale*/ )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return OUString( "Libhyphen Hyphenator" );
}


void SAL_CALL Hyphenator::initialize( const Sequence< Any >& rArguments )
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
            

            
            
            
            
            pPropHelper = new PropertyHelper_Hyphenation( (XHyphenator *) this, xPropSet );
            pPropHelper->AddAsPropListener();   
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}


void SAL_CALL Hyphenator::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( (XHyphenator *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        if (pPropHelper)
        {
            pPropHelper->RemoveAsPropListener();
            delete pPropHelper;
            pPropHelper = NULL;
        }
    }
}


void SAL_CALL Hyphenator::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL Hyphenator::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


OUString SAL_CALL Hyphenator::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getImplementationName_Static();
}

sal_Bool SAL_CALL Hyphenator::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL Hyphenator::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getSupportedServiceNames_Static();
}

Sequence< OUString > Hyphenator::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aSNS( 1 ); 
    aSNS.getArray()[0] = SN_HYPHENATOR;
    return aSNS;
}

void * SAL_CALL Hyphenator_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( Hyphenator::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                Hyphenator::getImplementationName_Static(),
                Hyphenator_CreateInstance,
                Hyphenator::getSupportedServiceNames_Static());
        
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
