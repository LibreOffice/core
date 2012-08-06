/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr { namespace contact {

// ----------------------------------
// - ViewObjectContactOfSdrMediaObj -
// ----------------------------------

ViewObjectContactOfSdrMediaObj::ViewObjectContactOfSdrMediaObj( ObjectContact& rObjectContact,
                                                                ViewContact& rViewContact,
                                                                const ::avmedia::MediaItem& rMediaItem ) :
    ViewObjectContactOfSdrObj( rObjectContact, rViewContact ),
    mpMediaWindow( NULL )
{
    Window* pWindow = getWindow();

    if( pWindow )
    {
        mpMediaWindow = new SdrMediaWindow( pWindow, *this );
        mpMediaWindow->hide();
        executeMediaItem( rMediaItem );
    }
}

// ------------------------------------------------------------------------------

ViewObjectContactOfSdrMediaObj::~ViewObjectContactOfSdrMediaObj()
{
    delete mpMediaWindow;
    mpMediaWindow = NULL;
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
    {
        mpMediaWindow->updateMediaItem( rItem );

        // show/hide is now dependent of play state
        if(avmedia::MEDIASTATE_STOP == rItem.getState())
        {
            mpMediaWindow->hide();
        }
        else
        {
            basegfx::B2DRange aViewRange(getObjectRange());
            aViewRange.transform(GetObjectContact().getViewInformation2D().getViewTransformation());

            const Rectangle aViewRectangle(
                (sal_Int32)floor(aViewRange.getMinX()), (sal_Int32)floor(aViewRange.getMinY()),
                (sal_Int32)ceil(aViewRange.getMaxX()), (sal_Int32)ceil(aViewRange.getMaxY()));

            mpMediaWindow->setPosSize(aViewRectangle);
            mpMediaWindow->show();
        }
    }
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

}} // end of namespace sdr::contact

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
