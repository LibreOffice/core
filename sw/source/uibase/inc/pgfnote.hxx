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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_PGFNOTE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_PGFNOTE_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>

#include <vcl/group.hxx>
#include <svtools/ctrlbox.hxx>

// footnote settings TabPage
class SwFootNotePage: public SfxTabPage
{
    friend class VclPtr<SwFootNotePage>;
    static const sal_uInt16 aPageRg[];
    SwFootNotePage(vcl::Window *pParent, const SfxItemSet &rSet);
public:
    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

    virtual ~SwFootNotePage();
    virtual void dispose() override;
private:

    VclPtr<RadioButton>    m_pMaxHeightPageBtn;
    VclPtr<RadioButton>    m_pMaxHeightBtn;
    VclPtr<MetricField>    m_pMaxHeightEdit;
    VclPtr<MetricField>    m_pDistEdit;

    VclPtr<ListBox>        m_pLinePosBox;
    VclPtr<LineListBox>    m_pLineTypeBox;
    VclPtr<MetricField>    m_pLineWidthEdit;
    VclPtr<ColorListBox>   m_pLineColorBox;
    VclPtr<MetricField>    m_pLineLengthEdit;
    VclPtr<MetricField>    m_pLineDistEdit;

    DECL_LINK_TYPED( HeightPage, Button*, void );
    DECL_LINK_TYPED( HeightMetric, Button*, void );
    DECL_LINK_TYPED( HeightModify, Control&, void );
    DECL_LINK_TYPED( LineWidthChanged_Impl, Edit&, void );
    DECL_LINK_TYPED( LineColorSelected_Impl, ListBox&, void );

    long            lMaxHeight;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
