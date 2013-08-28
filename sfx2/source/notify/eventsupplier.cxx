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
#include <com/sun/star/task/JobExecutor.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svl/macitem.hxx>
#include <sfx2/appuno.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/evntconf.hxx>
#include <unotools/eventcfg.hxx>

#include <unotools/securityoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include "eventsupplier.hxx"

#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"

#include <sfx2/sfxsids.hrc>
#include "sfxlocal.hrc"
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>

//--------------------------------------------------------------------------------------------------------

#define MACRO_PRFIX         "macro://"
#define MACRO_POSTFIX       "()"

using namespace css;

//--------------------------------------------------------------------------------------------------------
    //  --- XNameReplace ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::replaceByName( const OUString & aName, const uno::Any & rElement )
                                throw( lang::IllegalArgumentException, container::NoSuchElementException,
                                       lang::WrappedTargetException, uno::RuntimeException )
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
                mpObjShell->SetModified( sal_True );

            ::comphelper::NamedValueCollection aNormalizedDescriptor;
            NormalizeMacro( aEventDescriptor, aNormalizedDescriptor, mpObjShell );

            OUString sType;
            if  (   ( aNormalizedDescriptor.size() == 1 )
                &&  ( aNormalizedDescriptor.has( PROP_EVENT_TYPE) == 0 )
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

//--------------------------------------------------------------------------------------------------------
//  --- XNameAccess ---
//--------------------------------------------------------------------------------------------------------
uno::Any SAL_CALL SfxEvents_Impl::getByName( const OUString& aName )
                                throw( container::NoSuchElementException, lang::WrappedTargetException,
                                       uno::RuntimeException )
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

//--------------------------------------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL SfxEvents_Impl::getElementNames() throw ( uno::RuntimeException )
{
    return maEventNames;
}

//--------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxEvents_Impl::hasByName( const OUString& aName ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    long nCount = maEventNames.getLength();

    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
            return sal_True;
    }

    return sal_False;
}

//--------------------------------------------------------------------------------------------------------
//  --- XElementAccess ( parent of XNameAccess ) ---
//--------------------------------------------------------------------------------------------------------
uno::Type SAL_CALL SfxEvents_Impl::getElementType() throw ( uno::RuntimeException )
{
    uno::Type aElementType = ::getCppuType( (const uno::Sequence < beans::PropertyValue > *)0 );
    return aElementType;
}

//--------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxEvents_Impl::hasElements() throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( maEventNames.getLength() )
        return sal_True;
    else
        return sal_False;
}

