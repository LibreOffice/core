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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/resource/StringResourceWithStorage.hpp>
#include <com/sun/star/resource/StringResourceWithLocation.hpp>
#include <com/sun/star/document/GraphicObjectResolver.hpp>
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
#include <svtools/grfmgr.hxx>

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

using com::sun::star::uno::Reference;




const sal_Char* SAL_CALL SfxDialogLibraryContainer::getInfoFileName() const { return "dialog"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getOldInfoFileName() const { return "dialogs"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getLibElementFileExtension() const { return "xdl"; }
const sal_Char* SAL_CALL SfxDialogLibraryContainer::getLibrariesDir() const { return "Dialogs"; }


SfxDialogLibraryContainer::SfxDialogLibraryContainer( void )
{
    
    
}

SfxDialogLibraryContainer::SfxDialogLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
{
    init( OUString(), xStorage );
}


SfxLibrary* SfxDialogLibraryContainer::implCreateLibrary( const OUString& aName )
{
    SfxLibrary* pRet = new SfxDialogLibrary( maModifiable, aName, mxContext, mxSFI, this );
    return pRet;
}

SfxLibrary* SfxDialogLibraryContainer::implCreateLibraryLink
    ( const OUString& aName, const OUString& aLibInfoFileURL,
      const OUString& StorageURL, sal_Bool ReadOnly )
{
    SfxLibrary* pRet = new SfxDialogLibrary
            ( maModifiable, aName, mxContext, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly, this );
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
    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );

    Reference< lang::XMultiComponentFactory > xSMgr(
        xContext->getServiceManager() );

    Reference< xml::sax::XParser > xParser =  xml::sax::Parser::create(xContext);

    Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);

    xWriter->setOutputStream( xOutput );

    Sequence<Any> aArgs( 1 );
    aArgs[0] <<= xWriter;

    Reference< xml::sax::XDocumentHandler > xHandler(
        xSMgr->createInstanceWithArgumentsAndContext(
            OUString("com.sun.star.comp.Oasis2OOoTransformer" ) ,
            aArgs, xContext ),
        UNO_QUERY );

    xParser->setDocumentHandler( xHandler );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = "virtual file";

    xParser->parseStream( source );

    return true;
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

    bool bComplete = false;
    if ( mbOasis2OOoFormat )
    {
        bComplete = writeOasis2OOoLibraryElement( xInput, xOutput );
    }

    if ( !bComplete )
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
    mbOasis2OOoFormat = false;

    if ( mxStorage.is() && xStorage.is() )
    {
        try
        {
            long nSource = SotStorage::GetVersion( mxStorage );
            long nTarget = SotStorage::GetVersion( xStorage );

            if ( nSource == SOFFICE_FILEFORMAT_CURRENT &&
                nTarget != SOFFICE_FILEFORMAT_CURRENT )
            {
                mbOasis2OOoFormat = true;
            }
        }
        catch (const Exception& )
        {
            
            
            OSL_ASSERT(false);
        }
    }

    SfxLibraryContainer::storeLibrariesToStorage( xStorage );

    
    
    
    Sequence< OUString > sLibraries = getElementNames();
    for ( sal_Int32 i=0; i < sLibraries.getLength(); ++i )
    {
        loadLibrary( sLibraries[ i ] );
        Reference< XNameContainer > xLib;
        getByName( sLibraries[ i ] ) >>= xLib;
        if ( xLib.is() )
        {
            Sequence< OUString > sDialogs = xLib->getElementNames();
            sal_Int32 nDialogs( sDialogs.getLength() );
            for ( sal_Int32 j=0; j < nDialogs; ++j )
            {
                
                Reference< io::XInputStreamProvider > xISP;
                xLib->getByName( sDialogs[ j ] ) >>= xISP;
                if ( xISP.is() )
                {
                    Reference< io::XInputStream > xInput( xISP->createInputStream() );
                    Reference< XNameContainer > xDialogModel(
                        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", mxContext),
                        UNO_QUERY );
                    ::xmlscript::importDialogModel( xInput, xDialogModel, mxContext, mxOwnerDocument );
                    std::vector< OUString > vEmbeddedImageURLs;
                    GraphicObject::InspectForGraphicObjectImageURL( xDialogModel,  vEmbeddedImageURLs );
                    if ( !vEmbeddedImageURLs.empty() )
                    {
                        
                        Reference< document::XGraphicObjectResolver > xGraphicResolver =
                            document::GraphicObjectResolver::createWithStorage( mxContext, xStorage );
                        std::vector< OUString >::iterator it = vEmbeddedImageURLs.begin();
                        std::vector< OUString >::iterator it_end = vEmbeddedImageURLs.end();
                        if ( xGraphicResolver.is() )
                        {
                            for ( sal_Int32 count = 0; it != it_end; ++it, ++count )
                                xGraphicResolver->resolveGraphicObjectURL( *it );
                        }
                    }
                }
            }
        }
    }
    mbOasis2OOoFormat = false;
}


Any SAL_CALL SfxDialogLibraryContainer::importLibraryElement
    ( const Reference < XNameContainer >& /*xLib*/,
      const OUString& /*aElementName */, const OUString& aFile,
      const uno::Reference< io::XInputStream >& xElementStream )
{
    Any aRetAny;

    
    
    
    
    
    
    

    Reference< XParser > xParser = xml::sax::Parser::create( mxContext );

    Reference< XNameContainer > xDialogModel(
        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", mxContext),
        UNO_QUERY );
    if( !xDialogModel.is() )
    {
        OSL_FAIL( "### couldn't create com.sun.star.awt.UnoControlDialogModel component\n" );
        return aRetAny;
    }

    
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
        catch(const Exception& )
        
        {
            
            
        }
    }
    if( !xInput.is() )
        return aRetAny;

    InputSource source;
    source.aInputStream = xInput;
    source.sSystemId    = aFile;

    try {
        
        xParser->setDocumentHandler( ::xmlscript::importDialogModel( xDialogModel, mxContext, mxOwnerDocument ) );
        xParser->parseStream( source );
    }
    catch(const Exception& )
    {
        OSL_FAIL( "Parsing error\n" );
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        sal_uIntPtr nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
        return aRetAny;
    }

    
    
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, mxContext, mxOwnerDocument );
    aRetAny <<= xISP;
    return aRetAny;
}

void SAL_CALL SfxDialogLibraryContainer::importFromOldStorage( const OUString& )
{
    
}

SfxLibraryContainer* SfxDialogLibraryContainer::createInstanceImpl( void )
{
    return new SfxDialogLibraryContainer();
}

const char aResourceFileNameBase[] = "DialogStrings";
const char aResourceFileCommentBase[] = "# Strings for Dialog Library ";


Reference< ::com::sun::star::resource::XStringResourcePersistence >
    SfxDialogLibraryContainer::implCreateStringResource( SfxDialogLibrary* pDialogLibrary )
{
    Reference< resource::XStringResourcePersistence > xRet;
    OUString aLibName = pDialogLibrary->getName();
    bool bReadOnly = pDialogLibrary->mbReadOnly;

    
    ::com::sun  ::star::lang::Locale aLocale = Application::GetSettings().GetUILanguageTag().getLocale();

    OUString aComment(aResourceFileCommentBase);
    aComment += aLibName;

    sal_Bool bStorage = mxStorage.is();
    if( bStorage )
    {
        uno::Reference< embed::XStorage > xLibrariesStor;
        uno::Reference< embed::XStorage > xLibraryStor;
        try {
            xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
                
            if ( !xLibrariesStor.is() )
                throw uno::RuntimeException();

            xLibraryStor = xLibrariesStor->openStorageElement( aLibName, embed::ElementModes::READ );
                
            if ( !xLibraryStor.is() )
                throw uno::RuntimeException();
        }
        catch(const uno::Exception& )
        {
            
            return xRet;
        }

        xRet = resource::StringResourceWithStorage::create(mxContext, xLibraryStor, bReadOnly, aLocale, OUString(aResourceFileNameBase), aComment);
    }
    else
    {
        OUString aLocation = createAppLibraryFolder( pDialogLibrary, aLibName );
        
        Reference< task::XInteractionHandler > xDummyHandler;

        xRet = resource::StringResourceWithLocation::create(mxContext, aLocation, bReadOnly, aLocale, OUString(aResourceFileNameBase), aComment, xDummyHandler);
    }

    return xRet;
}

