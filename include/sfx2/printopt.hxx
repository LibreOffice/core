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
#include <vcl/lstbox.hxx>
#include <vcl/print.hxx>

#include <sfx2/tabdlg.hxx>


class SFX2_DLLPUBLIC SfxCommonPrintOptionsTabPage : public SfxTabPage
{
private:

    std::unique_ptr<weld::RadioButton> m_xPrinterOutputRB;
    std::unique_ptr<weld::RadioButton> m_xPrintFileOutputRB;
    std::unique_ptr<weld::CheckButton> m_xReduceTransparencyCB;
    std::unique_ptr<weld::RadioButton> m_xReduceTransparencyAutoRB;
    std::unique_ptr<weld::RadioButton> m_xReduceTransparencyNoneRB;
    std::unique_ptr<weld::CheckButton> m_xReduceGradientsCB;
    std::unique_ptr<weld::RadioButton> m_xReduceGradientsStripesRB;
    std::unique_ptr<weld::RadioButton> m_xReduceGradientsColorRB;
    std::unique_ptr<weld::SpinButton> m_xReduceGradientsStepCountNF;
    std::unique_ptr<weld::CheckButton> m_xReduceBitmapsCB;
    std::unique_ptr<weld::RadioButton> m_xReduceBitmapsOptimalRB;
    std::unique_ptr<weld::RadioButton> m_xReduceBitmapsNormalRB;
    std::unique_ptr<weld::RadioButton> m_xReduceBitmapsResolutionRB;
    std::unique_ptr<weld::ComboBox> m_xReduceBitmapsResolutionLB;
    std::unique_ptr<weld::CheckButton> m_xReduceBitmapsTransparencyCB;
    std::unique_ptr<weld::CheckButton> m_xConvertToGreyscalesCB;
    std::unique_ptr<weld::CheckButton> m_xPDFCB;
    std::unique_ptr<weld::CheckButton> m_xPaperSizeCB;
    std::unique_ptr<weld::CheckButton> m_xPaperOrientationCB;
    std::unique_ptr<weld::CheckButton> m_xTransparencyCB;

private:

    PrinterOptions      maPrinterOptions;
    PrinterOptions      maPrintFileOptions;

                        DECL_DLLPRIVATE_LINK( ToggleOutputPrinterRBHdl, weld::ToggleButton&, void );
                        DECL_DLLPRIVATE_LINK( ToggleOutputPrintFileRBHdl, weld::ToggleButton&, void);

                        DECL_DLLPRIVATE_LINK( ClickReduceTransparencyCBHdl, weld::Button&, void );
                        DECL_DLLPRIVATE_LINK( ClickReduceGradientsCBHdl, weld::Button&, void );
                        DECL_DLLPRIVATE_LINK( ClickReduceBitmapsCBHdl, weld::Button&, void );

                        DECL_DLLPRIVATE_LINK( ToggleReduceGradientsStripesRBHdl, weld::ToggleButton&, void );
                        DECL_DLLPRIVATE_LINK( ToggleReduceBitmapsResolutionRBHdl, weld::ToggleButton&, void );

    SAL_DLLPRIVATE void ImplUpdateControls( const PrinterOptions* pCurrentOptions );
    SAL_DLLPRIVATE void ImplSaveControls( PrinterOptions* pCurrentOptions );

protected:

    using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:

                        SfxCommonPrintOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual             ~SfxCommonPrintOptionsTabPage() override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* );
};

#endif // INCLUDED_SFX2_PRINTOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
