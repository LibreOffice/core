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
