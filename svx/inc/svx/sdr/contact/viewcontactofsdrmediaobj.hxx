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



#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrMediaObj;
namespace avmedia { class MediaItem; }

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfSdrMediaObj : public ViewContactOfSdrObj
        {
            friend class ViewObjectContactOfSdrMediaObj;

        public:

            // basic constructor, used from SdrObject.
            ViewContactOfSdrMediaObj( SdrMediaObj& rMediaObj );
            virtual ~ViewContactOfSdrMediaObj();

        public:

            // access to SdrMediaObj
            SdrMediaObj& GetSdrMediaObj() const
            {
                return (SdrMediaObj&)GetSdrObject();
            }

            bool    hasPreferredSize() const;
            Size    getPreferredSize() const;

            void    updateMediaItem( ::avmedia::MediaItem& rItem ) const;
            void    executeMediaItem( const ::avmedia::MediaItem& rItem );

        protected:

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // get notified if some properties have changed
            virtual void mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX
