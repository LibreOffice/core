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

#ifndef INCLUDED_CUI_SOURCE_INC_CUIGRFFLT_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIGRFFLT_HXX

#include <tools/helpers.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/timer.hxx>
#include <svx/graphctl.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/rectenum.hxx>

class CuiGraphicPreviewWindow : public weld::CustomWidgetController
{
private:
    const Graphic* mpOrigGraphic;
    Size maOrigGraphicSizePixel;
    Size maOutputSizePixel;
    Link<LinkParamNone*,void>    maModifyHdl;
    Graphic   maScaledOrig;
    Graphic   maPreview;
    double    mfScaleX;
    double    mfScaleY;

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;
    virtual void Resize() override;

    void ScaleImageToFit();

public:
    CuiGraphicPreviewWindow();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void init(const Graphic* pOrigGraphic, const Link<LinkParamNone*,void>& rLink)
    {
        mpOrigGraphic = pOrigGraphic;
        maModifyHdl = rLink;
        maOrigGraphicSizePixel = GetDrawingArea()->get_ref_device().LogicToPixel(mpOrigGraphic->GetPrefSize(),
                                                                                 mpOrigGraphic->GetPrefMapMode());
        ScaleImageToFit();
    }

    void            SetPreview(const Graphic& rGraphic);
    const Graphic&  GetScaledOriginal() const { return maScaledOrig; }
    double          GetScaleX() const { return mfScaleX; }
    double          GetScaleY() const { return mfScaleY; }
    const Size&     GetGraphicSizePixel() const { return maOrigGraphicSizePixel; }
};

class GraphicFilterDialog : public weld::GenericDialogController
{
private:

    Timer           maTimer;
    Link<LinkParamNone*,void> maModifyHdl;
    bool            bIsBitmap;

    DECL_LINK( ImplPreviewTimeoutHdl, Timer *, void );
    DECL_LINK( ImplModifyHdl, LinkParamNone*, void);

    CuiGraphicPreviewWindow  maPreview;
    std::unique_ptr<weld::CustomWeld> mxPreview;

protected:
    const Link<LinkParamNone*,void>&   GetModifyHdl() const { return maModifyHdl; }
    const Size& GetGraphicSizePixel() const { return maPreview.GetGraphicSizePixel(); }

public:

    GraphicFilterDialog(weld::Window* pParent, const OUString& rUIXMLDescription, const OString& rID, const Graphic& rGraphic);
    virtual Graphic GetFilteredGraphic(const Graphic& rGraphic, double fScaleX, double fScaleY) = 0;
};

class GraphicFilterSmooth : public GraphicFilterDialog
{
private:
    std::unique_ptr<weld::SpinButton> mxMtrRadius;
    DECL_LINK(EditModifyHdl, weld::SpinButton&, void);

public:

    GraphicFilterSmooth(weld::Window* pParent, const Graphic& rGraphic, double nRadius);
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

class GraphicFilterMosaic : public GraphicFilterDialog
{
private:
    std::unique_ptr<weld::MetricSpinButton> mxMtrWidth;
    std::unique_ptr<weld::MetricSpinButton> mxMtrHeight;
    std::unique_ptr<weld::CheckButton> mxCbxEdges;
    DECL_LINK(CheckBoxModifyHdl, weld::ToggleButton&, void);
    DECL_LINK(EditModifyHdl, weld::MetricSpinButton&, void);
public:

    GraphicFilterMosaic(weld::Window* pParent, const Graphic& rGraphic,
        sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, bool bEnhanceEdges);

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    bool            IsEnhanceEdges() const { return mxCbxEdges->get_active(); }
};

class GraphicFilterSolarize : public GraphicFilterDialog
{
private:
    std::unique_ptr<weld::MetricSpinButton> mxMtrThreshold;
    std::unique_ptr<weld::CheckButton> mxCbxInvert;
    DECL_LINK(CheckBoxModifyHdl, weld::ToggleButton&, void);
    DECL_LINK(EditModifyHdl, weld::MetricSpinButton&, void);

public:
    GraphicFilterSolarize(weld::Window* pParent, const Graphic& rGraphic,
                          sal_uInt8 nGreyThreshold, bool bInvert);
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    bool            IsInvert() const { return mxCbxInvert->get_active(); }
};

class GraphicFilterSepia : public GraphicFilterDialog
{
private:
    std::unique_ptr<weld::MetricSpinButton> mxMtrSepia;
    DECL_LINK(EditModifyHdl, weld::MetricSpinButton&, void);
public:
    GraphicFilterSepia(weld::Window* pParent, const Graphic& rGraphic,
                       sal_uInt16 nSepiaPercent);
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

class GraphicFilterPoster : public GraphicFilterDialog
{
private:
    std::unique_ptr<weld::SpinButton> mxNumPoster;
    DECL_LINK(EditModifyHdl, weld::SpinButton&, void);
public:
    GraphicFilterPoster(weld::Window* pParent, const Graphic& rGraphic,
                        sal_uInt16 nPosterColorCount);
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

class EmbossControl : public SvxRectCtl
{
private:
    Link<LinkParamNone*, void> maModifyHdl;
    virtual bool MouseButtonDown( const MouseEvent& rEvt ) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
public:
    EmbossControl()
        : SvxRectCtl(nullptr)
    {
    }

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rHdl ) { maModifyHdl = rHdl; }
};

class GraphicFilterEmboss : public GraphicFilterDialog
{
private:
    EmbossControl  maCtlLight;
    std::unique_ptr<weld::CustomWeld> mxCtlLight;
public:
    GraphicFilterEmboss(weld::Window* pParent, const Graphic& rGraphic,
                        RectPoint eLightSource);
    virtual ~GraphicFilterEmboss() override;

    virtual Graphic GetFilteredGraphic(const Graphic& rGraphic, double fScaleX, double fScaleY) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
