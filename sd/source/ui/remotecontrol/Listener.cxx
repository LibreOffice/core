/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "Listener.hxx"

using namespace sd;
using namespace ::com::sun::star::presentation;
using rtl::OString;


Listener::Listener( const css::uno::Reference<XSlideShowController>& rxSlideShowController,
    osl::StreamSocket aSocket )
    : ::cppu::WeakComponentImplHelper1< XSlideShowListener >( m_aMutex ),
      mxSlideShowController(rxSlideShowController),
      mStreamSocket( aSocket )
{
    if( mxSlideShowController.is() )
    {
        // Listen for events from the slide show controller.
        mxSlideShowController->addSlideShowListener(static_cast<XSlideShowListener*>(this));
    }

}

Listener::~Listener()
{


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
}

void SAL_CALL Listener::slideAnimationsEnded (void)
    throw (css::uno::RuntimeException)
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */