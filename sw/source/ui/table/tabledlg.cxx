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
#include <tablepg.hxx>
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#include <cmdid.h>
#include <table.hrc>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>

using namespace ::com::sun::star;


SwFormatTablePage::SwFormatTablePage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "FormatTablePage", "modules/swriter/ui/formattablepage.ui", rSet)
    , pTblData(0)
    , nSaveWidth(0)
    , nMinTableWidth(MINLAY)
    , bModified(sal_False)
    , bFull(0)
    , bHtmlMode(sal_False)
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
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem))
        bHtmlMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);

    bool bCTL = SW_MOD()->GetCTLOptions().IsCTLFontEnabled();
    get<VclContainer>("properties")->Show(!bHtmlMode && bCTL);

    Init();
}


void  SwFormatTablePage::Init()
{
    m_aLeftMF.SetMetricFieldMin(-999999);
    m_aRightMF.SetMetricFieldMin(-999999);

    // handler
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
    sal_Bool bIsChecked = pBtn->IsChecked();
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
        m_aLeftMF.SetMetricFieldMin(0); // will be overwritten by the Percentfield
        m_aRightMF.SetMetricFieldMin(0); //                 -""-
        m_aLeftMF.SetMetricFieldMax(99); //
        m_aRightMF.SetMetricFieldMax(99); //
        m_aLeftMF.SetPrcntValue(m_aLeftMF.NormalizePercent(nLeft ), FUNIT_TWIP );
        m_aRightMF.SetPrcntValue(m_aRightMF.NormalizePercent(nRight ), FUNIT_TWIP );
    }
    else
        ModifyHdl(m_aLeftMF.get());    // correct values again

    if(m_pFreeBtn->IsChecked())
    {
        sal_Bool bEnable = !pBtn->IsChecked();
        m_aRightMF.Enable(bEnable);
        m_pRightFT->Enable(bEnable);
    }
    bModified = sal_True;

    return 0;
}


IMPL_LINK( SwFormatTablePage, AutoClickHdl, CheckBox *, pBox )
{
    sal_Bool bRestore = sal_True,
         bLeftEnable = sal_False,
         bRightEnable= sal_False,
         bWidthEnable= sal_False,
         bOthers = sal_True;
    if ((RadioButton *)pBox == m_pFullBtn)
    {
        m_aLeftMF.SetPrcntValue(0);
        m_aRightMF.SetPrcntValue(0);
        nSaveWidth = static_cast< SwTwips >(m_aWidthMF.DenormalizePercent(m_aWidthMF.GetValue(FUNIT_TWIP )));
        m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(pTblData->GetSpace() ), FUNIT_TWIP );
        bFull = sal_True;
        bRestore = sal_False;
    }
    else if ((RadioButton *)pBox == m_pLeftBtn)
    {
        bRightEnable = bWidthEnable = sal_True;
        m_aLeftMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pFromLeftBtn)
    {
        bLeftEnable = bWidthEnable = sal_True;
        m_aRightMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pRightBtn)
    {
        bLeftEnable = bWidthEnable = sal_True;
        m_aRightMF.SetPrcntValue(0);
    }
    else if ((RadioButton *) pBox == m_pCenterBtn)
    {
        bLeftEnable = bWidthEnable = sal_True;
    }
    else if ((RadioButton *) pBox == m_pFreeBtn)
    {
        RightModify();
        bLeftEnable = sal_True;
        bWidthEnable = sal_True;
        bOthers = sal_False;
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
        // After being switched on automatic, the width was pinned
        // in order to restore the width while switching back to.
        bFull = sal_False;
        m_aWidthMF.SetPrcntValue(m_aWidthMF.NormalizePercent(nSaveWidth ), FUNIT_TWIP );
    }
    ModifyHdl(m_aWidthMF.get());
    bModified = sal_True;
    return 0;
}

void SwFormatTablePage::RightModify()
{
    if(m_pFreeBtn->IsChecked())
    {
        sal_Bool bEnable = m_aRightMF.GetValue() == 0;
        m_pRelWidthCB->Enable(bEnable);
        if ( !bEnable )
        {
            m_pRelWidthCB->Check(sal_False);
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
            sal_Bool bCenter = m_pCenterBtn->IsChecked();
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
    bModified = sal_True;
}


SfxTabPage*  SwFormatTablePage::Create( Window* pParent,
                                   const SfxItemSet& rAttrSet)
{
    return new SwFormatTablePage( pParent, rAttrSet );
}


sal_Bool  SwFormatTablePage::FillItemSet( SfxItemSet& rCoreSet )
{
    // Test if one of the controls still has the focus.
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
        if( m_pBottomMF->GetText() != m_pBottomMF->GetSavedValue() ||
            m_pTopMF->GetText() != m_pTopMF->GetSavedValue() )
        {
            SvxULSpaceItem aULSpace(RES_UL_SPACE);
            aULSpace.SetUpper( (sal_uInt16) m_pTopMF->Denormalize(
                                        m_pTopMF->GetValue( FUNIT_TWIP )));
            aULSpace.SetLower( (sal_uInt16) m_pBottomMF->Denormalize(
                                        m_pBottomMF->GetValue( FUNIT_TWIP )));
            rCoreSet.Put(aULSpace);
        }

    }
    if(m_pNameED->GetText() != m_pNameED->GetSavedValue())
    {
        rCoreSet.Put(SfxStringItem( FN_PARAM_TABLE_NAME, m_pNameED->GetText()));
        bModified = sal_True;
    }

    sal_uInt16 nPos;
    if( m_pTextDirectionLB->IsVisible() &&
        ( nPos = m_pTextDirectionLB->GetSelectEntryPos() ) !=
                                            m_pTextDirectionLB->GetSavedValue() )
    {
        sal_uInt32 nDirection = (sal_uInt32)(sal_uIntPtr)m_pTextDirectionLB->GetEntryData( nPos );
        rCoreSet.Put( SvxFrameDirectionItem( (SvxFrameDirection)nDirection, RES_FRAMEDIR));
        bModified = sal_True;
    }

    return bModified;
}


void  SwFormatTablePage::Reset( const SfxItemSet& )
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
        m_pFreeBtn->Enable(sal_False);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    m_aWidthMF.SetMetric(aMetric);
    m_aRightMF.SetMetric(aMetric);
    m_aLeftMF.SetMetric(aMetric);
    SetMetric(*m_pTopMF, aMetric);
    SetMetric(*m_pBottomMF, aMetric);

    //Name
    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, sal_False, &pItem ))
    {
        m_pNameED->SetText(((const SfxStringItem*)pItem)->GetValue());
        m_pNameED->SaveValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, sal_False, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nMinTableWidth = pTblData->GetColCount() * MINLAY;

        if(pTblData->GetWidthPercent())
        {
            m_pRelWidthCB->Check(sal_True);
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

        nOldAlign = pTblData->GetAlign();

        bool bSetRight = false, bSetLeft = false;
        switch( nOldAlign )
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
                m_aWidthMF.Enable(sal_False);
                m_pRelWidthCB->Enable(sal_False);
                m_pWidthFT->Enable(sal_False);
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
            m_aRightMF.Enable(sal_False);
            m_pRightFT->Enable(sal_False);
        }
        if ( bSetLeft )
        {
            m_aLeftMF.Enable(sal_False);
            m_pLeftFT->Enable(sal_False);
        }

    }

    //Margins
    if(SFX_ITEM_SET == rSet.GetItemState( RES_UL_SPACE, sal_False,&pItem ))
    {
        m_pTopMF->SetValue(m_pTopMF->Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetUpper()), FUNIT_TWIP);
        m_pBottomMF->SetValue(m_pBottomMF->Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetLower()), FUNIT_TWIP);
        m_pTopMF->SaveValue();
        m_pBottomMF->SaveValue();
    }

    //text direction
    if( SFX_ITEM_SET == rSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem ) )
    {
        sal_uIntPtr nVal  = ((SvxFrameDirectionItem*)pItem)->GetValue();
        sal_uInt16 nPos = m_pTextDirectionLB->GetEntryPos( (void*) nVal );
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
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP ))
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
    // os: VCL sorgt nicht dafuer, dass das aktive Control im
    // dialog bei OK den focus verliert
    m_pNameED->GrabFocus();
    // Test the table name for spaces
    String sTblName = m_pNameED->GetText();
    if(sTblName.Search(' ') != STRING_NOTFOUND)
    {
        InfoBox(this, SW_RES(MSG_WRONG_TABLENAME)).Execute();
        m_pNameED->GrabFocus();
        return KEEP_PAGE;
    }
    if(_pSet)
    {
        FillItemSet(*_pSet);
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
                sal_uInt16 nPercentWidth = (sal_uInt16)m_aWidthMF.GetValue(FUNIT_CUSTOM);
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
            sal_uInt16 i;

            for( i = 0; i < pTblData->GetColCount(); i++)
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
                    for( i = 0; i < pTblData->GetColCount(); i++)
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
SwTableColumnPage::SwTableColumnPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableColumnPage",
        "modules/swriter/ui/tablecolumnpage.ui", rSet)
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
    Init((SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, sal_False,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON));

};
//Description: Page column configuration
 SwTableColumnPage::~SwTableColumnPage()
{
};


