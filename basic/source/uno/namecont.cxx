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

#include <config_features.h>
#include <config_folders.h>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/errinf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/sequence.hxx>

#include "namecont.hxx"
#include <basic/basicmanagerrepository.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <basic/basmgr.hxx>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/script/LibraryNotLoadedException.hpp>
#include <com/sun/star/script/vba/VBAScriptEventId.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <basic/sbmod.hxx>
#include <memory>

namespace basic
{

using namespace com::sun::star::document;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::ucb;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::util;
using namespace com::sun::star::task;
using namespace com::sun::star::embed;
using namespace com::sun::star::frame;
using namespace com::sun::star::deployment;
using namespace com::sun::star;
using namespace cppu;
using namespace osl;

using com::sun::star::uno::Reference;

// #i34411: Flag for error handling during migration
static bool GbMigrationSuppressErrors = false;


// Implementation class NameContainer

// Methods XElementAccess
Type NameContainer::getElementType()
    throw(RuntimeException, std::exception)
{
    return mType;
}

sal_Bool NameContainer::hasElements()
    throw(RuntimeException, std::exception)
{
    bool bRet = (mnElementCount > 0);
    return bRet;
}

// Methods XNameAccess
Any NameContainer::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }
    sal_Int32 iHashResult = (*aIt).second;
    Any aRetAny = mValues[ iHashResult ];
    return aRetAny;
}

Sequence< OUString > NameContainer::getElementNames()
    throw(RuntimeException, std::exception)
{
    return comphelper::containerToSequence(mNames);
}

sal_Bool NameContainer::hasByName( const OUString& aName )
    throw(RuntimeException, std::exception)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    bool bRet = ( aIt != mHashMap.end() );
    return bRet;
}


// Methods XNameReplace
void NameContainer::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    const Type& aAnyType = aElement.getValueType();
    if( mType != aAnyType )
    {
        throw IllegalArgumentException();
    }
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }
    sal_Int32 iHashResult = (*aIt).second;
    Any aOldElement = mValues[ iHashResult ];
    mValues[ iHashResult ] = aElement;


    // Fire event
    if( maContainerListeners.getLength() > 0 )
    {
        ContainerEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Accessor <<= aName;
        aEvent.Element = aElement;
        aEvent.ReplacedElement = aOldElement;
        maContainerListeners.notifyEach( &XContainerListener::elementReplaced, aEvent );
    }

    /*  After the container event has been fired (one listener will update the
        core Basic manager), fire change event. Listeners can rely that the
        Basic source code of the core Basic manager is up-to-date. */
    if( maChangesListeners.getLength() > 0 )
    {
        ChangesEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Base <<= aEvent.Source;
        aEvent.Changes.realloc( 1 );
        aEvent.Changes[ 0 ].Accessor <<= aName;
        aEvent.Changes[ 0 ].Element <<= aElement;
        aEvent.Changes[ 0 ].ReplacedElement = aOldElement;
        maChangesListeners.notifyEach( &XChangesListener::changesOccurred, aEvent );
    }
}

void NameContainer::insertCheck(const OUString& aName, const Any& aElement)
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    NameContainerNameMap::iterator aIt = mHashMap.find(aName);
    if( aIt != mHashMap.end() )
    {
        throw ElementExistException();
    }
    insertNoCheck(aName, aElement);
}

void NameContainer::insertNoCheck(const OUString& aName, const Any& aElement)
    throw(IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    const Type& aAnyType = aElement.getValueType();
    if( mType != aAnyType )
    {
        throw IllegalArgumentException();
    }

    sal_Int32 nCount = mNames.size();
    mNames.push_back( aName );
    mValues.push_back( aElement );

    mHashMap[ aName ] = nCount;
    mnElementCount++;

    // Fire event
    if( maContainerListeners.getLength() > 0 )
    {
        ContainerEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Accessor <<= aName;
        aEvent.Element = aElement;
        maContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
    }

    /*  After the container event has been fired (one listener will update the
        core Basic manager), fire change event. Listeners can rely that the
        Basic source code of the core Basic manager is up-to-date. */
    if( maChangesListeners.getLength() > 0 )
    {
        ChangesEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Base <<= aEvent.Source;
        aEvent.Changes.realloc( 1 );
        aEvent.Changes[ 0 ].Accessor <<= aName;
        aEvent.Changes[ 0 ].Element <<= aElement;
        maChangesListeners.notifyEach( &XChangesListener::changesOccurred, aEvent );
    }
}

// Methods XNameContainer
void NameContainer::insertByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    insertCheck(aName, aElement);
}

void NameContainer::removeByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        OUString sMessage = "\"" + aName + "\" not found";
        throw NoSuchElementException(sMessage);
    }

    sal_Int32 iHashResult = (*aIt).second;
    Any aOldElement = mValues[ iHashResult ];
    mHashMap.erase( aIt );
    sal_Int32 iLast = mNames.size() - 1;
    if( iLast != iHashResult )
    {
        mNames[ iHashResult ] = mNames[ iLast ];
        mValues[ iHashResult ] = mValues[ iLast ];
        mHashMap[ mNames[ iHashResult ] ] = iHashResult;
    }
    mNames.resize( iLast );
    mValues.resize( iLast );
    mnElementCount--;

    // Fire event
    if( maContainerListeners.getLength() > 0 )
    {
        ContainerEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Accessor <<= aName;
        aEvent.Element = aOldElement;
        maContainerListeners.notifyEach( &XContainerListener::elementRemoved, aEvent );
    }

    /*  After the container event has been fired (one listener will update the
        core Basic manager), fire change event. Listeners can rely that the
        Basic source code of the core Basic manager is up-to-date. */
    if( maChangesListeners.getLength() > 0 )
    {
        ChangesEvent aEvent;
        aEvent.Source = mpxEventSource;
        aEvent.Base <<= aEvent.Source;
        aEvent.Changes.realloc( 1 );
        aEvent.Changes[ 0 ].Accessor <<= aName;
        // aEvent.Changes[ 0 ].Element remains empty (meaning "replaced with nothing")
        aEvent.Changes[ 0 ].ReplacedElement = aOldElement;
        maChangesListeners.notifyEach( &XChangesListener::changesOccurred, aEvent );
    }
}


// Methods XContainer
void SAL_CALL NameContainer::addContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    if( !xListener.is() )
    {
        throw RuntimeException("addContainerListener called with null xListener",
                               static_cast< cppu::OWeakObject * >(this));
    }
    maContainerListeners.addInterface( Reference<XInterface>(xListener, UNO_QUERY) );
}

void SAL_CALL NameContainer::removeContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    if( !xListener.is() )
    {
        throw RuntimeException();
    }
    maContainerListeners.removeInterface( Reference<XInterface>(xListener, UNO_QUERY) );
}

// Methods XChangesNotifier
void SAL_CALL NameContainer::addChangesListener( const Reference< XChangesListener >& xListener )
    throw (RuntimeException, std::exception)
{
    if( !xListener.is() )
    {
        throw RuntimeException();
    }
    maChangesListeners.addInterface( Reference<XInterface>(xListener, UNO_QUERY) );
}

void SAL_CALL NameContainer::removeChangesListener( const Reference< XChangesListener >& xListener )
    throw (RuntimeException, std::exception)
{
    if( !xListener.is() )
    {
        throw RuntimeException();
    }
    maChangesListeners.removeInterface( Reference<XInterface>(xListener, UNO_QUERY) );
}


// ModifiableHelper

void ModifiableHelper::setModified( bool _bModified )
{
    if ( _bModified == mbModified )
    {
        return;
    }
    mbModified = _bModified;

    if ( m_aModifyListeners.getLength() == 0 )
    {
        return;
    }
    EventObject aModifyEvent( m_rEventSource );
    m_aModifyListeners.notifyEach( &XModifyListener::modified, aModifyEvent );
}


VBAScriptListenerContainer::VBAScriptListenerContainer( ::osl::Mutex& rMutex ) :
    VBAScriptListenerContainer_BASE( rMutex )
{
}

bool VBAScriptListenerContainer::implTypedNotify( const Reference< vba::XVBAScriptListener >& rxListener, const vba::VBAScriptEvent& rEvent )
    throw (Exception)
{
    rxListener->notifyVBAScriptEvent( rEvent );
    return true;    // notify all other listeners too
}

// Ctor
SfxLibraryContainer::SfxLibraryContainer()
    : SfxLibraryContainer_BASE( maMutex )

    , maVBAScriptListeners( maMutex )
    , mnRunningVBAScripts( 0 )
    , mbVBACompat( false )
    , maModifiable( *this, maMutex )
    , maNameContainer( new NameContainer(cppu::UnoType<XNameAccess>::get()) )
    , mbOldInfoFormat( false )
    , mbOasis2OOoFormat( false )
    , mpBasMgr( nullptr )
    , mbOwnBasMgr( false )
    , meInitMode(DEFAULT)
{
    mxContext = comphelper::getProcessComponentContext();

    mxSFI = ucb::SimpleFileAccess::create( mxContext );

    mxStringSubstitution = util::PathSubstitution::create( mxContext );
}

SfxLibraryContainer::~SfxLibraryContainer()
{
    if( mbOwnBasMgr )
    {
        BasicManager::LegacyDeleteBasicManager( mpBasMgr );
    }
}

void SfxLibraryContainer::checkDisposed() const
{
    if ( isDisposed() )
    {
        throw DisposedException( OUString(),
                                 *const_cast< SfxLibraryContainer* >( this ) );
    }
}

void SfxLibraryContainer::enterMethod()
{
    Application::GetSolarMutex().acquire();
    checkDisposed();
}

void SfxLibraryContainer::leaveMethod()
{
    Application::GetSolarMutex().release();
}

BasicManager* SfxLibraryContainer::getBasicManager()
{
    try
    {
        if ( mpBasMgr )
        {
            return mpBasMgr;
        }
        Reference< XModel > xDocument( mxOwnerDocument.get(), UNO_QUERY );
        SAL_WARN_IF(
            !xDocument.is(), "basic",
            ("SfxLibraryContainer::getBasicManager: cannot obtain a BasicManager"
             " without document!"));
        if ( xDocument.is() )
        {
            mpBasMgr = BasicManagerRepository::getDocumentBasicManager( xDocument );
        }
    }
    catch (const css::ucb::ContentCreationException& e)
    {
        SAL_WARN( "basic", "SfxLibraryContainer::getBasicManager : Caught exception: " << e.Message );
    }
    return mpBasMgr;
}

// Methods XStorageBasedLibraryContainer
Reference< XStorage > SAL_CALL SfxLibraryContainer::getRootStorage() throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return mxStorage;
}

void SAL_CALL SfxLibraryContainer::setRootStorage( const Reference< XStorage >& _rxRootStorage )
    throw (IllegalArgumentException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    if ( !_rxRootStorage.is() )
    {
        throw IllegalArgumentException();
    }
    mxStorage = _rxRootStorage;
    onNewRootStorage();
}

void SAL_CALL SfxLibraryContainer::storeLibrariesToStorage( const Reference< XStorage >& _rxRootStorage )
    throw (IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    if ( !_rxRootStorage.is() )
    {
        throw IllegalArgumentException();
    }
    try
    {
        storeLibraries_Impl( _rxRootStorage, true );
    }
    catch( const Exception& )
    {
        throw WrappedTargetException( OUString(),
                                      *this, ::cppu::getCaughtException() );
    }
}


// Methods XModifiable
sal_Bool SfxLibraryContainer::isModified()
    throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    if ( maModifiable.isModified() )
    {
        return true;
    }
    // the library container is not modified, go through the libraries and check whether they are modified
    Sequence< OUString > aNames = maNameContainer->getElementNames();
    const OUString* pNames = aNames.getConstArray();
    sal_Int32 nNameCount = aNames.getLength();

    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
    {
        OUString aName = pNames[ i ];
        try
        {
            SfxLibrary* pImplLib = getImplLib( aName );
            if( pImplLib->isModified() )
            {
                if ( aName == "Standard" )
                {
                    // this is a workaround that has to be implemented because
                    // empty standard library should stay marked as modified
                    // but should not be treated as modified while it is empty
                    if ( pImplLib->hasElements() )
                        return true;
                }
                else
                {
                    return true;
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
    }

    return false;
}

void SAL_CALL SfxLibraryContainer::setModified( sal_Bool _bModified )
    throw (PropertyVetoException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    maModifiable.setModified( _bModified );
}

void SAL_CALL SfxLibraryContainer::addModifyListener( const Reference< XModifyListener >& _rxListener )
    throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    maModifiable.addModifyListener( _rxListener );
}

void SAL_CALL SfxLibraryContainer::removeModifyListener( const Reference< XModifyListener >& _rxListener )
    throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    maModifiable.removeModifyListener( _rxListener );
}

// Methods XPersistentLibraryContainer
Any SAL_CALL SfxLibraryContainer::getRootLocation() throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return makeAny( getRootStorage() );
}

OUString SAL_CALL SfxLibraryContainer::getContainerLocationName() throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return maLibrariesDir;
}

void SAL_CALL SfxLibraryContainer::storeLibraries(  )
    throw (WrappedTargetException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    try
    {
        storeLibraries_Impl( mxStorage, mxStorage.is()  );
        // we need to store *all* libraries if and only if we are based on a storage:
        // in this case, storeLibraries_Impl will remove the source storage, after loading
        // all libraries, so we need to force them to be stored, again
    }
    catch( const Exception& )
    {
        throw WrappedTargetException( OUString(), *this, ::cppu::getCaughtException() );
    }
}

