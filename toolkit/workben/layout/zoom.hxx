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

// include ---------------------------------------------------------------

#include <layout/layout.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

// define ----------------------------------------------------------------
/* CHINA001
#define ZOOMBTN_OPTIMAL     ((USHORT)0x0001)
#define ZOOMBTN_PAGEWIDTH   ((USHORT)0x0002)
#define ZOOMBTN_WHOLEPAGE   ((USHORT)0x0004)
*/
#include "zoom_def.hxx"
// class SvxZoomDialog ---------------------------------------------------
/*
    {k:\svx\prototyp\dialog\zoom.bmp}

    [Beschreibung]
    Mit diesem Dialog wird ein Zoom-Faktor eingestellt.

    [Items]
    SvxZoomItem <SID_ATTR_ZOOM>
*/

#include <layout/layout-pre.hxx>

class SvxZoomDialog : public SfxModalDialog
{
private:
    FixedLine           aZoomFl;
    RadioButton         aOptimalBtn;
    RadioButton         aWholePageBtn;
    RadioButton         aPageWidthBtn;
    RadioButton         a100Btn;
    RadioButton         aUserBtn;
    MetricField         aUserEdit;

    FixedLine           aViewLayoutFl;
    RadioButton         aAutomaticBtn;
    RadioButton         aSingleBtn;
    RadioButton         aColumnsBtn;
    MetricField         aColumnsEdit;
    CheckBox            aBookModeChk;

    FixedLine           aBottomFl;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

#if !TEST_LAYOUT
    const SfxItemSet&   rSet;
#endif /* !TEST_LAYOUT */
    SfxItemSet*         pOutSet;
    BOOL                bModified;

#ifdef _SVX_ZOOM_CXX
    DECL_LINK( UserHdl, RadioButton* );
    DECL_LINK( SpinHdl, MetricField* );
    DECL_LINK( ViewLayoutUserHdl, RadioButton* );
    DECL_LINK( ViewLayoutSpinHdl, MetricField* );
    DECL_LINK( ViewLayoutCheckHdl, CheckBox* );
    DECL_LINK( OKHdl, Button* );
#endif

public:
#if TEST_LAYOUT
    SvxZoomDialog( Window* pParent );
#else /* !TEST_LAYOUT */
    SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet );
#endif /* !TEST_LAYOUT */
    ~SvxZoomDialog();

    static USHORT*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    USHORT              GetFactor() const;
    void                SetFactor( USHORT nNewFactor, USHORT nBtnId = 0 );

    void                SetButtonText( USHORT nBtnId, const String& aNewTxt );
    void                HideButton( USHORT nBtnId );
    void                SetLimits( USHORT nMin, USHORT nMax );
    void                SetSpinSize( USHORT nNewSpin );
};

#include <layout/layout-post.hxx>

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
