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
    , m_aTextFilter(" .<>")
    , pTblData(0)
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
        bHtmlMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);

    bool bCTL = SW_MOD()->GetCTLOptions().IsCTLFontEnabled();
    get<VclContainer>("properties")->Show(!bHtmlMode && bCTL);

    Init();
}

void  SwFormatTablePage::Init()
{
    m_aLeftMF.SetMetricFieldMin(-999999);
    m_aRightMF.SetMetricFieldMin(-999999);

    //handler
    Link aLk = LINK( this, SwFormatTablePage, AutoClickHdl );
    m_pFullBtn->SetClickHdl( aLk );
    m_pFreeBtn->SetClickHdl( aLk );
    m_pLeftBtn->SetClickHdl( aLk );
    m_pFromLeftBtn->SetClickHdl( aLk );
    m_pRightBtn->SetClickHdl( aLk );
    m_pCenterBtn->SetClickHdl( aLk );

    aLk = LINK( this, SwFormatTablePage, UpDownLoseFocusHdl );
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

    m_pTopMF->SetLoseFocusHdl( aLk );
    m_pBottomMF->SetLoseFocusHdl( aLk );
    m_aRightMF.SetLoseFocusHdl( aLk );
    m_aLeftMF.SetLoseFocusHdl( aLk );
    m_aWidthMF.SetLoseFocusHdl( aLk );

    m_pRelWidthCB->SetClickHdl(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

IMPL_LINK( SwFormatTablePage, RelWidthClickHdl, CheckBox *, pBtn )
{
    OSL_ENSURE(pTblData, "table data not available?");
    bool bIsChecked = pBtn->IsChecked();
    sal_Int64 nLeft  = m_aLeftMF.DenormalizePercent(m_aLeftMF.GetValue(FUNIT_TWIP ));
    sal_Int64 nRight = m_aRightMF.DenormalizePercent(m_aRightMF.GetValue(FUNIT_TWIP ));
    m_aWidthMF.ShowPercent(bIsChecked);
    m_aLeftMF.ShowPercent(bIsChecked);
    m_aRightMF.ShowPercent(bIsChecked);

    if (bIsChecked)
    {
        m_aWidthMF.SetRefValue(pTblData->GetSpace());
        m_aLeftMF.SetRefValue(pTblData->GetSpace());
        m_aRightMF.SetRefValue(pTblData->GetSpace());
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

    return 0;
}

IMPL_LINK( SwFormatTablePage, AutoClickHdl, CheckBox *, pBox )
{
    bool bRestore = true,
         bLeftEnable = false,
         bRightEnable= false,
         bWidthEnable= false,
         bOthers = true;
    if ((RadioButton *)pBox == m_pFullBtn)
    {
        m_aLeftMF.SetPrcntValue(0);
        m_aRightMF.SetPrcntValue(0);
        nSaveWidth = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue(FUNIT_TWIP )));
        m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(pTblData->GetSpace() ), FUNIT_TWIP );
        bFull = true;
        bRestore = false;
    }
    else if ((RadioButton *)pBox == m_pLeftBtn)
    {
        bRightEnable = bWidthEnable = true;
        m_aLeftMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pFromLeftBtn)
    {
        bLeftEnable = bWidthEnable = true;
        m_aRightMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pRightBtn)
    {
        bLeftEnable = bWidthEnable = true;
        m_aRightMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pCenterBtn)
    {
        bLeftEnable = bWidthEnable = true;
    }
    else if ((RadioButton *) pBox == m_pFreeBtn)
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
    return 0;
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

IMPL_LINK_INLINE_START( SwFormatTablePage, UpDownLoseFocusHdl, MetricField *, pEdit )
{
    if( m_aRightMF.get() == pEdit)
        RightModify();
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwFormatTablePage, UpDownLoseFocusHdl, MetricField *, pEdit )

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
        nDiff = nRight + nLeft + nCurWidth - pTblData->GetSpace() ;
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
                    nCurWidth = pTblData->GetSpace();
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

        if( nRight + nLeft > pTblData->GetSpace() - MINLAY )
            nRight = pTblData->GetSpace() -nLeft - MINLAY;

        nCurWidth = pTblData->GetSpace() - nLeft - nRight;
    }
    if (pEdit == m_aLeftMF.get())
    {
        if(!m_pFromLeftBtn->IsChecked())
        {
            bool bCenter = m_pCenterBtn->IsChecked();
            if( bCenter )
                nRight = nLeft;
            if(nRight + nLeft > pTblData->GetSpace() - MINLAY )
            {
                nLeft  = bCenter ?  (pTblData->GetSpace() - MINLAY) /2 :
                                    (pTblData->GetSpace() - MINLAY) - nRight;
                nRight = bCenter ?  (pTblData->GetSpace() - MINLAY) /2 : nRight;
            }
            nCurWidth = pTblData->GetSpace() - nLeft - nRight;
        }
        else
        {
            //Upon changes on the left side the right margin will be changed at first,
            //thereafter the width.
            nDiff = nRight + nLeft + nCurWidth - pTblData->GetSpace() ;

            nRight -= nDiff;
            nCurWidth = pTblData->GetSpace() - nLeft - nRight;
        }
    }
    if (nCurWidth != nPrevWidth )
        m_aWidthMF.SetPrcntValue( m_aWidthMF.NormalizePercent( nCurWidth ), FUNIT_TWIP );
    m_aRightMF.SetPrcntValue( m_aRightMF.NormalizePercent( nRight ), FUNIT_TWIP );
    m_aLeftMF.SetPrcntValue( m_aLeftMF.NormalizePercent( nLeft ), FUNIT_TWIP );
    bModified = true;
}

SfxTabPage*  SwFormatTablePage::Create( vcl::Window* pParent,
                                   const SfxItemSet* rAttrSet)
{
    return new SwFormatTablePage( pParent, *rAttrSet );
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
                             (sal_uInt32)(sal_uIntPtr)m_pTextDirectionLB->GetEntryData( nPos );
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
        m_pNameED->SetText(((const SfxStringItem*)pItem)->GetValue());
        m_pNameED->SaveValue();
    }

    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP, false, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nMinTableWidth = pTblData->GetColCount() * MINLAY;

        if(pTblData->GetWidthPercent())
        {
            m_pRelWidthCB->Check(true);
            RelWidthClickHdl(m_pRelWidthCB);
            m_aWidthMF.SetPrcntValue(pTblData->GetWidthPercent(), FUNIT_CUSTOM);

            m_aWidthMF.SaveValue();
            nSaveWidth = static_cast< SwTwips >(m_aWidthMF.GetValue(FUNIT_CUSTOM));
        }
        else
        {
            m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(
                    pTblData->GetWidth()), FUNIT_TWIP);
            m_aWidthMF.SaveValue();
            nSaveWidth = pTblData->GetWidth();
            nMinTableWidth = std::min( nSaveWidth, nMinTableWidth );
        }

        m_aWidthMF.SetRefValue(pTblData->GetSpace());
        m_aWidthMF.SetLast(m_aWidthMF.NormalizePercent( pTblData->GetSpace() ));
        m_aLeftMF.SetLast(m_aLeftMF.NormalizePercent( pTblData->GetSpace() ));
        m_aRightMF.SetLast(m_aRightMF.NormalizePercent( pTblData->GetSpace() ));

        m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(
                    pTblData->GetLeftSpace()), FUNIT_TWIP);
        m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(
                    pTblData->GetRightSpace()), FUNIT_TWIP);
        m_aLeftMF.SaveValue();
        m_aRightMF.SaveValue();

        bool bSetRight = false, bSetLeft = false;
        switch( pTblData->GetAlign() )
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
                        ((const SvxULSpaceItem*)pItem)->GetUpper()), FUNIT_TWIP);
        m_pBottomMF->SetValue(m_pBottomMF->Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetLower()), FUNIT_TWIP);
        m_pTopMF->SaveValue();
        m_pBottomMF->SaveValue();
    }

    //Text direction
    if( SfxItemState::SET == rSet.GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        sal_uIntPtr nVal  = ((SvxFrameDirectionItem*)pItem)->GetValue();
        const sal_Int32 nPos = m_pTextDirectionLB->GetEntryPos( (void*) nVal );
        m_pTextDirectionLB->SelectEntryPos( nPos );
        m_pTextDirectionLB->SaveValue();
    }

    m_aWidthMF.SetMax( 2*m_aWidthMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    m_aRightMF.SetMax( m_aRightMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    m_aLeftMF.SetMax( m_aLeftMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    m_aWidthMF.SetMin( m_aWidthMF.NormalizePercent( nMinTableWidth ), FUNIT_TWIP );

}

void    SwFormatTablePage::ActivatePage( const SfxItemSet& rSet )
{
    OSL_ENSURE(pTblData, "table data not available?");
    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP ))
    {
        SwTwips nCurWidth = text::HoriOrientation::FULL != pTblData->GetAlign() ?
                                        pTblData->GetWidth() :
                                            pTblData->GetSpace();
        if(pTblData->GetWidthPercent() == 0 &&
                nCurWidth != m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue(FUNIT_TWIP )))
        {
            m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(
                            nCurWidth), FUNIT_TWIP);
            m_aWidthMF.SaveValue();
            nSaveWidth = nCurWidth;
            m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(
                            pTblData->GetLeftSpace()), FUNIT_TWIP);
            m_aLeftMF.SaveValue();
            m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(
                            pTblData->GetRightSpace()), FUNIT_TWIP);
            m_aRightMF.SaveValue();
        }
    }

}

