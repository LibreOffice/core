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
#pragma once

#include <sfx2/tabdlg.hxx>

namespace chart
{

class AxisPositionsTabPage : public SfxTabPage
{
public:
    AxisPositionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~AxisPositionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs );
    virtual bool FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void Reset( const SfxItemSet* rInAttrs ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pItemSet ) override;

    void SetNumFormatter( SvNumberFormatter* pFormatter );

    void SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis );
    void SetCategories( const css::uno::Sequence< OUString >& rCategories );

    void SupportAxisPositioning( bool bSupportAxisPositioning );
    void SupportCategoryPositioning( bool bSupportCategoryPositioning );

private: //methods:
    DECL_LINK(CrossesAtSelectHdl, weld::ComboBox&, void);
    DECL_LINK(PlaceLabelsSelectHdl, weld::ComboBox&, void);

private: //member:
    SvNumberFormatter*  m_pNumFormatter;

    bool    m_bCrossingAxisIsCategoryAxis;
    css::uno::Sequence< OUString > m_aCategories;

    bool    m_bSupportAxisPositioning;
    bool    m_bSupportCategoryPositioning;

    std::unique_ptr<weld::Frame> m_xFL_AxisLine;
    std::unique_ptr<weld::ComboBox> m_xLB_CrossesAt;
    std::unique_ptr<weld::FormattedSpinButton> m_xED_CrossesAt;
    std::unique_ptr<weld::ComboBox> m_xED_CrossesAtCategory;
    std::unique_ptr<weld::CheckButton> m_xCB_AxisBetweenCategories;

    std::unique_ptr<weld::Frame> m_xFL_Position;
    std::unique_ptr<weld::RadioButton> m_xRB_On;
    std::unique_ptr<weld::RadioButton> m_xRB_Between;

    std::unique_ptr<weld::Frame> m_xFL_Labels;
    std::unique_ptr<weld::ComboBox> m_xLB_PlaceLabels;
    std::unique_ptr<weld::FormattedSpinButton> m_xED_LabelDistance;

    std::unique_ptr<weld::CheckButton> m_xCB_TicksInner;
    std::unique_ptr<weld::CheckButton> m_xCB_TicksOuter;

    std::unique_ptr<weld::CheckButton> m_xCB_MinorInner;
    std::unique_ptr<weld::CheckButton> m_xCB_MinorOuter;

    std::unique_ptr<weld::Widget> m_xBxPlaceTicks;
    std::unique_ptr<weld::ComboBox> m_xLB_PlaceTicks;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
