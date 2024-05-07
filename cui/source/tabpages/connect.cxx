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

#include <strings.hrc>
#include <dialmgr.hxx>

#include <svx/connctrl.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/ofaitem.hxx>

#include <svx/svdview.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svtools/unitconv.hxx>

#include <connect.hxx>

const WhichRangesContainer SvxConnectionPage::pRanges(
    svl::Items<SDRATTR_EDGE_FIRST, SDRATTR_EDGE_LAST>);

/*************************************************************************
|*
|* dialog for changing connectors
|*
\************************************************************************/
SvxConnectionDialog::SvxConnectionDialog(weld::Window* pParent, const SfxItemSet& rInAttrs, const SdrView* pSdrView)
    : SfxSingleTabDialogController(pParent, &rInAttrs)
{
    auto xPage = std::make_unique<SvxConnectionPage>(get_content_area(), this, rInAttrs);

    xPage->SetView(pSdrView);
    xPage->Construct();

    SetTabPage(std::move(xPage));
    m_xDialog->set_title(CuiResId(RID_CUISTR_CONNECTOR));
}

/*************************************************************************
|*
|* page for changing connectors
|*
\************************************************************************/

SvxConnectionPage::SvxConnectionPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/connectortabpage.ui"_ustr, u"ConnectorTabPage"_ustr, &rInAttrs)
    , rOutAttrs(rInAttrs)
    , aAttrSet(*rInAttrs.GetPool())
    , pView(nullptr)
    , m_xLbType(m_xBuilder->weld_combo_box(u"LB_TYPE"_ustr))
    , m_xFtLine1(m_xBuilder->weld_label(u"FT_LINE_1"_ustr))
    , m_xMtrFldLine1(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_LINE_1"_ustr, FieldUnit::CM))
    , m_xFtLine2(m_xBuilder->weld_label(u"FT_LINE_2"_ustr))
    , m_xMtrFldLine2(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_LINE_2"_ustr, FieldUnit::CM))
    , m_xFtLine3(m_xBuilder->weld_label(u"FT_LINE_3"_ustr))
    , m_xMtrFldLine3(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_LINE_3"_ustr, FieldUnit::CM))
    , m_xMtrFldHorz1(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_HORZ_1"_ustr, FieldUnit::MM))
    , m_xMtrFldVert1(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_VERT_1"_ustr, FieldUnit::MM))
    , m_xMtrFldHorz2(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_HORZ_2"_ustr, FieldUnit::MM))
    , m_xMtrFldVert2(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_VERT_2"_ustr, FieldUnit::MM))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"CTL_PREVIEW"_ustr, m_aCtlPreview))
{
    SfxItemPool* pPool = rOutAttrs.GetPool();
    assert(pPool && "Where is the pool");
    eUnit = pPool->GetMetric( SDRATTR_EDGENODE1HORZDIST );

    FillTypeLB();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_xMtrFldHorz1, eFUnit );
    SetFieldUnit( *m_xMtrFldHorz2, eFUnit );
    SetFieldUnit( *m_xMtrFldVert1, eFUnit );
    SetFieldUnit( *m_xMtrFldVert2, eFUnit );
    SetFieldUnit( *m_xMtrFldLine1, eFUnit );
    SetFieldUnit( *m_xMtrFldLine2, eFUnit );
    SetFieldUnit( *m_xMtrFldLine3, eFUnit );
    if( eFUnit == FieldUnit::MM )
    {
        m_xMtrFldHorz1->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldHorz2->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldVert1->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldVert2->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldLine1->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldLine2->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldLine3->set_increments(50, 500, FieldUnit::NONE);
    }

    Link<weld::MetricSpinButton&,void> aLink(LINK(this, SvxConnectionPage, ChangeAttrEditHdl_Impl));
    m_xMtrFldHorz1->connect_value_changed(aLink);
    m_xMtrFldVert1->connect_value_changed(aLink);
    m_xMtrFldHorz2->connect_value_changed(aLink);
    m_xMtrFldVert2->connect_value_changed(aLink);
    m_xMtrFldLine1->connect_value_changed(aLink);
    m_xMtrFldLine2->connect_value_changed(aLink);
    m_xMtrFldLine3->connect_value_changed(aLink);
    m_xLbType->connect_changed(LINK(this, SvxConnectionPage, ChangeAttrListBoxHdl_Impl));
}

SvxConnectionPage::~SvxConnectionPage()
{
    m_xCtlPreview.reset();
}

template<class T>
void SvxConnectionPage::SetMetricValueAndSave(const SfxItemSet* rAttrs, weld::MetricSpinButton& rField, TypedWhichId<T> nWhich)
{
    const SfxPoolItem* pItem = GetItem( *rAttrs,  nWhich);
    const SfxItemPool* pPool = rAttrs->GetPool();
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( nWhich );
    SetMetricValue(rField, pItem->StaticWhichCast(nWhich).GetValue(), eUnit);
    rField.save_value();
}

/*************************************************************************
|*
|* reads passed Item-Set
|*
\************************************************************************/

void SvxConnectionPage::Reset( const SfxItemSet* rAttrs )
{
    const SfxPoolItem* pItem;
    const SfxItemPool* pPool = rAttrs->GetPool();

    // SdrEdgeNode1HorzDistItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldHorz1, SDRATTR_EDGENODE1HORZDIST);

    // SdrEdgeNode2HorzDistItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldHorz2, SDRATTR_EDGENODE2HORZDIST);

    // SdrEdgeNode1VertDistItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldVert1, SDRATTR_EDGENODE1VERTDIST);

    // SdrEdgeNode2VertDistItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldVert2, SDRATTR_EDGENODE2VERTDIST);

    // SdrEdgeLine1DeltaItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldLine1, SDRATTR_EDGELINE1DELTA);

    // SdrEdgeLine2DeltaItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldLine2, SDRATTR_EDGELINE2DELTA);

    // SdrEdgeLine3DeltaItem
    SetMetricValueAndSave(rAttrs, *m_xMtrFldLine3, SDRATTR_EDGELINE3DELTA);

    // SdrEdgeLineDeltaAnzItem
    pItem = GetItem( *rAttrs, SDRATTR_EDGELINEDELTACOUNT );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_EDGELINEDELTACOUNT );
    switch (pItem->StaticWhichCast(SDRATTR_EDGELINEDELTACOUNT).GetValue())
    {
        case 0:
            m_xFtLine1->set_sensitive(false);
            m_xMtrFldLine1->set_sensitive(false);
            m_xMtrFldLine1->set_text(u""_ustr);
            [[fallthrough]];
        case 1:
            m_xFtLine2->set_sensitive(false);
            m_xMtrFldLine2->set_sensitive(false);
            m_xMtrFldLine2->set_text(u""_ustr);
            [[fallthrough]];
        case 2:
            m_xFtLine3->set_sensitive(false);
            m_xMtrFldLine3->set_sensitive(false);
            m_xMtrFldLine3->set_text(u""_ustr);
            break;
    }

    // SdrEdgeKindItem
    pItem = GetItem( *rAttrs, SDRATTR_EDGEKIND );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_EDGEKIND );
    m_xLbType->set_active(
        sal::static_int_cast<sal_uInt16>(pItem->StaticWhichCast(SDRATTR_EDGEKIND).GetValue()));
    m_xLbType->save_value();
}

/*************************************************************************
|*
|* fills the passed Item-Set width Dialogbox attributes
|*
\************************************************************************/

bool SvxConnectionPage::FillItemSet( SfxItemSet* rAttrs)
{
    bool         bModified = false;
    sal_Int32    nValue;

    if (m_xMtrFldHorz1->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldHorz1, eUnit);
        rAttrs->Put( SdrEdgeNode1HorzDistItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldHorz2->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldHorz2, eUnit);
        rAttrs->Put( SdrEdgeNode2HorzDistItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldVert1->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldVert1, eUnit);
        rAttrs->Put( SdrEdgeNode1VertDistItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldVert2->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldVert2, eUnit);
        rAttrs->Put( SdrEdgeNode2VertDistItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldLine1->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldLine1, eUnit);
        rAttrs->Put( makeSdrEdgeLine1DeltaItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldLine2->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldLine2, eUnit);
        rAttrs->Put( makeSdrEdgeLine2DeltaItem( nValue ) );
        bModified = true;
    }

    if (m_xMtrFldLine3->get_value_changed_from_saved())
    {
        nValue = GetCoreValue(*m_xMtrFldLine3, eUnit);
        rAttrs->Put( makeSdrEdgeLine3DeltaItem( nValue ) );
        bModified = true;
    }

    int nPos = m_xLbType->get_active();
    if (m_xLbType->get_value_changed_from_saved())
    {
        if (nPos != -1)
        {
            rAttrs->Put( SdrEdgeKindItem( static_cast<SdrEdgeKind>(nPos) ) );
            bModified = true;
        }
    }

    return bModified;
}

void SvxConnectionPage::Construct()
{
    DBG_ASSERT( pView, "No valid View transfer!" );

    m_aCtlPreview.SetView(pView);
    m_aCtlPreview.Construct();
}

/*************************************************************************
|*
|* creates the page
|*
\************************************************************************/
std::unique_ptr<SfxTabPage> SvxConnectionPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxConnectionPage>(pPage, pController, *rAttrs);
}

IMPL_LINK_NOARG(SvxConnectionPage, ChangeAttrListBoxHdl_Impl, weld::ComboBox&, void)
{
    int nPos = m_xLbType->get_active();
    if (nPos != -1)
    {
        aAttrSet.Put( SdrEdgeKindItem( static_cast<SdrEdgeKind>(nPos) ) );
    }

    m_aCtlPreview.SetAttributes(aAttrSet);

    // get the number of line displacements
    sal_uInt16 nCount = m_aCtlPreview.GetLineDeltaCount();

    m_xFtLine3->set_sensitive( nCount > 2 );
    m_xMtrFldLine3->set_sensitive( nCount > 2 );
    if( nCount > 2 )
        m_xMtrFldLine3->set_value(m_xMtrFldLine3->get_value(FieldUnit::NONE), FieldUnit::NONE);
    else
        m_xMtrFldLine3->set_text(u""_ustr);

    m_xFtLine2->set_sensitive( nCount > 1 );
    m_xMtrFldLine2->set_sensitive( nCount > 1 );
    if( nCount > 1 )
        m_xMtrFldLine2->set_value(m_xMtrFldLine2->get_value(FieldUnit::NONE), FieldUnit::NONE);
    else
        m_xMtrFldLine2->set_text(u""_ustr);

    m_xFtLine1->set_sensitive( nCount > 0 );
    m_xMtrFldLine1->set_sensitive( nCount > 0 );
    if( nCount > 0 )
        m_xMtrFldLine1->set_value(m_xMtrFldLine1->get_value(FieldUnit::NONE), FieldUnit::NONE);
    else
        m_xMtrFldLine1->set_text(u""_ustr);
}

IMPL_LINK(SvxConnectionPage, ChangeAttrEditHdl_Impl, weld::MetricSpinButton&, r, void)
{
    if (&r == m_xMtrFldHorz1.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldHorz1, eUnit);
        aAttrSet.Put( SdrEdgeNode1HorzDistItem( nValue ) );
    }

    if (&r == m_xMtrFldHorz2.get())
    {
        sal_Int32 nValue = GetCoreValue( *m_xMtrFldHorz2, eUnit );
        aAttrSet.Put( SdrEdgeNode2HorzDistItem( nValue ) );
    }

    if (&r == m_xMtrFldVert1.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldVert1, eUnit);
        aAttrSet.Put( SdrEdgeNode1VertDistItem( nValue ) );
    }

    if (&r == m_xMtrFldVert2.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldVert2, eUnit);
        aAttrSet.Put( SdrEdgeNode2VertDistItem( nValue ) );
    }

    if (&r == m_xMtrFldLine1.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldLine1, eUnit);
        aAttrSet.Put( makeSdrEdgeLine1DeltaItem( nValue ) );
    }

    if (&r == m_xMtrFldLine2.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldLine2, eUnit);
        aAttrSet.Put( makeSdrEdgeLine2DeltaItem( nValue ) );
    }

    if (&r == m_xMtrFldLine3.get())
    {
        sal_Int32 nValue = GetCoreValue(*m_xMtrFldLine3, eUnit);
        aAttrSet.Put( makeSdrEdgeLine3DeltaItem( nValue ) );
    }

    m_aCtlPreview.SetAttributes(aAttrSet);
}

void SvxConnectionPage::FillTypeLB()
{
    // fill ListBox with connector names
    const SfxPoolItem* pItem = GetItem( rOutAttrs, SDRATTR_EDGEKIND );
    const SfxItemPool* pPool = rOutAttrs.GetPool();

    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_EDGEKIND );
    const SdrEdgeKindItem* pEdgeKindItem = &pItem->StaticWhichCast(SDRATTR_EDGEKIND);
    const sal_uInt16 nCount = pEdgeKindItem->GetValueCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        OUString aStr = SdrEdgeKindItem::GetValueTextByPos(i);
        m_xLbType->append_text(aStr);
    }
}
void SvxConnectionPage::PageCreated(const SfxAllItemSet& aSet)
{
    const OfaPtrItem* pOfaPtrItem = aSet.GetItem<OfaPtrItem>(SID_OBJECT_LIST, false);
    if (pOfaPtrItem)
        SetView( static_cast<SdrView *>(pOfaPtrItem->GetValue()) );

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
