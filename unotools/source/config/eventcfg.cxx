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

#include <boost/unordered_map.hpp>
#include <rtl/ustrbuf.hxx>

#include <itemholder1.hxx>

#include <algorithm>

using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

#define ROOTNODE_EVENTS OUString("Office.Events/ApplicationEvents")
#define PATHDELIMITER OUString("/")
#define SETNODE_BINDINGS OUString("Bindings")
#define PROPERTYNAME_BINDINGURL OUString("BindingURL")

const char* pEventAsciiNames[] =
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

GlobalEventConfig_Impl::GlobalEventConfig_Impl()
    :   ConfigItem( ROOTNODE_EVENTS, CONFIG_MODE_IMMEDIATE_UPDATE )
{
    // the supported event names
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_STARTAPP] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_CLOSEAPP] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_DOCCREATED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_CREATEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_LOADFINISHED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_OPENDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_PREPARECLOSEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_CLOSEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEDOCDONE] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEDOCFAILED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEASDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEASDOCDONE] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVEASDOCFAILED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVETODOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVETODOCDONE] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_SAVETODOCFAILED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_ACTIVATEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_DEACTIVATEDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_PRINTDOC] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_VIEWCREATED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_PREPARECLOSEVIEW] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_CLOSEVIEW] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_MODIFYCHANGED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_TITLECHANGED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_VISAREACHANGED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_MODECHANGED] ) );
    m_supportedEvents.push_back(OUString::createFromAscii( pEventAsciiNames[STR_EVENT_STORAGECHANGED] ) );

    initBindingInfo();

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys! */
    Sequence< OUString > aNotifySeq( 1 );
    aNotifySeq[0] = OUString( "Events" );
    EnableNotification( aNotifySeq, sal_True );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
GlobalEventConfig_Impl::~GlobalEventConfig_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

