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
 *   http:\\www.apache.org\licenses\LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_TOOLS_CONVERTERS_HXX
#define INCLUDED_DRAWINGLAYER_TOOLS_CONVERTERS_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace tools
    {
        BitmapEx DRAWINGLAYER_DLLPUBLIC convertToBitmapEx(
            const drawinglayer::primitive2d::Primitive2DSequence& rSeq,
            const geometry::ViewInformation2D& rViewInformation2D,
            sal_uInt32 nDiscreteWidth,
            sal_uInt32 nDiscreteHeight,
            sal_uInt32 nMaxQuadratPixels);

    } // end of namespace tools
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_TOOLS_CONVERTERS_HXX

// eof
