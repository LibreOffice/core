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
#ifndef _CHART2_TP_SCALES_HXX
#define _CHART2_TP_SCALES_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
// header for class FormattedField
#include <svtools/fmtfield.hxx>
// header for FixedText
#include <vcl/fixed.hxx>
// header for CheckBox
#include <vcl/button.hxx>
// header for MetricField
#include <vcl/field.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>

namespace chart
{

class ScaleTabPage : public SfxTabPage
{
public:
    ScaleTabPage( Window* pParent, const SfxItemSet& rInAttrs );

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );
    using TabPage::DeactivatePage;
    virtual int DeactivatePage( SfxItemSet* pItemSet = NULL );

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNumFormat();

    void ShowAxisOrigin( bool bShowOrigin );

    virtual void StateChanged( StateChangedType nType );

private:
    FixedLine           aFlScale;

    CheckBox            aCbxReverse;

    CheckBox            aCbxLogarithm;

    FixedText           m_aTxt_AxisType;
    ListBox             m_aLB_AxisType;

    FixedText           aTxtMin;
    FormattedField      aFmtFldMin;
    CheckBox            aCbxAutoMin;

    FixedText           aTxtMax;
    FormattedField      aFmtFldMax;
    CheckBox            aCbxAutoMax;

    FixedText           m_aTxt_TimeResolution;
    ListBox             m_aLB_TimeResolution;
    CheckBox            m_aCbx_AutoTimeResolution;

    FixedText           aTxtMain;
    FormattedField      aFmtFldStepMain;
    MetricField         m_aMt_MainDateStep;
    ListBox             m_aLB_MainTimeUnit;
    CheckBox            aCbxAutoStepMain;

    FixedText           aTxtHelpCount;
    FixedText           aTxtHelp;
    MetricField         aMtStepHelp;
    ListBox             m_aLB_HelpTimeUnit;
    CheckBox            aCbxAutoStepHelp;

    FixedText           aTxtOrigin;
    FormattedField      aFmtFldOrigin;
    CheckBox            aCbxAutoOrigin;

    double              fMin;
    double              fMax;
    double              fStepMain;
    sal_Int32           nStepHelp;
    double              fOrigin;
    sal_Int32           m_nTimeResolution;
    sal_Int32           m_nMainTimeUnit;
    sal_Int32           m_nHelpTimeUnit;
    int                 m_nAxisType;
    bool                m_bAllowDateAxis;
    SvNumberFormatter*  pNumFormatter;

    bool                m_bShowAxisOrigin;

    void AdjustControlPositions();
    void EnableControls();
    void PlaceIntervalControlsAccordingToAxisType();

    DECL_LINK( SelectAxisTypeHdl, void* );
    DECL_LINK( EnableValueHdl, CheckBox* );
    DECL_LINK( FmtFieldModifiedHdl, FormattedField* );

    /** shows a warning window due to an invalid input.

        @param nResIdMessage
            The resource identifier that represents the localized warning text.
            If this is 0, no warning is shown and false is returned.

        @param pControl
            If non-NULL, contains a pointer to the control in which the
            errornous value was in.  This method gives this control the focus
            and selects its content.

        @return false, if nResIdMessage was 0, true otherwise
     */
    bool ShowWarning( sal_uInt16 nResIdMessage, Control* pControl = NULL );

    void HideAllControls();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
