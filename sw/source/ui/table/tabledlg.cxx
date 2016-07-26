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
#include <comphelper/string.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
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

#include "access.hrc"

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>
#include <tabledlg.hxx>
#include <../../uibase/table/tablepg.hxx>
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#include <app.hrc>
#include <cmdid.h>
#include <table.hrc>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>

using namespace ::com::sun::star;

SwFormatTablePage::SwFormatTablePage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "FormatTablePage", "modules/swriter/ui/formattablepage.ui", &rSet)
    , pTableData(nullptr)
    , nSaveWidth(0)
    , nMinTableWidth(MINLAY)
    , bModified(false)
    , bFull(false)
    , bHtmlMode(false)
{
    get(m_pNameED, "name");
    get(m_pWidthFT, "widthft");
    m_aWidthMF.set(get<MetricField>("widthmf"));
    m_aLeftMF.set(get<MetricField>("leftmf"));
    m_aRightMF.set(get<MetricField>("rightmf"));
    get(m_pRelWidthCB, "relwidth");
    get(m_pFullBtn, "full");
    get(m_pLeftBtn, "left");
    get(m_pFromLeftBtn, "fromleft");
    get(m_pRightBtn, "right");
    get(m_pCenterBtn, "center");
    get(m_pFreeBtn, "free");
    get(m_pLeftFT, "leftft");
    get(m_pRightFT, "rightft");
    get(m_pTopFT, "aboveft");
    get(m_pTopMF, "abovemf");
    get(m_pBottomFT, "belowft");
    get(m_pBottomMF, "belowmf");
    get(m_pTextDirectionLB, "textdirection");

    SetExchangeSupport();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem))
        bHtmlMode = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);

    bool bCTL = SW_MOD()->GetCTLOptions().IsCTLFontEnabled();
    get<VclContainer>("properties")->Show(!bHtmlMode && bCTL);

    Init();
}

SwFormatTablePage::~SwFormatTablePage()
{
    disposeOnce();
}

void SwFormatTablePage::dispose()
{
    m_pNameED.clear();
    m_pWidthFT.clear();
    m_pRelWidthCB.clear();
    m_pFullBtn.clear();
    m_pLeftBtn.clear();
    m_pFromLeftBtn.clear();
    m_pRightBtn.clear();
    m_pCenterBtn.clear();
    m_pFreeBtn.clear();
    m_pLeftFT.clear();
    m_pRightFT.clear();
    m_pTopFT.clear();
    m_pTopMF.clear();
    m_pBottomFT.clear();
    m_pBottomMF.clear();
    m_pTextDirectionLB.clear();
    SfxTabPage::dispose();
}

void  SwFormatTablePage::Init()
{
    m_aLeftMF.SetMetricFieldMin(-999999);
    m_aRightMF.SetMetricFieldMin(-999999);

    //handler
    Link<Button*,void> aLk2 = LINK( this, SwFormatTablePage, AutoClickHdl );
    m_pFullBtn->SetClickHdl( aLk2 );
    m_pFreeBtn->SetClickHdl( aLk2 );
    m_pLeftBtn->SetClickHdl( aLk2 );
    m_pFromLeftBtn->SetClickHdl( aLk2 );
    m_pRightBtn->SetClickHdl( aLk2 );
    m_pCenterBtn->SetClickHdl( aLk2 );

    Link<SpinField&,void> aLk = LINK( this, SwFormatTablePage, UpDownHdl );
    m_pTopMF->SetUpHdl( aLk );
    m_pBottomMF->SetUpHdl( aLk );
    m_aRightMF.SetUpHdl( aLk );
    m_aLeftMF.SetUpHdl( aLk );
    m_aWidthMF.SetUpHdl( aLk );

    m_pTopMF->SetDownHdl( aLk );
    m_pBottomMF->SetDownHdl( aLk );
    m_aRightMF.SetDownHdl( aLk );
    m_aLeftMF.SetDownHdl( aLk );
    m_aWidthMF.SetDownHdl( aLk );

    Link<Control&,void> aLk3 = LINK( this, SwFormatTablePage, LoseFocusHdl );
    m_pTopMF->SetLoseFocusHdl( aLk3 );
    m_pBottomMF->SetLoseFocusHdl( aLk3 );
    m_aRightMF.SetLoseFocusHdl( aLk3 );
    m_aLeftMF.SetLoseFocusHdl( aLk3 );
    m_aWidthMF.SetLoseFocusHdl( aLk3 );

    m_pRelWidthCB->SetClickHdl(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

IMPL_LINK_TYPED( SwFormatTablePage, RelWidthClickHdl, Button*, p, void )
{
    CheckBox* pBtn = static_cast<CheckBox*>(p);
    OSL_ENSURE(pTableData, "table data not available?");
    bool bIsChecked = pBtn->IsChecked();
    sal_Int64 nLeft  = m_aLeftMF.DenormalizePercent(m_aLeftMF.GetValue(FUNIT_TWIP ));
    sal_Int64 nRight = m_aRightMF.DenormalizePercent(m_aRightMF.GetValue(FUNIT_TWIP ));
    m_aWidthMF.ShowPercent(bIsChecked);
    m_aLeftMF.ShowPercent(bIsChecked);
    m_aRightMF.ShowPercent(bIsChecked);

    if (bIsChecked)
    {
        m_aWidthMF.SetRefValue(pTableData->GetSpace());
        m_aLeftMF.SetRefValue(pTableData->GetSpace());
        m_aRightMF.SetRefValue(pTableData->GetSpace());
        m_aLeftMF.SetMetricFieldMin(0); //will be overwritten by the Percentfield
        m_aRightMF.SetMetricFieldMin(0); //dito
        m_aLeftMF.SetMetricFieldMax(99);
        m_aRightMF.SetMetricFieldMax(99);
        m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(nLeft ), FUNIT_TWIP );
        m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(nRight ), FUNIT_TWIP );
    }
    else
        ModifyHdl(m_aLeftMF.get());    //correct values again

    if(m_pFreeBtn->IsChecked())
    {
        bool bEnable = !pBtn->IsChecked();
        m_aRightMF.Enable(bEnable);
        m_pRightFT->Enable(bEnable);
    }
    bModified = true;
}

IMPL_LINK_TYPED( SwFormatTablePage, AutoClickHdl, Button*, pControl, void )
{
    bool bRestore = true,
         bLeftEnable = false,
         bRightEnable= false,
         bWidthEnable= false,
         bOthers = true;
    if (pControl == m_pFullBtn)
    {
        m_aLeftMF.SetPrcntValue(0);
        m_aRightMF.SetPrcntValue(0);
        nSaveWidth = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue(FUNIT_TWIP )));
        m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(pTableData->GetSpace() ), FUNIT_TWIP );
        bFull = true;
        bRestore = false;
    }
    else if (pControl == m_pLeftBtn)
    {
        bRightEnable = bWidthEnable = true;
        m_aLeftMF.SetPrcntValue(0);
    }
    else if (pControl == m_pFromLeftBtn)
    {
        bLeftEnable = bWidthEnable = true;
        m_aRightMF.SetPrcntValue(0);
    }
    else if (pControl == m_pRightBtn)
    {
        bLeftEnable = bWidthEnable = true;
        m_aRightMF.SetPrcntValue(0);
    }
    else if (pControl == m_pCenterBtn)
    {
        bLeftEnable = bWidthEnable = true;
    }
    else if (pControl == m_pFreeBtn)
    {
        RightModify();
        bLeftEnable = true;
        bWidthEnable = true;
        bOthers = false;
    }
    m_aLeftMF.Enable(bLeftEnable);
    m_pLeftFT->Enable(bLeftEnable);
    m_aWidthMF.Enable(bWidthEnable);
    m_pWidthFT->Enable(bWidthEnable);
    if ( bOthers )
    {
        m_aRightMF.Enable(bRightEnable);
        m_pRightFT->Enable(bRightEnable);
        m_pRelWidthCB->Enable(bWidthEnable);
    }

    if(bFull && bRestore)
    {
        //After being switched on automatic, the width was pinned
        //in order to restore the width while switching back to.
        bFull = false;
        m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(nSaveWidth ), FUNIT_TWIP );
    }
    ModifyHdl(m_aWidthMF.get());
    bModified = true;
}

void SwFormatTablePage::RightModify()
{
    if(m_pFreeBtn->IsChecked())
    {
        bool bEnable = m_aRightMF.GetValue() == 0;
        m_pRelWidthCB->Enable(bEnable);
        if ( !bEnable )
        {
            m_pRelWidthCB->Check(false);
            RelWidthClickHdl(m_pRelWidthCB);
        }
        bEnable = m_pRelWidthCB->IsChecked();
        m_aRightMF.Enable(!bEnable);
        m_pRightFT->Enable(!bEnable);
    }
}

IMPL_LINK_TYPED( SwFormatTablePage, LoseFocusHdl, Control&, rControl, void )
{
    UpDownHdl(static_cast<SpinField&>(rControl));
}
IMPL_LINK_TYPED( SwFormatTablePage, UpDownHdl, SpinField&, rEdit, void )
{
    if( m_aRightMF.get() == &rEdit)
        RightModify();
    ModifyHdl( &rEdit );
}

