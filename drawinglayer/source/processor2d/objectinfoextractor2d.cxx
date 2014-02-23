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



using namespace com::sun::star;



namespace drawinglayer
{
    namespace processor2d
    {
        void ObjectInfoPrimitiveExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            if(!mpFound)
            {
                switch(rCandidate.getPrimitive2DID())
                {
                    case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D :
                    {
                        mpFound = dynamic_cast< const primitive2d::ObjectInfoPrimitive2D* >(&rCandidate);
                        break;
                    }
                    default :
                    {
                        // process recursively
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                        break;
                    }
                }
            }
        }

        ObjectInfoPrimitiveExtractor2D::ObjectInfoPrimitiveExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            mpFound(0)
        {
        }

        ObjectInfoPrimitiveExtractor2D::~ObjectInfoPrimitiveExtractor2D()
        {
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer


// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
