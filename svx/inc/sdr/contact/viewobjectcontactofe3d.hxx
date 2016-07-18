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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

namespace sdr { namespace contact {
    class ViewObjectContactOfE3dScene;
}}

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactOfE3d : public ViewObjectContactOfSdrObj
        {
        protected:
            // Primitive3D sequence of the ViewContact. This contains all necessary information
            // for the graphical visualisation and needs to be supported by all VCs which
            // can be visualized.
            drawinglayer::primitive3d::Primitive3DContainer          mxPrimitive3DContainer;

            // This method is responsible for creating the graphical visualisation data which is
            // stored/cached in the local primitive. Default gets view-independent Primitive3D
            // from the ViewContact using ViewContact::getViewIndependentPrimitive3DContainer(), takes care of
            // visibility and ghosted.
            // This method will not handle included hierarchies and not check geometric visibility.
            drawinglayer::primitive3d::Primitive3DContainer createPrimitive3DContainer(const DisplayInfo& rDisplayInfo) const;

            // also override the 2d method to deliver a 2d object with embedd3d 3d and the 3d transformation which is able to
            // answer the get2DRange question accordingly
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            ViewObjectContactOfE3d(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfE3d();

            // access to the local primitive sequence. This will ensure that the list is
            // current in comparing the local list content with a fresh created incarnation
            // This method will not handle included hierarchies or visibility.
            drawinglayer::primitive3d::Primitive3DContainer const & getPrimitive3DContainer(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
