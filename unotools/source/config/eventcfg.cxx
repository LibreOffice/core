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

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <unotools/eventcfg.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include "itemholder1.hxx"

#include <algorithm>
#include <unordered_map>

using namespace ::std;
using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

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

typedef std::unordered_map< OUString, OUString > EventBindingHash;
typedef o3tl::enumarray< GlobalEventId, OUString > SupportedEventsVector;

static std::mutex& GetOwnStaticMutex()
{
    static std::mutex INSTANCE;
    return INSTANCE;
}

class GlobalEventConfig_Impl : public utl::ConfigItem
{
private:
    EventBindingHash m_eventBindingHash;
    SupportedEventsVector m_supportedEvents;

    void initBindingInfo();

    virtual void ImplCommit() override;

public:
    GlobalEventConfig_Impl( );
    virtual ~GlobalEventConfig_Impl( ) override;

    void            Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::NoSuchElementException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void replaceByName( const OUString& aName, const css::uno::Any& aElement );
    /// @throws css::container::NoSuchElementException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence < css::beans::PropertyValue > getByName( const OUString& aName );
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getElementNames(  );
    /// @throws css::uno::RuntimeException
    bool hasByName( const OUString& aName );
    /// @throws css::uno::RuntimeException
    static css::uno::Type const & getElementType(  );
    /// @throws css::uno::RuntimeException
    bool hasElements() const;
    OUString const & GetEventName( GlobalEventId nID ) const;
};


GlobalEventConfig_Impl::GlobalEventConfig_Impl()
    :   ConfigItem( "Office.Events/ApplicationEvents", ConfigItemMode::NONE )
{
    // the supported event names
    for (const GlobalEventId id : o3tl::enumrange<GlobalEventId>())
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

OUString const & GlobalEventConfig_Impl::GetEventName( GlobalEventId nIndex ) const
{
    return m_supportedEvents[nIndex];
}

//  public method

void GlobalEventConfig_Impl::Notify( const Sequence< OUString >& )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );

    initBindingInfo();
}

//  public method

void GlobalEventConfig_Impl::ImplCommit()
{
    //DF need to check it this is correct??
    SAL_INFO("unotools", "In GlobalEventConfig_Impl::ImplCommit");
    // clear the existing nodes
    ClearNodeSet( SETNODE_BINDINGS );
    OUString sNode;
    //step through the list of events
    for(const auto& rEntry : m_eventBindingHash)
    {
        //no point in writing out empty bindings!
        if(rEntry.second.isEmpty() )
            continue;
        sNode = SETNODE_BINDINGS PATHDELIMITER "BindingType['" +
                rEntry.first +
                "']" PATHDELIMITER PROPERTYNAME_BINDINGURL;
        SAL_INFO("unotools", "writing binding for: " << sNode);
        //write the data to the registry
        SetSetProperties(SETNODE_BINDINGS,{ comphelper::makePropertyValue(sNode, rEntry.second) });
    }
}

//  private method

void GlobalEventConfig_Impl::initBindingInfo()
{
    // Get ALL names of current existing list items in configuration!
    const Sequence< OUString > lEventNames = GetNodeNames( SETNODE_BINDINGS, utl::ConfigNameFormat::LocalPath );

    OUString aSetNode = SETNODE_BINDINGS PATHDELIMITER;
    OUString aCommandKey = PATHDELIMITER PROPERTYNAME_BINDINGURL;

    // Expand all keys
    Sequence< OUString > lMacros(1);
    auto plMacros = lMacros.getArray();
    for (const auto& rEventName : lEventNames )
    {
        plMacros[0] = aSetNode + rEventName + aCommandKey;
        SAL_INFO("unotools", "reading binding for: " << lMacros[0]);
        Sequence< Any > lValues = GetProperties( lMacros );
        if( lValues.hasElements() )
        {
            OUString sMacroURL;
            lValues[0] >>= sMacroURL;
            sal_Int32 startIndex = rEventName.indexOf('\'');
            sal_Int32 endIndex =  rEventName.lastIndexOf('\'');
            if( startIndex >=0 && endIndex > 0 )
            {
                startIndex++;
                OUString eventName = rEventName.copy(startIndex,endIndex-startIndex);
                m_eventBindingHash[ eventName ] = sMacroURL;
            }
        }
    }
}

