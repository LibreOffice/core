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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_BMPWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_BMPWIN_HXX

#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/window.hxx>

// extended page for graphics
class BmpWindow : public vcl::Window
{
private:
    Graphic     aGraphic;
    BitmapEx    aBmp;

    bool        bHorz : 1;
    bool        bVert : 1;
    bool        bGraphic : 1;
    bool        bLeftAlign : 1;

    virtual void Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect) override;

    virtual Size GetOptimalSize() const override;

public:
    BmpWindow(vcl::Window* pPar, WinBits nStyle);
    virtual ~BmpWindow();
    void MirrorVert(bool bMirror) { bVert = bMirror; Invalidate(); }
    void MirrorHorz(bool bMirror) { bHorz = bMirror; Invalidate(); }
    void SetGraphic(const Graphic& rGrf);
    void SetBitmapEx(const BitmapEx& rGrf);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
