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



#ifndef _SFX_PRINTOPT_HXX
#define _SFX_PRINTOPT_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX //autogen wg. FixedText
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/print.hxx>

#include <sfx2/tabdlg.hxx>

// --------------------------------
// - SfxCommonPrintOptionsTabPage -
// --------------------------------

class SvtBasePrintOptions;

class SFX2_DLLPUBLIC SfxCommonPrintOptionsTabPage : public SfxTabPage
{
private:

    FixedLine           aReduceGB;

    FixedText           aOutputTypeFT;
    RadioButton         aPrinterOutputRB;
    RadioButton         aPrintFileOutputRB;

    FixedLine           aOutputGB;

    CheckBox            aReduceTransparencyCB;
    RadioButton         aReduceTransparencyAutoRB;
    RadioButton         aReduceTransparencyNoneRB;

    CheckBox            aReduceGradientsCB;
    RadioButton         aReduceGradientsStripesRB;
    RadioButton         aReduceGradientsColorRB;
    NumericField        aReduceGradientsStepCountNF;

    CheckBox            aReduceBitmapsCB;
    RadioButton         aReduceBitmapsOptimalRB;
    RadioButton         aReduceBitmapsNormalRB;
    RadioButton         aReduceBitmapsResolutionRB;
    ListBox             aReduceBitmapsResolutionLB;
    CheckBox            aReduceBitmapsTransparencyCB;

    CheckBox            aConvertToGreyscalesCB;

    FixedLine           aWarnGB;

    CheckBox            aPaperSizeCB;
    CheckBox            aPaperOrientationCB;
    CheckBox            aTransparencyCB;

private:

    PrinterOptions      maPrinterOptions;
    PrinterOptions      maPrintFileOptions;

                        DECL_DLLPRIVATE_LINK( ToggleOutputPrinterRBHdl, RadioButton* pButton );
                        DECL_DLLPRIVATE_LINK( ToggleOutputPrintFileRBHdl, RadioButton* pButton );

                        DECL_DLLPRIVATE_LINK( ClickReduceTransparencyCBHdl, CheckBox* pBox );
                        DECL_DLLPRIVATE_LINK( ClickReduceGradientsCBHdl, CheckBox* pBox );
                        DECL_DLLPRIVATE_LINK( ClickReduceBitmapsCBHdl, CheckBox* pBox );

                        DECL_DLLPRIVATE_LINK( ToggleReduceGradientsStripesRBHdl, RadioButton* pButton );
                        DECL_DLLPRIVATE_LINK( ToggleReduceBitmapsResolutionRBHdl, RadioButton* pButton );

    SAL_DLLPRIVATE void ImplUpdateControls( const PrinterOptions* pCurrentOptions );
    SAL_DLLPRIVATE void ImplSaveControls( PrinterOptions* pCurrentOptions );

    // --> OD 2008-06-25 #i63982#
    SAL_DLLPRIVATE void ImplSetAccessibleNames();
    // <--

protected:

    using TabPage::DeactivatePage;
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:

                        SfxCommonPrintOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
                        ~SfxCommonPrintOptionsTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual Window*     GetParentLabeledBy( const Window* pLabel ) const;
    virtual Window*     GetParentLabelFor( const Window* pLabel ) const;

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
};

// -------------------------------
// - TransparencyPrintWarningBox -
// -------------------------------

class TransparencyPrintWarningBox : public ModalDialog
{
private:

    FixedImage      aWarnFI;
    FixedText       aWarnFT;
    OKButton        aYesBtn;
    PushButton      aNoBtn;
    CancelButton    aCancelBtn;
    CheckBox        aNoWarnCB;

                    DECL_LINK( ClickNoBtn, PushButton* );

public:

                    TransparencyPrintWarningBox( Window* pParent );
                    ~TransparencyPrintWarningBox();

    sal_Bool            IsNoWarningChecked() const { return aNoWarnCB.IsChecked(); }
};

#endif // #ifndef _SFX_PRINTOPT_HXX

