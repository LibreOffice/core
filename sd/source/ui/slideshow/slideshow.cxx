    /*************************************************************************
 *
 *  $RCSfile: slideshow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:34:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    return mpImpl->startPreview( xDrawPage, xAnimationNode, pParent );
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
    mpImpl->jumpToPageIndex( nPageIndex );
}

void Slideshow::jumpToPageNumber( sal_Int32 nPageNumber )
{
    mpImpl->jumpToPageNumber( nPageNumber );
}

sal_Int32 Slideshow::getCurrentPageNumber()
{
    return mpImpl->getCurrentPageNumber();
}

sal_Int32 Slideshow::getCurrentPageIndex()
{
    return mpImpl->getCurrentPageIndex();
}

void Slideshow::jumpToBookmark( const String& sBookmark )
{
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
    return mpImpl->getFirstPageNumber();
}

sal_Int32 Slideshow::getLastPageNumber()
{
    return mpImpl->getLastPageNumber();
}

bool Slideshow::isEndless()
{
    return mpImpl->isEndless();
}

bool Slideshow::isDrawingPossible()
{
    return mpImpl->isDrawingPossible();
}


