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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include "com/sun/star/resource/XStringResourceWithStorage.hpp"
#include "com/sun/star/resource/XStringResourceWithLocation.hpp"
#include "dlgcont.hxx"
#include "sbmodule.hxx"
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/pathoptions.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <cppuhelper/factory.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>


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
// Implementation class SfxDialogLibraryContainer

const sal_Char* SAL_CALL SfxDialogLibraryContainer::getInfoFileName() const { return "dialog"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getOldInfoFileName() const { return "dialogs"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getLibElementFileExtension() const { return "xdl"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getLibrariesDir() const { return "Dialogs"; }

// Ctor for service
SfxDialogLibraryContainer::SfxDialogLibraryContainer( void )
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

SfxDialogLibraryContainer::SfxDialogLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
{
    init( OUString(), xStorage );
}

// Methods to get library instances of the correct type
SfxLibrary* SfxDialogLibraryContainer::implCreateLibrary( const ::rtl::OUString& aName )
{
    SfxLibrary* pRet = new SfxDialogLibrary( maModifiable, aName, mxMSF, mxSFI, this );
    return pRet;
}

SfxLibrary* SfxDialogLibraryContainer::implCreateLibraryLink
    ( const ::rtl::OUString& aName, const OUString& aLibInfoFileURL,
      const OUString& StorageURL, sal_Bool ReadOnly )
{
    SfxLibrary* pRet = new SfxDialogLibrary
            ( maModifiable, aName, mxMSF, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly, this );
    return pRet;
}

Any SAL_CALL SfxDialogLibraryContainer::createEmptyLibraryElement( void )
{
    Reference< XInputStreamProvider > xISP;
    Any aRetAny;
    aRetAny <<= xISP;
    return aRetAny;
}

bool SAL_CALL SfxDialogLibraryContainer::isLibraryElementValid( Any aElement ) const
{
    return SfxDialogLibrary::containsValidDialog( aElement );
}

bool writeOasis2OOoLibraryElement(
    Reference< XInputStream > xInput, Reference< XOutputStream > xOutput )
{
    Reference< XMultiServiceFactory > xMSF(
        comphelper::getProcessServiceFactory() );

    Reference< XComponentContext > xContext;
    Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue(
        OUString::createFromAscii(("DefaultContext")) ) >>= xContext );

    Reference< lang::XMultiComponentFactory > xSMgr(
        xContext->getServiceManager() );

    if (! xSMgr.is())
    {
        return sal_False;
    }

    Reference< xml::sax::XParser > xParser(
        xSMgr->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.xml.sax.Parser" ) ),
            xContext ),
        UNO_QUERY );

    Reference< xml::sax::XExtendedDocumentHandler > xWriter(
        xSMgr->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.xml.sax.Writer" ) ),
            xContext ),
        UNO_QUERY );

    Reference< io::XActiveDataSource > xSource( xWriter, UNO_QUERY );
    xSource->setOutputStream( xOutput );

    if ( !xParser.is() || !xWriter.is() )
    {
        return sal_False;
    }

    Sequence<Any> aArgs( 1 );
    aArgs[0] <<= xWriter;

    Reference< xml::sax::XDocumentHandler > xHandler(
        xSMgr->createInstanceWithArgumentsAndContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.Oasis2OOoTransformer" ) ),
            aArgs, xContext ),
        UNO_QUERY );

    xParser->setDocumentHandler( xHandler );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUString::createFromAscii( "virtual file" );

    xParser->parseStream( source );

    return sal_True;
}

void SAL_CALL SfxDialogLibraryContainer::writeLibraryElement
(
    const Reference < XNameContainer >& xLib,
    const OUString& aElementName,
    const Reference< XOutputStream >& xOutput
)
    throw(Exception)
{
    Any aElement = xLib->getByName( aElementName );
    Reference< XInputStreamProvider > xISP;
    aElement >>= xISP;
    if( !xISP.is() )
        return;

    Reference< XInputStream > xInput( xISP->createInputStream() );

    bool bComplete = sal_False;
    if ( mbOasis2OOoFormat )
    {
        bComplete = writeOasis2OOoLibraryElement( xInput, xOutput );
    }

    if ( bComplete == sal_False )
    {
        Sequence< sal_Int8 > bytes;
        sal_Int32 nRead = xInput->readBytes( bytes, xInput->available() );
        for (;;)
        {
            if( nRead )
                xOutput->writeBytes( bytes );

            nRead = xInput->readBytes( bytes, 1024 );
            if (! nRead)
                break;
        }
    }
    xInput->closeInput();
}

