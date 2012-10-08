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
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <comphelper/string.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <sspellimp.hxx>

#include "linguistic/lngprops.hxx"
#include "linguistic/spelldta.hxx"

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;


sal_Bool operator == ( const Locale &rL1, const Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}



SpellChecker::SpellChecker() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = sal_False;
    pPropHelper = NULL;
}


SpellChecker::~SpellChecker()
{
    if (pPropHelper)
        pPropHelper->RemoveAsPropListener();
}


PropertyHelper_Spell & SpellChecker::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XPropertySet >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Spell( (XSpellChecker *) this, xPropSet );
        xPropHelper = pPropHelper;
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}


Sequence< Locale > SAL_CALL SpellChecker::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!aSuppLocales.getLength())
    {
        aSuppLocales.realloc( 3 );
        Locale *pLocale = aSuppLocales.getArray();
        pLocale[0] = Locale( A2OU("en"), A2OU("US"), OUString() );
        pLocale[1] = Locale( A2OU("de"), A2OU("DE"), OUString() );
        pLocale[2] = Locale( A2OU("de"), A2OU("CH"), OUString() );
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
    sal_Int32 nLen = aSuppLocales.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const Locale *pLocale = aSuppLocales.getConstArray();
        if (rLocale == pLocale[i])
        {
            bRes = sal_True;
            break;
        }
    }
    return bRes;
}


sal_Int16 SpellChecker::GetSpellFailure( const OUString &rWord, const Locale & )
{
    // Checks whether a word is OK in a given language (Locale) or not, and
    // provides a failure type for the incorrect ones.
    // - words with "liss" (case sensitiv) as substring will be negative.
    // - words with 'x' or 'X' will have incorrect spelling.
    // - words with 's' or 'S' as first letter will have the wrong caption.
    // - all other words will be OK.

    sal_Int16 nRes = -1;

    String aTmp( rWord );
    if (aTmp.Len())
    {
        if (STRING_NOTFOUND != aTmp.SearchAscii( "liss" ))
        {
            nRes = SpellFailure::IS_NEGATIVE_WORD;
        }
        else if (STRING_NOTFOUND != aTmp.Search( (sal_Unicode) 'x' )  ||
                 STRING_NOTFOUND != aTmp.Search( (sal_Unicode) 'X' ))
        {
            nRes = SpellFailure::SPELLING_ERROR;
        }
        else
        {
            sal_Unicode cChar = aTmp.GetChar( 0 );
            if (cChar == (sal_Unicode) 's'  ||  cChar == (sal_Unicode) 'S')
                nRes = SpellFailure::CAPTION_ERROR;
        }
    }

    return nRes;
}


sal_Bool SAL_CALL
    SpellChecker::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  !rWord.getLength())
        return sal_True;

    if (!hasLocale( rLocale ))
#ifdef LINGU_EXCEPTIONS
        throw( IllegalArgumentException() );
#else
        return sal_True;
#endif

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simplier solution than the provided
    // one using the PropertyHelper_Spell.
    PropertyHelper_Spell &rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1)
    {
        sal_Int16 nLang = LocaleToLanguage( rLocale );
        // postprocess result for errors that should be ignored
        if (   (!rHelper.IsSpellUpperCase()  && IsUpper( rWord, nLang ))
            || (!rHelper.IsSpellWithDigits() && HasDigits( rWord ))
            || (!rHelper.IsSpellCapitalization()
                &&  nFailure == SpellFailure::CAPTION_ERROR)
        )
            nFailure = -1;
    }
    return nFailure == -1;
}