SfxTabPage*   SwTableColumnPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTableColumnPage( pParent, rAttrSet );
};


void  SwTableColumnPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, sal_False, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nNoOfVisibleCols = pTblData->GetColCount();
        nNoOfCols = pTblData->GetAllColCount();
        nTableWidth = pTblData->GetAlign() != text::HoriOrientation::FULL &&
                            pTblData->GetAlign() != text::HoriOrientation::LEFT_AND_WIDTH?
                        pTblData->GetWidth() : pTblData->GetSpace();

        sal_uInt16 i;
        for( i = 0; i < nNoOfCols; i++ )
        {
            if( pTblData->GetColumns()[i].nWidth  < nMinWidth )
                    nMinWidth = pTblData->GetColumns()[i].nWidth;
        }
        sal_Int64 nMinTwips = m_aFieldArr[0].NormalizePercent( nMinWidth );
        sal_Int64 nMaxTwips = m_aFieldArr[0].NormalizePercent( nTableWidth );
        for( i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
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
        i = nNoOfVisibleCols;
        while( i < MET_FIELDS )
        {
            m_aFieldArr[i].SetText( aEmptyStr );
            m_pTextArr[i]->Disable();
            i++;
        }
    }
    ActivatePage(rSet);

};


void  SwTableColumnPage::Init(sal_Bool bWeb)
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
};


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
        String sEntry = OUString('~');
        String sIndex = OUString::number( aValueTbl[i] + 1 );
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
};


IMPL_LINK_INLINE_START( SwTableColumnPage, UpHdl, MetricField*, pEdit )
{
    bModified = sal_True;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, UpHdl, MetricField*, pEdit )


IMPL_LINK_INLINE_START( SwTableColumnPage, DownHdl, MetricField*, pEdit )
{
    bModified = sal_True;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, DownHdl, MetricField*, pEdit )


IMPL_LINK_INLINE_START( SwTableColumnPage, LoseFocusHdl, MetricField*, pEdit )
{
    if (pEdit->IsModified())
    {
        bModified = sal_True;
        ModifyHdl( pEdit );
    }
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, LoseFocusHdl, MetricField*, pEdit )


IMPL_LINK( SwTableColumnPage, ModeHdl, CheckBox*, pBox )
{
    sal_Bool bCheck = pBox->IsChecked();
    if (pBox == m_pProportionalCB)
    {
        if(bCheck)
            m_pModifyTableCB->Check();
        m_pModifyTableCB->Enable(!bCheck && bModifyTbl);
    }
    return 0;
};


sal_Bool  SwTableColumnPage::FillItemSet( SfxItemSet& )
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
};


void   SwTableColumnPage::ModifyHdl( MetricField* pField )
{
        PercentField *pEdit = NULL;
        sal_uInt16 nAktPos;
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
        nAktPos = aValueTbl[i];

        UpdateCols( nAktPos );
};


void   SwTableColumnPage::UpdateCols( sal_uInt16 nAktPos )
{
    SwTwips nSum = 0;
    sal_uInt16 i;

    for( i = 0; i < nNoOfCols; i++ )
    {
        nSum += (pTblData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - nTableWidth;

    sal_Bool bModifyTable = m_pModifyTableCB->IsChecked();
    sal_Bool bProp =    m_pProportionalCB->IsChecked();

    if(!bModifyTable && !bProp )
    {
//      the table width is constant, the difference is balanced with the other columns
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
//      Difference is balanced by the width of the table,
//      other columns remain unchanged.
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
    else if(bModifyTable & bProp)
    {
//      All columns will be changed proportionally with,
//      the table width is adjusted accordingly.
        OSL_ENSURE(nDiff * nNoOfVisibleCols <= pTblData->GetSpace() - nTableWidth, "wrong maximum" );
        long nAdd = nDiff;
        if(nDiff * nNoOfVisibleCols > pTblData->GetSpace() - nTableWidth)
        {
            nAdd = (pTblData->GetSpace() - nTableWidth) / nNoOfVisibleCols;
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nAdd );
            nDiff = nAdd;
        }
        if(nAdd)
            for(i = 0; i < nNoOfVisibleCols; i++ )
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

    for( i = 0; ( i < nNoOfVisibleCols ) && ( i < MET_FIELDS ); i++)
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

    sal_uInt16 nTblAlign = pTblData->GetAlign();
    if((text::HoriOrientation::FULL != nTblAlign && nTableWidth != pTblData->GetWidth()) ||
    (text::HoriOrientation::FULL == nTblAlign && nTableWidth != pTblData->GetSpace()))
    {
        nTableWidth = text::HoriOrientation::FULL == nTblAlign ?
                                    pTblData->GetSpace() :
                                        pTblData->GetWidth();
        UpdateCols(0);
    }
    bModifyTbl = sal_True;
    if(pTblData->GetWidthPercent() ||
                text::HoriOrientation::FULL == nTblAlign ||
                        pTblData->IsLineSelected()  )
        bModifyTbl = sal_False;
    if(bPercentMode)
    {
        m_pModifyTableCB->Check(sal_False);
        m_pProportionalCB->Check(sal_False);
    }
    else if( !bModifyTbl )
    {
        m_pProportionalCB->Check(sal_False);
        m_pModifyTableCB->Check(sal_False);
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
        FillItemSet(*_pSet);
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

SwTableTabDlg::SwTableTabDlg(Window* pParent, SfxItemPool&,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
    : SfxTabDialog(0, pParent, "TablePropertiesDialog",
        "modules/swriter/ui/tableproperties.ui", pItemSet, 0)
    , pShell(pSh)
    , m_nHtmlMode(::GetHtmlMode(pSh->GetView().GetDocShell()))
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
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
        const sal_uInt16 eType = pShell->GetFrmType(0,sal_True);
        if( !(FRMTYPE_BODY & eType) )
            ((SwTextFlowPage&)rPage).DisablePageBreak();
    }
}

SwTextFlowPage::SwTextFlowPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "TableTextFlowPage",
        "modules/swriter/ui/tabletextflowpage.ui", rSet)
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
    if(SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, sal_False,&pItem )
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

SfxTabPage*   SwTextFlowPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTextFlowPage(pParent, rAttrSet);
}

sal_Bool  SwTextFlowPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    // Repeat Heading
    if(m_pHeadLineCB->IsChecked() != m_pHeadLineCB->GetSavedValue() ||
        OUString::number( static_cast< sal_Int32 >(m_pRepeatHeaderNF->GetValue()) ) != m_pRepeatHeaderNF->GetSavedValue() )
    {
        bModified |= 0 != rSet.Put(
            SfxUInt16Item(FN_PARAM_TABLE_HEADLINE, m_pHeadLineCB->IsChecked()? sal_uInt16(m_pRepeatHeaderNF->GetValue()) : 0 ));
    }
    if(m_pKeepCB->IsChecked() != m_pKeepCB->GetSavedValue())
        bModified |= 0 != rSet.Put( SvxFmtKeepItem( m_pKeepCB->IsChecked(), RES_KEEP));

    if(m_pSplitCB->IsChecked() != m_pSplitCB->GetSavedValue())
        bModified |= 0 != rSet.Put( SwFmtLayoutSplit( m_pSplitCB->IsChecked()));

    if(m_pSplitRowCB->IsChecked() != m_pSplitRowCB->GetSavedValue())
        bModified |= 0 != rSet.Put( SwFmtRowSplit( m_pSplitRowCB->IsChecked()));


    const SvxFmtBreakItem* pBreak = (const SvxFmtBreakItem*)GetOldItem( rSet, RES_BREAK );
    const SwFmtPageDesc* pDesc = (const SwFmtPageDesc*) GetOldItem( rSet, RES_PAGEDESC );


    sal_Bool bState = m_pPageCollCB->IsChecked();

    // If we have a page style, then there's no break
    sal_Bool bPageItemPut = sal_False;
    if ( bState != m_pPageCollCB->GetSavedValue() ||
         ( bState &&
           m_pPageCollLB->GetSelectEntryPos() != m_pPageCollLB->GetSavedValue() )
           || (m_pPageNoNF->IsEnabled() && m_pPageNoNF->IsValueModified()) )
    {
        String sPage;

        if ( bState )
        {
            sPage = m_pPageCollLB->GetSelectEntry();
        }
        sal_uInt16 nPgNum = static_cast< sal_uInt16 >(m_pPageNoNF->GetValue());
        if ( !pDesc || !pDesc->GetPageDesc() ||
            ( pDesc->GetPageDesc() && ((pDesc->GetPageDesc()->GetName() != sPage) ||
                    !comphelper::string::equals(m_pPageNoNF->GetSavedValue(), nPgNum))))
        {
            SwFmtPageDesc aFmt( pShell->FindPageDescByName( sPage, sal_True ) );
            aFmt.SetNumOffset(bState ? nPgNum : 0);
            bModified |= 0 != rSet.Put( aFmt );
            bPageItemPut = bState;
        }
    }
    sal_Bool bIsChecked = m_pPgBrkCB->IsChecked();
    if ( !bPageItemPut &&
        (   bState != m_pPageCollCB->GetSavedValue() ||
            bIsChecked != m_pPgBrkCB->GetSavedValue()              ||
            m_pPgBrkBeforeRB->IsChecked() != m_pPgBrkBeforeRB->GetSavedValue()    ||
            m_pPgBrkRB->IsChecked() != m_pPgBrkRB->GetSavedValue() ))
    {
        SvxFmtBreakItem aBreak(
            (const SvxFmtBreakItem&)GetItemSet().Get( RES_BREAK ) );

        if(bIsChecked)
        {
            sal_Bool bBefore = m_pPgBrkBeforeRB->IsChecked();

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
            bModified |= 0 != rSet.Put( aBreak );
        }
    }

    if(m_pTextDirectionLB->GetSelectEntryPos() != m_pTextDirectionLB->GetSavedValue())
    {
          bModified |= 0 != rSet.Put(
                    SvxFrameDirectionItem(
                        (SvxFrameDirection)(sal_uLong)m_pTextDirectionLB->GetEntryData(m_pTextDirectionLB->GetSelectEntryPos())
                        , FN_TABLE_BOX_TEXTORIENTATION));
    }

    if(m_pVertOrientLB->GetSelectEntryPos() != m_pVertOrientLB->GetSavedValue())
    {
        sal_uInt16 nOrient = USHRT_MAX;
        switch(m_pVertOrientLB->GetSelectEntryPos())
        {
            case 0 : nOrient = text::VertOrientation::NONE; break;
            case 1 : nOrient = text::VertOrientation::CENTER; break;
            case 2 : nOrient = text::VertOrientation::BOTTOM; break;
        }
        if(nOrient != USHRT_MAX)
            bModified |= 0 != rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));
    }

    return bModified;

}

