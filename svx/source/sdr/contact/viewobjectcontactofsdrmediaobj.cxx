/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewobjectcontactofsdrmediaobj.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:32:16 $
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
#include "svdomedia.hxx"
#include "svdpagv.hxx"
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <avmedia/mediaitem.hxx>
#include "sdrmediawindow.hxx"

#ifndef _SDRPAGEWINDOW_HXX
#include <sdrpagewindow.hxx>
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
        executeMediaItem( rMediaItem );
        mpMediaWindow->show();
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
            const Rectangle aPaintRectPixel( pOutDev->LogicToPixel( aPaintRect.TopLeft() ),
                                                pOutDev->LogicToPixel( aPaintRect.GetSize() ) );

            mpMediaWindow->setPosSize( aPaintRectPixel );

            Window* pWindow = mpMediaWindow->getWindow();

            if( pWindow )
                pWindow->Invalidate();

            bWasPainted = true;
        }

        if( bWasPainted )
        {
            mbIsPainted = sal_True;
            mbIsInvalidated = sal_False;
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
        OutputDevice& rOutDev = pObjectContactOfPageView->GetPageWindow().GetPaintWindow().GetOutputDevice();

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

} }
