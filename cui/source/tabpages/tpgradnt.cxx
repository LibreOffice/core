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

#include <memory>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>

#include <strings.hrc>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <svx/xgrscit.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <basegfx/utils/gradienttools.hxx>
#include <sal/log.hxx>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

SvxGradientTabPage::SvxGradientTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/gradientpage.ui"_ustr, u"GradientPage"_ustr, &rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_nGradientListState(ChangeType::NONE)
    , m_pnColorListState(nullptr)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , aIconSize(60, 64)
    , m_xLbGradientType(m_xBuilder->weld_combo_box(u"gradienttypelb"_ustr))
    , m_xFtCenter(m_xBuilder->weld_label(u"centerft"_ustr))
    , m_xMtrCenterX(m_xBuilder->weld_metric_spin_button(u"centerxmtr"_ustr, FieldUnit::PERCENT))
    , m_xMtrCenterY(m_xBuilder->weld_metric_spin_button(u"centerymtr"_ustr, FieldUnit::PERCENT))
    , m_xFtAngle(m_xBuilder->weld_label(u"angleft"_ustr))
    , m_xMtrAngle(m_xBuilder->weld_metric_spin_button(u"anglemtr"_ustr, FieldUnit::DEGREE))
    , m_xSliderAngle(m_xBuilder->weld_scale(u"angleslider"_ustr))
    , m_xMtrBorder(m_xBuilder->weld_metric_spin_button(u"bordermtr"_ustr, FieldUnit::PERCENT))
    , m_xSliderBorder(m_xBuilder->weld_scale(u"borderslider"_ustr))
    , m_xLbColorFrom(new ColorListBox(m_xBuilder->weld_menu_button(u"colorfromlb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xMtrColorFrom(m_xBuilder->weld_metric_spin_button(u"colorfrommtr"_ustr, FieldUnit::PERCENT))
    , m_xLbColorTo(new ColorListBox(m_xBuilder->weld_menu_button(u"colortolb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xMtrColorTo(m_xBuilder->weld_metric_spin_button(u"colortomtr"_ustr, FieldUnit::PERCENT))
    , m_xGradientLB(m_xBuilder->weld_icon_view(u"gradientpresetlist"_ustr))
    , m_xMtrIncrement(m_xBuilder->weld_spin_button(u"incrementmtr"_ustr))
    , m_xCbIncrement(m_xBuilder->weld_check_button(u"autoincrement"_ustr))
    , m_xBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"modify"_ustr))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"previewctl"_ustr, m_aCtlPreview))
{
    Size aSize = getDrawPreviewOptimalSize(m_aCtlPreview.GetDrawingArea()->get_ref_device());
    m_xCtlPreview->set_size_request(aSize.Width(), aSize.Height());
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // as long as NOT supported by the item

    m_xMtrColorTo->set_value(100, FieldUnit::PERCENT);
    m_xMtrColorFrom->set_value(100, FieldUnit::PERCENT);

    // setting the output device
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_GRADIENT) );
    m_rXFSet.Put( XFillGradientItem(OUString(), basegfx::BGradient()));
    m_aCtlPreview.SetAttributes(m_aXFillAttr.GetItemSet());

    // set handler
    m_xGradientLB->connect_selection_changed(LINK(this, SvxGradientTabPage, ChangeGradientHdl));
    m_xGradientLB->connect_mouse_press(LINK(this, SvxGradientTabPage, MousePressHdl));
    m_xGradientLB->connect_query_tooltip(LINK(this, SvxGradientTabPage, QueryTooltipHdl));
    m_xBtnAdd->connect_clicked(LINK(this, SvxGradientTabPage, ClickAddHdl_Impl));
    m_xBtnModify->connect_clicked(LINK(this, SvxGradientTabPage, ClickModifyHdl_Impl));

    Link<weld::MetricSpinButton&,void> aLink = LINK( this, SvxGradientTabPage, ModifiedMetricHdl_Impl );
    Link<weld::ComboBox&,void> aLink2 = LINK( this, SvxGradientTabPage, ModifiedListBoxHdl_Impl );
    m_xLbGradientType->connect_changed( aLink2 );
    m_xCbIncrement->connect_toggled(LINK(this, SvxGradientTabPage, ChangeAutoStepHdl_Impl));
    m_xMtrIncrement->connect_value_changed(LINK(this, SvxGradientTabPage, ModifiedEditHdl_Impl));
    m_xMtrCenterX->connect_value_changed( aLink );
    m_xMtrCenterY->connect_value_changed( aLink );
    m_xMtrAngle->connect_value_changed( aLink );
    m_xSliderAngle->connect_value_changed(LINK(this, SvxGradientTabPage, ModifiedSliderHdl_Impl));
    m_xMtrBorder->connect_value_changed( aLink );
    m_xSliderBorder->connect_value_changed(LINK(this, SvxGradientTabPage, ModifiedSliderHdl_Impl));
    m_xMtrColorFrom->connect_value_changed( aLink );
    Link<ColorListBox&,void> aLink3 = LINK( this, SvxGradientTabPage, ModifiedColorListBoxHdl_Impl );
    m_xLbColorFrom->SetSelectHdl( aLink3 );
    m_xMtrColorTo->connect_value_changed( aLink );
    m_xLbColorTo->SetSelectHdl( aLink3 );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_aCtlPreview.EnableRTL(false);
}

