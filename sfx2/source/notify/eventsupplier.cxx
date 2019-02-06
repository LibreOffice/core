/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/util/URL.hpp>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svl/macitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/evntconf.hxx>
#include <unotools/eventcfg.hxx>
#include <sal/log.hxx>

#include <unotools/securityoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <eventsupplier.hxx>

#include <sfx2/app.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <macroloader.hxx>

using namespace css;


    //  --- XNameReplace ---

void SAL_CALL SfxEvents_Impl::replaceByName( const OUString & aName, const uno::Any & rElement )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and replace the data
    long nCount = maEventNames.getLength();
    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
        {
            // check for correct type of the element
            if ( !::comphelper::NamedValueCollection::canExtractFrom( rElement ) )
                throw lang::IllegalArgumentException();
            ::comphelper::NamedValueCollection const aEventDescriptor( rElement );

            // create Configuration at first, creation might call this method also and that would overwrite everything
            // we might have stored before!
            if ( mpObjShell && !mpObjShell->IsLoading() )
                mpObjShell->SetModified();

            ::comphelper::NamedValueCollection aNormalizedDescriptor;
            NormalizeMacro( aEventDescriptor, aNormalizedDescriptor, mpObjShell );

            OUString sType;
            if  (   ( aNormalizedDescriptor.size() == 1 )
                &&  !aNormalizedDescriptor.has( PROP_EVENT_TYPE ) //TODO
                &&  ( aNormalizedDescriptor.get( PROP_EVENT_TYPE ) >>= sType )
                &&  ( sType.isEmpty() )
                )
            {
                // An empty event type means no binding. Therefore reset data
                // to reflect that state.
                // (that's for compatibility only. Nowadays, the Tools/Customize dialog should
                // set an empty sequence to indicate the request for resetting the assignment.)
                OSL_ENSURE( false, "legacy event assignment format detected" );
                aNormalizedDescriptor.clear();
            }

            if ( !aNormalizedDescriptor.empty() )
            {
                maEventData[i] <<= aNormalizedDescriptor.getPropertyValues();
            }
            else
            {
                maEventData[i].clear();
            }
            return;
        }
    }

    throw container::NoSuchElementException();
}


//  --- XNameAccess ---

uno::Any SAL_CALL SfxEvents_Impl::getByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    long nCount = maEventNames.getLength();

    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
            return maEventData[i];
    }

    throw container::NoSuchElementException();
}


uno::Sequence< OUString > SAL_CALL SfxEvents_Impl::getElementNames()
{
    return maEventNames;
}


sal_Bool SAL_CALL SfxEvents_Impl::hasByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    long nCount = maEventNames.getLength();

    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
            return true;
    }

    return false;
}


//  --- XElementAccess ( parent of XNameAccess ) ---

uno::Type SAL_CALL SfxEvents_Impl::getElementType()
{
    uno::Type aElementType = cppu::UnoType<uno::Sequence < beans::PropertyValue >>::get();
    return aElementType;
}


sal_Bool SAL_CALL SfxEvents_Impl::hasElements()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( maEventNames.getLength() )
        return true;
    else
        return false;
}

