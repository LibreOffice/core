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

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <comphelper/sequence.hxx>


using namespace com::sun::star;


namespace drawinglayer::processor2d
{
        void BaseProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& /*rCandidate*/)
        {
        }

        BaseProcessor2D::BaseProcessor2D(const geometry::ViewInformation2D& rViewInformation)
        :   maViewInformation2D(rViewInformation)
        {
        }

        BaseProcessor2D::~BaseProcessor2D()
        {
        }

        void BaseProcessor2D::process(const primitive2d::BasePrimitive2D& rCandidate)
        {
            // use the visitor API to avoid the cost of constructing Primitive2DContainers
            rCandidate.get2DDecomposition(*this, getViewInformation2D());
        }

        // Primitive2DDecompositionVisitor
        void BaseProcessor2D::visit(const primitive2d::Primitive2DReference& rCandidate)
        {
            processBasePrimitive2D(*rCandidate);
        }
        void BaseProcessor2D::visit(const primitive2d::Primitive2DContainer& rContainer)
        {
            process(rContainer);
        }
        void BaseProcessor2D::visit(primitive2d::Primitive2DContainer&& rCandidate)
        {
            process(rCandidate);
        }

        void BaseProcessor2D::process(const primitive2d::Primitive2DContainer& rSource)
        {
            for (const primitive2d::Primitive2DReference& rCandidate : rSource)
                processBasePrimitive2D(*rCandidate);
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
