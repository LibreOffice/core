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

#include "scriptcont.hxx"
#include <filefmt.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <rtl/digest.h>
#include <rtl/strbuf.hxx>

// For password functionality
#include <tools/urlobj.hxx>


#include <unotools/pathoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/basicmanagerrepository.hxx>
#include <basic/modsizeexceeded.hxx>
#include <xmlscript/xmlmod_imexp.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/script/XLibraryQueryExecutable.hpp>
#include <memory>

namespace basic
{

using namespace com::sun::star::document;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star;
using namespace cppu;
using namespace osl;


// Implementation class SfxScriptLibraryContainer

const sal_Char* SAL_CALL SfxScriptLibraryContainer::getInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getOldInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibElementFileExtension() const { return "xba"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibrariesDir() const { return "Basic"; }

// OldBasicPassword interface
void SfxScriptLibraryContainer::setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword )
{
    try
    {
        SfxLibrary* pImplLib = getImplLib( rLibraryName );
        if( !rPassword.isEmpty() )
        {
            pImplLib->mbDoc50Password = true;
            pImplLib->mbPasswordProtected = true;
            pImplLib->maPassword = rPassword;
            SfxScriptLibrary *const pSL(dynamic_cast<SfxScriptLibrary *>(pImplLib));
            if (pSL && pSL->mbLoaded)
            {
                pSL->mbLoadedSource = true; // must store source code now!
            }
        }
    }
    catch(const NoSuchElementException& ) {}
}

// Ctor for service
SfxScriptLibraryContainer::SfxScriptLibraryContainer()
    :maScriptLanguage( "StarBasic"  )
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

SfxScriptLibraryContainer::SfxScriptLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
    :maScriptLanguage( "StarBasic"  )
{
    init( OUString(), xStorage );
}

// Methods to get library instances of the correct type
SfxLibrary* SfxScriptLibraryContainer::implCreateLibrary( const OUString& aName )
{
    (void)aName;    // Only needed for SfxDialogLibrary
    SfxLibrary* pRet = new SfxScriptLibrary( maModifiable, mxSFI );
    return pRet;
}

SfxLibrary* SfxScriptLibraryContainer::implCreateLibraryLink( const OUString& aName,
                                                              const OUString& aLibInfoFileURL,
                                                              const OUString& StorageURL,
                                                              bool ReadOnly )
{
    (void)aName;    // Only needed for SfxDialogLibrary
    SfxLibrary* pRet = new SfxScriptLibrary( maModifiable, mxSFI,
                                             aLibInfoFileURL, StorageURL, ReadOnly );
    return pRet;
}

Any SAL_CALL SfxScriptLibraryContainer::createEmptyLibraryElement()
{
    OUString aMod;
    Any aRetAny;
    aRetAny <<= aMod;
    return aRetAny;
}

bool SAL_CALL SfxScriptLibraryContainer::isLibraryElementValid(const Any& rElement) const
{
    return SfxScriptLibrary::containsValidModule(rElement);
}

void SAL_CALL SfxScriptLibraryContainer::writeLibraryElement( const Reference < XNameContainer >& xLib,
                                                              const OUString& aElementName,
                                                              const Reference< XOutputStream >& xOutput)
    throw(Exception)
{
    // Create sax writer
    Reference< XWriter > xWriter = xml::sax::Writer::create(mxContext);

    Reference< XTruncate > xTruncate( xOutput, UNO_QUERY );
    OSL_ENSURE( xTruncate.is(), "Currently only the streams that can be truncated are expected!" );
    if ( xTruncate.is() )
    {
        xTruncate->truncate();
    }
    xWriter->setOutputStream( xOutput );

    xmlscript::ModuleDescriptor aMod;
    aMod.aName = aElementName;
    aMod.aLanguage = maScriptLanguage;
    Any aElement = xLib->getByName( aElementName );
    aElement >>= aMod.aCode;

    Reference< script::vba::XVBAModuleInfo > xModInfo( xLib, UNO_QUERY );
    if( xModInfo.is() && xModInfo->hasModuleInfo( aElementName ) )
    {
        script::ModuleInfo aModInfo = xModInfo->getModuleInfo( aElementName );
        switch( aModInfo.ModuleType )
        {
        case ModuleType::NORMAL:
            aMod.aModuleType = "normal";
            break;
        case ModuleType::CLASS:
            aMod.aModuleType ="class";
            break;
        case ModuleType::FORM:
            aMod.aModuleType = "form";
            break;
        case ModuleType::DOCUMENT:
            aMod.aModuleType = "document";
            break;
        case ModuleType::UNKNOWN:
            // nothing
            break;
        }
    }

    xmlscript::exportScriptModule( xWriter, aMod );
}


Any SAL_CALL SfxScriptLibraryContainer::importLibraryElement
    ( const Reference < XNameContainer >& xLib,
      const OUString& aElementName, const OUString& aFile,
      const uno::Reference< io::XInputStream >& xInStream )
{
    Any aRetAny;

    Reference< XParser > xParser = xml::sax::Parser::create( mxContext );

    // Read from storage?
    bool bStorage = xInStream.is();
    Reference< XInputStream > xInput;

    if( bStorage )
    {
        xInput = xInStream;
    }
    else
    {
        try
        {
            xInput = mxSFI->openFileRead( aFile );
        }
        catch(const Exception& )
        //catch( Exception& e )
        {
            // TODO:
            //throw WrappedTargetException( e );
        }
    }

    if( !xInput.is() )
        return aRetAny;

    InputSource source;
    source.aInputStream = xInput;
    source.sSystemId    = aFile;

    // start parsing
    xmlscript::ModuleDescriptor aMod;

    try
    {
        xParser->setDocumentHandler( ::xmlscript::importScriptModule( aMod ) );
        xParser->parseStream( source );
    }
    catch(const Exception& )
    {
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    }

    aRetAny <<= aMod.aCode;

    // TODO: Check language
    // aMod.aLanguage
    // aMod.aName ignored
    if( !aMod.aModuleType.isEmpty() )
    {
        /*  If in VBA compatibility mode, force creation of the VBA Globals
            object. Each application will create an instance of its own
            implementation and store it in its Basic manager. Implementations
            will do all necessary additional initialization, such as
            registering the global "This***Doc" UNO constant, starting the
            document events processor etc.
         */
        if( getVBACompatibilityMode() ) try
        {
            Reference< frame::XModel > xModel( mxOwnerDocument );   // weak-ref -> ref
            Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
            xFactory->createInstance("ooo.vba.VBAGlobals");
        }
        catch(const Exception& )
        {
        }

        script::ModuleInfo aModInfo;
        aModInfo.ModuleType = ModuleType::UNKNOWN;
        if( aMod.aModuleType == "normal" )
        {
            aModInfo.ModuleType = ModuleType::NORMAL;
        }
        else if( aMod.aModuleType == "class" )
        {
            aModInfo.ModuleType = ModuleType::CLASS;
        }
        else if( aMod.aModuleType == "form" )
        {
            aModInfo.ModuleType = ModuleType::FORM;
            aModInfo.ModuleObject = mxOwnerDocument;
        }
        else if( aMod.aModuleType == "document" )
        {
            aModInfo.ModuleType = ModuleType::DOCUMENT;

            // #163691# use the same codename access instance for all document modules
            if( !mxCodeNameAccess.is() ) try
            {
                Reference<frame::XModel > xModel( mxOwnerDocument );
                Reference< XMultiServiceFactory> xSF( xModel, UNO_QUERY_THROW );
                mxCodeNameAccess.set( xSF->createInstance("ooo.vba.VBAObjectModuleObjectProvider"), UNO_QUERY );
            }
            catch(const Exception& ) {}

            if( mxCodeNameAccess.is() )
            {
                try
                {
                    aModInfo.ModuleObject.set( mxCodeNameAccess->getByName( aElementName), uno::UNO_QUERY );
                }
                catch(const uno::Exception&)
                {
                    OSL_TRACE("Failed to get document object for %s", OUStringToOString( aElementName, RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
        }

        Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, UNO_QUERY );
        if( xVBAModuleInfo.is() )
        {
            if( xVBAModuleInfo->hasModuleInfo( aElementName ) )
            {
                xVBAModuleInfo->removeModuleInfo( aElementName );
            }
            xVBAModuleInfo->insertModuleInfo( aElementName, aModInfo );
        }
    }

    return aRetAny;
}

SfxLibraryContainer* SfxScriptLibraryContainer::createInstanceImpl()
{
    return new SfxScriptLibraryContainer();
}

void SAL_CALL SfxScriptLibraryContainer::importFromOldStorage( const OUString& aFile )
{
    // TODO: move loading from old storage to binary filters?
    auto xStorage = tools::make_ref<SotStorage>( false, aFile );
    if( xStorage->GetError() == ERRCODE_NONE )
    {
        BasicManager* pBasicManager = new BasicManager( *(xStorage.get()), aFile );

        // Set info
        LibraryContainerInfo aInfo( this, nullptr, static_cast< OldBasicPassword* >( this ) );
        pBasicManager->SetLibraryContainerInfo( aInfo );

        // Now the libraries should be copied to this SfxScriptLibraryContainer
        BasicManager::LegacyDeleteBasicManager( pBasicManager );
    }
}


// Storing with password encryption

// Methods XLibraryContainerPassword
sal_Bool SAL_CALL SfxScriptLibraryContainer::isLibraryPasswordProtected( const OUString& Name )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    bool bRet = pImplLib->mbPasswordProtected;
    return bRet;
}

sal_Bool SAL_CALL SfxScriptLibraryContainer::isLibraryPasswordVerified( const OUString& Name )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( !pImplLib->mbPasswordProtected )
    {
        throw IllegalArgumentException();
    }
    bool bRet = pImplLib->mbPasswordVerified;
    return bRet;
}

sal_Bool SAL_CALL SfxScriptLibraryContainer::verifyLibraryPassword
    ( const OUString& Name, const OUString& Password )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( !pImplLib->mbPasswordProtected || pImplLib->mbPasswordVerified )
    {
        throw IllegalArgumentException();
    }
    // Test password
    bool bSuccess = false;
    if( pImplLib->mbDoc50Password )
    {
        bSuccess = ( Password == pImplLib->maPassword );
        if( bSuccess )
        {
            pImplLib->mbPasswordVerified = true;
        }
    }
    else
    {
        pImplLib->maPassword = Password;
        bSuccess = implLoadPasswordLibrary( pImplLib, Name, true );
        if( bSuccess )
        {
            // The library gets modified by verifying the password, because other-
            // wise for saving the storage would be copied and that doesn't work
            // with mtg's storages when the password is verified
            pImplLib->implSetModified( true );
            pImplLib->mbPasswordVerified = true;

            // Reload library to get source
            if( pImplLib->mbLoaded )
            {
                implLoadPasswordLibrary( pImplLib, Name );
            }
        }
    }
    return bSuccess;
}

void SAL_CALL SfxScriptLibraryContainer::changeLibraryPassword( const OUString& Name,
                                                                const OUString& OldPassword,
                                                                const OUString& NewPassword )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException, std::exception)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( OldPassword == NewPassword )
    {
        return;
    }
    bool bOldPassword = !OldPassword.isEmpty();
    bool bNewPassword = !NewPassword.isEmpty();
    bool bStorage = mxStorage.is() && !pImplLib->mbLink;

    if( pImplLib->mbReadOnly || (bOldPassword && !pImplLib->mbPasswordProtected) )
    {
        throw IllegalArgumentException();
    }
    // Library must be loaded
    loadLibrary( Name );

    bool bKillCryptedFiles = false;
    bool bKillUncryptedFiles = false;

    // Remove or change password?
    if( bOldPassword )
    {
        if( isLibraryPasswordVerified( Name ) )
        {
            if( pImplLib->maPassword != OldPassword )
            {
                   throw IllegalArgumentException();
            }
        }
        else
        {
            if( !verifyLibraryPassword( Name, OldPassword ) )
            {
                   throw IllegalArgumentException();
            }
            // Reload library to get source
            // Should be done in verifyLibraryPassword loadLibrary( Name );
        }

        if( !bNewPassword )
        {
            pImplLib->mbPasswordProtected = false;
            pImplLib->mbPasswordVerified = false;
            pImplLib->maPassword.clear();

            maModifiable.setModified( true );
            pImplLib->implSetModified( true );

            if( !bStorage && !pImplLib->mbDoc50Password )
            {
                // Store application basic uncrypted
                uno::Reference< embed::XStorage > xStorage;
                storeLibraries_Impl( xStorage, false );
                bKillCryptedFiles = true;
            }
        }
    }

    // Set new password?
    if( bNewPassword )
    {
        pImplLib->mbPasswordProtected = true;
        pImplLib->mbPasswordVerified = true;
        pImplLib->maPassword = NewPassword;
        SfxScriptLibrary *const pSL(dynamic_cast<SfxScriptLibrary *>(pImplLib));
        if (pSL && pSL->mbLoaded)
        {
            pSL->mbLoadedSource = true; // must store source code now!
        }

        maModifiable.setModified( true );
        pImplLib->implSetModified( true );

        if( !bStorage && !pImplLib->mbDoc50Password )
        {
            // Store application basic crypted
            uno::Reference< embed::XStorage > xStorage;
            storeLibraries_Impl( xStorage, false );
            bKillUncryptedFiles = true;
        }
    }

    if( bKillCryptedFiles || bKillUncryptedFiles )
    {
        Sequence< OUString > aElementNames = pImplLib->getElementNames();
        sal_Int32 nNameCount = aElementNames.getLength();
        const OUString* pNames = aElementNames.getConstArray();
        OUString aLibDirPath = createAppLibraryFolder( pImplLib, Name );
        try
        {
            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, false,
                                            INetURLObject::LAST_SEGMENT,
                                            INetURLObject::ENCODE_ALL );
                if( bKillUncryptedFiles )
                {
                    aElementInetObj.setExtension( maLibElementFileExtension );
                }
                else
                {
                    aElementInetObj.setExtension( "pba" );
                }
                OUString aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if( mxSFI->exists( aElementPath ) )
                {
                    mxSFI->kill( aElementPath );
                }
            }
        }
        catch(const Exception& ) {}
    }
}


