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
#ifndef _SVX_ZOOM_HXX
#define _SVX_ZOOM_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

class SvxZoomDialog : public SfxModalDialog
{
private:
    RadioButton*        m_pOptimalBtn;
    RadioButton*        m_pWholePageBtn;
    RadioButton*        m_pPageWidthBtn;
    RadioButton*        m_p100Btn;
    RadioButton*        m_pUserBtn;
    MetricField*        m_pUserEdit;

    VclContainer*       m_pViewFrame;
    RadioButton*        m_pAutomaticBtn;
    RadioButton*        m_pSingleBtn;
    RadioButton*        m_pColumnsBtn;
    MetricField*        m_pColumnsEdit;
    CheckBox*           m_pBookModeChk;

    OKButton*           m_pOKBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;
    sal_Bool            bModified;

#ifdef _SVX_ZOOM_CXX
    DECL_LINK( UserHdl, RadioButton* );
    DECL_LINK(SpinHdl, void *);
    DECL_LINK( ViewLayoutUserHdl, RadioButton* );
    DECL_LINK( ViewLayoutSpinHdl, MetricField* );
    DECL_LINK( ViewLayoutCheckHdl, CheckBox* );
    DECL_LINK( OKHdl, Button* );
#endif

public:
    SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet );
    ~SvxZoomDialog();

    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    sal_uInt16              GetFactor() const;
    void                SetFactor( sal_uInt16 nNewFactor, sal_uInt16 nBtnId = 0 );

    void                HideButton( sal_uInt16 nBtnId );
    void                SetLimits( sal_uInt16 nMin, sal_uInt16 nMax );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
