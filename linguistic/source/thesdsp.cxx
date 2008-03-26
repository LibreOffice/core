/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thesdsp.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-26 09:06:37 $
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
#include "precompiled_linguistic.hxx"

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LNGMISC_HXX_
#include <svtools/lngmisc.hxx>
#endif

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include "thesdsp.hxx"
#include "lngprops.hxx"

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

static BOOL SvcListHasLanguage(
        const Sequence< Reference< XThesaurus > > &rRefs,
        const Locale &rLocale )
{
    BOOL bHasLanguage = FALSE;

    const Reference< XThesaurus > *pRef = rRefs.getConstArray();
    INT32 nLen = rRefs.getLength();
    for (INT32 k = 0;  k < nLen  &&  !bHasLanguage;  ++k)
    {
        if (pRef[k].is())
            bHasLanguage = pRef[k]->hasLocale( rLocale );
    }

    return bHasLanguage;
}

///////////////////////////////////////////////////////////////////////////

SeqLangSvcEntry_Thes::~SeqLangSvcEntry_Thes()
{
}


SeqLangSvcEntry_Thes::SeqLangSvcEntry_Thes(
        const Sequence< OUString > &rSvcImplNames ) :
    aSvcImplNames   ( rSvcImplNames ),
    aSvcRefs        ( rSvcImplNames.getLength() )
{
}

///////////////////////////////////////////////////////////////////////////

ThesaurusDispatcher::ThesaurusDispatcher()
{
}


ThesaurusDispatcher::~ThesaurusDispatcher()
{
    ClearSvcList();
}


void ThesaurusDispatcher::ClearSvcList()
{
    // release memory for each table entry
    SeqLangSvcEntry_Thes *pItem = aSvcList.First();
    while (pItem)
    {
        SeqLangSvcEntry_Thes *pTmp = pItem;
        pItem = aSvcList.Next();
        delete pTmp;
    }
}


Sequence< Locale > SAL_CALL
    ThesaurusDispatcher::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    ULONG nCnt = aSvcList.Count();
    Sequence< Locale > aLocales( nCnt );
    Locale *pItem = aLocales.getArray();
    SeqLangSvcEntry_Thes *pEntry = aSvcList.First();
    for (ULONG i = 0;  i < nCnt;  i++)
    {
        DBG_ASSERT( pEntry, "lng : pEntry is NULL pointer" );
        pItem[i] = CreateLocale( (LanguageType) aSvcList.GetKey( pEntry ) );
        pEntry = aSvcList.Next();
    }
    return aLocales;
}


sal_Bool SAL_CALL
    ThesaurusDispatcher::hasLocale( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return 0 != aSvcList.Get( LocaleToLanguage( rLocale ) );
}


Sequence< Reference< XMeaning > > SAL_CALL
    ThesaurusDispatcher::queryMeanings(
            const OUString& rTerm, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Reference< XMeaning > >   aMeanings;

    INT16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !rTerm.getLength())
        return aMeanings;

    // search for entry with that language
    SeqLangSvcEntry_Thes *pEntry = aSvcList.Get( nLanguage );

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        OUString aChkWord( rTerm );
        aChkWord = aChkWord.replace( SVT_HARD_SPACE, ' ' );
        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        INT32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        INT32 i = 0;

        // try already instantiated services first
        {
            const Reference< XThesaurus > *pRef = pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->aFlags.nLastTriedSvcIndex
                   &&  aMeanings.getLength() == 0)
            {
                if (pRef[i].is()  &&  pRef[i]->hasLocale( rLocale ))
                    aMeanings = pRef[i]->queryMeanings( aChkWord, rLocale, rProperties );
                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if (aMeanings.getLength() == 0
            &&  pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XThesaurus > *pRef = pEntry->aSvcRefs.getArray();

            Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
            if (xMgr.is())
            {
                // build service initialization argument
                Sequence< Any > aArgs(1);
                aArgs.getArray()[0] <<= GetPropSet();

                while (i < nLen  &&  aMeanings.getLength() == 0)
                {
                    // create specific service via it's implementation name
                    Reference< XThesaurus > xThes;
                    try
                    {
                        xThes = Reference< XThesaurus >(
                                xMgr->createInstanceWithArguments(
                                pImplNames[i], aArgs ), UNO_QUERY );
                    }
                    catch (uno::Exception &)
                    {
                        DBG_ERROR( "createInstanceWithArguments failed" );
                    }
                    pRef[i] = xThes;

                    if (xThes.is()  &&  xThes->hasLocale( rLocale ))
                        aMeanings = xThes->queryMeanings( aChkWord, rLocale, rProperties );

                    pEntry->aFlags.nLastTriedSvcIndex = (INT16) i;
                    ++i;
                }

                // if language is not supported by any of the services
                // remove it from the list.
                if (i == nLen  &&  aMeanings.getLength() == 0)
                {
                    if (!SvcListHasLanguage( pEntry->aSvcRefs, rLocale ))
                        aSvcList.Remove( nLanguage );
                }
            }
        }
    }

    return aMeanings;
}


void ThesaurusDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 nLanguage = LocaleToLanguage( rLocale );

    if (0 == rSvcImplNames.getLength())
        // remove entry
        aSvcList.Remove( nLanguage );
    else
    {
        // modify/add entry
        SeqLangSvcEntry_Thes *pEntry = aSvcList.Get( nLanguage );
        if (pEntry)
        {
            pEntry->aSvcImplNames = rSvcImplNames;
            pEntry->aSvcRefs = Sequence< Reference < XThesaurus > >(
                                    rSvcImplNames.getLength() );
            pEntry->aFlags = SvcFlags();
        }
        else
        {
            pEntry = new SeqLangSvcEntry_Thes( rSvcImplNames );
            aSvcList.Insert( nLanguage, pEntry );
            DBG_ASSERT( aSvcList.Get( nLanguage ), "lng : Insert failed" );
        }
    }
}


Sequence< OUString >
    ThesaurusDispatcher::GetServiceList( const Locale &rLocale ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes;

    // search for entry with that language and use data from that
    INT16 nLanguage = LocaleToLanguage( rLocale );
    ThesaurusDispatcher         *pThis = (ThesaurusDispatcher *) this;
    const SeqLangSvcEntry_Thes  *pEntry = pThis->aSvcList.Get( nLanguage );
    if (pEntry)
        aRes = pEntry->aSvcImplNames;

    return aRes;
}


ThesaurusDispatcher::DspType
    ThesaurusDispatcher::GetDspType() const
{
    return DSP_THES;
}


///////////////////////////////////////////////////////////////////////////

