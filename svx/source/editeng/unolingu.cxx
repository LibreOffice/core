/*************************************************************************
 *
 *  $RCSfile: unolingu.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:02:04 $
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

#pragma hdrstop

#include <unolingu.hxx>

#include <cppuhelper/implbase1.hxx> // helper for implementations

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XAVAILABLELOCALES_HPP_
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#endif
#include <comphelper/processfactory.hxx>

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _LINGUISTIC_MISC_HXX_
#include <linguistic/misc.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#include "dialogs.hrc"

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;

#define A2OU(x) OUString::createFromAscii(x)

///////////////////////////////////////////////////////////////////////////


static Reference< XLinguServiceManager > GetLngSvcMgr_Impl()
{
    Reference< XLinguServiceManager > xRes;
    Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if (xMgr.is())
    {
        xRes = Reference< XLinguServiceManager > ( xMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.linguistic2.LinguServiceManager" ) ) ), UNO_QUERY ) ;
    }
    return xRes;
}

///////////////////////////////////////////////////////////////////////////

BOOL lcl_FindEntry( const OUString &rEntry, const Sequence< OUString > &rCfgSvcs )
{
    INT32 nRes = -1;
    INT32 nEntries = rCfgSvcs.getLength();
    const OUString *pEntry = rCfgSvcs.getConstArray();
    for (INT32 i = 0;  i < nEntries && nRes == -1;  ++i)
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
    INT32 nCnt = 0;

    INT32 nEntries = rCfgSvcs.getLength();
    const OUString *pEntry = rCfgSvcs.getConstArray();
    for (INT32 i = 0;  i < nEntries;  ++i)
    {
        if (pEntry[i].getLength() && lcl_FindEntry( pEntry[i], rAvailSvcs ))
            pRes[ nCnt++ ] = pEntry[i];
    }

    aRes.realloc( nCnt );
    return aRes;
}


Sequence< OUString > lcl_GetLastFoundSvcs(
        SvtLinguConfigItem &rCfg,
        const OUString &rLastFoundList ,
        const Locale &rAvailLocale )
{
    Sequence< OUString > aRes;

    OUString aCfgLocaleStr( ConvertLanguageToIsoString(
                                SvxLocaleToLanguage( rAvailLocale ) ) );

    Sequence< OUString > aNodeNames( rCfg.GetNodeNames(rLastFoundList) );
    BOOL bFound = lcl_FindEntry( aCfgLocaleStr, aNodeNames);

    if (bFound)
    {
        Sequence< OUString > aNames(1);
        OUString &rNodeName = aNames.getArray()[0];
        rNodeName = rLastFoundList;
        rNodeName += OUString::valueOf( (sal_Unicode)'/' );
        rNodeName += aCfgLocaleStr;
        Sequence< Any > aValues( rCfg.GetProperties( aNames ) );
#ifdef DEBUG
        const Any *pValue = aValues.getConstArray();
#endif
        if (aValues.getLength())
        {
            DBG_ASSERT( aValues.getLength() == 1, "unexpected length of sequence" );
            Sequence< OUString > aSvcImplNames;
            if (aValues.getConstArray()[0] >>= aSvcImplNames)
                aRes = aSvcImplNames;
            else
                DBG_ERROR( "type mismatch" );
        }
    }

    return aRes;
}


Sequence< OUString > lcl_GetNewEntries(
        const Sequence< OUString > &rLastFoundSvcs,
        const Sequence< OUString > &rAvailSvcs )
{
    INT32 nLen = rAvailSvcs.getLength();
    Sequence< OUString > aRes( nLen );
    OUString *pRes = aRes.getArray();
    INT32 nCnt = 0;

    const OUString *pEntry = rAvailSvcs.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
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
    INT32 nCnt = 0;

    for (INT32 k = 0;  k < 2;  ++k)
    {
        const Sequence< OUString > &rSeq = k == 0 ? rCfgSvcs : rNewSvcs;
        INT32 nLen = rSeq.getLength();
        const OUString *pEntry = rSeq.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
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
BOOL SvxLinguConfigUpdate::bUpdated = FALSE;

void SvxLinguConfigUpdate::UpdateAll()
{
    if (!IsUpdated())
    {
        Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        DBG_ASSERT( xLngSvcMgr.is(), "service manager missing")
        if (!xLngSvcMgr.is())
            return;

        SvtLinguConfigItem aCfg( A2OU( "Office.Linguistic/ServiceManager" ) );

        const sal_Char * apServices[3]      =  { SN_THESAURUS,         SN_SPELLCHECKER,            SN_HYPHENATOR };
        const sal_Char * apActiveLists[3]    =  { "ThesaurusList",      "SpellCheckerList",         "HyphenatorList" };
        const sal_Char * apLastFoundLists[3] =  { "LastFoundThesauri",  "LastFoundSpellCheckers",   "LastFoundHyphenators" };

        for (int k = 0;  k < 3;  ++k)
        {
            OUString aService( A2OU( (sal_Char *)apServices[k]) );
            OUString aActiveList( A2OU( (sal_Char *) apActiveLists[k]) );
            OUString aLastFoundList( A2OU( (sal_Char *) apLastFoundLists[k]) );
            INT32 i;

            //
            // remove configured but not available language/services entries
            //
            Sequence< OUString > aNodeNames( aCfg.GetNodeNames( aActiveList ) );   // list of configured locales
            INT32 nNodeNames = aNodeNames.getLength();
            const OUString *pNodeName = aNodeNames.getConstArray();
            for (i = 0;  i < nNodeNames;  ++i)
            {
                Locale aLocale( SvxCreateLocale( ConvertIsoStringToLanguage(pNodeName[i]) ) );
                Sequence< OUString > aCfgSvcs(
                        xLngSvcMgr->getConfiguredServices( aService, aLocale ));
                Sequence< OUString > aAvailSvcs(
                        xLngSvcMgr->getAvailableServices( aService, aLocale ));
                aCfgSvcs = lcl_RemoveMissingEntries( aCfgSvcs, aAvailSvcs );
                xLngSvcMgr->setConfiguredServices( aService, aLocale, aCfgSvcs );
            }

            //
            // add new available language/servcice entries
            //
            Reference< XAvailableLocales > xAvail( xLngSvcMgr, UNO_QUERY );
            Sequence< Locale > aAvailLocales( xAvail->getAvailableLocales(aService) );
            INT32 nAvailLocales = aAvailLocales.getLength();
            const Locale *pAvailLocale = aAvailLocales.getConstArray();
            for (i = 0;  i < nAvailLocales;  ++i)
            {
                Sequence< OUString > aAvailSvcs(
                        xLngSvcMgr->getAvailableServices( aService, pAvailLocale[i] ));
                Sequence< OUString > aLastFoundSvcs(
                        lcl_GetLastFoundSvcs( aCfg, aLastFoundList , pAvailLocale[i] ));
                Sequence< OUString > aNewSvcs =
                        lcl_GetNewEntries( aLastFoundSvcs, aAvailSvcs );

                Sequence< OUString > aCfgSvcs(
                        xLngSvcMgr->getConfiguredServices( aService, pAvailLocale[i] ));
                aCfgSvcs = lcl_MergeSeq( aCfgSvcs, aNewSvcs );
                xLngSvcMgr->setConfiguredServices( aService, pAvailLocale[i], aCfgSvcs );
            }

            //
            // set last found services to currently available ones
            //
            Sequence< PropertyValue > aValues( nAvailLocales );
            PropertyValue *pValue = aValues.getArray();
            for (i = 0;  i < nAvailLocales;  ++i)
            {
                Sequence< OUString > aSvcImplNames(
                        xLngSvcMgr->getConfiguredServices( aService, pAvailLocale[i] ) );

#ifdef DEBUG
                INT32 nSvcs = aSvcImplNames.getLength();
                const OUString *pSvcImplName = aSvcImplNames.getConstArray();
                for (INT32 j = 0;  j < nSvcs;  ++j)
                {
                    OUString aImplName( pSvcImplName[j] );
                }
#endif
                // build value to be written back to configuration
                Any aCfgAny;
                aCfgAny <<= aSvcImplNames;
                DBG_ASSERT( aCfgAny.hasValue(), "missing value for 'Any' type" );

                OUString aCfgLocaleStr( ConvertLanguageToIsoString(
                                            SvxLocaleToLanguage( pAvailLocale[i] ) ) );
                pValue->Value = aCfgAny;
                pValue->Name  = aLastFoundList;
                pValue->Name += OUString::valueOf( (sal_Unicode) '/' );
                pValue->Name += aCfgLocaleStr;
                pValue++;
            }
            // change, add new or replace existing entries.
            BOOL bRes = aCfg.ReplaceSetProperties( aLastFoundList, aValues );
            DBG_ASSERT( bRes, "failed to set LastFound list" );
        }

        bUpdated = TRUE;
    }
}

///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL
//! when only the XSupportedLocales interface is used.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when "real" work needs to be done only.
class ThesDummy_Impl :
    public cppu::WeakImplHelper1< XThesaurus >
{
    Reference< XThesaurus >     xThes;      // the real one...
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
        SvtLinguConfigItem aCfg( A2OU( "Office.Linguistic/ServiceManager" ) );
        String  aNode( A2OU( "ThesaurusList" ) );
        Sequence < OUString > aNodeNames( aCfg.GetNodeNames( aNode ) );
        const OUString *pNodeNames = aNodeNames.getConstArray();
        INT32 nLen = aNodeNames.getLength();
        pLocaleSeq = new Sequence< Locale >( nLen );
        Locale *pLocale = pLocaleSeq->getArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            pLocale[i] = SvxCreateLocale(
                            ConvertIsoStringToLanguage( pNodeNames[i] ) );
        }
    }
}


void ThesDummy_Impl::GetThes_Impl()
{
    // update configuration before accessing the service
    if (!SvxLinguConfigUpdate::IsUpdated())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xThes.is())
    {
        Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
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
    if (SvxLinguConfigUpdate::IsUpdated())
        GetThes_Impl();
    if (xThes.is())
        return xThes->getLocales();
    else if (!pLocaleSeq)
        GetCfgLocales();
    return *pLocaleSeq;
}


sal_Bool SAL_CALL
        ThesDummy_Impl::hasLocale( const lang::Locale& rLocale )
            throw(uno::RuntimeException)
{
    if (SvxLinguConfigUpdate::IsUpdated())
        GetThes_Impl();
    if (xThes.is())
        return xThes->hasLocale( rLocale );
    else if (!pLocaleSeq)
        GetCfgLocales();
    BOOL bFound = FALSE;
    INT32 nLen = pLocaleSeq->getLength();
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
    Reference< XSpellChecker1 >     xSpell;      // the real one...

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
    if (!SvxLinguConfigUpdate::IsUpdated())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xSpell.is())
    {
        Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
        if (xLngSvcMgr.is())
            xSpell = Reference< XSpellChecker1 >( xLngSvcMgr->getSpellChecker(), UNO_QUERY );
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
    BOOL bRes = FALSE;
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
    BOOL bRes = TRUE;
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
    Reference< XHyphenator >     xHyph;      // the real one...

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
    if (!SvxLinguConfigUpdate::IsUpdated())
        SvxLinguConfigUpdate::UpdateAll();

    if (!xHyph.is())
    {
        Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
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
    BOOL bRes = FALSE;
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
    Reference< XComponent >         xDesktop;

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

    Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if ( xMgr.is() )
    {
        xDesktop = Reference< XComponent > ( xMgr->createInstance(
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
Reference< XLinguServiceManager >   LinguMgr::xLngSvcMgr    = 0;
Reference< XSpellChecker1 >     LinguMgr::xSpell        = 0;
Reference< XHyphenator >        LinguMgr::xHyph         = 0;
Reference< XThesaurus >         LinguMgr::xThes         = 0;
Reference< XDictionaryList >    LinguMgr::xDicList      = 0;
Reference< XPropertySet >       LinguMgr::xProp         = 0;
Reference< XDictionary1 >       LinguMgr::xIgnoreAll    = 0;
Reference< XDictionary1 >       LinguMgr::xChangeAll    = 0;


Reference< XLinguServiceManager > LinguMgr::GetLngSvcMgr()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    return xLngSvcMgr;
}


Reference< XSpellChecker1 > LinguMgr::GetSpellChecker()
{
    return xSpell.is() ? xSpell : GetSpell();
}

Reference< XHyphenator > LinguMgr::GetHyphenator()
{
    return xHyph.is() ? xHyph : GetHyph();
}

Reference< XThesaurus > LinguMgr::GetThesaurus()
{
    return xThes.is() ? xThes : GetThes();
}

Reference< XDictionaryList > LinguMgr::GetDictionaryList()
{
    return xDicList.is() ? xDicList : GetDicList();
}

Reference< XPropertySet > LinguMgr::GetLinguPropertySet()
{
    return xProp.is() ? xProp : GetProp();
}

Reference< XDictionary1 > LinguMgr::GetStandardDic()
{
    //! don't hold reference to this
    //! (it may be removed from dictionary list and needs to be
    //! created empty if accessed again)
    return GetStandard();
}

Reference< XDictionary1 > LinguMgr::GetIgnoreAllList()
{
    return xIgnoreAll.is() ? xIgnoreAll : GetIgnoreAll();
}

Reference< XDictionary1 > LinguMgr::GetChangeAllList()
{
    return xChangeAll.is() ? xChangeAll : GetChangeAll();
}

Reference< XSpellChecker1 > LinguMgr::GetSpell()
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
        xSpell = Reference< XSpellChecker1 > (
                        xLngSvcMgr->getSpellChecker(), UNO_QUERY );
    }
*/
    return xSpell;
}

