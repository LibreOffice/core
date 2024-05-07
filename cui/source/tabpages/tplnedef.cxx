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

#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <strings.hrc>

#include <svx/xlineit0.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xtable.hxx>

#include <svx/dlgctrl.hxx>
#include <cuitabline.hxx>
#include <defdlgname.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <cuitabarea.hxx>
#include <svtools/unitconv.hxx>
#include <osl/diagnose.h>

#define XOUT_WIDTH    150

using namespace com::sun::star;


SvxLineDefTabPage::SvxLineDefTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/linestyletabpage.ui"_ustr, u"LineStylePage"_ustr, &rInAttrs)
    , rOutAttrs(rInAttrs)
    , aXLineAttr(rInAttrs.GetPool())
    , rXLSet(aXLineAttr.GetItemSet())
    , pnDashListState(nullptr)
    , pPageType(nullptr)
    , nDlgType(0)
    , pPosDashLb(nullptr)
    , m_xLbLineStyles(new SvxLineLB(m_xBuilder->weld_combo_box(u"LB_LINESTYLES"_ustr)))
    , m_xLbType1(m_xBuilder->weld_combo_box(u"LB_TYPE_1"_ustr))
    , m_xLbType2(m_xBuilder->weld_combo_box(u"LB_TYPE_2"_ustr))
    , m_xNumFldNumber1(m_xBuilder->weld_spin_button(u"NUM_FLD_1"_ustr))
    , m_xNumFldNumber2(m_xBuilder->weld_spin_button(u"NUM_FLD_2"_ustr))
    , m_xMtrLength1(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_LENGTH_1"_ustr, FieldUnit::CM))
    , m_xMtrLength2(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_LENGTH_2"_ustr, FieldUnit::CM))
    , m_xMtrDistance(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_DISTANCE"_ustr, FieldUnit::CM))
    , m_xCbxSynchronize(m_xBuilder->weld_check_button(u"CBX_SYNCHRONIZE"_ustr))
    , m_xBtnAdd(m_xBuilder->weld_button(u"BTN_ADD"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"BTN_MODIFY"_ustr))
    , m_xBtnDelete(m_xBuilder->weld_button(u"BTN_DELETE"_ustr))
    , m_xBtnLoad(m_xBuilder->weld_button(u"BTN_LOAD"_ustr))
    , m_xBtnSave(m_xBuilder->weld_button(u"BTN_SAVE"_ustr))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"CTL_PREVIEW"_ustr, m_aCtlPreview))
{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FieldUnit::M:
        case FieldUnit::KM:
            eFUnit = FieldUnit::MM;
            break;
        default: ; //prevent warning
    }
    SetFieldUnit(*m_xMtrDistance, eFUnit);
    SetFieldUnit(*m_xMtrLength1, eFUnit);
    SetFieldUnit(*m_xMtrLength2, eFUnit);

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    assert(pPool && "Where is the pool?");
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    rXLSet.Put( XLineStyleItem(drawing::LineStyle_DASH) );
    rXLSet.Put( XLineWidthItem(XOUT_WIDTH) );
    rXLSet.Put( XLineDashItem( OUString(), XDash( css::drawing::DashStyle_RECT, 3, 7, 2, 40, 15 ) ) );
    rXLSet.Put( XLineColorItem(OUString(), COL_BLACK) );

    // #i34740#
    m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

    m_xBtnAdd->connect_clicked(LINK(this, SvxLineDefTabPage, ClickAddHdl_Impl));
    m_xBtnModify->connect_clicked(LINK(this, SvxLineDefTabPage, ClickModifyHdl_Impl));
    m_xBtnDelete->connect_clicked(LINK(this, SvxLineDefTabPage, ClickDeleteHdl_Impl));
    m_xBtnLoad->connect_clicked(LINK(this, SvxLineDefTabPage, ClickLoadHdl_Impl));
    m_xBtnSave->connect_clicked(LINK(this, SvxLineDefTabPage, ClickSaveHdl_Impl));

    m_xNumFldNumber1->connect_value_changed(LINK(this, SvxLineDefTabPage, ChangeNumber1Hdl_Impl));
    m_xNumFldNumber2->connect_value_changed(LINK(this, SvxLineDefTabPage, ChangeNumber2Hdl_Impl));
    m_xLbLineStyles->connect_changed(LINK(this, SvxLineDefTabPage, SelectLinestyleListBoxHdl_Impl));

    // #i122042# switch off default adding of 'none' and 'solid' entries
    // for this ListBox; we want to select only editable/dashed styles
    m_xLbLineStyles->setAddStandardFields(false);

    // absolute (in mm) or relative (in %)
    m_xCbxSynchronize->connect_toggled(LINK(this, SvxLineDefTabPage, ChangeMetricHdl_Impl));

    // preview must be updated when there's something changed
    Link<weld::ComboBox&, void> aLink = LINK(this, SvxLineDefTabPage, SelectTypeListBoxHdl_Impl);
    m_xLbType1->connect_changed(aLink);
    m_xLbType2->connect_changed(aLink);
    Link<weld::MetricSpinButton&,void> aLink2 = LINK( this, SvxLineDefTabPage, ChangePreviewHdl_Impl );
    m_xMtrLength1->connect_value_changed(aLink2);
    m_xMtrLength2->connect_value_changed(aLink2);
    m_xMtrDistance->connect_value_changed(aLink2);

    pDashList = nullptr;
}