static void Execute( uno::Any& aEventData, const document::DocumentEvent& aTrigger, SfxObjectShell* pDoc )
{
    uno::Sequence < beans::PropertyValue > aProperties;
    if ( aEventData >>= aProperties )
    {
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

        if (aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(STAR_BASIC)) && !aScript.isEmpty())
        {
            uno::Any aAny;
            SfxMacroLoader::loadMacro( aScript, aAny, pDoc );
        }
        else if (aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Service")) ||
                  aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Script")))
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

                if ( pView != NULL )
                {
                    xProv = uno::Reference
                        < frame::XDispatchProvider > (
                            pView->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
                }
                else
                {
                    xProv = uno::Reference< frame::XDispatchProvider > (
                                frame::Desktop::create( ::comphelper::getProcessComponentContext() ),
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
}

//--------------------------------------------------------------------------------------------------------
// --- ::document::XEventListener ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );

    // get the event name, find the coresponding data, execute the data

    OUString aName   = aEvent.EventName;
    long        nCount  = maEventNames.getLength();
    long        nIndex  = 0;
    sal_Bool    bFound  = sal_False;

    while ( !bFound && ( nIndex < nCount ) )
    {
        if ( maEventNames[nIndex] == aName )
            bFound = sal_True;
        else
            nIndex += 1;
    }

    if ( !bFound )
        return;

    uno::Any aEventData = maEventData[ nIndex ];
    aGuard.clear();
    Execute( aEventData, document::DocumentEvent(aEvent.Source, aEvent.EventName, NULL, uno::Any()), mpObjShell );
}

//--------------------------------------------------------------------------------------------------------
// --- ::lang::XEventListener ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::disposing( const lang::EventObject& /*Source*/ ) throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mxBroadcaster.is() )
    {
        mxBroadcaster->removeEventListener( this );
        mxBroadcaster = NULL;
    }
}

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
SfxEvents_Impl::SfxEvents_Impl( SfxObjectShell* pShell,
                                uno::Reference< document::XEventBroadcaster > xBroadcaster )
{
    // get the list of supported events and store it
    if ( pShell )
        maEventNames = pShell->GetEventNames();
    else
        maEventNames = GlobalEventConfig().getElementNames();

    maEventData = uno::Sequence < uno::Any > ( maEventNames.getLength() );

    mpObjShell      = pShell;
    mxBroadcaster   = xBroadcaster;

    if ( mxBroadcaster.is() )
        mxBroadcaster->addEventListener( this );
}

//--------------------------------------------------------------------------------------------------------
SfxEvents_Impl::~SfxEvents_Impl()
{
}

//--------------------------------------------------------------------------------------------------------
SvxMacro* SfxEvents_Impl::ConvertToMacro( const uno::Any& rElement, SfxObjectShell* pObjShell, sal_Bool bNormalizeMacro )
{
    SvxMacro* pMacro = NULL;
    uno::Sequence < beans::PropertyValue > aProperties;
    uno::Any aAny;
    if ( bNormalizeMacro )
        NormalizeMacro( rElement, aAny, pObjShell );
    else
        aAny = rElement;

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
                OSL_FAIL("Unknown propery value!");
            }
            nIndex += 1;
        }

        // Get the type
        ScriptType  eType( STARBASIC );
        if ( aType == STAR_BASIC )
            eType = STARBASIC;
        else if (aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Script")) && !aScriptURL.isEmpty())
            eType = EXTENDED_STYPE;
        else if ( aType == SVX_MACRO_LANGUAGE_JAVASCRIPT )
            eType = JAVASCRIPT;
        else {
            SAL_WARN( "sfx.notify", "ConvertToMacro: Unknown macro type" );
        }

        if ( !aMacroName.isEmpty() )
        {
            if ( aLibrary == "application" )
                aLibrary = SFX_APP()->GetName();
            else
                aLibrary = "";
            pMacro = new SvxMacro( aMacroName, aLibrary, eType );
        }
        else if ( eType == EXTENDED_STYPE )
            pMacro = new SvxMacro( aScriptURL, aType );
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

    if ( aType == STAR_BASIC )
    {
        if ( !aScript.isEmpty() )
        {
            if ( aMacroName.isEmpty() || aLibrary.isEmpty() )
            {
                sal_Int32 nHashPos = aScript.indexOf( '/', 8 );
                sal_Int32 nArgsPos = aScript.indexOf( '(' );
                if ( ( nHashPos != -1 ) && ( nHashPos < nArgsPos ) )
                {
                    OUString aBasMgrName( INetURLObject::decode( aScript.copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET ) );
                    if ( aBasMgrName == "." )
                        aLibrary = pDoc->GetTitle();
                    else
                        aLibrary = SFX_APP()->GetName();

                    // Get the macro name
                    aMacroName = aScript.copy( nHashPos+1, nArgsPos - nHashPos - 1 );
                }
                else
                {
                    SAL_WARN( "sfx.notify", "ConvertToMacro: Unknown macro url format" );
                }
            }
        }
        else if ( !aMacroName.isEmpty() )
        {
            aScript = OUString( MACRO_PRFIX  );
            if ( aLibrary != SFX_APP()->GetName() && aLibrary != "StarDesktop" && aLibrary != "application" )
                aScript += OUString('.');

            aScript += OUString('/');
            aScript += aMacroName;
            aScript += OUString( MACRO_POSTFIX  );
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
}

ModelCollectionEnumeration::ModelCollectionEnumeration()
    : ModelCollectionMutexBase(                 )
    , m_pEnumerationIt        (m_lModels.begin())
{
}

ModelCollectionEnumeration::~ModelCollectionEnumeration()
{
}

void ModelCollectionEnumeration::setModelList(const TModelList& rList)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    m_lModels        = rList;
    m_pEnumerationIt = m_lModels.begin();
    aLock.clear();
    // <- SAFE
}

sal_Bool SAL_CALL ModelCollectionEnumeration::hasMoreElements()
    throw(uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return (m_pEnumerationIt != m_lModels.end());
    // <- SAFE
}

uno::Any SAL_CALL ModelCollectionEnumeration::nextElement()
    throw(container::NoSuchElementException,
          lang::WrappedTargetException     ,
          uno::RuntimeException            )
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_pEnumerationIt == m_lModels.end())
        throw container::NoSuchElementException(
                    OUString("End of model enumeration reached."),
                    static_cast< container::XEnumeration* >(this));
    uno::Reference< frame::XModel > xModel(*m_pEnumerationIt, uno::UNO_QUERY);
    ++m_pEnumerationIt;
    aLock.clear();
    // <- SAFE

    return uno::makeAny(xModel);
}

