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

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_SHADOWOVERLAYOBJECT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_SHADOWOVERLAYOBJECT_HXX

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
        virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

private:
        basegfx::B2DPoint maSecondPosition;
        ShadowState mShadowState;

        ShadowOverlayObject( const basegfx::B2DPoint& rBasePos,
                             const basegfx::B2DPoint& rSecondPosition,
                             Color aBaseColor );

public:
        virtual ~ShadowOverlayObject() override;

        void SetShadowState(ShadowState aState);
        ShadowState GetShadowState() {return mShadowState;}

        void SetPosition( const basegfx::B2DPoint& rPoint1,
                          const basegfx::B2DPoint& rPoint2 );

        static ShadowOverlayObject* CreateShadowOverlayObject( SwView const & rDocView );
};

} } // end of namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
