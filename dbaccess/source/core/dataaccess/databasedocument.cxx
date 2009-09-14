/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: databasedocument.cxx,v $
 * $Revision: 1.40.6.21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "core_resource.hxx"
#include "core_resource.hrc"
#include "datasource.hxx"
#include "databasedocument.hxx"
#include "dbastrings.hrc"
#include "module_dba.hxx"
#include "documenteventexecutor.hxx"
#include "databasecontext.hxx"
#include "documentcontainer.hxx"

#include <comphelper/documentconstants.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/numberedcollection.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/property.hxx>
#include <svtools/saveopt.hxx>

#include <framework/titlehelper.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
/** === end UNO includes === **/

#include <comphelper/documentconstants.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/numberedcollection.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <framework/titlehelper.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/errcode.hxx>
#include <tools/urlobj.hxx>

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>
#include <list>

#define MAP_LEN(x) x, sizeof(x) - 1

#define MAP_LEN(x) x, sizeof(x) - 1

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::provider;
using namespace ::com::sun::star::ui;
using namespace ::cppu;
using namespace ::osl;

using ::com::sun::star::awt::XWindow;

//........................................................................
namespace dbaccess
{
//........................................................................

//============================================================
//= ViewMonitor
//============================================================
//--------------------------------------------------------------------------
bool ViewMonitor::onControllerConnected( const Reference< XController >& _rxController )
{
    bool bFirstControllerEver = ( m_bEverHadController == false );
    m_bEverHadController = true;

    m_xLastConnectedController = _rxController;
    m_bLastIsFirstEverController = bFirstControllerEver;

    return bFirstControllerEver;
}

//--------------------------------------------------------------------------
void ViewMonitor::onSetCurrentController( const Reference< XController >& _rxController )
{
    // we interpret this as "loading the document (including UI) is finished",
    // if and only if this is the controller which was last connected, and it was the
    // first controller ever connected
    bool bLoadFinished = ( _rxController == m_xLastConnectedController ) && m_bLastIsFirstEverController;

    // notify the respective events
    if ( bLoadFinished )
        m_rEventNotifier.notifyDocumentEventAsync( m_bIsNewDocument ? "OnNew" : "OnLoad" );
}

//============================================================
//= ODatabaseDocument
//============================================================
DBG_NAME(ODatabaseDocument)
//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ODatabaseDocument()
{
    static ::dba::OAutoRegistration< ODatabaseDocument > aAutoRegistration;
}

//--------------------------------------------------------------------------
ODatabaseDocument::ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl )
            :ModelDependentComponent( _pImpl )
            ,ODatabaseDocument_OfficeDocument( getMutex() )
            ,m_aModifyListeners( getMutex() )
            ,m_aCloseListener( getMutex() )
            ,m_aStorageListeners( getMutex() )
            ,m_pEventContainer( new DocumentEvents( *this, getMutex(), _pImpl->getDocumentEvents() ) )
            ,m_pEventExecutor( NULL )   // initialized below, ref-count-protected
            ,m_aEventNotifier( *this, getMutex() )
            ,m_aViewMonitor( m_aEventNotifier )
            ,m_eInitState( NotInitialized )
            ,m_bClosing( false )
            ,m_bAllowDocumentScripting( false )
{
    DBG_CTOR(ODatabaseDocument,NULL);

    osl_incrementInterlockedCount( &m_refCount );
    {
        impl_reparent_nothrow( m_xForms );
        impl_reparent_nothrow( m_xReports );
        impl_reparent_nothrow( m_pImpl->m_xTableDefinitions );
        impl_reparent_nothrow( m_pImpl->m_xCommandDefinitions );

        m_pEventExecutor = new DocumentEventExecutor( m_pImpl->m_aContext, this );
    }
    osl_decrementInterlockedCount( &m_refCount );

    // if there previously was a document instance for the same Impl which was already initialized,
    // then consider ourself initialized, too.
    // #i94840#
    if ( m_pImpl->hadInitializedDocument() )
    {
        impl_setInitialized();
        m_bAllowDocumentScripting = ( m_pImpl->determineEmbeddedMacros() != ODatabaseModelImpl::eSubDocumentMacros );
    }
}

