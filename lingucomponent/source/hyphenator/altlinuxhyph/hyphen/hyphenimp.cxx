/*************************************************************************
 *
 *  $RCSfile: hyphenimp.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:34:53 $
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

//#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
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

#include <hyphen.h>

#ifndef _HYPHENIMP_HXX
#include <hyphenimp.hxx>
#endif

#include <linguistic/hyphdta.hxx>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <linguistic/lngprops.hxx>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#include <osl/file.hxx>

#include "dictmgr.hxx"
#include "csutil.hxx"

#include <stdio.h>
#include <iostream>
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





Hyphenator::Hyphenator() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = FALSE;
    pPropHelper = NULL;
        numdict = 0;

}


Hyphenator::~Hyphenator()
{
    if (pPropHelper)
        pPropHelper->RemoveAsPropListener();
        if (numdict) {
            if (aDicts) delete[] aDicts;
        aDicts = NULL;
            numdict = 0;
        }
}


PropertyHelper_Hyphen & Hyphenator::GetPropHelper_Impl()
{

    if (!pPropHelper)
    {
        Reference< XPropertySet >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Hyphen ((XHyphenator *) this, xPropSet );
        xPropHelper = pPropHelper;
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;

}


Sequence< Locale > SAL_CALL Hyphenator::getLocales()
        throw(RuntimeException)
{

    MutexGuard  aGuard( GetLinguMutex() );
    SvtPathOptions  aPathOpt;

    if (!numdict)
    {

            // this routine should return the locales supported by the installed
            // hyph dictionaries.  So here we need to parse both the user edited
            // dictionary list and the shared dictionary list
            // to see what dictionaries the admin/user has installed

            int numusr;          // number of user dictionary entries
            int numshr;          // number of shared dictionary entries
            dictentry * spdict;  // shared dict entry pointer
            dictentry * updict;  // user dict entry pointer

            // invoke a dictionary manager to get the user dictionary list
            OUString usrlst = aPathOpt.GetUserDictionaryPath() + A2OU("/dictionary.lst");
            OUString ulst;
        osl::FileBase::getSystemPathFromFileURL(usrlst,ulst);
            OString uTmp(OU2ENC(ulst,osl_getThreadTextEncoding()));
        DictMgr* udMgr = new DictMgr(uTmp.getStr(),"HYPH");
            numusr = 0;
            if (udMgr)
                 numusr = udMgr->get_list(&updict);

            // invoke a second  dictionary manager to get the shared dictionary list
            OUString shrlst = aPathOpt.GetLinguisticPath() + A2OU("/ooo/dictionary.lst");
            OUString slst;
        osl::FileBase::getSystemPathFromFileURL(shrlst,slst);
            OString sTmp(OU2ENC(slst,osl_getThreadTextEncoding()));
        DictMgr* sdMgr = new DictMgr(sTmp.getStr(),"HYPH");
            numshr = 0;
            if (sdMgr)
                 numshr = sdMgr->get_list(&spdict);


            // we really should merge these and remove duplicates but since
            // users can name their dictionaries anything they want it would
            // be impossible to know if a real duplication exists unless we
            // add some unique key to each hyphenation dictionary
            // we can worry about that when and if issuezilla's flood in
            numdict = numusr + numshr;

            if (numdict) {
          aDicts = new HDInfo[numdict];
              aSuppLocales.realloc(numdict);
              Locale * pLocale = aSuppLocales.getArray();
              int numlocs = 0;
              int newloc;
              int i,j;
              int k = 0;

              // first add the user dictionaries
              // keeping track of unique locales only
              for (i = 0; i < numusr; i++) {
             Locale nLoc( A2OU(updict->lang), A2OU(updict->region), OUString() );

                 newloc = 1;
             for (j = 0; j < numlocs; j++) {
                     if (nLoc == pLocale[j]) newloc = 0;
                 }
                 if (newloc) {
                     pLocale[numlocs] = nLoc;
                     numlocs++;
                 }
                 aDicts[k].aPtr = NULL;
                 aDicts[k].aLoc = nLoc;
                 aDicts[k].aEnc = 0;
                 aDicts[k].aName = A2OU(updict->filename);
                 k++;
                 updict++;
              }

              // now add the shared dictionaries
              // keeping track of unique locales only
              for (i = 0; i < numshr; i++) {
             Locale nLoc( A2OU(spdict->lang), A2OU(spdict->region), OUString() );

                 newloc = 1;
             for (j = 0; j < numlocs; j++) {
                     if (nLoc == pLocale[j]) newloc = 0;
                 }
                 if (newloc) {
                     pLocale[numlocs] = nLoc;
                     numlocs++;
                 }
                 aDicts[k].aPtr = NULL;
                 aDicts[k].aLoc = nLoc;
                 aDicts[k].aEnc = 0;
                 aDicts[k].aName = A2OU(spdict->filename);
                 k++;
                 spdict++;
              }

              // reallocate the size to just cover the unique locales
              aSuppLocales.realloc(numlocs);

            } else {
             // no dictionary.lst files found so register no dics
                 numdict = 0;
                 aSuppLocales.realloc(0);
                 aDicts = NULL;
            }

            /* de-allocation of memory is handled inside the DictMgr */
            updict = NULL;
            if (udMgr) {
                  delete udMgr;
                  udMgr = NULL;
            }
            spdict = NULL;
            if (sdMgr) {
                  delete sdMgr;
                  sdMgr = NULL;
            }

        }

    return aSuppLocales;
}



