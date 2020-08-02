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

#include <config_features.h>

#include <sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <avmedia/mediaitem.hxx>
#include "sdrmediawindow.hxx"

namespace sdr::contact {

ViewObjectContactOfSdrMediaObj::ViewObjectContactOfSdrMediaObj( ObjectContact& rObjectContact,
                                                                ViewContact& rViewContact,
                                                                const ::avmedia::MediaItem& rMediaItem ) :
    ViewObjectContactOfSdrObj( rObjectContact, rViewContact )
{
#if HAVE_FEATURE_AVMEDIA
    vcl::Window* pWindow = getWindow();

    if( pWindow )
    {
        mpMediaWindow.reset( new SdrMediaWindow( pWindow, *this ) );
        mpMediaWindow->hide();
        executeMediaItem( rMediaItem );
    }
#else
    (void) rMediaItem;
#endif
}

ViewObjectContactOfSdrMediaObj::~ViewObjectContactOfSdrMediaObj()
{
}


vcl::Window* ViewObjectContactOfSdrMediaObj::getWindow() const
{
    vcl::Window* pRetval = nullptr;

    const OutputDevice* oPageOutputDev = getPageViewOutputDevice();
    if( oPageOutputDev )
    {
        if(OUTDEV_WINDOW == oPageOutputDev->GetOutDevType())
        {
            pRetval = static_cast< vcl::Window* >(&const_cast<OutputDevice&>(*oPageOutputDev));
        }
    }

    return pRetval;
}


Size ViewObjectContactOfSdrMediaObj::getPreferredSize() const
{
    Size aRet;

#if HAVE_FEATURE_AVMEDIA
    if( mpMediaWindow )
        aRet = mpMediaWindow->getPreferredSize();
#else
   aRet = Size(0,0);
#endif

    return aRet;
}


void ViewObjectContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
#if HAVE_FEATURE_AVMEDIA
    if( !mpMediaWindow )
        return;

    mpMediaWindow->updateMediaItem( rItem );

    // show/hide is now dependent of play state
    if(avmedia::MediaState::Stop == rItem.getState())
    {
        mpMediaWindow->hide();
    }
    else
    {
        basegfx::B2DRange aViewRange(getObjectRange());
        aViewRange.transform(GetObjectContact().getViewInformation2D().getViewTransformation());

        const tools::Rectangle aViewRectangle(
            static_cast<sal_Int32>(floor(aViewRange.getMinX())), static_cast<sal_Int32>(floor(aViewRange.getMinY())),
            static_cast<sal_Int32>(ceil(aViewRange.getMaxX())), static_cast<sal_Int32>(ceil(aViewRange.getMaxY())));

        // mpMediaWindow contains a SalObject window and gtk won't accept
        // the size until after the SalObject widget is shown but if we
        // show it before setting a size then vcl will detect that the
        // vcl::Window has no size and make it invisible instead. If we
        // call setPosSize twice with the same size before and after show
        // then the second attempt is a no-op as vcl caches the size.

        // so call it initially with a size arbitrarily 1 pixel wider than
        // we want so we have an initial size to make vcl happy
        tools::Rectangle aInitialRect(aViewRectangle);
        aInitialRect.AdjustRight(1);
        mpMediaWindow->setPosSize(aInitialRect);

        // then make it visible
        mpMediaWindow->show();

        // set the final desired size which is different to let vcl send it
        // through to gtk which will now accept it as the underlying
        // m_pSocket of GtkSalObject::SetPosSize is now visible
        mpMediaWindow->setPosSize(aViewRectangle);
    }
#else
    (void) rItem;
#endif
}


void ViewObjectContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
#if HAVE_FEATURE_AVMEDIA
    if( mpMediaWindow )
    {
        ::avmedia::MediaItem aUpdatedItem;

        mpMediaWindow->executeMediaItem( rItem );

        // query new properties after trying to set the new properties
        updateMediaItem( aUpdatedItem );
        static_cast< ViewContactOfSdrMediaObj& >( GetViewContact() ).mediaPropertiesChanged( aUpdatedItem );
    }
#else
    (void) rItem;
#endif
}


} // end of namespace sdr::contact

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
