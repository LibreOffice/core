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

#include <tools/shl.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>

#define _SVX_ZOOM_CXX

#include <cuires.hrc>
#include "zoom.hrc"

#include "zoom.hxx"
#include <sfx2/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <dialmgr.hxx>
#include <svx/zoom_def.hxx>

// static ----------------------------------------------------------------

#define SPECIAL_FACTOR  ((sal_uInt16)0xFFFF)

// class SvxZoomDialog ---------------------------------------------------

sal_uInt16 SvxZoomDialog::GetFactor() const
{
    if ( m_p100Btn->IsChecked() )
        return 100;
    if ( m_pUserBtn->IsChecked() )
        return (sal_uInt16)m_pUserEdit->GetValue();
    else
        return SPECIAL_FACTOR;
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetFactor( sal_uInt16 nNewFactor, sal_uInt16 nBtnId )
{
    m_pUserEdit->Disable();

    if ( !nBtnId )
    {
        if ( nNewFactor == 100 )
        {
            m_p100Btn->Check();
            m_p100Btn->GrabFocus();
        }
        else
        {
            m_pUserBtn->Check();
            m_pUserEdit->Enable();
            m_pUserEdit->SetValue( (long)nNewFactor );
            m_pUserEdit->GrabFocus();
        }
    }
    else
    {
        m_pUserEdit->SetValue( (long)nNewFactor );

        if ( ZOOMBTN_OPTIMAL == nBtnId )
        {
            m_pOptimalBtn->Check();
            m_pOptimalBtn->GrabFocus();
        }
        else if ( ZOOMBTN_PAGEWIDTH == nBtnId )
        {
            m_pPageWidthBtn->Check();
            m_pPageWidthBtn->GrabFocus();
        }
        else if ( ZOOMBTN_WHOLEPAGE == nBtnId )
        {
            m_pWholePageBtn->Check();
            m_pWholePageBtn->GrabFocus();
        }
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::HideButton( sal_uInt16 nBtnId )
{
    switch ( nBtnId )
    {
        case ZOOMBTN_OPTIMAL:
            m_pOptimalBtn->Hide();
            break;

        case ZOOMBTN_PAGEWIDTH:
            m_pPageWidthBtn->Hide();
            break;

        case ZOOMBTN_WHOLEPAGE:
            m_pWholePageBtn->Hide();
            break;

        default:
            OSL_FAIL( "Falsche Button-Nummer!!!" );
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetLimits( sal_uInt16 nMin, sal_uInt16 nMax )
{
    DBG_ASSERT( nMin < nMax, "invalid limits" );
    m_pUserEdit->SetMin( nMin );
    m_pUserEdit->SetFirst( nMin );
    m_pUserEdit->SetMax( nMax );
    m_pUserEdit->SetLast( nMax );
}

// -----------------------------------------------------------------------

SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet )
    : SfxModalDialog(pParent, "ZoomDialog", "cui/ui/zoomdialog.ui")
    , rSet(rCoreSet)
    , pOutSet(NULL)
    , bModified(false)

{
    get(m_pOptimalBtn, "optimal");
    get(m_pWholePageBtn, "fitwandh");
    get(m_pPageWidthBtn, "fitw");
    get(m_p100Btn, "100pc");
    get(m_pUserBtn, "variable");
    get(m_pUserEdit, "zoomsb");
    get(m_pViewFrame, "viewframe");
    get(m_pAutomaticBtn, "automatic");
    get(m_pSingleBtn, "singlepage");
    get(m_pColumnsBtn, "columns");
    get(m_pColumnsEdit, "columnssb");
    get(m_pBookModeChk, "bookmode");
    get(m_pOKBtn, "ok");
    Link aLink = LINK( this, SvxZoomDialog, UserHdl );
    m_p100Btn->SetClickHdl( aLink );
    m_pOptimalBtn->SetClickHdl( aLink );
    m_pPageWidthBtn->SetClickHdl( aLink );
    m_pWholePageBtn->SetClickHdl( aLink );
    m_pUserBtn->SetClickHdl( aLink );

    Link aViewLayoutLink = LINK( this, SvxZoomDialog, ViewLayoutUserHdl );
    m_pAutomaticBtn->SetClickHdl( aViewLayoutLink );
    m_pSingleBtn->SetClickHdl( aViewLayoutLink );
    m_pColumnsBtn->SetClickHdl( aViewLayoutLink );

    Link aViewLayoutSpinLink = LINK( this, SvxZoomDialog, ViewLayoutSpinHdl );
    m_pColumnsEdit->SetModifyHdl( aViewLayoutSpinLink );

    Link aViewLayoutCheckLink = LINK( this, SvxZoomDialog, ViewLayoutCheckHdl );
    m_pBookModeChk->SetClickHdl( aViewLayoutCheckLink );

    m_pOKBtn->SetClickHdl( LINK( this, SvxZoomDialog, OKHdl ) );
    m_pUserEdit->SetModifyHdl( LINK( this, SvxZoomDialog, SpinHdl ) );

    // default values
    sal_uInt16 nValue = 100;
    sal_uInt16 nMin = 10;
    sal_uInt16 nMax = 1000;

    // maybe get the old value first
    const SfxUInt16Item* pOldUserItem = 0;
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if ( pSh )
        pOldUserItem = (const SfxUInt16Item*)pSh->GetItem( SID_ATTR_ZOOM_USER );

    if ( pOldUserItem )
        nValue = pOldUserItem->GetValue();

    // initialize UserEdit
    if ( nMin > nValue )
        nMin = nValue;
    if ( nMax < nValue )
        nMax = nValue;
    m_pUserEdit->SetMin( nMin );
    m_pUserEdit->SetFirst( nMin );
    m_pUserEdit->SetMax( nMax );
    m_pUserEdit->SetLast( nMax );
    m_pUserEdit->SetValue( nValue );

    m_pUserEdit->SetAccessibleRelationLabeledBy(m_pUserBtn);
    m_pUserEdit->SetAccessibleName(m_pUserBtn->GetText());
    m_pColumnsEdit->SetAccessibleRelationLabeledBy(m_pColumnsBtn);
    m_pColumnsEdit->SetAccessibleName(m_pColumnsBtn->GetText());
    m_pColumnsEdit->SetAccessibleRelationMemberOf(m_pColumnsBtn);
    m_pBookModeChk->SetAccessibleRelationLabeledBy(m_pColumnsBtn);
    m_pBookModeChk->SetAccessibleRelationMemberOf(m_pColumnsBtn);

    const SfxPoolItem& rItem = rSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );

    if ( rItem.ISA(SvxZoomItem) )
    {
        const SvxZoomItem& rZoomItem = (const SvxZoomItem&)rItem;
        const sal_uInt16 nZoom = rZoomItem.GetValue();
        const SvxZoomType eType = rZoomItem.GetType();
        const sal_uInt16 nValSet = rZoomItem.GetValueSet();
        sal_uInt16 nBtnId = 0;

        switch ( eType )
        {
            case SVX_ZOOM_OPTIMAL:
                nBtnId = ZOOMBTN_OPTIMAL;
                break;
            case SVX_ZOOM_PAGEWIDTH:
                nBtnId = ZOOMBTN_PAGEWIDTH;
                break;
            case SVX_ZOOM_WHOLEPAGE:
                nBtnId = ZOOMBTN_WHOLEPAGE;
                break;
            case SVX_ZOOM_PERCENT:
                break;
            case SVX_ZOOM_PAGEWIDTH_NOBORDER:
                break;
        }

        if ( !(SVX_ZOOM_ENABLE_100 & nValSet) )
            m_p100Btn->Disable();
        if ( !(SVX_ZOOM_ENABLE_OPTIMAL & nValSet) )
            m_pOptimalBtn->Disable();
        if ( !(SVX_ZOOM_ENABLE_PAGEWIDTH & nValSet) )
            m_pPageWidthBtn->Disable();
        if ( !(SVX_ZOOM_ENABLE_WHOLEPAGE & nValSet) )
            m_pWholePageBtn->Disable();
        SetFactor( nZoom, nBtnId );
    }
    else
    {
        const sal_uInt16 nZoom = ( (const SfxUInt16Item&)rItem ).GetValue();
        SetFactor( nZoom );
    }

    const SfxPoolItem* pViewLayoutItem = 0;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_VIEWLAYOUT, sal_False, &pViewLayoutItem ) )
    {
        const sal_uInt16 nColumns = static_cast<const SvxViewLayoutItem*>(pViewLayoutItem)->GetValue();
        const bool bBookMode  = static_cast<const SvxViewLayoutItem*>(pViewLayoutItem)->IsBookMode();

        if ( 0 == nColumns )
        {
            m_pAutomaticBtn->Check();
            m_pColumnsEdit->SetValue( 2 );
            m_pColumnsEdit->Disable();
            m_pBookModeChk->Disable();
        }
        else if ( 1 == nColumns)
        {
            m_pSingleBtn->Check();
            m_pColumnsEdit->SetValue( 2 );
            m_pColumnsEdit->Disable();
            m_pBookModeChk->Disable();
        }
        else
        {
            m_pColumnsBtn->Check();
            if ( !bBookMode )
            {
                m_pColumnsEdit->SetValue( nColumns );
                if ( 0 != nColumns % 2 )
                    m_pBookModeChk->Disable();
            }
            else
            {
                m_pColumnsEdit->SetValue( nColumns );
                m_pBookModeChk->Check();
            }
        }
    }
    else
    {
        // hide view layout related controls:
        m_pViewFrame->Disable();
    }
}

// -----------------------------------------------------------------------

SvxZoomDialog::~SvxZoomDialog()
{
    delete pOutSet;
    pOutSet = 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, UserHdl, RadioButton *, pBtn )
{
    bModified |= sal_True;

    if (pBtn == m_pUserBtn)
    {
        m_pUserEdit->Enable();
        m_pUserEdit->GrabFocus();
    }
    else
        m_pUserEdit->Disable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxZoomDialog, SpinHdl)
{
    if ( !m_pUserBtn->IsChecked() )
        return 0;
    bModified |= sal_True;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutUserHdl, RadioButton *, pBtn )
{
    bModified |= sal_True;

    if (pBtn == m_pAutomaticBtn)
    {
        m_pColumnsEdit->Disable();
        m_pBookModeChk->Disable();
    }
    else if (pBtn == m_pSingleBtn)
    {
        m_pColumnsEdit->Disable();
        m_pBookModeChk->Disable();
    }
    else if (pBtn == m_pColumnsBtn)
    {
        m_pColumnsEdit->Enable();
        m_pColumnsEdit->GrabFocus();
        if ( 0 == m_pColumnsEdit->GetValue() % 2 )
            m_pBookModeChk->Enable();
    }
    else
    {
        OSL_FAIL( "Wrong Button" );
        return 0;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutSpinHdl, MetricField *, pEdt )
{
    if ( pEdt == m_pColumnsEdit && !m_pColumnsBtn->IsChecked() )
        return 0;

    if ( 0 == m_pColumnsEdit->GetValue() % 2 )
        m_pBookModeChk->Enable();
    else
    {
        m_pBookModeChk->Check( sal_False );
        m_pBookModeChk->Disable();
    }

    bModified |= sal_True;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutCheckHdl, CheckBox *, pChk )
{
    if (pChk == m_pBookModeChk && !m_pColumnsBtn->IsChecked())
        return 0;

    bModified |= sal_True;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, OKHdl, Button *, pBtn )
{
    if ( bModified || m_pOKBtn != pBtn )
    {
        SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, 0, rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );
        SvxViewLayoutItem aViewLayoutItem( 0, false, rSet.GetPool()->GetWhich( SID_ATTR_VIEWLAYOUT ) );

        if ( m_pOKBtn == pBtn )
        {
            sal_uInt16 nFactor = GetFactor();

            if ( SPECIAL_FACTOR == nFactor )
            {
                if ( m_pOptimalBtn->IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_OPTIMAL );
                else if ( m_pPageWidthBtn->IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_PAGEWIDTH );
                else if ( m_pWholePageBtn->IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_WHOLEPAGE );
            }
            else
                aZoomItem.SetValue( nFactor );

            if ( m_pAutomaticBtn->IsChecked() )
            {
                aViewLayoutItem.SetValue( 0 );
                aViewLayoutItem.SetBookMode( false );
            }
            if ( m_pSingleBtn->IsChecked() )
            {
                aViewLayoutItem.SetValue( 1 );
                aViewLayoutItem.SetBookMode( false );
            }
            else if ( m_pColumnsBtn->IsChecked() )
            {
                aViewLayoutItem.SetValue( static_cast<sal_uInt16>(m_pColumnsEdit->GetValue()) );
                aViewLayoutItem.SetBookMode( m_pBookModeChk->IsChecked() );
            }
        }
        else
        {
            OSL_FAIL( "Wrong Button" );
            return 0;
        }
        pOutSet = new SfxItemSet( rSet );
        pOutSet->Put( aZoomItem );

        // don't set attribute in case the whole viewlayout stuff is disabled:
        if (m_pViewFrame->IsEnabled())
            pOutSet->Put(aViewLayoutItem);

        // memorize value from the UserEdit beyond the dialog
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
            pSh->PutItem( SfxUInt16Item( SID_ATTR_ZOOM_USER,
                                         (sal_uInt16)m_pUserEdit->GetValue() ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
