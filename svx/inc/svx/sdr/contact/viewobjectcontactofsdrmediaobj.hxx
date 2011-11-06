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



#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>

namespace avmedia { class MediaItem; }
class Window;

namespace sdr
{
    namespace contact
    {
        class SdrMediaWindow;

        class ViewObjectContactOfSdrMediaObj : public ViewObjectContactOfSdrObj
        {
        public:

            ViewObjectContactOfSdrMediaObj( ObjectContact& rObjectContact,
                                            ViewContact& rViewContact,
                                            const ::avmedia::MediaItem& rMediaItem );
            virtual ~ViewObjectContactOfSdrMediaObj();

        public:

            Window* getWindow() const;

            bool    hasPreferredSize() const;
            Size    getPreferredSize() const;

            void    updateMediaItem( ::avmedia::MediaItem& rItem ) const;
            void    executeMediaItem( const ::avmedia::MediaItem& rItem );

        private:

            ::sdr::contact::SdrMediaWindow* mpMediaWindow;

        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX
