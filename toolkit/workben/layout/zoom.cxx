/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#if !TEST_LAYOUT
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#endif /* !TEST_LAYOUT */

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#if TEST_LAYOUT
#include <cstdio>
#endif /* TEST_LAYOUT */
#include <tools/shl.hxx>
#include <svtools/itemset.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>

#define _SVX_ZOOM_CXX

#include <svx/dialogs.hrc>
#if !TEST_LAYOUT
#include "zoom.hrc"
#endif /* !TEST_LAYOUT */

#include "zoom.hxx"
#if TEST_LAYOUT
#define SVX_ZOOM_OPTIMAL "Zoom optimal"
#define SVX_ZOOM_PAGEWIDTH "Zoom page width"
#define SVX_ZOOM_WHOLEPAGE "Zoom whole page"
struct SvxZoomItem
{
    void SetType( char const* s )
    {
        fputs( s, stderr );
    }

    void SetValue( int n )
    {
        fprintf( stderr, "Zoom factor: %d\n", int( n ) );
    }
};
#define SfxPoolItem LAYOUT_SfxPoolItem
class LAYOUT_SfxPoolItem
{
};
struct SvxViewLayoutItem : public LAYOUT_SfxPoolItem
{
    bool IsBookMode()
    {
        return false;
    }
    USHORT GetValue()
    {
        return 0;
    }

    void SetValue( int n )
    {
        fprintf( stderr, "Zoom factor: %d\n", int( n ) );
    }
    void SetBookMode( bool )
    {
    }
};
#else /* !TEST_LAYOUT */
#include <svx/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/dialmgr.hxx>
#endif /* !TEST_LAYOUT */
#include "zoom_def.hxx"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef SVX_RES
#define SVX_RES(x) #x
#undef SfxModalDialog
#define SfxModalDialog( parent, id ) Dialog( parent, "zoom.xml", id )
#endif /* ENABLE_LAYOUT */

// static ----------------------------------------------------------------

static USHORT pRanges[] =
{
    SID_ATTR_ZOOM,
    SID_ATTR_ZOOM,
    0
};

#define SPECIAL_FACTOR  ((USHORT)0xFFFF)

// class SvxZoomDialog ---------------------------------------------------

USHORT SvxZoomDialog::GetFactor() const
{
    if ( a100Btn.IsChecked() )
        return 100;
    if ( aUserBtn.IsChecked() )
        return (USHORT)aUserEdit.GetValue();
    else
        return SPECIAL_FACTOR;
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetFactor( USHORT nNewFactor, USHORT nBtnId )
{
#if TEST_LAYOUT
    printf ("\t\t set factor %d\n", (int)nNewFactor);
#endif /* !TEST_LAYOUT */
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

void SvxZoomDialog::SetButtonText( USHORT nBtnId, const String& rNewTxt )
{
    switch ( nBtnId )
    {
        case ZOOMBTN_OPTIMAL: // Optimal-Button
            aOptimalBtn.SetText( rNewTxt );
            break;

        case ZOOMBTN_PAGEWIDTH: // Seitenbreite-Button
            aPageWidthBtn.SetText( rNewTxt );
            break;

        case ZOOMBTN_WHOLEPAGE: // Ganze Seite-Button
            aWholePageBtn.SetText( rNewTxt );
            break;

        default:
            OSL_FAIL( "wrong button number" );
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::HideButton( USHORT nBtnId )
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
            OSL_FAIL( "Falsche Button-Nummer!!!" );
    }
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetLimits( USHORT nMin, USHORT nMax )
{
    DBG_ASSERT( nMin < nMax, "invalid limits" );
    aUserEdit.SetMin( nMin );
    aUserEdit.SetFirst( nMin );
    aUserEdit.SetMax( nMax );
    aUserEdit.SetLast( nMax );
}

// -----------------------------------------------------------------------

void SvxZoomDialog::SetSpinSize( USHORT nNewSpin )
{
    aUserEdit.SetSpinSize( nNewSpin );
}

// -----------------------------------------------------------------------

#if TEST_LAYOUT
SvxZoomDialog::SvxZoomDialog( Window* pParent ) :
#else /* !TEST_LAYOUT */
SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet ) :
#endif /* !TEST_LAYOUT */

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_ZOOM ) ),

    aZoomFl         ( this, SVX_RES( FL_ZOOM ) ),
    aOptimalBtn     ( this, SVX_RES( BTN_OPTIMAL ) ),
    aWholePageBtn   ( this, SVX_RES( BTN_WHOLE_PAGE ) ),
    aPageWidthBtn   ( this, SVX_RES( BTN_PAGE_WIDTH ) ),
    a100Btn         ( this, SVX_RES( BTN_100 ) ),
    aUserBtn        ( this, SVX_RES( BTN_USER ) ),
    aUserEdit       ( this, SVX_RES( ED_USER ) ),

    aViewLayoutFl   ( this, SVX_RES( FL_VIEWLAYOUT ) ),
    aAutomaticBtn   ( this, SVX_RES( BTN_AUTOMATIC ) ),
    aSingleBtn      ( this, SVX_RES( BTN_SINGLE ) ),
    aColumnsBtn     ( this, SVX_RES( BTN_COLUMNS ) ),
    aColumnsEdit    ( this, SVX_RES( ED_COLUMNS ) ),
    aBookModeChk    ( this, SVX_RES( CHK_BOOK ) ),

    aBottomFl       ( this, SVX_RES( FL_BOTTOM ) ),
    aOKBtn          ( this, SVX_RES( BTN_ZOOM_OK ) ),
    aCancelBtn      ( this, SVX_RES( BTN_ZOOM_CANCEL ) ),
    aHelpBtn        ( this, SVX_RES( BTN_ZOOM_HELP ) ),

#if !TEST_LAYOUT
    rSet        ( rCoreSet ),
#endif /* !TEST_LAYOUT */
    pOutSet     ( NULL ),
    bModified   ( FALSE )

{
#if ENABLE_LAYOUT
    SetHelpId (SID_ATTR_ZOOM);
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
    USHORT nValue = 100;
    USHORT nMin = 10;
    USHORT nMax = 1000;

#if !TEST_LAYOUT
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
#endif /* !TEST_LAYOUT */
    aUserEdit.SetMin( nMin );
    aUserEdit.SetFirst( nMin );
    aUserEdit.SetMax( nMax );
    aUserEdit.SetLast( nMax );
    aUserEdit.SetValue( nValue );

#if TEST_LAYOUT
    USHORT nZoom = 100;
    SetFactor( nZoom );
#else /* !TEST_LAYOUT */
    const SfxPoolItem& rItem = rSet.Get( rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );

    if ( rItem.ISA(SvxZoomItem) )
    {
        const SvxZoomItem& rZoomItem = (const SvxZoomItem&)rItem;
        const USHORT nZoom = rZoomItem.GetValue();
        const SvxZoomType eType = rZoomItem.GetType();
        const USHORT nValSet = rZoomItem.GetValueSet();
        USHORT nBtnId = 0;

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
        const USHORT nZoom = ( (const SfxUInt16Item&)rItem ).GetValue();
        SetFactor( nZoom );
    }

    const SfxPoolItem* pViewLayoutItem = 0;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_VIEWLAYOUT, FALSE, &pViewLayoutItem ) )
    {
        const USHORT nColumns = static_cast<const SvxViewLayoutItem*>(pViewLayoutItem)->GetValue();
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
#endif /* !TEST_LAYOUT */

    FreeResource();
}

// -----------------------------------------------------------------------

SvxZoomDialog::~SvxZoomDialog()
{
    delete pOutSet;
    pOutSet = 0;
}

// -----------------------------------------------------------------------

USHORT* SvxZoomDialog::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, UserHdl, RadioButton *, pBtn )
{
#if TEST_LAYOUT
    fprintf( stderr, "SvxZoomDialog::UserHdl\n" );
#endif /* TEST_LAYOUT */
    bModified |= TRUE;

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
    bModified |= TRUE;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutUserHdl, RadioButton *, pBtn )
{
    bModified |= TRUE;

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
        OSL_FAIL( "Wrong Button" );
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

    bModified |= TRUE;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, ViewLayoutCheckHdl, CheckBox *, pChk )
{
    if ( pChk == &aBookModeChk && !aColumnsBtn.IsChecked() )
        return 0;

    bModified |= TRUE;

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxZoomDialog, OKHdl, Button *, pBtn )
{
    if ( bModified || &aOKBtn != pBtn )
    {
#if TEST_LAYOUT
        SvxZoomItem aZoomItem;
        SvxViewLayoutItem aViewLayoutItem;
#else /* !TEST_LAYOUT */
        SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, 0, rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );
        SvxViewLayoutItem aViewLayoutItem( 0, false, rSet.GetPool()->GetWhich( SID_ATTR_VIEWLAYOUT ) );
#endif /* !TEST_LAYOUT */

        if ( &aOKBtn == pBtn )
        {
            USHORT nFactor = GetFactor();

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
                aViewLayoutItem.SetValue( static_cast<USHORT>(aColumnsEdit.GetValue()) );
                aViewLayoutItem.SetBookMode( aBookModeChk.IsChecked() );
            }
        }
        else
        {
            OSL_FAIL( "Wrong Button" );
            return 0;
        }
#if !TEST_LAYOUT
        pOutSet = new SfxItemSet( rSet );
        pOutSet->Put( aZoomItem );

        // don't set attribute in case the whole viewlayout stuff is disabled:
        if ( aViewLayoutFl.IsEnabled() )
            pOutSet->Put( aViewLayoutItem );

        // Wert aus dem UserEdit "uber den Dialog hinaus merken
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
            pSh->PutItem( SfxUInt16Item( SID_ATTR_ZOOM_USER,
                                         (UINT16)aUserEdit.GetValue() ) );
#endif /* !TEST_LAYOUT */
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
