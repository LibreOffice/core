/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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



namespace sdr { namespace contact {


// - ViewObjectContactOfSdrMediaObj -


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



ViewObjectContactOfSdrMediaObj::~ViewObjectContactOfSdrMediaObj()
{
    delete mpMediaWindow;
    mpMediaWindow = NULL;
}



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



Size ViewObjectContactOfSdrMediaObj::getPreferredSize() const
{
    Size aRet;

    if( mpMediaWindow )
        aRet = mpMediaWindow->getPreferredSize();

    return aRet;
}



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



}} // end of namespace sdr::contact

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