void  SwFormatTablePage::ModifyHdl(const Edit * pEdit)
{
    SwTwips nCurWidth  = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue( FUNIT_TWIP )));
    SwTwips nPrevWidth = nCurWidth;
    SwTwips nRight = static_cast< SwTwips >(m_aRightMF.DenormalizePercent(m_aRightMF.GetValue( FUNIT_TWIP )));
    SwTwips nLeft  = static_cast< SwTwips >(m_aLeftMF.DenormalizePercent(m_aLeftMF.GetValue( FUNIT_TWIP )));
    SwTwips nDiff;

    if (pEdit == m_aWidthMF.get())
    {
        if( nCurWidth < MINLAY )
            nCurWidth = MINLAY;
        nDiff = nRight + nLeft + nCurWidth - pTableData->GetSpace() ;
        //right aligned: only change the left margin
        if(m_pRightBtn->IsChecked())
            nLeft -= nDiff;
        //left aligned: only change the right margin
        else if(m_pLeftBtn->IsChecked())
            nRight -= nDiff;
        //left margin and width allowed - first right - then left
        else if(m_pFromLeftBtn->IsChecked())
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
        else if(m_pCenterBtn->IsChecked())
        {
            if((nLeft != nRight))
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
        else if(m_pFreeBtn->IsChecked())
        {
            nLeft -= nDiff/2;
            nRight -= nDiff/2;
        }
    }
    if (pEdit == m_aRightMF.get())
    {

        if( nRight + nLeft > pTableData->GetSpace() - MINLAY )
            nRight = pTableData->GetSpace() -nLeft - MINLAY;

        nCurWidth = pTableData->GetSpace() - nLeft - nRight;
    }
    if (pEdit == m_aLeftMF.get())
    {
        if(!m_pFromLeftBtn->IsChecked())
        {
            bool bCenter = m_pCenterBtn->IsChecked();
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
        m_aWidthMF.SetPrcntValue( m_aWidthMF.NormalizePercent( nCurWidth ), FUNIT_TWIP );
    m_aRightMF.SetPrcntValue( m_aRightMF.NormalizePercent( nRight ), FUNIT_TWIP );
    m_aLeftMF.SetPrcntValue( m_aLeftMF.NormalizePercent( nLeft ), FUNIT_TWIP );
    bModified = true;
}

VclPtr<SfxTabPage> SwFormatTablePage::Create( vcl::Window* pParent,
                                              const SfxItemSet* rAttrSet)
{
    return VclPtr<SwFormatTablePage>::Create( pParent, *rAttrSet );
}

bool  SwFormatTablePage::FillItemSet( SfxItemSet* rCoreSet )
{
    //Test if one of the controls still has the focus
    if (m_aWidthMF.HasFocus())
        ModifyHdl(m_aWidthMF.get());
    else if (m_aLeftMF.HasFocus())
        ModifyHdl(m_aLeftMF.get());
    else if(m_aRightMF.HasFocus())
        ModifyHdl(m_aRightMF.get());
    else if(m_pTopMF->HasFocus())
        ModifyHdl(m_pTopMF);
    else if(m_pBottomMF->HasFocus())
        ModifyHdl(m_pBottomMF);

    if(bModified)
    {
        if( m_pBottomMF->IsValueChangedFromSaved() ||
            m_pTopMF->IsValueChangedFromSaved() )
        {
            SvxULSpaceItem aULSpace(RES_UL_SPACE);
            aULSpace.SetUpper( m_pTopMF->Denormalize(m_pTopMF->GetValue( FUNIT_TWIP )));
            aULSpace.SetLower( m_pBottomMF->Denormalize(m_pBottomMF->GetValue( FUNIT_TWIP )));
            rCoreSet->Put(aULSpace);
        }

    }
    if(m_pNameED->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxStringItem( FN_PARAM_TABLE_NAME, m_pNameED->GetText()));
        bModified = true;
    }

    if( m_pTextDirectionLB->IsVisible() )
    {
        const sal_Int32 nPos = m_pTextDirectionLB->GetSelectEntryPos();
        if ( m_pTextDirectionLB->IsValueChangedFromSaved() )
        {
            const sal_uInt32 nDirection =
                             (sal_uInt32)reinterpret_cast<sal_uIntPtr>(m_pTextDirectionLB->GetEntryData( nPos ));
            rCoreSet->Put( SvxFrameDirectionItem( (SvxFrameDirection)nDirection, RES_FRAMEDIR));
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
        m_pNameED->Disable();
        m_pTopFT->Hide();
        m_pTopMF->Hide();
        m_pBottomFT->Hide();
        m_pBottomMF->Hide();
        m_pFreeBtn->Enable(false);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    m_aWidthMF.SetMetric(aMetric);
    m_aRightMF.SetMetric(aMetric);
    m_aLeftMF.SetMetric(aMetric);
    SetMetric(*m_pTopMF, aMetric);
    SetMetric(*m_pBottomMF, aMetric);

    //Name
    if(SfxItemState::SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, false, &pItem ))
    {
        m_pNameED->SetText(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_pNameED->SaveValue();
    }

    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP, false, &pItem ))
    {
        pTableData = static_cast<SwTableRep*>(static_cast<const SwPtrItem*>( pItem)->GetValue());
        nMinTableWidth = pTableData->GetColCount() * MINLAY;

        if(pTableData->GetWidthPercent())
        {
            m_pRelWidthCB->Check();
            RelWidthClickHdl(m_pRelWidthCB);
            m_aWidthMF.SetPrcntValue(pTableData->GetWidthPercent(), FUNIT_CUSTOM);

            m_aWidthMF.SaveValue();
            nSaveWidth = static_cast< SwTwips >(m_aWidthMF.GetValue(FUNIT_CUSTOM));
        }
        else
        {
            m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(
                    pTableData->GetWidth()), FUNIT_TWIP);
            m_aWidthMF.SaveValue();
            nSaveWidth = pTableData->GetWidth();
            nMinTableWidth = std::min( nSaveWidth, nMinTableWidth );
        }

        m_aWidthMF.SetRefValue(pTableData->GetSpace());
        m_aWidthMF.SetLast(m_aWidthMF.NormalizePercent( pTableData->GetSpace() ));
        m_aLeftMF.SetLast(m_aLeftMF.NormalizePercent( pTableData->GetSpace() ));
        m_aRightMF.SetLast(m_aRightMF.NormalizePercent( pTableData->GetSpace() ));

        m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(
                    pTableData->GetLeftSpace()), FUNIT_TWIP);
        m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(
                    pTableData->GetRightSpace()), FUNIT_TWIP);
        m_aLeftMF.SaveValue();
        m_aRightMF.SaveValue();

        bool bSetRight = false, bSetLeft = false;
        switch( pTableData->GetAlign() )
        {
            case text::HoriOrientation::NONE:
                m_pFreeBtn->Check();
                if(m_pRelWidthCB->IsChecked())
                    bSetRight = true;
            break;
            case text::HoriOrientation::FULL:
            {
                bSetRight = bSetLeft = true;
                m_pFullBtn->Check();
                m_aWidthMF.Enable(false);
                m_pRelWidthCB->Enable(false);
                m_pWidthFT->Enable(false);
            }
            break;
            case text::HoriOrientation::LEFT:
            {
                bSetLeft = true;
                m_pLeftBtn->Check();
            }
            break;
            case text::HoriOrientation::LEFT_AND_WIDTH :
            {
                bSetRight = true;
                m_pFromLeftBtn->Check();
            }
            break;
            case text::HoriOrientation::RIGHT:
            {
                bSetRight = true;
                m_pRightBtn->Check();
            }
            break;
            case text::HoriOrientation::CENTER:
            {
                bSetRight = true;
                m_pCenterBtn->Check();
            }
            break;
        }
        if ( bSetRight )
        {
            m_aRightMF.Enable(false);
            m_pRightFT->Enable(false);
        }
        if ( bSetLeft )
        {
            m_aLeftMF.Enable(false);
            m_pLeftFT->Enable(false);
        }

    }

    //Margins
    if(SfxItemState::SET == rSet.GetItemState( RES_UL_SPACE, false,&pItem ))
    {
        m_pTopMF->SetValue(m_pTopMF->Normalize(
                        static_cast<const SvxULSpaceItem*>(pItem)->GetUpper()), FUNIT_TWIP);
        m_pBottomMF->SetValue(m_pBottomMF->Normalize(
                        static_cast<const SvxULSpaceItem*>(pItem)->GetLower()), FUNIT_TWIP);
        m_pTopMF->SaveValue();
        m_pBottomMF->SaveValue();
    }

    //Text direction
    if( SfxItemState::SET == rSet.GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        sal_uIntPtr nVal  = static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue();
        const sal_Int32 nPos = m_pTextDirectionLB->GetEntryPos( reinterpret_cast<void*>(nVal) );
        m_pTextDirectionLB->SelectEntryPos( nPos );
        m_pTextDirectionLB->SaveValue();
    }

    m_aWidthMF.SetMax( 2*m_aWidthMF.NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_aRightMF.SetMax( m_aRightMF.NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_aLeftMF.SetMax( m_aLeftMF.NormalizePercent( pTableData->GetSpace() ), FUNIT_TWIP );
    m_aWidthMF.SetMin( m_aWidthMF.NormalizePercent( nMinTableWidth ), FUNIT_TWIP );

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
                nCurWidth != m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue(FUNIT_TWIP )))
        {
            m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(
                            nCurWidth), FUNIT_TWIP);
            m_aWidthMF.SaveValue();
            nSaveWidth = nCurWidth;
            m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(
                            pTableData->GetLeftSpace()), FUNIT_TWIP);
            m_aLeftMF.SaveValue();
            m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(
                            pTableData->GetRightSpace()), FUNIT_TWIP);
            m_aRightMF.SaveValue();
        }
    }

}