static void checkAndCopyFileImpl( const INetURLObject& rSourceFolderInetObj,
                                  const INetURLObject& rTargetFolderInetObj,
                                  const OUString& rCheckFileName,
                                  const OUString& rCheckExtension,
                                  const Reference< XSimpleFileAccess3 >& xSFI )
{
    INetURLObject aTargetFolderInetObj( rTargetFolderInetObj );
    aTargetFolderInetObj.insertName( rCheckFileName, true, INetURLObject::LAST_SEGMENT,
                                     INetURLObject::ENCODE_ALL );
    aTargetFolderInetObj.setExtension( rCheckExtension );
    OUString aTargetFile = aTargetFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
    if( !xSFI->exists( aTargetFile ) )
    {
        INetURLObject aSourceFolderInetObj( rSourceFolderInetObj );
        aSourceFolderInetObj.insertName( rCheckFileName, true, INetURLObject::LAST_SEGMENT,
                                         INetURLObject::ENCODE_ALL );
        aSourceFolderInetObj.setExtension( rCheckExtension );
        OUString aSourceFile = aSourceFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
        xSFI->copy( aSourceFile, aTargetFile );
    }
}

static void createVariableURL( OUString& rStr, const OUString& rLibName,
                               const OUString& rInfoFileName, bool bUser )
{
    if( bUser )
    {
        rStr = "$(USER)/basic/";
    }
    else
    {
        rStr = "$(INST)/" LIBO_SHARE_FOLDER "/basic/";
    }
    rStr += rLibName + "/" + rInfoFileName + ".xlb/";
}

void SfxLibraryContainer::init( const OUString& rInitialDocumentURL, const uno::Reference< embed::XStorage >& rxInitialStorage )
{
    // this might be called from within the ctor, and the impl_init might (indirectly) create
    // an UNO reference to ourself.
    // Ensure that we're not destroyed while we're in here
    osl_atomic_increment( &m_refCount );
    init_Impl( rInitialDocumentURL, rxInitialStorage );
    osl_atomic_decrement( &m_refCount );
}

void SfxLibraryContainer::init_Impl( const OUString& rInitialDocumentURL,
                                     const uno::Reference< embed::XStorage >& rxInitialStorage )
{
    uno::Reference< embed::XStorage > xStorage = rxInitialStorage;

    maInitialDocumentURL = rInitialDocumentURL;
    maInfoFileName = OUString::createFromAscii( getInfoFileName() );
    maOldInfoFileName = OUString::createFromAscii( getOldInfoFileName() );
    maLibElementFileExtension = OUString::createFromAscii( getLibElementFileExtension() );
    maLibrariesDir = OUString::createFromAscii( getLibrariesDir() );

    meInitMode = DEFAULT;
    INetURLObject aInitUrlInetObj( maInitialDocumentURL );
    OUString aInitFileName = aInitUrlInetObj.GetMainURL( INetURLObject::NO_DECODE );
    if( !aInitFileName.isEmpty() )
    {
        // We need a BasicManager to avoid problems
        StarBASIC* pBas = new StarBASIC();
        mpBasMgr = new BasicManager( pBas );
        mbOwnBasMgr = true;

        OUString aExtension = aInitUrlInetObj.getExtension();
        if( aExtension == "xlc" )
        {
            meInitMode = CONTAINER_INIT_FILE;
            INetURLObject aLibPathInetObj( aInitUrlInetObj );
            aLibPathInetObj.removeSegment();
            maLibraryPath = aLibPathInetObj.GetMainURL( INetURLObject::NO_DECODE );
        }
        else if( aExtension == "xlb" )
        {
            meInitMode = LIBRARY_INIT_FILE;
            uno::Reference< embed::XStorage > xDummyStor;
            ::xmlscript::LibDescriptor aLibDesc;
            implLoadLibraryIndexFile( nullptr, aLibDesc, xDummyStor, aInitFileName );
            return;
        }
        else
        {
            // Decide between old and new document
            bool bOldStorage = SotStorage::IsOLEStorage( aInitFileName );
            if ( bOldStorage )
            {
                meInitMode = OLD_BASIC_STORAGE;
                importFromOldStorage( aInitFileName );
                return;
            }
            else
            {
                meInitMode = OFFICE_DOCUMENT;
                try
                {
                    xStorage = ::comphelper::OStorageHelper::GetStorageFromURL( aInitFileName, embed::ElementModes::READ );
                }
                catch (const uno::Exception& )
                {
                    // TODO: error handling
                }
            }
        }
    }
    else
    {
        // Default paths
        maLibraryPath = SvtPathOptions().GetBasicPath();
    }

    Reference< XParser > xParser = xml::sax::Parser::create(mxContext);

    uno::Reference< io::XInputStream > xInput;

    mxStorage = xStorage;
    bool bStorage = mxStorage.is();


    // #110009: Scope to force the StorageRefs to be destructed and
    // so the streams to be closed before the preload operation
    {

    uno::Reference< embed::XStorage > xLibrariesStor;
    OUString aFileName;

    int nPassCount = 1;
    if( !bStorage && meInitMode == DEFAULT )
    {
        nPassCount = 2;
    }
    for( int nPass = 0 ; nPass < nPassCount ; nPass++ )
    {
        if( bStorage )
        {
            SAL_WARN_IF(
                meInitMode != DEFAULT && meInitMode != OFFICE_DOCUMENT, "basic",
                "Wrong InitMode for document");
            try
            {
                uno::Reference< io::XStream > xStream;
                xLibrariesStor = xStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );

                if ( xLibrariesStor.is() )
                {
                    aFileName = maInfoFileName + "-lc.xml";
                    try
                    {
                        xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                    }
                    catch(const uno::Exception& )
                    {}

                    if( !xStream.is() )
                    {
                        mbOldInfoFormat = true;

                        // Check old version
                        aFileName = maOldInfoFileName + ".xml";
                        try
                        {
                            xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                        }
                        catch(const uno::Exception& )
                        {}

                        if( !xStream.is() )
                        {
                            // Check for EA2 document version with wrong extensions
                            aFileName = maOldInfoFileName + ".xli";
                            xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                        }
                    }
                }

                if ( xStream.is() )
                {
                    xInput = xStream->getInputStream();
                }
            }
            catch(const uno::Exception& )
            {
                // TODO: error handling?
            }
        }
        else
        {
            std::unique_ptr<INetURLObject> pLibInfoInetObj;
            if( meInitMode == CONTAINER_INIT_FILE )
            {
                aFileName = aInitFileName;
            }
            else
            {
                if( nPass == 1 )
                {
                    pLibInfoInetObj.reset(new INetURLObject( maLibraryPath.getToken(0, (sal_Unicode)';') ));
                }
                else
                {
                    pLibInfoInetObj.reset(new INetURLObject( maLibraryPath.getToken(1, (sal_Unicode)';') ));
                }
                pLibInfoInetObj->insertName( maInfoFileName, false, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
                pLibInfoInetObj->setExtension( "xlc" );
                aFileName = pLibInfoInetObj->GetMainURL( INetURLObject::NO_DECODE );
            }

            try
            {
                xInput = mxSFI->openFileRead( aFileName );
            }
            catch(const Exception& )
            {
                // Silently tolerate empty or missing files
                xInput.clear();
            }

            // Old variant?
            if( !xInput.is() && nPass == 0 )
            {
                INetURLObject aLibInfoInetObj( maLibraryPath.getToken(1, (sal_Unicode)';') );
                aLibInfoInetObj.insertName( maOldInfoFileName, false, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
                aLibInfoInetObj.setExtension( "xli" );
                aFileName = aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE );

                try
                {
                    xInput = mxSFI->openFileRead( aFileName );
                    mbOldInfoFormat = true;
                }
                catch(const Exception& )
                {
                    xInput.clear();
                }
            }
        }

        if( xInput.is() )
        {
            InputSource source;
            source.aInputStream = xInput;
            source.sSystemId    = aFileName;

            // start parsing
            std::unique_ptr< ::xmlscript::LibDescriptorArray> pLibArray(new ::xmlscript::LibDescriptorArray());

            try
            {
                xParser->setDocumentHandler( ::xmlscript::importLibraryContainer( pLibArray.get() ) );
                xParser->parseStream( source );
            }
            catch ( const xml::sax::SAXException& e )
            {
                SAL_WARN("basic", e.Message);
                return;
            }
            catch ( const io::IOException& e )
            {
                SAL_WARN("basic", e.Message);
                return;
            }

            sal_Int32 nLibCount = pLibArray->mnLibCount;
            for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
            {
                ::xmlscript::LibDescriptor& rLib = pLibArray->mpLibs[i];

                // Check storage URL
                OUString aStorageURL = rLib.aStorageURL;
                if( !bStorage && aStorageURL.isEmpty() && nPass == 0 )
                {
                    OUString aLibraryPath;
                    if( meInitMode == CONTAINER_INIT_FILE )
                    {
                        aLibraryPath = maLibraryPath;
                    }
                    else
                    {
                        aLibraryPath = maLibraryPath.getToken(1, (sal_Unicode)';');
                    }
                    INetURLObject aInetObj( aLibraryPath );

                    aInetObj.insertName( rLib.aName, true, INetURLObject::LAST_SEGMENT,
                                         INetURLObject::ENCODE_ALL );
                    OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
                    if( mxSFI->isFolder( aLibDirPath ) )
                    {
                        createVariableURL( rLib.aStorageURL, rLib.aName, maInfoFileName, true );
                        maModifiable.setModified( true );
                    }
                    else if( rLib.bLink )
                    {
                        // Check "share" path
                        INetURLObject aShareInetObj( maLibraryPath.getToken(0, (sal_Unicode)';') );
                        aShareInetObj.insertName( rLib.aName, true, INetURLObject::LAST_SEGMENT,
                                                  INetURLObject::ENCODE_ALL );
                        OUString aShareLibDirPath = aShareInetObj.GetMainURL( INetURLObject::NO_DECODE );
                        if( mxSFI->isFolder( aShareLibDirPath ) )
                        {
                            createVariableURL( rLib.aStorageURL, rLib.aName, maInfoFileName, false );
                            maModifiable.setModified( true );
                        }
                        else
                        {
                            // #i25537: Ignore lib if library folder does not really exist
                            continue;
                        }
                    }
                }

                OUString aLibName = rLib.aName;

                // If the same library name is used by the shared and the
                // user lib container index files the user file wins
                if( nPass == 1 && hasByName( aLibName ) )
                {
                    continue;
                }
                SfxLibrary* pImplLib;
                if( rLib.bLink )
                {
                    Reference< XNameAccess > xLib =
                        createLibraryLink( aLibName, rLib.aStorageURL, rLib.bReadOnly );
                    pImplLib = static_cast< SfxLibrary* >( xLib.get() );
                }
                else
                {
                    Reference< XNameContainer > xLib = createLibrary( aLibName );
                    pImplLib = static_cast< SfxLibrary* >( xLib.get() );
                    pImplLib->mbLoaded = false;
                    pImplLib->mbReadOnly = rLib.bReadOnly;
                    if( !bStorage )
                    {
                        checkStorageURL( rLib.aStorageURL, pImplLib->maLibInfoFileURL,
                                         pImplLib->maStorageURL, pImplLib->maUnexpandedStorageURL );
                    }
                }
                maModifiable.setModified( false );

                // Read library info files
                if( !mbOldInfoFormat )
                {
                    uno::Reference< embed::XStorage > xLibraryStor;
                    if( !pImplLib->mbInitialised && bStorage )
                    {
                        try
                        {
                            xLibraryStor = xLibrariesStor->openStorageElement( rLib.aName,
                                                                                embed::ElementModes::READ );
                        }
                        catch(const uno::Exception& )
                        {
                            #if OSL_DEBUG_LEVEL > 0
                            Any aError( ::cppu::getCaughtException() );
                            SAL_WARN(
                                "basic",
                                "couldn't open sub storage for library \""
                                    << rLib.aName << "\". Exception: "
                                    << comphelper::anyToString(aError));
                            #endif
                        }
                    }

                    // Link is already initialised in createLibraryLink()
                    if( !pImplLib->mbInitialised && (!bStorage || xLibraryStor.is()) )
                    {
                        OUString aIndexFileName;
                        bool bLoaded = implLoadLibraryIndexFile( pImplLib, rLib, xLibraryStor, aIndexFileName );
                        SAL_WARN_IF(
                            bLoaded && aLibName != rLib.aName, "basic",
                            ("Different library names in library container and"
                             " library info files!"));
                        if( GbMigrationSuppressErrors && !bLoaded )
                        {
                            removeLibrary( aLibName );
                        }
                    }
                }
                else if( !bStorage )
                {
                    // Write new index file immediately because otherwise
                    // the library elements will be lost when storing into
                    // the new info format
                    uno::Reference< embed::XStorage > xTmpStorage;
                    implStoreLibraryIndexFile( pImplLib, rLib, xTmpStorage );
                }

                implImportLibDescriptor( pImplLib, rLib );

                if( nPass == 1 )
                {
                    pImplLib->mbSharedIndexFile = true;
                    pImplLib->mbReadOnly = true;
                }
            }

            // Keep flag for documents to force writing the new index files
            if( !bStorage )
            {
                mbOldInfoFormat = false;
            }
        }
    }

    // #110009: END Scope to force the StorageRefs to be destructed
    }

    if( !bStorage && meInitMode == DEFAULT )
    {
        try
        {
            implScanExtensions();
        }
        catch(const uno::Exception& )
        {
            // TODO: error handling?
            SAL_WARN("basic", "Cannot access extensions!");
        }
    }

    // Preload?
    {
        Sequence< OUString > aNames = maNameContainer->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nNameCount = aNames.getLength();
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aName = pNames[ i ];
            SfxLibrary* pImplLib = getImplLib( aName );
            if( pImplLib->mbPreload )
            {
                loadLibrary( aName );
            }
        }
    }

    if( meInitMode == DEFAULT )
    {
        INetURLObject aUserBasicInetObj( maLibraryPath.getToken(1, (sal_Unicode)';') );
        OUString aStandardStr("Standard");

        INetURLObject aPrevUserBasicInetObj_1( aUserBasicInetObj );
        aPrevUserBasicInetObj_1.removeSegment();
        INetURLObject aPrevUserBasicInetObj_2 = aPrevUserBasicInetObj_1;
        aPrevUserBasicInetObj_1.Append( "__basic_80" );
        aPrevUserBasicInetObj_2.Append( "__basic_80_2" );

        // #i93163
        bool bCleanUp = false;
        try
        {
            INetURLObject aPrevUserBasicInetObj = aPrevUserBasicInetObj_1;
            OUString aPrevFolder = aPrevUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
            if( mxSFI->isFolder( aPrevFolder ) )
            {
                // Check if Standard folder exists and is complete
                INetURLObject aUserBasicStandardInetObj( aUserBasicInetObj );
                aUserBasicStandardInetObj.insertName( aStandardStr, true, INetURLObject::LAST_SEGMENT,
                                                      INetURLObject::ENCODE_ALL );
                INetURLObject aPrevUserBasicStandardInetObj( aPrevUserBasicInetObj );
                aPrevUserBasicStandardInetObj.insertName( aStandardStr, true, INetURLObject::LAST_SEGMENT,
                                                        INetURLObject::ENCODE_ALL );
                OUString aPrevStandardFolder = aPrevUserBasicStandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
                if( mxSFI->isFolder( aPrevStandardFolder ) )
                {
                    OUString aXlbExtension( "xlb" );
                    OUString aCheckFileName;

                    // Check if script.xlb exists
                    aCheckFileName = "script";
                    checkAndCopyFileImpl( aUserBasicStandardInetObj,
                                          aPrevUserBasicStandardInetObj,
                                          aCheckFileName, aXlbExtension, mxSFI );

                    // Check if dialog.xlb exists
                    aCheckFileName = "dialog";
                    checkAndCopyFileImpl( aUserBasicStandardInetObj,
                                          aPrevUserBasicStandardInetObj,
                                          aCheckFileName, aXlbExtension, mxSFI );

                    // Check if module1.xba exists
                    OUString aXbaExtension("xba");
                    aCheckFileName = "Module1";
                    checkAndCopyFileImpl( aUserBasicStandardInetObj,
                                          aPrevUserBasicStandardInetObj,
                                          aCheckFileName, aXbaExtension, mxSFI );
                }
                else
                {
                    OUString aStandardFolder = aUserBasicStandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
                    mxSFI->copy( aStandardFolder, aPrevStandardFolder );
                }

                OUString aPrevCopyToFolder = aPrevUserBasicInetObj_2.GetMainURL( INetURLObject::NO_DECODE );
                mxSFI->copy( aPrevFolder, aPrevCopyToFolder );
            }
            else
            {
                aPrevUserBasicInetObj = aPrevUserBasicInetObj_2;
                aPrevFolder = aPrevUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
            }
            if( mxSFI->isFolder( aPrevFolder ) )
            {
                SfxLibraryContainer* pPrevCont = createInstanceImpl();
                Reference< XInterface > xRef = static_cast< XInterface* >( static_cast< OWeakObject* >(pPrevCont) );

                // Rename previous basic folder to make storage URLs correct during initialisation
                OUString aFolderUserBasic = aUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
                INetURLObject aUserBasicTmpInetObj( aUserBasicInetObj );
                aUserBasicTmpInetObj.removeSegment();
                aUserBasicTmpInetObj.Append( "__basic_tmp" );
                OUString aFolderTmp = aUserBasicTmpInetObj.GetMainURL( INetURLObject::NO_DECODE );

                mxSFI->move( aFolderUserBasic, aFolderTmp );
                try
                {
                    mxSFI->move( aPrevFolder, aFolderUserBasic );
                }
                catch(const Exception& )
                {
                    // Move back user/basic folder
                    try
                    {
                           mxSFI->kill( aFolderUserBasic );
                    }
                    catch(const Exception& )
                    {}
                    mxSFI->move( aFolderTmp, aFolderUserBasic );
                    throw;
                }

                INetURLObject aPrevUserBasicLibInfoInetObj( aUserBasicInetObj );
                aPrevUserBasicLibInfoInetObj.insertName( maInfoFileName, false, INetURLObject::LAST_SEGMENT,
                                                    INetURLObject::ENCODE_ALL );
                aPrevUserBasicLibInfoInetObj.setExtension( "xlc");
                OUString aLibInfoFileName = aPrevUserBasicLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE );
                Sequence<Any> aInitSeq( 1 );
                aInitSeq.getArray()[0] <<= aLibInfoFileName;
                GbMigrationSuppressErrors = true;
                pPrevCont->initialize( aInitSeq );
                GbMigrationSuppressErrors = false;

                // Rename folders back
                mxSFI->move( aFolderUserBasic, aPrevFolder );
                mxSFI->move( aFolderTmp, aFolderUserBasic );

                OUString aUserSearchStr("vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE");
                OUString aSharedSearchStr("vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE");
                OUString aBundledSearchStr("vnd.sun.star.expand:$BUNDLED_EXTENSIONS");
                OUString aInstSearchStr("$(INST)");

                Sequence< OUString > aNames = pPrevCont->getElementNames();
                const OUString* pNames = aNames.getConstArray();
                sal_Int32 nNameCount = aNames.getLength();

                for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
                {
                    OUString aLibName = pNames[ i ];
                    if( hasByName( aLibName ) )
                    {
                        if( aLibName == aStandardStr )
                        {
                            SfxLibrary* pImplLib = getImplLib( aStandardStr );
                            OUString aStandardFolder = pImplLib->maStorageURL;
                            mxSFI->kill( aStandardFolder );
                        }
                        else
                        {
                            continue;
                        }
                    }

                    SfxLibrary* pImplLib = pPrevCont->getImplLib( aLibName );
                    if( pImplLib->mbLink )
                    {
                        OUString aStorageURL = pImplLib->maUnexpandedStorageURL;
                        bool bCreateLink = true;
                        if( aStorageURL.indexOf( aUserSearchStr   ) != -1 ||
                            aStorageURL.indexOf( aSharedSearchStr ) != -1 ||
                            aStorageURL.indexOf( aBundledSearchStr ) != -1 ||
                            aStorageURL.indexOf( aInstSearchStr   ) != -1 )
                        {
                            bCreateLink = false;
                        }
                        if( bCreateLink )
                        {
                            createLibraryLink( aLibName, pImplLib->maStorageURL, pImplLib->mbReadOnly );
                        }
                    }
                    else
                    {
                        // Move folder if not already done
                        INetURLObject aUserBasicLibFolderInetObj( aUserBasicInetObj );
                        aUserBasicLibFolderInetObj.Append( aLibName );
                        OUString aLibFolder = aUserBasicLibFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );

                        INetURLObject aPrevUserBasicLibFolderInetObj( aPrevUserBasicInetObj );
                        aPrevUserBasicLibFolderInetObj.Append( aLibName );
                        OUString aPrevLibFolder = aPrevUserBasicLibFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );

                        if( mxSFI->isFolder( aPrevLibFolder ) && !mxSFI->isFolder( aLibFolder ) )
                        {
                            mxSFI->move( aPrevLibFolder, aLibFolder );
                        }

                        if( aLibName == aStandardStr )
                        {
                            maNameContainer->removeByName( aLibName );
                        }

                        // Create library
                        Reference< XNameContainer > xLib = createLibrary( aLibName );
                           SfxLibrary* pNewLib = static_cast< SfxLibrary* >( xLib.get() );
                        pNewLib->mbLoaded = false;
                        pNewLib->implSetModified( false );
                        checkStorageURL( aLibFolder, pNewLib->maLibInfoFileURL,
                                         pNewLib->maStorageURL, pNewLib->maUnexpandedStorageURL );

                        uno::Reference< embed::XStorage > xDummyStor;
                        ::xmlscript::LibDescriptor aLibDesc;
                        implLoadLibraryIndexFile( pNewLib, aLibDesc, xDummyStor, pNewLib->maLibInfoFileURL );
                        implImportLibDescriptor( pNewLib, aLibDesc );
                    }
                }
                mxSFI->kill( aPrevFolder );
            }
        }
        catch(const Exception& e)
        {
            bCleanUp = true;
            SAL_WARN("basic", "Upgrade of Basic installation failed somehow: " << e.Message);
        }

        // #i93163
        if( bCleanUp )
        {
            INetURLObject aPrevUserBasicInetObj_Err( aUserBasicInetObj );
            aPrevUserBasicInetObj_Err.removeSegment();
            aPrevUserBasicInetObj_Err.Append( "__basic_80_err" );
            OUString aPrevFolder_Err = aPrevUserBasicInetObj_Err.GetMainURL( INetURLObject::NO_DECODE );

            bool bSaved = false;
            try
            {
                OUString aPrevFolder_1 = aPrevUserBasicInetObj_1.GetMainURL( INetURLObject::NO_DECODE );
                if( mxSFI->isFolder( aPrevFolder_1 ) )
                {
                    mxSFI->move( aPrevFolder_1, aPrevFolder_Err );
                    bSaved = true;
                }
            }
            catch(const Exception& )
            {}
            try
            {
                OUString aPrevFolder_2 = aPrevUserBasicInetObj_2.GetMainURL( INetURLObject::NO_DECODE );
                if( !bSaved && mxSFI->isFolder( aPrevFolder_2 ) )
                {
                    mxSFI->move( aPrevFolder_2, aPrevFolder_Err );
                }
                else
                {
                    mxSFI->kill( aPrevFolder_2 );
                }
            }
            catch(const Exception& )
            {}
        }
    }
}

