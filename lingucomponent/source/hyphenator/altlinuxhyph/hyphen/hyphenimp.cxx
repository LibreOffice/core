/*************************************************************************
 *
 *  $RCSfile: hyphenimp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nidd $ $Date: 2001-12-25 08:24:12 $
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

}


Hyphenator::~Hyphenator()
{
    if (pPropHelper)
        pPropHelper->RemoveAsPropListener();
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


    if (!aSuppLocales.getLength())
    {

        aSuppLocales.realloc( 2 );
        Locale *pLocale = aSuppLocales.getArray();
        pLocale[0] = Locale( A2OU("en"), A2OU("US"), OUString() );
        pLocale[1] = Locale( A2OU("ru"), A2OU("RU"), OUString() );

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

rtl_TextEncoding L2TE( LanguageType nLang )
{
  switch( nLang )
    {
    case 0x419  : return RTL_TEXTENCODING_KOI8_R; break;         // RUSSIAN

    }
  return RTL_TEXTENCODING_MS_1252;
}

Reference< XHyphenatedWord > SAL_CALL
Hyphenator::hyphenate( const ::rtl::OUString& aWord,
               const ::com::sun::star::lang::Locale& aLocale,
               sal_Int16 nMaxLeading,
               const ::com::sun::star::beans::PropertyValues& aProperties )
{
    SvtPathOptions aPathOpt;
    CharClass chclass( rSMgr, aLocale );

    int nHyphenationPos = -1;
    int wordlen;
    char *hyphens;

    HyphenDict *dict;
    OString encWord;
    OUString tohyphenate;
    Reference< XHyphenatedWord > xRes;

    if ( ( dict = aOpenedDicts.Get ( LocaleToLanguage ( aLocale ) ) ) == NULL )
      {
        OUString DictFN = A2OU("hyph_") + aLocale.Language + A2OU(".dic");
        OUString userdictpath;
        OUString dictpath;
        osl::FileBase::getSystemPathFromFileURL(
                            aPathOpt.GetUserDictionaryPath() + A2OU("/"),
                            userdictpath);

        osl::FileBase::getSystemPathFromFileURL(
                            aPathOpt.GetDictionaryPath() + A2OU("/"),
                            dictpath);

        if ( ( dict = hnj_hyphen_load ( OU2A(userdictpath + DictFN) ) ) == NULL )
          if ( ( dict = hnj_hyphen_load ( OU2A(dictpath + DictFN) ) ) == NULL )
        {
          fprintf(stderr, "Couldn't find file %s and %s\n", OU2A(userdictpath + DictFN),  OU2A(userdictpath + DictFN ));
          return NULL;
        }
        aOpenedDicts.Insert( LocaleToLanguage ( aLocale ), dict );
      }

    rtl_TextEncoding DictionaryEnc = L2TE( LocaleToLanguage( aLocale ) );


//  chclass.toLower(aWord);
    encWord = OUStringToOString (aWord, DictionaryEnc);

    wordlen = encWord.getLength();

    hyphens = new char[wordlen+5];

    if (hnj_hyphen_hyphenate(dict, encWord.getStr(), wordlen, hyphens))
      {
        delete hyphens;
        return NULL;
      }

    OUStringBuffer  hyphenatedWord;
    INT32 Leading =  GetPosInWordToCheck( aWord, nMaxLeading );

    for (INT32 i = 0; i < encWord.getLength(); i++)
      {
        hyphenatedWord.append(aWord[i]);
        if ((hyphens[i]&1)  && (i < Leading))
          {
        nHyphenationPos = i;
        hyphenatedWord.append(sal_Unicode('='));
          }
      }

    if (nHyphenationPos  == -1)
      xRes = NULL;
    else
      {
        xRes = new HyphenatedWord( aWord, LocaleToLanguage( aLocale ), nHyphenationPos,
                       hyphenatedWord.makeStringAndClear(), nHyphenationPos );
      }

    delete hyphens;
    return xRes;
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
  CharClass chclass( rSMgr, aLocale );

  int nHyphenationPos = -1;
  int wordlen;
  char *hyphens;

  HyphenDict *dict;
  OString encWord;
  OUString tohyphenate;
  Reference< XPossibleHyphens > xRes;

  if ( ( dict = aOpenedDicts.Get ( LocaleToLanguage ( aLocale ) ) ) == NULL )
    {
      OUString DictFN = A2OU("hyph_") + aLocale.Language + A2OU(".dic");
      OUString userdictpath;
      OUString dictpath;
      osl::FileBase::getSystemPathFromFileURL(
                          aPathOpt.GetUserDictionaryPath() + A2OU("/"),
                          userdictpath);

      osl::FileBase::getSystemPathFromFileURL(
                          aPathOpt.GetDictionaryPath() + A2OU("/"),
                          dictpath);

      if ( ( dict = hnj_hyphen_load ( OU2A(userdictpath + DictFN) ) ) == NULL )
    if ( ( dict = hnj_hyphen_load ( OU2A(dictpath + DictFN) ) ) == NULL )
      {
        fprintf(stderr, "Couldn't find file %s and %s\n", OU2A(userdictpath + DictFN),  OU2A(userdictpath + DictFN ));
        return NULL;
      }
      aOpenedDicts.Insert( LocaleToLanguage ( aLocale ), dict );
    }

  rtl_TextEncoding DictionaryEnc = L2TE( LocaleToLanguage( aLocale ) );


  //    chclass.toLower(aWord);
  encWord = OUStringToOString (aWord, DictionaryEnc);
  wordlen = encWord.getLength();
  hyphens = new char[wordlen+5];

  if (hnj_hyphen_hyphenate(dict, encWord.getStr(), wordlen, hyphens))
    {
      delete hyphens;
      return NULL;
    }

  Sequence< INT16 > aHyphPos( encWord.getLength());
  INT16 *pPos = aHyphPos.getArray();
  OUStringBuffer hyphenatedWordBuffer;
  OUString hyphenatedWord;
  INT16 nHyphCount = 0;

  for (INT16 i = 0; i < encWord.getLength(); i++)
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

  xRes = new PossibleHyphens( aWord, LocaleToLanguage( aLocale ),
                hyphenatedWord, aHyphPos );

  delete hyphens;
  return xRes;
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