SvxLineDefTabPage::~SvxLineDefTabPage()
{
    m_xCtlPreview.reset();
    m_xLbLineStyles.reset();
}

void SvxLineDefTabPage::Construct()
{
    // Line style fill; do *not* add default fields here
    m_xLbLineStyles->Fill( pDashList );
}

void SvxLineDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( nDlgType != 0 ) // area dialog
        return;

    // ActivatePage() is called before the dialog receives PageCreated() !!!
    if( !pDashList.is() )
        return;

    if (*pPageType == PageType::Gradient &&
        *pPosDashLb != -1)
    {
        m_xLbLineStyles->set_active(*pPosDashLb);
    }
    // so that a possibly existing line style is discarded
    SelectLinestyleHdl_Impl( nullptr );

    // determining (and possibly cutting) the name
    // and displaying it in the GroupBox
//             OUString        aString( CuiResId( RID_CUISTR_TABLE ) );
//             aString         += ": ";
    INetURLObject   aURL( pDashList->GetPath() );

    aURL.Append( pDashList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    *pPageType = PageType::Area; // 2
    *pPosDashLb = -1;
}


DeactivateRC SvxLineDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    CheckChanges_Impl();

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

void SvxLineDefTabPage::CheckChanges_Impl()
{
    // is here used to NOT lose changes
    //css::drawing::DashStyle eXDS;

    if( m_xNumFldNumber1->get_value_changed_from_saved() ||
        m_xMtrLength1->get_value_changed_from_saved() ||
        m_xLbType1->get_value_changed_from_saved() ||
        m_xNumFldNumber2->get_value_changed_from_saved() ||
        m_xMtrLength2->get_value_changed_from_saved() ||
        m_xLbType2->get_value_changed_from_saved() ||
        m_xMtrDistance->get_value_changed_from_saved() )
    {
        std::unique_ptr<weld::MessageDialog> xMessDlg(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Warning, VclButtonsType::Cancel,
                                                      CuiResId(RID_CUISTR_ASK_CHANGE_LINESTYLE)));
        xMessDlg->set_title(SvxResId(RID_SVXSTR_LINESTYLE));
        xMessDlg->add_button(CuiResId(RID_CUISTR_CHANGE), RET_BTN_1);
        xMessDlg->add_button(CuiResId(RID_CUISTR_ADD), RET_BTN_2);

        short nRet = xMessDlg->run();

        switch( nRet )
        {
            case RET_BTN_1:
            {
                ClickModifyHdl_Impl(*m_xBtnModify);
            }
            break;

            case RET_BTN_2:
            {
                ClickAddHdl_Impl(*m_xBtnAdd);
            }
            break;

            case RET_CANCEL:
            break;
        }
    }

    int nPos = m_xLbLineStyles->get_active();
    if (nPos != -1)
    {
        *pPosDashLb = nPos;
    }
}