void SfxLibraryContainer::implScanExtensions()
{
#if HAVE_FEATURE_EXTENSIONS
    ScriptExtensionIterator aScriptIt;
    OUString aLibURL;

    bool bPureDialogLib = false;
    while ( !(aLibURL = aScriptIt.nextBasicOrDialogLibrary( bPureDialogLib )).isEmpty())
    {
        if( bPureDialogLib && maInfoFileName == "script" )
        {
            continue;
        }
        // Extract lib name
        sal_Int32 nLen = aLibURL.getLength();
        sal_Int32 indexLastSlash = aLibURL.lastIndexOf( '/' );
        sal_Int32 nReduceCopy = 0;
        if( indexLastSlash == nLen - 1 )
        {
            nReduceCopy = 1;
            indexLastSlash = aLibURL.lastIndexOf( '/', nLen - 1 );
        }

        OUString aLibName = aLibURL.copy( indexLastSlash + 1, nLen - indexLastSlash - nReduceCopy - 1 );

        // If a library of the same exists the existing library wins
        if( hasByName( aLibName ) )
        {
            continue;
        }
        // Add index file to URL
        OUString aIndexFileURL = aLibURL;
        if( nReduceCopy == 0 )
        {
            aIndexFileURL += "/";
        }
        aIndexFileURL += maInfoFileName + ".xlb";

        // Create link
        const bool bReadOnly = false;
        Reference< XNameAccess > xLib = createLibraryLink( aLibName, aIndexFileURL, bReadOnly );
    }
#endif
}

// Handle maLibInfoFileURL and maStorageURL correctly
void SfxLibraryContainer::checkStorageURL( const OUString& aSourceURL,
                                           OUString& aLibInfoFileURL, OUString& aStorageURL,
                                           OUString& aUnexpandedStorageURL )
{
    OUString aExpandedSourceURL = expand_url( aSourceURL );
    if( aExpandedSourceURL != aSourceURL )
    {
        aUnexpandedStorageURL = aSourceURL;
    }
    INetURLObject aInetObj( aExpandedSourceURL );
    OUString aExtension = aInetObj.getExtension();
    if( aExtension == "xlb" )
    {
        // URL to xlb file
        aLibInfoFileURL = aExpandedSourceURL;
        aInetObj.removeSegment();
        aStorageURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
    }
    else
    {
        // URL to library folder
        aStorageURL = aExpandedSourceURL;
        aInetObj.insertName( maInfoFileName, false, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
        aInetObj.setExtension( "xlb" );
        aLibInfoFileURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
    }
}

SfxLibrary* SfxLibraryContainer::getImplLib( const OUString& rLibraryName )
{
    Any aLibAny = maNameContainer->getByName( rLibraryName ) ;
    Reference< XNameAccess > xNameAccess;
    aLibAny >>= xNameAccess;
    SfxLibrary* pImplLib = static_cast< SfxLibrary* >( xNameAccess.get() );
    return pImplLib;
}


// Storing with password encryption

// Empty implementation, avoids unnecessary implementation in dlgcont.cxx
bool SfxLibraryContainer::implStorePasswordLibrary( SfxLibrary*,
                                                    const OUString&,
                                                    const uno::Reference< embed::XStorage >&,
                                                    const uno::Reference< task::XInteractionHandler >&  )
{
    return false;
}

bool SfxLibraryContainer::implStorePasswordLibrary(
    SfxLibrary* /*pLib*/,
    const OUString& /*aName*/,
    const css::uno::Reference< css::embed::XStorage >& /*xStorage*/,
    const OUString& /*aTargetURL*/,
    const Reference< XSimpleFileAccess3 >& /*xToUseSFI*/,
    const uno::Reference< task::XInteractionHandler >&  )
{
    return false;
}

bool SfxLibraryContainer::implLoadPasswordLibrary(
    SfxLibrary* /*pLib*/,
    const OUString& /*Name*/,
    bool /*bVerifyPasswordOnly*/ )
throw(WrappedTargetException, RuntimeException, std::exception)
{
    return true;
}

OUString SfxLibraryContainer::createAppLibraryFolder( SfxLibrary* pLib, const OUString& aName )
{
    OUString aLibDirPath = pLib->maStorageURL;
    if( aLibDirPath.isEmpty() )
    {
        INetURLObject aInetObj( maLibraryPath.getToken(1, (sal_Unicode)';') );
        aInetObj.insertName( aName, true, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
        checkStorageURL( aInetObj.GetMainURL( INetURLObject::NO_DECODE ), pLib->maLibInfoFileURL,
                         pLib->maStorageURL, pLib->maUnexpandedStorageURL );
        aLibDirPath = pLib->maStorageURL;
    }

    if( !mxSFI->isFolder( aLibDirPath ) )
    {
        try
        {
            mxSFI->createFolder( aLibDirPath );
        }
        catch(const Exception& )
        {}
    }

    return aLibDirPath;
}

// Storing
void SfxLibraryContainer::implStoreLibrary( SfxLibrary* pLib,
                                            const OUString& aName,
                                            const uno::Reference< embed::XStorage >& xStorage )
{
    OUString aDummyLocation;
    Reference< XSimpleFileAccess3 > xDummySFA;
    Reference< XInteractionHandler > xDummyHandler;
    implStoreLibrary( pLib, aName, xStorage, aDummyLocation, xDummySFA, xDummyHandler );
}

// New variant for library export
void SfxLibraryContainer::implStoreLibrary( SfxLibrary* pLib,
                                            const OUString& aName,
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& aTargetURL,
                                            const Reference< XSimpleFileAccess3 >& rToUseSFI,
                                            const Reference< XInteractionHandler >& xHandler )
{
    bool bLink = pLib->mbLink;
    bool bStorage = xStorage.is() && !bLink;

    Sequence< OUString > aElementNames = pLib->getElementNames();
    sal_Int32 nNameCount = aElementNames.getLength();
    const OUString* pNames = aElementNames.getConstArray();

    if( bStorage )
    {
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];
            OUString aStreamName = aElementName + ".xml";

            if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
            {
                SAL_WARN(
                    "basic",
                    "invalid library element \"" << aElementName << '"');
                continue;
            }
            try
            {
                uno::Reference< io::XStream > xElementStream = xStorage->openStreamElement(
                                                                    aStreamName,
                                                                    embed::ElementModes::READWRITE );
                //    throw uno::RuntimeException(); // TODO: method must either return the stream or throw an exception

                OUString aMime( "text/xml" );

                uno::Reference< beans::XPropertySet > xProps( xElementStream, uno::UNO_QUERY );
                SAL_WARN_IF(
                    !xProps.is(), "basic",
                    "The StorageStream must implement XPropertySet interface!");
                //if ( !xProps.is() ) //TODO

                if ( xProps.is() )
                {
                    xProps->setPropertyValue("MediaType", uno::makeAny( aMime ) );

                    // #87671 Allow encryption
                    xProps->setPropertyValue("UseCommonStoragePasswordEncryption", uno::makeAny( true ) );

                    Reference< XOutputStream > xOutput = xElementStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOutput );
                }
            }
            catch(const uno::Exception& )
            {
                SAL_WARN("basic", "Problem during storing of library!");
                // TODO: error handling?
            }
        }
        pLib->storeResourcesToStorage( xStorage );
    }
    else
    {
        // Export?
        bool bExport = !aTargetURL.isEmpty();
        try
        {
            Reference< XSimpleFileAccess3 > xSFI = mxSFI;
            if( rToUseSFI.is() )
            {
                xSFI = rToUseSFI;
            }
            OUString aLibDirPath;
            if( bExport )
            {
                INetURLObject aInetObj( aTargetURL );
                aInetObj.insertName( aName, true, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
                aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

                if( !xSFI->isFolder( aLibDirPath ) )
                {
                    xSFI->createFolder( aLibDirPath );
                }
                pLib->storeResourcesToURL( aLibDirPath, xHandler );
            }
            else
            {
                aLibDirPath = createAppLibraryFolder( pLib, aName );
                pLib->storeResources();
            }

            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, false,
                                            INetURLObject::LAST_SEGMENT,
                                            INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( maLibElementFileExtension );
                OUString aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
                {
                    SAL_WARN(
                        "basic",
                        "invalid library element \"" << aElementName << '"');
                    continue;
                }

                // TODO: Check modified
                try
                {
                    if( xSFI->exists( aElementPath ) )
                    {
                        xSFI->kill( aElementPath );
                    }
                    Reference< XOutputStream > xOutput = xSFI->openFileWrite( aElementPath );
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOutput );
                    xOutput->closeOutput();
                }
                catch(const Exception& )
                {
                    if( bExport )
                    {
                        throw;
                    }
                    SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aElementPath );
                    ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                }
            }
        }
        catch(const Exception& )
        {
            if( bExport )
            {
                throw;
            }
        }
    }
}

void SfxLibraryContainer::implStoreLibraryIndexFile( SfxLibrary* pLib,
                                                     const ::xmlscript::LibDescriptor& rLib,
                                                     const uno::Reference< embed::XStorage >& xStorage )
{
    OUString aDummyLocation;
    Reference< XSimpleFileAccess3 > xDummySFA;
    implStoreLibraryIndexFile( pLib, rLib, xStorage, aDummyLocation, xDummySFA );
}

