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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>

#include <fmtrowsplt.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/sfxdlg.hxx>

#include <strings.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <tabledlg.hxx>
#include "../../uibase/table/tablepg.hxx"
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <swtablerep.hxx>
#include <SwStyleNameMapper.hxx>

#include <cmdid.h>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

using namespace ::com::sun::star;

SwFormatTablePage::SwFormatTablePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/formattablepage.ui"_ustr, u"FormatTablePage"_ustr, &rSet)
    , m_pTableData(nullptr)
    , m_nSaveWidth(0)
    , m_nMinTableWidth(MINLAY)
    , m_bModified(false)
    , m_bFull(false)
    , m_bHtmlMode(false)
    , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xWidthFT(m_xBuilder->weld_label(u"widthft"_ustr))
    , m_xWidthMF(new SwPercentField(m_xBuilder->weld_metric_spin_button(u"widthmf"_ustr, FieldUnit::CM)))
    , m_xRelWidthCB(m_xBuilder->weld_check_button(u"relwidth"_ustr))
    , m_xFullBtn(m_xBuilder->weld_radio_button(u"full"_ustr))
    , m_xLeftBtn(m_xBuilder->weld_radio_button(u"left"_ustr))
    , m_xFromLeftBtn(m_xBuilder->weld_radio_button(u"fromleft"_ustr))
    , m_xRightBtn(m_xBuilder->weld_radio_button(u"right"_ustr))
    , m_xCenterBtn(m_xBuilder->weld_radio_button(u"center"_ustr))
    , m_xFreeBtn(m_xBuilder->weld_radio_button(u"free"_ustr))
    , m_xLeftFT(m_xBuilder->weld_label(u"leftft"_ustr))
    , m_xLeftMF(new SwPercentField(m_xBuilder->weld_metric_spin_button(u"leftmf"_ustr, FieldUnit::CM)))
    , m_xRightFT(m_xBuilder->weld_label(u"rightft"_ustr))
    , m_xRightMF(new SwPercentField(m_xBuilder->weld_metric_spin_button(u"rightmf"_ustr, FieldUnit::CM)))
    , m_xTopFT(m_xBuilder->weld_label(u"aboveft"_ustr))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button(u"abovemf"_ustr, FieldUnit::CM))
    , m_xBottomFT(m_xBuilder->weld_label(u"belowft"_ustr))
    , m_xBottomMF(m_xBuilder->weld_metric_spin_button(u"belowmf"_ustr, FieldUnit::CM))
    , m_xTextDirectionLB(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box(u"textdirection"_ustr)))
    , m_xProperties(m_xBuilder->weld_widget(u"properties"_ustr))
{
    m_xWidthMF->GetMetricFieldRange(m_nOrigWidthMin, m_nOrigWidthMax);
    m_xLeftMF->GetMetricFieldRange(m_nOrigLeftMin, m_nOrigLeftMax);
    m_xRightMF->GetMetricFieldRange(m_nOrigRightMin, m_nOrigRightMax);

    //lock these to initial sizes so they don't change on percent to non percent change
    Size aPrefSize(m_xLeftMF->get()->get_preferred_size());
    m_xLeftMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    m_xRightMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    m_xWidthMF->get()->set_size_request(aPrefSize.Width(), aPrefSize.Height());

    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xTextDirectionLB->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));

    SetExchangeSupport();

    if(const SfxUInt16Item* pModeItem = rSet.GetItemIfSet(SID_HTML_MODE, false))
        m_bHtmlMode = 0 != (pModeItem->GetValue() & HTMLMODE_ON);

    bool bCTL = SvtCTLOptions::IsCTLFontEnabled();
    m_xProperties->set_visible(!m_bHtmlMode && bCTL);

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
    Link<weld::Toggleable&,void> aLk2 = LINK( this, SwFormatTablePage, AutoClickHdl );
    m_xFullBtn->connect_toggled( aLk2 );
    m_xFreeBtn->connect_toggled( aLk2 );
    m_xLeftBtn->connect_toggled( aLk2 );
    m_xFromLeftBtn->connect_toggled( aLk2 );
    m_xRightBtn->connect_toggled( aLk2 );
    m_xCenterBtn->connect_toggled( aLk2 );

    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwFormatTablePage, ValueChangedHdl);
    m_xTopMF->connect_value_changed(aLk);
    m_xBottomMF->connect_value_changed(aLk);
    m_xRightMF->connect_value_changed(aLk);
    m_xLeftMF->connect_value_changed(aLk);
    m_xWidthMF->connect_value_changed(aLk);

    m_xRelWidthCB->connect_toggled(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

IMPL_LINK( SwFormatTablePage, RelWidthClickHdl, weld::Toggleable&, rBtn, void )
{
    OSL_ENSURE(m_pTableData, "table data not available?");
    bool bIsChecked = rBtn.get_active();
    sal_Int64 nLeft  = m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FieldUnit::TWIP));
    sal_Int64 nRight = m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FieldUnit::TWIP));
    m_xWidthMF->ShowPercent(bIsChecked);
    m_xLeftMF->ShowPercent(bIsChecked);
    m_xRightMF->ShowPercent(bIsChecked);

    if (bIsChecked)
    {
        m_xWidthMF->SetRefValue(m_pTableData->GetSpace());
        m_xLeftMF->SetRefValue(m_pTableData->GetSpace());
        m_xRightMF->SetRefValue(m_pTableData->GetSpace());
        m_xLeftMF->SetMetricFieldMin(0); //will be overwritten by the Percentfield
        m_xRightMF->SetMetricFieldMin(0); //ditto
        m_xLeftMF->SetMetricFieldMax(99);
        m_xRightMF->SetMetricFieldMax(99);
        m_xLeftMF->set_value(m_xLeftMF->NormalizePercent(nLeft ), FieldUnit::TWIP );
        m_xRightMF->set_value(m_xRightMF->NormalizePercent(nRight ), FieldUnit::TWIP );
    }
    else
        ModifyHdl(*m_xLeftMF->get());    //correct values again

    if (m_xFreeBtn->get_active())
    {
        bool bEnable = !rBtn.get_active();
        m_xRightMF->set_sensitive(bEnable);
        m_xRightFT->set_sensitive(bEnable);
    }
    m_bModified = true;
}

IMPL_LINK_NOARG(SwFormatTablePage, AutoClickHdl, weld::Toggleable&, void)
{
    bool bRestore = true,
         bLeftEnable = false,
         bRightEnable= false,
         bWidthEnable= false,
         bOthers = true;
    if (m_xFullBtn->get_active())
    {
        m_xLeftMF->set_value(0);
        m_xRightMF->set_value(0);
        m_nSaveWidth = static_cast<SwTwips>(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FieldUnit::TWIP)));
        m_xWidthMF->set_value(m_xWidthMF->NormalizePercent(m_pTableData->GetSpace()), FieldUnit::TWIP);
        m_bFull = true;
        bRestore = false;
    }
    else if (m_xLeftBtn->get_active())
    {
        bRightEnable = bWidthEnable = true;
        m_xLeftMF->set_value(0);
    }
    else if (m_xFromLeftBtn->get_active())
    {
        bLeftEnable = bWidthEnable = true;
        m_xRightMF->set_value(0);
    }
    else if (m_xRightBtn->get_active())
    {
        bLeftEnable = bWidthEnable = true;
        m_xRightMF->set_value(0);
    }
    else if (m_xCenterBtn->get_active())
    {
        bLeftEnable = bWidthEnable = true;
    }
    else if (m_xFreeBtn->get_active())
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

    if(m_bFull && bRestore)
    {
        //After being switched on automatic, the width was pinned
        //in order to restore the width while switching back to.
        m_bFull = false;
        m_xWidthMF->set_value(m_xWidthMF->NormalizePercent(m_nSaveWidth ), FieldUnit::TWIP );
    }
    ModifyHdl(*m_xWidthMF->get());
    m_bModified = true;
}

