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

#include <drawinglayer/processor3d/baseprocessor3d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace processor3d
    {
        void BaseProcessor3D::processBasePrimitive3D(const primitive3d::BasePrimitive3D& /*rCandidate*/)
        {
        }

        BaseProcessor3D::BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation)
        :   maViewInformation3D(rViewInformation)
        {
        }

        BaseProcessor3D::~BaseProcessor3D()
        {
        }

        void BaseProcessor3D::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive3D implementation
                        const primitive3d::BasePrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            processBasePrimitive3D(*pBasePrimitive);
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue >& rViewParameters(getViewInformation3D().getViewInformationSequence());
                            process(xReference->getDecomposition(rViewParameters));
                        }
                    }
                }
            }
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
