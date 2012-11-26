/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    {
        mpMediaWindow->updateMediaItem( rItem );

        // show/hide is now dependent of play state
        if(avmedia::MEDIASTATE_STOP == rItem.getState())
        {
            mpMediaWindow->hide();
        }
        else
        {
            basegfx::B2DRange aViewRange(getViewDependentRange());
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

//////////////////////////////////////////////////////////////////////////////
// eof
