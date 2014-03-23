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



#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrOle2Obj;
class Graphic;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrOle2Obj : public ViewContactOfSdrRectObj
        {
        private:
            // #123539# allow local buffering of chart data (if chart)
            drawinglayer::primitive2d::Primitive2DReference mxChartContent;

        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // access to SdrOle2Obj
            SdrOle2Obj& GetOle2Obj() const
            {
                return (SdrOle2Obj&)GetSdrObject();
            }

            // basic constructor, used from SdrObject.
            ViewContactOfSdrOle2Obj(SdrOle2Obj& rOle2Obj);
            virtual ~ViewContactOfSdrOle2Obj();

            // helper for creating a OLE sequence for this object. It takes care od attributes, needed
            // scaling (e.g. for EmptyPresObj's), the correct graphic and other stuff. It is used from
            // createViewIndependentPrimitive2DSequence with false, and with evtl. HighContrast true
            // from the VOC which knows that
            drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceWithParameters(bool bHighContrast) const;

            // #123539# get rid of buffered chart content (if there) on change
            virtual void ActionChanged();

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data, just wraps to call createPrimitive2DSequenceWithParameters(false)
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

// eof
