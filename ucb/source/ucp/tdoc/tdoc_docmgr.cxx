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


#include <rtl/ref.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <tools/datetime.hxx>

#include <comphelper/documentinfo.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>

#include "tdoc_docmgr.hxx"
#include "tdoc_provider.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;

// OfficeDocumentsCloseListener Implementation.


// util::XCloseListener


// virtual
void SAL_CALL OfficeDocumentsManager::OfficeDocumentsCloseListener::queryClosing(
         const lang::EventObject& /*Source*/, sal_Bool /*GetsOwnership*/ )
{
}


void SAL_CALL OfficeDocumentsManager::OfficeDocumentsCloseListener::notifyClosing(
         const lang::EventObject& Source )
{
    if (!m_pManager) return; // disposed?

    document::DocumentEvent aDocEvent;
    aDocEvent.Source = Source.Source;
    aDocEvent.EventName = "OfficeDocumentsListener::notifyClosing";
    m_pManager->documentEventOccured( aDocEvent );
}


// lang::XDocumentEventListener (base of util::XCloseListener)


// virtual
void SAL_CALL OfficeDocumentsManager::OfficeDocumentsCloseListener::disposing(
        const lang::EventObject& /*Source*/ )
{
}


// OfficeDocumentsManager Implementation.


OfficeDocumentsManager::OfficeDocumentsManager(
            const uno::Reference< uno::XComponentContext > & rxContext,
            ContentProvider * pDocEventListener )
: m_xContext( rxContext ),
  m_xDocEvtNotifier( frame::theGlobalEventBroadcaster::get( rxContext ) ),
  m_pDocEventListener( pDocEventListener ),
  m_xDocCloseListener( new OfficeDocumentsCloseListener( this ) )
{
    // Order is important (multithreaded environment)
    uno::Reference< document::XDocumentEventBroadcaster >(
        m_xDocEvtNotifier, uno::UNO_QUERY_THROW )->addDocumentEventListener( this );
    buildDocumentsList();
}


// virtual
OfficeDocumentsManager::~OfficeDocumentsManager()
{
    //OSL_ENSURE( m_aDocs.empty(), "document list not empty!" );
    // no need to assert this: Normal shutdown of LibreOffice could already trigger it, since the order
    // in which objects are actually released/destroyed upon shutdown is not defined. And when we
    // arrive *here*, LibreOffice *is* shutting down currently, since we're held by the TDOC provider,
    // which is disposed upon shutdown.
    m_xDocCloseListener->Dispose();
}


void OfficeDocumentsManager::destroy()
{
    uno::Reference< document::XDocumentEventBroadcaster >(
        m_xDocEvtNotifier, uno::UNO_QUERY_THROW )->removeDocumentEventListener( this );
}


static OUString
getDocumentId( const uno::Reference< uno::XInterface > & xDoc )
{
    OUString aId;

    // Try to get the UID directly from the document.
    uno::Reference< beans::XPropertySet > xPropSet( xDoc, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            uno::Any aValue = xPropSet->getPropertyValue(u"RuntimeUID"_ustr);
            aValue >>= aId;
        }
        catch ( beans::UnknownPropertyException const & )
        {
            // Not actually an error. Property is optional.
        }
        catch ( lang::WrappedTargetException const & )
        {
            TOOLS_WARN_EXCEPTION("ucb.ucp", "Caught WrappedTargetException!");
        }
    }

    if ( aId.isEmpty() )
    {
        // fallback: generate UID from document's this pointer.
        // normalize the interface pointer first. Else, calls with different
        // interfaces to the same object (say, XFoo and XBar) will produce
        // different IDs
        uno::Reference< uno::XInterface > xNormalizedIFace( xDoc, uno::UNO_QUERY );
        sal_Int64 nId = reinterpret_cast< sal_Int64 >( xNormalizedIFace.get() );
        aId = OUString::number( nId );
    }

    OSL_ENSURE( !aId.isEmpty(), "getDocumentId - Empty id!" );
    return aId;
}


// document::XDocumentEventListener


