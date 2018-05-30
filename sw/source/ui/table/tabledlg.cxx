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

#include <hintids.hxx>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>

#include <svtools/htmlcfg.hxx>
#include <fmtrowsplt.hxx>
#include <sfx2/htmlmode.hxx>

#include <strings.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>
#include <tabledlg.hxx>
#include "../../uibase/table/tablepg.hxx"
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#include <app.hrc>
#include <cmdid.h>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>

using namespace ::com::sun::star;

SwFormatTablePage::SwFormatTablePage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/formattablepage.ui", "FormatTablePage", &rSet)
    , pTableData(nullptr)
    , nSaveWidth(0)
    , nMinTableWidth(MINLAY)
    , bModified(false)
    , bFull(false)
    , bHtmlMode(false)
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xWidthFT(m_xBuilder->weld_label("widthft"))
    , m_xWidthMF(new SwPercentField(m_xBuilder->weld_metric_spin_button("widthmf", FUNIT_CM)))
    , m_xRelWidthCB(m_xBuilder->weld_check_button("relwidth"))
    , m_xFullBtn(m_xBuilder->weld_radio_button("full"))
    , m_xLeftBtn(m_xBuilder->weld_radio_button("left"))
    , m_xFromLeftBtn(m_xBuilder->weld_radio_button("fromleft"))
    , m_xRightBtn(m_xBuilder->weld_radio_button("right"))
    , m_xCenterBtn(m_xBuilder->weld_radio_button("center"))
    , m_xFreeBtn(m_xBuilder->weld_radio_button("free"))
    , m_xLeftFT(m_xBuilder->weld_label("leftft"))
    , m_xLeftMF(new SwPercentField(m_xBuilder->weld_metric_spin_button("leftmf", FUNIT_CM)))
    , m_xRightFT(m_xBuilder->weld_label("rightft"))
    , m_xRightMF(new SwPercentField(m_xBuilder->weld_metric_spin_button("rightmf", FUNIT_CM)))
    , m_xTopFT(m_xBuilder->weld_label("aboveft"))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button("abovemf", FUNIT_CM))
    , m_xBottomFT(m_xBuilder->weld_label("belowft"))
    , m_xBottomMF(m_xBuilder->weld_metric_spin_button("belowmf", FUNIT_CM))
    , m_xTextDirectionLB(new svx::SvxFrameDirectionListBox(m_xBuilder->weld_combo_box_text("textdirection")))
    , m_xProperties(m_xBuilder->weld_widget("properties"))
{
    //lock these to initial sizes so they don't change on percent to non percent change
    Size aPrefSize(m_xLeftMF->get()->get_preferred_size());
    m_xLeftMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    m_xRightMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    m_xWidthMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());

    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xTextDirectionLB->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));

    SetExchangeSupport();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem))
        bHtmlMode = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);

    bool bCTL = SW_MOD()->GetCTLOptions().IsCTLFontEnabled();
    m_xProperties->show(!bHtmlMode && bCTL);

    Init();
}

SwFormatTablePage::~SwFormatTablePage()
{
}

void  SwFormatTablePage::Init()
{
    m_xLeftMF->SetMetricFieldMin(-999999);
    m_xRightMF->SetMetricFieldMin(-999999);

    //handler
    Link<weld::ToggleButton&,void> aLk2 = LINK( this, SwFormatTablePage, AutoClickHdl );
    m_xFullBtn->connect_toggled( aLk2 );
    m_xFreeBtn->connect_toggled( aLk2 );
    m_xLeftBtn->connect_toggled( aLk2 );
    m_xFromLeftBtn->connect_toggled( aLk2 );
    m_xRightBtn->connect_toggled( aLk2 );
    m_xCenterBtn->connect_toggled( aLk2 );

    Link<weld::MetricSpinButton&,void> aLk = LINK( this, SwFormatTablePage, UpDownHdl );
    m_xTopMF->connect_value_changed(aLk);
    m_xBottomMF->connect_value_changed(aLk);
    m_xRightMF->connect_value_changed(aLk);
    m_xLeftMF->connect_value_changed(aLk);
    m_xWidthMF->connect_value_changed(aLk);

    m_xRelWidthCB->connect_toggled(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

IMPL_LINK( SwFormatTablePage, RelWidthClickHdl, weld::ToggleButton&, rBtn, void )
{
    OSL_ENSURE(pTableData, "table data not available?");
    bool bIsChecked = rBtn.get_active();
    sal_Int64 nLeft  = m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FUNIT_TWIP));
    sal_Int64 nRight = m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FUNIT_TWIP));
    m_xWidthMF->ShowPercent(bIsChecked);
    m_xLeftMF->ShowPercent(bIsChecked);
    m_xRightMF->ShowPercent(bIsChecked);

    if (bIsChecked)
    {
        m_xWidthMF->SetRefValue(pTableData->GetSpace());
        m_xLeftMF->SetRefValue(pTableData->GetSpace());
        m_xRightMF->SetRefValue(pTableData->GetSpace());
        m_xLeftMF->SetMetricFieldMin(0); //will be overwritten by the Percentfield
        m_xRightMF->SetMetricFieldMin(0); //dito
        m_xLeftMF->SetMetricFieldMax(99);
        m_xRightMF->SetMetricFieldMax(99);
        m_xLeftMF->SetPrcntValue(m_xLeftMF->NormalizePercent(nLeft ), FUNIT_TWIP );
        m_xRightMF->SetPrcntValue(m_xRightMF->NormalizePercent(nRight ), FUNIT_TWIP );
    }
    else
        ModifyHdl(*m_xLeftMF->get());    //correct values again

    if (m_xFreeBtn->get_active())
    {
        bool bEnable = !rBtn.get_active();
        m_xRightMF->set_sensitive(bEnable);
        m_xRightFT->set_sensitive(bEnable);
    }
    bModified = true;
}

IMPL_LINK(SwFormatTablePage, AutoClickHdl, weld::ToggleButton&, rControl, void)
{
    bool bRestore = true,
         bLeftEnable = false,
         bRightEnable= false,
         bWidthEnable= false,
         bOthers = true;
    if (&rControl == m_xFullBtn.get())
    {
        m_xLeftMF->SetPrcntValue(0);
        m_xRightMF->SetPrcntValue(0);
        nSaveWidth = static_cast<SwTwips>(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FUNIT_TWIP)));
        m_xWidthMF->SetPrcntValue(m_xWidthMF->NormalizePercent(pTableData->GetSpace()), FUNIT_TWIP);
        bFull = true;
        bRestore = false;
    }
    else if (&rControl == m_xLeftBtn.get())
    {
        bRightEnable = bWidthEnable = true;
        m_xLeftMF->SetPrcntValue(0);
    }
    else if (&rControl == m_xFromLeftBtn.get())
    {
        bLeftEnable = bWidthEnable = true;
        m_xRightMF->SetPrcntValue(0);
    }
    else if (&rControl == m_xRightBtn.get())
    {
        bLeftEnable = bWidthEnable = true;
        m_xRightMF->SetPrcntValue(0);
    }
    else if (&rControl == m_xCenterBtn.get())
    {
        bLeftEnable = bWidthEnable = true;
    }
    else if (&rControl == m_xFreeBtn.get())
    {
        RightModify();
        bLeftEnable = true;
        bWidthEnable = true;
        bOthers = false;
    }
    m_xLeftMF->set_sensitive(bLeftEnable);
    m_xLeftFT->set_sensitive(bLeftEnable);
    m_xWidthMF->set_sensitive(bWidthEnable);
    m_xWidthFT->set_sensitive(bWidthEnable);
    if ( bOthers )
    {
        m_xRightMF->set_sensitive(bRightEnable);
        m_xRightFT->set_sensitive(bRightEnable);
        m_xRelWidthCB->set_sensitive(bWidthEnable);
    }

    if(bFull && bRestore)
    {
        //After being switched on automatic, the width was pinned
        //in order to restore the width while switching back to.
        bFull = false;
        m_xWidthMF->SetPrcntValue(m_xWidthMF->NormalizePercent(nSaveWidth ), FUNIT_TWIP );
    }
    ModifyHdl(*m_xWidthMF->get());
    bModified = true;
}

