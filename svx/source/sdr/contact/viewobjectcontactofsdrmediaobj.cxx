/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewobjectcontactofsdrmediaobj.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:36:01 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <avmedia/mediaitem.hxx>
#include "sdrmediawindow.hxx"

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

namespace sdr { namespace contact {

// ----------------------------------
// - ViewObjectContactOfSdrMediaObj -
// ----------------------------------

ViewObjectContactOfSdrMediaObj::ViewObjectContactOfSdrMediaObj( ObjectContact& rObjectContact,
                                                                ViewContact& rViewContact,
                                                                const ::avmedia::MediaItem& rMediaItem ) :
    ViewObjectContact( rObjectContact, rViewContact ),
    mpMediaWindow( NULL )
{
    Window* pWindow = getWindow();

    if( pWindow )
    {
        mpMediaWindow = new SdrMediaWindow( pWindow, *this );

        // #i72701#
        // To avoid popping up of a window on a non-initialized position, the
        // window will be invisible now as initial state. It will be made visible
        // in paint
        mpMediaWindow->hide();

        executeMediaItem( rMediaItem );
    }
}

// ------------------------------------------------------------------------------

ViewObjectContactOfSdrMediaObj::~ViewObjectContactOfSdrMediaObj()
{
    DBG_ASSERT( !mpMediaWindow, "ViewObjectContactOfSdrMediaObj::~ViewObjectContactOfSdrMediaObj(): mpMediaWindow != NULL" );
}

// ------------------------------------------------------------------------------

void ViewObjectContactOfSdrMediaObj::PrepareDelete()
{
    ViewObjectContact::PrepareDelete();
    delete mpMediaWindow;
    mpMediaWindow = NULL;
}

// ------------------------------------------------------------------------------

void ViewObjectContactOfSdrMediaObj::PaintObject(DisplayInfo& rDisplayInfo)
{
    SdrObject* pObj = GetViewContact().TryToGetSdrObject();

    ViewObjectContact::PaintObject( rDisplayInfo );

    if( pObj )
    {
        Rectangle       aPaintRect( pObj->GetCurrentBoundRect() );
        OutputDevice*   pOutDev = rDisplayInfo.GetOutputDevice();
        sal_Int32       nOffset( pOutDev->PixelToLogic( Size( 4, 4 ) ).Width() );
        bool            bWasPainted = false;

        aPaintRect.Left() += nOffset;
        aPaintRect.Top() += nOffset;
        aPaintRect.Right() -= nOffset;
        aPaintRect.Bottom() -= nOffset;

        if( !mpMediaWindow )
        {
            //OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();

            if( pOutDev &&
                ( aPaintRect.Left() < aPaintRect.Right() &&
                  aPaintRect.Top() < aPaintRect.Bottom() ) )
            {
                const Color aBackgroundColor( 67, 67, 67 );

                pOutDev->SetLineColor( aBackgroundColor );
                pOutDev->SetFillColor( aBackgroundColor );
                pOutDev->DrawRect( aPaintRect );

                if( pObj->ISA( SdrMediaObj ) )
                {
                    const Graphic& rGraphic = static_cast< SdrMediaObj* >( pObj )->getGraphic();

                    if( rGraphic.GetType() != GRAPHIC_NONE )
                    {
                        rGraphic.Draw( pOutDev, aPaintRect.TopLeft(), aPaintRect.GetSize() );
                    }
                }

                bWasPainted = true;
            }
        }
        else
        {
            // #i72701#
            // Take care of position
            checkMediaWindowPosition(rDisplayInfo);

            // make visible and invalidate associated window
            mpMediaWindow->show();

            Window* pWindow = mpMediaWindow->getWindow();

            if(pWindow)
            {
                pWindow->Invalidate();
            }

            bWasPainted = true;
        }

        if( bWasPainted )
        {
            mbIsPainted = sal_True;
            maPaintedRectangle = pObj->GetCurrentBoundRect();
        }
    }
}

// ------------------------------------------------------------------------------

Window* ViewObjectContactOfSdrMediaObj::getWindow() const
{
    Window* pRetval = 0;

    const ObjectContactOfPageView* pObjectContactOfPageView = dynamic_cast< const ObjectContactOfPageView* >(&GetObjectContact());

    if(pObjectContactOfPageView)
    {
        const SdrPageWindow& rPageWindow = pObjectContactOfPageView->GetPageWindow();
        const SdrPaintWindow* pPaintWindow = &rPageWindow.GetPaintWindow();

        if(rPageWindow.GetOriginalPaintWindow())
        {
            // #i83183# prefer OriginalPaintWindow if set; this is
            // the real target device. GetPaintWindow() may return
            // the current buffer device instead
            pPaintWindow = rPageWindow.GetOriginalPaintWindow();
        }

        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

        if(OUTDEV_WINDOW == rOutDev.GetOutDevType())
        {
            pRetval = static_cast< Window* >(&rOutDev);
        }
    }

    return pRetval;
}

// ------------------------------------------------------------------------------

bool ViewObjectContactOfSdrMediaObj::hasPreferredSize() const
{
    return( mpMediaWindow != NULL && mpMediaWindow->hasPreferredSize() );
}

// ------------------------------------------------------------------------------

Size ViewObjectContactOfSdrMediaObj::getPreferredSize() const
{
    Size aRet;

    if( mpMediaWindow )
        aRet = mpMediaWindow->getPreferredSize();

    return aRet;
}

// ------------------------------------------------------------------------------

void ViewObjectContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
    if( mpMediaWindow )
        mpMediaWindow->updateMediaItem( rItem );
}

// ------------------------------------------------------------------------------

void ViewObjectContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
    if( mpMediaWindow )
    {
        ::avmedia::MediaItem aUpdatedItem;

        mpMediaWindow->executeMediaItem( rItem );

        // query new properties after trying to set the new properties
        updateMediaItem( aUpdatedItem );
        static_cast< ViewContactOfSdrMediaObj& >( GetViewContact() ).mediaPropertiesChanged( aUpdatedItem );
    }
}

// ------------------------------------------------------------------------------
// #i72701#

void ViewObjectContactOfSdrMediaObj::checkMediaWindowPosition(DisplayInfo& rDisplayInfo) const
{
    if(mpMediaWindow)
    {
        SdrObject* pObj = GetViewContact().TryToGetSdrObject();

        if(pObj)
        {
            // get pixel rect
            OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
            Rectangle aPaintRectPixel(pOutDev->LogicToPixel(pObj->GetCurrentBoundRect()));

            // shrink by 4 pixel to avoid control overlap
            aPaintRectPixel.Left() += 4L;
            aPaintRectPixel.Top() += 4L;
            aPaintRectPixel.Right() -= 4L;
            aPaintRectPixel.Bottom() -= 4L;

            // justify rect. Take no special action when the rect is empty
            aPaintRectPixel.Justify();

            // set size
            mpMediaWindow->setPosSize(aPaintRectPixel);
        }
    }
}

// ------------------------------------------------------------------------------

}} // end of namespace sdr::contact

// eof
