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
    Link      maModifyHdl;
    Graphic   maScaledOrig;
    Graphic   maPreview;
    double    mfScaleX;
    double    mfScaleY;

    virtual void Paint(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
    virtual Size GetOptimalSize() const SAL_OVERRIDE;

    void ScaleImageToFit();

public:

    GraphicPreviewWindow(Window* pParent, WinBits nStyle);
    void init(const Graphic* pOrigGraphic, const Link& rLink)
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
    Link            maModifyHdl;
    Size            maSizePixel;
    bool            bIsBitmap;

    DECL_LINK(ImplPreviewTimeoutHdl, void *);
    DECL_LINK( ImplModifyHdl, void* p );

protected:
    GraphicPreviewWindow*  mpPreview;

    const Link&     GetModifyHdl() const { return maModifyHdl; }
    const Size&     GetGraphicSizePixel() const { return maSizePixel; }

public:

    GraphicFilterDialog(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const Graphic& rGraphic);

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};



// - GraphicFilterSmooth -


class GraphicFilterSmooth : public GraphicFilterDialog
{
private:
    NumericField*   mpMtrRadius;

public:

    GraphicFilterSmooth( Window* pParent, const Graphic& rGraphic, double nRadius);

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    double          GetRadius() const { return mpMtrRadius->GetValue() / 10.0; }
};


// - GraphicFilterMosaic -


class GraphicFilterMosaic : public GraphicFilterDialog
{
private:
    MetricField*    mpMtrWidth;
    MetricField*    mpMtrHeight;
    CheckBox*       mpCbxEdges;

public:

    GraphicFilterMosaic(Window* pParent, const Graphic& rGraphic,
        sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, sal_Bool bEnhanceEdges);

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    long            GetTileWidth() const { return static_cast<long>(mpMtrWidth->GetValue()); }
    long            GetTileHeight() const { return static_cast<long>(mpMtrHeight->GetValue()); }
    sal_Bool        IsEnhanceEdges() const { return mpCbxEdges->IsChecked(); }
};


// - GraphicFilterSolarize -


class GraphicFilterSolarize : public GraphicFilterDialog
{
private:
    MetricField*    mpMtrThreshold;
    CheckBox*       mpCbxInvert;

public:

    GraphicFilterSolarize( Window* pParent, const Graphic& rGraphic,
                                           sal_uInt8 nGreyThreshold, sal_Bool bInvert );

    virtual Graphic     GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    sal_uInt8           GetGreyThreshold() const { return( (sal_uInt8) FRound( mpMtrThreshold->GetValue() * 2.55 ) ); }
    sal_Bool            IsInvert() const { return mpCbxInvert->IsChecked(); }
};


// - GraphicFilterSepia -


class GraphicFilterSepia : public GraphicFilterDialog
{
private:
    MetricField*    mpMtrSepia;
public:
    GraphicFilterSepia( Window* pParent, const Graphic& rGraphic,
                        sal_uInt16 nSepiaPercent );
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    sal_uInt16 GetSepiaPercent() const
    {
        return sal::static_int_cast< sal_uInt16 >(mpMtrSepia->GetValue());
    }
};


// - GraphicFilterPoster -


class GraphicFilterPoster : public GraphicFilterDialog
{
private:
    NumericField*   mpNumPoster;
public:
    GraphicFilterPoster( Window* pParent, const Graphic& rGraphic,
                         sal_uInt16 nPosterColorCount );

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    sal_uInt16      GetPosterColorCount() const { return( (sal_uInt16) mpNumPoster->GetValue() ); }
};


// - GraphicFilterEmboss -


class EmbossControl : public SvxRectCtl
{
private:
    Link            maModifyHdl;
    virtual void    MouseButtonDown( const MouseEvent& rEvt ) SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
public:
    EmbossControl(Window* pParent)
        : SvxRectCtl(pParent) {}

    void            SetModifyHdl( const Link& rHdl ) { maModifyHdl = rHdl; }
};

class GraphicFilterEmboss : public GraphicFilterDialog
{
private:
    EmbossControl*  mpCtlLight;
public:
    GraphicFilterEmboss( Window* pParent, const Graphic& rGraphic,
                         RECT_POINT eLightSource );

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
    RECT_POINT      GetLightSource() const { return mpCtlLight->GetActualRP(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
