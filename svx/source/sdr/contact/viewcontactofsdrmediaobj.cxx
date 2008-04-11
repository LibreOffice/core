/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrmediaobj.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>

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

sal_Bool ViewContactOfSdrMediaObj::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
{
    return ViewContactOfSdrObj::PaintObject( rDisplayInfo, rPaintRectangle, rAssociatedVOC );
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
    bool bRetval(true);

    for(sal_uInt32 a(0L); bRetval && a < maVOCList.Count(); a++)
    {
        if(!static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( 0 ) )->hasPreferredSize())
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
    for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
    {
        Size aSize(static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( 0 ) )->getPreferredSize());

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
    for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
    {
        static_cast< ViewObjectContactOfSdrMediaObj* >(maVOCList.GetObject(a))->updateMediaItem(rItem);
    }
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
    for( sal_uInt32 n(0L); n < maVOCList.Count(); n++ )
    {
        static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( n ) )->executeMediaItem( rItem );
    }
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState )
{
    static_cast< SdrMediaObj& >( GetSdrObject() ).mediaPropertiesChanged( rNewState );
}

// ------------------------------------------------------------------------------
// #i72701#
sal_Bool ViewContactOfSdrMediaObj::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
{
    // set pos/size of associated media window
    const ViewObjectContactOfSdrMediaObj& rVOC(dynamic_cast< const ViewObjectContactOfSdrMediaObj& >(rAssociatedVOC));
    rVOC.checkMediaWindowPosition(rDisplayInfo);

    // call parent
    return ViewContactOfSdrObj::ShouldPaintObject(rDisplayInfo, rAssociatedVOC);
}

// ------------------------------------------------------------------------------

}} // end of namespace sdr::contact

// eof
