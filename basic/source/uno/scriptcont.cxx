/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scriptcont.hxx"
#include "sbmodule.hxx"
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
#include <cppuhelper/implbase1.hxx>
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




const sal_Char* SAL_CALL SfxScriptLibraryContainer::getInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getOldInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibElementFileExtension() const { return "xba"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibrariesDir() const { return "Basic"; }


void SfxScriptLibraryContainer::setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword )
{
    try
    {
        SfxLibrary* pImplLib = getImplLib( rLibraryName );
        if( !rPassword.isEmpty() )
        {
            pImplLib->mbDoc50Password = true;
            pImplLib->mbPasswordProtected = sal_True;
            pImplLib->maPassword = rPassword;
        }
    }
    catch(const NoSuchElementException& ) {}
}

OUString SfxScriptLibraryContainer::getLibraryPassword( const OUString& rLibraryName )
{
    SfxLibrary* pImplLib = getImplLib( rLibraryName );
    OUString aPassword;
    if( pImplLib->mbPasswordVerified )
    {
        aPassword = pImplLib->maPassword;
    }
    return aPassword;
}

void SfxScriptLibraryContainer::clearLibraryPassword( const OUString& rLibraryName )
{
    try
    {
        SfxLibrary* pImplLib = getImplLib( rLibraryName );
        pImplLib->mbDoc50Password = false;
        pImplLib->mbPasswordProtected = sal_False;
        pImplLib->maPassword = "";
    }
    catch(const NoSuchElementException& ) {}
}

sal_Bool SfxScriptLibraryContainer::hasLibraryPassword( const OUString& rLibraryName )
{
    SfxLibrary* pImplLib = getImplLib( rLibraryName );
    return pImplLib->mbPasswordProtected;
}


SfxScriptLibraryContainer::SfxScriptLibraryContainer( void )
    :maScriptLanguage( "StarBasic"  )
{
    
    
}

SfxScriptLibraryContainer::SfxScriptLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
    :maScriptLanguage( "StarBasic"  )
{
    init( OUString(), xStorage );
}


SfxLibrary* SfxScriptLibraryContainer::implCreateLibrary( const OUString& aName )
{
    (void)aName;    
    SfxLibrary* pRet = new SfxScriptLibrary( maModifiable, mxContext, mxSFI );
    return pRet;
}

SfxLibrary* SfxScriptLibraryContainer::implCreateLibraryLink( const OUString& aName,
                                                              const OUString& aLibInfoFileURL,
                                                              const OUString& StorageURL,
                                                              sal_Bool ReadOnly )
{
    (void)aName;    
    SfxLibrary* pRet = new SfxScriptLibrary( maModifiable, mxContext, mxSFI,
                                             aLibInfoFileURL, StorageURL, ReadOnly );
    return pRet;
}

Any SAL_CALL SfxScriptLibraryContainer::createEmptyLibraryElement( void )
{
    OUString aMod;
    Any aRetAny;
    aRetAny <<= aMod;
    return aRetAny;
}

bool SAL_CALL SfxScriptLibraryContainer::isLibraryElementValid( Any aElement ) const
{
    return SfxScriptLibrary::containsValidModule( aElement );
}

void SAL_CALL SfxScriptLibraryContainer::writeLibraryElement( const Reference < XNameContainer >& xLib,
                                                              const OUString& aElementName,
                                                              const Reference< XOutputStream >& xOutput)
    throw(Exception)
{
    
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

    
    sal_Bool bStorage = xInStream.is();
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
        
        {
            
            
        }
    }

    if( !xInput.is() )
        return aRetAny;

    InputSource source;
    source.aInputStream = xInput;
    source.sSystemId    = aFile;

    
    xmlscript::ModuleDescriptor aMod;

    try
    {
        xParser->setDocumentHandler( ::xmlscript::importScriptModule( aMod ) );
        xParser->parseStream( source );
    }
    catch(const Exception& )
    {
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        sal_uIntPtr nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
    }

    aRetAny <<= aMod.aCode;

    
    
    
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
            Reference< frame::XModel > xModel( mxOwnerDocument );   
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
                    OSL_TRACE("Failed to get documument object for %s", OUStringToOString( aElementName, RTL_TEXTENCODING_UTF8 ).getStr() );
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

SfxLibraryContainer* SfxScriptLibraryContainer::createInstanceImpl( void )
{
    return new SfxScriptLibraryContainer();
}

