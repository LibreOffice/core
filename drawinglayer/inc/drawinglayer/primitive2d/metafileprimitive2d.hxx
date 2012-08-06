/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/gdimtf.hxx>

//////////////////////////////////////////////////////////////////////////////
// MetafilePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MetafilePrimitive2D class

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
        class DRAWINGLAYER_DLLPUBLIC MetafilePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometry definition
            basegfx::B2DHomMatrix                       maMetaFileTransform;

            /// the content definition
            GDIMetaFile                                 maMetaFile;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
