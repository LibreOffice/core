/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include "Communicator.hxx"
#include "Listener.hxx"
#include "ImagePreparer.hxx"
#include "Transmitter.hxx"

#include <com/sun/star/presentation/XSlideShowController.hpp>

using namespace sd;
using namespace ::com::sun::star::presentation;

Listener::Listener( const ::rtl::Reference<Communicator>& rCommunicator,
                    sd::Transmitter *aTransmitter  ):
      ::cppu::WeakComponentImplHelper< XSlideShowListener >( m_aMutex ),
      mCommunicator( rCommunicator ),
      pTransmitter( nullptr )
{
    pTransmitter = aTransmitter;
}

Listener::~Listener()
{
}

void Listener::init( const css::uno::Reference< css::presentation::XSlideShowController >& aController)
{
    if ( aController.is() )
    {
        mController.set( aController );
        aController->addSlideShowListener( this );

        sal_Int32 aSlides = aController->getSlideCount();
        sal_Int32 aCurrentSlide = aController->getCurrentSlideIndex();
        OString aBuffer = "slideshow_started\n" +
            OString::number( aSlides ) + "\n" +
            OString::number( aCurrentSlide ) + "\n\n";

        pTransmitter->addMessage( aBuffer,
                                  Transmitter::PRIORITY_HIGH );

        {
            SolarMutexGuard aGuard;
            /* ImagePreparer* pPreparer = */ new ImagePreparer( aController, pTransmitter );
        }
    }
    else
    {
        SAL_INFO( "sdremote", "Listener::init but no controller - so no preview push queued" );
    }
}

//----- XAnimationListener ----------------------------------------------------

void SAL_CALL Listener::beginEvent(const css::uno::Reference<
    css::animations::XAnimationNode >& )
{}

void SAL_CALL Listener::endEvent( const css::uno::Reference<
    css::animations::XAnimationNode >& )
{}

void SAL_CALL Listener::repeat( const css::uno::Reference<
    css::animations::XAnimationNode >&, ::sal_Int32 )
{}

//----- XSlideShowListener ----------------------------------------------------

void SAL_CALL Listener::paused()
{
}

void SAL_CALL Listener::resumed()
{
}

void SAL_CALL Listener::slideEnded (sal_Bool)
{
}

void SAL_CALL Listener::hyperLinkClicked (const OUString &)
{
}

void SAL_CALL Listener::slideTransitionStarted()
{
    sal_Int32 aSlide = mController->getCurrentSlideIndex();

    OString aBuilder = "slide_updated\n" +
        OString::number( aSlide ) +
        "\n\n";

    if ( pTransmitter )
    {
        pTransmitter->addMessage( aBuilder,
                               Transmitter::PRIORITY_HIGH );
    }
}

void SAL_CALL Listener::slideTransitionEnded()
{
}

void SAL_CALL Listener::slideAnimationsEnded()
{
}

void SAL_CALL Listener::disposing()
{
    pTransmitter = nullptr;
    if ( mController.is() )
    {
        mController->removeSlideShowListener( this );
        mController = nullptr;
    }
    mCommunicator->informListenerDestroyed();
}

void SAL_CALL Listener::disposing (
    const css::lang::EventObject&)
{
    dispose();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