// New variant for library export
void SfxLibraryContainer::implStoreLibraryIndexFile( SfxLibrary* pLib,
                                                     const ::xmlscript::LibDescriptor& rLib,
                                                     const uno::Reference< embed::XStorage >& xStorage,
                                                     const OUString& aTargetURL,
                                                     const Reference< XSimpleFileAccess3 >& rToUseSFI )
{
    // Create sax writer
    Reference< XWriter > xWriter = xml::sax::Writer::create(mxContext);

    bool bLink = pLib->mbLink;
    bool bStorage = xStorage.is() && !bLink;

    // Write info file
    uno::Reference< io::XOutputStream > xOut;
    uno::Reference< io::XStream > xInfoStream;
    if( bStorage )
    {
        OUString aStreamName = maInfoFileName + "-lb.xml";

        try
        {
            xInfoStream = xStorage->openStreamElement( aStreamName, embed::ElementModes::READWRITE );
            SAL_WARN_IF(!xInfoStream.is(), "basic", "No stream!");
            uno::Reference< beans::XPropertySet > xProps( xInfoStream, uno::UNO_QUERY );
            //    throw uno::RuntimeException(); // TODO

            if ( xProps.is() )
            {
                OUString aMime("text/xml");
                xProps->setPropertyValue("MediaType", uno::makeAny( aMime ) );

                // #87671 Allow encryption
                xProps->setPropertyValue("UseCommonStoragePasswordEncryption", uno::makeAny( true ) );

                xOut = xInfoStream->getOutputStream();
            }
        }
        catch(const uno::Exception& )
        {
            SAL_WARN("basic", "Problem during storing of library index file!");
            // TODO: error handling?
        }
    }
    else
    {
        // Export?
        bool bExport = !aTargetURL.isEmpty();
        Reference< XSimpleFileAccess3 > xSFI = mxSFI;
        if( rToUseSFI.is() )
        {
            xSFI = rToUseSFI;
        }
        OUString aLibInfoPath;
        if( bExport )
        {
            INetURLObject aInetObj( aTargetURL );
            aInetObj.insertName( rLib.aName, true, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
            OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
            if( !xSFI->isFolder( aLibDirPath ) )
            {
                xSFI->createFolder( aLibDirPath );
            }
            aInetObj.insertName( maInfoFileName, false, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
            aInetObj.setExtension( "xlb" );
            aLibInfoPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
        }
        else
        {
            createAppLibraryFolder( pLib, rLib.aName );
            aLibInfoPath = pLib->maLibInfoFileURL;
        }

        try
        {
            if( xSFI->exists( aLibInfoPath ) )
            {
                xSFI->kill( aLibInfoPath );
            }
            xOut = xSFI->openFileWrite( aLibInfoPath );
        }
        catch(const Exception& )
        {
            if( bExport )
            {
                throw;
            }
            SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
            ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
        }
    }
    if( !xOut.is() )
    {
        SAL_WARN("basic", "couldn't open output stream");
        return;
    }
    xWriter->setOutputStream( xOut );
    xmlscript::exportLibrary( xWriter, rLib );
}


bool SfxLibraryContainer::implLoadLibraryIndexFile(  SfxLibrary* pLib,
                                                     ::xmlscript::LibDescriptor& rLib,
                                                     const uno::Reference< embed::XStorage >& xStorage,
                                                     const OUString& aIndexFileName )
{
    Reference< XParser > xParser = xml::sax::Parser::create(mxContext);

    bool bStorage = false;
    if( pLib )
    {
        bool bLink = pLib->mbLink;
        bStorage = xStorage.is() && !bLink;
    }

    // Read info file
    uno::Reference< io::XInputStream > xInput;
    OUString aLibInfoPath;
    if( bStorage )
    {
        aLibInfoPath = maInfoFileName + "-lb.xml";

        try
        {
            uno::Reference< io::XStream > xInfoStream =
                xStorage->openStreamElement( aLibInfoPath, embed::ElementModes::READ );
            xInput = xInfoStream->getInputStream();
        }
        catch(const uno::Exception& )
        {}
    }
    else
    {
        // Create Input stream
        //String aLibInfoPath; // attention: THIS PROBLEM MUST BE REVIEWED BY SCRIPTING OWNER!!!

        if( pLib )
        {
            createAppLibraryFolder( pLib, rLib.aName );
            aLibInfoPath = pLib->maLibInfoFileURL;
        }
        else
        {
            aLibInfoPath = aIndexFileName;
        }
        try
        {
            xInput = mxSFI->openFileRead( aLibInfoPath );
        }
        catch(const Exception& )
        {
            xInput.clear();
            if( !GbMigrationSuppressErrors )
            {
                SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
                ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
            }
        }
    }
    if( !xInput.is() )
    {
        return false;
    }

    InputSource source;
    source.aInputStream = xInput;
    source.sSystemId    = aLibInfoPath;

    // start parsing
    try
    {
        xParser->setDocumentHandler( ::xmlscript::importLibrary( rLib ) );
        xParser->parseStream( source );
    }
    catch(const Exception& )
    {
        SAL_WARN("basic", "Parsing error");
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
        ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
        return false;
    }

    if( !pLib )
    {
        Reference< XNameContainer > xLib = createLibrary( rLib.aName );
        pLib = static_cast< SfxLibrary* >( xLib.get() );
        pLib->mbLoaded = false;
        rLib.aStorageURL = aIndexFileName;
        checkStorageURL( rLib.aStorageURL, pLib->maLibInfoFileURL, pLib->maStorageURL,
                         pLib->maUnexpandedStorageURL );

        implImportLibDescriptor( pLib, rLib );
    }

    return true;
}

void SfxLibraryContainer::implImportLibDescriptor( SfxLibrary* pLib,
                                                   ::xmlscript::LibDescriptor& rLib )
{
    if( !pLib->mbInitialised )
    {
        sal_Int32 nElementCount = rLib.aElementNames.getLength();
        const OUString* pElementNames = rLib.aElementNames.getConstArray();
        Any aDummyElement = createEmptyLibraryElement();
        for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
        {
            pLib->maNameContainer->insertByName( pElementNames[i], aDummyElement );
        }
        pLib->mbPasswordProtected = rLib.bPasswordProtected;
        pLib->mbReadOnly = rLib.bReadOnly;
        pLib->mbPreload  = rLib.bPreload;
        pLib->implSetModified( false );
        pLib->mbInitialised = true;
    }
}


// Methods of new XLibraryStorage interface?
void SfxLibraryContainer::storeLibraries_Impl( const uno::Reference< embed::XStorage >& i_rStorage,
                                               bool bComplete )
{
    const Sequence< OUString > aNames = maNameContainer->getElementNames();
    const sal_Int32 nNameCount = aNames.getLength();
    const OUString* pName = aNames.getConstArray();
    const OUString* pNamesEnd = aNames.getConstArray() + nNameCount;

    // Don't count libs from shared index file
    sal_Int32 nLibsToSave = nNameCount;
    for( ; pName != pNamesEnd; ++pName )
    {
        SfxLibrary* pImplLib = getImplLib( *pName );
        if( pImplLib->mbSharedIndexFile || pImplLib->mbExtension )
        {
            nLibsToSave--;
        }
    }
    // Write to storage?
    bool bStorage = i_rStorage.is();
    uno::Reference< embed::XStorage > xSourceLibrariesStor;
    uno::Reference< embed::XStorage > xTargetLibrariesStor;
    OUString sTempTargetStorName;
    const bool bInplaceStorage = bStorage && ( i_rStorage == mxStorage );

    if( nLibsToSave == 0 )
    {
        if ( bInplaceStorage && mxStorage->hasByName(maLibrariesDir) )
        {
            mxStorage->removeElement(maLibrariesDir);
        }
        return;
    }

    if ( bStorage )
    {
        // Don't write if only empty standard lib exists
        if ( ( nLibsToSave == 1 ) && ( aNames[0] == "Standard" ) )
        {
            Any aLibAny = maNameContainer->getByName( aNames[0] );
            Reference< XNameAccess > xNameAccess;
            aLibAny >>= xNameAccess;
            if ( ! xNameAccess->hasElements() )
            {
                if ( bInplaceStorage && mxStorage->hasByName(maLibrariesDir) )
                {
                    mxStorage->removeElement(maLibrariesDir);
                }
                return;
            }
        }

        // create the empty target storage
        try
        {
            OUString sTargetLibrariesStoreName;
            if ( bInplaceStorage )
            {
                // create a temporary target storage
                const OUStringBuffer aTempTargetNameBase = maLibrariesDir + "_temp_";
                sal_Int32 index = 0;
                do
                {
                    OUStringBuffer aTempTargetName( aTempTargetNameBase );
                    aTempTargetName.append( index++ );

                    sTargetLibrariesStoreName = aTempTargetName.makeStringAndClear();
                    if ( !i_rStorage->hasByName( sTargetLibrariesStoreName ) )
                    {
                        break;
                    }
                }
                while ( true );
                sTempTargetStorName = sTargetLibrariesStoreName;
            }
            else
            {
                sTargetLibrariesStoreName = maLibrariesDir;
                if ( i_rStorage->hasByName( sTargetLibrariesStoreName ) )
                {
                    i_rStorage->removeElement( sTargetLibrariesStoreName );
                }
            }

            xTargetLibrariesStor.set( i_rStorage->openStorageElement( sTargetLibrariesStoreName, embed::ElementModes::READWRITE ), UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return;
        }

        // open the source storage which might be used to copy yet-unmodified libraries
        try
        {
            if ( mxStorage->hasByName( maLibrariesDir ) || bInplaceStorage )
            {
                xSourceLibrariesStor = mxStorage->openStorageElement( maLibrariesDir,
                                                   bInplaceStorage ? embed::ElementModes::READWRITE : embed::ElementModes::READ );
            }
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return;
        }
    }

    int iArray = 0;
    pName = aNames.getConstArray();
    ::xmlscript::LibDescriptor aLibDescriptorForExtensionLibs;
    std::unique_ptr< ::xmlscript::LibDescriptorArray > pLibArray(new ::xmlscript::LibDescriptorArray(nLibsToSave));
    for( ; pName != pNamesEnd; ++pName )
    {
        SfxLibrary* pImplLib = getImplLib( *pName );
        if( pImplLib->mbSharedIndexFile )
        {
            continue;
        }
        const bool bExtensionLib = pImplLib->mbExtension;
        ::xmlscript::LibDescriptor& rLib = bExtensionLib ?
              aLibDescriptorForExtensionLibs : pLibArray->mpLibs[iArray];
        if( !bExtensionLib )
        {
            iArray++;
        }
        rLib.aName = *pName;

        rLib.bLink = pImplLib->mbLink;
        if( !bStorage || pImplLib->mbLink )
        {
            rLib.aStorageURL = ( pImplLib->maUnexpandedStorageURL.getLength() ) ?
                pImplLib->maUnexpandedStorageURL : pImplLib->maLibInfoFileURL;
        }
        rLib.bReadOnly = pImplLib->mbReadOnly;
        rLib.bPreload = pImplLib->mbPreload;
        rLib.bPasswordProtected = pImplLib->mbPasswordProtected;
        rLib.aElementNames = pImplLib->getElementNames();

        if( pImplLib->implIsModified() || bComplete )
        {
// Testing pImplLib->implIsModified() is not reliable,
// IMHO the value of pImplLib->implIsModified() should
// reflect whether the library ( in-memory ) model
// is in sync with the library container's own storage. Currently
// whenever the library model is written to *any* storage
// pImplLib->implSetModified( sal_False ) is called
// The way the code works, especially the way that sfx uses
// temp storage when saving ( and later sets the root storage of the
// library container ) and similar madness in dbaccess means some surgery
// is required to make it possible to successfully use this optimisation
// It would be possible to do the implSetModified() call below only
// conditionally, but that would require an additional boolean to be
// passed in via the XStorageBasedDocument::storeLibrariesToStorage()...
// fdo#68983: If there's a password and the password is not known, only
// copying the storage works!
            // Can we simply copy the storage?
            bool isCopyStorage = !mbOldInfoFormat && !mbOasis2OOoFormat
                    && !pImplLib->isLoadedStorable()
                    && xSourceLibrariesStor.is() /* null for user profile */;
            if (isCopyStorage)
            {
                try
                {
                    (void)xSourceLibrariesStor->isStorageElement(rLib.aName);
                }
                catch (container::NoSuchElementException const&)
                {
                    isCopyStorage = false;
                }
            }
            if (isCopyStorage)
            {
                try
                {
                    xSourceLibrariesStor->copyElementTo( rLib.aName, xTargetLibrariesStor, rLib.aName );
                }
                catch( const uno::Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                    // TODO: error handling?
                }
            }
            else
            {
                uno::Reference< embed::XStorage > xLibraryStor;
                if( bStorage )
                {
                    try
                    {
                        xLibraryStor = xTargetLibrariesStor->openStorageElement(
                                                                        rLib.aName,
                                                                        embed::ElementModes::READWRITE );
                    }
                    catch(const uno::Exception& )
                    {
                        #if OSL_DEBUG_LEVEL > 0
                        Any aError( ::cppu::getCaughtException() );
                        SAL_WARN(
                            "basic",
                            "couldn't create sub storage for library \""
                                << rLib.aName << "\". Exception: "
                                << comphelper::anyToString(aError));
                        #endif
                        throw;
                    }
                }

                // Maybe lib is not loaded?!
                if( bComplete )
                {
                    loadLibrary( rLib.aName );
                }
                if( pImplLib->mbPasswordProtected )
                {
                    implStorePasswordLibrary( pImplLib, rLib.aName, xLibraryStor, uno::Reference< task::XInteractionHandler >() );
                    // TODO: Check return value
                }
                else
                {
                    implStoreLibrary( pImplLib, rLib.aName, xLibraryStor );
                }
                implStoreLibraryIndexFile( pImplLib, rLib, xLibraryStor );
                if( bStorage )
                {
                    try
                    {
                        uno::Reference< embed::XTransactedObject > xTransact( xLibraryStor, uno::UNO_QUERY_THROW );
                        xTransact->commit();
                    }
                    catch(const uno::Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                        // TODO: error handling
                        throw;
                    }
                }
            }
            maModifiable.setModified( true );
            pImplLib->implSetModified( false );
        }

        // For container info ReadOnly refers to mbReadOnlyLink
        rLib.bReadOnly = pImplLib->mbReadOnlyLink;
    }

    // if we did an in-place save into a storage (i.e. a save into the storage we were already based on),
    // then we need to clean up the temporary storage we used for this
    if ( bInplaceStorage && !sTempTargetStorName.isEmpty() )
    {
        SAL_WARN_IF(
            !xSourceLibrariesStor.is(), "basic",
            ("SfxLibrariesContainer::storeLibraries_impl: unexpected: we should"
             " have a source storage here!"));
        try
        {
            // for this, we first remove everything from the source storage, then copy the complete content
            // from the temporary target storage. From then on, what used to be the "source storage" becomes
            // the "target storage" for all subsequent operations.

            // (We cannot simply remove the storage, denoted by maLibrariesDir, from i_rStorage - there might be
            // open references to it.)

            if ( xSourceLibrariesStor.is() )
            {
                // remove
                const Sequence< OUString > aRemoveNames( xSourceLibrariesStor->getElementNames() );
                for ( const OUString* pRemoveName = aRemoveNames.getConstArray();
                      pRemoveName != aRemoveNames.getConstArray() + aRemoveNames.getLength();
                      ++pRemoveName
                    )
                {
                    xSourceLibrariesStor->removeElement( *pRemoveName );
                }

                // copy
                const Sequence< OUString > aCopyNames( xTargetLibrariesStor->getElementNames() );
                for ( const OUString* pCopyName = aCopyNames.getConstArray();
                      pCopyName != aCopyNames.getConstArray() + aCopyNames.getLength();
                      ++pCopyName
                    )
                {
                    xTargetLibrariesStor->copyElementTo( *pCopyName, xSourceLibrariesStor, *pCopyName );
                }
            }

            // close and remove temp target
            xTargetLibrariesStor->dispose();
            i_rStorage->removeElement( sTempTargetStorName );
            xTargetLibrariesStor.clear();
            sTempTargetStorName.clear();

            // adjust target
            xTargetLibrariesStor = xSourceLibrariesStor;
            xSourceLibrariesStor.clear();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            throw;
        }
    }

    if( !mbOldInfoFormat && !maModifiable.isModified() )
    {
        return;
    }
    maModifiable.setModified( false );
    mbOldInfoFormat = false;

    // Write library container info
    // Create sax writer
    Reference< XWriter > xWriter = xml::sax::Writer::create(mxContext);

    // Write info file
    uno::Reference< io::XOutputStream > xOut;
    uno::Reference< io::XStream > xInfoStream;
    if( bStorage )
    {
        OUString aStreamName = maInfoFileName + "-lc.xml";
        try
        {
            xInfoStream = xTargetLibrariesStor->openStreamElement( aStreamName, embed::ElementModes::READWRITE );
            uno::Reference< beans::XPropertySet > xProps( xInfoStream, uno::UNO_QUERY );
            SAL_WARN_IF(
                !xProps.is(), "basic",
                "The stream must implement XPropertySet!");
            if ( !xProps.is() )
            {
                throw uno::RuntimeException();
            }
            OUString aMime( "text/xml" );
            xProps->setPropertyValue("MediaType", uno::makeAny( aMime ) );

            // #87671 Allow encryption
            xProps->setPropertyValue("UseCommonStoragePasswordEncryption", uno::makeAny( true ) );

            xOut = xInfoStream->getOutputStream();
        }
        catch(const uno::Exception& )
        {
            ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
        }
    }
    else
    {
        // Create Output stream
        INetURLObject aLibInfoInetObj( maLibraryPath.getToken(1, (sal_Unicode)';') );
        aLibInfoInetObj.insertName( maInfoFileName, false, INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
        aLibInfoInetObj.setExtension( "xlc" );
        OUString aLibInfoPath( aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

        try
        {
            if( mxSFI->exists( aLibInfoPath ) )
            {
                mxSFI->kill( aLibInfoPath );
            }
            xOut = mxSFI->openFileWrite( aLibInfoPath );
        }
        catch(const Exception& )
        {
            xOut.clear();
            SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
            ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
        }

    }
    if( !xOut.is() )
    {
        SAL_WARN("basic", "couldn't open output stream");
        return;
    }

    xWriter->setOutputStream( xOut );

    try
    {
        xmlscript::exportLibraryContainer( xWriter, pLibArray.get() );
        if ( bStorage )
        {
            uno::Reference< embed::XTransactedObject > xTransact( xTargetLibrariesStor, uno::UNO_QUERY );
            SAL_WARN_IF(
                !xTransact.is(), "basic",
                "The storage must implement XTransactedObject!");
            if ( !xTransact.is() )
            {
                throw uno::RuntimeException();
            }
            xTransact->commit();
        }
    }
    catch(const uno::Exception& )
    {
        SAL_WARN("basic", "Problem during storing of libraries!");
        ErrorHandler::HandleError(  ERRCODE_IO_GENERAL );
    }
}


// Methods XElementAccess
Type SAL_CALL SfxLibraryContainer::getElementType()
    throw(RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return maNameContainer->getElementType();
}

sal_Bool SfxLibraryContainer::hasElements()
    throw(RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    bool bRet = maNameContainer->hasElements();
    return bRet;
}

// Methods XNameAccess
Any SfxLibraryContainer::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    Any aRetAny = maNameContainer->getByName( aName ) ;
    return aRetAny;
}

Sequence< OUString > SfxLibraryContainer::getElementNames()
    throw(RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return maNameContainer->getElementNames();
}

sal_Bool SfxLibraryContainer::hasByName( const OUString& aName )
    throw(RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return maNameContainer->hasByName( aName ) ;
}

// Methods XLibraryContainer
Reference< XNameContainer > SAL_CALL SfxLibraryContainer::createLibrary( const OUString& Name )
    throw(IllegalArgumentException, ElementExistException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pNewLib = implCreateLibrary( Name );
    pNewLib->maLibElementFileExtension = maLibElementFileExtension;

    createVariableURL( pNewLib->maUnexpandedStorageURL, Name, maInfoFileName, true );

    Reference< XNameAccess > xNameAccess = static_cast< XNameAccess* >( pNewLib );
    Any aElement;
    aElement <<= xNameAccess;
    maNameContainer->insertByName( Name, aElement );
    maModifiable.setModified( true );
    Reference< XNameContainer > xRet( xNameAccess, UNO_QUERY );
    return xRet;
}

Reference< XNameAccess > SAL_CALL SfxLibraryContainer::createLibraryLink
    ( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly )
    throw(IllegalArgumentException, ElementExistException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    // TODO: Check other reasons to force ReadOnly status
    //if( !ReadOnly )
    //{
    //}

    OUString aLibInfoFileURL;
    OUString aLibDirURL;
    OUString aUnexpandedStorageURL;
    checkStorageURL( StorageURL, aLibInfoFileURL, aLibDirURL, aUnexpandedStorageURL );


    SfxLibrary* pNewLib = implCreateLibraryLink( Name, aLibInfoFileURL, aLibDirURL, ReadOnly );
    pNewLib->maLibElementFileExtension = maLibElementFileExtension;
    pNewLib->maUnexpandedStorageURL = aUnexpandedStorageURL;
    pNewLib->maOriginalStorageURL = StorageURL;

    OUString aInitFileName;
    uno::Reference< embed::XStorage > xDummyStor;
    ::xmlscript::LibDescriptor aLibDesc;
    implLoadLibraryIndexFile( pNewLib, aLibDesc, xDummyStor, aInitFileName );
    implImportLibDescriptor( pNewLib, aLibDesc );

    Reference< XNameAccess > xRet = static_cast< XNameAccess* >( pNewLib );
    Any aElement;
    aElement <<= xRet;
    maNameContainer->insertByName( Name, aElement );
    maModifiable.setModified( true );

    OUString aUserSearchStr("vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE");
    OUString aSharedSearchStr("vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE");
    OUString aBundledSearchStr("vnd.sun.star.expand:$BUNDLED_EXTENSIONS");
    if( StorageURL.indexOf( aUserSearchStr ) != -1 )
    {
        pNewLib->mbExtension = true;
    }
    else if( StorageURL.indexOf( aSharedSearchStr ) != -1 || StorageURL.indexOf( aBundledSearchStr ) != -1 )
    {
        pNewLib->mbExtension = true;
        pNewLib->mbReadOnly = true;
    }

    return xRet;
}

void SAL_CALL SfxLibraryContainer::removeLibrary( const OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    // Get and hold library before removing
    Any aLibAny = maNameContainer->getByName( Name ) ;
    Reference< XNameAccess > xNameAccess;
    aLibAny >>= xNameAccess;
    SfxLibrary* pImplLib = static_cast< SfxLibrary* >( xNameAccess.get() );
    if( pImplLib->mbReadOnly && !pImplLib->mbLink )
    {
        throw IllegalArgumentException();
    }
    // Remove from container
    maNameContainer->removeByName( Name );
    maModifiable.setModified( true );

    // Delete library files, but not for linked libraries
    if( !pImplLib->mbLink )
    {
        if( mxStorage.is() )
        {
            return;
        }
        if( xNameAccess->hasElements() )
        {
            Sequence< OUString > aNames = pImplLib->getElementNames();
            sal_Int32 nNameCount = aNames.getLength();
            const OUString* pNames = aNames.getConstArray();
            for( sal_Int32 i = 0 ; i < nNameCount ; ++i, ++pNames )
            {
                pImplLib->removeElementWithoutChecks( *pNames, SfxLibrary::LibraryContainerAccess() );
            }
        }

        // Delete index file
        createAppLibraryFolder( pImplLib, Name );
        OUString aLibInfoPath = pImplLib->maLibInfoFileURL;
        try
        {
            if( mxSFI->exists( aLibInfoPath ) )
            {
                mxSFI->kill( aLibInfoPath );
            }
        }
        catch(const Exception& ) {}

        // Delete folder if empty
        INetURLObject aInetObj( maLibraryPath.getToken(1, (sal_Unicode)';') );
        aInetObj.insertName( Name, true, INetURLObject::LAST_SEGMENT,
                             INetURLObject::ENCODE_ALL );
        OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

        try
        {
            if( mxSFI->isFolder( aLibDirPath ) )
            {
                Sequence< OUString > aContentSeq = mxSFI->getFolderContents( aLibDirPath, true );
                sal_Int32 nCount = aContentSeq.getLength();
                if( !nCount )
                {
                    mxSFI->kill( aLibDirPath );
                }
            }
        }
        catch(const Exception& )
        {
        }
    }
}

sal_Bool SAL_CALL SfxLibraryContainer::isLibraryLoaded( const OUString& Name )
    throw(NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bRet = pImplLib->mbLoaded;
    return bRet;
}


void SAL_CALL SfxLibraryContainer::loadLibrary( const OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    Any aLibAny = maNameContainer->getByName( Name ) ;
    Reference< XNameAccess > xNameAccess;
    aLibAny >>= xNameAccess;
    SfxLibrary* pImplLib = static_cast< SfxLibrary* >( xNameAccess.get() );

    bool bLoaded = pImplLib->mbLoaded;
    pImplLib->mbLoaded = true;
    if( !bLoaded && xNameAccess->hasElements() )
    {
        if( pImplLib->mbPasswordProtected )
        {
            implLoadPasswordLibrary( pImplLib, Name );
            return;
        }

        bool bLink = pImplLib->mbLink;
        bool bStorage = mxStorage.is() && !bLink;

        uno::Reference< embed::XStorage > xLibrariesStor;
        uno::Reference< embed::XStorage > xLibraryStor;
        if( bStorage )
        {
            try
            {
                xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
                SAL_WARN_IF(
                    !xLibrariesStor.is(), "basic",
                    ("The method must either throw exception or return a"
                     " storage!"));
                if ( !xLibrariesStor.is() )
                {
                    throw uno::RuntimeException();
                }

                xLibraryStor = xLibrariesStor->openStorageElement( Name, embed::ElementModes::READ );
                SAL_WARN_IF(
                    !xLibraryStor.is(), "basic",
                    ("The method must either throw exception or return a"
                     " storage!"));
                if ( !xLibrariesStor.is() )
                {
                    throw uno::RuntimeException();
                }
            }
            catch(const uno::Exception& )
            {
            #if OSL_DEBUG_LEVEL > 0
                Any aError( ::cppu::getCaughtException() );
                SAL_WARN(
                    "basic",
                    "couldn't open sub storage for library \"" << Name
                        << "\". Exception: "
                        << comphelper::anyToString(aError));
            #endif
                throw;
            }
        }

        Sequence< OUString > aNames = pImplLib->getElementNames();
        sal_Int32 nNameCount = aNames.getLength();
        const OUString* pNames = aNames.getConstArray();
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            OUString aFile;
            uno::Reference< io::XInputStream > xInStream;

            if( bStorage )
            {
                uno::Reference< io::XStream > xElementStream;

                aFile = aElementName + ".xml";
                try
                {
                    xElementStream = xLibraryStor->openStreamElement( aFile, embed::ElementModes::READ );
                }
                catch(const uno::Exception& )
                {}

                if( !xElementStream.is() )
                {
                    // Check for EA2 document version with wrong extensions
                    aFile = aElementName + "." + maLibElementFileExtension;
                    try
                    {
                        xElementStream = xLibraryStor->openStreamElement( aFile, embed::ElementModes::READ );
                    }
                    catch(const uno::Exception& )
                    {}
                }

                if ( xElementStream.is() )
                {
                    xInStream = xElementStream->getInputStream();
                }
                if ( !xInStream.is() )
                {
                    SAL_WARN(
                        "basic",
                        "couldn't open library element stream - attempted to"
                            " open library \"" << Name << '"');
                    throw RuntimeException("couldn't open library element stream", *this);
                }
            }
            else
            {
                OUString aLibDirPath = pImplLib->maStorageURL;
                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, false,
                                            INetURLObject::LAST_SEGMENT,
                                            INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( maLibElementFileExtension );
                aFile = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );
            }

            Reference< XNameContainer > xLib( pImplLib );
            Any aAny = importLibraryElement( xLib, aElementName,
                                             aFile, xInStream );
            if( pImplLib->hasByName( aElementName ) )
            {
                if( aAny.hasValue() )
                {
                    pImplLib->maNameContainer->replaceByName( aElementName, aAny );
                }
            }
            else
            {
                pImplLib->maNameContainer->insertNoCheck(aElementName, aAny);
            }
        }
        pImplLib->implSetModified( false );
    }
}

// Methods XLibraryContainer2
sal_Bool SAL_CALL SfxLibraryContainer::isLibraryLink( const OUString& Name )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bRet = pImplLib->mbLink;
    return bRet;
}

OUString SAL_CALL SfxLibraryContainer::getLibraryLinkURL( const OUString& Name )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bLink = pImplLib->mbLink;
    if( !bLink )
    {
        throw IllegalArgumentException();
    }
    OUString aRetStr = pImplLib->maLibInfoFileURL;
    return aRetStr;
}