SvxGradientTabPage::~SvxGradientTabPage()
{
    m_xCtlPreview.reset();
    m_xGradientLB.reset();
    m_xLbColorTo.reset();
    m_xLbColorFrom.reset();

    if (m_nGradientListState & ChangeType::MODIFIED)
    {
        m_pGradientList->SetPath(AreaTabHelper::GetPalettePath());
        m_pGradientList->Save();

        // ToolBoxControls are informed:
        SfxObjectShell* pShell = SfxObjectShell::Current();
        if (pShell)
            pShell->PutItem(SvxGradientListItem(m_pGradientList, SID_GRADIENT_LIST));
    }
}

void SvxGradientTabPage::Construct()
{
    FillPresetListBox();
}


void SvxGradientTabPage::FillPresetListBox()
{
    m_xGradientLB->clear();

    m_xGradientLB->freeze();
    for (tools::Long nId = 0; nId < m_pGradientList->Count(); nId++)
    {
        const OUString aString(m_pGradientList->GetGradient(nId)->GetName());

        OUString sId = OUString::number(nId);
        BitmapEx aBitmap = m_pGradientList->GetBitmapForPreview(nId, aIconSize);
        VclPtr<VirtualDevice> aVDev = GetVirtualDevice(aBitmap);

        if (!m_xGradientLB->get_id(nId).isEmpty())
        {
            m_xGradientLB->set_image(nId, aVDev);
            m_xGradientLB->set_id(nId, sId);
            m_xGradientLB->set_text(nId, aString);
        }
        else
        {
            m_xGradientLB->insert(-1, &aString, &sId, aVDev, nullptr);
        }
    }

    m_xGradientLB->thaw();
}

void SvxGradientTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( !m_pColorList.is() )
        return;

    // ColorList
    if( *m_pnColorListState & ChangeType::CHANGED ||
        *m_pnColorListState & ChangeType::MODIFIED )
    {
        SvxAreaTabDialog* pArea = (*m_pnColorListState & ChangeType::CHANGED) ?
            dynamic_cast<SvxAreaTabDialog*>(GetDialogController()) : nullptr;
        if (pArea)
            m_pColorList = pArea->GetNewColorList();

        ModifiedHdl_Impl( this );
    }

    // determining (and possibly cutting) the name and
    // displaying it in the GroupBox
    OUString        aString = CuiResId( RID_CUISTR_TABLE ) + ": ";
    INetURLObject   aURL( m_pGradientList->GetPath() );

    aURL.Append( m_pGradientList->GetName() );
    SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

    if ( aURL.getBase().getLength() > 18 )
    {
        aString += OUString::Concat(aURL.getBase().subView( 0, 15 )) + "...";
    }
    else
        aString += aURL.getBase();

    sal_Int32 nPos = SearchGradientList( rSet.Get(XATTR_FILLGRADIENT).GetName() );
    if ( nPos != -1)
    {
        m_xGradientLB->select( nPos );
    }
    // colors could have been deleted
    ChangeGradientHdl_Impl();
}


