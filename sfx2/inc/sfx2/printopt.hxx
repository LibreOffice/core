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

