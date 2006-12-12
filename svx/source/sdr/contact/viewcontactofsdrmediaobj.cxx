/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofsdrmediaobj.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:38:38 $
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

#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include "svdomedia.hxx"

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#endif

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

} }

// eof