DeactivateRC SvxGradientTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

bool SvxGradientTabPage::FillItemSet( SfxItemSet* rSet )
{
    std::unique_ptr<basegfx::BGradient> pBGradient;
    OUString sId = m_xGradientLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    sal_uInt16 nValue = 0; // automatic step count
    if (!m_xCbIncrement->get_active())
        nValue = m_xMtrIncrement->get_value();

    if( nPos != -1 )
    {
        pBGradient.reset(new basegfx::BGradient( m_pGradientList->GetGradient( static_cast<sal_uInt16>(nPos) )->GetGradient() ));
        const OUString aString(m_pGradientList->GetGradient(nPos)->GetName());
        // update StepCount to current value to be in sync with FillGradientStepCount
        pBGradient->SetSteps(nValue);
        rSet->Put( XFillGradientItem( aString, *pBGradient ) );
    }
    else
    // gradient was passed (unidentified)
    {
        pBGradient.reset(new basegfx::BGradient(
                    createColorStops(),
                    static_cast<css::awt::GradientStyle>(m_xLbGradientType->get_active()),
                    Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)), // should be changed in resource
                    static_cast<sal_uInt16>(m_xMtrCenterX->get_value(FieldUnit::NONE)),
                    static_cast<sal_uInt16>(m_xMtrCenterY->get_value(FieldUnit::NONE)),
                    static_cast<sal_uInt16>(m_xMtrBorder->get_value(FieldUnit::NONE)),
                    static_cast<sal_uInt16>(m_xMtrColorFrom->get_value(FieldUnit::NONE)),
                    static_cast<sal_uInt16>(m_xMtrColorTo->get_value(FieldUnit::NONE)),
                    nValue));
        rSet->Put( XFillGradientItem( OUString(), *pBGradient ) );
    }

    assert( pBGradient && "basegfx::BGradient could not be created" );
    rSet->Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
    rSet->Put( XGradientStepCountItem( nValue ) );
    return true;
}

void SvxGradientTabPage::Reset( const SfxItemSet* )
{
    ChangeGradientHdl_Impl(); // includes setting m_xCbIncrement and m_xMtrIncrement

    // determine state of the buttons
    if( m_pGradientList->Count() )
        m_xBtnModify->set_sensitive(true);
    else
        m_xBtnModify->set_sensitive(false);
}

std::unique_ptr<SfxTabPage> SvxGradientTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet* rOutAttrs )
{
    return std::make_unique<SvxGradientTabPage>(pPage, pController, *rOutAttrs);
}

IMPL_LINK( SvxGradientTabPage, ModifiedListBoxHdl_Impl, weld::ComboBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
}

IMPL_LINK( SvxGradientTabPage, ModifiedColorListBoxHdl_Impl, ColorListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
}

IMPL_LINK( SvxGradientTabPage, ModifiedEditHdl_Impl, weld::SpinButton&, rBox, void )
{
    ModifiedHdl_Impl(&rBox);
}

IMPL_LINK( SvxGradientTabPage, ModifiedMetricHdl_Impl, weld::MetricSpinButton&, rBox, void )
{
    ModifiedHdl_Impl(&rBox);
}

IMPL_LINK( SvxGradientTabPage, ModifiedSliderHdl_Impl, weld::Scale&, rSlider, void )
{
    ModifiedHdl_Impl(&rSlider);
}

IMPL_LINK_NOARG( SvxGradientTabPage, ChangeAutoStepHdl_Impl, weld::Toggleable&, void )
{
    if (m_xCbIncrement->get_active())
    {
        m_xMtrIncrement->set_sensitive(false);
    }
    else
    {
        m_xMtrIncrement->set_sensitive(true);
    }
    ModifiedHdl_Impl(m_xMtrIncrement.get());
}

