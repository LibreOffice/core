/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapeattributelayerholder.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:01:42 $
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

#ifndef _SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX
#define _SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX

#include <attributableshape.hxx>
#include <shapeattributelayer.hxx>

#include <boost/utility.hpp>

namespace slideshow
{
    namespace internal
    {
        /** Holds a ShapeAttributeLayer, together with the associated
            Shape

            Use this class to hold ShapeAttributeLayer objects the
            RAII way. When this object gets deleted, it will
            automatically revoke the attribute layer for the given
            shape (this encapsulates the somewhat clumsy notification
            process that is required for shape and attribute layer
            interaction).
         */
        class ShapeAttributeLayerHolder : private boost::noncopyable
        {
        public:
            /** Create a ShapeAttributeLayerHolder instance.

                This constructor creates an empty attribute holder, to
                generate an attribute layer, you have to manually call
                createAttributeLayer().
             */
            ShapeAttributeLayerHolder() :
                mpShape(),
                mpAttributeLayer()
            {
            }

            ~ShapeAttributeLayerHolder()
            {
                reset(); // ensures that the last attribute layer is
                         // correctly deregistered from the shape.
            }

            void reset()
            {
                if( mpShape && mpAttributeLayer )
                    mpShape->revokeAttributeLayer( mpAttributeLayer );
            }

            /** This constructor receives a pointer to the Shape, from
                which attribute layers should be generated. Initially,
                this object does not create an attribute layer, you
                have to manually call createAttributeLayer().

                @param rShape
                Shape for which attribute layers should be generated.
            */
            bool createAttributeLayer( const AttributableShapeSharedPtr& rShape )
            {
                reset();

                mpShape = rShape;

                if( mpShape )
                    mpAttributeLayer = mpShape->createAttributeLayer();

                return mpAttributeLayer;
            }

            ShapeAttributeLayerSharedPtr get() const
            {
                return mpAttributeLayer;
            }

        private:
            AttributableShapeSharedPtr      mpShape;
            ShapeAttributeLayerSharedPtr    mpAttributeLayer;
        };

    }
}

#endif /* _SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX */
