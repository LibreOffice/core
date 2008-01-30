/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databasedocument.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:33:34 $
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
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/enumhelper.hxx>
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

namespace css = ::com::sun::star;
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
using namespace ::cppu;
using namespace ::osl;
namespace css = ::com::sun::star;
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

// -----------------------------------------------------------------------------
// local functions
// -----------------------------------------------------------------------------
void lcl_stripLoadArguments( ::comphelper::MediaDescriptor& _rDescriptor, Sequence< PropertyValue >& _rArgs )
{
    _rDescriptor.erase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StatusIndicator" ) ) );
    _rDescriptor.erase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Model" ) ) );
    _rDescriptor >> _rArgs;
}
// -----------------------------------------------------------------------------
void lcl_extractAndStartStatusIndicator( const ::comphelper::MediaDescriptor& _rDescriptor, Reference< XStatusIndicator >& _rxStatusIndicator,
    Sequence< Any >& _rCallArgs )
{
    try
    {
        _rxStatusIndicator = _rDescriptor.getUnpackedValueOrDefault( _rDescriptor.PROP_STATUSINDICATOR(), _rxStatusIndicator );
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
        OSL_ENSURE( sal_False, "lcl_extractAndStartStatusIndicator: caught an exception!" );
    }
}

// -----------------------------------------------------------------------------
// XModel
// ATTENTION: The Application controller attaches the same resource to force a reload.
sal_Bool SAL_CALL ODatabaseDocument::attachResource( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _aArguments ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    try
    {
        m_pImpl->clearConnections();
        m_pImpl->disposeStorages();

        if ( m_pImpl->m_bOwnStorage )
            ::comphelper::disposeComponent(m_pImpl->m_xStorage);

        clearObjectContainer( m_xForms);
        clearObjectContainer( m_xReports);
        clearObjectContainer( m_pImpl->m_xTableDefinitions);
        clearObjectContainer( m_pImpl->m_xCommandDefinitions);

        m_pImpl->reset();
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
        m_pImpl->m_xStorage = NULL;
    }

    m_pImpl->m_bDocumentReadOnly = sal_False;

    ::comphelper::MediaDescriptor aDescriptor( _aArguments );
    lcl_stripLoadArguments( aDescriptor, m_pImpl->m_aArgs );

    m_pImpl->m_sFileURL = _rURL;
    m_pImpl->m_sRealFileURL = aDescriptor.getUnpackedValueOrDefault(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SalvagedFile" ) ), _rURL );
    if ( !m_pImpl->m_sRealFileURL.getLength() )
        m_pImpl->m_sRealFileURL = m_pImpl->m_sFileURL;

    if ( !m_pImpl->m_sName.getLength() )
        m_pImpl->m_sName = m_pImpl->m_sRealFileURL;

    m_pImpl->getStorage();

    try
    {
        Sequence<Any> aFilterArgs;
        Reference<XStatusIndicator> xStatusIndicator;
        lcl_extractAndStartStatusIndicator( aDescriptor, xStatusIndicator, aFilterArgs );

        Reference<XImporter> xImporter(
            m_pImpl->m_xServiceFactory->createInstanceWithArguments(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sdb.DBFilter" ) ),
                aFilterArgs
            ),
            UNO_QUERY
        );

        if ( xImporter.is() )
        {
            Reference<XComponent> xComponent(*this,UNO_QUERY);
            xImporter->setTargetDocument(xComponent);
            Reference<XFilter> xFilter(xImporter,UNO_QUERY);

            xFilter->filter(_aArguments);
            if ( xStatusIndicator.is() )
                xStatusIndicator->end();
        }
        else
            return sal_False;
    }
    catch(const RuntimeException& e)
    {
        throw e;
    }
    catch(const Exception&)
    {
        return sal_False;
    }
    if ( m_pImpl->m_pDBContext && m_pImpl->m_sRealFileURL.getLength() )
    {
        m_pImpl->m_pDBContext->registerPrivate(m_pImpl->m_sRealFileURL,m_pImpl);
        m_pImpl->setModified(sal_False);
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getURL(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL;
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getArgs(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_aArgs;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::connectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );

    m_aControllers.push_back( _xController );

    if ( m_aControllers.size() == 1 )
    {
        // check/adjust our macro mode. Note: This is only temporary. When we fully support the
        // XEmbeddedScripts interface, then the controller is able to do this itself, since
        // we'll then have a UNO method for this.
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
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL.getLength() != 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getLocation(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL;
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
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    if ( m_pImpl->m_sFileURL == m_pImpl->m_sRealFileURL )
        store( m_pImpl->m_sFileURL, m_pImpl->m_aArgs ,aGuard);
    else
        storeAsURL( m_pImpl->m_sRealFileURL, m_pImpl->m_aArgs );

    impl_notifyEvent( "OnSaveDone", aGuard );
}
// -----------------------------------------------------------------------------
void ODatabaseDocument::store(const ::rtl::OUString& _rURL
                             ,const Sequence< PropertyValue >& _rArguments
                             ,ModelMethodGuard& _rGuard)
{
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");
    if ( m_pImpl->m_bDocumentReadOnly )
        throw IOException();

    m_pImpl->commitStorages();

    Reference<XStorage> xMyStorage = m_pImpl->getStorage();
    OSL_ENSURE( xMyStorage.is(), "ODatabaseDocument::storeToURL: no own storage?" );
    if ( !xMyStorage.is() )
    {
        IOException aError;
        aError.Message = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Internal error: no source storage available." ) );
        aError.Context = *this;
        throw IOException( aError );
    }
    writeStorage(_rURL,_rArguments,xMyStorage);

    m_pImpl->commitRootStorage();

    setModified( sal_False,_rGuard );
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeAsURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory() );
    if ( !xStorageFactory.is() )
        throw RuntimeException();

    // don't use _rURL - we might be recovering/salvaging a file currently ...
    // #i45314# / 2005-03-21 / frank.schoenheit@sun.com
    ::comphelper::MediaDescriptor aDescriptor( _rArguments );

    sal_Bool bLocationChanged = ( _rURL != m_pImpl->m_sFileURL );
    if ( bLocationChanged )
    {
        Sequence<Any> aParam(2);
        aParam[0] <<= _rURL;
        aParam[1] <<= ElementModes::READWRITE | ElementModes::TRUNCATE;

        Reference<XStorage> xStorage;
        ::rtl::OUString sOriginalExceptionType;
        ::rtl::OUString sOriginalExceptionMessage;
        try
        {
            xStorage.set(xStorageFactory->createInstanceWithArguments( aParam ),UNO_QUERY);
        }
        catch ( const Exception& e )
        {
            Any aException( ::cppu::getCaughtException() );
            sOriginalExceptionType = aException.getValueTypeName();
            sOriginalExceptionMessage = e.Message;
        }

        if ( !xStorage.is() )
        {
            // TODO: resource
            ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii( "Could not store the database document to '" );
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
            throw IOException( sMessage, *this );
        }

        if ( m_pImpl->isEmbeddedDatabase() )
            m_pImpl->clearConnections();
        m_pImpl->commitEmbeddedStorage();

        Reference<XStorage> xMyStorage = m_pImpl->getStorage();
        if ( xMyStorage.is() )
        {
            m_pImpl->commitStorages();
            xMyStorage->copyToStorage( xStorage );
        }

        m_pImpl->disposeStorages();

        m_pImpl->m_xStorage = xStorage;
        if ( m_pImpl->m_bOwnStorage )
            ::comphelper::disposeComponent(xMyStorage);
        else
            m_pImpl->m_bOwnStorage = sal_True;

        m_pImpl->m_bDocumentReadOnly = sal_False;
        if ( _rURL != m_pImpl->m_sRealFileURL )
        {
            if ( m_pImpl->m_pDBContext )
            {
                if ( m_pImpl->m_sRealFileURL.getLength() )
                    m_pImpl->m_pDBContext->nameChangePrivate(m_pImpl->m_sRealFileURL,_rURL);
                else
                    m_pImpl->m_pDBContext->registerPrivate(_rURL,m_pImpl);
            }

            INetURLObject aURL( _rURL );
            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                m_pImpl->m_sName = _rURL;
        }
        m_pImpl->m_sRealFileURL = m_pImpl->m_sFileURL = _rURL;
    }
    lcl_stripLoadArguments( aDescriptor, m_pImpl->m_aArgs );
    store(m_pImpl->m_sFileURL,_rArguments,aGuard);

    impl_notifyEvent( "OnSaveAsDone", aGuard );
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeToURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory() );
    Sequence<Any> aParam(2);
    aParam[0] <<= _rURL;
    aParam[1] <<= ElementModes::READWRITE  | ElementModes::TRUNCATE;
    Reference<XStorage> xStorage;
    if ( xStorageFactory.is() )
        xStorage = xStorage.query( xStorageFactory->createInstanceWithArguments( aParam ) );
    OSL_ENSURE( xStorage.is(), "ODatabaseDocument::storeToURL: no storage factory!" );
    if ( !xStorage.is() )
    {
        IOException aError;
        aError.Message = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Could not create a target storage." ) );
        aError.Context = *this;
        throw IOException( aError );
    }

    Reference<XStorage> xMyStorage = m_pImpl->getStorage();
    OSL_ENSURE( xMyStorage.is(), "ODatabaseDocument::storeToURL: no own storage?" );
    if ( !xMyStorage.is() )
    {
        IOException aError;
        aError.Message = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Internal error: no source storage available." ) );
        aError.Context = *this;
        throw IOException( aError );
    }

    m_pImpl->commitEmbeddedStorage();
    xMyStorage->copyToStorage( xStorage );
    writeStorage(_rURL,_rArguments,xStorage);
    try
    {
        Reference<XTransactedObject> xTransact(xStorage,UNO_QUERY);
        if ( xTransact.is() )
            xTransact->commit();
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception Caught: Could not store database!");
        throw IOException();
    }
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
    setModified( _bModified,aGuard );
}
// -----------------------------------------------------------------------------
void ODatabaseDocument::setModified( sal_Bool _bModified,ModelMethodGuard& _rGuard )
{
    if ( m_pImpl->m_bModified == _bModified )
        return;

    m_pImpl->m_bModified = _bModified;
    lang::EventObject aEvt( *this );

    _rGuard.clear();
    m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvt );

    _rGuard.reset();
    impl_notifyEvent( "OnModifyChanged", _rGuard );
}
// -----------------------------------------------------------------------------

// ::com::sun::star::document::XEventBroadcaster
void SAL_CALL ODatabaseDocument::addEventListener(const css::uno::Reference< css::document::XEventListener >& _xListener ) throw (css::uno::RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aDocEventListeners.addInterface(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeEventListener( const css::uno::Reference< css::document::XEventListener >& _xListener ) throw (css::uno::RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    m_aDocEventListeners.removeInterface(_xListener);
}
// -----------------------------------------------------------------------------
// ::com::sun::star::document::XEventListener
void SAL_CALL ODatabaseDocument::notifyEvent( const css::document::EventObject& aEvent ) throw (css::uno::RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    // used only to forward external events (e.g. for doc creation) from the frame loader
    // to the global event broadcaster and all other interested doc event listener.
    impl_notifyEvent( aEvent.EventName, aGuard );
}
// -----------------------------------------------------------------------------
// ::com::sun::star::view::XPrintable
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
        rContainerRef = xContainer = new ODocumentContainer( m_pImpl->m_xServiceFactory, *this, rContainerData, bFormsContainer );
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

    lang::EventObject aEvent( *this );

    {
        aGuard.clear();
        m_aCloseListener.forEach< XCloseListener >(
            boost::bind( &XCloseListener::queryClosing, _1, boost::cref( aEvent ), boost::cref( _bDeliverOwnership ) ) );
        aGuard.reset();
    }

    impl_closeControllerFrames( _bDeliverOwnership );

    {
        aGuard.clear();
        m_aCloseListener.notifyEach( &XCloseListener::notifyClosing, aEvent );
        aGuard.reset();
    }

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
sal_Bool ODatabaseDocument::WriteThroughComponent(
    const Reference<XComponent> & xComponent,
    const sal_Char* pStreamName,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc,
    sal_Bool bPlainStream
    ,const Reference<XStorage>& _xStorageToSaveTo)
{
    OSL_ENSURE( NULL != pStreamName, "Need stream name!" );
    OSL_ENSURE( NULL != pServiceName, "Need service name!" );

    Reference<XStorage> xMyStorage = _xStorageToSaveTo;
    // open stream
    ::rtl::OUString sStreamName = ::rtl::OUString::createFromAscii( pStreamName );
    Reference<XStream> xStream = xMyStorage->openStreamElement( sStreamName,ElementModes::READWRITE | ElementModes::TRUNCATE );
    if ( !xStream.is() )
        return sal_False;
    Reference<XOutputStream> xOutputStream = xStream->getOutputStream();
    OSL_ENSURE(xOutputStream.is(), "Can't create output stream in package!");
    if ( ! xOutputStream.is() )
        return sal_False;

    Reference<XPropertySet> xStreamProp(xOutputStream,UNO_QUERY);
    OSL_ENSURE(xStreamProp.is(),"No valid preoperty set for the output stream!");

    Reference<XSeekable> xSeek(xStreamProp,UNO_QUERY);
    if ( xSeek.is() )
    {
        OSL_TRACE("Length of stream %i",(int)xSeek->getPosition());
        xSeek->seek(0);
    }

    ::rtl::OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
    Any aAny;
    aAny <<= aMime;
    xStreamProp->setPropertyValue( INFO_MEDIATYPE, aAny );

    if( bPlainStream )
    {
        sal_Bool bFalse = sal_False;
        aAny.setValue( &bFalse, ::getBooleanCppuType() );
        xStreamProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Compressed") ), aAny );
    }
    else
    {
        sal_Bool bTrue = sal_True;
        aAny.setValue( &bTrue, ::getBooleanCppuType() );
        xStreamProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), aAny );
    }


    // set buffer and create outputstream

    // write the stuff
    sal_Bool bRet = WriteThroughComponent(
        xOutputStream, xComponent,
        pServiceName, rArguments, rMediaDesc );

    // finally, commit stream.
    return bRet;
}

sal_Bool ODatabaseDocument::WriteThroughComponent(
    const Reference<XOutputStream> & xOutputStream,
    const Reference<XComponent> & xComponent,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc)
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( NULL != pServiceName, "Need component name!" );

    // get component
    Reference< XActiveDataSource > xSaxWriter(
        m_pImpl->m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))),
        UNO_QUERY );
    OSL_ENSURE( xSaxWriter.is(), "can't instantiate XML com.sun.star.xml.sax.Writer" );
    if(!xSaxWriter.is())
        return sal_False;

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference<XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);
    Sequence<Any> aArgs( 1 + rArguments.getLength() );
    aArgs[0] <<= xDocHandler;
    for(sal_Int32 i = 0; i < rArguments.getLength(); i++)
        aArgs[i+1] = rArguments[i];

    // get filter component
    Reference< XExporter > xExporter(
        m_pImpl->m_xServiceFactory->createInstanceWithArguments(
            ::rtl::OUString::createFromAscii(pServiceName), aArgs), UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if( !xExporter.is() )
        return sal_False;


    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    Reference<XFilter> xFilter( xExporter, UNO_QUERY );
    return xFilter->filter( rMediaDesc );
}
// -----------------------------------------------------------------------------
void ODatabaseDocument::writeStorage(const ::rtl::OUString& _rURL
                                    ,const Sequence< PropertyValue >& _rArguments
                                    ,const Reference<XStorage>& _xStorageToSaveTo)
{
    // create XStatusIndicator
    Reference<XStatusIndicator> xStatusIndicator;
    Sequence< Any > aDelegatorArguments;
    ::comphelper::MediaDescriptor aDescriptor( _rArguments );
    lcl_extractAndStartStatusIndicator( aDescriptor, xStatusIndicator, aDelegatorArguments );

    // properties
    Sequence < PropertyValue > aProps( _rURL.getLength() ? 1 : 0 );
    if( _rURL.getLength() )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FileName") );
        (pProps++)->Value <<= _rURL;
    }

    // export sub streams for package, else full stream into a file
    sal_Bool bWarn = sal_False, bErr = sal_False;
    String sWarnFile, sErrFile;

    Reference<XPropertySet> xProp(_xStorageToSaveTo,UNO_QUERY);
    if ( xProp.is() )
    {
        Any aAny;
        aAny <<= MIMETYPE_OASIS_OPENDOCUMENT_DATABASE;
        xProp->setPropertyValue( INFO_MEDIATYPE, aAny );
    }

    Reference<XComponent> xCom(static_cast<OWeakObject*>(this),UNO_QUERY);
    if( !bErr )
    {
        if( !WriteThroughComponent(
            xCom, "settings.xml",
            "com.sun.star.comp.sdb.XMLSettingsExporter",
            aDelegatorArguments, aProps, sal_True,_xStorageToSaveTo ) )
        {
            if( !bWarn )
            {
                bWarn = sal_True;
                sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("settings.xml"),
                                    RTL_TEXTENCODING_ASCII_US );
            }
        }
    }

    if ( !bErr )
    {
        if( !WriteThroughComponent(
                xCom, "content.xml",
                "com.sun.star.comp.sdb.DBExportFilter",
                aDelegatorArguments, aProps, sal_True,_xStorageToSaveTo ) )
        {
            bErr = sal_True;
            sErrFile = String( RTL_CONSTASCII_STRINGPARAM("content.xml"),
                               RTL_TEXTENCODING_ASCII_US );
        }
    }


    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::ui::XUIConfigurationManager > SAL_CALL ODatabaseDocument::getUIConfigurationManager(  ) throw (RuntimeException)
{
    ModelMethodGuard aGuard( *this );
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    if ( !m_xUIConfigurationManager.is() )
    {
        m_xUIConfigurationManager = Reference< ::com::sun::star::ui::XUIConfigurationManager >(
            m_pImpl->m_xServiceFactory->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.ui.UIConfigurationManager" )),
                UNO_QUERY );

        Reference< ::com::sun::star::ui::XUIConfigurationStorage > xUIConfigStorage( m_xUIConfigurationManager, UNO_QUERY );
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
void ODatabaseDocument::impl_notifyEvent( const ::rtl::OUString& _sEventName, ::osl::ClearableMutexGuard& _rGuard )
{
    try
    {
        css::document::EventObject aEvt(*this, _sEventName);
        _rGuard.clear();
        m_aDocEventListeners.notifyEach( &css::document::XEventListener::notifyEvent, aEvt );
    }
    catch(Exception&)
    {
    }
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
            ::osl::ClearableMutexGuard aGuard( getMutex() );
            impl_notifyEvent( "OnUnload", aGuard );
        }

        css::lang::EventObject aDisposeEvent(static_cast<XWeak*>(this));
        m_aModifyListeners.disposeAndClear( aDisposeEvent );
        m_aCloseListener.disposeAndClear( aDisposeEvent );
        m_aDocEventListeners.disposeAndClear( aDisposeEvent );

        m_xUIConfigurationManager = NULL;

        clearObjectContainer( m_xForms);
        clearObjectContainer( m_xReports);

        m_pImpl->modelIsDisposing( ODatabaseModelImpl::ResetModelAccess() );

        // now, at the latest, the controller array should be empty. Controllers are
        // expected to listen for our disposal, and disconnect then
        DBG_ASSERT( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
        impl_disposeControllerFrames_nothrow();
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
void SAL_CALL ODatabaseDocument::addEventListener( const Reference< css::lang::XEventListener >& _xListener ) throw (RuntimeException)
{
    ::cppu::WeakComponentImplHelperBase::addEventListener(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::removeEventListener( const Reference< css::lang::XEventListener >& _xListener ) throw (RuntimeException)
{
    ::cppu::WeakComponentImplHelperBase::removeEventListener(_xListener);
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
void SAL_CALL ODatabaseDocument::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->disposing(Source);
}

//------------------------------------------------------------------
Reference< XInterface > ODatabaseDocument::getThis()
{
    return *this;
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

//------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................