void SwFormatTablePage::RightModify()
{
    if (m_xFreeBtn->get_active())
    {
        bool bEnable = m_xRightMF->get_value() == 0;
        m_xRelWidthCB->set_sensitive(bEnable);
        if ( !bEnable )
        {
            m_xRelWidthCB->set_active(false);
            RelWidthClickHdl(*m_xRelWidthCB);
        }
        bEnable = m_xRelWidthCB->get_active();
        m_xRightMF->set_sensitive(!bEnable);
        m_xRightFT->set_sensitive(!bEnable);
    }
}

IMPL_LINK( SwFormatTablePage, UpDownHdl, weld::MetricSpinButton&, rEdit, void )
{
    if( m_xRightMF->get() == &rEdit)
        RightModify();
    ModifyHdl(rEdit);
}

void  SwFormatTablePage::ModifyHdl(const weld::MetricSpinButton& rEdit)
{
    SwTwips nCurWidth  = static_cast< SwTwips >(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FUNIT_TWIP)));
    SwTwips nPrevWidth = nCurWidth;
    SwTwips nRight = static_cast< SwTwips >(m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FUNIT_TWIP)));
    SwTwips nLeft  = static_cast< SwTwips >(m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FUNIT_TWIP)));
    SwTwips nDiff;

    if (&rEdit == m_xWidthMF->get())
    {
        if( nCurWidth < MINLAY )
            nCurWidth = MINLAY;
        nDiff = nRight + nLeft + nCurWidth - pTableData->GetSpace() ;
        //right aligned: only change the left margin
        if (m_xRightBtn->get_active())
            nLeft -= nDiff;
        //left aligned: only change the right margin
        else if(m_xLeftBtn->get_active())
            nRight -= nDiff;
        //left margin and width allowed - first right - then left
        else if (m_xFromLeftBtn->get_active())
        {
            if( nRight >= nDiff )
                nRight -= nDiff;
            else
            {
                nDiff -= nRight;
                nRight = 0;
                if(nLeft >= nDiff)
                    nLeft -= nDiff;
                else
                {
                    nRight += nLeft - nDiff;
                    nLeft = 0;
                    nCurWidth = pTableData->GetSpace();
                }

            }
        }
        //centered: change both sides equally
        else if (m_xCenterBtn->get_active())
        {
            if(nLeft != nRight)
            {
                nDiff += nLeft + nRight;
                nLeft = nDiff/2;
                nRight = nDiff/2;
            }
            else
            {
                    nLeft -= nDiff/2;
                    nRight -= nDiff/2;
            }
        }
        //free alignment: decrease both margins
        else if (m_xFreeBtn->get_active())
        {
            nLeft -= nDiff/2;
            nRight -= nDiff/2;
        }
    }
    if (&rEdit == m_xRightMF->get())
    {

        if( nRight + nLeft > pTableData->GetSpace() - MINLAY )
            nRight = pTableData->GetSpace() -nLeft - MINLAY;

        nCurWidth = pTableData->GetSpace() - nLeft - nRight;
    }
    if (&rEdit == m_xLeftMF->get())
    {
        if(!m_xFromLeftBtn->get_active())
        {
            bool bCenter = m_xCenterBtn->get_active();
            if( bCenter )
                nRight = nLeft;
            if(nRight + nLeft > pTableData->GetSpace() - MINLAY )
            {
                nLeft  = bCenter ?  (pTableData->GetSpace() - MINLAY) /2 :
                                    (pTableData->GetSpace() - MINLAY) - nRight;
                nRight = bCenter ?  (pTableData->GetSpace() - MINLAY) /2 : nRight;
            }
            nCurWidth = pTableData->GetSpace() - nLeft - nRight;
        }
        else
        {
            //Upon changes on the left side the right margin will be changed at first,
            //thereafter the width.
            nDiff = nRight + nLeft + nCurWidth - pTableData->GetSpace() ;

            nRight -= nDiff;
            nCurWidth = pTableData->GetSpace() - nLeft - nRight;
        }
    }
    if (nCurWidth != nPrevWidth )
        m_xWidthMF->SetPrcntValue( m_xWidthMF->NormalizePercent( nCurWidth ), FUNIT_TWIP );
    m_xRightMF->SetPrcntValue( m_xRightMF->NormalizePercent( nRight ), FUNIT_TWIP );
    m_xLeftMF->SetPrcntValue( m_xLeftMF->NormalizePercent( nLeft ), FUNIT_TWIP );
    bModified = true;
}

VclPtr<SfxTabPage> SwFormatTablePage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SwFormatTablePage>::Create(pParent, *rAttrSet);
}

bool  SwFormatTablePage::FillItemSet( SfxItemSet* rCoreSet )
{
    //Test if one of the controls still has the focus
    if (m_xWidthMF->has_focus())
        ModifyHdl(*m_xWidthMF->get());
    else if (m_xLeftMF->has_focus())
        ModifyHdl(*m_xLeftMF->get());
    else if (m_xRightMF->has_focus())
        ModifyHdl(*m_xRightMF->get());
    else if (m_xTopMF->has_focus())
        ModifyHdl(*m_xTopMF);
    else if (m_xBottomMF->has_focus())
        ModifyHdl(*m_xBottomMF);

    if (bModified)
    {
        if (m_xBottomMF->get_value_changed_from_saved() ||
            m_xTopMF->get_value_changed_from_saved() )
        {
            SvxULSpaceItem aULSpace(RES_UL_SPACE);
            aULSpace.SetUpper(m_xTopMF->denormalize(m_xTopMF->get_value(FUNIT_TWIP)));
            aULSpace.SetLower(m_xBottomMF->denormalize(m_xBottomMF->get_value(FUNIT_TWIP)));
            rCoreSet->Put(aULSpace);
        }

    }
    if (m_xNameED->get_value_changed_from_saved())
    {
        rCoreSet->Put(SfxStringItem(FN_PARAM_TABLE_NAME, m_xNameED->get_text()));
        bModified = true;
    }

    if (m_xTextDirectionLB->get_visible())
    {
        if (m_xTextDirectionLB->get_value_changed_from_saved())
        {
            SvxFrameDirection nDirection = m_xTextDirectionLB->get_active_id();
            rCoreSet->Put(SvxFrameDirectionItem(nDirection, RES_FRAMEDIR));
            bModified = true;
        }
    }

    return bModified;
}

void  SwFormatTablePage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();
    const SfxPoolItem*  pItem;

    if(bHtmlMode)
    {
        m_xNameED->set_sensitive(false);
        m_xTopFT->hide();
        m_xTopMF->hide();
        m_xBottomFT->hide();
        m_xBottomMF->hide();
        m_xFreeBtn->set_sensitive(false);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    m_xWidthMF->SetMetric(aMetric);
    m_xRightMF->SetMetric(aMetric);
    m_xLeftMF->SetMetric(aMetric);
    SetFieldUnit(*m_xTopMF, aMetric);
    SetFieldUnit(*m_xBottomMF, aMetric);

    //Name
    if(SfxItemState::SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, false, &pItem ))
    {
        m_xNameED->set_text(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_xNameED->save_value();
    }

    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP, false, &pItem ))
    {
        pTableData = static_cast<SwTableRep*>(static_cast<const SwPtrItem*>( pItem)->GetValue());
        nMinTableWidth = pTableData->GetColCount() * MINLAY;

        if(pTableData->GetWidthPercent())
        {
            m_xRelWidthCB->set_active(true);
            RelWidthClickHdl(*m_xRelWidthCB);
            m_xWidthMF->SetPrcntValue(pTableData->GetWidthPercent(), FUNIT_PERCENT);

            m_xWidthMF->save_value();
            nSaveWidth = static_cast< SwTwips >(m_xWidthMF->get_value(FUNIT_PERCENT));
        }
        else
        {
            m_xWidthMF->SetPrcntValue(m_xWidthMF->NormalizePercent(
                    pTableData->GetWidth()), FUNIT_TWIP);
            m_xWidthMF->save_value();
            nSaveWidth = pTableData->GetWidth();
            nMinTableWidth = std::min( nSaveWidth, nMinTableWidth );
        }

        m_xWidthMF->SetRefValue(pTableData->GetSpace());

        m_xLeftMF->SetPrcntValue(m_xLeftMF->NormalizePercent(
                    pTableData->GetLeftSpace()), FUNIT_TWIP);
        m_xRightMF->SetPrcntValue(m_xRightMF->NormalizePercent(
                    pTableData->GetRightSpace()), FUNIT_TWIP);
        m_xLeftMF->save_value();
        m_xRightMF->save_value();

        bool bSetRight = false, bSetLeft = false;
        switch( pTableData->GetAlign() )
        {
            case text::HoriOrientation::NONE:
                m_xFreeBtn->set_active(true);
                if (m_xRelWidthCB->get_active())
                    bSetRight = true;
            break;
            case text::HoriOrientation::FULL:
            {
                bSetRight = bSetLeft = true;
                m_xFullBtn->set_active(true);
                m_xWidthMF->set_sensitive(false);
                m_xRelWidthCB->set_sensitive(false);
                m_xWidthFT->set_sensitive(false);
            }
            break;
            case text::HoriOrientation::LEFT:
            {
                bSetLeft = true;
                m_xLeftBtn->set_active(true);
            }
            break;
            case text::HoriOrientation::LEFT_AND_WIDTH :
            {
                bSetRight = true;
                m_xFromLeftBtn->set_active(true);
            }
            break;
            case text::HoriOrientation::RIGHT:
            {
                bSetRight = true;
                m_xRightBtn->set_active(true);
            }
            break;
            case text::HoriOrientation::CENTER:
            {
                bSetRight = true;
                m_xCenterBtn->set_active(true);
            }
            break;
        }
        if ( bSetRight )
        {
            m_xRightMF->set_sensitive(false);
            m_xRightFT->set_sensitive(false);
        }
        if ( bSetLeft )
        {
            m_xLeftMF->set_sensitive(false);
            m_xLeftFT->set_sensitive(false);
        }

    }

    //Margins
    if(SfxItemState::SET == rSet.GetItemState( RES_UL_SPACE, false,&pItem ))
    {
        m_xTopMF->set_value(m_xTopMF->normalize(
                        static_cast<const SvxULSpaceItem*>(pItem)->GetUpper()), FUNIT_TWIP);
        m_xBottomMF->set_value(m_xBottomMF->normalize(
                        static_cast<const SvxULSpaceItem*>(pItem)->GetLower()), FUNIT_TWIP);
        m_xTopMF->save_value();
        m_xBottomMF->save_value();
    }

    //Text direction
    if( SfxItemState::SET == rSet.GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        SvxFrameDirection nVal = static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue();
        m_xTextDirectionLB->set_active_id(nVal);
        m_xTextDirectionLB->save_value();
    }

    m_xWidthMF->set_max( 2*m_xWidthMF->NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_xRightMF->set_max( m_xRightMF->NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_xLeftMF->set_max( m_xLeftMF->NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_xWidthMF->set_min( m_xWidthMF->NormalizePercent( nMinTableWidth ), FUNIT_TWIP );
}

void    SwFormatTablePage::ActivatePage( const SfxItemSet& rSet )
{
    OSL_ENSURE(pTableData, "table data not available?");
    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP ))
    {
        SwTwips nCurWidth = text::HoriOrientation::FULL != pTableData->GetAlign() ?
                                        pTableData->GetWidth() :
                                            pTableData->GetSpace();
        if(pTableData->GetWidthPercent() == 0 &&
                nCurWidth != m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FUNIT_TWIP)))
        {
            m_xWidthMF->SetPrcntValue(m_xWidthMF->NormalizePercent(
                            nCurWidth), FUNIT_TWIP);
            m_xWidthMF->save_value();
            nSaveWidth = nCurWidth;
            m_xLeftMF->SetPrcntValue(m_xLeftMF->NormalizePercent(
                            pTableData->GetLeftSpace()), FUNIT_TWIP);
            m_xLeftMF->save_value();
            m_xRightMF->SetPrcntValue(m_xRightMF->NormalizePercent(
                            pTableData->GetRightSpace()), FUNIT_TWIP);
            m_xRightMF->save_value();
        }
    }
}

DeactivateRC SwFormatTablePage::DeactivatePage( SfxItemSet* _pSet )
{
    //os: VCL doesn't take care of making the active widget
    //in the dialog lose the focus
    m_xNameED->grab_focus();
    //test the table name for spaces
    OUString sTableName = m_xNameED->get_text();
    if(sTableName.indexOf(' ') != -1)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(STR_WRONG_TABLENAME)));
        xInfoBox->run();
        m_xNameED->grab_focus();
        return DeactivateRC::KeepPage;
    }
    if(_pSet)
    {
        FillItemSet(_pSet);
        if(bModified)
        {
            SwTwips lLeft  = static_cast< SwTwips >(m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FUNIT_TWIP)));
            SwTwips lRight = static_cast< SwTwips >(m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FUNIT_TWIP)));

            if( m_xLeftMF->get_value_changed_from_saved() ||
                m_xRightMF->get_value_changed_from_saved() )
            {
                pTableData->SetWidthChanged();
                pTableData->SetLeftSpace( lLeft);
                pTableData->SetRightSpace( lRight);
            }

            SwTwips lWidth;
            if (m_xRelWidthCB->get_active() && m_xRelWidthCB->get_sensitive())
            {
                lWidth = pTableData->GetSpace() - lRight - lLeft;
                const sal_uInt16 nPercentWidth = m_xWidthMF->get_value(FUNIT_PERCENT);
                if(pTableData->GetWidthPercent() != nPercentWidth)
                {
                    pTableData->SetWidthPercent(nPercentWidth);
                    pTableData->SetWidthChanged();
                }
            }
            else
            {
                pTableData->SetWidthPercent(0);
                lWidth = static_cast<SwTwips>(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FUNIT_TWIP)));
            }
            pTableData->SetWidth(lWidth);

            SwTwips nColSum = 0;

            for( sal_uInt16 i = 0; i < pTableData->GetColCount(); i++)
            {
                nColSum += pTableData->GetColumns()[i].nWidth;
            }
            if(nColSum != pTableData->GetWidth())
            {
                SwTwips nMinWidth = std::min( long(MINLAY),
                                    static_cast<long>(pTableData->GetWidth() /
                                            pTableData->GetColCount() - 1));
                SwTwips nDiff = nColSum - pTableData->GetWidth();
                while ( std::abs(nDiff) > pTableData->GetColCount() + 1 )
                {
                    SwTwips nSub = nDiff / pTableData->GetColCount();
                    for( sal_uInt16 i = 0; i < pTableData->GetColCount(); i++)
                    {
                        if(pTableData->GetColumns()[i].nWidth - nMinWidth > nSub)
                        {
                            pTableData->GetColumns()[i].nWidth -= nSub;
                            nDiff -= nSub;
                        }
                        else
                        {
                            nDiff -= pTableData->GetColumns()[i].nWidth - nMinWidth;
                            pTableData->GetColumns()[i].nWidth = nMinWidth;
                        }

                    }
                }
            }

            sal_Int16 nAlign = 0;
            if (m_xRightBtn->get_active())
                nAlign = text::HoriOrientation::RIGHT;
            else if(m_xLeftBtn->get_active())
                nAlign = text::HoriOrientation::LEFT;
            else if(m_xFromLeftBtn->get_active())
                nAlign = text::HoriOrientation::LEFT_AND_WIDTH;
            else if(m_xCenterBtn->get_active())
                nAlign = text::HoriOrientation::CENTER;
            else if(m_xFreeBtn->get_active())
                nAlign = text::HoriOrientation::NONE;
            else if(m_xFullBtn->get_active())
            {
                nAlign = text::HoriOrientation::FULL;
                lWidth = lAutoWidth;
            }
            if(nAlign != pTableData->GetAlign())
            {
                pTableData->SetWidthChanged();
                pTableData->SetAlign(nAlign);
            }

            if(pTableData->GetWidth() != lWidth )
            {
                pTableData->SetWidthChanged();
                pTableData->SetWidth(
                    nAlign == text::HoriOrientation::FULL ? pTableData->GetSpace() : lWidth );
            }
            if(pTableData->HasWidthChanged())
                _pSet->Put(SwPtrItem(FN_TABLE_REP, pTableData));
        }
    }
    return DeactivateRC::LeavePage;
}

//Description: Page column configuration
SwTableColumnPage::SwTableColumnPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableColumnPage",
        "modules/swriter/ui/tablecolumnpage.ui", &rSet)
    , pTableData(nullptr)
    , nTableWidth(0)
    , nMinWidth(MINLAY)
    , nNoOfCols(0)
    , nNoOfVisibleCols(0)
    , bModified(false)
    , bModifyTable(false)
    , bPercentMode(false)
{
    get(m_pModifyTableCB, "adaptwidth");
    get(m_pProportionalCB, "adaptcolumns");
    get(m_pSpaceFT, "spaceft");
    get(m_pSpaceED, "space-nospin");
    get(m_pUpBtn, "next");
    get(m_pDownBtn, "back");

    m_aFieldArr[0].set(get<MetricField>("width1"));
    m_aFieldArr[1].set(get<MetricField>("width2"));
    m_aFieldArr[2].set(get<MetricField>("width3"));
    m_aFieldArr[3].set(get<MetricField>("width4"));
    m_aFieldArr[4].set(get<MetricField>("width5"));
    m_aFieldArr[5].set(get<MetricField>("width6"));

    m_pTextArr[0] = get<FixedText>("1");
    m_pTextArr[1] = get<FixedText>("2");
    m_pTextArr[2] = get<FixedText>("3");
    m_pTextArr[3] = get<FixedText>("4");
    m_pTextArr[4] = get<FixedText>("5");
    m_pTextArr[5] = get<FixedText>("6");

    SetExchangeSupport();

    const SfxPoolItem* pItem;
    Init(SfxItemState::SET == rSet.GetItemState( SID_HTML_MODE, false,&pItem )
         && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);
}

SwTableColumnPage::~SwTableColumnPage()
{
    disposeOnce();
}

void SwTableColumnPage::dispose()
{
    m_pModifyTableCB.clear();
    m_pProportionalCB.clear();
    m_pSpaceFT.clear();
    m_pSpaceED.clear();
    m_pUpBtn.clear();
    m_pDownBtn.clear();
    for (auto& p : m_pTextArr)
        p.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTableColumnPage::Create( TabPageParent pParent,
                                              const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTableColumnPage>::Create( pParent.pParent, *rAttrSet );
}

void  SwTableColumnPage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP, false, &pItem ))
    {
        pTableData = static_cast<SwTableRep*>(static_cast<const SwPtrItem*>( pItem)->GetValue());
        nNoOfVisibleCols = pTableData->GetColCount();
        nNoOfCols = pTableData->GetAllColCount();
        nTableWidth = pTableData->GetAlign() != text::HoriOrientation::FULL &&
                            pTableData->GetAlign() != text::HoriOrientation::LEFT_AND_WIDTH?
                        pTableData->GetWidth() : pTableData->GetSpace();

        for( sal_uInt16 i = 0; i < nNoOfCols; i++ )
        {
            if( pTableData->GetColumns()[i].nWidth  < nMinWidth )
                    nMinWidth = pTableData->GetColumns()[i].nWidth;
        }
        sal_Int64 nMinTwips = m_aFieldArr[0].NormalizePercent( nMinWidth );
        sal_Int64 nMaxTwips = m_aFieldArr[0].NormalizePercent( nTableWidth );
        for( sal_uInt16 i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
        {
            m_aFieldArr[i].SetPrcntValue( m_aFieldArr[i].NormalizePercent(
                                                GetVisibleWidth(i) ), FUNIT_TWIP );
            m_aFieldArr[i].SetMin( nMinTwips , FUNIT_TWIP );
            m_aFieldArr[i].SetMax( nMaxTwips , FUNIT_TWIP );
            m_aFieldArr[i].Enable();
            m_pTextArr[i]->Enable();
        }

        if( nNoOfVisibleCols > MET_FIELDS )
            m_pUpBtn->Enable();

        for( sal_uInt16 i = nNoOfVisibleCols; i < MET_FIELDS; ++i )
        {
            m_aFieldArr[i].SetText( OUString() );
            m_pTextArr[i]->Disable();
        }
    }
    ActivatePage(rSet);

}

void  SwTableColumnPage::Init(bool bWeb)
{
    FieldUnit aMetric = ::GetDfltMetric(bWeb);
    Link<SpinField&,void> aLkUp = LINK( this, SwTableColumnPage, UpHdl );
    Link<SpinField&,void> aLkDown = LINK( this, SwTableColumnPage, DownHdl );
    Link<Control&,void> aLkLF = LINK( this, SwTableColumnPage, LoseFocusHdl );
    for( sal_uInt16 i = 0; i < MET_FIELDS; i++ )
    {
        aValueTable[i] = i;
        m_aFieldArr[i].SetMetric(aMetric);
        m_aFieldArr[i].SetUpHdl( aLkUp );
        m_aFieldArr[i].SetDownHdl( aLkDown );
        m_aFieldArr[i].SetLoseFocusHdl( aLkLF );
    }
    SetMetric(*m_pSpaceED, aMetric);

    Link<Button*,void> aLk = LINK( this, SwTableColumnPage, AutoClickHdl );
    m_pUpBtn->SetClickHdl( aLk );
    m_pDownBtn->SetClickHdl( aLk );

    aLk = LINK( this, SwTableColumnPage, ModeHdl );
    m_pModifyTableCB->SetClickHdl( aLk );
    m_pProportionalCB->SetClickHdl( aLk );
}

IMPL_LINK( SwTableColumnPage, AutoClickHdl, Button*, pControl, void )
{
    //move display window
    if(pControl == m_pDownBtn.get())
    {
        if(aValueTable[0] > 0)
        {
            for(sal_uInt16 & rn : aValueTable)
                rn -= 1;
        }
    }
    if (pControl == m_pUpBtn.get())
    {
        if( aValueTable[ MET_FIELDS -1 ] < nNoOfVisibleCols -1  )
        {
            for(sal_uInt16 & rn : aValueTable)
                rn += 1;
        }
    }
    for( sal_uInt16 i = 0; (i < nNoOfVisibleCols ) && ( i < MET_FIELDS); i++ )
    {
        OUString sEntry('~');
        OUString sIndex = OUString::number( aValueTable[i] + 1 );
        sEntry += sIndex;
        m_pTextArr[i]->SetText( sEntry );
    }

    m_pDownBtn->Enable(aValueTable[0] > 0);
    m_pUpBtn->Enable(aValueTable[ MET_FIELDS -1 ] < nNoOfVisibleCols -1 );
    UpdateCols(0);
}

IMPL_LINK( SwTableColumnPage, UpHdl, SpinField&, rEdit, void )
{
    bModified = true;
    ModifyHdl( static_cast<MetricField*>(&rEdit) );
}

IMPL_LINK( SwTableColumnPage, DownHdl, SpinField&, rEdit, void )
{
    bModified = true;
    ModifyHdl( static_cast<MetricField*>(&rEdit) );
}

IMPL_LINK( SwTableColumnPage, LoseFocusHdl, Control&, rControl, void )
{
    MetricField* pEdit = static_cast<MetricField*>(&rControl);
    if (pEdit->IsModified())
    {
        bModified = true;
        ModifyHdl( pEdit );
    }
}

IMPL_LINK( SwTableColumnPage, ModeHdl, Button*, pBox, void )
{
    bool bCheck = static_cast<CheckBox*>(pBox)->IsChecked();
    if (pBox == m_pProportionalCB)
    {
        if(bCheck)
            m_pModifyTableCB->Check();
        m_pModifyTableCB->Enable(!bCheck && bModifyTable);
    }
}

bool  SwTableColumnPage::FillItemSet( SfxItemSet* )
{
    for(PercentField & i : m_aFieldArr)
    {
        if (i.HasFocus())
        {
            LoseFocusHdl(*i.get());
            break;
        }
    }

    if(bModified)
    {
        pTableData->SetColsChanged();
    }
    return bModified;
}

void   SwTableColumnPage::ModifyHdl( MetricField const * pField )
{
        PercentField *pEdit = nullptr;
        sal_uInt16 i;

        for( i = 0; i < MET_FIELDS; i++)
        {
            if (pField == m_aFieldArr[i].get())
            {
                pEdit = &m_aFieldArr[i];
                break;
            }
        }

        if (MET_FIELDS <= i || !pEdit)
        {
            OSL_ENSURE(false, "cannot happen.");
            return;
        }

        SetVisibleWidth(aValueTable[i], static_cast< SwTwips >(pEdit->DenormalizePercent(pEdit->GetValue( FUNIT_TWIP ))) );

        UpdateCols( aValueTable[i] );
}

void SwTableColumnPage::UpdateCols( sal_uInt16 nCurrentPos )
{
    SwTwips nSum = 0;

    for( sal_uInt16 i = 0; i < nNoOfCols; i++ )
    {
        nSum += (pTableData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - nTableWidth;

    bool bModifyTableChecked = m_pModifyTableCB->IsChecked();
    bool bProp =    m_pProportionalCB->IsChecked();

    if (!bModifyTableChecked && !bProp)
    {
        //The table width is constant, the difference is balanced with the other columns
        sal_uInt16 nLoopCount = 0;
        while( nDiff )
        {
            if( ++nCurrentPos == nNoOfVisibleCols)
            {
                nCurrentPos = 0;
                ++nLoopCount;
                //#i101353# in small tables it might not be possible to balance column width
                if( nLoopCount > 1 )
                    break;
            }
            if( nDiff < 0 )
            {
                SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) -nDiff);
                nDiff = 0;
            }
            else if( GetVisibleWidth(nCurrentPos) >= nDiff + nMinWidth )
            {
                SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) -nDiff);
                nDiff = 0;
            }
            if( nDiff > 0 && GetVisibleWidth(nCurrentPos) > nMinWidth )
            {
                if( nDiff >= (GetVisibleWidth(nCurrentPos) - nMinWidth) )
                {
                    nDiff -= (GetVisibleWidth(nCurrentPos) - nMinWidth);
                    SetVisibleWidth(nCurrentPos, nMinWidth);
                }
                else
                {
                    nDiff = 0;
                    SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) -nDiff);
                }
                OSL_ENSURE(nDiff >= 0, "nDiff < 0 cannot be here!");
            }
        }
    }
    else if (bModifyTableChecked && !bProp)
    {
        //Difference is balanced by the width of the table,
        //other columns remain unchanged.
        OSL_ENSURE(nDiff <= pTableData->GetSpace() - nTableWidth, "wrong maximum" );
        SwTwips nActSpace = pTableData->GetSpace() - nTableWidth;
        if(nDiff > nActSpace)
        {
            nTableWidth = pTableData->GetSpace();
            SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) - nDiff + nActSpace );
        }
        else
        {
            nTableWidth += nDiff;
        }
    }
    else if (bModifyTableChecked && bProp)
    {
        //All columns will be changed proportionally with,
        //the table width is adjusted accordingly.
        OSL_ENSURE(nDiff * nNoOfVisibleCols <= pTableData->GetSpace() - nTableWidth, "wrong maximum" );
        long nAdd = nDiff;
        if(nDiff * nNoOfVisibleCols > pTableData->GetSpace() - nTableWidth)
        {
            nAdd = (pTableData->GetSpace() - nTableWidth) / nNoOfVisibleCols;
            SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) - nDiff + nAdd );
            nDiff = nAdd;
        }
        if(nAdd)
            for( sal_uInt16 i = 0; i < nNoOfVisibleCols; i++ )
            {
                if(i == nCurrentPos)
                    continue;
                SwTwips nVisWidth;
                if((nVisWidth = GetVisibleWidth(i)) + nDiff < MINLAY)
                {
                    nAdd += nVisWidth - MINLAY;
                    SetVisibleWidth(i, MINLAY);
                }
                else
                {
                    SetVisibleWidth(i, nVisWidth + nDiff);
                    nAdd += nDiff;
                }

            }
        nTableWidth += nAdd;
    }

    if(!bPercentMode)
        m_pSpaceED->SetValue(m_pSpaceED->Normalize( pTableData->GetSpace() - nTableWidth) , FUNIT_TWIP);

    for( sal_uInt16 i = 0; ( i < nNoOfVisibleCols ) && ( i < MET_FIELDS ); i++)
    {
        m_aFieldArr[i].SetPrcntValue(m_aFieldArr[i].NormalizePercent(
                        GetVisibleWidth(aValueTable[i]) ), FUNIT_TWIP);
        m_aFieldArr[i].ClearModifyFlag();
    }
}

void    SwTableColumnPage::ActivatePage( const SfxItemSet& )
{
    bPercentMode = pTableData->GetWidthPercent() != 0;
    for( sal_uInt16 i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
    {
        m_aFieldArr[i].SetRefValue(pTableData->GetWidth());
        m_aFieldArr[i].ShowPercent( bPercentMode );
    }

    const sal_uInt16 nTableAlign = pTableData->GetAlign();
    if((text::HoriOrientation::FULL != nTableAlign && nTableWidth != pTableData->GetWidth()) ||
    (text::HoriOrientation::FULL == nTableAlign && nTableWidth != pTableData->GetSpace()))
    {
        nTableWidth = text::HoriOrientation::FULL == nTableAlign ?
                                    pTableData->GetSpace() :
                                        pTableData->GetWidth();
        UpdateCols(0);
    }
    bModifyTable = true;
    if(pTableData->GetWidthPercent() ||
                text::HoriOrientation::FULL == nTableAlign ||
                        pTableData->IsLineSelected()  )
        bModifyTable = false;
    if(bPercentMode)
    {
        m_pModifyTableCB->Check(false);
        m_pProportionalCB->Check(false);
    }
    else if( !bModifyTable )
    {
        m_pProportionalCB->Check(false);
        m_pModifyTableCB->Check(false);
    }
    m_pSpaceFT->Enable(!bPercentMode);
    m_pSpaceED->Enable(!bPercentMode);
    m_pModifyTableCB->Enable( !bPercentMode && bModifyTable );
    m_pProportionalCB->Enable(!bPercentMode && bModifyTable );

    m_pSpaceED->SetValue(m_pSpaceED->Normalize(
                pTableData->GetSpace() - nTableWidth) , FUNIT_TWIP);

}

DeactivateRC SwTableColumnPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
        if(text::HoriOrientation::FULL != pTableData->GetAlign() && pTableData->GetWidth() != nTableWidth)
        {
            pTableData->SetWidth(nTableWidth);
            SwTwips nDiff = pTableData->GetSpace() - pTableData->GetWidth() -
                            pTableData->GetLeftSpace() - pTableData->GetRightSpace();
            switch( pTableData->GetAlign()  )
            {
                case text::HoriOrientation::RIGHT:
                    pTableData->SetLeftSpace(pTableData->GetLeftSpace() + nDiff);
                break;
                case text::HoriOrientation::LEFT:
                    pTableData->SetRightSpace(pTableData->GetRightSpace() + nDiff);
                break;
                case text::HoriOrientation::NONE:
                {
                    SwTwips nDiff2 = nDiff/2;
                    if( nDiff > 0 ||
                        (-nDiff2 < pTableData->GetRightSpace() && - nDiff2 < pTableData->GetLeftSpace()))
                    {
                        pTableData->SetRightSpace(pTableData->GetRightSpace() + nDiff2);
                        pTableData->SetLeftSpace(pTableData->GetLeftSpace() + nDiff2);
                    }
                    else
                    {
                        if(pTableData->GetRightSpace() > pTableData->GetLeftSpace())
                        {
                            pTableData->SetLeftSpace(0);
                            pTableData->SetRightSpace(pTableData->GetSpace() - pTableData->GetWidth());
                        }
                        else
                        {
                            pTableData->SetRightSpace(0);
                            pTableData->SetLeftSpace(pTableData->GetSpace() - pTableData->GetWidth());
                        }
                    }
                }
                break;
                case text::HoriOrientation::CENTER:
                    pTableData->SetRightSpace(pTableData->GetRightSpace() + nDiff/2);
                    pTableData->SetLeftSpace(pTableData->GetLeftSpace() + nDiff/2);
                break;
                case text::HoriOrientation::LEFT_AND_WIDTH :
                    if(nDiff > pTableData->GetRightSpace())
                    {
                        pTableData->SetLeftSpace(pTableData->GetSpace() - pTableData->GetWidth());
                    }
                    pTableData->SetRightSpace(
                        pTableData->GetSpace() - pTableData->GetWidth() - pTableData->GetLeftSpace());
                break;
            }
            pTableData->SetWidthChanged();
        }
        _pSet->Put(SwPtrItem( FN_TABLE_REP, pTableData ));
    }
    return DeactivateRC::LeavePage;
}

SwTwips  SwTableColumnPage::GetVisibleWidth(sal_uInt16 nPos)
{
    sal_uInt16 i=0;

    while( nPos )
    {
        if(pTableData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    SwTwips nReturn = pTableData->GetColumns()[i].nWidth;
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    while(!pTableData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        nReturn += pTableData->GetColumns()[++i].nWidth;

    return nReturn;
}

void SwTableColumnPage::SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth)
{
    sal_uInt16 i=0;
    while( nPos )
    {
        if(pTableData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    pTableData->GetColumns()[i].nWidth = nNewWidth;
    while(!pTableData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        pTableData->GetColumns()[++i].nWidth = 0;

}

SwTableTabDlg::SwTableTabDlg(vcl::Window* pParent,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
    : SfxTabDialog(pParent, "TablePropertiesDialog",
        "modules/swriter/ui/tableproperties.ui", pItemSet)
    , pShell(pSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage("table", &SwFormatTablePage::Create, nullptr);
    m_nTextFlowId = AddTabPage("textflow", &SwTextFlowPage::Create, nullptr);
    AddTabPage("columns", &SwTableColumnPage::Create, nullptr);
    m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), nullptr);
    m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), nullptr);
}

void  SwTableTabDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nBackgroundId)
    {
        SvxBackgroundTabFlags const nFlagType = SvxBackgroundTabFlags::SHOW_TBLCTL;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(nFlagType)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBorderId)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE, static_cast<sal_uInt16>(SwBorderModes::TABLE)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTextFlowId)
    {
        static_cast<SwTextFlowPage&>(rPage).SetShell(pShell);
        const FrameTypeFlags eType = pShell->GetFrameType(nullptr,true);
        if( !(FrameTypeFlags::BODY & eType) )
            static_cast<SwTextFlowPage&>(rPage).DisablePageBreak();
    }
}

SwTextFlowPage::SwTextFlowPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableTextFlowPage",
        "modules/swriter/ui/tabletextflowpage.ui", &rSet)
    , pShell(nullptr)
    , bPageBreak(true)
    , bHtmlMode(false)
{
    get(m_pPgBrkCB, "break");

    get(m_pPgBrkRB, "page");
    get(m_pColBrkRB, "column");

    get(m_pPgBrkBeforeRB, "before");
    get(m_pPgBrkAfterRB, "after");

    get(m_pPageCollCB, "pagestyle");
    get(m_pPageCollLB, "pagestylelb");
    get(m_pPageNoCB, "pagenoft");
    get(m_pPageNoNF, "pagenonf");

    get(m_pSplitCB, "split");
    get(m_pSplitRowCB, "splitrow");
    get(m_pKeepCB, "keep");
    get(m_pHeadLineCB, "headline");

    get(m_pRepeatHeaderCombo, "repeatheader");
    get(m_pRepeatHeaderNF, "repeatheadernf");

    get(m_pTextDirectionLB, "textdirection");
    get(m_pVertOrientLB, "vertorient");

    m_pPgBrkCB->SetClickHdl(LINK(this, SwTextFlowPage, PageBreakHdl_Impl));
    m_pPgBrkBeforeRB->SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakPosHdl_Impl ) );
    m_pPgBrkAfterRB->SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakPosHdl_Impl ) );
    m_pPageCollCB->SetClickHdl(
        LINK( this, SwTextFlowPage, ApplyCollClickHdl_Impl ) );
    m_pColBrkRB->SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakTypeHdl_Impl ) );
    m_pPgBrkRB->SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakTypeHdl_Impl ) );
    m_pPageNoCB->SetClickHdl(
        LINK(this, SwTextFlowPage, PageNoClickHdl_Impl));
    m_pSplitCB->SetClickHdl(
        LINK( this, SwTextFlowPage, SplitHdl_Impl));
    m_pSplitRowCB->SetClickHdl(
        LINK( this, SwTextFlowPage, SplitRowHdl_Impl));
    m_pHeadLineCB->SetClickHdl( LINK( this, SwTextFlowPage, HeadLineCBClickHdl ) );

    const SfxPoolItem *pItem;
    if(SfxItemState::SET == rSet.GetItemState( SID_HTML_MODE, false,&pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pKeepCB->Hide();
        m_pSplitCB->Hide();
        m_pSplitRowCB->Hide();
    }

    HeadLineCBClickHdl(nullptr);
}

SwTextFlowPage::~SwTextFlowPage()
{
    disposeOnce();
}