void SwFormatTablePage::RightModify()
{
    if (!m_xFreeBtn->get_active())
        return;

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

IMPL_LINK( SwFormatTablePage, ValueChangedHdl, weld::MetricSpinButton&, rEdit, void )
{
    if (m_xRightMF->get() == &rEdit)
        RightModify();
    ModifyHdl(rEdit);
}

void  SwFormatTablePage::ModifyHdl(const weld::MetricSpinButton& rEdit, bool bAllowInconsistencies)
{
    SwTwips nCurWidth  = static_cast< SwTwips >(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FieldUnit::TWIP)));
    SwTwips nPrevWidth = nCurWidth;
    SwTwips nRight = static_cast< SwTwips >(m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FieldUnit::TWIP)));
    SwTwips nLeft  = static_cast< SwTwips >(m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FieldUnit::TWIP)));
    SwTwips nDiff;

    if (&rEdit == m_xWidthMF->get())
    {
        if( nCurWidth < MINLAY )
            nCurWidth = MINLAY;
        nDiff = nRight + nLeft + nCurWidth - m_pTableData->GetSpace() ;
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
                    nCurWidth = m_pTableData->GetSpace();
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

        if( nRight + nLeft > m_pTableData->GetSpace() - MINLAY )
            nRight = m_pTableData->GetSpace() -nLeft - MINLAY;

        nCurWidth = m_pTableData->GetSpace() - nLeft - nRight;
    }
    if (&rEdit == m_xLeftMF->get())
    {
        if(!m_xFromLeftBtn->get_active())
        {
            bool bCenter = m_xCenterBtn->get_active();
            if( bCenter )
                nRight = nLeft;
            if(nRight + nLeft > m_pTableData->GetSpace() - MINLAY )
            {
                nLeft  = bCenter ?  (m_pTableData->GetSpace() - MINLAY) /2 :
                                    (m_pTableData->GetSpace() - MINLAY) - nRight;
                nRight = bCenter ?  (m_pTableData->GetSpace() - MINLAY) /2 : nRight;
            }
            nCurWidth = m_pTableData->GetSpace() - nLeft - nRight;
        }
        else
        {
            //Upon changes on the left side the right margin will be changed at first,
            //thereafter the width.
            nDiff = nRight + nLeft + nCurWidth - m_pTableData->GetSpace() ;

            nRight -= nDiff;
            nCurWidth = m_pTableData->GetSpace() - nLeft - nRight;
        }
    }

    m_xRightMF->set_value( m_xRightMF->NormalizePercent( nRight ), FieldUnit::TWIP );
    m_xLeftMF->set_value( m_xLeftMF->NormalizePercent( nLeft ), FieldUnit::TWIP );

    if (nCurWidth != nPrevWidth )
    {
        m_xWidthMF->set_value(m_xWidthMF->NormalizePercent(nCurWidth), FieldUnit::TWIP);

        // tdf#135021 if the user changed the width spinbutton, and in this
        // ModifyHdl we changed the value of that width spinbutton, then rerun
        // the ModifyHdl on the replaced value so the left/right/width value
        // relationships are consistent.
        // But (tdf#135693) only make one effort of rectifying the inconsistency
        if (&rEdit == m_xWidthMF->get() && !bAllowInconsistencies)
            ModifyHdl(rEdit, true);
    }

    m_bModified = true;
}

std::unique_ptr<SfxTabPage> SwFormatTablePage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwFormatTablePage>(pPage, pController, *rAttrSet);
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

    if (m_bModified)
    {
        if (m_xBottomMF->get_value_changed_from_saved() ||
            m_xTopMF->get_value_changed_from_saved() )
        {
            SvxULSpaceItem aULSpace(RES_UL_SPACE);
            aULSpace.SetUpper(m_xTopMF->denormalize(m_xTopMF->get_value(FieldUnit::TWIP)));
            aULSpace.SetLower(m_xBottomMF->denormalize(m_xBottomMF->get_value(FieldUnit::TWIP)));
            rCoreSet->Put(aULSpace);
        }

    }
    if (m_xNameED->get_value_changed_from_saved())
    {
        rCoreSet->Put(SfxStringItem(FN_PARAM_TABLE_NAME, m_xNameED->get_text()));
        m_bModified = true;
    }

    if (m_xTextDirectionLB->get_visible())
    {
        if (m_xTextDirectionLB->get_value_changed_from_saved())
        {
            SvxFrameDirection nDirection = m_xTextDirectionLB->get_active_id();
            rCoreSet->Put(SvxFrameDirectionItem(nDirection, RES_FRAMEDIR));
            m_bModified = true;
        }
    }

    return m_bModified;
}

void  SwFormatTablePage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();

    if(m_bHtmlMode)
    {
        m_xNameED->set_sensitive(false);
        m_xTopFT->hide();
        m_xTopMF->hide();
        m_xBottomFT->hide();
        m_xBottomMF->hide();
        m_xFreeBtn->set_sensitive(false);
    }

    // set back to original state
    m_xRelWidthCB->set_active(false);
    m_xWidthMF->ShowPercent(false);
    m_xLeftMF->ShowPercent(false);
    m_xRightMF->ShowPercent(false);
    m_xWidthMF->SetMetricFieldRange(m_nOrigWidthMin, m_nOrigWidthMax);
    m_xLeftMF->SetMetricFieldRange(m_nOrigLeftMin, m_nOrigLeftMax);
    m_xRightMF->SetMetricFieldRange(m_nOrigRightMin, m_nOrigRightMax);

    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    m_xWidthMF->SetMetric(aMetric);
    m_xRightMF->SetMetric(aMetric);
    m_xLeftMF->SetMetric(aMetric);
    SetFieldUnit(*m_xTopMF, aMetric);
    SetFieldUnit(*m_xBottomMF, aMetric);

    //Name
    if(const SfxStringItem* pNameItem = rSet.GetItemIfSet( FN_PARAM_TABLE_NAME, false ))
    {
        m_xNameED->set_text(pNameItem->GetValue());
        m_xNameED->save_value();
    }

    if(const SwPtrItem* pRepItem = rSet.GetItemIfSet( FN_TABLE_REP, false ))
    {
        m_pTableData = static_cast<SwTableRep*>( pRepItem->GetValue());
        if (!m_xOrigTableData)
            m_xOrigTableData.reset(new SwTableRep(*m_pTableData));
        else // tdf#134925 and tdf#134913, reset back to the original data seen on dialog creation
            *m_pTableData = *m_xOrigTableData;

        m_nMinTableWidth = m_pTableData->GetColCount() * MINLAY;

        if(m_pTableData->GetWidthPercent())
        {
            m_xRelWidthCB->set_active(true);
            RelWidthClickHdl(*m_xRelWidthCB);
            m_xWidthMF->set_value(m_pTableData->GetWidthPercent(), FieldUnit::PERCENT);

            m_xWidthMF->save_value();
            m_nSaveWidth = static_cast< SwTwips >(m_xWidthMF->get_value(FieldUnit::PERCENT));
        }
        else
        {
            m_xWidthMF->set_value(m_xWidthMF->NormalizePercent(
                    m_pTableData->GetWidth()), FieldUnit::TWIP);
            m_xWidthMF->save_value();
            m_nSaveWidth = m_pTableData->GetWidth();
            m_nMinTableWidth = std::min( m_nSaveWidth, m_nMinTableWidth );
        }

        m_xWidthMF->SetRefValue(m_pTableData->GetSpace());

        m_xLeftMF->set_value(m_xLeftMF->NormalizePercent(
                    m_pTableData->GetLeftSpace()), FieldUnit::TWIP);
        m_xRightMF->set_value(m_xRightMF->NormalizePercent(
                    m_pTableData->GetRightSpace()), FieldUnit::TWIP);
        m_xLeftMF->save_value();
        m_xRightMF->save_value();

        bool bSetRight = false, bSetLeft = false;
        switch( m_pTableData->GetAlign() )
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
    if(const SvxULSpaceItem* pSpaceItem = rSet.GetItemIfSet( RES_UL_SPACE, false ))
    {
        m_xTopMF->set_value(m_xTopMF->normalize(
                        pSpaceItem->GetUpper()), FieldUnit::TWIP);
        m_xBottomMF->set_value(m_xBottomMF->normalize(
                        pSpaceItem->GetLower()), FieldUnit::TWIP);
        m_xTopMF->save_value();
        m_xBottomMF->save_value();
    }

    //Text direction
    if( const SvxFrameDirectionItem* pDirectionItem = rSet.GetItemIfSet( RES_FRAMEDIR ) )
    {
        SvxFrameDirection nVal = pDirectionItem->GetValue();
        m_xTextDirectionLB->set_active_id(nVal);
        m_xTextDirectionLB->save_value();
    }

    m_xWidthMF->set_max( 2*m_xWidthMF->NormalizePercent( m_pTableData->GetSpace() ), FieldUnit::TWIP );
    m_xRightMF->set_max( m_xRightMF->NormalizePercent( m_pTableData->GetSpace() ), FieldUnit::TWIP );
    m_xLeftMF->set_max( m_xLeftMF->NormalizePercent( m_pTableData->GetSpace() ), FieldUnit::TWIP );
    m_xWidthMF->set_min( m_xWidthMF->NormalizePercent( m_nMinTableWidth ), FieldUnit::TWIP );
}

