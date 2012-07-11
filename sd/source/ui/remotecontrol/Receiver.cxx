/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Receiver.hxx"
#include <string.h>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <xmlsec/base64.h>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
using namespace sd;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using rtl::OUString;
using rtl::OString;
using namespace ::osl;

Receiver::Receiver()
{
    g_type_init ();
}

Receiver::~Receiver()
{
}

void Receiver::executeCommand( JsonObject *aObject, Reference<XSlideShowController> xSlideShowController )
{
    const char* aInstruction = json_node_get_string( json_object_get_member( aObject, "command" ) );

    fprintf( stderr, "instruction:%s\n", aInstruction );

    if ( strcmp( aInstruction, "transition_next" ) == 0 )
    {

        xSlideShowController->gotoNextEffect();
      // Next slide;
    }
    else if ( strcmp( aInstruction, "transition_previous" ) == 0 )
    {
        xSlideShowController->gotoPreviousEffect();
    }
    else if ( strcmp( aInstruction, "goto_slide" ) == 0 )
    {
        //
    }

}

void Receiver::parseCommand( const char* aCommand, sal_Int32 size, osl::StreamSocket &aStreamSocket )
{
    Reference<XSlideShowController> xSlideShowController;
    try {
        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< XFramesSupplier > xFramesSupplier( xServiceManager->createInstance(
        "com.sun.star.frame.Desktop" ) , UNO_QUERY_THROW );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), UNO_QUERY_THROW );
        Reference<XPresentationSupplier> xPS ( xFrame->getController()->getModel(), UNO_QUERY_THROW);
        Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);
        // Throws an exception if now slideshow running
       xSlideShowController =  Reference<XSlideShowController>( xPresentation->getController(), UNO_QUERY_THROW );
       sendPreview( 1, xSlideShowController, aStreamSocket );
    }
    catch ( com::sun::star::uno::RuntimeException &e )
    {
        return;
    }

    // Parsing
    JsonParser *parser;
    JsonNode *root;
    GError *error;

    parser = json_parser_new();
    error = NULL;
    json_parser_load_from_data( parser, aCommand, size, &error );

    if (error) {
        g_error_free( error );
        g_object_unref( parser );
    }

    root = json_parser_get_root( parser );
    JsonObject *aObject = json_node_get_object( root );

    executeCommand( aObject, xSlideShowController );

    g_object_unref( parser );

}


void sendPreview(sal_uInt32 aSlideNumber, Reference<XSlideShowController> xSlideShowController, osl::StreamSocket &mStreamSocket )
{

    sal_uInt64 aSize;

    uno::Sequence<sal_Int8> aData = preparePreview( aSlideNumber, xSlideShowController, 320, 240, aSize );

    rtl::OUStringBuffer aStrBuffer;
//     char* aDataEncoded = (char*) xmlSecBase64Encode( (xmlSecByte *) aData, aSize, 0 );
    ::sax::Converter::encodeBase64( aStrBuffer, aData );

    OUString aEncodedString = aStrBuffer.makeStringAndClear();
    OString aEncodedShortString = rtl::OUStringToOString( aEncodedString, RTL_TEXTENCODING_UTF8 );
//     aEncodedString.convertToString( &aEncodedShortString, RTL_TEXTENCODING_UTF8 , 0);

    JsonBuilder *aBuilder = json_builder_new();


    json_builder_begin_object( aBuilder );

    json_builder_set_member_name( aBuilder, "command" );
    json_builder_add_string_value( aBuilder, "slide_preview" );

    json_builder_set_member_name( aBuilder, "slide_number" );
    json_builder_add_int_value( aBuilder, 2 );

    json_builder_set_member_name( aBuilder, "image_preview" );
    json_builder_add_string_value( aBuilder, aEncodedShortString.getStr() );

    // FIXME: get the slide number
    json_builder_end_object( aBuilder );

    JsonGenerator *aGen = json_generator_new();
    JsonNode *aRoot = json_builder_get_root( aBuilder );
    json_generator_set_root( aGen, aRoot );
    char *aCommand = json_generator_to_data( aGen, NULL);

    json_node_free( aRoot );
    g_object_unref ( aGen );
    g_object_unref ( aBuilder );

    sal_Int32 aLen = strlen( aCommand );

    OString aLengthString = OString::valueOf( aLen );
    const char *aLengthChar = aLengthString.getStr();

    sal_Int32 aLengthLength = aLengthString.getLength();

    fprintf( stderr, "%s\n", aCommand );

    mStreamSocket.write( aLengthChar, aLengthLength );
    mStreamSocket.write( "\n", 1 );
    mStreamSocket.write( aCommand, aLen );
    // Transmit here.

    g_free( aCommand );

}

Sequence<sal_Int8> preparePreview(sal_uInt32 aSlideNumber, Reference<XSlideShowController> xSlideShowController, sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &rSize )
{

    // Create temp file
    OUString aFileURL;
    FileBase::createTempFile( 0, 0, &aFileURL );


    uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );

    uno::Reference< XFilter > xFilter( xServiceManager->createInstance(
        "com.sun.star.drawing.GraphicExportFilter"  ) , UNO_QUERY_THROW );

    uno::Reference< XExporter > xExporter( xFilter, uno::UNO_QUERY_THROW );


    uno::Reference< lang::XComponent > xSourceDoc(
        xSlideShowController->getSlideByIndex( aSlideNumber ) , uno::UNO_QUERY_THROW );

    xExporter->setSourceDocument( xSourceDoc );

    Sequence< beans::PropertyValue > aFilterData(3);
    aFilterData[0].Name = "PixelWidth";
    aFilterData[0].Value <<= 2000;
    aFilterData[1].Name = "PixelHeight";
    aFilterData[1].Value <<= 2000;

    // Add quality if jpg "Quality" [1-100]
    // FIXME: is setting color mode needed.
    aFilterData[2].Name = "ColorMode";
    aFilterData[2].Value <<= 0; // Color

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = "MediaType";
    aProps[0].Value <<= OUString( "image/png" );

    aProps[1].Name = "URL";
    aProps[1].Value <<= aFileURL;

    aProps[2].Name = "FilterData";
    aProps[2].Value <<= aFilterData;

    xFilter->filter( aProps );

    fprintf( stderr, "%s\n", rtl::OUStringToOString( aFileURL , RTL_TEXTENCODING_UTF8 ).getStr() );

    // FIXME: error handling.

    File aFile( aFileURL );
    aFile.open(0);
    sal_uInt64 aRead;
    rSize = 0;
    aFile.getSize( rSize );
    uno::Sequence<sal_Int8> aContents( rSize );

    aFile.read( aContents.getArray(), rSize, aRead );
    aFile.close();
    File::remove( aFileURL );
    return aContents;

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */