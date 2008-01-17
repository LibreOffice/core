/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
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

#ifndef OOX_VML_SHAPE_HXX
#define OOX_VML_SHAPE_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <rtl/ustring.hxx>

namespace oox { namespace vml {

struct Shape
{
    Shape();
    ~Shape();

    void applyAttributes( const Shape& rSource );

    rtl::OUString   msId;
    rtl::OUString   msType;
    sal_Int32       mnShapeType;

    sal_Int32       mnCoordWidth;
    sal_Int32       mnCoordHeight;
    sal_Int32       mnStroked;
    sal_Int32       mnFilled;

    rtl::OUString   msGraphicURL;
    rtl::OUString   msImageTitle;
    rtl::OUString   msPath;
};

typedef boost::shared_ptr< Shape > ShapePtr;

} }

#endif  //  OOX_VML_SHAPE_HXX
