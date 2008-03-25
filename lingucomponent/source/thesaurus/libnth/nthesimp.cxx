/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nthesimp.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:52:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"

// include <stdio.h>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include "nthesimp.hxx"
#include <linguistic/misc.hxx>
#include <linguistic/lngprops.hxx>
#include "nthesdta.hxx"
#include <dictmgr.hxx>

#include <list>
#include <set>

// values asigned to capitalization types
#define CAPTYPE_UNKNOWN 0
#define CAPTYPE_NOCAP   1
#define CAPTYPE_INITCAP 2
#define CAPTYPE_ALLCAP  3
#define CAPTYPE_MIXED   4



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

Thesaurus::Thesaurus() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = FALSE;
    pPropHelper = NULL;
    aThes = NULL;
    aCharSetInfo = NULL;
    aTEncs = NULL;
    aTLocs = NULL;
    aTNames = NULL;
        numthes = 0;
}


Thesaurus::~Thesaurus()
{

    if (aThes) {
        for (int i = 0; i < numthes; i++) {
            if (aThes[i]) delete aThes[i];
            aThes[i] = NULL;
        }
        delete[] aThes;
    }
    aThes = NULL;
    if (aCharSetInfo) {
        for (int i = 0; i < numthes; i++) {
            if (aCharSetInfo[i]) delete aCharSetInfo[i];
            aCharSetInfo[i] = NULL;
        }
        delete[] aCharSetInfo;
    }
    aCharSetInfo = NULL;
    numthes = 0;
    if (aTEncs) delete[] aTEncs;
    aTEncs = NULL;
    if (aTLocs) delete[] aTLocs;
    aTLocs = NULL;
    if (aTNames) delete[] aTNames;
    aTNames = NULL;

    if (pPropHelper)
        pPropHelper->RemoveAsPropListener();
}


PropertyHelper_Thes & Thesaurus::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XPropertySet >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Thes( (XThesaurus *) this, xPropSet );
        xPropHelper = pPropHelper;
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}


Sequence< Locale > SAL_CALL Thesaurus::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.

    if (!numthes)
    {
        SvtLinguConfig aLinguCfg;

        // get list of dictionaries-to-use
        std::list< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< rtl::OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( A2OU("Thesauri"),
                A2OU("org.openoffice.lingu.new.Thesaurus"), aFormatList );
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
            std::set< rtl::OUString, lt_rtl_OUString > aLocaleNamesSet;
            std::list< SvtLinguConfigDictionaryEntry >::const_iterator aDictIt;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                uno::Sequence< rtl::OUString > aLocaleNames( aDictIt->aLocaleNames );
                sal_Int32 nLen2 = aLocaleNames.getLength();
                for (k = 0;  k < nLen2;  ++k)
                {
                    aLocaleNamesSet.insert( aLocaleNames[k] );
                }
            }
            // ... and add them to the resulting sequence
            aSuppLocales.realloc( aLocaleNamesSet.size() );
            std::set< rtl::OUString, lt_rtl_OUString >::const_iterator aItB;
            k = 0;
            for (aItB = aLocaleNamesSet.begin();  aItB != aLocaleNamesSet.end();  ++aItB)
            {
                Locale aTmp( MsLangId::convertLanguageToLocale(
                        MsLangId::convertIsoStringToLanguage( *aItB )));
                aSuppLocales[k++] = aTmp;
            }

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
                    aThes[k]  = NULL;
                    aTEncs[k]  = 0;
                    // currently HunSpell supports only one language per dictionary...
                    aTLocs[k]  = MsLangId::convertLanguageToLocale(
                                    MsLangId::convertIsoStringToLanguage( aDictIt->aLocaleNames[0] ));
                    aCharSetInfo[k] = new CharClass( aTLocs[k] );
                    // also both files have to be in the same directory and the
                    // file names must only differ in the extension (.aff/.dic).
                    // Thus we use the first location only and strip the extension part.
                    rtl::OUString aLocation = aDictIt->aLocations[0];
                    sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                    aLocation = aLocation.copy( 0, nPos );
                    aTNames[k] = aLocation;
                }
                ++k;
            }
        }
        else
        {
            /* no dictionary found so register no dictionaries */
            numthes = 0;
            aThes  = NULL;
            aTEncs  = NULL;
            aTLocs  = NULL;
            aTNames = NULL;
            aCharSetInfo = NULL;
            aSuppLocales.realloc(0);
        }
    }

    return aSuppLocales;
}



sal_Bool SAL_CALL Thesaurus::hasLocale(const Locale& rLocale)
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


