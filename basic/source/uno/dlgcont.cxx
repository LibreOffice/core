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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/resource/StringResourceWithStorage.hpp>
#include <com/sun/star/resource/StringResourceWithLocation.hpp>
#include <com/sun/star/document/GraphicStorageHandler.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <dlgcont.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <sot/storage.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <vcl/GraphicObject.hxx>
#include <i18nlangtag/languagetag.hxx>

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

using com::sun::star::uno::Reference;


// Implementation class SfxDialogLibraryContainer

const char* SfxDialogLibraryContainer::getInfoFileName() const { return "dialog"; }
const char* SfxDialogLibraryContainer::getOldInfoFileName() const { return "dialogs"; }
const char* SfxDialogLibraryContainer::getLibElementFileExtension() const { return "xdl"; }
const char* SfxDialogLibraryContainer::getLibrariesDir() const { return "Dialogs"; }

// Ctor for service
SfxDialogLibraryContainer::SfxDialogLibraryContainer()
{
    // all initialisation has to be done
    // by calling XInitialization::initialize
}

SfxDialogLibraryContainer::SfxDialogLibraryContainer( const uno::Reference< embed::XStorage >& xStorage )
{
    init( OUString(), xStorage );
}

// Methods to get library instances of the correct type
rtl::Reference<SfxLibrary> SfxDialogLibraryContainer::implCreateLibrary( const OUString& aName )
{
    return new SfxDialogLibrary( maModifiable, aName, mxSFI, this );
}

rtl::Reference<SfxLibrary> SfxDialogLibraryContainer::implCreateLibraryLink
    ( const OUString& aName, const OUString& aLibInfoFileURL,
      const OUString& StorageURL, bool ReadOnly )
{
    return new SfxDialogLibrary
            ( maModifiable, aName, mxSFI, aLibInfoFileURL, StorageURL, ReadOnly, this );
}

Any SfxDialogLibraryContainer::createEmptyLibraryElement()
{
    Reference< XInputStreamProvider > xISP;
    Any aRetAny;
    aRetAny <<= xISP;
    return aRetAny;
}

bool SfxDialogLibraryContainer::isLibraryElementValid(const Any& rElement) const
{
    return SfxDialogLibrary::containsValidDialog(rElement);
}

static bool writeOasis2OOoLibraryElement(
    const Reference< XInputStream >& xInput, const Reference< XOutputStream >& xOutput )
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
            "com.sun.star.comp.Oasis2OOoTransformer",
            aArgs, xContext ),
        UNO_QUERY );

    xParser->setDocumentHandler( xHandler );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = "virtual file";

    xParser->parseStream( source );

    return true;
}

void SfxDialogLibraryContainer::writeLibraryElement
(
    const Reference < XNameContainer >& xLib,
    const OUString& aElementName,
    const Reference< XOutputStream >& xOutput
)
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

void SfxDialogLibraryContainer::storeLibrariesToStorage( const uno::Reference< embed::XStorage >& xStorage )
{
    LibraryContainerMethodGuard aGuard( *this );
    mbOasis2OOoFormat = false;

    if ( mxStorage.is() && xStorage.is() )
    {
        try
        {
            tools::Long nSource = SotStorage::GetVersion( mxStorage );
            tools::Long nTarget = SotStorage::GetVersion( xStorage );

            if ( nSource == SOFFICE_FILEFORMAT_CURRENT &&
                nTarget != SOFFICE_FILEFORMAT_CURRENT )
            {
                mbOasis2OOoFormat = true;
            }
        }
        catch (const Exception& )
        {
            TOOLS_WARN_EXCEPTION("basic", "");
            // if we cannot get the version then the
            // Oasis2OOoTransformer will not be used
            assert(false);
        }
    }

    SfxLibraryContainer::storeLibrariesToStorage( xStorage );

    // we need to export out any embedded image object(s)
    // associated with any Dialogs. First, we need to actually gather any such urls
    // for each dialog in this container
    const Sequence< OUString > sLibraries = getElementNames();
    for ( const OUString& rName : sLibraries )
    {
        loadLibrary( rName );
        Reference< XNameContainer > xLib;
        getByName( rName ) >>= xLib;
        if ( xLib.is() )
        {
            Sequence< OUString > sDialogs = xLib->getElementNames();
            sal_Int32 nDialogs( sDialogs.getLength() );
            for ( sal_Int32 j=0; j < nDialogs; ++j )
            {
                // Each Dialog has an associated xISP
                Reference< io::XInputStreamProvider > xISP;
                xLib->getByName( sDialogs[ j ] ) >>= xISP;
                if ( xISP.is() )
                {
                    Reference< io::XInputStream > xInput( xISP->createInputStream() );
                    Reference< XNameContainer > xDialogModel(
                        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", mxContext),
                        UNO_QUERY );
                    ::xmlscript::importDialogModel( xInput, xDialogModel, mxContext, mxOwnerDocument );
                    std::vector<uno::Reference<graphic::XGraphic>> vxGraphicList;
                    vcl::graphic::SearchForGraphics(Reference<XInterface>(xDialogModel, UNO_QUERY), vxGraphicList);
                    if (!vxGraphicList.empty())
                    {
                        // Export the images to the storage
                        Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
                        xGraphicStorageHandler.set(document::GraphicStorageHandler::createWithStorage(mxContext, xStorage));
                        if (xGraphicStorageHandler.is())
                        {
                            for (uno::Reference<graphic::XGraphic> const & rxGraphic : vxGraphicList)
                            {
                                xGraphicStorageHandler->saveGraphic(rxGraphic);
                            }
                        }
                    }
                }
            }
        }
    }
    mbOasis2OOoFormat = false;
}