void setStreamKey( const uno::Reference< io::XStream >& xStream, const OUString& aPass )
{
    uno::Reference< embed::XEncryptionProtectedSource > xEncrStream( xStream, uno::UNO_QUERY );
    if ( xEncrStream.is() )
    {
        xEncrStream->setEncryptionPassword( aPass );
    }
}


// Impl methods
bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib,
                                                          const OUString& aName,
                                                          const uno::Reference< embed::XStorage >& xStorage,
                                                          const css::uno::Reference< css::task::XInteractionHandler >& xHandler )
{
    OUString aDummyLocation;
    Reference< XSimpleFileAccess3 > xDummySFA;
    return implStorePasswordLibrary( pLib, aName, xStorage, aDummyLocation, xDummySFA, xHandler );
}

bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                                                          const css::uno::Reference< css::embed::XStorage >& xStorage,
                                                          const OUString& aTargetURL,
                                                          const Reference< XSimpleFileAccess3 >& rToUseSFI,
                                                          const css::uno::Reference< css::task::XInteractionHandler >& xHandler )
{
    bool bExport = !aTargetURL.isEmpty();

    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implStorePasswordLibrary: cannot do this without a BasicManager!" );
    if ( !pBasicMgr )
    {
        return false;
    }
    // Only need to handle the export case here,
    // save/saveas etc are handled in sfxbasemodel::storeSelf &
    // sfxbasemodel::impl_store
    std::vector<OUString> aNames;
    if ( bExport && pBasicMgr->LegacyPsswdBinaryLimitExceeded(aNames) )
    {
        if ( xHandler.is() )
        {
            ModuleSizeExceeded* pReq =  new ModuleSizeExceeded( aNames );
            uno::Reference< task::XInteractionRequest > xReq( pReq );
            xHandler->handle( xReq );
            if ( pReq->isAbort() )
            {
                throw util::VetoException();
            }
        }
    }

    StarBASIC* pBasicLib = pBasicMgr->GetLib( aName );
    if( !pBasicLib )
    {
        return false;
    }
    Sequence< OUString > aElementNames = pLib->getElementNames();
    sal_Int32 nNameCount = aElementNames.getLength();
    const OUString* pNames = aElementNames.getConstArray();

    bool bLink = pLib->mbLink;
    bool bStorage = xStorage.is() && !bLink;
    if( bStorage )
    {
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            // Write binary image stream
            SbModule* pMod = pBasicLib->FindModule( aElementName );
            if( pMod )
            {
                OUString aCodeStreamName = aElementName + ".bin";
                try
                {
                    uno::Reference< io::XStream > xCodeStream = xStorage->openStreamElement(
                            aCodeStreamName,
                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                    if ( !xCodeStream.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    SvMemoryStream aMemStream;
                    /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream, B_CURVERSION );

                    sal_Int32 const nSize = aMemStream.Tell();
                    if (nSize < 0) { abort(); }
                    Sequence< sal_Int8 > aBinSeq( nSize );
                    sal_Int8* pData = aBinSeq.getArray();
                    memcpy( pData, aMemStream.GetData(), nSize );

                       Reference< XOutputStream > xOut = xCodeStream->getOutputStream();
                    if ( !xOut.is() )
                    {
                        throw io::IOException(); // access denied because the stream is readonly
                    }
                    xOut->writeBytes( aBinSeq );
                    xOut->closeOutput();
                }
                catch(const uno::Exception& )
                {
                    // TODO: handle error
                }
            }

            if( pLib->mbPasswordVerified || pLib->mbDoc50Password )
            {
                if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
                {
                    #if OSL_DEBUG_LEVEL > 0
                    OString aMessage = "invalid library element '" +
                                        OUStringToOString( aElementName, osl_getThreadTextEncoding() ) +
                                        "'.";
                    OSL_FAIL( aMessage.getStr());
                    #endif
                    continue;
                }

                OUString aSourceStreamName = aElementName + ".xml";
                try
                {
                    uno::Reference< io::XStream > xSourceStream = xStorage->openStreamElement(
                            aSourceStreamName,
                            embed::ElementModes::READWRITE );
                    uno::Reference< beans::XPropertySet > xProps( xSourceStream, uno::UNO_QUERY );
                    if ( !xProps.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    OUString aMime( "text/xml" );
                    xProps->setPropertyValue("MediaType", uno::makeAny( aMime ) );

                    // Set encryption key
                    setStreamKey( xSourceStream, pLib->maPassword );

                    Reference< XOutputStream > xOutput = xSourceStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOutput );
                }
                catch(const uno::Exception& )
                {
                    OSL_FAIL( "Problem on storing of password library!\n" );
                    // TODO: error handling
                }
            }
            else    // !mbPasswordVerified
            {
                // TODO
                // What to do if not verified?! In any case it's already loaded here
            }
        }

    }
    // Application libraries have only to be saved if the password
    // is verified because otherwise they can't be modified
    else if( pLib->mbPasswordVerified || bExport )
    {
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
                aInetObj.insertName( aName, true, INetURLObject::LAST_SEGMENT,
                                     INetURLObject::ENCODE_ALL );
                aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

                if( !xSFI->isFolder( aLibDirPath ) )
                {
                    xSFI->createFolder( aLibDirPath );
                }
            }
            else
            {
                aLibDirPath = createAppLibraryFolder( pLib, aName );
            }

            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, false,
                                            INetURLObject::LAST_SEGMENT,
                                            INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( "pba" );
                OUString aElementPath = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

                if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
                {
                    #if OSL_DEBUG_LEVEL > 0
                    OString aMessage = "invalid library element '" +
                                       OUStringToOString( aElementName, osl_getThreadTextEncoding() ) +
                                       "'.";
                    OSL_FAIL( aMessage.getStr());
                    #endif
                    continue;
                }

                try
                {
                    uno::Reference< embed::XStorage > xElementRootStorage =
                        ::comphelper::OStorageHelper::GetStorageFromURL(
                                aElementPath,
                                embed::ElementModes::READWRITE );
                    if ( !xElementRootStorage.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    // Write binary image stream
                    SbModule* pMod = pBasicLib->FindModule( aElementName );
                    if( pMod )
                    {
                        OUString aCodeStreamName( "code.bin" );

                        uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                            aCodeStreamName,
                                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                        SvMemoryStream aMemStream;
                        /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream, B_CURVERSION );

                        sal_Int32 const nSize = aMemStream.Tell();
                        if (nSize < 0) { abort(); }
                        Sequence< sal_Int8 > aBinSeq( nSize );
                        sal_Int8* pData = aBinSeq.getArray();
                        memcpy( pData, aMemStream.GetData(), nSize );

                        Reference< XOutputStream > xOut = xCodeStream->getOutputStream();
                        if ( xOut.is() )
                        {
                            xOut->writeBytes( aBinSeq );
                            xOut->closeOutput();
                        }
                    }

                    // Write encrypted source stream
                    OUString aSourceStreamName( "source.xml" );

                    uno::Reference< io::XStream > xSourceStream;
                    try
                    {
                        xSourceStream = xElementRootStorage->openStreamElement(
                            aSourceStreamName,
                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                        // #87671 Allow encryption
                        uno::Reference< embed::XEncryptionProtectedSource > xEncr( xSourceStream, uno::UNO_QUERY );
                        OSL_ENSURE( xEncr.is(),
                                    "StorageStream opened for writing must implement XEncryptionProtectedSource!\n" );
                        if ( !xEncr.is() )
                        {
                            throw uno::RuntimeException();
                        }
                        xEncr->setEncryptionPassword( pLib->maPassword );
                    }
                    catch(const css::packages::WrongPasswordException& )
                    {
                        xSourceStream = xElementRootStorage->openEncryptedStreamElement(
                                aSourceStreamName,
                                embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,
                                pLib->maPassword );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSourceStream, uno::UNO_QUERY );
                    if ( !xProps.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    OUString aMime( "text/xml" );
                    xProps->setPropertyValue("MediaType", uno::makeAny( aMime ) );

                    Reference< XOutputStream > xOut = xSourceStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOut );
                    // i50568: sax writer already closes stream
                    // xOut->closeOutput();

                    uno::Reference< embed::XTransactedObject > xTransact( xElementRootStorage, uno::UNO_QUERY );
                    OSL_ENSURE( xTransact.is(), "The storage must implement XTransactedObject!\n" );
                    if ( !xTransact.is() )
                    {
                        throw uno::RuntimeException();
                    }

                    xTransact->commit();
                }
                catch(const uno::Exception& )
                {
                    // TODO: handle error
                }

            }
        }
        catch(const Exception& )
        {
        }
    }
    return true;
}

