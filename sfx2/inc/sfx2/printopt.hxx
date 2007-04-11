/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printopt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:27:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SFX_PRINTOPT_HXX
#define _SFX_PRINTOPT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen wg. FixedText
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen wg. FixedText
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif

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

protected:

    using TabPage::DeactivatePage;
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:

                        SfxCommonPrintOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
                        ~SfxCommonPrintOptionsTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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

    BOOL            IsNoWarningChecked() const { return aNoWarnCB.IsChecked(); }
};

#endif // #ifndef _SFX_PRINTOPT_HXX