void SfxDialogLibraryContainer::onNewRootStorage()
{
    
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
            catch(const uno::Exception& )
            {
                
            }
        }
    }
}

sal_Bool SAL_CALL
SfxDialogLibraryContainer:: HasExecutableCode( const OUString& /*Library*/ ) throw (uno::RuntimeException)
{
    return sal_False; 
}



void createRegistryInfo_SfxDialogLibraryContainer()
{
    static OAutoRegistration< SfxDialogLibraryContainer > aAutoRegistration;
}

OUString SAL_CALL SfxDialogLibraryContainer::getImplementationName( ) throw (RuntimeException)
{
    return getImplementationName_static();
}

Sequence< OUString > SAL_CALL SfxDialogLibraryContainer::getSupportedServiceNames( ) throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SfxDialogLibraryContainer::getSupportedServiceNames_static()
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[0] = "com.sun.star.script.DocumentDialogLibraryContainer";
    
    aServiceNames[1] = "com.sun.star.script.DialogLibraryContainer";
    return aServiceNames;
}

OUString SfxDialogLibraryContainer::getImplementationName_static()
{
    return OUString("com.sun.star.comp.sfx2.DialogLibraryContainer");
}

Reference< XInterface > SAL_CALL SfxDialogLibraryContainer::Create( const Reference< XComponentContext >& ) throw( Exception )
{
    Reference< XInterface > xRet =
        static_cast< XInterface* >( static_cast< OWeakObject* >(new SfxDialogLibraryContainer()) );
    return xRet;
}






SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const OUString& aName,
                                    const Reference< XComponentContext >& xContext,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, getCppuType( (const Reference< XInputStreamProvider > *)0 ), xContext, xSFI )
    , m_pParent( pParent )
    , m_aName( aName )
{
}

SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const OUString& aName,
                                    const Reference< XComponentContext >& xContext,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    sal_Bool ReadOnly,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, getCppuType( (const Reference< XInputStreamProvider > *)0 ),
                       xContext, xSFI, aLibInfoFileURL, aStorageURL, ReadOnly)
    , m_pParent( pParent )
    , m_aName( aName )
{
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );


sal_Bool SfxDialogLibrary::isModified( void )
{
    sal_Bool bRet = implIsModified();

    if( !bRet && m_xStringResourcePersistence.is() )
        bRet = m_xStringResourcePersistence->isModified();
    

    return bRet;
}

void SfxDialogLibrary::storeResources( void )
{
    if( m_xStringResourcePersistence.is() )
        m_xStringResourcePersistence->store();
}

void SfxDialogLibrary::storeResourcesAsURL
    ( const OUString& URL, const OUString& NewName )
{
    OUString aComment(aResourceFileCommentBase);
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
    OUString aComment(aResourceFileCommentBase);
    aComment += m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToURL
            ( URL, OUString(aResourceFileNameBase), aComment, xHandler );
    }
}

void SfxDialogLibrary::storeResourcesToStorage( const ::com::sun::star::uno::Reference
    < ::com::sun::star::embed::XStorage >& xStorage )
{
    OUString aComment(aResourceFileCommentBase);
    aComment += m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToStorage
            ( xStorage, OUString(aResourceFileNameBase), aComment );
    }
}



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
