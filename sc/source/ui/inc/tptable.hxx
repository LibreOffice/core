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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>

class ScTablePage : public SfxTabPage
{
    friend class VclPtr<ScTablePage>;
    static const sal_uInt16 pPageTableRanges[];
public:
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*           pParent,
                                          const SfxItemSet* rCoreSet );
    static  const sal_uInt16* GetRanges () { return pPageTableRanges; }
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt ) override;

    virtual         ~ScTablePage();
    virtual void    dispose() override;
private:
                    ScTablePage( vcl::Window* pParent, const SfxItemSet& rCoreSet );
    void            ShowImage();

private:
    VclPtr<RadioButton>     m_pBtnTopDown;
    VclPtr<RadioButton>     m_pBtnLeftRight;
    VclPtr<FixedImage>      m_pBmpPageDir;
    VclPtr<CheckBox>        m_pBtnPageNo;
    VclPtr<NumericField>    m_pEdPageNo;

    VclPtr<CheckBox>        m_pBtnHeaders;
    VclPtr<CheckBox>        m_pBtnGrid;
    VclPtr<CheckBox>        m_pBtnNotes;
    VclPtr<CheckBox>        m_pBtnObjects;
    VclPtr<CheckBox>        m_pBtnCharts;
    VclPtr<CheckBox>        m_pBtnDrawings;
    VclPtr<CheckBox>        m_pBtnFormulas;
    VclPtr<CheckBox>        m_pBtnNullVals;

    VclPtr<ListBox>             m_pLbScaleMode;
    VclPtr<VclHBox>             m_pBxScaleAll;
    VclPtr<MetricField>         m_pEdScaleAll;
    VclPtr<VclGrid>             m_pGrHeightWidth;
    VclPtr<NumericField>        m_pEdScalePageWidth;
    VclPtr<NumericField>        m_pEdScalePageHeight;
    VclPtr<VclHBox>             m_pBxScalePageNum;
    VclPtr<NumericField>        m_pEdScalePageNum;

private:

    // Handler:
    DECL_LINK_TYPED(PageDirHdl, Button*, void);
    DECL_LINK_TYPED( PageNoHdl, Button*, void );
    DECL_LINK_TYPED(ScaleHdl, ListBox&, void);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