void    SwFormatTablePage::ActivatePage( const SfxItemSet& rSet )
{
    if(SfxItemState::SET != rSet.GetItemState( FN_TABLE_REP ))
        return;

    assert(m_pTableData && "table data not available?");

    SwTwips nCurWidth = text::HoriOrientation::FULL != m_pTableData->GetAlign() ?
                                    m_pTableData->GetWidth() :
                                        m_pTableData->GetSpace();
    if(m_pTableData->GetWidthPercent() != 0 ||
       nCurWidth == m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FieldUnit::TWIP)))
        return;

    m_xWidthMF->set_value(m_xWidthMF->NormalizePercent(
                    nCurWidth), FieldUnit::TWIP);
    m_xWidthMF->save_value();
    m_nSaveWidth = nCurWidth;
    m_xLeftMF->set_value(m_xLeftMF->NormalizePercent(
                    m_pTableData->GetLeftSpace()), FieldUnit::TWIP);
    m_xLeftMF->save_value();
    m_xRightMF->set_value(m_xRightMF->NormalizePercent(
                    m_pTableData->GetRightSpace()), FieldUnit::TWIP);
    m_xRightMF->save_value();
}

DeactivateRC SwFormatTablePage::DeactivatePage( SfxItemSet* _pSet )
{
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
        if(m_bModified)
        {
            SwTwips lLeft  = static_cast< SwTwips >(m_xLeftMF->DenormalizePercent(m_xLeftMF->get_value(FieldUnit::TWIP)));
            SwTwips lRight = static_cast< SwTwips >(m_xRightMF->DenormalizePercent(m_xRightMF->get_value(FieldUnit::TWIP)));

            if( m_xLeftMF->get_value_changed_from_saved() ||
                m_xRightMF->get_value_changed_from_saved() )
            {
                m_pTableData->SetWidthChanged();
                m_pTableData->SetLeftSpace( lLeft);
                m_pTableData->SetRightSpace( lRight);
            }

            SwTwips lWidth;
            if (m_xRelWidthCB->get_active() && m_xRelWidthCB->get_sensitive())
            {
                lWidth = m_pTableData->GetSpace() - lRight - lLeft;
                const sal_uInt16 nPercentWidth = m_xWidthMF->get_value(FieldUnit::PERCENT);
                if(m_pTableData->GetWidthPercent() != nPercentWidth)
                {
                    m_pTableData->SetWidthPercent(nPercentWidth);
                    m_pTableData->SetWidthChanged();
                }
            }
            else
            {
                m_pTableData->SetWidthPercent(0);
                lWidth = static_cast<SwTwips>(m_xWidthMF->DenormalizePercent(m_xWidthMF->get_value(FieldUnit::TWIP)));
            }
            m_pTableData->SetWidth(lWidth);

            SwTwips nColSum = 0;

            for( sal_uInt16 i = 0; i < m_pTableData->GetColCount(); i++)
            {
                nColSum += m_pTableData->GetColumns()[i].nWidth;
            }
            if(nColSum != m_pTableData->GetWidth())
            {
                SwTwips nMinWidth = std::min( tools::Long(MINLAY),
                                    static_cast<tools::Long>(m_pTableData->GetWidth() /
                                            m_pTableData->GetColCount() - 1));
                SwTwips nDiff = nColSum - m_pTableData->GetWidth();
                while ( std::abs(nDiff) > m_pTableData->GetColCount() + 1 )
                {
                    SwTwips nSub = nDiff / m_pTableData->GetColCount();
                    for( sal_uInt16 i = 0; i < m_pTableData->GetColCount(); i++)
                    {
                        if(m_pTableData->GetColumns()[i].nWidth - nMinWidth > nSub)
                        {
                            m_pTableData->GetColumns()[i].nWidth -= nSub;
                            nDiff -= nSub;
                        }
                        else
                        {
                            nDiff -= m_pTableData->GetColumns()[i].nWidth - nMinWidth;
                            m_pTableData->GetColumns()[i].nWidth = nMinWidth;
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
            if(nAlign != m_pTableData->GetAlign())
            {
                m_pTableData->SetWidthChanged();
                m_pTableData->SetAlign(nAlign);
            }

            if(m_pTableData->GetWidth() != lWidth )
            {
                m_pTableData->SetWidthChanged();
                m_pTableData->SetWidth(
                    nAlign == text::HoriOrientation::FULL ? m_pTableData->GetSpace() : lWidth );
            }
            if(m_pTableData->HasWidthChanged())
                _pSet->Put(SwPtrItem(FN_TABLE_REP, m_pTableData));
        }
    }
    return DeactivateRC::LeavePage;
}

//Description: Page column configuration
SwTableColumnPage::SwTableColumnPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/tablecolumnpage.ui"_ustr, u"TableColumnPage"_ustr, &rSet)
    , m_pTableData(nullptr)
    , m_pSizeHdlEvent(nullptr)
    , m_nTableWidth(0)
    , m_nMinWidth(MINLAY)
    , m_nMetFields(MET_FIELDS)
    , m_nNoOfCols(0)
    , m_nNoOfVisibleCols(0)
    , m_bModified(false)
    , m_bModifyTable(false)
    , m_bPercentMode(false)
    , m_aFieldArr { m_xBuilder->weld_metric_spin_button(u"width1"_ustr, FieldUnit::CM),
                    m_xBuilder->weld_metric_spin_button(u"width2"_ustr, FieldUnit::CM),
                    m_xBuilder->weld_metric_spin_button(u"width3"_ustr, FieldUnit::CM),
                    m_xBuilder->weld_metric_spin_button(u"width4"_ustr, FieldUnit::CM),
                    m_xBuilder->weld_metric_spin_button(u"width5"_ustr, FieldUnit::CM)}
    , m_aTextArr { m_xBuilder->weld_label(u"1"_ustr),
                   m_xBuilder->weld_label(u"2"_ustr),
                   m_xBuilder->weld_label(u"3"_ustr),
                   m_xBuilder->weld_label(u"4"_ustr),
                   m_xBuilder->weld_label(u"5"_ustr)}
    , m_xModifyTableCB(m_xBuilder->weld_check_button(u"adaptwidth"_ustr))
    , m_xProportionalCB(m_xBuilder->weld_check_button(u"adaptcolumns"_ustr))
    , m_xSpaceFT(m_xBuilder->weld_label(u"spaceft"_ustr))
    , m_xSpaceSFT(m_xBuilder->weld_label(u"space"_ustr))
    , m_xSpaceED(m_xBuilder->weld_metric_spin_button(u"spacefmt"_ustr, FieldUnit::CM))
    , m_xUpBtn(m_xBuilder->weld_button(u"next"_ustr))
    , m_xDownBtn(m_xBuilder->weld_button(u"back"_ustr))
{
    SetExchangeSupport();

    // fire off this handler to happen on next event loop when all the rest of
    // the pages are instantiated and the dialog preferred size is that of the
    // all the pages that currently exist and the rest to come after this one
    m_pSizeHdlEvent = Application::PostUserEvent(LINK(this, SwTableColumnPage, SizeHdl));

    const SfxUInt16Item* pModeItem = GetItemSet().GetItemIfSet(SID_HTML_MODE, false);
    Init(pModeItem && pModeItem->GetValue() & HTMLMODE_ON);
}

IMPL_LINK_NOARG(SwTableColumnPage, SizeHdl, void*, void)
{
    m_pSizeHdlEvent = nullptr;

    //tdf#120420 keeping showing column width fields unless
    //the dialog begins to grow, then stop adding them
    weld::Window* pTopLevel = GetFrameWeld();
    Size aOrigSize = pTopLevel->get_preferred_size();
    for (sal_uInt16 i = 0; i < MET_FIELDS; ++i)
    {
        m_aFieldArr[i].show();
        m_aTextArr[i]->show();

        if (pTopLevel->get_preferred_size().Width() > aOrigSize.Width())
        {
            m_nMetFields = i + 1;
            m_aTextArr[i]->set_grid_width(1);
            m_xUpBtn->set_grid_left_attach(m_nMetFields * 2 - 1);
            break;
        }
    }

    // tdf#143142 m_nMetFields has been updated and we need to re-check whether to show right button or not.
    if (m_nNoOfVisibleCols > m_nMetFields)
    {
        m_xUpBtn->set_sensitive(true);
    }

}

SwTableColumnPage::~SwTableColumnPage()
{
    if (m_pSizeHdlEvent)
    {
        Application::RemoveUserEvent(m_pSizeHdlEvent);
        m_pSizeHdlEvent = nullptr;
    }
}

std::unique_ptr<SfxTabPage> SwTableColumnPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwTableColumnPage>(pPage, pController, *rAttrSet);
}

void  SwTableColumnPage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();

    if(const SwPtrItem* pRepItem = rSet.GetItemIfSet( FN_TABLE_REP, false ))
    {
        m_pTableData = static_cast<SwTableRep*>( pRepItem->GetValue());
        if (!m_xOrigTableData)
            m_xOrigTableData.reset(new SwTableRep(*m_pTableData));
        else // tdf#134925 and tdf#134913, reset back to the original data seen on dialog creation
            *m_pTableData = *m_xOrigTableData;

        m_nNoOfVisibleCols = m_pTableData->GetColCount();
        m_nNoOfCols = m_pTableData->GetAllColCount();
        m_nTableWidth = m_pTableData->GetAlign() != text::HoriOrientation::FULL &&
                            m_pTableData->GetAlign() != text::HoriOrientation::LEFT_AND_WIDTH?
                        m_pTableData->GetWidth() : m_pTableData->GetSpace();

        for( sal_uInt16 i = 0; i < m_nNoOfCols; i++ )
        {
            if (m_pTableData->GetColumns()[i].nWidth  < m_nMinWidth)
                m_nMinWidth = m_pTableData->GetColumns()[i].nWidth;
        }
        sal_Int64 nMinTwips = m_aFieldArr[0].NormalizePercent( m_nMinWidth );
        sal_Int64 nMaxTwips = m_aFieldArr[0].NormalizePercent( m_nTableWidth );
        for( sal_uInt16 i = 0; (i < m_nMetFields) && (i < m_nNoOfVisibleCols); i++ )
        {
            m_aFieldArr[i].set_value( m_aFieldArr[i].NormalizePercent(
                                                GetVisibleWidth(i) ), FieldUnit::TWIP );
            m_aFieldArr[i].set_min(nMinTwips, FieldUnit::TWIP);
            m_aFieldArr[i].set_max(nMaxTwips, FieldUnit::TWIP);
            m_aFieldArr[i].set_sensitive(true);
            m_aTextArr[i]->set_sensitive(true);
        }

        if (m_nNoOfVisibleCols > m_nMetFields)
        {
            m_xUpBtn->set_sensitive(true);
        }

        for( sal_uInt16 i = m_nNoOfVisibleCols; i < m_nMetFields; ++i )
        {
            m_aFieldArr[i].set_text(OUString());
            m_aTextArr[i]->set_sensitive(false);
        }
    }
    ActivatePage(rSet);
}

void  SwTableColumnPage::Init(bool bWeb)
{
    FieldUnit aMetric = ::GetDfltMetric(bWeb);
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwTableColumnPage, ValueChangedHdl);
    for (sal_uInt16 i = 0; i < MET_FIELDS; ++i)
    {
        m_aValueTable[i] = i;
        m_aFieldArr[i].SetMetric(aMetric);
        m_aFieldArr[i].connect_value_changed(aLk);
    }
    SetFieldUnit(*m_xSpaceED, aMetric);

    Link<weld::Button&,void> aClickLk = LINK(this, SwTableColumnPage, AutoClickHdl);
    m_xUpBtn->connect_clicked(aClickLk);
    m_xDownBtn->connect_clicked(aClickLk);

    Link<weld::Toggleable&,void> aToggleLk = LINK(this, SwTableColumnPage, ModeHdl);
    m_xModifyTableCB->connect_toggled(aToggleLk);
    m_xProportionalCB->connect_toggled(aToggleLk);
}

IMPL_LINK(SwTableColumnPage, AutoClickHdl, weld::Button&, rControl, void)
{
    //move display window
    if (&rControl == m_xDownBtn.get())
    {
        if(m_aValueTable[0] > 0)
        {
            for(sal_uInt16 & rn : m_aValueTable)
                rn -= 1;
        }
    }
    if (&rControl == m_xUpBtn.get())
    {
        if( m_aValueTable[ m_nMetFields -1 ] < m_nNoOfVisibleCols -1  )
        {
            for(sal_uInt16 & rn : m_aValueTable)
                rn += 1;
        }
    }
    for( sal_uInt16 i = 0; (i < m_nNoOfVisibleCols ) && ( i < m_nMetFields); i++ )
    {
        OUString sEntry('~');
        OUString sIndex = OUString::number( m_aValueTable[i] + 1 );
        sEntry += sIndex;
        m_aTextArr[i]->set_label(sEntry);
    }

    m_xDownBtn->set_sensitive(m_aValueTable[0] > 0);
    m_xUpBtn->set_sensitive(m_aValueTable[ m_nMetFields -1 ] < m_nNoOfVisibleCols -1 );
    UpdateCols(0);
}

IMPL_LINK(SwTableColumnPage, ValueChangedHdl, weld::MetricSpinButton&, rEdit, void)
{
    m_bModified = true;
    ModifyHdl(&rEdit);
}

IMPL_LINK(SwTableColumnPage, ModeHdl, weld::Toggleable&, rBox, void)
{
    const bool bCheck = rBox.get_active();
    if (&rBox == m_xProportionalCB.get())
    {
        if (bCheck)
            m_xModifyTableCB->set_active(true);
        m_xModifyTableCB->set_sensitive(!bCheck && m_bModifyTable);
    }
}

bool SwTableColumnPage::FillItemSet( SfxItemSet* )
{
    for (SwPercentField & i : m_aFieldArr)
    {
        if (i.has_focus())
        {
            ModifyHdl(i.get());
            break;
        }
    }

    if (m_bModified)
    {
        m_pTableData->SetColsChanged();
    }
    return m_bModified;
}

void SwTableColumnPage::ModifyHdl(const weld::MetricSpinButton* pField)
{
    SwPercentField *pEdit = nullptr;
    sal_uInt16 i;

    for( i = 0; i < m_nMetFields; i++)
    {
        if (pField == m_aFieldArr[i].get())
        {
            pEdit = &m_aFieldArr[i];
            break;
        }
    }

    if (m_nMetFields <= i || !pEdit)
    {
        OSL_ENSURE(false, "cannot happen.");
        return;
    }

    SetVisibleWidth(m_aValueTable[i], pEdit->DenormalizePercent(pEdit->get_value(FieldUnit::TWIP)));

    UpdateCols( m_aValueTable[i] );
}

