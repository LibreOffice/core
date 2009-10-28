/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metafileprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/gdimtf.hxx>

//////////////////////////////////////////////////////////////////////////////
// MetafilePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MediaPrimitive2D class

            This is the MetaFile representing primitive. It's geometry is defined
            by MetaFileTransform. The content (defined by MetaFile) will be scaled
            to the geometric definiton by using PrefMapMode and PrefSize of the
            Metafile.

            It has shown that this not always guarantees that all Metafile content
            is inside the geometric definition, but this primitive defines that this
            is the case to allow a getB2DRange implementation. If it cannot be
            guaranteed that the Metafile is inside the geometric definition, it should
            be embedded to a MaskPrimitive2D.

            This primitive has no decomposition yet, so when not supported by a renderer,
            it will not be visualized.

            In the future, a decomposition implementation would be appreciated and would
            have many advantages; Metafile would no longer have to be rendered by
            sub-systems and a standard way for converting Metafiles would exist.
         */
        class MetafilePrimitive2D : public BasePrimitive2D
        {
        private:
            /// the geometry definition
            basegfx::B2DHomMatrix                       maMetaFileTransform;

            /// the content definition
            GDIMetaFile                                 maMetaFile;

        public:
            /// constructor
            MetafilePrimitive2D(
                const basegfx::B2DHomMatrix& rMetaFileTransform,
                const GDIMetaFile& rMetaFile);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maMetaFileTransform; }
            const GDIMetaFile& getMetaFile() const { return maMetaFile; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
