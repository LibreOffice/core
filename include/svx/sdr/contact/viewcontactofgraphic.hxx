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

#ifndef _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#define _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontactoftextobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrGrafObj;
namespace drawinglayer { namespace attribute { class SdrLineFillShadowTextAttribute; }}
class GraphicAttr;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfGraphic : public ViewContactOfTextObj
        {
        private:
            // helpers for constructing various primitive visualisations in various states
            drawinglayer::primitive2d::Primitive2DSequence createVIP2DSForPresObj(
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute,
                const GraphicAttr& rLocalGrafInfo) const;
            drawinglayer::primitive2d::Primitive2DSequence createVIP2DSForDraft(
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute) const;

        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // access to SdrObject
            SdrGrafObj& GetGrafObject() const
            {
                return ((SdrGrafObj&)GetSdrObject());
            }

            // basic constructor, destructor
            explicit ViewContactOfGraphic(SdrGrafObj& rGrafObj);
            virtual ~ViewContactOfGraphic();

            // #i102380#
            void flushGraphicObjects();

            // helpers for viusualisation state
            bool visualisationUsesPresObj() const;
            bool visualisationUsesDraft() const;

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
