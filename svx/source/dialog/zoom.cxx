/*************************************************************************
 *
 *  $RCSfile: zoom.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#pragma hdrstop

#define _SVX_ZOOM_CXX

#include "dialogs.hrc"
#include "zoom.hrc"

#include "zoom.hxx"
#include "zoomitem.hxx"
#include "dialmgr.hxx"

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

SvxZoomDialog::SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_ZOOM ) ),

    a200Btn         ( this, ResId( BTN_200 ) ),
    a150Btn         ( this, ResId( BTN_150 ) ),
    a100Btn         ( this, ResId( BTN_100 ) ),
    a75Btn          ( this, ResId( BTN_75 ) ),
    a50Btn          ( this, ResId( BTN_50 ) ),
    aOptimalBtn     ( this, ResId( BTN_OPTIMAL ) ),
    aPageWidthBtn   ( this, ResId( BTN_PAGE_WIDTH ) ),
    aWholePageBtn   ( this, ResId( BTN_WHOLE_PAGE ) ),
    aUserBtn        ( this, ResId( BTN_USER ) ),
    aUserEdit       ( this, ResId( ED_USER ) ),
    aZoomBox        ( this, ResId( GB_ZOOM ) ),
    aOKBtn          ( this, ResId( BTN_ZOOM_OK ) ),
    aCancelBtn      ( this, ResId( BTN_ZOOM_CANCEL ) ),
    aHelpBtn        ( this, ResId( BTN_ZOOM_HELP ) ),

    rSet        ( rCoreSet ),
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
        USHORT nValue = rZoomItem.GetValue();
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
        SetFactor( nValue, nBtnId );
    }
    else
    {
        USHORT nValue = ( (const SfxUInt16Item&)rItem ).GetValue();
        SetFactor( nValue );
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
    USHORT nFactor = GetFactor();

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
        SvxZoomItem aItem( SVX_ZOOM_PERCENT, 0,
                           rSet.GetPool()->GetWhich( SID_ATTR_ZOOM ) );

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
        pOutSet = new SfxItemSet( rSet );
        pOutSet->Put( aItem );

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


