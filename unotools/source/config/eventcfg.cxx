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

#include <unotools/eventcfg.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/weakref.hxx>
#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include "itemholder1.hxx"

#include <algorithm>

using namespace ::std;
using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

static const char ROOTNODE_EVENTS[] = "Office.Events/ApplicationEvents";
#define PATHDELIMITER "/"
#define SETNODE_BINDINGS "Bindings"
#define PROPERTYNAME_BINDINGURL "BindingURL"

static o3tl::enumarray<GlobalEventId, const char*> pEventAsciiNames =
{
"OnStartApp",
"OnCloseApp",
"OnCreate",
"OnNew",
"OnLoadFinished",
"OnLoad",
"OnPrepareUnload",
"OnUnload",
"OnSave",
"OnSaveDone",
"OnSaveFailed",
"OnSaveAs",
"OnSaveAsDone",
"OnSaveAsFailed",
"OnCopyTo",
"OnCopyToDone",
"OnCopyToFailed",
"OnFocus",
"OnUnfocus",
"OnPrint",
"OnViewCreated",
"OnPrepareViewClosing",
"OnViewClosed",
"OnModifyChanged",
"OnTitleChanged",
"OnVisAreaChanged",
"OnModeChanged",
"OnStorageChanged"
};

typedef std::unordered_map< OUString, OUString, OUStringHash, std::equal_to< OUString > > EventBindingHash;
typedef std::vector< css::uno::WeakReference< css::frame::XFrame > > FrameVector;
typedef o3tl::enumarray< GlobalEventId, OUString > SupportedEventsVector;

class GlobalEventConfig_Impl : public utl::ConfigItem
{
private:
    EventBindingHash m_eventBindingHash;
    FrameVector m_lFrames;
    SupportedEventsVector m_supportedEvents;

    void initBindingInfo();

    virtual void ImplCommit() override;

public:
    GlobalEventConfig_Impl( );
    virtual ~GlobalEventConfig_Impl( );

    void            Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException);
    bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException);
    static css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException);
    bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException);
    OUString GetEventName( GlobalEventId nID );
};


GlobalEventConfig_Impl::GlobalEventConfig_Impl()
    :   ConfigItem( ROOTNODE_EVENTS, ConfigItemMode::ImmediateUpdate )
{
    // the supported event names
    for (GlobalEventId id : o3tl::enumrange<GlobalEventId>())
        m_supportedEvents[id] = OUString::createFromAscii( pEventAsciiNames[id] );

    initBindingInfo();

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys! */
    Sequence<OUString> aNotifySeq { "Events" };
    EnableNotification( aNotifySeq, true );
}

//  destructor

GlobalEventConfig_Impl::~GlobalEventConfig_Impl()
{
    assert(!IsModified()); // should have been committed
}

OUString GlobalEventConfig_Impl::GetEventName( GlobalEventId nIndex )
{
    return m_supportedEvents[nIndex];
}

//  public method

void GlobalEventConfig_Impl::Notify( const Sequence< OUString >& )
{
    MutexGuard aGuard( GlobalEventConfig::GetOwnStaticMutex() );

    initBindingInfo();

    // don't forget to update all existing frames and her might cached dispatch objects!
    // But look for already killed frames. We hold weak references instead of hard ones ...
    for (FrameVector::const_iterator pIt  = m_lFrames.begin();
                                        pIt != m_lFrames.end();
                                      ++pIt                     )
    {
        css::uno::Reference< css::frame::XFrame > xFrame(pIt->get(), css::uno::UNO_QUERY);
        if (xFrame.is())
            xFrame->contextChanged();
    }
}

//  public method

