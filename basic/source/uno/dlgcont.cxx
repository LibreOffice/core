/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgcont.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:41:05 $
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
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_RESOURCE_XSTRINGRESOURCEWITHSTORAGE_HPP_
#include "com/sun/star/resource/XStringResourceWithStorage.hpp"
#endif
#ifndef _COM_SUN_STAR_RESOURCE_XSTRINGRESOURCEWITHLOCATION_HPP_
#include "com/sun/star/resource/XStringResourceWithLocation.hpp"
#endif

#ifndef BASIC_DIALOGCONTAINER_HXX
#include "dlgcont.hxx"
#endif
#ifndef SBMODULE_HXX
#include "sbmodule.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/pathoptions.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <cppuhelper/factory.hxx>


namespace basic
{

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


//============================================================================
// Implementation class SfxDialogLibraryContainer

sal_Bool SfxDialogLibraryContainer::init( const OUString& aInitialisationParam,
    const uno::Reference< embed::XStorage >& xStorage )
{
    return SfxLibraryContainer::init(
          aInitialisationParam,
          OUString ( RTL_CONSTASCII_USTRINGPARAM("dialog") ),
          OUString ( RTL_CONSTASCII_USTRINGPARAM("dialogs") ),
          OUString ( RTL_CONSTASCII_USTRINGPARAM("xdl") ),
          OUString ( RTL_CONSTASCII_USTRINGPARAM("Dialogs") ),
          xStorage );
}

// Ctor for service
SfxDialogLibraryContainer::SfxDialogLibraryContainer( void )
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

SfxDialogLibraryContainer::SfxDialogLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
{
    OUString aInitialisationParam;
    init( aInitialisationParam, xStorage );
}

// Methods to get library instances of the correct type
SfxLibrary* SfxDialogLibraryContainer::implCreateLibrary( const ::rtl::OUString& aName )
{
    SfxLibrary* pRet = (SfxLibrary*) new SfxDialogLibrary( aName, mxMSF, mxSFI, this );
    return pRet;
}

SfxLibrary* SfxDialogLibraryContainer::implCreateLibraryLink
    ( const ::rtl::OUString& aName, const OUString& aLibInfoFileURL,
      const OUString& StorageURL, sal_Bool ReadOnly )
{
    SfxLibrary* pRet =
        (SfxLibrary*) new SfxDialogLibrary
            ( aName, mxMSF, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly, this );
    return pRet;
}

Any SAL_CALL SfxDialogLibraryContainer::createEmptyLibraryElement( void )
{
    Reference< XInputStreamProvider > xISP;
    Any aRetAny;
    aRetAny <<= xISP;
    return aRetAny;
}

sal_Bool SAL_CALL SfxDialogLibraryContainer::isLibraryElementValid( Any aElement )
{
    Reference< XInputStreamProvider > xISP;
    aElement >>= xISP;
    sal_Bool bRet = xISP.is();
    return bRet;
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
        return FALSE;
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
        return FALSE;
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

    return TRUE;
}

void SAL_CALL SfxDialogLibraryContainer::writeLibraryElement
(
    Any aElement,
    const OUString& /*aElementName*/,
    Reference< XOutputStream > xOutput
)
    throw(Exception)
{
    Reference< XInputStreamProvider > xISP;
    aElement >>= xISP;
    if( !xISP.is() )
        return;

    Reference< XInputStream > xInput( xISP->createInputStream() );

    bool bComplete = FALSE;
    if ( mbOasis2OOoFormat )
    {
        bComplete = writeOasis2OOoLibraryElement( xInput, xOutput );
    }

    if ( bComplete == FALSE )
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

void SfxDialogLibraryContainer::storeLibrariesToStorage( const uno::Reference< embed::XStorage >& xStorage )
{
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
            OSL_ASSERT(FALSE);
        }
    }

    SfxLibraryContainer::storeLibrariesToStorage( xStorage );

    mbOasis2OOoFormat = sal_False;
}


Any SAL_CALL SfxDialogLibraryContainer::importLibraryElement
    ( const OUString& aFile, const uno::Reference< io::XInputStream >& xElementStream )
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

    // start parsing
    xParser->setDocumentHandler( ::xmlscript::importDialogModel( xDialogModel, xContext ) );
    xParser->parseStream( source );

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


//============================================================================
// Methods XInitialization
void SAL_CALL SfxDialogLibraryContainer::initialize( const Sequence< Any >& aArguments )
    throw (::com::sun::star::uno::Exception,
           ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nArgCount = aArguments.getLength();
    OSL_ENSURE( nArgCount, "SfxDialogLibraryContainer::initialize() called with no arguments\n" );

    OUString aInitialisationParam;
    if( nArgCount )
    {
        const Any* pArgs = aArguments.getConstArray();
        pArgs[0] >>= aInitialisationParam;
        OSL_ENSURE( aInitialisationParam.getLength(),
            "SfxDialogLibraryContainer::initialize() called with empty url\n" );
    }

    init( aInitialisationParam );
}

static OUString aResourceFileNameBase = OUString::createFromAscii( "DialogStrings" );
static OUString aResourceFileCommentBase = OUString::createFromAscii( "# Strings for Dialog Library " );

