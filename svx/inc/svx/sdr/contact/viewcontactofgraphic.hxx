/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute) const;
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
            ViewContactOfGraphic(SdrGrafObj& rGrafObj);
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

// eof