int  SwFormatTablePage::DeactivatePage( SfxItemSet* _pSet )
{
    //os: VCL doesn't take care of making the active widget
    //in the dialog lose the focus
    m_pNameED->GrabFocus();
    //test the table name for spaces
    OUString sTblName = m_pNameED->GetText();
    if(sTblName.indexOf(' ') != -1)
    {
        MessageDialog(this, SW_RES(STR_WRONG_TABLENAME), VCL_MESSAGE_INFO).Execute();
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
                pTblData->SetWidthChanged();
                pTblData->SetLeftSpace( lLeft);
                pTblData->SetRightSpace( lRight);
            }

            SwTwips lWidth;
            if (m_pRelWidthCB->IsChecked() && m_pRelWidthCB->IsEnabled())
            {
                lWidth = pTblData->GetSpace() - lRight - lLeft;
                const sal_uInt16 nPercentWidth = m_aWidthMF.GetValue(FUNIT_CUSTOM);
                if(pTblData->GetWidthPercent() != nPercentWidth)
                {
                    pTblData->SetWidthPercent(nPercentWidth);
                    pTblData->SetWidthChanged();
                }
            }
            else
            {
                pTblData->SetWidthPercent(0);
                lWidth = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue( FUNIT_TWIP )));
            }
            pTblData->SetWidth(lWidth);

            SwTwips nColSum = 0;

            for( sal_uInt16 i = 0; i < pTblData->GetColCount(); i++)
            {
                nColSum += pTblData->GetColumns()[i].nWidth;
            }
            if(nColSum != pTblData->GetWidth())
            {
                SwTwips nMinWidth = std::min( (long)MINLAY,
                                    (long) (pTblData->GetWidth() /
                                            pTblData->GetColCount() - 1));
                SwTwips nDiff = nColSum - pTblData->GetWidth();
                while ( std::abs(nDiff) > pTblData->GetColCount() + 1 )
                {
                    SwTwips nSub = nDiff / pTblData->GetColCount();
                    for( sal_uInt16 i = 0; i < pTblData->GetColCount(); i++)
                    {
                        if(pTblData->GetColumns()[i].nWidth - nMinWidth > nSub)
                        {
                            pTblData->GetColumns()[i].nWidth -= nSub;
                            nDiff -= nSub;
                        }
                        else
                        {
                            nDiff -= pTblData->GetColumns()[i].nWidth - nMinWidth;
                            pTblData->GetColumns()[i].nWidth = nMinWidth;
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
            if(nAlign != pTblData->GetAlign())
            {
                pTblData->SetWidthChanged();
                pTblData->SetAlign(nAlign);
            }

            if(pTblData->GetWidth() != lWidth )
            {
                pTblData->SetWidthChanged();
                pTblData->SetWidth(
                    nAlign == text::HoriOrientation::FULL ? pTblData->GetSpace() : lWidth );
            }
            if(pTblData->HasWidthChanged())
                _pSet->Put(SwPtrItem(FN_TABLE_REP, pTblData));
        }
    }
    return sal_True;
}

//Description: Page column configuration
SwTableColumnPage::SwTableColumnPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableColumnPage",
        "modules/swriter/ui/tablecolumnpage.ui", &rSet)
    , pTblData(0)
    , nTableWidth(0)
    , nMinWidth(MINLAY)
    , nNoOfCols(0)
    , nNoOfVisibleCols(0)
    , bModified(false)
    , bModifyTbl(false)
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
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON));
}

//Description: Page column configuration
 SwTableColumnPage::~SwTableColumnPage()
{
}

SfxTabPage*   SwTableColumnPage::Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet)
{
    return new SwTableColumnPage( pParent, *rAttrSet );
}

void  SwTableColumnPage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState( FN_TABLE_REP, false, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nNoOfVisibleCols = pTblData->GetColCount();
        nNoOfCols = pTblData->GetAllColCount();
        nTableWidth = pTblData->GetAlign() != text::HoriOrientation::FULL &&
                            pTblData->GetAlign() != text::HoriOrientation::LEFT_AND_WIDTH?
                        pTblData->GetWidth() : pTblData->GetSpace();

        for( sal_uInt16 i = 0; i < nNoOfCols; i++ )
        {
            if( pTblData->GetColumns()[i].nWidth  < nMinWidth )
                    nMinWidth = pTblData->GetColumns()[i].nWidth;
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
    Link aLkUp = LINK( this, SwTableColumnPage, UpHdl );
    Link aLkDown = LINK( this, SwTableColumnPage, DownHdl );
    Link aLkLF = LINK( this, SwTableColumnPage, LoseFocusHdl );
    for( sal_uInt16 i = 0; i < MET_FIELDS; i++ )
    {
        aValueTbl[i] = i;
        m_aFieldArr[i].SetMetric(aMetric);
        m_aFieldArr[i].SetUpHdl( aLkUp );
        m_aFieldArr[i].SetDownHdl( aLkDown );
        m_aFieldArr[i].SetLoseFocusHdl( aLkLF );
    }
    SetMetric(*m_pSpaceED, aMetric);

    Link aLk = LINK( this, SwTableColumnPage, AutoClickHdl );
    m_pUpBtn->SetClickHdl( aLk );
    m_pDownBtn->SetClickHdl( aLk );

    aLk = LINK( this, SwTableColumnPage, ModeHdl );
    m_pModifyTableCB->SetClickHdl( aLk );
    m_pProportionalCB->SetClickHdl( aLk );
}

IMPL_LINK( SwTableColumnPage, AutoClickHdl, CheckBox *, pBox )
{
    //move display window
    if(pBox == (CheckBox *)m_pDownBtn)
    {
        if(aValueTbl[0] > 0)
        {
            for( sal_uInt16 i=0; i < MET_FIELDS; i++ )
                aValueTbl[i] -= 1;
        }
    }
    if (pBox == (CheckBox *)m_pUpBtn)
    {
        if( aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1  )
        {
            for(sal_uInt16 i=0;i < MET_FIELDS;i++)
                aValueTbl[i] += 1;
        }
    }
    for( sal_uInt16 i = 0; (i < nNoOfVisibleCols ) && ( i < MET_FIELDS); i++ )
    {
        OUString sEntry('~');
        OUString sIndex = OUString::number( aValueTbl[i] + 1 );
        sEntry += sIndex;
        m_pTextArr[i]->SetText( sEntry );

        //added by menghu for SODC_5143,12/12/2006
        OUString sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH);
        sColumnWidth = sColumnWidth.replaceFirst( "%1", sIndex );
        m_aFieldArr[i].SetAccessibleName( sColumnWidth );
    }

    m_pDownBtn->Enable(aValueTbl[0] > 0);
    m_pUpBtn->Enable(aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1 );
    UpdateCols(0);
    return 0;
}

IMPL_LINK_INLINE_START( SwTableColumnPage, UpHdl, MetricField*, pEdit )
{
    bModified = true;
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwTableColumnPage, UpHdl, MetricField*, pEdit )