void SvxGradientTabPage::ModifiedHdl_Impl( void const * pControl )
{
    if (pControl == m_xMtrBorder.get())
        m_xSliderBorder->set_value(m_xMtrBorder->get_value(FieldUnit::NONE));
    if (pControl == m_xSliderBorder.get())
        m_xMtrBorder->set_value(m_xSliderBorder->get_value(), FieldUnit::NONE);
    if (pControl == m_xMtrAngle.get())
        m_xSliderAngle->set_value(m_xMtrAngle->get_value(FieldUnit::NONE));
    if (pControl == m_xSliderAngle.get())
        m_xMtrAngle->set_value(m_xSliderAngle->get_value(), FieldUnit::NONE);

    css::awt::GradientStyle eXGS = static_cast<css::awt::GradientStyle>(m_xLbGradientType->get_active());

    sal_uInt16 nValue = 0; // automatic
    if (!m_xCbIncrement->get_active())
        nValue = static_cast<sal_uInt16>(m_xMtrIncrement->get_value());

    basegfx::BGradient aBGradient(
                          createColorStops(),
                          eXGS,
                          Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)), // should be changed in resource
                          static_cast<sal_uInt16>(m_xMtrCenterX->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrCenterY->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrBorder->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrColorFrom->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrColorTo->get_value(FieldUnit::NONE)),
                          nValue);

    // enable/disable controls
    if (pControl == m_xLbGradientType.get() || pControl == this)
        SetControlState_Impl( eXGS );

    m_rXFSet.Put( XGradientStepCountItem( nValue ) );

    // displaying in XOutDev
    m_rXFSet.Put( XFillGradientItem( OUString(), aBGradient ) );
    m_aCtlPreview.SetAttributes(m_aXFillAttr.GetItemSet());
    m_aCtlPreview.Invalidate();
}

IMPL_LINK_NOARG(SvxGradientTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_GRADIENT ) );
    OUString aDesc( CuiResId( RID_CUISTR_DESC_GRADIENT ) );
    OUString aName;

    tools::Long nCount = m_pGradientList->Count();
    tools::Long j = 1;
    bool bValidGradientName = false;

    while( !bValidGradientName )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bValidGradientName = (SearchGradientList(aName) == -1);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
    sal_uInt16 nError   = 1;

    while (pDlg->Execute() == RET_OK)
    {
        aName = pDlg->GetName();

        bValidGradientName = (SearchGradientList(aName) == -1);

        if (bValidGradientName)
        {
            nError = 0;
            break;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
        if (xWarnBox->run() != RET_OK)
            break;
    }
    pDlg.disposeAndClear();

    if( !nError )
    {
        sal_uInt16 nValue = 0; // automatic step count
        if (!m_xCbIncrement->get_active())
            nValue = m_xMtrIncrement->get_value();
        basegfx::BGradient aBGradient(
                              createColorStops(),
                              static_cast<css::awt::GradientStyle>(m_xLbGradientType->get_active()),
                              Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)), // should be changed in resource
                              static_cast<sal_uInt16>(m_xMtrCenterX->get_value(FieldUnit::NONE)),
                              static_cast<sal_uInt16>(m_xMtrCenterY->get_value(FieldUnit::NONE)),
                              static_cast<sal_uInt16>(m_xMtrBorder->get_value(FieldUnit::NONE)),
                              static_cast<sal_uInt16>(m_xMtrColorFrom->get_value(FieldUnit::NONE)),
                              static_cast<sal_uInt16>(m_xMtrColorTo->get_value(FieldUnit::NONE)),
                              nValue);

        m_pGradientList->Insert(std::make_unique<XGradientEntry>(aBGradient, aName), nCount);

        OUString sId = nCount > 0 ? m_xGradientLB->get_id( nCount - 1 ) : OUString();
        sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;
        BitmapEx aBitmap = m_pGradientList->GetBitmapForPreview( nCount, aIconSize );
        VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

        m_xGradientLB->insert( nId + 1, &aName, &sId, pVDev, nullptr);
        FillPresetListBox();
        m_xGradientLB->select( nId + 1 );

        m_nGradientListState |= ChangeType::MODIFIED;

        ChangeGradientHdl_Impl();
    }

    // determine button state
    if (m_pGradientList->Count())
        m_xBtnModify->set_sensitive(true);
}

