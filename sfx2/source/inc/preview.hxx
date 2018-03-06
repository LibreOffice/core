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
#ifndef INCLUDED_SFX2_SOURCE_INC_PREVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_PREVIEW_HXX

#include <tools/link.hxx>

class SfxObjectShell;
class GDIMetaFile;

class SfxPreviewWin_Impl
{
protected:
    std::shared_ptr<GDIMetaFile> xMetaFile;
    std::unique_ptr<weld::DrawingArea> m_xDrawingArea;
public:
    SfxPreviewWin_Impl(weld::DrawingArea* pArea);
    void            SetObjectShell( SfxObjectShell const * pObj );
    DECL_LINK(DoPaint, vcl::RenderContext&, void);
    DECL_LINK(DoResize, const Size& rSize, void);
    void queue_draw() { m_xDrawingArea->queue_draw(); }
    void show() { m_xDrawingArea->show(); }
    void set_size_request(int nWidth, int nHeight) { m_xDrawingArea->set_size_request(nWidth, nHeight); }
    static void ImpPaint(vcl::RenderContext& rRenderContext, GDIMetaFile* pFile);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