Sequence < Reference < ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
        Thesaurus::queryMeanings( const OUString& rTerm, const Locale& rLocale,
                                  const PropertyValues& rProperties)
                                  throw(IllegalArgumentException, RuntimeException)
{
        MutexGuard      aGuard( GetLinguMutex() );

        uno::Sequence< Reference< XMeaning > > aMeanings( 1 );
        uno::Sequence< Reference< XMeaning > > noMeanings( 0 );

        INT16 nLanguage = LocaleToLanguage( rLocale );

        if (nLanguage == LANGUAGE_NONE  || !rTerm.getLength())
                return noMeanings;

    if (!hasLocale( rLocale ))
#ifdef LINGU_EXCEPTIONS
        throw( IllegalArgumentException() );
#else
        return noMeanings;
#endif

        mentry * pmean = NULL;
    sal_Int32 nmean = 0;

        PropertyHelper_Thes &rHelper = GetPropHelper();
        rHelper.SetTmpPropVals( rProperties );

        MyThes * pTH = NULL;
        rtl_TextEncoding aEnc = 0;
        CharClass * pCC = NULL;


        // find the first thesaurus that matches the locale
        for (int i =0; i < numthes; i++) {
            if (rLocale == aTLocs[i])
            {
           // open up and intialize this thesaurus if need be
                   if (!aThes[i])
                   {
                      OUString datpath = aTNames[i] + A2OU(".dat");
                      OUString idxpath = aTNames[i] + A2OU(".idx");
                      OUString ndat;
                      OUString nidx;
                      osl::FileBase::getSystemPathFromFileURL(datpath,ndat);
                      osl::FileBase::getSystemPathFromFileURL(idxpath,nidx);
                      OString aTmpidx(OU2ENC(nidx,osl_getThreadTextEncoding()));
                      OString aTmpdat(OU2ENC(ndat,osl_getThreadTextEncoding()));

#if defined(WNT)
                      // workaround for Windows specifc problem that the
                      // path length in calls to 'fopen' is limted to somewhat
                      // about 120+ characters which will usually be exceed when
                      // using dictionaries as extensions.
                      aTmpidx = Win_GetShortPathName( nidx );
                      aTmpdat = Win_GetShortPathName( ndat );
#endif

                      aThes[i] = new MyThes(aTmpidx.getStr(),aTmpdat.getStr());
                      if (aThes[i]) {
                        const char * enc_string = aThes[i]->get_th_encoding();
                        if (!enc_string) {
                          aTEncs[i] = rtl_getTextEncodingFromUnixCharset("ISO8859-1");
                        } else {
                          aTEncs[i] = rtl_getTextEncodingFromUnixCharset(enc_string);
                          if (aTEncs[i] == RTL_TEXTENCODING_DONTKNOW) {
                            if (strcmp("ISCII-DEVANAGARI", enc_string) == 0) {
                              aTEncs[i] = RTL_TEXTENCODING_ISCII_DEVANAGARI;
                            } else if (strcmp("UTF-8", enc_string) == 0) {
                              aTEncs[i] = RTL_TEXTENCODING_UTF8;
                            }
                          }
                      }
                   }
               }
               pTH = aThes[i];
                   aEnc = aTEncs[i];
                   pCC = aCharSetInfo[i];

               if (pTH)
                   break;
        }
    }

        if (pTH) {
         // convert word to all lower case for searching
             sal_uInt16 ct = CAPTYPE_UNKNOWN;
             ct = capitalType(rTerm, pCC);
             OUString nTerm(makeLowerCase(rTerm, pCC));
             OString aTmp( OU2ENC(nTerm, aEnc) );
             nmean = pTH->Lookup(aTmp.getStr(),aTmp.getLength(),&pmean);

             if (nmean) aMeanings.realloc( nmean );

             mentry * pe = pmean;
         for (int j = 0; j < nmean; j++) {
             int count = pe->count;
                 if (count) {
                     Sequence< OUString > aStr( count );
                     OUString *pStr = aStr.getArray();
                     for (int i=0; i < count; i++) {
                       OUString sTerm(pe->psyns[i],strlen(pe->psyns[i]),aEnc );
                       sal_uInt16 ct1 = capitalType(sTerm, pCC);
                       if (CAPTYPE_MIXED == ct1)
                            ct = ct1;
                       OUString cTerm;
                       switch (ct) {
                       case CAPTYPE_ALLCAP:
                     {
                               cTerm = makeUpperCase(sTerm, pCC);
                               break;
                             }
                       case CAPTYPE_INITCAP:
                     {
                               cTerm = makeInitCap(sTerm, pCC);
                               break;
                             }
                           default:
                     {
                               cTerm = sTerm;
                               break;
                             }
               }
                       OUString aAlt( cTerm );
                       pStr[i] = aAlt;
             }
#if 0
                     Meaning * pMn = new Meaning(rTerm,nLanguage,rHelper);
#endif
                     Meaning * pMn = new Meaning(rTerm,nLanguage);
                     OUString dTerm(pe->defn,strlen(pe->defn),aEnc );
                     pMn->SetMeaning(dTerm);
                     pMn->SetSynonyms(aStr);
                     Reference<XMeaning>* pMeaning = aMeanings.getArray();
                     pMeaning[j] = pMn;
         }
                 pe++;
         }
             pTH->CleanUpAfterLookup(&pmean,nmean);
    }
        if (nmean) {
            return aMeanings;
    }
    return noMeanings;
}



