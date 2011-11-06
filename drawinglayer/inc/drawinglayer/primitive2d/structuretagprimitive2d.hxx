/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

//////////////////////////////////////////////////////////////////////////////
// eof
