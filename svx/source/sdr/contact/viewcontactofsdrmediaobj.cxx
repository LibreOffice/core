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


#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#include <avmedia/mediawindow.hxx>

namespace sdr { namespace contact {

// ----------------------------
// - ViewContactOfSdrMediaObj -
// ----------------------------

ViewContactOfSdrMediaObj::ViewContactOfSdrMediaObj( SdrMediaObj& rMediaObj ) :
    ViewContactOfSdrObj( rMediaObj )
{
}

// ------------------------------------------------------------------------------

ViewContactOfSdrMediaObj::~ViewContactOfSdrMediaObj()
{
}

// ------------------------------------------------------------------------------

ViewObjectContact& ViewContactOfSdrMediaObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    return *( new ViewObjectContactOfSdrMediaObj( rObjectContact, *this, static_cast< SdrMediaObj& >( GetSdrObject() ).getMediaProperties() ) );
}

// ------------------------------------------------------------------------------

Size ViewContactOfSdrMediaObj::getPreferredSize() const
{
    // #i71805# Since we may have a whole bunch of VOCs here, make a loop
    // return first useful size -> the size from the first which is visualized as a window
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);
        Size aSize(pCandidate ? static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->getPreferredSize() : Size());

        if(0 != aSize.getWidth() || 0 != aSize.getHeight())
        {
            return aSize;
        }
    }

    return Size();
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
    // #i71805# Since we may have a whole bunch of VOCs here, make a loop
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);

        if(pCandidate)
        {
            static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->updateMediaItem(rItem);
        }
    }
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);

        if(pCandidate)
        {
            static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->executeMediaItem(rItem);
        }
    }
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState )
{
    static_cast< SdrMediaObj& >(GetSdrObject()).mediaPropertiesChanged(rNewState);
}

}} // end of namespace sdr::contact

namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrMediaObj::createViewIndependentPrimitive2DSequence() const
        {
            // create range using the model data directly. This is in SdrTextObj::aRect which i will access using
            // GetGeoRect() to not trigger any calculations. It's the unrotated geometry which is okay for MediaObjects ATM.
            const Rectangle& rRectangle(GetSdrMediaObj().GetGeoRect());
            const basegfx::B2DRange aRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());

            // create object transform
            basegfx::B2DHomMatrix aTransform;
            aTransform.set(0, 0, aRange.getWidth());
            aTransform.set(1, 1, aRange.getHeight());
            aTransform.set(0, 2, aRange.getMinX());
            aTransform.set(1, 2, aRange.getMinY());

            // create media primitive. Always create primitives to allow the
            // decomposition of MediaPrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            const basegfx::BColor aBackgroundColor(67.0 / 255.0, 67.0 / 255.0, 67.0 / 255.0);
            const rtl::OUString& rURL(GetSdrMediaObj().getURL());
            const sal_uInt32 nPixelBorder(4L);
            const drawinglayer::primitive2d::Primitive2DReference xRetval(
                new drawinglayer::primitive2d::MediaPrimitive2D(
                    aTransform, rURL, aBackgroundColor, nPixelBorder,
                            avmedia::MediaWindow::grabFrame(rURL, true)));

            return drawinglayer::primitive2d::Primitive2DSequence(&xRetval, 1);
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
