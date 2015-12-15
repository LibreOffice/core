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

#include "core_resource.hxx"
#include "core_resource.hrc"
#include "datasource.hxx"
#include "databasedocument.hxx"
#include "dbastrings.hrc"
#include "documenteventexecutor.hxx"
#include "databasecontext.hxx"
#include "documentcontainer.hxx"
#include "sdbcoretools.hxx"
#include "recovery/dbdocrecovery.hxx"

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/GraphicObjectResolver.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/UIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/DialogProvider.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>

#include <comphelper/documentconstants.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/numberedcollection.hxx>
#include <comphelper/property.hxx>
#include <comphelper/storagehelper.hxx>

#include <connectivity/dbtools.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <framework/titlehelper.hxx>
#include <unotools/saveopt.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <tools/errcode.hxx>

#include <boost/bind.hpp>

#include <functional>
#include <list>

#include <svtools/grfmgr.hxx>
#include <tools/urlobj.hxx>

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
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::provider;
using namespace ::com::sun::star::ui;
using namespace ::cppu;

namespace dbaccess
{

// ViewMonitor

bool ViewMonitor::onControllerConnected( const Reference< XController >& _rxController )
{
    bool bFirstControllerEver = !m_bEverHadController;
    m_bEverHadController = true;

    m_xLastConnectedController = _rxController;
    m_bLastIsFirstEverController = bFirstControllerEver;

    return bFirstControllerEver;
}

bool ViewMonitor::onSetCurrentController( const Reference< XController >& _rxController )
{
    // we interpret this as "loading the document (including UI) is finished",
    // if and only if this is the controller which was last connected, and it was the
    // first controller ever connected
    bool bLoadFinished = ( _rxController == m_xLastConnectedController ) && m_bLastIsFirstEverController;

    // notify the respective events
    if ( bLoadFinished )
        m_rEventNotifier.notifyDocumentEventAsync( m_bIsNewDocument ? "OnNew" : "OnLoad" );

    return bLoadFinished;
}


ODatabaseDocument::ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl )
            :ModelDependentComponent( _pImpl )
            ,ODatabaseDocument_OfficeDocument( getMutex() )
            ,m_aModifyListeners( getMutex() )
            ,m_aCloseListener( getMutex() )
            ,m_aStorageListeners( getMutex() )
            ,m_pEventContainer( new DocumentEvents( *this, getMutex(), _pImpl->getDocumentEvents() ) )
            ,m_pEventExecutor( nullptr )   // initialized below, ref-count-protected
            ,m_aEventNotifier( *this, getMutex() )
            ,m_aViewMonitor( m_aEventNotifier )
            ,m_eInitState( NotInitialized )
            ,m_bClosing( false )
            ,m_bAllowDocumentScripting( false )
            ,m_bHasBeenRecovered( false )
            ,m_bEmbedded(false)
{
    OSL_TRACE( "DD: ctor: %p: %p", this, m_pImpl.get() );

    osl_atomic_increment( &m_refCount );
    {
        impl_reparent_nothrow( m_xForms );
        impl_reparent_nothrow( m_xReports );
        impl_reparent_nothrow( m_pImpl->m_xTableDefinitions );
        impl_reparent_nothrow( m_pImpl->m_xCommandDefinitions );

        m_pEventExecutor = new DocumentEventExecutor( m_pImpl->m_aContext, this );
    }
    osl_atomic_decrement( &m_refCount );

    // if there previously was a document instance for the same Impl which was already initialized,
    // then consider ourself initialized, too.
    // #i94840#
    if ( m_pImpl->hadInitializedDocument() )
    {
        // Note we set our init-state to "Initializing", not "Initialized". We're created from inside the ModelImpl,
        // which is expected to call attachResource in case there was a previous incarnation of the document,
        // so we can properly finish our initialization then.
        impl_setInitializing();

        if ( !m_pImpl->getURL().isEmpty() )
        {
            // if the previous incarnation of the DatabaseDocument already had an URL, then creating this incarnation
            // here is effectively loading the document.
            // #i105505#
            m_aViewMonitor.onLoadedDocument();
        }
    }
}

