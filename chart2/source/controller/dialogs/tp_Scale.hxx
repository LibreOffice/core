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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SCALE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SCALE_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/fmtfield.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

namespace chart
{

class ScaleTabPage : public SfxTabPage
{
public:
    ScaleTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~ScaleTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rInAttrs );
    virtual bool FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void Reset( const SfxItemSet* rInAttrs ) override;
    using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pItemSet ) override;

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNumFormat();

    void ShowAxisOrigin( bool bShowOrigin );

    virtual void StateChanged( StateChangedType nType ) override;

private:
    VclPtr<CheckBox>            m_pCbxReverse;
    VclPtr<CheckBox>            m_pCbxLogarithm;

    VclPtr<VclBox>              m_pBxType;
    VclPtr<ListBox>             m_pLB_AxisType;

    VclPtr<VclGrid>              m_pBxMinMax;
    VclPtr<FormattedField>      m_pFmtFldMin;
    VclPtr<CheckBox>            m_pCbxAutoMin;

    VclPtr<FormattedField>      m_pFmtFldMax;
    VclPtr<CheckBox>            m_pCbxAutoMax;

    VclPtr<VclBox>              m_pBxResolution;
    VclPtr<ListBox>             m_pLB_TimeResolution;
    VclPtr<CheckBox>            m_pCbx_AutoTimeResolution;

    VclPtr<FixedText>           m_pTxtMain;
    VclPtr<FormattedField>      m_pFmtFldStepMain;
    VclPtr<MetricField>         m_pMt_MainDateStep;
    VclPtr<ListBox>             m_pLB_MainTimeUnit;
    VclPtr<CheckBox>            m_pCbxAutoStepMain;

    VclPtr<FixedText>           m_pTxtHelpCount;
    VclPtr<FixedText>           m_pTxtHelp;
    VclPtr<MetricField>         m_pMtStepHelp;
    VclPtr<ListBox>             m_pLB_HelpTimeUnit;
    VclPtr<CheckBox>            m_pCbxAutoStepHelp;

    VclPtr<FormattedField>      m_pFmtFldOrigin;
    VclPtr<CheckBox>            m_pCbxAutoOrigin;
    VclPtr<VclBox>              m_pBxOrigin;

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

    void EnableControls();

    DECL_LINK_TYPED( SelectAxisTypeHdl, ListBox&, void );
    DECL_LINK_TYPED( EnableValueHdl, Button*, void );
    DECL_STATIC_LINK_TYPED( ScaleTabPage, FmtFieldModifiedHdl, Edit&, void);

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
    bool ShowWarning( sal_uInt16 nResIdMessage, Control* pControl = nullptr );

    void HideAllControls();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