bool SvxLineDefTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    if( nDlgType == 0 ) // line dialog
    {
        if( *pPageType == PageType::Hatch )
        {
            FillDash_Impl();

            OUString aString(m_xLbLineStyles->get_active_text());
            rAttrs->Put( XLineStyleItem( drawing::LineStyle_DASH ) );
            rAttrs->Put( XLineDashItem( aString, aDash ) );
        }
    }
    return true;
}


void SvxLineDefTabPage::Reset( const SfxItemSet* rAttrs )
{
    if( rAttrs->GetItemState( GetWhich( XATTR_LINESTYLE ) ) != SfxItemState::INVALID )
    {
        drawing::LineStyle eXLS = rAttrs->Get( GetWhich( XATTR_LINESTYLE ) ).GetValue();

        switch( eXLS )
        {
            case drawing::LineStyle_NONE:
            case drawing::LineStyle_SOLID:
                m_xLbLineStyles->set_active(0);
                break;
            case drawing::LineStyle_DASH:
            {
                const XLineDashItem& rDashItem = rAttrs->Get( XATTR_LINEDASH );
                aDash = rDashItem.GetDashValue();

                m_xLbLineStyles->set_active(-1);
                m_xLbLineStyles->set_active_text(rDashItem.GetName());
                break;
            }
            default:
                break;
        }
    }
    SelectLinestyleHdl_Impl( nullptr );

    // determine button state
    if( pDashList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
    else
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

std::unique_ptr<SfxTabPage> SvxLineDefTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs )
{
    return std::make_unique<SvxLineDefTabPage>(pPage, pController, *rOutAttrs);
}

IMPL_LINK(SvxLineDefTabPage, SelectLinestyleListBoxHdl_Impl, weld::ComboBox&, rListBox, void)
{
    SelectLinestyleHdl_Impl(&rListBox);
}

void SvxLineDefTabPage::SelectLinestyleHdl_Impl(const weld::ComboBox* p)
{
    if(!pDashList->Count())
        return;

    int nTmp = m_xLbLineStyles->get_active();
    if (nTmp == -1)
    {
        OSL_ENSURE(false, "OOps, non-existent LineDash selected (!)");
        nTmp = 1;
    }

    aDash = pDashList->GetDash( nTmp )->GetDash();

    FillDialog_Impl();

    rXLSet.Put( XLineDashItem( OUString(), aDash ) );

    // #i34740#
    m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());
    m_aCtlPreview.Invalidate();

    // Is not set before, in order to take the new style
    // only if there was an entry selected in the ListBox.
    // If it was called via Reset(), then p is == NULL
    if( p )
        *pPageType = PageType::Hatch;
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangePreviewHdl_Impl, weld::MetricSpinButton&, void)
{
    FillDash_Impl();
    m_aCtlPreview.Invalidate();
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber1Hdl_Impl, weld::SpinButton&, void)
{
    if (m_xNumFldNumber1->get_value() == 0)
    {
        m_xNumFldNumber2->set_min(1);
    }
    else
    {
        m_xNumFldNumber2->set_min(0);
    }

    ChangePreviewHdl_Impl(*m_xMtrLength1);
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber2Hdl_Impl, weld::SpinButton&, void)
{
    if (m_xNumFldNumber2->get_value() == 0)
    {
        m_xNumFldNumber1->set_min(1);
    }
    else
    {
        m_xNumFldNumber1->set_min(0);
    }

    ChangePreviewHdl_Impl(*m_xMtrLength1);
}

IMPL_LINK( SvxLineDefTabPage, ChangeMetricHdl_Impl, weld::Toggleable&, r, void)
{
    ChangeMetricHdl_Impl(&r);
}

