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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
/**************************************************************************
                                TODO
 **************************************************************************

 - filter unwanted models notified by global document event broadcaster
   - help documents
   - others, which I don't know yet

 *************************************************************************/

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "cppuhelper/weak.hxx"

#include "comphelper/namedvaluecollection.hxx"
#include "comphelper/documentinfo.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/XEnumerationAccess.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/document/XStorageBasedDocument.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"

#include "tdoc_docmgr.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;
using ::comphelper::DocumentInfo;

//=========================================================================
//=========================================================================
//
// OfficeDocumentsManager Implementation.
//
//=========================================================================
//=========================================================================

OfficeDocumentsManager::OfficeDocumentsManager(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            OfficeDocumentsEventListener * pDocEventListener )
: m_xSMgr( xSMgr ),
  m_xDocEvtNotifier( createDocumentEventNotifier( xSMgr ) ),
  m_pDocEventListener( pDocEventListener )
{
    if ( m_xDocEvtNotifier.is() )
    {
        // Order is important (multithreaded environment)
        m_xDocEvtNotifier->addEventListener( this );
        buildDocumentsList();
    }
}

//=========================================================================
// virtual
OfficeDocumentsManager::~OfficeDocumentsManager()
{
    //OSL_ENSURE( m_aDocs.empty(), "document list not empty!" );
    // no need to assert this: Normal shutdown of OOo could already trigger it, since the order in which
    // objects are actually released/destroyed upon shutdown is not defined. And when we arrive *here*,
    // OOo *is* shutting down currently, since we're held by the TDOC provider, which is disposed
    // upon shutdown.
}

//=========================================================================
void OfficeDocumentsManager::destroy()
{
    if ( m_xDocEvtNotifier.is() )
        m_xDocEvtNotifier->removeEventListener( this );
}

//=========================================================================
static rtl::OUString
getDocumentId( const uno::Reference< uno::XInterface > & xDoc )
{
    rtl::OUString aId;

    // Try to get the UID directly from the document.
    uno::Reference< beans::XPropertySet > xPropSet( xDoc, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            uno::Any aValue = xPropSet->getPropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "RuntimeUID" ) ) );
            aValue >>= aId;
        }
        catch ( beans::UnknownPropertyException const & )
        {
            // Not actually an error. Property is optional.
        }
        catch ( lang::WrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught WrappedTargetException!" );
        }
    }

    if ( aId.getLength() == 0 )
    {
        // fallback: generate UID from document's this pointer.
        // normalize the interface pointer first. Else, calls with different
        // interfaces to the same object (say, XFoo and XBar) will produce
        // different IDs
        uno::Reference< uno::XInterface > xNormalizedIFace( xDoc, uno::UNO_QUERY );
        sal_Int64 nId = reinterpret_cast< sal_Int64 >( xNormalizedIFace.get() );
        aId = rtl::OUString::valueOf( nId );
    }

    OSL_ENSURE( aId.getLength() > 0, "getDocumentId - Empty id!" );
    return aId;
}

//=========================================================================
//
// document::XEventListener
//
//=========================================================================