bool SfxScriptLibraryContainer::implLoadPasswordLibrary
    ( SfxLibrary* pLib, const OUString& Name, bool bVerifyPasswordOnly )
        throw(WrappedTargetException, RuntimeException, std::exception)
{
    bool bRet = true;

    bool bLink = pLib->mbLink;
    bool bStorage = mxStorage.is() && !bLink;

    // Already loaded? Then only verifiedPassword can change something
    SfxScriptLibrary* pScriptLib = static_cast< SfxScriptLibrary* >( pLib );
    if( pScriptLib->mbLoaded )
    {
        if( pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly &&
            (pScriptLib->mbLoadedSource || !pLib->mbPasswordVerified) )
        {
            return false;
        }
    }

    StarBASIC* pBasicLib = nullptr;
    bool bLoadBinary = false;
    if( !pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly && !pLib->mbPasswordVerified )
    {
        BasicManager* pBasicMgr = getBasicManager();
        OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implLoadPasswordLibrary: cannot do this without a BasicManager!" );
        bool bLoaded = pScriptLib->mbLoaded;
        pScriptLib->mbLoaded = true;        // Necessary to get lib
        pBasicLib = pBasicMgr ? pBasicMgr->GetLib( Name ) : nullptr;
        pScriptLib->mbLoaded = bLoaded;    // Restore flag
        if( !pBasicLib )
        {
            return false;
        }
        bLoadBinary = true;
        pScriptLib->mbLoadedBinary = true;
    }

    bool bLoadSource = false;
    if( !pScriptLib->mbLoadedSource && pLib->mbPasswordVerified && !bVerifyPasswordOnly )
    {
        bLoadSource = true;
        pScriptLib->mbLoadedSource = true;
    }

    Sequence< OUString > aElementNames = pLib->getElementNames();
    sal_Int32 nNameCount = aElementNames.getLength();
    const OUString* pNames = aElementNames.getConstArray();

    if( bStorage )
    {
        uno::Reference< embed::XStorage > xLibrariesStor;
        uno::Reference< embed::XStorage > xLibraryStor;
        if( bStorage )
        {
            try {
                xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
                if ( !xLibrariesStor.is() )
                {
                    throw uno::RuntimeException();
                }
                xLibraryStor = xLibrariesStor->openStorageElement( Name, embed::ElementModes::READ );
                if ( !xLibraryStor.is() )
                {
                    throw uno::RuntimeException();
                }
            }
            catch(const uno::Exception& )
            {
                OSL_FAIL( "### couldn't open sub storage for library\n" );
                return false;
            }
        }

        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            // Load binary
            if( bLoadBinary )
            {
                SbModule* pMod = pBasicLib->FindModule( aElementName );
                if( !pMod )
                {
                    pMod = pBasicLib->MakeModule( aElementName, OUString() );
                    pBasicLib->SetModified( false );
                }

                OUString aCodeStreamName= aElementName + ".bin";
                try
                {
                    uno::Reference< io::XStream > xCodeStream = xLibraryStor->openStreamElement(
                                                                                        aCodeStreamName,
                                                                                        embed::ElementModes::READ );
                    if ( !xCodeStream.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream( xCodeStream ));
                    if ( !pStream || pStream->GetError() )
                    {
                        sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                        throw task::ErrorCodeIOException(
                            ("utl::UcbStreamHelper::CreateStream failed for \""
                             + aCodeStreamName + "\": 0x"
                             + OUString::number(nError, 16)),
                            uno::Reference< uno::XInterface >(), nError);
                    }

                    /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                    // TODO: Check return value
                }
                catch(const uno::Exception& )
                {
                    // TODO: error handling
                }
            }

            // Load source
            if( bLoadSource || bVerifyPasswordOnly )
            {
                // Access encrypted source stream
                OUString aSourceStreamName = aElementName + ".xml";
                try
                {
                    uno::Reference< io::XStream > xSourceStream = xLibraryStor->openEncryptedStreamElement(
                                                                    aSourceStreamName,
                                                                    embed::ElementModes::READ,
                                                                    pLib->maPassword );
                    if ( !xSourceStream.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    // if this point is reached then the password is correct
                    if ( !bVerifyPasswordOnly )
                    {
                        uno::Reference< io::XInputStream > xInStream = xSourceStream->getInputStream();
                        if ( !xInStream.is() )
                        {
                            throw io::IOException(); // read access denied, seems to be impossible
                        }
                        Reference< XNameContainer > xLib( pLib );
                        Any aAny = importLibraryElement( xLib,
                                                         aElementName, aSourceStreamName,
                                                         xInStream );
                        if( pLib->hasByName( aElementName ) )
                        {
                            if( aAny.hasValue() )
                            {
                                pLib->maNameContainer->replaceByName( aElementName, aAny );
                            }
                        }
                        else
                        {
                            pLib->maNameContainer->insertByName( aElementName, aAny );
                        }
                    }
                }
                catch(const uno::Exception& )
                {
                    bRet = false;
                }
            }
        }
    }
    else
    {
        try
        {
            OUString aLibDirPath = createAppLibraryFolder( pLib, Name );

            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, false,
                    INetURLObject::LAST_SEGMENT, INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( "pba" );
                OUString aElementPath = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

                uno::Reference< embed::XStorage > xElementRootStorage;
                try
                {
                    xElementRootStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                            aElementPath,
                            embed::ElementModes::READ );
                } catch(const uno::Exception& )
                {
                    // TODO: error handling
                }

                if ( xElementRootStorage.is() )
                {
                    // Load binary
                    if( bLoadBinary )
                    {
                        SbModule* pMod = pBasicLib->FindModule( aElementName );
                        if( !pMod )
                        {
                            pMod = pBasicLib->MakeModule( aElementName, OUString() );
                            pBasicLib->SetModified( false );
                        }

                        try
                        {
                            OUString aCodeStreamName( "code.bin" );
                            uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                                                        aCodeStreamName,
                                                                        embed::ElementModes::READ );

                            std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream( xCodeStream ));
                            if ( !pStream || pStream->GetError() )
                            {
                                sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                                throw task::ErrorCodeIOException(
                                    ("utl::UcbStreamHelper::CreateStream failed"
                                     " for code.bin: 0x"
                                     + OUString::number(nError, 16)),
                                    uno::Reference< uno::XInterface >(),
                                    nError);
                            }

                            /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                            // TODO: Check return value
                        }
                        catch(const uno::Exception& )
                        {
                            // TODO: error handling
                        }
                    }

                    // Load source
                    if( bLoadSource || bVerifyPasswordOnly )
                    {
                        // Access encrypted source stream
                        OUString aSourceStreamName( "source.xml" );
                        try
                        {
                            uno::Reference< io::XStream > xSourceStream = xElementRootStorage->openEncryptedStreamElement(
                                                                    aSourceStreamName,
                                                                    embed::ElementModes::READ,
                                                                    pLib->maPassword );
                            if ( !xSourceStream.is() )
                            {
                                throw uno::RuntimeException();
                            }
                            if ( !bVerifyPasswordOnly )
                            {
                                uno::Reference< io::XInputStream > xInStream = xSourceStream->getInputStream();
                                if ( !xInStream.is() )
                                {
                                    throw io::IOException(); // read access denied, seems to be impossible
                                }
                                Reference< XNameContainer > xLib( pLib );
                                Any aAny = importLibraryElement( xLib,
                                                                 aElementName,
                                                                 aSourceStreamName,
                                                                 xInStream );
                                if( pLib->hasByName( aElementName ) )
                                {
                                    if( aAny.hasValue() )
                                    {
                                        pLib->maNameContainer->replaceByName( aElementName, aAny );
                                    }
                                }
                                else
                                {
                                    pLib->maNameContainer->insertByName( aElementName, aAny );
                                }
                            }
                        }
                        catch (const uno::Exception& )
                        {
                            bRet = false;
                        }
                    }
                }
            }
        }
        catch(const Exception& )
        {
            // TODO
            //throw e;
        }
    }

    return bRet;
}


