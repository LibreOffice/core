/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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



#include <sot/exchange.hxx>
#include "bf_basic/sbxmeth.hxx"
#include <rtl/ustrbuf.hxx>
#include <bf_svtools/stritem.hxx>

#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>


#include <rtl/logfile.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

#include <bf_svtools/ctypeitm.hxx>
#include <bf_svtools/sfxecode.hxx>

#include <bf_svtools/syslocale.hxx>

#include <tools/urlobj.hxx>

#include "docfile.hxx"
#include "sfxsids.hrc"
#include "fltlst.hxx"

#include "request.hxx"
#include "arrdecl.hxx"
#include "app.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
namespace binfilter {

#define SFX_STR_OVERRIDE "Override"

#ifndef ERRCODE_SFX_RESTART
#define ERRCODE_SFX_RESTART 0
#endif

struct FlagMapping_Impl
{
    const char* pName;
    SfxFilterFlags nValue;
};

//----------------------------------------------------------------
/*N*/ inline String ToUpper_Impl( const String &rStr )
/*N*/ {
/*N*/     return SvtSysLocale().GetCharClass().upper( rStr );
/*N*/ }

//----------------------------------------------------------------
class SfxFilterContainer_Impl
{
public:
    SfxFilterContainer_Impl() : bLoadPending( sal_False ) {}
    SfxFilterList_Impl aList;
    String aName;
    sal_Bool bLoadPending;
    SfxFilterContainerFlags eFlags;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xCacheSync; // listener on framework::FilterCache to synchronize our two caches!

};


//----------------------------------------------------------------

/*N*/ SfxFilterContainer::SfxFilterContainer( const String& rName )
/*N*/ {
/*N*/     pImpl = new SfxFilterContainer_Impl;
/*N*/ 	pImpl->eFlags = 0;
/*N*/     pImpl->aName = rName;
/*N*/
/*N*/     // Start synchronize listener for framework::FilterCache changes ...
/*N*/     // But don't do it for basic factory - they has no filters!
/*N*/     if( rName.EqualsAscii("sbasic") == sal_False )
/*N*/     {
/*N*/         SfxFilterListener* pListener = new SfxFilterListener( rName, this );
/*N*/         pImpl->xCacheSync = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >( static_cast< ::com::sun::star::util::XFlushListener* >(pListener), ::com::sun::star::uno::UNO_QUERY );
/*N*/     }
/*N*/ }

//----------------------------------------------------------------

/*N*/ SfxFilterContainer::~SfxFilterContainer()
/*N*/ {
/*N*/     // Let synchronize listener die - we don't need it any longer!
/*N*/     pImpl->xCacheSync = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >();
/*N*/
/*N*/     SfxFilterList_Impl& rList = pImpl->aList;
/*N*/     sal_uInt16 nCount = (sal_uInt16 )rList.Count();
/*N*/     for( sal_uInt16 n = 0; n<nCount; n++ )
/*N*/         delete rList.GetObject( n );
/*N*/     delete pImpl;
/*N*/ }

/*N*/ SfxFilterContainerFlags SfxFilterContainer::GetFlags() const
/*N*/ {
/*N*/ 	return pImpl->eFlags;
/*N*/ }

/*N*/ void SfxFilterContainer::SetFlags( SfxFilterContainerFlags eFlags )
/*N*/ {
/*N*/ 	pImpl->eFlags = eFlags;
/*N*/ }

//----------------------------------------------------------------

/*N*/ const SfxFilter* SfxFilterContainer::GetFilter4Protocol(
/*N*/     SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]