void SwTableColumnPage::UpdateCols( sal_uInt16 nCurrentPos )
{
    SwTwips nSum = 0;

    for( sal_uInt16 i = 0; i < m_nNoOfCols; i++ )
    {
        nSum += (m_pTableData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - m_nTableWidth;

    bool bModifyTableChecked = m_xModifyTableCB->get_active();
    bool bProp = m_xProportionalCB->get_active();

    if (!bModifyTableChecked && !bProp)
    {
        //The table width is constant, the difference is balanced with the other columns
        sal_uInt16 nLoopCount = 0;
        while( nDiff )
        {
            if( ++nCurrentPos == m_nNoOfVisibleCols)
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
            else if( GetVisibleWidth(nCurrentPos) >= nDiff + m_nMinWidth )
            {
                SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) -nDiff);
                nDiff = 0;
            }
            if( nDiff > 0 && GetVisibleWidth(nCurrentPos) > m_nMinWidth )
            {
                if( nDiff >= (GetVisibleWidth(nCurrentPos) - m_nMinWidth) )
                {
                    nDiff -= (GetVisibleWidth(nCurrentPos) - m_nMinWidth);
                    SetVisibleWidth(nCurrentPos, m_nMinWidth);
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
        OSL_ENSURE(nDiff <= m_pTableData->GetSpace() - m_nTableWidth, "wrong maximum" );
        SwTwips nActSpace = m_pTableData->GetSpace() - m_nTableWidth;
        if(nDiff > nActSpace)
        {
            m_nTableWidth = m_pTableData->GetSpace();
            SetVisibleWidth(nCurrentPos, GetVisibleWidth(nCurrentPos) - nDiff + nActSpace );
        }
        else
        {
            m_nTableWidth += nDiff;
        }
    }
    else if (bModifyTableChecked && bProp)
    {
        //All columns will be changed proportionally with,
        //the table width is adjusted accordingly.
        const double fOrigColWidth = std::max(SwTwips(1), GetVisibleWidth(nCurrentPos) - nDiff);
        const double fMaxWidth = std::max(m_pTableData->GetSpace(), m_nTableWidth);
        const double fMaxPercent = fMaxWidth / m_nTableWidth;
        const double fPercentChange = std::min(fMaxPercent, GetVisibleWidth(nCurrentPos)/fOrigColWidth);
        SwTwips nNewTableSize = 0;
        for( sal_uInt16 i = 0; i < m_nNoOfVisibleCols; i++ )
        {
            SwTwips nNewColWidth = round( fPercentChange * (i == nCurrentPos ? fOrigColWidth : GetVisibleWidth(i)) );
            if ( nNewColWidth < MINLAY )
                nNewColWidth = MINLAY;
            SetVisibleWidth(i, nNewColWidth);
            nNewTableSize += nNewColWidth;
        }
        m_nTableWidth = nNewTableSize;
    }

    if (!m_bPercentMode)
    {
        m_xSpaceED->set_value(m_xSpaceED->normalize(m_pTableData->GetSpace() - m_nTableWidth), FieldUnit::TWIP);
        m_xSpaceSFT->set_label(m_xSpaceED->get_text());
    }
    else
        m_xSpaceSFT->set_label(OUString());

    for( sal_uInt16 i = 0; ( i < m_nNoOfVisibleCols ) && ( i < m_nMetFields ); i++)
    {
        m_aFieldArr[i].set_value(m_aFieldArr[i].NormalizePercent(
                        GetVisibleWidth(m_aValueTable[i]) ), FieldUnit::TWIP);
    }
}

void SwTableColumnPage::ActivatePage( const SfxItemSet& )
{
    m_bPercentMode = m_pTableData->GetWidthPercent() != 0;
    for( sal_uInt16 i = 0; (i < m_nMetFields) && (i < m_nNoOfVisibleCols); i++ )
    {
        m_aFieldArr[i].SetRefValue(m_pTableData->GetWidth());
        m_aFieldArr[i].ShowPercent( m_bPercentMode );
    }

    const sal_uInt16 nTableAlign = m_pTableData->GetAlign();
    if((text::HoriOrientation::FULL != nTableAlign && m_nTableWidth != m_pTableData->GetWidth()) ||
    (text::HoriOrientation::FULL == nTableAlign && m_nTableWidth != m_pTableData->GetSpace()))
    {
        m_nTableWidth = text::HoriOrientation::FULL == nTableAlign ?
                                    m_pTableData->GetSpace() :
                                        m_pTableData->GetWidth();
        UpdateCols(0);
    }
    m_bModifyTable = true;
    if (m_pTableData->GetWidthPercent() ||
                text::HoriOrientation::FULL == nTableAlign ||
                        m_pTableData->IsLineSelected()  )
        m_bModifyTable = false;
    if (m_bPercentMode)
    {
        m_xModifyTableCB->set_active(false);
        m_xProportionalCB->set_active(false);
    }
    else if (!m_bModifyTable)
    {
        m_xProportionalCB->set_active(false);
        m_xModifyTableCB->set_active(false);
    }
    m_xSpaceFT->set_sensitive(!m_bPercentMode);
    m_xSpaceSFT->set_sensitive(!m_bPercentMode);
    m_xModifyTableCB->set_sensitive( !m_bPercentMode && m_bModifyTable );
    m_xProportionalCB->set_sensitive(!m_bPercentMode && m_bModifyTable );

    m_xSpaceED->set_value(m_xSpaceED->normalize(
                m_pTableData->GetSpace() - m_nTableWidth), FieldUnit::TWIP);
    m_xSpaceSFT->set_label(m_xSpaceED->get_text());

}

DeactivateRC SwTableColumnPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
        if(text::HoriOrientation::FULL != m_pTableData->GetAlign() && m_pTableData->GetWidth() != m_nTableWidth)
        {
            m_pTableData->SetWidth(m_nTableWidth);
            SwTwips nDiff = m_pTableData->GetSpace() - m_pTableData->GetWidth() -
                            m_pTableData->GetLeftSpace() - m_pTableData->GetRightSpace();
            switch( m_pTableData->GetAlign()  )
            {
                case text::HoriOrientation::RIGHT:
                    m_pTableData->SetLeftSpace(m_pTableData->GetLeftSpace() + nDiff);
                break;
                case text::HoriOrientation::LEFT:
                    m_pTableData->SetRightSpace(m_pTableData->GetRightSpace() + nDiff);
                break;
                case text::HoriOrientation::NONE:
                {
                    SwTwips nDiff2 = nDiff/2;
                    if( nDiff > 0 ||
                        (-nDiff2 < m_pTableData->GetRightSpace() && - nDiff2 < m_pTableData->GetLeftSpace()))
                    {
                        m_pTableData->SetRightSpace(m_pTableData->GetRightSpace() + nDiff2);
                        m_pTableData->SetLeftSpace(m_pTableData->GetLeftSpace() + nDiff2);
                    }
                    else
                    {
                        if(m_pTableData->GetRightSpace() > m_pTableData->GetLeftSpace())
                        {
                            m_pTableData->SetLeftSpace(0);
                            m_pTableData->SetRightSpace(m_pTableData->GetSpace() - m_pTableData->GetWidth());
                        }
                        else
                        {
                            m_pTableData->SetRightSpace(0);
                            m_pTableData->SetLeftSpace(m_pTableData->GetSpace() - m_pTableData->GetWidth());
                        }
                    }
                }
                break;
                case text::HoriOrientation::CENTER:
                    m_pTableData->SetRightSpace(m_pTableData->GetRightSpace() + nDiff/2);
                    m_pTableData->SetLeftSpace(m_pTableData->GetLeftSpace() + nDiff/2);
                break;
                case text::HoriOrientation::LEFT_AND_WIDTH :
                    if(nDiff > m_pTableData->GetRightSpace())
                    {
                        m_pTableData->SetLeftSpace(m_pTableData->GetSpace() - m_pTableData->GetWidth());
                    }
                    m_pTableData->SetRightSpace(
                        m_pTableData->GetSpace() - m_pTableData->GetWidth() - m_pTableData->GetLeftSpace());
                break;
            }
            m_pTableData->SetWidthChanged();
        }
        _pSet->Put(SwPtrItem( FN_TABLE_REP, m_pTableData ));
    }
    return DeactivateRC::LeavePage;
}

SwTwips  SwTableColumnPage::GetVisibleWidth(sal_uInt16 nPos)
{
    sal_uInt16 i=0;

    while( nPos )
    {
        if(m_pTableData->GetColumns()[i].bVisible)
            nPos--;
        i++;
    }
    SwTwips nReturn = m_pTableData->GetColumns()[i].nWidth;
    OSL_ENSURE(i < m_nNoOfCols, "Array index out of range");
    while(!m_pTableData->GetColumns()[i].bVisible && (i + 1) < m_nNoOfCols)
        nReturn += m_pTableData->GetColumns()[++i].nWidth;

    return nReturn;
}

void SwTableColumnPage::SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth)
{
    sal_uInt16 i=0;
    while( nPos )
    {
        if(m_pTableData->GetColumns()[i].bVisible)
            nPos--;
        i++;
    }
    OSL_ENSURE(i < m_nNoOfCols, "Array index out of range");
    m_pTableData->GetColumns()[i].nWidth = nNewWidth;
    while(!m_pTableData->GetColumns()[i].bVisible && (i + 1) < m_nNoOfCols)
        m_pTableData->GetColumns()[++i].nWidth = 0;

}

