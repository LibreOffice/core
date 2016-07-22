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

#ifndef INCLUDED_SFX2_PRINTOPT_HXX
#define INCLUDED_SFX2_PRINTOPT_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/print.hxx>

#include <sfx2/tabdlg.hxx>


class SvtBasePrintOptions;

class SFX2_DLLPUBLIC SfxCommonPrintOptionsTabPage : public SfxTabPage
{
private:

    VclPtr<RadioButton>        m_pPrinterOutputRB;
    VclPtr<RadioButton>        m_pPrintFileOutputRB;

    VclPtr<CheckBox>           m_pReduceTransparencyCB;
    VclPtr<RadioButton>        m_pReduceTransparencyAutoRB;
    VclPtr<RadioButton>        m_pReduceTransparencyNoneRB;

    VclPtr<CheckBox>           m_pReduceGradientsCB;
    VclPtr<RadioButton>        m_pReduceGradientsStripesRB;
    VclPtr<RadioButton>        m_pReduceGradientsColorRB;
    VclPtr<NumericField>       m_pReduceGradientsStepCountNF;

    VclPtr<CheckBox>           m_pReduceBitmapsCB;
    VclPtr<RadioButton>        m_pReduceBitmapsOptimalRB;
    VclPtr<RadioButton>        m_pReduceBitmapsNormalRB;
    VclPtr<RadioButton>        m_pReduceBitmapsResolutionRB;
    VclPtr<ListBox>            m_pReduceBitmapsResolutionLB;
    VclPtr<CheckBox>           m_pReduceBitmapsTransparencyCB;

    VclPtr<CheckBox>           m_pConvertToGreyscalesCB;

    VclPtr<CheckBox>           m_pPDFCB;

    VclPtr<CheckBox>           m_pPaperSizeCB;
    VclPtr<CheckBox>           m_pPaperOrientationCB;
    VclPtr<CheckBox>           m_pTransparencyCB;

private:

    PrinterOptions      maPrinterOptions;
    PrinterOptions      maPrintFileOptions;

                        DECL_DLLPRIVATE_LINK_TYPED( ToggleOutputPrinterRBHdl, RadioButton&, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ToggleOutputPrintFileRBHdl, RadioButton&, void);

                        DECL_DLLPRIVATE_LINK_TYPED( ClickReduceTransparencyCBHdl, Button*, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ClickReduceGradientsCBHdl, Button*, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ClickReduceBitmapsCBHdl, Button*, void );

                        DECL_DLLPRIVATE_LINK_TYPED( ToggleReduceGradientsStripesRBHdl, RadioButton&, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ToggleReduceBitmapsResolutionRBHdl, RadioButton&, void );

    SAL_DLLPRIVATE void ImplUpdateControls( const PrinterOptions* pCurrentOptions );
    SAL_DLLPRIVATE void ImplSaveControls( PrinterOptions* pCurrentOptions );

protected:

    using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:

                        SfxCommonPrintOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual             ~SfxCommonPrintOptionsTabPage();
    virtual void        dispose() override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual vcl::Window*     GetParentLabeledBy( const vcl::Window* pLabel ) const override;
    virtual vcl::Window*     GetParentLabelFor( const vcl::Window* pLabel ) const override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* );
};

#endif // INCLUDED_SFX2_PRINTOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