Reference< XHyphenator > LinguMgr::GetHyph()
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

Reference< XThesaurus > LinguMgr::GetThes()
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


Reference< XDictionaryList > LinguMgr::GetDicList()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xDicList = Reference< XDictionaryList > ( xMgr->createInstance(
                    A2OU("com.sun.star.linguistic2.DictionaryList") ), UNO_QUERY );
    }
    return xDicList;
}

Reference< XPropertySet > LinguMgr::GetProp()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xProp = Reference< XPropertySet > ( xMgr->createInstance(
                    A2OU("com.sun.star.linguistic2.LinguProperties") ), UNO_QUERY );
    }
    return xProp;
}

Reference< XDictionary1 > LinguMgr::GetIgnoreAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (xTmpDicList.is())
    {
        xIgnoreAll = Reference< XDictionary1 > ( xTmpDicList->getDictionaryByName(
                    A2OU("IgnoreAllList") ), UNO_QUERY );
    }
    return xIgnoreAll;
}

Reference< XDictionary1 > LinguMgr::GetChangeAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XDictionaryList >  xDicList( GetDictionaryList() , UNO_QUERY );
    if (xDicList.is())
    {
        xChangeAll = Reference< XDictionary1 > (
                        xDicList->createDictionary(
                            A2OU("ChangeAllList"),
                            SvxCreateLocale( LANGUAGE_NONE ),
                            DictionaryType_NEGATIVE, String() ), UNO_QUERY );
    }
    return xChangeAll;
}

