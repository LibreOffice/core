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
#include <svl/intitem.hxx>

namespace chart
{

SchAlignmentTabPage::SchAlignmentTabPage(TabPageParent pParent,
                                         const SfxItemSet& rInAttrs, bool bWithRotation)
    : SfxTabPage(pParent, "modules/schart/ui/titlerotationtabpage.ui", "TitleRotationTabPage", &rInAttrs)
    , m_xFtRotate(m_xBuilder->weld_label("degreeL"))
    , m_xNfRotate(m_xBuilder->weld_spin_button("OrientDegree"))
    , m_xCbStacked(m_xBuilder->weld_check_button("stackedCB"))
    , m_xFtTextDirection(m_xBuilder->weld_label("textdirL"))
    , m_xFtABCD(m_xBuilder->weld_label("labelABCD"))
    , m_xLbTextDirection(new TextDirectionListBox(m_xBuilder->weld_combo_box("textdirLB")))
    , m_xCtrlDial(new weld::CustomWeld(*m_xBuilder, "dialCtrl", m_aCtrlDial))
{
    m_aCtrlDial.SetLinkedField(m_xNfRotate.get());
    m_aCtrlDial.SetText(m_xFtABCD->get_label());
    m_xCbStacked->connect_toggled(LINK(this, SchAlignmentTabPage, StackedToggleHdl));

    m_xCtrlDial->set_sensitive(true);
    m_xNfRotate->set_sensitive(true);
    m_xCbStacked->set_sensitive(true);
    m_xFtRotate->set_sensitive(true);

    if( !bWithRotation )
    {
        m_xCtrlDial->hide();
        m_xNfRotate->hide();
        m_xCbStacked->hide();
        m_xFtRotate->hide();
    }
}

IMPL_LINK_NOARG(SchAlignmentTabPage, StackedToggleHdl, weld::ToggleButton&, void)
{
    bool bActive = m_xCbStacked->get_active();
    m_xNfRotate->set_sensitive(bActive);
    m_xCtrlDial->set_sensitive(bActive);
    m_aCtrlDial.StyleUpdated();
    m_xFtRotate->set_sensitive(bActive);
}

SchAlignmentTabPage::~SchAlignmentTabPage()
{
    disposeOnce();
}

void SchAlignmentTabPage::dispose()
{
    m_xCtrlDial.reset();
    m_xLbTextDirection.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SchAlignmentTabPage::Create(TabPageParent pParent,
                                               const SfxItemSet* rInAttrs)
{
    return VclPtr<SchAlignmentTabPage>::Create(pParent, *rInAttrs);
}

VclPtr<SfxTabPage> SchAlignmentTabPage::CreateWithoutRotation(TabPageParent pParent,
                                                              const SfxItemSet* rInAttrs)
{
    return VclPtr<SchAlignmentTabPage>::Create(pParent, *rInAttrs, false);
}

bool SchAlignmentTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    //Since 04/1998 text can be rotated by an arbitrary angle: SCHATTR_TEXT_DEGREES
    bool bStacked = m_xCbStacked->get_active();
    rOutAttrs->Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    sal_Int32 nDegrees = bStacked ? 0 : m_aCtrlDial.GetRotation();
    rOutAttrs->Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );

    SvxFrameDirection aDirection( m_xLbTextDirection->get_active_id() );
    rOutAttrs->Put( SvxFrameDirectionItem( aDirection, EE_PARA_WRITINGDIR ) );

    return true;
}

void SchAlignmentTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem* pItem = GetItem( *rInAttrs, SCHATTR_TEXT_DEGREES );

    sal_Int32 nDegrees = pItem ? static_cast<const SfxInt32Item*>(pItem)->GetValue() : 0;
    m_aCtrlDial.SetRotation( nDegrees );

    pItem = GetItem( *rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue();
    m_xCbStacked->set_active(bStacked);
    StackedToggleHdl(*m_xCbStacked);

    if( rInAttrs->GetItemState(EE_PARA_WRITINGDIR, true, &pItem) == SfxItemState::SET)
        m_xLbTextDirection->set_active_id(static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue());
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