// virtual
void SAL_CALL OfficeDocumentsManager::notifyEvent(
        const document::EventObject & Event )
    throw ( uno::RuntimeException )
{
/*
    Events documentation: OOo Developer's Guide / Writing UNO Components / Jobs
*/

    if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnLoadFinished" ) ) // document loaded
         || Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnCreate" ) ) ) // document created
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            osl::MutexGuard aGuard( m_aMtx );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            DocumentList::const_iterator it = m_aDocs.begin();
            while ( it != m_aDocs.end() )
            {
                if ( (*it).second.xModel == xModel )
                {
                    // already known.
                    break;
                }
                ++it;
            }

            if ( it == m_aDocs.end() )
            {
                // new document

                uno::Reference< document::XStorageBasedDocument >
                    xDoc( Event.Source, uno::UNO_QUERY );
                OSL_ENSURE( xDoc.is(), "Got no document::XStorageBasedDocument!" );

                uno::Reference< embed::XStorage > xStorage
                    = xDoc->getDocumentStorage();
                OSL_ENSURE( xStorage.is(), "Got no document storage!" );

                rtl:: OUString aDocId = getDocumentId( Event.Source );
                rtl:: OUString aTitle = DocumentInfo::getDocumentTitle( uno::Reference< frame::XModel >( Event.Source, uno::UNO_QUERY ) );

                m_aDocs[ aDocId ] = StorageInfo( aTitle, xStorage, xModel );

                // Propagate document closure.
                OSL_ENSURE( m_pDocEventListener,
                    "OnLoadFinished/OnCreate event: no owner for insert event propagation!" );

                if ( m_pDocEventListener )
                    m_pDocEventListener->notifyDocumentOpened( aDocId );
            }
        }
    }
    else if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnUnload" ) ) )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            // Document has been closed (unloaded)

            osl::MutexGuard aGuard( m_aMtx );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            DocumentList::iterator it = m_aDocs.begin();
            while ( it != m_aDocs.end() )
            {
                if ( (*it).second.xModel == xModel )
                {
                    // Propagate document closure.
                    OSL_ENSURE( m_pDocEventListener,
                        "OnUnload event: no owner for close event propagation!" );

                    if ( m_pDocEventListener )
                    {
                        rtl::OUString aDocId( (*it).first );
                        m_pDocEventListener->notifyDocumentClosed( aDocId );
                    }


                    break;
                }
                ++it;
            }

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnUnload event notified for unknown document!" );

            if( it != m_aDocs.end() )
                m_aDocs.erase( it );
        }
    }
    else if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnSaveDone" ) ) )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            osl::MutexGuard aGuard( m_aMtx );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            DocumentList::iterator it = m_aDocs.begin();
            while ( it != m_aDocs.end() )
            {
                if ( (*it).second.xModel == xModel )
                {
                    // Storage gets exchanged while saving.
                    uno::Reference< document::XStorageBasedDocument >
                        xDoc( Event.Source, uno::UNO_QUERY );
                    OSL_ENSURE( xDoc.is(),
                                "Got no document::XStorageBasedDocument!" );

                    uno::Reference< embed::XStorage > xStorage
                        = xDoc->getDocumentStorage();
                    OSL_ENSURE( xStorage.is(), "Got no document storage!" );

                    (*it).second.xStorage = xStorage;
                    break;
                }
                ++it;
            }

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnSaveDone event notified for unknown document!" );
        }
    }
    else if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnSaveAsDone" ) ) )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            osl::MutexGuard aGuard( m_aMtx );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            DocumentList::iterator it = m_aDocs.begin();
            while ( it != m_aDocs.end() )
            {
                if ( (*it).second.xModel == xModel )
                {
                    // Storage gets exchanged while saving.
                    uno::Reference< document::XStorageBasedDocument >
                        xDoc( Event.Source, uno::UNO_QUERY );
                    OSL_ENSURE( xDoc.is(),
                                "Got no document::XStorageBasedDocument!" );

                    uno::Reference< embed::XStorage > xStorage
                        = xDoc->getDocumentStorage();
                    OSL_ENSURE( xStorage.is(), "Got no document storage!" );

                    (*it).second.xStorage = xStorage;

                    // Adjust title.
                    (*it).second.aTitle = DocumentInfo::getDocumentTitle( uno::Reference< frame::XModel >( Event.Source, uno::UNO_QUERY ) );
                    break;
                }
                ++it;
            }

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnSaveAsDone event notified for unknown document!" );
        }
    }
    else if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "OnTitleChanged" ) ) )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            osl::MutexGuard aGuard( m_aMtx );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            DocumentList::iterator it = m_aDocs.begin();
            while ( it != m_aDocs.end() )
            {
                if ( (*it).second.xModel == xModel )
                {
                    // Adjust title.
                    rtl:: OUString aTitle = DocumentInfo::getDocumentTitle( uno::Reference< frame::XModel >( Event.Source, uno::UNO_QUERY ) );
                    (*it).second.aTitle = aTitle;

                    // Adjust storage.
                    uno::Reference< document::XStorageBasedDocument >
                        xDoc( Event.Source, uno::UNO_QUERY );
                    OSL_ENSURE( xDoc.is(), "Got no document::XStorageBasedDocument!" );

                    uno::Reference< embed::XStorage > xStorage
                        = xDoc->getDocumentStorage();
                    OSL_ENSURE( xDoc.is(), "Got no document storage!" );

                    rtl:: OUString aDocId = getDocumentId( Event.Source );

                    m_aDocs[ aDocId ] = StorageInfo( aTitle, xStorage, xModel );
                    break;
                }
                ++it;
            }

            OSL_ENSURE( it != m_aDocs.end(),
                        "TitleChanged event notified for unknown document!" );
        }
    }
}