sal_Bool SAL_CALL SfxLibraryContainer::isLibraryReadOnly( const OUString& Name )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bRet = pImplLib->mbReadOnly || (pImplLib->mbLink && pImplLib->mbReadOnlyLink);
    return bRet;
}

void SAL_CALL SfxLibraryContainer::setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( pImplLib->mbLink )
    {
        if( pImplLib->mbReadOnlyLink != bool(bReadOnly) )
        {
            pImplLib->mbReadOnlyLink = bReadOnly;
            pImplLib->implSetModified( true );
            maModifiable.setModified( true );
        }
    }
    else
    {
        if( pImplLib->mbReadOnly != bool(bReadOnly) )
        {
            pImplLib->mbReadOnly = bReadOnly;
            pImplLib->implSetModified( true );
        }
    }
}

void SAL_CALL SfxLibraryContainer::renameLibrary( const OUString& Name, const OUString& NewName )
    throw (NoSuchElementException, ElementExistException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    if( maNameContainer->hasByName( NewName ) )
    {
        throw ElementExistException();
    }
    // Get and hold library before removing
    Any aLibAny = maNameContainer->getByName( Name ) ;

    // #i24094 Maybe lib is not loaded!
    Reference< XNameAccess > xNameAccess;
    aLibAny >>= xNameAccess;
    SfxLibrary* pImplLib = static_cast< SfxLibrary* >( xNameAccess.get() );
    if( pImplLib->mbPasswordProtected && !pImplLib->mbPasswordVerified )
    {
        return;     // Lib with unverified password cannot be renamed
    }
    loadLibrary( Name );

    // Remove from container
    maNameContainer->removeByName( Name );
    maModifiable.setModified( true );

    // Rename library folder, but not for linked libraries
    bool bMovedSuccessful = true;

    // Rename files
    bool bStorage = mxStorage.is();
    if( !bStorage && !pImplLib->mbLink )
    {
        bMovedSuccessful = false;

        OUString aLibDirPath = pImplLib->maStorageURL;

        INetURLObject aDestInetObj( maLibraryPath.getToken(1, (sal_Unicode)';'));
        aDestInetObj.insertName( NewName, true, INetURLObject::LAST_SEGMENT,
                                 INetURLObject::ENCODE_ALL );
        OUString aDestDirPath = aDestInetObj.GetMainURL( INetURLObject::NO_DECODE );

        // Store new URL
        OUString aLibInfoFileURL = pImplLib->maLibInfoFileURL;
        checkStorageURL( aDestDirPath, pImplLib->maLibInfoFileURL, pImplLib->maStorageURL,
                         pImplLib->maUnexpandedStorageURL );

        try
        {
            if( mxSFI->isFolder( aLibDirPath ) )
            {
                if( !mxSFI->isFolder( aDestDirPath ) )
                {
                    mxSFI->createFolder( aDestDirPath );
                }
                // Move index file
                try
                {
                    if( mxSFI->exists( pImplLib->maLibInfoFileURL ) )
                    {
                        mxSFI->kill( pImplLib->maLibInfoFileURL );
                    }
                    mxSFI->move( aLibInfoFileURL, pImplLib->maLibInfoFileURL );
                }
                catch(const Exception& )
                {
                }

                Sequence< OUString > aElementNames = xNameAccess->getElementNames();
                sal_Int32 nNameCount = aElementNames.getLength();
                const OUString* pNames = aElementNames.getConstArray();
                for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
                {
                    OUString aElementName = pNames[ i ];

                    INetURLObject aElementInetObj( aLibDirPath );
                    aElementInetObj.insertName( aElementName, false,
                        INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
                    aElementInetObj.setExtension( maLibElementFileExtension );
                    OUString aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                    INetURLObject aElementDestInetObj( aDestDirPath );
                    aElementDestInetObj.insertName( aElementName, false,
                                                    INetURLObject::LAST_SEGMENT,
                                                    INetURLObject::ENCODE_ALL );
                    aElementDestInetObj.setExtension( maLibElementFileExtension );
                    OUString aDestElementPath( aElementDestInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                    try
                    {
                        if( mxSFI->exists( aDestElementPath ) )
                        {
                            mxSFI->kill( aDestElementPath );
                        }
                        mxSFI->move( aElementPath, aDestElementPath );
                    }
                    catch(const Exception& )
                    {
                    }
                }
                pImplLib->storeResourcesAsURL( aDestDirPath, NewName );

                // Delete folder if empty
                Sequence< OUString > aContentSeq = mxSFI->getFolderContents( aLibDirPath, true );
                sal_Int32 nCount = aContentSeq.getLength();
                if( !nCount )
                {
                    mxSFI->kill( aLibDirPath );
                }

                bMovedSuccessful = true;
                pImplLib->implSetModified( true );
            }
        }
        catch(const Exception& )
        {
            // Restore old library
            maNameContainer->insertByName( Name, aLibAny ) ;
        }
    }

    if( bStorage && !pImplLib->mbLink )
    {
        pImplLib->implSetModified( true );
    }
    if( bMovedSuccessful )
    {
           maNameContainer->insertByName( NewName, aLibAny ) ;
    }
}


// Methods XInitialization
void SAL_CALL SfxLibraryContainer::initialize( const Sequence< Any >& _rArguments )
    throw (Exception, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    sal_Int32 nArgCount = _rArguments.getLength();
    if ( nArgCount == 1 )
    {
        OUString sInitialDocumentURL;
        Reference< XStorageBasedDocument > xDocument;
        if ( _rArguments[0] >>= sInitialDocumentURL )
        {
            initializeFromDocumentURL( sInitialDocumentURL );
            return;
        }

        if ( _rArguments[0] >>= xDocument )
        {
            initializeFromDocument( xDocument );
            return;
        }
    }

    throw IllegalArgumentException();
}

void SAL_CALL SfxLibraryContainer::initializeFromDocumentURL( const OUString& _rInitialDocumentURL )
{
    init( _rInitialDocumentURL, nullptr );
}

void SAL_CALL SfxLibraryContainer::initializeFromDocument( const Reference< XStorageBasedDocument >& _rxDocument )
{
    // check whether this is a valid OfficeDocument, and obtain the document's root storage
    Reference< XStorage > xDocStorage;
    try
    {
        Reference< XServiceInfo > xSI( _rxDocument, UNO_QUERY_THROW );
        if ( xSI->supportsService("com.sun.star.document.OfficeDocument"))
        {
            xDocStorage.set( _rxDocument->getDocumentStorage(), UNO_QUERY_THROW );
        }
        Reference< XModel > xDocument( _rxDocument, UNO_QUERY_THROW );
        Reference< XComponent > xDocComponent( _rxDocument, UNO_QUERY_THROW );

        mxOwnerDocument = xDocument;
        startComponentListening( xDocComponent );
    }
    catch( const Exception& ) { }

    if ( !xDocStorage.is() )
    {
        throw IllegalArgumentException();
    }
    init( OUString(), xDocStorage );
}

// OEventListenerAdapter
void SfxLibraryContainer::_disposing( const EventObject& _rSource )
{
#if OSL_DEBUG_LEVEL > 0
    Reference< XModel > xDocument( mxOwnerDocument.get(), UNO_QUERY );
    SAL_WARN_IF(
        xDocument != _rSource.Source || !xDocument.is(), "basic",
        "SfxLibraryContainer::_disposing: where does this come from?");
#else
    (void)_rSource;
#endif
    dispose();
}

// OComponentHelper
void SAL_CALL SfxLibraryContainer::disposing()
{
    Reference< XModel > xModel = mxOwnerDocument;
    EventObject aEvent( xModel.get() );
    maVBAScriptListeners.disposing( aEvent );
    stopAllComponentListening();
    mxOwnerDocument.clear();
}

// Methods XLibraryContainerPassword
sal_Bool SAL_CALL SfxLibraryContainer::isLibraryPasswordProtected( const OUString& )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return false;
}

sal_Bool SAL_CALL SfxLibraryContainer::isLibraryPasswordVerified( const OUString& )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    throw IllegalArgumentException();
}

