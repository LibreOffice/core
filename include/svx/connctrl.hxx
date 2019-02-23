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
#ifndef INCLUDED_SVX_CONNCTRL_HXX
#define INCLUDED_SVX_CONNCTRL_HXX

#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>
#include <tools/wintypes.hxx>
#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/outdev.hxx>
#include <memory>

namespace vcl { class Window; }

class SfxItemSet;
class SdrEdgeObj;
class SdrView;
class SdrPage;

/*************************************************************************
|*
|* SvxXConnectionPreview
|*
\************************************************************************/
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxXConnectionPreview : public weld::CustomWidgetController
{
 friend class SvxConnectionPage;

private:
    MapMode aNewMapMode;
    SdrEdgeObj*         pEdgeObj;
    std::unique_ptr<SdrPage> pSdrPage;
    const SdrView*      pView;

    SVX_DLLPRIVATE void AdaptSize();
    SVX_DLLPRIVATE void SetMapMode(const MapMode& rNewMapMode) { aNewMapMode = rNewMapMode; }
    SVX_DLLPRIVATE const MapMode& GetMapMode() const { return aNewMapMode; }
public:
    SvxXConnectionPreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SvxXConnectionPreview() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    void         SetAttributes( const SfxItemSet& rInAttrs );
    sal_uInt16   GetLineDeltaCount();

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};

#endif // INCLUDED_SVX_CONNCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
