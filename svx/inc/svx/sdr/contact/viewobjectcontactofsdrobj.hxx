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



#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfSdrObj : public ViewObjectContact
        {
        protected:
            const SdrObject& getSdrObject() const;

        public:
            ViewObjectContactOfSdrObj(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfSdrObj();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

// eof
