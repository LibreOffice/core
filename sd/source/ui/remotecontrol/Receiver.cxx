/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2012 Andrzej J.R. Hunt
 *
 * LibreOffice - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * LibreOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * Libreoffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with LibreOffice.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "Receiver.hxx"
#include <cstring>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <comphelper/processfactory.hxx>
using namespace sd;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
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

void Receiver::parseCommand( const char* aCommand, sal_Int32 size, XSlideShowController *aController )
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

// void preparePreview(sal_Int32 aSlideNumber, Reference<SlideShowController> aController)
// {
//     uno::Reference< XDrawPage > aSlide( aController->getSlideByIndex( aSlideNumber ) );
//
//     uno::Reference< lang::XMultiServiceFactory > xServiceManager(
//             ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
//
//     uno::Reference< XGraphicExportFilter > xGraphicExportFilter( xServiceManager->createInstance(
//         "com.sun.star.drawing.GraphicExportFilter" ) , UNO_QUERY_THROW );
//
//     xGraphicExportFilter->setSource( aSlide );



// }
