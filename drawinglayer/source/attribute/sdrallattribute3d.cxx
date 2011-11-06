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

#include <drawinglayer/attribute/sdrallattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowAttribute3D::SdrLineFillShadowAttribute3D(
            const SdrLineAttribute& rLine,
            const SdrFillAttribute& rFill,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const FillGradientAttribute& rFillFloatTransGradient)
        :   maLine(rLine),
            maFill(rFill),
            maLineStartEnd(rLineStartEnd),
            maShadow(rShadow),
            maFillFloatTransGradient(rFillFloatTransGradient)
        {
        }

        SdrLineFillShadowAttribute3D::SdrLineFillShadowAttribute3D()
        :   maLine(),
            maFill(),
            maLineStartEnd(),
            maShadow(),
            maFillFloatTransGradient()
        {
        }

        bool SdrLineFillShadowAttribute3D::isDefault() const
        {
            return(getLine().isDefault()
                && getFill().isDefault()
                && getLineStartEnd().isDefault()
                && getShadow().isDefault()
                && getFillFloatTransGradient().isDefault());
        }

        bool SdrLineFillShadowAttribute3D::operator==(const SdrLineFillShadowAttribute3D& rCandidate) const
        {
            return(getLine() == rCandidate.getLine()
                && getFill() == rCandidate.getFill()
                && getLineStartEnd() == rCandidate.getLineStartEnd()
                && getShadow() == rCandidate.getShadow()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient());
        }
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
