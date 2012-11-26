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
#include "precompiled_svx.hxx"
#include <svx/gluepoint.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace gluepoint
    {
        GluePointProvider::GluePointProvider()
        {
        }

        GluePointProvider::~GluePointProvider()
        {
        }

        GluePointProvider::GluePointProvider(const GluePointProvider& /*rCandidate*/)
        {
        }

        GluePointProvider& GluePointProvider::operator=(const GluePointProvider& /*rCandidate*/)
        {
            return *this;
        }

        GluePointProvider& GluePointProvider::Clone() const
        {
            return *(new GluePointProvider(*this));
        }

        sal_uInt32 GluePointProvider::getAutoGluePointCount() const
        {
            return 4;
        }

        GluePoint GluePointProvider::getAutoGluePointByIndex(sal_uInt32 nIndex) const
        {
            switch(nIndex)
            {
                case 0:
                {
                    return GluePoint(
                        basegfx::B2DPoint(0.5, 0.0),
                        nIndex,
                        GLUEPOINTTYPE_AUTO);
                }
                case 1:
                {
                    return GluePoint(
                        basegfx::B2DPoint(1.0, 0.5),
                        nIndex,
                        GLUEPOINTTYPE_AUTO);
                }
                case 2:
                {
                    return GluePoint(
                        basegfx::B2DPoint(0.5, 1.0),
                        nIndex,
                        GLUEPOINTTYPE_AUTO);
                }
                default:
                {
                    return GluePoint(
                        basegfx::B2DPoint(0.0, 0.5),
                        nIndex,
                        GLUEPOINTTYPE_AUTO);
                }
            }
        }

        sal_uInt32 GluePointProvider::getEdgeGluePointCount() const
        {
            return 0;
        }

        GluePoint GluePointProvider::getEdgeGluePointByIndex(sal_uInt32 /*nIndex*/) const
        {
            OSL_ENSURE(false, "access out of range (!)");
            return GluePoint();
        }

        sal_uInt32 GluePointProvider::getUserGluePointCount() const
        {
            return 0;
        }

        GluePoint GluePointProvider::getUserGluePointByIndex(sal_uInt32 /*nIndex*/) const
        {
            OSL_ENSURE(false, "access out of range (!)");
            return GluePoint();
        }

        sal_uInt32 GluePointProvider::getGluePointCount() const
        {
            return getAutoGluePointCount() + getEdgeGluePointCount() + getUserGluePointCount();
        }

        GluePoint GluePointProvider::getGluePointByIndex(sal_uInt32 nIndex) const
        {
            if(nIndex < getAutoGluePointCount())
            {
                return getAutoGluePointByIndex(nIndex);
            }

            nIndex -= getAutoGluePointCount();

            if(nIndex < getEdgeGluePointCount())
            {
                return getEdgeGluePointByIndex(nIndex);
            }

            nIndex -= getEdgeGluePointCount();

            if(nIndex < getUserGluePointCount())
            {
                return getUserGluePointByIndex(nIndex);
            }

            OSL_ENSURE(false, "access out of range (!)");
            return GluePoint();
        }

        bool GluePointProvider::areUserGluePointsAllowed() const
        {
            return false;
        }

        GluePoint GluePointProvider::addUserGluePoint(
            const basegfx::B2DPoint& /*rPosition*/,
            GluePointType /*eType*/,
            com::sun::star::drawing::Alignment /*eAlignment*/,
            com::sun::star::drawing::EscapeDirection /*eEscapeDirection*/)
        {
            OSL_ENSURE(false, "No UserGluePoints allowed at this object (!)");
            return GluePoint();
        }

        void GluePointProvider::removeUserGluePoint(const GluePoint& /*rCandidate*/)
        {
            OSL_ENSURE(false, "No UserGluePoints allowed at this object (!)");
        }

        bool GluePointProvider::existsUserGluePoint(const GluePoint& /*rCandidate*/) const
        {
            return false;
        }

    } // end of namespace gluepoint
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
