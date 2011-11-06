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



#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrGrafObj;

namespace sdr { namespace event {
    class AsynchGraphicLoadingEvent;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactOfGraphic : public ViewObjectContactOfSdrObj
        {
        private:
            // allow async loading event helper to call tooling methods
            friend class sdr::event::AsynchGraphicLoadingEvent;

            // Member which takes care for the asynch loading events which may be necessary
            // for asynch graphics loading.
            sdr::event::AsynchGraphicLoadingEvent*      mpAsynchLoadEvent;

            // async graphics loading helpers. Only to be used internally or from the
            // event helper class (in .cxx file)
            bool impPrepareGraphicWithAsynchroniousLoading();
            bool impPrepareGraphicWithSynchroniousLoading();
            void doAsynchGraphicLoading();
            void forgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent);

        protected:
            SdrGrafObj& getSdrGrafObj();

            // This method is responsible for creating the graphical visualisation data
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfGraphic(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfGraphic();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX

// eof
