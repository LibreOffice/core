/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyphenimp.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:48:36 $
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


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif

//#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

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
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#include <osl/file.hxx>

#include "dictmgr.hxx"

#include <stdio.h>

#include <list>
#include <set>

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

// values asigned to capitalization types
#define CAPTYPE_UNKNOWN 0
#define CAPTYPE_NOCAP   1
#define CAPTYPE_INITCAP 2
#define CAPTYPE_ALLCAP  3
#define CAPTYPE_MIXED   4

///////////////////////////////////////////////////////////////////////////


Hyphenator::Hyphenator() :
    aEvtListeners   ( GetLinguMutex() )
{
    bDisposing = FALSE;
    pPropHelper = NULL;
        aDicts = NULL;
        numdict = 0;

}


Hyphenator::~Hyphenator()
{
    if (pPropHelper)
        pPropHelper->RemoveAsPropListener();

        if ((numdict) && (aDicts)) {
      for (int i=0; i < numdict; i++) {
            if (aDicts[i].apCC) delete aDicts[i].apCC;
            aDicts[i].apCC = NULL;
      }
    }
        if (aDicts) delete[] aDicts;
    aDicts = NULL;
        numdict = 0;
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

    // this routine should return the locales supported by the installed
    // dictionaries.

    if (!numdict)
    {
        SvtLinguConfig aLinguCfg;

        // get list of dictionaries-to-use
        // (or better speaking: the list of dictionaries using the
        // new configuration entries).
        std::list< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< rtl::OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( A2OU("Hyphenators"),
                A2OU("org.openoffice.lingu.LibHnjHyphenator"), aFormatList );
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
                GetOldStyleDics( "HYPH" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list od new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

        numdict = aDics.size();
        if (numdict)
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
            aDicts = new HDInfo[numdict];
/*
            aTEncs  = new rtl_TextEncoding [numdict];
            aTLocs  = new Locale [numdict];
            aTNames = new OUString [numdict];
            aCharSetInfo = new CharClass* [numdict];
*/
            k = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                if (aDictIt->aLocaleNames.getLength() > 0 &&
                    aDictIt->aLocations.getLength() > 0)
                {
                    aDicts[k].aPtr = NULL;
                    aDicts[k].aEnc = 0;
                    // currently HunSpell supports only one language per dictionary...
                    aDicts[k].aLoc = MsLangId::convertLanguageToLocale(
                                    MsLangId::convertIsoStringToLanguage( aDictIt->aLocaleNames[0] ));
                    aDicts[k].apCC = new CharClass( aDicts[k].aLoc );
                    // also both files have to be in the same directory and the
                    // file names must only differ in the extension (.aff/.dic).
                    // Thus we use the first location only and strip the extension part.
                    rtl::OUString aLocation = aDictIt->aLocations[0];
                    sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                    aLocation = aLocation.copy( 0, nPos );
                    aDicts[k].aName = aLocation;
                }
                ++k;
            }
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

    BOOL bRes = FALSE;
    if (!aSuppLocales.getLength())
        getLocales();

    const Locale *pLocale = aSuppLocales.getConstArray();
    INT32 nLen = aSuppLocales.getLength();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
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
    int nHyphenationPos = -1;
        int nHyphenationPosAlt = -1;
        int nHyphenationPosAltHyph = -1;
    int wordlen;
    char *hyphens;
        char *lcword;
        int k = 0;

        PropertyHelper_Hyphen & rHelper = GetPropHelper();
        rHelper.SetTmpPropVals(aProperties);
    sal_Int16 minTrail = rHelper.GetMinTrailing();
    sal_Int16 minLead = rHelper.GetMinLeading();
    sal_Int16 minLen = rHelper.GetMinWordLength();

    HyphenDict *dict = NULL;
        rtl_TextEncoding aEnc = 0;
        CharClass * pCC = NULL;

    Reference< XHyphenatedWord > xRes;

        k = -1;
        for (int j = 0; j < numdict; j++)
          if (aLocale == aDicts[j].aLoc) k = j;


        // if we have a hyphenation dictionary matching this locale
        if (k != -1) {

        // if this dictinary has not been loaded yet do that
        if (!aDicts[k].aPtr) {

           OUString DictFN = aDicts[k].aName + A2OU(".dic");
           OUString dictpath;

           osl::FileBase::getSystemPathFromFileURL( DictFN, dictpath );
           OString sTmp( OU2ENC( dictpath, osl_getThreadTextEncoding() ) );

#if defined(WNT)
        // workaround for Windows specifc problem that the
        // path length in calls to 'fopen' is limted to somewhat
        // about 120+ characters which will usually be exceed when
        // using dictionaries as extensions.
        sTmp = Win_GetShortPathName( dictpath );
#endif

          if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
          {
             fprintf(stderr, "Couldn't find file %s\n", OU2ENC(dictpath, osl_getThreadTextEncoding()) );
             return NULL;
          }
           aDicts[k].aPtr = dict;
               aDicts[k].aEnc = rtl_getTextEncodingFromUnixCharset(dict->cset);
               if (aDicts[k].aEnc == RTL_TEXTENCODING_DONTKNOW) {
                  if (strcmp("ISCII-DEVANAGARI", dict->cset) == 0) {
                     aDicts[k].aEnc = RTL_TEXTENCODING_ISCII_DEVANAGARI;
                  } else if (strcmp("UTF-8", dict->cset) == 0) {
                     aDicts[k].aEnc = RTL_TEXTENCODING_UTF8;
                  }
               }
        }

            // other wise hyphenate the word with that dictionary
            dict = aDicts[k].aPtr;
            aEnc = aDicts[k].aEnc;
            pCC =  aDicts[k].apCC;

            sal_uInt16 ct = CAPTYPE_UNKNOWN;
            ct = capitalType(aWord, pCC);

            // first convert any smart quotes or apostrophes to normal ones
        OUStringBuffer rBuf(aWord);
            sal_Int32 nc = rBuf.getLength();
            sal_Unicode ch;
        for (sal_Int32 ix=0; ix < nc; ix++) {
            ch = rBuf.charAt(ix);
                if ((ch == 0x201C) || (ch == 0x201D)) rBuf.setCharAt(ix,(sal_Unicode)0x0022);
                if ((ch == 0x2018) || (ch == 0x2019)) rBuf.setCharAt(ix,(sal_Unicode)0x0027);
            }
            OUString nWord(rBuf.makeStringAndClear());

            // now convert word to all lowercase for pattern recognition
            OUString nTerm(makeLowerCase(nWord, pCC));

            // now convert word to needed encoding
            OString encWord(OU2ENC(nTerm,aEnc));

        wordlen = encWord.getLength();
            lcword = new char[wordlen + 1];
        hyphens = new char[wordlen + 5];

            char ** rep = NULL; // replacements of discretionary hyphenation
            int * pos = NULL; // array of [hyphenation point] minus [deletion position]
            int * cut = NULL; // length of deletions in original word

            // copy converted word into simple char buffer
            strcpy(lcword,encWord.getStr());

            // now strip off any ending periods
            int n = wordlen-1;
        while((n >=0) && (lcword[n] == '.')) n--;
            n++;
            if (n > 0) {
           if (hnj_hyphen_hyphenate2(dict, lcword, n, hyphens, NULL, &rep, &pos, &cut))
           {
              //whoops something did not work
              delete[] hyphens;
                  delete[] lcword;
                  if (rep) {
                    for(int j = 0; j < n; j++) {
                        if (rep[j]) free(rep[j]);
                    }
                    free(rep);
                  }
                  if (pos) free(pos);
                  if (cut) free(cut);
              return NULL;
           }
            }

            // now backfill hyphens[] for any removed trailing periods
            for (int c = n; c < wordlen; c++) hyphens[c] = '0';
            hyphens[wordlen] = '\0';

        INT32 Leading =  GetPosInWordToCheck( aWord, nMaxLeading );

        for (INT32 i = 0; i < n; i++)
        {
                int leftrep = 0;
                BOOL hit = (n >= minLen);
                if (!rep || !rep[i] || (i >= n)) {
                    hit = hit && (hyphens[i]&1) && (i < Leading);
                    hit = hit && (i >= (minLead-1) );
                    hit = hit && ((n - i - 1) >= minTrail);
                } else {
                    // calculate change character length before hyphenation point signed with '='
                    for (char * c = rep[i]; *c && (*c != '='); c++) {
                        if (aEnc == RTL_TEXTENCODING_UTF8) {
                            if (((unsigned char) *c) >> 6 != 2) leftrep++;
                        } else leftrep++;
                    }
                    hit = hit && (hyphens[i]&1) && ((i + leftrep - pos[i]) < Leading);
                    hit = hit && ((i + leftrep - pos[i]) >= (minLead-1) );
                    hit = hit && ((n - i - 1 + sal::static_int_cast< sal_sSize >(strlen(rep[i])) - leftrep - 1) >= minTrail);
                }
            if (hit) {
            nHyphenationPos = i;
                    if (rep && (i < n) && rep[i]) {
                        nHyphenationPosAlt = i - pos[i];
                        nHyphenationPosAltHyph = i + leftrep - pos[i];
                    }
                }
         }

         if (nHyphenationPos  == -1) {
             xRes = NULL;
             } else {
                if (rep && rep[nHyphenationPos]) {
                    // remove equal sign
                    char * s = rep[nHyphenationPos];
                    int eq = 0;
                    for (; *s; s++) {
                        if (*s == '=') eq = 1;
                        if (eq) *s = *(s + 1);
                    }
                    OUString repHyphlow(rep[nHyphenationPos], strlen(rep[nHyphenationPos]), aEnc);
                    OUString repHyph;
                    switch (ct) {
                       case CAPTYPE_ALLCAP:
                     {
                               repHyph = makeUpperCase(repHyphlow, pCC);
                               break;
                             }
                       case CAPTYPE_INITCAP:
                     {
                               if (nHyphenationPosAlt == 0) {
                                    repHyph = makeInitCap(repHyphlow, pCC);
                               } else {
                                    repHyph = repHyphlow;
                               }
                               break;
                             }
                           default:
                     {
                               repHyph = repHyphlow;
                               break;
                             }
            }

                    // handle shortening
                    INT16 nPos = (INT16) ((nHyphenationPosAltHyph < nHyphenationPos) ?
                        nHyphenationPosAltHyph : nHyphenationPos);
                    // dicretionary hyphenation
                xRes = new HyphenatedWord( aWord, LocaleToLanguage( aLocale ), nPos,
                aWord.replaceAt(nHyphenationPosAlt + 1, cut[nHyphenationPos], repHyph),
                        (INT16) nHyphenationPosAltHyph);
                } else {
                xRes = new HyphenatedWord( aWord, LocaleToLanguage( aLocale ), nHyphenationPos,
                       aWord, (INT16) nHyphenationPos);
                }
        }

            delete[] lcword;
        delete[] hyphens;
            if (rep) {
                for(int j = 0; j < n; j++) {
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


Reference < XHyphenatedWord > SAL_CALL
    Hyphenator::queryAlternativeSpelling( const ::rtl::OUString& /*aWord*/,
                  const ::com::sun::star::lang::Locale& /*aLocale*/,
                  sal_Int16 /*nIndex*/,
                  const ::com::sun::star::beans::PropertyValues& /*aProperties*/ )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)
{
  /* alternative spelling isn't supported by tex dictionaries */
  /* XXX: OOo's extended libhjn algorithm can support alternative spellings with extended TeX dic. */
  /* TASK: implement queryAlternativeSpelling() */
  return NULL;
}

Reference< XPossibleHyphens > SAL_CALL
    Hyphenator::createPossibleHyphens( const ::rtl::OUString& aWord,
                   const ::com::sun::star::lang::Locale& aLocale,
                   const ::com::sun::star::beans::PropertyValues& /*aProperties*/ )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)

{
  int wordlen;
  char *hyphens;
  char *lcword;
  int k;


  HyphenDict *dict = NULL;
  rtl_TextEncoding aEnc = 0;
  CharClass* pCC = NULL;

  Reference< XPossibleHyphens > xRes;

  k = -1;
  for (int j = 0; j < numdict; j++)
     if (aLocale == aDicts[j].aLoc) k = j;


  // if we have a hyphenation dictionary matching this locale
  if (k != -1) {

      // if this dictioanry has not been loaded yet do that
      if (!aDicts[k].aPtr) {

         OUString DictFN = aDicts[k].aName + A2OU(".dic");
         OUString dictpath;

         osl::FileBase::getSystemPathFromFileURL( DictFN, dictpath );
         OString sTmp( OU2ENC( dictpath, osl_getThreadTextEncoding() ) );

#if defined(WNT)
        // workaround for Windows specifc problem that the
        // path length in calls to 'fopen' is limted to somewhat
        // about 120+ characters which will usually be exceed when
        // using dictionaries as extensions.
        sTmp = Win_GetShortPathName( dictpath );
#endif

        if ( ( dict = hnj_hyphen_load ( sTmp.getStr()) ) == NULL )
        {
           fprintf(stderr, "Couldn't find file %s and %s\n", OU2ENC(dictpath, osl_getThreadTextEncoding()) );
           return NULL;
        }
        aDicts[k].aPtr = dict;
         aDicts[k].aEnc = rtl_getTextEncodingFromUnixCharset(dict->cset);
         if (aDicts[k].aEnc == RTL_TEXTENCODING_DONTKNOW) {
            if (strcmp("ISCII-DEVANAGARI", dict->cset) == 0) {
               aDicts[k].aEnc = RTL_TEXTENCODING_ISCII_DEVANAGARI;
            } else if (strcmp("UTF-8", dict->cset) == 0) {
               aDicts[k].aEnc = RTL_TEXTENCODING_UTF8;
            }
         }
      }

      // other wise hyphenate the word with that dictionary
      dict = aDicts[k].aPtr;
      aEnc = aDicts[k].aEnc;
      pCC  = aDicts[k].apCC;

      // first handle smart quotes both single and double
      OUStringBuffer rBuf(aWord);
      sal_Int32 nc = rBuf.getLength();
      sal_Unicode ch;
      for (sal_Int32 ix=0; ix < nc; ix++) {
      ch = rBuf.charAt(ix);
          if ((ch == 0x201C) || (ch == 0x201D)) rBuf.setCharAt(ix,(sal_Unicode)0x0022);
          if ((ch == 0x2018) || (ch == 0x2019)) rBuf.setCharAt(ix,(sal_Unicode)0x0027);
      }
      OUString nWord(rBuf.makeStringAndClear());

      // now convert word to all lowercase for pattern recognition
      OUString nTerm(makeLowerCase(nWord, pCC));

      // now convert word to needed encoding
      OString encWord(OU2ENC(nTerm,aEnc));

      wordlen = encWord.getLength();
      lcword = new char[wordlen+1];
      hyphens = new char[wordlen+5];

      // copy converted word into simple char buffer
      strcpy(lcword,encWord.getStr());

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
      INT16 i;

      for ( i = 0; i < encWord.getLength(); i++)
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
      //fprintf(stderr,"result is %s\n",OU2A(hyphenatedWord));
      //fflush(stderr);

      xRes = new PossibleHyphens( aWord, LocaleToLanguage( aLocale ),
                hyphenatedWord, aHyphPos );

      delete[] hyphens;
      delete[] lcword;
      return xRes;
  }

  return NULL;

}

sal_uInt16 SAL_CALL Hyphenator::capitalType(const OUString& aTerm, CharClass * pCC)
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

OUString SAL_CALL Hyphenator::makeLowerCase(const OUString& aTerm, CharClass * pCC)
{
        if (pCC)
      return pCC->toLower_rtl(aTerm, 0, aTerm.getLength());
        return aTerm;
}

OUString SAL_CALL Hyphenator::makeUpperCase(const OUString& aTerm, CharClass * pCC)
{
        if (pCC)
              return pCC->toUpper_rtl(aTerm, 0, aTerm.getLength());
        return aTerm;
}


OUString SAL_CALL Hyphenator::makeInitCap(const OUString& aTerm, CharClass * pCC)
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




Reference< XInterface > SAL_CALL Hyphenator_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
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
    Hyphenator::getServiceDisplayName( const Locale& /*rLocale*/ )
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

#undef CAPTYPE_UNKNOWN
#undef CAPTYPE_NOCAP
#undef CAPTYPE_INITCAP
#undef CAPTYPE_ALLCAP
#undef CAPTYPE_MIXED