::rtl::OUString GlobalEventConfig_Impl::GetEventName( sal_Int32 nIndex )
{
    if ( nIndex < (sal_Int32) m_supportedEvents.size() )
        return m_supportedEvents[nIndex];
    else
        return rtl::OUString();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void GlobalEventConfig_Impl::Notify( const Sequence< OUString >& )
{
    MutexGuard aGuard( GlobalEventConfig::GetOwnStaticMutex() );

    initBindingInfo();

    // dont forget to update all existing frames and her might cached dispatch objects!
    // But look for already killed frames. We hold weak references instead of hard ones ...
    for (FrameVector::const_iterator pIt  = m_lFrames.begin();
                                        pIt != m_lFrames.end()  ;
                                      ++pIt                     )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame(pIt->get(), ::com::sun::star::uno::UNO_QUERY);
        if (xFrame.is())
            xFrame->contextChanged();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void GlobalEventConfig_Impl::Commit()
{
    //DF need to check it this is correct??
    OSL_TRACE("In GlobalEventConfig_Impl::Commit");
    EventBindingHash::const_iterator it = m_eventBindingHash.begin();
    EventBindingHash::const_iterator it_end = m_eventBindingHash.end();
    // clear the existing nodes
    ClearNodeSet( SETNODE_BINDINGS );
    Sequence< beans::PropertyValue > seqValues( 1 );
    OUString sNode;
    static const OUString sPrefix(SETNODE_BINDINGS + PATHDELIMITER + OUString("BindingType['"));
    static const OUString sPostfix(OUString("']") + PATHDELIMITER + PROPERTYNAME_BINDINGURL);
    //step through the list of events
    for(int i=0;it!=it_end;++it,++i)
    {
        //no point in writing out empty bindings!
        if(it->second.isEmpty() )
            continue;
        sNode = sPrefix + it->first + sPostfix;
        OSL_TRACE("writing binding for: %s",::rtl::OUStringToOString(sNode , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        seqValues[ 0 ].Name = sNode;
        seqValues[ 0 ].Value <<= it->second;
        //write the data to the registry
        SetSetProperties(SETNODE_BINDINGS,seqValues);
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
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
        OSL_TRACE("reading binding for: %s",::rtl::OUStringToOString(lMacros[0] , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
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
    if( sal_False == ( aElement >>= props ) )
    {
        throw lang::IllegalArgumentException( OUString(),
                Reference< XInterface > (), 2);
    }
    OUString macroURL;
    sal_Int32 nPropCount = props.getLength();
    for( sal_Int32 index = 0 ; index < nPropCount ; ++index )
    {
        if ( props[ index ].Name.compareToAscii( "Script" ) == 0 )
            props[ index ].Value >>= macroURL;
    }
    m_eventBindingHash[ aName ] = macroURL;
    SetModified();
}

Any SAL_CALL GlobalEventConfig_Impl::getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    Any aRet;
    Sequence< beans::PropertyValue > props(2);
    props[0].Name = OUString("EventType");
    props[0].Value <<= OUString("Script");
    props[1].Name = OUString("Script");
    EventBindingHash::const_iterator it = m_eventBindingHash.find( aName );
    if( it != m_eventBindingHash.end() )
    {
        props[1].Value <<= it->second;
    }
    else
    {
        // not yet accessed - is it a supported name?
        SupportedEventsVector::const_iterator pos = ::std::find(
            m_supportedEvents.begin(), m_supportedEvents.end(), aName );
        if ( pos == m_supportedEvents.end() )
            throw container::NoSuchElementException( aName, NULL );

        props[1].Value <<= OUString();
    }
    aRet <<= props;
    return aRet;
}

Sequence< OUString > SAL_CALL GlobalEventConfig_Impl::getElementNames(  ) throw (RuntimeException)
{
    const ::rtl::OUString* pRet = m_supportedEvents.empty() ? NULL : &m_supportedEvents[0];
    return uno::Sequence< ::rtl::OUString >(pRet, m_supportedEvents.size());
}

sal_Bool SAL_CALL GlobalEventConfig_Impl::hasByName( const OUString& aName ) throw (RuntimeException)
{
    if ( m_eventBindingHash.find( aName ) != m_eventBindingHash.end() )
        return sal_True;

    // never accessed before - is it supported in general?
    SupportedEventsVector::const_iterator pos = ::std::find(
        m_supportedEvents.begin(), m_supportedEvents.end(), aName );
    if ( pos != m_supportedEvents.end() )
        return sal_True;

    return sal_False;
}

Type SAL_CALL GlobalEventConfig_Impl::getElementType(  ) throw (RuntimeException)
{
    //DF definitly not sure about this??
    return ::getCppuType((const Sequence<beans::PropertyValue>*)0);
}

sal_Bool SAL_CALL GlobalEventConfig_Impl::hasElements(  ) throw (RuntimeException)
{
    return ( m_eventBindingHash.empty() );
}

// and now the wrapper


//initialize static member
GlobalEventConfig_Impl*     GlobalEventConfig::m_pImpl = NULL  ;
sal_Int32                   GlobalEventConfig::m_nRefCount      = 0     ;

GlobalEventConfig::GlobalEventConfig()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pImpl == NULL )
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
        m_pImpl = NULL;
    }
}

Reference< container::XNameReplace > SAL_CALL GlobalEventConfig::getEvents() throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    Reference< container::XNameReplace > ret(this);
    return ret;
}

void SAL_CALL GlobalEventConfig::replaceByName( const OUString& aName, const Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->replaceByName( aName, aElement );
}
Any SAL_CALL GlobalEventConfig::getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->getByName( aName );
}
Sequence< OUString > SAL_CALL GlobalEventConfig::getElementNames(  ) throw (RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->getElementNames( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasByName( const OUString& aName ) throw (RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->hasByName( aName );
}
Type SAL_CALL GlobalEventConfig::getElementType(  ) throw (RuntimeException)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->getElementType( );
}
sal_Bool SAL_CALL GlobalEventConfig::hasElements(  ) throw (RuntimeException)
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

::rtl::OUString GlobalEventConfig::GetEventName( sal_Int32 nIndex )
{
    return GlobalEventConfig().m_pImpl->GetEventName( nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