sal_Bool SAL_CALL SfxLibraryContainer::verifyLibraryPassword( const OUString&, const OUString& )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    throw IllegalArgumentException();
}

void SAL_CALL SfxLibraryContainer::changeLibraryPassword(const OUString&, const OUString&, const OUString& )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    throw IllegalArgumentException();
}

// Methods XContainer
void SAL_CALL SfxLibraryContainer::addContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    maNameContainer->setEventSource( static_cast< XInterface* >( static_cast<OWeakObject*>(this) ) );
    maNameContainer->addContainerListener( xListener );
}

void SAL_CALL SfxLibraryContainer::removeContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    maNameContainer->removeContainerListener( xListener );
}

// Methods XLibraryContainerExport
void SAL_CALL SfxLibraryContainer::exportLibrary( const OUString& Name, const OUString& URL,
    const Reference< XInteractionHandler >& Handler )
    throw ( uno::Exception, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );

    Reference< XSimpleFileAccess3 > xToUseSFI;
    if( Handler.is() )
    {
        xToUseSFI = ucb::SimpleFileAccess::create( mxContext );
        xToUseSFI->setInteractionHandler( Handler );
    }

    // Maybe lib is not loaded?!
    loadLibrary( Name );

    uno::Reference< css::embed::XStorage > xDummyStor;
    if( pImplLib->mbPasswordProtected )
    {
        implStorePasswordLibrary( pImplLib, Name, xDummyStor, URL, xToUseSFI, Handler );
    }
    else
    {
        implStoreLibrary( pImplLib, Name, xDummyStor, URL, xToUseSFI, Handler );
    }
    ::xmlscript::LibDescriptor aLibDesc;
    aLibDesc.aName = Name;
    aLibDesc.bLink = false;             // Link status gets lost?
    aLibDesc.bReadOnly = pImplLib->mbReadOnly;
    aLibDesc.bPreload = false;          // Preload status gets lost?
    aLibDesc.bPasswordProtected = pImplLib->mbPasswordProtected;
    aLibDesc.aElementNames = pImplLib->getElementNames();

    implStoreLibraryIndexFile( pImplLib, aLibDesc, xDummyStor, URL, xToUseSFI );
}

OUString SfxLibraryContainer::expand_url( const OUString& url )
    throw(css::uno::RuntimeException)
{
    if (url.startsWithIgnoreAsciiCase( "vnd.sun.star.expand:" ))
    {
        return comphelper::getExpandedUri(mxContext, url);
    }
    else if( mxStringSubstitution.is() )
    {
        OUString ret( mxStringSubstitution->substituteVariables( url, false ) );
        return ret;
    }
    else
    {
        return url;
    }
}

//XLibraryContainer3
OUString SAL_CALL SfxLibraryContainer::getOriginalLibraryLinkURL( const OUString& Name )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bLink = pImplLib->mbLink;
    if( !bLink )
    {
        throw IllegalArgumentException();
    }
    OUString aRetStr = pImplLib->maOriginalStorageURL;
    return aRetStr;
}


// XVBACompatibility
sal_Bool SAL_CALL SfxLibraryContainer::getVBACompatibilityMode() throw (RuntimeException, std::exception)
{
    return mbVBACompat;
}

void SAL_CALL SfxLibraryContainer::setVBACompatibilityMode( sal_Bool _vbacompatmodeon ) throw (RuntimeException, std::exception)
{
    /*  The member variable mbVBACompat must be set first, the following call
        to getBasicManager() may call getVBACompatibilityMode() which returns
        this value. */
    mbVBACompat = _vbacompatmodeon;
    if( BasicManager* pBasMgr = getBasicManager() )
    {
        // get the standard library
        OUString aLibName = pBasMgr->GetName();
        if ( aLibName.isEmpty())
        {
            aLibName = "Standard";
        }
        if( StarBASIC* pBasic = pBasMgr->GetLib( aLibName ) )
        {
            pBasic->SetVBAEnabled( _vbacompatmodeon );
        }
        /*  If in VBA compatibility mode, force creation of the VBA Globals
            object. Each application will create an instance of its own
            implementation and store it in its Basic manager. Implementations
            will do all necessary additional initialization, such as
            registering the global "This***Doc" UNO constant, starting the
            document events processor etc.
         */
        if( mbVBACompat ) try
        {
            Reference< XModel > xModel( mxOwnerDocument );   // weak-ref -> ref
            Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
            xFactory->createInstance("ooo.vba.VBAGlobals");
        }
        catch(const Exception& )
        {
        }
    }
}

void SAL_CALL SfxLibraryContainer::setProjectName( const OUString& _projectname ) throw (RuntimeException, std::exception)
{
    msProjectName = _projectname;
    BasicManager* pBasMgr = getBasicManager();
    // Temporary HACK
    // Some parts of the VBA handling ( e.g. in core basic )
    // code expect the name of the VBA project to be set as the name of
    // the basic manager. Provide fail back here.
    if( pBasMgr )
    {
        pBasMgr->SetName( msProjectName );
    }
}

sal_Int32 SAL_CALL SfxLibraryContainer::getRunningVBAScripts() throw (RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    return mnRunningVBAScripts;
}

void SAL_CALL SfxLibraryContainer::addVBAScriptListener( const Reference< vba::XVBAScriptListener >& rxListener ) throw (RuntimeException, std::exception)
{
    maVBAScriptListeners.addTypedListener( rxListener );
}

void SAL_CALL SfxLibraryContainer::removeVBAScriptListener( const Reference< vba::XVBAScriptListener >& rxListener ) throw (RuntimeException, std::exception)
{
    maVBAScriptListeners.removeTypedListener( rxListener );
}

void SAL_CALL SfxLibraryContainer::broadcastVBAScriptEvent( sal_Int32 nIdentifier, const OUString& rModuleName ) throw (RuntimeException, std::exception)
{
    // own lock for accessing the number of running scripts
    enterMethod();
    switch( nIdentifier )
    {
    case vba::VBAScriptEventId::SCRIPT_STARTED:
        ++mnRunningVBAScripts;
        break;
    case vba::VBAScriptEventId::SCRIPT_STOPPED:
        --mnRunningVBAScripts;
        break;
    }
    leaveMethod();

    Reference< XModel > xModel = mxOwnerDocument;  // weak-ref -> ref
    vba::VBAScriptEvent aEvent( Reference<XInterface>(xModel, UNO_QUERY), nIdentifier, rModuleName );
    maVBAScriptListeners.notify( aEvent );
}

// Methods XServiceInfo
sal_Bool SAL_CALL SfxLibraryContainer::supportsService( const OUString& _rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

// Implementation class SfxLibrary

// Ctor
SfxLibrary::SfxLibrary( ModifiableHelper& _rModifiable, const Type& aType,
    const Reference< XSimpleFileAccess3 >& xSFI )
        : OComponentHelper( m_aMutex )
        , mxSFI( xSFI )
        , mrModifiable( _rModifiable )
        , maNameContainer( new NameContainer(aType) )
        , mbLoaded( true )
        , mbIsModified( true )
        , mbInitialised( false )
        , mbLink( false )
        , mbReadOnly( false )
        , mbReadOnlyLink( false )
        , mbPreload( false )
        , mbPasswordProtected( false )
        , mbPasswordVerified( false )
        , mbDoc50Password( false )
        , mbSharedIndexFile( false )
        , mbExtension( false )
{
}

SfxLibrary::SfxLibrary( ModifiableHelper& _rModifiable, const Type& aType,
    const Reference< XSimpleFileAccess3 >& xSFI,
    const OUString& aLibInfoFileURL, const OUString& aStorageURL, bool ReadOnly )
        : OComponentHelper( m_aMutex )
        , mxSFI( xSFI )
        , mrModifiable( _rModifiable )
        , maNameContainer( new NameContainer(aType) )
        , mbLoaded( false )
        , mbIsModified( true )
        , mbInitialised( false )
        , maLibInfoFileURL( aLibInfoFileURL )
        , maStorageURL( aStorageURL )
        , mbLink( true )
        , mbReadOnly( false )
        , mbReadOnlyLink( ReadOnly )
        , mbPreload( false )
        , mbPasswordProtected( false )
        , mbPasswordVerified( false )
        , mbDoc50Password( false )
        , mbSharedIndexFile( false )
        , mbExtension( false )
{
}

bool SfxLibrary::isLoadedStorable()
{
    return mbLoaded && (!mbPasswordProtected || mbPasswordVerified);
}

void SfxLibrary::implSetModified( bool _bIsModified )
{
    if ( mbIsModified == _bIsModified )
    {
        return;
    }
    mbIsModified = _bIsModified;
    if ( mbIsModified )
    {
        mrModifiable.setModified( true );
    }
}

// Methods XInterface
Any SAL_CALL SfxLibrary::queryInterface( const Type& rType )
    throw( RuntimeException, std::exception )
{
    Any aRet;

    aRet = Any(
        ::cppu::queryInterface(
            rType,
            static_cast< XContainer * >( this ),
            static_cast< XNameContainer * >( this ),
            static_cast< XNameAccess * >( this ),
            static_cast< XElementAccess * >( this ),
            static_cast< XChangesNotifier * >( this ) ) );
    if( !aRet.hasValue() )
    {
        aRet = OComponentHelper::queryInterface( rType );
    }
    return aRet;
}

// Methods XElementAccess
Type SfxLibrary::getElementType()
    throw(RuntimeException, std::exception)
{
    return maNameContainer->getElementType();
}

sal_Bool SfxLibrary::hasElements()
    throw(RuntimeException, std::exception)
{
    bool bRet = maNameContainer->hasElements();
    return bRet;
}

// Methods XNameAccess
Any SfxLibrary::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    impl_checkLoaded();

    Any aRetAny = maNameContainer->getByName( aName ) ;
    return aRetAny;
}

Sequence< OUString > SfxLibrary::getElementNames()
    throw(RuntimeException, std::exception)
{
    return maNameContainer->getElementNames();
}

sal_Bool SfxLibrary::hasByName( const OUString& aName )
    throw(RuntimeException, std::exception)
{
    bool bRet = maNameContainer->hasByName( aName );
    return bRet;
}

void SfxLibrary::impl_checkReadOnly()
{
    if( mbReadOnly || (mbLink && mbReadOnlyLink) )
    {
        throw IllegalArgumentException(
            "Library is readonly.",
            // TODO: resource
            *this, 0
        );
    }
}

void SfxLibrary::impl_checkLoaded()
{
    if ( !mbLoaded )
    {
        throw WrappedTargetException(
            OUString(),
            *this,
            makeAny( LibraryNotLoadedException(
                OUString(),
                *this
            ) )
        );
    }
}

// Methods XNameReplace
void SfxLibrary::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    impl_checkReadOnly();
    impl_checkLoaded();

    SAL_WARN_IF(
        !isLibraryElementValid(aElement), "basic",
        "SfxLibrary::replaceByName: replacing element is invalid!");

    maNameContainer->replaceByName( aName, aElement );
    implSetModified( true );
}