//=========================================================================
//
// lang::XEventListener (base of document::XEventListener)
//
//=========================================================================

// virtual
void SAL_CALL OfficeDocumentsManager::disposing(
        const lang::EventObject& /*Source*/ )
    throw ( uno::RuntimeException )
{
}

//=========================================================================
//
// Non-interface.
//
//=========================================================================

// static
uno::Reference< document::XEventBroadcaster >
OfficeDocumentsManager::createDocumentEventNotifier(
        const uno::Reference< lang::XMultiServiceFactory >& rXSMgr )
{
    uno::Reference< uno::XInterface > xIfc;
    try
    {
        xIfc = rXSMgr->createInstance(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.frame.GlobalEventBroadcaster" ) ) );
    }
    catch ( uno::Exception const & )
    {
        // handled below.
    }

    OSL_ENSURE(
        xIfc.is(),
        "Could not instanciate com.sun.star.frame.GlobalEventBroadcaster" );

    if ( xIfc.is() )
    {
        uno::Reference< document::XEventBroadcaster > xBC(
            xIfc, uno::UNO_QUERY );

        OSL_ENSURE(
            xBC.is(),
            "com.sun.star.frame.GlobalEventBroadcaster does not implement "
            "interface com.sun.star.document.XEventBroadcaster!" );

        return xBC;
    }
    else
        return uno::Reference< document::XEventBroadcaster >();
}

//=========================================================================
void OfficeDocumentsManager::buildDocumentsList()
{
    OSL_ENSURE( m_xDocEvtNotifier.is(),
                "OfficeDocumentsManager::buildDocumentsList - "
                "No document event notifier!" );

    uno::Reference< container::XEnumerationAccess > xEnumAccess(
        m_xDocEvtNotifier, uno::UNO_QUERY_THROW );

    uno::Reference< container::XEnumeration > xEnum
        = xEnumAccess->createEnumeration();

    osl::MutexGuard aGuard( m_aMtx );

    while ( xEnum->hasMoreElements() )
    {
        uno::Any aValue = xEnum->nextElement();
        // container::NoSuchElementException
        // lang::WrappedTargetException

        try
        {
            uno::Reference< frame::XModel > xModel;
            aValue >>= xModel;

            if ( xModel.is() )
            {
                if ( isOfficeDocument( xModel ) )
                {
                    DocumentList::const_iterator it = m_aDocs.begin();
                    while ( it != m_aDocs.end() )
                    {
                        if ( (*it).second.xModel == xModel )
                        {
                            // already known.
                            break;
                        }
                        ++it;
                    }

                    if ( it == m_aDocs.end() )
                    {
                        // new document
                        rtl::OUString aDocId = getDocumentId( xModel );
                        rtl::OUString aTitle = DocumentInfo::getDocumentTitle( xModel );

                        uno::Reference< document::XStorageBasedDocument >
                                xDoc( xModel, uno::UNO_QUERY );
                        OSL_ENSURE( xDoc.is(),
                            "Got no document::XStorageBasedDocument!" );

                        uno::Reference< embed::XStorage > xStorage
                            = xDoc->getDocumentStorage();
                        OSL_ENSURE( xDoc.is(), "Got no document storage!" );

                        m_aDocs[ aDocId ]
                            = StorageInfo( aTitle, xStorage, xModel );
                    }
                }
            }
        }
        catch ( lang::DisposedException const & )
        {
            // Note: Due to race conditions the XEnumeration can
            //       contains docs that already have been closed
        }
    }
}

//=========================================================================
uno::Reference< embed::XStorage >
OfficeDocumentsManager::queryStorage( const rtl::OUString & rDocId )
{
    osl::MutexGuard aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return uno::Reference< embed::XStorage >();

    return (*it).second.xStorage;
}

//=========================================================================
rtl::OUString OfficeDocumentsManager::queryDocumentId(
    const uno::Reference< frame::XModel > & xModel )
{
    return getDocumentId( xModel );
}

//=========================================================================
uno::Reference< frame::XModel >
OfficeDocumentsManager::queryDocumentModel( const rtl::OUString & rDocId )
{
    osl::MutexGuard aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return uno::Reference< frame::XModel >();

    return (*it).second.xModel;
}

//=========================================================================
uno::Sequence< rtl::OUString > OfficeDocumentsManager::queryDocuments()
{
    osl::MutexGuard aGuard( m_aMtx );

    uno::Sequence< rtl::OUString > aRet( m_aDocs.size() );
    sal_Int32 nPos = 0;

    DocumentList::const_iterator it = m_aDocs.begin();
    while ( it != m_aDocs.end() )
    {
        aRet[ nPos ] = (*it).first;
        ++it;
        ++nPos;
    }
    return aRet;
}

//=========================================================================
rtl::OUString
OfficeDocumentsManager::queryStorageTitle( const rtl::OUString & rDocId )
{
    osl::MutexGuard aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return rtl::OUString();

    return (*it).second.aTitle;
}

//=========================================================================
bool OfficeDocumentsManager::isDocumentPreview(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !xModel.is() )
        return false;

    ::comphelper::NamedValueCollection aArgs(
        xModel->getArgs() );
    sal_Bool bIsPreview = aArgs.getOrDefault( "Preview", sal_False );
    return bIsPreview;
}