void SfxEvents_Impl::Execute( uno::Any const & aEventData, const document::DocumentEvent& aTrigger, SfxObjectShell* pDoc )
{
    uno::Sequence < beans::PropertyValue > aProperties;
    if ( !(aEventData >>= aProperties) )
        return;

    OUString aType;
    OUString aScript;
    OUString aLibrary;
    OUString aMacroName;

    sal_Int32 nCount = aProperties.getLength();

    if ( !nCount )
        return;

    sal_Int32 nIndex = 0;
    while ( nIndex < nCount )
    {
        if ( aProperties[ nIndex ].Name == PROP_EVENT_TYPE )
            aProperties[ nIndex ].Value >>= aType;
        else if ( aProperties[ nIndex ].Name == PROP_SCRIPT )
            aProperties[ nIndex ].Value >>= aScript;
        else if ( aProperties[ nIndex ].Name == PROP_LIBRARY )
            aProperties[ nIndex ].Value >>= aLibrary;
        else if ( aProperties[ nIndex ].Name == PROP_MACRO_NAME )
            aProperties[ nIndex ].Value >>= aMacroName;
        else {
            OSL_FAIL("Unknown property value!");
        }
        nIndex += 1;
    }

    if (aType == STAR_BASIC && !aScript.isEmpty())
    {
        uno::Any aAny;
        SfxMacroLoader::loadMacro( aScript, aAny, pDoc );
    }
    else if (aType == "Service" ||
              aType == "Script")
    {
        if ( !aScript.isEmpty() )
        {
            SfxViewFrame* pView = pDoc ?
                SfxViewFrame::GetFirst( pDoc ) :
                SfxViewFrame::Current();

            uno::Reference < util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );

            util::URL aURL;
            aURL.Complete = aScript;
            xTrans->parseStrict( aURL );

            uno::Reference
                < frame::XDispatchProvider > xProv;

            if ( pView != nullptr )
            {
                xProv = uno::Reference
                    < frame::XDispatchProvider > (
                        pView->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
            }
            else
            {
                xProv.set( frame::Desktop::create( ::comphelper::getProcessComponentContext() ),
                           uno::UNO_QUERY );
            }

            uno::Reference < frame::XDispatch > xDisp;
            if ( xProv.is() )
                xDisp = xProv->queryDispatch( aURL, OUString(), 0 );

            if ( xDisp.is() )
            {

                beans::PropertyValue aEventParam;
                aEventParam.Value <<= aTrigger;
                uno::Sequence< beans::PropertyValue > aDispatchArgs( &aEventParam, 1 );
                xDisp->dispatch( aURL, aDispatchArgs );
            }
        }
    }
    else if ( aType.isEmpty() )
    {
        // Empty type means no active binding for the event. Just ignore do nothing.
    }
    else
    {
        SAL_WARN( "sfx.notify", "notifyEvent(): Unsupported event type" );
    }
}


// --- ::document::XEventListener ---

void SAL_CALL SfxEvents_Impl::notifyEvent( const document::EventObject& aEvent )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );

    // get the event name, find the corresponding data, execute the data

    OUString aName   = aEvent.EventName;
    long        nCount  = maEventNames.getLength();
    long        nIndex  = 0;
    bool    bFound  = false;

    while ( !bFound && ( nIndex < nCount ) )
    {
        if ( maEventNames[nIndex] == aName )
            bFound = true;
        else
            nIndex += 1;
    }

    if ( !bFound )
        return;

    uno::Any aEventData = maEventData[ nIndex ];
    aGuard.clear();
    Execute( aEventData, document::DocumentEvent(aEvent.Source, aEvent.EventName, nullptr, uno::Any()), mpObjShell );
}


// --- ::lang::XEventListener ---

void SAL_CALL SfxEvents_Impl::disposing( const lang::EventObject& /*Source*/ )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mxBroadcaster.is() )
    {
        mxBroadcaster->removeEventListener( this );
        mxBroadcaster = nullptr;
    }
}


SfxEvents_Impl::SfxEvents_Impl( SfxObjectShell* pShell,
                                uno::Reference< document::XEventBroadcaster > const & xBroadcaster )
{
    // get the list of supported events and store it
    if ( pShell )
        maEventNames = pShell->GetEventNames();
    else
        maEventNames = rtl::Reference<GlobalEventConfig>(new GlobalEventConfig)->getElementNames();

    maEventData = uno::Sequence < uno::Any > ( maEventNames.getLength() );

    mpObjShell      = pShell;
    mxBroadcaster   = xBroadcaster;

    if ( mxBroadcaster.is() )
        mxBroadcaster->addEventListener( this );
}


SfxEvents_Impl::~SfxEvents_Impl()
{
}


std::unique_ptr<SvxMacro> SfxEvents_Impl::ConvertToMacro( const uno::Any& rElement, SfxObjectShell* pObjShell )
{
    std::unique_ptr<SvxMacro> pMacro;
    uno::Sequence < beans::PropertyValue > aProperties;
    uno::Any aAny;
    NormalizeMacro( rElement, aAny, pObjShell );

    if ( aAny >>= aProperties )
    {
        OUString aType;
        OUString aScriptURL;
        OUString aLibrary;
        OUString aMacroName;

        long nCount = aProperties.getLength();
        long nIndex = 0;

        if ( !nCount )
            return pMacro;

        while ( nIndex < nCount )
        {
            if ( aProperties[ nIndex ].Name == PROP_EVENT_TYPE )
                aProperties[ nIndex ].Value >>= aType;
            else if ( aProperties[ nIndex ].Name == PROP_SCRIPT )
                aProperties[ nIndex ].Value >>= aScriptURL;
            else if ( aProperties[ nIndex ].Name == PROP_LIBRARY )
                aProperties[ nIndex ].Value >>= aLibrary;
            else if ( aProperties[ nIndex ].Name == PROP_MACRO_NAME )
                aProperties[ nIndex ].Value >>= aMacroName;
            else {
                OSL_FAIL("Unknown property value!");
            }
            nIndex += 1;
        }

        // Get the type
        ScriptType  eType( STARBASIC );
        if ( aType == STAR_BASIC )
            eType = STARBASIC;
        else if (aType == "Script" && !aScriptURL.isEmpty())
            eType = EXTENDED_STYPE;
        else if ( aType == SVX_MACRO_LANGUAGE_JAVASCRIPT )
            eType = JAVASCRIPT;
        else {
            SAL_WARN( "sfx.notify", "ConvertToMacro: Unknown macro type" );
        }

        if ( !aMacroName.isEmpty() )
        {
            if ( aLibrary == "application" )
                aLibrary = SfxGetpApp()->GetName();
            else
                aLibrary.clear();
            pMacro.reset(new SvxMacro( aMacroName, aLibrary, eType ));
        }
        else if ( eType == EXTENDED_STYPE )
            pMacro.reset(new SvxMacro( aScriptURL, aType ));
    }

    return pMacro;
}