Reference< XDictionary1 > LinguMgr::GetStandard()
{
    // Tries to return a dictionary which may hold positive entries is
    // persistent and not read-only.

    if (bExiting)
        return 0;

    Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (!xTmpDicList.is())
        return NULL;

    const OUString aDicName( RTL_CONSTASCII_USTRINGPARAM( "standard.dic" ) );
    Reference< XDictionary1 >   xDic( xTmpDicList->getDictionaryByName( aDicName ),
                                      UNO_QUERY );
    if (!xDic.is())
    {
        // try to create standard dictionary
        Reference< XDictionary >    xTmp;
        try
        {
            xTmp =  xTmpDicList->createDictionary( aDicName,
                        SvxCreateLocale( LANGUAGE_NONE ),
                        DictionaryType_POSITIVE,
                        SvxGetDictionaryURL( aDicName, sal_True ) );
        }
        catch(com::sun::star::uno::Exception &)
        {
        }

        // add new dictionary to list
        if (xTmp.is())
            xTmpDicList->addDictionary( xTmp );
        xDic = Reference< XDictionary1 > ( xTmp, UNO_QUERY );
    }
#ifdef DEBUG
    Reference< XStorable >      xStor( xDic, UNO_QUERY );
    DBG_ASSERT( xDic.is() && xDic->getDictionaryType() == DictionaryType_POSITIVE,
            "wrong dictionary type");
    DBG_ASSERT( xDic.is() && xDic->getLanguage() == LANGUAGE_NONE,
            "wrong dictionary language");
    DBG_ASSERT( !xStor.is() || (xStor->hasLocation() && !xStor->isReadonly()),
            "dictionary not editable" );
#endif

    return xDic;
}