void GlobalEventConfig_Impl::ImplCommit()
{
    //DF need to check it this is correct??
    OSL_TRACE("In GlobalEventConfig_Impl::ImplCommit");
    EventBindingHash::const_iterator it = m_eventBindingHash.begin();
    EventBindingHash::const_iterator it_end = m_eventBindingHash.end();
    // clear the existing nodes
    ClearNodeSet( SETNODE_BINDINGS );
    Sequence< beans::PropertyValue > seqValues( 1 );
    OUString sNode;
    static const char sPrefix[] = SETNODE_BINDINGS PATHDELIMITER "BindingType['";
    static const char sPostfix[] = "']" PATHDELIMITER PROPERTYNAME_BINDINGURL;
    //step through the list of events
    for(int i=0;it!=it_end;++it,++i)
    {
        //no point in writing out empty bindings!
        if(it->second.isEmpty() )
            continue;
        sNode = sPrefix + it->first + sPostfix;
        OSL_TRACE("writing binding for: %s",OUStringToOString(sNode , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        seqValues[ 0 ].Name = sNode;
        seqValues[ 0 ].Value <<= it->second;
        //write the data to the registry
        SetSetProperties(SETNODE_BINDINGS,seqValues);
    }
}

//  private method

void GlobalEventConfig_Impl::initBindingInfo()
{
    // Get ALL names of current existing list items in configuration!
    Sequence< OUString > lEventNames      = GetNodeNames( SETNODE_BINDINGS, utl::CONFIG_NAME_LOCAL_PATH );

    OUString aSetNode( SETNODE_BINDINGS );
    aSetNode += PATHDELIMITER;

    OUString aCommandKey( PATHDELIMITER );
    aCommandKey += PROPERTYNAME_BINDINGURL;

    // Expand all keys
    Sequence< OUString > lMacros(1);
    for (sal_Int32 i=0; i<lEventNames.getLength(); ++i )
    {
        OUStringBuffer aBuffer( 32 );
        aBuffer.append( aSetNode );
        aBuffer.append( lEventNames[i] );
        aBuffer.append( aCommandKey );
        lMacros[0] = aBuffer.makeStringAndClear();
        OSL_TRACE("reading binding for: %s",OUStringToOString(lMacros[0] , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        Sequence< Any > lValues = GetProperties( lMacros );
        OUString sMacroURL;
        if( lValues.getLength() > 0 )
        {
            lValues[0] >>= sMacroURL;
            sal_Int32 startIndex = lEventNames[i].indexOf('\'');
            sal_Int32 endIndex =  lEventNames[i].lastIndexOf('\'');
            if( startIndex >=0 && endIndex > 0 )
            {
                startIndex++;
                OUString eventName = lEventNames[i].copy(startIndex,endIndex-startIndex);
                m_eventBindingHash[ eventName ] = sMacroURL;
            }
        }
    }
}

void SAL_CALL GlobalEventConfig_Impl::replaceByName( const OUString& aName, const Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    Sequence< beans::PropertyValue > props;
    //DF should we prepopulate the hash with a list of valid event Names?
    if( !( aElement >>= props ) )
    {
        throw lang::IllegalArgumentException( OUString(),
                Reference< XInterface > (), 2);
    }
    OUString macroURL;
    sal_Int32 nPropCount = props.getLength();
    for( sal_Int32 index = 0; index < nPropCount; ++index )
    {
        if ( props[ index ].Name == "Script" )
            props[ index ].Value >>= macroURL;
    }
    m_eventBindingHash[ aName ] = macroURL;
    SetModified();
}

Any SAL_CALL GlobalEventConfig_Impl::getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    Any aRet;
    Sequence< beans::PropertyValue > props(2);
    props[0].Name = "EventType";
    props[0].Value <<= OUString("Script");
    props[1].Name = "Script";
    EventBindingHash::const_iterator it = m_eventBindingHash.find( aName );
    if( it != m_eventBindingHash.end() )
    {
        props[1].Value <<= it->second;
    }
    else
    {
        // not yet accessed - is it a supported name?
        SupportedEventsVector::iterator pos = ::std::find(
            m_supportedEvents.begin(), m_supportedEvents.end(), aName );
        if ( pos == m_supportedEvents.end() )
            throw container::NoSuchElementException( aName );

        props[1].Value <<= OUString();
    }
    aRet <<= props;
    return aRet;
}

Sequence< OUString > SAL_CALL GlobalEventConfig_Impl::getElementNames(  ) throw (RuntimeException)
{
    return uno::Sequence< OUString >(m_supportedEvents.data(), SupportedEventsVector::size());
}

bool SAL_CALL GlobalEventConfig_Impl::hasByName( const OUString& aName ) throw (RuntimeException)
{
    if ( m_eventBindingHash.find( aName ) != m_eventBindingHash.end() )
        return true;

    // never accessed before - is it supported in general?
    SupportedEventsVector::iterator pos = ::std::find(
        m_supportedEvents.begin(), m_supportedEvents.end(), aName );
    if ( pos != m_supportedEvents.end() )
        return true;

    return false;
}

Type SAL_CALL GlobalEventConfig_Impl::getElementType(  ) throw (RuntimeException)
{
    //DF definitely not sure about this??
    return cppu::UnoType<Sequence<beans::PropertyValue>>::get();
}

bool SAL_CALL GlobalEventConfig_Impl::hasElements(  ) throw (RuntimeException)
{
    return ( m_eventBindingHash.empty() );
}

// and now the wrapper

//initialize static member
GlobalEventConfig_Impl*     GlobalEventConfig::m_pImpl = nullptr;
sal_Int32                   GlobalEventConfig::m_nRefCount      = 0;

GlobalEventConfig::GlobalEventConfig()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pImpl == nullptr )
    {
        m_pImpl = new GlobalEventConfig_Impl;
        ItemHolder1::holdConfigItem(E_EVENTCFG);
    }
}

GlobalEventConfig::~GlobalEventConfig()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease our refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

Reference< container::XNameReplace > SAL_CALL GlobalEventConfig::getEvents() throw (css::uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    Reference< container::XNameReplace > ret(this);
    return ret;
}

void SAL_CALL GlobalEventConfig::replaceByName( const OUString& aName, const Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->replaceByName( aName, aElement );
}
Any SAL_CALL GlobalEventConfig::getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->getByName( aName );
}
Sequence< OUString > SAL_CALL GlobalEventConfig::getElementNames(  ) throw (RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->getElementNames( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasByName( const OUString& aName ) throw (RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->hasByName( aName );
}
Type SAL_CALL GlobalEventConfig::getElementType(  ) throw (RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return GlobalEventConfig_Impl::getElementType( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasElements(  ) throw (RuntimeException, std::exception)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->hasElements( );
}

namespace
{
    class theGlobalEventConfigMutex : public rtl::Static<osl::Mutex, theGlobalEventConfigMutex>{};
}

Mutex& GlobalEventConfig::GetOwnStaticMutex()
{
    return theGlobalEventConfigMutex::get();
}

OUString GlobalEventConfig::GetEventName( GlobalEventId nIndex )
{
    if (utl::ConfigManager::IsAvoidConfig())
        return OUString();
    return GlobalEventConfig().m_pImpl->GetEventName( nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
