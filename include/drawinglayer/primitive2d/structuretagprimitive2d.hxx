/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <vcl/pdfwriter.hxx>


namespace drawinglayer
{
    namespace primitive2d
    {
        /** StructureTagPrimitive2D class

            This class is used to provide simple support for adding grouped
            pdf writer structured element information like used in sd from
            unomodel.cxx where a ViewObjectContactRedirector is used to add
            such information for diverse objects.
            This primitive encapsulates these and the VCLPdfRenderer uses it
            to apply the needed infos directly to the pdf export in a compatible
            way.
            If a renderer ignores this, it just decomposes to its child
            content.
         */
        class DRAWINGLAYER_DLLPUBLIC StructureTagPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the PDF structure element this grouping represents
            vcl::PDFWriter::StructElement           maStructureElement;

            ///Z flag for background contenht that may be handled as
            /// Tagged PDF '/Artifact'
            bool                                    mbBackground;

        public:
            /// constructor
            StructureTagPrimitive2D(
                const vcl::PDFWriter::StructElement& rStructureElement,
                bool bBackground,
                const Primitive2DContainer& rChildren);

            /// data read access
            const vcl::PDFWriter::StructElement& getStructureElement() const { return maStructureElement; }
            bool isBackground() const { return mbBackground; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_STRUCTURETAGPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
