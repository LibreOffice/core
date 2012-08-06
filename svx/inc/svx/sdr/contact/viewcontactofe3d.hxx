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

#ifndef _SDR_CONTACT_VIEWCONTACTOFE3D_HXX
#define _SDR_CONTACT_VIEWCONTACTOFE3D_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr { namespace contact {
    class ViewContactOfE3dScene;
}}

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
}}

namespace basegfx {
    class BColor;
    class B3DHomMatrix;
}

class E3dObject;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfE3d : public ViewContactOfSdrObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // basic constructor, used from E3dObject.
            explicit ViewContactOfE3d(E3dObject& rObj);
            virtual ~ViewContactOfE3d();

            // access to E3dObject
            E3dObject& GetE3dObject() const
            {
                return (E3dObject&)GetSdrObject();
            }

            // public helpers
            drawinglayer::primitive2d::Primitive2DSequence impCreateWithGivenPrimitive3DSequence(
                const drawinglayer::primitive3d::Primitive3DSequence& rxContent3D) const;

            //////////////////////////////////////////////////////////////////////////////
            // primitive stuff

        protected:
            // Primitive3DSequence of the ViewContact. This contains all necessary information
            // for the graphical visualisation and needs to be supported by all 3D VCs which
            // can be visualized. It does NOT contain the object transformation to be able to
            // buffer for all possible usages
            drawinglayer::primitive3d::Primitive3DSequence              mxViewIndependentPrimitive3DSequence;

            // This method is responsible for creating the graphical visualisation data which is
            // stored in mxViewIndependentPrimitive3DSequence, but without object transformation
            virtual drawinglayer::primitive3d::Primitive3DSequence createViewIndependentPrimitive3DSequence() const = 0;

            // This method is responsible for creating the graphical visualisation data derived ONLY from
            // the model data. The default implementation will try to create a 3D to 2D embedding (will work
            // when scene is known) which can then be used for 2D processing concerning ranges
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // access to the local primitive without the object's local 3D transform. This is e.g. needed
            // to get the not-yet transformed BoundVolume for e.g. interactions
            drawinglayer::primitive3d::Primitive3DSequence getVIP3DSWithoutObjectTransform() const;

            // access to the local primitive. This will ensure that the list is
            // current in comparing the local list content with a fresh created incarnation. It will
            // use getVIP3DSWithoutObjectTransform and embed to 3d transform primitive when object's
            // local 3d transform is used
            drawinglayer::primitive3d::Primitive3DSequence getViewIndependentPrimitive3DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