ODatabaseDocument::~ODatabaseDocument()
{
    OSL_TRACE( "DD: dtor: %p: %p", this, m_pImpl.get() );
    if ( !ODatabaseDocument_OfficeDocument::rBHelper.bInDispose && !ODatabaseDocument_OfficeDocument::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    delete m_pEventContainer, m_pEventContainer = nullptr;
}

Any SAL_CALL ODatabaseDocument::queryInterface( const Type& _rType ) throw (RuntimeException, std::exception)
{
    // strip XEmbeddedScripts and XScriptInvocationContext if we have any form/report
    // which already contains macros. In this case, the database document itself is not
    // allowed to contain macros, too.
    if  (   !m_bAllowDocumentScripting
        &&  (   _rType.equals( cppu::UnoType<XEmbeddedScripts>::get() )
            ||  _rType.equals( cppu::UnoType<XScriptInvocationContext>::get() )
            )
        )
        return Any();

    Any aReturn = ODatabaseDocument_OfficeDocument::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ODatabaseDocument_Title::queryInterface(_rType);
    return aReturn;
}

void SAL_CALL ODatabaseDocument::acquire(  ) throw ()
{
    ODatabaseDocument_OfficeDocument::acquire();
}

void SAL_CALL ODatabaseDocument::release(  ) throw ()
{
    ODatabaseDocument_OfficeDocument::release();
}

Sequence< Type > SAL_CALL ODatabaseDocument::getTypes(  ) throw (RuntimeException, std::exception)
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
                ::std::bind2nd( ::std::equal_to< Type >(), cppu::UnoType<XEmbeddedScripts>::get() )
            ) - pStripTo
        );

        // strip XScriptInvocationContext, and immediately re-assign to aTypes
        aTypes = Sequence< Type >(
            pStripTo,
            ::std::remove_copy_if(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                pStripTo,
                ::std::bind2nd( ::std::equal_to< Type >(), cppu::UnoType<XScriptInvocationContext>::get() )
            ) - pStripTo
        );
    }

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL ODatabaseDocument::getImplementationId(  ) throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// local functions
namespace
{
    Reference< XStatusIndicator > lcl_extractStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments )
    {
        Reference< XStatusIndicator > xStatusIndicator;
        return _rArguments.getOrDefault( "StatusIndicator", xStatusIndicator );
    }

    static void lcl_triggerStatusIndicator_throw( const ::comphelper::NamedValueCollection& _rArguments, DocumentGuard& _rGuard, const bool _bStart )
    {
        Reference< XStatusIndicator > xStatusIndicator( lcl_extractStatusIndicator( _rArguments ) );
        if ( !xStatusIndicator.is() )
            return;

        _rGuard.clear();
        try
        {
            if ( _bStart )
                xStatusIndicator->start( OUString(), (sal_Int32)1000000 );
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

    static void lcl_extractStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments, Sequence< Any >& _rCallArgs )
    {
        Reference< XStatusIndicator > xStatusIndicator( lcl_extractStatusIndicator( _rArguments ) );
        if ( !xStatusIndicator.is() )
            return;

        sal_Int32 nLength = _rCallArgs.getLength();
        _rCallArgs.realloc( nLength + 1 );
        _rCallArgs[ nLength ] <<= xStatusIndicator;
    }

    static void lcl_extractAndStartStatusIndicator( const ::comphelper::NamedValueCollection& _rArguments, Reference< XStatusIndicator >& _rxStatusIndicator,
        Sequence< Any >& _rCallArgs )
    {
        _rxStatusIndicator = lcl_extractStatusIndicator( _rArguments );
        if ( !_rxStatusIndicator.is() )
            return;

        try
        {
            _rxStatusIndicator->start( OUString(), (sal_Int32)1000000 );

            sal_Int32 nLength = _rCallArgs.getLength();
            _rCallArgs.realloc( nLength + 1 );
            _rCallArgs[ nLength ] <<= _rxStatusIndicator;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    static Sequence< PropertyValue > lcl_appendFileNameToDescriptor( const ::comphelper::NamedValueCollection& _rDescriptor, const OUString& _rURL )
    {
        ::comphelper::NamedValueCollection aMutableDescriptor( _rDescriptor );
        if ( !_rURL.isEmpty() )
        {
            aMutableDescriptor.put( "FileName", _rURL );
            aMutableDescriptor.put( "URL", _rURL );
        }
        return aMutableDescriptor.getPropertyValues();
    }
}

static const char sPictures[] = "Pictures";

// base documents seem to have a different behaviour to other documents, the
// root storage contents at least seem to be re-used over different saves, thus if there is a
// top level Picture directory it is never cleared.
// If we delete the 'Pictures' directory then the dialog library storage which does store
// any embed images will not work properly. ( this is due to the fact it will
// try to load the dialog which will try and access the embed images, if those images are not cached in
//  memory it will try to read them from the Picture directory which is now gone, so... we have to use this
// inglorious hack below which basically will
//
// a) create a temp storage
//
// b) introspect any dialogs for any embed graphics and grab the associate URL(s)
//
// c) populate the temp storage with the associated embed images ( will be stored in a 'Pictures' folder )
//
// d) delete the 'Picture' element from the root storage
//
// e) copy the Pictures element of the temp storage to the root storage
//
// this assumes that we don't use the Pictures folder in the root of the base
// document for anything, I believe this is a valid assumption ( as much as
// I could check anyway )

void lcl_uglyHackToStoreDialogeEmbedImages( const Reference< XStorageBasedLibraryContainer >& xDlgCont, const Reference< XStorage >& xStorage, const Reference< XModel >& rxModel, const Reference<XComponentContext >& rxContext ) throw ( RuntimeException )
{
    Sequence< OUString > sLibraries = xDlgCont->getElementNames();
    Reference< XStorage > xTmpPic = xStorage->openStorageElement( "tempPictures", ElementModes::READWRITE  );

    std::vector< OUString > vEmbedImgUrls;
    for ( sal_Int32 i=0; i < sLibraries.getLength(); ++i )
    {
        OUString sLibrary( sLibraries[ i ] );
        xDlgCont->loadLibrary( sLibrary );
        Reference< XNameContainer > xLib;
        xDlgCont->getByName( sLibrary ) >>= xLib;
        if ( xLib.is() )
        {
            Sequence< OUString > sDialogs = xLib->getElementNames();
            sal_Int32 nDialogs( sDialogs.getLength() );
            for ( sal_Int32 j=0; j < nDialogs; ++j )
            {
                Reference < awt::XDialogProvider > xDlgPrv = awt::DialogProvider::createWithModel(rxContext, rxModel);
                OUString sDialogUrl = "vnd.sun.star.script:";
                sDialogUrl = sDialogUrl.concat( sLibraries[ i ] ).concat( "." ).concat (  sDialogs[ j ]  ).concat( "?location=document" );

                Reference< css::awt::XControl > xDialog( xDlgPrv->createDialog( sDialogUrl ), UNO_QUERY );
                Reference< XInterface > xModel( xDialog->getModel() );
                GraphicObject::InspectForGraphicObjectImageURL( xModel, vEmbedImgUrls );
            }
        }
    }
    // if we have any image urls, make sure we copy the associated images into tempPictures
    if ( !vEmbedImgUrls.empty() )
    {
        // Export the images to the storage
        Reference< XGraphicObjectResolver > xGraphicResolver = GraphicObjectResolver::createWithStorage(rxContext, xTmpPic);
        std::vector< OUString >::iterator it = vEmbedImgUrls.begin();
        std::vector< OUString >::iterator it_end = vEmbedImgUrls.end();
        if ( xGraphicResolver.is() )
        {
            for ( sal_Int32 count = 0; it != it_end; ++it, ++count )
                xGraphicResolver->resolveGraphicObjectURL( *it );
        }

        // delete old 'Pictures' storage and copy the contents of tempPictures into xStorage
        xStorage->removeElement( sPictures );
        xTmpPic->copyElementTo( sPictures, xStorage, sPictures );
    }
    else
    {
        // clean up an existing Pictures dir
        if ( xStorage->isStorageElement( sPictures ) )
            xStorage->removeElement( sPictures );
    }
}

void ODatabaseDocument::impl_setInitialized()
{
    m_eInitState = Initialized;

    // start event notifications
    m_aEventNotifier.onDocumentInitialized();
}

void ODatabaseDocument::impl_reset_nothrow()
{
    try
    {
        m_pImpl->clearConnections();
        m_pImpl->disposeStorages();
        m_pImpl->resetRootStorage();

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
    m_pImpl->m_bDocumentReadOnly = false;
}

namespace
{
    /** property map for import/export info set */
    comphelper::PropertyMapEntry const aExportInfoMap[] =
     {
        { OUString("BaseURI"), 0, ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamName"), 0, ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("UsePrettyPrinting"), 0, ::cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
        {OUString("TargetStorage"), 0, cppu::UnoType<embed::XStorage>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
        {OUString("StreamRelPath"), 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
     };
}

void ODatabaseDocument::impl_import_nolck_throw( const Reference< XComponentContext >& _rContext, const Reference< XInterface >& _rxTargetComponent,
                                                 const ::comphelper::NamedValueCollection& _rResource )
{
    Sequence< Any > aFilterCreationArgs;
    Reference< XStatusIndicator > xStatusIndicator;
    lcl_extractAndStartStatusIndicator( _rResource, xStatusIndicator, aFilterCreationArgs );

     uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );
    OUString sBaseURI = _rResource.getOrDefault("BaseURI", OUString());
    if (sBaseURI.isEmpty())
        sBaseURI = _rResource.getOrDefault("URL",OUString());
    assert(!sBaseURI.isEmpty()); // needed for relative URLs
    xInfoSet->setPropertyValue("BaseURI", uno::makeAny(sBaseURI));
    xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("content.xml")));

    const sal_Int32 nCount = aFilterCreationArgs.getLength();
    aFilterCreationArgs.realloc(nCount + 1);
    aFilterCreationArgs[nCount] <<= xInfoSet;

    Reference< XImporter > xImporter(
        _rContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.comp.sdb.DBFilter", aFilterCreationArgs, _rContext),
        UNO_QUERY_THROW );

    Reference< XComponent > xComponent( _rxTargetComponent, UNO_QUERY_THROW );
    xImporter->setTargetDocument( xComponent );

    Reference< XFilter > xFilter( xImporter, UNO_QUERY_THROW );
    Sequence< PropertyValue > aFilterArgs( ODatabaseModelImpl::stripLoadArguments( _rResource ).getPropertyValues() );
    xFilter->filter( aFilterArgs );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

void SAL_CALL ODatabaseDocument::initNew(  ) throw (DoubleInitializationException, IOException, Exception, RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::InitMethod );

    impl_reset_nothrow();

    impl_setInitializing();

    // create a temporary storage
    Reference< XStorage > xTempStor( ::comphelper::OStorageHelper::GetTemporaryStorage( m_pImpl->m_aContext ) );

    // store therein
    impl_storeToStorage_throw( xTempStor, Sequence< PropertyValue >(), aGuard );

    // let the impl know we're now based on this storage
    m_pImpl->switchToStorage( xTempStor );

    // for the newly created document, allow document-wide scripting
    m_bAllowDocumentScripting = true;

    impl_setInitialized();

    m_aEventNotifier.notifyDocumentEventAsync( "OnTitleChanged" );

    impl_setModified_nothrow( false, aGuard );
    // <- SYNCHRONIZED

    m_aEventNotifier.notifyDocumentEvent( "OnCreate" );

    impl_notifyStorageChange_nolck_nothrow( xTempStor );
}

void SAL_CALL ODatabaseDocument::load( const Sequence< PropertyValue >& _Arguments ) throw (DoubleInitializationException, IOException, Exception, RuntimeException, std::exception)
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
    // however, in case of embedding, XModel::attachResource is already called.
    if (m_bEmbedded)
        impl_setInitialized();

    impl_setModified_nothrow( false, aGuard );
    // <- SYNCHRONIZED
}

namespace
{
    bool lcl_hasAnyModifiedSubComponent_throw( const Reference< XController >& i_rController )
    {
        Reference< css::sdb::application::XDatabaseDocumentUI > xDatabaseUI( i_rController, UNO_QUERY_THROW );

        Sequence< Reference< XComponent > > aComponents( xDatabaseUI->getSubComponents() );
        const Reference< XComponent >* component = aComponents.getConstArray();
        const Reference< XComponent >* componentsEnd = aComponents.getConstArray() + aComponents.getLength();

        bool isAnyModified = false;
        for ( ; component != componentsEnd; ++component )
        {
            Reference< XModifiable > xModify( *component, UNO_QUERY );
            if ( xModify.is() )
            {
                isAnyModified = xModify->isModified();
                continue;
            }

            // TODO: clarify: anything else to care for? Both the sub componbents with and without model
            // should support the XModifiable interface, so I think nothing more is needed here.
            OSL_FAIL( "lcl_hasAnyModifiedSubComponent_throw: anything left to do here?" );
        }

        return isAnyModified;
    }
}

sal_Bool SAL_CALL ODatabaseDocument::wasModifiedSinceLastSave() throw ( RuntimeException, std::exception )
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    // The implementation here is somewhat sloppy, in that it returns whether *any* part of the whole
    // database document, including opened sub components, is modified. This is more than what is requested:
    // We need to return <TRUE/> if the doc itself, or any of the opened sub components, has been modified
    // since the last call to any of the save* methods, or since the document has been loaded/created.
    // However, the API definition explicitly allows to be that sloppy ...

    if ( isModified() )
        return sal_True;

    // auto recovery is an "UI feature", it is to restore the UI the user knows. Thus,
    // we ask our connected controllers, not simply our existing form/report definitions.
    // (There is some information which even cannot be obtained without asking the controller.
    // For instance, newly created, but not yet saved, forms/reports are accessible via the
    // controller only, but not via the model.)

    try
    {
        for (   Controllers::const_iterator ctrl = m_aControllers.begin();
                ctrl != m_aControllers.end();
                ++ctrl
            )
        {
            if ( lcl_hasAnyModifiedSubComponent_throw( *ctrl ) )
                return sal_True;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sal_False;
}

void SAL_CALL ODatabaseDocument::storeToRecoveryFile( const OUString& i_TargetLocation, const Sequence< PropertyValue >& i_MediaDescriptor ) throw ( RuntimeException, IOException, WrappedTargetException, std::exception )
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    ModifyLock aLock( *this );

    try
    {
        // create a storage for the target location
        Reference< XStorage > xTargetStorage( impl_createStorageFor_throw( i_TargetLocation ) );

        // first store the document as a whole into this storage
        impl_storeToStorage_throw( xTargetStorage, i_MediaDescriptor, aGuard );

        // save the sub components which need saving
        DatabaseDocumentRecovery aDocRecovery( m_pImpl->m_aContext);
        aDocRecovery.saveModifiedSubComponents( xTargetStorage, m_aControllers );

        // commit the root storage
        tools::stor::commitStorageIfWriteable( xTargetStorage );
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();
        if  (   aError.isExtractableTo( ::cppu::UnoType< IOException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< WrappedTargetException >::get() )
            )
        {
            // allowed to leave
            throw;
        }

        throw WrappedTargetException( OUString(), *this, aError );
    }
}

void SAL_CALL ODatabaseDocument::recoverFromFile( const OUString& i_SourceLocation, const OUString& i_SalvagedFile, const Sequence< PropertyValue >& i_MediaDescriptor ) throw ( RuntimeException, IOException, WrappedTargetException, std::exception )
{
    try
    {
        DocumentGuard aGuard( *this, DocumentGuard::InitMethod );

        if ( i_SourceLocation.isEmpty() )
            throw IllegalArgumentException( OUString(), *this, 1 );


        // load the document itself, by simply delegating to our "load" method

        // our load implementation expects the SalvagedFile and URL to be in the media descriptor
        ::comphelper::NamedValueCollection aMediaDescriptor( i_MediaDescriptor );
        aMediaDescriptor.put( "SalvagedFile", i_SalvagedFile );
        aMediaDescriptor.put( "URL", i_SourceLocation );

        aGuard.clear(); // (load has an own guarding scheme)
        load( aMediaDescriptor.getPropertyValues() );

        // Without a controller, we are unable to recover the sub components, as they're always tied to a controller.
        // So, everything else is done when the first controller is connected.
        m_bHasBeenRecovered = true;

        // tell the impl that we've been loaded from the given location
        m_pImpl->setDocFileLocation( i_SourceLocation );

        // by definition (of XDocumentRecovery), we're responsible for delivering a fully-initialized document,
        // which includes an attachResource call.
        const OUString sLogicalDocumentURL( i_SalvagedFile.isEmpty() ?  i_SourceLocation : i_SalvagedFile  );
        impl_attachResource( sLogicalDocumentURL, aMediaDescriptor.getPropertyValues(), aGuard );
        // <- SYNCHRONIZED
    }
    catch( const IOException& )
    {
        throw;
    }
    catch( const RuntimeException& )
    {
        throw;
    }
    catch( const WrappedTargetException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();
        throw WrappedTargetException( OUString(), *this, aError );
    }
}

// XModel
sal_Bool SAL_CALL ODatabaseDocument::attachResource( const OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException, std::exception)
{
    if (_rURL.isEmpty() && _rArguments.getLength() == 1 && _rArguments[0].Name == "SetEmbedded")
    {
        m_bEmbedded = true;
        return true;
    }

    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    bool bRet = false;
    try
    {
        bRet = impl_attachResource( _rURL, _rArguments, aGuard );
    }
    catch( const RuntimeException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();
        throw WrappedTargetRuntimeException( OUString(), *this, aError );
    }
    return bRet;
}

bool ODatabaseDocument::impl_attachResource( const OUString& i_rLogicalDocumentURL,
            const Sequence< PropertyValue >& i_rMediaDescriptor, DocumentGuard& _rDocGuard )
{
    if  (   ( i_rLogicalDocumentURL == getURL() )
        &&  ( i_rMediaDescriptor.getLength() == 1 )
        &&  ( i_rMediaDescriptor[0].Name == "BreakMacroSignature" )
        )
    {
        // this is a BAD hack of the Basic importer code ... there should be a dedicated API for this,
        // not this bad mis-using of existing interfaces
        return false;
            // (we do not support macro signatures, so we can ignore this call)
    }

    // if no URL has been provided, the caller was lazy enough to not call our getURL - which is not allowed anymore,
    // now since getURL and getLocation both return the same, so calling one of those should be simple.
    OUString sDocumentURL( i_rLogicalDocumentURL );
    OSL_ENSURE( !sDocumentURL.isEmpty(), "ODatabaseDocument::impl_attachResource: invalid URL!" );
    if ( sDocumentURL.isEmpty() )
        sDocumentURL = getURL();

    m_pImpl->setResource( sDocumentURL, i_rMediaDescriptor );

    if ( impl_isInitializing() )
    {   // this means we've just been loaded, and this is the attachResource call which follows
        // the load call.
        impl_setInitialized();

        // determine whether the document as a whole, or sub documents, have macros. Especially the latter
        // controls the availability of our XEmbeddedScripts and XScriptInvocationContext interfaces, and we
        // should know this before anybody actually uses the object.
        m_bAllowDocumentScripting = ( m_pImpl->determineEmbeddedMacros() != ODatabaseModelImpl::eSubDocumentMacros );

        _rDocGuard.clear();
        // <- SYNCHRONIZED
        m_aEventNotifier.notifyDocumentEvent( "OnLoadFinished" );
    }

    return true;
}

OUString SAL_CALL ODatabaseDocument::getURL(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getURL();
}

Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getArgs(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getMediaDescriptor().getPropertyValues();
}

void SAL_CALL ODatabaseDocument::connectController( const Reference< XController >& _xController ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

#if OSL_DEBUG_LEVEL > 0
    for (   Controllers::const_iterator controller = m_aControllers.begin();
            controller != m_aControllers.end();
            ++controller
        )
    {
        OSL_ENSURE( *controller != _xController, "ODatabaseDocument::connectController: this controller is already connected!" );
    }
#endif

    m_aControllers.push_back( _xController );

    m_aEventNotifier.notifyDocumentEventAsync( "OnViewCreated", Reference< XController2 >( _xController, UNO_QUERY ) );

    bool bFirstControllerEver = m_aViewMonitor.onControllerConnected( _xController );
    if ( !bFirstControllerEver )
        return;

    // check/adjust our macro mode.
    m_pImpl->checkMacrosOnLoading();
}

void SAL_CALL ODatabaseDocument::disconnectController( const Reference< XController >& _xController ) throw (RuntimeException, std::exception)
{
    bool bNotifyViewClosed = false;
    bool bLastControllerGone = false;
    bool bIsClosing = false;

    // SYNCHRONIZED ->
    {
        DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

        Controllers::iterator pos = ::std::find( m_aControllers.begin(), m_aControllers.end(), _xController );
        OSL_ENSURE( pos != m_aControllers.end(), "ODatabaseDocument::disconnectController: don't know this controller!" );
        if ( pos != m_aControllers.end() )
        {
            m_aControllers.erase( pos );
            bNotifyViewClosed = true;
        }

        if ( m_xCurrentController == _xController )
            m_xCurrentController = nullptr;

        bLastControllerGone = m_aControllers.empty();
        bIsClosing = m_bClosing;
    }
    // <- SYNCHRONIZED

    if ( bNotifyViewClosed )
        m_aEventNotifier.notifyDocumentEvent( "OnViewClosed", Reference< XController2 >( _xController, UNO_QUERY ) );

    if ( bLastControllerGone && !bIsClosing )
    {
        // if this was the last view, close the document as a whole
        // #i51157#
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

void SAL_CALL ODatabaseDocument::lockControllers(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    ++m_pImpl->m_nControllerLockCount;
}

void SAL_CALL ODatabaseDocument::unlockControllers(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    --m_pImpl->m_nControllerLockCount;
}

sal_Bool SAL_CALL ODatabaseDocument::hasControllersLocked(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    return m_pImpl->m_nControllerLockCount != 0;
}

Reference< XController > SAL_CALL ODatabaseDocument::getCurrentController() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    return m_xCurrentController.is() ? m_xCurrentController : ( m_aControllers.empty() ? Reference< XController >() : *m_aControllers.begin() );
}

void SAL_CALL ODatabaseDocument::setCurrentController( const Reference< XController >& _xController ) throw (NoSuchElementException, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    m_xCurrentController = _xController;

    if ( !m_aViewMonitor.onSetCurrentController( _xController ) )
        return;

    // check if there are sub components to recover from our document storage
    bool bAttemptRecovery = m_bHasBeenRecovered;
    if ( !bAttemptRecovery && m_pImpl->getMediaDescriptor().has( "ForceRecovery" ) )
        // do not use getOrDefault, it will throw for invalid types, which is not desired here
        m_pImpl->getMediaDescriptor().get( "ForceRecovery" ) >>= bAttemptRecovery;

    if ( !bAttemptRecovery )
        return;

    try
    {
        DatabaseDocumentRecovery aDocRecovery( m_pImpl->m_aContext );
        aDocRecovery.recoverSubDocuments( m_pImpl->getRootStorage(), _xController );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

Reference< XInterface > SAL_CALL ODatabaseDocument::getCurrentSelection(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    Reference< XInterface > xRet;
    Reference< XSelectionSupplier >  xDocView( getCurrentController(), UNO_QUERY );
    if ( xDocView.is() )
        xRet.set(xDocView->getSelection(),UNO_QUERY);

    return xRet;
}

// XStorable
sal_Bool SAL_CALL ODatabaseDocument::hasLocation(  ) throw (RuntimeException, std::exception)
{
    return !getLocation().isEmpty();
}

OUString SAL_CALL ODatabaseDocument::getLocation(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getURL();
        // both XStorable::getLocation and XModel::getURL have to return the URL of the document, *not*
        // the location of the file which the docunment was possibly recovered from (which would be getDocFileLocation)
}

sal_Bool SAL_CALL ODatabaseDocument::isReadonly(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->m_bDocumentReadOnly;
}

void SAL_CALL ODatabaseDocument::store(  ) throw (IOException, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    OUString sDocumentURL( m_pImpl->getURL() );
    if ( !sDocumentURL.isEmpty() )
    {
        if ( m_pImpl->getDocFileLocation() == m_pImpl->getURL() )
            if ( m_pImpl->m_bDocumentReadOnly )
                throw IOException();

        impl_storeAs_throw( m_pImpl->getURL(), m_pImpl->getMediaDescriptor(), SAVE, aGuard );
        return;
    }

    // if we have no URL, but did survive the DocumentGuard above, then we've been inited via XLoadable::initNew,
    // i.e. we're based on a temporary storage
    OSL_ENSURE( m_pImpl->getDocFileLocation().isEmpty(), "ODatabaseDocument::store: unexpected URL inconsistency!" );

    try
    {
        impl_storeToStorage_throw( m_pImpl->getRootStorage(), m_pImpl->getMediaDescriptor().getPropertyValues(), aGuard );
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();
        if  (   aError.isExtractableTo( ::cppu::UnoType< IOException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() )
            )
        {
            // allowed to leave
            throw;
        }
        impl_throwIOExceptionCausedBySave_throw( aError, OUString() );
    }
}

void ODatabaseDocument::impl_throwIOExceptionCausedBySave_throw( const Any& i_rError, const OUString& i_rTargetURL ) const
{
    OUString sErrorMessage = extractExceptionMessage( m_pImpl->m_aContext, i_rError );
    sErrorMessage = ResourceManager::loadString(
        RID_STR_ERROR_WHILE_SAVING,
        "$location$", i_rTargetURL,
        "$message$", sErrorMessage
    );
    throw IOException( sErrorMessage, *const_cast< ODatabaseDocument* >( this ) );
}

void ODatabaseDocument::impl_storeAs_throw( const OUString& _rURL, const ::comphelper::NamedValueCollection& _rArguments,
    const StoreType _eType, DocumentGuard& _rGuard ) throw (IOException, RuntimeException, std::exception)
{
    OSL_PRECOND( ( _eType == SAVE ) || ( _eType == SAVE_AS ),
        "ODatabaseDocument::impl_storeAs_throw: you introduced a new type which cannot be handled here!" );

    // if we're in the process of initializing the document (which effectively means it is an implicit
    // initialization triggered in storeAsURL), the we do not notify events, since to an observer, the SaveAs
    // should not be noticeable
    bool bIsInitializationProcess = impl_isInitializing();

    if ( !bIsInitializationProcess )
    {
        _rGuard.clear();
        m_aEventNotifier.notifyDocumentEvent( _eType == SAVE ? "OnSave" : "OnSaveAs", nullptr, makeAny( _rURL ) );
        _rGuard.reset();
    }

    Reference< XStorage > xNewRootStorage;
        // will be non-NULL if our storage changed

    try
    {
        ModifyLock aLock( *this );
            // ignore all changes of our "modified" state during storing

        bool bLocationChanged = ( _rURL != m_pImpl->getDocFileLocation() );
        if ( bLocationChanged )
        {
            // create storage for target URL
            uno::Reference<embed::XStorage> xTargetStorage;
            _rArguments.get("TargetStorage") >>= xTargetStorage;
            if (!xTargetStorage.is())
                xTargetStorage = impl_createStorageFor_throw(_rURL);

            // In case we got a StreamRelPath, then xTargetStorage should reference that sub-storage.
            OUString sStreamRelPath = _rArguments.getOrDefault("StreamRelPath", OUString());
            if (!sStreamRelPath.isEmpty())
                xTargetStorage = xTargetStorage->openStorageElement(sStreamRelPath, embed::ElementModes::READWRITE);

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

            // each and every document definition obtained via m_xForms and m_xReports depends
            // on the sub storages which we just disposed. So, dispose the forms/reports collections, too.
            // This ensures that they're re-created when needed.
            clearObjectContainer( m_xForms );
            clearObjectContainer( m_xReports );

            xNewRootStorage = m_pImpl->switchToStorage( xTargetStorage );

            m_pImpl->m_bDocumentReadOnly = false;
        }

        // store to current storage
        Reference< XStorage > xCurrentStorage( m_pImpl->getOrCreateRootStorage(), UNO_QUERY_THROW );
        Sequence< PropertyValue > aMediaDescriptor( lcl_appendFileNameToDescriptor( _rArguments, _rURL ) );
        impl_storeToStorage_throw( xCurrentStorage, aMediaDescriptor, _rGuard );

        // success - tell our impl
        m_pImpl->setDocFileLocation( _rURL );
        m_pImpl->setResource( _rURL, aMediaDescriptor );

        // if we are in an initialization process, then this is finished, now that we stored the document
        if ( bIsInitializationProcess )
            impl_setInitialized();
    }
    catch( const IOException& )
    {
        if ( !bIsInitializationProcess )
            m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveFailed" : "OnSaveAsFailed", nullptr, makeAny( _rURL ) );
        throw;
    }
    catch( const RuntimeException& )
    {
        if ( !bIsInitializationProcess )
            m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveFailed" : "OnSaveAsFailed", nullptr, makeAny( _rURL ) );
        throw;
    }
    catch( const Exception& )
    {
        Any aError = ::cppu::getCaughtException();

        // notify the failure
        if ( !bIsInitializationProcess )
            m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveFailed" : "OnSaveAsFailed", nullptr, makeAny( _rURL ) );

        impl_throwIOExceptionCausedBySave_throw( aError, _rURL );
    }

    // notify the document event
    if ( !bIsInitializationProcess )
        m_aEventNotifier.notifyDocumentEventAsync( _eType == SAVE ? "OnSaveDone" : "OnSaveAsDone", nullptr, makeAny( _rURL ) );

    // reset our "modified" flag, and clear the guard
    impl_setModified_nothrow( false, _rGuard );
    // <- SYNCHRONIZED

    // notify storage listeners
    if ( xNewRootStorage.is() )
        impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

Reference< XStorage > ODatabaseDocument::impl_createStorageFor_throw( const OUString& _rURL ) const
{
    Reference< ucb::XSimpleFileAccess3 > xTempAccess(ucb::SimpleFileAccess::create(m_pImpl->m_aContext));
    Reference< io::XStream > xStream = xTempAccess->openFileReadWrite( _rURL );
    Reference< io::XTruncate > xTruncate(xStream,UNO_QUERY);
    if ( xTruncate.is() )
    {
        xTruncate->truncate();
    }
    Sequence<Any> aParam(2);
    aParam[0] <<= xStream;
    aParam[1] <<= ElementModes::READWRITE | ElementModes::TRUNCATE;

    Reference< XSingleServiceFactory > xStorageFactory( m_pImpl->createStorageFactory(), UNO_SET_THROW );
    return Reference< XStorage >( xStorageFactory->createInstanceWithArguments( aParam ), UNO_QUERY_THROW );
}

void SAL_CALL ODatabaseDocument::storeAsURL( const OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );

    // Normally, a document initialization is done via XLoadable::load or XLoadable::initNew. For convenience
    // reasons, and to not break existing API clients, it's allowed to call storeAsURL without having initialized
    // the document, in which case the initialization will be done implicitly.
    bool bImplicitInitialization = !impl_isInitialized();
    // implicit initialization while another initialization is just running is not possible
    if ( bImplicitInitialization && impl_isInitializing() )
        throw RuntimeException();

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

void ODatabaseDocument::impl_storeToStorage_throw( const Reference< XStorage >& _rxTargetStorage, const Sequence< PropertyValue >& _rMediaDescriptor,
                                                   DocumentGuard& _rDocGuard ) const
{
    if ( !_rxTargetStorage.is() )
        throw IllegalArgumentException( OUString(), *const_cast< ODatabaseDocument* >( this ), 1 );

    if ( !m_pImpl.is() )
        throw DisposedException( OUString(), *const_cast< ODatabaseDocument* >( this ) );

    try
    {
        // commit everything
        m_pImpl->commitEmbeddedStorage();
        m_pImpl->commitStorages();

        // copy own storage to target storage
        if ( impl_isInitialized() )
        {
            Reference< XStorage > xCurrentStorage = m_pImpl->getOrCreateRootStorage();
            // Root storage may be empty in case of embedding.
            if ( xCurrentStorage.is() && xCurrentStorage != _rxTargetStorage )
                xCurrentStorage->copyToStorage( _rxTargetStorage );
        }

        // write into target storage
        ::comphelper::NamedValueCollection aWriteArgs( _rMediaDescriptor );
        lcl_triggerStatusIndicator_throw( aWriteArgs, _rDocGuard, true );
        impl_writeStorage_throw( _rxTargetStorage, aWriteArgs );
        lcl_triggerStatusIndicator_throw( aWriteArgs, _rDocGuard, false );

        // commit target storage
        OSL_VERIFY( tools::stor::commitStorageIfWriteable( _rxTargetStorage ) );
    }
    catch( const IOException& ) { throw; }
    catch( const RuntimeException& ) { throw; }
    catch ( const Exception& e )
    {
        throw IOException( e.Message, *const_cast< ODatabaseDocument* >( this ) );
    }
}

void SAL_CALL ODatabaseDocument::storeToURL( const OUString& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (IOException, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    ModifyLock aLock( *this );

    {
        aGuard.clear();
        m_aEventNotifier.notifyDocumentEvent( "OnSaveTo", nullptr, makeAny( _rURL ) );
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
        m_aEventNotifier.notifyDocumentEventAsync( "OnSaveToFailed", nullptr, aError );

        if  (   aError.isExtractableTo( ::cppu::UnoType< IOException >::get() )
            ||  aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() )
            )
        {
            // allowed to leave
            throw;
        }

        impl_throwIOExceptionCausedBySave_throw( aError, _rURL );
    }

    m_aEventNotifier.notifyDocumentEventAsync( "OnSaveToDone", nullptr, makeAny( _rURL ) );
}

// XModifyBroadcaster
void SAL_CALL ODatabaseDocument::addModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aModifyListeners.addInterface(_xListener);
}

void SAL_CALL ODatabaseDocument::removeModifyListener( const Reference< XModifyListener >& _xListener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aModifyListeners.removeInterface(_xListener);
}

// XModifiable
sal_Bool SAL_CALL ODatabaseDocument::isModified(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    return m_pImpl->m_bModified;
}

void SAL_CALL ODatabaseDocument::setModified( sal_Bool _bModified ) throw (PropertyVetoException, RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    if ( impl_isInitialized() )
        impl_setModified_nothrow( _bModified, aGuard );
    // it's allowed to call setModified without the document being initialized already. In this case,
    // we simply ignore the call - when the initialization is finished, the respective code will set
    // a proper "modified" flag
}

void ODatabaseDocument::impl_setModified_nothrow( bool _bModified, DocumentGuard& _rGuard )
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

// css::document::XEventBroadcaster
void SAL_CALL ODatabaseDocument::addEventListener(const uno::Reference< document::XEventListener >& _Listener ) throw (uno::RuntimeException, std::exception)
{
    m_aEventNotifier.addLegacyEventListener( _Listener );
}

void SAL_CALL ODatabaseDocument::removeEventListener( const uno::Reference< document::XEventListener >& _Listener ) throw (uno::RuntimeException, std::exception)
{
    m_aEventNotifier.removeLegacyEventListener( _Listener );
}

void SAL_CALL ODatabaseDocument::addDocumentEventListener( const Reference< XDocumentEventListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aEventNotifier.addDocumentEventListener( _Listener );
}

void SAL_CALL ODatabaseDocument::removeDocumentEventListener( const Reference< XDocumentEventListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_aEventNotifier.removeDocumentEventListener( _Listener );
}

void SAL_CALL ODatabaseDocument::notifyDocumentEvent( const OUString& _EventName, const Reference< XController2 >& _ViewController, const Any& _Supplement ) throw (IllegalArgumentException, NoSupportException, RuntimeException, std::exception)
{
    if ( _EventName.isEmpty() )
        throw IllegalArgumentException( OUString(), *this, 1 );

    // SYNCHRONIZED ->
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    if ( !DocumentEvents::needsSynchronousNotification( _EventName ) )
    {
        m_aEventNotifier.notifyDocumentEventAsync( _EventName, _ViewController, _Supplement );
        return;
    }
    aGuard.clear();
    // <- SYNCHRONIZED

    m_aEventNotifier.notifyDocumentEvent( _EventName, _ViewController, _Supplement );
}

Sequence< PropertyValue > SAL_CALL ODatabaseDocument::getPrinter(  ) throw (RuntimeException, std::exception)
{
    OSL_FAIL( "ODatabaseDocument::getPrinter: not supported!" );
    return Sequence< PropertyValue >();
}

void SAL_CALL ODatabaseDocument::setPrinter( const Sequence< PropertyValue >& /*aPrinter*/ ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    OSL_FAIL( "ODatabaseDocument::setPrinter: not supported!" );
}

void SAL_CALL ODatabaseDocument::print( const Sequence< PropertyValue >& /*xOptions*/ ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    OSL_FAIL( "ODatabaseDocument::print: not supported!" );
}

void ODatabaseDocument::impl_reparent_nothrow( const WeakReference< XNameAccess >& _rxContainer )
{
    Reference< XChild > xChild( _rxContainer.get(), UNO_QUERY );
    if  ( xChild.is() )
        xChild->setParent( *this );
}

void ODatabaseDocument::clearObjectContainer( WeakReference< XNameAccess >& _rxContainer)
{
    Reference< XNameAccess > xContainer = _rxContainer;
    ::comphelper::disposeComponent( xContainer );

    Reference< XChild > xChild( _rxContainer.get(),UNO_QUERY );
    if ( xChild.is() )
        xChild->setParent( nullptr );
    _rxContainer.clear();
}

Reference< XNameAccess > ODatabaseDocument::impl_getDocumentContainer_throw( ODatabaseModelImpl::ObjectType _eType )
{
    if ( ( _eType != ODatabaseModelImpl::E_FORM ) && ( _eType != ODatabaseModelImpl::E_REPORT ) )
        throw IllegalArgumentException();

    bool bFormsContainer = _eType == ODatabaseModelImpl::E_FORM;

    WeakReference< XNameAccess >& rContainerRef( bFormsContainer ? m_xForms : m_xReports );
    Reference< XNameAccess > xContainer = rContainerRef;
    if ( !xContainer.is() )
    {
        Any aValue;
        css::uno::Reference< css::uno::XInterface > xMy(*this);
        if ( dbtools::getDataSourceSetting(xMy,bFormsContainer ? "Forms" : "Reports",aValue) )
        {
            OUString sSupportService;
            aValue >>= sSupportService;
            if ( !sSupportService.isEmpty() )
            {
                Sequence<Any> aArgs(1);
                aArgs[0] <<= NamedValue("DatabaseDocument",makeAny(xMy));
                xContainer.set(
                       m_pImpl->m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext(sSupportService, aArgs, m_pImpl->m_aContext),
                       UNO_QUERY);
                rContainerRef = xContainer;
            }
        }
        if ( !xContainer.is() )
        {
            TContentPtr& rContainerData( m_pImpl->getObjectContainer( _eType ) );
            rContainerRef = xContainer = new ODocumentContainer( m_pImpl->m_aContext, *this, rContainerData, bFormsContainer );
        }
        impl_reparent_nothrow( xContainer );
    }
    return xContainer;
}

void ODatabaseDocument::impl_closeControllerFrames_nolck_throw( bool _bDeliverOwnership )
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

void ODatabaseDocument::impl_disposeControllerFrames_nothrow()
{
    Controllers aCopy;
    aCopy.swap( m_aControllers );   // ensure m_aControllers is empty afterwards
    for( const auto& rController : aCopy )
    {
        try
        {
            if( rController.is() )
            {
                Reference< XFrame > xFrame( rController->getFrame() );
                ::comphelper::disposeComponent( xFrame );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void SAL_CALL ODatabaseDocument::close( sal_Bool _bDeliverOwnership ) throw (CloseVetoException, RuntimeException, std::exception)
{
    // nearly everything below can/must be done without our mutex locked, the below is just for
    // the checks for being disposed and the like
    // SYNCHRONIZED ->
    {
        DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
        assert (!m_bClosing);
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

void SAL_CALL ODatabaseDocument::addCloseListener( const Reference< css::util::XCloseListener >& Listener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aCloseListener.addInterface(Listener);
}

void SAL_CALL ODatabaseDocument::removeCloseListener( const Reference< css::util::XCloseListener >& Listener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aCloseListener.removeInterface(Listener);
}

Reference< XNameAccess > SAL_CALL ODatabaseDocument::getFormDocuments(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getDocumentContainer_throw( ODatabaseModelImpl::E_FORM );
}

Reference< XNameAccess > SAL_CALL ODatabaseDocument::getReportDocuments(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getDocumentContainer_throw( ODatabaseModelImpl::E_REPORT );
}

void ODatabaseDocument::WriteThroughComponent( const Reference< XComponent >& xComponent, const sal_Char* pStreamName,
    const sal_Char* pServiceName, const Sequence< Any >& _rArguments, const Sequence< PropertyValue >& rMediaDesc,
    const Reference<XStorage>& _xStorageToSaveTo ) const
{
    OSL_ENSURE( pStreamName, "Need stream name!" );
    OSL_ENSURE( pServiceName, "Need service name!" );

    // open stream
    OUString sStreamName = OUString::createFromAscii( pStreamName );
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
    xStreamProp->setPropertyValue( INFO_MEDIATYPE, makeAny( OUString( "text/xml" ) ) );
    xStreamProp->setPropertyValue( "Compressed", makeAny( true ) );

    // write the stuff
    WriteThroughComponent( xOutputStream, xComponent, pServiceName, _rArguments, rMediaDesc );
}

void ODatabaseDocument::WriteThroughComponent( const Reference< XOutputStream >& xOutputStream,
    const Reference< XComponent >& xComponent, const sal_Char* pServiceName, const Sequence< Any >& _rArguments,
    const Sequence< PropertyValue >& rMediaDesc ) const
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( nullptr != pServiceName, "Need component name!" );

    // get component
    Reference< XWriter > xSaxWriter = xml::sax::Writer::create( m_pImpl->m_aContext );

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference< XDocumentHandler > xDocHandler( xSaxWriter,UNO_QUERY);
    Sequence<Any> aArgs( 1 + _rArguments.getLength() );
    aArgs[0] <<= xDocHandler;
    for ( sal_Int32 i = 0; i < _rArguments.getLength(); ++i )
        aArgs[ i+1 ] = _rArguments[i];

    // get filter component
    Reference< XExporter > xExporter( m_pImpl->m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext(OUString::createFromAscii(pServiceName), aArgs, m_pImpl->m_aContext), UNO_QUERY_THROW );

    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter
    Reference< XFilter > xFilter( xExporter, UNO_QUERY_THROW );
    xFilter->filter( rMediaDesc );
}

void ODatabaseDocument::impl_writeStorage_throw( const Reference< XStorage >& _rxTargetStorage, const ::comphelper::NamedValueCollection& _rMediaDescriptor ) const
{
    // extract status indicator
    Sequence< Any > aDelegatorArguments;
    lcl_extractStatusIndicator( _rMediaDescriptor, aDelegatorArguments );

    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    SvtSaveOptions aSaveOpt;
    xInfoSet->setPropertyValue("UsePrettyPrinting", uno::makeAny(aSaveOpt.IsPrettyPrinting()));
    if ( aSaveOpt.IsSaveRelFSys() )
    {
        OUString sBaseURI = _rMediaDescriptor.getOrDefault("BaseURI", OUString());
        if (sBaseURI.isEmpty())
            sBaseURI = _rMediaDescriptor.getOrDefault("URL",OUString());
        xInfoSet->setPropertyValue("BaseURI", uno::makeAny(sBaseURI));
    }

    // Set TargetStorage, so it doesn't have to be re-constructed based on possibly empty URL.
    xInfoSet->setPropertyValue("TargetStorage", uno::makeAny(m_pImpl->getRootStorage()));

    // Set StreamRelPath, in case this document is an embedded one.
    OUString sStreamRelPath;
    OUString sURL = _rMediaDescriptor.getOrDefault("URL", OUString());
    if (sURL.startsWithIgnoreAsciiCase("vnd.sun.star.pkg:"))
    {
        // In this case the host contains the real path, and the path is the embedded stream name.
        INetURLObject aURL(sURL);
        sStreamRelPath = aURL.GetURLPath(INetURLObject::DECODE_WITH_CHARSET);
        if (sStreamRelPath.startsWith("/"))
            sStreamRelPath = sStreamRelPath.copy(1);
    }
    if (!sStreamRelPath.isEmpty())
        xInfoSet->setPropertyValue("StreamRelPath", uno::makeAny(sStreamRelPath));

    sal_Int32 nArgsLen = aDelegatorArguments.getLength();
    aDelegatorArguments.realloc(nArgsLen+1);
    aDelegatorArguments[nArgsLen++] <<= xInfoSet;

    Reference< XPropertySet > xProp( _rxTargetStorage, UNO_QUERY_THROW );
    xProp->setPropertyValue( INFO_MEDIATYPE, makeAny( OUString(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII) ) );

    OUString aVersion;
    SvtSaveOptions::ODFDefaultVersion const nDefVersion =
        aSaveOpt.GetODFDefaultVersion();
    // older versions can not have this property set,
    // it exists only starting from ODF1.2
    if (nDefVersion >= SvtSaveOptions::ODFVER_012)
        aVersion = ODFVER_012_TEXT;

    if (!aVersion.isEmpty())
    {
        try
        {
            xProp->setPropertyValue("Version" , uno::makeAny(aVersion));
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("dbaccess", "exception setting Version: " << e.Message);
        }
    }

    Reference< XComponent > xComponent( *const_cast< ODatabaseDocument* >( this ), UNO_QUERY_THROW );

    Sequence< PropertyValue > aMediaDescriptor;
    _rMediaDescriptor >>= aMediaDescriptor;

    xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("settings.xml")));
    WriteThroughComponent( xComponent, "settings.xml", "com.sun.star.comp.sdb.XMLSettingsExporter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("content.xml")));
    WriteThroughComponent( xComponent, "content.xml", "com.sun.star.comp.sdb.DBExportFilter",
        aDelegatorArguments, aMediaDescriptor, _rxTargetStorage );

    if ( _rxTargetStorage->hasByName ( sPictures ) )
    {
        try
        {
           // Delete any previously existing Pictures folder and regenerate
           // any needed content if needed
           Reference< XStorageBasedLibraryContainer > xDlgs = m_pImpl->getLibraryContainer( false );
           if ( xDlgs.is() )
           {
               Reference< XModel > xModel(const_cast< ODatabaseDocument*>(this));
               lcl_uglyHackToStoreDialogeEmbedImages( m_pImpl->getLibraryContainer(false), _rxTargetStorage, xModel, m_pImpl->m_aContext );
           }
       }
       catch ( const Exception& )
       {
            DBG_UNHANDLED_EXCEPTION();
       }
    }
    m_pImpl->storeLibraryContainersTo( _rxTargetStorage );
}

Reference< XUIConfigurationManager > SAL_CALL ODatabaseDocument::getUIConfigurationManager(  ) throw (RuntimeException, std::exception)
{
    return Reference< XUIConfigurationManager >( getUIConfigurationManager2(), UNO_QUERY_THROW );
}

Reference< XUIConfigurationManager2 > ODatabaseDocument::getUIConfigurationManager2(  ) throw (RuntimeException)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    if ( !m_xUIConfigurationManager.is() )
    {
        m_xUIConfigurationManager = UIConfigurationManager::create( m_pImpl->m_aContext );

        OUString aUIConfigFolderName( "Configurations2" );
        Reference< XStorage > xConfigStorage;

        // First try to open with READWRITE and then READ
        xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, ElementModes::READWRITE );
        if ( xConfigStorage.is() )
        {
            OUString aUIConfigMediaType( "application/vnd.sun.xml.ui.configuration" );
            OUString aMediaType;
            Reference< XPropertySet > xPropSet( xConfigStorage, UNO_QUERY );
            Any a = xPropSet->getPropertyValue( INFO_MEDIATYPE );
            if ( !( a >>= aMediaType ) ||  aMediaType.isEmpty() )
            {
                a <<= aUIConfigMediaType;
                xPropSet->setPropertyValue( INFO_MEDIATYPE, a );
            }
        }
        else
            xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, ElementModes::READ );

        // initialize ui configuration manager with document substorage
        m_xUIConfigurationManager->setStorage( xConfigStorage );
    }

    return m_xUIConfigurationManager;
}

Reference< XStorage > SAL_CALL ODatabaseDocument::getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStorage( aStorageName, nMode );
}

Sequence< OUString > SAL_CALL ODatabaseDocument::getDocumentSubStoragesNames(  ) throw (css::io::IOException, RuntimeException, std::exception)
{
    Reference< XDocumentSubStorageSupplier > xStorageAccess( m_pImpl->getDocumentSubStorageSupplier() );
    return xStorageAccess->getDocumentSubStoragesNames();
}

void ODatabaseDocument::impl_notifyStorageChange_nolck_nothrow( const Reference< XStorage >& _rxNewRootStorage )
{
    Reference< XInterface > xMe( *this );

    m_aStorageListeners.forEach< XStorageChangeListener >(
        boost::bind( &XStorageChangeListener::notifyStorageChange, _1, boost::cref( xMe ), boost::cref( _rxNewRootStorage ) ) );
}

void ODatabaseDocument::disposing()
{
    OSL_TRACE( "DD: disp: %p: %p", this, m_pImpl.get() );
    if ( !m_pImpl.is() )
    {
        // this means that we're already disposed
        OSL_ENSURE( ODatabaseDocument_OfficeDocument::rBHelper.bDisposed, "ODatabaseDocument::disposing: no impl anymore, but not yet disposed!" );
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

    OSL_ENSURE( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
    // normally, nobody should explicitly dispose, but only XCloseable::close
    // the document. And upon closing, our controllers are closed, too

    {
        uno::Reference<uno::XInterface> xUIInterface( m_xUIConfigurationManager );
        aKeepAlive.push_back( xUIInterface );
    }
    m_xUIConfigurationManager = nullptr;

    clearObjectContainer( m_xForms );
    clearObjectContainer( m_xReports );

    // reset the macro mode: in case the our impl struct stays alive (e.g. because our DataSource
    // object still exists), and somebody subsequently re-opens the document, we want to have
    // the security warning, again.
    m_pImpl->resetMacroExecutionMode();

    // similar arguing for our ViewMonitor
    m_aViewMonitor.reset();

    // tell our Impl to forget us
    m_pImpl->modelIsDisposing( impl_isInitialized(), ODatabaseModelImpl::ResetModelAccess() );

    // now, at the latest, the controller array should be empty. Controllers are
    // expected to listen for our disposal, and disconnect then
    OSL_ENSURE( m_aControllers.empty(), "ODatabaseDocument::disposing: there still are controllers!" );
    impl_disposeControllerFrames_nothrow();

    {
        uno::Reference<uno::XInterface> xModuleInterface( m_xModuleManager );
        aKeepAlive.push_back( xModuleInterface );
    }
    m_xModuleManager.clear();

    {
        uno::Reference<uno::XInterface> xTitleInterface( m_xTitleHelper );
        aKeepAlive.push_back( xTitleInterface );
    }
    m_xTitleHelper.clear();

    m_pImpl.clear();

    aGuard.clear();
    // <- SYNCHRONIZED

    aKeepAlive.clear();
}

// XComponent
void SAL_CALL ODatabaseDocument::dispose(  ) throw (RuntimeException, std::exception)
{
    ::cppu::WeakComponentImplHelperBase::dispose();
}

void SAL_CALL ODatabaseDocument::addEventListener( const Reference< lang::XEventListener >& _xListener ) throw (RuntimeException, std::exception)
{
    ::cppu::WeakComponentImplHelperBase::addEventListener( _xListener );
}

void SAL_CALL ODatabaseDocument::removeEventListener( const Reference< lang::XEventListener >& _xListener ) throw (RuntimeException, std::exception)
{
    ::cppu::WeakComponentImplHelperBase::removeEventListener( _xListener );
}

// XServiceInfo
OUString ODatabaseDocument::getImplementationName() throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.dba.ODatabaseDocument");
}

Sequence< OUString > ODatabaseDocument::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    return { "com.sun.star.sdb.OfficeDatabaseDocument", "com.sun.star.document.OfficeDocument" };
}

sal_Bool ODatabaseDocument::supportsService( const OUString& _rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Reference< XDataSource > SAL_CALL ODatabaseDocument::getDataSource() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodWithoutInit );
    return m_pImpl->getOrCreateDataSource();
}

namespace
{
/// Property map for embedded import info set.
comphelper::PropertyMapEntry const aEmbeddedImportInfoMap[] =
{
    {OUString("StreamRelPath"), 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
    {OUString("StreamName"), 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
    {OUString("SourceStorage"), 0, cppu::UnoType<embed::XStorage>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
    {OUString(), 0, css::uno::Type(), 0, 0}
};
}

void SAL_CALL ODatabaseDocument::loadFromStorage(const Reference<XStorage>& xStorage, const Sequence<PropertyValue>& rMediaDescriptor) throw (IllegalArgumentException, DoubleInitializationException, IOException, Exception, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::InitMethod);

    uno::Reference<beans::XPropertySet> xInfoSet(comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aEmbeddedImportInfoMap)));
    comphelper::NamedValueCollection aDescriptor(rMediaDescriptor);
    xInfoSet->setPropertyValue("StreamRelPath", uno::makeAny(aDescriptor.getOrDefault("HierarchicalDocumentName", OUString())));
    xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("content.xml")));
    xInfoSet->setPropertyValue("SourceStorage", uno::makeAny(xStorage));

    uno::Sequence<uno::Any> aFilterCreationArgs(1);
    aFilterCreationArgs[0] <<= xInfoSet;

    uno::Reference<document::XImporter> xImporter(m_pImpl->m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.comp.sdb.DBFilter", aFilterCreationArgs, m_pImpl->m_aContext), uno::UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xComponent(*this, uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xComponent);

    uno::Reference<document::XFilter> xFilter(xImporter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aFilterArgs;
    xFilter->filter(aFilterArgs);

    // In case of embedding, XModel::attachResource is already called.
    if (m_bEmbedded)
        impl_setInitialized();

    impl_setModified_nothrow(false, aGuard);
}

void SAL_CALL ODatabaseDocument::storeToStorage( const Reference< XStorage >& _rxStorage, const Sequence< PropertyValue >& _rMediaDescriptor ) throw (IllegalArgumentException, IOException, Exception, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    impl_storeToStorage_throw( _rxStorage, _rMediaDescriptor, aGuard );
}

void SAL_CALL ODatabaseDocument::switchToStorage( const Reference< XStorage >& _rxNewRootStorage ) throw (IllegalArgumentException, IOException, Exception, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    Reference< XStorage > xNewRootStorage( m_pImpl->switchToStorage( _rxNewRootStorage ) );

    aGuard.clear();
    impl_notifyStorageChange_nolck_nothrow( xNewRootStorage );
}

Reference< XStorage > SAL_CALL ODatabaseDocument::getDocumentStorage(  ) throw (IOException, Exception, RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    return m_pImpl->getOrCreateRootStorage();
}

void SAL_CALL ODatabaseDocument::addStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aStorageListeners.addInterface( _Listener );
}

void SAL_CALL ODatabaseDocument::removeStorageChangeListener( const Reference< XStorageChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    m_aStorageListeners.addInterface( _Listener );
}

Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getBasicLibraries() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return m_pImpl->getLibraryContainer( true );
}

Reference< XStorageBasedLibraryContainer > SAL_CALL ODatabaseDocument::getDialogLibraries() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    return m_pImpl->getLibraryContainer( false );
}

sal_Bool SAL_CALL ODatabaseDocument::getAllowMacroExecution() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    return m_pImpl->adjustMacroMode_AutoReject();
}

