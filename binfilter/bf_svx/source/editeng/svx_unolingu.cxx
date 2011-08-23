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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <unolingu.hxx>

#include <cppuhelper/implbase1.hxx>	// helper for implementations

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <bf_svtools/pathoptions.hxx>
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
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#endif

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <bf_svtools/lingucfg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

//#include <bf_linguistic/misc.hxx>


#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#include "dialogs.hrc"
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
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


/*N*/ static Reference< XLinguServiceManager > GetLngSvcMgr_Impl()
/*N*/ {
/*N*/ 	Reference< XLinguServiceManager > xRes;
/*N*/ 	Reference< XMultiServiceFactory >  xMgr = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	if (xMgr.is())
/*N*/ 	{
/*N*/ 		xRes = Reference< XLinguServiceManager > ( xMgr->createInstance(
/*N*/ 				OUString( RTL_CONSTASCII_USTRINGPARAM(
/*N*/ 					"com.sun.star.linguistic2.LinguServiceManager" ) ) ), UNO_QUERY ) ;
/*N*/ 	}
/*N*/ 	return xRes;
/*N*/ }

///////////////////////////////////////////////////////////////////////////

// static member initialization
BOOL SvxLinguConfigUpdate::bUpdated = FALSE;

/*N*/ void SvxLinguConfigUpdate::UpdateAll()
/*N*/ {
/*N*/ }

///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL
//! when only the XSupportedLocales interface is used.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when "real" work needs to be done only.
/*N*/ class ThesDummy_Impl :
/*N*/     public cppu::WeakImplHelper1< XThesaurus >
/*N*/ {
/*N*/     Reference< XThesaurus >     xThes;      // the real one...
/*N*/     Sequence< Locale >         *pLocaleSeq;
/*N*/ 
/*N*/     void GetCfgLocales();
/*N*/ 
/*N*/     void GetThes_Impl();
/*N*/ 
/*N*/ public:
/*N*/     ThesDummy_Impl() : pLocaleSeq(0)  {}
/*N*/     ~ThesDummy_Impl();
/*N*/ 
/*N*/     // XSupportedLocales
/*N*/     virtual ::com::sun::star::uno::Sequence<
/*N*/ 			::com::sun::star::lang::Locale > SAL_CALL
/*N*/ 		getLocales()
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/     virtual sal_Bool SAL_CALL
/*N*/         hasLocale( const ::com::sun::star::lang::Locale& rLocale )
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/ 
/*N*/ 	// XThesaurus
/*N*/     virtual ::com::sun::star::uno::Sequence<
/*N*/ 			::com::sun::star::uno::Reference<
/*N*/ 				::com::sun::star::linguistic2::XMeaning > > SAL_CALL
/*N*/         queryMeanings( const ::rtl::OUString& rTerm,
/*N*/                 const ::com::sun::star::lang::Locale& rLocale,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/ };


/*N*/ ThesDummy_Impl::~ThesDummy_Impl()
/*N*/ {
/*N*/     delete pLocaleSeq;
/*N*/ }


/*N*/ void ThesDummy_Impl::GetCfgLocales()
/*N*/ {
/*N*/     if (!pLocaleSeq)
/*N*/     {
/*N*/         SvtLinguConfig aCfg;
/*N*/         String  aNode( A2OU( "ServiceManager/ThesaurusList" ) );
/*N*/         Sequence < OUString > aNodeNames( aCfg.GetNodeNames( aNode ) );
/*N*/         const OUString *pNodeNames = aNodeNames.getConstArray();
/*N*/         INT32 nLen = aNodeNames.getLength();
/*N*/         pLocaleSeq = new Sequence< Locale >( nLen );
/*N*/         Locale *pLocale = pLocaleSeq->getArray();
/*N*/         for (INT32 i = 0;  i < nLen;  ++i)
/*N*/         {
/*N*/             pLocale[i] = SvxCreateLocale(
/*N*/                             MsLangId::convertIsoStringToLanguage( pNodeNames[i] ) );
/*N*/         }
/*N*/     }
/*N*/ }


/*N*/ void ThesDummy_Impl::GetThes_Impl()
/*N*/ {
/*N*/     // update configuration before accessing the service
/*N*/     if (!SvxLinguConfigUpdate::IsUpdated())
/*?*/         SvxLinguConfigUpdate::UpdateAll();
/*N*/ 
/*N*/     if (!xThes.is())
/*N*/     {
/*N*/         Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
/*N*/         if (xLngSvcMgr.is())
/*N*/             xThes = xLngSvcMgr->getThesaurus();
/*N*/ 
/*?*/         if (xThes.is())
/*?*/         {
/*?*/             // no longer needed...
/*?*/             delete pLocaleSeq;    pLocaleSeq = 0;
/*?*/         }
/*N*/     }
/*N*/ }


/*N*/ uno::Sequence< lang::Locale > SAL_CALL
/*N*/         ThesDummy_Impl::getLocales()
/*N*/             throw(uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); uno::Sequence< lang::Locale > aa; return aa; //STRIP001 
/*N*/ }