void SfxDialogLibraryContainer::storeLibrariesToStorage( const uno::Reference< embed::XStorage >& xStorage ) throw ( RuntimeException )
{
    LibraryContainerMethodGuard aGuard( *this );
    mbOasis2OOoFormat = sal_False;

    if ( mxStorage.is() && xStorage.is() )
    {
        try
        {
            long nSource = SotStorage::GetVersion( mxStorage );
            long nTarget = SotStorage::GetVersion( xStorage );

            if ( nSource == SOFFICE_FILEFORMAT_CURRENT &&
                nTarget != SOFFICE_FILEFORMAT_CURRENT )
            {
                mbOasis2OOoFormat = sal_True;
            }
        }
        catch ( Exception& )
        {
            // if we cannot get the version then the
            // Oasis2OOoTransformer will not be used
            OSL_ASSERT(sal_False);
        }
    }

    SfxLibraryContainer::storeLibrariesToStorage( xStorage );

    mbOasis2OOoFormat = sal_False;
}


Any SAL_CALL SfxDialogLibraryContainer::importLibraryElement
    ( const Reference < XNameContainer >& /*xLib*/,
      const OUString& /*aElementName */, const OUString& aFile,
      const uno::Reference< io::XInputStream >& xElementStream )
{
    Any aRetAny;

    // TODO: Member because later it will be a component
    //Reference< XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory() );
    //if( !xMSF.is() )
    //{
    //  OSL_ENSURE( 0, "### couln't get ProcessServiceFactory\n" );
    //  return aRetAny;
    //}

    Reference< XParser > xParser( mxMSF->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
    if( !xParser.is() )
    {
        OSL_ENSURE( 0, "### couln't create sax parser component\n" );
        return aRetAny;
    }

    Reference< XNameContainer > xDialogModel( mxMSF->createInstance
        ( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY );
    if( !xDialogModel.is() )
    {
        OSL_ENSURE( 0, "### couln't create com.sun.star.awt.UnoControlDialogModel component\n" );
        return aRetAny;
    }

    // Read from storage?
    sal_Bool bStorage = xElementStream.is();
    Reference< XInputStream > xInput;

    if( bStorage )
    {
        xInput = xElementStream;
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

    Reference< XComponentContext > xContext;
    Reference< beans::XPropertySet > xProps( mxMSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );

    InputSource source;
    source.aInputStream = xInput;
    source.sSystemId    = aFile;

    try {
        // start parsing
        xParser->setDocumentHandler( ::xmlscript::importDialogModel( xDialogModel, xContext ) );
        xParser->parseStream( source );
    }
    catch( Exception& )
    {
        OSL_ENSURE( 0, "Parsing error\n" );
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        sal_uIntPtr nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
        return aRetAny;
    }

    // Create InputStream, TODO: Implement own InputStreamProvider
    // to avoid creating the DialogModel here!
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext );
    aRetAny <<= xISP;
    return aRetAny;
}

void SAL_CALL SfxDialogLibraryContainer::importFromOldStorage( const OUString& )
{
    // Nothing to do here, old dialogs cannot be imported
}

SfxLibraryContainer* SfxDialogLibraryContainer::createInstanceImpl( void )
{
    return new SfxDialogLibraryContainer();
}


static OUString aResourceFileNameBase = OUString::createFromAscii( "DialogStrings" );
static OUString aResourceFileCommentBase = OUString::createFromAscii( "# Strings for Dialog Library " );

// Resource handling
Reference< ::com::sun::star::resource::XStringResourcePersistence >
    SfxDialogLibraryContainer::implCreateStringResource( SfxDialogLibrary* pDialogLibrary )
{
    Reference< resource::XStringResourcePersistence > xRet;
    OUString aLibName = pDialogLibrary->getName();
    bool bReadOnly = pDialogLibrary->mbReadOnly;

    // get ui locale
    ::com::sun  ::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

    OUString aComment = aResourceFileCommentBase;
    aComment += aLibName;

    sal_Bool bStorage = mxStorage.is();
    if( bStorage )
    {
        Sequence<Any> aArgs( 5 );
        aArgs[1] <<= bReadOnly;
        aArgs[2] <<= aLocale;
        aArgs[3] <<= aResourceFileNameBase;
        aArgs[4] <<= aComment;

        // TODO: Ctor
        xRet = Reference< resource::XStringResourcePersistence >( mxMSF->createInstance
            ( OUString::createFromAscii( "com.sun.star.resource.StringResourceWithStorage" ) ), UNO_QUERY );

        uno::Reference< embed::XStorage > xLibrariesStor;
        uno::Reference< embed::XStorage > xLibraryStor;
        try {
            xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
                // TODO: Should be READWRITE with new storage concept using store() instead of storeTo()
            if ( !xLibrariesStor.is() )
                throw uno::RuntimeException();

            xLibraryStor = xLibrariesStor->openStorageElement( aLibName, embed::ElementModes::READ );
                // TODO: Should be READWRITE with new storage concept using store() instead of storeTo()
            if ( !xLibraryStor.is() )
                throw uno::RuntimeException();

            aArgs[0] <<= xLibraryStor;
        }
        catch( uno::Exception& )
        {
            // TODO: Error handling?
            return xRet;
        }

        // TODO: Ctor
        if( xRet.is() )
        {
            Reference< XInitialization > xInit( xRet, UNO_QUERY );
            if( xInit.is() )
                xInit->initialize( aArgs );
        }
    }
    else
    {
        Sequence<Any> aArgs( 6 );

        OUString aLocation = createAppLibraryFolder( pDialogLibrary, aLibName );
        aArgs[0] <<= aLocation;
        aArgs[1] <<= bReadOnly;
        aArgs[2] <<= aLocale;
        aArgs[3] <<= aResourceFileNameBase;
        aArgs[4] <<= aComment;

        // TODO: Real handler?
        Reference< task::XInteractionHandler > xDummyHandler;
        aArgs[5] <<= xDummyHandler;

        // TODO: Ctor
        xRet = Reference< resource::XStringResourcePersistence >( mxMSF->createInstance
            ( OUString::createFromAscii( "com.sun.star.resource.StringResourceWithLocation" ) ), UNO_QUERY );

        // TODO: Ctor
        if( xRet.is() )
        {
            Reference< XInitialization > xInit( xRet, UNO_QUERY );
            if( xInit.is() )
                xInit->initialize( aArgs );
        }
    }

    return xRet;
}

void SfxDialogLibraryContainer::onNewRootStorage()
{
    // the library container is not modified, go through the libraries and check whether they are modified
    Sequence< OUString > aNames = maNameContainer.getElementNames();
    const OUString* pNames = aNames.getConstArray();
    sal_Int32 nNameCount = aNames.getLength();

    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
    {
        OUString aName = pNames[ i ];
        SfxDialogLibrary* pDialogLibrary = static_cast<SfxDialogLibrary*>( getImplLib( aName ) );

        Reference< resource::XStringResourcePersistence > xStringResourcePersistence =
            pDialogLibrary->getStringResourcePersistence();

        if( xStringResourcePersistence.is() )
        {
            Reference< embed::XStorage > xLibrariesStor;
            Reference< embed::XStorage > xLibraryStor;
            try {
                xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READWRITE );
                if ( !xLibrariesStor.is() )
                    throw uno::RuntimeException();

                OUString aLibName = pDialogLibrary->getName();
                xLibraryStor = xLibrariesStor->openStorageElement( aLibName, embed::ElementModes::READWRITE );
                if ( !xLibraryStor.is() )
                    throw uno::RuntimeException();

                Reference< resource::XStringResourceWithStorage >
                    xStringResourceWithStorage( xStringResourcePersistence, UNO_QUERY );
                if( xStringResourceWithStorage.is() )
                    xStringResourceWithStorage->setStorage( xLibraryStor );
            }
            catch( uno::Exception& )
            {
                // TODO: Error handling?
            }
        }
    }
}


//============================================================================
// Service

void createRegistryInfo_SfxDialogLibraryContainer()
{
    static OAutoRegistration< SfxDialogLibraryContainer > aAutoRegistration;
}

::rtl::OUString SAL_CALL SfxDialogLibraryContainer::getImplementationName( ) throw (RuntimeException)
{
    return getImplementationName_static();
}

Sequence< ::rtl::OUString > SAL_CALL SfxDialogLibraryContainer::getSupportedServiceNames( ) throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SfxDialogLibraryContainer::getSupportedServiceNames_static()
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[0] = OUString::createFromAscii( "com.sun.star.script.DocumentDialogLibraryContainer" );
    // plus, for compatibility:
    aServiceNames[1] = OUString::createFromAscii( "com.sun.star.script.DialogLibraryContainer" );
    return aServiceNames;
}

