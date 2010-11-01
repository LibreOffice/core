/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _ANCHOROVERLAYOBJECT_HXX
#define _ANCHOROVERLAYOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

class SwView;
class SwRect;
class Point;

namespace sw { namespace sidebarwindows {

enum AnchorState
{
    AS_ALL,
    AS_START,
    AS_END,
    AS_TRI
};

class AnchorOverlayObject: public sdr::overlay::OverlayObjectWithBasePosition
{
    public:
        static AnchorOverlayObject* CreateAnchorOverlayObject( SwView& rDocView,
                                                               const SwRect& aAnchorRect,
                                                               const long& aPageBorder,
                                                               const Point& aLineStart,
                                                               const Point& aLineEnd,
                                                               const Color& aColorAnchor );
        static void DestroyAnchorOverlayObject( AnchorOverlayObject* pAnchor );

        inline const basegfx::B2DPoint& GetSecondPosition() const { return maSecondPosition; }
        const basegfx::B2DPoint& GetThirdPosition() const { return maThirdPosition; }
        const basegfx::B2DPoint& GetFourthPosition() const { return maFourthPosition; }
        const basegfx::B2DPoint& GetFifthPosition() const { return maFifthPosition; }
        const basegfx::B2DPoint& GetSixthPosition() const { return maSixthPosition; }
        const basegfx::B2DPoint& GetSeventhPosition() const { return maSeventhPosition; }

        void SetAllPosition( const basegfx::B2DPoint& rPoint1,
                             const basegfx::B2DPoint& rPoint2,
                             const basegfx::B2DPoint& rPoint3,
                             const basegfx::B2DPoint& rPoint4,
                             const basegfx::B2DPoint& rPoint5,
                             const basegfx::B2DPoint& rPoint6,
                             const basegfx::B2DPoint& rPoint7 );
        void SetTriPosition( const basegfx::B2DPoint& rPoint1,
                             const basegfx::B2DPoint& rPoint2,
                             const basegfx::B2DPoint& rPoint3,
                             const basegfx::B2DPoint& rPoint4,
                             const basegfx::B2DPoint& rPoint5 );
        void SetSixthPosition( const basegfx::B2DPoint& rNew );
        void SetSeventhPosition( const basegfx::B2DPoint& rNew );

        void setLineSolid( const bool bNew );
        inline bool getLineSolid() const { return mbLineSolid; }

        inline void SetHeight( const unsigned long aHeight ) { mHeight = aHeight; };

        bool getShadowedEffect() const { return mbShadowedEffect; }

        void SetAnchorState( const AnchorState aState );
        inline AnchorState GetAnchorState() const { return mAnchorState; }

    protected:
        /*                        6------------7
             1                   /
            /4\ ---------------5
           2 - 3
        */

        basegfx::B2DPoint maSecondPosition;
        basegfx::B2DPoint maThirdPosition;
        basegfx::B2DPoint maFourthPosition;
        basegfx::B2DPoint maFifthPosition;
        basegfx::B2DPoint maSixthPosition;
        basegfx::B2DPoint maSeventhPosition;

        // helpers to fill and reset geometry
        void implEnsureGeometry();
        void implResetGeometry();

        // geometry creation for OverlayObject
        virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

    private:
        // object's geometry
        basegfx::B2DPolygon maTriangle;
        basegfx::B2DPolygon maLine;
        basegfx::B2DPolygon maLineTop;
        unsigned long mHeight;
        AnchorState mAnchorState;

        bool mbShadowedEffect : 1;
        bool mbLineSolid : 1;

        AnchorOverlayObject( const basegfx::B2DPoint& rBasePos,
                             const basegfx::B2DPoint& rSecondPos,
                             const basegfx::B2DPoint& rThirdPos,
                             const basegfx::B2DPoint& rFourthPos,
                             const basegfx::B2DPoint& rFifthPos,
                             const basegfx::B2DPoint& rSixthPos,
                             const basegfx::B2DPoint& rSeventhPos,
                             const Color aBaseColor,
                             const bool bShadowedEffect,
                             const bool bLineSolid );
        virtual ~AnchorOverlayObject();
};

} } // end of namespace sw::annotation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