Reference< XInterface > SAL_CALL Thesaurus_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*) new Thesaurus;
    return xService;
}


OUString SAL_CALL
    Thesaurus::getServiceDisplayName( const Locale& /*rLocale*/ )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return A2OU( "OpenOffice.org New Thesaurus" );
}


void SAL_CALL
    Thesaurus::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
    {
        INT32 nLen = rArguments.getLength();
        if (1 == nLen)
        {
            Reference< XPropertySet >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Thes( (XThesaurus *) this, xPropSet );
            xPropHelper = pPropHelper;
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            DBG_ERROR( "wrong number of arguments in sequence" );
    }
}



sal_uInt16 SAL_CALL Thesaurus::capitalType(const OUString& aTerm, CharClass * pCC)
{
        sal_Int32 tlen = aTerm.getLength();
        if ((pCC) && (tlen)) {
              String aStr(aTerm);
              sal_Int32 nc = 0;
              for (sal_Int32 tindex = 0; tindex < tlen;  tindex++) {
               if (pCC->getCharacterType(aStr,tindex) &
                       ::com::sun::star::i18n::KCharacterType::UPPER) nc++;
          }

              if (nc == 0) return (sal_uInt16) CAPTYPE_NOCAP;

              if (nc == tlen) return (sal_uInt16) CAPTYPE_ALLCAP;

              if ((nc == 1) && (pCC->getCharacterType(aStr,0) &
                      ::com::sun::star::i18n::KCharacterType::UPPER))
                   return (sal_uInt16) CAPTYPE_INITCAP;

              return (sal_uInt16) CAPTYPE_MIXED;
    }
        return (sal_uInt16) CAPTYPE_UNKNOWN;
}



OUString SAL_CALL Thesaurus::makeLowerCase(const OUString& aTerm, CharClass * pCC)
{
        if (pCC)
      return pCC->toLower_rtl(aTerm, 0, aTerm.getLength());
        return aTerm;
}


OUString SAL_CALL Thesaurus::makeUpperCase(const OUString& aTerm, CharClass * pCC)
{
        if (pCC)
              return pCC->toUpper_rtl(aTerm, 0, aTerm.getLength());
        return aTerm;
}


OUString SAL_CALL Thesaurus::makeInitCap(const OUString& aTerm, CharClass * pCC)
{
        sal_Int32 tlen = aTerm.getLength();
        if ((pCC) && (tlen)) {
              OUString bTemp = aTerm.copy(0,1);
              if (tlen > 1)
                   return ( pCC->toUpper_rtl(bTemp, 0, 1)
                             + pCC->toLower_rtl(aTerm,1,(tlen-1)) );

          return pCC->toUpper_rtl(bTemp, 0, 1);
    }
        return aTerm;
}



void SAL_CALL
    Thesaurus::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;
        EventObject aEvtObj( (XThesaurus *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    Thesaurus::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL
    Thesaurus::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

OUString SAL_CALL Thesaurus::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL Thesaurus::supportsService( const OUString& ServiceName )
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


Sequence< OUString > SAL_CALL Thesaurus::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


Sequence< OUString > Thesaurus::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aSNS( 1 ); // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_THESAURUS );
    return aSNS;
}


sal_Bool SAL_CALL Thesaurus_writeInfo(
            void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += Thesaurus::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
                pRegistryKey->createKey( aImpl );
        Sequence< OUString > aServices =
                Thesaurus::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}


void * SAL_CALL Thesaurus_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !Thesaurus::getImplementationName_Static().compareToAscii( pImplName ) )
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


///////////////////////////////////////////////////////////////////////////


#undef CAPTYPE_UNKNOWN
#undef CAPTYPE_NOCAP
#undef CAPTYPE_INITCAP
#undef CAPTYPE_ALLCAP
#undef CAPTYPE_MIXED
