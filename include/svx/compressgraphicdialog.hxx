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
#ifndef INCLUDED_SVX_COMPRESSGRAPHICDIALOG_HXX
#define INCLUDED_SVX_COMPRESSGRAPHICDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/graph.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/slider.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svxdllapi.h>

class SdrGrafObj;

class SVX_DLLPUBLIC SAL_WARN_UNUSED CompressGraphicsDialog : public ModalDialog
{
private:
    VclPtr<FixedText>       m_pLabelGraphicType;
    VclPtr<FixedText>       m_pFixedText2;
    VclPtr<FixedText>       m_pFixedText3;
    VclPtr<FixedText>       m_pFixedText5;
    VclPtr<FixedText>       m_pFixedText6;

    VclPtr<CheckBox>        m_pReduceResolutionCB;
    VclPtr<NumericField>    m_pMFNewWidth;
    VclPtr<NumericField>    m_pMFNewHeight;
    VclPtr<ComboBox>        m_pResolutionLB;
    VclPtr<RadioButton>     m_pLosslessRB;
    VclPtr<RadioButton>     m_pJpegCompRB;
    VclPtr<NumericField>    m_pCompressionMF;
    VclPtr<Slider>          m_pCompressionSlider;
    VclPtr<NumericField>    m_pQualityMF;
    VclPtr<Slider>          m_pQualitySlider;
    VclPtr<PushButton>      m_pBtnCalculate;
    VclPtr<ListBox>         m_pInterpolationCombo;

    SdrGrafObj*     m_pGraphicObj;
    Graphic         m_aGraphic;
    Size            m_aViewSize100mm;
    Rectangle       m_aCropRectangle;
    SfxBindings&    m_rBindings;

    double          m_dResolution;

    void Initialize();

    DECL_LINK_TYPED( NewWidthModifiedHdl, Edit&, void );
    DECL_LINK_TYPED( NewHeightModifiedHdl, Edit&, void );
    DECL_LINK_TYPED( ResolutionModifiedHdl, Edit&, void );
    DECL_LINK_TYPED( ToggleCompressionRB, RadioButton&, void );
    DECL_LINK_TYPED( ToggleReduceResolutionRB, CheckBox&, void );

    DECL_LINK_TYPED( CalculateClickHdl, Button*, void );

    void Update();
    void UpdateNewWidthMF();
    void UpdateNewHeightMF();
    void UpdateResolutionLB();

    void Compress(SvStream& aStream);

    double GetViewWidthInch();
    double GetViewHeightInch();

    BmpScaleFlag GetSelectedInterpolationType();

public:
    CompressGraphicsDialog( vcl::Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings );
    CompressGraphicsDialog( vcl::Window* pParent, Graphic& rGraphic, Size rViewSize100mm, Rectangle& rCropRectangle, SfxBindings& rBindings );
    virtual ~CompressGraphicsDialog();
    virtual void dispose() override;

    SdrGrafObj* GetCompressedSdrGrafObj();
    Graphic GetCompressedGraphic();

    Rectangle GetScaledCropRectangle();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