Reference< XSpellAlternatives >
    SpellChecker::GetProposals( const OUString &rWord, const Locale &rLocale )
{
    // Retrieves the return values for the 'spell' function call in case
    // of a misspelled word.
    // Especially it may give a list of suggested (correct) words:
    // - a "liss" substring will be replaced by "liz".
    // - 'x' or 'X' will be replaced by 'u' or 'U' for the first proposal
    //   and they will be removed from the word for the second proposal.
    // - 's' or 'S' as first letter will be changed to the other caption.

    Reference< XSpellAlternatives > xRes;

    String aTmp( rWord );
    if (aTmp.Len())
    {
        sal_Int16 nLang = LocaleToLanguage( rLocale );

        if (STRING_NOTFOUND != aTmp.SearchAscii( "liss" ))
        {
            aTmp.SearchAndReplaceAllAscii( "liss", A2OU("liz") );
            xRes = new SpellAlternatives( aTmp, nLang,
                        SpellFailure::IS_NEGATIVE_WORD, ::com::sun::star::uno::Sequence< ::rtl::OUString >() );
        }
        else if (STRING_NOTFOUND != aTmp.Search( (sal_Unicode) 'x' )  ||
                 STRING_NOTFOUND != aTmp.Search( (sal_Unicode) 'X' ))
        {
            Sequence< OUString > aStr( 2 );
            OUString *pStr = aStr.getArray();
            String  aAlt1( aTmp ),
                    aAlt2( aTmp );
            aAlt1.SearchAndReplaceAll( (sal_Unicode) 'x', (sal_Unicode) 'u');
            aAlt1.SearchAndReplaceAll( (sal_Unicode) 'X', (sal_Unicode) 'U');
            aAlt2 = comphelper::string::remove(aAlt2, 'x');
            aAlt2 = comphelper::string::remove(aAlt2, 'X');
            pStr[0] = aAlt1;
            pStr[1] = aAlt2;

            SpellAlternatives *pAlt = new SpellAlternatives;
            pAlt->SetWordLanguage( aTmp, nLang );
            pAlt->SetFailureType( SpellFailure::SPELLING_ERROR );
            pAlt->SetAlternatives( aStr );

            xRes = pAlt;
        }
        else
        {
            sal_Unicode cChar = aTmp.GetChar( 0 );
            if (cChar == (sal_Unicode) 's'  ||  cChar == (sal_Unicode) 'S')
            {
                sal_Unicode cNewChar = cChar == (sal_Unicode) 's' ?
                        (sal_Unicode) 'S': (sal_Unicode) 's';
                aTmp.GetBufferAccess()[0] = cNewChar;
                xRes = new SpellAlternatives( aTmp, nLang,
                        SpellFailure::CAPTION_ERROR, ::com::sun::star::uno::Sequence< ::rtl::OUString >() );
            }
        }
    }

    return xRes;
}


Reference< XSpellAlternatives > SAL_CALL
    SpellChecker::spell( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

     if (rLocale == Locale()  ||  !rWord.getLength())
        return NULL;

    if (!hasLocale( rLocale ))
#ifdef LINGU_EXCEPTIONS
        throw( IllegalArgumentException() );
#else
        return NULL;
#endif

    Reference< XSpellAlternatives > xAlt;
    if (!isValid( rWord, rLocale, rProperties ))
    {
        xAlt =  GetProposals( rWord, rLocale );
    }
    return xAlt;
}


Reference< XInterface > SAL_CALL SpellChecker_CreateInstance(
            const Reference< XMultiServiceFactory > & )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*) new SpellChecker;
    return xService;
}


sal_Bool SAL_CALL
    SpellChecker::addLinguServiceEventListener(
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


sal_Bool SAL_CALL
    SpellChecker::removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!bDisposing && rxLstnr.is())
    {
        DBG_ASSERT( xPropHelper.is(), "xPropHelper non existent" );
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}


OUString SAL_CALL
    SpellChecker::getServiceDisplayName( const Locale& )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return A2OU( "OpenOffice example spellchecker" );
}


void SAL_CALL
    SpellChecker::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference< XPropertySet >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Spell( (XSpellChecker *) this, xPropSet );
            xPropHelper = pPropHelper;
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            OSL_FAIL( "wrong number of arguments in sequence" );
    }
}


void SAL_CALL
    SpellChecker::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = sal_True;
        EventObject aEvtObj( (XSpellChecker *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    SpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL
    SpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


// Service specific part

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

    Sequence< OUString > aSNS( 1 ); // more than 1 service possible
    aSNS.getArray()[0] = A2OU( SN_SPELLCHECKER );
    return aSNS;
}


sal_Bool SAL_CALL SpellChecker_writeInfo(
            void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += SpellChecker::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
                pRegistryKey->createKey( aImpl );
        Sequence< OUString > aServices =
                SpellChecker::getSupportedServiceNames_Static();
        for( sal_Int32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
