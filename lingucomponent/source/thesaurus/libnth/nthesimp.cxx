/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nthesimp.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 15:49:14 $
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

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include "nthesimp.hxx"
#include <linguistic/misc.hxx>
#include <linguistic/lngprops.hxx>
#include "nthesdta.hxx"
#include <dictmgr.hxx>


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

BOOL operator == ( const Locale &rL1, const Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}

/////////////////////////////////////////////////////////////////////////


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

        dictentry * spthes = NULL;  // shared thesaurus list entry pointer
        dictentry * upthes = NULL;  // shared thesaurus list entry pointer
    std::vector<dictentry*> postspthes;
    std::vector<dictentry*> postupthes;
        SvtPathOptions aPathOpt;
        int numusr;          // number of user dictionary entries
        int numshr = 0;          // number of shared dictionary entries

    if (!numthes) {

            // invoke a dictionary manager to get the user dictionary list
            OUString usrlst = aPathOpt.GetUserDictionaryPath() + A2OU("/dictionary.lst");
            OUString ulst;
        osl::FileBase::getSystemPathFromFileURL(usrlst,ulst);
            OString uTmp(OU2ENC(ulst, osl_getThreadTextEncoding()));
        DictMgr* udMgr = new DictMgr(uTmp.getStr(),"THES");
            numusr = 0;
            if (udMgr)
                 numusr = udMgr->get_list(&upthes);

            // invoke a dictionary manager to get the shared thesaurus list
            OUString shrlst = aPathOpt.GetLinguisticPath() + A2OU("/ooo/dictionary.lst");
            OUString slst;
        osl::FileBase::getSystemPathFromFileURL(shrlst,slst);
            OString sTmp(OU2ENC(slst, osl_getThreadTextEncoding()));
        DictMgr* sdMgr = new DictMgr(sTmp.getStr(),"THES");
            if (sdMgr)
                 numshr = sdMgr->get_list(&spthes);

            //Test for existence of the dictionaries
            for (int i = 0; i < numusr; i++)
        {
                OUString str = aPathOpt.GetUserDictionaryPath() + A2OU("/") + A2OU(upthes[i].filename) +
            A2OU(".idx");
        osl::File aTest(str);
        if (aTest.open(osl_File_OpenFlag_Read))
            continue;
        aTest.close();
        postupthes.push_back(&upthes[i]);
            }

            for (int i = 0; i < numshr; i++)
        {
                OUString str = aPathOpt.GetLinguisticPath() + A2OU("/ooo/") + A2OU(spthes[i].filename) +
            A2OU(".idx");
        osl::File aTest(str);
                OString aTmpidx(OU2ENC(str,osl_getThreadTextEncoding()));
        if (aTest.open(osl_File_OpenFlag_Read))
            continue;
        aTest.close();
        postspthes.push_back(&spthes[i]);
            }

        numusr = postupthes.size();
            numshr = postspthes.size();


            // we really should merge these and remove duplicates but since
            // users can name their dictionaries anything they want it would
            // be impossible to know if a real duplication exists unless we
            // add some unique key to each dictionary
            numthes = numshr + numusr;

            if (numthes) {
            aThes = new MyThes* [numthes];
            aTEncs  = new rtl_TextEncoding [numthes];
                aTLocs = new Locale [numthes];
                aTNames = new OUString [numthes];
                aCharSetInfo = new CharClass* [numthes];
            aSuppLocales.realloc(numthes);
                Locale * pLocale = aSuppLocales.getArray();
                int numlocs = 0;
                int newloc;
                int i,j;
                int k = 0;

                //first add the user dictionaries
                for (i = 0; i < numusr; i++) {
                Locale nLoc( A2OU(postupthes[i]->lang), A2OU(postupthes[i]->region), OUString() );
                    newloc = 1;
                for (j = 0; j < numlocs; j++) {
                        if (nLoc == pLocale[j]) newloc = 0;
                    }
                    if (newloc) {
                        pLocale[numlocs] = nLoc;
                        numlocs++;
                    }
                    aTLocs[k] = nLoc;
                    aThes[k] = NULL;
                    aTEncs[k] = 0;

                    aTNames[k] = aPathOpt.GetUserDictionaryPath() + A2OU("/") + A2OU(postupthes[i]->filename);
                     aCharSetInfo[k] = new CharClass(nLoc);
                    k++;
                }

                // now add the shared thesauri
                for (i = 0; i < numshr; i++) {
                Locale nLoc( A2OU(postspthes[i]->lang), A2OU(postspthes[i]->region), OUString() );
                    newloc = 1;
                for (j = 0; j < numlocs; j++) {
                        if (nLoc == pLocale[j]) newloc = 0;
                    }
                    if (newloc) {
                        pLocale[numlocs] = nLoc;
                        numlocs++;
                    }
                    aTLocs[k] = nLoc;
                    aThes[k] = NULL;
                    aTEncs[k] = 0;
                    aTNames[k] = aPathOpt.GetLinguisticPath() + A2OU("/ooo/") + A2OU(postspthes[i]->filename);
                    aCharSetInfo[k] = new CharClass(nLoc);
                    k++;
                }

                aSuppLocales.realloc(numlocs);

            } else {
            // no dictionary.lst so don't register any
            numthes = 0;
            aThes = NULL;
                aTEncs  = NULL;
                aTLocs = NULL;
                aTNames = NULL;
            aSuppLocales.realloc(0);
                aCharSetInfo = NULL;
            }

            /* de-allocation of memory is handled inside the DictMgr */
            upthes = NULL;
            if (udMgr) {
                  delete udMgr;
                  udMgr = NULL;
            }
            spthes = NULL;
            if (sdMgr) {
                  delete sdMgr;
                  sdMgr = NULL;
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