// virtual
void SAL_CALL OfficeDocumentsManager::documentEventOccured(
        const document::DocumentEvent & Event )
{
/*
    Events documentation: OOo Developer's Guide / Writing UNO Components /
    Integrating Components into OpenOffice.org / Jobs
*/

    if ( Event.EventName == "OnLoadFinished" // document loaded
      || Event.EventName == "OnCreate" )     // document created
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            uno::Reference<frame::XModel> const xModel(
                    Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            bool found(false);

            {
                std::scoped_lock aGuard( m_aMtx );

                found = std::any_of(m_aDocs.begin(), m_aDocs.end(),
                    [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });
            }

            if (!found)
            {
                // no mutex to avoid deadlocks!
                // need no lock to access const members, ContentProvider is safe

                // new document

                uno::Reference< document::XStorageBasedDocument >
                    xDoc( Event.Source, uno::UNO_QUERY );
                OSL_ENSURE( xDoc.is(), "Got no document::XStorageBasedDocument!" );

                uno::Reference< embed::XStorage > xStorage
                    = xDoc->getDocumentStorage();
                OSL_ENSURE( xStorage.is(), "Got no document storage!" );

                rtl:: OUString aDocId = getDocumentId( Event.Source );
                rtl:: OUString aTitle = comphelper::DocumentInfo::getDocumentTitle(
                    uno::Reference< frame::XModel >( Event.Source, uno::UNO_QUERY ) );

                {
                    std::scoped_lock g(m_aMtx);
                    m_aDocs[ aDocId ] = StorageInfo( aTitle, xStorage, xModel );
                }

                uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster(
                    Event.Source, uno::UNO_QUERY );
                OSL_ENSURE( xCloseBroadcaster.is(),
                    "OnLoadFinished/OnCreate event: got no close broadcaster!" );

                if ( xCloseBroadcaster.is() )
                    xCloseBroadcaster->addCloseListener(m_xDocCloseListener);

                // Propagate document closure.
                OSL_ENSURE( m_pDocEventListener,
                    "OnLoadFinished/OnCreate event: no owner for insert event propagation!" );

                if ( m_pDocEventListener )
                    m_pDocEventListener->notifyDocumentOpened( aDocId );
            }
        }
    }
    else if ( Event.EventName == "OfficeDocumentsListener::notifyClosing" )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            // Document has been closed (unloaded)

            // Official event "OnUnload" does not work here. Event
            // gets fired too early. Other OnUnload listeners called after this
            // listener may still need TDOC access to the document. Remove the
            // document from TDOC docs list on XCloseListener::notifyClosing.
            // See OfficeDocumentsManager::OfficeDocumentsListener::notifyClosing.

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            bool found(false);
            OUString aDocId;

            {
                std::scoped_lock aGuard( m_aMtx );

                auto it = std::find_if(m_aDocs.begin(), m_aDocs.end(),
                    [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });
                if ( it != m_aDocs.end() )
                {
                    aDocId = (*it).first;
                    found = true;
                    m_aDocs.erase( it );
                }
            }

            OSL_ENSURE( found,
                        "OnUnload event notified for unknown document!" );

            if (found)
            {
                // Propagate document closure.
                OSL_ENSURE( m_pDocEventListener,
                    "OnUnload event: no owner for close event propagation!" );
                if (m_pDocEventListener)
                {
                    m_pDocEventListener->notifyDocumentClosed(aDocId);
                }
                uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster(
                    Event.Source, uno::UNO_QUERY );
                OSL_ENSURE( xCloseBroadcaster.is(),
                    "OnUnload event: got no XCloseBroadcaster from XModel" );
                if ( xCloseBroadcaster.is() )
                    xCloseBroadcaster->removeCloseListener(m_xDocCloseListener);
            }
        }
    }
    else if ( Event.EventName == "OnSaveDone" )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            // Storage gets exchanged while saving.
            uno::Reference<document::XStorageBasedDocument> const xDoc(
                    Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xDoc.is(),
                        "Got no document::XStorageBasedDocument!" );
            uno::Reference<embed::XStorage> const xStorage(
                xDoc->getDocumentStorage());
            OSL_ENSURE( xStorage.is(), "Got no document storage!" );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            std::scoped_lock aGuard( m_aMtx );

            DocumentList::iterator it = std::find_if(m_aDocs.begin(), m_aDocs.end(),
                [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnSaveDone event notified for unknown document!" );
            if ( it != m_aDocs.end() )
            {
                (*it).second.xStorage = xStorage;
            }
        }
    }
    else if ( Event.EventName == "OnSaveAsDone" )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            // Storage gets exchanged while saving.
            uno::Reference<document::XStorageBasedDocument> const xDoc(
                    Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xDoc.is(),
                        "Got no document::XStorageBasedDocument!" );
            uno::Reference<embed::XStorage> const xStorage(
                xDoc->getDocumentStorage());
            OSL_ENSURE( xStorage.is(), "Got no document storage!" );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            OUString const title(comphelper::DocumentInfo::getDocumentTitle(xModel));

            std::scoped_lock aGuard( m_aMtx );

            DocumentList::iterator it = std::find_if(m_aDocs.begin(), m_aDocs.end(),
                [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnSaveAsDone event notified for unknown document!" );
            if ( it != m_aDocs.end() )
            {
                (*it).second.xStorage = xStorage;

                // Adjust title.
                (*it).second.aTitle = title;
            }
        }
    }
    else if ( Event.EventName == "OnTitleChanged"
           || Event.EventName == "OnStorageChanged" )
    {
        if ( isOfficeDocument( Event.Source ) )
        {
            // Storage gets exchanged while saving.
            uno::Reference<document::XStorageBasedDocument> const xDoc(
                    Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xDoc.is(),
                        "Got no document::XStorageBasedDocument!" );
            uno::Reference<embed::XStorage> const xStorage(
                xDoc->getDocumentStorage());
            OSL_ENSURE( xStorage.is(), "Got no document storage!" );

            uno::Reference< frame::XModel >
                 xModel( Event.Source, uno::UNO_QUERY );
            OSL_ENSURE( xModel.is(), "Got no frame::XModel!" );

            OUString const aTitle(comphelper::DocumentInfo::getDocumentTitle(xModel));

            OUString const aDocId(getDocumentId(Event.Source));

            std::scoped_lock aGuard( m_aMtx );

            DocumentList::iterator it = std::find_if(m_aDocs.begin(), m_aDocs.end(),
                [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });
            if ( it != m_aDocs.end() )
            {
                // Adjust title.
                (*it).second.aTitle = aTitle;

                m_aDocs[ aDocId ] = StorageInfo( aTitle, xStorage, xModel );
            }

//            OSL_ENSURE( it != m_aDocs.end(),
//                        "TitleChanged event notified for unknown document!" );
            // TODO: re-enable this assertion. It has been disabled for now, since it breaks the assertion-free smoketest,
            // and the fix is more difficult than what can be done now.
            // The problem is that at the moment, when you close a SFX-based document via API, it will first
            // fire the notifyClosing event, which will make the OfficeDocumentsManager remove the doc from its list.
            // Then, it will notify an OnTitleChanged, then an OnUnload. Documents closed via call the notifyClosing
            // *after* OnUnload and all other On* events.
            // In agreement with MBA, the implementation for SfxBaseModel::Close should be changed to also send notifyClosing
            // as last event. When this happens, the assertion here must be enabled, again.
        }
    }
}