sal_Bool SAL_CALL Hyphenator::hasLocale(const Locale& rLocale)
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


Reference< XHyphenatedWord > SAL_CALL
Hyphenator::hyphenate( const ::rtl::OUString& aWord,
               const ::com::sun::star::lang::Locale& aLocale,
               sal_Int16 nMaxLeading,
               const ::com::sun::star::beans::PropertyValues& aProperties )
               throw (com::sun::star::uno::RuntimeException,
               com::sun::star::lang::IllegalArgumentException)
{
    SvtPathOptions aPathOpt;

    int nHyphenationPos = -1;
    int wordlen;
    char *hyphens;
        char *lcword;
        int k = 0;

        PropertyHelper_Hyphen & rHelper = GetPropHelper();
        //rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    HyphenDict *dict = NULL;
        rtl_TextEncoding aEnc = 0;

    Reference< XHyphenatedWord > xRes;

        k = -1;
        for (int j = 0; j < numdict; j++)
          if (aLocale == aDicts[j].aLoc) k = j;


        // if we have a hyphenation dictionary matching this locale
        if (k != -1) {

        // if this dictinary has not been loaded yet do that
        if (!aDicts[k].aPtr) {

           OUString DictFN = aDicts[k].aName + A2OU(".dic");
           OUString userdictpath;
           OUString dictpath;

           osl::FileBase::getSystemPathFromFileURL(
                            aPathOpt.GetUserDictionaryPath() + A2OU("/"),
                            userdictpath);

           osl::FileBase::getSystemPathFromFileURL(
                            aPathOpt.GetLinguisticPath() + A2OU("/ooo/"),
                            dictpath);

               OString uTmp(OU2ENC(userdictpath + DictFN,osl_getThreadTextEncoding()));
               OString sTmp(OU2ENC(dictpath + DictFN,osl_getThreadTextEncoding()));

           if ( ( dict = hnj_hyphen_load ( uTmp.getStr() ) ) == NULL )
              if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
          {
             fprintf(stderr, "Couldn't find file %s and %s\n", OU2ENC(userdictpath + DictFN, osl_getThreadTextEncoding()),  OU2ENC(userdictpath + DictFN, osl_getThreadTextEncoding() ));
             return NULL;
          }
           aDicts[k].aPtr = dict;
               aDicts[k].aEnc = rtl_getTextEncodingFromUnixCharset(dict->cset);
        }

            // other wise hyphenate the word with that dictionary
            dict = aDicts[k].aPtr;
            aEnc = aDicts[k].aEnc;

            OString encWord(OU2ENC(aWord,aEnc));

        wordlen = encWord.getLength();
            lcword = new char[wordlen+1];
        hyphens = new char[wordlen+5];
            enmkallsmall(lcword,encWord.getStr(),dict->cset);
            // now strip off any ending periods
            int n = wordlen-1;
        while((n >=0) && (lcword[n] == '.')) n--;
            n++;
            // fprintf(stderr,"hyphenate... %s\n",lcword); fflush(stderr);
            if (n > 0) {
           if (hnj_hyphen_hyphenate(dict, lcword, n, hyphens))
           {
              //whoops something did not work
              delete[] hyphens;
                  delete[] lcword;
              return NULL;
           }
            }
            // now backfill hyphens[] for any removed trailing periods
            for (int c = n; c < wordlen; c++) hyphens[c] = '0';
            hyphens[wordlen] = '\0';

            // fprintf(stderr,"... %s\n",hyphens); fflush(stderr);
        OUStringBuffer  hyphenatedWordBuffer;
            OUString hyphenatedWord;
        INT32 Leading =  GetPosInWordToCheck( aWord, nMaxLeading );

        for (INT32 i = 0; i < encWord.getLength(); i++)
        {
            hyphenatedWordBuffer.append(aWord[i]);
                BOOL hit = (wordlen >= minLen);
                hit = hit && (hyphens[i]&1) && (i < Leading);
                hit = hit && (i >= (minLead-1) );
                hit = hit && ((wordlen - i - 1) >= minTrail);
            if (hit)
            {
            nHyphenationPos = i;
            hyphenatedWordBuffer.append(sal_Unicode('='));
            }
         }

            hyphenatedWord = hyphenatedWordBuffer.makeStringAndClear();
        // fprintf(stderr,"result is %s\n",OU2A(hyphenatedWord));
            // fflush(stderr);
         if (nHyphenationPos  == -1)
             xRes = NULL;
         else
         {
             xRes = new HyphenatedWord( aWord, LocaleToLanguage( aLocale ), nHyphenationPos,
                       aWord, nHyphenationPos );
         }

         delete[] hyphens;
             delete[] lcword;
         return xRes;

    }
        return NULL;
}