IMPL_LINK_INLINE_START( SwTableColumnPage, DownHdl, MetricField*, pEdit )
{
    bModified = true;
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwTableColumnPage, DownHdl, MetricField*, pEdit )

IMPL_LINK_INLINE_START( SwTableColumnPage, LoseFocusHdl, MetricField*, pEdit )
{
    if (pEdit->IsModified())
    {
        bModified = true;
        ModifyHdl( pEdit );
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwTableColumnPage, LoseFocusHdl, MetricField*, pEdit )

IMPL_LINK( SwTableColumnPage, ModeHdl, CheckBox*, pBox )
{
    bool bCheck = pBox->IsChecked();
    if (pBox == m_pProportionalCB)
    {
        if(bCheck)
            m_pModifyTableCB->Check();
        m_pModifyTableCB->Enable(!bCheck && bModifyTbl);
    }
    return 0;
}

bool  SwTableColumnPage::FillItemSet( SfxItemSet* )
{
    for( sal_uInt16 i = 0; i < MET_FIELDS; i++ )
    {
        if (m_aFieldArr[i].HasFocus())
        {
            LoseFocusHdl(m_aFieldArr[i].get());
            break;
        }
    }

    if(bModified)
    {
        pTblData->SetColsChanged();
    }
    return bModified;
}

void   SwTableColumnPage::ModifyHdl( MetricField* pField )
{
        PercentField *pEdit = NULL;
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

        SetVisibleWidth(aValueTbl[i], static_cast< SwTwips >(pEdit->DenormalizePercent(pEdit->GetValue( FUNIT_TWIP ))) );

        UpdateCols( aValueTbl[i] );
}

void   SwTableColumnPage::UpdateCols( sal_uInt16 nAktPos )
{
    SwTwips nSum = 0;

    for( sal_uInt16 i = 0; i < nNoOfCols; i++ )
    {
        nSum += (pTblData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - nTableWidth;

    bool bModifyTable = m_pModifyTableCB->IsChecked();
    bool bProp =    m_pProportionalCB->IsChecked();

    if(!bModifyTable && !bProp )
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
    else if(bModifyTable && !bProp)
    {
        //Difference is balanced by the width of the table,
        //other columns remain unchanged.
        OSL_ENSURE(nDiff <= pTblData->GetSpace() - nTableWidth, "wrong maximum" );
        SwTwips nActSpace = pTblData->GetSpace() - nTableWidth;
        if(nDiff > nActSpace)
        {
            nTableWidth = pTblData->GetSpace();
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nActSpace );
        }
        else
        {
            nTableWidth += nDiff;
        }
    }
    else if(bModifyTable && bProp)
    {
        //All columns will be changed proportionally with,
        //the table width is adjusted accordingly.
        OSL_ENSURE(nDiff * nNoOfVisibleCols <= pTblData->GetSpace() - nTableWidth, "wrong maximum" );
        long nAdd = nDiff;
        if(nDiff * nNoOfVisibleCols > pTblData->GetSpace() - nTableWidth)
        {
            nAdd = (pTblData->GetSpace() - nTableWidth) / nNoOfVisibleCols;
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
        m_pSpaceED->SetValue(m_pSpaceED->Normalize( pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

    for( sal_uInt16 i = 0; ( i < nNoOfVisibleCols ) && ( i < MET_FIELDS ); i++)
    {
        m_aFieldArr[i].SetPrcntValue(m_aFieldArr[i].NormalizePercent(
                        GetVisibleWidth(aValueTbl[i]) ), FUNIT_TWIP);
        m_aFieldArr[i].ClearModifyFlag();
    }
}

void    SwTableColumnPage::ActivatePage( const SfxItemSet& )
{
    bPercentMode = pTblData->GetWidthPercent() != 0;
    for( sal_uInt16 i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
    {
        m_aFieldArr[i].SetRefValue(pTblData->GetWidth());
        m_aFieldArr[i].ShowPercent( bPercentMode );
    }

    const sal_uInt16 nTblAlign = pTblData->GetAlign();
    if((text::HoriOrientation::FULL != nTblAlign && nTableWidth != pTblData->GetWidth()) ||
    (text::HoriOrientation::FULL == nTblAlign && nTableWidth != pTblData->GetSpace()))
    {
        nTableWidth = text::HoriOrientation::FULL == nTblAlign ?
                                    pTblData->GetSpace() :
                                        pTblData->GetWidth();
        UpdateCols(0);
    }
    bModifyTbl = true;
    if(pTblData->GetWidthPercent() ||
                text::HoriOrientation::FULL == nTblAlign ||
                        pTblData->IsLineSelected()  )
        bModifyTbl = false;
    if(bPercentMode)
    {
        m_pModifyTableCB->Check(false);
        m_pProportionalCB->Check(false);
    }
    else if( !bModifyTbl )
    {
        m_pProportionalCB->Check(false);
        m_pModifyTableCB->Check(false);
    }
    m_pSpaceFT->Enable(!bPercentMode);
    m_pSpaceED->Enable(!bPercentMode);
    m_pModifyTableCB->Enable( !bPercentMode && bModifyTbl );
    m_pProportionalCB->Enable(!bPercentMode && bModifyTbl );

    m_pSpaceED->SetValue(m_pSpaceED->Normalize(
                pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

}

int  SwTableColumnPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
        if(text::HoriOrientation::FULL != pTblData->GetAlign() && pTblData->GetWidth() != nTableWidth)
        {
            pTblData->SetWidth(nTableWidth);
            SwTwips nDiff = pTblData->GetSpace() - pTblData->GetWidth() -
                            pTblData->GetLeftSpace() - pTblData->GetRightSpace();
            switch( pTblData->GetAlign()  )
            {
                case text::HoriOrientation::RIGHT:
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff);
                break;
                case text::HoriOrientation::LEFT:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff);
                break;
                case text::HoriOrientation::NONE:
                {
                    SwTwips nDiff2 = nDiff/2;
                    if( nDiff > 0 ||
                        (-nDiff2 < pTblData->GetRightSpace() && - nDiff2 < pTblData->GetLeftSpace()))
                    {
                        pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff2);
                        pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff2);
                    }
                    else
                    {
                        if(pTblData->GetRightSpace() > pTblData->GetLeftSpace())
                        {
                            pTblData->SetLeftSpace(0);
                            pTblData->SetRightSpace(pTblData->GetSpace() - pTblData->GetWidth());
                        }
                        else
                        {
                            pTblData->SetRightSpace(0);
                            pTblData->SetLeftSpace(pTblData->GetSpace() - pTblData->GetWidth());
                        }
                    }
                }
                break;
                case text::HoriOrientation::CENTER:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff/2);
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff/2);
                break;
                case text::HoriOrientation::LEFT_AND_WIDTH :
                    if(nDiff > pTblData->GetRightSpace())
                    {
                        pTblData->SetLeftSpace(pTblData->GetSpace() - pTblData->GetWidth());
                    }
                    pTblData->SetRightSpace(
                        pTblData->GetSpace() - pTblData->GetWidth() - pTblData->GetLeftSpace());
                break;
            }
            pTblData->SetWidthChanged();
        }
        _pSet->Put(SwPtrItem( FN_TABLE_REP, pTblData ));
    }
    return sal_True;
}

SwTwips  SwTableColumnPage::GetVisibleWidth(sal_uInt16 nPos)
{
    sal_uInt16 i=0;

    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    SwTwips nReturn = pTblData->GetColumns()[i].nWidth;
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        nReturn += pTblData->GetColumns()[++i].nWidth;

    return nReturn;
}

void SwTableColumnPage::SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth)
{
    sal_uInt16 i=0;
    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    pTblData->GetColumns()[i].nWidth = nNewWidth;
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        pTblData->GetColumns()[++i].nWidth = 0;

}

