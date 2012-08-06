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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <vcl/pdfwriter.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** StructureTagPrimitive2D class

            This class is used to provode simple support for adding grouped
            pdf writer structured element information like used in sd from
            unomodel.cxx where a ViewObjectContactRedirector is used to add
            such information for diverse objects.
            This primitive encapsulates these and the VCLPdfRenderer uses it
            to apply the needed infos directly to the pdf export in a compatible
            way.
            If a renderer ignores this, it just decomposes to it's child
            content.
         */
        class DRAWINGLAYER_DLLPUBLIC StructureTagPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the PDF structure element this grouping represents
            vcl::PDFWriter::StructElement           maStructureElement;

        public:
            /// constructor
            StructureTagPrimitive2D(
                const vcl::PDFWriter::StructElement& rStructureElement,
                const Primitive2DSequence& rChildren);

            /// data read access
            const vcl::PDFWriter::StructElement& getStructureElement() const { return maStructureElement; }

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
