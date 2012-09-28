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


#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "InterfaceContainer.hxx"
#include "componenttools.hxx"
#include "property.hrc"
#include "services.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/form/XForm.hpp>

#include <comphelper/container.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/eventattachermgr.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <rtl/logfile.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <memory>

//.........................................................................
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <ooo/vba/XVBAToOOEventDescGen.hpp>
#include <comphelper/processfactory.hxx>

namespace frm
{
//.........................................................................

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;

namespace
{
    //---------------------------------------------------------------------
    static void lcl_throwIllegalArgumentException()
    {
        throw IllegalArgumentException();
    }
}

bool
lcl_hasVbaEvents( const Sequence< ScriptEventDescriptor >& sEvents  )
{
    const ScriptEventDescriptor* pDesc = sEvents.getConstArray();
    const ScriptEventDescriptor* pEnd = ( pDesc + sEvents.getLength() );
    for ( ; pDesc != pEnd; ++pDesc )
    {
        if ( pDesc->ScriptType == "VBAInterop" )
            return true;
    }
    return false;
}

Sequence< ScriptEventDescriptor >
lcl_stripVbaEvents( const Sequence< ScriptEventDescriptor >& sEvents )
{
    Sequence< ScriptEventDescriptor > sStripped( sEvents.getLength() );

    const ScriptEventDescriptor* pDesc = sEvents.getConstArray();
    const ScriptEventDescriptor* pEnd = ( pDesc + sEvents.getLength() );
    sal_Int32 nCopied = 0;
    for ( ; pDesc != pEnd; ++pDesc )
    {
        if ( pDesc->ScriptType != "VBAInterop" )
        {
            sStripped[ nCopied++ ] = *pDesc;
        }
    }
    if ( nCopied )
        sStripped.realloc( nCopied );
    return sStripped;
}

void OInterfaceContainer::impl_addVbEvents_nolck_nothrow(  const sal_Int32 i_nIndex )
{
    // we are dealing with form controls
    try
    {
        do
        {
            Reference< XModel > xDoc( getXModel( static_cast< XContainer *> ( this ) ) );
            if ( !xDoc.is() )
                break;

            Reference< XMultiServiceFactory > xDocFac( xDoc, UNO_QUERY_THROW );
            Reference< XCodeNameQuery > xNameQuery( xDocFac->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooo.vba.VBACodeNameProvider") ) ), UNO_QUERY );
            if ( !xNameQuery.is() )
                break;

            ::osl::MutexGuard aGuard( m_rMutex );
            bool hasVBABindings = lcl_hasVbaEvents( m_xEventAttacher->getScriptEvents( i_nIndex ) );
            if ( hasVBABindings )
                break;

            Reference< XInterface > xElement( getByIndex( i_nIndex ) , UNO_QUERY_THROW );
            Reference< XForm > xElementAsForm( xElement, UNO_QUERY );
            if ( xElementAsForm.is() )
                break;

            // Try getting the code name from the container first (faster),
            // then from the element if that fails (slower).
            Reference<XInterface> xThis = static_cast<XContainer*>(this);
            rtl::OUString sCodeName = xNameQuery->getCodeNameForContainer(xThis);
            if (sCodeName.isEmpty())
                sCodeName = xNameQuery->getCodeNameForObject(xElement);

            Reference< XPropertySet > xProps( xElement, UNO_QUERY_THROW );
            ::rtl::OUString sServiceName;
            xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultControl") ) ) >>= sServiceName;

            Reference< ooo::vba::XVBAToOOEventDescGen > xDescSupplier( m_xServiceFactory->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooo.vba.VBAToOOEventDesc") ) ), UNO_QUERY_THROW );
            Sequence< ScriptEventDescriptor > vbaEvents = xDescSupplier->getEventDescriptions( m_xServiceFactory->createInstance( sServiceName ), sCodeName );
            // register the vba script events
            m_xEventAttacher->registerScriptEvents( i_nIndex, vbaEvents );
        }
        while ( false );
    }
    catch ( const ServiceNotRegisteredException& )
    {
        // silence this, not all document types support the ooo.vba.VBACodeNameProvider service
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

}
//==================================================================
//= ElementDescription
//==================================================================
//------------------------------------------------------------------
ElementDescription::ElementDescription( )
{
}

//------------------------------------------------------------------
ElementDescription::~ElementDescription()
{
}

//==================================================================
//= OInterfaceContainer
//==================================================================
//------------------------------------------------------------------
OInterfaceContainer::OInterfaceContainer(
                const Reference<XMultiServiceFactory>& _rxFactory,
                ::osl::Mutex& _rMutex,
                const Type& _rElementType)
    :OInterfaceContainer_BASE()
    ,m_rMutex(_rMutex)
    ,m_aContainerListeners(_rMutex)
    ,m_aElementType(_rElementType)
    ,m_xServiceFactory(_rxFactory)
{
    impl_createEventAttacher_nothrow();
}

//------------------------------------------------------------------------------
OInterfaceContainer::OInterfaceContainer( ::osl::Mutex& _rMutex, const OInterfaceContainer& _cloneSource )
    :OInterfaceContainer_BASE()
    ,m_rMutex( _rMutex )
    ,m_aContainerListeners( _rMutex )
    ,m_aElementType( _cloneSource.m_aElementType )
    ,m_xServiceFactory( _cloneSource.m_xServiceFactory )
{
    impl_createEventAttacher_nothrow();
}

//------------------------------------------------------------------------------
void OInterfaceContainer::clonedFrom( const OInterfaceContainer& _cloneSource )
{
    try
    {
        const Reference< XIndexAccess > xSourceHierarchy( const_cast< OInterfaceContainer* >( &_cloneSource ) );
        const sal_Int32 nCount = xSourceHierarchy->getCount();
        for ( sal_Int32 i=0; i<nCount; ++i )
        {
            Reference< XCloneable > xCloneable( xSourceHierarchy->getByIndex( i ), UNO_QUERY_THROW );
            Reference< XInterface > xClone( xCloneable->createClone() );
            insertByIndex( i, makeAny( xClone ) );
        }
    }
    catch( const Exception& )
    {
        throw WrappedTargetException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Could not clone the given interface hierarchy." ) ),
            static_cast< XIndexContainer* >( const_cast< OInterfaceContainer* >( &_cloneSource ) ),
            ::cppu::getCaughtException()
        );
    }
}

//------------------------------------------------------------------------------
void OInterfaceContainer::impl_createEventAttacher_nothrow()
{
    try
    {
        m_xEventAttacher.set( ::comphelper::createEventAttacherManager( m_xServiceFactory ), UNO_SET_THROW );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
OInterfaceContainer::~OInterfaceContainer()
{
}

//------------------------------------------------------------------------------
void OInterfaceContainer::disposing()
{
    // dispose all elements
    for (sal_Int32 i = m_aItems.size(); i > 0; --i)
    {
        Reference<XPropertySet>  xSet(m_aItems[i - 1], UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener(PROPERTY_NAME, this);

        // revoke event knittings
        if ( m_xEventAttacher.is() )
        {
            Reference< XInterface > xIfc( xSet, UNO_QUERY );
            m_xEventAttacher->detach( i - 1, xIfc );
            m_xEventAttacher->removeEntry( i - 1 );
        }

        Reference<XComponent>  xComponent(xSet, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
    m_aMap.clear();
    m_aItems.clear();

    EventObject aEvt(static_cast<XContainer*>(this));
    m_aContainerListeners.disposeAndClear(aEvt);
}

// XPersistObject
//------------------------------------------------------------------------------
namespace
{
    //..........................................................................
    void lcl_saveEvents( ::std::vector< Sequence< ScriptEventDescriptor > >& _rSave,
        const Reference< XEventAttacherManager >& _rxManager, const sal_Int32 _nItemCount )
    {
        OSL_ENSURE( _rxManager.is(), "lcl_saveEvents: invalid event attacher manager!" );
        if ( !_rxManager.is() )
            return;

        // reserve the space needed
        _rSave.reserve( _nItemCount );

        // copy the events
        for (sal_Int32 i=0; i<_nItemCount; ++i)
            _rSave.push_back(_rxManager->getScriptEvents( i ));
    }

    //..........................................................................
    void lcl_restoreEvents( const ::std::vector< Sequence< ScriptEventDescriptor > >& _rSave,
        const Reference< XEventAttacherManager >& _rxManager )
    {
        OSL_ENSURE( _rxManager.is(), "lcl_restoreEvents: invalid event attacher manager!" );
        if ( !_rxManager.is() )
            return;

        ::std::vector< Sequence< ScriptEventDescriptor > >::const_iterator aLoop = _rSave.begin();
        ::std::vector< Sequence< ScriptEventDescriptor > >::const_iterator aEnd = _rSave.end();
        for ( sal_Int32 i=0; aLoop != aEnd; ++aLoop, ++i )
        {
            _rxManager->revokeScriptEvents( i );
            _rxManager->registerScriptEvents( i, *aLoop );
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::writeEvents(const Reference<XObjectOutputStream>& _rxOutStream)
{
    // We're writing a document in SO 5.2 format (or even from earlier versions)
    // -> convert the events from the new runtime format to the format of the 5.2 files
    // but before, remember the current script events set for our children
    ::std::vector< Sequence< ScriptEventDescriptor > > aSave;
    if ( m_xEventAttacher.is() )
        lcl_saveEvents( aSave, m_xEventAttacher, m_aItems.size() );

    transformEvents( efVersionSO5x );

    try
    {
        Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);
        sal_Int32 nMark = xMark->createMark();

        sal_Int32 nObjLen = 0;
        _rxOutStream->writeLong(nObjLen);

        Reference<XPersistObject>  xScripts(m_xEventAttacher, UNO_QUERY);
        if (xScripts.is())
            xScripts->write(_rxOutStream);

        // feststellen der Laenge
        nObjLen = xMark->offsetToMark(nMark) - 4;
        xMark->jumpToMark(nMark);
        _rxOutStream->writeLong(nObjLen);
        xMark->jumpToFurthest();
        xMark->deleteMark(nMark);
    }
    catch( const Exception& )
    {
        // restore the events
        if ( m_xEventAttacher.is() )
            lcl_restoreEvents( aSave, m_xEventAttacher );
        throw;
    }

    // restore the events
    if ( m_xEventAttacher.is() )
        lcl_restoreEvents( aSave, m_xEventAttacher );
}

//------------------------------------------------------------------------------
struct TransformEventTo52Format : public ::std::unary_function< ScriptEventDescriptor, void >
{
    void operator()( ScriptEventDescriptor& _rDescriptor )
    {
        if ( 0 == _rDescriptor.ScriptType.compareToAscii( "StarBasic" ) )
        {   // it's a starbasic macro
            sal_Int32 nPrefixLength = _rDescriptor.ScriptCode.indexOf( ':' );
            if ( 0 <= nPrefixLength )
            {   // the macro name does not already contain a :
#ifdef DBG_UTIL
                const ::rtl::OUString sPrefix = _rDescriptor.ScriptCode.copy( 0, nPrefixLength );
                DBG_ASSERT( 0 == sPrefix.compareToAscii( "document" )
                        ||  0 == sPrefix.compareToAscii( "application" ),
                        "TransformEventTo52Format: invalid (unknown) prefix!" );
#endif
                // cut the prefix
                _rDescriptor.ScriptCode = _rDescriptor.ScriptCode.copy( nPrefixLength + 1 );
            }
        }
    }
};

//------------------------------------------------------------------------------
struct TransformEventTo60Format : public ::std::unary_function< ScriptEventDescriptor, void >
{
    void operator()( ScriptEventDescriptor& _rDescriptor )
    {
        if ( 0 == _rDescriptor.ScriptType.compareToAscii( "StarBasic" ) )
        {   // it's a starbasic macro
            if ( _rDescriptor.ScriptCode.indexOf( ':' ) < 0 )
            {   // the macro name does not already contain a :
                // -> default the type to "document"
                ::rtl::OUString sNewScriptCode( RTL_CONSTASCII_USTRINGPARAM( "document:" ) );
                sNewScriptCode += _rDescriptor.ScriptCode;
                _rDescriptor.ScriptCode = sNewScriptCode;
            }
        }
    }
};

//------------------------------------------------------------------------------
void OInterfaceContainer::transformEvents( const EventFormat _eTargetFormat )
{
    OSL_ENSURE( m_xEventAttacher.is(), "OInterfaceContainer::transformEvents: no event attacher manager!" );
    if ( !m_xEventAttacher.is() )
        return;

    try
    {
        // loop through all our children
        sal_Int32 nItems = m_aItems.size();
        Sequence< ScriptEventDescriptor > aChildEvents;

        for (sal_Int32 i=0; i<nItems; ++i)
        {
            // get the script events for this object
            aChildEvents = m_xEventAttacher->getScriptEvents( i );

            if ( aChildEvents.getLength() )
            {
                // the "iterators" for the events for this child
                ScriptEventDescriptor* pChildEvents     =                       aChildEvents.getArray();
                ScriptEventDescriptor* pChildEventsEnd  =   pChildEvents    +   aChildEvents.getLength();

                // do the transformation
                if ( efVersionSO6x == _eTargetFormat )
                    ::std::for_each( pChildEvents, pChildEventsEnd, TransformEventTo60Format() );
                else
                    ::std::for_each( pChildEvents, pChildEventsEnd, TransformEventTo52Format() );

                // revoke the script events
                m_xEventAttacher->revokeScriptEvents( i );
                // and re-register them
                m_xEventAttacher->registerScriptEvents( i, aChildEvents );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::readEvents(const Reference<XObjectInputStream>& _rxInStream)
{
    ::osl::MutexGuard aGuard( m_rMutex );

    // Scripting Info lesen
    Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);
    sal_Int32 nObjLen = _rxInStream->readLong();
    if (nObjLen)
    {
        sal_Int32 nMark = xMark->createMark();
        Reference<XPersistObject>  xObj(m_xEventAttacher, UNO_QUERY);
        if (xObj.is())
            xObj->read(_rxInStream);
        xMark->jumpToMark(nMark);
        _rxInStream->skipBytes(nObjLen);
        xMark->deleteMark(nMark);
    }

    // Attachement lesen
    if ( m_xEventAttacher.is() )
    {
        OInterfaceArray::const_iterator aAttach = m_aItems.begin();
        OInterfaceArray::const_iterator aAttachEnd = m_aItems.end();
        for ( sal_Int32 i=0; aAttach != aAttachEnd; ++aAttach, ++i )
        {
            Reference< XInterface > xAsIFace( *aAttach, UNO_QUERY );    // important to normalize this ....
            Reference< XPropertySet > xAsSet( xAsIFace, UNO_QUERY );
            m_xEventAttacher->attach( i, xAsIFace, makeAny( xAsSet ) );
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::write( const Reference< XObjectOutputStream >& _rxOutStream ) throw(IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    sal_Int32 nLen = m_aItems.size();

    // schreiben der laenge
    _rxOutStream->writeLong(nLen);

    if (nLen)
    {
        // 1. Version
        _rxOutStream->writeShort(0x0001);

        // 2. Objekte
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            Reference<XPersistObject>  xObj(m_aItems[i], UNO_QUERY);
            if (xObj.is())
                _rxOutStream->writeObject(xObj);
            else
            {
                // ::com::sun::star::chaos::Error
            }
        }

        // 3. Scripts
        writeEvents(_rxOutStream);
    }
}

//------------------------------------------------------------------------------
namespace
{
    Reference< XPersistObject > lcl_createPlaceHolder( const Reference< XMultiServiceFactory >& _rxORB )
    {
        Reference< XPersistObject > xObject( _rxORB->createInstance( FRM_COMPONENT_HIDDENCONTROL ), UNO_QUERY );
        DBG_ASSERT( xObject.is(), "lcl_createPlaceHolder: could not create a substitute for the unknown object!" );
        if ( xObject.is() )
        {
            // set some properties describing what we did
            Reference< XPropertySet > xObjProps( xObject, UNO_QUERY );
            if ( xObject.is()  )
            {
                try
                {
                    xObjProps->setPropertyValue( PROPERTY_NAME, makeAny( FRM_RES_STRING( RID_STR_CONTROL_SUBSTITUTED_NAME ) ) );
                    xObjProps->setPropertyValue( PROPERTY_TAG, makeAny( FRM_RES_STRING( RID_STR_CONTROL_SUBSTITUTED_EPXPLAIN ) ) );
                }
                catch(const Exception&)
                {
                }
            }
        }
        return xObject;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::read( const Reference< XObjectInputStream >& _rxInStream ) throw(IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );

    // after ::read the object is expected to be in the state it was when ::write was called, so we have
    // to empty ourself here
    while (getCount())
        removeByIndex(0);

    // Schreibt nur in Abhaengigkeit der Laenge
    sal_Int32 nLen = _rxInStream->readLong();

    if (nLen)
    {
        // 1. Version
        sal_uInt16 nVersion = _rxInStream->readShort(); (void)nVersion;

        // 2. Objekte
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            Reference<XPersistObject>  xObj;
            try
            {
                xObj = _rxInStream->readObject();
            }
            catch(const WrongFormatException&)
            {
                // the object could not be read
                // create a object (so the readEvents below will assign the events to the right controls)
                xObj = lcl_createPlaceHolder( m_xServiceFactory );
                if ( !xObj.is() )
                    // couldn't handle it
                    throw;
            }
            catch(const Exception&)
            {
                // unsere Map leeren
                while (!m_aItems.empty())
                    removeElementsNoEvents(0);

                // und die Exception nach aussen
                throw;
            }

            if ( xObj.is() )
            {
                Reference< XPropertySet > xElement( xObj, UNO_QUERY );
                try
                {
                    implInsert(
                        m_aItems.size(),    // position
                        xElement,           // element to insert
                        sal_False,          // no event attacher manager handling
                        NULL,               // not yet approved - let implInsert do it
                        sal_True            // fire the event
                    );
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "OInterfaceContainerHelper::read: reading succeeded, but not inserting!" );
                    // create a placeholder
                    xElement = xElement.query( lcl_createPlaceHolder( m_xServiceFactory ) );
                    if ( !xElement.is() )
                        // couldn't handle it
                        throw;
                    // insert the placeholder
                    implInsert( m_aItems.size(), xElement, sal_False, NULL, sal_True );
                }
            }
        }

        readEvents(_rxInStream);
    }
    else
    {
        try
        {
            m_xEventAttacher = ::comphelper::createEventAttacherManager( m_xServiceFactory );
            OSL_ENSURE( m_xEventAttacher.is(), "OInterfaceContainer::read: could not create an event attacher manager!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

// XContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addContainerListener(const Reference<XContainerListener>& _rxListener) throw( RuntimeException )
{
    m_aContainerListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeContainerListener(const Reference<XContainerListener>& _rxListener) throw( RuntimeException )
{
    m_aContainerListeners.removeInterface(_rxListener);
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::disposing(const EventObject& _rSource) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );

    Reference< XInterface > xSource( _rSource.Source, UNO_QUERY );
        // normalized source

    OInterfaceArray::iterator j;
    for ( j = m_aItems.begin(); j != m_aItems.end(); ++j )
    {
        DBG_ASSERT( j->get() == Reference< XInterface >( *j, UNO_QUERY ).get(),
            "OInterfaceContainer::disposing: vector element not normalized!" );

        if ( xSource.get() == j->get() )
            // found the element
            break;
    }

    if ( m_aItems.end() != j )
    {
        m_aItems.erase(j);

        // look up in, and erase from, m_aMap, too
        OInterfaceMap::iterator i = m_aMap.begin();
        while ( i != m_aMap.end() )
        {
            DBG_ASSERT( i->second.get() == Reference< XInterface >( i->second, UNO_QUERY ).get(),
                "OInterfaceContainer::disposing: map element not normalized!" );

            if ( i->second.get() == xSource.get() )
            {
                // found it
                m_aMap.erase(i);
                break;
            }

            ++i;

            DBG_ASSERT( i != m_aMap.end(), "OInterfaceContainer::disposing: inconsistency: the element was in m_aItems, but not in m_aMap!" );
        }
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void OInterfaceContainer::propertyChange(const PropertyChangeEvent& evt)
throw (::com::sun::star::uno::RuntimeException) {
    if (evt.PropertyName == PROPERTY_NAME)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OInterfaceMap::iterator i = ::std::find(m_aMap.begin(), m_aMap.end(),
            ::std::pair<const ::rtl::OUString, InterfaceRef >(::comphelper::getString(evt.OldValue),evt.Source));
        if (i != m_aMap.end())
        {
            InterfaceRef  xCorrectType((*i).second);
            m_aMap.erase(i);
            m_aMap.insert(::std::pair<const ::rtl::OUString, InterfaceRef >(::comphelper::getString(evt.NewValue),xCorrectType));
        }
    }
}

// XElementAccess
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OInterfaceContainer::hasElements() throw( RuntimeException )
{
    return !m_aMap.empty();
}

//------------------------------------------------------------------------------
Type SAL_CALL OInterfaceContainer::getElementType() throw(RuntimeException)
{
    return m_aElementType;
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference<XEnumeration> SAL_CALL OInterfaceContainer::createEnumeration() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

// XNameAccess
//------------------------------------------------------------------------------
Any SAL_CALL OInterfaceContainer::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ::std::pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);

    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    return (*aPair.first).second->queryInterface( m_aElementType );
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OInterfaceContainer::getElementNames() throw(RuntimeException)
{
    StringSequence aNameList(m_aItems.size());
    ::rtl::OUString* pStringArray = aNameList.getArray();

    for (OInterfaceMap::const_iterator i = m_aMap.begin(); i != m_aMap.end(); ++i, ++pStringArray)
    {
        *pStringArray = (*i).first;
    }
    return aNameList;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OInterfaceContainer::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
{
    ::std::pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(_rName);
    return aPair.first != aPair.second;
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OInterfaceContainer::getCount() throw( RuntimeException )
{
    return m_aItems.size();
}

//------------------------------------------------------------------------------
Any OInterfaceContainer::getByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if (_nIndex < 0 || (_nIndex >= (sal_Int32)m_aItems.size()))
        throw IndexOutOfBoundsException();

    return m_aItems[_nIndex]->queryInterface( m_aElementType );
}

//------------------------------------------------------------------------------
void OInterfaceContainer::approveNewElement( const Reference< XPropertySet >& _rxObject, ElementDescription* _pElement )
{
    // it has to be non-NULL
    if ( !_rxObject.is() )
        throw IllegalArgumentException(FRM_RES_STRING(RID_STR_NEED_NON_NULL_OBJECT), static_cast<XContainer*>(this), 1);

    // it has to support our element type interface
    Any aCorrectType = _rxObject->queryInterface( m_aElementType );
    if ( !aCorrectType.hasValue() )
        lcl_throwIllegalArgumentException();

    // it has to have a "Name" property
    if ( !hasProperty( PROPERTY_NAME, _rxObject ) )
        lcl_throwIllegalArgumentException();

    // it has to be a child, and it must not have a parent already
    Reference< XChild > xChild( _rxObject, UNO_QUERY );
    if ( !xChild.is() || xChild->getParent().is() )
    {
        lcl_throwIllegalArgumentException();
    }

    // passed all tests. cache the information we have so far
    DBG_ASSERT( _pElement, "OInterfaceContainer::approveNewElement: invalid event descriptor!" );
    if ( _pElement )
    {
        _pElement->xPropertySet = _rxObject;
        _pElement->xChild = xChild;
        _pElement->aElementTypeInterface = aCorrectType;
        _pElement->xInterface = Reference< XInterface >( _rxObject, UNO_QUERY );    // normalized XInterface
    }
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implInsert(sal_Int32 _nIndex, const Reference< XPropertySet >& _rxElement,
    sal_Bool _bEvents, ElementDescription* _pApprovalResult, sal_Bool _bFire ) throw( IllegalArgumentException )
{
    const bool bHandleEvents = _bEvents && m_xEventAttacher.is();

    // SYNCHRONIZED ----->
    ::osl::ClearableMutexGuard aGuard( m_rMutex );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ElementDescription > aAutoDeleteMetaData;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    ElementDescription* pElementMetaData = _pApprovalResult;
    if ( !pElementMetaData )
    {   // not yet approved by the caller -> do ourself
        pElementMetaData = createElementMetaData();
        DBG_ASSERT( pElementMetaData, "OInterfaceContainer::implInsert: createElementMetaData returned nonsense!" );

        // ensure that the meta data structure will be deleted later on
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        aAutoDeleteMetaData = ::std::auto_ptr< ElementDescription >( pElementMetaData );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        // will throw an exception if necessary
        approveNewElement( _rxElement, pElementMetaData );
    }


    // approveNewElement (no matter if called here or outside) has ensure that all relevant interfaces
    // exist

    // set the name, and add as change listener for the name
    ::rtl::OUString sName;
    _rxElement->getPropertyValue(PROPERTY_NAME) >>= sName;
    _rxElement->addPropertyChangeListener(PROPERTY_NAME, this);

    // insert the object into our internal structures
    if (_nIndex > (sal_Int32)m_aItems.size()) // ermitteln des tatsaechlichen Indexs
    {
        _nIndex = m_aItems.size();
        m_aItems.push_back( pElementMetaData->xInterface );
    }
    else
        m_aItems.insert( m_aItems.begin() + _nIndex, pElementMetaData->xInterface );

    m_aMap.insert( ::std::pair< const ::rtl::OUString, InterfaceRef >( sName, pElementMetaData->xInterface ) );

    // announce ourself as parent to the new element
    pElementMetaData->xChild->setParent(static_cast<XContainer*>(this));

    // handle the events
    if ( bHandleEvents )
    {
        m_xEventAttacher->insertEntry(_nIndex);
        m_xEventAttacher->attach( _nIndex, pElementMetaData->xInterface, makeAny( _rxElement ) );
    }

    // notify derived classes
    implInserted( pElementMetaData );

    aGuard.clear();
    // <----- SYNCHRONIZED

    // insert faked VBA events?
    bool bHandleVbaEvents = false;
    try
    {
        _rxElement->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("GenerateVbaEvents") ) ) >>= bHandleVbaEvents;
    }
    catch( const Exception& )
    {
    }
    if ( bHandleVbaEvents )
    {
        Reference< XEventAttacherManager > xMgr ( pElementMetaData->xInterface, UNO_QUERY );
        if ( xMgr.is() )
        {
            OInterfaceContainer* pIfcMgr = dynamic_cast< OInterfaceContainer* >( xMgr.get() );
            sal_Int32 nLen = pIfcMgr->getCount();
            for ( sal_Int32 i = 0; (i < nLen) && pIfcMgr ; ++i )
            {
                // add fake events to the control at index i
                pIfcMgr->impl_addVbEvents_nolck_nothrow( i );
            }
        }
        else
        {
            // add fake events to the control at index i
            impl_addVbEvents_nolck_nothrow( _nIndex );
        }
    }

    // fire the notification about the change
    if ( _bFire )
    {
        // notify listeners
        ContainerEvent aEvt;
        aEvt.Source   = static_cast<XContainer*>(this);
        aEvt.Accessor <<= _nIndex;
        aEvt.Element  = pElementMetaData->aElementTypeInterface;

        aGuard.clear();
        m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvt );
    }
}

//------------------------------------------------------------------------------
void OInterfaceContainer::removeElementsNoEvents(sal_Int32 nIndex)
{
    OInterfaceArray::iterator i = m_aItems.begin() + nIndex;
    InterfaceRef  xElement(*i);

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xElement) ++j;

    m_aItems.erase(i);
    m_aMap.erase(j);

    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    Reference<XChild>  xChild(xElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implInserted( const ElementDescription* /*_pElement*/ )
{
    // not inrerested in
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implRemoved( const InterfaceRef& /*_rxObject*/ )
{
    // not inrerested in
}

//------------------------------------------------------------------------------
void OInterfaceContainer::impl_replacedElement( const ContainerEvent& _rEvent, ::osl::ClearableMutexGuard& _rInstanceLock )
{
    _rInstanceLock.clear();
    m_aContainerListeners.notifyEach( &XContainerListener::elementReplaced, _rEvent );
}

// XIndexContainer
//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByIndex( sal_Int32 _nIndex, const Any& _rElement ) throw(IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    Reference< XPropertySet > xElement;
    _rElement >>= xElement;
    implInsert( _nIndex, xElement, sal_True /* event handling */ , NULL /* not yet approved */ , sal_True /* notification */ );
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implReplaceByIndex( const sal_Int32 _nIndex, const Any& _rNewElement, ::osl::ClearableMutexGuard& _rClearBeforeNotify )
{
    OSL_PRECOND( ( _nIndex >= 0 ) && ( _nIndex < (sal_Int32)m_aItems.size() ), "OInterfaceContainer::implReplaceByIndex: precondition not met (index)!" );

    // approve the new object
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ElementDescription > aElementMetaData( createElementMetaData() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    DBG_ASSERT( aElementMetaData.get(), "OInterfaceContainer::implReplaceByIndex: createElementMetaData returned nonsense!" );
    {
        Reference< XPropertySet > xElementProps;
        _rNewElement >>= xElementProps;
        approveNewElement( xElementProps, aElementMetaData.get() );
    }

    // get the old element
    InterfaceRef  xOldElement( m_aItems[ _nIndex ] );
    DBG_ASSERT( xOldElement.get() == Reference< XInterface >( xOldElement, UNO_QUERY ).get(),
        "OInterfaceContainer::implReplaceByIndex: elements should be held normalized!" );

    // locate the old element in the map
    OInterfaceMap::iterator j = m_aMap.begin();
    while ( ( j != m_aMap.end() ) && ( j->second.get() != xOldElement.get() ) )
        ++j;

    // remove event knittings
    if ( m_xEventAttacher.is() )
    {
        InterfaceRef xNormalized( xOldElement, UNO_QUERY );
        m_xEventAttacher->detach( _nIndex, xNormalized );
        m_xEventAttacher->removeEntry( _nIndex );
    }

    // don't listen for property changes anymore
    Reference<XPropertySet>  xSet( xOldElement, UNO_QUERY );
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    // give the old element a new (void) parent
    Reference<XChild>  xChild(xOldElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // remove the old one
    m_aMap.erase(j);

    // examine the new element
    ::rtl::OUString sName;
    DBG_ASSERT( aElementMetaData.get()->xPropertySet.is(), "OInterfaceContainer::implReplaceByIndex: what did approveNewElement do?" );

    aElementMetaData.get()->xPropertySet->getPropertyValue(PROPERTY_NAME) >>= sName;
    aElementMetaData.get()->xPropertySet->addPropertyChangeListener(PROPERTY_NAME, this);

    // insert the new one
    m_aMap.insert( ::std::pair<const ::rtl::OUString, InterfaceRef  >( sName, aElementMetaData.get()->xInterface ) );
    m_aItems[ _nIndex ] = aElementMetaData.get()->xInterface;

    aElementMetaData.get()->xChild->setParent(static_cast<XContainer*>(this));

    if ( m_xEventAttacher.is() )
    {
        m_xEventAttacher->insertEntry( _nIndex );
        m_xEventAttacher->attach( _nIndex, aElementMetaData.get()->xInterface, makeAny( aElementMetaData.get()->xPropertySet ) );
    }

    ContainerEvent aReplaceEvent;
    aReplaceEvent.Source   = static_cast< XContainer* >( this );
    aReplaceEvent.Accessor <<= _nIndex;
    aReplaceEvent.Element  = aElementMetaData.get()->xInterface->queryInterface( m_aElementType );
    aReplaceEvent.ReplacedElement = xOldElement->queryInterface( m_aElementType );

    impl_replacedElement( aReplaceEvent, _rClearBeforeNotify );
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implCheckIndex( const sal_Int32 _nIndex ) SAL_THROW( ( ::com::sun::star::lang::IndexOutOfBoundsException ) )
{
    if (_nIndex < 0 || _nIndex >= (sal_Int32)m_aItems.size())
        throw IndexOutOfBoundsException();
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByIndex(sal_Int32 _nIndex, const Any& Element) throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( m_rMutex );
    // check the index
    implCheckIndex( _nIndex );
    // do the replace
    implReplaceByIndex( _nIndex, Element, aGuard );
}

//------------------------------------------------------------------------------
void OInterfaceContainer::implRemoveByIndex( const sal_Int32 _nIndex, ::osl::ClearableMutexGuard& _rClearBeforeNotify )
{
    OSL_PRECOND( ( _nIndex >= 0 ) && ( _nIndex < (sal_Int32)m_aItems.size() ), "OInterfaceContainer::implRemoveByIndex: precondition not met (index)!" );

    OInterfaceArray::iterator i = m_aItems.begin() + _nIndex;
    InterfaceRef  xElement(*i);

    OInterfaceMap::iterator j = m_aMap.begin();
    while (j != m_aMap.end() && (*j).second != xElement) ++j;

    m_aItems.erase(i);
    m_aMap.erase(j);

    // remove event knittings
    if ( m_xEventAttacher.is() )
    {
        InterfaceRef xNormalized( xElement, UNO_QUERY );
        m_xEventAttacher->detach( _nIndex, xNormalized );
        m_xEventAttacher->removeEntry( _nIndex );
    }

    Reference<XPropertySet>  xSet(xElement, UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_NAME, this);

    Reference<XChild>  xChild(xElement, UNO_QUERY);
    if (xChild.is())
        xChild->setParent(InterfaceRef ());

    // notify derived classes
    implRemoved(xElement);

    // notify listeners
    ContainerEvent aEvt;
    aEvt.Source     = static_cast<XContainer*>(this);
    aEvt.Element    = xElement->queryInterface( m_aElementType );
    aEvt.Accessor   <<= _nIndex;

    _rClearBeforeNotify.clear();
    m_aContainerListeners.notifyEach( &XContainerListener::elementRemoved, aEvt );
}

//------------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( m_rMutex );
    // check the index
    implCheckIndex( _nIndex );
    // do the removal
    implRemoveByIndex( _nIndex, aGuard );
}

//------------------------------------------------------------------------
ElementDescription* OInterfaceContainer::createElementMetaData( )
{
    return new ElementDescription;
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertByName(const ::rtl::OUString& _rName, const Any& _rElement) throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
{
    Reference< XPropertySet > xElementProps;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ElementDescription > aElementMetaData( createElementMetaData() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    DBG_ASSERT( aElementMetaData.get(), "OInterfaceContainer::insertByName: createElementMetaData returned nonsense!" );

    // ensure the correct name of the element
    try
    {
        _rElement >>= xElementProps;
        approveNewElement( xElementProps, aElementMetaData.get() );

        xElementProps->setPropertyValue( PROPERTY_NAME, makeAny( _rName ) );
    }
    catch( const IllegalArgumentException& )
    {
        throw;  // allowed to leave
    }
    catch( const ElementExistException& )
    {
        throw;  // allowed to leave
    }
    catch( const Exception& )
    {
        OSL_FAIL( "OInterfaceContainer::insertByName: caught an exception!" );
    }
    implInsert( m_aItems.size(), xElementProps, sal_True, aElementMetaData.get(), sal_True );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::replaceByName(const ::rtl::OUString& Name, const Any& Element) throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( m_rMutex );
    ::std::pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
        lcl_throwIllegalArgumentException();

    Reference<XPropertySet> xSet;
    Element >>= xSet;
    if (xSet.is())
    {
        if (!hasProperty(PROPERTY_NAME, xSet))
            lcl_throwIllegalArgumentException();

        xSet->setPropertyValue(PROPERTY_NAME, makeAny(Name));
    }

    // determine the element pos
    sal_Int32 nPos = ::std::find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();

    implReplaceByIndex( nPos, Element, aGuard );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeByName(const ::rtl::OUString& Name) throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutex );
    ::std::pair <OInterfaceMap::iterator,
          OInterfaceMap::iterator> aPair = m_aMap.equal_range(Name);
    if (aPair.first == aPair.second)
        throw NoSuchElementException();

    sal_Int32 nPos = ::std::find(m_aItems.begin(), m_aItems.end(), (*aPair.first).second) - m_aItems.begin();
    removeByIndex(nPos);
}


// XEventAttacherManager
//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvent( sal_Int32 nIndex, const ScriptEventDescriptor& aScriptEvent ) throw(IllegalArgumentException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_rMutex );
    if ( m_xEventAttacher.is() )
    {
        m_xEventAttacher->registerScriptEvent( nIndex, aScriptEvent );
        aGuard.clear();
        impl_addVbEvents_nolck_nothrow( nIndex ); // add fake vba events
    }
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::registerScriptEvents( sal_Int32 nIndex, const Sequence< ScriptEventDescriptor >& aScriptEvents ) throw(IllegalArgumentException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_rMutex );
    if ( m_xEventAttacher.is() )
    {
        m_xEventAttacher->registerScriptEvents( nIndex, aScriptEvents );
        aGuard.clear();
        impl_addVbEvents_nolck_nothrow( nIndex ); // add fake vba events
    }
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvent( sal_Int32 nIndex, const ::rtl::OUString& aListenerType, const ::rtl::OUString& aEventMethod, const ::rtl::OUString& aRemoveListenerParam ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->revokeScriptEvent( nIndex, aListenerType, aEventMethod, aRemoveListenerParam );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::revokeScriptEvents( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->revokeScriptEvents( nIndex );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::insertEntry( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->insertEntry( nIndex );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeEntry( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->removeEntry( nIndex );
}

//------------------------------------------------------------------------
Sequence< ScriptEventDescriptor > SAL_CALL OInterfaceContainer::getScriptEvents( sal_Int32 nIndex ) throw(IllegalArgumentException, RuntimeException)
{
    Sequence< ScriptEventDescriptor > aReturn;
    if ( m_xEventAttacher.is() )
    {
        aReturn = m_xEventAttacher->getScriptEvents( nIndex );
            if ( lcl_hasVbaEvents( aReturn ) )
            {
                aReturn = lcl_stripVbaEvents( aReturn );
            }
    }
    return aReturn;
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::attach( sal_Int32 nIndex, const Reference< XInterface >& xObject, const Any& aHelper ) throw(IllegalArgumentException, ServiceNotRegisteredException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->attach( nIndex, xObject, aHelper );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::detach( sal_Int32 nIndex, const Reference< XInterface >& xObject ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->detach( nIndex, xObject );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::addScriptListener( const Reference< XScriptListener >& xListener ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->addScriptListener( xListener );
}

//------------------------------------------------------------------------
void SAL_CALL OInterfaceContainer::removeScriptListener( const Reference< XScriptListener >& xListener ) throw(IllegalArgumentException, RuntimeException)
{
    if ( m_xEventAttacher.is() )
        m_xEventAttacher->removeScriptListener( xListener );
}

//==================================================================
//= OFormComponents
//==================================================================
//------------------------------------------------------------------------------
Any SAL_CALL OFormComponents::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    Any aReturn = OFormComponents_BASE::queryInterface(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OInterfaceContainer::queryInterface(_rType);

        if (!aReturn.hasValue())
            aReturn = FormComponentsBase::queryAggregation(_rType);
    }

    return aReturn;
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL OFormComponents::getTypes() throw(RuntimeException)
{
    return ::comphelper::concatSequences(OInterfaceContainer::getTypes(), FormComponentsBase::getTypes(), OFormComponents_BASE::getTypes());
}

//------------------------------------------------------------------------------
OFormComponents::OFormComponents(const Reference<XMultiServiceFactory>& _rxFactory)
    :FormComponentsBase( m_aMutex )
    ,OInterfaceContainer( _rxFactory, m_aMutex, XFormComponent::static_type() )
    ,OFormComponents_BASE()
{
}

//------------------------------------------------------------------------------
OFormComponents::OFormComponents( const OFormComponents& _cloneSource )
    :FormComponentsBase( m_aMutex )
    ,OInterfaceContainer( m_aMutex, _cloneSource )
    ,OFormComponents_BASE()
{
}

//------------------------------------------------------------------------------
OFormComponents::~OFormComponents()
{
    if (!FormComponentsBase::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

// OComponentHelper
//------------------------------------------------------------------------------
void OFormComponents::disposing()
{
    OInterfaceContainer::disposing();
    FormComponentsBase::disposing();
    m_xParent = NULL;
}

//XChild
//------------------------------------------------------------------------------
void OFormComponents::setParent(const InterfaceRef& Parent) throw( NoSupportException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

//------------------------------------------------------------------------------
InterfaceRef OFormComponents::getParent() throw( RuntimeException )
{
    return m_xParent;
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