void GlobalEventConfig_Impl::replaceByName( const OUString& aName, const Any& aElement )
{
    Sequence< beans::PropertyValue > props;
    //DF should we prepopulate the hash with a list of valid event Names?
    if( !( aElement >>= props ) )
    {
        throw lang::IllegalArgumentException( OUString(),
                Reference< XInterface > (), 2);
    }
    OUString macroURL;
    for( const auto& rProp : std::as_const(props) )
    {
        if ( rProp.Name == "Script" )
            rProp.Value >>= macroURL;
    }
    m_eventBindingHash[ aName ] = macroURL;
    SetModified();
}

css::uno::Sequence < css::beans::PropertyValue > GlobalEventConfig_Impl::getByName( const OUString& aName )
{
    static constexpr OUStringLiteral sEventType = u"EventType";
    static constexpr OUStringLiteral sScript = u"Script";
    Sequence< beans::PropertyValue > props(2);
    auto pProps = props.getArray();
    pProps[0].Name = sEventType;
    pProps[0].Value <<= OUString(sScript);
    pProps[1].Name = sScript;
    EventBindingHash::const_iterator it = m_eventBindingHash.find( aName );
    if( it != m_eventBindingHash.end() )
    {
        pProps[1].Value <<= it->second;
    }
    else
    {
        // not yet accessed - is it a supported name?
        SupportedEventsVector::iterator pos = ::std::find(
            m_supportedEvents.begin(), m_supportedEvents.end(), aName );
        if ( pos == m_supportedEvents.end() )
            throw container::NoSuchElementException( aName );

        pProps[1].Value <<= OUString();
    }
    return props;
}

Sequence< OUString > GlobalEventConfig_Impl::getElementNames(  )
{
    return uno::Sequence< OUString >(m_supportedEvents.data(), SupportedEventsVector::size());
}

bool GlobalEventConfig_Impl::hasByName( const OUString& aName )
{
    if ( m_eventBindingHash.find( aName ) != m_eventBindingHash.end() )
        return true;

    // never accessed before - is it supported in general?
    SupportedEventsVector::iterator pos = ::std::find(
        m_supportedEvents.begin(), m_supportedEvents.end(), aName );
    return pos != m_supportedEvents.end();
}

Type const & GlobalEventConfig_Impl::getElementType(  )
{
    //DF definitely not sure about this??
    return cppu::UnoType<Sequence<beans::PropertyValue>>::get();
}

bool GlobalEventConfig_Impl::hasElements() const
{
    return !m_eventBindingHash.empty();
}

// and now the wrapper

//initialize static member
GlobalEventConfig_Impl*     GlobalEventConfig::m_pImpl = nullptr;
sal_Int32                   GlobalEventConfig::m_nRefCount      = 0;

GlobalEventConfig::GlobalEventConfig()
{
    // Global access, must be guarded (multithreading!).
    std::unique_lock aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pImpl == nullptr )
    {
        m_pImpl = new GlobalEventConfig_Impl;
        aGuard.unlock();
        ItemHolder1::holdConfigItem(EItem::EventConfig);
    }
}

GlobalEventConfig::~GlobalEventConfig()
{
    // Global access, must be guarded (multithreading!)
    std::unique_lock aGuard( GetOwnStaticMutex() );
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

Reference< container::XNameReplace > SAL_CALL GlobalEventConfig::getEvents()
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    Reference< container::XNameReplace > ret(this);
    return ret;
}

void SAL_CALL GlobalEventConfig::replaceByName( const OUString& aName, const Any& aElement )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    m_pImpl->replaceByName( aName, aElement );
}
Any SAL_CALL GlobalEventConfig::getByName( const OUString& aName )
{
    return Any(getByName2(aName));
}
css::uno::Sequence < css::beans::PropertyValue > GlobalEventConfig::getByName2( const OUString& aName )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->getByName( aName );
}
Sequence< OUString > SAL_CALL GlobalEventConfig::getElementNames(  )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->getElementNames( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasByName( const OUString& aName )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->hasByName( aName );
}
Type SAL_CALL GlobalEventConfig::getElementType(  )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return GlobalEventConfig_Impl::getElementType( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasElements(  )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->hasElements( );
}

OUString GlobalEventConfig::GetEventName( GlobalEventId nIndex )
{
    if (utl::ConfigManager::IsFuzzing())
        return OUString();
    static rtl::Reference<GlobalEventConfig> createImpl(new GlobalEventConfig);
    return GlobalEventConfig::m_pImpl->GetEventName( nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
