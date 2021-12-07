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

#include <sal/config.h>

#include <algorithm>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <vcl/graph.hxx>

namespace drawinglayer::attribute
{
        class ImpFillGraphicAttribute
        {
        public:
            // data definitions
            Graphic                                 maGraphic;
            basegfx::B2DRange                       maGraphicRange;

            bool                                    mbTiling : 1;

            // tiling definitions, offsets in X/Y in percent for each 2nd row.
            // If both are set, Y is ignored (X has precedence)
            double                                  mfOffsetX;
            double                                  mfOffsetY;

            ImpFillGraphicAttribute(
                const Graphic& rGraphic,
                const basegfx::B2DRange& rGraphicRange,
                bool bTiling,
                double fOffsetX,
                double fOffsetY)
            :   maGraphic(rGraphic),
                maGraphicRange(rGraphicRange),
                mbTiling(bTiling),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY)
            {
                // access once to ensure that the buffered bitmap exists, else
                // the SolarMutex may be needed to create it. This may not be
                // available when a renderer works with multi-threading.
                // When changing this, please check if it is still possible to
                // use a metafile as texture for a 3D object
                maGraphic.GetBitmapEx();
            }

            ImpFillGraphicAttribute()
            :   mbTiling(false),
                mfOffsetX(0.0),
                mfOffsetY(0.0)
            {
            }

            // data read access
            const Graphic& getGraphic() const { return maGraphic; }
            const basegfx::B2DRange& getGraphicRange() const { return maGraphicRange; }
            bool getTiling() const { return mbTiling; }
            double getOffsetX() const { return mfOffsetX; }
            double getOffsetY() const { return mfOffsetY; }

            bool operator==(const ImpFillGraphicAttribute& rCandidate) const
            {
                return (getGraphic() == rCandidate.getGraphic()
                    && getGraphicRange() == rCandidate.getGraphicRange()
                    && getTiling() == rCandidate.getTiling()
                    && getOffsetX() == rCandidate.getOffsetX()
                    && getOffsetY() == rCandidate.getOffsetY());
            }
        };

        namespace
        {
            FillGraphicAttribute::ImplType& theGlobalDefault()
            {
                static FillGraphicAttribute::ImplType SINGLETON;
                return SINGLETON;
            }
        }

        FillGraphicAttribute::FillGraphicAttribute(
            const Graphic& rGraphic,
            const basegfx::B2DRange& rGraphicRange,
            bool bTiling,
            double fOffsetX,
            double fOffsetY)
        :   mpFillGraphicAttribute(ImpFillGraphicAttribute(
                rGraphic, rGraphicRange, bTiling,
                    std::clamp(fOffsetX, 0.0, 1.0),
                    std::clamp(fOffsetY, 0.0, 1.0)))
        {
        }

        FillGraphicAttribute::FillGraphicAttribute(const FillGraphicAttribute&) = default;

        FillGraphicAttribute::~FillGraphicAttribute() = default;

        bool FillGraphicAttribute::isDefault() const
        {
            return mpFillGraphicAttribute.same_object(theGlobalDefault());
        }

        FillGraphicAttribute& FillGraphicAttribute::operator=(const FillGraphicAttribute&) = default;

        bool FillGraphicAttribute::operator==(const FillGraphicAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpFillGraphicAttribute == mpFillGraphicAttribute;
        }

        const Graphic& FillGraphicAttribute::getGraphic() const
        {
            return mpFillGraphicAttribute->getGraphic();
        }

        const basegfx::B2DRange& FillGraphicAttribute::getGraphicRange() const
        {
            return mpFillGraphicAttribute->getGraphicRange();
        }

        bool FillGraphicAttribute::getTiling() const
        {
            return mpFillGraphicAttribute->getTiling();
        }

        double FillGraphicAttribute::getOffsetX() const
        {
            return mpFillGraphicAttribute->getOffsetX();
        }

        double FillGraphicAttribute::getOffsetY() const
        {
            return mpFillGraphicAttribute->getOffsetY();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
