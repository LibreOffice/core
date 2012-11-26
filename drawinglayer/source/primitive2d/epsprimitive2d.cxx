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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence EpsPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const GDIMetaFile& rSubstituteContent = getMetaFile();

            if(rSubstituteContent.GetActionCount())
            {
                // the default decomposition will use the Metafile replacement visualisation.
                // To really use the Eps data, a renderer has to know and interpret this primitive
                // directly.
                xRetval.realloc(1);

                xRetval[0] = Primitive2DReference(
                    new MetafilePrimitive2D(
                        getEpsTransform(),
                        rSubstituteContent));
            }

            return xRetval;
        }

        EpsPrimitive2D::EpsPrimitive2D(
            const basegfx::B2DHomMatrix& rEpsTransform,
            const GfxLink& rGfxLink,
            const GDIMetaFile& rMetaFile)
        :   BufferedDecompositionPrimitive2D(),
            maEpsTransform(rEpsTransform),
            maGfxLink(rGfxLink),
            maMetaFile(rMetaFile)
        {
        }

        basegfx::B2DRange EpsPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use own implementation to quickly answer the getB2DRange question.
            return getEpsTransform() * basegfx::B2DRange::getUnitB2DRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(EpsPrimitive2D, PRIMITIVE2D_ID_EPSPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
