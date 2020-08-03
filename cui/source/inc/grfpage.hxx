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
#include <vcl/graph.hxx>
#include <sfx2/tabdlg.hxx>

class SvxCropExample : public weld::CustomWidgetController
{
    MapMode     m_aMapMode;
    Size        m_aFrameSize;
    Point       m_aTopLeft, m_aBottomRight;
    Graphic     m_aGrf;

public:
    SvxCropExample();

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    void SetTop( long nVal )                { m_aTopLeft.setX(nVal); }
    void SetBottom( long nVal )             { m_aBottomRight.setX(nVal); }
    void SetLeft( long nVal )               { m_aTopLeft.setY(nVal); }
    void SetRight( long nVal)               { m_aBottomRight.setY(nVal); }
    void SetFrameSize( const Size& rSz );
    void SetGraphic( const Graphic& rGrf )  { m_aGrf = rGrf; }
};

class SvxGrfCropPage : public SfxTabPage
{
    friend class VclPtr<SvxGrfCropPage>;

    OUString        aGraphicName;
    Size            aOrigSize;
    Size            aOrigPixelSize;
    Size            aPageSize;
    long            nOldWidth;
    long            nOldHeight;
    bool            bSetOrigSize;

    SvxCropExample m_aExampleWN;

    std::unique_ptr<weld::Widget> m_xCropFrame;
    std::unique_ptr<weld::RadioButton> m_xZoomConstRB;
    std::unique_ptr<weld::RadioButton> m_xSizeConstRB;
    std::unique_ptr<weld::MetricSpinButton> m_xLeftMF;
    std::unique_ptr<weld::MetricSpinButton> m_xRightMF;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMF;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMF;

    std::unique_ptr<weld::Widget> m_xScaleFrame;
    std::unique_ptr<weld::MetricSpinButton> m_xWidthZoomMF;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightZoomMF;

    std::unique_ptr<weld::Widget> m_xSizeFrame;
    std::unique_ptr<weld::MetricSpinButton> m_xWidthMF;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightMF;

    std::unique_ptr<weld::Widget> m_xOrigSizeGrid;
    std::unique_ptr<weld::Label> m_xOrigSizeFT;
    std::unique_ptr<weld::Button> m_xOrigSizePB;

    // Example
    std::unique_ptr<weld::CustomWeld> m_xExampleWN;

    DECL_LINK(ZoomHdl, weld::MetricSpinButton&, void);
    DECL_LINK(SizeHdl, weld::MetricSpinButton&, void);
    DECL_LINK(CropModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(OrigSizeHdl, weld::Button&, void);

    void            CalcZoom();
    void            CalcMinMaxBorder();
    void            GraphicHasChanged(bool bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet) override;

    static Size     GetGrfOrigSize(const Graphic&);
public:
    SvxGrfCropPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet );
    virtual ~SvxGrfCropPage() override;

    virtual bool FillItemSet( SfxItemSet *rSet ) override;
    virtual void Reset( const SfxItemSet *rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet *pSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
