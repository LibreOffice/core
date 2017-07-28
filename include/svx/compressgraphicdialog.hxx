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

#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

class Button;
class CheckBox;
class ComboBox;
class Edit;
class FixedText;
class ListBox;
class NumericField;
class PushButton;
class RadioButton;
class SdrGrafObj;
class SfxBindings;
class Slider;
class SvStream;

class SAL_WARN_UNUSED SVX_DLLPUBLIC CompressGraphicsDialog : public ModalDialog
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
    tools::Rectangle       m_aCropRectangle;
    SfxBindings&    m_rBindings;

    double          m_dResolution;

    void Initialize();

    DECL_LINK( EndSlideHdl, Slider*, void );
    DECL_LINK( NewInterpolationModifiedHdl, ListBox&, void );
    DECL_LINK( NewQualityModifiedHdl, Edit&, void );
    DECL_LINK( NewCompressionModifiedHdl, Edit&, void );
    DECL_LINK( NewWidthModifiedHdl, Edit&, void );
    DECL_LINK( NewHeightModifiedHdl, Edit&, void );
    DECL_LINK( ResolutionModifiedHdl, Edit&, void );
    DECL_LINK( ToggleCompressionRB, RadioButton&, void );
    DECL_LINK( ToggleReduceResolutionRB, CheckBox&, void );

    DECL_LINK( CalculateClickHdl, Button*, void );

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
    CompressGraphicsDialog( vcl::Window* pParent, Graphic const & rGraphic, Size rViewSize100mm, tools::Rectangle const & rCropRectangle, SfxBindings& rBindings );
    virtual ~CompressGraphicsDialog() override;
    virtual void dispose() override;

    SdrGrafObj* GetCompressedSdrGrafObj();
    Graphic GetCompressedGraphic();

    tools::Rectangle GetScaledCropRectangle();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