void SwTextFlowPage::dispose()
{
    m_pPgBrkCB.clear();
    m_pPgBrkRB.clear();
    m_pColBrkRB.clear();
    m_pPgBrkBeforeRB.clear();
    m_pPgBrkAfterRB.clear();
    m_pPageCollCB.clear();
    m_pPageCollLB.clear();
    m_pPageNoCB.clear();
    m_pPageNoNF.clear();
    m_pSplitCB.clear();
    m_pSplitRowCB.clear();
    m_pKeepCB.clear();
    m_pHeadLineCB.clear();
    m_pRepeatHeaderNF.clear();
    m_pRepeatHeaderCombo.clear();
    m_pTextDirectionLB.clear();
    m_pVertOrientLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTextFlowPage::Create( TabPageParent pParent,
                                           const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTextFlowPage>::Create(pParent.pParent, *rAttrSet);
}

bool  SwTextFlowPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;

    //Repeat Heading
    if(m_pHeadLineCB->IsValueChangedFromSaved() ||
       m_pRepeatHeaderNF->IsValueChangedFromSaved() )
    {
        bModified |= nullptr != rSet->Put(
            SfxUInt16Item(FN_PARAM_TABLE_HEADLINE, m_pHeadLineCB->IsChecked()? sal_uInt16(m_pRepeatHeaderNF->GetValue()) : 0 ));
    }
    if(m_pKeepCB->IsValueChangedFromSaved())
        bModified |= nullptr != rSet->Put( SvxFormatKeepItem( m_pKeepCB->IsChecked(), RES_KEEP));

    if(m_pSplitCB->IsValueChangedFromSaved())
        bModified |= nullptr != rSet->Put( SwFormatLayoutSplit( m_pSplitCB->IsChecked()));

    if(m_pSplitRowCB->IsValueChangedFromSaved())
        bModified |= nullptr != rSet->Put( SwFormatRowSplit( m_pSplitRowCB->IsChecked()));

    const SvxFormatBreakItem* pBreak = GetOldItem( *rSet, RES_BREAK );
    const SwFormatPageDesc* pDesc = GetOldItem( *rSet, RES_PAGEDESC );

    bool bState = m_pPageCollCB->IsChecked();

    //If we have a page style, then there's no break
    bool bPageItemPut = false;
    if (   bState != (m_pPageCollCB->GetSavedValue() == TRISTATE_TRUE)
        || (bState && m_pPageCollLB->IsValueChangedFromSaved())
        || (m_pPageNoCB->IsEnabled() && m_pPageNoCB->IsValueChangedFromSaved())
        || (m_pPageNoNF->IsEnabled() && m_pPageNoNF->IsValueModified()))
    {
        OUString sPage;

        if ( bState )
        {
            sPage = m_pPageCollLB->GetSelectedEntry();
        }
        sal_uInt16 nPgNum = static_cast< sal_uInt16 >(m_pPageNoNF->GetValue());
        bool const usePageNo(bState && m_pPageNoCB->IsChecked());
        boost::optional<sal_uInt16> const oPageNum(
                usePageNo ? nPgNum : boost::optional<sal_Int16>());
        if (!pDesc || !pDesc->GetPageDesc()
            || (pDesc->GetPageDesc()->GetName() != sPage)
            || (pDesc->GetNumOffset() != oPageNum))
        {
            SwFormatPageDesc aFormat( pShell->FindPageDescByName( sPage, true ) );
            aFormat.SetNumOffset(oPageNum);
            bModified |= nullptr != rSet->Put( aFormat );
            bPageItemPut = bState;
        }
    }
    bool bIsChecked = m_pPgBrkCB->IsChecked();
    if ( !bPageItemPut &&
        (   bState != (m_pPageCollCB->GetSavedValue() == TRISTATE_TRUE) ||
            bIsChecked != (m_pPgBrkCB->GetSavedValue() == TRISTATE_TRUE) ||
            m_pPgBrkBeforeRB->IsValueChangedFromSaved()    ||
            m_pPgBrkRB->IsValueChangedFromSaved() ))
    {
        SvxFormatBreakItem aBreak( GetItemSet().Get( RES_BREAK ) );

        if(bIsChecked)
        {
            bool bBefore = m_pPgBrkBeforeRB->IsChecked();

            if ( m_pPgBrkRB->IsChecked() )
            {
                if ( bBefore )
                    aBreak.SetValue( SvxBreak::PageBefore );
                else
                    aBreak.SetValue( SvxBreak::PageAfter );
            }
            else
            {
                if ( bBefore )
                    aBreak.SetValue( SvxBreak::ColumnBefore );
                else
                    aBreak.SetValue( SvxBreak::ColumnAfter );
            }
        }
        else
        {
                aBreak.SetValue( SvxBreak::NONE );
        }

        if ( !pBreak || !( *pBreak == aBreak ) )
        {
            bModified |= nullptr != rSet->Put( aBreak );
        }
    }

    if(m_pTextDirectionLB->IsValueChangedFromSaved())
    {
          bModified |= nullptr != rSet->Put(
                    SvxFrameDirectionItem( static_cast<SvxFrameDirection>(
                        reinterpret_cast<sal_IntPtr>(m_pTextDirectionLB->GetSelectedEntryData()))
                        , FN_TABLE_BOX_TEXTORIENTATION));
    }

    if(m_pVertOrientLB->IsValueChangedFromSaved())
    {
        sal_uInt16 nOrient = USHRT_MAX;
        switch(m_pVertOrientLB->GetSelectedEntryPos())
        {
            case 0 : nOrient = text::VertOrientation::NONE; break;
            case 1 : nOrient = text::VertOrientation::CENTER; break;
            case 2 : nOrient = text::VertOrientation::BOTTOM; break;
        }
        if(nOrient != USHRT_MAX)
            bModified |= nullptr != rSet->Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));
    }

    return bModified;

}

void   SwTextFlowPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem;
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    bool bFlowAllowed = !bHtmlMode || rHtmlOpt.IsPrintLayoutExtension();
    if(bFlowAllowed)
    {
        //Inserting of the existing page templates in the list box
        const size_t nCount = pShell->GetPageDescCnt();

        for( size_t i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = pShell->GetPageDesc(i);
            m_pPageCollLB->InsertEntry(rPageDesc.GetName());
        }

        OUString aFormatName;
        for(sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
            if( LISTBOX_ENTRY_NOTFOUND == m_pPageCollLB->GetEntryPos(
                    aFormatName = SwStyleNameMapper::GetUIName( i, aFormatName ) ))
                m_pPageCollLB->InsertEntry( aFormatName );

        if(SfxItemState::SET == rSet->GetItemState( RES_KEEP, false, &pItem ))
        {
            m_pKeepCB->Check( static_cast<const SvxFormatKeepItem*>(pItem)->GetValue() );
            m_pKeepCB->SaveValue();
        }
        if(SfxItemState::SET == rSet->GetItemState( RES_LAYOUT_SPLIT, false, &pItem ))
        {
            m_pSplitCB->Check( static_cast<const SwFormatLayoutSplit*>(pItem)->GetValue() );
        }
        else
            m_pSplitCB->Check();

        m_pSplitCB->SaveValue();
        SplitHdl_Impl(m_pSplitCB);

        if(SfxItemState::SET == rSet->GetItemState( RES_ROW_SPLIT, false, &pItem ))
        {
            m_pSplitRowCB->Check( static_cast<const SwFormatRowSplit*>(pItem)->GetValue() );
        }
        else
            m_pSplitRowCB->SetState(TRISTATE_INDET);
        m_pSplitRowCB->SaveValue();

        if(bPageBreak)
        {
            if(SfxItemState::SET == rSet->GetItemState( RES_PAGEDESC, false, &pItem ))
            {
                OUString sPageDesc;
                const SwPageDesc* pDesc = static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc();

                ::boost::optional<sal_uInt16> oNumOffset = static_cast<const SwFormatPageDesc*>(pItem)->GetNumOffset();
                if (oNumOffset)
                {
                    m_pPageNoCB->Check();
                    m_pPageNoNF->Enable(true);
                    m_pPageNoNF->SetValue(oNumOffset.get());
                }
                else
                {
                    m_pPageNoCB->Check(false);
                    m_pPageNoNF->Enable(false);
                }

                if(pDesc)
                    sPageDesc = pDesc->GetName();
                if ( !sPageDesc.isEmpty() &&
                        m_pPageCollLB->GetEntryPos( sPageDesc ) != LISTBOX_ENTRY_NOTFOUND )
                {
                    m_pPageCollLB->SelectEntry( sPageDesc );
                    m_pPageCollCB->Check();

                    m_pPgBrkCB->Enable();
                    m_pPgBrkRB->Enable();
                    m_pColBrkRB->Enable();
                    m_pPgBrkBeforeRB->Enable();
                    m_pPgBrkAfterRB->Enable();
                    m_pPageCollCB->Enable();
                    m_pPgBrkCB->Check();

                    m_pPgBrkCB->Check();
                    m_pColBrkRB->Check( false );
                    m_pPgBrkBeforeRB->Check();
                    m_pPgBrkAfterRB->Check( false );
                }
                else
                {
                    m_pPageCollLB->SetNoSelection();
                    m_pPageCollCB->Check(false);
                }
            }

            if(SfxItemState::SET == rSet->GetItemState( RES_BREAK, false, &pItem ))
            {
                const SvxFormatBreakItem* pPageBreak = static_cast<const SvxFormatBreakItem*>(pItem);
                SvxBreak eBreak = pPageBreak->GetBreak();

                if ( eBreak != SvxBreak::NONE )
                {
                    m_pPgBrkCB->Check();
                    m_pPageCollCB->Enable(false);
                    m_pPageCollLB->Enable(false);
                    m_pPageNoCB->Enable(false);
                    m_pPageNoNF->Enable(false);
                }
                switch ( eBreak )
                {
                    case SvxBreak::PageBefore:
                        m_pPgBrkRB->Check();
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check();
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SvxBreak::PageAfter:
                        m_pPgBrkRB->Check();
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check( false );
                        m_pPgBrkAfterRB->Check();
                        break;
                    case SvxBreak::ColumnBefore:
                        m_pPgBrkRB->Check( false );
                        m_pColBrkRB->Check();
                        m_pPgBrkBeforeRB->Check();
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SvxBreak::ColumnAfter:
                        m_pPgBrkRB->Check( false );
                        m_pColBrkRB->Check();
                        m_pPgBrkBeforeRB->Check( false );
                        m_pPgBrkAfterRB->Check();
                        break;
                    default:; //prevent warning
                }

            }
            if ( m_pPgBrkBeforeRB->IsChecked() )
                PageBreakPosHdl_Impl(m_pPgBrkBeforeRB);
            else if ( m_pPgBrkAfterRB->IsChecked() )
                PageBreakPosHdl_Impl(m_pPgBrkAfterRB);
            PageBreakHdl_Impl( m_pPgBrkCB );
        }
    }
    else
    {
        m_pPgBrkRB->Enable(false);
        m_pColBrkRB->Enable(false);
        m_pPgBrkBeforeRB->Enable(false);
        m_pPgBrkAfterRB->Enable(false);
        m_pKeepCB->Enable(false);
        m_pSplitCB->Enable(false);
        m_pPgBrkCB->Enable(false);
        m_pPageCollCB->Enable(false);
        m_pPageCollLB->Enable(false);
    }

    if(SfxItemState::SET == rSet->GetItemState( FN_PARAM_TABLE_HEADLINE, false, &pItem ))
    {
        sal_uInt16 nRep = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        m_pHeadLineCB->Check( nRep > 0 );
        m_pHeadLineCB->SaveValue();
        m_pRepeatHeaderNF->SetValue( nRep );
        m_pRepeatHeaderNF->SetMin( 1 );
        m_pRepeatHeaderNF->SaveValue();
    }
    if ( rSet->GetItemState(FN_TABLE_BOX_TEXTORIENTATION) > SfxItemState::DEFAULT )
    {
        SvxFrameDirection nDirection =
                static_cast<const SvxFrameDirectionItem&>(rSet->Get(FN_TABLE_BOX_TEXTORIENTATION)).GetValue();
        m_pTextDirectionLB->SelectEntryPos(m_pTextDirectionLB->GetEntryPos( reinterpret_cast<void*>(nDirection) ));
    }

    if ( rSet->GetItemState(FN_TABLE_SET_VERT_ALIGN) > SfxItemState::DEFAULT )
    {
        sal_uInt16 nVert = static_cast<const SfxUInt16Item&>(rSet->Get(FN_TABLE_SET_VERT_ALIGN)).GetValue();
        sal_uInt16 nPos = 0;
        switch(nVert)
        {
            case text::VertOrientation::NONE:     nPos = 0;   break;
            case text::VertOrientation::CENTER:   nPos = 1;   break;
            case text::VertOrientation::BOTTOM:   nPos = 2;   break;
        }
        m_pVertOrientLB->SelectEntryPos(nPos);
    }

    m_pPageCollCB->SaveValue();
    m_pPageCollLB->SaveValue();
    m_pPgBrkCB->SaveValue();
    m_pPgBrkRB->SaveValue();
    m_pColBrkRB->SaveValue();
    m_pPgBrkBeforeRB->SaveValue();
    m_pPgBrkAfterRB->SaveValue();
    m_pPageNoCB->SaveValue();
    m_pPageNoNF->SaveValue();
    m_pTextDirectionLB->SaveValue();
    m_pVertOrientLB->SaveValue();

    HeadLineCBClickHdl(nullptr);
}

void SwTextFlowPage::SetShell(SwWrtShell* pSh)
{
    pShell = pSh;
    bHtmlMode = 0 != (::GetHtmlMode(pShell->GetView().GetDocShell()) & HTMLMODE_ON);
    if(bHtmlMode)
    {
        m_pPageNoNF->Enable(false);
        m_pPageNoCB->Enable(false);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakHdl_Impl, Button*, void)
{
    if( m_pPgBrkCB->IsChecked() )
    {
            m_pPgBrkRB->       Enable();
            m_pColBrkRB->      Enable();
            m_pPgBrkBeforeRB-> Enable();
            m_pPgBrkAfterRB->  Enable();

            if ( m_pPgBrkRB->IsChecked() && m_pPgBrkBeforeRB->IsChecked() )
            {
                m_pPageCollCB->Enable();

                bool bEnable = m_pPageCollCB->IsChecked() &&
                                            m_pPageCollLB->GetEntryCount();
                m_pPageCollLB->Enable(bEnable);
                if(!bHtmlMode)
                {
                    m_pPageNoCB->Enable(bEnable);
                    m_pPageNoNF->Enable(bEnable && m_pPageNoCB->IsChecked());
                }
            }
    }
    else
    {
            m_pPageCollCB->Check( false );
            m_pPageCollCB->Enable(false);
            m_pPageCollLB->Enable(false);
            m_pPageNoCB->Enable(false);
            m_pPageNoNF->Enable(false);
            m_pPgBrkRB->       Enable(false);
            m_pColBrkRB->      Enable(false);
            m_pPgBrkBeforeRB-> Enable(false);
            m_pPgBrkAfterRB->  Enable(false);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, ApplyCollClickHdl_Impl, Button*, void)
{
    bool bEnable = false;
    if ( m_pPageCollCB->IsChecked() &&
         m_pPageCollLB->GetEntryCount() )
    {
        bEnable = true;
        m_pPageCollLB->SelectEntryPos( 0 );
    }
    else
    {
        m_pPageCollLB->SetNoSelection();
    }
    m_pPageCollLB->Enable(bEnable);
    if(!bHtmlMode)
    {
        m_pPageNoCB->Enable(bEnable);
        m_pPageNoNF->Enable(bEnable && m_pPageNoCB->IsChecked());
    }
}

IMPL_LINK( SwTextFlowPage, PageBreakPosHdl_Impl, Button*, pBtn, void )
{
    if ( m_pPgBrkCB->IsChecked() )
    {
        if ( pBtn == m_pPgBrkBeforeRB && m_pPgBrkRB->IsChecked() )
        {
            m_pPageCollCB->Enable();

            bool bEnable = m_pPageCollCB->IsChecked()  &&
                                        m_pPageCollLB->GetEntryCount();

            m_pPageCollLB->Enable(bEnable);
            if(!bHtmlMode)
            {
                m_pPageNoCB->Enable(bEnable);
                m_pPageNoNF->Enable(bEnable && m_pPageNoCB->IsChecked());
            }
        }
        else if (pBtn == m_pPgBrkAfterRB)
        {
            m_pPageCollCB->Check( false );
            m_pPageCollCB->Enable(false);
            m_pPageCollLB->Enable(false);
            m_pPageNoCB->Enable(false);
            m_pPageNoNF->Enable(false);
        }
    }
}

IMPL_LINK( SwTextFlowPage, PageBreakTypeHdl_Impl, Button*, pBtn, void )
{
    if ( pBtn == m_pColBrkRB || m_pPgBrkAfterRB->IsChecked() )
    {
        m_pPageCollCB->Check(false);
        m_pPageCollCB->Enable(false);
        m_pPageCollLB->Enable(false);
        m_pPageNoCB->Enable(false);
        m_pPageNoNF->Enable(false);
    }
    else if ( m_pPgBrkBeforeRB->IsChecked() )
        PageBreakPosHdl_Impl(m_pPgBrkBeforeRB);
}

IMPL_LINK_NOARG(SwTextFlowPage, PageNoClickHdl_Impl, Button*, void)
{
    m_pPageNoNF->Enable(m_pPageNoCB->IsChecked());
}

IMPL_LINK( SwTextFlowPage, SplitHdl_Impl, Button*, pBox, void )
{
    m_pSplitRowCB->Enable(static_cast<CheckBox*>(pBox)->IsChecked());
}

IMPL_STATIC_LINK(
    SwTextFlowPage, SplitRowHdl_Impl, Button*, pBox, void )
{
    static_cast<TriStateBox*>(pBox)->EnableTriState(false);
}

IMPL_LINK_NOARG(SwTextFlowPage, HeadLineCBClickHdl, Button*, void)
{
    m_pRepeatHeaderCombo->Enable(m_pHeadLineCB->IsChecked());
}

void SwTextFlowPage::DisablePageBreak()
{
    bPageBreak = false;
    m_pPgBrkCB->Disable();
    m_pPgBrkRB->Disable();
    m_pColBrkRB->Disable();
    m_pPgBrkBeforeRB->Disable();
    m_pPgBrkAfterRB->Disable();
    m_pPageCollCB->Disable();
    m_pPageCollLB->Disable();
    m_pPageNoCB->Disable();
    m_pPageNoNF->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
