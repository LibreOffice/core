/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: zoom.cxx,v $
 * $Revision: 1.3 $
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
#if !ENABLE_LAYOUT
#include "zoom.hrc"
#endif /* !ENABLE_LAYOUT */

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
#else /* !TEST_LAYOUT */
#include <svx/zoomitem.hxx>
#include <svx/dialmgr.hxx>
#endif /* !TEST_LAYOUT */
#include "zoom_def.hxx"

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
    if ( a200Btn.IsChecked() )
        return 200;
    if ( a150Btn.IsChecked() )
        return 150;
    if ( a100Btn.IsChecked() )
        return 100;
    if ( a75Btn.IsChecked() )
        return 75;
    if ( a50Btn.IsChecked() )
        return 50;
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
        if ( nNewFactor == 200 )
        {
            a200Btn.Check();
            a200Btn.GrabFocus();
        }
        else if ( nNewFactor == 150 )
        {
            a150Btn.Check();
            a150Btn.GrabFocus();
        }
        else if ( nNewFactor == 100 )
        {
            a100Btn.Check();
            a100Btn.GrabFocus();
        }
        else if ( nNewFactor == 75 )
        {
            a75Btn.Check();
            a75Btn.GrabFocus();
        }
        else if ( nNewFactor == 50 )
        {
            a50Btn.Check();
            a50Btn.GrabFocus();
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

#if TEST_LAYOUT
SvxZoomDialog::SvxZoomDialog( Window* pParent ) :
#else /* !TEST_LAYOUT */
SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet ) :
#endif /* !TEST_LAYOUT */

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_ZOOM ) ),

    aZoomFl         ( this, SVX_RES( FL_ZOOM ) ),
    aWholePageBtn   ( this, SVX_RES( BTN_WHOLE_PAGE ) ),
    aPageWidthBtn   ( this, SVX_RES( BTN_PAGE_WIDTH ) ),
    aOptimalBtn     ( this, SVX_RES( BTN_OPTIMAL ) ),
    a200Btn         ( this, SVX_RES( BTN_200 ) ),
    a150Btn         ( this, SVX_RES( BTN_150 ) ),
    a100Btn         ( this, SVX_RES( BTN_100 ) ),
    a75Btn          ( this, SVX_RES( BTN_75 ) ),
    a50Btn          ( this, SVX_RES( BTN_50 ) ),
    aUserBtn        ( this, SVX_RES( BTN_USER ) ),
    aUserEdit       ( this, SVX_RES( ED_USER ) ),
    aOKBtn          ( this, SVX_RES( BTN_ZOOM_OK ) ),
    aCancelBtn      ( this, SVX_RES( BTN_ZOOM_CANCEL ) ),
    aHelpBtn        ( this, SVX_RES( BTN_ZOOM_HELP ) ),

#if !TEST_LAYOUT
    rSet        ( rCoreSet ),
#endif /* !TEST_LAYOUT */
    pOutSet     ( NULL ),
    bModified   ( FALSE )

{
    Link aLink = LINK( this, SvxZoomDialog, UserHdl );
    a200Btn.SetClickHdl( aLink );
    a150Btn.SetClickHdl( aLink );
    a100Btn.SetClickHdl( aLink );
    a75Btn.SetClickHdl( aLink );
    a50Btn.SetClickHdl( aLink );
    aOptimalBtn.SetClickHdl( aLink );
    aPageWidthBtn.SetClickHdl( aLink );
    aWholePageBtn.SetClickHdl( aLink );
    aUserBtn.SetClickHdl( aLink );
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
        USHORT nZoom = rZoomItem.GetValue();
        SvxZoomType eType = rZoomItem.GetType();
        USHORT nValSet = rZoomItem.GetValueSet();
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

        // ggf. Buttons disablen
        if ( !(SVX_ZOOM_ENABLE_50 & nValSet) )
            a50Btn.Disable();
        if ( !(SVX_ZOOM_ENABLE_75 & nValSet) )
            a75Btn.Disable();
        if ( !(SVX_ZOOM_ENABLE_100 & nValSet) )
            a100Btn.Disable();
        if ( !(SVX_ZOOM_ENABLE_150 & nValSet) )
            a150Btn.Disable();
        if ( !(SVX_ZOOM_ENABLE_200 & nValSet) )
            a200Btn.Disable();
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
        USHORT nZoom = ( (const SfxUInt16Item&)rItem ).GetValue();
        SetFactor( nZoom );
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

IMPL_LINK( SvxZoomDialog, OKHdl, Button *, pBtn )
{
    if ( bModified || &aOKBtn != pBtn )
    {
#if TEST_LAYOUT
        SvxZoomItem aItem;
#else /* !TEST_LAYOUT */
        SvxZoomItem aItem( SVX_ZOOM_PERCENT, 0,
                           rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );

#endif /* !TEST_LAYOUT */
        if ( &aOKBtn == pBtn )
        {
            USHORT nFactor = GetFactor();

            if ( SPECIAL_FACTOR == nFactor )
            {
                if ( aOptimalBtn.IsChecked() )
                    aItem.SetType( SVX_ZOOM_OPTIMAL );
                else if ( aPageWidthBtn.IsChecked() )
                    aItem.SetType( SVX_ZOOM_PAGEWIDTH );
                else if ( aWholePageBtn.IsChecked() )
                    aItem.SetType( SVX_ZOOM_WHOLEPAGE );
            }
            else
                aItem.SetValue( nFactor );
        }
        else
        {
            DBG_ERROR( "Wrong Button" );
            return 0;
        }
#if !TEST_LAYOUT
        pOutSet = new SfxItemSet( rSet );
        pOutSet->Put( aItem );

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