Any SfxDialogLibraryContainer::importLibraryElement
    ( const Reference < XNameContainer >& /*xLib*/,
      const OUString& /*aElementName */, const OUString& aFile,
      const uno::Reference< io::XInputStream >& xElementStream )
{
    Any aRetAny;

    // TODO: Member because later it will be a component
    //Reference< XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory() );
    //if( !xMSF.is() )
    //{
    //  OSL_FAIL( "### couldn't get ProcessServiceFactory" );
    //  return aRetAny;
    //}

    Reference< XParser > xParser = xml::sax::Parser::create( mxContext );

    Reference< XNameContainer > xDialogModel(
        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", mxContext),
        UNO_QUERY );
    if( !xDialogModel.is() )
    {
        OSL_FAIL( "### couldn't create com.sun.star.awt.UnoControlDialogModel component" );
        return aRetAny;
    }

    // Read from storage?
    bool bStorage = xElementStream.is();
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

    try {
        // start parsing
        xParser->setDocumentHandler( ::xmlscript::importDialogModel( xDialogModel, mxContext, mxOwnerDocument ) );
        xParser->parseStream( source );
    }
    catch(const Exception& )
    {
        OSL_FAIL( "Parsing error" );
        SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFile );
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        return aRetAny;
    }

    // Create InputStream, TODO: Implement own InputStreamProvider
    // to avoid creating the DialogModel here!
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, mxContext, mxOwnerDocument );
    aRetAny <<= xISP;
    return aRetAny;
}

void SfxDialogLibraryContainer::importFromOldStorage( const OUString& )
{
    // Nothing to do here, old dialogs cannot be imported
}

rtl::Reference<SfxLibraryContainer> SfxDialogLibraryContainer::createInstanceImpl()
{
    return new SfxDialogLibraryContainer();
}

constexpr OUStringLiteral aResourceFileNameBase = u"DialogStrings";
constexpr OUStringLiteral aResourceFileCommentBase = u"# Strings for Dialog Library ";

// Resource handling
Reference< css::resource::XStringResourcePersistence >
    SfxDialogLibraryContainer::implCreateStringResource( SfxDialogLibrary* pDialogLibrary )
{
    Reference< resource::XStringResourcePersistence > xRet;
    OUString aLibName = pDialogLibrary->getName();
    bool bReadOnly = pDialogLibrary->mbReadOnly;

    // get ui locale
    ::com::sun  ::star::lang::Locale aLocale = Application::GetSettings().GetUILanguageTag().getLocale();

    OUString aComment= aResourceFileCommentBase + aLibName;

    bool bStorage = mxStorage.is();
    if( bStorage )
    {
        uno::Reference< embed::XStorage > xLibrariesStor;
        uno::Reference< embed::XStorage > xLibraryStor;
        try {
            xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
                // TODO: Should be READWRITE with new storage concept using store() instead of storeTo()
            if ( !xLibrariesStor.is() )
                throw uno::RuntimeException("null returned from openStorageElement");

            xLibraryStor = xLibrariesStor->openStorageElement( aLibName, embed::ElementModes::READ );
                // TODO: Should be READWRITE with new storage concept using store() instead of storeTo()
            if ( !xLibraryStor.is() )
                throw uno::RuntimeException("null returned from openStorageElement");
        }
        catch(const uno::Exception& )
        {
            // Something went wrong while trying to get the storage library.
            // Return an object that supports StringResourceWithStorage, give it a storage location later.
            xRet = Reference< resource::XStringResourcePersistence >(
              mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.resource.StringResourceWithStorage", mxContext),
              UNO_QUERY );
            return xRet;
        }

        xRet = resource::StringResourceWithStorage::create(mxContext, xLibraryStor, bReadOnly, aLocale, aResourceFileNameBase, aComment);
    }
    else
    {
        OUString aLocation = createAppLibraryFolder( pDialogLibrary, aLibName );
        // TODO: Real handler?
        Reference< task::XInteractionHandler > xDummyHandler;

        xRet = resource::StringResourceWithLocation::create(mxContext, aLocation, bReadOnly, aLocale, aResourceFileNameBase, aComment, xDummyHandler);
    }

    return xRet;
}

void SfxDialogLibraryContainer::onNewRootStorage()
{
    // the library container is not modified, go through the libraries and check whether they are modified
    Sequence< OUString > aNames = maNameContainer->getElementNames();
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
                    throw uno::RuntimeException("null returned from openStorageElement");

                OUString aLibName = pDialogLibrary->getName();
                xLibraryStor = xLibrariesStor->openStorageElement( aLibName, embed::ElementModes::READWRITE );
                if ( !xLibraryStor.is() )
                    throw uno::RuntimeException("null returned from openStorageElement");

                Reference< resource::XStringResourceWithStorage >
                    xStringResourceWithStorage( xStringResourcePersistence, UNO_QUERY );
                if( xStringResourceWithStorage.is() )
                    xStringResourceWithStorage->setStorage( xLibraryStor );
            }
            catch(const uno::Exception& )
            {
                // TODO: Error handling?
            }
        }
    }
}

sal_Bool SAL_CALL
SfxDialogLibraryContainer:: HasExecutableCode( const OUString& /*Library*/ )
{
    return false; // dialog library has no executable code
}

// Service

OUString SAL_CALL SfxDialogLibraryContainer::getImplementationName( )
{
    return "com.sun.star.comp.sfx2.DialogLibraryContainer";
}

Sequence< OUString > SAL_CALL SfxDialogLibraryContainer::getSupportedServiceNames( )
{
    return {"com.sun.star.script.DocumentDialogLibraryContainer",
            "com.sun.star.script.DialogLibraryContainer"}; // for compatibility
}

// Implementation class SfxDialogLibrary

// Ctor
SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const OUString& aName,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, cppu::UnoType<XInputStreamProvider>::get(), xSFI )
    , m_pParent( pParent )
    , m_aName( aName )
{
}

SfxDialogLibrary::SfxDialogLibrary( ModifiableHelper& _rModifiable,
                                    const OUString& aName,
                                    const Reference< XSimpleFileAccess3 >& xSFI,
                                    const OUString& aLibInfoFileURL,
                                    const OUString& aStorageURL,
                                    bool ReadOnly,
                                    SfxDialogLibraryContainer* pParent )
    : SfxLibrary( _rModifiable, cppu::UnoType<XInputStreamProvider>::get(),
                       xSFI, aLibInfoFileURL, aStorageURL, ReadOnly)
    , m_pParent( pParent )
    , m_aName( aName )
{
}

IMPLEMENT_FORWARD_XINTERFACE2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SfxDialogLibrary, SfxLibrary, SfxDialogLibrary_BASE );

// Provide modify state including resources
bool SfxDialogLibrary::isModified()
{
    bool bRet = implIsModified();

    if( !bRet && m_xStringResourcePersistence.is() )
        bRet = m_xStringResourcePersistence->isModified();
    // else: Resources not accessed so far -> not modified

    return bRet;
}

void SfxDialogLibrary::storeResources()
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
    OUString aComment = aResourceFileCommentBase + m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToURL
            ( URL, aResourceFileNameBase, aComment, xHandler );
    }
}

void SfxDialogLibrary::storeResourcesToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage )
{
    OUString aComment = aResourceFileCommentBase + m_aName;

    if( m_xStringResourcePersistence.is() )
    {
        m_xStringResourcePersistence->storeToStorage
            ( xStorage, aResourceFileNameBase, aComment );
    }
}


// XStringResourceSupplier
Reference< resource::XStringResourceResolver >
    SAL_CALL SfxDialogLibrary::getStringResource(  )
{
    if( !m_xStringResourcePersistence.is() )
        m_xStringResourcePersistence = m_pParent->implCreateStringResource( this );

    return m_xStringResourcePersistence;
}

bool SfxDialogLibrary::containsValidDialog( const css::uno::Any& aElement )
{
    Reference< XInputStreamProvider > xISP;
    aElement >>= xISP;
    return xISP.is();
}

bool SfxDialogLibrary::isLibraryElementValid(const css::uno::Any& rElement) const
{
    return SfxDialogLibrary::containsValidDialog(rElement);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sfx2_DialogLibraryContainer_get_implementation(css::uno::XComponentContext*,
                                                           css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new basic::SfxDialogLibraryContainer());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
