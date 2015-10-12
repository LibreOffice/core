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

#ifndef INCLUDED_CUI_SOURCE_INC_GRFPAGE_HXX
#define INCLUDED_CUI_SOURCE_INC_GRFPAGE_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/graph.hxx>
#include <sfx2/tabdlg.hxx>

class SvxCropExample : public vcl::Window
{
    Size        aFrameSize;
    Point       aTopLeft, aBottomRight;
    Graphic     aGrf;

public:
    SvxCropExample( vcl::Window* pPar, WinBits nStyle );

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    void SetTop( long nVal )                { aTopLeft.X() = nVal; }
    void SetBottom( long nVal )             { aBottomRight.X() = nVal; }
    void SetLeft( long nVal )               { aTopLeft.Y() = nVal; }
    void SetRight( long nVal)               { aBottomRight.Y() = nVal; }
    void SetFrameSize( const Size& rSz );
    void SetGraphic( const Graphic& rGrf )  { aGrf = rGrf; }
};

class SvxGrfCropPage : public SfxTabPage
{
    friend class VclPtr<SvxGrfCropPage>;
    using Window::CalcZoom;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<VclContainer>   m_pCropFrame;
    VclPtr<RadioButton>    m_pZoomConstRB;
    VclPtr<RadioButton>    m_pSizeConstRB;
    VclPtr<MetricField>    m_pLeftMF;
    VclPtr<MetricField>    m_pRightMF;
    VclPtr<MetricField>    m_pTopMF;
    VclPtr<MetricField>    m_pBottomMF;

    VclPtr<VclContainer>   m_pScaleFrame;
    VclPtr<MetricField>    m_pWidthZoomMF;
    VclPtr<MetricField>    m_pHeightZoomMF;

    VclPtr<VclContainer>   m_pSizeFrame;
    VclPtr<MetricField>    m_pWidthMF;
    VclPtr<MetricField>    m_pHeightMF;

    VclPtr<VclContainer>   m_pOrigSizeGrid;
    VclPtr<FixedText>      m_pOrigSizeFT;
    VclPtr<PushButton>     m_pOrigSizePB;

    // Example
    VclPtr<SvxCropExample> m_pExampleWN;


    Timer           aTimer;
    OUString        aGraphicName;
    Size            aOrigSize;
    Size            aOrigPixelSize;
    Size            aPageSize;
    VclPtr<MetricField>  pLastCropField;
    long            nOldWidth;
    long            nOldHeight;
    bool            bReset;
    bool            bInitialized;
    bool            bSetOrigSize;


    SvxGrfCropPage( vcl::Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxGrfCropPage();
    virtual void dispose() override;

    DECL_LINK( ZoomHdl, MetricField * );
    DECL_LINK( SizeHdl, MetricField * );
    DECL_LINK( CropHdl, const MetricField * );
    DECL_LINK_TYPED( CropLoseFocusHdl, Control&, void );
    DECL_LINK( CropModifyHdl, MetricField * );
    DECL_LINK_TYPED(OrigSizeHdl, Button*, void);
    DECL_LINK_TYPED(Timeout, Timer *, void);

    void            CalcZoom();
    void            CalcMinMaxBorder();
    void            GraphicHasChanged(bool bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet) override;

    Size            GetGrfOrigSize( const Graphic& ) const;
public:
    static VclPtr<SfxTabPage> Create( vcl::Window *pParent, const SfxItemSet *rSet );

    virtual bool FillItemSet( SfxItemSet *rSet ) override;
    virtual void Reset( const SfxItemSet *rSet ) override;
    virtual sfxpg DeactivatePage( SfxItemSet *pSet ) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
