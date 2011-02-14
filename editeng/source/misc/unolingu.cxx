/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <map>
#include <set>
#include <vector>
#include <slist>
#include <memory>
#include <editeng/unolingu.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <rtl/logfile.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <i18npool/mslangid.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/msgbox.hxx>
#include <tools/shl.hxx>
#include <linguistic/misc.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::linguistic;
using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;

#define CSS com::sun::star

///////////////////////////////////////////////////////////////////////////


static uno::Reference< XLinguServiceManager > GetLngSvcMgr_Impl()
{
    uno::Reference< XLinguServiceManager > xRes;
    uno::Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if (xMgr.is())
    {
        xRes = uno::Reference< XLinguServiceManager > ( xMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.linguistic2.LinguServiceManager" ) ) ), UNO_QUERY ) ;
    }
    return xRes;
}

///////////////////////////////////////////////////////////////////////////

sal_Bool lcl_FindEntry( const OUString &rEntry, const Sequence< OUString > &rCfgSvcs )
{
    sal_Int32 nRes = -1;
    sal_Int32 nEntries = rCfgSvcs.getLength();
    const OUString *pEntry = rCfgSvcs.getConstArray();
    for (sal_Int32 i = 0;  i < nEntries && nRes == -1;  ++i)
    {
        if (rEntry == pEntry[i])
            nRes = i;
    }
    return nRes != -1;
}


Sequence< OUString > lcl_RemoveMissingEntries(
        const Sequence< OUString > &rCfgSvcs,
        const Sequence< OUString > &rAvailSvcs )
{
    Sequence< OUString > aRes( rCfgSvcs.getLength() );
    OUString *pRes = aRes.getArray();
    sal_Int32 nCnt = 0;

    sal_Int32 nEntries = rCfgSvcs.getLength();
    const OUString *pEntry = rCfgSvcs.getConstArray();
    for (sal_Int32 i = 0;  i < nEntries;  ++i)
    {
        if (pEntry[i].getLength() && lcl_FindEntry( pEntry[i], rAvailSvcs ))
            pRes[ nCnt++ ] = pEntry[i];
    }

    aRes.realloc( nCnt );
    return aRes;
}


Sequence< OUString > lcl_GetLastFoundSvcs(
        SvtLinguConfig &rCfg,
        const OUString &rLastFoundList ,
        const Locale &rAvailLocale )
{
    Sequence< OUString > aRes;

    OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                SvxLocaleToLanguage( rAvailLocale ) ) );

    Sequence< OUString > aNodeNames( rCfg.GetNodeNames(rLastFoundList) );
    sal_Bool bFound = lcl_FindEntry( aCfgLocaleStr, aNodeNames);

    if (bFound)
    {
        Sequence< OUString > aNames(1);
        OUString &rNodeName = aNames.getArray()[0];
        rNodeName = rLastFoundList;
        rNodeName += OUString::valueOf( (sal_Unicode)'/' );
        rNodeName += aCfgLocaleStr;
        Sequence< Any > aValues( rCfg.GetProperties( aNames ) );
#if OSL_DEBUG_LEVEL > 1
        const Any *pValue;
        pValue = aValues.getConstArray();
#endif
        if (aValues.getLength())
        {
            DBG_ASSERT( aValues.getLength() == 1, "unexpected length of sequence" );
            Sequence< OUString > aSvcImplNames;
            if (aValues.getConstArray()[0] >>= aSvcImplNames)
                aRes = aSvcImplNames;
            else
            {
                DBG_ERROR( "type mismatch" );
            }
        }
    }

    return aRes;
}


Sequence< OUString > lcl_GetNewEntries(
        const Sequence< OUString > &rLastFoundSvcs,
        const Sequence< OUString > &rAvailSvcs )
{
    sal_Int32 nLen = rAvailSvcs.getLength();
    Sequence< OUString > aRes( nLen );
    OUString *pRes = aRes.getArray();
    sal_Int32 nCnt = 0;

    const OUString *pEntry = rAvailSvcs.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (pEntry[i].getLength() && !lcl_FindEntry( pEntry[i], rLastFoundSvcs ))
            pRes[ nCnt++ ] = pEntry[i];
    }

    aRes.realloc( nCnt );
    return aRes;
}


Sequence< OUString > lcl_MergeSeq(
        const Sequence< OUString > &rCfgSvcs,
        const Sequence< OUString > &rNewSvcs )
{
    Sequence< OUString > aRes( rCfgSvcs.getLength() + rNewSvcs.getLength() );
    OUString *pRes = aRes.getArray();
    sal_Int32 nCnt = 0;

    for (sal_Int32 k = 0;  k < 2;  ++k)
    {
        // add previously configuerd service first and append
        // new found services at the end
        const Sequence< OUString > &rSeq = k == 0 ? rCfgSvcs : rNewSvcs;

        sal_Int32 nLen = rSeq.getLength();
        const OUString *pEntry = rSeq.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if (pEntry[i].getLength() && !lcl_FindEntry( pEntry[i], aRes ))
                pRes[ nCnt++ ] = pEntry[i];
        }
    }

    aRes.realloc( nCnt );
    return aRes;
}

///////////////////////////////////////////////////////////////////////////

// static member initialization
sal_Int16 SvxLinguConfigUpdate::nNeedUpdating = -1;
sal_Int32 SvxLinguConfigUpdate::nCurrentDataFilesChangedCheckValue = -1;

void SvxLinguConfigUpdate::UpdateAll( sal_Bool bForceCheck )
{
    RTL_LOGFILE_CONTEXT( aLog, "svx: SvxLinguConfigUpdate::UpdateAll" );

    if (IsNeedUpdateAll( bForceCheck ))
    {
        typedef OUString OUstring_t;
        typedef Sequence< OUString > Sequence_OUString_t;
        typedef std::vector< OUstring_t > OUString_vector_t;
        typedef std::set< OUstring_t > OUString_set_t;
        std::vector< OUString_vector_t > aVector;
        typedef std::map< OUstring_t, Sequence_OUString_t > list_entry_map_t;

        RTL_LOGFILE_CONTEXT( aLog, "svx: SvxLinguConfigUpdate::UpdateAll - updating..." );

        DBG_ASSERT( nNeedUpdating == 1, "SvxLinguConfigUpdate::UpdateAll already updated!" );

        uno::Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        DBG_ASSERT( xLngSvcMgr.is(), "service manager missing");
        if (!xLngSvcMgr.is())
            return;

        SvtLinguConfig aCfg;

        const int nNumServices = 4;
        const sal_Char * apServices[nNumServices]       =  { SN_SPELLCHECKER, SN_GRAMMARCHECKER, SN_HYPHENATOR, SN_THESAURUS };
        const sal_Char * apCurLists[nNumServices]       =  { "ServiceManager/SpellCheckerList",       "ServiceManager/GrammarCheckerList",       "ServiceManager/HyphenatorList",       "ServiceManager/ThesaurusList" };
        const sal_Char * apLastFoundLists[nNumServices] =  { "ServiceManager/LastFoundSpellCheckers", "ServiceManager/LastFoundGrammarCheckers", "ServiceManager/LastFoundHyphenators", "ServiceManager/LastFoundThesauri" };

        // usage of indices as above: 0 = spell checker, 1 = grammar checker, 2 = hyphenator, 3 = thesaurus
        std::vector< list_entry_map_t > aLastFoundSvcs(nNumServices);
        std::vector< list_entry_map_t > aCurSvcs(nNumServices);

        for (int k = 0;  k < nNumServices;  ++k)
        {
            OUString aService( A2OU( apServices[k] ) );
            OUString aActiveList( A2OU( apCurLists[k] ) );
            OUString aLastFoundList( A2OU( apLastFoundLists[k] ) );
            sal_Int32 i;

            //
            // remove configured but not available language/services entries
            //
            Sequence< OUString > aNodeNames( aCfg.GetNodeNames( aActiveList ) );   // list of configured locales
            sal_Int32 nNodeNames = aNodeNames.getLength();
            const OUString *pNodeName = aNodeNames.getConstArray();
            for (i = 0;  i < nNodeNames;  ++i)
            {
                Locale aLocale( SvxCreateLocale( MsLangId::convertIsoStringToLanguage(pNodeName[i]) ) );
                Sequence< OUString > aCfgSvcs(
                        xLngSvcMgr->getConfiguredServices( aService, aLocale ));
                Sequence< OUString > aAvailSvcs(
                        xLngSvcMgr->getAvailableServices( aService, aLocale ));
#if OSL_DEBUG_LEVEL > 1
                const OUString * pCfgSvcs   = aCfgSvcs.getConstArray();;
                const OUString * pAvailSvcs = aAvailSvcs.getConstArray();;
                (void) pCfgSvcs;
                (void) pAvailSvcs;
#endif
                aCfgSvcs = lcl_RemoveMissingEntries( aCfgSvcs, aAvailSvcs );

                aCurSvcs[k][ pNodeName[i] ] = aCfgSvcs;
            }

            //
            // add new available language/servcice entries
            //
            uno::Reference< XAvailableLocales > xAvail( xLngSvcMgr, UNO_QUERY );
            Sequence< Locale > aAvailLocales( xAvail->getAvailableLocales(aService) );
            sal_Int32 nAvailLocales = aAvailLocales.getLength();
            const Locale *pAvailLocale = aAvailLocales.getConstArray();
            for (i = 0;  i < nAvailLocales;  ++i)
            {
                Sequence< OUString > aAvailSvcs(
                        xLngSvcMgr->getAvailableServices( aService, pAvailLocale[i] ));
                Sequence< OUString > aLastSvcs(
                        lcl_GetLastFoundSvcs( aCfg, aLastFoundList , pAvailLocale[i] ));
                Sequence< OUString > aNewSvcs =
                        lcl_GetNewEntries( aLastSvcs, aAvailSvcs );
#if OSL_DEBUG_LEVEL > 1
                const OUString * pAvailSvcs = aAvailSvcs.getConstArray();
                const OUString * pLastSvcs  = aLastSvcs.getConstArray();
                const OUString * pNewSvcs   = aNewSvcs.getConstArray();
                (void) pAvailSvcs;
                (void) pLastSvcs;
                (void) pNewSvcs;
#endif

                OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                            SvxLocaleToLanguage( pAvailLocale[i] ) ) );
                Sequence< OUString > aCfgSvcs( aCurSvcs[k][ aCfgLocaleStr ] );

                // merge services list (previously configured to be listed first).
                aCfgSvcs = lcl_MergeSeq( aCfgSvcs, aNewSvcs );

/*
                // there is at most one Hyphenator per language allowed
                // to be configured, thus we only use the first one found.
                if (k == 2 && aCfgSvcs.getLength() > 1)
                    aCfgSvcs.realloc(1);
*/
                aCurSvcs[k][ aCfgLocaleStr ] = aCfgSvcs;
            }

            //
            // set last found services to currently available ones
            //
            for (i = 0;  i < nAvailLocales;  ++i)
            {
                Sequence< OUString > aSvcImplNames(
                        xLngSvcMgr->getAvailableServices( aService, pAvailLocale[i] ) );

#if OSL_DEBUG_LEVEL > 1
                sal_Int32 nSvcs = aSvcImplNames.getLength();
                const OUString *pSvcImplName = aSvcImplNames.getConstArray();
                for (sal_Int32 j = 0;  j < nSvcs;  ++j)
                {
                    OUString aImplName( pSvcImplName[j] );
                }
#endif

                OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                            SvxLocaleToLanguage( pAvailLocale[i] ) ) );
                aLastFoundSvcs[k][ aCfgLocaleStr ] = aSvcImplNames;
            }
        }

        //
        // write new data back to configuration
        //
        for (int k = 0;  k < nNumServices;  ++k)
        {
            for (int i = 0;  i < 2;  ++i)
            {
                const sal_Char *pSubNodeName = (i == 0) ? apCurLists[k] : apLastFoundLists[k];
                OUString aSubNodeName( A2OU(pSubNodeName) );

                list_entry_map_t &rCurMap = (i == 0) ? aCurSvcs[k] : aLastFoundSvcs[k];
                list_entry_map_t::const_iterator aIt( rCurMap.begin() );
                sal_Int32 nVals = static_cast< sal_Int32 >( rCurMap.size() );
                Sequence< PropertyValue > aNewValues( nVals );
                PropertyValue *pNewValue = aNewValues.getArray();
                while (aIt != rCurMap.end())
                {
                    OUString aCfgEntryName( aSubNodeName );
                    aCfgEntryName += OUString::valueOf( (sal_Unicode) '/' );
                    aCfgEntryName += (*aIt).first;

#if OSL_DEBUG_LEVEL > 1
                    Sequence< OUString > aSvcImplNames( (*aIt).second );
                    sal_Int32 nSvcs = aSvcImplNames.getLength();
                    const OUString *pSvcImplName = aSvcImplNames.getConstArray();
                    for (sal_Int32 j = 0;  j < nSvcs;  ++j)
                    {
                        OUString aImplName( pSvcImplName[j] );
                    }
#endif
                    pNewValue->Name  = aCfgEntryName;
                    pNewValue->Value <<= (*aIt).second;
                    ++pNewValue;
                    ++aIt;
                }
                DBG_ASSERT( pNewValue - aNewValues.getArray() == nVals,
                        "possible mismatch of sequence size and property number" );

                {
                    RTL_LOGFILE_CONTEXT( aLog, "svx: SvxLinguConfigUpdate::UpdateAll - ReplaceSetProperties" );
                    // add new or replace existing entries.
                    sal_Bool bRes = aCfg.ReplaceSetProperties( aSubNodeName, aNewValues );
                    if (!bRes)
                    {
#if OSL_DEBUG_LEVEL > 1
                        DBG_ERROR( "failed to set new configuration values" );
#endif
                    }
                }
            }
        }
        DBG_ASSERT( nCurrentDataFilesChangedCheckValue != -1, "SvxLinguConfigUpdate::UpdateAll DataFilesChangedCheckValue not yet calculated!" );
        Any aAny;

        // for the time being (developer builds until OOo 3.0)
        // we should always check for everything available
        // otherwise we may miss a new installed extension dicitonary
        // just because e.g. the spellchecker is not asked what
        // languages it does support currently...
        // Since the check is on-demand occuring and executed once it should
        // not be too troublesome.
        // In OOo 3.0 we will not need the respective code anymore at all.
//      aAny <<= nCurrentDataFilesChangedCheckValue;
        aAny <<= (sal_Int32) -1;    // keep the value set to 'need to check'

        aCfg.SetProperty( A2OU( "DataFilesChangedCheckValue" ), aAny );

        //! Note 1: the new values are commited when the 'aCfg' object
        //!     gets destroyed.
        //! Note 2: the new settings in the configuration get applied
        //!     because the 'LngSvcMgr' (in linguistic/source/lngsvcmgr.hxx)
        //!     listens to the configuration for changes of the relevant
        //!     properties and then applies the new settings.

        // nothing needs to be done anymore
        nNeedUpdating = 0;
    }
}


sal_Int32 SvxLinguConfigUpdate::CalcDataFilesChangedCheckValue()
{
    RTL_LOGFILE_CONTEXT( aLog, "svx: SvxLinguConfigUpdate::CalcDataFilesChangedCheckValue" );

    sal_Int32 nHashVal = 0;
    // nothing to be checked anymore since those old directory paths are gone by now
    return nHashVal;
}


sal_Bool SvxLinguConfigUpdate::IsNeedUpdateAll( sal_Bool bForceCheck )
{
    RTL_LOGFILE_CONTEXT( aLog, "svx: SvxLinguConfigUpdate::IsNeedUpdateAll" );
    if (nNeedUpdating == -1 || bForceCheck )    // need to check if updating is necessary
    {
        // calculate hash value for current data files
        nCurrentDataFilesChangedCheckValue = CalcDataFilesChangedCheckValue();

        // compare hash value and check value to see if anything has changed
        // and thus the configuration needs to be updated
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig aCfg;
        aCfg.GetOptions( aLinguOpt );
        nNeedUpdating = (nCurrentDataFilesChangedCheckValue == aLinguOpt.nDataFilesChangedCheckValue) ? 0 : 1;
    }
    DBG_ASSERT( nNeedUpdating != -1,
            "need for linguistic configuration update should have been already checked." );

    return nNeedUpdating == 1;
}