// lang::XDocumentEventListener (base of document::XDocumentEventListener)

// virtual
void SAL_CALL OfficeDocumentsManager::disposing(
        const lang::EventObject& /*Source*/ )
{
}

// Non-interface.

void OfficeDocumentsManager::buildDocumentsList()
{
    uno::Reference< container::XEnumeration > xEnum
        = m_xDocEvtNotifier->createEnumeration();

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
                    bool found(false);

                    {
                        std::scoped_lock aGuard( m_aMtx );

                        found = std::any_of(m_aDocs.begin(), m_aDocs.end(),
                            [&xModel](const DocumentList::value_type& rEntry) { return rEntry.second.xModel == xModel; });
                    }

                    if (!found)
                    {
                        // new document
                        OUString aDocId = getDocumentId( xModel );
                        OUString aTitle = comphelper::DocumentInfo::getDocumentTitle( xModel );

                        uno::Reference< document::XStorageBasedDocument >
                                xDoc( xModel, uno::UNO_QUERY );
                        OSL_ENSURE( xDoc.is(),
                            "Got no document::XStorageBasedDocument!" );

                        uno::Reference< embed::XStorage > xStorage
                            = xDoc->getDocumentStorage();
                        OSL_ENSURE( xStorage.is(), "Got no document storage!" );

                        {
                            std::scoped_lock aGuard( m_aMtx );
                            m_aDocs[ aDocId ]
                                = StorageInfo( aTitle, xStorage, xModel );
                        }

                        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster(
                            xModel, uno::UNO_QUERY );
                        OSL_ENSURE( xCloseBroadcaster.is(),
                            "buildDocumentsList: got no close broadcaster!" );

                        if ( xCloseBroadcaster.is() )
                            xCloseBroadcaster->addCloseListener(m_xDocCloseListener);
                    }
                }
            }
        }
        catch ( lang::DisposedException const & )
        {
            // Note: Due to race conditions the XEnumeration can
            //       contain docs that have already been closed
        }
        catch ( lang::NotInitializedException const & )
        {
            // Note: Due to race conditions the XEnumeration can
            //       contain docs that are still uninitialized
        }
    }
}

uno::Reference< embed::XStorage >
OfficeDocumentsManager::queryStorage( const OUString & rDocId )
{
    std::scoped_lock aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return uno::Reference< embed::XStorage >();

    return (*it).second.xStorage;
}