void SfxScriptLibraryContainer::onNewRootStorage()
{
}

sal_Bool SAL_CALL SfxScriptLibraryContainer:: HasExecutableCode( const OUString& Library )
    throw (uno::RuntimeException, std::exception)
{
    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "we need a basicmanager, really we do" );
    if ( pBasicMgr )
    {
        return pBasicMgr->HasExeCode( Library ); // need to change this to take name
    }
    // default to it has code if we can't decide
    return true;
}


// Service
OUString SAL_CALL SfxScriptLibraryContainer::getImplementationName( )
    throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.sfx2.ScriptLibraryContainer" );
}

Sequence< OUString > SAL_CALL SfxScriptLibraryContainer::getSupportedServiceNames( )
    throw (RuntimeException, std::exception)
{
    return {"com.sun.star.script.DocumentScriptLibraryContainer",
            "com.sun.star.script.ScriptLibraryContainer"}; // for compatibility
}

// Implementation class SfxScriptLibrary

// Ctor
SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XSimpleFileAccess3 >& xSFI )
    : SfxLibrary( _rModifiable, cppu::UnoType<OUString>::get(), xSFI )
    , mbLoadedSource( false )
    , mbLoadedBinary( false )
{
}

SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    bool ReadOnly )
    : SfxLibrary( _rModifiable, cppu::UnoType<OUString>::get(), xSFI,
                        aLibInfoFileURL, aStorageURL, ReadOnly)
    , mbLoadedSource( false )
    , mbLoadedBinary( false )
{
}

