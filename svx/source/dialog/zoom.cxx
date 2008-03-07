/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoom.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:16:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. RET_OK, RET_CANCEL
#include <vcl/msgbox.hxx>
#endif

#define _SVX_ZOOM_CXX

#include <svx/dialogs.hrc>
#if !ENABLE_LAYOUT
#include "zoom.hrc"
#endif /* !ENABLE_LAYOUT */

#include "zoom.hxx"
#include <svx/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/dialmgr.hxx>

#ifndef _SVX_ZOOM_DEF_HXX
#include "zoom_def.hxx"
#endif

#include <layout/layout-pre.hxx>

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
            DBG_ERROR( "wrong button number" );
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
            DBG_ERROR( "Falsche Button-Nummer!!!" );
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

SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet ) :

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

    rSet        ( rCoreSet ),
    pOutSet     ( NULL ),
    bModified   ( FALSE )

{
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
        SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, 0, rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );
        SvxViewLayoutItem aViewLayoutItem( 0, false, rSet.GetPool()->GetWhich( SID_ATTR_VIEWLAYOUT ) );

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
                                         (UINT16)aUserEdit.GetValue() ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}