void SAL_CALL SfxScriptLibraryContainer::importFromOldStorage( const OUString& aFile )
{
    
    SotStorageRef xStorage = new SotStorage( false, aFile );
    if( xStorage.Is() && xStorage->GetError() == ERRCODE_NONE )
    {
        BasicManager* pBasicManager = new BasicManager( *(SotStorage*)xStorage, aFile );

        
        LibraryContainerInfo aInfo( this, NULL, static_cast< OldBasicPassword* >( this ) );
        pBasicManager->SetLibraryContainerInfo( aInfo );

        
        BasicManager::LegacyDeleteBasicManager( pBasicManager );
    }
}





sal_Bool SAL_CALL SfxScriptLibraryContainer::isLibraryPasswordProtected( const OUString& Name )
    throw (NoSuchElementException, RuntimeException)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    sal_Bool bRet = pImplLib->mbPasswordProtected;
    return bRet;
}

sal_Bool SAL_CALL SfxScriptLibraryContainer::isLibraryPasswordVerified( const OUString& Name )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( !pImplLib->mbPasswordProtected )
    {
        throw IllegalArgumentException();
    }
    sal_Bool bRet = pImplLib->mbPasswordVerified;
    return bRet;
}

sal_Bool SAL_CALL SfxScriptLibraryContainer::verifyLibraryPassword
    ( const OUString& Name, const OUString& Password )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( !pImplLib->mbPasswordProtected || pImplLib->mbPasswordVerified )
    {
        throw IllegalArgumentException();
    }
    
    sal_Bool bSuccess = sal_False;
    if( pImplLib->mbDoc50Password )
    {
        bSuccess = ( Password == pImplLib->maPassword );
        if( bSuccess )
        {
            pImplLib->mbPasswordVerified = sal_True;
        }
    }
    else
    {
        pImplLib->maPassword = Password;
        bSuccess = implLoadPasswordLibrary( pImplLib, Name, sal_True );
        if( bSuccess )
        {
            
            
            
            pImplLib->implSetModified( sal_True );
            pImplLib->mbPasswordVerified = sal_True;

            
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
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( OldPassword == NewPassword )
    {
        return;
    }
    sal_Bool bOldPassword = !OldPassword.isEmpty();
    sal_Bool bNewPassword = !NewPassword.isEmpty();
    sal_Bool bStorage = mxStorage.is() && !pImplLib->mbLink;

    if( pImplLib->mbReadOnly || (bOldPassword && !pImplLib->mbPasswordProtected) )
    {
        throw IllegalArgumentException();
    }
    
    loadLibrary( Name );

    bool bKillCryptedFiles = false;
    bool bKillUncryptedFiles = false;

    
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
            
            
        }

        if( !bNewPassword )
        {
            pImplLib->mbPasswordProtected = sal_False;
            pImplLib->mbPasswordVerified = sal_False;
            pImplLib->maPassword = "";

            maModifiable.setModified( sal_True );
            pImplLib->implSetModified( sal_True );

            if( !bStorage && !pImplLib->mbDoc50Password )
            {
                
                uno::Reference< embed::XStorage > xStorage;
                storeLibraries_Impl( xStorage, false );
                bKillCryptedFiles = true;
            }
        }
    }

    
    if( bNewPassword )
    {
        pImplLib->mbPasswordProtected = sal_True;
        pImplLib->mbPasswordVerified = sal_True;
        pImplLib->maPassword = NewPassword;

        maModifiable.setModified( sal_True );
        pImplLib->implSetModified( sal_True );

        if( !bStorage && !pImplLib->mbDoc50Password )
        {
            
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
                                            INetURLObject::LAST_SEGMENT, true,
                                            INetURLObject::ENCODE_ALL );
                if( bKillUncryptedFiles )
                {
                    aElementInetObj.setExtension( maLibElementFileExtension );
                }
                else
                {
                    aElementInetObj.setExtension( OUString( "pba" ) );
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


void setStreamKey( uno::Reference< io::XStream > xStream, const OUString& aPass )
{
    uno::Reference< embed::XEncryptionProtectedSource > xEncrStream( xStream, uno::UNO_QUERY );
    if ( xEncrStream.is() )
    {
        xEncrStream->setEncryptionPassword( aPass );
    }
}



sal_Bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib,
                                                              const OUString& aName,
                                                              const uno::Reference< embed::XStorage >& xStorage,
                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
{
    OUString aDummyLocation;
    Reference< XSimpleFileAccess3 > xDummySFA;
    return implStorePasswordLibrary( pLib, aName, xStorage, aDummyLocation, xDummySFA, xHandler );
}

sal_Bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                                              const OUString& aTargetURL,
                                                              const Reference< XSimpleFileAccess3 > xToUseSFI,
                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
{
    bool bExport = !aTargetURL.isEmpty();

    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implStorePasswordLibrary: cannot do this without a BasicManager!" );
    if ( !pBasicMgr )
    {
        return sal_False;
    }
    
    
    
    uno::Sequence<OUString> aNames;
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
        return sal_False;
    }
    Sequence< OUString > aElementNames = pLib->getElementNames();
    sal_Int32 nNameCount = aElementNames.getLength();
    const OUString* pNames = aElementNames.getConstArray();

    sal_Bool bLink = pLib->mbLink;
    sal_Bool bStorage = xStorage.is() && !bLink;
    if( bStorage )
    {
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            
            SbModule* pMod = pBasicLib->FindModule( aElementName );
            if( pMod )
            {
                OUString aCodeStreamName = aElementName;
                aCodeStreamName += ".bin";

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
                    /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream );

                    sal_Int32 nSize = (sal_Int32)aMemStream.Tell();
                    Sequence< sal_Int8 > aBinSeq( nSize );
                    sal_Int8* pData = aBinSeq.getArray();
                    memcpy( pData, aMemStream.GetData(), nSize );

                       Reference< XOutputStream > xOut = xCodeStream->getOutputStream();
                    if ( !xOut.is() )
                    {
                        throw io::IOException(); 
                    }
                    xOut->writeBytes( aBinSeq );
                    xOut->closeOutput();
                }
                catch(const uno::Exception& )
                {
                    
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

                OUString aSourceStreamName = aElementName;
                aSourceStreamName += ".xml";

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

                    
                    setStreamKey( xSourceStream, pLib->maPassword );

                    Reference< XOutputStream > xOutput = xSourceStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOutput );
                }
                catch(const uno::Exception& )
                {
                    OSL_FAIL( "Problem on storing of password library!\n" );
                    
                }
            }
            else    
            {
                
                
            }
        }

    }
    
    
    else if( pLib->mbPasswordVerified || bExport )
    {
        try
        {
            Reference< XSimpleFileAccess3 > xSFI = mxSFI;
            if( xToUseSFI.is() )
            {
                xSFI = xToUseSFI;
            }
            OUString aLibDirPath;
            if( bExport )
            {
                INetURLObject aInetObj( aTargetURL );
                aInetObj.insertName( aName, true, INetURLObject::LAST_SEGMENT, true,
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
                                            INetURLObject::LAST_SEGMENT, true,
                                            INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( OUString( "pba" ) );
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
                    
                    SbModule* pMod = pBasicLib->FindModule( aElementName );
                    if( pMod )
                    {
                        OUString aCodeStreamName( "code.bin" );

                        uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                            aCodeStreamName,
                                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                        SvMemoryStream aMemStream;
                        /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream );

                        sal_Int32 nSize = (sal_Int32)aMemStream.Tell();
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

                    
                    OUString aSourceStreamName( "source.xml" );

                    uno::Reference< io::XStream > xSourceStream;
                    try
                    {
                        xSourceStream = xElementRootStorage->openStreamElement(
                            aSourceStreamName,
                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                        
                        uno::Reference< embed::XEncryptionProtectedSource > xEncr( xSourceStream, uno::UNO_QUERY );
                        OSL_ENSURE( xEncr.is(),
                                    "StorageStream opened for writing must implement XEncryptionProtectedSource!\n" );
                        if ( !xEncr.is() )
                        {
                            throw uno::RuntimeException();
                        }
                        xEncr->setEncryptionPassword( pLib->maPassword );
                    }
                    catch(const ::com::sun::star::packages::WrongPasswordException& )
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
                    
                }

            }
        }
        catch(const Exception& )
        {
        }
    }
    return sal_True;
}

