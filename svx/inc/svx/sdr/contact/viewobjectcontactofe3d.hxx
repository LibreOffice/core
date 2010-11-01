/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr { namespace contact {
    class ViewObjectContactOfE3dScene;
}}

//////////////////////////////////////////////////////////////////////////////

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
            drawinglayer::primitive3d::Primitive3DSequence          mxPrimitive3DSequence;

            // This method is responsible for creating the graphical visualisation data which is
            // stored/cached in the local primitive. Default gets view-independent Primitive3D
            // from the ViewContact using ViewContact::getViewIndependentPrimitive3DSequence(), takes care of
            // visibility and ghosted.
            // This method will not handle included hierarchies and not check geometric visibility.
            drawinglayer::primitive3d::Primitive3DSequence createPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const;

            // also overload the 2d method to deliver a 2d object with embedd3d 3d and the 3d transformation which is able to
            // answer the get2DRange question accordingly
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfE3d(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfE3d();

            // access to the local primitive sequence. This will ensure that the list is
            // current in comparing the local list content with a fresh created incarnation
            // This method will not handle included hierarchies or visibility.
            drawinglayer::primitive3d::Primitive3DSequence getPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFE3D_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