     Gibt den Filter zu einem bestimmten Protokoll zurueck. Diese Methode
     darf sich nicht auf Inhalte beziehen, sondern soll allein anhand von
     Protokoll / ::com::sun::star::util::URL einen Filter auswaehlen.
     In der Basisimplementierung werden einige Protokolle direkt auf
     GetFilter4FilterName abgebildet, man zur Integration des Filters
     in die Filterdetection einfach einen Filter des Protokollnames anlegen
     kann.
 */
/*N*/ {
/*N*/ 	const INetURLObject& rObj = rMed.GetURLObject();
/*N*/ 	String aName = rMed.GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 	// Hier noch pruefen, ob das Doc im Browser dargestellt werden soll
/*N*/ 	SFX_ITEMSET_ARG( rMed.GetItemSet(), pTargetItem, SfxStringItem,
/*N*/ 					 SID_TARGETNAME, sal_False);
/*N*/     if( pTargetItem && pTargetItem->GetValue().EqualsAscii("_beamer") )
/*N*/ 		return 0;
/*N*/     return GetFilter4Protocol( aName, nMust, nDont );
/*N*/ }

/*N*/ const SfxFilter* SfxFilterContainer::GetFilter4Protocol( const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*N*/ {
/*N*/     String aName( rName );
/*N*/ 	aName.ToLowerAscii();
/*N*/ 	sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();
/*N*/ 	for( sal_uInt16 n = 0; n < nCount; n++ )
/*N*/ 	{
/*N*/ 		const SfxFilter* pFilter = pImpl->aList.GetObject( n );
/*N*/ 		SfxFilterFlags nFlags = pFilter->GetFilterFlags();
/*N*/ 		String aString( pFilter->GetURLPattern());
/*N*/ 		if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) &&
/*N*/ 			 WildCard(aString ).Matches( aName ) )
/*N*/ 			return pFilter;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

//----------------------------------------------------------------

/*?*/ ULONG SfxFilterContainer::Execute( SfxMedium& rMedium, SfxFrame*& pFrame) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001
/*?*/ }

//----------------------------------------------------------------

/*N*/ sal_uInt16 SfxFilterContainer::GetFilterCount() const
/*N*/ {
/*N*/ 	// Dazu muessen die Filter geladen werden
/*N*/     return (sal_uInt16) pImpl->aList.Count();
/*N*/ }

//----------------------------------------------------------------

/*N*/ const SfxFilter* SfxFilterContainer::GetFilter( sal_uInt16 nPos ) const
/*N*/ {
/*N*/     return pImpl->aList.GetObject( nPos );
/*N*/ }

//----------------------------------------------------------------
/*   [Beschreibung]

     Da die meisten GetFilter4xxx Routinen in einem Container einfach eine
     Abfrage ueber alle enthaltenen Filter durchlaufen, sind diese in
     einem Makro kodiert.

     nMust sind die Flags, die gesetzt sein muessen, damit der Filter
     in Betracht gezogen wird, nDont duerfen nicht gesetzt sein.
 */

#define IMPL_CONTAINER_LOOP( aMethod, aArgType, aTest )         \
/*?*/ const SfxFilter* SfxFilterContainer::aMethod(                   \
/*?*/     aArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
/*?*/ {                                                               \
/*?*/     const SfxFilter* pFirstFilter=0;                            \
/*?*/     sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();    \
/*?*/     for( sal_uInt16 n = 0; n < nCount; n++ )                    \
/*?*/     {                                                           \
/*?*/         const SfxFilter* pFilter = pImpl->aList.GetObject( n ); \
/*?*/         SfxFilterFlags nFlags = pFilter->GetFilterFlags();      \
/*?*/         if ( (nFlags & nMust) == nMust &&                       \
/*?*/              !(nFlags & nDont ) && aTest )                      \
/*?*/         {                                                       \
/*?*/             if ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED ) \
/*?*/                 return pFilter;                                 \
/*?*/             else if ( !pFirstFilter )                           \
/*?*/                 pFirstFilter = pFilter;                         \
/*?*/         }                                                       \
/*?*/     }                                                           \
/*?*/     return pFirstFilter;                                        \
/*?*/ }

/*   [Beschreibung]

     Ermitelt einen Filter nach seinem Namen. Dieser enthaelt vorangestellt
     den Namen des Containers, falls er nicht im DefaultContainer steckt.
 */

/*N*/ IMPL_CONTAINER_LOOP( GetFilter, const String&,
/*N*/     ( pFilter->GetFilterNameWithPrefix().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL ||
/*N*/         pFilter->GetFilterName().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL ) )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem Mimetypen.
 */
/*?*/ IMPL_CONTAINER_LOOP(
/*?*/     GetFilter4Mime, const String&,
/*?*/     pFilter->GetMimeType().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem FilterNamen. Dies ist der Name ohne
     vorangestellten Containernamen.
 */
/*N*/ IMPL_CONTAINER_LOOP(
/*N*/     GetFilter4FilterName, const String&,
/*N*/     pFilter->GetFilterName().CompareIgnoreCaseToAscii(aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner ClipboardID. Diese steckt im Storage.
 */
/*N*/ IMPL_CONTAINER_LOOP(
/*N*/     GetFilter4ClipBoardId, ULONG, aArg && pFilter->GetFormat() == aArg )

#ifdef MAC
#define CONDITION \
     String( aArg ).Erase( 5 ) == String( pFilter->GetTypeName() ).Erase( 5 )
#else
#define CONDITION \
     aArg == pFilter->GetTypeName()
#endif
/*   [Beschreibung]
     Ermitelt einen Filter nach seinen Extended Attributes.
     Nur auf MAC und OS/2 von Interesse.
 */
/*N*/ IMPL_CONTAINER_LOOP(
/*N*/     GetFilter4EA, const String&, CONDITION )
#undef CONDITION

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner Extension.
     (2Ah = '*')
 */
/*N*/ IMPL_CONTAINER_LOOP(
/*N*/     GetFilter4Extension, const String&,
/*N*/     pFilter->GetWildcard() != String() && pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != '*' &&
/*N*/ 	WildCard( ToUpper_Impl( pFilter->GetWildcard()() ), ';' ) == ToUpper_Impl( aArg ))


//----------------------------------------------------------------

/*N*/ const String SfxFilterContainer::GetName() const
/*N*/ {
/*N*/     return pImpl->aName;
/*N*/ }

//----------------------------------------------------------------


//----------------------------------------------------------------

/*N*/ void SfxFilterContainer::AddFilter( SfxFilter* pFilter, sal_uInt16 nPos )
/*N*/ {
/*   [Beschreibung]

     Fuegt einen Filter in einen Container ein.
 */
/*N*/     if ( !pFilter->GetFilterName().Len() ||
/*N*/ 		 !GetFilter4FilterName( pFilter->GetFilterName() ))
/*N*/ 		pImpl->aList.Insert( pFilter, nPos );
/*N*/ #if defined(DBG_UTIL)
/*N*/ 	else
/*N*/ 		delete pFilter;
/*N*/ #endif
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxFilterContainer::LoadFilters(
/*N*/     const String& rGroup,  // ConfigGruppe, aus der gelesen werden soll
/*N*/ 	sal_Bool bInstallIni,      // Falls sal_True aus install.ini lesen,
/*N*/ 	                       // sonst soffice3.ini
/*N*/ 	SfxFilterFlags nOrFlags,       // Flags zusaetzlich zu denen in der Ini
/*N*/ 	SfxFilterFlags nNotFlags )     // Flags, die nicht aus der ini genommen werden
/*   [Beschreibung]

     Stoesst das Laden eines FilterContainers an. Das eigentliche
     Laden erfolgt im LateInitHandler bzw. in ForceFilterLoad_Impl,
     falls vor LateInit auf den FilterContainer zugegriffen wird.  */
/*N*/ {
/*N*/ }

//----------------------------------------------------------------


//-------------------------------------------------------------------------

/*?*/ ULONG SfxFilterContainer::GetFilter4Content(
/*?*/     SfxMedium& rMedium, const SfxFilter** ppFilter,
/*?*/ 	SfxFilterFlags, SfxFilterFlags ) const
/*?*/ {
/*?*/     return 0;
/*?*/ }

//----------------------------------------------------------------

/*N*/ SfxFactoryFilterContainer::SfxFactoryFilterContainer(
/*N*/     const String& rName, const SfxObjectFactory& rFactP )
/*N*/     : SfxFilterContainer( rName ), rFact( rFactP ), pFunc(0)
/*   [Beschreibung]

     Im SfxFactoryFilterContainer befinden sich die Filter einer
     SfxObjectFactory. Wird kein DetectFilter registriert, so wird
     fuer die DefaultFactory <SfxApplication::DetectFilter> genommen
 */
/*N*/ {
/*N*/     //Defaults in den Container der DefaultFactory laden
/*N*/ 	SetFlags( GetFlags() | SFX_FILTER_CONTAINER_FACTORY );
/*N*/ }

//----------------------------------------------------------------

/*N*/ ULONG SfxFactoryFilterContainer::GetFilter4Content(
/*N*/     SfxMedium& rMedium, const SfxFilter** ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*N*/ {
/*N*/ 	SFX_ITEMSET_ARG( rMedium.GetItemSet(), pTargetItem, SfxStringItem,
/*N*/ 					 SID_TARGETNAME, sal_False);
/*N*/
/*N*/     if ( *ppFilter && ( (*ppFilter)->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
/*N*/ 		return 0;
/*N*/
/*N*/     if( pFunc )
/*N*/     {
/*N*/         ULONG nErr = (*pFunc)(rMedium, ppFilter, nMust, nDont);
/*N*/         DBG_ASSERT( !*ppFilter ||
/*N*/                     (((*ppFilter)->GetFilterFlags() & nMust ) == nMust &&
/*N*/                      ((*ppFilter)->GetFilterFlags() & nDont ) == 0 ),
/*N*/                     "DetectFilter Spec nicht eingehalten" );
/*N*/         return nErr;
/*N*/     }
/*
    else
    {
        DBG_ASSERT( !GetFilterCount(), "No DetectFilter function set!" );
        return 0;
    }
*/
/*N*/     return 0;
/*N*/ }

//----------------------------------------------------------------

DECLARE_LIST( SfxFContainerList_Impl, SfxFilterContainer * )//STRIP008 ;

class SfxFilterMatcher_Impl
{
public:
    sal_uInt16 nAkt;
    SfxFContainerList_Impl aList;
    sal_Bool bDeleteContainers;
};

/*?*/ SfxFilterContainer* SfxFilterMatcher::GetContainer( const String &rName ) const
/*STRIP003*/{ // DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001
/*STRIP003*/     SfxFContainerList_Impl& rList = pImpl->aList;
/*STRIP003*/     sal_uInt16 nCount = (sal_uInt16) rList.Count();
/*STRIP003*/     for( sal_uInt16 n = 0; n < nCount; n++ )
/*STRIP003*/         if( rList.GetObject( n )->GetName() == rName )
/*STRIP003*/             return rList.GetObject( n );
/*STRIP003*/     return 0;
/*?*/ }

//----------------------------------------------------------------

/*N*/ SfxFilterMatcher::SfxFilterMatcher( SfxFilterContainer* pCont)
/*N*/ {
/*N*/     pImpl = new SfxFilterMatcher_Impl;
/*N*/ 	pImpl->bDeleteContainers = sal_False;
/*N*/     AddContainer( pCont );
/*N*/ }

//----------------------------------------------------------------


/*N*/ SfxFilterMatcher::SfxFilterMatcher(sal_Bool bDeleteContainers)
/*N*/ {
/*N*/     pImpl = new SfxFilterMatcher_Impl;
/*N*/ 	pImpl->bDeleteContainers = bDeleteContainers;
/*N*/ }

//----------------------------------------------------------------

/*N*/ SfxFilterMatcher::~SfxFilterMatcher()
/*N*/ {
/*N*/ 	if ( pImpl->bDeleteContainers )
/*N*/ 	{
/*N*/ 		for ( sal_uInt32 n = pImpl->aList.Count(); n--; )
/*N*/ 		{
/*N*/ 			SfxFilterContainer *pFCont = pImpl->aList.Remove(n);
/*N*/ 			delete pFCont;
/*N*/ 		}
/*N*/ 	}
/*N*/     delete pImpl;
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxFilterMatcher::AddContainer( SfxFilterContainer* pC )
/*N*/ {
/*N*/     pImpl->aList.Insert( pC, pImpl->aList.Count() );
/*N*/ }

//----------------------------------------------------------------

/*N*/ ULONG SfxFilterMatcher::GuessFilterIgnoringContent(
/*N*/     SfxMedium& rMedium, const SfxFilter**ppFilter,
/*N*/     SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*N*/ {
/*N*/     String aFileName = rMedium.GetName();
/*N*/     ULONG nErr = ERRCODE_NONE;
/*N*/ 	const SfxFilter* pFilter = *ppFilter;
/*N*/ 	const INetURLObject& rObj = rMedium.GetURLObject();
/*N*/
/*N*/ 	if( !pFilter )
/*N*/ 		pFilter = SFX_APP()->GetFilterMatcher().GetFilter4Protocol( rMedium );
/*N*/
/*N*/     sal_Bool bCheckExternBrowser = sal_False;
/*N*/     if( !pFilter )
/*N*/ 	{
/*N*/     	// Falls Medium Remote, Zunaechst ueber Mimetypen pruefen ( nicht bei ExternBrowser, sofern dabei angeladen wird )
/*N*/     	if( !pFilter && rMedium.SupportsMIME_Impl() && ( !bCheckExternBrowser || rObj.GetProtocol() != INET_PROT_HTTP && rObj.GetProtocol() != INET_PROT_HTTPS ) )
/*N*/     	{
/*N*/         	// Mime Typen holen
/*N*/         	String aMime;
/*N*/         	nErr = rMedium.GetMIMEAndRedirect( aMime );
/*N*/         	nErr = ERRCODE_TOERROR( nErr );
/*N*/ 			if ( nErr == ERRCODE_IO_PENDING )
/*N*/ 				return nErr;
/*N*/         	else if( !nErr && aMime.Len() )
/*N*/ 			{
/*?*/ 				if ( aMime.EqualsAscii(CONTENT_TYPE_STR_X_CNT_HTTPFILE) )
/*?*/ 					// FilterDetection nur "uber den Content
/*?*/ 					return ERRCODE_NONE;
/*?*/ 				else
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pFilter = GetFilter4Mime( aMime, nMust, nDont );
/*?*/
/*?*/ 				if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_APP_OCTSTREAM) )
/*?*/ 				{
/*?*/ 					// Damit eigene Formate wie sdw auch bei falsch konfiguriertem Server erkannt werden, bevor
/*?*/ 					// wir GetInStream rufen
/*?*/ 					const SfxFilter* pMimeFilter = pFilter;
/*?*/ 	        		pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
/*?*/ 	        		if( pFilter )
/*?*/ 						nErr = ERRCODE_NONE;
/*?*/                     else
/*?*/ 						pFilter = pMimeFilter;
/*?*/ 				}
/*?*/
/*?*/ 				// Bei MIME Typen keinen Storage anfordern
/*?*/                 if( !nErr && rMedium.SupportsMIME_Impl() && pFilter && !pFilter->UsesStorage() )
/*?*/ 				{
/*?*/ 					rMedium.GetInStream();
/*?*/ 					nErr = rMedium.GetError();
/*?*/ 				}
/*N*/ 			}
/*N*/
/*N*/         	if( nErr )
/*N*/ 			{
/*?*/ 				if ( nErr == ERRCODE_SFX_CONSULTUSER )
/*?*/ 					*ppFilter = pFilter;
/*?*/             	return nErr;
/*N*/ 			}
/*N*/ 			else if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_TEXT_HTML) )
/*N*/ 			{
/*N*/ 				// MIME ist vertrauenswuerdig ( !? )
/*N*/ 				nErr = ERRCODE_SFX_NEVERCHECKCONTENT;
/*N*/ 			}
/*N*/     	}
/*N*/
/*N*/         if( !pFilter && rMedium.IsDownloadDone_Impl() )
/*N*/     	{
/*N*/         	// dann ueber Storage CLSID
/*N*/ 			// Remote macht das keinen Sinn, wenn der Download noch la"uft
/*N*/             SvStorageRef aStor = rMedium.GetStorage();
/*N*/             if ( aStor.Is() )
/*N*/                 pFilter = GetFilter4ClipBoardId( aStor->GetFormat(), nMust, nDont );
/*N*/     	}
/*N*/
/*N*/     	// Zu allerletzt ueber Extension mappen
/*N*/     	if( !pFilter )
/*N*/     	{
/*N*/         	pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
/*N*/         	if( !pFilter || pFilter->GetWildcard()==DEFINE_CONST_UNICODE("*.*") || pFilter->GetWildcard() == '*' )
/*N*/             	pFilter = 0;
/*N*/     	}
/*N*/ 	}
/*N*/
/*N*/ 	*ppFilter = pFilter;
/*N*/ 	return nErr;
/*N*/ }

//----------------------------------------------------------------

#define CHECKERROR()											\
if( nErr == 1 || nErr == USHRT_MAX || nErr == ULONG_MAX )		\
{																\
    ByteString aText = "Fehler in FilterDetection: Returnwert ";\
    aText += ByteString::CreateFromInt32(nErr);					\
    if( pFilter )												\
    {															\
        aText += ' ';											\
        aText += ByteString(U2S(pFilter->GetFilterName()));     \
    }															\
    DBG_ERROR( aText.GetBuffer() );								\
    nErr = ERRCODE_ABORT;										\
}

//----------------------------------------------------------------

/*N*/ ULONG SfxFilterMatcher::GuessFilter(
/*N*/     SfxMedium& rMedium, const SfxFilter**ppFilter,
/*N*/     SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*N*/ {
/*N*/     const SfxFilter* pOldFilter = *ppFilter;
/*N*/     const SfxFilter* pFilter = pOldFilter;
/*N*/
/*N*/ 	sal_Bool bConsultUser = sal_False;
/*N*/ 	sal_Bool bSupportsMime = rMedium.SupportsMIME_Impl();
/*N*/
/*N*/ 	// Zunaechst, falls Filter mitkommt einmal testen, ob dieser in Ordnung ist.
/*N*/ 	ErrCode nErr = ERRCODE_NONE;
/*N*/     if( pFilter && ( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY ) )
/*N*/ 	{
/*?*/         rMedium.StartDownload();
/*?*/ 		if ( !rMedium.IsDownloadDone_Impl() )
/*?*/ 		{
/*?*/ 			if ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC )
/*?*/ 			{
/*?*/ 				// kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
/*?*/ 				// zu detecten
/*?*/ 				rMedium.ForceSynchronStream_Impl( sal_True );
/*?*/ 				if ( !rMedium.GetInStream() )
/*?*/ 				{
/*?*/ 					ErrCode e = rMedium.GetErrorCode();
/*?*/ 					if ( e == ERRCODE_NONE )
/*?*/ 					{
/*?*/ 						*ppFilter = pFilter;
/*?*/ 						return ERRCODE_IO_PENDING;
/*?*/ 					}
/*?*/ 					else
/*?*/ 						return e;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 			 	*ppFilter = pFilter;
/*?*/ 				return ERRCODE_IO_PENDING;
/*?*/ 			}
/*?*/ 		}
/*?*/
/*?*/ 		if( bSupportsMime && !pFilter->UsesStorage() )
/*?*/ 			rMedium.GetInStream();
/*?*/
/*?*/     	nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
/*?*/ 		CHECKERROR();
/*?*/
/*?*/ 		rMedium.ForceSynchronStream_Impl( sal_False );
/*?*/
/*?*/ 		// ABORT bedeutet Filter ungueltig
/*?*/ 		if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
/*?*/ 			return nErr;
/*?*/
/*?*/ 		// War der Filter ungueltig oder wurde ein anderer zurueckgegeben,
/*?*/ 		// so detecten wir selbst (wg. redirection)
/*?*/ 		if( nErr == ERRCODE_ABORT && pFilter )
/*?*/ 			bConsultUser = sal_True;
/*?*/
/*?*/ 		if( nErr != ERRCODE_SFX_FORCEQUIET && pOldFilter && pFilter != pOldFilter )
/*N*/ 			pFilter = 0;
/*N*/ 	}
/*N*/
/*N*/ 	if( !pFilter )
/*N*/ 	{
/*N*/ 		bConsultUser = sal_False;
/*N*/ //DV !!!! don't close InStream when using the new medium
/*N*/ //		rMedium.CloseInStream();
/*N*/
/*N*/ 		// Als erstes Protocol, MIME-Type, Extension etc. probieren
/*N*/ 		nErr = GuessFilterIgnoringContent( rMedium, &pFilter, nMust, nDont );
/*N*/ 		if ( nErr == ERRCODE_IO_PENDING )
/*N*/ 		{
/*?*/ 			*ppFilter = pFilter;
/*?*/ 			return nErr;
/*N*/ 		}
/*N*/
/*N*/ 		if ( pFilter && nErr == ERRCODE_SFX_CONSULTUSER )
/*?*/ 			*ppFilter = pFilter;
/*N*/
/*N*/ 		if( nErr && nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET && nErr != ERRCODE_SFX_NEVERCHECKCONTENT )
/*N*/ 			return nErr;
/*N*/
/*N*/ 		if( nErr == ERRCODE_ABORT )
/*N*/ 			pFilter = 0;
/*N*/
/*N*/ 		// Jetzt wird geprueft, ob das Modul auch einverstanden ist; ist das nicht der Fall, wird auf
/*N*/ 		// jeden Fall auf ConsultUser umgeschaltet
/*N*/         if( pFilter )
/*N*/ 		{
/*N*/ 			if( nErr == ERRCODE_SFX_NEVERCHECKCONTENT )
/*N*/ 				nErr = ERRCODE_NONE;
/*N*/ 			else if( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY )
/*N*/ 			{
/*N*/         		rMedium.StartDownload();
/*N*/ 				if ( !rMedium.IsDownloadDone_Impl() )
/*N*/ 				{
/*?*/ 					if ( !pFilter->UsesStorage() && rMedium.GetInStream() && ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC ) )
/*?*/ 						// kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
/*?*/ 						// zu detecten
/*?*/ 						rMedium.ForceSynchronStream_Impl( sal_True );
/*?*/ 					else
/*?*/ 					{
/*?*/ 						*ppFilter = pFilter;
/*?*/ 						return ERRCODE_IO_PENDING;
/*?*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				const SfxFilter* pTmpFilter = pFilter;
/*N*/ 				nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
/*N*/ 				CHECKERROR();
/*N*/
/*N*/ 				rMedium.ForceSynchronStream_Impl( sal_False );
/*N*/
/*N*/ 				// ABORT bedeutet Filter ungueltig
/*N*/ 				if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
/*N*/ 	 				return nErr;
/*N*/
/*N*/ 				if( nErr == ERRCODE_ABORT && pFilter )
/*N*/ 					pFilter = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// Jetzt einmal drueberiterieren und nur die perfekten Filter nehmen
/*N*/ 	if( !pFilter )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/
/*N*/ 	// Letzte Moeglichkeit ist die Befragung aller ObjectFactories.
/*N*/ 	if( !pFilter )
/*N*/ 	{
/*N*/ 		// Achtung: hier k"onnte auch asynchron detected werden!
/*?*/ 		if ( !rMedium.IsDownloadDone_Impl() )
/*?*/ 			return ERRCODE_IO_PENDING;
/*?*/
/*?*/ //DV !!!! don't close InStream when using the new Medium
/*?*/ //rMedium.CloseInStream();
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/
/*N*/ 	*ppFilter = pFilter;
/*N*/
/*N*/ 	if ( ( ( pOldFilter && pOldFilter!=pFilter &&
/*N*/ 			( !pOldFilter->IsOwnTemplateFormat() || !pFilter->IsOwnFormat() ) ) ) &&
/*N*/ 		 	nErr != ERRCODE_SFX_FORCEQUIET || bConsultUser )
/*N*/         return ERRCODE_SFX_CONSULTUSER;
/*N*/
/*N*/     if( !pOldFilter )
/*N*/ 		pOldFilter = pFilter;
/*N*/
/*N*/ 	// Checken, ob Daten vorliegen. Dies verhindert Fehler, die in
/*N*/ 	// GetFilter4Content auftreten und die Auswahlbox hochbringen.
/*N*/ 	ErrCode nMediumError = rMedium.GetErrorCode();
/*N*/ 	if( nMediumError )
/*N*/ 		return nMediumError;
/*N*/
/*N*/ 	*ppFilter = pFilter;
/*N*/     if ( ( nErr ||
/*N*/           ( pOldFilter && pOldFilter!=pFilter
/*N*/ 			&& ( !pOldFilter->IsOwnTemplateFormat() ||
/*N*/ 				 !pFilter->IsOwnFormat() ) ))
/*N*/          	&& nErr != ERRCODE_SFX_FORCEQUIET )
/*N*/         nErr = ERRCODE_SFX_CONSULTUSER;
/*N*/
/*N*/     if( nErr == ERRCODE_SFX_FORCEQUIET )
/*N*/         nErr = 0;
/*N*/     return nErr;
/*N*/ };

//----------------------------------------------------------------



//----------------------------------------------------------------


//----------------------------------------------------------------

/*STRIP003*/ ULONG SfxFilterMatcher::GetFilter4Content(
/*STRIP003*/     SfxMedium& rMedium, const SfxFilter** ppFilter,  SfxFilterFlags nMust, SfxFilterFlags nDont, sal_Bool bOnlyGoodOnes ) const
/*STRIP003*/ {
/*STRIP003*/     ULONG nErr = ERRCODE_NONE;
/*STRIP003*/     SfxFContainerList_Impl& rList = pImpl->aList;
/*STRIP003*/     sal_uInt16 nCount = (sal_uInt16)rList.Count();
/*STRIP003*/     for( sal_uInt16 n = 0; n<nCount; n++ )
/*STRIP003*/     {
/*STRIP003*/         const SfxFilter* pFilter = 0;
/*STRIP003*/         nErr = rList.GetObject( n )->GetFilter4Content(
/*STRIP003*/             rMedium, &pFilter, nMust, nDont );
/*STRIP003*/ 		CHECKERROR();
/*STRIP003*/         if( rMedium.GetError() != ERRCODE_NONE )
/*STRIP003*/         {
/*STRIP003*/             pFilter = 0;
/*STRIP003*/             return rMedium.GetError();
/*STRIP003*/         }
/*STRIP003*/ 		if( nErr == ERRCODE_ABORT && bOnlyGoodOnes ) pFilter = 0;
/*STRIP003*/         if( pFilter )
/*STRIP003*/         {
/*STRIP003*/             *ppFilter = pFilter;
/*STRIP003*/             return nErr;
/*STRIP003*/         }
/*STRIP003*/     }
/*STRIP003*/     return 0;
/*STRIP003*/ }

#define IMPL_LOOP( Type, ArgType )                              \
const SfxFilter* SfxFilterMatcher::Type(                        \
    ArgType rStr, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{                                                               \
    const SfxFilter* pFirstFilter = 0;                          \
    SfxFContainerList_Impl& rList = pImpl->aList;               \
    sal_uInt16 nCount = (sal_uInt16)rList.Count();              \
    for( sal_uInt16 n = 0; n<nCount; n++ )                      \
    {                                                           \
        const SfxFilter* pFilter =                              \
            rList.GetObject( n )->Type(rStr, nMust, nDont );    \
        if( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED ) ) \
            return pFilter;                                     \
        else if ( !pFirstFilter )                               \
            pFirstFilter = pFilter;                             \
    }                                                           \
    return pFirstFilter;                                        \
}

/*N*/ IMPL_LOOP( GetFilter4Extension, const String& )
/*N*/ IMPL_LOOP( GetFilter4Protocol, SfxMedium& )
/*N*/ IMPL_LOOP( GetFilter4ClipBoardId, ULONG )
/*N*/ IMPL_LOOP( GetFilter4FilterName, const String& )

//----------------------------------------------------------------


//----------------------------------------------------------------

/*?*/ IMPL_STATIC_LINK( SfxFilterMatcher, MaybeFileHdl_Impl, String*, pString )
/*?*/ {
/*N*/ 	const SfxFilter* pFilter =
/*N*/ 		pThis->GetFilter4Extension( *pString, SFX_FILTER_IMPORT );
/*N*/ 	if( pFilter && !pFilter->GetWildcard().Matches( String() ) &&
/*N*/ 		pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != '*' )
/*N*/ 		return sal_True;
/*N*/ 	return sal_False;
/*?*/ }

/*---------------------------------------------------------------
    helper to build own formated string from given stringlist by
    using given seperator
  ---------------------------------------------------------------*/
/*N*/ ::rtl::OUString implc_convertStringlistToString( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& lList     ,
/*N*/                                                  const sal_Unicode&                                        cSeperator,
/*N*/                                                  const ::rtl::OUString&                                    sPrefix   )
/*N*/ {
/*N*/     ::rtl::OUStringBuffer   sString ( 1000 )           ;
/*N*/     sal_Int32               nCount  = lList.getLength();
/*N*/     sal_Int32               nItem   = 0                ;
/*N*/     for( nItem=0; nItem<nCount; ++nItem )
/*N*/     {
/*N*/         if( sPrefix.getLength() > 0 )
/*N*/         {
/*N*/             sString.append( sPrefix );
/*N*/         }
/*N*/         sString.append( lList[nItem] );
/*N*/         if( nItem+1<nCount )
/*N*/         {
/*N*/             sString.append( cSeperator );
/*N*/         }
/*N*/     }
/*N*/     return sString.makeStringAndClear();
/*N*/ }

// com.sun.star.sheet.SpreadsheetDocument
#define NNUMFILTERNAMESFORSPREADSHEETDOCUMENT (8)
static const sal_Char* sFilterNamesForSpreadsheetDocument[NNUMFILTERNAMESFORSPREADSHEETDOCUMENT] =
{
    "StarOffice XML (Calc)",
    "StarCalc 1.0",
    "StarCalc 3.0",
    "StarCalc 3.0 Vorlage/Template",
    "StarCalc 4.0",
    "StarCalc 4.0 Vorlage/Template",
    "StarCalc 5.0",
    "StarCalc 5.0 Vorlage/Template"
};

// com.sun.star.chart.ChartDocument
#define SFILTERNAMESFORCHARTDOCUMENT (4)
static const sal_Char* sFilterNamesForChartDocument[SFILTERNAMESFORCHARTDOCUMENT] =
{
    "StarOffice XML (Chart)",
    "StarChart 3.0",
    "StarChart 4.0",
    "StarChart 5.0"
};

// com.sun.star.drawing.DrawingDocument
#define SFILTERNAMESFORDRAWINGDOCUMENT (5)
static const sal_Char* sFilterNamesForDrawingDocument[SFILTERNAMESFORDRAWINGDOCUMENT] =
{
    "StarOffice XML (Draw)",
    "StarDraw 3.0",
    "StarDraw 3.0 Vorlage",
    "StarDraw 5.0",
    "StarDraw 5.0 Vorlage"
};

// com.sun.star.presentation.PresentationDocument
#define SFILTERNAMESFORPRESENTATIONDOCUMENT (10)
static const sal_Char* sFilterNamesForPresentationDocument[SFILTERNAMESFORPRESENTATIONDOCUMENT] =
{
    "StarOffice XML (Impress)",
    "StarDraw 3.0 (StarImpress)",
    "StarDraw 3.0 Vorlage (StarImpress)",
    "StarDraw 5.0 (StarImpress)",
    "StarDraw 5.0 Vorlage (StarImpress)",
    "StarImpress 4.0",
    "StarImpress 4.0 Vorlage",
    "StarImpress 5.0",
    "StarImpress 5.0 (packed)",
    "StarImpress 5.0 Vorlage"
};

// com.sun.star.formula.FormulaProperties
#define SFILTERNAMESFORFORMULAPROPERTIES (5)
static const sal_Char* sFilterNamesForFormulaProperties[SFILTERNAMESFORFORMULAPROPERTIES] =
{
    "StarOffice XML (Math)",
    "StarMath 2.0",
    "StarMath 3.0",
    "StarMath 4.0",
    "StarMath 5.0"
};

// com.sun.star.text.GlobalDocument
#define SFILTERNAMESFORGLOBALDOCUMENT (6)
static const sal_Char* sFilterNamesForGlobalDocument[SFILTERNAMESFORGLOBALDOCUMENT] =
{
    "StarOffice XML (Writer)",
    "StarWriter 3.0 (StarWriter/GlobalDocument)",
    "StarWriter 4.0 (StarWriter/GlobalDocument)",
    "StarWriter 4.0/GlobalDocument",
    "StarWriter 5.0 (StarWriter/GlobalDocument)",
    "StarWriter 5.0/GlobalDocument"
};
// com.sun.star.text.WebDocument
#define SFILTERNAMESFORWEBDOCUMENT 6
static const sal_Char* sFilterNamesForWebDocument[SFILTERNAMESFORWEBDOCUMENT] =
{
    "StarOffice XML (Writer)",
    "StarWriter 3.0 (StarWriter/Web)",
    "StarWriter/Web 4.0 Vorlage/Template",
    "StarWriter 4.0 (StarWriter/Web)",
    "StarWriter/Web 5.0 Vorlage/Template",
    "StarWriter 5.0 (StarWriter/Web)"
};
// com.sun.star.text.TextDocument
#define SFILTERNAMESFORTEXTDOCUMENT (123)
static const sal_Char* sFilterNamesForTextDocument[SFILTERNAMESFORTEXTDOCUMENT] =
{
    "StarOffice XML (Writer)",
    "StarWriter DOS",
    "Lotus 1-2-3 1.0 (DOS) (StarWriter)",
    "Lotus 1-2-3 1.0 (WIN) (StarWriter)",
    "MS Excel 4.0 (StarWriter)",
    "MS Excel 5.0 (StarWriter)",
    "MS Excel 95 (StarWriter)",
    "StarWriter 1.0",
    "StarWriter 2.0",
    "StarWriter 3.0",
    "StarWriter 3.0 Vorlage/Template",
    "StarWriter 4.0",
    "StarWriter 4.0 Vorlage/Template",
    "StarWriter 5.0",
    "StarWriter 5.0 Vorlage/Template",
    "Ami Pro 1.x-3.1 (W4W)",
    "CTOS DEF (W4W)",
    "Claris Works (W4W)",
    "DCA Revisable Form Text (W4W)",
    "DCA with Display Write 5 (W4W)",
    "DCA/FFT-Final Form Text (W4W)",
    "DEC DX (W4W)",
    "DEC WPS-PLUS (W4W)",
    "DataGeneral CEO Write (W4W)",
    "DisplayWrite 2.0-4.x (W4W)",
    "DisplayWrite 5.x (W4W)",
    "EBCDIC (W4W)",
    "Enable (W4W)",
    "Frame Maker MIF 3.0 (W4W)",
    "Frame Maker MIF 4.0 (W4W)",
    "Frame Maker MIF 5.0 (W4W)",
    "Frame Work III (W4W)",
    "Frame Work IV  (W4W)",
    "HP AdvanceWrite Plus (W4W)",
    "ICL Office Power 6 (W4W)",
    "ICL Office Power 7 (W4W)",
    "Interleaf (W4W)",
    "Interleaf 5 - 6 (W4W)",
    "Legacy Winstar onGO (W4W)",
    "Lotus Manuscript (W4W)",
    "MASS 11 Rel. 8.0-8.3 (W4W)",
    "MASS 11 Rel. 8.5-9.0 (W4W)",
    "MS MacWord 3.0 (W4W)",
    "MS MacWord 4.0 (W4W)",
    "MS MacWord 5.x (W4W)",
    "MS WinWord 1.x (W4W)",
    "MS WinWord 2.x (W4W)",
    "MS Word 3.x (W4W)",
    "MS Word 4.x (W4W)",
    "MS Word 5.x (W4W)",
    "MS Word 6.x (W4W)",
    "MS Works 2.0 DOS (W4W)",
    "MS Works 3.0 Win (W4W)",
    "MS Works 4.0 Mac (W4W)",
    "Mac Write 4.x 5.0 (W4W)",
    "Mac Write II (W4W)",
    "Mac Write Pro (W4W)",
    "MultiMate 3.3 (W4W)",
    "MultiMate 4 (W4W)",
    "MultiMate Adv. 3.6 (W4W)",
    "MultiMate Adv. II 3.7 (W4W)",
    "NAVY DIF (W4W)",
    "OfficeWriter 4.0 (W4W)",
    "OfficeWriter 5.0 (W4W)",
    "OfficeWriter 6.x (W4W)",
    "PFS First Choice 1.0 (W4W)",
    "PFS First Choice 2.0 (W4W)",
    "PFS First Choice 3.0 (W4W)",
    "PFS Write (W4W)",
    "Peach Text (W4W)",
    "Professional Write 1.0 (W4W)",
    "Professional Write 2.x (W4W)",
    "Professional Write Plus (W4W)",
    "Q&A Write 1.0-3.0 (W4W)",
    "Q&A Write 4.0 (W4W)",
    "Rapid File 1.0 (W4W)",
    "Rapid File 1.2 (W4W)",
    "Samna Word IV-IV Plus (W4W)",
    "Total Word (W4W)",
    "Uniplex V7-V8 (W4W)",
    "Uniplex onGO (W4W)",
    "VolksWriter 3 and 4 (W4W)",
    "VolksWriter Deluxe (W4W)",
    "WITA (W4W)",
    "Wang II SWP (W4W)",
    "Wang PC (W4W)",
    "Wang WP Plus (W4W)",
    "Win Write 3.x (W4W)",
    "WiziWord 3.0 (W4W)",
    "WordPerfect (Win) 5.1-5.2 (W4W)",
    "WordPerfect (Win) 6.0 (W4W)",
    "WordPerfect (Win) 6.1 (W4W)",
    "WordPerfect (Win) 7.0 (W4W)",
    "WordPerfect 4.1 (W4W)",
    "WordPerfect 4.2 (W4W)",
    "WordPerfect 5.0 (W4W)",
    "WordPerfect 5.1 (W4W)",
    "WordPerfect 6.0 (W4W)",
    "WordPerfect 6.1 (W4W)",
    "WordPerfect Mac 1 (W4W)",
    "WordPerfect Mac 2 (W4W)",
    "WordPerfect Mac 3 (W4W)",
    "WordStar (Win) 1.x-2.0 (W4W)",
    "WordStar 2000 Rel. 3.0 (W4W)",
    "WordStar 2000 Rel. 3.5 (W4W)",
    "WordStar 3.3x (W4W)",
    "WordStar 3.45 (W4W)",
    "WordStar 4.0  (W4W)",
    "WordStar 5.0  (W4W)",
    "WordStar 5.5  (W4W)",
    "WordStar 6.0  (W4W)",
    "WordStar 7.0  (W4W)",
    "WriteNow 3.0 (Macintosh) (W4W)",
    "Writing Assistant (W4W)",
    "XEROX XIF 5.0 (Illustrator) (W4W)",
    "XEROX XIF 5.0 (W4W)",
    "XEROX XIF 6.0 (Color Bitmap) (W4W)",
    "XEROX XIF 6.0 (Res Graphic) (W4W)",
    "XyWrite (Win) 1.0 (W4W)",
    "XyWrite III ( W4W)",
    "XyWrite III+ ( W4W)",
    "XyWrite IV (W4W)",
    "XyWrite Sig. (Win) (W4W)"
};

/*N*/ void SfxFilterContainer::ReadExternalFilters( const String& rDocServiceName )
/*N*/ {
/*N*/     RTL_LOGFILE_CONTEXT( aMeasure, "sfx2 (as96863) ::SfxFilterContainer::ReadExternalFilters" );
/*N*/
/*N*/     try
/*N*/     {
/*N*/         // get the FilterFactory service to access the registered filters ... and types!
/*N*/         ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager  =::legacy_binfilters::getLegacyProcessServiceFactory();//STRIP002 ::comphelper::getProcessServiceFactory();
/*N*/         ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xFilterCFG                                                ;
/*N*/         ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xTypeCFG                                                  ;
/*N*/         if( xServiceManager.is() == sal_True )
/*N*/         {
/*N*/             xFilterCFG = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );
/*N*/             xTypeCFG   = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );
/*N*/         }
/*N*/
/*N*/         if(
/*N*/             ( xFilterCFG.is() == sal_True ) &&
/*N*/             ( xTypeCFG.is()   == sal_True )
/*N*/           )
/*N*/         {
/*N*/             // select right query to get right set of filters for search modul
///*N*/             ::rtl::OUString sQuery;
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.text.TextDocument"                ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_writer:sort_prop=uiname:use_order:default_first"    ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.text.WebDocument"                 ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_web:sort_prop=uiname:use_order:default_first"       ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.text.GlobalDocument"              ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_global:sort_prop=uiname:use_order:default_first"    ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.chart.ChartDocument"              ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_chart:sort_prop=uiname:use_order:default_first"     ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.sheet.SpreadsheetDocument"        ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_calc:sort_prop=uiname:use_order:default_first"      ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.presentation.PresentationDocument") == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_impress:sort_prop=uiname:use_order:default_first"   ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.drawing.DrawingDocument"          ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_draw:sort_prop=uiname:use_order:default_first"      ); else
///*N*/             if( rDocServiceName.EqualsAscii("com.sun.star.formula.FormulaProperties"        ) == TRUE ) sQuery = DEFINE_CONST_UNICODE("_query_math:sort_prop=uiname:use_order:default_first"      );
///*N*/
///*N*/             DBG_ASSERT( !(sQuery.getLength()<1), "SfxFilterContainer::ReadExternalFilters()\nCouldn't find right filter query for given modul! Filters will be ignored ...\n" );

                // select right query to get right set of filters for search modul
                ::std::vector< ::rtl::OUString > aFilterNameList;

                if(rDocServiceName.EqualsAscii("com.sun.star.text.TextDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORTEXTDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForTextDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.text.WebDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORWEBDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForWebDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.text.GlobalDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORGLOBALDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForGlobalDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.chart.ChartDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORCHARTDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForChartDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.sheet.SpreadsheetDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < NNUMFILTERNAMESFORSPREADSHEETDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForSpreadsheetDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.presentation.PresentationDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORPRESENTATIONDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForPresentationDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.drawing.DrawingDocument") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORDRAWINGDOCUMENT; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForDrawingDocument[a] ));
                }
                else if(rDocServiceName.EqualsAscii("com.sun.star.formula.FormulaProperties") == TRUE)
                {
                    for(sal_uInt32 a(0L); a < SFILTERNAMESFORFORMULAPROPERTIES; a++)
                        aFilterNameList.push_back( OUString::createFromAscii( sFilterNamesForFormulaProperties[a] ));
                }

/*N*/             if( aFilterNameList.size() ) //sQuery.getLength() > 0 )
/*N*/             {
/*N*/                 // get all internal filter names, which are match given doc service name (modul)
/*N*/                 //::com::sun::star::uno::Sequence< ::rtl::OUString > lFilterNames;
/*N*/                 ::com::sun::star::uno::Any aResult;
/*N*/
/*N*/                 //aResult = xFilterCFG->getByName( sQuery );
///*N*/                 if(sFilterNameArray) //  aResult >>= lFilterNames )
///*N*/                 {
/*N*/                     // If list of filters already exist ...
/*N*/                     // ReadExternalFilters must work in update mode.
/*N*/                     // Best way seams to mark all filters NOT_INSTALLED
/*N*/                     // and change it back for all valid filters afterwards.
///*N*/                     BOOL bUpdated = FALSE;
///*N*/                     if( pImpl->aList.Count() > 0 )
///*N*/                     {
///*?*/                         bUpdated = TRUE;
///*?*/                         SfxFilterList_Impl& rList = pImpl->aList;
///*?*/                         USHORT nCount = (USHORT)rList.Count();
///*?*/                         SfxFilter* pFilter;
///*?*/                         for (USHORT f=0; f<nCount; ++f)
///*?*/                         {
///*?*/ 							pFilter = NULL;
///*?*/                             pFilter = rList.GetObject(f);
///*?*/                             pFilter->nFormatType |= SFX_FILTER_NOTINSTALLED;
///*?*/                         }
///*N*/                     }
/*N*/
/*N*/                     // get all properties of filters ... put it into the filter container
/*N*/                     ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lFilterProperties                           ;
///*N*/                     sal_Int32                                                                 nFilterCount      = lFilterNames.getLength();
///*N*/                     sal_Int32                                                                 nFilter           = 0                       ;
/*N*/
/*N*/                     for(sal_uInt32 nFilter(0L); nFilter < aFilterNameList.size(); nFilter++) // nFilter=0; nFilter<nFilterCount; ++nFilter )
/*N*/                     {
/*N*/                         // Try to get filter .. but look for any exceptions!
/*N*/                         // May be filter was deleted by another thread ...
/*N*/                         ::rtl::OUString sFilterName(aFilterNameList[nFilter]); //::rtl::OUString sFilterName = lFilterNames[nFilter];
/*N*/                         try
/*N*/                         {
/*N*/                             aResult = xFilterCFG->getByName( sFilterName );
/*N*/                         }
/*N*/                         catch( ::com::sun::star::container::NoSuchElementException& )
/*N*/                         {
/*?*/                             aResult = ::com::sun::star::uno::Any();
/*N*/                         }
/*N*/
/*N*/                         if( aResult >>= lFilterProperties )
/*N*/                         {
/*N*/                             // collect informations to add filter to container
/*N*/                             // (attention: some informations aren't available on filter directly ... you must search for corresponding type too!)
/*N*/                             sal_Int32       nFlags          = 0 ;
/*N*/                             sal_Int32       nClipboardId    = 0 ;
/*N*/                             sal_Int32       nDocumentIconId = 0 ;
/*N*/                             sal_Int32       nFormatVersion  = 0 ;
/*N*/                             ::rtl::OUString sMimeType           ;
/*N*/                             ::rtl::OUString sType               ;
/*N*/                             ::rtl::OUString sUIName             ;
/*N*/                             ::rtl::OUString sHumanName          ;
/*N*/                             ::rtl::OUString sDefaultTemplate    ;
/*N*/                             ::rtl::OUString sUserData           ;
/*N*/                             ::rtl::OUString sExtension          ;
/*N*/
/*N*/                             // first get directly available properties
/*N*/                             sal_Int32 nFilterPropertyCount = lFilterProperties.getLength();
/*N*/                             sal_Int32 nFilterProperty      = 0                            ;
/*N*/                             for( nFilterProperty=0; nFilterProperty<nFilterPropertyCount; ++nFilterProperty )
/*N*/                             {
/*N*/                                 if( lFilterProperties[nFilterProperty].Name.compareToAscii( "FileFormatVersion" ) == 0 )
/*N*/                                 {
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= nFormatVersion;
/*N*/                                 }
/*N*/                                 else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "TemplateName" ) == 0 )
/*N*/                                 {
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= sDefaultTemplate;
/*N*/                                 }
/*N*/                                 else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "Flags" ) == 0 )
/*N*/                                 {
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= nFlags;
/*N*/                                 }
/*N*/                                 else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "UIName" ) == 0 )
/*N*/                                 {
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= sUIName;
/*N*/                                 }
/*N*/                                 else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "UserData" ) == 0 )
/*N*/                                 {
/*N*/                                     ::com::sun::star::uno::Sequence< ::rtl::OUString > lUserData;
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= lUserData;
/*N*/                                     sUserData = implc_convertStringlistToString( lUserData, ',', ::rtl::OUString() );
/*N*/                                 }
/*N*/                                 else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "Type" ) == 0 )
/*N*/                                 {
/*N*/                                     lFilterProperties[nFilterProperty].Value >>= sType;
/*N*/                                     // Try to get filter .. but look for any exceptions!
/*N*/                                     // May be filter was deleted by another thread ...
/*N*/                                     try
/*N*/                                     {
/*N*/                                         aResult = xTypeCFG->getByName( sType );
/*N*/                                     }
/*N*/                                     catch( ::com::sun::star::container::NoSuchElementException& )
/*N*/                                     {
/*?*/                                         aResult = ::com::sun::star::uno::Any();
/*N*/                                     }
/*N*/
/*N*/                                     ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lTypeProperties;
/*N*/                                     if( aResult >>= lTypeProperties )
/*N*/                                     {
/*N*/                                         // get indirect available properties then (types)
/*N*/                                         sal_Int32 nTypePropertyCount = lTypeProperties.getLength();
/*N*/                                         sal_Int32 nTypeProperty      = 0                          ;
/*N*/                                         for( nTypeProperty=0; nTypeProperty<nTypePropertyCount; ++nTypeProperty )
/*N*/                                         {
/*N*/                                             if( lTypeProperties[nTypeProperty].Name.compareToAscii( "ClipboardFormat" ) == 0 )
/*N*/                                             {
/*N*/                                                 lTypeProperties[nTypeProperty].Value >>= sHumanName;
/*N*/                                             }
/*N*/                                             else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "DocumentIconID" ) == 0 )
/*N*/                                             {
/*N*/                                                 lTypeProperties[nTypeProperty].Value >>= nDocumentIconId;
/*N*/                                             }
/*N*/                                             else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "MediaType" ) == 0 )
/*N*/                                             {
/*N*/                                                 lTypeProperties[nTypeProperty].Value >>= sMimeType;
/*N*/                                             }
/*N*/                                             else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "Extensions" ) == 0 )
/*N*/                                             {
/*N*/                                                 ::com::sun::star::uno::Sequence< ::rtl::OUString > lExtensions;
/*N*/                                                 lTypeProperties[nTypeProperty].Value >>= lExtensions;
/*N*/                                                 sExtension = implc_convertStringlistToString( lExtensions, ';', DEFINE_CONST_UNICODE("*.") );
/*N*/                                             }
/*N*/                                             else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "URLPattern" ) == 0 )
/*N*/                                             {
/*N*/                                                 lTypeProperties[nTypeProperty].Value >>= nFormatVersion;
/*N*/                                             }
/*N*/                                         }
/*N*/                                     }
/*N*/                                 }
/*N*/                             }
/*N*/                             // old formats are found ... using HumanPresentableName!
/*N*/                             if( sHumanName.getLength() )
/*N*/                             {
/*N*/                                 nClipboardId = SotExchange::RegisterFormatName( sHumanName );
/*N*/
//STRIP007 /*N*/ 			/*remove the bugid 100570*/					// #100570# For external filters ignore clipboard IDs
//STRIP007 /*N*/ 								if((nFlags & SFX_FILTER_STARONEFILTER) == SFX_FILTER_STARONEFILTER)
//STRIP007 /*N*/ 								{
//STRIP007 /*?*/ 									nClipboardId = 0;
//STRIP007 /*N*/ 								}
/*N*/                             }
/*N*/                             // register SfxFilter
/*N*/                             // first erase module name from old filter names!
/*N*/                             // e.g: "scalc: DIF" => "DIF"
/*N*/                             sal_Int32 nStartRealName = sFilterName.indexOf( DEFINE_CONST_UNICODE(": "), 0 );
/*N*/                             if( nStartRealName != -1 )
/*N*/                             {
/*?*/                                 DBG_ERROR("Old format, not supported!");
/*?*/                                 sFilterName = sFilterName.copy( nStartRealName+2 );
/*N*/                             }
/*N*/
/*N*/                             USHORT nCachePos = 0;
/*N*/                             if (!((nFlags & SFX_FILTER_DEFAULT) == SFX_FILTER_DEFAULT))
/*N*/                                 nCachePos = GetFilterCount();
/*N*/
/*N*/                             SfxFilter* pFilter = this->GetFilter4FilterName(sFilterName,0,0);
/*N*/                             const SfxFilter* pCheck = this->GetFilter4FilterName(sFilterName,0,0);
/*N*/                             BOOL bNew = FALSE;
/*N*/                             if (!pFilter)
/*N*/                             {
                                      if ( nFormatVersion == 6200 )
                                          nFlags = nFlags - 1; // only export!

/*N*/                                 bNew = TRUE;
/*N*/                                 pFilter = new SfxFilter( sFilterName             ,
/*N*/                                                          sExtension              ,
/*N*/                                                          nFlags                  ,
/*N*/                                                          nClipboardId            ,
/*N*/                                                          sType                   ,
/*N*/                                                          sType                   ,
/*N*/                                                          (USHORT)nDocumentIconId ,
/*N*/                                                          sMimeType               ,
/*N*/                                                          this                    ,
/*N*/                                                          sUserData               );
/*N*/                             }
/*N*/                             else
/*N*/                             {
/*?*/                                 pFilter->aName        = sFilterName;
/*?*/                                 pFilter->aWildCard    = WildCard(sExtension, ';');
/*?*/                                 pFilter->nFormatType  = nFlags;
/*?*/                                 pFilter->lFormat      = nClipboardId;
/*?*/                                 pFilter->aMacType     = sType;
/*?*/                                 pFilter->aTypeName    = sType;
/*?*/                                 pFilter->nDocIcon     = (USHORT)nDocumentIconId;
/*?*/                                 pFilter->aMimeType    = sMimeType;
/*?*/                                 pFilter->pContainer   = this;
/*?*/                                 pFilter->aUserData    = sUserData;
/*?*/                             }