sal_Bool SfxScriptLibraryContainer::implLoadPasswordLibrary
    ( SfxLibrary* pLib, const OUString& Name, sal_Bool bVerifyPasswordOnly )
        throw(WrappedTargetException, RuntimeException)
{
    sal_Bool bRet = sal_True;

    sal_Bool bLink = pLib->mbLink;
    sal_Bool bStorage = mxStorage.is() && !bLink;

    
    SfxScriptLibrary* pScriptLib = static_cast< SfxScriptLibrary* >( pLib );
    if( pScriptLib->mbLoaded )
    {
        if( pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly &&
            (pScriptLib->mbLoadedSource || !pLib->mbPasswordVerified) )
        {
            return sal_False;
        }
    }

    StarBASIC* pBasicLib = NULL;
    sal_Bool bLoadBinary = sal_False;
    if( !pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly && !pLib->mbPasswordVerified )
    {
        BasicManager* pBasicMgr = getBasicManager();
        OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implLoadPasswordLibrary: cannot do this without a BasicManager!" );
        sal_Bool bLoaded = pScriptLib->mbLoaded;
        pScriptLib->mbLoaded = sal_True;        
        pBasicLib = pBasicMgr ? pBasicMgr->GetLib( Name ) : NULL;
        pScriptLib->mbLoaded = bLoaded;    
        if( !pBasicLib )
        {
            return sal_False;
        }
        bLoadBinary = sal_True;
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
                return sal_False;
            }
        }

        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            
            if( bLoadBinary )
            {
                SbModule* pMod = pBasicLib->FindModule( aElementName );
                if( !pMod )
                {
                    pMod = pBasicLib->MakeModule( aElementName, OUString() );
                    pBasicLib->SetModified( sal_False );
                }

                OUString aCodeStreamName= aElementName;
                aCodeStreamName += ".bin";

                try
                {
                    uno::Reference< io::XStream > xCodeStream = xLibraryStor->openStreamElement(
                                                                                        aCodeStreamName,
                                                                                        embed::ElementModes::READ );
                    if ( !xCodeStream.is() )
                    {
                        throw uno::RuntimeException();
                    }
                    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xCodeStream );
                    if ( !pStream || pStream->GetError() )
                    {
                        sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                        delete pStream;
                        throw task::ErrorCodeIOException(
                            ("utl::UcbStreamHelper::CreateStream failed for \""
                             + aCodeStreamName + "\": 0x"
                             + OUString::number(nError, 16)),
                            uno::Reference< uno::XInterface >(), nError);
                    }

                    /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                    

                    delete pStream;
                }
                catch(const uno::Exception& )
                {
                    
                }
            }

            
            if( bLoadSource || bVerifyPasswordOnly )
            {
                
                OUString aSourceStreamName = aElementName;
                aSourceStreamName += ".xml";

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
                    
                    if ( !bVerifyPasswordOnly )
                    {
                        uno::Reference< io::XInputStream > xInStream = xSourceStream->getInputStream();
                        if ( !xInStream.is() )
                        {
                            throw io::IOException(); 
                        }
                        Reference< XNameContainer > xLib( pLib );
                        Any aAny = importLibraryElement( xLib,
                                                         aElementName, aSourceStreamName,
                                                         xInStream );
                        if( pLib->hasByName( aElementName ) )
                        {
                            if( aAny.hasValue() )
                            {
                                pLib->maNameContainer.replaceByName( aElementName, aAny );
                            }
                        }
                        else
                        {
                            pLib->maNameContainer.insertByName( aElementName, aAny );
                        }
                    }
                }
                catch(const uno::Exception& )
                {
                    bRet = sal_False;
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
                    INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( OUString( "pba" ) );
                OUString aElementPath = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

                uno::Reference< embed::XStorage > xElementRootStorage;
                try
                {
                    xElementRootStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                            aElementPath,
                            embed::ElementModes::READ );
                } catch(const uno::Exception& )
                {
                    
                }

                if ( xElementRootStorage.is() )
                {
                    
                    if( bLoadBinary )
                    {
                        SbModule* pMod = pBasicLib->FindModule( aElementName );
                        if( !pMod )
                        {
                            pMod = pBasicLib->MakeModule( aElementName, OUString() );
                            pBasicLib->SetModified( sal_False );
                        }

                        try
                        {
                            OUString aCodeStreamName( "code.bin" );
                            uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                                                        aCodeStreamName,
                                                                        embed::ElementModes::READ );

                            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xCodeStream );
                            if ( !pStream || pStream->GetError() )
                            {
                                sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                                delete pStream;
                                throw task::ErrorCodeIOException(
                                    ("utl::UcbStreamHelper::CreateStream failed"
                                     " for code.bin: 0x"
                                     + OUString::number(nError, 16)),
                                    uno::Reference< uno::XInterface >(),
                                    nError);
                            }

                            /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                            

                            delete pStream;
                        }
                        catch(const uno::Exception& )
                        {
                            
                        }
                    }

                    
                    if( bLoadSource || bVerifyPasswordOnly )
                    {
                        
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
                                    throw io::IOException(); 
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
                                        pLib->maNameContainer.replaceByName( aElementName, aAny );
                                    }
                                }
                                else
                                {
                                    pLib->maNameContainer.insertByName( aElementName, aAny );
                                }
                            }
                        }
                        catch (const uno::Exception& )
                        {
                            bRet = sal_False;
                        }
                    }
                }
            }
        }
        catch(const Exception& )
        {
            
            
        }
    }

    return bRet;
}


