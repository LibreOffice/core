/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef RANDOM_NUMBER_GENERATION_HXX
#define RANDOM_NUMBER_GENERATION_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"


#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

class ScRandomNumberGeneratorDialog : public ScAnyRefDlg
{
public:
    ScRandomNumberGeneratorDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        Window* pParent, ScViewData* pViewData );

    virtual ~ScRandomNumberGeneratorDialog();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void        SetActive();
    virtual sal_Bool    Close();

private:
    // Widgets
    FixedText*          mpFtVariableCell;
    formula::RefEdit*   mpEdVariableCell;
    formula::RefButton* mpRBVariableCell;
    ListBox*            mpDistributionCombo;
    FixedText*          mpParameter1Text;
    NumericField*       mpParameter1Value;
    FixedText*          mpParameter2Text;
    NumericField*       mpParameter2Value;
    NumericField*       mpSeed;
    CheckBox*           mpEnableSeed;
    PushButton*         mpButtonApply;
    OKButton*           mpButtonOk;
    CancelButton*       mpButtonCancel;

    // Data
    ScViewData*         mViewData;
    ScDocument*         mDocument;

    ScRange             mCurrentRange;
    ScAddress::Details  mAddressDetails;

    bool                mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    template<class RNG>
    void GenerateNumbers(RNG randomGenerator, OUString aDistributionName);

    void SelectGeneratorAndGenerateNumbers();

    DECL_LINK( OkClicked,        PushButton* );
    DECL_LINK( CancelClicked,    PushButton* );
    DECL_LINK( ApplyClicked,     PushButton* );
    DECL_LINK( GetFocusHandler,  Control* );
    DECL_LINK( LoseFocusHandler, void* );

    DECL_LINK( Parameter1ValueModified, void* );
    DECL_LINK( Parameter2ValueModified, void* );
    DECL_LINK( DistributionChanged, void* );
    DECL_LINK( SeedCheckChanged, void* );

};

#endif // SC_SOLVRDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
