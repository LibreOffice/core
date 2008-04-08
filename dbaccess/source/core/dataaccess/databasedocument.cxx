/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databasedocument.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-08 12:43:21 $
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

#include "datasource.hxx"
#include "databasedocument.hxx"
#include "dbastrings.hrc"
#include "module_dba.hxx"

#include <comphelper/documentconstants.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/numberedcollection.hxx>
#include <framework/titlehelper.hxx>
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_ERRORCODEIOEXCEPTION_HPP_
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _DBA_COREDATAACCESS_DOCUMENTCONTAINER_HXX_
#include "documentcontainer.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_
#include "databasecontext.hxx"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONSTORAGE_HPP_
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONBROADCASTER_HPP_
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_XSCRIPTPROVIDERFACTORY_HPP_
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

#include <algorithm>
#include <functional>

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

//........................................................................
namespace dbaccess
{
//........................................................................

//============================================================
//= ODatabaseContext
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
            ,m_aDocEventListeners( getMutex() )
            ,m_aStorageListeners( getMutex() )
{
    DBG_CTOR(ODatabaseDocument,NULL);

    osl_incrementInterlockedCount( &m_refCount );
    {
        impl_reparent_nothrow( m_xForms );
        impl_reparent_nothrow( m_xReports );
        impl_reparent_nothrow( m_pImpl->m_xTableDefinitions );
        impl_reparent_nothrow( m_pImpl->m_xCommandDefinitions );
    }
    osl_decrementInterlockedCount( &m_refCount );
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
}
//------------------------------------------------------------------------------
Any SAL_CALL ODatabaseDocument::queryInterface(const Type& _rType) throw (RuntimeException)
{
    // strip XEmbeddedScripts and XScriptInvocationContext if we have any form/report
    // which already contains macros. In this case, the database document itself is not
    // allowed to contain macros, too.
    if  (   impl_shouldDisallowScripting_nolck_nothrow()
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
    if ( impl_shouldDisallowScripting_nolck_nothrow() )
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
bool ODatabaseDocument::impl_shouldDisallowScripting_nolck_nothrow() const
{
    ::osl::MutexGuard aGuard( getMutex() );
    // TODO: revert to the disabled code. The current version is just to be able
    // to integrate an intermediate version of the CWS, which should behave as
    // if no macros in DB docs are allowed
//    if ( m_pImpl.is() && m_pImpl->hasAnyObjectWithMacros() )
        return true;
//    return false;
}

// -----------------------------------------------------------------------------
// local functions
// -----------------------------------------------------------------------------
namespace
{
    static void lcl_stripLoadArguments( ::comphelper::NamedValueCollection& _rArguments, Sequence< PropertyValue >& _rArgs )
    {
        _rArguments.remove( "Model" );
        _rArguments >>= _rArgs;
    }

    // -----------------------------------------------------------------------------
    static void lcl_extractAndStartStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments, Reference< XStatusIndicator >& _rxStatusIndicator,
        Sequence< Any >& _rCallArgs )
    {
        try
        {
            _rxStatusIndicator = _rArguments.getOrDefault( "StatusIndicator", _rxStatusIndicator );
            if ( _rxStatusIndicator.is() )
            {
                _rxStatusIndicator->start( ::rtl::OUString(), (sal_Int32)1000000 );

                sal_Int32 nLength = _rCallArgs.getLength();
                _rCallArgs.realloc( nLength + 1 );
                _rCallArgs[ nLength ] <<= _rxStatusIndicator;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    static Sequence< PropertyValue > lcl_appendFileNameToDescriptor( const Sequence< PropertyValue >& _rDescriptor, const ::rtl::OUString _rURL )
    {
        Sequence< PropertyValue > aMediaDescriptor( _rDescriptor );
        if ( _rURL.getLength() )
        {
            aMediaDescriptor.realloc( _rDescriptor.getLength() + 1 );
            aMediaDescriptor[ _rDescriptor.getLength() ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
            aMediaDescriptor[ _rDescriptor.getLength() ].Value <<= _rURL;
        }
        return aMediaDescriptor;
    }
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

        m_pImpl->reset();
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_pImpl->m_bDocumentReadOnly = sal_False;
}

// -----------------------------------------------------------------------------
bool ODatabaseDocument::impl_import_throw( const ::comphelper::NamedValueCollection& _rResource )
{
    try
    {
        Sequence< Any > aFilterArgs;
        Reference< XStatusIndicator > xStatusIndicator;
        lcl_extractAndStartStatusIndicator( _rResource, xStatusIndicator, aFilterArgs );

        Reference< XImporter > xImporter(
            m_pImpl->m_aContext.createComponentWithArguments( "com.sun.star.comp.sdb.DBFilter", aFilterArgs ),
            UNO_QUERY_THROW );

        Reference< XComponent > xComponent( *this, UNO_QUERY_THROW );
        xImporter->setTargetDocument( xComponent );

        Reference< XFilter > xFilter( xImporter, UNO_QUERY_THROW );
        xFilter->filter( m_pImpl->m_aArgs );

        if ( xStatusIndicator.is() )
            xStatusIndicator->end();
    }
    catch( const RuntimeException& e )
    {
        throw e;
    }
    catch( const Exception& )
    {
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// XModel
// ATTENTION: The Application controller attaches the same resource to force a reload.
// TODO: this is a bug. By (API) definition, attachResource is only for notifying the document
// of its resource, *not* for loading it. We should implement an XLoadable, and move all the
// load logic therein.
sal_Bool SAL_CALL ODatabaseDocument::attachResource( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _aArguments ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    impl_reset_nothrow();

    ::comphelper::NamedValueCollection aResource( _aArguments );
    lcl_stripLoadArguments( aResource, m_pImpl->m_aArgs );

    ::rtl::OUString sDocumentURL( aResource.getOrDefault( "SalvagedFile", _rURL ) );
    if ( !sDocumentURL.getLength() )
        // this indicates "the document is being recovered, but _rURL already is the real document URL,
        // not the temporary document location"
        sDocumentURL = _rURL;
    m_pImpl->switchToURL( _rURL, sDocumentURL );

    bool bSuccess =
        (   m_pImpl->getOrCreateRootStorage().is()
        &&  impl_import_throw( aResource )  // TODO: this doesn't belong here, but into an (externally called) XLoadable::load implementation
        );

    if ( !bSuccess )
    {
        m_pImpl->revokeDataSource();
        return sal_False;
    }

    impl_setModified_throw( sal_False, aGuard );
    return sal_True;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getURL(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getURL();
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getArgs(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->m_aArgs;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::connectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    m_aControllers.push_back( _xController );

    if ( m_aControllers.size() != 1 )
        return;

    // it's the first controller

    // check/adjust our macro mode. Note: This is only temporary. When we fully support the
    // XEmbeddedScripts interface, plus related functionality, then the controller is able
    // to do this itself, since we'll then have a UNO method for this.
    //
    // Also, the same has to happen in the loader then, since the checks must be made
    // *before* OnLoad events are triggered - finally, the user can bind events to OnLoad ...
    // (This, at the latest, implies we need a UNO equivalent for checkMacrosOnLoading, else
    //  the loader can't call it.)
    //
    // For now, as long as we do not have own macros, but only those in the embedded
    // forms/reports, it's sufficient to do the check here.
    //
    m_pImpl->checkMacrosOnLoading();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::disconnectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Controllers::iterator pos = ::std::find( m_aControllers.begin(), m_aControllers.end(), _xController );
    OSL_ENSURE( pos != m_aControllers.end(), "ODatabaseDocument::disconnectController: don't know this controller!" );
    if ( pos != m_aControllers.end() )
        m_aControllers.erase( pos );

    if ( m_xCurrentController == _xController )
        m_xCurrentController = NULL;

    if ( m_aControllers.empty() )
    {
        // reset the macro mode: in case the our impl struct stays alive (e.g. because our DataSource
        // object still exists), and somebody subsequently re-opens the document, we want to have
        // the security warning, again.
        m_pImpl->resetMacroExecutionMode();

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
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    ++m_pImpl->m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::unlockControllers(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    --m_pImpl->m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::hasControllersLocked(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_nControllerLockCount != 0;
}
// -----------------------------------------------------------------------------
Reference< XController > SAL_CALL ODatabaseDocument::getCurrentController() throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_xCurrentController.is() ? m_xCurrentController : ( m_aControllers.empty() ? Reference< XController >() : *m_aControllers.begin() );
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setCurrentController( const Reference< XController >& _xController ) throw (NoSuchElementException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    m_xCurrentController = _xController;
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseDocument::getCurrentSelection(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

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
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getLocation().getLength() > 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getLocation(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getLocation();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::isReadonly(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_bDocumentReadOnly;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::store(  ) throw (IOException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    //ModifyLock aLock( *this );

    if ( m_pImpl->getLocation() == m_pImpl->getURL() )
        if ( m_pImpl->m_bDocumentReadOnly )
            throw IOException();

    impl_storeAs_throw( m_pImpl->getURL(), m_pImpl->m_aArgs, "OnSaveDone", aGuard );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_storeAs_throw( const ::rtl::OUString& _rURL, const Sequence< PropertyValue>& _rArguments,
        const sal_Char* _pAsciiDocumentEventName, ModelMethodGuard& _rGuard )
{
    Reference< XStorage > xNewRootStorage;
        // will be non-NULL if our storage changed

    sal_Bool bLocationChanged = ( _rURL != m_pImpl->getLocation() );
    if ( bLocationChanged )
    {
        // create storage for target URL
        Reference< XStorage > xTargetStorage;
        if ( !impl_createStorageFor_throw( _rURL, xTargetStorage ) )
            // failed, but handled by an interaction handler
            return;

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

    // adjust arguments
    ::comphelper::NamedValueCollection aResource( _rArguments );
    lcl_stripLoadArguments( aResource, m_pImpl->m_aArgs );
    Sequence< PropertyValue > aMediaDescriptor( lcl_appendFileNameToDescriptor( m_pImpl->m_aArgs, _rURL ) );

    // store to current storage
    Reference< XStorage > xCurrentStorage( m_pImpl->getOrCreateRootStorage(), UNO_QUERY_THROW );
    impl_storeToStorage_throw( xCurrentStorage, aMediaDescriptor );

    // success - tell our impl the new URL
    m_pImpl->switchToURL( _rURL, _rURL );

    // create a document event (mutex still locked)
    document::EventObject aEvent( *this, ::rtl::OUString::createFromAscii( _pAsciiDocumentEventName ) );

    // reset our "modified" flag, and clear the guard
    impl_setModified_throw( sal_False, _rGuard );

    // notify the document event
    impl_notifyEvent_nolck_nothrow( aEvent );

    // notify storage listeners
    impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

// -----------------------------------------------------------------------------
bool ODatabaseDocument::impl_createStorageFor_throw( const ::rtl::OUString& _rURL, Reference< XStorage >& _out_rxStorage ) const
{
    _out_rxStorage.clear();

    Sequence<Any> aParam(2);
    aParam[0] <<= _rURL;
    aParam[1] <<= ElementModes::READWRITE | ElementModes::TRUNCATE;

    Any aOriginalError;
    try
    {
        Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory(), UNO_SET_THROW );
        _out_rxStorage.set( xStorageFactory->createInstanceWithArguments( aParam ), UNO_QUERY_THROW );
    }
    catch ( const Exception& )
    {
        aOriginalError = ::cppu::getCaughtException();
    }

    if ( _out_rxStorage.is() )
        return true;

    // try handling the error with the interaction handler
    ::comphelper::NamedValueCollection aArgs( m_pImpl->m_aArgs );
    Reference< XInteractionHandler > xHandler( aArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );
    if ( xHandler.is() )
    {
        ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( aOriginalError ) );
        ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
        pRequest->addContinuation( pApprove.get() );

        try
        {
            xHandler->handle( pRequest.get() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( pApprove->wasSelected() )
            return false;
    }

    Exception aException;
    OSL_VERIFY( aOriginalError >>= aException );
    ::rtl::OUString sOriginalExceptionType = aOriginalError.getValueTypeName();
    ::rtl::OUString sOriginalExceptionMessage = aException.Message;

    // TODO: resource
    ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii( "Could not create a storage for '" );
    sMessage += _rURL;
    sMessage += ::rtl::OUString::createFromAscii( "'." );
    if ( sOriginalExceptionMessage.getLength() )
    {
        sMessage += ::rtl::OUString::createFromAscii( "\noriginal error message: " );
        sMessage += sOriginalExceptionMessage;
    }
    if ( sOriginalExceptionType.getLength() )
    {
        sMessage += ::rtl::OUString::createFromAscii( "\noriginal error type: " );
        sMessage += sOriginalExceptionType;
    }
    throw IOException( sMessage, *const_cast< ODatabaseDocument* >( this ) );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeAsURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    //ModifyLock aLock( *this );

    impl_storeAs_throw( _rURL, _rArguments, "OnSaveAsDone", aGuard );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_storeToStorage_throw( const Reference< XStorage >& _rxTargetStorage, const Sequence< PropertyValue >& _rMediaDescriptor ) const
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
        Reference< XStorage > xCurrentStorage( m_pImpl->getOrCreateRootStorage(), UNO_QUERY_THROW );
        if ( xCurrentStorage != _rxTargetStorage )
            xCurrentStorage->copyToStorage( _rxTargetStorage );

        // write into target storage
        ::comphelper::NamedValueCollection aWriteArgs( _rMediaDescriptor );
        writeStorage( _rxTargetStorage, aWriteArgs );

        // commit target storage
        OSL_VERIFY( m_pImpl->commitStorageIfWriteable_ignoreErrors( _rxTargetStorage ) );
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
    ModelMethodGuard aGuard( *this );
    ModifyLock aLock( *this );

    // create storage for target URL
    Reference< XStorage > xTargetStorage;
    if ( !impl_createStorageFor_throw( _rURL, xTargetStorage ) )
        // failed, but handled by an interaction handler
        return;

    // extend media descriptor with URL
    Sequence< PropertyValue > aMediaDescriptor( lcl_appendFileNameToDescriptor( _rArguments, _rURL ) );

    // store to this storage
    impl_storeToStorage_throw( xTargetStorage, aMediaDescriptor );
}

// -----------------------------------------------------------------------------
// XModifyBroadcaster
void SAL_CALL ODatabaseDocument::addModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aModifyListeners.addInterface(_xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aModifyListeners.removeInterface(_xListener);
}

// -----------------------------------------------------------------------------
// XModifiable
sal_Bool SAL_CALL ODatabaseDocument::isModified(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_bModified;
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setModified( sal_Bool _bModified ) throw (PropertyVetoException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    impl_setModified_throw( _bModified, aGuard );
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_setModified_throw( sal_Bool _bModified, ModelMethodGuard& _rGuard )
{
    if ( m_pImpl->m_bModified == _bModified )
        return;

    if ( m_pImpl->isModifyLocked() )
        return;

    m_pImpl->m_bModified = _bModified;

    document::EventObject aEvent( *this, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnModifyChanged" ) ) );
    _rGuard.clear();

    m_aModifyListeners.notifyEach( &XModifyListener::modified, (const lang::EventObject&)aEvent );
    impl_notifyEvent_nolck_nothrow( aEvent );
}

// -----------------------------------------------------------------------------
// ::com::sun::star::document::XEventBroadcaster
void SAL_CALL ODatabaseDocument::addEventListener(const uno::Reference< document::XEventListener >& _xListener ) throw (uno::RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aDocEventListeners.addInterface(_xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeEventListener( const uno::Reference< document::XEventListener >& _xListener ) throw (uno::RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aDocEventListeners.removeInterface(_xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::notifyEvent( const document::EventObject& _rEvent ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    // used only to forward external events (e.g. for doc creation) from the frame loader
    // to the global event broadcaster and all other interested doc event listener.
    document::EventObject aEvent( *this, _rEvent.EventName );
    aGuard.clear();
    impl_notifyEvent_nolck_nothrow( aEvent );
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
    ModelMethodGuard aGuard( *this );
    OSL_POSTCOND( m_pImpl.is(), "ODatabaseDocument::impl_getDocumentContainer_throw: Impl is NULL" );

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
void ODatabaseDocument::impl_closeControllerFrames( sal_Bool _bDeliverOwnership )
{
    Controllers aCopy = m_aControllers;

    for ( Controllers::iterator aIter = aCopy.begin(); aIter != aCopy.end() ; ++aIter )
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
void SAL_CALL ODatabaseDocument::close( sal_Bool _bDeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    document::EventObject aEvent( *this, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnUnload" ) ) );

    {
        aGuard.clear();
        m_aCloseListener.forEach< XCloseListener >(
            boost::bind( &XCloseListener::queryClosing, _1, boost::cref( aEvent ), boost::cref( _bDeliverOwnership ) ) );
        aGuard.reset();
    }

    impl_closeControllerFrames( _bDeliverOwnership );

    aGuard.clear();

    m_aCloseListener.notifyEach( &XCloseListener::notifyClosing, (const lang::EventObject&)aEvent );

    // notify the OnUnload at the earliest possibility - which is here and now
    impl_notifyEvent_nolck_nothrow( aEvent );

    dispose();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aCloseListener.addInterface(Listener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aCloseListener.removeInterface(Listener);
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseDocument::getFormDocuments(  ) throw (RuntimeException)
{
    return impl_getDocumentContainer_throw( ODatabaseModelImpl::E_FORM );
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseDocument::getReportDocuments(  ) throw (RuntimeException)
{
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
void ODatabaseDocument::writeStorage( const Reference< XStorage >& _rxTargetStorage, const ::comphelper::NamedValueCollection& _rMediaDescriptor ) const
{
    // extract status indicator
    Sequence< Any > aDelegatorArguments;
    Reference< XStatusIndicator > xStatusIndicator;
    lcl_extractAndStartStatusIndicator( _rMediaDescriptor, xStatusIndicator, aDelegatorArguments );

    Reference< XPropertySet > xProp( _rxTargetStorage, UNO_QUERY_THROW );
    xProp->setPropertyValue( INFO_MEDIATYPE, makeAny( (rtl::OUString)MIMETYPE_OASIS_OPENDOCUMENT_DATABASE ) );

    Reference< XComponent > xComponent( *const_cast< ODatabaseDocument* >( this ), UNO_QUERY_THROW );

    Sequence< PropertyValue > aMediaDescriptor;
    _rMediaDescriptor >>= aMediaDescriptor;

    WriteThroughComponent( xComponent, "settings.xml", "com.sun.star.comp.sdb.XMLSettingsExporter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    WriteThroughComponent( xComponent, "content.xml", "com.sun.star.comp.sdb.DBExportFilter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    m_pImpl->storeLibraryContainersTo( _rxTargetStorage );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

// -----------------------------------------------------------------------------
Reference< XUIConfigurationManager > SAL_CALL ODatabaseDocument::getUIConfigurationManager(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

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
    ModelMethodGuard aGuard( *this );

    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStorage( aStorageName, nMode );
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseDocument::getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, RuntimeException)
{
    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStoragesNames();
}

// -----------------------------------------------------------------------------
void ODatabaseDocument::impl_notifyEvent_nolck_nothrow( const document::EventObject& _rEvent )
{
    try
    {
        m_aDocEventListeners.notifyEach( &XEventListener::notifyEvent, _rEvent );
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
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

    DBG_ASSERT( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
        // normally, nobody should explicitly dispose, but only XCloseable::close the document. And upon
        // closing, our controllers are closed, too

    Reference< XModel > xHoldAlive( this );
    {
        {
            document::EventObject aEvent( *this, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnLoad" ) ) );
            impl_notifyEvent_nolck_nothrow( aEvent );
        }

        lang::EventObject aDisposeEvent(static_cast<XWeak*>(this));
        m_aModifyListeners.disposeAndClear( aDisposeEvent );
        m_aCloseListener.disposeAndClear( aDisposeEvent );
        m_aDocEventListeners.disposeAndClear( aDisposeEvent );
        m_aStorageListeners.disposeAndClear( aDisposeEvent );

        m_xUIConfigurationManager = NULL;

        clearObjectContainer( m_xForms );
        clearObjectContainer( m_xReports );

        m_pImpl->modelIsDisposing( ODatabaseModelImpl::ResetModelAccess() );

        // now, at the latest, the controller array should be empty. Controllers are
        // expected to listen for our disposal, and disconnect then
        DBG_ASSERT( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
        impl_disposeControllerFrames_nothrow();
        m_xModuleManager.clear();
        m_xTitleHelper.clear();
    }

    m_pImpl.clear();
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

    ::rtl::Reference<ODatabaseModelImpl> pImpl( new ODatabaseModelImpl( aContext.getLegacyServiceFactory() ) );
    pImpl->m_pDBContext = pContext;
    Reference< XModel > xModel( pImpl->createNewModel_deliverOwnership() );
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
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");
    return m_pImpl->getDataSource();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::loadFromStorage( const Reference< XStorage >& /*xStorage*/, const Sequence< PropertyValue >& /*aMediaDescriptor*/ ) throw (IllegalArgumentException, DoubleInitializationException, IOException, Exception, RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    throw Exception(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Embedding of database documents is not supported." ) ),
            // TODO: resource
        *this
    );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeToStorage( const Reference< XStorage >& _rxStorage, const Sequence< PropertyValue >& _rMediaDescriptor ) throw (IllegalArgumentException, IOException, Exception, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    //ModifyLock aLock( *this );

    impl_storeToStorage_throw( _rxStorage, _rMediaDescriptor );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::switchToStorage( const Reference< XStorage >& _rxNewRootStorage ) throw (IllegalArgumentException, IOException, Exception, RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    Reference< XStorage > xNewRootStorage( m_pImpl->switchToStorage( _rxNewRootStorage ) );

    aGuard.clear();
    impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

// -----------------------------------------------------------------------------
Reference< XStorage > SAL_CALL ODatabaseDocument::getDocumentStorage(  ) throw (IOException, Exception, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getRootStorage();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::addStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    m_aStorageListeners.addInterface( _Listener );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    m_aStorageListeners.addInterface( _Listener );
}

// -----------------------------------------------------------------------------
Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getBasicLibraries() throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getLibraryContainer( true );
}

// -----------------------------------------------------------------------------
Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getDialogLibraries() throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->getLibraryContainer( false );
}

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL ODatabaseDocument::getAllowMacroExecution() throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->adjustMacroMode_AutoReject();
}

// -----------------------------------------------------------------------------
Reference< XEmbeddedScripts > SAL_CALL ODatabaseDocument::getScriptContainer() throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    return this;
}

// -----------------------------------------------------------------------------
Reference< provider::XScriptProvider > SAL_CALL ODatabaseDocument::getScriptProvider(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    Reference< XScriptProvider > xScriptProvider( m_xScriptProvider );
    if ( !xScriptProvider.is() )
    {
        Reference < XScriptProviderFactory > xFactory(
            m_pImpl->m_aContext.getSingleton( "com.sun.star.script.provider.theMasterScriptProviderFactory" ), UNO_QUERY_THROW );

        Any aScriptProviderContext;
        if ( !impl_shouldDisallowScripting_nolck_nothrow() )
            aScriptProviderContext <<= Reference< XModel >( this );

        xScriptProvider.set( xFactory->createScriptProvider( aScriptProviderContext ), UNO_SET_THROW );
        m_xScriptProvider = xScriptProvider;
    }

    return xScriptProvider;
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
    ModelMethodGuard aGuard( *this );
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
Reference< XController > SAL_CALL ODatabaseDocument::createDefaultViewController( const Reference< XFrame >& /*Frame*/, Reference< ::com::sun::star::awt::XWindow >& /*ComponentWindow*/ ) throw (IllegalArgumentException, Exception, RuntimeException)
{
    return Reference< XController >();
}
// -----------------------------------------------------------------------------
Reference< XController > SAL_CALL ODatabaseDocument::createViewController( const ::rtl::OUString& /*ViewName*/, const Sequence< PropertyValue >& /*Arguments*/, const Reference< XFrame >& /*Frame*/, Reference< ::com::sun::star::awt::XWindow >& /*ComponentWindow*/ ) throw (IllegalArgumentException, Exception, RuntimeException)
{
    return Reference< XController >();
}
// -----------------------------------------------------------------------------
//=============================================================================
Reference< XTitle > ODatabaseDocument::impl_getTitleHelper_throw()
{
    ModelMethodGuard aGuard( *this );

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
    ModelMethodGuard aGuard( *this );

    if ( !m_xModuleManager.is() )
        m_xModuleManager.set( m_pImpl->m_aContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );

    ::rtl::OUString sModuleId = m_xModuleManager->identify( _xComponent );

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
    ModelMethodGuard aGuard( *this );

    return impl_getTitleHelper_throw()->getTitle ();
}

//=============================================================================
// css.frame.XTitle
void SAL_CALL ODatabaseDocument::setTitle( const ::rtl::OUString& sTitle )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ModelMethodGuard aGuard( *this );

    impl_getTitleHelper_throw()->setTitle (sTitle);
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::addTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ModelMethodGuard aGuard( *this );

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::removeTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ModelMethodGuard aGuard( *this );

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

//=============================================================================
// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL ODatabaseDocument::leaseNumber( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    // object already disposed?
    ModelMethodGuard aGuard( *this );

    return impl_getUntitledHelper_throw(xComponent)->leaseNumber (xComponent);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumber( ::sal_Int32 nNumber )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    // object already disposed?
    ModelMethodGuard aGuard( *this );

    impl_getUntitledHelper_throw()->releaseNumber (nNumber);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumberForComponent( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException         )
{
    // object already disposed?
    ModelMethodGuard aGuard( *this );
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


