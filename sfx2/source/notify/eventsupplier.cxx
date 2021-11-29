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
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <tools/urlobj.hxx>
#include <svl/macitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/evntconf.hxx>
#include <unotools/eventcfg.hxx>
#include <sal/log.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <eventsupplier.hxx>

#include <sfx2/app.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <macroloader.hxx>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/unistr.h>

using namespace css;
using namespace ::com::sun::star;



    //  --- XNameReplace ---

void SAL_CALL SfxEvents_Impl::replaceByName( const OUString & aName, const uno::Any & rElement )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and replace the data
    auto nIndex = comphelper::findValue(maEventNames, aName);
    if (nIndex != -1)
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
            maEventData[nIndex] <<= aNormalizedDescriptor.getPropertyValues();
        }
        else
        {
            maEventData[nIndex].clear();
        }
        return;
    }

    throw container::NoSuchElementException();
}


//  --- XNameAccess ---

uno::Any SAL_CALL SfxEvents_Impl::getByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    auto nIndex = comphelper::findValue(maEventNames, aName);
    if (nIndex != -1)
        return maEventData[nIndex];

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

    return comphelper::findValue(maEventNames, aName) != -1;
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

    return maEventNames.hasElements();
}

bool SfxEvents_Impl::isScriptURLAllowed(const OUString& aScriptURL)
{
    std::optional<css::uno::Sequence<OUString>> allowedEvents(
        officecfg::Office::Common::Security::Scripting::AllowedDocumentEventURLs::get());
    // When AllowedDocumentEventURLs is empty, all event URLs are allowed
    if (!allowedEvents)
        return true;

    icu::ErrorCode status;
    const uint32_t rMatcherFlags = UREGEX_CASE_INSENSITIVE;
    icu::UnicodeString usInput(aScriptURL.getStr());
    const css::uno::Sequence<OUString>& rAllowedEvents = *allowedEvents;
    for (auto const& allowedEvent : rAllowedEvents)
    {
        icu::UnicodeString usRegex(allowedEvent.getStr());
        icu::RegexMatcher rmatch1(usRegex, usInput, rMatcherFlags, status);
        if (aScriptURL.startsWith(allowedEvent) || rmatch1.matches(status))
        {
            return true;
        }
    }

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

    if ( !aProperties.hasElements() )
        return;

    for ( const auto& rProp : std::as_const(aProperties) )
    {
        if ( rProp.Name == PROP_EVENT_TYPE )
            rProp.Value >>= aType;
        else if ( rProp.Name == PROP_SCRIPT )
            rProp.Value >>= aScript;
        else if ( rProp.Name == PROP_LIBRARY )
            rProp.Value >>= aLibrary;
        else if ( rProp.Name == PROP_MACRO_NAME )
            rProp.Value >>= aMacroName;
        else {
            OSL_FAIL("Unknown property value!");
        }
    }

    if (aType.isEmpty())
    {
        // Empty type means no active binding for the event. Just ignore do nothing.
        return;
    }

    if (aScript.isEmpty())
        return;

    if (!isScriptURLAllowed(aScript))
        return;

    if (!pDoc)
        pDoc = SfxObjectShell::Current();

    if (pDoc && !SfxObjectShell::isScriptAccessAllowed(pDoc->GetModel()))
        return;

    if (aType == STAR_BASIC)
    {
        uno::Any aAny;
        SfxMacroLoader::loadMacro( aScript, aAny, pDoc );
    }
    else if (aType == "Service" || aType == "Script")
    {
        util::URL aURL;
        uno::Reference < util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );

        aURL.Complete = aScript;
        xTrans->parseStrict( aURL );

        bool bAllowed = !SfxObjectShell::UnTrustedScript(aURL.Complete);

        if (bAllowed)
        {
            SfxViewFrame* pView = SfxViewFrame::GetFirst(pDoc);

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
                xProv = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
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
    else
    {
        SAL_WARN( "sfx.notify", "notifyEvent(): Unsupported event type" );
    }
}


// --- ::document::XEventListener ---

void SAL_CALL SfxEvents_Impl::documentEventOccured( const document::DocumentEvent& aEvent )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );

    // get the event name, find the corresponding data, execute the data

    auto nIndex = comphelper::findValue(maEventNames, aEvent.EventName);
    if ( nIndex == -1 )
        return;

    uno::Any aEventData = maEventData[ nIndex ];
    aGuard.clear();
    Execute( aEventData, aEvent, mpObjShell );
}


// --- ::lang::XEventListener ---

void SAL_CALL SfxEvents_Impl::disposing( const lang::EventObject& /*Source*/ )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mxBroadcaster.is() )
    {
        mxBroadcaster->removeDocumentEventListener( this );
        mxBroadcaster = nullptr;
    }
}


SfxEvents_Impl::SfxEvents_Impl( SfxObjectShell* pShell,
                                uno::Reference< document::XDocumentEventBroadcaster > const & xBroadcaster )
{
    // get the list of supported events and store it
    if ( pShell )
        maEventNames = pShell->GetEventNames();
    else
        maEventNames = rtl::Reference<GlobalEventConfig>(new GlobalEventConfig)->getElementNames();

    maEventData.resize( maEventNames.getLength() );

    mpObjShell      = pShell;
    mxBroadcaster   = xBroadcaster;

    if ( mxBroadcaster.is() )
        mxBroadcaster->addDocumentEventListener( this );
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

        if ( !aProperties.hasElements() )
            return pMacro;

        for ( const auto& rProp : std::as_const(aProperties) )
        {
            if ( rProp.Name == PROP_EVENT_TYPE )
                rProp.Value >>= aType;
            else if ( rProp.Name == PROP_SCRIPT )
                rProp.Value >>= aScriptURL;
            else if ( rProp.Name == PROP_LIBRARY )
                rProp.Value >>= aLibrary;
            else if ( rProp.Name == PROP_MACRO_NAME )
                rProp.Value >>= aMacroName;
            else {
                OSL_FAIL("Unknown property value!");
            }
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
                OUString aBasMgrName( INetURLObject::decode( aScript.subView( 8, nThirdSlashPos-8 ), INetURLObject::DecodeMechanism::WithCharset ) );
                if (pDoc && aBasMgrName == ".")
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