SfxTabPage::sfxpg SwFormatTablePage::DeactivatePage( SfxItemSet* _pSet )
{
    //os: VCL doesn't take care of making the active widget
    //in the dialog lose the focus
    m_pNameED->GrabFocus();
    //test the table name for spaces
    OUString sTableName = m_pNameED->GetText();
    if(sTableName.indexOf(' ') != -1)
    {
        ScopedVclPtrInstance<MessageDialog>(this, SW_RES(STR_WRONG_TABLENAME), VCL_MESSAGE_INFO)->Execute();
        m_pNameED->GrabFocus();
        return KEEP_PAGE;
    }
    if(_pSet)
    {
        FillItemSet(_pSet);
        if(bModified)
        {
            SwTwips lLeft  = static_cast< SwTwips >(m_aLeftMF.DenormalizePercent(m_aLeftMF.GetValue( FUNIT_TWIP )));
            SwTwips lRight = static_cast< SwTwips >(m_aRightMF.DenormalizePercent(m_aRightMF.GetValue( FUNIT_TWIP )));

            if( m_aLeftMF.GetText() != m_aLeftMF.GetSavedValue() ||
                m_aRightMF.GetText() != m_aRightMF.GetSavedValue() )
            {
                pTableData->SetWidthChanged();
                pTableData->SetLeftSpace( lLeft);
                pTableData->SetRightSpace( lRight);
            }

            SwTwips lWidth;
            if (m_pRelWidthCB->IsChecked() && m_pRelWidthCB->IsEnabled())
            {
                lWidth = pTableData->GetSpace() - lRight - lLeft;
                const sal_uInt16 nPercentWidth = m_aWidthMF.GetValue(FUNIT_CUSTOM);
                if(pTableData->GetWidthPercent() != nPercentWidth)
                {
                    pTableData->SetWidthPercent(nPercentWidth);
                    pTableData->SetWidthChanged();
                }
            }
            else
            {
                pTableData->SetWidthPercent(0);
                lWidth = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue( FUNIT_TWIP )));
            }
            pTableData->SetWidth(lWidth);

            SwTwips nColSum = 0;

            for( sal_uInt16 i = 0; i < pTableData->GetColCount(); i++)
            {
                nColSum += pTableData->GetColumns()[i].nWidth;
            }
            if(nColSum != pTableData->GetWidth())
            {
                SwTwips nMinWidth = std::min( (long)MINLAY,
                                    (long) (pTableData->GetWidth() /
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
            if(m_pRightBtn->IsChecked())
                nAlign = text::HoriOrientation::RIGHT;
            else if(m_pLeftBtn->IsChecked())
                nAlign = text::HoriOrientation::LEFT;
            else if(m_pFromLeftBtn->IsChecked())
                nAlign = text::HoriOrientation::LEFT_AND_WIDTH;
            else if(m_pCenterBtn->IsChecked())
                nAlign = text::HoriOrientation::CENTER;
            else if(m_pFreeBtn->IsChecked())
                nAlign = text::HoriOrientation::NONE;
            else if(m_pFullBtn->IsChecked())
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
    return LEAVE_PAGE;
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
    Init((SfxItemState::SET == rSet.GetItemState( SID_HTML_MODE, false,&pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON));
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

VclPtr<SfxTabPage> SwTableColumnPage::Create( vcl::Window* pParent,
                                              const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTableColumnPage>::Create( pParent, *rAttrSet );
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

IMPL_LINK_TYPED( SwTableColumnPage, AutoClickHdl, Button*, pControl, void )
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

        //added by menghu for SODC_5143,12/12/2006
        OUString sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH);
        sColumnWidth = sColumnWidth.replaceFirst( "%1", sIndex );
        m_aFieldArr[i].SetAccessibleName( sColumnWidth );
    }

    m_pDownBtn->Enable(aValueTable[0] > 0);
    m_pUpBtn->Enable(aValueTable[ MET_FIELDS -1 ] < nNoOfVisibleCols -1 );
    UpdateCols(0);
}

IMPL_LINK_TYPED( SwTableColumnPage, UpHdl, SpinField&, rEdit, void )
{
    bModified = true;
    ModifyHdl( static_cast<MetricField*>(&rEdit) );
}

IMPL_LINK_TYPED( SwTableColumnPage, DownHdl, SpinField&, rEdit, void )
{
    bModified = true;
    ModifyHdl( static_cast<MetricField*>(&rEdit) );
}

IMPL_LINK_TYPED( SwTableColumnPage, LoseFocusHdl, Control&, rControl, void )
{
    MetricField* pEdit = static_cast<MetricField*>(&rControl);
    if (pEdit->IsModified())
    {
        bModified = true;
        ModifyHdl( pEdit );
    }
}

IMPL_LINK_TYPED( SwTableColumnPage, ModeHdl, Button*, pBox, void )
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

void   SwTableColumnPage::ModifyHdl( MetricField* pField )
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

void SwTableColumnPage::UpdateCols( sal_uInt16 nAktPos )
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
            if( ++nAktPos == nNoOfVisibleCols)
            {
                nAktPos = 0;
                ++nLoopCount;
                //#i101353# in small tables it might not be possible to balance column width
                if( nLoopCount > 1 )
                    break;
            }
            if( nDiff < 0 )
            {
                SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
                nDiff = 0;
            }
            else if( GetVisibleWidth(nAktPos) >= nDiff + nMinWidth )
            {
                SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
                nDiff = 0;
            }
            if( nDiff > 0 && GetVisibleWidth(nAktPos) > nMinWidth )
            {
                if( nDiff >= (GetVisibleWidth(nAktPos) - nMinWidth) )
                {
                    nDiff -= (GetVisibleWidth(nAktPos) - nMinWidth);
                    SetVisibleWidth(nAktPos, nMinWidth);
                }
                else
                {
                    nDiff = 0;
                    SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
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
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nActSpace );
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
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nAdd );
            nDiff = nAdd;
        }
        if(nAdd)
            for( sal_uInt16 i = 0; i < nNoOfVisibleCols; i++ )
            {
                if(i == nAktPos)
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

SfxTabPage::sfxpg SwTableColumnPage::DeactivatePage( SfxItemSet* _pSet )
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
    return LEAVE_PAGE;
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

SwTableTabDlg::SwTableTabDlg(vcl::Window* pParent, SfxItemPool&,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
    : SfxTabDialog(pParent, "TablePropertiesDialog",
        "modules/swriter/ui/tableproperties.ui", pItemSet)
    , pShell(pSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    AddTabPage("table", &SwFormatTablePage::Create, nullptr);
    m_nTextFlowId = AddTabPage("textflow", &SwTextFlowPage::Create, nullptr);
    AddTabPage("columns", &SwTableColumnPage::Create, nullptr);
    m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BACKGROUND), nullptr);
    m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), nullptr);
}

void  SwTableTabDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nBackgroundId)
    {
        SvxBackgroundTabFlags nFlagType = SvxBackgroundTabFlags::SHOW_TBLCTL | SvxBackgroundTabFlags::SHOW_SELECTOR;
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
    get(m_pPageNoFT, "pagenoft");
    get(m_pPageNoNF, "pagenonf");

    get(m_pSplitCB, "split");
    get(m_pSplitRowCB, "splitrow");
    get(m_pKeepCB, "keep");
    get(m_pHeadLineCB, "headline");

    get(m_pRepeatHeaderCombo, "repeatheader");
    get(m_pRepeatHeaderNF, "repeatheadernf");

    get(m_pTextDirectionLB, "textdirection");
    get(m_pVertOrientLB, "vertorient");

    m_pPgBrkRB->SetAccessibleRelationMemberOf(m_pPgBrkCB);
    m_pColBrkRB->SetAccessibleRelationMemberOf(m_pPgBrkCB);
    m_pPgBrkBeforeRB->SetAccessibleRelationMemberOf(m_pPgBrkCB);
    m_pPgBrkAfterRB->SetAccessibleRelationMemberOf(m_pPgBrkCB);
    m_pPageCollLB->SetAccessibleName(m_pPageCollCB->GetText());

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
    m_pSplitCB->SetClickHdl(
        LINK( this, SwTextFlowPage, SplitHdl_Impl));
    m_pSplitRowCB->SetClickHdl(
        LINK( this, SwTextFlowPage, SplitRowHdl_Impl));
    m_pHeadLineCB->SetClickHdl( LINK( this, SwTextFlowPage, HeadLineCBClickHdl ) );

#ifndef SW_FILEFORMAT_40
    const SfxPoolItem *pItem;
    if(SfxItemState::SET == rSet.GetItemState( SID_HTML_MODE, false,&pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
#endif
    {
        m_pKeepCB->Hide();
        m_pSplitCB->Hide();
        m_pSplitRowCB->Hide();
    }

    HeadLineCBClickHdl();
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
    m_pPageNoFT.clear();
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

VclPtr<SfxTabPage> SwTextFlowPage::Create( vcl::Window* pParent,
                                           const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTextFlowPage>::Create(pParent, *rAttrSet);
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

    const SvxFormatBreakItem* pBreak = static_cast<const SvxFormatBreakItem*>(GetOldItem( *rSet, RES_BREAK ));
    const SwFormatPageDesc* pDesc = static_cast<const SwFormatPageDesc*>(GetOldItem( *rSet, RES_PAGEDESC ));

    bool bState = m_pPageCollCB->IsChecked();

    //If we have a page style, then there's no break
    bool bPageItemPut = false;
    if ( bState != (m_pPageCollCB->GetSavedValue() == 1) ||
         ( bState &&
           m_pPageCollLB->IsValueChangedFromSaved() )
           || (m_pPageNoNF->IsEnabled() && m_pPageNoNF->IsValueModified()) )
    {
        OUString sPage;

        if ( bState )
        {
            sPage = m_pPageCollLB->GetSelectEntry();
        }
        sal_uInt16 nPgNum = static_cast< sal_uInt16 >(m_pPageNoNF->GetValue());
        if ( !pDesc || !pDesc->GetPageDesc() ||
            ( pDesc->GetPageDesc() && ((pDesc->GetPageDesc()->GetName() != sPage) ||
                    !comphelper::string::equals(m_pPageNoNF->GetSavedValue(), nPgNum))))
        {
            SwFormatPageDesc aFormat( pShell->FindPageDescByName( sPage, true ) );
            aFormat.SetNumOffset(bState ? nPgNum : 0);
            bModified |= nullptr != rSet->Put( aFormat );
            bPageItemPut = bState;
        }
    }
    bool bIsChecked = m_pPgBrkCB->IsChecked();
    if ( !bPageItemPut &&
        (   bState != (m_pPageCollCB->GetSavedValue() == 1) ||
            bIsChecked != (m_pPgBrkCB->GetSavedValue() ==1) ||
            m_pPgBrkBeforeRB->IsValueChangedFromSaved()    ||
            m_pPgBrkRB->IsValueChangedFromSaved() ))
    {
        SvxFormatBreakItem aBreak(
            static_cast<const SvxFormatBreakItem&>(GetItemSet().Get( RES_BREAK )) );

        if(bIsChecked)
        {
            bool bBefore = m_pPgBrkBeforeRB->IsChecked();

            if ( m_pPgBrkRB->IsChecked() )
            {
                if ( bBefore )
                    aBreak.SetValue( SVX_BREAK_PAGE_BEFORE );
                else
                    aBreak.SetValue( SVX_BREAK_PAGE_AFTER );
            }
            else
            {
                if ( bBefore )
                    aBreak.SetValue( SVX_BREAK_COLUMN_BEFORE );
                else
                    aBreak.SetValue( SVX_BREAK_COLUMN_AFTER );
            }
        }
        else
        {
                aBreak.SetValue( SVX_BREAK_NONE );
        }

        if ( !pBreak || !( *pBreak == aBreak ) )
        {
            bModified |= nullptr != rSet->Put( aBreak );
        }
    }

    if(m_pTextDirectionLB->IsValueChangedFromSaved())
    {
          bModified |= nullptr != rSet->Put(
                    SvxFrameDirectionItem(
                        (SvxFrameDirection)reinterpret_cast<sal_uLong>(m_pTextDirectionLB->GetSelectEntryData())
                        , FN_TABLE_BOX_TEXTORIENTATION));
    }

    if(m_pVertOrientLB->IsValueChangedFromSaved())
    {
        sal_uInt16 nOrient = USHRT_MAX;
        switch(m_pVertOrientLB->GetSelectEntryPos())
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

                //m_pPageNoNF->SetValue(static_cast<const SwFormatPageDesc*>(pItem)->GetNumOffset());
                ::boost::optional<sal_uInt16> oNumOffset = static_cast<const SwFormatPageDesc*>(pItem)->GetNumOffset();
                if (oNumOffset)
                    m_pPageNoNF->SetValue(oNumOffset.get());
                else
                    m_pPageNoNF->Enable(false);

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
                SvxBreak eBreak = (SvxBreak)pPageBreak->GetValue();

                if ( eBreak != SVX_BREAK_NONE )
                {
                    m_pPgBrkCB->Check();
                    m_pPageCollCB->Enable(false);
                    m_pPageCollLB->Enable(false);
                    m_pPageNoFT->Enable(false);
                    m_pPageNoNF->Enable(false);
                }
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        m_pPgBrkRB->Check();
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check();
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        m_pPgBrkRB->Check();
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check( false );
                        m_pPgBrkAfterRB->Check();
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        m_pPgBrkRB->Check( false );
                        m_pColBrkRB->Check();
                        m_pPgBrkBeforeRB->Check();
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
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
        sal_uLong nDirection = static_cast<const SvxFrameDirectionItem&>(rSet->Get(FN_TABLE_BOX_TEXTORIENTATION)).GetValue();
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
    m_pPageNoNF->SaveValue();
    m_pTextDirectionLB->SaveValue();
    m_pVertOrientLB->SaveValue();

    HeadLineCBClickHdl();
}

void SwTextFlowPage::SetShell(SwWrtShell* pSh)
{
    pShell = pSh;
    bHtmlMode = 0 != (::GetHtmlMode(pShell->GetView().GetDocShell()) & HTMLMODE_ON);
    if(bHtmlMode)
    {
        m_pPageNoNF->Enable(false);
        m_pPageNoFT->Enable(false);
    }
}

IMPL_LINK_NOARG_TYPED(SwTextFlowPage, PageBreakHdl_Impl, Button*, void)
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
                    m_pPageNoFT->Enable(bEnable);
                    m_pPageNoNF->Enable(bEnable);
                }
            }
    }
    else
    {
            m_pPageCollCB->Check( false );
            m_pPageCollCB->Enable(false);
            m_pPageCollLB->Enable(false);
            m_pPageNoFT->Enable(false);
            m_pPageNoNF->Enable(false);
            m_pPgBrkRB->       Enable(false);
            m_pColBrkRB->      Enable(false);
            m_pPgBrkBeforeRB-> Enable(false);
            m_pPgBrkAfterRB->  Enable(false);
    }
}

