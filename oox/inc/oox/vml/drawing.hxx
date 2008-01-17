/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawing.hxx,v $
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

#ifndef OOX_VML_DRAWING_HXX
#define OOX_VML_DRAWING_HXX

#include <boost/shared_ptr.hpp>
#include "oox/vml/shape.hxx"
#include <vector>

namespace oox { namespace vml {

class Drawing
{
public:

    Drawing();
    ~Drawing();

    std::vector< ShapePtr >&    getShapeTypes() { return maShapeTypes; };
    std::vector< ShapePtr >&    getShapes() { return maShapes; };

    ShapePtr createShapeById( const rtl::OUString sId ) const;
    rtl::OUString getGraphicUrlById( const rtl::OUString sId ) const;

private:

    std::vector< ShapePtr > maShapeTypes;
    std::vector< ShapePtr > maShapes;
};

typedef boost::shared_ptr< Drawing > DrawingPtr;

} }

#endif  //  OOX_VML_DRAWING_HXX
