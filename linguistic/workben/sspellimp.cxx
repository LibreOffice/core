/*************************************************************************
 *
 *  $RCSfile: sspellimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SPELLIMP_HXX
#include <sspellimp.hxx>
#endif

#include "lngprops.hxx"
#include "spelldta.hxx"

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


///////////////////////////////////////////////////////////////////////////

BOOL operator == ( const Locale &rL1, const Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}

///////////////////////////////////////////////////////////////////////////


SpellChecker::SpellChecker() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = FALSE;
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

    BOOL bRes = FALSE;
    if (!aSuppLocales.getLength())
        getLocales();
    INT32 nLen = aSuppLocales.getLength();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        const Locale *pLocale = aSuppLocales.getConstArray();
        if (rLocale == pLocale[i])
        {
            bRes = TRUE;
            break;
        }
    }
    return bRes;
}


INT16 SpellChecker::GetSpellFailure( const OUString &rWord, const Locale &rLocale )
{
    // Checks wether a word is OK in a given language (Locale) or not, and
    // provides a failure type for the incorrect ones.
    // - words with "liss" (case sensitiv) as substring will be negative.
    // - words with 'x' or 'X' will have incorrect spelling.
    // - words with 's' or 'S' as first letter will have the wrong caption.
    // - all other words will be OK.

    INT16 nRes = -1;

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
        return TRUE;

    if (!hasLocale( rLocale ))
#ifdef LINGU_EXCEPTIONS
        throw( IllegalArgumentException() );
#else
        return TRUE;
#endif

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simplier solution than the provided
    // one using the PropertyHelper_Spell.
    PropertyHelper_Spell &rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    INT16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1)
    {
        INT16 nLang = LocaleToLanguage( rLocale );
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
        INT16 nLang = LocaleToLanguage( rLocale );

        if (STRING_NOTFOUND != aTmp.SearchAscii( "liss" ))
        {
            aTmp.SearchAndReplaceAllAscii( "liss", A2OU("liz") );
            xRes = new SpellAlternatives( aTmp, nLang,
                        SpellFailure::IS_NEGATIVE_WORD, aTmp );
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
            aAlt2.EraseAllChars( (sal_Unicode) 'x' );
            aAlt2.EraseAllChars( (sal_Unicode) 'X' );
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
                        SpellFailure::CAPTION_ERROR, aTmp );
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
            const Reference< XMultiServiceFactory > & rSMgr )
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

    BOOL bRes = FALSE;
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

    BOOL bRes = FALSE;
    if (!bDisposing && rxLstnr.is())
    {
        DBG_ASSERT( xPropHelper.is(), "xPropHelper non existent" );
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}


OUString SAL_CALL
    SpellChecker::getServiceDisplayName( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return A2OU( "StarOffice example SpellChecker" );
}


void SAL_CALL
    SpellChecker::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
    {
        INT32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference< XPropertySet >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;
            //rArguments.getConstArray()[1] >>= xDicList;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Spell( (XSpellChecker *) this, xPropSet );
            xPropHelper = pPropHelper;
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            DBG_ERROR( "wrong number of arguments in sequence" );
    }
}


void SAL_CALL
    SpellChecker::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;
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
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
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
        for( INT32 i = 0; i < aServices.getLength(); i++ )
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


///////////////////////////////////////////////////////////////////////////

