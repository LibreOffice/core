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

class SvxCropExample : public Window
{
    Size        aFrameSize;
    Point       aTopLeft, aBottomRight;
    Graphic     aGrf;

public:
    SvxCropExample( Window* pPar, WinBits nStyle );

    virtual void Paint( const Rectangle& rRect );
    virtual void Resize();
    virtual Size GetOptimalSize() const;

    void SetTopLeft( const Point& rNew )    { aTopLeft = rNew; }
    void SetTop( long nVal )                { aTopLeft.X() = nVal; }
    void SetBottom( long nVal )             { aBottomRight.X() = nVal; }
    void SetLeft( long nVal )               { aTopLeft.Y() = nVal; }
    void SetRight( long nVal)               { aBottomRight.Y() = nVal; }
    void SetBottomRight(const Point& rNew ) { aBottomRight = rNew; }
    void SetFrameSize( const Size& rSz );
    void SetGraphic( const Graphic& rGrf )  { aGrf = rGrf; }
    const Graphic& GetGraphic() const       { return aGrf; }
};

class SvxGrfCropPage : public SfxTabPage
{
    using Window::CalcZoom;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclContainer*   m_pCropFrame;
    RadioButton*    m_pZoomConstRB;
    RadioButton*    m_pSizeConstRB;
    MetricField*    m_pLeftMF;
    MetricField*    m_pRightMF;
    MetricField*    m_pTopMF;
    MetricField*    m_pBottomMF;

    VclContainer*   m_pScaleFrame;
    MetricField*    m_pWidthZoomMF;
    MetricField*    m_pHeightZoomMF;

    VclContainer*   m_pSizeFrame;
    MetricField*    m_pWidthMF;
    MetricField*    m_pHeightMF;

    VclContainer*   m_pOrigSizeGrid;
    FixedText*      m_pOrigSizeFT;
    PushButton*     m_pOrigSizePB;

    // Example
    SvxCropExample* m_pExampleWN;


    Timer           aTimer;
    OUString        aGraphicName;
    Size            aOrigSize;
    Size            aOrigPixelSize;
    Size            aPageSize;
    const MetricField*  pLastCropField;
    long            nOldWidth;
    long            nOldHeight;
    sal_Bool            bReset;
    sal_Bool            bInitialized;
    sal_Bool            bSetOrigSize;


    SvxGrfCropPage( Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxGrfCropPage();

    DECL_LINK( ZoomHdl, MetricField * );
    DECL_LINK( SizeHdl, MetricField * );
    DECL_LINK( CropHdl, const MetricField * );
    DECL_LINK( CropLoseFocusHdl, MetricField * );
    DECL_LINK( CropModifyHdl, MetricField * );
    DECL_LINK(OrigSizeHdl, void *);
    DECL_LINK(Timeout, void *);

    void            CalcZoom();
    void            CalcMinMaxBorder();
    void            GraphicHasChanged(sal_Bool bFound);
    virtual void    ActivatePage(const SfxItemSet& rSet);

    Size            GetGrfOrigSize( const Graphic& ) const;
public:
    static SfxTabPage *Create( Window *pParent, const SfxItemSet &rSet );

    virtual bool FillItemSet( SfxItemSet &rSet );
    virtual void Reset( const SfxItemSet &rSet );
    virtual int DeactivatePage( SfxItemSet *pSet );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
