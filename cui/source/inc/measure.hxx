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
#ifndef INCLUDED_CUI_SOURCE_INC_MEASURE_HXX
#define INCLUDED_CUI_SOURCE_INC_MEASURE_HXX

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
    static const sal_uInt16 pRanges[];

    VclPtr<MetricField>         m_pMtrFldLineDist;
    VclPtr<MetricField>         m_pMtrFldHelplineOverhang;
    VclPtr<MetricField>         m_pMtrFldHelplineDist;
    VclPtr<MetricField>         m_pMtrFldHelpline1Len;
    VclPtr<MetricField>         m_pMtrFldHelpline2Len;
    VclPtr<TriStateBox>         m_pTsbBelowRefEdge;
    VclPtr<MetricField>         m_pMtrFldDecimalPlaces;

    VclPtr<SvxRectCtl>          m_pCtlPosition;
    VclPtr<TriStateBox>         m_pTsbAutoPosV;
    VclPtr<TriStateBox>         m_pTsbAutoPosH;
    VclPtr<TriStateBox>         m_pTsbShowUnit;
    VclPtr<ListBox>             m_pLbUnit;
    VclPtr<TriStateBox>         m_pTsbParallel;
    VclPtr<FixedText>           m_pFtAutomatic;

    VclPtr<SvxXMeasurePreview>  m_pCtlPreview;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

    bool            bPositionModified;

    void                FillUnitLB();

    DECL_LINK_TYPED( ClickAutoPosHdl_Impl, Button*, void );
    DECL_LINK( ChangeAttrHdl_Impl, void * );
    DECL_LINK_TYPED( ChangeAttrListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangeAttrClickHdl_Impl, Button*, void );

public:

    SvxMeasurePage( vcl::Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxMeasurePage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;

    virtual void        PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;

};

/* Derived from SfxSingleTabDialog, in order to be able to be
   informed about virtual methods by the control. */
class SvxMeasureDialog : public SfxSingleTabDialog
{
public:
    SvxMeasureDialog(vcl::Window* pParent, const SfxItemSet& rAttr,
        const SdrView* pView);
};

#endif // INCLUDED_CUI_SOURCE_INC_MEASURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