/*N*/ sal_Bool SAL_CALL
/*N*/         ThesDummy_Impl::hasLocale( const lang::Locale& rLocale )
/*N*/             throw(uno::RuntimeException)
/*N*/ {
/*N*/     if (SvxLinguConfigUpdate::IsUpdated())
/*N*/         GetThes_Impl();
/*N*/     if (xThes.is())
/*?*/         return xThes->hasLocale( rLocale );
/*N*/     else if (!pLocaleSeq)
/*N*/         GetCfgLocales();
/*N*/     BOOL bFound = FALSE;
/*N*/     INT32 nLen = pLocaleSeq->getLength();
/*N*/     const Locale *pLocale = pLocaleSeq->getConstArray();
/*N*/     const Locale *pEnd = pLocale + nLen;
/*N*/     for ( ;  pLocale < pEnd  &&  !bFound;  ++pLocale)
/*N*/     {
/*N*/         bFound = pLocale->Language == rLocale.Language  &&
/*N*/                  pLocale->Country  == rLocale.Country   &&
/*N*/                  pLocale->Variant  == rLocale.Variant;
/*N*/     }
/*N*/     return bFound;
/*N*/ }


/*N*/ uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL
/*N*/         ThesDummy_Impl::queryMeanings(
/*N*/                 const ::rtl::OUString& rTerm,
/*N*/                 const lang::Locale& rLocale,
/*N*/                 const beans::PropertyValues& rProperties )
/*N*/             throw(lang::IllegalArgumentException,
/*N*/                   uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); uno::Sequence< uno::Reference< linguistic2::XMeaning > > aRes;return aRes; //STRIP001 
/*N*/ }