///////////////////////////////////////////////////////////////////////////

Reference< XSpellChecker1 >  SvxGetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

Reference< XHyphenator >  SvxGetHyphenator()
{
    return LinguMgr::GetHyphenator();
}

Reference< XThesaurus >  SvxGetThesaurus()
{
    return LinguMgr::GetThesaurus();
}

Reference< XDictionaryList >  SvxGetDictionaryList()
{
    return LinguMgr::GetDictionaryList();
}

Reference< XPropertySet >   SvxGetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one
Reference< XDictionary1 >  SvxGetOrCreatePosDic(
        Reference< XDictionaryList >  xDicList )
{
    return LinguMgr::GetStandardDic();
}

Reference< XDictionary1 >  SvxGetIgnoreAllList()
{
    return LinguMgr::GetIgnoreAllList();
}

Reference< XDictionary1 >  SvxGetChangeAllList()
{
    return LinguMgr::GetChangeAllList();
}

///////////////////////////////////////////////////////////////////////////


#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord )
{
    SvxAlternativeSpelling aRes;
    if (rHyphWord.is() && rHyphWord->isAlternativeSpelling())
    {
        OUString aWord( rHyphWord->getWord() ),
                 aAltWord( rHyphWord->getHyphenatedWord() );
        INT16   nHyphenationPos     = rHyphWord->getHyphenationPos(),
                nHyphenPos          = rHyphWord->getHyphenPos();
        INT16   nLen    = (INT16)aWord.getLength();
        INT16   nAltLen = (INT16)aAltWord.getLength();
        const sal_Unicode *pWord    = aWord.getStr(),
                          *pAltWord = aAltWord.getStr();

        // count number of chars from the left to the
        // hyphenation pos / hyphen pos that are equal
        INT16 nL = 0;
        while (nL <= nHyphenationPos && nL <= nHyphenPos
               && pWord[ nL ] == pAltWord[ nL ])
            ++nL;
        // count number of chars from the right to the
        // hyphenation pos / hyphen pos that are equal
        INT16 nR = 0;
        INT32 nIdx    = nLen - 1;
        INT32 nAltIdx = nAltLen - 1;
        while (nIdx > nHyphenationPos && nAltIdx > nHyphenPos
               && pWord[ nIdx-- ] == pAltWord[ nAltIdx-- ])
            ++nR;

        aRes.aReplacement       = OUString( aAltWord.copy( nL, nAltLen - nL - nR ) );
        aRes.nChangedPos        = (INT16) nL;
        aRes.nChangedLength     = nLen - nL - nR;
        aRes.bIsAltSpelling     = TRUE;
        aRes.xHyphWord          = rHyphWord;
    }
    return aRes;
}


