/*************************************************************************
 *
 *  $RCSfile: databasedocument.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:09:11 $
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
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
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
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONSTORAGE_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationStorage.hpp>
#endif
#ifndef DBA_COREDATAACCESS_COMMITLISTENER_HXX
#include "commitlistener.hxx"
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONBROADCASTER_HPP_
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

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
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::cppu;
using namespace ::osl;

//........................................................................
namespace dbaccess
{
//........................................................................
// XComponent
void SAL_CALL ODatabaseSource::dispose(  ) throw (RuntimeException)
{
    OSubComponent::dispose();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::addEventListener( const Reference< XEventListener >& _xListener ) throw (RuntimeException)
{
    OSubComponent::addEventListener(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::removeEventListener( const Reference< XEventListener >& _xListener ) throw (RuntimeException)
{
    OSubComponent::removeEventListener(_xListener);
}
// -----------------------------------------------------------------------------
// XModel
sal_Bool SAL_CALL ODatabaseSource::attachResource( const ::rtl::OUString& _sURL, const Sequence< PropertyValue >& _aArguments ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    m_aArgs = _aArguments;
    m_sFileURL = _sURL;
    if ( !m_sName.getLength() )
        m_sName = m_sFileURL;
    if ( m_pDBContext )
        m_pDBContext->registerPrivate(_sURL,*this);
    getStorage();
    return sal_True;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseSource::getURL(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_sFileURL;
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseSource::getArgs(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_aArgs;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::connectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    m_aControllers.push_back(_xController);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::disconnectController( const Reference< XController >& _xController ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    m_aControllers.erase(::std::find(m_aControllers.begin(),m_aControllers.end(),_xController));
    if ( m_xCurrentController == _xController )
        m_xCurrentController = NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::lockControllers(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    ++m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::unlockControllers(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    --m_nControllerLockCount;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseSource::hasControllersLocked(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_nControllerLockCount != 0;
}
// -----------------------------------------------------------------------------
Reference< XController > SAL_CALL ODatabaseSource::getCurrentController() throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_xCurrentController.is() ? m_xCurrentController : ( m_aControllers.empty() ? Reference< XController >() : *m_aControllers.begin() );
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::setCurrentController( const Reference< XController >& _xController ) throw (NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    m_xCurrentController = _xController;
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseSource::getCurrentSelection(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XInterface > xRet;
    Reference< XSelectionSupplier >  xDocView( getCurrentController(), UNO_QUERY );
    if ( xDocView.is() )
        xRet.set(xDocView->getSelection(),UNO_QUERY);

    return xRet;
}
// -----------------------------------------------------------------------------

// XStorable
sal_Bool SAL_CALL ODatabaseSource::hasLocation(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_sFileURL.getLength() != 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseSource::getLocation(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_sFileURL;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseSource::isReadonly(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_bDocumentReadOnly;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::store(  ) throw (IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    store(m_sFileURL,m_aArgs);
}
// -----------------------------------------------------------------------------
void ODatabaseSource::store(const ::rtl::OUString& sURL, const Sequence< PropertyValue >& lArguments )
{
    if ( m_bDocumentReadOnly )
        throw IOException();

    try
    {

//      if ( m_aContainer[E_FORM].get() )
//      {
//          Reference<XTransactedObject> xForms(getFormDocuments(),UNO_QUERY);
//          if ( xForms.is() )
//              xForms->commit();
//      }

//      if ( m_aContainer[E_REPORT].get() )
//      {
//          Reference<XTransactedObject> xReports(getReportDocuments(),UNO_QUERY);
//          if ( xReports.is() )
//              xReports->commit();
//      }
        TStorages::iterator aIter = m_aStorages.begin();
        TStorages::iterator aEnd = m_aStorages.end();
        for (; aIter != aEnd ; ++aIter)
        {
            Reference<XTransactedObject> xTrans(aIter->second,UNO_QUERY);
            if ( xTrans.is() )
                xTrans->commit();
        }
    }
    catch(WrappedTargetException)
    {
        throw IOException();
    }

    writeStorage(sURL,lArguments);

    try
    {
        Reference<XTransactedObject> xTransact(getStorage(),UNO_QUERY);
        if ( xTransact.is() )
            xTransact->commit();
    }
    catch(Exception)
    {
    }
    m_bModified = sal_False;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::storeAsURL( const ::rtl::OUString& sURL, const Sequence< PropertyValue >& lArguments ) throw (IOException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference<XSingleServiceFactory> xStorageFactory(m_xServiceFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.StorageFactory")) ),UNO_QUERY);
    if ( xStorageFactory.is() )
    {
        if ( sURL != m_sFileURL )
        {
            Sequence<Any> aParam(2);
            aParam[0] <<= sURL;
            aParam[1] <<= ElementModes::READWRITE  | ElementModes::TRUNCATE;
            Reference<XStorage> xStorage(xStorageFactory->createInstanceWithArguments( aParam ),UNO_QUERY);
            if ( !xStorage.is() )
                throw IOException();

            Reference<XStorage> xMyStorage = getStorage();
            if ( xMyStorage.is() )
            {
                xMyStorage->copyToStorage( xStorage );
            }

            TStorages::iterator aIter = m_aStorages.begin();
            TStorages::iterator aEnd = m_aStorages.end();
            for (; aIter != aEnd ; ++aIter)
            {
                ::comphelper::disposeComponent(aIter->second);
            }
            m_aStorages.clear();
            m_xStorage = xStorage;
            ::comphelper::disposeComponent(xMyStorage);

            m_bDocumentReadOnly = sal_False;
            if ( sURL != m_sFileURL )
            {
                if ( m_pDBContext )
                    m_pDBContext->nameChangePrivate(m_sFileURL,sURL);

                INetURLObject aURL( m_sName );
                if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                    m_sName = sURL;
            }
            m_sFileURL = sURL;

        }
        m_aArgs = lArguments;
        store();
    }
    else
        throw IOException();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::storeToURL( const ::rtl::OUString& sURL, const Sequence< PropertyValue >& lArguments ) throw (IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    Reference<XSingleServiceFactory> xStorageFactory(m_xServiceFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.StorageFactory")) ),UNO_QUERY);
    if ( xStorageFactory.is() )
    {
        Reference<XStorage> xMyStorage = getStorage();
        if ( xMyStorage.is() )
        {
            Sequence<Any> aParam(2);
            aParam[0] <<= sURL;
            aParam[1] <<= ElementModes::READWRITE  | ElementModes::TRUNCATE;
            Reference<XStorage> xStorage(xStorageFactory->createInstanceWithArguments( aParam ),UNO_QUERY);
            if ( !xStorage.is() )
                throw IOException();

            if ( ! m_bDocumentReadOnly )
                store(sURL,lArguments);
            xMyStorage->copyToStorage( xStorage );
        }
        else
            throw IOException();
    }
}
// -----------------------------------------------------------------------------
// XModifyBroadcaster
void SAL_CALL ODatabaseSource::addModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        return;
    m_aModifyListeners.addInterface(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::removeModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    m_aModifyListeners.removeInterface(_xListener);
}
// -----------------------------------------------------------------------------
// XModifiable
sal_Bool SAL_CALL ODatabaseSource::isModified(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    return m_bModified;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::setModified( sal_Bool _bModified ) throw (PropertyVetoException, RuntimeException)
{
    ResettableMutexGuard _rGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    if ( m_bModified != _bModified )
    {
        m_bModified = _bModified;
        lang::EventObject aEvt(*this);
        NOTIFY_LISTERNERS(m_aModifyListeners,XModifyListener,modified)
    }
}
// -----------------------------------------------------------------------------
// ::com::sun::star::view::XPrintable
Sequence< PropertyValue > SAL_CALL ODatabaseSource::getPrinter(  ) throw (RuntimeException)
{
    return Sequence< PropertyValue >();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::setPrinter( const Sequence< PropertyValue >& aPrinter ) throw (IllegalArgumentException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::print( const Sequence< PropertyValue >& xOptions ) throw (IllegalArgumentException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::close( sal_Bool bDeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, RuntimeException)
{
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
    ::cppu::OInterfaceIteratorHelper pIterator(m_aCloseListener);
    while (pIterator.hasMoreElements())
    {
        try
        {
            ((com::sun::star::util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
        }
        catch( RuntimeException& )
        {
            pIterator.remove();
        }
    }

    ::std::vector< Reference< XController> > aCopy = m_aControllers;
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
    m_aControllers.clear();
    dispose();

    ::cppu::OInterfaceIteratorHelper pCloseIterator(m_aCloseListener);
    while (pCloseIterator.hasMoreElements())
    {
        try
        {
            ((com::sun::star::util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
        }
        catch( RuntimeException& )
        {
            pCloseIterator.remove();
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::addCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if ( Listener.is() && !OComponentHelper::rBHelper.bDisposed )
        m_aCloseListener.addInterface(Listener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::removeCloseListener( const Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if ( Listener.is() && !OComponentHelper::rBHelper.bDisposed )
        m_aCloseListener.removeInterface(Listener);
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getFormDocuments(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XNameAccess > xContainer = m_xForms;
    if ( !xContainer.is() )
    {
        if ( !m_aContainer[E_FORM].get() )
        {
            ::rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("forms"));
            m_aContainer[E_FORM] = TContentPtr(new ODefinitionContainer_Impl);
            m_aContainer[E_FORM]->m_pDataSource = this;
            m_aContainer[E_FORM]->m_aProps.aTitle = sName;
        }
        xContainer = new ODocumentContainer(m_xServiceFactory,*this,m_aContainer[E_FORM],sal_True);
        m_xForms = xContainer;
    }
    return xContainer;
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getReportDocuments(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XNameAccess > xContainer = m_xReports;
    if ( !xContainer.is() )
    {
        if ( !m_aContainer[E_REPORT].get() )
        {
            m_aContainer[E_REPORT] = TContentPtr(new ODefinitionContainer_Impl);
            ::rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("reports"));
            m_aContainer[E_REPORT]->m_pDataSource = this;
            m_aContainer[E_REPORT]->m_aProps.aTitle = sName;
        }
        xContainer = new ODocumentContainer(m_xServiceFactory,*this,m_aContainer[E_REPORT],sal_False);
        m_xReports = xContainer;
    }
    return xContainer;
}
// -----------------------------------------------------------------------------
Reference<XStorage> ODatabaseSource::getStorage()
{
    if ( !m_xStorage.is() )
    {
        Reference< XSingleServiceFactory> xStorageFactory;
        xStorageFactory.set(m_xServiceFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.StorageFactory"))),UNO_QUERY);

        if ( xStorageFactory.is() && m_sFileURL.getLength() )
        {
            Sequence<Any> aArgs(2);
            const PropertyValue* pValue =::std::find_if(m_aArgs.getConstArray(),
                                                        m_aArgs.getConstArray() + m_aArgs.getLength(),
                                                        ::std::bind2nd(::comphelper::TPropertyValueEqualFunctor(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"))));
            if ( pValue && pValue != (m_aArgs.getConstArray() + m_aArgs.getLength()) )
                aArgs[0] = pValue->Value;
            else
                aArgs[0] <<= m_sFileURL;

            aArgs[1] <<= ElementModes::READWRITE;

            try
            {
                m_xStorage.set( xStorageFactory->createInstanceWithArguments( aArgs ),UNO_QUERY );
            }
            catch(Exception)
            {
                m_bDocumentReadOnly = sal_True;
                aArgs[1] <<= ElementModes::READ;
                try
                {
                    m_xStorage.set( xStorageFactory->createInstanceWithArguments( aArgs ),UNO_QUERY );
                }
                catch(Exception)
                {
                }
            }
        }
    }
    return m_xStorage;
}
// -----------------------------------------------------------------------------
Reference<XStorage> ODatabaseSource::getStorage(const ::rtl::OUString& _sStorageName, sal_Int32 nMode)
{
    OSL_ENSURE(_sStorageName.getLength(),"ODatabaseSource::getStorage: Invalid storage name!");
    Reference<XStorage> xStorage;
    TStorages::iterator aFind = m_aStorages.find(_sStorageName);
    if ( aFind == m_aStorages.end() )
    {
        Reference<XStorage> xMyStorage = getStorage();
        Reference<XNameAccess> xNames(xMyStorage,UNO_QUERY);
        if ( xMyStorage.is() )
        {
            try
            {
                xStorage = xMyStorage->openStorageElement(_sStorageName, m_bDocumentReadOnly ? ElementModes::READ : nMode);
                aFind = m_aStorages.insert(TStorages::value_type(_sStorageName,xStorage)).first;
            }
            catch(Exception&)
            {
                OSL_ENSURE(0,"Stroage could not be accessed!");
            }
        }
    }

    if ( aFind != m_aStorages.end() )
        xStorage = aFind->second;

    return xStorage;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
sal_Bool ODatabaseSource::WriteThroughComponent(
    const Reference<XComponent> & xComponent,
    const sal_Char* pStreamName,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc,
    sal_Bool bPlainStream )
{
    OSL_ENSURE( NULL != pStreamName, "Need stream name!" );
    OSL_ENSURE( NULL != pServiceName, "Need service name!" );

    Reference<XStorage> xMyStorage = getStorage();
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
        sal_Int64 nPos = xSeek->getPosition();
        OSL_TRACE("Length of stream %d",&nPos);
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

sal_Bool ODatabaseSource::WriteThroughComponent(
    const Reference<XOutputStream> & xOutputStream,
    const Reference<XComponent> & xComponent,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc )
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( NULL != pServiceName, "Need component name!" );

    // get component
    Reference< XActiveDataSource > xSaxWriter(
        m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))),
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
        m_xServiceFactory->createInstanceWithArguments(
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
void ODatabaseSource::writeStorage(const ::rtl::OUString& _sURL, const Sequence< PropertyValue >& lArguments )
{
    // create XStatusIndicator
    Reference<XStatusIndicator> xStatusIndicator;
    try
    {
        const PropertyValue* pValue =::std::find_if(m_aArgs.getConstArray(),
                                                    m_aArgs.getConstArray() + m_aArgs.getLength(),
                                                    ::std::bind2nd(::comphelper::TPropertyValueEqualFunctor(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StatusIndicator"))));

        if ( pValue && pValue != (m_aArgs.getConstArray() + m_aArgs.getLength()) )
        {
            xStatusIndicator.set(pValue->Value,UNO_QUERY);

            // set progress range and start status indicator
            sal_Int32 nProgressRange(1000000);
            if ( xStatusIndicator.is() )
                xStatusIndicator->start(::rtl::OUString(), nProgressRange);
        }
    }
    catch( const RuntimeException& )
    {
        xStatusIndicator = NULL;
    }

    // filter arguments
    // - status indicator
    // - else empty
    sal_Int32 nArgs = 0;
    if( xStatusIndicator.is() )
        nArgs++;

    Sequence < Any > aEmptyArgs( nArgs );
    Any *pArgs = aEmptyArgs.getArray();
    if( xStatusIndicator.is() )
        *pArgs++ <<= xStatusIndicator;

    Sequence < Any > aFilterArgs( nArgs );
    pArgs = aFilterArgs.getArray();
    if( xStatusIndicator.is() )
        *pArgs++ <<= xStatusIndicator;

    // properties
    Sequence < PropertyValue > aProps( _sURL.getLength() ? 1 : 0 );
    if( _sURL.getLength() )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FileName") );
        (pProps++)->Value <<= _sURL;
    }

    // export sub streams for package, else full stream into a file
    sal_Bool bWarn = sal_False, bErr = sal_False;
    String sWarnFile, sErrFile;

//  if ( !bOrganizerMode && !bBlock &&
//      SFX_CREATE_MODE_EMBEDDED != pDoc->GetDocShell()->GetCreateMode() )
//  {
//      if( !WriteThroughComponent(
//              xModelComp, "meta.xml", xServiceFactory,
//              "com.sun.star.comp.Writer.XMLMetaExporter",
//              aEmptyArgs, aProps, sal_True ) )
//      {
//          bWarn = sal_True;
//          sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("meta.xml"),
//                              RTL_TEXTENCODING_ASCII_US );
//      }
//  }

//  if( !WriteThroughComponent(
//          xModelComp, "styles.xml", xServiceFactory,
//          "com.sun.star.comp.Writer.XMLStylesExporter",
//          aFilterArgs, aProps, sal_False ) )
//  {
//      bErr = sal_True;
//      sErrFile = String( RTL_CONSTASCII_STRINGPARAM("styles.xml"),
//                         RTL_TEXTENCODING_ASCII_US );
//  }

    Reference<XPropertySet> xProp(getStorage(),UNO_QUERY);
    if ( xProp.is() )
    {
        static const ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
        Any aAny;
        aAny <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.xml.base") );
        xProp->setPropertyValue( sPropName, aAny );
    }

    Reference<XComponent> xCom(static_cast<OWeakObject*>(this),UNO_QUERY);
    if( !bErr )
    {
        if( !WriteThroughComponent(
            xCom, "settings.xml",
            "com.sun.star.comp.sdb.XMLSettingsExporter",
            aEmptyArgs, aProps, sal_True ) )
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
                aFilterArgs, aProps, sal_True ) )
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
void SAL_CALL ODatabaseSource::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    setModified(sal_True);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
{
    setModified(sal_True);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
{
    setModified(sal_True);
}
// -----------------------------------------------------------------------------
Reference< ::drafts::com::sun::star::ui::XUIConfigurationManager > SAL_CALL ODatabaseSource::getUIConfigurationManager(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();
    if ( !m_xUIConfigurationManager.is() )
    {
        m_xUIConfigurationManager = Reference< ::drafts::com::sun::star::ui::XUIConfigurationManager >(
            m_xServiceFactory->createInstance(
                ::rtl::OUString::createFromAscii( "drafts.com.sun.star.ui.UIConfigurationManager" )),
                UNO_QUERY );

        Reference< ::drafts::com::sun::star::ui::XUIConfigurationStorage > xUIConfigStorage( m_xUIConfigurationManager, UNO_QUERY );
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
Reference< XStorage > SAL_CALL ODatabaseSource::getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XStorage > xResult = getStorage(aStorageName,nMode);
    if ( xResult.is() )
    {
        Reference< XTransactionBroadcaster > xBroadcaster( xResult, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            if ( m_pChildCommitListen == NULL )
            {
                m_pChildCommitListen = new OChildCommitListen_Impl( static_cast<XModifiable*>(this) );
                m_pChildCommitListen->acquire();
            }
            xBroadcaster->addTransactionListener( static_cast< XTransactionListener* >( m_pChildCommitListen ) );
        }
    }
    return xResult;
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseSource::getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(2);
    sal_Int32 nPos = 0;
    aRet[nPos++] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("forms"));
    aRet[nPos++] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("reports"));
    return aRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::flush(  ) throw (RuntimeException)
{
    try
    {
        ResettableMutexGuard _rGuard(m_aMutex);
        if (OComponentHelper::rBHelper.bDisposed)
            throw DisposedException();
        store(  );

        lang::EventObject aEvt(*this);
        NOTIFY_LISTERNERS(m_aFlushListeners,XFlushListener,flushed)
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::addFlushListener( const Reference< ::com::sun::star::util::XFlushListener >& _xListener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        return;
    m_aFlushListeners.addInterface(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::removeFlushListener( const Reference< ::com::sun::star::util::XFlushListener >& _xListener ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    m_aFlushListeners.removeInterface(_xListener);
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................