SwTableTabDlg::SwTableTabDlg(vcl::Window* pParent, SfxItemPool&,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
    : SfxTabDialog(0, pParent, "TablePropertiesDialog",
        "modules/swriter/ui/tableproperties.ui", pItemSet)
    , pShell(pSh)
    , m_nHtmlMode(::GetHtmlMode(pSh->GetView().GetDocShell()))
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    AddTabPage("table", &SwFormatTablePage::Create, 0);
    m_nTextFlowId = AddTabPage("textflow", &SwTextFlowPage::Create, 0);
    AddTabPage("columns", &SwTableColumnPage::Create, 0);
    m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BACKGROUND), 0);
    m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), 0);
}

void  SwTableTabDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nBackgroundId)
    {
        sal_Int32 nFlagType = SVX_SHOW_TBLCTL;
        if(!( m_nHtmlMode & HTMLMODE_ON ) ||
            m_nHtmlMode & HTMLMODE_SOME_STYLES)
            nFlagType |= SVX_SHOW_SELECTOR;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBorderId)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_TABLE));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTextFlowId)
    {
        ((SwTextFlowPage&)rPage).SetShell(pShell);
        const sal_uInt16 eType = pShell->GetFrmType(0,true);
        if( !(FRMTYPE_BODY & eType) )
            ((SwTextFlowPage&)rPage).DisablePageBreak();
    }
}