///////////////////////////////////////////////////////////////////////////

SvxDicListChgClamp::SvxDicListChgClamp( Reference< XDictionaryList >  &rxDicList ) :
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

String SvxGetDictionaryURL(const String &rDicName, sal_Bool bIsUserDic)
{
    // build URL to use for new (persistent) dictionaries

    SvtPathOptions aPathOpt;
    String aDirName( bIsUserDic ?
            aPathOpt.GetUserDictionaryPath() : aPathOpt.GetDictionaryPath() );

    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( aDirName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
    aURLObj.Append( rDicName, INetURLObject::ENCODE_ALL );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");

    return aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
}

//TL:TODO: soll mal den rictigen Rückgabetyp bekommen!
sal_Bool SvxAddEntryToDic(
        Reference< XDictionary >  &rxDic,
        const OUString &rWord, sal_Bool bIsNeg,
        const OUString &rRplcTxt, sal_Int16 nRplcLang,
        sal_Bool bStripDot )
{
    if (!rxDic.is())
        return DIC_ERR_NOT_EXISTS;

    OUString aTmp( rWord );
    if (bStripDot)
    {
        sal_Int32 nLen = rWord.getLength();
        if (nLen > 0  &&  '.' == rWord[ nLen - 1])
        {
            // remove trailing '.'
            // (this is the official way to do this :-( )
            aTmp = aTmp.copy( 0, nLen - 1 );
        }
    }
    sal_Bool bAddOk = rxDic->add( aTmp, bIsNeg, rRplcTxt );

    sal_Int16 nRes = DIC_ERR_NONE;
    if (!bAddOk)
    {
        if (rxDic->isFull())
            nRes = DIC_ERR_FULL;
        else
        {
            Reference< XStorable >  xStor( rxDic, UNO_QUERY );
            if (xStor.is() && xStor->isReadonly())
                nRes = DIC_ERR_READONLY;
            else
                nRes = DIC_ERR_UNKNOWN;
        }
    }

    return (sal_Bool)nRes;
}

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
        nRes = InfoBox( pParent, SVX_RESSTR( nRid ) ).Execute();
    }
    return nRes;
}