OUString SfxDialogLibraryContainer::getImplementationName_static()
{
    static OUString aImplName;
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        aImplName = OUString::createFromAscii( "com.sun.star.comp.sfx2.DialogLibraryContainer" );
        bNeedsInit = sal_False;
    }
    return aImplName;
}

Reference< XInterface > SAL_CALL SfxDialogLibraryContainer::Create( const Reference< XComponentContext >& ) throw( Exception )
{
    Reference< XInterface > xRet =
        static_cast< XInterface* >( static_cast< OWeakObject* >(new SfxDialogLibraryContainer()) );
    return xRet;
}


//============================================================================
// Implementation class SfxDialogLibrary

// Ctor
SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const ::rtl::OUString& aName,
                                    const Reference< XMultiServiceFactory >& xMSF,
                                    const Reference< XSimpleFileAccess >& xSFI,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, getCppuType( (const Reference< XInputStreamProvider > *)0 ), xMSF, xSFI )
    , m_pParent( pParent )
    , m_aName( aName )
{
}

SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const ::rtl::OUString& aName,
                                    const Reference< XMultiServiceFactory >& xMSF,
                                    const Reference< XSimpleFileAccess >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    sal_Bool ReadOnly,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, getCppuType( (const Reference< XInputStreamProvider > *)0 ),
                       xMSF, xSFI, aLibInfoFileURL, aStorageURL, ReadOnly)
    , m_pParent( pParent )
    , m_aName( aName )
{
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );

// Provide modify state including resources
sal_Bool SfxDialogLibrary::isModified( void )
{
    sal_Bool bRet = implIsModified();

    if( !bRet && m_xStringResourcePersistence.is() )
        bRet = m_xStringResourcePersistence->isModified();
    // else: Resources not accessed so far -> not modified

    return bRet;
}

void SfxDialogLibrary::storeResources( void )
{
    if( m_xStringResourcePersistence.is() )
        m_xStringResourcePersistence->store();
}

void SfxDialogLibrary::storeResourcesAsURL
    ( const ::rtl::OUString& URL, const ::rtl::OUString& NewName )
{
    OUString aComment = aResourceFileCommentBase;
    m_aName = NewName;
    aComment += m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->setComment( aComment );

        Reference< resource::XStringResourceWithLocation >
            xStringResourceWithLocation( m_xStringResourcePersistence, UNO_QUERY );
        if( xStringResourceWithLocation.is() )
            xStringResourceWithLocation->storeAsURL( URL );
    }
}

void SfxDialogLibrary::storeResourcesToURL( const OUString& URL,
    const Reference< task::XInteractionHandler >& xHandler )
{
    OUString aComment = aResourceFileCommentBase;
    aComment += m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToURL
            ( URL, aResourceFileNameBase, aComment, xHandler );
    }
}

void SfxDialogLibrary::storeResourcesToStorage( const ::com::sun::star::uno::Reference
    < ::com::sun::star::embed::XStorage >& xStorage )
{
    OUString aComment = aResourceFileCommentBase;
    aComment += m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToStorage
            ( xStorage, aResourceFileNameBase, aComment );
    }
}


// XStringResourceSupplier
Reference< resource::XStringResourceResolver >
    SAL_CALL SfxDialogLibrary::getStringResource(  ) throw (RuntimeException)
{
    if( !m_xStringResourcePersistence.is() )
        m_xStringResourcePersistence = m_pParent->implCreateStringResource( this );

    Reference< resource::XStringResourceResolver > xRet( m_xStringResourcePersistence, UNO_QUERY );
    return xRet;
}

bool SfxDialogLibrary::containsValidDialog( const ::com::sun::star::uno::Any& aElement )
{
    Reference< XInputStreamProvider > xISP;
    aElement >>= xISP;
    return xISP.is();
}

bool SAL_CALL SfxDialogLibrary::isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const
{
    return SfxDialogLibrary::containsValidDialog( aElement );
}

}
//============================================================================

