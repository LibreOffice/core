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

//--------------------------------------------------------------------------------------------------------

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/util/URL.hpp>

#include <com/sun/star/util/XURLTransformer.hpp>

#include <tools/urlobj.hxx>

#include <bf_svtools/macitem.hxx>

#include <appuno.hxx>
#include <objsh.hxx>
#include <sfxbasemodel.hxx>
#include <evntconf.hxx>

#include <bf_svtools/securityoptions.hxx>
#include <comphelper/processfactory.hxx>

#include "eventsupplier.hxx"

#include "app.hxx"

#include "sfxsids.hrc"
#include "docfile.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

//--------------------------------------------------------------------------------------------------------

#define MACRO_PRFIX			"macro://"
#define MACRO_POSTFIX		"()"

//--------------------------------------------------------------------------------------------------------

#define PROPERTYVALUE		::com::sun::star::beans::PropertyValue
#define	UNO_QUERY			::com::sun::star::uno::UNO_QUERY

//--------------------------------------------------------------------------------------------------------
    //  --- XNameReplace ---
//--------------------------------------------------------------------------------------------------------
/*N*/ void SAL_CALL SfxEvents_Impl::replaceByName( const OUSTRING & aName, const ANY & rElement )
/*N*/ 								throw( ILLEGALARGUMENTEXCEPTION, NOSUCHELEMENTEXCEPTION,
/*N*/ 									   WRAPPEDTARGETEXCEPTION, RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( maMutex );
/*N*/ 
/*N*/ 	// find the event in the list and replace the data
/*N*/ 	long nCount	= maEventNames.getLength();
/*N*/ 	for ( long i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		if ( maEventNames[i] == aName )
/*N*/ 		{
/*N*/ 			// check for correct type of the element
/*N*/             if ( ::getCppuType( (const SEQUENCE < PROPERTYVALUE > *)0 ) == rElement.getValueType() )
/*N*/ 			{
/*N*/ 				// create Configuration at first, creation might call this method also and that would overwrite everything
/*N*/ 				// we might have stored before!
/*N*/                 USHORT nID = (USHORT) SfxEventConfiguration::GetEventId_Impl( aName );
/*N*/                 if ( nID && mpObjShell )
/*N*/ 				{
/*N*/ 					SfxEventConfigItem_Impl* pConfig =
/*N*/ 						mpObjShell->GetEventConfig_Impl(TRUE);
/*N*/ 
/*N*/ 					ANY aValue;
/*N*/ 					BlowUpMacro( rElement, aValue, mpObjShell );
/*N*/ 
/*N*/ 					// pConfig becomes the owner of the new SvxMacro
/*N*/ 					SvxMacro *pMacro = ConvertToMacro( aValue, mpObjShell, FALSE );
/*N*/ 				 	pConfig->ConfigureEvent( nID, pMacro );
/*N*/ 		            maEventData[i] = aValue;
/*N*/ 
/*N*/ 		            SEQUENCE < PROPERTYVALUE > aProperties;
/*N*/ 		            if ( aValue >>= aProperties )
/*N*/ 		            {
/*N*/ 		                long nCount = aProperties.getLength();
/*N*/ 		                for ( long nIndex = 0; nIndex < nCount; nIndex++ )
/*N*/ 		                {
/*N*/ 		                    if ( aProperties[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
/*N*/ 		                    {
/*N*/ 		                        ::rtl::OUString aType;
/*N*/ 		                        aProperties[ nIndex ].Value >>= aType;
/*N*/ 		                        break;
/*N*/ 		                    }
/*N*/ 		                }
/*N*/ 		            }
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				throw ILLEGALARGUMENTEXCEPTION();
/*N*/ 
/*N*/ 			return;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	throw NOSUCHELEMENTEXCEPTION();
/*N*/ }

//--------------------------------------------------------------------------------------------------------
//  --- XNameAccess ---
//--------------------------------------------------------------------------------------------------------
/*N*/ ANY SAL_CALL SfxEvents_Impl::getByName( const OUSTRING& aName )
/*N*/ 								throw( NOSUCHELEMENTEXCEPTION, WRAPPEDTARGETEXCEPTION,
/*N*/ 									   RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( maMutex );
/*N*/ 
/*N*/ 	// find the event in the list and return the data
/*N*/ 
/*N*/ 	long nCount	= maEventNames.getLength();
/*N*/ 
/*N*/ 	for ( long i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		if ( maEventNames[i] == aName )
/*N*/ 			return maEventData[i];
/*N*/ 	}
/*N*/ 
/*N*/ 	throw NOSUCHELEMENTEXCEPTION();
/*N*/ 
/*N*/ 	return ANY();
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*N*/ SEQUENCE< OUSTRING > SAL_CALL SfxEvents_Impl::getElementNames() throw ( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	return maEventNames;
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*?*/ sal_Bool SAL_CALL SfxEvents_Impl::hasByName( const OUSTRING& aName ) throw ( RUNTIMEEXCEPTION )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------------------------------------------
//  --- XElementAccess ( parent of XNameAccess ) ---
//--------------------------------------------------------------------------------------------------------
/*?*/ UNOTYPE SAL_CALL SfxEvents_Impl::getElementType() throw ( RUNTIMEEXCEPTION )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); UNOTYPE aUNOTYPE; return aUNOTYPE; //STRIP001 
/*?*/ }

//--------------------------------------------------------------------------------------------------------
/*?*/ sal_Bool SAL_CALL SfxEvents_Impl::hasElements() throw ( RUNTIMEEXCEPTION )
/*?*/ {DBG_BF_ASSERT(0, "STRIP");  return FALSE;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------------------------------------------
// --- ::document::XEventListener ---
//--------------------------------------------------------------------------------------------------------
/*N*/ void SAL_CALL SfxEvents_Impl::notifyEvent( const DOCEVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ }

//--------------------------------------------------------------------------------------------------------
// --- ::lang::XEventListener ---
//--------------------------------------------------------------------------------------------------------
/*N*/ void SAL_CALL SfxEvents_Impl::disposing( const EVENTOBJECT& Source ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( maMutex );
/*N*/ 
/*N*/ 	if ( mxBroadcaster.is() )
/*N*/ 	{
/*N*/ 		mxBroadcaster->removeEventListener( this );
/*N*/ 		mxBroadcaster = NULL;
/*N*/ 	}
/*N*/ }

//--------------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------------
/*N*/ SfxEvents_Impl::SfxEvents_Impl( SfxObjectShell* pShell,
/*N*/ 							    REFERENCE< XEVENTBROADCASTER > xBroadcaster )
/*N*/ {
/*N*/ 	// get the list of supported events and store it
/*N*/ 	if ( pShell )
/*N*/ 		maEventNames = pShell->GetEventNames();
/*N*/ 	else
/*N*/ 		maEventNames = SfxObjectShell::GetEventNames_Impl();
/*N*/ 
/*N*/ 	maEventData = SEQUENCE < ANY > ( maEventNames.getLength() );
/*N*/ 
/*N*/ 	mpObjShell		= pShell;
/*N*/ 	mxBroadcaster	= xBroadcaster;
/*N*/ 
/*N*/ 	if ( mxBroadcaster.is() )
/*N*/ 		mxBroadcaster->addEventListener( this );
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*N*/ SfxEvents_Impl::~SfxEvents_Impl()
/*N*/ {
/*N*/ 	if ( mxBroadcaster.is() )
/*?*/ 		mxBroadcaster->removeEventListener( this );
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*N*/ SvxMacro* SfxEvents_Impl::ConvertToMacro( const ANY& rElement, SfxObjectShell* pObjShell, BOOL bBlowUp )
/*N*/ {
/*N*/ 	SvxMacro* pMacro = NULL;
/*N*/ 	SEQUENCE < PROPERTYVALUE > aProperties;
/*N*/ 	ANY aAny;
/*N*/ 	if ( bBlowUp )
/*?*/ 		BlowUpMacro( rElement, aAny, pObjShell );
/*N*/ 	else
/*N*/ 		aAny = rElement;
/*N*/ 
/*N*/ 	if ( aAny >>= aProperties )
/*N*/ 	{
/*N*/ 		OUSTRING		aType;
/*N*/ 		OUSTRING		aScriptURL;
/*N*/ 		OUSTRING		aLibrary;
/*N*/ 		OUSTRING		aMacroName;
/*N*/ 
/*N*/ 		long nCount = aProperties.getLength();
/*N*/ 		long nIndex = 0;
/*N*/ 
/*N*/ 		if ( !nCount )
/*N*/ 			return pMacro;
/*N*/ 
/*N*/ 		while ( nIndex < nCount )
/*N*/ 		{
/*N*/ 			if ( aProperties[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
/*N*/ 				aProperties[ nIndex ].Value >>= aType;
/*N*/ 			else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
/*N*/ 				aProperties[ nIndex ].Value >>= aScriptURL;
/*N*/ 			else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
/*N*/ 				aProperties[ nIndex ].Value >>= aLibrary;
/*N*/ 			else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
/*N*/ 				aProperties[ nIndex ].Value >>= aMacroName;
/*N*/ 			else
/*N*/ 				DBG_ERROR("Unknown propery value!");
/*N*/ 			nIndex += 1;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Get the type
/*N*/ 		ScriptType	eType( STARBASIC );
/*N*/ 		if ( aType.compareToAscii( STAR_BASIC ) == COMPARE_EQUAL )
/*N*/ 			eType = STARBASIC;
/*N*/ 		else if ( aType.compareToAscii( "Script" ) == COMPARE_EQUAL && aScriptURL.getLength() )
/*N*/ 			eType = EXTENDED_STYPE;
/*N*/ 		else if ( aType.compareToAscii( SVX_MACRO_LANGUAGE_JAVASCRIPT ) == COMPARE_EQUAL )
/*N*/ 			eType = JAVASCRIPT;
/*N*/ 		else
/*N*/ 			DBG_ERRORFILE( "ConvertToMacro: Unknown macro type" );
/*N*/ 
/*N*/ 		if ( aMacroName.getLength() )
/*N*/ 		{
/*N*/ 			if ( aLibrary.compareToAscii("application") == 0 )
/*N*/ 				aLibrary = SFX_APP()->GetName();
/*N*/ 			else
/*N*/ 				aLibrary = ::rtl::OUString();
/*N*/ 			pMacro = new SvxMacro( aMacroName, aLibrary, eType );
/*N*/ 		}
/*N*/ 		else if ( eType == EXTENDED_STYPE )
/*?*/ 			pMacro = new SvxMacro( aScriptURL, aType );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pMacro;
/*N*/ }

/*N*/ void SfxEvents_Impl::BlowUpMacro( const ANY& rEvent, ANY& rRet, SfxObjectShell* pDoc )
/*N*/ {
/*N*/ 	if ( !pDoc )
/*N*/ 		pDoc = SfxObjectShell::Current();
/*N*/ 
/*N*/ 	SEQUENCE < PROPERTYVALUE > aInProps;
/*N*/ 	SEQUENCE < PROPERTYVALUE > aOutProps(2);
/*N*/ 
/*N*/ 	if ( !( rEvent >>= aInProps ) )
/*N*/ 		return;
/*N*/ 
/*N*/     sal_Int32 nCount = aInProps.getLength();
/*N*/ 
/*N*/ 	if ( !nCount )
/*N*/ 		return;
/*N*/ 
/*N*/     OUSTRING aType;
/*N*/ 	OUSTRING aScript;
/*N*/ 	OUSTRING aLibrary;
/*N*/ 	OUSTRING aMacroName;
/*N*/ 
/*N*/ 	sal_Int32 nIndex = 0;
/*N*/ 
/*N*/     while ( nIndex < nCount )
/*N*/ 	{
/*N*/ 		if ( aInProps[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
/*N*/ 		{
/*?*/ 			aInProps[nIndex].Value >>= aType;
/*?*/ 			aOutProps[0] = aInProps[nIndex];
/*N*/ 		}
/*N*/ 		else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
/*N*/ 		{
/*N*/ 			aInProps[nIndex].Value >>= aScript;
/*N*/ 			aOutProps[1] = aInProps[nIndex];
/*N*/ 		}
/*N*/ 		else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
/*N*/ 		{
/*N*/ 			aInProps[ nIndex ].Value >>= aLibrary;
/*N*/ 		}
/*N*/ 		else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
/*N*/ 		{
/*N*/ 			aInProps[ nIndex ].Value >>= aMacroName;
/*N*/ 		}
/*N*/ 		nIndex += 1;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( aType.compareToAscii( STAR_BASIC ) == 0 )
/*N*/ 	{
/*N*/ 		aOutProps.realloc(4);
/*N*/ 		if ( aScript.getLength() )
/*N*/ 		{
/*N*/ 			if( ! aMacroName.getLength() || ! aLibrary.getLength() )
/*?*/ 			{
/*?*/ 				sal_Int32 nHashPos = aScript.indexOf( '/', 8 );
/*?*/ 				sal_Int32 nArgsPos = aScript.indexOf( '(' );
/*?*/ 				if ( ( nHashPos != STRING_NOTFOUND ) && ( nHashPos < nArgsPos ) )
/*?*/ 				{
/*?*/ 					OUSTRING aBasMgrName( INetURLObject::decode( aScript.copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET ) );
/*?*/ 					if ( aBasMgrName.compareToAscii(".") == 0 )
/*?*/ 						aLibrary = pDoc->GetTitle( SFX_TITLE_APINAME );
/*
                    else if ( aBasMgrName.getLength() )
                        aLibrary = aBasMgrName;
 */
/*?*/ 					else
/*?*/ 						aLibrary = SFX_APP()->GetName();
/*?*/ 
/*?*/ 					// Get the macro name
/*?*/ 					aMacroName = aScript.copy( nHashPos+1, nArgsPos - nHashPos - 1 );
/*?*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					DBG_ERRORFILE( "ConvertToMacro: Unknown macro url format" );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( aMacroName.getLength() )
/*N*/ 		{
/*N*/             aScript = OUSTRING( RTL_CONSTASCII_USTRINGPARAM( MACRO_PRFIX ) );
/*N*/ 			if ( aLibrary.compareTo( SFX_APP()->GetName() ) != 0 && aLibrary.compareToAscii("StarDesktop") != 0 && aLibrary.compareToAscii("application") != 0 )
/*N*/ 				aScript += OUSTRING('.');
/*N*/ 
/*N*/ 			aScript += OUSTRING('/');
/*N*/ 			aScript += aMacroName;
/*N*/ 			aScript += OUSTRING( RTL_CONSTASCII_USTRINGPARAM( MACRO_POSTFIX ) );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			// wrong properties
/*N*/ 			return;
/*N*/ 
            OUSTRING sApplication( OUSTRING::createFromAscii( "application" ) );
            OUSTRING sDocument( OUSTRING::createFromAscii( "document" ) );
            if ( aLibrary != sApplication && aLibrary != sDocument )
            {
                if ( aLibrary == OUSTRING( SFX_APP()->GetName() ) || aLibrary.equalsAscii( "StarDesktop" ) )
                    aLibrary = sApplication;
                else
                    aLibrary = sDocument;
            }
/*N*/ 
/*N*/ 		aOutProps[1].Name = OUSTRING::createFromAscii( PROP_SCRIPT );
/*N*/ 		aOutProps[1].Value <<= aScript;
/*N*/ 		aOutProps[2].Name = OUSTRING::createFromAscii( PROP_LIBRARY );
/*N*/ 		aOutProps[2].Value <<= aLibrary;
/*N*/ 		aOutProps[3].Name = OUSTRING::createFromAscii( PROP_MACRO_NAME );
/*N*/ 		aOutProps[3].Value <<= aMacroName;
/*N*/ 		rRet <<= aOutProps;
/*N*/     }
/*N*/ 	else if ( aType.compareToAscii( SVX_MACRO_LANGUAGE_JAVASCRIPT ) == 0 )
/*N*/ 	{
/*?*/ 		aOutProps[1] = aInProps[1];
/*?*/ 		rRet <<= aOutProps;
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		rRet <<= aOutProps;
/*N*/ 	}
/*N*/ }

/*N*/ SFX_IMPL_XSERVICEINFO( SfxGlobalEvents_Impl, "com.sun.star.frame.GlobalEventBroadcaster", "com.sun.star.comp.sfx2.GlobalEventBroadcaster" )
/*N*/ SFX_IMPL_ONEINSTANCEFACTORY( SfxGlobalEvents_Impl );

/*N*/ SfxGlobalEvents_Impl::SfxGlobalEvents_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xSmgr )
/*N*/ 	: m_aInterfaceContainer( m_aMutex )
/*N*/ {
/*N*/ 	m_refCount++;
/*N*/ 	pImp = new SfxEvents_Impl( NULL, this );
/*N*/ 	m_xEvents = pImp;
/*N*/     m_xJobsBinding = REFERENCE< XJOBEXECUTOR >(xSmgr->createInstance(OUSTRING::createFromAscii("com.sun.star.task.JobExecutor")), UNO_QUERY);
/*N*/ 	m_refCount--;
/*N*/ 	StartListening(*SFX_APP());
/*N*/ }

/*?*/ SfxGlobalEvents_Impl::~SfxGlobalEvents_Impl()
/*?*/ {
/*?*/ }

/*N*/ REFERENCE< XNAMEREPLACE > SAL_CALL SfxGlobalEvents_Impl::getEvents() throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	return m_xEvents;
/*N*/ }

/*N*/ void SAL_CALL SfxGlobalEvents_Impl::addEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
/*N*/ {
/*N*/ 	m_aInterfaceContainer.addInterface( xListener );
/*N*/ }


/*N*/ void SfxGlobalEvents_Impl::Notify( SfxBroadcaster& aBC, const SfxHint& aHint )
/*N*/ {
/*N*/ 	SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &aHint );
/*N*/ 	if ( pNamedHint )
/*N*/ 	{
/*N*/ 		OUSTRING aName = SfxEventConfiguration::GetEventName_Impl( pNamedHint->GetEventId() );
/*N*/ 		REFERENCE < XEVENTSSUPPLIER > xSup;
/*N*/ 		if ( pNamedHint->GetObjShell() )
/*N*/ 			xSup = REFERENCE < XEVENTSSUPPLIER >( pNamedHint->GetObjShell()->GetModel(), UNO_QUERY );
/*N*/ //		else
/*N*/ //			xSup = (XEVENTSSUPPLIER*) this;
/*N*/ 
/*N*/         DOCEVENTOBJECT aEvent( xSup, aName );
/*N*/ 
/*N*/         // Attention: This listener is a special one. It binds the global document events
/*N*/         // to the generic job execution framework. It's a loose binding (using weak references).
/*N*/         // So we hold this listener outside our normal listener container.
/*N*/         // The implementation behind this job executor can be replaced ...
/*N*/         // but we check for this undocumented interface!
/*N*/         REFERENCE< XDOCEVENTLISTENER > xJobExecutor(m_xJobsBinding.get(), UNO_QUERY);
/*N*/         if (xJobExecutor.is())
/*N*/             xJobExecutor->notifyEvent(aEvent);
/*N*/ 
/*N*/ 		::cppu::OInterfaceIteratorHelper aIt( m_aInterfaceContainer );
/*N*/ 		while( aIt.hasMoreElements() )
/*N*/ 	    {
/*N*/ 	        try
/*N*/ 	        {
/*N*/ 	            ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
/*N*/ 	        }
/*N*/ 	        catch( RUNTIMEEXCEPTION& )
/*N*/ 	        {
/*?*/ 	            aIt.remove();
/*N*/ 	        }
/*N*/ 	    }
/*N*/ 	}
/*N*/ }}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