Reference< XEmbeddedScripts > SAL_CALL ODatabaseDocument::getScriptContainer() throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    return this;
}

Reference< provider::XScriptProvider > SAL_CALL ODatabaseDocument::getScriptProvider(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    Reference< XScriptProvider > xScriptProvider( m_xScriptProvider );
    if ( !xScriptProvider.is() )
    {
        Reference < XScriptProviderFactory > xFactory =
            theMasterScriptProviderFactory::get( m_pImpl->m_aContext );

        Any aScriptProviderContext;
        if ( m_bAllowDocumentScripting )
            aScriptProviderContext <<= Reference< XModel >( this );

        xScriptProvider.set( xFactory->createScriptProvider( aScriptProviderContext ), UNO_SET_THROW );
        m_xScriptProvider = xScriptProvider;
    }

    return xScriptProvider;
}

Reference< XNameReplace > SAL_CALL ODatabaseDocument::getEvents(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return m_pEventContainer;
}

Reference< XInterface > ODatabaseDocument::getThis() const
{
    return *const_cast< ODatabaseDocument* >( this );
}

struct CreateAny : public ::std::unary_function< Reference<XController>, Any>
{
    Any operator() (const Reference<XController>& lhs) const
    {
        return makeAny(lhs);
    }
};

// XModel2
Reference< XEnumeration > SAL_CALL ODatabaseDocument::getControllers(  ) throw (RuntimeException, std::exception)
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    uno::Sequence< Any> aController( m_aControllers.size() );
    ::std::transform( m_aControllers.begin(), m_aControllers.end(), aController.getArray(), CreateAny() );
    return new ::comphelper::OAnyEnumeration(aController);
}

