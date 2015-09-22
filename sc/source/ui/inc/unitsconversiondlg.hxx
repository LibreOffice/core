/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SC_SOURCE_UI_INC_UNITSCONVERSIONDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNITSCONVERSIONDLG_HXX

#include <formula/funcutl.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclptr.hxx>

#include "anyrefdg.hxx"
#include "rangelst.hxx"
#include "units.hxx"

class ScDocument;
class ScViewData;

class ScUnitsConversionDialog : public ScAnyRefDlg
{
public:
    ScUnitsConversionDialog(SfxBindings* pB,
                            SfxChildWindow* pCW,
                            vcl::Window* pParent,
                            ScViewData* pViewData);
    virtual ~ScUnitsConversionDialog();

    virtual void dispose() SAL_OVERRIDE;

    virtual void SetReference(const ScRange& rRef, ScDocument* pDoc) SAL_OVERRIDE;

    virtual void SetActive() SAL_OVERRIDE;

    virtual bool Close() SAL_OVERRIDE;

private:
    ScViewData* mpViewData;
    ScDocument* mpDoc;

    ScRangeListRef mInputRange;

    bool mbDialogLostFocus;

    boost::shared_ptr< sc::units::Units > mpUnits;

    VclPtr<SelectableFixedText> mpLabelInputUnits;
    OUString msNone;

    VclPtr<PushButton> mpButtonOk;

    /*
     * Cache the list of units so that we can quickly
     * check the compatibility of the desired output
     * units. (Otherwise we would have to reload the
     * input units everytime the desired output units
     * are changed, and this is likely to be the most
     * common operation in the (probably rare) case
     * that the desired output units aren't compatible.)
     */
    sc::units::RangeUnits mUnits;

    VclPtr<FixedText> mpInputRangeLabel;
    VclPtr<formula::RefEdit> mpInputRangeEdit;
    VclPtr<formula::RefButton> mpInputRangeButton;

    VclPtr<VclBox> mpIncompatibleInputsBox;

    VclPtr<ComboBox> mpOutputUnitsEdit;
    VclPtr<VclBox> mpIncompatibleOutputBox;

    void Init();
    void GetRangeFromSelection();
    void UpdateInputUnits();

    // Verify that at least one of the input units
    // can be converted to the desired output units.
    // If false, a conversion will not change any data
    // in the sheet.
    bool CheckUnitsAreConvertible();
    void PerformConversion();

    DECL_LINK_TYPED( OkClicked,             Button*, void );
    DECL_LINK( GetFocusHandler,             Control* );
    DECL_LINK( LoseFocusHandler,            void* );
    DECL_LINK( OutputUnitsModified,         void* );
    DECL_LINK( OutputUnitsGetFocusHandler,  void* );
    DECL_LINK( OutputUnitsLoseFocusHandler, void* );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_UNITSCONVERSIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
