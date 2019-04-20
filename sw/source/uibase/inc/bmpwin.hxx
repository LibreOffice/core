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

#include <vcl/customweld.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>

// extended page for graphics
class BmpWindow : public weld::CustomWidgetController
{
private:
    Graphic     aGraphic;
    BitmapEx    aBmp;

    bool        bHorz : 1;
    bool        bVert : 1;
    bool        bGraphic : 1;

    virtual void Paint(vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect) override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

public:
    BmpWindow();
    virtual ~BmpWindow() override;
    void MirrorVert(bool bMirror) { bVert = bMirror; Invalidate(); }
    void MirrorHorz(bool bMirror) { bHorz = bMirror; Invalidate(); }
    void SetGraphic(const Graphic& rGrf);
    void SetBitmapEx(const BitmapEx& rGrf);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
