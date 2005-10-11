    /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:17:41 $
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
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif

#ifndef _SD_SLIDESHOWIMPL_HXX_
#include "slideshowimpl.hxx"
#endif

using namespace ::sd;
using namespace ::rtl;

Slideshow::Slideshow( ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc )
: mpImpl( new SlideshowImpl(pViewSh, pView, pDoc ) )
{
    mpImpl->acquire();
}

Slideshow::~Slideshow()
{
    dispose();
    mpImpl->release();
}

bool Slideshow::startShow( PresentationSettings* pPresSettings )
{
    return mpImpl->startShow( pPresSettings );
}

bool Slideshow::startPreview(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent )
{
    if( mpImpl->startPreview( xDrawPage, xAnimationNode, pParent ) )
    {
        ShowWindow* pShowWindow = mpImpl->getShowWindow();
        if( pShowWindow )
            pShowWindow->setSlideshow( this );
        return true;
    }
    else
    {
        return false;
    }
}

void Slideshow::paint( const Rectangle& rRect )
{
    mpImpl->paint( rRect );
}

void Slideshow::stopShow()
{
    mpImpl->stopShow();
}

void Slideshow::dispose()
{
    mpImpl->stopShow();
}

ShowWindow* Slideshow::getShowWindow()
{
    return mpImpl->mpShowWindow;
}

int Slideshow::getAnimationMode()
{
    return mpImpl->meAnimationMode;
}

void Slideshow::jumpToPageIndex( sal_Int32 nPageIndex )
{
    mpImpl->displaySlideIndex( nPageIndex );
}

void Slideshow::jumpToPageNumber( sal_Int32 nPageNumber )
{
    mpImpl->displaySlideNumber( nPageNumber );
}

sal_Int32 Slideshow::getCurrentPageNumber()
{
    return mpImpl->getCurrentSlideNumber();
}

sal_Int32 Slideshow::getCurrentPageIndex()
{
    return mpImpl->getCurrentSlideIndex();
}

void Slideshow::jumpToBookmark( const String& sBookmark )
{
    mpImpl->jumpToBookmark( sBookmark );
}

void Slideshow::setRehearseTimings( bool bRehearseTimings )
{
    mpImpl->mbRehearseTimings = bRehearseTimings;
}

bool Slideshow::isFullScreen()
{
    return mpImpl->maPresSettings.mbFullScreen;
}

void Slideshow::resize( const Size &rSize )
{
    mpImpl->resize( rSize );
}

void Slideshow::activate()
{
    mpImpl->activate();
}

void Slideshow::deactivate()
{
    mpImpl->deactivate();
}

void Slideshow::setWindow( sd::Window* pWindow )
{
}

bool Slideshow::requestHelp(const HelpEvent& rHEvt)
{
    return false;
}

bool Slideshow::isTerminated()
{
    return !mpImpl->mxShow.is();
}

bool Slideshow::keyInput(const KeyEvent& rKEvt)
{
    return mpImpl->keyInput(rKEvt);
}

void Slideshow::mouseButtonDown(const MouseEvent& rMEvt)
{
}

void Slideshow::mouseMove(const MouseEvent& rMEvt)
{
}

void Slideshow::mouseButtonUp(const MouseEvent& rMEvt)
{
    mpImpl->mouseButtonUp( rMEvt );
}

void Slideshow::command(const CommandEvent& rCEvt)
{
}

bool Slideshow::isAlwaysOnTop()
{
    return mpImpl->maPresSettings.mbAlwaysOnTop;
}

bool Slideshow::pause( bool bPause )
{
    return mpImpl->pause( bPause );
}

void Slideshow::receiveRequest(SfxRequest& rReq)
{
    mpImpl->receiveRequest( rReq );
}

sal_Int32 Slideshow::getFirstPageNumber()
{
    return mpImpl->getFirstSlideNumber();
}

sal_Int32 Slideshow::getLastPageNumber()
{
    return mpImpl->getLastSlideNumber();
}

bool Slideshow::isEndless()
{
    return mpImpl->isEndless();
}

bool Slideshow::isDrawingPossible()
{
    return mpImpl->isDrawingPossible();
}


