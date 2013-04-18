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
            explicit ViewContactOfE3dScene(E3dScene& rScene);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