Sequence< OUString > SAL_CALL ODatabaseDocument::getAvailableViewControllerNames(  ) throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames { SERVICE_SDB_APPLICATIONCONTROLLER };
    return aNames;
}

Reference< XController2 > SAL_CALL ODatabaseDocument::createDefaultViewController( const Reference< XFrame >& _Frame ) throw (IllegalArgumentException, Exception, RuntimeException, std::exception)
{
    return createViewController( "Default", Sequence< PropertyValue >(), _Frame);
}

Reference< XController2 > SAL_CALL ODatabaseDocument::createViewController( const OUString& _ViewName, const Sequence< PropertyValue >& _Arguments, const Reference< XFrame >& _Frame ) throw (IllegalArgumentException, Exception, RuntimeException, std::exception)
{
    if ( _ViewName != "Default" && _ViewName != "Preview" )
        throw IllegalArgumentException( OUString(), *this, 1 );
    if ( !_Frame.is() )
        throw IllegalArgumentException( OUString(), *this, 3 );

    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    aGuard.clear();

    Reference< XController2 > xController(
         m_pImpl->m_aContext->getServiceManager()->createInstanceWithContext("org.openoffice.comp.dbu.OApplicationController", m_pImpl->m_aContext),
         UNO_QUERY_THROW );

    ::comphelper::NamedValueCollection aInitArgs( _Arguments );
    aInitArgs.put( "Frame", _Frame );
    if ( _ViewName == "Preview" )
        aInitArgs.put( "Preview", true );
    Reference< XInitialization > xInitController( xController, UNO_QUERY_THROW );
    xInitController->initialize( aInitArgs.getWrappedPropertyValues() );

    return xController;
}