Reference < XHyphenatedWord > SAL_CALL
    Hyphenator::queryAlternativeSpelling( const ::rtl::OUString& aWord,
                  const ::com::sun::star::lang::Locale& aLocale,
                  sal_Int16 nIndex,
                  const ::com::sun::star::beans::PropertyValues& aProperties )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)
{
  /* alternative spelling isn't  supported by tex dictionaries */
  return NULL;
}

Reference< XPossibleHyphens > SAL_CALL
    Hyphenator::createPossibleHyphens( const ::rtl::OUString& aWord,
                   const ::com::sun::star::lang::Locale& aLocale,
                   const ::com::sun::star::beans::PropertyValues& aProperties )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)

{

  SvtPathOptions aPathOpt;
  //  CharClass chclass( rSMgr, aLocale );

  int nHyphenationPos = -1;
  int wordlen;
  char *hyphens;
  char *lcword;
  int k;


  HyphenDict *dict;
  rtl_TextEncoding aEnc;

  Reference< XPossibleHyphens > xRes;

  k = -1;
  for (int j = 0; j < numdict; j++)
     if (aLocale == aDicts[j].aLoc) k = j;

  dict = NULL;
  aEnc = 0;

  // if we have a hyphenation dictionary matching this locale
  if (k != -1) {

      // if this dictioanry has not been loaded yet do that
      if (!aDicts[k].aPtr) {

         OUString DictFN = aDicts[k].aName + A2OU(".dic");
         OUString userdictpath;
         OUString dictpath;
         osl::FileBase::getSystemPathFromFileURL(
                          aPathOpt.GetUserDictionaryPath() + A2OU("/"),
                          userdictpath);

     osl::FileBase::getSystemPathFromFileURL(
                          aPathOpt.GetLinguisticPath() + A2OU("/ooo/"),
                          dictpath);
         OString uTmp(OU2ENC(userdictpath + DictFN,osl_getThreadTextEncoding()));
         OString sTmp(OU2ENC(dictpath + DictFN,osl_getThreadTextEncoding()));


     if ( ( dict = hnj_hyphen_load ( uTmp.getStr() ) ) == NULL )
        if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
        {
           fprintf(stderr, "Couldn't find file %s and %s\n", OU2ENC(userdictpath + DictFN, osl_getThreadTextEncoding()),  OU2ENC(userdictpath + DictFN, osl_getThreadTextEncoding() ));
           return NULL;
        }
     aDicts[k].aPtr = dict;
         aDicts[k].aEnc = rtl_getTextEncodingFromUnixCharset(dict->cset);
      }

      // other wise hyphenate the word with that dictionary
      dict = aDicts[k].aPtr;
      aEnc = aDicts[k].aEnc;


      OString encWord(OU2ENC(aWord, aEnc));

      wordlen = encWord.getLength();
      lcword = new char[wordlen+1];
      hyphens = new char[wordlen+5];
      enmkallsmall(lcword,encWord.getStr(),dict->cset);
      // first remove any trailing periods
      int n = wordlen-1;
      while((n >=0) && (lcword[n] == '.')) n--;
      n++;
      // fprintf(stderr,"hyphenate... %s\n",lcword); fflush(stderr);
      if (n > 0) {
     if (hnj_hyphen_hyphenate(dict, lcword, n, hyphens))
         {
             delete[] hyphens;
             delete[] lcword;
             return NULL;
         }
      }
      // now backfill hyphens[] for any removed periods
      for (int c = n; c < wordlen; c++) hyphens[c] = '0';
      hyphens[wordlen] = '\0';
      // fprintf(stderr,"... %s\n",hyphens); fflush(stderr);

      INT16 nHyphCount = 0;

      for (INT16 i = 0; i < encWord.getLength(); i++)
        if (hyphens[i]&1)
          nHyphCount++;

      Sequence< INT16 > aHyphPos(nHyphCount);
      INT16 *pPos = aHyphPos.getArray();
      OUStringBuffer hyphenatedWordBuffer;
      OUString hyphenatedWord;
      nHyphCount = 0;

      for (i = 0; i < encWord.getLength(); i++)
      {
          hyphenatedWordBuffer.append(aWord[i]);
          if (hyphens[i]&1)
      {
          pPos[nHyphCount] = i;
          hyphenatedWordBuffer.append(sal_Unicode('='));
          nHyphCount++;
      }
      }

      hyphenatedWord = hyphenatedWordBuffer.makeStringAndClear();
      // fprintf(stderr,"result is %s\n",OU2A(hyphenatedWord));
      // fflush(stderr);

      xRes = new PossibleHyphens( aWord, LocaleToLanguage( aLocale ),
                hyphenatedWord, aHyphPos );

      delete[] hyphens;
      delete[] lcword;
      return xRes;
  }

  return NULL;

}



