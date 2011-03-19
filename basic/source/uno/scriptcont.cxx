/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_basic.hxx"
#include "scriptcont.hxx"
#include "sbmodule.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
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
#include "basic/modsizeexceeded.hxx"
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

using ::rtl::OUString;

using ::rtl::OUString;

//============================================================================
// Implementation class SfxScriptLibraryContainer

const sal_Char* SAL_CALL SfxScriptLibraryContainer::getInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getOldInfoFileName() const { return "script"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibElementFileExtension() const { return "xba"; }
const sal_Char* SAL_CALL SfxScriptLibraryContainer::getLibrariesDir() const { return "Basic"; }

// OldBasicPassword interface
void SfxScriptLibraryContainer::setLibraryPassword
    ( const String& rLibraryName, const String& rPassword )
{
    try
    {
        SfxLibrary* pImplLib = getImplLib( rLibraryName );
        if( rPassword.Len() )
        {
            pImplLib->mbDoc50Password = sal_True;
            pImplLib->mbPasswordProtected = sal_True;
            pImplLib->maPassword = rPassword;
        }
    }
    catch( NoSuchElementException& ) {}
}

String SfxScriptLibraryContainer::getLibraryPassword( const String& rLibraryName )
{
    SfxLibrary* pImplLib = getImplLib( rLibraryName );
    String aPassword;
    if( pImplLib->mbPasswordVerified )
        aPassword = pImplLib->maPassword;
    return aPassword;
}

void SfxScriptLibraryContainer::clearLibraryPassword( const String& rLibraryName )
{
    try
    {
        SfxLibrary* pImplLib = getImplLib( rLibraryName );
        pImplLib->mbDoc50Password = sal_False;
        pImplLib->mbPasswordProtected = sal_False;
        pImplLib->maPassword = OUString();
    }
    catch( NoSuchElementException& ) {}
}

sal_Bool SfxScriptLibraryContainer::hasLibraryPassword( const String& rLibraryName )
{
    SfxLibrary* pImplLib = getImplLib( rLibraryName );
    return pImplLib->mbPasswordProtected;
}

// Ctor for service
SfxScriptLibraryContainer::SfxScriptLibraryContainer( void )
    :maScriptLanguage( RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ) )
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

SfxScriptLibraryContainer::SfxScriptLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
    :maScriptLanguage( RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ) )
{
    init( OUString(), xStorage );
}

// Methods to get library instances of the correct type
SfxLibrary* SfxScriptLibraryContainer::implCreateLibrary( const OUString& aName )
{
    (void)aName;    // Only needed for SfxDialogLibrary
    SfxLibrary* pRet = new SfxScriptLibrary( maModifiable, mxMSF, mxSFI );
    return pRet;
}

SfxLibrary* SfxScriptLibraryContainer::implCreateLibraryLink
    ( const OUString& aName, const OUString& aLibInfoFileURL,
      const OUString& StorageURL, sal_Bool ReadOnly )
{
    (void)aName;    // Only needed for SfxDialogLibrary
    SfxLibrary* pRet =
        new SfxScriptLibrary
            ( maModifiable, mxMSF, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly );
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

void SAL_CALL SfxScriptLibraryContainer::writeLibraryElement
(
    const Reference < XNameContainer >& xLib,
    const OUString& aElementName,
    const Reference< XOutputStream >& xOutput
)
    throw(Exception)
{
    // Create sax writer
    Reference< XExtendedDocumentHandler > xHandler(
        mxMSF->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
    if( !xHandler.is() )
    {
        OSL_FAIL( "### couln't create sax-writer component\n" );
        return;
    }

    Reference< XTruncate > xTruncate( xOutput, UNO_QUERY );
    OSL_ENSURE( xTruncate.is(), "Currently only the streams that can be truncated are expected!" );
    if ( xTruncate.is() )
        xTruncate->truncate();

    Reference< XActiveDataSource > xSource( xHandler, UNO_QUERY );
    xSource->setOutputStream( xOutput );

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
            aMod.aModuleType = OUString( RTL_CONSTASCII_USTRINGPARAM("normal") );
            break;
        case ModuleType::CLASS:
            aMod.aModuleType = OUString( RTL_CONSTASCII_USTRINGPARAM("class") );
            break;
        case ModuleType::FORM:
            aMod.aModuleType = OUString( RTL_CONSTASCII_USTRINGPARAM("form") );
            break;
        case ModuleType::DOCUMENT:
            aMod.aModuleType = OUString( RTL_CONSTASCII_USTRINGPARAM("document") );
            break;
        case ModuleType::UNKNOWN:
            // nothing
            break;
        }
    }

    xmlscript::exportScriptModule( xHandler, aMod );
}


Any SAL_CALL SfxScriptLibraryContainer::importLibraryElement
    ( const Reference < XNameContainer >& xLib,
      const OUString& aElementName, const OUString& aFile,
      const uno::Reference< io::XInputStream >& xInStream )
{
    Any aRetAny;

    Reference< XParser > xParser( mxMSF->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
    if( !xParser.is() )
    {
        OSL_FAIL( "### couln't create sax parser component\n" );
        return aRetAny;
    }


    // Read from storage?
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
        catch( Exception& )
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
    catch( Exception& )
    {
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        sal_uIntPtr nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
    }

    aRetAny <<= aMod.aCode;

    // TODO: Check language
    // aMod.aLanguage
    // aMod.aName ignored
    if( aMod.aModuleType.getLength() > 0 )
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
            xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAGlobals" ) ) );
        }
        catch( Exception& )
        {
        }

        script::ModuleInfo aModInfo;
        aModInfo.ModuleType = ModuleType::UNKNOWN;
        if( aMod.aModuleType.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("normal") ))
        {
            aModInfo.ModuleType = ModuleType::NORMAL;
        }
        else if( aMod.aModuleType.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("class") ))
        {
            aModInfo.ModuleType = ModuleType::CLASS;
        }
        else if( aMod.aModuleType.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("form") ))
        {
            aModInfo.ModuleType = ModuleType::FORM;
            aModInfo.ModuleObject = mxOwnerDocument;
        }
        else if( aMod.aModuleType.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("document") ))
        {
            aModInfo.ModuleType = ModuleType::DOCUMENT;

            // #163691# use the same codename access instance for all document modules
            if( !mxCodeNameAccess.is() ) try
            {
                Reference<frame::XModel > xModel( mxOwnerDocument );
                Reference< XMultiServiceFactory> xSF( xModel, UNO_QUERY_THROW );
                mxCodeNameAccess.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAObjectModuleObjectProvider" ) ) ), UNO_QUERY );
            }
            catch( Exception& ) {}

            if( mxCodeNameAccess.is() )
            {
                try
                {
                    aModInfo.ModuleObject.set( mxCodeNameAccess->getByName( aElementName), uno::UNO_QUERY );
                }
                catch(uno::Exception&)
                {
                    OSL_TRACE("Failed to get documument object for %s", rtl::OUStringToOString( aElementName, RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
        }

        Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, UNO_QUERY );
        if( xVBAModuleInfo.is() )
        {
            if( xVBAModuleInfo->hasModuleInfo( aElementName ) )
                xVBAModuleInfo->removeModuleInfo( aElementName );
            xVBAModuleInfo->insertModuleInfo( aElementName, aModInfo );
        }
    }

    return aRetAny;
}

SfxLibraryContainer* SfxScriptLibraryContainer::createInstanceImpl( void )
{
    return new SfxScriptLibraryContainer();
}

void SAL_CALL SfxScriptLibraryContainer::importFromOldStorage( const ::rtl::OUString& aFile )
{
    // TODO: move loading from old storage to binary filters?
    SotStorageRef xStorage = new SotStorage( sal_False, aFile );
    if( xStorage.Is() && xStorage->GetError() == ERRCODE_NONE )
    {
        // We need a BasicManager to avoid problems
        // StarBASIC* pBas = new StarBASIC();
        BasicManager* pBasicManager = new BasicManager( *(SotStorage*)xStorage, aFile );

        // Set info
        LibraryContainerInfo aInfo( this, NULL, static_cast< OldBasicPassword* >( this ) );
        pBasicManager->SetLibraryContainerInfo( aInfo );

        // Now the libraries should be copied to this SfxScriptLibraryContainer
        BasicManager::LegacyDeleteBasicManager( pBasicManager );
    }
}


// Storing with password encryption

// Methods XLibraryContainerPassword
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
        throw IllegalArgumentException();
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
        throw IllegalArgumentException();

    // Test password
    sal_Bool bSuccess = sal_False;
    if( pImplLib->mbDoc50Password )
    {
        bSuccess = ( Password == pImplLib->maPassword );
        if( bSuccess )
            pImplLib->mbPasswordVerified = sal_True;
    }
    else
    {
        pImplLib->maPassword = Password;
        bSuccess = implLoadPasswordLibrary( pImplLib, Name, sal_True );
        if( bSuccess )
        {
            // The library gets modified by verifiying the password, because other-
            // wise for saving the storage would be copied and that doesn't work
            // with mtg's storages when the password is verified
            pImplLib->implSetModified( sal_True );
            pImplLib->mbPasswordVerified = sal_True;

            // Reload library to get source
            if( pImplLib->mbLoaded )
                implLoadPasswordLibrary( pImplLib, Name );
        }
    }
    return bSuccess;
}

void SAL_CALL SfxScriptLibraryContainer::changeLibraryPassword( const OUString& Name,
    const OUString& OldPassword, const OUString& NewPassword )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
    LibraryContainerMethodGuard aGuard( *this );
    SfxLibrary* pImplLib = getImplLib( Name );
    if( OldPassword == NewPassword )
        return;

    sal_Bool bOldPassword = ( OldPassword.getLength() > 0 );
    sal_Bool bNewPassword = ( NewPassword.getLength() > 0 );
    sal_Bool bStorage = mxStorage.is() && !pImplLib->mbLink;

    if( pImplLib->mbReadOnly || (bOldPassword && !pImplLib->mbPasswordProtected) )
        throw IllegalArgumentException();

    // Library must be loaded
    loadLibrary( Name );

    sal_Bool bKillCryptedFiles = sal_False;
    sal_Bool bKillUncryptedFiles = sal_False;

    // Remove or change password?
    if( bOldPassword )
    {
        if( isLibraryPasswordVerified( Name ) )
        {
            if( pImplLib->maPassword != OldPassword )
                   throw IllegalArgumentException();
        }
        else
        {
            if( !verifyLibraryPassword( Name, OldPassword ) )
                   throw IllegalArgumentException();

            // Reload library to get source
            // Should be done in verifyLibraryPassword loadLibrary( Name );
        }

        if( !bNewPassword )
        {
            pImplLib->mbPasswordProtected = sal_False;
            pImplLib->mbPasswordVerified = sal_False;
            pImplLib->maPassword = OUString();

            maModifiable.setModified( sal_True );
            pImplLib->implSetModified( sal_True );

            if( !bStorage && !pImplLib->mbDoc50Password )
            {
                // Store application basic uncrypted
                uno::Reference< embed::XStorage > xStorage;
                storeLibraries_Impl( xStorage, sal_False );
                bKillCryptedFiles = sal_True;
            }
        }
    }

    // Set new password?
    if( bNewPassword )
    {
        pImplLib->mbPasswordProtected = sal_True;
        pImplLib->mbPasswordVerified = sal_True;
        pImplLib->maPassword = NewPassword;

        maModifiable.setModified( sal_True );
        pImplLib->implSetModified( sal_True );

        if( !bStorage && !pImplLib->mbDoc50Password )
        {
            // Store applictaion basic crypted
            uno::Reference< embed::XStorage > xStorage;
            storeLibraries_Impl( xStorage, sal_False );
            bKillUncryptedFiles = sal_True;
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
                aElementInetObj.insertName( aElementName, sal_False,
                    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
                if( bKillUncryptedFiles )
                    aElementInetObj.setExtension( maLibElementFileExtension );
                else
                    aElementInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("pba") ) );
                String aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if( mxSFI->exists( aElementPath ) )
                    mxSFI->kill( aElementPath );
            }
        }
        catch( Exception& ) {}
    }
}


void setStreamKey( uno::Reference< io::XStream > xStream, const ::rtl::OUString& aPass )
{
    uno::Reference< embed::XEncryptionProtectedSource > xEncrStream( xStream, uno::UNO_QUERY );
    if ( xEncrStream.is() )
        xEncrStream->setEncryptionPassword( aPass );
}


// Impl methods
sal_Bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib,
    const ::rtl::OUString& aName, const uno::Reference< embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
{
    OUString aDummyLocation;
    Reference< XSimpleFileAccess > xDummySFA;
    return implStorePasswordLibrary( pLib, aName, xStorage, aDummyLocation, xDummySFA, xHandler );
}

sal_Bool SfxScriptLibraryContainer::implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const ::rtl::OUString& aTargetURL, const Reference< XSimpleFileAccess > xToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
{
    bool bExport = aTargetURL.getLength();

    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implStorePasswordLibrary: cannot do this without a BasicManager!" );
    if ( !pBasicMgr )
        return sal_False;

    // Only need to handle the export case here,
    // save/saveas etc are handled in sfxbasemodel::storeSelf &
    // sfxbasemodel::impl_store
    uno::Sequence<rtl::OUString> aNames;
    if ( bExport && pBasicMgr->LegacyPsswdBinaryLimitExceeded(aNames) )
    {
        if ( xHandler.is() )
        {
            ModuleSizeExceeded* pReq =  new ModuleSizeExceeded( aNames );
            uno::Reference< task::XInteractionRequest > xReq( pReq );
            xHandler->handle( xReq );
            if ( pReq->isAbort() )
                throw util::VetoException();
        }
    }

    StarBASIC* pBasicLib = pBasicMgr->GetLib( aName );
    if( !pBasicLib )
        return sal_False;

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

            // Write binary image stream
            SbModule* pMod = pBasicLib->FindModule( aElementName );
            if( pMod )
            {
                //OUString aCodeStreamName( RTL_CONSTASCII_USTRINGPARAM("code.bin") );
                OUString aCodeStreamName = aElementName;
                aCodeStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".bin") );

                try {
                    uno::Reference< io::XStream > xCodeStream = xStorage->openStreamElement(
                                        aCodeStreamName,
                                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                    if ( !xCodeStream.is() )
                        throw uno::RuntimeException();

                    SvMemoryStream aMemStream;
                    /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream );

                    sal_Int32 nSize = (sal_Int32)aMemStream.Tell();
                    Sequence< sal_Int8 > aBinSeq( nSize );
                    sal_Int8* pData = aBinSeq.getArray();
                    ::rtl_copyMemory( pData, aMemStream.GetData(), nSize );

                       Reference< XOutputStream > xOut = xCodeStream->getOutputStream();
                    if ( !xOut.is() )
                        throw io::IOException(); // access denied because the stream is readonly

                       xOut->writeBytes( aBinSeq );
                    xOut->closeOutput();
                }
                catch( uno::Exception& )
                {
                    // TODO: handle error
                }
            }

            if( pLib->mbPasswordVerified || pLib->mbDoc50Password )
            {
                /*Any aElement = pLib->getByName( aElementName );*/
                if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
                {
                #if OSL_DEBUG_LEVEL > 0
                    ::rtl::OStringBuffer aMessage;
                    aMessage.append( "invalid library element '" );
                    aMessage.append( ::rtl::OUStringToOString( aElementName, osl_getThreadTextEncoding() ) );
                    aMessage.append( "'." );
                    OSL_FAIL( aMessage.makeStringAndClear().getStr() );
                #endif
                    continue;
                }

                OUString aSourceStreamName = aElementName;
                aSourceStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

                try {
                    uno::Reference< io::XStream > xSourceStream = xStorage->openStreamElement(
                                                                aSourceStreamName,
                                                                embed::ElementModes::READWRITE );
                    uno::Reference< beans::XPropertySet > xProps( xSourceStream, uno::UNO_QUERY );
                    if ( !xProps.is() )
                        throw uno::RuntimeException();

                    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
                    OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

                    // Set encryption key
                    setStreamKey( xSourceStream, pLib->maPassword );

                    Reference< XOutputStream > xOutput = xSourceStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOutput );
                    // writeLibraryElement should have the stream already closed
                    // xOutput->closeOutput();
                }
                catch( uno::Exception& )
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
            Reference< XSimpleFileAccess > xSFI = mxSFI;
            if( xToUseSFI.is() )
                xSFI = xToUseSFI;

            OUString aLibDirPath;
            if( bExport )
            {
                INetURLObject aInetObj( aTargetURL );
                aInetObj.insertName( aName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
                aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

                if( !xSFI->isFolder( aLibDirPath ) )
                    xSFI->createFolder( aLibDirPath );
            }
            else
            {
                aLibDirPath = createAppLibraryFolder( pLib, aName );
            }

            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, sal_False,
                    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("pba") ) );
                String aElementPath = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

                /*Any aElement = pLib->getByName( aElementName );*/
                if( !isLibraryElementValid( pLib->getByName( aElementName ) ) )
                {
                #if OSL_DEBUG_LEVEL > 0
                    ::rtl::OStringBuffer aMessage;
                    aMessage.append( "invalid library element '" );
                    aMessage.append( ::rtl::OUStringToOString( aElementName, osl_getThreadTextEncoding() ) );
                    aMessage.append( "'." );
                    OSL_FAIL( aMessage.makeStringAndClear().getStr() );
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
                        throw uno::RuntimeException();

                    // Write binary image stream
                    SbModule* pMod = pBasicLib->FindModule( aElementName );
                    if( pMod )
                    {
                        OUString aCodeStreamName( RTL_CONSTASCII_USTRINGPARAM("code.bin") );

                        uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                            aCodeStreamName,
                                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                        SvMemoryStream aMemStream;
                        /*sal_Bool bStore = */pMod->StoreBinaryData( aMemStream );

                        sal_Int32 nSize = (sal_Int32)aMemStream.Tell();
                        Sequence< sal_Int8 > aBinSeq( nSize );
                        sal_Int8* pData = aBinSeq.getArray();
                        ::rtl_copyMemory( pData, aMemStream.GetData(), nSize );

                        Reference< XOutputStream > xOut = xCodeStream->getOutputStream();
                        if ( xOut.is() )
                        {
                            xOut->writeBytes( aBinSeq );
                            xOut->closeOutput();
                        }
                    }

                    // Write encrypted source stream
                    OUString aSourceStreamName( RTL_CONSTASCII_USTRINGPARAM("source.xml") );

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
                            throw uno::RuntimeException();
                        xEncr->setEncryptionPassword( pLib->maPassword );
                    }
                    catch( ::com::sun::star::packages::WrongPasswordException& )
                    {
                        xSourceStream = xElementRootStorage->openEncryptedStreamElement(
                            aSourceStreamName,
                            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,
                            pLib->maPassword );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSourceStream, uno::UNO_QUERY );
                    if ( !xProps.is() )
                        throw uno::RuntimeException();
                    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
                    OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

                    Reference< XOutputStream > xOut = xSourceStream->getOutputStream();
                    Reference< XNameContainer > xLib( pLib );
                    writeLibraryElement( xLib, aElementName, xOut );
                    // i50568: sax writer already closes stream
                    // xOut->closeOutput();

                    uno::Reference< embed::XTransactedObject > xTransact( xElementRootStorage, uno::UNO_QUERY );
                    OSL_ENSURE( xTransact.is(), "The storage must implement XTransactedObject!\n" );
                    if ( !xTransact.is() )
                        throw uno::RuntimeException();

                    xTransact->commit();
                }
                catch( uno::Exception& )
                {
                    // TODO: handle error
                }

                // Storage Dtor commits too, that makes problems
                // xElementRootStorage->Commit();
            }
        }
        catch( Exception& )
        {
            //throw e;
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

    // Already loaded? Then only verifiedPassword can change something
    SfxScriptLibrary* pScriptLib = static_cast< SfxScriptLibrary* >( pLib );
    if( pScriptLib->mbLoaded )
    {
        if( pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly &&
            (pScriptLib->mbLoadedSource || !pLib->mbPasswordVerified) )
                return sal_False;
    }

    StarBASIC* pBasicLib = NULL;
    sal_Bool bLoadBinary = sal_False;
    if( !pScriptLib->mbLoadedBinary && !bVerifyPasswordOnly && !pLib->mbPasswordVerified )
    {
        BasicManager* pBasicMgr = getBasicManager();
        OSL_ENSURE( pBasicMgr, "SfxScriptLibraryContainer::implLoadPasswordLibrary: cannot do this without a BasicManager!" );
        sal_Bool bLoaded = pScriptLib->mbLoaded;
        pScriptLib->mbLoaded = sal_True;        // Necessary to get lib
        pBasicLib = pBasicMgr ? pBasicMgr->GetLib( Name ) : NULL;
        pScriptLib->mbLoaded = bLoaded;    // Restore flag
        if( !pBasicLib )
            return sal_False;

        bLoadBinary = sal_True;
        pScriptLib->mbLoadedBinary = sal_True;
    }

    sal_Bool bLoadSource = sal_False;
    if( !pScriptLib->mbLoadedSource && pLib->mbPasswordVerified && !bVerifyPasswordOnly )
    {
        bLoadSource = sal_True;
        pScriptLib->mbLoadedSource = sal_True;
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
                    throw uno::RuntimeException();

                xLibraryStor = xLibrariesStor->openStorageElement( Name, embed::ElementModes::READ );
                if ( !xLibraryStor.is() )
                    throw uno::RuntimeException();
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "### couln't open sub storage for library\n" );
                return sal_False;
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
                    pMod = pBasicLib->MakeModule( aElementName, String() );
                    pBasicLib->SetModified( sal_False );
                }

                //OUString aCodeStreamName( RTL_CONSTASCII_USTRINGPARAM("code.bin") );
                OUString aCodeStreamName= aElementName;
                aCodeStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".bin") );

                try {
                    uno::Reference< io::XStream > xCodeStream = xLibraryStor->openStreamElement(
                                                                                        aCodeStreamName,
                                                                                        embed::ElementModes::READ );
                    if ( !xCodeStream.is() )
                        throw uno::RuntimeException();

                    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xCodeStream );
                    if ( !pStream || pStream->GetError() )
                    {
                        sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                        delete pStream;
                        throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), nError );
                    }

                    /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                    // TODO: Check return value

                    delete pStream;
                }
                catch( uno::Exception& )
                {
                    // TODO: error handling
                }
            }

            // Load source
            if( bLoadSource || bVerifyPasswordOnly )
            {
                // Access encrypted source stream
                OUString aSourceStreamName = aElementName;
                aSourceStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

                try {
                    uno::Reference< io::XStream > xSourceStream = xLibraryStor->openEncryptedStreamElement(
                                                                    aSourceStreamName,
                                                                    embed::ElementModes::READ,
                                                                    pLib->maPassword );
                    if ( !xSourceStream.is() )
                        throw uno::RuntimeException();

                    // if this point is reached then the password is correct
                    if ( !bVerifyPasswordOnly )
                    {
                        uno::Reference< io::XInputStream > xInStream = xSourceStream->getInputStream();
                        if ( !xInStream.is() )
                            throw io::IOException(); // read access denied, seems to be impossible

                        Reference< XNameContainer > xLib( pLib );
                        Any aAny = importLibraryElement( xLib,
                                        aElementName, aSourceStreamName,
                                           xInStream );
                        if( pLib->hasByName( aElementName ) )
                        {
                            if( aAny.hasValue() )
                                pLib->maNameContainer.replaceByName( aElementName, aAny );
                        }
                        else
                        {
                            pLib->maNameContainer.insertByName( aElementName, aAny );
                        }
                    }
                }
                catch( uno::Exception& )
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
                aElementInetObj.insertName( aElementName, sal_False,
                    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("pba") ) );
                String aElementPath = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

                uno::Reference< embed::XStorage > xElementRootStorage;
                try {
                    xElementRootStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                                                                    aElementPath,
                                                                    embed::ElementModes::READ );
                } catch( uno::Exception& )
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
                            pMod = pBasicLib->MakeModule( aElementName, String() );
                            pBasicLib->SetModified( sal_False );
                        }

                        try {
                            OUString aCodeStreamName( RTL_CONSTASCII_USTRINGPARAM("code.bin") );
                            uno::Reference< io::XStream > xCodeStream = xElementRootStorage->openStreamElement(
                                                                        aCodeStreamName,
                                                                        embed::ElementModes::READ );

                            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xCodeStream );
                            if ( !pStream || pStream->GetError() )
                            {
                                sal_Int32 nError = pStream ? pStream->GetError() : ERRCODE_IO_GENERAL;
                                delete pStream;
                                throw task::ErrorCodeIOException( ::rtl::OUString(),
                                                                    uno::Reference< uno::XInterface >(),
                                                                    nError );
                            }

                            /*sal_Bool bRet = */pMod->LoadBinaryData( *pStream );
                            // TODO: Check return value

                            delete pStream;
                        }
                        catch( uno::Exception& )
                        {
                            // TODO: error handling
                        }
                    }

                    // Load source
                    if( bLoadSource || bVerifyPasswordOnly )
                    {
                        // Access encrypted source stream
                        OUString aSourceStreamName( RTL_CONSTASCII_USTRINGPARAM("source.xml") );
                        try {
                            uno::Reference< io::XStream > xSourceStream = xElementRootStorage->openEncryptedStreamElement(
                                                                    aSourceStreamName,
                                                                    embed::ElementModes::READ,
                                                                    pLib->maPassword );
                            if ( !xSourceStream.is() )
                                throw uno::RuntimeException();

                            if ( !bVerifyPasswordOnly )
                            {
                                uno::Reference< io::XInputStream > xInStream = xSourceStream->getInputStream();
                                if ( !xInStream.is() )
                                    throw io::IOException(); // read access denied, seems to be impossible

                                Reference< XNameContainer > xLib( pLib );
                                Any aAny = importLibraryElement( xLib,
                                                aElementName,
                                                aSourceStreamName,
                                                xInStream );
                                if( pLib->hasByName( aElementName ) )
                                {
                                    if( aAny.hasValue() )
                                        pLib->maNameContainer.replaceByName( aElementName, aAny );
                                }
                                else
                                {
                                    pLib->maNameContainer.insertByName( aElementName, aAny );
                                }
                            }
                        }
                        catch ( uno::Exception& )
                        {
                            bRet = sal_False;
                        }
                    }
                }
            }

        }
        catch( Exception& )
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

sal_Bool SAL_CALL
SfxScriptLibraryContainer:: HasExecutableCode( const ::rtl::OUString& Library ) throw (uno::RuntimeException)
{
    BasicManager* pBasicMgr = getBasicManager();
    OSL_ENSURE( pBasicMgr, "we need a basicmanager, really we do" );
    if ( pBasicMgr )
        return pBasicMgr->HasExeCode( Library ); // need to change this to take name
    // default to it has code if we can't decide
    return sal_True;
}

//============================================================================
// Service
void createRegistryInfo_SfxScriptLibraryContainer()
{
    static OAutoRegistration< SfxScriptLibraryContainer > aAutoRegistration;
}

::rtl::OUString SAL_CALL SfxScriptLibraryContainer::getImplementationName( ) throw (RuntimeException)
{
    return getImplementationName_static();
}

Sequence< ::rtl::OUString > SAL_CALL SfxScriptLibraryContainer::getSupportedServiceNames( ) throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SfxScriptLibraryContainer::getSupportedServiceNames_static()
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.DocumentScriptLibraryContainer" ));
    // plus, for compatibility:
    aServiceNames[1] = OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.ScriptLibraryContainer" ));
    return aServiceNames;
}

OUString SfxScriptLibraryContainer::getImplementationName_static()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sfx2.ScriptLibraryContainer" ));
        bNeedsInit = sal_False;
    }
    return aImplName;
}

Reference< XInterface > SAL_CALL SfxScriptLibraryContainer::Create
    ( const Reference< XComponentContext >& )
        throw( Exception )
{
    Reference< XInterface > xRet =
        static_cast< XInterface* >( static_cast< OWeakObject* >(new SfxScriptLibraryContainer()) );
    return xRet;
}

//============================================================================
// Implementation class SfxScriptLibrary

// Ctor
SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XMultiServiceFactory >& xMSF,
                                    const Reference< XSimpleFileAccess >& xSFI )
    : SfxLibrary( _rModifiable, getCppuType( (const OUString *)0 ), xMSF, xSFI )
    , mbLoadedSource( sal_False )
    , mbLoadedBinary( sal_False )
{
}

SfxScriptLibrary::SfxScriptLibrary( ModifiableHelper& _rModifiable,
                                    const Reference< XMultiServiceFactory >& xMSF,
                                    const Reference< XSimpleFileAccess >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    sal_Bool ReadOnly )
    : SfxLibrary( _rModifiable, getCppuType( (const OUString *)0 ), xMSF, xSFI,
                        aLibInfoFileURL, aStorageURL, ReadOnly)
    , mbLoadedSource( sal_False )
    , mbLoadedBinary( sal_False )
{
}

// Provide modify state including resources
sal_Bool SfxScriptLibrary::isModified( void )
{
    return implIsModified();    // No resources
}

void SfxScriptLibrary::storeResources( void )
{
    // No resources
}

void SfxScriptLibrary::storeResourcesToURL( const ::rtl::OUString& URL,
    const Reference< task::XInteractionHandler >& Handler )
{
    (void)URL;
    (void)Handler;
}

void SfxScriptLibrary::storeResourcesAsURL
    ( const ::rtl::OUString& URL, const ::rtl::OUString& NewName )
{
    (void)URL;
    (void)NewName;
}

void SfxScriptLibrary::storeResourcesToStorage( const ::com::sun::star::uno::Reference
    < ::com::sun::star::embed::XStorage >& xStorage )
{
    // No resources
    (void)xStorage;
}

bool SfxScriptLibrary::containsValidModule( const Any& aElement )
{
    OUString sModuleText;
    aElement >>= sModuleText;
    return ( sModuleText.getLength() > 0 );
}

bool SAL_CALL SfxScriptLibrary::isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const
{
    return SfxScriptLibrary::containsValidModule( aElement );
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxScriptLibrary, SfxLibrary, SfxScriptLibrary_BASE );

script::ModuleInfo SAL_CALL
SfxScriptLibrary::getModuleInfo( const ::rtl::OUString& ModuleName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( !hasModuleInfo( ModuleName ) )
        throw NoSuchElementException();
    return mModuleInfos[ ModuleName ];
}

sal_Bool SAL_CALL
SfxScriptLibrary::hasModuleInfo( const ::rtl::OUString& ModuleName ) throw (RuntimeException)
{
    sal_Bool bRes = sal_False;
    ModuleInfoMap::iterator it = mModuleInfos.find( ModuleName );

    if ( it != mModuleInfos.end() )
        bRes = sal_True;

    return bRes;
}

void SAL_CALL SfxScriptLibrary::insertModuleInfo( const ::rtl::OUString& ModuleName, const script::ModuleInfo& ModuleInfo ) throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if ( hasModuleInfo( ModuleName ) )
        throw ElementExistException();
    mModuleInfos[ ModuleName ] = ModuleInfo;
}

void SAL_CALL SfxScriptLibrary::removeModuleInfo( const ::rtl::OUString& ModuleName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
        // #FIXME add NoSuchElementException to the spec
    if ( !hasModuleInfo( ModuleName ) )
        throw NoSuchElementException();
    mModuleInfos.erase( mModuleInfos.find( ModuleName ) );
}


//============================================================================

}   // namespace basic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
