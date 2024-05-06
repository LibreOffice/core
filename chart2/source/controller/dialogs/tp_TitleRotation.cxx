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

#include "tp_TitleRotation.hxx"

#include <chartview/ChartSfxItemIds.hxx>
#include <TextDirectionListBox.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/sdangitm.hxx>

namespace chart
{

SchAlignmentTabPage::SchAlignmentTabPage(weld::Container* pPage, weld::DialogController* pController,
                                         const SfxItemSet& rInAttrs, bool bWithRotation)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/titlerotationtabpage.ui"_ustr, u"TitleRotationTabPage"_ustr, &rInAttrs)
    , m_xFtRotate(m_xBuilder->weld_label(u"degreeL"_ustr))
    , m_xNfRotate(m_xBuilder->weld_metric_spin_button(u"OrientDegree"_ustr, FieldUnit::DEGREE))
    , m_xCbStacked(m_xBuilder->weld_check_button(u"stackedCB"_ustr))
    , m_xFtABCD(m_xBuilder->weld_label(u"labelABCD"_ustr))
    , m_aLbTextDirection(m_xBuilder->weld_combo_box(u"textdirLB"_ustr))
    , m_xCtrlDial(new svx::DialControl)
    , m_xCtrlDialWin(new weld::CustomWeld(*m_xBuilder, u"dialCtrl"_ustr, *m_xCtrlDial))
{
    m_xCtrlDial->SetLinkedField(m_xNfRotate.get());
    m_xCtrlDial->SetText(m_xFtABCD->get_label());
    m_xCbStacked->connect_toggled(LINK(this, SchAlignmentTabPage, StackedToggleHdl));

    m_xCtrlDialWin->set_sensitive(true);
    m_xNfRotate->set_sensitive(true);
    m_xCbStacked->set_sensitive(true);
    m_xFtRotate->set_sensitive(true);

    if( !bWithRotation )
    {
        m_xCtrlDialWin->hide();
        m_xNfRotate->hide();
        m_xCbStacked->hide();
        m_xFtRotate->hide();
    }
}

IMPL_LINK_NOARG(SchAlignmentTabPage, StackedToggleHdl, weld::Toggleable&, void)
{
    bool bActive = m_xCbStacked->get_active();
    m_xNfRotate->set_sensitive(!bActive);
    m_xCtrlDialWin->set_sensitive(!bActive);
    m_xCtrlDial->StyleUpdated();
    m_xFtRotate->set_sensitive(!bActive);
}

SchAlignmentTabPage::~SchAlignmentTabPage()
{
    m_xCtrlDialWin.reset();
    m_xCtrlDial.reset();
}

std::unique_ptr<SfxTabPage> SchAlignmentTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet* rInAttrs)
{
    return std::make_unique<SchAlignmentTabPage>(pPage, pController, *rInAttrs);
}

std::unique_ptr<SfxTabPage> SchAlignmentTabPage::CreateWithoutRotation(weld::Container* pPage, weld::DialogController* pController,
                                                              const SfxItemSet* rInAttrs)
{
    return std::make_unique<SchAlignmentTabPage>(pPage, pController, *rInAttrs, false);
}

bool SchAlignmentTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    //Since 04/1998 text can be rotated by an arbitrary angle: SCHATTR_TEXT_DEGREES
    bool bStacked = m_xCbStacked->get_active();
    rOutAttrs->Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    Degree100 nDegrees = bStacked ? 0_deg100 : m_xCtrlDial->GetRotation();
    rOutAttrs->Put( SdrAngleItem( SCHATTR_TEXT_DEGREES, nDegrees ) );

    SvxFrameDirection aDirection( m_aLbTextDirection.get_active_id() );
    rOutAttrs->Put( SvxFrameDirectionItem( aDirection, EE_PARA_WRITINGDIR ) );

    return true;
}

void SchAlignmentTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem* pItem = GetItem( *rInAttrs, SCHATTR_TEXT_DEGREES );

    Degree100 nDegrees = pItem ? static_cast<const SdrAngleItem*>(pItem)->GetValue() : 0_deg100;
    m_xCtrlDial->SetRotation( nDegrees );

    pItem = GetItem( *rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue();
    m_xCbStacked->set_active(bStacked);
    StackedToggleHdl(*m_xCbStacked);

    if( const SvxFrameDirectionItem* pDirectionItem = rInAttrs->GetItemIfSet(EE_PARA_WRITINGDIR) )
        m_aLbTextDirection.set_active_id(pDirectionItem->GetValue());
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
