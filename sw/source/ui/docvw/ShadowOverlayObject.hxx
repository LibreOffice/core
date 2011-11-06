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



#ifndef _SHADOWOVERLAYOBJECT_HXX
#define _SHADOWOVERLAYOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

class SwView;

namespace sw { namespace sidebarwindows {

enum ShadowState
{
    SS_NORMAL,
    SS_VIEW,
    SS_EDIT
};

class ShadowOverlayObject: public sdr::overlay::OverlayObjectWithBasePosition
{
    protected:
        // geometry creation for OverlayObject
        virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

    private:
        basegfx::B2DPoint maSecondPosition;
        ShadowState mShadowState;

        ShadowOverlayObject( const basegfx::B2DPoint& rBasePos,
                             const basegfx::B2DPoint& rSecondPosition,
                             Color aBaseColor,
                             ShadowState aState );
        virtual ~ShadowOverlayObject();

    public:
        void SetShadowState(ShadowState aState);
        inline ShadowState GetShadowState() {return mShadowState;}

        inline const basegfx::B2DPoint& GetSecondPosition() const { return maSecondPosition; }
        void SetSecondPosition( const basegfx::B2DPoint& rNew );

        void SetPosition( const basegfx::B2DPoint& rPoint1,
                          const basegfx::B2DPoint& rPoint2 );

        static ShadowOverlayObject* CreateShadowOverlayObject( SwView& rDocView );
        static void DestroyShadowOverlayObject( ShadowOverlayObject* pShadow );
};

} } // end of namespace sw::sidebarwindows

#endif
