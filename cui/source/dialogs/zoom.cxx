/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>

#define _SVX_ZOOM_CXX

#include <cuires.hrc>
#include "zoom.hrc"

#include "zoom.hxx"
#include <svx/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <dialmgr.hxx>
#include <svx/zoom_def.hxx>
#include <svx/dialogs.hrc>  // RID_SVXDLG_ZOOM
#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef CUI_RES
#define CUI_RES(x) #x
#undef SfxModalDialog
#define SfxModalDialog( parent, id ) Dialog( parent, "zoom.xml", id )
#endif /* ENABLE_LAYOUT */

// static ----------------------------------------------------------------

#define SPECIAL_FACTOR  ((sal_uInt16)0xFFFF)

// class SvxZoomDialog ---------------------------------------------------

sal_uInt16 SvxZoomDialog::GetFactor() const
{
    if ( a100Btn.IsChecked() )
        return 100;
    if ( aUserBtn.IsChecked() )
        return (sal_uInt16)aUserEdit.GetValue();
    else
        return SPECIAL_FACTOR;
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetFactor( sal_uInt16 nNewFactor, sal_uInt16 nBtnId )
{
    aUserEdit.Disable();

    if ( !nBtnId )
    {
        if ( nNewFactor == 100 )
        {
            a100Btn.Check();
            a100Btn.GrabFocus();
        }
        else
        {
            aUserBtn.Check();
            aUserEdit.Enable();
            aUserEdit.SetValue( (long)nNewFactor );
            aUserEdit.GrabFocus();
        }
    }
    else
    {
        aUserEdit.SetValue( (long)nNewFactor );

        if ( ZOOMBTN_OPTIMAL == nBtnId )
        {
            aOptimalBtn.Check();
            aOptimalBtn.GrabFocus();
        }
        else if ( ZOOMBTN_PAGEWIDTH == nBtnId )
        {
            aPageWidthBtn.Check();
            aPageWidthBtn.GrabFocus();
        }
        else if ( ZOOMBTN_WHOLEPAGE == nBtnId )
        {
            aWholePageBtn.Check();
            aWholePageBtn.GrabFocus();
        }
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::HideButton( sal_uInt16 nBtnId )
{
    switch ( nBtnId )
    {
        case ZOOMBTN_OPTIMAL: // Optimal-Button
            aOptimalBtn.Hide();
            break;

        case ZOOMBTN_PAGEWIDTH: // Seitenbreite-Button
            aPageWidthBtn.Hide();
            break;

        case ZOOMBTN_WHOLEPAGE: // Ganze Seite-Button
            aWholePageBtn.Hide();
            break;

        default:
            DBG_ERROR( "Falsche Button-Nummer!!!" );
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetLimits( sal_uInt16 nMin, sal_uInt16 nMax )
{
    DBG_ASSERT( nMin < nMax, "invalid limits" );
    aUserEdit.SetMin( nMin );
    aUserEdit.SetFirst( nMin );
    aUserEdit.SetMax( nMax );
    aUserEdit.SetLast( nMax );
}

// -----------------------------------------------------------------------

SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxModalDialog( pParent, CUI_RES( RID_SVXDLG_ZOOM ) ),

    aZoomFl         ( this, CUI_RES( FL_ZOOM ) ),
    aOptimalBtn     ( this, CUI_RES( BTN_OPTIMAL ) ),
    aWholePageBtn   ( this, CUI_RES( BTN_WHOLE_PAGE ) ),
    aPageWidthBtn   ( this, CUI_RES( BTN_PAGE_WIDTH ) ),
    a100Btn         ( this, CUI_RES( BTN_100 ) ),
    aUserBtn        ( this, CUI_RES( BTN_USER ) ),
    aUserEdit       ( this, CUI_RES( ED_USER ) ),

    aViewLayoutFl   ( this, CUI_RES( FL_VIEWLAYOUT ) ),
    aAutomaticBtn   ( this, CUI_RES( BTN_AUTOMATIC ) ),
    aSingleBtn      ( this, CUI_RES( BTN_SINGLE ) ),
    aColumnsBtn     ( this, CUI_RES( BTN_COLUMNS ) ),
    aColumnsEdit    ( this, CUI_RES( ED_COLUMNS ) ),
    aBookModeChk    ( this, CUI_RES( CHK_BOOK ) ),

    aBottomFl       ( this, CUI_RES( FL_BOTTOM ) ),
    aOKBtn          ( this, CUI_RES( BTN_ZOOM_OK ) ),
    aCancelBtn      ( this, CUI_RES( BTN_ZOOM_CANCEL ) ),
    aHelpBtn        ( this, CUI_RES( BTN_ZOOM_HELP ) ),

    rSet        ( rCoreSet ),
    pOutSet     ( NULL ),
    bModified   ( sal_False )

{
#if ENABLE_LAYOUT
    SetHelpId (".uno:Zoom");
#endif /* ENABLE_LAYOUT */
    Link aLink = LINK( this, SvxZoomDialog, UserHdl );
    a100Btn.SetClickHdl( aLink );
    aOptimalBtn.SetClickHdl( aLink );
    aPageWidthBtn.SetClickHdl( aLink );
    aWholePageBtn.SetClickHdl( aLink );
    aUserBtn.SetClickHdl( aLink );

    Link aViewLayoutLink = LINK( this, SvxZoomDialog, ViewLayoutUserHdl );
    aAutomaticBtn.SetClickHdl( aViewLayoutLink );
    aSingleBtn.SetClickHdl( aViewLayoutLink );
    aColumnsBtn.SetClickHdl( aViewLayoutLink );

    Link aViewLayoutSpinLink = LINK( this, SvxZoomDialog, ViewLayoutSpinHdl );
    aColumnsEdit.SetModifyHdl( aViewLayoutSpinLink );

    Link aViewLayoutCheckLink = LINK( this, SvxZoomDialog, ViewLayoutCheckHdl );
    aBookModeChk.SetClickHdl( aViewLayoutCheckLink );

    aOKBtn.SetClickHdl( LINK( this, SvxZoomDialog, OKHdl ) );
    aUserEdit.SetModifyHdl( LINK( this, SvxZoomDialog, SpinHdl ) );

    // Default-Werte
    sal_uInt16 nValue = 100;
    sal_uInt16 nMin = 10;
    sal_uInt16 nMax = 1000;

    // ggf. erst den alten Wert besorgen
    const SfxUInt16Item* pOldUserItem = 0;
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if ( pSh )
        pOldUserItem = (const SfxUInt16Item*)pSh->GetItem( SID_ATTR_ZOOM_USER );

    if ( pOldUserItem )
        nValue = pOldUserItem->GetValue();

    // UserEdit initialisieren
    if ( nMin > nValue )
        nMin = nValue;
    if ( nMax < nValue )
        nMax = nValue;
    aUserEdit.SetMin( nMin );
    aUserEdit.SetFirst( nMin );
    aUserEdit.SetMax( nMax );
    aUserEdit.SetLast( nMax );
    aUserEdit.SetValue( nValue );

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
            a100Btn.Disable();
        if ( !(SVX_ZOOM_ENABLE_OPTIMAL & nValSet) )
            aOptimalBtn.Disable();
        if ( !(SVX_ZOOM_ENABLE_PAGEWIDTH & nValSet) )
            aPageWidthBtn.Disable();
        if ( !(SVX_ZOOM_ENABLE_WHOLEPAGE & nValSet) )
            aWholePageBtn.Disable();
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
            aAutomaticBtn.Check();
            aColumnsEdit.SetValue( 2 );
            aColumnsEdit.Disable();
            aBookModeChk.Disable();
        }
        else if ( 1 == nColumns)
        {
            aSingleBtn.Check();
            aColumnsEdit.SetValue( 2 );
            aColumnsEdit.Disable();
            aBookModeChk.Disable();
        }
        else
        {
            aColumnsBtn.Check();
            if ( !bBookMode )
            {
                aColumnsEdit.SetValue( nColumns );
                if ( 0 != nColumns % 2 )
                    aBookModeChk.Disable();
            }
            else
            {
                aColumnsEdit.SetValue( nColumns );
                aBookModeChk.Check();
            }
        }
    }
    else
    {
        // hide view layout related controls:
        aViewLayoutFl.Disable();
        aAutomaticBtn.Disable();
        aSingleBtn.Disable();
        aColumnsBtn.Disable();
        aColumnsEdit.Disable();
        aBookModeChk.Disable();
    }

    FreeResource();
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

    if ( pBtn == &aUserBtn )
    {
        aUserEdit.Enable();
        aUserEdit.GrabFocus();
    }
    else
        aUserEdit.Disable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, SpinHdl, MetricField *, EMPTYARG )
{
    if ( !aUserBtn.IsChecked() )
        return 0;
    bModified |= sal_True;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutUserHdl, RadioButton *, pBtn )
{
    bModified |= sal_True;

    if ( pBtn == &aAutomaticBtn )
    {
        aColumnsEdit.Disable();
        aBookModeChk.Disable();
    }
    else if ( pBtn == &aSingleBtn )
    {
        aColumnsEdit.Disable();
        aBookModeChk.Disable();
    }
    else if ( pBtn == &aColumnsBtn )
    {
        aColumnsEdit.Enable();
        aColumnsEdit.GrabFocus();
        if ( 0 == aColumnsEdit.GetValue() % 2 )
            aBookModeChk.Enable();
    }
    else
    {
        DBG_ERROR( "Wrong Button" );
        return 0;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutSpinHdl, MetricField *, pEdt )
{
    if ( pEdt == &aColumnsEdit && !aColumnsBtn.IsChecked() )
        return 0;

    if ( 0 == aColumnsEdit.GetValue() % 2 )
        aBookModeChk.Enable();
    else
    {
        aBookModeChk.Check( sal_False );
        aBookModeChk.Disable();
    }

    bModified |= sal_True;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutCheckHdl, CheckBox *, pChk )
{
    if ( pChk == &aBookModeChk && !aColumnsBtn.IsChecked() )
        return 0;

    bModified |= sal_True;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, OKHdl, Button *, pBtn )
{
    if ( bModified || &aOKBtn != pBtn )
    {
        SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, 0, rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );
        SvxViewLayoutItem aViewLayoutItem( 0, false, rSet.GetPool()->GetWhich( SID_ATTR_VIEWLAYOUT ) );

        if ( &aOKBtn == pBtn )
        {
            sal_uInt16 nFactor = GetFactor();

            if ( SPECIAL_FACTOR == nFactor )
            {
                if ( aOptimalBtn.IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_OPTIMAL );
                else if ( aPageWidthBtn.IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_PAGEWIDTH );
                else if ( aWholePageBtn.IsChecked() )
                    aZoomItem.SetType( SVX_ZOOM_WHOLEPAGE );
            }
            else
                aZoomItem.SetValue( nFactor );

            if ( aAutomaticBtn.IsChecked() )
            {
                aViewLayoutItem.SetValue( 0 );
                aViewLayoutItem.SetBookMode( false );
            }
            if ( aSingleBtn.IsChecked() )
            {
                aViewLayoutItem.SetValue( 1 );
                aViewLayoutItem.SetBookMode( false );
            }
            else if ( aColumnsBtn.IsChecked() )
            {
                aViewLayoutItem.SetValue( static_cast<sal_uInt16>(aColumnsEdit.GetValue()) );
                aViewLayoutItem.SetBookMode( aBookModeChk.IsChecked() );
            }
        }
        else
        {
            DBG_ERROR( "Wrong Button" );
            return 0;
        }
        pOutSet = new SfxItemSet( rSet );
        pOutSet->Put( aZoomItem );

        // don't set attribute in case the whole viewlayout stuff is disabled:
        if ( aViewLayoutFl.IsEnabled() )
            pOutSet->Put( aViewLayoutItem );

        // Wert aus dem UserEdit "uber den Dialog hinaus merken
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
            pSh->PutItem( SfxUInt16Item( SID_ATTR_ZOOM_USER,
                                         (sal_uInt16)aUserEdit.GetValue() ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}


