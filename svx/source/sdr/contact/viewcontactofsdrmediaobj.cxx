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

#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>

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

bool ViewContactOfSdrMediaObj::hasPreferredSize() const
{
    // #i71805# Since we may have a whole bunch of VOCs here, make a loop
    // return true if all have their preferred size
    const sal_uInt32 nCount(getViewObjectContactCount());
    bool bRetval(true);

    for(sal_uInt32 a(0); bRetval && a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);

        if(pCandidate && !static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->hasPreferredSize())
        {
            bRetval = false;
        }
    }

    return bRetval;
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
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // get object transformation
            const basegfx::B2DHomMatrix& rObjectMatrix(GetSdrMediaObj().getSdrObjectTransformation());

            // create media primitive. Always create primitives to allow the
            // decomposition of MediaPrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            const basegfx::BColor aBackgroundColor(67.0 / 255.0, 67.0 / 255.0, 67.0 / 255.0);
            const rtl::OUString& rURL(GetSdrMediaObj().getURL());
            const sal_uInt32 nPixelBorder(4L);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::MediaPrimitive2D(
                    rObjectMatrix,
                    rURL,
                    aBackgroundColor,
                    nPixelBorder));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