//--------------------------------------------------------------------------
ODatabaseDocument::~ODatabaseDocument()
{
    DBG_DTOR(ODatabaseDocument,NULL);
    if ( !ODatabaseDocument_OfficeDocument::rBHelper.bInDispose && !ODatabaseDocument_OfficeDocument::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    delete m_pEventContainer, m_pEventContainer = NULL;
}
// -----------------------------------------------------------------------------
Any SAL_CALL ODatabaseDocument::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    // strip XEmbeddedScripts and XScriptInvocationContext if we have any form/report
    // which already contains macros. In this case, the database document itself is not
    // allowed to contain macros, too.
    if  (   !m_bAllowDocumentScripting
        &&  (   _rType.equals( XEmbeddedScripts::static_type() )
            ||  _rType.equals( XScriptInvocationContext::static_type() )
            )
        )
        return Any();

    Any aReturn = ODatabaseDocument_OfficeDocument::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ODatabaseDocument_Title::queryInterface(_rType);
    return aReturn;
}
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::acquire(  ) throw ()
{
    ODatabaseDocument_OfficeDocument::acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::release(  ) throw ()
{
    ODatabaseDocument_OfficeDocument::release();
}
//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODatabaseDocument::getTypes(  ) throw (RuntimeException)
{
    Sequence< Type > aTypes = ::comphelper::concatSequences(
        ODatabaseDocument_OfficeDocument::getTypes(),
        ODatabaseDocument_Title::getTypes()
    );

    // strip XEmbeddedScripts and XScriptInvocationContext if we have any form/report
    // which already contains macros. In this case, the database document itself is not
    // allowed to contain macros, too.
    if ( !m_bAllowDocumentScripting )
    {
        Sequence< Type > aStrippedTypes( aTypes.getLength() );
        Type* pStripTo( aStrippedTypes.getArray() );

        // strip XEmbeddedScripts, and immediately re-assign to aTypes
        aTypes = Sequence< Type >(
            pStripTo,
            ::std::remove_copy_if(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                pStripTo,
                ::std::bind2nd( ::std::equal_to< Type >(), XEmbeddedScripts::static_type() )
            ) - pStripTo
        );

        // strip XScriptInvocationContext, and immediately re-assign to aTypes
        aTypes = Sequence< Type >(
            pStripTo,
            ::std::remove_copy_if(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                pStripTo,
                ::std::bind2nd( ::std::equal_to< Type >(), XScriptInvocationContext::static_type() )
            ) - pStripTo
        );
    }

    return aTypes;
}
//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ODatabaseDocument::getImplementationId(  ) throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// -----------------------------------------------------------------------------
// local functions
// -----------------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------------
    Reference< XStatusIndicator > lcl_extractStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments )
    {
        Reference< XStatusIndicator > xStatusIndicator;
        return _rArguments.getOrDefault( "StatusIndicator", xStatusIndicator );
    }

    // -----------------------------------------------------------------------------
    static void lcl_triggerStatusIndicator_throw( const ::comphelper::NamedValueCollection& _rArguments, DocumentGuard& _rGuard, const bool _bStart )
    {
        Reference< XStatusIndicator > xStatusIndicator( lcl_extractStatusIndicator( _rArguments ) );
        if ( !xStatusIndicator.is() )
            return;

        _rGuard.clear();
        try
        {
            if ( _bStart )
                xStatusIndicator->start( ::rtl::OUString(), (sal_Int32)1000000 );
            else
                xStatusIndicator->end();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        _rGuard.reset();
            // note that |reset| can throw a DisposedException
    }

    // -----------------------------------------------------------------------------
    static void lcl_extractStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments, Sequence< Any >& _rCallArgs )
    {
        Reference< XStatusIndicator > xStatusIndicator( lcl_extractStatusIndicator( _rArguments ) );
        if ( !xStatusIndicator.is() )
            return;

        sal_Int32 nLength = _rCallArgs.getLength();
        _rCallArgs.realloc( nLength + 1 );
        _rCallArgs[ nLength ] <<= xStatusIndicator;
    }

    // -----------------------------------------------------------------------------
    static void lcl_extractAndStartStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments, Reference< XStatusIndicator >& _rxStatusIndicator,
        Sequence< Any >& _rCallArgs )
    {
        _rxStatusIndicator = lcl_extractStatusIndicator( _rArguments );
        if ( !_rxStatusIndicator.is() )
            return;

        try
        {
            _rxStatusIndicator->start( ::rtl::OUString(), (sal_Int32)1000000 );

            sal_Int32 nLength = _rCallArgs.getLength();
            _rCallArgs.realloc( nLength + 1 );
            _rCallArgs[ nLength ] <<= _rxStatusIndicator;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // -----------------------------------------------------------------------------
    static Sequence< PropertyValue > lcl_appendFileNameToDescriptor( const Sequence< PropertyValue >& _rDescriptor, const ::rtl::OUString _rURL )
    {
        ::comphelper::NamedValueCollection aMediaDescriptor( _rDescriptor );
        if ( _rURL.getLength() )
        {
            aMediaDescriptor.put( "FileName", _rURL );
            aMediaDescriptor.put( "URL", _rURL );
        }
        return aMediaDescriptor.getPropertyValues();
    }
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_setInitialized()
{
    m_eInitState = Initialized;

    // start event notifications
    m_aEventNotifier.onDocumentInitialized();
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_reset_nothrow()
{
    try
    {
        m_pImpl->clearConnections();
        m_pImpl->disposeStorages();
        m_pImpl->resetRootStroage();

        clearObjectContainer( m_xForms );
        clearObjectContainer( m_xReports );
        clearObjectContainer( m_pImpl->m_xTableDefinitions );
        clearObjectContainer( m_pImpl->m_xCommandDefinitions );

        m_eInitState = NotInitialized;

        m_pImpl->reset();
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_pImpl->m_bDocumentReadOnly = sal_False;
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_import_nolck_throw( const ::comphelper::ComponentContext _rContext, const Reference< XInterface >& _rxTargetComponent,
                                                 const ::comphelper::NamedValueCollection& _rResource )
{
    Sequence< Any > aFilterArgs;
    Reference< XStatusIndicator > xStatusIndicator;
    lcl_extractAndStartStatusIndicator( _rResource, xStatusIndicator, aFilterArgs );

    /** property map for import info set */
    comphelper::PropertyMapEntry aExportInfoMap[] =
     {
        { MAP_LEN( "BaseURI"), 0,&::getCppuType( (::rtl::OUString *)0 ),beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamName"), 0,&::getCppuType( (::rtl::OUString *)0 ),beans::PropertyAttribute::MAYBEVOID, 0 },
          { NULL, 0, 0, NULL, 0, 0 }
     };
     uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );
    xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseURI")), uno::makeAny(_rResource.getOrDefault("URL",::rtl::OUString())));
    xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamName")), uno::makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml"))));

    const sal_Int32 nCount = aFilterArgs.getLength();
    aFilterArgs.realloc(nCount + 1);
    aFilterArgs[nCount] <<= xInfoSet;

    Reference< XImporter > xImporter(
        _rContext.createComponentWithArguments( "com.sun.star.comp.sdb.DBFilter", aFilterArgs ),
        UNO_QUERY_THROW );

    Reference< XComponent > xComponent( _rxTargetComponent, UNO_QUERY_THROW );
    xImporter->setTargetDocument( xComponent );

    Reference< XFilter > xFilter( xImporter, UNO_QUERY_THROW );
    xFilter->filter( ODatabaseModelImpl::stripLoadArguments( _rResource ) );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::initNew(  ) throw (DoubleInitializationException, IOException, Exception, RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::InitMethod );

    impl_reset_nothrow();

    impl_setInitializing();

    // create a temporary storage
    Reference< XStorage > xTempStor( ::comphelper::OStorageHelper::GetTemporaryStorage(
        m_pImpl->m_aContext.getLegacyServiceFactory() ) );

    // store therein
    impl_storeToStorage_throw( xTempStor, Sequence< PropertyValue >(), aGuard );

    // let the impl know we're now based on this storage
    m_pImpl->switchToStorage( xTempStor );

    // for the newly created document, allow document-wide scripting
    m_bAllowDocumentScripting = true;

    impl_setInitialized();

    m_aEventNotifier.notifyDocumentEventAsync( "OnTitleChanged" );

    impl_setModified_nothrow( sal_False, aGuard );
    // <- SYNCHRONIZED

    m_aEventNotifier.notifyDocumentEvent( "OnCreate" );

    impl_notifyStorageChange_nolck_nothrow( xTempStor );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::load( const Sequence< PropertyValue >& _Arguments ) throw (DoubleInitializationException, IOException, Exception, RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::InitMethod );

    impl_reset_nothrow();

    ::comphelper::NamedValueCollection aResource( _Arguments );
    if ( aResource.has( "FileName" ) && !aResource.has( "URL" ) )
        // FileName is the compatibility name for URL, so we might have clients passing
        // a FileName only. However, some of our code works with the URL only, so ensure
        // we have one.
        aResource.put( "URL", aResource.get( "FileName" ) );
    if ( aResource.has( "URL" ) && !aResource.has( "FileName" ) )
        // similar ... just in case there is legacy code which expects a FileName only
        aResource.put( "FileName", aResource.get( "URL" ) );

    // now that somebody (perhaps) told us an macro execution mode, remember it as
    // ImposedMacroExecMode
    m_pImpl->setImposedMacroExecMode(
        aResource.getOrDefault( "MacroExecutionMode", m_pImpl->getImposedMacroExecMode() ) );

    impl_setInitializing();
    try
    {
        aGuard.clear();
        impl_import_nolck_throw( m_pImpl->m_aContext, *this, aResource );
        aGuard.reset();
    }
    catch( const Exception& )
    {
        impl_reset_nothrow();
        throw;
    }
    // tell our view monitor that the document has been loaded - this way it will fire the proper
    // event (OnLoad instead of OnCreate) later on
    m_aViewMonitor.onLoadedDocument();

    // note that we do *not* call impl_setInitialized() here: The initialization is only complete
    // when the XModel::attachResource has been called, not sooner.

    impl_setModified_nothrow( sal_False, aGuard );
    // <- SYNCHRONIZED
}

// -----------------------------------------------------------------------------
// XModel
sal_Bool SAL_CALL ODatabaseDocument::attachResource( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );

    if  (   ( _rURL == getURL() )
        &&  ( _rArguments.getLength() == 1 )
        &&  ( _rArguments[0].Name.compareToAscii( "BreakMacroSignature" ) == 0 )
        )
    {
        // this is a BAD hack of the Basic importer code ... there should be a dedicated API for this,
        // not this bad mis-using of existing interfaces
        return sal_False;
            // (we do not support macro signatures, so we can ignore this call)
    }

    m_pImpl->attachResource( _rURL, _rArguments );

    if ( impl_isInitializing() )
    {   // this means we've just been loaded, and this is the attachResource call which follows
        // the load call.
        impl_setInitialized();

        // determine whether the document as a whole, or sub documents, have macros. Especially the latter
        // controls the availability of our XEmbeddedScripts and XScriptInvocationContext interfaces, and we
        // should know this before anybody actually uses the object.
        m_bAllowDocumentScripting = ( m_pImpl->determineEmbeddedMacros() != ODatabaseModelImpl::eSubDocumentMacros );

        m_aEventNotifier.notifyDocumentEvent( "OnLoadFinished" );
    }

    return sal_True;
}

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getURL(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getURL();
}

// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getArgs(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getResource();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::connectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    m_aControllers.push_back( _xController );

    m_aEventNotifier.notifyDocumentEventAsync( "OnViewCreated", Reference< XController2 >( _xController, UNO_QUERY ) );

    bool bFirstControllerEver = m_aViewMonitor.onControllerConnected( _xController );
    if ( !bFirstControllerEver )
        return;

    // check/adjust our macro mode.
    m_pImpl->checkMacrosOnLoading();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::disconnectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    bool bNotifyViewClosed = false;
    bool bLastControllerGone = false;
    bool bIsClosing = false;

    // SYNCHRONIZED ->
    {
        DocumentGuard aGuard( *this );

        Controllers::iterator pos = ::std::find( m_aControllers.begin(), m_aControllers.end(), _xController );
        OSL_ENSURE( pos != m_aControllers.end(), "ODatabaseDocument::disconnectController: don't know this controller!" );
        if ( pos != m_aControllers.end() )
        {
            m_aControllers.erase( pos );
            bNotifyViewClosed = true;
        }

        if ( m_xCurrentController == _xController )
            m_xCurrentController = NULL;

        bLastControllerGone = m_aControllers.empty();
        bIsClosing = m_bClosing;
    }
    // <- SYNCHRONIZED

    if ( bNotifyViewClosed )
        m_aEventNotifier.notifyDocumentEvent( "OnViewClosed", Reference< XController2 >( _xController, UNO_QUERY ) );

    if ( bLastControllerGone && !bIsClosing )
    {
        // if this was the last view, close the document as a whole
        // #i51157# / 2006-03-16 / frank.schoenheit@sun.com
        try
        {
            close( sal_True );
        }
        catch( const CloseVetoException& )
        {
            // okay, somebody vetoed and took ownership
        }
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::lockControllers(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    ++m_pImpl->m_nControllerLockCount;
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::unlockControllers(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    --m_pImpl->m_nControllerLockCount;
}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::hasControllersLocked(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    return m_pImpl->m_nControllerLockCount != 0;
}

// -----------------------------------------------------------------------------
Reference< XController > SAL_CALL ODatabaseDocument::getCurrentController() throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    return m_xCurrentController.is() ? m_xCurrentController : ( m_aControllers.empty() ? Reference< XController >() : *m_aControllers.begin() );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setCurrentController( const Reference< XController >& _xController ) throw (NoSuchElementException, RuntimeException)
{
    DocumentGuard aGuard( *this );

    m_xCurrentController = _xController;

    m_aViewMonitor.onSetCurrentController( _xController );
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseDocument::getCurrentSelection(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    Reference< XInterface > xRet;
    Reference< XSelectionSupplier >  xDocView( getCurrentController(), UNO_QUERY );
    if ( xDocView.is() )
        xRet.set(xDocView->getSelection(),UNO_QUERY);

    return xRet;
}
// -----------------------------------------------------------------------------

// XStorable
sal_Bool SAL_CALL ODatabaseDocument::hasLocation(  ) throw (RuntimeException)
{
    return getLocation().getLength() > 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getLocation(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getURL();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::isReadonly(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->m_bDocumentReadOnly;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::store(  ) throw (IOException, RuntimeException)
{
    DocumentGuard aGuard( *this );

    if ( m_pImpl->getDocFileLocation() == m_pImpl->getURL() )
        if ( m_pImpl->m_bDocumentReadOnly )
            throw IOException();

    impl_storeAs_throw( m_pImpl->getURL(), m_pImpl->getResource(), SAVE, aGuard );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_storeAs_throw( const ::rtl::OUString& _rURL, const Sequence< PropertyValue>& _rArguments,
    const StoreType _eType, DocumentGuard& _rGuard ) throw ( IOException, RuntimeException )
{
    OSL_PRECOND( ( _eType == SAVE ) || ( _eType == SAVE_AS ),
        "ODatabaseDocument::impl_storeAs_throw: you introduced a new type which cannot be handled here!" );

    // if we're in the process of initializing the document (which effectively means it is an implicit
    // initialization triggered in storeAsURL), the we do not notify events, since to an observer, the SaveAs
    // should not be noticable
    bool bIsInitializationProcess = impl_isInitializing();

    if ( !bIsInitializationProcess )
    {
        _rGuard.clear();
        m_aEventNotifier.notifyDocumentEvent( _eType == SAVE ? "OnSave" : "OnSaveAs", NULL, makeAny( _rURL ) );
        _rGuard.reset();
    }

    Reference< XStorage > xNewRootStorage;
        // will be non-NULL if our storage changed

    try
    {
        ModifyLock aLock( *this );
            // ignore all changes of our "modified" state during storing

        sal_Bool bLocationChanged = ( _rURL != m_pImpl->getDocFileLocation() );
        if ( bLocationChanged )
        {
            // create storage for target URL
            Reference< XStorage > xTargetStorage( impl_createStorageFor_throw( _rURL ) );

            if ( m_pImpl->isEmbeddedDatabase() )
                m_pImpl->clearConnections();

            // commit everything
            m_pImpl->commitEmbeddedStorage();
            m_pImpl->commitStorages();

            // copy own storage to target storage
            Reference< XStorage > xCurrentStorage( m_pImpl->getRootStorage() );
            if ( xCurrentStorage.is() )
                xCurrentStorage->copyToStorage( xTargetStorage );

            m_pImpl->disposeStorages();

            xNewRootStorage = m_pImpl->switchToStorage( xTargetStorage );

            m_pImpl->m_bDocumentReadOnly = sal_False;
        }

        // store to current storage
        Reference< XStorage > xCurrentStorage( m_pImpl->getOrCreateRootStorage(), UNO_QUERY_THROW );
        Sequence< PropertyValue > aMediaDescriptor( lcl_appendFileNameToDescriptor( _rArguments, _rURL ) );
        impl_storeToStorage_throw( xCurrentStorage, aMediaDescriptor, _rGuard );

        // success - tell our impl
        m_pImpl->attachResource( _rURL, aMediaDescriptor );

        // if we are in an initialization process, then this is finished, now that we stored the document
        if ( bIsInitializationProcess )
            impl_setInitialized();
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();

        // notify the failure
        if ( !bIsInitializationProcess )
            m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveFailed" : "OnSaveAsFailed", NULL, makeAny( _rURL ) );

        if  (   aError.isExtractableTo( ::cppu::UnoType< IOException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() )
            )
        {
            // allowed to leave
            throw;
        }

        Exception aExcept;
        aError >>= aExcept;

        ::rtl::OUString sErrorMessage = ResourceManager::loadString(
            RID_STR_ERROR_WHILE_SAVING,
            "$except$", aError.getValueTypeName(),
            "$message$", aExcept.Message
        );
        throw IOException( sErrorMessage, *this );
    }

    // notify the document event
    if ( !bIsInitializationProcess )
        m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveDone" : "OnSaveAsDone", NULL, makeAny( _rURL ) );

    // reset our "modified" flag, and clear the guard
    impl_setModified_nothrow( sal_False, _rGuard );
    // <- SYNCHRONIZED

    // notify storage listeners
    if ( xNewRootStorage.is() )
        impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

// -----------------------------------------------------------------------------
Reference< XStorage > ODatabaseDocument::impl_createStorageFor_throw( const ::rtl::OUString& _rURL ) const
{
    Sequence<Any> aParam(2);
    aParam[0] <<= _rURL;
    aParam[1] <<= ElementModes::READWRITE | ElementModes::TRUNCATE;

    Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory(), UNO_SET_THROW );
    return Reference< XStorage >( xStorageFactory->createInstanceWithArguments( aParam ), UNO_QUERY_THROW );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeAsURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );

    // Normally, a document initialization is done via XLoadable::load or XLoadable::initNew. For convenience
    // reasons, and to not break existing API clients, it's allowed to call storeAsURL without having initialized
    // the document, in which case the initialization will be done implicitly.
    bool bImplicitInitialization = !impl_isInitialized();
    // implicit initialization while another initialization is just running is not possible
    if ( bImplicitInitialization && impl_isInitializing() )
        throw DoubleInitializationException();

    if ( bImplicitInitialization )
        impl_setInitializing();

    try
    {
        impl_storeAs_throw( _rURL, _rArguments, SAVE_AS, aGuard );
        // <- SYNCHRONIZED

        // impl_storeAs_throw cleared the lock on our mutex, but the below lines need this lock
        // SYNCHRONIZED ->
        aGuard.reset();

        // our title might have changed, potentially at least
        // Sadly, we cannot check this: Calling getTitle here and now would not deliver
        // an up-to-date result, as the call is delegated to our TitleHelper instance, which itself
        // updates its title only if it gets the OnSaveAsDone event (which was sent asynchronously
        // by impl_storeAs_throw). So, we simply notify always, and also asynchronously
        m_aEventNotifier.notifyDocumentEventAsync( "OnTitleChanged" );
    }
    catch( const Exception& )
    {
        impl_reset_nothrow();
        throw;
    }

    if ( bImplicitInitialization )
        m_bAllowDocumentScripting = true;

    aGuard.clear();
    // <- SYNCHRONIZED

    if ( bImplicitInitialization )
        m_aEventNotifier.notifyDocumentEvent( "OnCreate" );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_storeToStorage_throw( const Reference< XStorage >& _rxTargetStorage, const Sequence< PropertyValue >& _rMediaDescriptor,
                                                   DocumentGuard& _rDocGuard ) const
{
    if ( !_rxTargetStorage.is() )
        throw IllegalArgumentException( ::rtl::OUString(), *const_cast< ODatabaseDocument* >( this ), 1 );

    if ( !m_pImpl.is() )
        throw DisposedException( ::rtl::OUString(), *const_cast< ODatabaseDocument* >( this ) );

    try
    {
        // commit everything
        m_pImpl->commitEmbeddedStorage();
        m_pImpl->commitStorages();

        // copy own storage to target storage
        if ( impl_isInitialized() )
        {
            Reference< XStorage > xCurrentStorage( m_pImpl->getOrCreateRootStorage(), UNO_QUERY_THROW );
            if ( xCurrentStorage != _rxTargetStorage )
                xCurrentStorage->copyToStorage( _rxTargetStorage );
        }

        // write into target storage
        ::comphelper::NamedValueCollection aWriteArgs( _rMediaDescriptor );
        lcl_triggerStatusIndicator_throw( aWriteArgs, _rDocGuard, true );
        impl_writeStorage_throw( _rxTargetStorage, aWriteArgs );
        lcl_triggerStatusIndicator_throw( aWriteArgs, _rDocGuard, false );

        // commit target storage
        OSL_VERIFY( ODatabaseModelImpl::commitStorageIfWriteable( _rxTargetStorage ) );
    }
    catch( const IOException& ) { throw; }
    catch( const RuntimeException& ) { throw; }
    catch ( const Exception& e )
    {
        throw IOException( e.Message, *const_cast< ODatabaseDocument* >( this ) );
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeToURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    DocumentGuard aGuard( *this );
    ModifyLock aLock( *this );

    {
        aGuard.clear();
        m_aEventNotifier.notifyDocumentEvent( "OnSaveTo", NULL, makeAny( _rURL ) );
        aGuard.reset();
    }

    try
    {
        // create storage for target URL
        Reference< XStorage > xTargetStorage( impl_createStorageFor_throw( _rURL ) );

        // extend media descriptor with URL
        Sequence< PropertyValue > aMediaDescriptor( lcl_appendFileNameToDescriptor( _rArguments, _rURL ) );

        // store to this storage
        impl_storeToStorage_throw( xTargetStorage, aMediaDescriptor, aGuard );
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();
        m_aEventNotifier.notifyDocumentEventAsync( "OnSaveToFailed", NULL, aError );

        if  (   aError.isExtractableTo( ::cppu::UnoType< IOException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() )
            )
        {
            // allowed to leave
            throw;
        }

        Exception aExcept;
        aError >>= aExcept;

        ::rtl::OUString sErrorMessage = ResourceManager::loadString(
            RID_STR_ERROR_WHILE_SAVING,
            "$except$", aError.getValueTypeName(),
            "$message$", aExcept.Message
        );
        throw IOException( sErrorMessage, *this );
    }

    m_aEventNotifier.notifyDocumentEventAsync( "OnSaveToDone", NULL, makeAny( _rURL ) );
}

// -----------------------------------------------------------------------------
// XModifyBroadcaster
void SAL_CALL ODatabaseDocument::addModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aModifyListeners.addInterface(_xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aModifyListeners.removeInterface(_xListener);
}

// -----------------------------------------------------------------------------
// XModifiable
sal_Bool SAL_CALL ODatabaseDocument::isModified(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    return m_pImpl->m_bModified;
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setModified( sal_Bool _bModified ) throw (PropertyVetoException, RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    if ( impl_isInitialized() )
        impl_setModified_nothrow( _bModified, aGuard );
    // it's allowed to call setModified without the document being initialized already. In this case,
    // we simply ignore the call - when the initialization is finished, the respective code will set
    // a proper "modified" flag
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_setModified_nothrow( sal_Bool _bModified, DocumentGuard& _rGuard )
{
    // SYNCHRONIZED ->
    bool bModifiedChanged = ( m_pImpl->m_bModified != _bModified ) && ( !m_pImpl->isModifyLocked() );

    if ( bModifiedChanged )
    {
        m_pImpl->m_bModified = _bModified;
        m_aEventNotifier.notifyDocumentEventAsync( "OnModifyChanged" );
    }
    _rGuard.clear();
    // <- SYNCHRONIZED

    if ( bModifiedChanged )
    {
        lang::EventObject aEvent( *this );
        m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }
}

// -----------------------------------------------------------------------------
// ::com::sun::star::document::XEventBroadcaster
void SAL_CALL ODatabaseDocument::addEventListener(const uno::Reference< document::XEventListener >& _Listener ) throw (uno::RuntimeException)
{
    m_aEventNotifier.addLegacyEventListener( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeEventListener( const uno::Reference< document::XEventListener >& _Listener ) throw (uno::RuntimeException)
{
    m_aEventNotifier.removeLegacyEventListener( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addDocumentEventListener( const Reference< XDocumentEventListener >& _Listener ) throw (RuntimeException)
{
    m_aEventNotifier.addDocumentEventListener( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeDocumentEventListener( const Reference< XDocumentEventListener >& _Listener ) throw (RuntimeException)
{
    m_aEventNotifier.removeDocumentEventListener( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::notifyDocumentEvent( const ::rtl::OUString& _EventName, const Reference< XController2 >& _ViewController, const Any& _Supplement ) throw (IllegalArgumentException, NoSupportException, RuntimeException)
{
    if ( !_EventName.getLength() )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this );

    if ( !DocumentEvents::needsSynchronousNotification( _EventName ) )
    {
        m_aEventNotifier.notifyDocumentEventAsync( _EventName, _ViewController, _Supplement );
        return;
    }
    aGuard.clear();
    // <- SYNCHRONIZED

    m_aEventNotifier.notifyDocumentEvent( _EventName, _ViewController, _Supplement );
}

// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getPrinter(  ) throw (RuntimeException)
{
    DBG_ERROR( "ODatabaseDocument::getPrinter: not supported!" );
    return Sequence< PropertyValue >();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setPrinter( const Sequence< PropertyValue >& /*aPrinter*/ ) throw (IllegalArgumentException, RuntimeException)
{
    DBG_ERROR( "ODatabaseDocument::setPrinter: not supported!" );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::print( const Sequence< PropertyValue >& /*xOptions*/ ) throw (IllegalArgumentException, RuntimeException)
{
    DBG_ERROR( "ODatabaseDocument::print: not supported!" );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_reparent_nothrow( const WeakReference< XNameAccess >& _rxContainer )
{
    Reference< XChild > xChild( _rxContainer.get(), UNO_QUERY );
    if  ( xChild.is() )
        xChild->setParent( *this );
}
// -----------------------------------------------------------------------------
void ODatabaseDocument::clearObjectContainer( WeakReference< XNameAccess >& _rxContainer)
{
    Reference< XNameAccess > xContainer = _rxContainer;
    ::comphelper::disposeComponent( xContainer );

    Reference< XChild > xChild( _rxContainer.get(),UNO_QUERY );
    if ( xChild.is() )
        xChild->setParent( NULL );
    _rxContainer = Reference< XNameAccess >();
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > ODatabaseDocument::impl_getDocumentContainer_throw( ODatabaseModelImpl::ObjectType _eType )
{
    if ( ( _eType != ODatabaseModelImpl::E_FORM ) && ( _eType != ODatabaseModelImpl::E_REPORT ) )
        throw IllegalArgumentException();

    bool bFormsContainer = _eType == ODatabaseModelImpl::E_FORM;

    WeakReference< XNameAccess >& rContainerRef( bFormsContainer ? m_xForms : m_xReports );
    Reference< XNameAccess > xContainer = rContainerRef;
    if ( !xContainer.is() )
    {
        TContentPtr& rContainerData( m_pImpl->getObjectContainer( _eType ) );
        rContainerRef = xContainer = new ODocumentContainer( m_pImpl->m_aContext.getLegacyServiceFactory(), *this, rContainerData, bFormsContainer );
        impl_reparent_nothrow( xContainer );
    }
    return xContainer;
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_closeControllerFrames_nolck_throw( sal_Bool _bDeliverOwnership )
{
    Controllers aCopy = m_aControllers;

    Controllers::iterator aEnd = aCopy.end();
    for ( Controllers::iterator aIter = aCopy.begin(); aIter != aEnd ; ++aIter )
    {
        if ( !aIter->is() )
            continue;

        try
        {
            Reference< XCloseable> xFrame( (*aIter)->getFrame(), UNO_QUERY );
            if ( xFrame.is() )
                xFrame->close( _bDeliverOwnership );
        }
        catch( const CloseVetoException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

// -----------------------------------------------------------------------------
struct DisposeControllerFrame : public ::std::unary_function< Reference< XController >, void >
{
    void operator()( const Reference< XController >& _rxController ) const
    {
        try
        {
            if ( !_rxController.is() )
                return;

            Reference< XFrame > xFrame( _rxController->getFrame() );
            ::comphelper::disposeComponent( xFrame );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    };
};

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_disposeControllerFrames_nothrow()
{
    Controllers aCopy;
    aCopy.swap( m_aControllers );   // ensure m_aControllers is empty afterwards
    ::std::for_each( aCopy.begin(), aCopy.end(), DisposeControllerFrame() );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::close( sal_Bool _bDeliverOwnership ) throw (CloseVetoException, RuntimeException)
{
    // nearly everything below can/must be done without our mutex locked, the below is just for
    // the checks for being disposed and the like
    // SYNCHRONIZED ->
    {
        DocumentGuard aGuard( *this );
        m_bClosing = true;
    }
    // <- SYNCHRONIZED

    try
    {
        // allow listeners to veto
        lang::EventObject aEvent( *this );
        m_aCloseListener.forEach< XCloseListener >(
            boost::bind( &XCloseListener::queryClosing, _1, boost::cref( aEvent ), boost::cref( _bDeliverOwnership ) ) );

        // notify that we're going to unload
        m_aEventNotifier.notifyDocumentEvent( "OnPrepareUnload" );

        impl_closeControllerFrames_nolck_throw( _bDeliverOwnership );

        m_aCloseListener.notifyEach( &XCloseListener::notifyClosing, (const lang::EventObject&)aEvent );

        dispose();
    }
    catch ( const Exception& )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bClosing = false;
        throw;
    }

    // SYNCHRONIZED ->
    ::osl::MutexGuard aGuard( m_aMutex );
    m_bClosing = false;
    // <- SYNCHRONIZED
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aCloseListener.addInterface(Listener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aCloseListener.removeInterface(Listener);
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseDocument::getFormDocuments(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getDocumentContainer_throw( ODatabaseModelImpl::E_FORM );
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseDocument::getReportDocuments(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getDocumentContainer_throw( ODatabaseModelImpl::E_REPORT );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::WriteThroughComponent( const Reference< XComponent >& xComponent, const sal_Char* pStreamName,
    const sal_Char* pServiceName, const Sequence< Any >& _rArguments, const Sequence< PropertyValue >& rMediaDesc,
    const Reference<XStorage>& _xStorageToSaveTo ) const
{
    OSL_ENSURE( pStreamName, "Need stream name!" );
    OSL_ENSURE( pServiceName, "Need service name!" );

    // open stream
    ::rtl::OUString sStreamName = ::rtl::OUString::createFromAscii( pStreamName );
    Reference< XStream > xStream = _xStorageToSaveTo->openStreamElement( sStreamName, ElementModes::READWRITE | ElementModes::TRUNCATE );
    if ( !xStream.is() )
        return;

    Reference< XOutputStream > xOutputStream( xStream->getOutputStream() );
    OSL_ENSURE( xOutputStream.is(), "Can't create output stream in package!" );
    if ( !xOutputStream.is() )
        return;

    Reference< XSeekable > xSeek( xOutputStream, UNO_QUERY );
    if ( xSeek.is() )
        xSeek->seek(0);

    Reference< XPropertySet > xStreamProp( xOutputStream, UNO_QUERY_THROW );
    xStreamProp->setPropertyValue( INFO_MEDIATYPE, makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) ) ) );
    xStreamProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ), makeAny( (sal_Bool)sal_True ) );

    // write the stuff
    WriteThroughComponent( xOutputStream, xComponent, pServiceName, _rArguments, rMediaDesc );
}

void ODatabaseDocument::WriteThroughComponent( const Reference< XOutputStream >& xOutputStream,
    const Reference< XComponent >& xComponent, const sal_Char* pServiceName, const Sequence< Any >& _rArguments,
    const Sequence< PropertyValue >& rMediaDesc ) const
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( NULL != pServiceName, "Need component name!" );

    // get component
    Reference< XActiveDataSource > xSaxWriter;
    OSL_VERIFY( m_pImpl->m_aContext.createComponent( "com.sun.star.xml.sax.Writer", xSaxWriter ) );
    if ( !xSaxWriter.is() )
        return;

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference< XDocumentHandler > xDocHandler( xSaxWriter,UNO_QUERY);
    Sequence<Any> aArgs( 1 + _rArguments.getLength() );
    aArgs[0] <<= xDocHandler;
    for ( sal_Int32 i = 0; i < _rArguments.getLength(); ++i )
        aArgs[ i+1 ] = _rArguments[i];

    // get filter component
    Reference< XExporter > xExporter;
    OSL_VERIFY( m_pImpl->m_aContext.createComponentWithArguments( pServiceName, aArgs, xExporter ) );
    if ( !xExporter.is() )
        return;

    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter
    Reference< XFilter > xFilter( xExporter, UNO_QUERY_THROW );
    xFilter->filter( rMediaDesc );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_writeStorage_throw( const Reference< XStorage >& _rxTargetStorage, const ::comphelper::NamedValueCollection& _rMediaDescriptor ) const
{
    // extract status indicator
    Sequence< Any > aDelegatorArguments;
    lcl_extractStatusIndicator( _rMediaDescriptor, aDelegatorArguments );

    /** property map for export info set */
    comphelper::PropertyMapEntry aExportInfoMap[] =
    {
        { MAP_LEN( "BaseURI"), 0,&::getCppuType( (::rtl::OUString *)0 ),beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamName"), 0,&::getCppuType( (::rtl::OUString *)0 ),beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "UsePrettyPrinting" ), 0, &::getCppuType((sal_Bool*)0), beans::PropertyAttribute::MAYBEVOID, 0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    SvtSaveOptions aSaveOpt;
    xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting")), uno::makeAny(aSaveOpt.IsPrettyPrinting()));
    if ( aSaveOpt.IsSaveRelFSys() )
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseURI")), uno::makeAny(_rMediaDescriptor.getOrDefault("URL",::rtl::OUString())));

    sal_Int32 nArgsLen = aDelegatorArguments.getLength();
    aDelegatorArguments.realloc(nArgsLen+1);
    aDelegatorArguments[nArgsLen++] <<= xInfoSet;

    Reference< XPropertySet > xProp( _rxTargetStorage, UNO_QUERY_THROW );
    xProp->setPropertyValue( INFO_MEDIATYPE, makeAny( (rtl::OUString)MIMETYPE_OASIS_OPENDOCUMENT_DATABASE ) );

    Reference< XComponent > xComponent( *const_cast< ODatabaseDocument* >( this ), UNO_QUERY_THROW );

    Sequence< PropertyValue > aMediaDescriptor;
    _rMediaDescriptor >>= aMediaDescriptor;

    xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamName")), uno::makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("settings.xml"))));
    WriteThroughComponent( xComponent, "settings.xml", "com.sun.star.comp.sdb.XMLSettingsExporter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamName")), uno::makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml"))));
    WriteThroughComponent( xComponent, "content.xml", "com.sun.star.comp.sdb.DBExportFilter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    m_pImpl->storeLibraryContainersTo( _rxTargetStorage );
}

// -----------------------------------------------------------------------------
Reference< XUIConfigurationManager > SAL_CALL ODatabaseDocument::getUIConfigurationManager(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    if ( !m_xUIConfigurationManager.is() )
    {
        m_pImpl->m_aContext.createComponent( "com.sun.star.ui.UIConfigurationManager", m_xUIConfigurationManager );
        Reference< XUIConfigurationStorage > xUIConfigStorage( m_xUIConfigurationManager, UNO_QUERY );
        if ( xUIConfigStorage.is() )
        {
            rtl::OUString aUIConfigFolderName( RTL_CONSTASCII_USTRINGPARAM( "Configurations2" ));
            Reference< XStorage > xConfigStorage;

            // First try to open with READWRITE and then READ
            xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, ElementModes::READWRITE );
            if ( xConfigStorage.is() )
            {
                rtl::OUString aUIConfigMediaType( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.xml.ui.configuration" ));
                rtl::OUString aMediaType;
                Reference< XPropertySet > xPropSet( xConfigStorage, UNO_QUERY );
                Any a = xPropSet->getPropertyValue( INFO_MEDIATYPE );
                if ( !( a >>= aMediaType ) || ( aMediaType.getLength() == 0 ))
                {
                    a <<= aUIConfigMediaType;
                    xPropSet->setPropertyValue( INFO_MEDIATYPE, a );
                }
            }
            else
                xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, ElementModes::READ );

            // initialize ui configuration manager with document substorage
            xUIConfigStorage->setStorage( xConfigStorage );
        }
    }

    return m_xUIConfigurationManager;
}
// -----------------------------------------------------------------------------
Reference< XStorage > SAL_CALL ODatabaseDocument::getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStorage( aStorageName, nMode );
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseDocument::getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, RuntimeException)
{
    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStoragesNames();
}

//------------------------------------------------------------------------------
void ODatabaseDocument::impl_notifyStorageChange_nolck_nothrow( const Reference< XStorage >& _rxNewRootStorage )
{
    Reference< XInterface > xMe( *const_cast< ODatabaseDocument* >( this ) );

    m_aStorageListeners.forEach< XStorageChangeListener >(
        boost::bind( &XStorageChangeListener::notifyStorageChange, _1, boost::cref( xMe ), boost::cref( _rxNewRootStorage ) ) );
}

//------------------------------------------------------------------------------
void ODatabaseDocument::disposing()
{
    if ( !m_pImpl.is() )
    {
        // this means that we're already disposed
        DBG_ASSERT( ODatabaseDocument_OfficeDocument::rBHelper.bDisposed, "ODatabaseDocument::disposing: no impl anymore, but not yet disposed!" );
        return;
    }

    if ( impl_isInitialized() )
        m_aEventNotifier.notifyDocumentEvent( "OnUnload" );

    Reference< XModel > xHoldAlive( this );

    m_aEventNotifier.disposing();

    lang::EventObject aDisposeEvent(static_cast<XWeak*>(this));
    m_aModifyListeners.disposeAndClear( aDisposeEvent );
    m_aCloseListener.disposeAndClear( aDisposeEvent );
    m_aStorageListeners.disposeAndClear( aDisposeEvent );

    // this is the list of objects which we currently hold as member. Upon resetting
    // those members, we can (potentially) release the last reference to them, in which
    // case they will be deleted - if they're C++ implementations, that is :).
    // Some of those implementations are offending enough to require the SolarMutex, which
    // means we should not release the last reference while our own mutex is locked ...
    ::std::list< Reference< XInterface > > aKeepAlive;

    // SYNCHRONIZED ->
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    DBG_ASSERT( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
        // normally, nobody should explicitly dispose, but only XCloseable::close the document. And upon
        // closing, our controllers are closed, too

    aKeepAlive.push_back( m_xUIConfigurationManager );
    m_xUIConfigurationManager = NULL;

    clearObjectContainer( m_xForms );
    clearObjectContainer( m_xReports );

    // reset the macro mode: in case the our impl struct stays alive (e.g. because our DataSource
    // object still exists), and somebody subsequently re-opens the document, we want to have
    // the security warning, again.
    m_pImpl->resetMacroExecutionMode();

    // similar argueing for our ViewMonitor
    m_aViewMonitor.reset();

    // tell our Impl to forget us
    m_pImpl->modelIsDisposing( impl_isInitialized(), ODatabaseModelImpl::ResetModelAccess() );

    // now, at the latest, the controller array should be empty. Controllers are
    // expected to listen for our disposal, and disconnect then
    DBG_ASSERT( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
    impl_disposeControllerFrames_nothrow();

    aKeepAlive.push_back( m_xModuleManager );
    m_xModuleManager.clear();

    aKeepAlive.push_back( m_xTitleHelper );
    m_xTitleHelper.clear();

    m_pImpl.clear();

    aGuard.clear();
    // <- SYNCHRONIZED

    aKeepAlive.clear();
}
// -----------------------------------------------------------------------------
// XComponent
void SAL_CALL ODatabaseDocument::dispose(  ) throw (RuntimeException)
{
    ::cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addEventListener( const Reference< lang::XEventListener >& _xListener ) throw (RuntimeException)
{
    ::cppu::WeakComponentImplHelperBase::addEventListener( _xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeEventListener( const Reference< lang::XEventListener >& _xListener ) throw (RuntimeException)
{
    ::cppu::WeakComponentImplHelperBase::removeEventListener( _xListener );
}
// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODatabaseDocument::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}

//------------------------------------------------------------------------------
rtl::OUString ODatabaseDocument::getImplementationName_static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.dba.ODatabaseDocument");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseDocument::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------------
Reference< XInterface > ODatabaseDocument::Create( const Reference< XComponentContext >& _rxContext )
{
    ::comphelper::ComponentContext aContext( _rxContext );
    Reference< XUnoTunnel > xDBContextTunnel( aContext.createComponent( (::rtl::OUString)SERVICE_SDB_DATABASECONTEXT ), UNO_QUERY_THROW );
    ODatabaseContext* pContext = reinterpret_cast< ODatabaseContext* >( xDBContextTunnel->getSomething( ODatabaseContext::getUnoTunnelImplementationId() ) );

    ::rtl::Reference<ODatabaseModelImpl> pImpl( new ODatabaseModelImpl( aContext.getLegacyServiceFactory(), *pContext ) );
    Reference< XModel > xModel( pImpl->createNewModel_deliverOwnership( false ) );
    return xModel.get();
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseDocument::getSupportedServiceNames_static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OfficeDatabaseDocument"));
    aSNS[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OfficeDocument"));
    return aSNS;
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseDocument::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
// -----------------------------------------------------------------------------
Reference< XDataSource > SAL_CALL ODatabaseDocument::getDataSource() throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getOrCreateDataSource();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::loadFromStorage( const Reference< XStorage >& /*xStorage*/, const Sequence< PropertyValue >& /*aMediaDescriptor*/ ) throw (IllegalArgumentException, DoubleInitializationException, IOException, Exception, RuntimeException)
{
    DocumentGuard aGuard( *this );

    throw Exception(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Embedding of database documents is not supported." ) ),
            // TODO: resource
        *this
    );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeToStorage( const Reference< XStorage >& _rxStorage, const Sequence< PropertyValue >& _rMediaDescriptor ) throw (IllegalArgumentException, IOException, Exception, RuntimeException)
{
    DocumentGuard aGuard( *this );
    impl_storeToStorage_throw( _rxStorage, _rMediaDescriptor, aGuard );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::switchToStorage( const Reference< XStorage >& _rxNewRootStorage ) throw (IllegalArgumentException, IOException, Exception, RuntimeException)
{
    DocumentGuard aGuard( *this );

    Reference< XStorage > xNewRootStorage( m_pImpl->switchToStorage( _rxNewRootStorage ) );

    aGuard.clear();
    impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

// -----------------------------------------------------------------------------
Reference< XStorage > SAL_CALL ODatabaseDocument::getDocumentStorage(  ) throw (IOException, Exception, RuntimeException)
{
    DocumentGuard aGuard( *this );
    return m_pImpl->getOrCreateRootStorage();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aStorageListeners.addInterface( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    m_aStorageListeners.addInterface( _Listener );
}

// -----------------------------------------------------------------------------
Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getBasicLibraries() throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return m_pImpl->getLibraryContainer( true );
}

// -----------------------------------------------------------------------------
Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getDialogLibraries() throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    return m_pImpl->getLibraryContainer( false );
}

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL ODatabaseDocument::getAllowMacroExecution() throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    return m_pImpl->adjustMacroMode_AutoReject();
}

// -----------------------------------------------------------------------------
Reference< XEmbeddedScripts > SAL_CALL ODatabaseDocument::getScriptContainer() throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    return this;
}

// -----------------------------------------------------------------------------
Reference< provider::XScriptProvider > SAL_CALL ODatabaseDocument::getScriptProvider(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );

    Reference< XScriptProvider > xScriptProvider( m_xScriptProvider );
    if ( !xScriptProvider.is() )
    {
        Reference < XScriptProviderFactory > xFactory(
            m_pImpl->m_aContext.getSingleton( "com.sun.star.script.provider.theMasterScriptProviderFactory" ), UNO_QUERY_THROW );

        Any aScriptProviderContext;
        if ( m_bAllowDocumentScripting )
            aScriptProviderContext <<= Reference< XModel >( this );

        xScriptProvider.set( xFactory->createScriptProvider( aScriptProviderContext ), UNO_SET_THROW );
        m_xScriptProvider = xScriptProvider;
    }

    return xScriptProvider;
}

// -----------------------------------------------------------------------------
Reference< XNameReplace > SAL_CALL ODatabaseDocument::getEvents(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return m_pEventContainer;
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->disposing(Source);
}

//------------------------------------------------------------------
Reference< XInterface > ODatabaseDocument::getThis() const
{
    return *const_cast< ODatabaseDocument* >( this );
}
// -----------------------------------------------------------------------------
struct CreateAny : public ::std::unary_function< Reference<XController>, Any>
{
    Any operator() (const Reference<XController>& lhs) const
    {
        return makeAny(lhs);
    }
};

// XModel2
Reference< XEnumeration > SAL_CALL ODatabaseDocument::getControllers(  ) throw (RuntimeException)
{
    DocumentGuard aGuard( *this );
    uno::Sequence< Any> aController( m_aControllers.size() );
    ::std::transform( m_aControllers.begin(), m_aControllers.end(), aController.getArray(), CreateAny() );
    return new ::comphelper::OAnyEnumeration(aController);
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseDocument::getAvailableViewControllerNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = SERVICE_SDB_APPLICATIONCONTROLLER;
    return aNames;
}
// -----------------------------------------------------------------------------
Reference< XController2 > SAL_CALL ODatabaseDocument::createDefaultViewController( const Reference< XFrame >& _Frame ) throw (IllegalArgumentException, Exception, RuntimeException)
{
    return createViewController(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Default" ) ),
        Sequence< PropertyValue >(),
        _Frame
    );
}

// -----------------------------------------------------------------------------
Reference< XController2 > SAL_CALL ODatabaseDocument::createViewController( const ::rtl::OUString& _ViewName, const Sequence< PropertyValue >& _Arguments, const Reference< XFrame >& _Frame ) throw (IllegalArgumentException, Exception, RuntimeException)
{
    if ( !_ViewName.equalsAscii( "Default" ) && !_ViewName.equalsAscii( "Preview" ) )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );
    if ( !_Frame.is() )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 3 );

    DocumentGuard aGuard( *this );
    ::comphelper::ComponentContext aContext( m_pImpl->m_aContext );
    aGuard.clear();

    Reference< XController2 > xController;
    aContext.createComponent( "org.openoffice.comp.dbu.OApplicationController", xController );

    ::comphelper::NamedValueCollection aInitArgs( _Arguments );
    aInitArgs.put( "Frame", _Frame );
    if ( _ViewName.equalsAscii( "Preview" ) )
        aInitArgs.put( "Preview", sal_Bool( sal_True ) );
    Reference< XInitialization > xInitController( xController, UNO_QUERY_THROW );
    xInitController->initialize( aInitArgs.getWrappedPropertyValues() );

    return xController;
}

// -----------------------------------------------------------------------------
//=============================================================================
Reference< XTitle > ODatabaseDocument::impl_getTitleHelper_throw()
{
    if ( ! m_xTitleHelper.is ())
    {
        Reference< XUntitledNumbers > xDesktop(
            m_pImpl->m_aContext.createComponent( "com.sun.star.frame.Desktop" ),
            UNO_QUERY_THROW );
        uno::Reference< frame::XModel > xThis   (getThis(), uno::UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper(m_pImpl->m_aContext.getLegacyServiceFactory());
        m_xTitleHelper.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);
        pHelper->setOwner                   (xThis   );
        pHelper->connectWithUntitledNumbers (xDesktop);
    }

    return m_xTitleHelper;
}

//=============================================================================
uno::Reference< frame::XUntitledNumbers > ODatabaseDocument::impl_getUntitledHelper_throw(const uno::Reference< uno::XInterface >& _xComponent)
{
    if ( !m_xModuleManager.is() )
        m_xModuleManager.set( m_pImpl->m_aContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );

    ::rtl::OUString sModuleId;
    try
    {
        sModuleId = m_xModuleManager->identify( _xComponent );
    }
    catch(uno::Exception)
    {
        // ni
    }
    uno::Reference< frame::XUntitledNumbers > xNumberedControllers;

    TNumberedController::iterator aFind = m_aNumberedControllers.find(sModuleId);
    if ( aFind == m_aNumberedControllers.end() )
    {
        uno::Reference< frame::XModel > xThis(static_cast< frame::XModel* >(this), uno::UNO_QUERY_THROW);
        ::comphelper::NumberedCollection* pHelper = new ::comphelper::NumberedCollection();
        xNumberedControllers.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);

        pHelper->setOwner          (xThis);
        //pHelper->setUntitledPrefix (::rtl::OUString::createFromAscii(" : "));

        m_aNumberedControllers.insert(TNumberedController::value_type(sModuleId,xNumberedControllers));
    }
    else
        xNumberedControllers = aFind->second;

    return xNumberedControllers;
}

//=============================================================================
// css.frame.XTitle
::rtl::OUString SAL_CALL ODatabaseDocument::getTitle()
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getTitleHelper_throw()->getTitle();
}

//=============================================================================
// css.frame.XTitle
void SAL_CALL ODatabaseDocument::setTitle( const ::rtl::OUString& sTitle )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this );
    impl_getTitleHelper_throw()->setTitle( sTitle );
    m_aEventNotifier.notifyDocumentEventAsync( "OnTitleChanged" );
    // <- SYNCHRONIZED
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::addTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this );

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster( impl_getTitleHelper_throw(), uno::UNO_QUERY_THROW );
    xBroadcaster->addTitleChangeListener( xListener );
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::removeTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this );

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster( impl_getTitleHelper_throw(), uno::UNO_QUERY_THROW );
    xBroadcaster->removeTitleChangeListener( xListener );
}

//=============================================================================
// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL ODatabaseDocument::leaseNumber( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    DocumentGuard aGuard( *this );
    return impl_getUntitledHelper_throw(xComponent)->leaseNumber (xComponent);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumber( ::sal_Int32 nNumber )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    DocumentGuard aGuard( *this );
    impl_getUntitledHelper_throw()->releaseNumber (nNumber);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumberForComponent( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    DocumentGuard aGuard( *this );
    impl_getUntitledHelper_throw(xComponent)->releaseNumberForComponent (xComponent);
}

//=============================================================================
// css.frame.XUntitledNumbers
::rtl::OUString SAL_CALL ODatabaseDocument::getUntitledPrefix()    throw (uno::RuntimeException)
{
    return ::rtl::OUString();/*RTL_CONSTASCII_USTRINGPARAM(" : "));*/
}

//------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................


