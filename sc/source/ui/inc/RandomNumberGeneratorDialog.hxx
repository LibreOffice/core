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

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

#include <boost/optional.hpp>

class ScRandomNumberGeneratorDialog : public ScAnyRefDlg
{
public:
    ScRandomNumberGeneratorDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScRandomNumberGeneratorDialog();

    virtual void SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;

private:
    // Widgets
    FixedText*          mpInputRangeText;
    formula::RefEdit*   mpInputRangeEdit;
    formula::RefButton* mpInputRangeButton;
    ListBox*            mpDistributionCombo;
    FixedText*          mpParameter1Text;
    NumericField*       mpParameter1Value;
    FixedText*          mpParameter2Text;
    NumericField*       mpParameter2Value;
    NumericField*       mpSeed;
    CheckBox*           mpEnableSeed;
    NumericField*       mpDecimalPlaces;
    CheckBox*           mpEnableRounding;
    PushButton*         mpButtonApply;
    OKButton*           mpButtonOk;
    CloseButton*        mpButtonClose;

    // Data
    ScViewData*         mpViewData;
    ScDocument*         mpDoc;

    ScRange             maInputRange;

    bool                mbDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    template<class RNG>

    void GenerateNumbers(RNG randomGenerator, const sal_Int16 aDistributionStringId, const boost::optional<sal_Int8> aDecimalPlaces);

    void SelectGeneratorAndGenerateNumbers();

    DECL_LINK( OkClicked,        PushButton* );
    DECL_LINK( CloseClicked,     PushButton* );
    DECL_LINK( ApplyClicked,     PushButton* );
    DECL_LINK( GetFocusHandler,  Control* );
    DECL_LINK( LoseFocusHandler, void* );

    DECL_LINK( Parameter1ValueModified, void* );
    DECL_LINK( Parameter2ValueModified, void* );
    DECL_LINK( DistributionChanged, void* );
    DECL_LINK( CheckChanged, void* );

};

#endif // INCLUDED_SC_SOURCE_UI_INC_RANDOMNUMBERGENERATORDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