SwTableTabDlg::SwTableTabDlg(weld::Window* pParent, const SfxItemSet* pItemSet, SwWrtShell* pSh)
    : SfxTabDialogController(pParent, u"modules/swriter/ui/tableproperties.ui"_ustr, u"TablePropertiesDialog"_ustr, pItemSet)
    , m_pShell(pSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage(u"table"_ustr, &SwFormatTablePage::Create, nullptr);
    AddTabPage(u"textflow"_ustr, &SwTextFlowPage::Create, nullptr);
    AddTabPage(u"columns"_ustr, &SwTableColumnPage::Create, nullptr);
    AddTabPage(u"background"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), nullptr);
    AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), nullptr);
}

void  SwTableTabDlg::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "background")
    {
        SvxBackgroundTabFlags const nFlagType = SvxBackgroundTabFlags::SHOW_TBLCTL;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(nFlagType)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "borders")
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE, static_cast<sal_uInt16>(SwBorderModes::TABLE)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "textflow")
    {
        static_cast<SwTextFlowPage&>(rPage).SetShell(m_pShell);
        const FrameTypeFlags eType = m_pShell->GetFrameType(nullptr,true);
        if( !(FrameTypeFlags::BODY & eType) )
            static_cast<SwTextFlowPage&>(rPage).DisablePageBreak();
    }
}

SwTextFlowPage::SwTextFlowPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/tabletextflowpage.ui"_ustr, u"TableTextFlowPage"_ustr, &rSet)
    , m_pShell(nullptr)
    , m_bPageBreak(true)
    , m_bHtmlMode(false)
    , m_xPgBrkCB(m_xBuilder->weld_check_button(u"break"_ustr))
    , m_xPgBrkRB(m_xBuilder->weld_radio_button(u"page"_ustr))
    , m_xColBrkRB(m_xBuilder->weld_radio_button(u"column"_ustr))
    , m_xPgBrkBeforeRB(m_xBuilder->weld_radio_button(u"before"_ustr))
    , m_xPgBrkAfterRB(m_xBuilder->weld_radio_button(u"after"_ustr))
    , m_xPageCollCB(m_xBuilder->weld_check_button(u"pagestyle"_ustr))
    , m_xPageCollLB(m_xBuilder->weld_combo_box(u"pagestylelb"_ustr))
    , m_xPageNoCB(m_xBuilder->weld_check_button(u"pagenoft"_ustr))
    , m_xPageNoNF(m_xBuilder->weld_spin_button(u"pagenonf"_ustr))
    , m_xSplitCB(m_xBuilder->weld_check_button(u"split"_ustr))
    , m_xSplitRowCB(m_xBuilder->weld_check_button(u"splitrow"_ustr))
    , m_xKeepCB(m_xBuilder->weld_check_button(u"keep"_ustr))
    , m_xHeadLineCB(m_xBuilder->weld_check_button(u"headline"_ustr))
    , m_xRepeatHeaderCombo(m_xBuilder->weld_widget(u"repeatheader"_ustr))
    , m_xRepeatHeaderNF(m_xBuilder->weld_spin_button(u"repeatheadernf"_ustr))
    , m_xTextDirectionLB(m_xBuilder->weld_combo_box(u"textorientation"_ustr))
    , m_xVertOrientLB(m_xBuilder->weld_combo_box(u"vertorient"_ustr))
{
    m_xPgBrkCB->connect_toggled(LINK(this, SwTextFlowPage, PageBreakHdl_Impl));
    m_xPgBrkBeforeRB->connect_toggled(
        LINK(this, SwTextFlowPage, PageBreakPosHdl_Impl));
    m_xPgBrkAfterRB->connect_toggled(
        LINK(this, SwTextFlowPage, PageBreakPosHdl_Impl));
    m_xPageCollCB->connect_toggled(
        LINK(this, SwTextFlowPage, ApplyCollClickHdl_Impl));
    m_xColBrkRB->connect_toggled(
        LINK(this, SwTextFlowPage, PageBreakTypeHdl_Impl));
    m_xPgBrkRB->connect_toggled(
        LINK(this, SwTextFlowPage, PageBreakTypeHdl_Impl));
    m_xPageNoCB->connect_toggled(
        LINK(this, SwTextFlowPage, PageNoClickHdl_Impl));
    m_xSplitCB->connect_toggled(
        LINK(this, SwTextFlowPage, SplitHdl_Impl));
    m_xHeadLineCB->connect_toggled(LINK(this, SwTextFlowPage, HeadLineCBClickHdl));

    const SfxUInt16Item *pModeItem = rSet.GetItemIfSet( SID_HTML_MODE, false );
    if(pModeItem && pModeItem->GetValue() & HTMLMODE_ON)
    {
        m_xKeepCB->hide();
        m_xSplitCB->hide();
        m_xSplitRowCB->hide();
    }

    HeadLineCBClickHdl(*m_xHeadLineCB);
}

SwTextFlowPage::~SwTextFlowPage()
{
}

std::unique_ptr<SfxTabPage> SwTextFlowPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                                   const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwTextFlowPage>(pPage, pController, *rAttrSet);
}