///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL
//! when only the XSupportedLocales interface is used.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when "real" work needs to be done only.
class ThesDummy_Impl :
    public cppu::WeakImplHelper1< XThesaurus >
{
    uno::Reference< XThesaurus >     xThes;      // the real one...
    Sequence< Locale >         *pLocaleSeq;

    void GetCfgLocales();

    void GetThes_Impl();

public:
    ThesDummy_Impl() : pLocaleSeq(0)  {}
    ~ThesDummy_Impl();

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& rLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XThesaurus
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
        queryMeanings( const ::rtl::OUString& rTerm,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
};


ThesDummy_Impl::~ThesDummy_Impl()
{
    delete pLocaleSeq;
}


void ThesDummy_Impl::GetCfgLocales()
{
    if (!pLocaleSeq)
    {
        SvtLinguConfig aCfg;
        String  aNode( A2OU( "ServiceManager/ThesaurusList" ) );
        Sequence < OUString > aNodeNames( aCfg.GetNodeNames( aNode ) );
        const OUString *pNodeNames = aNodeNames.getConstArray();
        sal_Int32 nLen = aNodeNames.getLength();
        pLocaleSeq = new Sequence< Locale >( nLen );
        Locale *pLocale = pLocaleSeq->getArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            pLocale[i] = SvxCreateLocale(
                            MsLangId::convertIsoStringToLanguage( pNodeNames[i] ) );
        }
    }
}


void ThesDummy_Impl::GetThes_Impl()
{
    // update configuration before accessing the service
    if (SvxLinguConfigUpdate::IsNeedUpdateAll())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xThes.is())
    {
        uno::Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        if (xLngSvcMgr.is())
            xThes = xLngSvcMgr->getThesaurus();

        if (xThes.is())
        {
            // no longer needed...
            delete pLocaleSeq;    pLocaleSeq = 0;
        }
    }
}


uno::Sequence< lang::Locale > SAL_CALL
        ThesDummy_Impl::getLocales()
            throw(uno::RuntimeException)
{
    if (!SvxLinguConfigUpdate::IsNeedUpdateAll())   // configuration already update and thus lingu DLL's already loaded ?
        GetThes_Impl();
    if (xThes.is())
        return xThes->getLocales();
    else if (!pLocaleSeq)       // if not already loaded save startup time by avoiding loading them now
        GetCfgLocales();
    return *pLocaleSeq;
}


sal_Bool SAL_CALL
        ThesDummy_Impl::hasLocale( const lang::Locale& rLocale )
            throw(uno::RuntimeException)
{
    if (!SvxLinguConfigUpdate::IsNeedUpdateAll())   // configuration already update and thus lingu DLL's already loaded ?
        GetThes_Impl();
    if (xThes.is())
        return xThes->hasLocale( rLocale );
    else if (!pLocaleSeq)       // if not already loaded save startup time by avoiding loading them now
        GetCfgLocales();
        GetCfgLocales();
    sal_Bool bFound = sal_False;
    sal_Int32 nLen = pLocaleSeq->getLength();
    const Locale *pLocale = pLocaleSeq->getConstArray();
    const Locale *pEnd = pLocale + nLen;
    for ( ;  pLocale < pEnd  &&  !bFound;  ++pLocale)
    {
        bFound = pLocale->Language == rLocale.Language  &&
                 pLocale->Country  == rLocale.Country   &&
                 pLocale->Variant  == rLocale.Variant;
    }
    return bFound;
}


uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL
        ThesDummy_Impl::queryMeanings(
                const rtl::OUString& rTerm,
                const lang::Locale& rLocale,
                const beans::PropertyValues& rProperties )
            throw(lang::IllegalArgumentException,
                  uno::RuntimeException)
{
    GetThes_Impl();
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aRes;
    DBG_ASSERT( xThes.is(), "Thesaurus missing" );
    if (xThes.is())
        aRes = xThes->queryMeanings( rTerm, rLocale, rProperties );
    return aRes;
}


///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
class SpellDummy_Impl :
    public cppu::WeakImplHelper1< XSpellChecker1 >
{
    uno::Reference< XSpellChecker1 >     xSpell;      // the real one...

    void    GetSpell_Impl();

public:

    // XSupportedLanguages (for XSpellChecker1)
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL
        getLanguages()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLanguage( sal_Int16 nLanguage )
            throw(::com::sun::star::uno::RuntimeException);

    // XSpellChecker1 (same as XSpellChecker but sal_Int16 for language)
    virtual sal_Bool SAL_CALL
        isValid( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL
        spell( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
};


void SpellDummy_Impl::GetSpell_Impl()
{
    // update configuration before accessing the service
    if (SvxLinguConfigUpdate::IsNeedUpdateAll())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xSpell.is())
    {
        uno::Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        if (xLngSvcMgr.is())
            xSpell = uno::Reference< XSpellChecker1 >( xLngSvcMgr->getSpellChecker(), UNO_QUERY );
    }
}


uno::Sequence< sal_Int16 > SAL_CALL
    SpellDummy_Impl::getLanguages()
        throw(uno::RuntimeException)
{
    GetSpell_Impl();
    if (xSpell.is())
        return xSpell->getLanguages();
    else
        return uno::Sequence< sal_Int16 >();
}


sal_Bool SAL_CALL
    SpellDummy_Impl::hasLanguage( sal_Int16 nLanguage )
        throw(uno::RuntimeException)
{
    GetSpell_Impl();
    sal_Bool bRes = sal_False;
    if (xSpell.is())
        bRes = xSpell->hasLanguage( nLanguage );
    return bRes;
}


sal_Bool SAL_CALL
    SpellDummy_Impl::isValid( const rtl::OUString& rWord, sal_Int16 nLanguage,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException)
{
    GetSpell_Impl();
    sal_Bool bRes = sal_True;
    if (xSpell.is())
        bRes = xSpell->isValid( rWord, nLanguage, rProperties );
    return bRes;
}


uno::Reference< linguistic2::XSpellAlternatives > SAL_CALL
    SpellDummy_Impl::spell( const rtl::OUString& rWord, sal_Int16 nLanguage,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException)
{
    GetSpell_Impl();
    uno::Reference< linguistic2::XSpellAlternatives > xRes;
    if (xSpell.is())
        xRes = xSpell->spell( rWord, nLanguage, rProperties );
    return xRes;
}


///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
class HyphDummy_Impl :
    public cppu::WeakImplHelper1< XHyphenator >
{
    uno::Reference< XHyphenator >     xHyph;      // the real one...

    void    GetHyph_Impl();

public:

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& rLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XHyphenator
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const ::rtl::OUString& rWord,
                const ::com::sun::star::lang::Locale& rLocale,
                sal_Int16 nMaxLeading,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const ::rtl::OUString& rWord,
                const ::com::sun::star::lang::Locale& rLocale,
                sal_Int16 nIndex,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const ::rtl::OUString& rWord,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
};


void HyphDummy_Impl::GetHyph_Impl()
{
    // update configuration before accessing the service
    if (SvxLinguConfigUpdate::IsNeedUpdateAll())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xHyph.is())
    {
        uno::Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        if (xLngSvcMgr.is())
            xHyph = xLngSvcMgr->getHyphenator();
    }
}


uno::Sequence< lang::Locale > SAL_CALL
    HyphDummy_Impl::getLocales()
        throw(uno::RuntimeException)
{
    GetHyph_Impl();
    if (xHyph.is())
        return xHyph->getLocales();
    else
        return uno::Sequence< lang::Locale >();
}


sal_Bool SAL_CALL
    HyphDummy_Impl::hasLocale( const lang::Locale& rLocale )
        throw(uno::RuntimeException)
{
    GetHyph_Impl();
    sal_Bool bRes = sal_False;
    if (xHyph.is())
        bRes = xHyph->hasLocale( rLocale );
    return bRes;
}


uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
    HyphDummy_Impl::hyphenate(
            const rtl::OUString& rWord,
            const lang::Locale& rLocale,
            sal_Int16 nMaxLeading,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XHyphenatedWord > xRes;
    if (xHyph.is())
        xRes = xHyph->hyphenate( rWord, rLocale, nMaxLeading, rProperties );
    return xRes;
}


uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
    HyphDummy_Impl::queryAlternativeSpelling(
            const rtl::OUString& rWord,
            const lang::Locale& rLocale,
            sal_Int16 nIndex,
            const PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XHyphenatedWord > xRes;
    if (xHyph.is())
        xRes = xHyph->queryAlternativeSpelling( rWord, rLocale, nIndex, rProperties );
    return xRes;
}


uno::Reference< linguistic2::XPossibleHyphens > SAL_CALL
    HyphDummy_Impl::createPossibleHyphens(
            const rtl::OUString& rWord,
            const lang::Locale& rLocale,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException,
              uno::RuntimeException)
{
    GetHyph_Impl();
    uno::Reference< linguistic2::XPossibleHyphens > xRes;
    if (xHyph.is())
        xRes = xHyph->createPossibleHyphens( rWord, rLocale, rProperties );
    return xRes;
}


///////////////////////////////////////////////////////////////////////////


typedef cppu::WeakImplHelper1 < XEventListener > LinguMgrAppExitLstnrBaseClass;

class LinguMgrAppExitLstnr : public LinguMgrAppExitLstnrBaseClass
{
    uno::Reference< XComponent >        xDesktop;

public:
    LinguMgrAppExitLstnr();
    virtual ~LinguMgrAppExitLstnr();

    virtual void    AtExit() = 0;


    // lang::XEventListener
    virtual void    SAL_CALL disposing(const EventObject& rSource)
            throw( RuntimeException );
};

LinguMgrAppExitLstnr::LinguMgrAppExitLstnr()
{
    // add object to frame::Desktop EventListeners in order to properly call
    // the AtExit function at appliction exit.

    uno::Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if ( xMgr.is() )
    {
        xDesktop = uno::Reference< XComponent > ( xMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY ) ;
        if (xDesktop.is())
            xDesktop->addEventListener( this );
    }
}

LinguMgrAppExitLstnr::~LinguMgrAppExitLstnr()
{
    if (xDesktop.is())
    {
        xDesktop->removeEventListener( this );
        xDesktop = NULL;    //! release reference to desktop
    }
    DBG_ASSERT(!xDesktop.is(), "reference to desktop should be realeased");
}

void LinguMgrAppExitLstnr::disposing(const EventObject& rSource)
        throw( RuntimeException )
{
    if (xDesktop.is()  &&  rSource.Source == xDesktop)
    {
        xDesktop->removeEventListener( this );
        xDesktop = NULL;    //! release reference to desktop

        AtExit();
    }
}

///////////////////////////////////////////////////////////////////////////

class LinguMgrExitLstnr : public LinguMgrAppExitLstnr
{
public:
    virtual void    AtExit();
};

void LinguMgrExitLstnr::AtExit()
{
    // release references
    LinguMgr::xLngSvcMgr    = 0;
    LinguMgr::xSpell        = 0;
    LinguMgr::xHyph         = 0;
    LinguMgr::xThes         = 0;
    LinguMgr::xDicList      = 0;
    LinguMgr::xProp         = 0;
    LinguMgr::xIgnoreAll    = 0;
    LinguMgr::xChangeAll    = 0;

    LinguMgr::bExiting      = sal_True;

    //TL:TODO: MBA fragen wie ich ohne Absturz hier meinen Speicher
    //  wieder freibekomme...
    //delete LinguMgr::pExitLstnr;
    LinguMgr::pExitLstnr    = 0;
}

///////////////////////////////////////////////////////////////////////////


// static member initialization
LinguMgrExitLstnr *             LinguMgr::pExitLstnr    = 0;
sal_Bool                        LinguMgr::bExiting      = sal_False;
uno::Reference< XLinguServiceManager >  LinguMgr::xLngSvcMgr    = 0;
uno::Reference< XSpellChecker1 >    LinguMgr::xSpell        = 0;
uno::Reference< XHyphenator >       LinguMgr::xHyph         = 0;
uno::Reference< XThesaurus >        LinguMgr::xThes         = 0;
uno::Reference< XDictionaryList >   LinguMgr::xDicList      = 0;
uno::Reference< XPropertySet >      LinguMgr::xProp         = 0;
uno::Reference< XDictionary >       LinguMgr::xIgnoreAll    = 0;
uno::Reference< XDictionary >       LinguMgr::xChangeAll    = 0;


uno::Reference< XLinguServiceManager > LinguMgr::GetLngSvcMgr()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    return xLngSvcMgr;
}


uno::Reference< XSpellChecker1 > LinguMgr::GetSpellChecker()
{
    return xSpell.is() ? xSpell : GetSpell();
}

uno::Reference< XHyphenator > LinguMgr::GetHyphenator()
{
    return xHyph.is() ? xHyph : GetHyph();
}

uno::Reference< XThesaurus > LinguMgr::GetThesaurus()
{
    return xThes.is() ? xThes : GetThes();
}

