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
    virtual void dispose() SAL_OVERRIDE;

    virtual void SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;

private:
    // Widgets
    VclPtr<FixedText>          mpInputRangeText;
    VclPtr<formula::RefEdit>   mpInputRangeEdit;
    VclPtr<formula::RefButton> mpInputRangeButton;
    VclPtr<ListBox>            mpDistributionCombo;
    VclPtr<FixedText>          mpParameter1Text;
    VclPtr<NumericField>       mpParameter1Value;
    VclPtr<FixedText>          mpParameter2Text;
    VclPtr<NumericField>       mpParameter2Value;
    VclPtr<NumericField>       mpSeed;
    VclPtr<CheckBox>           mpEnableSeed;
    VclPtr<NumericField>       mpDecimalPlaces;
    VclPtr<CheckBox>           mpEnableRounding;
    VclPtr<PushButton>         mpButtonApply;
    VclPtr<OKButton>           mpButtonOk;
    VclPtr<CloseButton>        mpButtonClose;

    // Data
    ScViewData*         mpViewData;
    ScDocument*         mpDoc;

    ScRange             maInputRange;

    bool                mbDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    template<class RNG>

    void GenerateNumbers(RNG& randomGenerator, const sal_Int16 aDistributionStringId, const boost::optional<sal_Int8> aDecimalPlaces);

    void SelectGeneratorAndGenerateNumbers();

    DECL_LINK_TYPED( OkClicked,        Button*, void );
    DECL_LINK_TYPED( CloseClicked,     Button*, void );
    DECL_LINK_TYPED( ApplyClicked,     Button*, void );
    DECL_LINK( GetFocusHandler,  Control* );
    DECL_LINK( LoseFocusHandler, void* );

    DECL_LINK( InputRangeModified, void* );
    DECL_LINK( Parameter1ValueModified, void* );
    DECL_LINK( Parameter2ValueModified, void* );
    DECL_LINK( DistributionChanged, void* );
    DECL_LINK_TYPED( CheckChanged, CheckBox&, void );

};

#endif // INCLUDED_SC_SOURCE_UI_INC_RANDOMNUMBERGENERATORDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