SwTextFlowPage::SwTextFlowPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableTextFlowPage",
        "modules/swriter/ui/tabletextflowpage.ui", &rSet)
    , pShell(0)
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
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
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
}

SfxTabPage*   SwTextFlowPage::Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet)
{
    return new SwTextFlowPage(pParent, *rAttrSet);
}

bool  SwTextFlowPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;

    //Repeat Heading
    if(m_pHeadLineCB->IsValueChangedFromSaved() ||
       m_pRepeatHeaderNF->IsValueChangedFromSaved() )
    {
        bModified |= 0 != rSet->Put(
            SfxUInt16Item(FN_PARAM_TABLE_HEADLINE, m_pHeadLineCB->IsChecked()? sal_uInt16(m_pRepeatHeaderNF->GetValue()) : 0 ));
    }
    if(m_pKeepCB->IsValueChangedFromSaved())
        bModified |= 0 != rSet->Put( SvxFmtKeepItem( m_pKeepCB->IsChecked(), RES_KEEP));

    if(m_pSplitCB->IsValueChangedFromSaved())
        bModified |= 0 != rSet->Put( SwFmtLayoutSplit( m_pSplitCB->IsChecked()));

    if(m_pSplitRowCB->IsValueChangedFromSaved())
        bModified |= 0 != rSet->Put( SwFmtRowSplit( m_pSplitRowCB->IsChecked()));

    const SvxFmtBreakItem* pBreak = (const SvxFmtBreakItem*)GetOldItem( *rSet, RES_BREAK );
    const SwFmtPageDesc* pDesc = (const SwFmtPageDesc*) GetOldItem( *rSet, RES_PAGEDESC );

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
            SwFmtPageDesc aFmt( pShell->FindPageDescByName( sPage, true ) );
            aFmt.SetNumOffset(bState ? nPgNum : 0);
            bModified |= 0 != rSet->Put( aFmt );
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
        SvxFmtBreakItem aBreak(
            (const SvxFmtBreakItem&)GetItemSet().Get( RES_BREAK ) );

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

        if ( !pBreak || !( *(const SvxFmtBreakItem*)pBreak == aBreak ) )
        {
            bModified |= 0 != rSet->Put( aBreak );
        }
    }

    if(m_pTextDirectionLB->IsValueChangedFromSaved())
    {
          bModified |= 0 != rSet->Put(
                    SvxFrameDirectionItem(
                        (SvxFrameDirection)(sal_uLong)m_pTextDirectionLB->GetEntryData(m_pTextDirectionLB->GetSelectEntryPos())
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
            bModified |= 0 != rSet->Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));
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
        const sal_uInt16 nCount = pShell->GetPageDescCnt();

        for( sal_uInt16 i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = pShell->GetPageDesc(i);
            m_pPageCollLB->InsertEntry(rPageDesc.GetName());
        }

        OUString aFmtName;
        for(sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
            if( LISTBOX_ENTRY_NOTFOUND == m_pPageCollLB->GetEntryPos(
                    aFmtName = SwStyleNameMapper::GetUIName( i, aFmtName ) ))
                m_pPageCollLB->InsertEntry( aFmtName );

        if(SfxItemState::SET == rSet->GetItemState( RES_KEEP, false, &pItem ))
        {
            m_pKeepCB->Check( ((const SvxFmtKeepItem*)pItem)->GetValue() );
            m_pKeepCB->SaveValue();
        }
        if(SfxItemState::SET == rSet->GetItemState( RES_LAYOUT_SPLIT, false, &pItem ))
        {
            m_pSplitCB->Check( ((const SwFmtLayoutSplit*)pItem)->GetValue() );
        }
        else
            m_pSplitCB->Check();

        m_pSplitCB->SaveValue();
        SplitHdl_Impl(m_pSplitCB);

        if(SfxItemState::SET == rSet->GetItemState( RES_ROW_SPLIT, false, &pItem ))
        {
            m_pSplitRowCB->Check( ((const SwFmtRowSplit*)pItem)->GetValue() );
        }
        else
            m_pSplitRowCB->SetState(TRISTATE_INDET);
        m_pSplitRowCB->SaveValue();

        if(bPageBreak)
        {
            if(SfxItemState::SET == rSet->GetItemState( RES_PAGEDESC, false, &pItem ))
            {
                OUString sPageDesc;
                const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();

                //m_pPageNoNF->SetValue(((const SwFmtPageDesc*)pItem)->GetNumOffset());
                ::boost::optional<sal_uInt16> oNumOffset = ((const SwFmtPageDesc*)pItem)->GetNumOffset();
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

                    m_pPgBrkCB->Check( true );
                    m_pColBrkRB->Check( false );
                    m_pPgBrkBeforeRB->Check( true );
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
                const SvxFmtBreakItem* pPageBreak = (const SvxFmtBreakItem*)pItem;
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
                        m_pPgBrkRB->Check( true );
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check( true );
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        m_pPgBrkRB->Check( true );
                        m_pColBrkRB->Check( false );
                        m_pPgBrkBeforeRB->Check( false );
                        m_pPgBrkAfterRB->Check( true );
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        m_pPgBrkRB->Check( false );
                        m_pColBrkRB->Check( true );
                        m_pPgBrkBeforeRB->Check( true );
                        m_pPgBrkAfterRB->Check( false );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        m_pPgBrkRB->Check( false );
                        m_pColBrkRB->Check( true );
                        m_pPgBrkBeforeRB->Check( false );
                        m_pPgBrkAfterRB->Check( true );
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
        sal_uInt16 nRep = ((const SfxUInt16Item*)pItem)->GetValue();
        m_pHeadLineCB->Check( nRep > 0 );
        m_pHeadLineCB->SaveValue();
        m_pRepeatHeaderNF->SetValue( nRep );
        m_pRepeatHeaderNF->SetMin( 1 );
        m_pRepeatHeaderNF->SaveValue();
    }
    if ( rSet->GetItemState(FN_TABLE_BOX_TEXTORIENTATION) > SfxItemState::DEFAULT )
    {
        sal_uLong nDirection = ((const SvxFrameDirectionItem&)rSet->Get(FN_TABLE_BOX_TEXTORIENTATION)).GetValue();
        m_pTextDirectionLB->SelectEntryPos(m_pTextDirectionLB->GetEntryPos( (const void*)nDirection ));
    }

    if ( rSet->GetItemState(FN_TABLE_SET_VERT_ALIGN) > SfxItemState::DEFAULT )
    {
        sal_uInt16 nVert = ((const SfxUInt16Item&)rSet->Get(FN_TABLE_SET_VERT_ALIGN)).GetValue();
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

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakHdl_Impl)
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
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, ApplyCollClickHdl_Impl)
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
    return 0;
}

IMPL_LINK( SwTextFlowPage, PageBreakPosHdl_Impl, RadioButton*, pBtn )
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
    return 0;
}

IMPL_LINK( SwTextFlowPage, PageBreakTypeHdl_Impl, RadioButton*, pBtn )
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
    return 0;
}

IMPL_LINK( SwTextFlowPage, SplitHdl_Impl, CheckBox*, pBox )
{
    m_pSplitRowCB->Enable(pBox->IsChecked());
    return 0;
}

IMPL_LINK( SwTextFlowPage, SplitRowHdl_Impl, TriStateBox*, pBox )
{
    pBox->EnableTriState(false);
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, HeadLineCBClickHdl)
{
    m_pRepeatHeaderCombo->Enable(m_pHeadLineCB->IsChecked());

    return 0;
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
