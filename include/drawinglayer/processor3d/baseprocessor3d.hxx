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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>



namespace drawinglayer
{
    namespace processor3d
    {
        /** BaseProcessor3D class

            Baseclass for all C++ implementations of instances which process
            primitives.

            Please have a look at baseprocessor2d.hxx for more comments.
         */
        class DRAWINGLAYER_DLLPUBLIC BaseProcessor3D
        {
        private:
            geometry::ViewInformation3D                     maViewInformation3D;

        protected:
            void updateViewInformation(const geometry::ViewInformation3D& rViewInformation3D)
            {
                maViewInformation3D = rViewInformation3D;
            }

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
                Default implementation does nothing
             */
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            explicit BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~BaseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer



#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