bool  SwTextFlowPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;

    //Repeat Heading
    if (m_xHeadLineCB->get_state_changed_from_saved() ||
        m_xRepeatHeaderNF->get_value_changed_from_saved() )
    {
        bModified |= nullptr != rSet->Put(
            SfxUInt16Item(FN_PARAM_TABLE_HEADLINE, m_xHeadLineCB->get_active() ? sal_uInt16(m_xRepeatHeaderNF->get_value()) : 0));
    }
    if (m_xKeepCB->get_state_changed_from_saved())
        bModified |= nullptr != rSet->Put( SvxFormatKeepItem( m_xKeepCB->get_active(), RES_KEEP));

    if (m_xSplitCB->get_state_changed_from_saved())
        bModified |= nullptr != rSet->Put( SwFormatLayoutSplit( m_xSplitCB->get_active()));

    if (m_xSplitRowCB->get_state_changed_from_saved())
        bModified |= nullptr != rSet->Put( SwFormatRowSplit( m_xSplitRowCB->get_active()));

    const SvxFormatBreakItem* pBreak = GetOldItem( *rSet, RES_BREAK );
    const SwFormatPageDesc* pDesc = GetOldItem( *rSet, RES_PAGEDESC );

    bool bState = m_xPageCollCB->get_active();

    //If we have a page style, then there's no break
    bool bPageItemPut = false;
    if (   bState != (m_xPageCollCB->get_saved_state() == TRISTATE_TRUE)
        || (bState && m_xPageCollLB->get_value_changed_from_saved())
        || (m_xPageNoCB->get_sensitive() && m_xPageNoCB->get_state_changed_from_saved())
        || (m_xPageNoNF->get_sensitive() && m_xPageNoNF->get_value_changed_from_saved()))
    {
        OUString sPage;

        if ( bState )
        {
            sPage = m_xPageCollLB->get_active_text();
        }
        sal_uInt16 nPgNum = o3tl::narrowing<sal_uInt16>(m_xPageNoNF->get_value());
        bool const usePageNo(bState && m_xPageNoCB->get_active());
        std::optional<sal_uInt16> const oPageNum(
                usePageNo ? nPgNum : std::optional<sal_Int16>());
        if (!pDesc || !pDesc->GetPageDesc()
            || (pDesc->GetPageDesc()->GetName() != sPage)
            || (pDesc->GetNumOffset() != oPageNum))
        {
            SwFormatPageDesc aFormat( m_pShell->FindPageDescByName( sPage, true ) );
            aFormat.SetNumOffset(oPageNum);
            bModified |= nullptr != rSet->Put( aFormat );
            bPageItemPut = bState;
        }
    }
    bool bIsChecked = m_xPgBrkCB->get_active();
    if ( !bPageItemPut &&
        (   bState != (m_xPageCollCB->get_saved_state() == TRISTATE_TRUE) ||
            bIsChecked != (m_xPgBrkCB->get_saved_state() == TRISTATE_TRUE) ||
            m_xPgBrkBeforeRB->get_state_changed_from_saved()    ||
            m_xPgBrkRB->get_state_changed_from_saved() ))
    {
        SvxFormatBreakItem aBreak( GetItemSet().Get( RES_BREAK ) );

        if(bIsChecked)
        {
            bool bBefore = m_xPgBrkBeforeRB->get_active();

            if (m_xPgBrkRB->get_active())
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

    if (m_xTextDirectionLB->get_value_changed_from_saved())
    {
          OUString sId = m_xTextDirectionLB->get_active_id();
          bModified |= nullptr != rSet->Put(SvxFrameDirectionItem(static_cast<SvxFrameDirection>(sId.toUInt32()), FN_TABLE_BOX_TEXTORIENTATION));
    }

    if (m_xVertOrientLB->get_value_changed_from_saved())
    {
        sal_uInt16 nOrient = USHRT_MAX;
        switch (m_xVertOrientLB->get_active())
        {
            case 0 : nOrient = text::VertOrientation::NONE; break;
            case 1 : nOrient = text::VertOrientation::CENTER; break;
            case 2 : nOrient = text::VertOrientation::BOTTOM; break;
        }
        if (nOrient != USHRT_MAX)
            bModified |= nullptr != rSet->Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));
    }

    return bModified;

}