uno::Reference< XDictionaryList > LinguMgr::GetDictionaryList()
{
    return xDicList.is() ? xDicList : GetDicList();
}

uno::Reference< XPropertySet > LinguMgr::GetLinguPropertySet()
{
    return xProp.is() ? xProp : GetProp();
}

uno::Reference< XDictionary > LinguMgr::GetStandardDic()
{
    //! don't hold reference to this
    //! (it may be removed from dictionary list and needs to be
    //! created empty if accessed again)
    return GetStandard();
}

uno::Reference< XDictionary > LinguMgr::GetIgnoreAllList()
{
    return xIgnoreAll.is() ? xIgnoreAll : GetIgnoreAll();
}

uno::Reference< XDictionary > LinguMgr::GetChangeAllList()
{
    return xChangeAll.is() ? xChangeAll : GetChangeAll();
}

uno::Reference< XSpellChecker1 > LinguMgr::GetSpell()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    xSpell = new SpellDummy_Impl;

/*    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xSpell = uno::Reference< XSpellChecker1 > (
                        xLngSvcMgr->getSpellChecker(), UNO_QUERY );
    }
*/
    return xSpell;
}

uno::Reference< XHyphenator > LinguMgr::GetHyph()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    xHyph = new HyphDummy_Impl;

/*
    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xHyph = xLngSvcMgr->getHyphenator();
    }
*/
    return xHyph;
}

uno::Reference< XThesaurus > LinguMgr::GetThes()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    //! use dummy implementation in order to avoid loading of lingu DLL
    //! when only the XSupportedLocales interface is used.
    //! The dummy accesses the real implementation (and thus loading the DLL)
    //! when "real" work needs to be done only.
    xThes = new ThesDummy_Impl;
/*
    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xThes = xLngSvcMgr->getThesaurus();
    }
*/
    return xThes;
}


void LinguMgr::UpdateAll()
{
}


uno::Reference< XDictionaryList > LinguMgr::GetDicList()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xDicList = uno::Reference< XDictionaryList > ( xMgr->createInstance(
                    A2OU("com.sun.star.linguistic2.DictionaryList") ), UNO_QUERY );
    }
    return xDicList;
}

uno::Reference< XPropertySet > LinguMgr::GetProp()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xProp = uno::Reference< XPropertySet > ( xMgr->createInstance(
                    A2OU("com.sun.star.linguistic2.LinguProperties") ), UNO_QUERY );
    }
    return xProp;
}

uno::Reference< XDictionary > LinguMgr::GetIgnoreAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (xTmpDicList.is())
    {
        xIgnoreAll = uno::Reference< XDictionary > ( xTmpDicList->getDictionaryByName(
                    A2OU("IgnoreAllList") ), UNO_QUERY );
    }
    return xIgnoreAll;
}

uno::Reference< XDictionary > LinguMgr::GetChangeAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    uno::Reference< XDictionaryList > _xDicList( GetDictionaryList() , UNO_QUERY );
    if (_xDicList.is())
    {
        xChangeAll = uno::Reference< XDictionary > (
                        _xDicList->createDictionary(
                            A2OU("ChangeAllList"),
                            SvxCreateLocale( LANGUAGE_NONE ),
                            DictionaryType_NEGATIVE, String() ), UNO_QUERY );
    }
    return xChangeAll;
}

uno::Reference< XDictionary > LinguMgr::GetStandard()
{
    // Tries to return a dictionary which may hold positive entries is
    // persistent and not read-only.

    if (bExiting)
        return 0;

    uno::Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (!xTmpDicList.is())
        return NULL;

    const OUString aDicName( RTL_CONSTASCII_USTRINGPARAM( "standard.dic" ) );
    uno::Reference< XDictionary >   xDic( xTmpDicList->getDictionaryByName( aDicName ),
                                      UNO_QUERY );
    if (!xDic.is())
    {
        // try to create standard dictionary
        uno::Reference< XDictionary >    xTmp;
        try
        {
            xTmp =  xTmpDicList->createDictionary( aDicName,
                        SvxCreateLocale( LANGUAGE_NONE ),
                        DictionaryType_POSITIVE,
                        linguistic::GetWritableDictionaryURL( aDicName ) );
        }
        catch(com::sun::star::uno::Exception &)
        {
        }

        // add new dictionary to list
        if (xTmp.is())
        {
            xTmpDicList->addDictionary( xTmp );
            xTmp->setActive( sal_True );
        }
        xDic = uno::Reference< XDictionary > ( xTmp, UNO_QUERY );
    }
#if OSL_DEBUG_LEVEL > 1
    uno::Reference< XStorable >      xStor( xDic, UNO_QUERY );
    DBG_ASSERT( xDic.is() && xDic->getDictionaryType() == DictionaryType_POSITIVE,
            "wrong dictionary type");
    DBG_ASSERT( xDic.is() && SvxLocaleToLanguage( xDic->getLocale() ) == LANGUAGE_NONE,
            "wrong dictionary language");
    DBG_ASSERT( !xStor.is() || (xStor->hasLocation() && !xStor->isReadonly()),
            "dictionary not editable" );
#endif

    return xDic;
}