void SvxLineDefTabPage::ChangeMetricHdl_Impl(const weld::Toggleable* p)
{
    if( !m_xCbxSynchronize->get_active() && m_xMtrLength1->get_unit() != eFUnit )
    {
        tools::Long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_xMtrLength1, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp2 = GetCoreValue( *m_xMtrLength2, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp3 = GetCoreValue( *m_xMtrDistance, ePoolUnit ) * XOUT_WIDTH / 100;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_xMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_xMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_xMtrDistance, ePoolUnit );
        }
        m_xMtrLength1->set_digits(2);
        m_xMtrLength2->set_digits(2);
        m_xMtrDistance->set_digits(2);

        // adjust metric
        m_xMtrLength1->set_unit(eFUnit);
        m_xMtrLength2->set_unit(eFUnit);
        m_xMtrDistance->set_unit(eFUnit);

        // tdf#126736 max 5cm
        m_xMtrLength1->set_range(0, 500, FieldUnit::CM);
        m_xMtrLength2->set_range(0, 500, FieldUnit::CM);
        m_xMtrDistance->set_range(0, 500, FieldUnit::CM);

        SetMetricValue( *m_xMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( *m_xMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( *m_xMtrDistance, nTmp3, ePoolUnit );
    }
    else if( m_xCbxSynchronize->get_active() && m_xMtrLength1->get_unit() != FieldUnit::PERCENT )
    {
        tools::Long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_xMtrLength1, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp2 = GetCoreValue( *m_xMtrLength2, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp3 = GetCoreValue( *m_xMtrDistance, ePoolUnit ) * 100 / XOUT_WIDTH;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_xMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_xMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_xMtrDistance, ePoolUnit );
        }

        m_xMtrLength1->set_digits(0);
        m_xMtrLength2->set_digits(0);
        m_xMtrDistance->set_digits(0);

        m_xMtrLength1->set_unit(FieldUnit::PERCENT);
        m_xMtrLength2->set_unit(FieldUnit::PERCENT);
        m_xMtrDistance->set_unit(FieldUnit::PERCENT);

        // tdf#126736 800%
        m_xMtrLength1->set_range(0, 800, FieldUnit::PERCENT);
        m_xMtrLength2->set_range(0, 800, FieldUnit::PERCENT);
        m_xMtrDistance->set_range(0, 800, FieldUnit::PERCENT);

        m_xMtrLength1->set_value(nTmp1, FieldUnit::PERCENT);
        m_xMtrLength2->set_value(nTmp2, FieldUnit::PERCENT);
        m_xMtrDistance->set_value(nTmp3, FieldUnit::PERCENT);
    }
    SelectTypeHdl_Impl( nullptr );
}

IMPL_LINK( SvxLineDefTabPage, SelectTypeListBoxHdl_Impl, weld::ComboBox&, rListBox, void )
{
    SelectTypeHdl_Impl(&rListBox);
}

