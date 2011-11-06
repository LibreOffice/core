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



#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFE3DSCENE_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFE3DSCENE_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfE3dScene : public ViewObjectContactOfSdrObj
        {
        protected:
            // 2d primitive creator
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            // basic constructor.
            ViewObjectContactOfE3dScene(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfE3dScene();

            // process this primitive: Eventually also recursively travel an existing hierarchy,
            // e.g. for group objects, scenes or pages. This method will test geometrical visibility.
            virtual drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFE3DSCENE_HXX

// eof
