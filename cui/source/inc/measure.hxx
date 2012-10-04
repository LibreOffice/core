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
#ifndef _SVX_MEASURE_HXX
#define _SVX_MEASURE_HXX

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/measctrl.hxx>

class SdrView;

/// Dialog for changing TextAtrributes
class SvxMeasurePage : public SvxTabPage
{
private:

    FixedLine           aFlLine;
    FixedText           aFtLineDist;
    MetricField         aMtrFldLineDist;
    FixedText           aFtHelplineOverhang;
    MetricField         aMtrFldHelplineOverhang;
    FixedText           aFtHelplineDist;
    MetricField         aMtrFldHelplineDist;
    FixedText           aFtHelpline1Len;
    MetricField         aMtrFldHelpline1Len;
    FixedText           aFtHelpline2Len;
    MetricField         aMtrFldHelpline2Len;
    TriStateBox         aTsbBelowRefEdge;
    FixedText           aFtDecimalPlaces;
    MetricField         aMtrFldDecimalPlaces;

    FixedLine           aFlLabel;
    FixedText           aFtPosition;
    SvxRectCtl          aCtlPosition;
    TriStateBox         aTsbAutoPosV;
    TriStateBox         aTsbAutoPosH;
    TriStateBox         aTsbShowUnit;
    ListBox             aLbUnit;
    TriStateBox         aTsbParallel;

    SvxXMeasurePreview  aCtlPreview;

    FixedLine           aFlVert;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

    sal_Bool                bPositionModified;

#ifdef _SVX_MEASURE_CXX
    void                FillUnitLB();

    DECL_LINK( ClickAutoPosHdl_Impl, void * );
    DECL_LINK( ChangeAttrHdl_Impl, void * );
#endif

public:

    SvxMeasurePage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxMeasurePage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated (SfxAllItemSet aSet);
};

/* Derived from SfxSingleTabDialog, in order to be able to be
   informed about virtual methods by the control. */
class SvxMeasureDialog : public SfxSingleTabDialog
{
public:
    SvxMeasureDialog( Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    ~SvxMeasureDialog();
};


#endif // _SVX_MEASURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