// Methods XNameContainer
void SfxLibrary::insertByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    impl_checkReadOnly();
    impl_checkLoaded();

    SAL_WARN_IF(
        !isLibraryElementValid(aElement), "basic",
        "SfxLibrary::insertByName: to-be-inserted element is invalid!");

    maNameContainer->insertByName( aName, aElement );
    implSetModified( true );
}

void SfxLibrary::impl_removeWithoutChecks( const OUString& _rElementName )
{
    maNameContainer->removeByName( _rElementName );
    implSetModified( true );

    // Remove element file
    if( !maStorageURL.isEmpty() )
    {
        INetURLObject aElementInetObj( maStorageURL );
        aElementInetObj.insertName( _rElementName, false,
                                    INetURLObject::LAST_SEGMENT,
                                    INetURLObject::ENCODE_ALL );
        aElementInetObj.setExtension( maLibElementFileExtension );
        OUString aFile = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

        try
        {
            if( mxSFI->exists( aFile ) )
            {
                mxSFI->kill( aFile );
            }
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void SfxLibrary::removeByName( const OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    impl_checkReadOnly();
    impl_checkLoaded();
    impl_removeWithoutChecks( Name );
}

// XTypeProvider
Sequence< Type > SfxLibrary::getTypes()
    throw( RuntimeException, std::exception )
{
    static OTypeCollection * s_pTypes_NameContainer = nullptr;
    if( !s_pTypes_NameContainer )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !s_pTypes_NameContainer )
        {
            static OTypeCollection s_aTypes_NameContainer(
                cppu::UnoType<XNameContainer>::get(),
                cppu::UnoType<XContainer>::get(),
                cppu::UnoType<XChangesNotifier>::get(),
                OComponentHelper::getTypes() );
            s_pTypes_NameContainer = &s_aTypes_NameContainer;
        }
    }
    return s_pTypes_NameContainer->getTypes();
}


Sequence< sal_Int8 > SfxLibrary::getImplementationId()
    throw( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// Methods XContainer
void SAL_CALL SfxLibrary::addContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    maNameContainer->setEventSource( static_cast< XInterface* >( static_cast<OWeakObject*>(this) ) );
    maNameContainer->addContainerListener( xListener );
}

void SAL_CALL SfxLibrary::removeContainerListener( const Reference< XContainerListener >& xListener )
    throw (RuntimeException, std::exception)
{
    maNameContainer->removeContainerListener( xListener );
}

// Methods XChangesNotifier
void SAL_CALL SfxLibrary::addChangesListener( const Reference< XChangesListener >& xListener )
    throw (RuntimeException, std::exception)
{
    maNameContainer->setEventSource( static_cast< XInterface* >( static_cast<OWeakObject*>(this) ) );
    maNameContainer->addChangesListener( xListener );
}

void SAL_CALL SfxLibrary::removeChangesListener( const Reference< XChangesListener >& xListener )
    throw (RuntimeException, std::exception)
{
    maNameContainer->removeChangesListener( xListener );
}


// Implementation class ScriptExtensionIterator

#define sBasicLibMediaType "application/vnd.sun.star.basic-library"
#define sDialogLibMediaType "application/vnd.sun.star.dialog-library"

ScriptExtensionIterator::ScriptExtensionIterator()
    : m_xContext( comphelper::getProcessComponentContext() )
    , m_eState( USER_EXTENSIONS )
    , m_bUserPackagesLoaded( false )
    , m_bSharedPackagesLoaded( false )
    , m_bBundledPackagesLoaded( false )
    , m_iUserPackage( 0 )
    , m_iSharedPackage( 0 )
       , m_iBundledPackage( 0 )
    , m_pScriptSubPackageIterator( nullptr )
{}

OUString ScriptExtensionIterator::nextBasicOrDialogLibrary( bool& rbPureDialogLib )
{
    OUString aRetLib;

    while( aRetLib.isEmpty() && m_eState != END_REACHED )
    {
        switch( m_eState )
        {
            case USER_EXTENSIONS:
            {
                Reference< deployment::XPackage > xScriptPackage =
                    implGetNextUserScriptPackage( rbPureDialogLib );
                if( !xScriptPackage.is() )
                {
                    break;
                }
                aRetLib = xScriptPackage->getURL();
                break;
            }

            case SHARED_EXTENSIONS:
            {
                Reference< deployment::XPackage > xScriptPackage =
                    implGetNextSharedScriptPackage( rbPureDialogLib );
                if( !xScriptPackage.is() )
                {
                    break;
                }
                aRetLib = xScriptPackage->getURL();
                break;
            }
            case BUNDLED_EXTENSIONS:
            {
                Reference< deployment::XPackage > xScriptPackage =
                    implGetNextBundledScriptPackage( rbPureDialogLib );
                if( !xScriptPackage.is() )
                {
                    break;
                }
                aRetLib = xScriptPackage->getURL();
                break;
            }
            case END_REACHED:
                SAL_WARN(
                    "basic",
                    ("ScriptExtensionIterator::nextBasicOrDialogLibrary():"
                     " Invalid case END_REACHED"));
                break;
        }
    }

    return aRetLib;
}

ScriptSubPackageIterator::ScriptSubPackageIterator( Reference< deployment::XPackage > const & xMainPackage )
    : m_xMainPackage( xMainPackage )
    , m_bIsValid( false )
    , m_bIsBundle( false )
    , m_nSubPkgCount( 0 )
    , m_iNextSubPkg( 0 )
{
    Reference< deployment::XPackage > xScriptPackage;
    if( !m_xMainPackage.is() )
    {
        return;
    }
    // Check if parent package is registered
    beans::Optional< beans::Ambiguous<sal_Bool> > option( m_xMainPackage->isRegistered
        ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() ) );
    bool bRegistered = false;
    if( option.IsPresent )
    {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if( !reg.IsAmbiguous && reg.Value )
        {
            bRegistered = true;
        }
    }
    if( bRegistered )
    {
        m_bIsValid = true;
        if( m_xMainPackage->isBundle() )
        {
            m_bIsBundle = true;
            m_aSubPkgSeq = m_xMainPackage->getBundle( Reference<task::XAbortChannel>(),
                                                      Reference<ucb::XCommandEnvironment>() );
            m_nSubPkgCount = m_aSubPkgSeq.getLength();
        }
    }
}

Reference< deployment::XPackage > ScriptSubPackageIterator::getNextScriptSubPackage( bool& rbPureDialogLib )
{
    rbPureDialogLib = false;

    Reference< deployment::XPackage > xScriptPackage;
    if( !m_bIsValid )
    {
        return xScriptPackage;
    }
    if( m_bIsBundle )
    {
        const Reference< deployment::XPackage >* pSeq = m_aSubPkgSeq.getConstArray();
        sal_Int32 iPkg;
        for( iPkg = m_iNextSubPkg ; iPkg < m_nSubPkgCount ; ++iPkg )
        {
            const Reference< deployment::XPackage > xSubPkg = pSeq[ iPkg ];
            xScriptPackage = implDetectScriptPackage( xSubPkg, rbPureDialogLib );
            if( xScriptPackage.is() )
            {
                break;
            }
        }
        m_iNextSubPkg = iPkg + 1;
    }
    else
    {
        xScriptPackage = implDetectScriptPackage( m_xMainPackage, rbPureDialogLib );
        m_bIsValid = false;     // No more script packages
    }

    return xScriptPackage;
}

Reference< deployment::XPackage > ScriptSubPackageIterator::implDetectScriptPackage ( const Reference< deployment::XPackage >& rPackage,
                                                                                      bool& rbPureDialogLib )
{
    Reference< deployment::XPackage > xScriptPackage;

    if( rPackage.is() )
    {
        const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = rPackage->getPackageType();
        OUString aMediaType = xPackageTypeInfo->getMediaType();
        if ( aMediaType == sBasicLibMediaType )
        {
            xScriptPackage = rPackage;
        }
        else if ( aMediaType == sDialogLibMediaType )
        {
            rbPureDialogLib = true;
            xScriptPackage = rPackage;
        }
    }

    return xScriptPackage;
}

Reference< deployment::XPackage > ScriptExtensionIterator::implGetNextUserScriptPackage( bool& rbPureDialogLib )
{
    Reference< deployment::XPackage > xScriptPackage;

    if( !m_bUserPackagesLoaded )
    {
        try
        {
            Reference< XExtensionManager > xManager = ExtensionManager::get( m_xContext );
            m_aUserPackagesSeq = xManager->getDeployedExtensions("user",
                                                                 Reference< task::XAbortChannel >(),
                                                                 Reference< ucb::XCommandEnvironment >() );
        }
        catch(const css::uno::DeploymentException& )
        {
            // Special Office installations may not contain deployment code
            m_eState = END_REACHED;
            return xScriptPackage;
        }

        m_bUserPackagesLoaded = true;
    }

    if( m_iUserPackage == m_aUserPackagesSeq.getLength() )
    {
        m_eState = SHARED_EXTENSIONS;       // Later: SHARED_MODULE
    }
    else
    {
        if( m_pScriptSubPackageIterator == nullptr )
        {
            const Reference< deployment::XPackage >* pUserPackages = m_aUserPackagesSeq.getConstArray();
            Reference< deployment::XPackage > xPackage = pUserPackages[ m_iUserPackage ];
            SAL_WARN_IF(
                !xPackage.is(), "basic",
                ("ScriptExtensionIterator::implGetNextUserScriptPackage():"
                 " Invalid package"));
            m_pScriptSubPackageIterator = new ScriptSubPackageIterator( xPackage );
        }

        if( m_pScriptSubPackageIterator != nullptr )
        {
            xScriptPackage = m_pScriptSubPackageIterator->getNextScriptSubPackage( rbPureDialogLib );
            if( !xScriptPackage.is() )
            {
                delete m_pScriptSubPackageIterator;
                m_pScriptSubPackageIterator = nullptr;
                m_iUserPackage++;
            }
        }
    }

    return xScriptPackage;
}

Reference< deployment::XPackage > ScriptExtensionIterator::implGetNextSharedScriptPackage( bool& rbPureDialogLib )
{
    Reference< deployment::XPackage > xScriptPackage;

    if( !m_bSharedPackagesLoaded )
    {
        try
        {
            Reference< XExtensionManager > xSharedManager = ExtensionManager::get( m_xContext );
            m_aSharedPackagesSeq = xSharedManager->getDeployedExtensions("shared",
                                                                         Reference< task::XAbortChannel >(),
                                                                         Reference< ucb::XCommandEnvironment >() );
        }
        catch(const css::uno::DeploymentException& )
        {
            // Special Office installations may not contain deployment code
            return xScriptPackage;
        }

        m_bSharedPackagesLoaded = true;
    }

    if( m_iSharedPackage == m_aSharedPackagesSeq.getLength() )
    {
        m_eState = BUNDLED_EXTENSIONS;
    }
    else
    {
        if( m_pScriptSubPackageIterator == nullptr )
        {
            const Reference< deployment::XPackage >* pSharedPackages = m_aSharedPackagesSeq.getConstArray();
            Reference< deployment::XPackage > xPackage = pSharedPackages[ m_iSharedPackage ];
            SAL_WARN_IF(
                !xPackage.is(), "basic",
                ("ScriptExtensionIterator::implGetNextSharedScriptPackage():"
                 " Invalid package"));
            m_pScriptSubPackageIterator = new ScriptSubPackageIterator( xPackage );
        }

        if( m_pScriptSubPackageIterator != nullptr )
        {
            xScriptPackage = m_pScriptSubPackageIterator->getNextScriptSubPackage( rbPureDialogLib );
            if( !xScriptPackage.is() )
            {
                delete m_pScriptSubPackageIterator;
                m_pScriptSubPackageIterator = nullptr;
                m_iSharedPackage++;
            }
        }
    }

    return xScriptPackage;
}

Reference< deployment::XPackage > ScriptExtensionIterator::implGetNextBundledScriptPackage( bool& rbPureDialogLib )
{
    Reference< deployment::XPackage > xScriptPackage;

    if( !m_bBundledPackagesLoaded )
    {
        try
        {
            Reference< XExtensionManager > xManager = ExtensionManager::get( m_xContext );
            m_aBundledPackagesSeq = xManager->getDeployedExtensions("bundled",
                                                                    Reference< task::XAbortChannel >(),
                                                                    Reference< ucb::XCommandEnvironment >() );
        }
        catch(const css::uno::DeploymentException& )
        {
            // Special Office installations may not contain deployment code
            return xScriptPackage;
        }

        m_bBundledPackagesLoaded = true;
    }

    if( m_iBundledPackage == m_aBundledPackagesSeq.getLength() )
    {
        m_eState = END_REACHED;
    }
    else
    {
        if( m_pScriptSubPackageIterator == nullptr )
        {
            const Reference< deployment::XPackage >* pBundledPackages = m_aBundledPackagesSeq.getConstArray();
            Reference< deployment::XPackage > xPackage = pBundledPackages[ m_iBundledPackage ];
            SAL_WARN_IF(
                !xPackage.is(), "basic",
                ("ScriptExtensionIterator::implGetNextBundledScriptPackage():"
                 " Invalid package"));
            m_pScriptSubPackageIterator = new ScriptSubPackageIterator( xPackage );
        }

        if( m_pScriptSubPackageIterator != nullptr )
        {
            xScriptPackage = m_pScriptSubPackageIterator->getNextScriptSubPackage( rbPureDialogLib );
            if( !xScriptPackage.is() )
            {
                delete m_pScriptSubPackageIterator;
                m_pScriptSubPackageIterator = nullptr;
                m_iBundledPackage++;
            }
        }
    }

    return xScriptPackage;
}

}   // namespace basic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
