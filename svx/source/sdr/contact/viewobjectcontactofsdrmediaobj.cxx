/*************************************************************************
 *
 *  $RCSfile: viewobjectcontactofsdrmediaobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:04:47 $
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

#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <avmedia/mediaitem.hxx>
#include "sdrmediawindow.hxx"

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
            OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();

            if( pOutDev &&
                ( aPaintRect.Left() < aPaintRect.Right() &&
                  aPaintRect.Top() < aPaintRect.Bottom() ) )
            {
                pOutDev->SetLineColor( COL_BLACK );
                pOutDev->SetFillColor( COL_BLACK );
                pOutDev->DrawRect( aPaintRect );
                bWasPainted = true;
            }
        }
        else
        {
            Rectangle   aCurPaintRect( pOutDev->LogicToPixel( aPaintRect.TopLeft() ),
                                       pOutDev->LogicToPixel( aPaintRect.GetSize() ) );
            const bool  bNewPaintRect = ( maLastPaintRect.IsEmpty() || ( maLastPaintRect != aCurPaintRect ) );

            if( bNewPaintRect )
            {
                mpMediaWindow->setPosSize( aCurPaintRect );
                maLastPaintRect = aCurPaintRect;
            }
            else
            {
                Window* pWindow = mpMediaWindow->getWindow();

                if( pWindow )
                {
                    pWindow->Invalidate();
                    pWindow->Update();
                }
            }

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
    OutputDevice& rOutDev = static_cast< ObjectContactOfPageView& >( GetObjectContact() ).GetPageViewWindow().GetOutputDevice();

    return( ( rOutDev.GetOutDevType() == OUTDEV_WINDOW ) ? static_cast< Window* >( &rOutDev ) : NULL );
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
