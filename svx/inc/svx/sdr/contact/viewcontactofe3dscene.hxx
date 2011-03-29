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

#ifndef _SDR_CONTACT_VIEWCONTACTOFE3DSCENE_HXX
#define _SDR_CONTACT_VIEWCONTACTOFE3DSCENE_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class E3dScene;

namespace basegfx {
    class B3DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfE3dScene : public ViewContactOfSdrObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something. Default is to create
            // a standard ViewObjectContact containing the given ObjectContact and *this
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfE3dScene(E3dScene& rScene);

            // access to SdrObject
            E3dScene& GetE3dScene() const
            {
                return (E3dScene&)GetSdrObject();
            }

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // access to ViewInformation3D and ObjectTransformation
            const drawinglayer::geometry::ViewInformation3D& getViewInformation3D(const ::basegfx::B3DRange& rContentRange) const;
            const drawinglayer::geometry::ViewInformation3D& getViewInformation3D() const;
            const basegfx::B2DHomMatrix& getObjectTransformation() const;

            // attribute providers
            const drawinglayer::attribute::SdrSceneAttribute& getSdrSceneAttribute() const;
            const drawinglayer::attribute::SdrLightingAttribute& getSdrLightingAttribute() const;

            // scene primitive creators. If pLayerVisibility is given, a visibility test with the LayerID and the
            // given SetOfByte is done.
            drawinglayer::primitive2d::Primitive2DSequence createScenePrimitive2DSequence(const SetOfByte* pLayerVisibility) const;

            // helpers to get the sequence of all contained 3D primitives and it's range,
            // regardless of layer or visibility constraints and using a neutral ViewInformation3D
            drawinglayer::primitive3d::Primitive3DSequence getAllPrimitive3DSequence() const;
            basegfx::B3DRange getAllContentRange3D() const;

        protected:
            // the 3d transformation stack
            drawinglayer::geometry::ViewInformation3D           maViewInformation3D;

            // the object transformation
            basegfx::B2DHomMatrix                               maObjectTransformation;

            // attributes
            drawinglayer::attribute::SdrSceneAttribute          maSdrSceneAttribute;
            drawinglayer::attribute::SdrLightingAttribute       maSdrLightingAttribute;

            // create methods for ViewInformation3D and ObjectTransformation
            void createViewInformation3D(const ::basegfx::B3DRange& rContentRange);
            void createObjectTransformation();

            // attribute creators
            void createSdrSceneAttribute();
            void createSdrLightingAttribute();

            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFE3DSCENE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
