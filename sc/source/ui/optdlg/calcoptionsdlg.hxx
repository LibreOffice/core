/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_OPTDLG_CALCOPTIONSDLG_HXX
#define INCLUDED_SC_SOURCE_UI_OPTDLG_CALCOPTIONSDLG_HXX

#include <vcl/weld.hxx>
#include <calcconfig.hxx>

class ScCalcOptionsDialog : public weld::GenericDialogController
{
public:
    ScCalcOptionsDialog(weld::Window* pParent, const ScCalcConfig& rConfig, bool bWriteConfig);
    virtual ~ScCalcOptionsDialog() override;

    DECL_LINK(AsZeroModifiedHdl, weld::ToggleButton&, void);
    DECL_LINK(ConversionModifiedHdl, weld::ComboBox&, void);
    DECL_LINK(SyntaxModifiedHdl, weld::ComboBox&, void);
    DECL_LINK(CurrentDocOnlyHdl, weld::ToggleButton&, void);

    const ScCalcConfig& GetConfig() const { return maConfig;}
    bool GetWriteCalcConfig() const { return mbWriteConfig;}

private:
    void CoupleEmptyAsZeroToStringConversion();

private:
    ScCalcConfig maConfig;
    bool mbSelectedEmptyStringAsZero;
    bool mbWriteConfig;

    std::unique_ptr<weld::CheckButton> mxEmptyAsZero;
    std::unique_ptr<weld::ComboBox> mxConversion;
    std::unique_ptr<weld::CheckButton> mxCurrentDocOnly;
    std::unique_ptr<weld::ComboBox> mxSyntax;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
