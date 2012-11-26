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

#ifndef _GLUEPOINT_HXX
#define _GLUEPOINT_HXX

#include <sal/types.h>
#include <basegfx/point/b2dpoint.hxx>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/drawing/EscapeDirection.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace gluepoint
    {
        enum GluePointType
        {
            GLUEPOINTTYPE_AUTO,
            GLUEPOINTTYPE_EDGE,
            GLUEPOINTTYPE_USER
        };

        class GluePoint
        {
        private:
            basegfx::B2DPoint                           maPosition;
            sal_uInt32                                  mnIdentifier;
            GluePointType                               meType;
            com::sun::star::drawing::Alignment          meAlignment;
            com::sun::star::drawing::EscapeDirection    meEscapeDirection;

        public:
            GluePoint()
            :   maPosition(0.0, 0.0),
                mnIdentifier(0),
                meType(GLUEPOINTTYPE_AUTO),
                meAlignment(com::sun::star::drawing::Alignment_TOP_LEFT),
                meEscapeDirection(com::sun::star::drawing::EscapeDirection_SMART)
            {
            }

            GluePoint(
                const basegfx::B2DPoint& rPosition,
                sal_uInt32 nIdentifier,
                GluePointType eType,
                com::sun::star::drawing::Alignment eAlignment = com::sun::star::drawing::Alignment_CENTER,
                com::sun::star::drawing::EscapeDirection eEscapeDirection = com::sun::star::drawing::EscapeDirection_SMART)
            :   maPosition(rPosition),
                mnIdentifier(nIdentifier),
                meType(eType),
                meAlignment(eAlignment),
                meEscapeDirection(eEscapeDirection)
            {
            }

            const basegfx::B2DPoint& getPosition() const { return maPosition; }
            sal_uInt32 getIdentifier() const { return mnIdentifier; }
            GluePointType getType() const { return meType; }
            com::sun::star::drawing::Alignment getAlignment() const { return meAlignment; }
            com::sun::star::drawing::EscapeDirection getEscapeDirection() const { return meEscapeDirection; }
        };
    } // end of namespace gluepoint
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace gluepoint
    {
        class GluePointProvider
        {
        public:
            // construction, destruction, copying
            GluePointProvider();
            virtual ~GluePointProvider();
            GluePointProvider(const GluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

            // Clone() operator calls the local copy constructor
            virtual GluePointProvider& Clone() const;

            // AutoGluePoint access
            virtual sal_uInt32 getAutoGluePointCount() const;
            virtual GluePoint getAutoGluePointByIndex(sal_uInt32 nIndex) const;

            // EdgeGluePoint access
            virtual sal_uInt32 getEdgeGluePointCount() const;
            virtual GluePoint getEdgeGluePointByIndex(sal_uInt32 nIndex) const;

            // UserGluePoint access
            virtual sal_uInt32 getUserGluePointCount() const;
            virtual GluePoint getUserGluePointByIndex(sal_uInt32 nIndex) const;

            // bundled GluePoint access
            sal_uInt32 getGluePointCount() const;
            GluePoint getGluePointByIndex(sal_uInt32 nIndex) const;

            // UserGluePoint manipulation
            virtual bool areUserGluePointsAllowed() const;
            virtual GluePoint addUserGluePoint(
                const basegfx::B2DPoint& rPosition,
                GluePointType eType,
                com::sun::star::drawing::Alignment eAlignment,
                com::sun::star::drawing::EscapeDirection eEscapeDirection);
            virtual void removeUserGluePoint(const GluePoint& rCandidate);
            virtual bool existsUserGluePoint(const GluePoint& rCandidate) const;
        };
    } // end of namespace gluepoint
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_GLUEPOINT_HXX

// eof
