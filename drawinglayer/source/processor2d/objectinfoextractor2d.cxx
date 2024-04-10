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

#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>

namespace drawinglayer::processor2d
{
        void ObjectInfoPrimitiveExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            if(mpFound)
                return;

            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D :
                {
                    mpFound = dynamic_cast< const primitive2d::ObjectInfoPrimitive2D* >(&rCandidate);
                    break;
                }
                default :
                {
                    // we look for an encapsulated primitive, so do not decompose primitives
                    // based on GroupPrimitive2D, just visit their children. It may be that more
                    // group-like primitives need to be added here, but all primitives with
                    // grouping functionality should be implemented based on the GroupPrimitive2D
                    // class and have their main content accessible as children
                    const primitive2d::GroupPrimitive2D* pGroupPrimitive2D = dynamic_cast< const primitive2d::GroupPrimitive2D* >(&rCandidate);

                    if(pGroupPrimitive2D)
                    {
                        // process group children recursively
                        process(pGroupPrimitive2D->getChildren());
                    }
                    else
                    {
                        // do not process recursively, we *only* want to find existing
                        // ObjectInfoPrimitive2D entries
                    }

                    break;
                }
            }
        }

        ObjectInfoPrimitiveExtractor2D::ObjectInfoPrimitiveExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            mpFound(nullptr)
        {
        }

        ObjectInfoPrimitiveExtractor2D::~ObjectInfoPrimitiveExtractor2D()
        {
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
