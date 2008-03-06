/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModelImpl.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:57:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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

/** === begin UNO includes === **/
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/XDriverManager.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/script/DocumentDialogLibraryContainer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
/** === end UNO includes === **/

#include <comphelper/interaction.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/digest.h>
#include <sfx2/signaturestate.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
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
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::script;
using namespace ::cppu;
using namespace ::osl;
using namespace ::vos;
using namespace ::dbtools;
using namespace ::comphelper;
namespace css = ::com::sun::star;

//........................................................................
namespace dbaccess
{
//........................................................................

//========================================================================
//= SharedMutex
//========================================================================
//------------------------------------------------------------------------
SharedMutex::SharedMutex()
    :m_refCount( 0 )
{
}

//------------------------------------------------------------------------
SharedMutex::~SharedMutex()
{
}

//------------------------------------------------------------------------
void SAL_CALL SharedMutex::acquire()
{
    osl_incrementInterlockedCount( &m_refCount );
}

//------------------------------------------------------------------------
void SAL_CALL SharedMutex::release()
{
    if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        delete this;
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

public:
    DocumentStorageAccess( ODatabaseModelImpl& _rModelImplementation )
        :m_pModelImplementation( &_rModelImplementation )
        ,m_bPropagateCommitToRoot( true )
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

    void    suspendCommitPropagation()
    {
        DBG_ASSERT( m_bPropagateCommitToRoot, "DocumentStorageAccess:: suspendCommitPropagation: already suspended" );
        m_bPropagateCommitToRoot = false;
    }
    void    resumeCommitPropagation()
    {
        DBG_ASSERT( !m_bPropagateCommitToRoot, "DocumentStorageAccess:: suspendCommitPropagation: already suspended" );
        m_bPropagateCommitToRoot = true;
    }

    // XDocumentSubStorageSupplier
    virtual Reference< XStorage > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, ::sal_Int32 nMode ) throw (RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (IOException, RuntimeException);

    // XTransactionListener
    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
};

//--------------------------------------------------------------------------
void DocumentStorageAccess::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    for ( NamedStorages::iterator loop = m_aExposedStorages.begin();
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
            OSL_ENSURE( sal_False, "DocumentStorageAccess::dispose: caught an exception!" );
        }
    }

    m_aExposedStorages.clear();

    m_pModelImplementation = NULL;
}

