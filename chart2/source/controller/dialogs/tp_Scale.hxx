/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

//.............................................................................
namespace chart
{
//.............................................................................

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
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
