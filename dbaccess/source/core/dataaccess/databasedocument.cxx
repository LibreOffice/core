/*************************************************************************
 *
 *  $RCSfile: databasedocument.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:36:11 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_
#include "databasedocument.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
#include <comphelper/documentconstants.hxx>
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
    static OMultiInstanceAutoRegistration< ODatabaseDocument > aAutoRegistration;
}

//--------------------------------------------------------------------------
Reference< XInterface > ODatabaseDocument_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    ODatabaseContext* pContext = NULL;
    try
    {
        Reference<XUnoTunnel> xUnoTunnel(_rxFactory->createInstance(SERVICE_SDB_DATABASECONTEXT),UNO_QUERY);
        if ( xUnoTunnel.is() )
            pContext = reinterpret_cast<ODatabaseContext*>(xUnoTunnel->getSomething(ODatabaseContext::getUnoTunnelImplementationId()));
    }
    catch(Exception)
    {
    }

    ::rtl::Reference<ODatabaseModelImpl> pImpl(new ODatabaseModelImpl(_rxFactory));
    pImpl->m_pDBContext = pContext;
    Reference< XModel > xModel( pImpl->createNewModel_deliverOwnership() );
    return xModel.get();
}
//--------------------------------------------------------------------------
ODatabaseDocument::ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl )
            :ODatabaseDocument_OfficeDocument(m_aMutex)
            ,m_pImpl(_pImpl)
            ,m_aModifyListeners(m_aMutex)
            ,m_aCloseListener(m_aMutex)
            ,m_aDocEventListeners(m_aMutex)
{
    DBG_CTOR(ODatabaseDocument,NULL);

    // adjust our readonly flag
    try
    {
        m_xDocEventBroadcaster.set(m_pImpl->m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.GlobalEventBroadcaster"))),
            UNO_QUERY);
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Could not create GlobalEventBroadcaster!");
    }
    Reference<XChild> xChild(m_pImpl->m_xForms.get(),UNO_QUERY);
    if ( xChild.is() )
        xChild->setParent(static_cast<OWeakObject*>(this));

    xChild.set(m_pImpl->m_xReports.get(),UNO_QUERY);
    if ( xChild.is() )
        xChild->setParent(static_cast<OWeakObject*>(this));
    xChild.set(m_pImpl->m_xTableDefinitions.get(),UNO_QUERY);
    if ( xChild.is() )
        xChild->setParent(static_cast<OWeakObject*>(this));
    xChild.set(m_pImpl->m_xCommandDefinitions.get(),UNO_QUERY);
    if ( xChild.is() )
        xChild->setParent(static_cast<OWeakObject*>(this));
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
    _rDescriptor.erase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ) ) );
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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    try
    {
        m_pImpl->clearConnections();
        m_pImpl->disposeStorages();

        if ( m_pImpl->m_bOwnStorage )
            ::comphelper::disposeComponent(m_pImpl->m_xStorage);
        Reference< XNameAccess > xContainer = m_pImpl->m_xForms;
        ::comphelper::disposeComponent(xContainer);
        xContainer = m_pImpl->m_xReports;
        ::comphelper::disposeComponent(xContainer);
        xContainer = m_pImpl->m_xTableDefinitions;
        ::comphelper::disposeComponent(xContainer);

        xContainer = m_pImpl->m_xCommandDefinitions;
        ::comphelper::disposeComponent(xContainer);

        m_pImpl->m_aContainer.clear();
        m_pImpl->lateInit();
    }
    catch(const Exception&)
    {
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
    if ( m_pImpl->m_pDBContext )
    {
        m_pImpl->m_pDBContext->registerPrivate(m_pImpl->m_sRealFileURL,m_pImpl);
        m_pImpl->setModified(sal_False);
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getURL(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL;
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getArgs(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_aArgs;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::connectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    m_pImpl->m_aControllers.push_back(_xController);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::disconnectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    m_pImpl->m_aControllers.erase(::std::find(m_pImpl->m_aControllers.begin(),m_pImpl->m_aControllers.end(),_xController));
    if ( m_pImpl->m_xCurrentController == _xController )
        m_pImpl->m_xCurrentController = NULL;

    // TODO: The below fragment is conceptually wrong.
    //
    // There are more clients of a database document (aka XModel) than its controllers.
    // In particular, people might programmatically obtain a DataSource from the
    // DatabaseContext, script it, and at some point obtain the document from
    // the data source (XDocumentDataSource::getDatabaseDocument). All this might happen
    // without any controller being involved, which means the document gets never disposed,
    // which imlpies a resource leak.
    //
    // You might argue that the scripter who obtained the model is responsible for disposing
    // it. However, she cannot know whether the model she just got from getDatabaseDocument
    // is really hers (since it was newly created), or already owned by somebody else. So,
    // she cannot know whether she is really allowed to dispose it.
    //
    // There is a pattern which could prevent this dilemma: closing with ownership delivery
    // (XCloseable::close). With this pattern, every client of a component (here: the model)
    // adds itself as XCloseListener to the component. When the client dies, it tries to
    // close the component, with the DeliverOwnership parameter set to <TRUE/>. If there is
    // another client of the component, it will veto the closing, and take the ownership
    // (and in turn do an own close attempt later on). If there is no other client, closing
    // will succeed.
    //
    // We should implement this for models, too. Then, controllers would be clients of the
    // model, and do a close attempt when they disconnect. The model would never dispose
    // itself (as it does now), but it would automatically be closed when the last client
    // dies (provided that all clients respect this pattern). It turn, it would not be
    // allowed to dispose a model directly.
    //
    // #i50905# / 2005-06-21 / frank.schoenheit@sun.com
    if ( m_pImpl.is() && m_pImpl->m_aControllers.empty() )
    {
        aGuard.clear();
        dispose();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::lockControllers(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    ++m_pImpl->m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::unlockControllers(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    --m_pImpl->m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::hasControllersLocked(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_nControllerLockCount != 0;
}
// -----------------------------------------------------------------------------
Reference< XController > SAL_CALL ODatabaseDocument::getCurrentController() throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_xCurrentController.is() ? m_pImpl->m_xCurrentController : ( m_pImpl->m_aControllers.empty() ? Reference< XController >() : *m_pImpl->m_aControllers.begin() );
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setCurrentController( const Reference< XController >& _xController ) throw (NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    m_pImpl->m_xCurrentController = _xController;
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseDocument::getCurrentSelection(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL.getLength() != 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseDocument::getLocation(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_sRealFileURL;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseDocument::isReadonly(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_bDocumentReadOnly;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::store(  ) throw (IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    if ( m_pImpl->m_sFileURL == m_pImpl->m_sRealFileURL )
        store( m_pImpl->m_sFileURL, m_pImpl->m_aArgs );
    else
        storeAsURL( m_pImpl->m_sRealFileURL, m_pImpl->m_aArgs );

    notifyEvent(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSaveDone")));
}
// -----------------------------------------------------------------------------
void ODatabaseDocument::store(const ::rtl::OUString& _rURL
                             ,const Sequence< PropertyValue >& _rArguments)
{
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");
    if ( m_pImpl->m_bDocumentReadOnly )
        throw IOException();

    m_bCommitMasterStorage = sal_False;
    m_pImpl->commitStorages();
    m_bCommitMasterStorage = sal_True;

    writeStorage(_rURL,_rArguments,m_pImpl->getStorage());

    m_pImpl->commitRootStorage();

    setModified(sal_False);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeAsURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory() );
    if ( xStorageFactory.is() )
    {
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
            try
            {
                xStorage.set(xStorageFactory->createInstanceWithArguments( aParam ),UNO_QUERY);
            }
            catch(Exception&)
            {
            }
            if ( !xStorage.is() )
                throw IOException();

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
        store(m_pImpl->m_sFileURL,_rArguments);
        notifyEvent(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSaveAsDone")));
    }
    else
        throw IOException();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::storeToURL( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException)
{
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");
    MutexGuard aGuard(m_aMutex);

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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    return m_pImpl->m_bModified;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setModified( sal_Bool _bModified ) throw (PropertyVetoException, RuntimeException)
{
    ResettableMutexGuard _rGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);

    if ( m_pImpl->m_bModified != _bModified )
    {
        m_pImpl->m_bModified = _bModified;
        lang::EventObject aEvt(*this);
        NOTIFY_LISTERNERS(m_aModifyListeners,XModifyListener,modified)
        notifyEvent(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnModifyChanged")));
    }
}
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
    // used only to forward external events (e.g. for doc creation) from the frame loader
    // to the global event broadcaster and all other interested doc event listener.
    notifyEvent(aEvent.EventName);
}
// -----------------------------------------------------------------------------
// ::com::sun::star::view::XPrintable
Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getPrinter(  ) throw (RuntimeException)
{
    return Sequence< PropertyValue >();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::setPrinter( const Sequence< PropertyValue >& aPrinter ) throw (IllegalArgumentException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::print( const Sequence< PropertyValue >& xOptions ) throw (IllegalArgumentException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseDocument::close( sal_Bool bDeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, RuntimeException)
{
    ResettableMutexGuard _rGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);

    lang::EventObject aEvt( static_cast< ::cppu::OWeakObject* >( this ) );
    NOTIFY_LISTERNERS1(m_aCloseListener,com::sun::star::util::XCloseListener,queryClosing,bDeliverOwnership);

    ::std::vector< Reference< XController> > aCopy = m_pImpl->m_aControllers;
    ::std::vector< Reference< XController> >::iterator aIter = aCopy.begin();
    ::std::vector< Reference< XController> >::iterator aEnd = aCopy.end();
    for (;aIter != aEnd ; ++aIter)
    {
        if ( aIter->is() )
        {
            Reference< XCloseable> xFrame((*aIter)->getFrame(),UNO_QUERY);
            if ( xFrame.is() )
                xFrame->close(bDeliverOwnership);
        }
    }
    if ( m_pImpl.is() )
         m_pImpl->m_aControllers.clear();
    dispose();
    {
        NOTIFY_LISTERNERS(m_aCloseListener,com::sun::star::util::XCloseListener,notifyClosing);
    }
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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Reference< XNameAccess > xContainer = m_pImpl->m_xForms;
    if ( !xContainer.is() )
    {
        if ( !m_pImpl->m_aContainer[ODatabaseModelImpl::E_FORM].get() )
        {
            ::rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("forms"));
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_FORM] = TContentPtr(new ODefinitionContainer_Impl);
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_FORM]->m_pDataSource = m_pImpl.get();
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_FORM]->m_aProps.aTitle = sName;
        }
        xContainer = new ODocumentContainer(m_pImpl->m_xServiceFactory,*this,m_pImpl->m_aContainer[ODatabaseModelImpl::E_FORM],sal_True);
        m_pImpl->m_xForms = xContainer;
    }
    return xContainer;
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseDocument::getReportDocuments(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
    OSL_ENSURE(m_pImpl.is(),"Impl is NULL");

    Reference< XNameAccess > xContainer = m_pImpl->m_xReports;
    if ( !xContainer.is() )
    {
        if ( !m_pImpl->m_aContainer[ODatabaseModelImpl::E_REPORT].get() )
        {
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_REPORT] = TContentPtr(new ODefinitionContainer_Impl);
            ::rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("reports"));
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_REPORT]->m_pDataSource = m_pImpl.get();
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_REPORT]->m_aProps.aTitle = sName;
        }
        xContainer = new ODocumentContainer(m_pImpl->m_xServiceFactory,*this,m_pImpl->m_aContainer[ODatabaseModelImpl::E_REPORT],sal_False);
        m_pImpl->m_xReports = xContainer;
    }
    return xContainer;
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

    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
    ::rtl::OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
    Any aAny;
    aAny <<= aMime;
    xStreamProp->setPropertyValue( aPropName, aAny );

    if( bPlainStream )
    {
        ::rtl::OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Compressed") );
        sal_Bool bFalse = sal_False;
        aAny.setValue( &bFalse, ::getBooleanCppuType() );
        xStreamProp->setPropertyValue( aPropName, aAny );
    }
    else
    {
        ::rtl::OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Encrypted") );
        sal_Bool bTrue = sal_True;
        aAny.setValue( &bTrue, ::getBooleanCppuType() );
        xStreamProp->setPropertyValue( aPropName, aAny );
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
        static const ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
        Any aAny;
        aAny <<= MIMETYPE_OASIS_OPENDOCUMENT_DATABASE;
        xProp->setPropertyValue( sPropName, aAny );
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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);
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
                rtl::OUString aMediaTypeProp( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ));
                rtl::OUString aUIConfigMediaType( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.xml.ui.configuration" ));
                rtl::OUString aMediaType;
                Reference< XPropertySet > xPropSet( xConfigStorage, UNO_QUERY );
                Any a = xPropSet->getPropertyValue( aMediaTypeProp );
                if ( !( a >>= aMediaType ) || ( aMediaType.getLength() == 0 ))
                {
                    a <<= aUIConfigMediaType;
                    xPropSet->setPropertyValue( aMediaTypeProp, a );
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
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ODatabaseDocument_OfficeDocument::rBHelper.bDisposed);

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
void ODatabaseDocument::notifyEvent(const ::rtl::OUString& _sEventName)
{
    try
    {
        ResettableMutexGuard _rGuard(m_aMutex);
        if (ODatabaseDocument_OfficeDocument::rBHelper.bDisposed)
            throw DisposedException();

        css::document::EventObject aEvt(*this, _sEventName);
        /// TODO: this code has to be deleted after as cws will be integrated
        try
        {
            Reference< ::com::sun::star::document::XEventListener > xDocEventBroadcaster(m_pImpl->m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.GlobalEventBroadcaster"))),
                UNO_QUERY);
            if ( xDocEventBroadcaster.is() )
            {
                xDocEventBroadcaster->notifyEvent(aEvt);
            }
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Could not create GlobalEventBroadcaster!");
        }
        NOTIFY_LISTERNERS(m_aDocEventListeners,css::document::XEventListener,notifyEvent)
    }
    catch(Exception&)
    {
    }
}
//------------------------------------------------------------------------------
void ODatabaseDocument::disposing()
{
    Reference< XModel > xHoldAlive( this );
    {
        notifyEvent(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnUnload")));

        css::lang::EventObject aDisposeEvent(static_cast<XWeak*>(this));
        m_aModifyListeners.disposeAndClear( aDisposeEvent );
        m_aCloseListener.disposeAndClear( aDisposeEvent );
        m_aDocEventListeners.disposeAndClear( aDisposeEvent );

        m_xDocEventBroadcaster = NULL;
        m_xUIConfigurationManager = NULL;

        Reference<XChild> xChild(m_pImpl->m_xForms.get(),UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(NULL);

        xChild.set(m_pImpl->m_xReports.get(),UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(NULL);
        xChild.set(m_pImpl->m_xTableDefinitions.get(),UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(NULL);
        xChild.set(m_pImpl->m_xCommandDefinitions.get(),UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(NULL);

        m_pImpl->modelIsDisposing( ODatabaseModelImpl::ResetModelAccess() );
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
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
rtl::OUString ODatabaseDocument::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.dba.ODatabaseDocument");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseDocument::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------------------
Reference< XInterface > ODatabaseDocument::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return ODatabaseDocument_CreateInstance(_rxFactory);
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseDocument::getSupportedServiceNames_Static(  ) throw (RuntimeException)
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
//........................................................................
}   // namespace dbaccess
//........................................................................


