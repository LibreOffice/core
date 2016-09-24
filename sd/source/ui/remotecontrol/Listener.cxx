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
#include <rtl/strbuf.hxx>
#include <vcl/svapp.hxx>

#include "Listener.hxx"
#include "ImagePreparer.hxx"

using namespace sd;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::frame;

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
        OStringBuffer aBuffer;
        aBuffer.append( "slideshow_started\n" )
               .append( OString::number( aSlides ) ).append("\n")
        .append( OString::number( aCurrentSlide ) ).append( "\n\n" );

        pTransmitter->addMessage( aBuffer.makeStringAndClear(),
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
    css::animations::XAnimationNode >&  rNode ) throw (css::uno::RuntimeException, std::exception)
{
    (void) rNode;
}

void SAL_CALL Listener::endEvent( const css::uno::Reference<
    css::animations::XAnimationNode >& rNode ) throw (css::uno::RuntimeException, std::exception)
{
    (void) rNode;
}

void SAL_CALL Listener::repeat( const css::uno::Reference<
    css::animations::XAnimationNode >& rNode, ::sal_Int32 aRepeat )
     throw (css::uno::RuntimeException, std::exception)
{
    (void) rNode;
    (void) aRepeat;
}

//----- XSlideShowListener ----------------------------------------------------

void SAL_CALL Listener::paused()
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Listener::resumed()
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Listener::slideEnded (sal_Bool bReverse)
    throw (css::uno::RuntimeException, std::exception)
{
    (void) bReverse;
}

void SAL_CALL Listener::hyperLinkClicked (const OUString &)
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Listener::slideTransitionStarted()
    throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 aSlide = mController->getCurrentSlideIndex();

    OStringBuffer aBuilder( "slide_updated\n" );
    aBuilder.append( OString::number( aSlide ) );
    aBuilder.append( "\n\n" );

    if ( pTransmitter )
    {
        pTransmitter->addMessage( aBuilder.makeStringAndClear(),
                               Transmitter::PRIORITY_HIGH );
    }
}

void SAL_CALL Listener::slideTransitionEnded()
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Listener::slideAnimationsEnded()
    throw (css::uno::RuntimeException, std::exception)
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
    const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void) rEvent;
    dispose();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
