/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scriptcont.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:54:17 $
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
#include "precompiled_basic.hxx"

#ifndef BASIC_SCRIPTCONTAINER_HXX
#include "scriptcont.hxx"
#endif
#ifndef SBMODULE_HXX
#include "sbmodule.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XENCRYPTIONPROTECTEDSOURCE_HPP_
#include <com/sun/star/embed/XEncryptionProtectedSource.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_ERRORCODEIOEXCEPTION_HPP_
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX_
#include <comphelper/storagehelper.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif

// For password functionality
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif


#include <svtools/pathoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include "modsizeexceeded.hxx"
#include <xmlscript/xmlmod_imexp.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/util/VetoException.hpp>

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
using namespace rtl;
using namespace osl;

using com::sun::star::uno::Reference;

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

sal_Bool SAL_CALL SfxScriptLibraryContainer::isLibraryElementValid( Any aElement )
{
    OUString aMod;
    aElement >>= aMod;
    sal_Bool bRet = (aMod.getLength() > 0);
    return bRet;
}

void SAL_CALL SfxScriptLibraryContainer::writeLibraryElement
(
    Any aElement,
    const OUString& aElementName,
    Reference< XOutputStream > xOutput
)
    throw(Exception)
{
    // Create sax writer
    Reference< XExtendedDocumentHandler > xHandler(
        mxMSF->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
    if( !xHandler.is() )
    {
        OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
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
    aElement >>= aMod.aCode;
    xmlscript::exportScriptModule( xHandler, aMod );
}


Any SAL_CALL SfxScriptLibraryContainer::importLibraryElement
    ( const OUString& aFile, const uno::Reference< io::XInputStream >& xInStream )
{
    Any aRetAny;

    Reference< XParser > xParser( mxMSF->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
    if( !xParser.is() )
    {
        OSL_ENSURE( 0, "### couln't create sax parser component\n" );
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
        ULONG nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
    }

    aRetAny <<= aMod.aCode;

    // TODO: Check language
    // aMod.aLanguage
    // aMod.aName ignored

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
                    /*BOOL bStore = */pMod->StoreBinaryData( aMemStream );

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
                Any aElement = pLib->getByName( aElementName );
                if( isLibraryElementValid( aElement ) )
                {
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
                        writeLibraryElement( aElement, aElementName, xOutput );
                        // writeLibraryElement should have the stream already closed
                        // xOutput->closeOutput();
                    }
                    catch( uno::Exception& )
                    {
                        OSL_ENSURE( sal_False, "Problem on storing of password library!\n" );
                        // TODO: error handling
                    }
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

                Any aElement = pLib->getByName( aElementName );
                if( isLibraryElementValid( aElement ) )
                {
                    try {
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
                            /*BOOL bStore = */pMod->StoreBinaryData( aMemStream );

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
                        writeLibraryElement( aElement, aElementName, xOut );
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
                OSL_ENSURE( 0, "### couln't open sub storage for library\n" );
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
                    pBasicLib->SetModified( FALSE );
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

                    /*BOOL bRet = */pMod->LoadBinaryData( *pStream );
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

                        Any aAny = importLibraryElement( aSourceStreamName, xInStream );
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
                                                                    embed::ElementModes::READWRITE );
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
                            pBasicLib->SetModified( FALSE );
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

                            /*BOOL bRet = */pMod->LoadBinaryData( *pStream );
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

                                Any aAny = importLibraryElement( aSourceStreamName, xInStream );
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

//REMOVE        // If the password is verified the library must remain modified, because
//REMOVE        // otherwise for saving the storage would be copied and that doesn't work
//REMOVE        // with mtg's storages when the password is verified
//REMOVE        if( !pLib->mbPasswordVerified )
//REMOVE            pLib->mbModified = sal_False;
    return bRet;
}


void SfxScriptLibraryContainer::onNewRootStorage()
{
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
    aServiceNames[0] = OUString::createFromAscii( "com.sun.star.script.DocumentScriptLibraryContainer" );
    // plus, for compatibility:
    aServiceNames[1] = OUString::createFromAscii( "com.sun.star.script.ScriptLibraryContainer" );
    return aServiceNames;
}

OUString SfxScriptLibraryContainer::getImplementationName_static()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString::createFromAscii( "com.sun.star.comp.sfx2.ScriptLibraryContainer" );
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

//============================================================================

}   // namespace basic