sal_Bool SvxSaveDictionaries( const Reference< XDictionaryList >  &xDicList )
{
    if (!xDicList.is())
        return sal_True;

    sal_Bool bRet = sal_True;

    Sequence< Reference< XDictionary >  > aDics( xDicList->getDictionaries() );
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    INT32 nCount = aDics.getLength();
    for (INT32 i = 0;  i < nCount;  i++)
    {
        try
        {
            Reference< XStorable >  xStor( pDic[i], UNO_QUERY );
            if (xStor.is())
            {
                if (!xStor->isReadonly() && xStor->hasLocation())
                    xStor->store();
            }
        }
        catch(com::sun::star::uno::Exception &)
        {
            bRet = sal_False;
        }
    }

    return bRet;
}

LanguageType SvxLocaleToLanguage( const Locale& rLocale )
{
    //  empty Locale -> LANGUAGE_NONE
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_NONE;

    //  Variant of Locale is ignored
    return ConvertIsoNamesToLanguage( rLocale.Language, rLocale.Country );
}

Locale& SvxLanguageToLocale( Locale& rLocale, LanguageType eLang )
{
    String aLangStr, aCtryStr;
    if ( eLang != LANGUAGE_NONE /* &&  eLang != LANGUAGE_SYSTEM */)
        ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );

    rLocale.Language = aLangStr;
    rLocale.Country  = aCtryStr;
    rLocale.Variant  = OUString();

    return rLocale;
}

Locale SvxCreateLocale( LanguageType eLang )
{
    String aLangStr, aCtryStr;
    if ( eLang != LANGUAGE_NONE /* &&  eLang != LANGUAGE_SYSTEM */)
        ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );

    return Locale( aLangStr, aCtryStr, OUString() );
}


