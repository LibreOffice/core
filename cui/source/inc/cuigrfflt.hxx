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

#ifndef _CUI_GRFFLT_HXX
#define _CUI_GRFFLT_HXX

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

// -----------------------
// - GraphicFilterDialog -
// -----------------------

class GraphicFilterDialog : public ModalDialog
{
private:

    class PreviewWindow : public Control
    {
    private:

        Graphic   maGraphic;

        virtual void    Paint( const Rectangle& rRect );

    public:

                        PreviewWindow( Window* pParent, const ResId& rResId );
                        ~PreviewWindow();

        void            SetGraphic( const Graphic& rGraphic );
    };

private:

    Timer           maTimer;
    Link            maModifyHdl;
    Graphic         maGraphic;
    double          mfScaleX;
    double          mfScaleY;
    Size            maSizePixel;

                    DECL_LINK(ImplPreviewTimeoutHdl, void *);
                    DECL_LINK( ImplModifyHdl, void* p );

protected:
    PreviewWindow   maPreview;
    OKButton        maBtnOK;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;
    FixedLine       maFlParameter;

    const Link&     GetModifyHdl() const { return maModifyHdl; }
    const Size&     GetGraphicSizePixel() const { return maSizePixel; }

public:

                    GraphicFilterDialog( Window* pParent, const ResId& rResId, const Graphic& rGraphic );
                    ~GraphicFilterDialog();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};

// -------------------------
// - GraphicFilterSmooth -
// -------------------------

class GraphicFilterSmooth : public GraphicFilterDialog
{
private:

    FixedText       maFtRadius;
    NumericField    maMtrRadius;

public:

    GraphicFilterSmooth( Window* pParent, const Graphic& rGraphic, double nRadius);
    ~GraphicFilterSmooth();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    double          GetRadius() const { return maMtrRadius.GetValue() / 10.0; }
};

// -----------------------
// - GraphicFilterMosaic -
// -----------------------

class GraphicFilterMosaic : public GraphicFilterDialog
{
private:

    FixedText       maFtWidth;
    MetricField     maMtrWidth;
    FixedText       maFtHeight;
    MetricField     maMtrHeight;
    CheckBox        maCbxEdges;

public:

                    GraphicFilterMosaic( Window* pParent, const Graphic& rGraphic,
                                         sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, sal_Bool bEnhanceEdges );
                    ~GraphicFilterMosaic();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    long            GetTileWidth() const { return static_cast<long>(maMtrWidth.GetValue()); }
    long            GetTileHeight() const { return static_cast<long>(maMtrHeight.GetValue()); }
    sal_Bool            IsEnhanceEdges() const { return maCbxEdges.IsChecked(); }
};

// -------------------------
// - GraphicFilterSolarize -
// -------------------------

class GraphicFilterSolarize : public GraphicFilterDialog
{
private:

    FixedText       maFtThreshold;
    MetricField     maMtrThreshold;
    CheckBox        maCbxInvert;

public:

                    GraphicFilterSolarize( Window* pParent, const Graphic& rGraphic,
                                           sal_uInt8 nGreyThreshold, sal_Bool bInvert );
                    ~GraphicFilterSolarize();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    sal_uInt8           GetGreyThreshold() const { return( (sal_uInt8) FRound( maMtrThreshold.GetValue() * 2.55 ) ); }
    sal_Bool            IsInvert() const { return maCbxInvert.IsChecked(); }
};

// ----------------------
// - GraphicFilterSepia -
// ----------------------

class GraphicFilterSepia : public GraphicFilterDialog
{
private:

    FixedText       maFtSepia;
    MetricField     maMtrSepia;

public:

                    GraphicFilterSepia( Window* pParent, const Graphic& rGraphic,
                                        sal_uInt16 nSepiaPercent );
                    ~GraphicFilterSepia();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    sal_uInt16          GetSepiaPercent() const
    { return sal::static_int_cast< sal_uInt16 >(maMtrSepia.GetValue()); }
};

// -----------------------
// - GraphicFilterPoster -
// -----------------------

class GraphicFilterPoster : public GraphicFilterDialog
{
private:

    FixedText       maFtPoster;
    NumericField    maNumPoster;

public:

                    GraphicFilterPoster( Window* pParent, const Graphic& rGraphic,
                                         sal_uInt16 nPosterColorCount );
                    ~GraphicFilterPoster();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    sal_uInt16          GetPosterColorCount() const { return( (sal_uInt16) maNumPoster.GetValue() ); }
};

// -----------------------
// - GraphicFilterEmboss -
// -----------------------

class GraphicFilterEmboss : public GraphicFilterDialog
{
private:

    class EmbossControl : public SvxRectCtl
    {
    private:

        Link            maModifyHdl;

        virtual void    MouseButtonDown( const MouseEvent& rEvt );

    public:

                        EmbossControl( Window* pParent, const ResId& rResId, RECT_POINT eRectPoint ) :
                            SvxRectCtl( pParent, rResId ) { SetActualRP( eRectPoint ); }

        void            SetModifyHdl( const Link& rHdl ) { maModifyHdl = rHdl; }
    };

private:

    FixedText       maFtLight;
    EmbossControl   maCtlLight;

public:

                    GraphicFilterEmboss( Window* pParent, const Graphic& rGraphic,
                                         RECT_POINT eLightSource );
                    ~GraphicFilterEmboss();

    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
    RECT_POINT      GetLightSource() const { return maCtlLight.GetActualRP(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
