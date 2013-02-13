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

#ifndef _SFX_PRINTOPT_HXX
#define _SFX_PRINTOPT_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
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

    RadioButton*        m_pPrinterOutputRB;
    RadioButton*        m_pPrintFileOutputRB;

    CheckBox*           m_pReduceTransparencyCB;
    RadioButton*        m_pReduceTransparencyAutoRB;
    RadioButton*        m_pReduceTransparencyNoneRB;

    CheckBox*           m_pReduceGradientsCB;
    RadioButton*        m_pReduceGradientsStripesRB;
    RadioButton*        m_pReduceGradientsColorRB;
    NumericField*       m_pReduceGradientsStepCountNF;

    CheckBox*           m_pReduceBitmapsCB;
    RadioButton*        m_pReduceBitmapsOptimalRB;
    RadioButton*        m_pReduceBitmapsNormalRB;
    RadioButton*        m_pReduceBitmapsResolutionRB;
    ListBox*            m_pReduceBitmapsResolutionLB;
    CheckBox*           m_pReduceBitmapsTransparencyCB;

    CheckBox*           m_pConvertToGreyscalesCB;

    CheckBox*           m_pPDFCB;

    CheckBox*           m_pPaperSizeCB;
    CheckBox*           m_pPaperOrientationCB;
    CheckBox*           m_pTransparencyCB;

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

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual Window*     GetParentLabeledBy( const Window* pLabel ) const;
    virtual Window*     GetParentLabelFor( const Window* pLabel ) const;

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
};

#endif // #ifndef _SFX_PRINTOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