void SfxScriptLibraryContainer::onNewRootStorage()
{
}

sal_Bool SAL_CALL SfxScriptLibraryContainer:: HasExecutableCode( const OUString& Library )
    throw (uno::RuntimeException)
{
    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "we need a basicmanager, really we do" );
    if ( pBasicMgr )
    {
        return pBasicMgr->HasExeCode( Library ); 
    }
    
    return sal_True;
}



void createRegistryInfo_SfxScriptLibraryContainer()
{
    static OAutoRegistration< SfxScriptLibraryContainer > aAutoRegistration;
}

OUString SAL_CALL SfxScriptLibraryContainer::getImplementationName( )
    throw (RuntimeException)
{
    return getImplementationName_static();
}

Sequence< OUString > SAL_CALL SfxScriptLibraryContainer::getSupportedServiceNames( )
    throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SfxScriptLibraryContainer::getSupportedServiceNames_static()
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[0] = "com.sun.star.script.DocumentScriptLibraryContainer";
    
    aServiceNames[1] = "com.sun.star.script.ScriptLibraryContainer";
    return aServiceNames;
}

OUString SfxScriptLibraryContainer::getImplementationName_static()
{
    return OUString("com.sun.star.comp.sfx2.ScriptLibraryContainer" );
}

Reference< XInterface > SAL_CALL SfxScriptLibraryContainer::Create( const Reference< XComponentContext >& )
    throw( Exception )
{
    Reference< XInterface > xRet = static_cast< XInterface* >( static_cast< OWeakObject* >(new SfxScriptLibraryContainer()) );
    return xRet;
}





SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XComponentContext >& xContext,
                                    const Reference< XSimpleFileAccess3 >& xSFI )
    : SfxLibrary( _rModifiable, getCppuType( (const OUString *)0 ), xContext, xSFI )
    , mbLoadedSource( false )
    , mbLoadedBinary( false )
{
}

SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XComponentContext >& xContext,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    sal_Bool ReadOnly )
    : SfxLibrary( _rModifiable, getCppuType( (const OUString *)0 ), xContext, xSFI,
                        aLibInfoFileURL, aStorageURL, ReadOnly)
    , mbLoadedSource( false )
    , mbLoadedBinary( false )
{
}


sal_Bool SfxScriptLibrary::isModified( void )
{
    return implIsModified();    
}

void SfxScriptLibrary::storeResources( void )
{
    
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

void SfxScriptLibrary::storeResourcesToStorage( const ::com::sun::star::uno::Reference
    < ::com::sun::star::embed::XStorage >& xStorage )
{
    
    (void)xStorage;
}

bool SfxScriptLibrary::containsValidModule( const Any& aElement )
{
    OUString sModuleText;
    aElement >>= sModuleText;
    return ( !sModuleText.isEmpty() );
}

bool SAL_CALL SfxScriptLibrary::isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const
{
    return SfxScriptLibrary::containsValidModule( aElement );
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );

script::ModuleInfo SAL_CALL SfxScriptLibrary::getModuleInfo( const OUString& ModuleName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( !hasModuleInfo( ModuleName ) )
    {
        throw NoSuchElementException();
    }
    return mModuleInfos[ ModuleName ];
}

sal_Bool SAL_CALL SfxScriptLibrary::hasModuleInfo( const OUString& ModuleName )
    throw (RuntimeException)
{
    sal_Bool bRes = sal_False;
    ModuleInfoMap::iterator it = mModuleInfos.find( ModuleName );

    if ( it != mModuleInfos.end() )
    {
        bRes = sal_True;
    }
    return bRes;
}

void SAL_CALL SfxScriptLibrary::insertModuleInfo( const OUString& ModuleName, const script::ModuleInfo& ModuleInfo )
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if ( hasModuleInfo( ModuleName ) )
    {
        throw ElementExistException();
    }
    mModuleInfos[ ModuleName ] = ModuleInfo;
}

void SAL_CALL SfxScriptLibrary::removeModuleInfo( const OUString& ModuleName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
        
    if ( !hasModuleInfo( ModuleName ) )
    {
        throw NoSuchElementException();
    }
    mModuleInfos.erase( mModuleInfos.find( ModuleName ) );
}




}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
