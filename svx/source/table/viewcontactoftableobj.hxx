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



#ifndef _SDR_CONTACT_VIEWCONTACTOFTABLEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFTABLEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace table
    {
        class SdrTableObj;
    }

    namespace contact
    {
        class ViewContactOfTableObj : public ViewContactOfSdrObj
        {
        protected:
            // internal access to SdrTextObj
            ::sdr::table::SdrTableObj& GetTableObj() const
            {
                return (::sdr::table::SdrTableObj&)GetSdrObject();
            }

            // This method is responsible for creating the graphical visualisation data derived ONLY from
            // the model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfTableObj(::sdr::table::SdrTableObj& rTextObj);
            virtual ~ViewContactOfTableObj();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif

// eof