Reference< XTitle > ODatabaseDocument::impl_getTitleHelper_throw()
{
    if ( ! m_xTitleHelper.is ())
    {
        Reference< XUntitledNumbers >  xDesktop(Desktop::create(m_pImpl->m_aContext), uno::UNO_QUERY_THROW);
        Reference< frame::XModel >     xThis   (getThis(), uno::UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper(m_pImpl->m_aContext);
        m_xTitleHelper.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);
        pHelper->setOwner                   (xThis   );
        pHelper->connectWithUntitledNumbers (xDesktop);
    }

    return m_xTitleHelper;
}

uno::Reference< frame::XUntitledNumbers > ODatabaseDocument::impl_getUntitledHelper_throw(const uno::Reference< uno::XInterface >& _xComponent)
{
    if ( !m_xModuleManager.is() )
        m_xModuleManager.set( ModuleManager::create(m_pImpl->m_aContext) );

    OUString sModuleId;
    try
    {
        sModuleId = m_xModuleManager->identify( _xComponent );
    }
    catch(const uno::Exception&)
    {
    }
    uno::Reference< frame::XUntitledNumbers > xNumberedControllers;

    TNumberedController::iterator aFind = m_aNumberedControllers.find(sModuleId);
    if ( aFind == m_aNumberedControllers.end() )
    {
        uno::Reference< frame::XModel > xThis(static_cast< frame::XModel* >(this), uno::UNO_QUERY_THROW);
        ::comphelper::NumberedCollection* pHelper = new ::comphelper::NumberedCollection();
        xNumberedControllers.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);

        pHelper->setOwner          (xThis);

        m_aNumberedControllers.insert(TNumberedController::value_type(sModuleId,xNumberedControllers));
    }
    else
        xNumberedControllers = aFind->second;

    return xNumberedControllers;
}