OUString OfficeDocumentsManager::queryDocumentId(
    const uno::Reference< frame::XModel > & xModel )
{
    return getDocumentId( xModel );
}


uno::Reference< frame::XModel >
OfficeDocumentsManager::queryDocumentModel( const OUString & rDocId )
{
    std::scoped_lock aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return uno::Reference< frame::XModel >();

    return (*it).second.xModel;
}


uno::Sequence< OUString > OfficeDocumentsManager::queryDocuments()
{
    std::scoped_lock aGuard( m_aMtx );

    return comphelper::mapKeysToSequence( m_aDocs );
}


OUString
OfficeDocumentsManager::queryStorageTitle( const OUString & rDocId )
{
    std::scoped_lock aGuard( m_aMtx );

    DocumentList::const_iterator it = m_aDocs.find( rDocId );
    if ( it == m_aDocs.end() )
        return OUString();

    return (*it).second.aTitle;
}


css::util::DateTime OfficeDocumentsManager::queryStreamDateModified(OUString const & uri) {
    std::scoped_lock g(m_aMtx);
    auto const i1 = m_aDocs.find(Uri(uri).getDocumentId());
    if (i1 != m_aDocs.end()) {
        auto const i2 = i1->second.streamDateModified.find(uri);
        if (i2 != i1->second.streamDateModified.end()) {
            return i2->second;
        }
    }
    return {};
}


void OfficeDocumentsManager::updateStreamDateModified(OUString const & uri) {
    std::scoped_lock g(m_aMtx);
    auto const i = m_aDocs.find(Uri(uri).getDocumentId());
    if (i == m_aDocs.end()) {
        SAL_WARN("ucb.ucp.tdoc", "No document info for <" << uri << ">");
        return;
    }
    i->second.streamDateModified[uri] = DateTime(DateTime::SYSTEM).GetUNODateTime();
}


bool OfficeDocumentsManager::isDocumentPreview(
        const uno::Reference< frame::XModel3 > & xModel )
{
    if ( !xModel.is() )
        return false;

    uno::Sequence<beans::PropertyValue> props = xModel->getArgs2( { u"Preview"_ustr } );
    for (const auto & rProp : props)
        if (rProp.Name == "Preview")
        {
            bool bIsPreview = false;
            rProp.Value >>= bIsPreview;
            return bIsPreview;
        }
    return false;
}


bool OfficeDocumentsManager::isHelpDocument(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !xModel.is() )
        return false;

    OUString sURL( xModel->getURL() );
    return sURL.match( "vnd.sun.star.help://" );
}


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


bool OfficeDocumentsManager::isBasicIDE(
        const uno::Reference< frame::XModel > & xModel )
{
    if ( !m_xModuleMgr.is() )
    {
        std::scoped_lock aGuard( m_aMtx );
        if ( !m_xModuleMgr.is() )
        {
            try
            {
                m_xModuleMgr = frame::ModuleManager::create( m_xContext );
            }
            catch ( uno::Exception const & )
            {
                // handled below.
            }

            OSL_ENSURE( m_xModuleMgr .is(),
                        "Could not instantiate ModuleManager service!" );
        }
    }

    if ( m_xModuleMgr.is() )
    {
        OUString aModule;
        try
        {
            aModule = m_xModuleMgr->identify( xModel );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            TOOLS_WARN_EXCEPTION("ucb.ucp", "");
        }
        catch ( frame::UnknownModuleException const & )
        {
            TOOLS_WARN_EXCEPTION("ucb.ucp", "");
        }

        if ( !aModule.isEmpty() )
        {
            // Filter unwanted items, that are no real documents.
            if ( aModule == "com.sun.star.script.BasicIDE" )
            {
                return true;
            }
        }
    }

    return false;
}


bool OfficeDocumentsManager::isOfficeDocument(
        const uno::Reference< uno::XInterface > & xDoc )
{
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    uno::Reference< document::XStorageBasedDocument >
        xStorageBasedDoc( xModel, uno::UNO_QUERY );
    if ( !xStorageBasedDoc.is() )
        return false;
    uno::Reference< frame::XModel3 > xModel3( xDoc, uno::UNO_QUERY );
    assert(xModel3 && "anything implementing frame:XModel is expected to implement XModel3 as well");

    if ( !isWithoutOrInTopLevelFrame( xModel ) )
        return false;

    if ( isDocumentPreview( xModel3 ) )
        return false;

    if ( isHelpDocument( xModel ) )
        return false;

    if ( isBasicIDE( xModel ) )
        return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