//=========================================================================
bool OfficeDocumentsManager::isHelpDocument(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !xModel.is() )
        return false;

    ::rtl::OUString sURL( xModel->getURL() );
    if ( sURL.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.help://" ) ) )
        return true;

    return false;
}

//=========================================================================
bool OfficeDocumentsManager::isWithoutOrInTopLevelFrame(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !xModel.is() )
        return false;

    uno::Reference< frame::XController > xController
        = xModel->getCurrentController();
    if ( xController.is() )
    {
        uno::Reference< frame::XFrame > xFrame
            = xController->getFrame();
        if ( xFrame.is() )
        {
            // don't use XFrame::isTop here. This nowadays excludes
            // "sub documents" such as forms embedded in database documents
            uno::Reference< awt::XTopWindow > xFrameContainer(
                xFrame->getContainerWindow(), uno::UNO_QUERY );
            if ( !xFrameContainer.is() )
                return false;
        }
    }

    return true;
}

//=========================================================================
bool OfficeDocumentsManager::isBasicIDE(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !m_xModuleMgr.is() )
    {
        osl::MutexGuard aGuard( m_aMtx );
        if ( !m_xModuleMgr.is() )
        {
            try
            {
                m_xModuleMgr
                    = uno::Reference<
                        frame::XModuleManager >(
                            m_xSMgr->createInstance(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.frame.ModuleManager" ) ) ),
                            uno::UNO_QUERY );
            }
            catch ( uno::Exception const & )
            {
                // handled below.
            }

            OSL_ENSURE( m_xModuleMgr .is(),
                        "Could not instanciate ModuleManager service!" );
        }
    }

    if ( m_xModuleMgr.is() )
    {
        rtl::OUString aModule;
        try
        {
            aModule = m_xModuleMgr->identify( xModel );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( frame::UnknownModuleException const & )
        {
            OSL_ENSURE( false, "Caught UnknownModuleException!" );
        }

        if ( aModule.getLength() > 0 )
        {
            // Filter unwanted items, that are no real documents.
            if ( aModule.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.star.script.BasicIDE" ) ) )
            {
                return true;
            }
        }
    }

    return false;
}

//=========================================================================
bool OfficeDocumentsManager::isOfficeDocument(
        const uno::Reference< uno::XInterface > & xDoc )
{
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    uno::Reference< document::XStorageBasedDocument >
        xStorageBasedDoc( xModel, uno::UNO_QUERY );
    if ( !xStorageBasedDoc.is() )
        return false;

    if ( !isWithoutOrInTopLevelFrame( xModel ) )
        return false;

    if ( isDocumentPreview( xModel ) )
        return false;

    if ( isHelpDocument( xModel ) )
        return false;

    if ( isBasicIDE( xModel ) )
        return false;

    return true;
}