IMPL_LINK_NOARG_TYPED(SwTextFlowPage, ApplyCollClickHdl_Impl, Button*, void)
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
        m_pPageNoFT->Enable(bEnable);
        m_pPageNoNF->Enable(bEnable);
    }
}

IMPL_LINK_TYPED( SwTextFlowPage, PageBreakPosHdl_Impl, Button*, pBtn, void )
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
                m_pPageNoFT->Enable(bEnable);
                m_pPageNoNF->Enable(bEnable);
            }
        }
        else if (pBtn == m_pPgBrkAfterRB)
        {
            m_pPageCollCB->Check( false );
            m_pPageCollCB->Enable(false);
            m_pPageCollLB->Enable(false);
            m_pPageNoFT->Enable(false);
            m_pPageNoNF->Enable(false);
        }
    }
}

IMPL_LINK_TYPED( SwTextFlowPage, PageBreakTypeHdl_Impl, Button*, pBtn, void )
{
    if ( pBtn == m_pColBrkRB || m_pPgBrkAfterRB->IsChecked() )
    {
        m_pPageCollCB->Check(false);
        m_pPageCollCB->Enable(false);
        m_pPageCollLB->Enable(false);
        m_pPageNoFT->Enable(false);
        m_pPageNoNF->Enable(false);
    }
    else if ( m_pPgBrkBeforeRB->IsChecked() )
        PageBreakPosHdl_Impl(m_pPgBrkBeforeRB);
}

IMPL_LINK_TYPED( SwTextFlowPage, SplitHdl_Impl, Button*, pBox, void )
{
    m_pSplitRowCB->Enable(static_cast<CheckBox*>(pBox)->IsChecked());
}

IMPL_STATIC_LINK_TYPED(
    SwTextFlowPage, SplitRowHdl_Impl, Button*, pBox, void )
{
    static_cast<TriStateBox*>(pBox)->EnableTriState(false);
}

IMPL_LINK_NOARG_TYPED(SwTextFlowPage, HeadLineCBClickHdl, Button*, void)
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
    m_pPageNoFT->Disable();
    m_pPageNoNF->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
