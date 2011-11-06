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



#ifndef _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPageObj;
class SdrPage;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfPageObj : public ViewContactOfSdrObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // create graphical visualisation data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

            // Access to referenced page
            const SdrPage* GetReferencedPage() const;

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfPageObj(SdrPageObj& rPageObj);
            virtual ~ViewContactOfPageObj();

            // #WIP# React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // access to SdrObject
            SdrPageObj& GetPageObj() const
            {
                return (SdrPageObj&)GetSdrObject();
            }
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

// eof
