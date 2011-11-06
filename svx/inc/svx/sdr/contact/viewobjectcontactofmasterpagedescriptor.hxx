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



#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFMASTERPAGEDESCRIPTOR_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr {
    class MasterPageDescriptor;
}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactOfMasterPageDescriptor : public ViewObjectContact
        {
        public:
            ViewObjectContactOfMasterPageDescriptor(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfMasterPageDescriptor();

            // access to MasterPageDescriptor
            sdr::MasterPageDescriptor& GetMasterPageDescriptor() const;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
            virtual drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX

// eof
