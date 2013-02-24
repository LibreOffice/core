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
#ifndef _COMPRESS_GRAPHIC_DIALOG_HXX
#define _COMPRESS_GRAPHIC_DIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <sfx2/bindings.hxx>
#include "svx/svxdllapi.h"

class SdrGrafObj;

class SVX_DLLPUBLIC CompressGraphicsDialog : public ModalDialog
{
private:
    FixedText*       m_pFixedText2;
    FixedText*       m_pFixedText3;
    FixedText*       m_pFixedText5;
    FixedText*       m_pFixedText6;

    CheckBox*        m_pReduceResolutionCB;
    NumericField*    m_pMFNewWidth;
    NumericField*    m_pMFNewHeight;
    ComboBox*        m_pResolutionLB;
    RadioButton*     m_pLosslessRB;
    RadioButton*     m_pJpegCompRB;
    NumericField*    m_pCompressionMF;
    NumericField*    m_pQualityMF;
    PushButton*      m_pBtnCalculate;
    ListBox*         m_pInterpolationCombo;

    SdrGrafObj*     m_pGraphicObj;
    Graphic         m_aGraphic;
    Size            m_aViewSize100mm;
    Rectangle       m_aCropRectangle;
    SfxBindings&    m_rBindings;

    double          m_dResolution;

    void Initialize();

    DECL_LINK( NewWidthModifiedHdl, void* );
    DECL_LINK( NewHeightModifiedHdl, void* );
    DECL_LINK( ResolutionModifiedHdl, void* );
    DECL_LINK( ToggleCompressionRB, void* );
    DECL_LINK( ToggleReduceResolutionRB, void* );

    DECL_LINK( CalculateClickHdl, void* );

    void Update();
    void UpdateNewWidthMF();
    void UpdateNewHeightMF();
    void UpdateResolutionLB();

    void Compress(SvStream& aStream);

    double GetViewWidthInch();
    double GetViewHeightInch();

    sal_uLong GetSelectedInterpolationType();

public:
    CompressGraphicsDialog( Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings );
    CompressGraphicsDialog( Window* pParent, Graphic& rGraphic, Size rViewSize100mm, Rectangle& rCropRectangle, SfxBindings& rBindings );

    virtual ~CompressGraphicsDialog();

    SdrGrafObj* GetCompressedSdrGrafObj();
    Graphic GetCompressedGraphic();

    Rectangle GetScaledCropRectangle();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