///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
/*N*/ class SpellDummy_Impl :
/*N*/     public cppu::WeakImplHelper1< XSpellChecker1 >
/*N*/ {
/*N*/     Reference< XSpellChecker1 >     xSpell;      // the real one...
/*N*/ 
/*N*/     void    GetSpell_Impl();
/*N*/ 
/*N*/ public:
/*N*/ 
/*N*/ 	// XSupportedLanguages (for XSpellChecker1)
/*N*/     virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL
/*N*/ 		getLanguages()
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/     virtual sal_Bool SAL_CALL
/*N*/ 		hasLanguage( sal_Int16 nLanguage )
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/ 
/*N*/ 	// XSpellChecker1 (same as XSpellChecker but sal_Int16 for language)
/*N*/ 	virtual sal_Bool SAL_CALL
/*N*/         isValid( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/ 	virtual ::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL
/*N*/         spell( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/ };


/*N*/ void SpellDummy_Impl::GetSpell_Impl()
/*N*/ { DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }


/*N*/ uno::Sequence< sal_Int16 > SAL_CALL
/*N*/     SpellDummy_Impl::getLanguages()
/*N*/         throw(uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Sequence< sal_Int16 >(); //STRIP001 
/*N*/ }


/*N*/ sal_Bool SAL_CALL
/*N*/     SpellDummy_Impl::hasLanguage( sal_Int16 nLanguage )
/*N*/         throw(uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP");return FALSE; //STRIP001 
/*N*/ }


/*N*/ sal_Bool SAL_CALL
/*N*/     SpellDummy_Impl::isValid( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
/*N*/             const beans::PropertyValues& rProperties )
/*N*/         throw(lang::IllegalArgumentException,
/*N*/               uno::RuntimeException)
/*N*/ {
/*?*/     GetSpell_Impl();
/*?*/     BOOL bRes = TRUE;
/*?*/     if (xSpell.is())
/*?*/         bRes = xSpell->isValid( rWord, nLanguage, rProperties );
/*?*/     return bRes;
/*N*/ }


/*N*/ uno::Reference< linguistic2::XSpellAlternatives > SAL_CALL
/*N*/     SpellDummy_Impl::spell( const ::rtl::OUString& rWord, sal_Int16 nLanguage,
/*N*/             const beans::PropertyValues& rProperties )
/*N*/         throw(lang::IllegalArgumentException,
/*N*/               uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); uno::Reference< linguistic2::XSpellAlternatives > xRes; return xRes; //STRIP001 
/*N*/ }


///////////////////////////////////////////////////////////////////////////


//! Dummy implementation in order to avoid loading of lingu DLL.
//! The dummy accesses the real implementation (and thus loading the DLL)
//! when it needs to be done only.
/*N*/ class HyphDummy_Impl :
/*N*/     public cppu::WeakImplHelper1< XHyphenator >
/*N*/ {
/*N*/     Reference< XHyphenator >     xHyph;      // the real one...
/*N*/ 
/*N*/     void    GetHyph_Impl();
/*N*/ 
/*N*/ public:
/*N*/ 
/*N*/     // XSupportedLocales
/*N*/     virtual ::com::sun::star::uno::Sequence<
/*N*/ 			::com::sun::star::lang::Locale > SAL_CALL
/*N*/ 		getLocales()
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/     virtual sal_Bool SAL_CALL
/*N*/         hasLocale( const ::com::sun::star::lang::Locale& rLocale )
/*N*/ 			throw(::com::sun::star::uno::RuntimeException);
/*N*/ 
/*N*/     // XHyphenator
/*N*/     virtual ::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
/*N*/         hyphenate( const ::rtl::OUString& rWord,
/*N*/                 const ::com::sun::star::lang::Locale& rLocale,
/*N*/ 				sal_Int16 nMaxLeading,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/     virtual ::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
/*N*/         queryAlternativeSpelling( const ::rtl::OUString& rWord,
/*N*/                 const ::com::sun::star::lang::Locale& rLocale,
/*N*/ 				sal_Int16 nIndex,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/     virtual ::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL
/*N*/ 		createPossibleHyphens(
/*N*/                 const ::rtl::OUString& rWord,
/*N*/                 const ::com::sun::star::lang::Locale& rLocale,
/*N*/                 const ::com::sun::star::beans::PropertyValues& rProperties )
/*N*/ 			throw(::com::sun::star::lang::IllegalArgumentException,
/*N*/ 				  ::com::sun::star::uno::RuntimeException);
/*N*/ };


/*N*/ void HyphDummy_Impl::GetHyph_Impl()
/*N*/ {
/*N*/     // update configuration before accessing the service
/*N*/     if (!SvxLinguConfigUpdate::IsUpdated())
/*N*/         SvxLinguConfigUpdate::UpdateAll();
/*N*/ 
/*N*/     if (!xHyph.is())
/*N*/     {
/*N*/         Reference< XLinguServiceManager > xLngSvcMgr( GetLngSvcMgr_Impl() );
/*N*/         if (xLngSvcMgr.is())
/*N*/             xHyph = xLngSvcMgr->getHyphenator();
/*N*/     }
/*N*/ }


/*N*/ uno::Sequence< lang::Locale > SAL_CALL
/*N*/     HyphDummy_Impl::getLocales()
/*N*/         throw(uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Sequence< lang::Locale >();//STRIP001 
/*N*/ }


/*N*/ sal_Bool SAL_CALL
/*N*/     HyphDummy_Impl::hasLocale( const lang::Locale& rLocale )
/*N*/         throw(uno::RuntimeException)
/*N*/ {
/*N*/     GetHyph_Impl();
/*N*/     BOOL bRes = FALSE;
/*N*/     if (xHyph.is())
/*N*/         bRes = xHyph->hasLocale( rLocale );
/*N*/     return bRes;
/*N*/ }


/*N*/ uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
/*N*/     HyphDummy_Impl::hyphenate(
/*N*/             const ::rtl::OUString& rWord,
/*N*/             const lang::Locale& rLocale,
/*N*/             sal_Int16 nMaxLeading,
/*N*/             const beans::PropertyValues& rProperties )
/*N*/         throw(lang::IllegalArgumentException,
/*N*/               uno::RuntimeException)
/*N*/ {
/*N*/     GetHyph_Impl();
/*N*/     uno::Reference< linguistic2::XHyphenatedWord > xRes;
/*N*/     if (xHyph.is())
/*N*/         xRes = xHyph->hyphenate( rWord, rLocale, nMaxLeading, rProperties );
/*N*/     return xRes;
/*N*/ }


/*N*/ uno::Reference< linguistic2::XHyphenatedWord > SAL_CALL
/*N*/     HyphDummy_Impl::queryAlternativeSpelling(
/*N*/             const ::rtl::OUString& rWord,
/*N*/             const lang::Locale& rLocale,
/*N*/             sal_Int16 nIndex,
/*N*/             const PropertyValues& rProperties )
/*N*/         throw(lang::IllegalArgumentException,
/*N*/               uno::RuntimeException)
/*N*/ {
/*N*/     GetHyph_Impl();
/*N*/     uno::Reference< linguistic2::XHyphenatedWord > xRes;
/*N*/     if (xHyph.is())
/*N*/         xRes = xHyph->queryAlternativeSpelling( rWord, rLocale, nIndex, rProperties );
/*N*/     return xRes;
/*N*/ }


/*N*/ uno::Reference< linguistic2::XPossibleHyphens > SAL_CALL
/*N*/     HyphDummy_Impl::createPossibleHyphens(
/*N*/             const ::rtl::OUString& rWord,
/*N*/             const lang::Locale& rLocale,
/*N*/             const beans::PropertyValues& rProperties )
/*N*/         throw(lang::IllegalArgumentException,
/*N*/               uno::RuntimeException)
/*N*/ {
/*?*/     GetHyph_Impl();
/*?*/     uno::Reference< linguistic2::XPossibleHyphens > xRes;
/*?*/     if (xHyph.is())
/*?*/         xRes = xHyph->createPossibleHyphens( rWord, rLocale, rProperties );
/*?*/     return xRes;
/*N*/ }


///////////////////////////////////////////////////////////////////////////


/*N*/ typedef cppu::WeakImplHelper1 < XEventListener > LinguMgrAppExitLstnrBaseClass;

/*N*/ class LinguMgrAppExitLstnr : public LinguMgrAppExitLstnrBaseClass
/*N*/ {
/*N*/ 	Reference< XComponent > 		xDesktop;
/*N*/ 
/*N*/ public:
/*N*/ 	LinguMgrAppExitLstnr();
/*N*/ 	virtual ~LinguMgrAppExitLstnr();
/*N*/ 
/*N*/ 	virtual	void	AtExit() = 0;
/*N*/ 
/*N*/ 
/*N*/ 	// lang::XEventListener
/*N*/     virtual void 	SAL_CALL disposing(const EventObject& rSource)
/*N*/ 			throw( RuntimeException );
/*N*/ };

/*N*/ LinguMgrAppExitLstnr::LinguMgrAppExitLstnr()
/*N*/ {
/*N*/ 	// add object to frame::Desktop EventListeners in order to properly call
/*N*/ 	// the AtExit function at appliction exit.
/*N*/ 
/*N*/ 	Reference< XMultiServiceFactory >  xMgr = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	if ( xMgr.is() )
/*N*/ 	{
/*N*/ 		xDesktop = Reference< XComponent > ( xMgr->createInstance(
/*N*/ 				OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY ) ;
/*N*/ 		if (xDesktop.is())
/*N*/ 			xDesktop->addEventListener( this );
/*N*/ 	}
/*N*/ }

/*N*/ LinguMgrAppExitLstnr::~LinguMgrAppExitLstnr()
/*N*/ {
/*N*/ 	if (xDesktop.is())
/*N*/ 	{
/*?*/ 		xDesktop->removeEventListener( this );
/*?*/ 		xDesktop = NULL;	//! release reference to desktop
/*N*/ 	}
/*N*/ 	DBG_ASSERT(!xDesktop.is(), "reference to desktop should be realeased");
/*N*/ }

/*N*/ void LinguMgrAppExitLstnr::disposing(const EventObject& rSource)
/*N*/ 		throw( RuntimeException )
/*N*/ {
/*N*/ 	if (xDesktop.is()  &&  rSource.Source == xDesktop)
/*N*/ 	{
/*N*/ 		xDesktop->removeEventListener( this );
/*N*/ 		xDesktop = NULL;	//! release reference to desktop
/*N*/ 
/*N*/ 		AtExit();
/*N*/ 	}
/*N*/ }

///////////////////////////////////////////////////////////////////////////

/*N*/ class LinguMgrExitLstnr : public LinguMgrAppExitLstnr
/*N*/ {
/*N*/ public:
/*N*/ 	virtual	void	AtExit();
/*N*/ };

/*N*/ void LinguMgrExitLstnr::AtExit()
/*N*/ {
/*N*/ 	// release references
/*N*/ 	LinguMgr::xLngSvcMgr	= 0;
/*N*/ 	LinguMgr::xSpell		= 0;
/*N*/ 	LinguMgr::xHyph			= 0;
/*N*/ 	LinguMgr::xThes			= 0;
/*N*/ 	LinguMgr::xDicList		= 0;
/*N*/ 	LinguMgr::xProp			= 0;
/*N*/ 	LinguMgr::xIgnoreAll	= 0;
/*N*/ 	LinguMgr::xChangeAll	= 0;
/*N*/ 
/*N*/ 	LinguMgr::bExiting		= sal_True;
/*N*/ 
/*N*/ 	//TL:TODO: MBA fragen wie ich ohne Absturz hier meinen Speicher
/*N*/ 	//  wieder freibekomme...
/*N*/ 	//delete LinguMgr::pExitLstnr;
/*N*/ 	LinguMgr::pExitLstnr	= 0;
/*N*/ }

///////////////////////////////////////////////////////////////////////////


// static member initialization
/*N*/ LinguMgrExitLstnr * 			LinguMgr::pExitLstnr	= 0;
/*N*/ sal_Bool						LinguMgr::bExiting		= sal_False;
/*N*/ Reference< XLinguServiceManager >	LinguMgr::xLngSvcMgr	= 0;
/*N*/ Reference< XSpellChecker1 > 	LinguMgr::xSpell		= 0;
/*N*/ Reference< XHyphenator > 		LinguMgr::xHyph			= 0;
/*N*/ Reference< XThesaurus > 		LinguMgr::xThes			= 0;
/*N*/ Reference< XDictionaryList > 	LinguMgr::xDicList		= 0;
/*N*/ Reference< XPropertySet > 		LinguMgr::xProp			= 0;
/*N*/ Reference< XDictionary1 > 		LinguMgr::xIgnoreAll	= 0;
/*N*/ Reference< XDictionary1 > 		LinguMgr::xChangeAll	= 0;

/*N*/ Reference< XHyphenator > LinguMgr::GetHyphenator()
/*N*/ {
/*N*/ 	return xHyph.is() ? xHyph : GetHyph();
/*N*/ }

/*N*/ Reference< XDictionaryList > LinguMgr::GetDictionaryList()
/*N*/ {
/*N*/ 	return xDicList.is() ? xDicList : GetDicList();
/*N*/ }

/*N*/ Reference< XHyphenator > LinguMgr::GetHyph()
/*N*/ {
/*N*/ 	if (bExiting)
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	if (!pExitLstnr)
/*N*/ 		pExitLstnr = new LinguMgrExitLstnr;
/*N*/ 
/*N*/     //! use dummy implementation in order to avoid loading of lingu DLL
/*N*/     xHyph = new HyphDummy_Impl;
/*N*/ 
/*
    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xHyph = xLngSvcMgr->getHyphenator();
    }
*/
/*N*/ 	return xHyph;
/*N*/ }

/*N*/ Reference< XDictionaryList > LinguMgr::GetDicList()
/*N*/ {
/*N*/ 	if (bExiting)
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	if (!pExitLstnr)
/*N*/ 		pExitLstnr = new LinguMgrExitLstnr;
/*N*/ 
/*N*/ 	Reference< XMultiServiceFactory >  xMgr( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	if (xMgr.is())
/*N*/ 	{
/*N*/ 		xDicList = Reference< XDictionaryList > ( xMgr->createInstance(
/*N*/                     A2OU("com.sun.star.linguistic2.DictionaryList") ), UNO_QUERY );
/*N*/ 	}
/*N*/ 	return xDicList;
/*N*/ }





///////////////////////////////////////////////////////////////////////////






//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one



///////////////////////////////////////////////////////////////////////////


#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif



///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////


//TL:TODO: soll mal den rictigen Rückgabetyp bekommen!



/*N*/ LanguageType SvxLocaleToLanguage( const Locale& rLocale )
/*N*/ {
/*N*/ 	//	empty Locale -> LANGUAGE_NONE
/*N*/ 	if ( rLocale.Language.getLength() == 0 )
/*N*/ 		return LANGUAGE_NONE;
/*N*/ 
/*N*/ 	return MsLangId::convertLocaleToLanguage( rLocale );
/*N*/ }

/*N*/ Locale& SvxLanguageToLocale( Locale& rLocale, LanguageType eLang )
/*N*/ {
/*N*/ 	if ( eLang != LANGUAGE_NONE	/* &&  eLang != LANGUAGE_SYSTEM */)
/*N*/ 		MsLangId::convertLanguageToLocale( eLang, rLocale );
/*N*/   else
/*N*/       rLocale = Locale();
/*N*/ 
/*N*/ 	return rLocale;
/*N*/ }

/*N*/ Locale SvxCreateLocale( LanguageType eLang )
/*N*/ {
/*N*/   Locale aLocale;
/*N*/ 	if ( eLang != LANGUAGE_NONE /* &&  eLang != LANGUAGE_SYSTEM */)
/*N*/ 		MsLangId::convertLanguageToLocale( eLang, aLocale );
/*N*/ 
/*N*/ 	return aLocale;
/*N*/ }


}
