/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_COLRDLG_HXX
#define _SV_COLRDLG_HXX

#include "svtools/svtdllapi.h"
#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
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