///////////////////////////////////////////////////////////////////////////

uno::Reference< XSpellChecker1 >  SvxGetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

uno::Reference< XHyphenator >  SvxGetHyphenator()
{
    return LinguMgr::GetHyphenator();
}

uno::Reference< XThesaurus >  SvxGetThesaurus()
{
    return LinguMgr::GetThesaurus();
}

uno::Reference< XDictionaryList >  SvxGetDictionaryList()
{
    return LinguMgr::GetDictionaryList();
}

uno::Reference< XPropertySet >  SvxGetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one
uno::Reference< XDictionary >  SvxGetOrCreatePosDic(
        uno::Reference< XDictionaryList >  /* xDicList */ )
{
    return LinguMgr::GetStandardDic();
}

uno::Reference< XDictionary >  SvxGetIgnoreAllList()
{
    return LinguMgr::GetIgnoreAllList();
}

uno::Reference< XDictionary >  SvxGetChangeAllList()
{
    return LinguMgr::GetChangeAllList();
}

///////////////////////////////////////////////////////////////////////////


#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord )
{
    SvxAlternativeSpelling aRes;
    if (rHyphWord.is() && rHyphWord->isAlternativeSpelling())
    {
        OUString aWord( rHyphWord->getWord() ),
                 aAltWord( rHyphWord->getHyphenatedWord() );
        sal_Int16   nHyphenationPos     = rHyphWord->getHyphenationPos(),
                nHyphenPos          = rHyphWord->getHyphenPos();
        sal_Int16   nLen    = (sal_Int16)aWord.getLength();
        sal_Int16   nAltLen = (sal_Int16)aAltWord.getLength();
        const sal_Unicode *pWord    = aWord.getStr(),
                          *pAltWord = aAltWord.getStr();

        // count number of chars from the left to the
        // hyphenation pos / hyphen pos that are equal
        sal_Int16 nL = 0;
        while (nL <= nHyphenationPos && nL <= nHyphenPos
               && pWord[ nL ] == pAltWord[ nL ])
            ++nL;
        // count number of chars from the right to the
        // hyphenation pos / hyphen pos that are equal
        sal_Int16 nR = 0;
        sal_Int32 nIdx    = nLen - 1;
        sal_Int32 nAltIdx = nAltLen - 1;
        while (nIdx > nHyphenationPos && nAltIdx > nHyphenPos
               && pWord[ nIdx-- ] == pAltWord[ nAltIdx-- ])
            ++nR;

        aRes.aReplacement       = OUString( aAltWord.copy( nL, nAltLen - nL - nR ) );
        aRes.nChangedPos        = (sal_Int16) nL;
        aRes.nChangedLength     = nLen - nL - nR;
        aRes.bIsAltSpelling     = sal_True;
        aRes.xHyphWord          = rHyphWord;
    }
    return aRes;
}


///////////////////////////////////////////////////////////////////////////

SvxDicListChgClamp::SvxDicListChgClamp( uno::Reference< XDictionaryList >  &rxDicList ) :
    xDicList    ( rxDicList )
{
    if (xDicList.is())
    {
        xDicList->beginCollectEvents();
    }
}

SvxDicListChgClamp::~SvxDicListChgClamp()
{
    if (xDicList.is())
    {
        xDicList->endCollectEvents();
    }
}

///////////////////////////////////////////////////////////////////////////

short SvxDicError( Window *pParent, sal_Int16 nError )
{
    short nRes = 0;
    if (DIC_ERR_NONE != nError)
    {
        int nRid;
        switch (nError)
        {
            case DIC_ERR_FULL     : nRid = RID_SVXSTR_DIC_ERR_FULL;  break;
            case DIC_ERR_READONLY : nRid = RID_SVXSTR_DIC_ERR_READONLY;  break;
            default:
                nRid = RID_SVXSTR_DIC_ERR_UNKNOWN;
                DBG_ASSERT(0, "unexpected case");
        }
        nRes = InfoBox( pParent, EE_RESSTR( nRid ) ).Execute();
    }
    return nRes;
}

LanguageType SvxLocaleToLanguage( const Locale& rLocale )
{
    //  empty Locale -> LANGUAGE_NONE
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_NONE;

    return MsLangId::convertLocaleToLanguage( rLocale );
}

Locale& SvxLanguageToLocale( Locale& rLocale, LanguageType eLang )
{
    if ( eLang != LANGUAGE_NONE /* &&  eLang != LANGUAGE_SYSTEM */)
        MsLangId::convertLanguageToLocale( eLang, rLocale );
    else
        rLocale = Locale();

    return rLocale;
}

Locale SvxCreateLocale( LanguageType eLang )
{
    Locale aLocale;
    if ( eLang != LANGUAGE_NONE /* &&  eLang != LANGUAGE_SYSTEM */)
        MsLangId::convertLanguageToLocale( eLang, aLocale );

    return aLocale;
}


