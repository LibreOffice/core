/*************************************************************************
 *
 *  $RCSfile: eventcfg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:12:02 $
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
7 ************************************************************************/

#pragma hdrstop

#include "eventcfg.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <algorithm>

using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

#define ROOTNODE_EVENTS OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Events/ApplicationEvents" ))
#define PATHDELIMITER OUString(RTL_CONSTASCII_USTRINGPARAM("/"))
#define SETNODE_BINDINGS OUString(RTL_CONSTASCII_USTRINGPARAM("Bindings" ))
#define PROPERTYNAME_BINDINGURL OUString(RTL_CONSTASCII_USTRINGPARAM("BindingURL"))

GlobalEventConfig_Impl::GlobalEventConfig_Impl()
    :   ConfigItem( ROOTNODE_EVENTS, CONFIG_MODE_IMMEDIATE_UPDATE )
{
    // the supported event names
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnStartApp"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnCloseApp"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnNew"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnUnload"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnPrepareUnload"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnLoad"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnSave"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnSaveAs"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnSaveDone"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnSaveAsDone"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnFocus"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnUnfocus"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnPrint"));
    m_supportedEvents.push_back(::rtl::OUString::createFromAscii("OnModifyChanged"));

    initBindingInfo();

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys! */
    Sequence< OUString > aNotifySeq( 1 );
    aNotifySeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Events" ));
    sal_Bool bEnabled  = EnableNotification( aNotifySeq, sal_True );
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

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void GlobalEventConfig_Impl::Notify( const Sequence< OUString >& lPropertyNames )
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
    sal_Int32 nBindingsCount = m_eventBindingHash.size();
    EventBindingHash::const_iterator it = m_eventBindingHash.begin();
    EventBindingHash::const_iterator it_end = m_eventBindingHash.end();
    // clear the existing nodes
    ClearNodeSet( SETNODE_BINDINGS );
    Sequence< beans::PropertyValue > seqValues( 1 );
    OUString sNode;
    //step through the list of events
    for(int i=0;it!=it_end;++it,++i)
    {
        //no point in writing out empty bindings!
        if(it->second.getLength() == 0 )
            continue;
        sNode = SETNODE_BINDINGS + PATHDELIMITER + OUString::createFromAscii("BindingType['") + it->first + OUString::createFromAscii("']") + PATHDELIMITER + PROPERTYNAME_BINDINGURL;
        OSL_TRACE("writing binding for: %s",::rtl::OUStringToOString(sNode , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        seqValues[ 0 ].Name = sNode;
        seqValues[ 0 ].Value <<= it->second;
        //write the data to the registry
        SetSetProperties(SETNODE_BINDINGS,seqValues);
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void GlobalEventConfig_Impl::EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    // check if frame already exists inside list
    // ignore double registrations
    // every frame must be notified one times only!
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > xWeak(xFrame);
    FrameVector::const_iterator pIt = ::std::find(m_lFrames.begin(), m_lFrames.end(), xWeak);
    if (pIt == m_lFrames.end())
        m_lFrames.push_back(xWeak);
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

Reference< container::XNameReplace > SAL_CALL GlobalEventConfig_Impl::getEvents() throw (::com::sun::star::uno::RuntimeException)
{
    //how to return this object as an XNameReplace?
    Reference< container::XNameReplace > ret;
    return ret;
}

void SAL_CALL GlobalEventConfig_Impl::replaceByName( const OUString& aName, const Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    Sequence< beans::PropertyValue > props;
    //DF should we prepopulate the hash with a list of valid event Names?
    if( sal_False == ( aElement >>= props ) )
    {
        throw lang::IllegalArgumentException( OUString::createFromAscii(""),
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
    props[0].Name = OUString::createFromAscii("EventType");
    props[0].Value <<= OUString::createFromAscii("Script");
    props[1].Name = OUString::createFromAscii("Script");
    EventBindingHash::const_iterator it = m_eventBindingHash.find( aName );
    if( it != m_eventBindingHash.end() )
    {
        props[1].Value <<= it->second;
    }
    else
    {
        SupportedEventsVector::const_iterator vit = m_supportedEvents.begin();
        SupportedEventsVector::const_iterator vit_end = m_supportedEvents.end();
        for(;vit!=vit_end;++vit)
        {
            if(vit->equals(aName))
            {
                props[1].Value <<= OUString::createFromAscii("");
                break;
            }
        }
        if(vit==vit_end)
            throw container::NoSuchElementException( OUString::createFromAscii("No such element in event configuration"),
                Reference< XInterface > () );
    }
    aRet <<= props;
    return aRet;
}

Sequence< OUString > SAL_CALL GlobalEventConfig_Impl::getElementNames(  ) throw (RuntimeException)
{
    Sequence< OUString > ret(m_supportedEvents.size());
    SupportedEventsVector::const_iterator it = m_supportedEvents.begin();
    SupportedEventsVector::const_iterator it_end = m_supportedEvents.end();
    for(int i=0;it!=it_end;++it,++i)
    {
        ret[ i ] = *it;
    }
    return ret;
}

sal_Bool SAL_CALL GlobalEventConfig_Impl::hasByName( const OUString& aName ) throw (RuntimeException)
{
    return ( m_eventBindingHash.find( aName ) != m_eventBindingHash.end() );
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

void GlobalEventConfig::EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->EstablishFrameCallback( xFrame );
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

Mutex& GlobalEventConfig::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect following code with
        // the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that
        // another instance of our class will be faster then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}