VclPtr<VirtualDevice> SvxGradientTabPage::GetVirtualDevice(BitmapEx aBitmap)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aBitmap);

    return pVDev;
}

IMPL_LINK_NOARG(SvxGradientTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    OUString sId = m_xGradientLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
        return;

    OUString aName( m_pGradientList->GetGradient( static_cast<sal_uInt16>(nPos) )->GetName() );

    sal_uInt16 nValue = 0; // automatic step count
    if (!m_xCbIncrement->get_active())
        nValue = m_xMtrIncrement->get_value();

    basegfx::BGradient aBGradient(
                          createColorStops(),
                          static_cast<css::awt::GradientStyle>(m_xLbGradientType->get_active()),
                          Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)), // should be changed in resource
                          static_cast<sal_uInt16>(m_xMtrCenterX->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrCenterY->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrBorder->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrColorFrom->get_value(FieldUnit::NONE)),
                          static_cast<sal_uInt16>(m_xMtrColorTo->get_value(FieldUnit::NONE)),
                          nValue);

    m_pGradientList->Replace(std::make_unique<XGradientEntry>(aBGradient, aName), nPos);

    BitmapEx aBitmap = m_pGradientList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), aIconSize );
    VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

    m_xGradientLB->remove( nPos );
    m_xGradientLB->insert( nPos, &aName, &sId, pVDev, nullptr);
    FillPresetListBox();
    m_xGradientLB->select( nPos );

    m_nGradientListState |= ChangeType::MODIFIED;
}

IMPL_LINK(SvxGradientTabPage, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = m_xGradientLB->get_id(rIter);
    sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;

    if (nId >= 0)
    {
        return m_pGradientList->GetGradient(nId)->GetName();
    }
    return OUString();
}

IMPL_LINK(SvxGradientTabPage, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    if (!rMEvt.IsRight())
        return false;

    // Disable context menu for LibreOfficeKit mode
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    const Point& pPos = rMEvt.GetPosPixel();
    for (int i = 0; i < m_xGradientLB->n_children(); i++)
    {
        const ::tools::Rectangle aRect = m_xGradientLB->get_rect(i);
        if (aRect.Contains(pPos))
        {
            ShowContextMenu(pPos);
            break;
        }
    }
    return false;
}

void SvxGradientTabPage::ShowContextMenu(const Point& pPos)
{
    ::tools::Rectangle aRect(pPos, Size(1, 1));
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(m_xGradientLB.get(), u"svx/ui/presetmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));

    xMenu->connect_activate(LINK(this, SvxGradientTabPage, OnPopupEnd));
    xMenu->popup_at_rect(m_xGradientLB.get(), aRect);
}

IMPL_LINK(SvxGradientTabPage, OnPopupEnd, const OUString&, sCommand, void)
{
    sLastItemIdent = sCommand;
    if (sLastItemIdent.isEmpty())
        return;

    Application::PostUserEvent(LINK(this, SvxGradientTabPage, MenuSelectAsyncHdl));
}

IMPL_LINK_NOARG(SvxGradientTabPage, MenuSelectAsyncHdl, void*, void)
{
    if (sLastItemIdent == u"rename")
    {
        ClickRenameHdl();
    }
    else if (sLastItemIdent == u"delete")
    {
        ClickDeleteHdl();
    }
}

void SvxGradientTabPage::ClickDeleteHdl()
{
    const OUString sId = m_xGradientLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos != -1 )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletegradientdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelGradientDialog"_ustr));
        if (xQueryBox->run() == RET_YES)
        {
            m_pGradientList->Remove(nPos);
            m_xGradientLB->remove( nPos );

            FillPresetListBox();

            sal_Int32 nNextId = nPos;
            if (nPos >= m_xGradientLB->n_children())
                nNextId = m_xGradientLB->n_children() - 1;

            if(m_xGradientLB->n_children() > 0)
                m_xGradientLB->select(nNextId);
            m_aCtlPreview.Invalidate();

            ChangeGradientHdl_Impl();

            m_nGradientListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pGradientList->Count() )
        m_xBtnModify->set_sensitive(false);
}