SFX_IMPL_XSERVICEINFO_CTX( SfxGlobalEvents_Impl, "com.sun.star.frame.GlobalEventBroadcaster", "com.sun.star.comp.sfx2.GlobalEventBroadcaster" )
SFX_IMPL_ONEINSTANCEFACTORY( SfxGlobalEvents_Impl );

//-----------------------------------------------------------------------------
SfxGlobalEvents_Impl::SfxGlobalEvents_Impl( const uno::Reference < uno::XComponentContext >& rxContext)
    : ModelCollectionMutexBase(       )
    , m_xJobExecutorListener( task::JobExecutor::create( rxContext ), uno::UNO_QUERY_THROW )
    , m_aLegacyListeners      (m_aLock)
    , m_aDocumentListeners    (m_aLock)
    , pImp                    (0      )
{
    m_refCount++;
    SFX_APP();
    pImp                   = new GlobalEventConfig();
    m_xEvents              = pImp;
    m_refCount--;
}

//-----------------------------------------------------------------------------
SfxGlobalEvents_Impl::~SfxGlobalEvents_Impl()
{
}

//-----------------------------------------------------------------------------
uno::Reference< container::XNameReplace > SAL_CALL SfxGlobalEvents_Impl::getEvents()
    throw(uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_xEvents;
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::addEventListener(const uno::Reference< document::XEventListener >& xListener)
    throw(uno::RuntimeException)
{
    // container is threadsafe
    m_aLegacyListeners.addInterface(xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::removeEventListener(const uno::Reference< document::XEventListener >& xListener)
    throw(uno::RuntimeException)
{
    // container is threadsafe
    m_aLegacyListeners.removeInterface(xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& _Listener )
    throw(uno::RuntimeException)
{
    m_aDocumentListeners.addInterface( _Listener );
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& _Listener )
    throw(uno::RuntimeException)
{
    m_aDocumentListeners.removeInterface( _Listener );
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::notifyDocumentEvent( const OUString& /*_EventName*/,
        const uno::Reference< frame::XController2 >& /*_ViewController*/, const uno::Any& /*_Supplement*/ )
        throw (lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    // we're a multiplexer only, no chance to generate artifical events here
    throw lang::NoSupportException(OUString(), *this);
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::notifyEvent(const document::EventObject& aEvent)
    throw(uno::RuntimeException)
{
    document::DocumentEvent aDocEvent(aEvent.Source, aEvent.EventName, NULL, uno::Any());
    implts_notifyJobExecution(aEvent);
    implts_checkAndExecuteEventBindings(aDocEvent);
    implts_notifyListener(aDocEvent);
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::documentEventOccured( const document::DocumentEvent& _Event )
    throw (uno::RuntimeException)
{
    implts_notifyJobExecution(document::EventObject(_Event.Source, _Event.EventName));
    implts_checkAndExecuteEventBindings(_Event);
    implts_notifyListener(_Event);
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::disposing(const lang::EventObject& aEvent)
    throw(uno::RuntimeException)
{
    uno::Reference< frame::XModel > xDoc(aEvent.Source, uno::UNO_QUERY);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        m_lModels.erase(pIt);
    aLock.clear();
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxGlobalEvents_Impl::has(const uno::Any& aElement)
    throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;

    sal_Bool bHas = sal_False;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        bHas = sal_True;
    aLock.clear();
    // <- SAFE

    return bHas;
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::insert( const uno::Any& aElement )
    throw (lang::IllegalArgumentException  ,
           container::ElementExistException,
           uno::RuntimeException           )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                OUString("Cant locate at least the model parameter."),
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt != m_lModels.end())
        throw container::ElementExistException(
                OUString(),
                static_cast< container::XSet* >(this));
    m_lModels.push_back(xDoc);
    aLock.clear();
    // <- SAFE

    uno::Reference< document::XDocumentEventBroadcaster > xDocBroadcaster(xDoc, uno::UNO_QUERY );
    if (xDocBroadcaster.is())
        xDocBroadcaster->addDocumentEventListener(this);
    else
    {
        // try the "legacy version" of XDocumentEventBroadcaster, which is XEventBroadcaster
        uno::Reference< document::XEventBroadcaster > xBroadcaster(xDoc, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addEventListener(static_cast< document::XEventListener* >(this));
    }
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxGlobalEvents_Impl::remove( const uno::Any& aElement )
    throw (lang::IllegalArgumentException   ,
           container::NoSuchElementException,
           uno::RuntimeException            )
{
    uno::Reference< frame::XModel > xDoc;
    aElement >>= xDoc;
    if (!xDoc.is())
        throw lang::IllegalArgumentException(
                OUString("Cant locate at least the model parameter."),
                static_cast< container::XSet* >(this),
                0);

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    TModelList::iterator pIt = impl_searchDoc(xDoc);
    if (pIt == m_lModels.end())
        throw container::NoSuchElementException(
                OUString(),
                static_cast< container::XSet* >(this));
    m_lModels.erase(pIt);
    aLock.clear();
    // <- SAFE

    uno::Reference< document::XDocumentEventBroadcaster > xDocBroadcaster(xDoc, uno::UNO_QUERY );
    if (xDocBroadcaster.is())
        xDocBroadcaster->removeDocumentEventListener(this);
    else
    {
        // try the "legacy version" of XDocumentEventBroadcaster, which is XEventBroadcaster
        uno::Reference< document::XEventBroadcaster > xBroadcaster(xDoc, uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener(static_cast< document::XEventListener* >(this));
    }
}

//-----------------------------------------------------------------------------
uno::Reference< container::XEnumeration > SAL_CALL SfxGlobalEvents_Impl::createEnumeration()
    throw (uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    ModelCollectionEnumeration* pEnum = new ModelCollectionEnumeration();
    pEnum->setModelList(m_lModels);
    uno::Reference< container::XEnumeration > xEnum(
        static_cast< container::XEnumeration* >(pEnum),
        uno::UNO_QUERY);
    aLock.clear();
    // <- SAFE

    return xEnum;
}

//-----------------------------------------------------------------------------
uno::Type SAL_CALL SfxGlobalEvents_Impl::getElementType()
    throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference< frame::XModel >* >(NULL));
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxGlobalEvents_Impl::hasElements()
    throw (uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return (m_lModels.size()>0);
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SfxGlobalEvents_Impl::implts_notifyJobExecution(const document::EventObject& aEvent)
{
    try
    {
        m_xJobExecutorListener->notifyEvent(aEvent);
    }
    catch(const uno::RuntimeException&)
        { throw; }
    catch(const uno::Exception&)
        {}
}

//-----------------------------------------------------------------------------
void SfxGlobalEvents_Impl::implts_checkAndExecuteEventBindings(const document::DocumentEvent& aEvent)
{
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);
        uno::Reference< container::XNameReplace > xEvents = m_xEvents;
        aLock.clear();
        // <- SAFE

        uno::Any aAny;
        if ( xEvents.is() && xEvents->hasByName( aEvent.EventName ) )
            aAny = xEvents->getByName(aEvent.EventName);
        Execute(aAny, aEvent, 0);
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
       DBG_UNHANDLED_EXCEPTION();
    }
}

//-----------------------------------------------------------------------------
void SfxGlobalEvents_Impl::implts_notifyListener(const document::DocumentEvent& aEvent)
{
    // containers are threadsafe
    document::EventObject aLegacyEvent(aEvent.Source, aEvent.EventName);
    m_aLegacyListeners.notifyEach( &document::XEventListener::notifyEvent, aLegacyEvent );

    m_aDocumentListeners.notifyEach( &document::XDocumentEventListener::documentEventOccured, aEvent );
}

//-----------------------------------------------------------------------------
// not threadsafe ... must be locked from outside!
TModelList::iterator SfxGlobalEvents_Impl::impl_searchDoc(const uno::Reference< frame::XModel >& xModel)
{
    if (!xModel.is())
        return m_lModels.end();

    TModelList::iterator pIt;
    for (  pIt  = m_lModels.begin();
           pIt != m_lModels.end()  ;
         ++pIt                     )
    {
        uno::Reference< frame::XModel > xContainerDoc(*pIt, uno::UNO_QUERY);
        if (xContainerDoc == xModel)
            break;
    }

    return pIt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
