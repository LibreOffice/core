/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:  $
 *
 * $Revision:  $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
