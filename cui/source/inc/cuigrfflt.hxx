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

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/timer.hxx>
#include <vcl/dialog.hxx>
#include <vcl/group.hxx>
#include <tools/helpers.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/graphctl.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/rectenum.hxx>

class GraphicPreviewWindow : public Control
{
private:
    const Graphic* mpOrigGraphic;
    Link<>    maModifyHdl;
    Graphic   maScaledOrig;
    Graphic   maPreview;
    double    mfScaleX;
    double    mfScaleY;

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    void ScaleImageToFit();

public:

    GraphicPreviewWindow(vcl::Window* pParent, WinBits nStyle);
    void init(const Graphic* pOrigGraphic, const Link<>& rLink)
    {
        mpOrigGraphic = pOrigGraphic;
        maModifyHdl = rLink;
        ScaleImageToFit();
    }

    void            SetPreview(const Graphic& rGraphic);
    const Graphic&  GetScaledOriginal() const { return maScaledOrig; }
    double          GetScaleX() const { return mfScaleX; }
    double          GetScaleY() const { return mfScaleY; }
};

class GraphicFilterDialog : public ModalDialog
{
private:

    Timer           maTimer;
    Link<>          maModifyHdl;
    Size            maSizePixel;
    bool            bIsBitmap;

    DECL_LINK_TYPED(ImplPreviewTimeoutHdl, Timer *, void);
    DECL_LINK( ImplModifyHdl, void* );

protected:
    VclPtr<GraphicPreviewWindow>  mpPreview;

    const Link<>&   GetModifyHdl() const { return maModifyHdl; }
    const Size&     GetGraphicSizePixel() const { return maSizePixel; }

public:

    GraphicFilterDialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, const Graphic& rGraphic);
    virtual ~GraphicFilterDialog();
    virtual void dispose() override;

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};

class GraphicFilterSmooth : public GraphicFilterDialog
{
private:
    VclPtr<NumericField>   mpMtrRadius;
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);

public:

    GraphicFilterSmooth( vcl::Window* pParent, const Graphic& rGraphic, double nRadius);
    virtual ~GraphicFilterSmooth();
    virtual void dispose() override;

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    double          GetRadius() const { return mpMtrRadius->GetValue() / 10.0; }
};

class GraphicFilterMosaic : public GraphicFilterDialog
{
private:
    VclPtr<MetricField>    mpMtrWidth;
    VclPtr<MetricField>    mpMtrHeight;
    VclPtr<CheckBox>       mpCbxEdges;
    DECL_LINK_TYPED(CheckBoxModifyHdl, CheckBox&, void);
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);
public:

    GraphicFilterMosaic(vcl::Window* pParent, const Graphic& rGraphic,
        sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, bool bEnhanceEdges);
    virtual ~GraphicFilterMosaic();
    virtual void dispose() override;

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    long            GetTileWidth() const { return static_cast<long>(mpMtrWidth->GetValue()); }
    long            GetTileHeight() const { return static_cast<long>(mpMtrHeight->GetValue()); }
    bool            IsEnhanceEdges() const { return mpCbxEdges->IsChecked(); }
};

class GraphicFilterSolarize : public GraphicFilterDialog
{
private:
    VclPtr<MetricField>    mpMtrThreshold;
    VclPtr<CheckBox>       mpCbxInvert;
    DECL_LINK_TYPED(CheckBoxModifyHdl, CheckBox&, void);
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);

public:

    GraphicFilterSolarize( vcl::Window* pParent, const Graphic& rGraphic,
                                           sal_uInt8 nGreyThreshold, bool bInvert );
    virtual ~GraphicFilterSolarize();
    virtual void dispose() override;

    virtual Graphic     GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    sal_uInt8           GetGreyThreshold() const { return( (sal_uInt8) FRound( mpMtrThreshold->GetValue() * 2.55 ) ); }
    bool            IsInvert() const { return mpCbxInvert->IsChecked(); }
};

class GraphicFilterSepia : public GraphicFilterDialog
{
private:
    VclPtr<MetricField>    mpMtrSepia;
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);
public:
    GraphicFilterSepia( vcl::Window* pParent, const Graphic& rGraphic,
                        sal_uInt16 nSepiaPercent );
    virtual ~GraphicFilterSepia();
    virtual void dispose() override;
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    sal_uInt16 GetSepiaPercent() const
    {
        return sal::static_int_cast< sal_uInt16 >(mpMtrSepia->GetValue());
    }
};

class GraphicFilterPoster : public GraphicFilterDialog
{
private:
    VclPtr<NumericField>   mpNumPoster;
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);
public:
    GraphicFilterPoster( vcl::Window* pParent, const Graphic& rGraphic,
                         sal_uInt16 nPosterColorCount );
    virtual ~GraphicFilterPoster();
    virtual void dispose() override;

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
    sal_uInt16      GetPosterColorCount() const { return( (sal_uInt16) mpNumPoster->GetValue() ); }
};

class EmbossControl : public SvxRectCtl
{
private:
    Link<>          maModifyHdl;
    virtual void    MouseButtonDown( const MouseEvent& rEvt ) override;
    virtual Size    GetOptimalSize() const override;
public:
    EmbossControl(vcl::Window* pParent)
        : SvxRectCtl(pParent) {}

    void            SetModifyHdl( const Link<>& rHdl ) { maModifyHdl = rHdl; }
};

class GraphicFilterEmboss : public GraphicFilterDialog
{
private:
    VclPtr<EmbossControl>  mpCtlLight;
public:
    GraphicFilterEmboss( vcl::Window* pParent, const Graphic& rGraphic,
                         RECT_POINT eLightSource );
    virtual ~GraphicFilterEmboss();
    virtual void dispose() override;

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
