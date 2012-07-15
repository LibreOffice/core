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

class SVX_DLLPUBLIC CompressGraphicsDialog : public ModalDialog
{
private:
    FixedLine       m_aImageDetailsFL;
    FixedText       m_aFixedText2X;
    FixedText       m_aFixedText2;
    FixedText       m_aFixedText3X;
    FixedText       m_aFixedText3;
    FixedText       m_aFixedText5X;
    FixedText       m_aFixedText5;
    FixedText       m_aFixedText6X;
    FixedText       m_aFixedText6;

    FixedLine       m_aSettingsFL;
    CheckBox        m_aReduceResolutionCB;
    FixedText       m_aNewWidthFT;
    MetricField     m_aMFNewWidth;
    FixedText       m_aNewHeightFT;
    MetricField     m_aMFNewHeight;
    FixedText       m_aResolutionFT;
    ComboBox        m_aResolutionLB;
    FixedText       m_aFixedTextDPI;
    RadioButton     m_aLosslessRB;
    RadioButton     m_aJpegCompRB;
    FixedText       m_aCompressionFT;
    MetricField     m_aCompressionMF;
    FixedText       m_aQualityFT;
    MetricField     m_aQualityMF;
    CheckBox        m_aCropCB;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;
    HelpButton      m_aBtnHelp;
    PushButton      m_aBtnCalculate;

    Graphic         m_aGraphic;
    Size            m_aViewSize100mm;
    SfxBindings&    m_rBindings;

    double          m_dResolution;

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

public:
    CompressGraphicsDialog( Window* pParent, const Graphic& rGraphic, const Size& rViewSize100mm, SfxBindings& rBindings );
    virtual ~CompressGraphicsDialog();

    Graphic GetCompressedGraphic();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