void SvxGradientTabPage::ClickRenameHdl()
{
    const OUString sId = m_xGradientLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
        return;

    OUString aDesc( CuiResId( RID_CUISTR_DESC_GRADIENT ) );
    OUString aName( m_pGradientList->GetGradient( nPos )->GetName() );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));

    bool bLoop = true;
    while( bLoop && pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();
        sal_Int32 nGradientPos = SearchGradientList(aName);
        bool bValidGradientName = (nGradientPos == nPos ) || (nGradientPos == -1);

        if( bValidGradientName )
        {
            bLoop = false;
            m_pGradientList->GetGradient(nPos)->SetName(aName);

            m_xGradientLB->set_text( nPos, aName );

            m_nGradientListState |= ChangeType::MODIFIED;
        }
        else
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
            xBox->run();
        }
    }
}

IMPL_LINK_NOARG(SvxGradientTabPage, ChangeGradientHdl, weld::IconView&, void)
{
    ChangeGradientHdl_Impl();
}

void SvxGradientTabPage::ChangeGradientHdl_Impl()
{
    std::unique_ptr<basegfx::BGradient> pGradient;
    OUString sId = m_xGradientLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos != -1 )
        pGradient.reset(new basegfx::BGradient( m_pGradientList->GetGradient( static_cast<sal_uInt16>( nPos ) )->GetGradient() ));
    else
    {
        if( const XFillStyleItem* pFillStyleItem = m_rOutAttrs.GetItemIfSet( GetWhich( XATTR_FILLSTYLE ) ) )
        {
            const XFillGradientItem* pGradientItem;
            if( ( drawing::FillStyle_GRADIENT == pFillStyleItem->GetValue() ) &&
                ( pGradientItem = m_rOutAttrs.GetItemIfSet( GetWhich( XATTR_FILLGRADIENT ) ) ) )
            {
                pGradient.reset(new basegfx::BGradient( pGradientItem->GetGradientValue() ));
            }
        }
        if(!pGradient && m_xGradientLB->n_children() > 0)
        {
            m_xGradientLB->select(0);
            pGradient.reset(new basegfx::BGradient( m_pGradientList->GetGradient( 0 )->GetGradient() ));
        }
    }

    if( !pGradient )
        return;

    css::awt::GradientStyle eXGS = pGradient->GetGradientStyle();
    sal_uInt16 nValue = pGradient->GetSteps();
    if (const XGradientStepCountItem* pGradientStepCountItem = m_rOutAttrs.GetItemIfSet(GetWhich(XATTR_GRADIENTSTEPCOUNT)))
    {
        nValue = pGradientStepCountItem->GetValue();
    }
    if( nValue == 0 )
    {
        m_xCbIncrement->set_state(TRISTATE_TRUE);
        m_xMtrIncrement->set_sensitive(false);
    }
    else
    {
        m_xCbIncrement->set_state(TRISTATE_FALSE);
        m_xMtrIncrement->set_sensitive(true);
        m_xMtrIncrement->set_value( nValue );
    }
    m_xLbGradientType->set_active(
        sal::static_int_cast< sal_Int32 >( eXGS ) );
    // if the entry is not in the listbox,
    // colors are added temporarily
    m_xLbColorFrom->SetNoSelection();
    m_xLbColorFrom->SelectEntry(Color(pGradient->GetColorStops().front().getStopColor()));

    m_xLbColorTo->SetNoSelection();
    m_xLbColorTo->SelectEntry(Color(pGradient->GetColorStops().back().getStopColor()));

    // MCGR: preserve ColorStops if given.
    // tdf#155901 We need offset of first and last stop, so include them.
    if (pGradient->GetColorStops().size() >= 2)
        m_aColorStops = pGradient->GetColorStops();
    else
        m_aColorStops.clear();

    m_xMtrAngle->set_value(pGradient->GetAngle().get() / 10, FieldUnit::NONE); // should be changed in resource
    m_xSliderAngle->set_value(pGradient->GetAngle().get() / 10);
    m_xMtrBorder->set_value(pGradient->GetBorder(), FieldUnit::NONE);
    m_xSliderBorder->set_value(pGradient->GetBorder());
    m_xMtrCenterX->set_value(pGradient->GetXOffset(), FieldUnit::NONE);
    m_xMtrCenterY->set_value(pGradient->GetYOffset(), FieldUnit::NONE);
    m_xMtrColorFrom->set_value(pGradient->GetStartIntens(), FieldUnit::NONE);
    m_xMtrColorTo->set_value(pGradient->GetEndIntens(), FieldUnit::NONE);

    // disable/enable controls
    SetControlState_Impl( eXGS );

    // fill ItemSet and pass it on to aCtlPreview
    m_rXFSet.Put( XFillGradientItem( OUString(), *pGradient ) );
    m_rXFSet.Put( XGradientStepCountItem( nValue ) );
    m_aCtlPreview.SetAttributes(m_aXFillAttr.GetItemSet());

    m_aCtlPreview.Invalidate();
}

void SvxGradientTabPage::SetControlState_Impl( css::awt::GradientStyle eXGS )
{
    switch( eXGS )
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_xFtCenter->set_sensitive(false);
            m_xMtrCenterX->set_sensitive(false);
            m_xMtrCenterY->set_sensitive(false);
            m_xFtAngle->set_sensitive(true);
            m_xMtrAngle->set_sensitive(true);
            m_xSliderAngle->set_sensitive(true);
            break;

        case css::awt::GradientStyle_RADIAL:
            m_xFtCenter->set_sensitive(true);
            m_xMtrCenterX->set_sensitive(true);
            m_xMtrCenterY->set_sensitive(true);
            m_xFtAngle->set_sensitive(false);
            m_xMtrAngle->set_sensitive(false);
            m_xSliderAngle->set_sensitive(false);
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_xFtCenter->set_sensitive(true);
            m_xMtrCenterX->set_sensitive(true);
            m_xMtrCenterY->set_sensitive(true);
            m_xFtAngle->set_sensitive(true);
            m_xMtrAngle->set_sensitive(true);
            m_xSliderAngle->set_sensitive(true);
            break;

        default:
            break;
    }
}

sal_Int32 SvxGradientTabPage::SearchGradientList(std::u16string_view rGradientName)
{
    tools::Long nCount = m_pGradientList->Count();
    bool bValidGradientName = true;
    sal_Int32 nPos = -1;

    for(tools::Long i = 0;i < nCount && bValidGradientName;i++)
    {
        if(rGradientName == m_pGradientList->GetGradient( i )->GetName())
        {
            nPos = i;
            bValidGradientName = false;
        }
    }
    return nPos;
}

basegfx::BColorStops SvxGradientTabPage::createColorStops()
{
    basegfx::BColorStops aColorStops;

    if(m_aColorStops.size() >= 2)
    {
        aColorStops = m_aColorStops;
        aColorStops.front() = basegfx::BColorStop(m_aColorStops.front().getStopOffset(),
                                                  m_xLbColorFrom->GetSelectEntryColor().getBColor());
        aColorStops.back() = basegfx::BColorStop(m_aColorStops.back().getStopOffset(),
                                                 m_xLbColorTo->GetSelectEntryColor().getBColor());
    }
    else
    {
        aColorStops.emplace_back(0.0, m_xLbColorFrom->GetSelectEntryColor().getBColor());
        aColorStops.emplace_back(1.0, m_xLbColorTo->GetSelectEntryColor().getBColor());
    }

    return aColorStops;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
