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


#include "commandcontainer.hxx"
#include "connection.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "databasecontext.hxx"
#include "databasedocument.hxx"
#include "datasource.hxx"
#include "dbastrings.hrc"
#include "ModelImpl.hxx"
#include "userinformation.hxx"
#include "sdbcoretools.hxx"

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/script/DocumentDialogLibraryContainer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/form/XLoadable.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/digest.h>
#include <sfx2/signaturestate.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <tools/errcode.hxx>
#include <tools/urlobj.hxx>
#include <unotools/sharedunocomponent.hxx>

#include <algorithm>

using namespace ::com::sun::star::document;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::script;
using namespace ::cppu;
using namespace ::osl;
using namespace ::dbtools;
using namespace ::comphelper;
namespace css = ::com::sun::star;

namespace dbaccess
{

//============================================================
//= VosMutexFacade
//============================================================
VosMutexFacade::VosMutexFacade( ::osl::Mutex& _rMutex )
    :m_rMutex( _rMutex )
{
}

void SAL_CALL VosMutexFacade::acquire()
{
    m_rMutex.acquire();
}

sal_Bool SAL_CALL VosMutexFacade::tryToAcquire()
{
    return m_rMutex.tryToAcquire();
}

void SAL_CALL VosMutexFacade::release()
{
    m_rMutex.release();
}

//============================================================
//= DocumentStorageAccess
//============================================================
DBG_NAME( DocumentStorageAccess )
class DocumentStorageAccess : public ::cppu::WeakImplHelper2<   XDocumentSubStorageSupplier
                                                            ,   XTransactionListener >
{
    typedef ::std::map< ::rtl::OUString, Reference< XStorage > >    NamedStorages;

    ::osl::Mutex        m_aMutex;
    /// all sub storages which we ever gave to the outer world
    NamedStorages       m_aExposedStorages;
    ODatabaseModelImpl* m_pModelImplementation;
    bool                m_bPropagateCommitToRoot;
    bool                m_bDisposingSubStorages;

public:
    DocumentStorageAccess( ODatabaseModelImpl& _rModelImplementation )
        :m_pModelImplementation( &_rModelImplementation )
        ,m_bPropagateCommitToRoot( true )
        ,m_bDisposingSubStorages( false )
    {
        DBG_CTOR( DocumentStorageAccess, NULL );
    }

protected:
    ~DocumentStorageAccess()
    {
        DBG_DTOR( DocumentStorageAccess, NULL );
    }

public:
    void dispose();

    // XDocumentSubStorageSupplier
    virtual Reference< XStorage > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, ::sal_Int32 _nMode ) throw (RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (IOException, RuntimeException);

    // XTransactionListener
    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    /// disposes all storages managed by this instance
    void disposeStorages();

    /// disposes all known sub storages
    void commitStorages() SAL_THROW(( IOException, RuntimeException ));

    /// commits the dedicated "database" storage
    bool commitEmbeddedStorage( bool _bPreventRootCommits );

private:
    /** opens the sub storage with the given name, in the given mode
    */
    Reference< XStorage > impl_openSubStorage_nothrow( const ::rtl::OUString& _rStorageName, sal_Int32 _nMode );

    void impl_suspendCommitPropagation()
    {
        OSL_ENSURE( m_bPropagateCommitToRoot, "DocumentStorageAccess::impl_suspendCommitPropagation: already suspended" );
        m_bPropagateCommitToRoot = false;
    }
    void impl_resumeCommitPropagation()
    {
        OSL_ENSURE( !m_bPropagateCommitToRoot, "DocumentStorageAccess::impl_resumeCommitPropagation: not suspended" );
        m_bPropagateCommitToRoot = true;
    }

};

void DocumentStorageAccess::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    for (   NamedStorages::iterator loop = m_aExposedStorages.begin();
            loop != m_aExposedStorages.end();
            ++loop
        )
    {
        try
        {
            Reference< XTransactionBroadcaster > xBroadcaster( loop->second, UNO_QUERY );
            if ( xBroadcaster.is() )
                xBroadcaster->removeTransactionListener( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    m_aExposedStorages.clear();

    m_pModelImplementation = NULL;
}

Reference< XStorage > DocumentStorageAccess::impl_openSubStorage_nothrow( const ::rtl::OUString& _rStorageName, sal_Int32 _nDesiredMode )
{
    OSL_ENSURE( !_rStorageName.isEmpty(),"ODatabaseModelImpl::impl_openSubStorage_nothrow: Invalid storage name!" );

    Reference< XStorage > xStorage;
    try
    {
        Reference< XStorage > xRootStorage( m_pModelImplementation->getOrCreateRootStorage() );
        if ( xRootStorage.is() )
        {
            sal_Int32 nRealMode = m_pModelImplementation->m_bDocumentReadOnly ? ElementModes::READ : _nDesiredMode;
            if ( nRealMode == ElementModes::READ )
            {
                Reference< XNameAccess > xSubStorageNames( xRootStorage, UNO_QUERY );
                if ( xSubStorageNames.is() && !xSubStorageNames->hasByName( _rStorageName ) )
                    return xStorage;
            }

            xStorage = xRootStorage->openStorageElement( _rStorageName, nRealMode );

            Reference< XTransactionBroadcaster > xBroad( xStorage, UNO_QUERY );
            if ( xBroad.is() )
                xBroad->addTransactionListener( this );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xStorage;
}

void DocumentStorageAccess::disposeStorages()
{
    m_bDisposingSubStorages = true;

    NamedStorages::iterator aEnd = m_aExposedStorages.end();
    for (   NamedStorages::iterator aIter = m_aExposedStorages.begin();
            aIter != aEnd ;
            ++aIter
        )
    {
        try
        {
            ::comphelper::disposeComponent( aIter->second );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    m_aExposedStorages.clear();

    m_bDisposingSubStorages = false;
}

void DocumentStorageAccess::commitStorages() SAL_THROW(( IOException, RuntimeException ))
{
    try
    {
        for (   NamedStorages::const_iterator aIter = m_aExposedStorages.begin();
                aIter != m_aExposedStorages.end();
                ++aIter
            )
        {
            tools::stor::commitStorageIfWriteable( aIter->second );
        }
    }
    catch(const WrappedTargetException&)
    {
        // WrappedTargetException not allowed to leave
        throw IOException();
    }
}

bool DocumentStorageAccess::commitEmbeddedStorage( bool _bPreventRootCommits )
{
    if ( _bPreventRootCommits )
        impl_suspendCommitPropagation();

    bool bSuccess = false;
    try
    {
        NamedStorages::const_iterator pos = m_aExposedStorages.find( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "database" ) ) );
        if ( pos != m_aExposedStorages.end() )
            bSuccess = tools::stor::commitStorageIfWriteable( pos->second );
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( _bPreventRootCommits )
        impl_resumeCommitPropagation();

    return bSuccess;

}

Reference< XStorage > SAL_CALL DocumentStorageAccess::getDocumentSubStorage( const ::rtl::OUString& aStorageName, ::sal_Int32 _nDesiredMode ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    NamedStorages::iterator pos = m_aExposedStorages.find( aStorageName );
    if ( pos == m_aExposedStorages.end() )
    {
        Reference< XStorage > xResult = impl_openSubStorage_nothrow( aStorageName, _nDesiredMode );
        pos = m_aExposedStorages.insert( NamedStorages::value_type( aStorageName, xResult ) ).first;
    }

    return pos->second;
}

Sequence< ::rtl::OUString > SAL_CALL DocumentStorageAccess::getDocumentSubStoragesNames(  ) throw (IOException, RuntimeException)
{
    Reference< XStorage > xRootStor( m_pModelImplementation->getRootStorage() );
    if ( !xRootStor.is() )
        return Sequence< ::rtl::OUString >();

    ::std::vector< ::rtl::OUString > aNames;

    Reference< XNameAccess > xNames( xRootStor, UNO_QUERY_THROW );
    Sequence< ::rtl::OUString > aElementNames( xNames->getElementNames() );
    for ( sal_Int32 i=0; i<aElementNames.getLength(); ++i )
    {
        if ( xRootStor->isStorageElement( aElementNames[i] ) )
            aNames.push_back( aElementNames[i] );
    }
    return aNames.empty()
        ?  Sequence< ::rtl::OUString >()
        :  Sequence< ::rtl::OUString >( &aNames[0], aNames.size() );
}

void SAL_CALL DocumentStorageAccess::preCommit( const css::lang::EventObject& /*aEvent*/ ) throw (Exception, RuntimeException)
{
    // not interested in
}

void SAL_CALL DocumentStorageAccess::commited( const css::lang::EventObject& aEvent ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pModelImplementation )
        m_pModelImplementation->setModified( sal_True );

    if ( m_pModelImplementation && m_bPropagateCommitToRoot )
    {
        Reference< XStorage > xStorage( aEvent.Source, UNO_QUERY );

        // check if this is the dedicated "database" sub storage
        NamedStorages::const_iterator pos = m_aExposedStorages.find( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "database" ) ) );
        if  (   ( pos != m_aExposedStorages.end() )
            &&  ( pos->second == xStorage )
            )
        {
            // if so, also commit the root storage
            m_pModelImplementation->commitRootStorage();
        }
    }
}

void SAL_CALL DocumentStorageAccess::preRevert( const css::lang::EventObject& /*aEvent*/ ) throw (Exception, RuntimeException)
{
    // not interested in
}

void SAL_CALL DocumentStorageAccess::reverted( const css::lang::EventObject& /*aEvent*/ ) throw (RuntimeException)
{
    // not interested in
}

void SAL_CALL DocumentStorageAccess::disposing( const css::lang::EventObject& Source ) throw ( RuntimeException )
{
    OSL_ENSURE( Reference< XStorage >( Source.Source, UNO_QUERY ).is(), "DocumentStorageAccess::disposing: No storage? What's this?" );

    if ( m_bDisposingSubStorages )
        return;

    for (   NamedStorages::iterator find = m_aExposedStorages.begin();
            find != m_aExposedStorages.end();
            ++find
        )
        if ( find->second == Source.Source )
        {
            m_aExposedStorages.erase( find );
            break;
        }
}

//============================================================
//= ODatabaseModelImpl
//============================================================
DBG_NAME(ODatabaseModelImpl)

ODatabaseModelImpl::ODatabaseModelImpl( const Reference< XMultiServiceFactory >& _rxFactory, ODatabaseContext& _rDBContext )
            :m_xModel()
            ,m_xDataSource()
            ,m_pStorageAccess( NULL )
            ,m_aMutex()
            ,m_aMutexFacade( m_aMutex )
            ,m_aContainer(4)
            ,m_aMacroMode( *this )
            ,m_nImposedMacroExecMode( MacroExecMode::NEVER_EXECUTE )
            ,m_pDBContext( &_rDBContext )
            ,m_refCount(0)
            ,m_aEmbeddedMacros()
            ,m_bModificationLock( false )
            ,m_bDocumentInitialized( false )
            ,m_aContext( _rxFactory )
            ,m_nLoginTimeout(0)
            ,m_bReadOnly(sal_False)
            ,m_bPasswordRequired(sal_False)
            ,m_bSuppressVersionColumns(sal_True)
            ,m_bModified(sal_False)
            ,m_bDocumentReadOnly(sal_False)
            ,m_pSharedConnectionManager(NULL)
            ,m_nControllerLockCount(0)
{
    // some kind of default
    DBG_CTOR(ODatabaseModelImpl,NULL);
    m_sConnectURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jdbc:"));
    m_aTableFilter.realloc(1);
    m_aTableFilter[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));
    impl_construct_nothrow();
}

ODatabaseModelImpl::ODatabaseModelImpl(
                    const ::rtl::OUString& _rRegistrationName,
                    const Reference< XMultiServiceFactory >& _rxFactory,
                    ODatabaseContext& _rDBContext
                    )
            :m_xModel()
            ,m_xDataSource()
            ,m_pStorageAccess( NULL )
            ,m_aMutex()
            ,m_aMutexFacade( m_aMutex )
            ,m_aContainer(4)
            ,m_aMacroMode( *this )
            ,m_nImposedMacroExecMode( MacroExecMode::NEVER_EXECUTE )
            ,m_pDBContext( &_rDBContext )
            ,m_refCount(0)
            ,m_aEmbeddedMacros()
            ,m_bModificationLock( false )
            ,m_bDocumentInitialized( false )
            ,m_aContext( _rxFactory )
            ,m_sName(_rRegistrationName)
            ,m_nLoginTimeout(0)
            ,m_bReadOnly(sal_False)
            ,m_bPasswordRequired(sal_False)
            ,m_bSuppressVersionColumns(sal_True)
            ,m_bModified(sal_False)
            ,m_bDocumentReadOnly(sal_False)
            ,m_pSharedConnectionManager(NULL)
            ,m_nControllerLockCount(0)
{
    DBG_CTOR(ODatabaseModelImpl,NULL);
    impl_construct_nothrow();
}

ODatabaseModelImpl::~ODatabaseModelImpl()
{
    DBG_DTOR(ODatabaseModelImpl,NULL);
}

void ODatabaseModelImpl::impl_construct_nothrow()
{
    // create the property bag to hold the settings (also known as "Info" property)
    try
    {
        // the set of property value types in the bag is limited:
        Sequence< Type > aAllowedTypes(6);
        Type* pAllowedType = aAllowedTypes.getArray();
        *pAllowedType++ = ::getCppuType( static_cast< sal_Bool* >( NULL ) );
        *pAllowedType++ = ::getCppuType( static_cast< double* >( NULL ) );
        *pAllowedType++ = ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) );
        *pAllowedType++ = ::getCppuType( static_cast< sal_Int32* >( NULL ) );
        *pAllowedType++ = ::getCppuType( static_cast< sal_Int16* >( NULL ) );
        *pAllowedType++ = ::getCppuType( static_cast< Sequence< Any >* >( NULL ) );

        Sequence< Any > aInitArgs( 2 );
        aInitArgs[0] <<= NamedValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AutomaticAddition" ) ),
            makeAny( (sal_Bool)sal_True )
        );
        aInitArgs[1] <<= NamedValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowedTypes" ) ),
            makeAny( aAllowedTypes )
        );

        m_xSettings.set( m_aContext.createComponentWithArguments( "com.sun.star.beans.PropertyBag", aInitArgs ), UNO_QUERY_THROW );

        // insert the default settings
        Reference< XPropertyContainer > xContainer( m_xSettings, UNO_QUERY_THROW );
        Reference< XSet > xSettingsSet( m_xSettings, UNO_QUERY_THROW );
        const AsciiPropertyValue* pSettings = getDefaultDataSourceSettings();
        for ( ; pSettings->AsciiName; ++pSettings )
        {
            if ( !pSettings->DefaultValue.hasValue() )
            {
                Property aProperty(
                    ::rtl::OUString::createFromAscii( pSettings->AsciiName ),
                    -1,
                    pSettings->ValueType,
                    PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT | PropertyAttribute::MAYBEVOID
                );
                xSettingsSet->insert( makeAny( aProperty ) );
            }
            else
            {
                xContainer->addProperty(
                    ::rtl::OUString::createFromAscii( pSettings->AsciiName ),
                    PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT,
                    pSettings->DefaultValue
                );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_pDBContext->appendAtTerminateListener(*this);
}

namespace
{
    ::rtl::OUString lcl_getContainerStorageName_throw( ODatabaseModelImpl::ObjectType _eType )
    {
        const sal_Char* pAsciiName( NULL );
        switch ( _eType )
        {
        case ODatabaseModelImpl::E_FORM:   pAsciiName = "forms"; break;
        case ODatabaseModelImpl::E_REPORT: pAsciiName = "reports"; break;
        case ODatabaseModelImpl::E_QUERY:  pAsciiName = "queries"; break;
        case ODatabaseModelImpl::E_TABLE:  pAsciiName = "tables"; break;
        default:
            throw RuntimeException();
        }
        return ::rtl::OUString::createFromAscii( pAsciiName );
    }

    bool lcl_hasObjectWithMacros_throw( const ODefinitionContainer_Impl& _rObjectDefinitions, const Reference< XStorage >& _rxContainerStorage )
    {
        bool bSomeDocHasMacros = false;

        for (   ODefinitionContainer_Impl::const_iterator object = _rObjectDefinitions.begin();
                ( object != _rObjectDefinitions.end() ) && !bSomeDocHasMacros;
                ++object
            )
        {
#if OSL_DEBUG_LEVEL > 0
            const ::rtl::OUString& rName( object->first ); (void)rName;
#endif

            const TContentPtr& rDefinition( object->second );
            const ::rtl::OUString& rPersistentName( rDefinition->m_aProps.sPersistentName );

            if ( rPersistentName.isEmpty() )
            {   // it's a logical sub folder used to organize the real objects
                const ODefinitionContainer_Impl& rSubFoldersObjectDefinitions( dynamic_cast< const ODefinitionContainer_Impl& >( *rDefinition.get() ) );
                bSomeDocHasMacros = lcl_hasObjectWithMacros_throw( rSubFoldersObjectDefinitions, _rxContainerStorage );
                continue;
            }

            bSomeDocHasMacros = ODatabaseModelImpl::objectHasMacros( _rxContainerStorage, rPersistentName );
        }
        return bSomeDocHasMacros;
    }

    bool lcl_hasObjectsWithMacros_nothrow( ODatabaseModelImpl& _rModel, const ODatabaseModelImpl::ObjectType _eType )
    {
        bool bSomeDocHasMacros = false;

        const OContentHelper_Impl& rContainerData( *_rModel.getObjectContainer( _eType ).get() );
        const ODefinitionContainer_Impl& rObjectDefinitions = dynamic_cast< const ODefinitionContainer_Impl& >( rContainerData );

        try
        {
            Reference< XStorage > xContainerStorage( _rModel.getStorage( _eType, ElementModes::READWRITE ) );
            // note the READWRITE here: If the storage already existed before, then the OpenMode will
            // be ignored, anyway.
            // If the storage did not yet exist, then it will be created. If the database document
            // is read-only, the OpenMode will be automatically downgraded to READ. Otherwise,
            // the storage will in fact be created as READWRITE. While this is not strictly necessary
            // for this particular use case here, it is required since the storage is *cached*, and
            // later use cases will need the READWRITE mode.

            if ( xContainerStorage.is() )
                bSomeDocHasMacros = lcl_hasObjectWithMacros_throw( rObjectDefinitions, xContainerStorage );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            // be on the safe side: If we can't reliably determine whether there are macros,
            // assume there actually are. Better this way, than the other way round.
            bSomeDocHasMacros = true;
        }

        return bSomeDocHasMacros;
    }
}

bool ODatabaseModelImpl::objectHasMacros( const Reference< XStorage >& _rxContainerStorage, const ::rtl::OUString& _rPersistentName )
{
    OSL_PRECOND( _rxContainerStorage.is(), "ODatabaseModelImpl::objectHasMacros: this will crash!" );

    bool bHasMacros = true;
    try
    {
        if ( !_rxContainerStorage->hasByName( _rPersistentName ) )
            return false;

        Reference< XStorage > xObjectStor( _rxContainerStorage->openStorageElement(
            _rPersistentName, ElementModes::READ ) );

        bHasMacros = ::sfx2::DocumentMacroMode::storageHasMacros( xObjectStor );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bHasMacros;
}

void ODatabaseModelImpl::reset()
{
    m_bReadOnly = sal_False;
    ::std::vector< TContentPtr > aEmptyContainers( 4 );
    m_aContainer.swap( aEmptyContainers );

    if ( m_pStorageAccess )
    {
        m_pStorageAccess->dispose();
        m_pStorageAccess->release();
        m_pStorageAccess = NULL;
    }
}

void SAL_CALL ODatabaseModelImpl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
    Reference<XConnection> xCon(Source.Source,UNO_QUERY);
    if ( xCon.is() )
    {
        bool bStore = false;
        OWeakConnectionArray::iterator aEnd = m_aConnections.end();
        for (OWeakConnectionArray::iterator i = m_aConnections.begin(); aEnd != i; ++i)
        {
            if ( xCon == i->get() )
            {
                *i = OWeakConnection();
                bStore = true;
                break;
            }
        }

        if ( bStore )
            commitRootStorage();
    }
    else
    {
        OSL_FAIL( "ODatabaseModelImpl::disposing: where does this come from?" );
    }
}

void ODatabaseModelImpl::clearConnections()
{
    OWeakConnectionArray aConnections;
    aConnections.swap( m_aConnections );

    Reference< XConnection > xConn;
    OWeakConnectionArray::iterator aEnd = aConnections.end();
    for ( OWeakConnectionArray::iterator i = aConnections.begin(); aEnd != i; ++i )
    {
        xConn = *i;
        if ( xConn.is() )
        {
            try
            {
                xConn->close();
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    m_pSharedConnectionManager = NULL;
    m_xSharedConnectionManager = NULL;
}

void ODatabaseModelImpl::dispose()
{
    // dispose the data source and the model
    try
    {
        Reference< XDataSource > xDS( m_xDataSource );
        ::comphelper::disposeComponent( xDS );

        Reference< XModel > xModel( m_xModel );
        ::comphelper::disposeComponent( xModel );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_xDataSource = WeakReference<XDataSource>();
    m_xModel = WeakReference< XModel >();

    ::std::vector<TContentPtr>::iterator aIter = m_aContainer.begin();
    ::std::vector<TContentPtr>::iterator aEnd = m_aContainer.end();
    for (;aIter != aEnd ; ++aIter)
    {
        if ( aIter->get() )
            (*aIter)->m_pDataSource = NULL;
    }
    m_aContainer.clear();

    clearConnections();

    m_xNumberFormatsSupplier = NULL;

    try
    {
        sal_Bool bCouldStore = commitEmbeddedStorage( true );
            // "true" means that committing the embedded storage should not trigger committing the root
            // storage. This is because we are going to commit the root storage ourself, anyway
        disposeStorages();
        if ( bCouldStore )
            commitRootStorage();

        impl_switchToStorage_throw( NULL );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( m_pStorageAccess )
    {
        m_pStorageAccess->dispose();
        m_pStorageAccess->release();
        m_pStorageAccess = NULL;
    }
}

const Reference< XNumberFormatsSupplier > & ODatabaseModelImpl::getNumberFormatsSupplier()
{
    if (!m_xNumberFormatsSupplier.is())
    {
        // the arguments : the locale of the current user
        UserInformation aUserInfo;
        Sequence< Any > aArguments(1);
        aArguments.getArray()[0] <<= aUserInfo.getUserLanguage();

        m_xNumberFormatsSupplier.set(
            m_aContext.createComponentWithArguments( "com.sun.star.util.NumberFormatsSupplier", aArguments ), UNO_QUERY_THROW );
        OSL_ENSURE(m_xNumberFormatsSupplier.is(), "ODatabaseModelImpl::getNumberFormatsSupplier : could not instantiate the formats supplier !");
    }
    return m_xNumberFormatsSupplier;
}

void ODatabaseModelImpl::setDocFileLocation( const ::rtl::OUString& i_rLoadedFrom )
{
    ENSURE_OR_THROW( !i_rLoadedFrom.isEmpty(), "invalid URL" );
    m_sDocFileLocation = i_rLoadedFrom;
}

void ODatabaseModelImpl::setResource( const ::rtl::OUString& i_rDocumentURL, const Sequence< PropertyValue >& _rArgs )
{
    ENSURE_OR_THROW( !i_rDocumentURL.isEmpty(), "invalid URL" );

    ::comphelper::NamedValueCollection aMediaDescriptor( _rArgs );
#if OSL_DEBUG_LEVEL > 0
    if ( aMediaDescriptor.has( "SalvagedFile" ) )
    {
        ::rtl::OUString sSalvagedFile( aMediaDescriptor.getOrDefault( "SalvagedFile", ::rtl::OUString() ) );
        // If SalvagedFile is an empty string, this indicates "the document is being recovered, but i_rDocumentURL already
        // is the real document URL, not the temporary document location"
        if ( sSalvagedFile.isEmpty() )
            sSalvagedFile = i_rDocumentURL;

        OSL_ENSURE( sSalvagedFile == i_rDocumentURL, "ODatabaseModelImpl::setResource: inconsistency!" );
            // nowadays, setResource should only be called with the logical URL of the document
    }
#endif

    m_aMediaDescriptor = stripLoadArguments( aMediaDescriptor );

    impl_switchToLogicalURL( i_rDocumentURL );
}

::comphelper::NamedValueCollection ODatabaseModelImpl::stripLoadArguments( const ::comphelper::NamedValueCollection& _rArguments )
{
    OSL_ENSURE( !_rArguments.has( "Model" ), "ODatabaseModelImpl::stripLoadArguments: this is suspicious (1)!" );
    OSL_ENSURE( !_rArguments.has( "ViewName" ), "ODatabaseModelImpl::stripLoadArguments: this is suspicious (2)!" );

    ::comphelper::NamedValueCollection aMutableArgs( _rArguments );
    aMutableArgs.remove( "Model" );
    aMutableArgs.remove( "ViewName" );
    return aMutableArgs;
}

void ODatabaseModelImpl::disposeStorages() SAL_THROW(())
{
    getDocumentStorageAccess()->disposeStorages();
}

Reference< XSingleServiceFactory > ODatabaseModelImpl::createStorageFactory() const
{
    return Reference< XSingleServiceFactory >( m_aContext.createComponent( "com.sun.star.embed.StorageFactory" ), UNO_QUERY_THROW );
}

void ODatabaseModelImpl::commitRootStorage()
{
    Reference< XStorage > xStorage( getOrCreateRootStorage() );
#if OSL_DEBUG_LEVEL > 0
    bool bSuccess =
#endif
    commitStorageIfWriteable_ignoreErrors( xStorage );
    OSL_ENSURE( bSuccess || !xStorage.is(),
        "ODatabaseModelImpl::commitRootStorage: could commit the storage!" );
}

Reference< XStorage > ODatabaseModelImpl::getOrCreateRootStorage()
{
    if ( !m_xDocumentStorage.is() )
    {
        Reference< XSingleServiceFactory> xStorageFactory = createStorageFactory();
        if ( xStorageFactory.is() )
        {
            Any aSource;
            aSource = m_aMediaDescriptor.get( "Stream" );
            if ( !aSource.hasValue() )
                aSource = m_aMediaDescriptor.get( "InputStream" );
            if ( !aSource.hasValue() && !m_sDocFileLocation.isEmpty() )
                aSource <<= m_sDocFileLocation;
            // TODO: shouldn't we also check URL?

            OSL_ENSURE( aSource.hasValue(), "ODatabaseModelImpl::getOrCreateRootStorage: no source to create the storage from!" );

            if ( aSource.hasValue() )
            {
                Sequence< Any > aStorageCreationArgs(2);
                aStorageCreationArgs[0] = aSource;
                aStorageCreationArgs[1] <<= ElementModes::READWRITE;

                Reference< XStorage > xDocumentStorage;
                try
                {
                    xDocumentStorage.set( xStorageFactory->createInstanceWithArguments( aStorageCreationArgs ), UNO_QUERY_THROW );
                }
                catch( const Exception& )
                {
                    m_bDocumentReadOnly = sal_True;
                    aStorageCreationArgs[1] <<= ElementModes::READ;
                    try
                    {
                        xDocumentStorage.set( xStorageFactory->createInstanceWithArguments( aStorageCreationArgs ), UNO_QUERY_THROW );
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }

                impl_switchToStorage_throw( xDocumentStorage );
            }
        }
    }
    return m_xDocumentStorage.getTyped();
}

DocumentStorageAccess* ODatabaseModelImpl::getDocumentStorageAccess()
{
    if ( !m_pStorageAccess )
    {
        m_pStorageAccess = new DocumentStorageAccess( *this );
        m_pStorageAccess->acquire();
    }
    return m_pStorageAccess;
}

void ODatabaseModelImpl::modelIsDisposing( const bool _wasInitialized, ResetModelAccess )
{
    m_xModel = Reference< XModel >();

    // Basic libraries and Dialog libraries are a model facet, though held at this impl class.
    // They automatically dispose themself when the model they belong to is being disposed.
    // So, to not be tempted to do anything with them, again, we reset them.
    m_xBasicLibraries.clear();
    m_xDialogLibraries.clear();

    m_bDocumentInitialized = _wasInitialized;
}

Reference< XDocumentSubStorageSupplier > ODatabaseModelImpl::getDocumentSubStorageSupplier()
{
    return getDocumentStorageAccess();
}

bool ODatabaseModelImpl::commitEmbeddedStorage( bool _bPreventRootCommits )
{
    return getDocumentStorageAccess()->commitEmbeddedStorage( _bPreventRootCommits );
}

bool ODatabaseModelImpl::commitStorageIfWriteable_ignoreErrors( const Reference< XStorage >& _rxStorage ) SAL_THROW(())
{
    bool bSuccess = false;
    try
    {
        bSuccess = tools::stor::commitStorageIfWriteable( _rxStorage );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bSuccess;
}

void ODatabaseModelImpl::setModified( sal_Bool _bModified )
{
    if ( isModifyLocked() )
        return;

    try
    {
        Reference< XModifiable > xModi( m_xModel.get(), UNO_QUERY );
        if ( xModi.is() )
            xModi->setModified( _bModified );
        else
            m_bModified = _bModified;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

Reference<XDataSource> ODatabaseModelImpl::getOrCreateDataSource()
{
    Reference<XDataSource> xDs = m_xDataSource;
    if ( !xDs.is() )
    {
        xDs = new ODatabaseSource(this);
        m_xDataSource = xDs;
    }
    return xDs;
}

Reference< XModel> ODatabaseModelImpl::getModel_noCreate() const
{
    return m_xModel;
}

Reference< XModel > ODatabaseModelImpl::createNewModel_deliverOwnership( bool _bInitialize )
{
    Reference< XModel > xModel( m_xModel );
    OSL_PRECOND( !xModel.is(), "ODatabaseModelImpl::createNewModel_deliverOwnership: not to be called if there already is a model!" );
    if ( !xModel.is() )
    {
        bool bHadModelBefore = m_bDocumentInitialized;

        xModel = ODatabaseDocument::createDatabaseDocument( this, ODatabaseDocument::FactoryAccess() );
        m_xModel = xModel;

        try
        {
            Reference< XGlobalEventBroadcaster > xModelCollection = GlobalEventBroadcaster::create( m_aContext.getUNOContext() );
            xModelCollection->insert( makeAny( xModel ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( bHadModelBefore )
        {
            // do an attachResources
            // In case the document is loaded regularly, this is not necessary, as our loader will do it.
            // However, in case that the document is implicitly created by asking the data source for the document,
            // then nobody would call the doc's attachResource. So, we do it here, to ensure it's in a proper
            // state, fires all events, and so on.
            // #i105505#
            xModel->attachResource( xModel->getURL(), m_aMediaDescriptor.getPropertyValues() );
        }

        if ( _bInitialize )
        {
            try
            {
                Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );
                xLoad->initNew();
            }
            catch( RuntimeException& ) { throw; }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    return xModel;
}

oslInterlockedCount SAL_CALL ODatabaseModelImpl::acquire()
{
    return osl_atomic_increment(&m_refCount);
}

oslInterlockedCount SAL_CALL ODatabaseModelImpl::release()
{
    if ( osl_atomic_decrement(&m_refCount) == 0 )
    {
        acquire();  // prevent multiple releases
        m_pDBContext->removeFromTerminateListener(*this);
        dispose();
        m_pDBContext->storeTransientProperties(*this);
        revokeDataSource();
        delete this;
        return 0;
    }
    return m_refCount;
}

void ODatabaseModelImpl::commitStorages() SAL_THROW(( IOException, RuntimeException ))
{
    getDocumentStorageAccess()->commitStorages();
}

Reference< XStorage > ODatabaseModelImpl::getStorage( const ObjectType _eType, const sal_Int32 _nDesiredMode )
{
    return getDocumentStorageAccess()->getDocumentSubStorage( getObjectContainerStorageName( _eType ), _nDesiredMode );
}

const AsciiPropertyValue* ODatabaseModelImpl::getDefaultDataSourceSettings()
{
    static const AsciiPropertyValue aKnownSettings[] =
    {
        // known JDBC settings
        AsciiPropertyValue( "JavaDriverClass",            makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "JavaDriverClassPath",       makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "IgnoreCurrency",             makeAny( (sal_Bool)sal_False ) ),
        // known settings for file-based drivers
        AsciiPropertyValue( "Extension",                  makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "CharSet",                    makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "HeaderLine",                 makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "FieldDelimiter",             makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "," ) ) ) ),
        AsciiPropertyValue( "StringDelimiter",            makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\"" ) ) ) ),
        AsciiPropertyValue( "DecimalDelimiter",           makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "." ) ) ) ),
        AsciiPropertyValue( "ThousandDelimiter",          makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "ShowDeleted",                makeAny( (sal_Bool)sal_False ) ),
        // known ODBC settings
        AsciiPropertyValue( "SystemDriverSettings",       makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "UseCatalog",                 makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "TypeInfoSettings",           makeAny( Sequence< Any >()) ),
        // settings related to auto increment handling
        AsciiPropertyValue( "AutoIncrementCreation",      makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "AutoRetrievingStatement",    makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "IsAutoRetrievingEnabled",    makeAny( (sal_Bool)sal_False ) ),
        // known LDAP driver settings
        AsciiPropertyValue( "HostName",                   makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "PortNumber",                 makeAny( (sal_Int32)389 ) ),
        AsciiPropertyValue( "BaseDN",                     makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "MaxRowCount",                makeAny( (sal_Int32)100 ) ),
        // known MySQLNative driver settings
        AsciiPropertyValue( "LocalSocket",                makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "NamedPipe",                  makeAny( ::rtl::OUString() ) ),
        // misc known driver settings
        AsciiPropertyValue( "ParameterNameSubstitution",  makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "AddIndexAppendix",           makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "IgnoreDriverPrivileges",     makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "ImplicitCatalogRestriction", ::cppu::UnoType< ::rtl::OUString >::get() ),
        AsciiPropertyValue( "ImplicitSchemaRestriction",  ::cppu::UnoType< ::rtl::OUString >::get() ),
        AsciiPropertyValue( "PrimaryKeySupport",          ::cppu::UnoType< sal_Bool >::get() ),
        AsciiPropertyValue( "ShowColumnDescription",      makeAny( (sal_Bool)sal_False ) ),
        // known SDB level settings
        AsciiPropertyValue( "NoNameLengthLimit",          makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "AppendTableAliasName",       makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "GenerateASBeforeCorrelationName",  makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "ColumnAliasInOrderBy",       makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "EnableSQL92Check",           makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "BooleanComparisonMode",      makeAny( BooleanComparisonMode::EQUAL_INTEGER ) ),
        AsciiPropertyValue( "TableTypeFilterMode",        makeAny( (sal_Int32)3 ) ),
        AsciiPropertyValue( "RespectDriverResultSetType", makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "UseSchemaInSelect",          makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "UseCatalogInSelect",         makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "EnableOuterJoinEscape",      makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "PreferDosLikeLineEnds",      makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "FormsCheckRequiredFields",   makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "EscapeDateTime",             makeAny( (sal_Bool)sal_True ) ),

        // known services to handle database tasks
        AsciiPropertyValue( "TableAlterationServiceName", makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "TableRenameServiceName",     makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "ViewAlterationServiceName",  makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "ViewAccessServiceName",      makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "CommandDefinitions",         makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "Forms",                      makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "Reports",                    makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "KeyAlterationServiceName",   makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "IndexAlterationServiceName", makeAny( ::rtl::OUString() ) ),

        AsciiPropertyValue()
    };
    return aKnownSettings;
}

TContentPtr& ODatabaseModelImpl::getObjectContainer( ObjectType _eType )
{
    OSL_PRECOND( _eType >= E_FORM && _eType <= E_TABLE, "ODatabaseModelImpl::getObjectContainer: illegal index!" );
    TContentPtr& rContentPtr = m_aContainer[ _eType ];

    if ( !rContentPtr.get() )
    {
        rContentPtr = TContentPtr( new ODefinitionContainer_Impl );
        rContentPtr->m_pDataSource = this;
        rContentPtr->m_aProps.aTitle = lcl_getContainerStorageName_throw( _eType );
    }
    return rContentPtr;
}

void ODatabaseModelImpl::revokeDataSource() const
{
    if ( m_pDBContext && !m_sDocumentURL.isEmpty() )
        m_pDBContext->revokeDatabaseDocument( *this );
}

bool ODatabaseModelImpl::adjustMacroMode_AutoReject()
{
    return m_aMacroMode.adjustMacroMode( NULL );
}

bool ODatabaseModelImpl::checkMacrosOnLoading()
{
    Reference< XInteractionHandler > xInteraction;
    xInteraction = m_aMediaDescriptor.getOrDefault( "InteractionHandler", xInteraction );
    return m_aMacroMode.checkMacrosOnLoading( xInteraction );
}

void ODatabaseModelImpl::resetMacroExecutionMode()
{
    m_aMacroMode = ::sfx2::DocumentMacroMode( *this );
}

Reference< XStorageBasedLibraryContainer > ODatabaseModelImpl::getLibraryContainer( bool _bScript )
{
    Reference< XStorageBasedLibraryContainer >& rxContainer( _bScript ? m_xBasicLibraries : m_xDialogLibraries );
    if ( rxContainer.is() )
        return rxContainer;

    Reference< XStorageBasedDocument > xDocument( getModel_noCreate(), UNO_QUERY_THROW );
        // this is only to be called if there already exists a document model - in fact, it is
        // to be called by the document model only

    try
    {
        Reference< XStorageBasedLibraryContainer > (*Factory)( const Reference< XComponentContext >&, const Reference< XStorageBasedDocument >&)
            = _bScript ? &DocumentScriptLibraryContainer::create : &DocumentDialogLibraryContainer::create;

        rxContainer.set(
            (*Factory)( m_aContext.getUNOContext(), xDocument ),
            UNO_QUERY_THROW
        );
    }
    catch( const RuntimeException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        throw WrappedTargetRuntimeException(
            ::rtl::OUString(),
            xDocument,
            ::cppu::getCaughtException()
        );
    }
    return rxContainer;
}

void ODatabaseModelImpl::storeLibraryContainersTo( const Reference< XStorage >& _rxToRootStorage )
{
    if ( m_xBasicLibraries.is() )
        m_xBasicLibraries->storeLibrariesToStorage( _rxToRootStorage );

    if ( m_xDialogLibraries.is() )
        m_xDialogLibraries->storeLibrariesToStorage( _rxToRootStorage );
}

Reference< XStorage > ODatabaseModelImpl::switchToStorage( const Reference< XStorage >& _rxNewRootStorage )
{
    if ( !_rxNewRootStorage.is() )
        throw IllegalArgumentException();

    return impl_switchToStorage_throw( _rxNewRootStorage );
}

namespace
{
    void lcl_modifyListening( ::sfx2::IModifiableDocument& _rDocument,
        const Reference< XStorage >& _rxStorage, ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >& _inout_rListener,
        ::osl::SolarMutex& _rMutex, bool _bListen )
    {
        Reference< XModifiable > xModify( _rxStorage, UNO_QUERY );
        OSL_ENSURE( xModify.is() || !_rxStorage.is(), "lcl_modifyListening: storage can't notify us!" );

        if ( xModify.is() && !_bListen && _inout_rListener.is() )
        {
            xModify->removeModifyListener( _inout_rListener.get() );
        }

        if ( _inout_rListener.is() )
        {
            _inout_rListener->dispose();
            _inout_rListener = NULL;
        }

        if ( xModify.is() && _bListen )
        {
            _inout_rListener = new ::sfx2::DocumentStorageModifyListener( _rDocument, _rMutex );
            xModify->addModifyListener( _inout_rListener.get() );
        }
    }
}

namespace
{
    static void lcl_rebaseScriptStorage_throw( const Reference< XStorageBasedLibraryContainer >& _rxContainer,
        const Reference< XStorage >& _rxNewRootStorage )
    {
        if ( _rxContainer.is() )
        {
            if ( _rxNewRootStorage.is() )
                _rxContainer->setRootStorage( _rxNewRootStorage );
//            else
                   // TODO: what to do here? dispose the container?
        }
    }
}

Reference< XStorage > ODatabaseModelImpl::impl_switchToStorage_throw( const Reference< XStorage >& _rxNewRootStorage )
{
    // stop listening for modifications at the old storage
    lcl_modifyListening( *this, m_xDocumentStorage.getTyped(), m_pStorageModifyListener, m_aMutexFacade, false );

    // set new storage
    m_xDocumentStorage.reset( _rxNewRootStorage, SharedStorage::TakeOwnership );

    // start listening for modifications
    lcl_modifyListening( *this, m_xDocumentStorage.getTyped(), m_pStorageModifyListener, m_aMutexFacade, true );

    // forward new storage to Basic and Dialog library containers
    lcl_rebaseScriptStorage_throw( m_xBasicLibraries, m_xDocumentStorage.getTyped() );
    lcl_rebaseScriptStorage_throw( m_xDialogLibraries, m_xDocumentStorage.getTyped() );

    m_bReadOnly = !tools::stor::storageIsWritable_nothrow( m_xDocumentStorage.getTyped() );
    // TODO: our data source, if it exists, must broadcast the change of its ReadOnly property

    return m_xDocumentStorage.getTyped();
}

void ODatabaseModelImpl::impl_switchToLogicalURL( const ::rtl::OUString& i_rDocumentURL )
{
    if ( i_rDocumentURL == m_sDocumentURL )
        return;

    const ::rtl::OUString sOldURL( m_sDocumentURL );
    // update our name, if necessary
    if  (   ( m_sName == m_sDocumentURL )   // our name is our old URL
        ||  ( m_sName.isEmpty() )        // we do not have a name, yet (i.e. are not registered at the database context)
        )
    {
        INetURLObject aURL( i_rDocumentURL );
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        {
            m_sName = i_rDocumentURL;
            // TODO: our data source must broadcast the change of the Name property
        }
    }

    // remember URL
    m_sDocumentURL = i_rDocumentURL;

    // update our location, if necessary
    if  ( m_sDocFileLocation.isEmpty() )
        m_sDocFileLocation = m_sDocumentURL;

    // register at the database context, or change registration
    if ( m_pDBContext )
    {
        if ( !sOldURL.isEmpty() )
            m_pDBContext->databaseDocumentURLChange( sOldURL, m_sDocumentURL );
        else
            m_pDBContext->registerDatabaseDocument( *this );
    }
}

::rtl::OUString ODatabaseModelImpl::getObjectContainerStorageName( const ObjectType _eType )
{
    return lcl_getContainerStorageName_throw( _eType );
}

sal_Int16 ODatabaseModelImpl::getCurrentMacroExecMode() const
{
    sal_Int16 nCurrentMode = MacroExecMode::NEVER_EXECUTE;
    try
    {
        nCurrentMode = m_aMediaDescriptor.getOrDefault( "MacroExecutionMode", nCurrentMode );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nCurrentMode;
}

sal_Bool ODatabaseModelImpl::setCurrentMacroExecMode( sal_uInt16 nMacroMode )
{
    m_aMediaDescriptor.put( "MacroExecutionMode", nMacroMode );
    return sal_True;
}

::rtl::OUString ODatabaseModelImpl::getDocumentLocation() const
{
    return getURL();
    // formerly, we returned getDocFileLocation here, which is the location of the file from which we
    // recovered the "real" document.
    // However, during CWS autorecovery evolving, we clarified (with MAV/MT) the role of XModel::getURL and
    // XStorable::getLocation. In this course, we agreed that for a macro security check, the *document URL*
    // (not the recovery file URL) is to be used: The recovery file lies in the backup folder, and by definition,
    // this folder is considered to be secure. So, the document URL needs to be used to decide about the security.
}

Reference< XStorage > ODatabaseModelImpl::getZipStorageToSign()
{
    // we do not support signing the scripting storages, so we're allowed to
    // return <NULL/> here.
    return Reference< XStorage >();
}

ODatabaseModelImpl::EmbeddedMacros ODatabaseModelImpl::determineEmbeddedMacros()
{
    if ( !m_aEmbeddedMacros )
    {
        if ( ::sfx2::DocumentMacroMode::storageHasMacros( const_cast< ODatabaseModelImpl* >( this )->getOrCreateRootStorage() ) )
        {
            m_aEmbeddedMacros.reset( eDocumentWideMacros );
        }
        else if (   lcl_hasObjectsWithMacros_nothrow( const_cast< ODatabaseModelImpl& >( *this ), E_FORM )
                ||  lcl_hasObjectsWithMacros_nothrow( const_cast< ODatabaseModelImpl& >( *this ), E_REPORT )
                )
        {
            m_aEmbeddedMacros.reset( eSubDocumentMacros );
        }
        else
        {
            m_aEmbeddedMacros.reset( eNoMacros );
        }
    }
    return *m_aEmbeddedMacros;
}

sal_Bool ODatabaseModelImpl::documentStorageHasMacros() const
{
    const_cast< ODatabaseModelImpl* >( this )->determineEmbeddedMacros();
    return ( *m_aEmbeddedMacros != eNoMacros );
}

Reference< XEmbeddedScripts > ODatabaseModelImpl::getEmbeddedDocumentScripts() const
{
    return Reference< XEmbeddedScripts >( getModel_noCreate(), UNO_QUERY );
}

sal_Int16 ODatabaseModelImpl::getScriptingSignatureState()
{
    // no support for signatures at the moment
    return SIGNATURESTATE_NOSIGNATURES;
}

sal_Bool ODatabaseModelImpl::hasTrustedScriptingSignature( sal_Bool /*bAllowUIToAddAuthor*/ )
{
    // no support for signatures at the moment
    return sal_False;
}

void ODatabaseModelImpl::showBrokenSignatureWarning( const Reference< XInteractionHandler >& /*_rxInteraction*/ ) const
{
    OSL_FAIL( "ODatabaseModelImpl::showBrokenSignatureWarning: signatures can't be broken - we do not support them!" );
}

void ODatabaseModelImpl::storageIsModified()
{
    setModified( sal_True );
}

ModelDependentComponent::ModelDependentComponent( const ::rtl::Reference< ODatabaseModelImpl >& _model )
    :m_pImpl( _model )
    ,m_aMutex( _model->getSharedMutex() )
{
}

ModelDependentComponent::~ModelDependentComponent()
{
}

}   // namespace dbaccess
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
