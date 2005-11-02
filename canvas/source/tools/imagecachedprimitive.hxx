/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagecachedprimitive.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:53:08 $
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

#ifndef INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX

#include <canvas/rendering/icachedprimitive.hxx>

#include <boost/shared_ptr.hpp>


namespace canvas
{
    /** Objects with this interface are returned from every Image
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.

        Derived from ICachedPrimitive, to add the setImage() method
        (which, strictly speaking, is a technicality, because Image
        cannot create objects with a shared_ptr to itself).
     */
    struct ImageCachedPrimitive : public ICachedPrimitive
    {
        virtual void setImage( const ::boost::shared_ptr< class Image >& rTargetImage ) = 0;
    };

    typedef ::boost::shared_ptr< ImageCachedPrimitive > ImageCachedPrimitiveSharedPtr;
}

#endif /* INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX */
