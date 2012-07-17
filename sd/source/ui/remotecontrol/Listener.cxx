/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include "Listener.hxx"

using namespace sd;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::frame;
using rtl::OString;


Listener::Listener( css::uno::Reference< css::presentation::XSlideShowController > aController, sd::Transmitter& rTransmitter  )
    : ::cppu::WeakComponentImplHelper2< XSlideShowListener,
            XFrameActionListener >( m_aMutex )
{
    fprintf( stderr, "Hello from a listener...\n" );
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::frame::XFrame > xDesktop( xServiceManager->createInstance(
        "com.sun.star.frame.Desktop" ) , css::uno::UNO_QUERY_THROW );
        xDesktop->addFrameActionListener( static_cast<XFrameActionListener*>(this));
    }
    catch ( css::uno::RuntimeException &e )
    {
        //return;
    }
    if( aController.is() )
    {
//         css::uno::Reference< css::lang::XMultiServiceFactory > xServiceManager(
//             ::comphelper::getProcessServiceFactory(), css::uno::UNO_QUERY_THROW );
//         css::uno::Reference< css::frame::XFramesSupplier > xFramesSupplier(
//             xServiceManager->createInstance( "com.sun.star.frame.Desktop" ) ,
//             css::uno::UNO_QUERY_THROW );
//         css::uno::Reference< css::frame::XFrame > xFrame (
//             xFramesSupplier->getActiveFrame(), css::uno::UNO_QUERY_THROW );
//         css::uno::Reference<css::presentation::XPresentationSupplier> xPS (
//             xFrame->getController()->getModel(), css::uno::UNO_QUERY_THROW);
//         css::uno::Reference<css::presentation::XPresentation2> xPresentation(
//             xPS->getPresentation(), css::uno::UNO_QUERY_THROW);
//         // Throws an exception if now slideshow running
//         css::uno::Reference<css::presentation::XSlideShowController> xSlideShowController(
//             xPresentation->getController(), css::uno::UNO_QUERY_THROW );
//         xSlideShowController->addSlideShowListener(static_cast<XSlideShowListener*>(this));
//         fprintf(stderr, "Registered the slideshowlistener\n" );
        fprintf(stderr, "Trying to add the slideshowlistener\n" );
        aController->addSlideShowListener(static_cast<XSlideShowListener*>(this));
        fprintf(stderr, "Registered the slideshowlistener\n" );
    }
     else
     {
        fprintf(stderr, "rController isn't\n" );
     }

}




//----- XAnimationListener ----------------------------------------------------
void SAL_CALL Listener::beginEvent(const css::uno::Reference<
    css::animations::XAnimationNode >&  rNode ) throw (css::uno::RuntimeException)
{
    (void) rNode;
}


void SAL_CALL Listener::endEvent( const css::uno::Reference<
    css::animations::XAnimationNode >& rNode ) throw (css::uno::RuntimeException)
{
    (void) rNode;
}

void SAL_CALL Listener::repeat( const css::uno::Reference<
    css::animations::XAnimationNode >& rNode, ::sal_Int32 Repeat )
     throw (css::uno::RuntimeException)
{
    (void) rNode;
}


//----- XSlideShowListener ----------------------------------------------------

void SAL_CALL Listener::paused (void)
    throw (com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL Listener::resumed (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL Listener::slideEnded (sal_Bool bReverse)
    throw (css::uno::RuntimeException)
{
    (void) bReverse;
    fprintf( stderr, "slidenede\n" );
//     (void) bReverse;
//     JsonBuilder *aBuilder = json_builder_new();
//
//
//     json_builder_begin_object( aBuilder );
//     json_builder_set_member_name( aBuilder, "slide_number");
//     json_builder_add_int_value( aBuilder, 2 );
//     // FIXME: get the slide number
//     json_builder_end_object( aBuilder );
//
//     JsonGenerator *aGen = json_generator_new();
//     JsonNode *aRoot = json_builder_get_root( aBuilder );
//     json_generator_set_root( aGen, aRoot );
//     char *aCommand = json_generator_to_data( aGen, NULL);
//
//     json_node_free( aRoot );
//     g_object_unref ( aGen );
//     g_object_unref ( aBuilder );
//
//     sal_Int32 aLen = strlen( aCommand );
//
//     OString aLengthString = OString::valueOf( aLen );
//     const char *aLengthChar = aLengthString.getStr();
//
//     sal_Int32 aLengthLength = aLengthString.getLength();
//
//     mStreamSocket.write( aLengthChar, aLengthLength );
//     mStreamSocket.write( "\n", 1 );
//     mStreamSocket.write( aCommand, aLen );
//     // Transmit here.
//
//     g_free( aCommand );
}

void SAL_CALL Listener::hyperLinkClicked (const rtl::OUString &)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL Listener::slideTransitionStarted (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL Listener::slideTransitionEnded (void)
    throw (css::uno::RuntimeException)
{
        fprintf( stderr, "slidetreatasdfanede\n" );
}

void SAL_CALL Listener::slideAnimationsEnded (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL Listener::frameAction (const css::frame::FrameActionEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException)
{
    fprintf( stderr, "FrameAction\n" );
}

void SAL_CALL Listener::disposing (void)
{
// FIXME: disconnect as appropriate
}

void SAL_CALL Listener::disposing (
    const css::lang::EventObject& rEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
// FIXME: disconnect as appropriate
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */