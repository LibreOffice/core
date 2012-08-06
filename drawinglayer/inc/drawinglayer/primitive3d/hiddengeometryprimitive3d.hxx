/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        // This primitive is used to represent geometry for non-visible objects,
        // e.g. a 3D cube without fill attributes. To still be able to use
        // primitives for HitTest functionality, the 3d decompositions produce
        // an as much as possible simplified fill geometry encapsulated in this
        // primtive when there is no fill geometry. Currently, the 3d hit test
        // uses only areas, so maybe in a further enchanced version this will change
        // to 'if neither filled nor lines' creation criteria. The whole primitive
        // decomposes to nothing, so no one not knowing it will be influenced. Only
        // helper processors for hit test (and maybe BoundRect extractors) will
        // use it and it's children subcontent.
        class DRAWINGLAYER_DLLPUBLIC HiddenGeometryPrimitive3D : public GroupPrimitive3D
        {
        public:
            explicit HiddenGeometryPrimitive3D(const Primitive3DSequence& rChildren);

            // despite returning an empty decomposition since it's no visualisation data,
            // range calculation is intended to use hidden geometry, so
            // the local implementation will return the children's range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// The default implementation returns an empty sequence
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
