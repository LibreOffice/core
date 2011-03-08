/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_COLRDLG_HXX
#define _SV_COLRDLG_HXX

#include "svtools/svtdllapi.h"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/colctrl.hxx>

// ---------------
// - ColorDialog -
// ---------------

class SVT_DLLPUBLIC SvColorDialog : public ModalDialog
{
private:
    ColorMixingControl  maColMixCtrl;
    PushButton      maBtn1;
    PushButton      maBtn2;
    //PushButton        maBtn3;
    //PushButton        maBtn4;
    //FixedText     maFtRGB;
    SvColorControl  maCtlColor;

    // CMYK controls.
    FixedText       maFtCyan;
    MetricField     maNumCyan;
    FixedText       maFtMagenta;
    MetricField     maNumMagenta;
    FixedText       maFtYellow;
    MetricField     maNumYellow;
    FixedText       maFtKey;
    MetricField     maNumKey;

    // RGB controls.
    FixedText       maFtRed;
    NumericField    maNumRed;
    FixedText       maFtGreen;
    NumericField    maNumGreen;
    FixedText       maFtBlue;
    NumericField    maNumBlue;

    // HSL controls.
    FixedText       maFtHue;
    NumericField    maNumHue;
    FixedText       maFtSaturation;
    NumericField    maNumSaturation;
    FixedText       maFtLuminance;
    NumericField    maNumLuminance;

    ColorPreviewControl maCtlPreview;
    ColorPreviewControl maCtlPreviewOld;

    OKButton        maBtnOK;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    Color           maColor;

    SVT_DLLPRIVATE void         Initialize();

    DECL_DLLPRIVATE_LINK( ColorModifyHdl, void * );
    DECL_DLLPRIVATE_LINK( ClickBtnHdl, void * );
    DECL_DLLPRIVATE_LINK( ClickMixCtrlHdl, void * );
    DECL_DLLPRIVATE_LINK( SelectMixCtrlHdl, void * );

public:
                    SvColorDialog( Window* pParent );
                    ~SvColorDialog();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    virtual short   Execute();
};

#endif  // _SV_COLRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