void   SwTextFlowPage::Reset( const SfxItemSet* rSet )
{
    bool bFlowAllowed = !m_bHtmlMode || officecfg::Office::Common::Filter::HTML::Export::PrintLayout::get();
    if(bFlowAllowed)
    {
        //Inserting of the existing page templates in the list box
        const size_t nCount = m_pShell->GetPageDescCnt();

        for( size_t i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = m_pShell->GetPageDesc(i);
            m_xPageCollLB->append_text(rPageDesc.GetName());
        }

        OUString aFormatName;
        for (sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
        {
            aFormatName = SwStyleNameMapper::GetUIName(i, aFormatName);
            if (m_xPageCollLB->find_text(aFormatName) == -1)
                m_xPageCollLB->append_text(aFormatName);
        }

        if(const SvxFormatKeepItem* pKeepItem = rSet->GetItemIfSet( RES_KEEP, false ))
        {
            m_xKeepCB->set_active( pKeepItem->GetValue() );
            m_xKeepCB->save_state();
        }
        if(const SwFormatLayoutSplit* pSplitItem = rSet->GetItemIfSet( RES_LAYOUT_SPLIT, false ))
        {
            m_xSplitCB->set_active( pSplitItem->GetValue() );
        }
        else
            m_xSplitCB->set_active(true);

        m_xSplitCB->save_state();
        SplitHdl_Impl(*m_xSplitCB);

        if(const SwFormatRowSplit* pSplitItem = rSet->GetItemIfSet( RES_ROW_SPLIT, false ))
        {
            m_xSplitRowCB->set_active( pSplitItem->GetValue() );
        }
        else
            m_xSplitRowCB->set_state(TRISTATE_INDET);
        m_xSplitRowCB->save_state();

        if(m_bPageBreak)
        {
            if(const SwFormatPageDesc* pPageDescItem = rSet->GetItemIfSet( RES_PAGEDESC, false ))
            {
                OUString sPageDesc;
                const SwPageDesc* pDesc = pPageDescItem->GetPageDesc();

                ::std::optional<sal_uInt16> oNumOffset = pPageDescItem->GetNumOffset();
                if (oNumOffset)
                {
                    m_xPageNoCB->set_active(true);
                    m_xPageNoNF->set_sensitive(true);
                    m_xPageNoNF->set_value(*oNumOffset);
                }
                else
                {
                    m_xPageNoCB->set_active(false);
                    m_xPageNoNF->set_sensitive(false);
                }

                if(pDesc)
                    sPageDesc = pDesc->GetName();
                if (!sPageDesc.isEmpty() && m_xPageCollLB->find_text(sPageDesc) != -1)
                {
                    m_xPageCollLB->set_active_text(sPageDesc);
                    m_xPageCollCB->set_active(true);

                    m_xPgBrkCB->set_sensitive(true);
                    m_xPgBrkRB->set_sensitive(true);
                    m_xColBrkRB->set_sensitive(true);
                    m_xPgBrkBeforeRB->set_sensitive(true);
                    m_xPgBrkAfterRB->set_sensitive(true);
                    m_xPageCollCB->set_sensitive(true);

                    m_xPgBrkCB->set_active(true);
                    m_xColBrkRB->set_active( false );
                    m_xPgBrkBeforeRB->set_active(true);
                    m_xPgBrkAfterRB->set_active( false );
                }
                else
                {
                    m_xPageCollLB->set_active(-1);
                    m_xPageCollCB->set_active(false);
                }
            }

            if(const SvxFormatBreakItem* pPageBreak = rSet->GetItemIfSet( RES_BREAK, false ))
            {
                SvxBreak eBreak = pPageBreak->GetBreak();

                if ( eBreak != SvxBreak::NONE )
                {
                    m_xPgBrkCB->set_active(true);
                    m_xPageCollCB->set_sensitive(false);
                    m_xPageCollLB->set_sensitive(false);
                    m_xPageNoCB->set_sensitive(false);
                    m_xPageNoNF->set_sensitive(false);
                }
                switch ( eBreak )
                {
                    case SvxBreak::PageBefore:
                        m_xPgBrkRB->set_active(true);
                        m_xColBrkRB->set_active( false );
                        m_xPgBrkBeforeRB->set_active(true);
                        m_xPgBrkAfterRB->set_active( false );
                        break;
                    case SvxBreak::PageAfter:
                        m_xPgBrkRB->set_active(true);
                        m_xColBrkRB->set_active( false );
                        m_xPgBrkBeforeRB->set_active( false );
                        m_xPgBrkAfterRB->set_active(true);
                        break;
                    case SvxBreak::ColumnBefore:
                        m_xPgBrkRB->set_active( false );
                        m_xColBrkRB->set_active(true);
                        m_xPgBrkBeforeRB->set_active(true);
                        m_xPgBrkAfterRB->set_active( false );
                        break;
                    case SvxBreak::ColumnAfter:
                        m_xPgBrkRB->set_active( false );
                        m_xColBrkRB->set_active(true);
                        m_xPgBrkBeforeRB->set_active( false );
                        m_xPgBrkAfterRB->set_active(true);
                        break;
                    default:; //prevent warning
                }

            }
            if (m_xPgBrkBeforeRB->get_active())
                PageBreakPosHdl_Impl(*m_xPgBrkBeforeRB);
            else if (m_xPgBrkAfterRB->get_active())
                PageBreakPosHdl_Impl(*m_xPgBrkAfterRB);
            PageBreakHdl_Impl(*m_xPgBrkCB);
        }
    }
    else
    {
        m_xPgBrkRB->set_sensitive(false);
        m_xColBrkRB->set_sensitive(false);
        m_xPgBrkBeforeRB->set_sensitive(false);
        m_xPgBrkAfterRB->set_sensitive(false);
        m_xKeepCB->set_sensitive(false);
        m_xSplitCB->set_sensitive(false);
        m_xPgBrkCB->set_sensitive(false);
        m_xPageCollCB->set_sensitive(false);
        m_xPageCollLB->set_sensitive(false);
    }

    if(const SfxUInt16Item* pHeadlineItem = rSet->GetItemIfSet( FN_PARAM_TABLE_HEADLINE, false ))
    {
        sal_uInt16 nRep = pHeadlineItem->GetValue();
        m_xHeadLineCB->set_active(nRep > 0);
        m_xHeadLineCB->save_state();
        m_xRepeatHeaderNF->set_value(nRep);
        m_xRepeatHeaderNF->set_min(1);
        m_xRepeatHeaderNF->save_value();
    }
    if ( rSet->GetItemState(FN_TABLE_BOX_TEXTORIENTATION) > SfxItemState::DEFAULT )
    {
        SvxFrameDirection nDirection = rSet->Get(FN_TABLE_BOX_TEXTORIENTATION).GetValue();
        m_xTextDirectionLB->set_active_id(OUString::number(static_cast<sal_uInt32>(nDirection)));
    }

    if ( rSet->GetItemState(FN_TABLE_SET_VERT_ALIGN) > SfxItemState::DEFAULT )
    {
        sal_uInt16 nVert = rSet->Get(FN_TABLE_SET_VERT_ALIGN).GetValue();
        sal_uInt16 nPos = 0;
        switch(nVert)
        {
            case text::VertOrientation::NONE:     nPos = 0;   break;
            case text::VertOrientation::CENTER:   nPos = 1;   break;
            case text::VertOrientation::BOTTOM:   nPos = 2;   break;
        }
        m_xVertOrientLB->set_active(nPos);
    }

    m_xPageCollCB->save_state();
    m_xPageCollLB->save_value();
    m_xPgBrkCB->save_state();
    m_xPgBrkRB->save_state();
    m_xColBrkRB->save_state();
    m_xPgBrkBeforeRB->save_state();
    m_xPgBrkAfterRB->save_state();
    m_xPageNoCB->save_state();
    m_xPageNoNF->save_value();
    m_xTextDirectionLB->save_value();
    m_xVertOrientLB->save_value();

    HeadLineCBClickHdl(*m_xHeadLineCB);
}

void SwTextFlowPage::SetShell(SwWrtShell* pSh)
{
    m_pShell = pSh;
    m_bHtmlMode = 0 != (::GetHtmlMode(m_pShell->GetView().GetDocShell()) & HTMLMODE_ON);
    if(m_bHtmlMode)
    {
        m_xPageNoNF->set_sensitive(false);
        m_xPageNoCB->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakHdl_Impl, weld::Toggleable&, void)
{
    if (m_xPgBrkCB->get_active())
    {
        m_xPgBrkRB->set_sensitive(true);
        m_xColBrkRB->set_sensitive(true);
        m_xPgBrkBeforeRB->set_sensitive(true);
        m_xPgBrkAfterRB->set_sensitive(true);

        if (m_xPgBrkRB->get_active() && m_xPgBrkBeforeRB->get_active())
        {
            m_xPageCollCB->set_sensitive(true);

            bool bEnable = m_xPageCollCB->get_active() && m_xPageCollLB->get_count();
            m_xPageCollLB->set_sensitive(bEnable);
            if (!m_bHtmlMode)
            {
                m_xPageNoCB->set_sensitive(bEnable);
                m_xPageNoNF->set_sensitive(bEnable && m_xPageNoCB->get_active());
            }
        }
    }
    else
    {
        m_xPageCollCB->set_active(false);
        m_xPageCollCB->set_sensitive(false);
        m_xPageCollLB->set_sensitive(false);
        m_xPageNoCB->set_sensitive(false);
        m_xPageNoNF->set_sensitive(false);
        m_xPgBrkRB->set_sensitive(false);
        m_xColBrkRB->set_sensitive(false);
        m_xPgBrkBeforeRB->set_sensitive(false);
        m_xPgBrkAfterRB->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, ApplyCollClickHdl_Impl, weld::Toggleable&, void)
{
    bool bEnable = false;
    if (m_xPageCollCB->get_active() && m_xPageCollLB->get_count())
    {
        bEnable = true;
        m_xPageCollLB->set_active(0);
    }
    else
    {
        m_xPageCollLB->set_active(-1);
    }
    m_xPageCollLB->set_sensitive(bEnable);
    if (!m_bHtmlMode)
    {
        m_xPageNoCB->set_sensitive(bEnable);
        m_xPageNoNF->set_sensitive(bEnable && m_xPageNoCB->get_active());
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakPosHdl_Impl, weld::Toggleable&, void)
{
    if (!m_xPgBrkCB->get_active())
        return;

    if (m_xPgBrkBeforeRB->get_active() && m_xPgBrkRB->get_active())
    {
        m_xPageCollCB->set_sensitive(true);

        bool bEnable = m_xPageCollCB->get_active() && m_xPageCollLB->get_count();

        m_xPageCollLB->set_sensitive(bEnable);
        if (!m_bHtmlMode)
        {
            m_xPageNoCB->set_sensitive(bEnable);
            m_xPageNoNF->set_sensitive(bEnable && m_xPageNoCB->get_active());
        }
    }
    else if (m_xPgBrkAfterRB->get_active())
    {
        m_xPageCollCB->set_active(false);
        m_xPageCollCB->set_sensitive(false);
        m_xPageCollLB->set_sensitive(false);
        m_xPageNoCB->set_sensitive(false);
        m_xPageNoNF->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakTypeHdl_Impl, weld::Toggleable&, void)
{
    if (m_xColBrkRB->get_active() || m_xPgBrkAfterRB->get_active())
    {
        m_xPageCollCB->set_active(false);
        m_xPageCollCB->set_sensitive(false);
        m_xPageCollLB->set_sensitive(false);
        m_xPageNoCB->set_sensitive(false);
        m_xPageNoNF->set_sensitive(false);
    }
    else if (m_xPgBrkBeforeRB->get_active())
        PageBreakPosHdl_Impl(*m_xPgBrkBeforeRB);
}

IMPL_LINK_NOARG(SwTextFlowPage, PageNoClickHdl_Impl, weld::Toggleable&, void)
{
    m_xPageNoNF->set_sensitive(m_xPageNoCB->get_active());
}

IMPL_LINK(SwTextFlowPage, SplitHdl_Impl, weld::Toggleable&, rBox, void)
{
    m_xSplitRowCB->set_sensitive(rBox.get_active());
}

IMPL_LINK_NOARG(SwTextFlowPage, HeadLineCBClickHdl, weld::Toggleable&, void)
{
    m_xRepeatHeaderCombo->set_sensitive(m_xHeadLineCB->get_active());
}

void SwTextFlowPage::DisablePageBreak()
{
    m_bPageBreak = false;
    m_xPgBrkCB->set_sensitive(false);
    m_xPgBrkRB->set_sensitive(false);
    m_xColBrkRB->set_sensitive(false);
    m_xPgBrkBeforeRB->set_sensitive(false);
    m_xPgBrkAfterRB->set_sensitive(false);
    m_xPageCollCB->set_sensitive(false);
    m_xPageCollLB->set_sensitive(false);
    m_xPageNoCB->set_sensitive(false);
    m_xPageNoNF->set_sensitive(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