void  SvxLineDefTabPage::SelectTypeHdl_Impl(const weld::ComboBox* p)
{
    if (p == m_xLbType1.get() || !p)
    {
        if (m_xLbType1->get_active() == 0)
        {
            m_xMtrLength1->set_sensitive(false);
            m_xMtrLength1->set_text(u""_ustr);
        }
        else if (!m_xMtrLength1->get_sensitive())
        {
            m_xMtrLength1->set_sensitive(true);
            m_xMtrLength1->reformat();
        }
    }

    if (p == m_xLbType2.get() || !p)
    {
        if (m_xLbType2->get_active() == 0)
        {
            m_xMtrLength2->set_sensitive(false);
            m_xMtrLength2->set_text(u""_ustr);
        }
        else if (!m_xMtrLength2->get_sensitive())
        {
            m_xMtrLength2->set_sensitive(true);
            m_xMtrLength2->reformat();
        }
    }
    ChangePreviewHdl_Impl(*m_xMtrLength1);
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    OUString aNewName(SvxResId(RID_SVXSTR_LINESTYLE));
    OUString aDesc(CuiResId(RID_CUISTR_DESC_LINESTYLE));
    OUString aName;

    tools::Long nCount = pDashList->Count();
    tools::Long j = 1;
    bool bDifferent = false;

    while ( !bDifferent )
    {
        aName = aNewName + " " + OUString::number( j++ );
        bDifferent = true;

        for ( tools::Long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
    bool bLoop = true;

    while ( bLoop && pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();
        bDifferent = true;

        for( tools::Long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = false;
        }

        if( bDifferent )
        {
            bLoop = false;
            FillDash_Impl();

            tools::Long nDashCount = pDashList->Count();
            pDashList->Insert( std::make_unique<XDashEntry>(aDash, aName), nDashCount );
            m_xLbLineStyles->Append( *pDashList->GetDash(nDashCount), pDashList->GetUiBitmap(nDashCount) );

            m_xLbLineStyles->set_active(m_xLbLineStyles->get_count() - 1);

            *pnDashListState |= ChangeType::MODIFIED;

            *pPageType = PageType::Hatch;

            // save values for changes recognition (-> method)
            m_xNumFldNumber1->save_value();
            m_xMtrLength1->save_value();
            m_xLbType1->save_value();
            m_xNumFldNumber2->save_value();
            m_xMtrLength2->save_value();
            m_xLbType2->save_value();
            m_xMtrDistance->save_value();
        }
        else
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
            xBox->run();
        }
    }
    pDlg.disposeAndClear();

    // determine button state
    if ( pDashList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    int nPos = m_xLbLineStyles->get_active();
    if (nPos == -1)
        return;

    OUString aDesc(CuiResId(RID_CUISTR_DESC_LINESTYLE));
    OUString aName( pDashList->GetDash( nPos )->GetName() );
    OUString aOldName = aName;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));

    tools::Long nCount = pDashList->Count();
    bool bLoop = true;

    while ( bLoop && pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();
        bool bDifferent = true;

        for( tools::Long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pDashList->GetDash( i )->GetName() &&
                aName != aOldName )
                bDifferent = false;
        }

        if ( bDifferent )
        {
            bLoop = false;
            FillDash_Impl();

            pDashList->Replace(std::make_unique<XDashEntry>(aDash, aName), nPos);
            m_xLbLineStyles->Modify(*pDashList->GetDash(nPos), nPos, pDashList->GetUiBitmap(nPos));

            m_xLbLineStyles->set_active(nPos);

            *pnDashListState |= ChangeType::MODIFIED;

            *pPageType = PageType::Hatch;

            // save values for changes recognition (-> method)
            m_xNumFldNumber1->save_value();
            m_xMtrLength1->save_value();
            m_xLbType1->save_value();
            m_xNumFldNumber2->save_value();
            m_xMtrLength2->save_value();
            m_xLbType2->save_value();
            m_xMtrDistance->save_value();
        }
        else
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
            xBox->run();
        }
    }
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickDeleteHdl_Impl, weld::Button&, void)
{
    int nPos = m_xLbLineStyles->get_active();
    if (nPos != -1)
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletelinestyledialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelLineStyleDialog"_ustr));
        if (xQueryBox->run() == RET_YES)
        {
            pDashList->Remove(nPos);
            m_xLbLineStyles->remove(nPos);
            m_xLbLineStyles->set_active(0);

            SelectLinestyleHdl_Impl( nullptr );
            *pPageType = PageType::Area; // style should not be taken

            *pnDashListState |= ChangeType::MODIFIED;

            ChangePreviewHdl_Impl( *m_xMtrLength1 );
        }
    }

    // determine button state
    if ( !pDashList->Count() )
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickLoadHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nReturn = RET_YES;

    if ( *pnDashListState & ChangeType::MODIFIED )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querysavelistdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"AskSaveList"_ustr));

        nReturn = xBox->run();

        if ( nReturn == RET_YES )
            pDashList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                      FileDialogFlags::NONE, GetFrameWeld());
        OUString aStrFilterType( u"*.sod"_ustr );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        OUString aPalettePath(SvtPathOptions().GetPalettePath());
        OUString aLastDir;
        sal_Int32 nIndex = 0;
        do
        {
            aLastDir = aPalettePath.getToken(0, ';', nIndex);
        }
        while (nIndex >= 0);

        INetURLObject aFile(aLastDir);
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XDashListRef pDshLst = XPropertyList::AsDashList(XPropertyList::CreatePropertyList( XPropertyListType::Dash, aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), u""_ustr ));
            pDshLst->SetName( aURL.getName() );

            if( pDshLst->Load() )
            {
                pDashList = pDshLst;
                static_cast<SvxLineTabDialog*>(GetDialogController())->SetNewDashList( pDashList );

                m_xLbLineStyles->clear();
                m_xLbLineStyles->Fill( pDashList );
                Reset( &rOutAttrs );

                pDashList->SetName( aURL.getName() );

                *pnDashListState |= ChangeType::CHANGED;
                *pnDashListState &= ~ChangeType::MODIFIED;
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querynoloadedfiledialog.ui"_ustr));
                std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"NoLoadedFileDialog"_ustr));
                xBox->run();
            }
        }
    }

    // determine button state
    if ( pDashList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
    else
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickSaveHdl_Impl, weld::Button&, void)
{
    ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
    OUString aStrFilterType( u"*.sod"_ustr );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aFile(aLastDir);
    DBG_ASSERT( aFile.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if( !pDashList->GetName().isEmpty() )
    {
        aFile.Append( pDashList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( u"sod" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if ( aDlg.Execute() != ERRCODE_NONE )
        return;

    INetURLObject aURL( aDlg.GetPath() );
    INetURLObject aPathURL( aURL );

    aPathURL.removeSegment();
    aPathURL.removeFinalSlash();

    pDashList->SetName( aURL.getName() );
    pDashList->SetPath( aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if( pDashList->Save() )
    {
        *pnDashListState &= ~ChangeType::MODIFIED;
    }
    else
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querynosavefiledialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"NoSaveFileDialog"_ustr));
        xBox->run();
    }
}

void SvxLineDefTabPage::FillDash_Impl()
{
    css::drawing::DashStyle eXDS;

    if (m_xCbxSynchronize->get_active())
        eXDS = css::drawing::DashStyle_RECTRELATIVE;
    else
        eXDS = css::drawing::DashStyle_RECT;

    aDash.SetDashStyle( eXDS );
    aDash.SetDots( static_cast<sal_uInt8>(m_xNumFldNumber1->get_value()) );
    aDash.SetDotLen( m_xLbType1->get_active() == 0 ? 0 : GetCoreValue( *m_xMtrLength1, ePoolUnit ) );
    aDash.SetDashes( static_cast<sal_uInt8>(m_xNumFldNumber2->get_value()) );
    aDash.SetDashLen( m_xLbType2->get_active() == 0 ? 0 : GetCoreValue( *m_xMtrLength2, ePoolUnit ) );
    aDash.SetDistance( GetCoreValue( *m_xMtrDistance, ePoolUnit ) );

    rXLSet.Put( XLineDashItem( OUString(), aDash ) );

    // #i34740#
    m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());
}

void SvxLineDefTabPage::FillDialog_Impl()
{
    css::drawing::DashStyle eXDS = aDash.GetDashStyle(); // css::drawing::DashStyle_RECT, css::drawing::DashStyle_ROUND
    if( eXDS == css::drawing::DashStyle_RECTRELATIVE )
        m_xCbxSynchronize->set_active(true);
    else
        m_xCbxSynchronize->set_active(false);

    m_xNumFldNumber1->set_value(aDash.GetDots());
    SetMetricValue( *m_xMtrLength1, aDash.GetDotLen(), ePoolUnit );
    m_xLbType1->set_active(aDash.GetDotLen() == 0 ? 0 : 1);
    m_xNumFldNumber2->set_value(aDash.GetDashes());
    SetMetricValue( *m_xMtrLength2, aDash.GetDashLen(), ePoolUnit );
    m_xLbType2->set_active(aDash.GetDashLen() == 0 ? 0 : 1);
    SetMetricValue( *m_xMtrDistance, aDash.GetDistance(), ePoolUnit );

    ChangeMetricHdl_Impl(nullptr);

    // save values for changes recognition (-> method)
    m_xNumFldNumber1->save_value();
    m_xMtrLength1->save_value();
    m_xLbType1->save_value();
    m_xNumFldNumber2->save_value();
    m_xMtrLength2->save_value();
    m_xLbType2->save_value();
    m_xMtrDistance->save_value();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