/*N*/
/*N*/                             // Don't forget to set right UIName!
/*N*/                             // Otherwise internal name is used as fallback ...
/*N*/                             pFilter->SetUIName( sUIName );
/*N*/                             pFilter->SetDefaultTemplate( sDefaultTemplate );
/*N*/                             if( nFormatVersion )
/*N*/                             {
/*N*/                                 pFilter->SetVersion( nFormatVersion );
/*N*/                             }

/*N*/                             if (bNew)
/*N*/                                 AddFilter(pFilter, nCachePos);
/*N*/                         }
/*N*/                     }
/*N*/
///*N*/                     // In case we updated an already filled cache, it was to complicated to
///*N*/                     // look for right place of the default filter!
///*N*/                     // It seams to be easiear to step over the whole container twice and
///*N*/                     // correct it now ...
///*N*/                     if (bUpdated)
///*N*/                     {
///*?*/                         SfxFilterList_Impl& rList       = pImpl->aList;
///*?*/                         SfxFilter*          pOldDefault = rList.First();
///*?*/                         SfxFilter*          pNewDefault = NULL         ;
///*?*/                         sal_Int32           nNewPos     = 0            ;
///*?*/
///*?*/                         if ((pOldDefault->nFormatType & SFX_FILTER_DEFAULT) != SFX_FILTER_DEFAULT)
///*?*/                         {
///*?*/                             USHORT nCount = (USHORT)rList.Count();
///*?*/                             SfxFilter* pFilter;
///*?*/                             for (USHORT f=0; f<nCount; ++f)
///*?*/                             {
///*?*/                                 pFilter = NULL;
///*?*/                                 pFilter = rList.GetObject(f);
///*?*/                                 if ((pFilter->nFormatType & SFX_FILTER_DEFAULT) == SFX_FILTER_DEFAULT)
///*?*/                                 {
///*?*/                                     pNewDefault = pFilter;
///*?*/                                     nNewPos     = f;
///*?*/                                     break;
///*?*/                                 }
///*?*/                             }
///*?*/
///*?*/                             if (nNewPos>0 && pNewDefault)
///*?*/                             {
///*?*/                                 rList.Remove( pNewDefault                 );
///*?*/                                 rList.Remove( pOldDefault                 );
///*?*/                                 rList.Insert( pNewDefault, (ULONG)0       );
///*?*/                                 rList.Insert( pOldDefault, (ULONG)nNewPos );
///*?*/                             }
///*N*/                         }
///*N*/                     }
///*N*/                 }
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/     catch( ::com::sun::star::uno::Exception& )
/*N*/     {
/*N*/         DBG_ASSERT( sal_False, "SfxFilterContainer::ReadExternalFilter()\nException detected. Possible not all filters could be cached.\n" );
/*N*/     }
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