bool SfxScriptLibrary::isLoadedStorable()
{
    // note: mbLoadedSource can only be true for password-protected lib!
    return SfxLibrary::isLoadedStorable() && (!mbPasswordProtected || mbLoadedSource);
}

// Provide modify state including resources
bool SfxScriptLibrary::isModified()
{
    return implIsModified();    // No resources
}

void SfxScriptLibrary::storeResources()
{
    // No resources
}

void SfxScriptLibrary::storeResourcesToURL( const OUString& URL,
    const Reference< task::XInteractionHandler >& Handler )
{
    (void)URL;
    (void)Handler;
}

void SfxScriptLibrary::storeResourcesAsURL
    ( const OUString& URL, const OUString& NewName )
{
    (void)URL;
    (void)NewName;
}

void SfxScriptLibrary::storeResourcesToStorage( const css::uno::Reference
    < css::embed::XStorage >& xStorage )
{
    // No resources
    (void)xStorage;
}

bool SfxScriptLibrary::containsValidModule(const Any& rElement)
{
    OUString sModuleText;
    rElement >>= sModuleText;
    return ( !sModuleText.isEmpty() );
}

bool SAL_CALL SfxScriptLibrary::isLibraryElementValid(const css::uno::Any& rElement) const
{
    return SfxScriptLibrary::containsValidModule(rElement);
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );

script::ModuleInfo SAL_CALL SfxScriptLibrary::getModuleInfo( const OUString& ModuleName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    if ( !hasModuleInfo( ModuleName ) )
    {
        throw NoSuchElementException();
    }
    return mModuleInfo[ ModuleName ];
}

sal_Bool SAL_CALL SfxScriptLibrary::hasModuleInfo( const OUString& ModuleName )
    throw (RuntimeException, std::exception)
{
    bool bRes = false;
    ModuleInfoMap::iterator it = mModuleInfo.find( ModuleName );

    if ( it != mModuleInfo.end() )
    {
        bRes = true;
    }
    return bRes;
}

void SAL_CALL SfxScriptLibrary::insertModuleInfo( const OUString& ModuleName, const script::ModuleInfo& ModuleInfo )
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    if ( hasModuleInfo( ModuleName ) )
    {
        throw ElementExistException();
    }
    mModuleInfo[ ModuleName ] = ModuleInfo;
}

void SAL_CALL SfxScriptLibrary::removeModuleInfo( const OUString& ModuleName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
        // #FIXME add NoSuchElementException to the spec
    if ( !hasModuleInfo( ModuleName ) )
    {
        throw NoSuchElementException();
    }
    mModuleInfo.erase( mModuleInfo.find( ModuleName ) );
}

}   // namespace basic


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_sfx2_ScriptLibraryContainer_get_implementation(css::uno::XComponentContext*,
                                                                 css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new basic::SfxScriptLibraryContainer());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
