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



#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRRECTOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDRRECTOBJ_HXX

#include <svx/sdr/contact/viewcontactoftextobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrRectObj;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrRectObj : public ViewContactOfTextObj
        {
        protected:
            // internal access to SdrRectObj
            SdrRectObj& GetRectObj() const
            {
                return (SdrRectObj&)GetSdrObject();
            }

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfSdrRectObj(SdrRectObj& rTextObj);
            virtual ~ViewContactOfSdrRectObj();

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDRRECTOBJ_HXX

// eof