// Resource handling
Reference< ::com::sun::star::resource::XStringResourcePersistance >
    SfxDialogLibraryContainer::implCreateStringResource( SfxDialogLibrary* pDialogLibrary )
{
    Reference< resource::XStringResourcePersistance > xRet;
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
        xRet = Reference< resource::XStringResourcePersistance >( mxMSF->createInstance
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
        xRet = Reference< resource::XStringResourcePersistance >( mxMSF->createInstance
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

void SfxDialogLibraryContainer::implSetStorage( const Reference< embed::XStorage >& xStorage )
{
    (void)xStorage;

    // the library container is not modified, go through the libraries and check whether they are modified
    Sequence< OUString > aNames = maNameContainer.getElementNames();
    const OUString* pNames = aNames.getConstArray();
    sal_Int32 nNameCount = aNames.getLength();

    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
    {
        OUString aName = pNames[ i ];
        SfxDialogLibrary* pDialogLibrary = static_cast<SfxDialogLibrary*>( getImplLib( aName ) );

        Reference< resource::XStringResourcePersistance > xStringResourcePersistance =
            pDialogLibrary->getStringResourcePersistance();

        if( xStringResourcePersistance.is() )
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
                    xStringResourceWithStorage( xStringResourcePersistance, UNO_QUERY );
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

Sequence< OUString > SfxDialogLibraryContainer::getSupportedServiceNames_static()
{
    static Sequence< OUString > seqServiceNames( 1 );
    static sal_Bool bNeedsInit = sal_True;

    MutexGuard aGuard( Mutex::getGlobalMutex() );
    if( bNeedsInit )
    {
        OUString* pSeq = seqServiceNames.getArray();
        pSeq[0] = OUString::createFromAscii( "com.sun.star.script.DialogLibraryContainer" );
        bNeedsInit = sal_False;
    }
    return seqServiceNames;
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
SfxDialogLibrary::SfxDialogLibrary( const ::rtl::OUString& aName,
                                    Reference< XMultiServiceFactory > xMSF,
                                    Reference< XSimpleFileAccess > xSFI,
                                    SfxDialogLibraryContainer* pParent )
    : SfxDialogLibrary_BASE( getCppuType( (const Reference< XInputStreamProvider > *)0 ), xMSF, xSFI )
    , m_pParent( pParent )
    , m_aName( aName )
{
}

SfxDialogLibrary::SfxDialogLibrary( const ::rtl::OUString& aName,
                                    Reference< XMultiServiceFactory > xMSF,
                                    Reference< XSimpleFileAccess > xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    sal_Bool ReadOnly,
                                    SfxDialogLibraryContainer* pParent )
    : SfxDialogLibrary_BASE( getCppuType( (const Reference< XInputStreamProvider > *)0 ),
                       xMSF, xSFI, aLibInfoFileURL, aStorageURL, ReadOnly)
    , m_pParent( pParent )
    , m_aName( aName )
{
}

// Provide modify state including resources
sal_Bool SfxDialogLibrary::isModified( void )
{
    sal_Bool bRet = implIsModified();

    if( !bRet && m_xStringResourcePersistance.is() )
        bRet = m_xStringResourcePersistance->isModified();
    // else: Resources not accessed so far -> not modified

    return bRet;
}

void SfxDialogLibrary::storeResources( void )
{
    if( m_xStringResourcePersistance.is() )
        m_xStringResourcePersistance->store();
}

void SfxDialogLibrary::storeResourcesAsURL
    ( const ::rtl::OUString& URL, const ::rtl::OUString& NewName )
{
    OUString aComment = aResourceFileCommentBase;
    m_aName = NewName;
    aComment += m_aName;

    if( m_xStringResourcePersistance.is() )
    {
        m_xStringResourcePersistance->setComment( aComment );

        Reference< resource::XStringResourceWithLocation >
            xStringResourceWithLocation( m_xStringResourcePersistance, UNO_QUERY );
        if( xStringResourceWithLocation.is() )
            xStringResourceWithLocation->storeAsURL( URL );
    }
}

void SfxDialogLibrary::storeResourcesToURL( const OUString& URL,
    const Reference< task::XInteractionHandler >& xHandler )
{
    OUString aComment = aResourceFileCommentBase;
    aComment += m_aName;

    if( m_xStringResourcePersistance.is() )
    {
        m_xStringResourcePersistance->storeToURL
            ( URL, aResourceFileNameBase, aComment, xHandler );
    }
}

void SfxDialogLibrary::storeResourcesToStorage( const ::com::sun::star::uno::Reference
    < ::com::sun::star::embed::XStorage >& xStorage )
{
    OUString aComment = aResourceFileCommentBase;
    aComment += m_aName;

    if( m_xStringResourcePersistance.is() )
    {
        m_xStringResourcePersistance->storeToStorage
            ( xStorage, aResourceFileNameBase, aComment );
    }
}


// XStringResourceSupplier
Reference< resource::XStringResourceResolver >
    SAL_CALL SfxDialogLibrary::getStringResource(  ) throw (RuntimeException)
{
    if( !m_xStringResourcePersistance.is() )
        m_xStringResourcePersistance = m_pParent->implCreateStringResource( this );

    Reference< resource::XStringResourceResolver > xRet( m_xStringResourcePersistance, UNO_QUERY );
    return xRet;
}

}
//============================================================================