//--------------------------------------------------------------------------
Reference< XStorage > SAL_CALL DocumentStorageAccess::getDocumentSubStorage( const ::rtl::OUString& aStorageName, ::sal_Int32 nMode ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    NamedStorages::iterator pos = m_aExposedStorages.find( aStorageName );
    if ( pos == m_aExposedStorages.end() )
    {
        Reference< XStorage > xResult = m_pModelImplementation->getStorage( aStorageName, nMode );
        Reference< XTransactionBroadcaster > xBroadcaster( xResult, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->addTransactionListener( this );

        pos = m_aExposedStorages.insert( NamedStorages::value_type( aStorageName, xResult ) ).first;
    }

    return pos->second;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL DocumentStorageAccess::getDocumentSubStoragesNames(  ) throw (IOException, RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(2);
    sal_Int32 nPos = 0;
    aRet[nPos++] = m_pModelImplementation->getObjectContainerStorageName( ODatabaseModelImpl::E_FORM );
    aRet[nPos++] = m_pModelImplementation->getObjectContainerStorageName( ODatabaseModelImpl::E_REPORT );
    return aRet;
}

//--------------------------------------------------------------------------
void SAL_CALL DocumentStorageAccess::preCommit( const css::lang::EventObject& /*aEvent*/ ) throw (Exception, RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------------
void SAL_CALL DocumentStorageAccess::commited( const css::lang::EventObject& aEvent ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pModelImplementation )
        m_pModelImplementation->setModified( sal_True );

    if ( m_pModelImplementation && m_bPropagateCommitToRoot )
    {
        Reference< XStorage > xStorage( aEvent.Source, UNO_QUERY );
        if ( m_pModelImplementation->isDatabaseStorage( xStorage ) )
        {
            m_pModelImplementation->commitRootStorage();
        }
    }
}

//--------------------------------------------------------------------------
void SAL_CALL DocumentStorageAccess::preRevert( const css::lang::EventObject& /*aEvent*/ ) throw (Exception, RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------------
void SAL_CALL DocumentStorageAccess::reverted( const css::lang::EventObject& /*aEvent*/ ) throw (RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------------
void SAL_CALL DocumentStorageAccess::disposing( const css::lang::EventObject& Source ) throw ( RuntimeException )
{
    ODatabaseModelImpl* pImpl = m_pModelImplementation;
    if ( pImpl )
        pImpl->disposing( Source );

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
//--------------------------------------------------------------------------
ODatabaseModelImpl::ODatabaseModelImpl(const Reference< XMultiServiceFactory >& _rxFactory
                                       , const Reference< XModel>& _xModel)
            :m_xModel(_xModel)
            ,m_xDataSource()
            ,m_pStorageAccess( NULL )
            ,m_xMutex( new SharedMutex )
            ,m_aContainer(4)
            ,m_aStorages()
            ,m_aMacroMode( *this )
            ,m_bHasAnyObjectWithMacros( false )
            ,m_bModificationLock( false )
            ,m_aContext( _rxFactory )
            ,m_nLoginTimeout(0)
            ,m_bReadOnly(sal_False)
            ,m_bPasswordRequired(sal_False)
            ,m_bSuppressVersionColumns(sal_True)
            ,m_bModified(sal_False)
            ,m_bDocumentReadOnly(sal_False)
            ,m_bDisposingSubStorages( sal_False )
            ,m_pDBContext(NULL)
            ,m_pSharedConnectionManager(NULL)
            ,m_refCount(0)
            ,m_nControllerLockCount(0)
{
    // some kind of default
    DBG_CTOR(ODatabaseModelImpl,NULL);
    m_sConnectURL = ::rtl::OUString::createFromAscii("jdbc:");
    m_aTableFilter.realloc(1);
    m_aTableFilter[0] = ::rtl::OUString::createFromAscii("%");
    impl_construct_nothrow();
}

//--------------------------------------------------------------------------
ODatabaseModelImpl::ODatabaseModelImpl(
                    const ::rtl::OUString& _rRegistrationName,
                    const Reference< XMultiServiceFactory >& _rxFactory,
                    ODatabaseContext* _pDBContext
                    )
            :m_xModel()
            ,m_xDataSource()
            ,m_pStorageAccess( NULL )
            ,m_xMutex( new SharedMutex )
            ,m_aContainer(4)
            ,m_aStorages()
            ,m_aMacroMode( *this )
            ,m_bHasAnyObjectWithMacros( false )
            ,m_bModificationLock( false )
            ,m_aContext( _rxFactory )
            ,m_sName(_rRegistrationName)
            ,m_nLoginTimeout(0)
            ,m_bReadOnly(sal_False)
            ,m_bPasswordRequired(sal_False)
            ,m_bSuppressVersionColumns(sal_True)
            ,m_bModified(sal_False)
            ,m_bDocumentReadOnly(sal_False)
            ,m_bDisposingSubStorages( sal_False )
            ,m_pDBContext(_pDBContext)
            ,m_pSharedConnectionManager(NULL)
            ,m_refCount(0)
            ,m_nControllerLockCount(0)
{
    DBG_CTOR(ODatabaseModelImpl,NULL);
    // adjust our readonly flag

    impl_construct_nothrow();
}

//--------------------------------------------------------------------------
ODatabaseModelImpl::~ODatabaseModelImpl()
{
    DBG_DTOR(ODatabaseModelImpl,NULL);
}

// -----------------------------------------------------------------------------
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
        const AsciiPropertyValue* pSettings = getDefaultDataSourceSettings();
        for ( ; pSettings->AsciiName; ++pSettings )
        {
            xContainer->addProperty(
                ::rtl::OUString::createFromAscii( pSettings->AsciiName ),
                PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT,
                pSettings->DefaultValue
            );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
namespace
{
    // .........................................................................
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

    // .........................................................................
    bool lcl_hasObjectWithMacros_throw( const ODefinitionContainer_Impl& _rObjectDefinitions, const SharedStorage& _rxContainerStorage )
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

            if ( !rPersistentName.getLength() )
            {   // it's a logical sub folder used to organize the real objects
                const ODefinitionContainer_Impl& rSubFoldersObjectDefinitions( dynamic_cast< const ODefinitionContainer_Impl& >( *rDefinition.get() ) );
                bSomeDocHasMacros = lcl_hasObjectWithMacros_throw( rSubFoldersObjectDefinitions, _rxContainerStorage );
                continue;
            }

            bSomeDocHasMacros = ODatabaseModelImpl::objectHasMacros( _rxContainerStorage, rPersistentName );
        }
        return bSomeDocHasMacros;
    }

    // .........................................................................
    bool lcl_hasObjectsWithMacros_nothrow( ODatabaseModelImpl& _rModel, const ODatabaseModelImpl::ObjectType _eType )
    {
        bool bSomeDocHasMacros = false;

        const OContentHelper_Impl& rContainerData( *_rModel.getObjectContainer( _eType ).get() );
        const ODefinitionContainer_Impl& rObjectDefinitions = dynamic_cast< const ODefinitionContainer_Impl& >( rContainerData );

        try
        {
            SharedStorage xContainerStorage( _rModel.getStorage(
                _rModel.getObjectContainerStorageName( _eType ), ElementModes::READ ) );

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

// -----------------------------------------------------------------------------
bool ODatabaseModelImpl::objectHasMacros( const Reference< XStorage >& _rxContainerStorage, const ::rtl::OUString& _rPersistentName )
{
    OSL_PRECOND( _rxContainerStorage.is(), "ODatabaseModelImpl::objectHasMacros: this will crash!" );

    bool bHasMacros = true;
    try
    {
        if ( !_rxContainerStorage->hasByName( _rPersistentName ) )
            return false;

        SharedStorage xObjectStor( _rxContainerStorage->openStorageElement(
            _rPersistentName, ElementModes::READ ) );

        bHasMacros = ::sfx2::DocumentMacroMode::storageHasMacros( xObjectStor );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bHasMacros;
}

// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
    else // storage
    {
        if ( !m_bDisposingSubStorages )
        {
            Reference<XStorage> xStorage(Source.Source,UNO_QUERY);
            TStorages::iterator aFind = ::std::find_if(m_aStorages.begin(),m_aStorages.end(),
                                                ::std::compose1(::std::bind2nd(::std::equal_to<Reference<XStorage> >(),xStorage),::std::select2nd<TStorages::value_type>()));
            if ( aFind != m_aStorages.end() )
                m_aStorages.erase(aFind);
        }
    }
}
//------------------------------------------------------------------------------
void ODatabaseModelImpl::clearConnections()
{
    OWeakConnectionArray aConnections;
    aConnections.swap( m_aConnections );

    Reference< XConnection > xConn;
    for ( OWeakConnectionArray::iterator i = aConnections.begin(); aConnections.end() != i; ++i )
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
//------------------------------------------------------------------------------
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
        sal_Bool bStore = commitEmbeddedStorage();
        disposeStorages();
        if ( bStore )
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
// -----------------------------------------------------------------------------
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
        DBG_ASSERT(m_xNumberFormatsSupplier.is(), "ODatabaseModelImpl::getNumberFormatsSupplier : could not instantiate the formats supplier !");
    }
    return m_xNumberFormatsSupplier;
}
// -----------------------------------------------------------------------------
void ODatabaseModelImpl::disposeStorages() SAL_THROW(())
{
    m_bDisposingSubStorages = sal_True;

    TStorages::iterator aEnd = m_aStorages.end();
    for ( TStorages::iterator aIter = m_aStorages.begin();
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
            OSL_ENSURE( sal_False, "ODatabaseModelImpl::disposeStorages: caught an exception!" );
        }
    }
    m_aStorages.clear();

    m_bDisposingSubStorages = sal_False;
}
// -----------------------------------------------------------------------------
Reference< XSingleServiceFactory > ODatabaseModelImpl::createStorageFactory() const
{
    return Reference< XSingleServiceFactory >( m_aContext.createComponent( "com.sun.star.embed.StorageFactory" ), UNO_QUERY_THROW );
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XStorage > ODatabaseModelImpl::getOrCreateRootStorage()
{
    if ( !m_xDocumentStorage.is() )
    {
        Reference< XSingleServiceFactory> xStorageFactory = createStorageFactory();
        if ( xStorageFactory.is() && m_sDocumentURL.getLength() )
        {
            Any aSource;
            ::comphelper::NamedValueCollection aArgs( m_aArgs );

            aSource = aArgs.get( "Stream" );
            if ( !aSource.hasValue() )
                aSource = aArgs.get( "InputStream" );
            if ( !aSource.hasValue() && m_sDocumentURL.getLength() )
                aSource <<= m_sDocumentURL;

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
// -----------------------------------------------------------------------------
DocumentStorageAccess* ODatabaseModelImpl::getDocumentStorageAccess()
{
    if ( !m_pStorageAccess )
    {
        m_pStorageAccess = new DocumentStorageAccess( *this );
        m_pStorageAccess->acquire();
    }
    return m_pStorageAccess;
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::modelIsDisposing( ResetModelAccess )
{
    m_xModel = Reference< XModel >();

    // Basic libraries and Dialog libraries are a model facet, though held at this impl class.
    // They automatically dispose themself when the model they belong to is being disposed.
    // So, to not be tempted to do anything with them, again, we reset them.
    m_xBasicLibraries.clear();
    m_xDialogLibraries.clear();
}

// -----------------------------------------------------------------------------
Reference< XDocumentSubStorageSupplier > ODatabaseModelImpl::getDocumentSubStorageSupplier()
{
    return getDocumentStorageAccess();
}
// -----------------------------------------------------------------------------
Reference<XStorage> ODatabaseModelImpl::getStorage( const ::rtl::OUString& _sStorageName, sal_Int32 _nMode )
{
    OSL_ENSURE(_sStorageName.getLength(),"ODatabaseModelImpl::getStorage: Invalid storage name!");
    Reference<XStorage> xStorage;
    TStorages::iterator aFind = m_aStorages.find(_sStorageName);
    if ( aFind == m_aStorages.end() )
    {
        try
        {
            Reference< XStorage > xMyStorage( getOrCreateRootStorage() );
            if ( xMyStorage.is() )
            {
                sal_Int32 nMode = m_bDocumentReadOnly ? ElementModes::READ : _nMode;
                if ( nMode == ElementModes::READ )
                {
                    Reference< XNameAccess > xSubStorageNames( xMyStorage, UNO_QUERY );
                    if ( xSubStorageNames.is() && !xSubStorageNames->hasByName( _sStorageName ) )
                        return xStorage;
                }

                xStorage = xMyStorage->openStorageElement( _sStorageName, nMode );
                Reference< XTransactionBroadcaster > xBroad( xStorage, UNO_QUERY );
                if ( xBroad.is() )
                    xBroad->addTransactionListener( getDocumentStorageAccess() );
                aFind = m_aStorages.insert( TStorages::value_type( _sStorageName, xStorage ) ).first;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    if ( aFind != m_aStorages.end() )
        xStorage = aFind->second;

    return xStorage;
}
// -----------------------------------------------------------------------------
sal_Bool ODatabaseModelImpl::commitEmbeddedStorage( sal_Bool _bPreventRootCommits )
{
    if ( _bPreventRootCommits && m_pStorageAccess )
        m_pStorageAccess->suspendCommitPropagation();

    sal_Bool bStore = sal_False;
    try
    {
        TStorages::iterator aFind = m_aStorages.find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("database")));
        if ( aFind != m_aStorages.end() )
            bStore = commitStorageIfWriteable(aFind->second);
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception Caught: Could not store embedded database!");
    }

    if ( _bPreventRootCommits && m_pStorageAccess )
        m_pStorageAccess->resumeCommitPropagation();

    return bStore;

}

// -----------------------------------------------------------------------------
namespace
{
    bool lcl_storageIsWritable_nothrow( const Reference< XStorage >& _rxStorage )
    {
        if ( !_rxStorage.is() )
            return false;

        sal_Int32 nMode = ElementModes::READ;
        try
        {
            Reference< XPropertySet > xStorageProps( _rxStorage, UNO_QUERY_THROW );
            xStorageProps->getPropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenMode" ) ) ) >>= nMode;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return ( nMode & ElementModes::WRITE ) != 0;
    }
}

// -----------------------------------------------------------------------------
bool ODatabaseModelImpl::commitStorageIfWriteable( const Reference< XStorage >& _rxStorage ) SAL_THROW(( IOException, WrappedTargetException, RuntimeException ))
{
    bool bSuccess = false;
    Reference<XTransactedObject> xTrans( _rxStorage, UNO_QUERY );
    if ( xTrans.is() )
    {
        if ( lcl_storageIsWritable_nothrow( _rxStorage ) )
            xTrans->commit();
        bSuccess = true;
    }
    return bSuccess;
}
// -----------------------------------------------------------------------------
bool ODatabaseModelImpl::commitStorageIfWriteable_ignoreErrors( const Reference< XStorage >& _rxStorage ) SAL_THROW(())
{
    bool bSuccess = false;
    try
    {
        bSuccess = commitStorageIfWriteable( _rxStorage );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "ODatabaseModelImpl::commitStorageIfWriteable_ignoreErrors: caught an exception!" );
    }
    return bSuccess;
}
// -----------------------------------------------------------------------------
void ODatabaseModelImpl::setModified( sal_Bool _bModified )
{
    if ( isModifyLocked() )
        return;

    try
    {
        Reference<XModifiable> xModi(m_xModel.get(),UNO_QUERY);
        if ( xModi.is() )
            xModi->setModified( _bModified );
        else
            m_bModified = _bModified;
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"ODatabaseModelImpl::setModified: Exception caught!");
    }
}

// -----------------------------------------------------------------------------
Reference<XDataSource> ODatabaseModelImpl::getDataSource( bool _bCreateIfNecessary )
{
    Reference<XDataSource> xDs = m_xDataSource;
    if ( !xDs.is() && _bCreateIfNecessary )
    { // no data source, so we have to create one and register it later on
        xDs = new ODatabaseSource(this);
        m_xDataSource = xDs;
    }
    return xDs;
}
// -----------------------------------------------------------------------------
Reference< XModel> ODatabaseModelImpl::getModel_noCreate() const
{
    return m_xModel;
}
// -----------------------------------------------------------------------------
Reference< XModel > ODatabaseModelImpl::createNewModel_deliverOwnership()
{
    Reference< XModel > xModel( m_xModel );
    OSL_PRECOND( !xModel.is(), "ODatabaseModelImpl::createNewModel_deliverOwnership: not to be called if there already is a model!" );
    if ( !xModel.is() )
    {
        xModel = ODatabaseDocument::createDatabaseDocument( this, ODatabaseDocument::FactoryAccess() );
        m_xModel = xModel;
    }
    return xModel;
}
// -----------------------------------------------------------------------------
oslInterlockedCount SAL_CALL ODatabaseModelImpl::acquire()
{
    return osl_incrementInterlockedCount(&m_refCount);
}
// -----------------------------------------------------------------------------
oslInterlockedCount SAL_CALL ODatabaseModelImpl::release()
{
    if ( osl_decrementInterlockedCount(&m_refCount) == 0 )
    {
        acquire();  // prevent multiple releases
        dispose();
        m_pDBContext->storeTransientProperties(*this);
        revokeDataSource();
        delete this;
        return 0;
    }
    return m_refCount;
}
// -----------------------------------------------------------------------------
void ODatabaseModelImpl::commitStorages() SAL_THROW(( IOException, RuntimeException ))
{
    try
    {
        TStorages::iterator aIter = m_aStorages.begin();
        TStorages::iterator aEnd = m_aStorages.end();
        for (; aIter != aEnd ; ++aIter)
            commitStorageIfWriteable( aIter->second );
    }
    catch(const WrappedTargetException&)
    {
        // WrappedTargetException not allowed to leave
        throw IOException();
    }
}

// -----------------------------------------------------------------------------
const AsciiPropertyValue* ODatabaseModelImpl::getDefaultDataSourceSettings()
{
    static const AsciiPropertyValue aKnownSettings[] =
    {
        // known JDBC settings
        AsciiPropertyValue( "JavaDriverClass",            makeAny( ::rtl::OUString() ) ),
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
        // settings related to auto increment handling
        AsciiPropertyValue( "AutoIncrementCreation",      makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "AutoRetrievingStatement",    makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "IsAutoRetrievingEnabled",    makeAny( (sal_Bool)sal_False ) ),
        // known Adabas D driver setting
        AsciiPropertyValue( "ShutdownDatabase",           makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "DataCacheSizeIncrement",     makeAny( (sal_Int32)20 ) ),
        AsciiPropertyValue( "DataCacheSize",              makeAny( (sal_Int32)20 ) ),
        AsciiPropertyValue( "ControlUser",                makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "ControlPassword",            makeAny( ::rtl::OUString() ) ),
        // known LDAP driver settings
        AsciiPropertyValue( "HostName",                   makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "PortNumber",                 makeAny( (sal_Int32)389 ) ),
        AsciiPropertyValue( "BaseDN",                     makeAny( ::rtl::OUString() ) ),
        AsciiPropertyValue( "MaxRowCount",                makeAny( (sal_Int32)100 ) ),
        // misc known driver settings
        AsciiPropertyValue( "ParameterNameSubstitution",  makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "AddIndexAppendix",           makeAny( (sal_Bool)sal_True ) ),
        // known SDB level settings
        AsciiPropertyValue( "IgnoreDriverPrivileges",     makeAny( (sal_Bool)sal_True ) ),
        AsciiPropertyValue( "NoNameLengthLimit",          makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "AppendTableAliasName",       makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "GenerateASBeforeCorrelationName",  makeAny( (sal_Bool)sal_True ) ),
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
        AsciiPropertyValue( "IgnoreCurrency",             makeAny( (sal_Bool)sal_False ) ),
        AsciiPropertyValue( "TypeInfoSettings",           makeAny( Sequence< Any >()) ),

        AsciiPropertyValue( NULL, Any() )
    };
    return aKnownSettings;
}

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
bool ODatabaseModelImpl::adjustMacroMode_AutoReject()
{
    return m_aMacroMode.adjustMacroMode( NULL );
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::revokeDataSource() const
{
    if ( m_pDBContext && m_sDocumentURL.getLength() )
        m_pDBContext->deregisterPrivate( m_sDocumentURL );
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::checkMacrosOnLoading()
{
    ::comphelper::NamedValueCollection aArgs( m_aArgs );
    Reference< XInteractionHandler > xInteraction;
    xInteraction = aArgs.getOrDefault( "InteractionHandler", xInteraction );
    m_aMacroMode.checkMacrosOnLoading( xInteraction );
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::resetMacroExecutionMode()
{
    m_aMacroMode = ::sfx2::DocumentMacroMode( *this );
}

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::storeLibraryContainersTo( const Reference< XStorage >& _rxToRootStorage )
{
    if ( m_xBasicLibraries.is() )
        m_xBasicLibraries->storeLibrariesToStorage( _rxToRootStorage );

    if ( m_xDialogLibraries.is() )
        m_xDialogLibraries->storeLibrariesToStorage( _rxToRootStorage );
}

// -----------------------------------------------------------------------------
Reference< XStorage > ODatabaseModelImpl::switchToStorage( const Reference< XStorage >& _rxNewRootStorage )
{
    if ( !_rxNewRootStorage.is() )
        throw IllegalArgumentException();

    return impl_switchToStorage_throw( _rxNewRootStorage );
}

// -----------------------------------------------------------------------------
namespace
{
    void lcl_modifyListening( ::osl::Mutex& _rMutex, ::sfx2::IModifiableDocument& _rDocument,
        const Reference< XStorage >& _rxStorage, ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >& _inout_rListener,
        bool _bListen )
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
            _inout_rListener = new ::sfx2::DocumentStorageModifyListener( _rMutex, _rDocument );
            xModify->addModifyListener( _inout_rListener.get() );
        }
    }
}

// -----------------------------------------------------------------------------
Reference< XStorage > ODatabaseModelImpl::impl_switchToStorage_throw( const Reference< XStorage >& _rxNewRootStorage )
{
    // stop listening for modifications at the old storage
    lcl_modifyListening( m_xMutex->getMutex(), *this, m_xDocumentStorage.getTyped(), m_pStorageModifyListener, false );

    // set new storage
    m_xDocumentStorage.reset( _rxNewRootStorage, SharedStorage::TakeOwnership );

    // start listening for modifications
    lcl_modifyListening( m_xMutex->getMutex(), *this, m_xDocumentStorage.getTyped(), m_pStorageModifyListener, true );

    // forward new storage to Basic and Dialog library containers
    if ( m_xBasicLibraries.is() )
        m_xBasicLibraries->setRootStorage( m_xDocumentStorage.getTyped() );
    if ( m_xDialogLibraries.is() )
        m_xDialogLibraries->setRootStorage( m_xDocumentStorage.getTyped() );

    m_bReadOnly = !lcl_storageIsWritable_nothrow( m_xDocumentStorage.getTyped() );
    // TODO: our data source, if it exists, must broadcast the change of its ReadOnly property

    return m_xDocumentStorage.getTyped();
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::switchToURL( const ::rtl::OUString& _rDocumentLocation, const ::rtl::OUString& _rDocumentURL )
{
    // register at the database context, or change registration
    if ( _rDocumentURL != m_sDocumentURL )
    {
        if ( m_pDBContext )
        {
            if ( m_sDocumentURL.getLength() )
                m_pDBContext->nameChangePrivate( m_sDocumentURL, _rDocumentURL );
            else
                m_pDBContext->registerPrivate( _rDocumentURL, this );
        }

        INetURLObject aURL( _rDocumentURL );
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            m_sName = _rDocumentURL;
    }

    // remember both
    m_sDocFileLocation = _rDocumentLocation;
    m_sDocumentURL = _rDocumentURL;
}

// -----------------------------------------------------------------------------
bool ODatabaseModelImpl::isDatabaseStorage( const Reference< XStorage >& _rxStorage ) const
{
    TStorages::const_iterator pos = m_aStorages.find( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "database" ) ) );
    if  (   ( pos != m_aStorages.end() )
        &&  ( pos->second == _rxStorage )
        )
    {
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
::rtl::OUString ODatabaseModelImpl::getObjectContainerStorageName( const ObjectType _eType )
{
    return lcl_getContainerStorageName_throw( _eType );
}

// -----------------------------------------------------------------------------
sal_Int16 ODatabaseModelImpl::getImposedMacroExecMode() const
{
    sal_Int16 nMacroExecMode( MacroExecMode::USE_CONFIG );
    try
    {
        ::comphelper::NamedValueCollection aArgs( m_aArgs );
        nMacroExecMode = aArgs.getOrDefault( "MacroExecutionMode", nMacroExecMode );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nMacroExecMode;
}

// -----------------------------------------------------------------------------
sal_Bool ODatabaseModelImpl::setImposedMacroExecMode( sal_uInt16 nMacroMode )
{
    try
    {
        ::comphelper::NamedValueCollection aArgs( m_aArgs );
        aArgs.put( "MacroExecutionMode", nMacroMode );
        aArgs >>= m_aArgs;
        return sal_True;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sal_False;
}

// -----------------------------------------------------------------------------
::rtl::OUString ODatabaseModelImpl::getDocumentLocation() const
{
    // don't return getURL() (or m_sDocumentURL, which is the same). In case we were recovered
    // after a previous crash of OOo, m_sDocFileLocation points to the file which were loaded from,
    // and this is the one we need for security checks.
    return getLocation();
}

// -----------------------------------------------------------------------------
Reference< XStorage > ODatabaseModelImpl::getLastCommitDocumentStorage()
{
    // we do not support signing the scripting storages, so we're allowed to
    // return <NULL/> here.
    return Reference< XStorage >();
}

// -----------------------------------------------------------------------------
sal_Bool ODatabaseModelImpl::documentStorageHasMacros() const
{
    // does our root storage contain macros?
    if ( ::sfx2::DocumentMacroMode::storageHasMacros( m_xDocumentStorage ) )
        return true;

    // do we have forms or reports with macros?
    if  (   lcl_hasObjectsWithMacros_nothrow( const_cast< ODatabaseModelImpl& >( *this ), E_FORM )
        ||  lcl_hasObjectsWithMacros_nothrow( const_cast< ODatabaseModelImpl& >( *this ), E_REPORT )
        )
    {
        const_cast< ODatabaseModelImpl* >( this )->m_bHasAnyObjectWithMacros = true;
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------
Reference< XEmbeddedScripts > ODatabaseModelImpl::getEmbeddedDocumentScripts() const
{
    return Reference< XEmbeddedScripts >( getModel_noCreate(), UNO_QUERY );
}

// -----------------------------------------------------------------------------
sal_Int16 ODatabaseModelImpl::getScriptingSignatureState() const
{
    // no support for signatures at the moment
    return SIGNATURESTATE_NOSIGNATURES;
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::showBrokenSignatureWarning( const Reference< XInteractionHandler >& /*_rxInteraction*/ ) const
{
    OSL_ENSURE( false, "ODatabaseModelImpl::showBrokenSignatureWarning: signatures can't be broken - we do not support them!" );
}

// -----------------------------------------------------------------------------
void ODatabaseModelImpl::storageIsModified()
{
    setModified( sal_True );
}

// -----------------------------------------------------------------------------
ModelDependentComponent::ModelDependentComponent( const ::rtl::Reference< ODatabaseModelImpl >& _model )
    :m_pImpl( _model )
    ,m_xMutex( _model->getSharedMutex() )
{
}

// -----------------------------------------------------------------------------
ModelDependentComponent::~ModelDependentComponent()
{
}

//........................................................................
}   // namespace dbaccess
//........................................................................

