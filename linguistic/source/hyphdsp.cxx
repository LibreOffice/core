/*************************************************************************
 *
 *  $RCSfile: hyphdsp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:35 $
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


#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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

#include "hyphdsp.hxx"
#include "hyphdta.hxx"
#include "lngprops.hxx"
#include "lngsvcmgr.hxx"


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

LangSvcEntry_Hyph::~LangSvcEntry_Hyph()
{
}


LangSvcEntry_Hyph::LangSvcEntry_Hyph( const ::rtl::OUString &rSvcImplName ) :
    aSvcImplName( rSvcImplName )
{
}

///////////////////////////////////////////////////////////////////////////

HyphenatorDispatcher::HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr ) :
    rMgr    (rLngSvcMgr)
{
}


HyphenatorDispatcher::~HyphenatorDispatcher()
{
    ClearSvcList();
}


void HyphenatorDispatcher::ClearSvcList()
{
    // release memory for each table entry
    LangSvcEntry_Hyph *pItem = aSvcList.First();
    while (pItem)
    {
        LangSvcEntry_Hyph *pTmp = pItem;
        pItem = aSvcList.Next();
        delete pTmp;
    }
}


Reference<XHyphenatedWord>  HyphenatorDispatcher::buildHyphWord(
            const Reference<XDictionaryEntry> &xEntry,
            INT16 nLang, INT16 nMaxLeading )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord > xRes;

    if (xEntry.is())
    {
        OUString aText( xEntry->getDictionaryWord() );
        INT32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=')
        {
            INT16 nHyphenationPos = -1;

            OUStringBuffer aTmp( nTextLen );
            BOOL  bSkip = FALSE;
            INT32 nHyphIdx = -1;
            INT32 nLeading = 0;
            for (INT32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp != '=')
                {
                    aTmp.append( cTmp );
                    nLeading++;
                    bSkip = FALSE;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                    {
                        if (nLeading <= nMaxLeading)
                            nHyphenationPos = nHyphIdx;
                    }
                    bSkip = TRUE;   //! multiple '=' should count as one only
                }
            }

            if (nHyphenationPos > 0)
            {
                aText = aTmp.makeStringAndClear();
                xRes = new HyphenatedWord( aText, nLang, nHyphenationPos,
                                aText, nHyphenationPos );
            }
        }
    }

    return xRes;
}


Reference< XPossibleHyphens > HyphenatorDispatcher::buildPossHyphens(
            const Reference< XDictionaryEntry > &xEntry, INT16 nLanguage )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference<XPossibleHyphens> xRes;

    if (xEntry.is())
    {
        // text with hyphenation info
        OUString aText( xEntry->getDictionaryWord() );
        INT32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=')
        {
            // sequence to hold hyphenation positions
            Sequence< INT16 > aHyphPos( nTextLen );
            INT16 *pPos = aHyphPos.getArray();
            INT32 nHyphCount = 0;

            OUStringBuffer aTmp( nTextLen );
            BOOL  bSkip = FALSE;
            INT32 nHyphIdx = -1;
            for (INT32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp != '=')
                {
                    aTmp.append( cTmp );
                    bSkip = FALSE;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                        pPos[ nHyphCount++ ] = nHyphIdx;
                    bSkip = TRUE;   //! multiple '=' should count as one only
                }
            }

            // ignore (multiple) trailing '='
            if (bSkip  &&  nHyphIdx >= 0)
            {
                nHyphCount--;
            }
            DBG_ASSERT( nHyphCount >= 0, "lng : invalid hyphenation count");

            if (nHyphCount > 0)
            {
                aHyphPos.realloc( nHyphCount );
                xRes = new PossibleHyphens( aTmp.makeStringAndClear(), nLanguage,
                                aText, aHyphPos );
            }
        }
    }

    return xRes;
}


Sequence< Locale > SAL_CALL HyphenatorDispatcher::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    ULONG nCnt = aSvcList.Count();
    Sequence< Locale > aLocales( nCnt );
    Locale *pItem = aLocales.getArray();
    LangSvcEntry_Hyph *pEntry = aSvcList.First();
    for (ULONG i = 0;  i < nCnt;  i++)
    {
        DBG_ASSERT( pEntry, "lng : pEntry is NULL pointer" );
        pItem[i] = CreateLocale( aSvcList.GetKey( pEntry ) );
        pEntry = aSvcList.Next();
    }
    return aLocales;
}


BOOL SAL_CALL HyphenatorDispatcher::hasLocale(const Locale& rLocale)
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return 0 != aSvcList.Seek( LocaleToLanguage( rLocale ) );
}


Reference< XHyphenatedWord > SAL_CALL
    HyphenatorDispatcher::hyphenate(
            const OUString& rWord, const Locale& rLocale, sal_Int16 nMaxLeading,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    INT16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return xRes;

    // search for entry with that language
    LangSvcEntry_Hyph *pEntry = aSvcList.Seek( nLanguage );

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( rWord, rLocale,
                        TRUE, FALSE );
        }

        if (xEntry.is())
        {
            xRes = buildHyphWord( xEntry, nLanguage, nMaxLeading );
        }
        else
        {
            INT32 nLen = pEntry->aSvcImplName.getLength() ? 1 : 0;
            DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            INT32 i = 0;
            Reference< XHyphenator > &rHyph = pEntry->aSvcRef;

            // try already instantiated service
            if (i <= pEntry->aFlags.nLastTriedSvcIndex)
            {
                if (rHyph.is())
                    xRes = rHyph->hyphenate( rWord, rLocale, nMaxLeading,
                                            rProperties );
                ++i;
            }
            else if (pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();
                    //! The dispatcher searches the dictionary-list
                    //! thus the service needs not to now about it
                    //aArgs.getArray()[1] <<= GetDicList();

                    // create specific service via it's implementation name
                    Reference< XHyphenator > xHyph(
                            xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplName, aArgs ),
                            UNO_QUERY );
                    rHyph = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (rHyph.is())
                        xRes = rHyph->hyphenate( rWord, rLocale, nMaxLeading,
                                                rProperties );

                    pEntry->aFlags.nLastTriedSvcIndex = i;
                    ++i;
                }
            }
        }   // if (xEntry.is())
    }

    return xRes;
}


Reference< XHyphenatedWord > SAL_CALL
    HyphenatorDispatcher::queryAlternativeSpelling(
            const OUString& rWord, const Locale& rLocale, sal_Int16 nIndex,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    INT16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return xRes;

    // search for entry with that language
    LangSvcEntry_Hyph *pEntry = aSvcList.Seek( nLanguage );

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( rWord, rLocale,
                        TRUE, FALSE );
        }

        if (xEntry.is())
        {
            //! alternative spellings not yet supported by dictionaries
        }
        else
        {
            INT32 nLen = pEntry->aSvcImplName.getLength() ? 1 : 0;
            DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            INT32 i = 0;
            Reference< XHyphenator > &rHyph = pEntry->aSvcRef;

            // try already instantiated service
            if (i <= pEntry->aFlags.nLastTriedSvcIndex)
            {
                if (rHyph.is())
                    xRes = rHyph->queryAlternativeSpelling( rWord, rLocale,
                                nIndex, rProperties );
                ++i;
            }
            else if (pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();
                    //! The dispatcher searches the dictionary-list
                    //! thus the service needs not to now about it
                    //aArgs.getArray()[1] <<= GetDicList();

                    // create specific service via it's implementation name
                    Reference< XHyphenator > xHyph(
                            xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplName, aArgs ),
                            UNO_QUERY );
                    rHyph = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (rHyph.is())
                        xRes = rHyph->queryAlternativeSpelling( rWord, rLocale,
                                    nIndex, rProperties );

                    pEntry->aFlags.nLastTriedSvcIndex = i;
                    ++i;
                }
            }
        }   // if (xEntry.is())
    }

    return xRes;
}


Reference< XPossibleHyphens > SAL_CALL
    HyphenatorDispatcher::createPossibleHyphens(
            const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XPossibleHyphens >   xRes;

    INT16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return xRes;

    // search for entry with that language
    LangSvcEntry_Hyph *pEntry = aSvcList.Seek( nLanguage );

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( rWord, rLocale,
                        TRUE, FALSE );
        }

        if (xEntry.is())
        {
            xRes = buildPossHyphens( xEntry, nLanguage );
        }
        else
        {
            INT32 nLen = pEntry->aSvcImplName.getLength() ? 1 : 0;
            DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            INT32 i = 0;
            Reference< XHyphenator > &rHyph = pEntry->aSvcRef;

            // try already instantiated service
            if (i <= pEntry->aFlags.nLastTriedSvcIndex)
            {
                if (rHyph.is())
                    xRes = rHyph->createPossibleHyphens( rWord, rLocale,
                                rProperties );
                ++i;
            }
            else if (pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();
                    //! The dispatcher searches the dictionary-list
                    //! thus the service needs not to now about it
                    //aArgs.getArray()[1] <<= GetDicList();

                    // create specific service via it's implementation name
                    Reference< XHyphenator > xHyph(
                            xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplName, aArgs ),
                            UNO_QUERY );
                    rHyph = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (rHyph.is())
                    xRes = rHyph->createPossibleHyphens( rWord, rLocale,
                                rProperties );

                    pEntry->aFlags.nLastTriedSvcIndex = i;
                    ++i;
                }
            }
        }   // if (xEntry.is())
    }

    return xRes;
}


void HyphenatorDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = rSvcImplNames.getLength();
    DBG_ASSERT( 1 == nLen, "unexpected size of sequence" );
    OUString aSvcImplName( nLen ? rSvcImplNames.getConstArray()[0] : OUString() );

    // search for entry with that language
    INT16 nLanguage = LocaleToLanguage( rLocale );
    LangSvcEntry_Hyph *pEntry = aSvcList.Seek( nLanguage );

    if (pEntry)
    {
        pEntry->aSvcImplName = aSvcImplName;
        pEntry->aSvcRef = NULL;
        pEntry->aFlags  = SvcFlags();
    }
    else
    {
        pEntry = new LangSvcEntry_Hyph( aSvcImplName );
        aSvcList.Insert( nLanguage, pEntry );
        DBG_ASSERT( aSvcList.Seek( nLanguage ), "lng : Insert failed" );
    }
}


Sequence< OUString >
    HyphenatorDispatcher::GetServiceList( const Locale &rLocale )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes(1);

    // search for entry with that language and use data from that
    INT16 nLanguage = LocaleToLanguage( rLocale );
    LangSvcEntry_Hyph *pEntry = aSvcList.Seek( nLanguage );
    if (pEntry)
        aRes.getArray()[0] = pEntry->aSvcImplName;
    else
        aRes.realloc(0);

    return aRes;
}


///////////////////////////////////////////////////////////////////////////

