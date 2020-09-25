/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_RANDOMNUMBERGENERATORDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_RANDOMNUMBERGENERATORDIALOG_HXX

#include <sal/config.h>

#include <optional>

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

class ScRandomNumberGeneratorDialog : public ScAnyRefDlgController
{
public:
    ScRandomNumberGeneratorDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData );

    virtual ~ScRandomNumberGeneratorDialog() override;

    virtual void SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual void SetActive() override;
    virtual void Close() override;

private:
    // Data
    ScViewData&                mrViewData;
    const ScDocument&          mrDoc;

    ScRange             maInputRange;

    bool                mbDialogLostFocus;

    // Widgets
    std::unique_ptr<weld::Label> mxInputRangeText;
    std::unique_ptr<formula::RefEdit>   mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;
    std::unique_ptr<weld::ComboBox> mxDistributionCombo;
    std::unique_ptr<weld::Label> mxParameter1Text;
    std::unique_ptr<weld::SpinButton> mxParameter1Value;
    std::unique_ptr<weld::Label> mxParameter2Text;
    std::unique_ptr<weld::SpinButton> mxParameter2Value;
    std::unique_ptr<weld::SpinButton> mxSeed;
    std::unique_ptr<weld::CheckButton> mxEnableSeed;
    std::unique_ptr<weld::SpinButton> mxDecimalPlaces;
    std::unique_ptr<weld::CheckButton> mxEnableRounding;
    std::unique_ptr<weld::Button> mxButtonApply;
    std::unique_ptr<weld::Button> mxButtonOk;
    std::unique_ptr<weld::Button> mxButtonClose;

    void Init();
    void GetRangeFromSelection();

    template<class RNG>

    void GenerateNumbers(RNG& randomGenerator, const char* pDistributionStringId, const std::optional<sal_Int8> aDecimalPlaces);

    void SelectGeneratorAndGenerateNumbers();

    DECL_LINK( OkClicked, weld::Button&, void );
    DECL_LINK( CloseClicked, weld::Button&, void );
    DECL_LINK( ApplyClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler,  formula::RefEdit&, void );
    DECL_LINK( GetButtonFocusHandler,  formula::RefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::RefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::RefButton&, void );

    DECL_LINK( InputRangeModified, formula::RefEdit&, void );
    DECL_LINK( Parameter1ValueModified, weld::SpinButton&, void );
    DECL_LINK( Parameter2ValueModified, weld::SpinButton&, void );
    DECL_LINK( DistributionChanged, weld::ComboBox&, void );
    DECL_LINK( CheckChanged, weld::ToggleButton&, void );

};

#endif // INCLUDED_SC_SOURCE_UI_INC_RANDOMNUMBERGENERATORDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
