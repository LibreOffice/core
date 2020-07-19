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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

#include <sdr/contact/viewcontactoftextobj.hxx>
#include <svx/svdograf.hxx>

namespace drawinglayer::attribute { class SdrLineFillEffectsTextAttribute; }
class GraphicAttr;

namespace sdr::contact
    {
        class ViewContactOfGraphic final : public ViewContactOfTextObj
        {
        private:
            // helpers for constructing various primitive visualisations in various states
            drawinglayer::primitive2d::Primitive2DContainer createVIP2DSForPresObj(
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const drawinglayer::attribute::SdrLineFillEffectsTextAttribute& rAttribute) const;
            drawinglayer::primitive2d::Primitive2DContainer createVIP2DSForDraft(
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const drawinglayer::attribute::SdrLineFillEffectsTextAttribute& rAttribute) const;

            // Create an Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

        public:
            // access to SdrObject
            const SdrGrafObj& GetGrafObject() const
            {
                return static_cast<const SdrGrafObj&>(GetSdrObject());
            }
            SdrGrafObj& GetGrafObject()
            {
                return static_cast<SdrGrafObj&>(GetSdrObject());
            }

            // basic constructor, destructor
            explicit ViewContactOfGraphic(SdrGrafObj& rGrafObj);
            virtual ~ViewContactOfGraphic() override;

            // helpers for visualisation state
            bool visualisationUsesPresObj() const;
            bool visualisationUsesDraft() const;

        private:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;
        };
} // end of namespace sdr::contact

#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