// css.frame.XTitle
OUString SAL_CALL ODatabaseDocument::getTitle()
    throw (uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard( *this, DocumentGuard::MethodUsedDuringInit );
    return impl_getTitleHelper_throw()->getTitle();
}

// css.frame.XTitle
void SAL_CALL ODatabaseDocument::setTitle( const OUString& sTitle )
    throw (uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    impl_getTitleHelper_throw()->setTitle( sTitle );
    m_aEventNotifier.notifyDocumentEventAsync( "OnTitleChanged" );
    // <- SYNCHRONIZED
}

// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::addTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster( impl_getTitleHelper_throw(), uno::UNO_QUERY_THROW );
    xBroadcaster->addTitleChangeListener( xListener );
}

// css.frame.XTitleChangeBroadcaster
void SAL_CALL ODatabaseDocument::removeTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
    throw (uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster( impl_getTitleHelper_throw(), uno::UNO_QUERY_THROW );
    xBroadcaster->removeTitleChangeListener( xListener );
}

// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL ODatabaseDocument::leaseNumber( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception         )
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    return impl_getUntitledHelper_throw(xComponent)->leaseNumber (xComponent);
}

// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumber( ::sal_Int32 nNumber )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception         )
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    impl_getUntitledHelper_throw()->releaseNumber (nNumber);
}

// css.frame.XUntitledNumbers
void SAL_CALL ODatabaseDocument::releaseNumberForComponent( const uno::Reference< uno::XInterface >& xComponent )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception         )
{
    DocumentGuard aGuard(*this, DocumentGuard::DefaultMethod);
    impl_getUntitledHelper_throw(xComponent)->releaseNumberForComponent (xComponent);
}

// css.frame.XUntitledNumbers
OUString SAL_CALL ODatabaseDocument::getUntitledPrefix()    throw (uno::RuntimeException, std::exception)
{
    return OUString();
}

}   // namespace dbaccess

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_dba_ODatabaseDocument(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    Reference<XUnoTunnel> xDBContextTunnel(DatabaseContext::create(context), UNO_QUERY_THROW);
    dbaccess::ODatabaseContext* pContext = reinterpret_cast<dbaccess::ODatabaseContext*>(
        xDBContextTunnel->getSomething(
            dbaccess::ODatabaseContext::getUnoTunnelImplementationId()));

    rtl::Reference<dbaccess::ODatabaseModelImpl> pImpl(
            new dbaccess::ODatabaseModelImpl(context, *pContext));
    css::uno::Reference<XInterface> inst(pImpl->createNewModel_deliverOwnership(false));
    inst->acquire();
    return inst.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