void   SwTextFlowPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    bool bFlowAllowed = !bHtmlMode || rHtmlOpt.IsPrintLayoutExtension();
    if(bFlowAllowed)
    {
        // Inserting of the existing page templates in the list box.
        const sal_uInt16 nCount = pShell->GetPageDescCnt();
        sal_uInt16 i;

        for( i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = pShell->GetPageDesc(i);
            m_pPageCollLB->InsertEntry(rPageDesc.GetName());
        }

        String aFmtName;
        for(i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
            if( LISTBOX_ENTRY_NOTFOUND == m_pPageCollLB->GetEntryPos(
                    aFmtName = SwStyleNameMapper::GetUIName( i, aFmtName ) ))
                m_pPageCollLB->InsertEntry( aFmtName );

        if(SFX_ITEM_SET == rSet.GetItemState( RES_KEEP, sal_False, &pItem ))
        {
            m_pKeepCB->Check( ((const SvxFmtKeepItem*)pItem)->GetValue() );
            m_pKeepCB->SaveValue();
        }
        if(SFX_ITEM_SET == rSet.GetItemState( RES_LAYOUT_SPLIT, sal_False, &pItem ))
        {
            m_pSplitCB->Check( ((const SwFmtLayoutSplit*)pItem)->GetValue() );
        }
        else
            m_pSplitCB->Check();

        m_pSplitCB->SaveValue();
        SplitHdl_Impl(m_pSplitCB);

        if(SFX_ITEM_SET == rSet.GetItemState( RES_ROW_SPLIT, sal_False, &pItem ))
        {
            m_pSplitRowCB->Check( ((const SwFmtRowSplit*)pItem)->GetValue() );
        }
        else
            m_pSplitRowCB->SetState(STATE_DONTKNOW);
        m_pSplitRowCB->SaveValue();

        if(bPageBreak)
        {
            if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_False, &pItem ))
            {
                String sPageDesc;
                const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                m_pPageNoNF->SetValue(((const SwFmtPageDesc*)pItem)->GetNumOffset());
                if(pDesc)
                    sPageDesc = pDesc->GetName();
                if ( sPageDesc.Len() &&
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

                    m_pPgBrkCB->Check( sal_True );
                    m_pColBrkRB->Check( sal_False );
                    m_pPgBrkBeforeRB->Check( sal_True );
                    m_pPgBrkAfterRB->Check( sal_False );
                }
                else
                {
                    m_pPageCollLB->SetNoSelection();
                    m_pPageCollCB->Check(sal_False);
                }
            }

            if(SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, sal_False, &pItem ))
            {
                const SvxFmtBreakItem* pPageBreak = (const SvxFmtBreakItem*)pItem;
                SvxBreak eBreak = (SvxBreak)pPageBreak->GetValue();

                if ( eBreak != SVX_BREAK_NONE )
                {
                    m_pPgBrkCB->Check();
                    m_pPageCollCB->Enable(sal_False);
                    m_pPageCollLB->Enable(sal_False);
                    m_pPageNoFT->Enable(sal_False);
                    m_pPageNoNF->Enable(sal_False);
                }
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        m_pPgBrkRB->Check( sal_True );
                        m_pColBrkRB->Check( sal_False );
                        m_pPgBrkBeforeRB->Check( sal_True );
                        m_pPgBrkAfterRB->Check( sal_False );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        m_pPgBrkRB->Check( sal_True );
                        m_pColBrkRB->Check( sal_False );
                        m_pPgBrkBeforeRB->Check( sal_False );
                        m_pPgBrkAfterRB->Check( sal_True );
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        m_pPgBrkRB->Check( sal_False );
                        m_pColBrkRB->Check( sal_True );
                        m_pPgBrkBeforeRB->Check( sal_True );
                        m_pPgBrkAfterRB->Check( sal_False );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        m_pPgBrkRB->Check( sal_False );
                        m_pColBrkRB->Check( sal_True );
                        m_pPgBrkBeforeRB->Check( sal_False );
                        m_pPgBrkAfterRB->Check( sal_True );
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
        m_pPgBrkRB->Enable(sal_False);
        m_pColBrkRB->Enable(sal_False);
        m_pPgBrkBeforeRB->Enable(sal_False);
        m_pPgBrkAfterRB->Enable(sal_False);
        m_pKeepCB->Enable(sal_False);
        m_pSplitCB->Enable(sal_False);
        m_pPgBrkCB->Enable(sal_False);
        m_pPageCollCB->Enable(sal_False);
        m_pPageCollLB->Enable(sal_False);
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem ))
    {
        sal_uInt16 nRep = ((const SfxUInt16Item*)pItem)->GetValue();
        m_pHeadLineCB->Check( nRep > 0 );
        m_pHeadLineCB->SaveValue();
        m_pRepeatHeaderNF->SetValue( nRep );
        m_pRepeatHeaderNF->SaveValue();
    }
    if ( rSet.GetItemState(FN_TABLE_BOX_TEXTORIENTATION) > SFX_ITEM_AVAILABLE )
    {
        sal_uLong nDirection = ((const SvxFrameDirectionItem&)rSet.Get(FN_TABLE_BOX_TEXTORIENTATION)).GetValue();
        m_pTextDirectionLB->SelectEntryPos(m_pTextDirectionLB->GetEntryPos( (const void*)nDirection ));
    }

    if ( rSet.GetItemState(FN_TABLE_SET_VERT_ALIGN) > SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nVert = ((const SfxUInt16Item&)rSet.Get(FN_TABLE_SET_VERT_ALIGN)).GetValue();
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
        m_pPageNoNF->Enable(sal_False);
        m_pPageNoFT->Enable(sal_False);
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

                sal_Bool bEnable = m_pPageCollCB->IsChecked() &&
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
            m_pPageCollCB->Check( sal_False );
            m_pPageCollCB->Enable(sal_False);
            m_pPageCollLB->Enable(sal_False);
            m_pPageNoFT->Enable(sal_False);
            m_pPageNoNF->Enable(sal_False);
            m_pPgBrkRB->       Enable(sal_False);
            m_pColBrkRB->      Enable(sal_False);
            m_pPgBrkBeforeRB-> Enable(sal_False);
            m_pPgBrkAfterRB->  Enable(sal_False);
    }
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, ApplyCollClickHdl_Impl)
{
    sal_Bool bEnable = sal_False;
    if ( m_pPageCollCB->IsChecked() &&
         m_pPageCollLB->GetEntryCount() )
    {
        bEnable = sal_True;
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

            sal_Bool bEnable = m_pPageCollCB->IsChecked()  &&
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
            m_pPageCollCB->Check( sal_False );
            m_pPageCollCB->Enable(sal_False);
            m_pPageCollLB->Enable(sal_False);
            m_pPageNoFT->Enable(sal_False);
            m_pPageNoNF->Enable(sal_False);
        }
    }
    return 0;
}

IMPL_LINK( SwTextFlowPage, PageBreakTypeHdl_Impl, RadioButton*, pBtn )
{
    if ( pBtn == m_pColBrkRB || m_pPgBrkAfterRB->IsChecked() )
    {
        m_pPageCollCB->Check(sal_False);
        m_pPageCollCB->Enable(sal_False);
        m_pPageCollLB->Enable(sal_False);
        m_pPageNoFT->Enable(sal_False);
        m_pPageNoNF->Enable(sal_False);
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
    pBox->EnableTriState(sal_False);
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, HeadLineCBClickHdl)
{
    m_pRepeatHeaderCombo->Enable(m_pHeadLineCB->IsChecked());

    return 0;
}

void SwTextFlowPage::DisablePageBreak()
{
    bPageBreak = sal_False;
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
