/*************************************************************************
 *
 *  $RCSfile: viewcontactofsdrmediaobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:04:36 $
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
    return *( new ViewObjectContactOfSdrMediaObj( rObjectContact, *this,
                                                  static_cast< SdrMediaObj& >( GetSdrObject() ).getMediaProperties() ) );
}

// ------------------------------------------------------------------------------

sal_Bool ViewContactOfSdrMediaObj::SupportsAnimation() const
{
    return false;
}

// ------------------------------------------------------------------------------

bool ViewContactOfSdrMediaObj::hasPreferredSize() const
{
    return( maVOCList.Count() &&
            static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( 0 ) )->hasPreferredSize() );
}

// ------------------------------------------------------------------------------

Size ViewContactOfSdrMediaObj::getPreferredSize() const
{
    Size aRet;

    if( maVOCList.Count() )
        aRet = static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( 0 ) )->getPreferredSize();

    return aRet;
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
    if( maVOCList.Count() )
        static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( 0 ) )->updateMediaItem( rItem );
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
    for( sal_Int32 n = 0; n < maVOCList.Count(); ++n )
        static_cast< ViewObjectContactOfSdrMediaObj* >( maVOCList.GetObject( n ) )->executeMediaItem( rItem );
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState )
{
    static_cast< SdrMediaObj& >( GetSdrObject() ).mediaPropertiesChanged( rNewState );
}

} }
