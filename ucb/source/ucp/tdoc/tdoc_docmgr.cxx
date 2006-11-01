/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_docmgr.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 10:13:14 $
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

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/XEnumerationAccess.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/document/XStorageBasedDocument.hpp"

#include "tdoc_docmgr.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace tdoc_ucp;

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
    OSL_ENSURE( m_aDocs.empty(), "document list not empty!" );
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
        sal_Int64 nId = reinterpret_cast< sal_Int64 >( xDoc.get() );
        aId = rtl::OUString::valueOf( nId );
    }

    OSL_ENSURE( aId.getLength() > 0, "getDocumentId - Empty id!" );
    return aId;
}

//=========================================================================
static rtl::OUString
getDocumentTitle( const uno::Reference< uno::XInterface > & xDoc )
{
    rtl::OUString aTitle;

    uno::Reference< frame::XStorable > xStorable( xDoc, uno::UNO_QUERY );

    OSL_ENSURE( xStorable.is(),
                "getDocumentTitle - Got no frame::XStorable interface!" );

    if ( xStorable.is() )
    {
        // Note: frame::XModel::getURL() is not what I need; URL never gets
        //       updated, for instance after saving a document with different
        //       name.

        rtl::OUString aURL = xStorable->getLocation();

        if ( aURL.getLength() > 0 )
        {
            sal_Int32 nLastSlash = aURL.lastIndexOf( '/' );
            bool bTrailingSlash = false;
            if ( nLastSlash == aURL.getLength() - 1 )
            {
                // ignore trailing slash
                bTrailingSlash = true;
                nLastSlash = aURL.lastIndexOf( '/', nLastSlash );
            }

            if ( nLastSlash == -1 )
            {
                aTitle = aURL; // what else?
            }
            else
            {
                if ( bTrailingSlash )
                    aTitle = aURL.copy( nLastSlash + 1,
                                        aURL.getLength() - nLastSlash - 2 );
                else
                    aTitle = aURL.copy( nLastSlash + 1 );
            }
        }
        else
        {
            uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );

            OSL_ENSURE( xStorable.is(),
                "getDocumentTitle - Got no frame::XModel interface!" );

            if ( xModel.is() )
            {
                // Newly created documents that have not yet been saved
                // have no URL. Try to get Title property from document model,
                // which, btw, must not match the title shown in the Office's
                // title bar. So, it has just limited value. But this is better
                // than nothing.

                uno::Sequence< beans::PropertyValue > aProps
                    = xModel->getArgs();
                for ( sal_Int32 n = 0; n < aProps.getLength(); ++n )
                {
                    if ( aProps[ n ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
                    {
                        aProps[ n ].Value >>= aTitle;
                        break;
                    }
                }
            }
        }
    }

    OSL_ENSURE( aTitle.getLength() > 0, "getDocumentTitle - Empty title!" );
    return aTitle;
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
                OSL_ENSURE( xDoc.is(), "Got no document storage!" );

                rtl:: OUString aDocId = getDocumentId( Event.Source );
                rtl:: OUString aTitle = getDocumentTitle( Event.Source );

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
                    OSL_ENSURE( xDoc.is(), "Got no document storage!" );

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
                    OSL_ENSURE( xDoc.is(), "Got no document storage!" );

                    (*it).second.xStorage = xStorage;

                    // Adjust title.
                    (*it).second.aTitle = getDocumentTitle( Event.Source );
                    break;
                }
                ++it;
            }

            OSL_ENSURE( it != m_aDocs.end(),
                        "OnSaveAsDone event notified for unknown document!" );
        }
    }
    else if ( Event.EventName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "TitleChanged" ) ) )
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
                    rtl:: OUString aTitle = getDocumentTitle( Event.Source );
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
                        rtl::OUString aTitle = getDocumentTitle( xModel );

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
star::uno::Reference< frame::XModel >
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
bool OfficeDocumentsManager::isOfficeDocument(
        const uno::Reference< uno::XInterface > & xDoc )
{
    uno::Reference< document::XStorageBasedDocument >
        xStorageBasedDoc( xDoc, uno::UNO_QUERY );
    if ( !xStorageBasedDoc.is() )
        return false;

    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if ( xModel.is() )
    {
        uno::Reference< frame::XController > xController
            = xModel->getCurrentController();
        if ( xController.is() )
        {
            uno::Reference< frame::XFrame > xFrame
                = xController->getFrame();
            if ( xFrame.is() )
            {
                // isTop returns true for all frames without a parent
                // or for the desktop itself.
                if ( !xFrame->isTop() )
                    return false;
            }
        }
    }

    if ( !m_xModuleMgr.is() )
    {
        osl::MutexGuard aGuard( m_aMtx );
        if ( !m_xModuleMgr.is() )
        {
            try
            {
                m_xModuleMgr
                    = uno::Reference<
                        ::com::sun::star::frame::XModuleManager >(
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
            aModule = m_xModuleMgr->identify( xDoc );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( ::com::sun::star::frame::UnknownModuleException const & )
        {
            OSL_ENSURE( false, "Caught UnknownModuleException!" );
        }

        if ( aModule.getLength() > 0 )
        {
            // Filter unwanted items, that are no real documents.
            if ( aModule.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.star.script.BasicIDE" ) ) )
            {
                // Basic-IDE
                return false;
            }
            /*
            else if ( aModule.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.star.text.WebDocument" ) ) )
            {
                // Office Help (embedded Writer doc)

                @@@ help docs cannot be safely distinguished from normal
                    writer web docs
            }
            */
        }
    }
    return true;
}
