/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_ANCHOROVERLAYOBJECT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_ANCHOROVERLAYOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

class SwView;
class SwRect;
class Point;

namespace sw { namespace sidebarwindows {

enum class AnchorState
{
    All,
    End,
    Tri
};

class AnchorOverlayObject final : public sdr::overlay::OverlayObjectWithBasePosition
{
    public:
        static AnchorOverlayObject* CreateAnchorOverlayObject( SwView const & rDocView,
                                                               const SwRect& aAnchorRect,
                                                               long aPageBorder,
                                                               const Point& aLineStart,
                                                               const Point& aLineEnd,
                                                               const Color& aColorAnchor );

        const basegfx::B2DPoint& GetSecondPosition() const { return maSecondPosition; }
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
        bool getLineSolid() const { return mbLineSolid; }

        void SetAnchorState( const AnchorState aState );
        AnchorState GetAnchorState() const { return mAnchorState; }

    private:
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
        virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        // object's geometry
        basegfx::B2DPolygon maTriangle;
        basegfx::B2DPolygon maLine;
        basegfx::B2DPolygon maLineTop;
        AnchorState mAnchorState;

        bool mbLineSolid : 1;

        AnchorOverlayObject( const basegfx::B2DPoint& rBasePos,
                             const basegfx::B2DPoint& rSecondPos,
                             const basegfx::B2DPoint& rThirdPos,
                             const basegfx::B2DPoint& rFourthPos,
                             const basegfx::B2DPoint& rFifthPos,
                             const basegfx::B2DPoint& rSixthPos,
                             const basegfx::B2DPoint& rSeventhPos,
                             const Color& rBaseColor );
    public:
        virtual ~AnchorOverlayObject() override;
};

} } // end of namespace sw::annotation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