void SfxEvents_Impl::NormalizeMacro( const uno::Any& rEvent, uno::Any& rRet, SfxObjectShell* pDoc )
{
    const ::comphelper::NamedValueCollection aEventDescriptor( rEvent );
    ::comphelper::NamedValueCollection aEventDescriptorOut;

    NormalizeMacro( aEventDescriptor, aEventDescriptorOut, pDoc );

    rRet <<= aEventDescriptorOut.getPropertyValues();
}

void SfxEvents_Impl::NormalizeMacro( const ::comphelper::NamedValueCollection& i_eventDescriptor,
        ::comphelper::NamedValueCollection& o_normalizedDescriptor, SfxObjectShell* i_document )
{
    SfxObjectShell* pDoc = i_document;
    if ( !pDoc )
        pDoc = SfxObjectShell::Current();

    OUString aType = i_eventDescriptor.getOrDefault( PROP_EVENT_TYPE, OUString() );
    OUString aScript = i_eventDescriptor.getOrDefault( PROP_SCRIPT, OUString() );
    OUString aLibrary = i_eventDescriptor.getOrDefault( PROP_LIBRARY, OUString() );
    OUString aMacroName = i_eventDescriptor.getOrDefault( PROP_MACRO_NAME, OUString() );

    if ( !aType.isEmpty() )
        o_normalizedDescriptor.put( PROP_EVENT_TYPE, aType );
    if ( !aScript.isEmpty() )
        o_normalizedDescriptor.put( PROP_SCRIPT, aScript );

    if ( aType != STAR_BASIC )
        return;

    if ( !aScript.isEmpty() )
    {
        if ( aMacroName.isEmpty() || aLibrary.isEmpty() )
        {
            sal_Int32 nThirdSlashPos = aScript.indexOf( '/', 8 );
            sal_Int32 nArgsPos = aScript.indexOf( '(' );
            if ( ( nThirdSlashPos != -1 ) && ( nArgsPos == -1 || nThirdSlashPos < nArgsPos ) )
            {
                OUString aBasMgrName( INetURLObject::decode( aScript.copy( 8, nThirdSlashPos-8 ), INetURLObject::DecodeMechanism::WithCharset ) );
                if ( aBasMgrName == "." )
                    aLibrary = pDoc->GetTitle();
                else
                    aLibrary = SfxGetpApp()->GetName();

                // Get the macro name
                aMacroName = aScript.copy( nThirdSlashPos+1, nArgsPos - nThirdSlashPos - 1 );
            }
            else
            {
                SAL_WARN( "sfx.notify", "ConvertToMacro: Unknown macro url format" );
            }
        }
    }
    else if ( !aMacroName.isEmpty() )
    {
        aScript = "macro://";
        if ( aLibrary != SfxGetpApp()->GetName() && aLibrary != "StarDesktop" && aLibrary != "application" )
            aScript += ".";
        aScript += "/" + aMacroName + "()";
    }
    else
        // wrong properties
        return;

    if (aLibrary != "document")
    {
        if ( aLibrary.isEmpty() || (pDoc && ( aLibrary == pDoc->GetTitle( SFX_TITLE_APINAME ) || aLibrary == pDoc->GetTitle() )) )
            aLibrary = "document";
        else
            aLibrary = "application";
    }

    o_normalizedDescriptor.put( PROP_SCRIPT, aScript );
    o_normalizedDescriptor.put( PROP_LIBRARY, aLibrary );
    o_normalizedDescriptor.put( PROP_MACRO_NAME, aMacroName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
