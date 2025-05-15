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

#pragma once

#include <vcl/timer.hxx>
#include <address.hxx>
#include <rtl/ref.hxx>
#include <tools/gen.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>

class ScGridWindow;
class SdrCaptionObj;

class ScNoteOverlay : public Timer
{
    ScGridWindow&                   mrScGridWindow;
    ScAddress                       maDocPos;
    OUString                        maUserText;
    sdr::overlay::OverlayObjectList maNoteOverlayGroup;
    rtl::Reference<SdrCaptionObj>   mxObject;
    drawinglayer::primitive2d::Primitive2DContainer maSequence;
    bool                            mbLeft;
    bool                            mbKeyboard;

    tools::Rectangle calculateVisibleRectangle();
    const drawinglayer::primitive2d::Primitive2DContainer& getOrCreatePrimitive2DSequence();
    void createAdditionalRepresentations();
    void createOverlaySubContent(
        ScGridWindow* pTarget,
        const basegfx::B2DHomMatrix& rTransformToPixels,
        const basegfx::B2DPoint& rTopLeft);

public:
    ScNoteOverlay(
        ScGridWindow& rScGridWindow,
        ScAddress& aPos,
        OUString aUser,
        bool bLeftEdge,
        bool bForce,
        bool bKeyboard);
    ~ScNoteOverlay();

    virtual void Invoke() override;
    bool IsByKeyboard() const { return mbKeyboard; }
    const ScAddress& GetDocPos() const { return maDocPos; }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