Reference< XInterface > SAL_CALL Hyphenator_CreateInstance(
            const Reference< XMultiServiceFactory > & rSMgr )
        throw(Exception)
{

    Reference< XInterface > xService = (cppu::OWeakObject*) new Hyphenator;
    return xService;
}


sal_Bool SAL_CALL
    Hyphenator::addLinguServiceEventListener(
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
    Hyphenator::removeLinguServiceEventListener(
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
    Hyphenator::getServiceDisplayName( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return A2OU( "ALTLinux LibHnj Hyphenator" );
}


void SAL_CALL
    Hyphenator::initialize( const Sequence< Any >& rArguments )
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
            pPropHelper = new PropertyHelper_Hyphen( (XHyphenator *) this, xPropSet );
            xPropHelper = pPropHelper;
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else
            DBG_ERROR( "wrong number of arguments in sequence" );

    }
}


void SAL_CALL
    Hyphenator::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;
        EventObject aEvtObj( (XHyphenator *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    Hyphenator::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL
    Hyphenator::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

OUString SAL_CALL Hyphenator::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getImplementationName_Static();
}


sal_Bool SAL_CALL Hyphenator::supportsService( const OUString& ServiceName )
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

    Sequence< OUString > aSNS( 1 ); // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_HYPHENATOR );
    return aSNS;
}


sal_Bool SAL_CALL Hyphenator_writeInfo(
            void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{

    try
    {
        String aImpl( '/' );
        aImpl += Hyphenator::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
                pRegistryKey->createKey( aImpl );
        Sequence< OUString > aServices =
                Hyphenator::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}


void * SAL_CALL Hyphenator_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !Hyphenator::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                Hyphenator::getImplementationName_Static(),
                Hyphenator_CreateInstance,
                Hyphenator::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


///////////////////////////////////////////////////////////////////////////
