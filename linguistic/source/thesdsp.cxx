/*************************************************************************
 *
 *  $RCSfile: thesdsp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-27 16:11:15 $
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

#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
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
        pItem[i] = CreateLocale( aSvcList.GetKey( pEntry ) );
        pEntry = aSvcList.Next();
    }
    return aLocales;
}


sal_Bool SAL_CALL
    ThesaurusDispatcher::hasLocale( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return 0 != aSvcList.Seek( LocaleToLanguage( rLocale ) );
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
    SeqLangSvcEntry_Thes *pEntry = aSvcList.Seek( nLanguage );

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        OUString aChkWord( rTerm );
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
                if (pRef[i].is())
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
                    Reference< XThesaurus > xThes(
                            xMgr->createInstanceWithArguments(
                                pImplNames[i], aArgs ),
                            UNO_QUERY );
                    pRef[i] = xThes;

                    if (xThes.is())
                        aMeanings = xThes->queryMeanings( aChkWord, rLocale, rProperties );

                    pEntry->aFlags.nLastTriedSvcIndex = i;
                    ++i;
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

    // search for entry with that language
    INT16 nLanguage = LocaleToLanguage( rLocale );
    SeqLangSvcEntry_Thes *pEntry = aSvcList.Seek( nLanguage );

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
        DBG_ASSERT( aSvcList.Seek( nLanguage ), "lng : Insert failed" );
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
    const SeqLangSvcEntry_Thes  *pEntry = pThis->aSvcList.Seek( nLanguage );
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

