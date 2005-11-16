/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optctl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 10:02:55 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _SVX_OPTCTL_HXX
#include "optctl.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_OPTCTL_HRC
#include "optctl.hrc"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif

// class SvxCTLOptionsPage -----------------------------------------------------

IMPL_LINK( SvxCTLOptionsPage, SequenceCheckingCB_Hdl, void*, NOTINTERESTEDIN )
{
    sal_Bool bIsSequenceChecking = m_aSequenceCheckingCB.IsChecked();
    m_aRestrictedCB.Enable( bIsSequenceChecking );
    m_aTypeReplaceCB.Enable( bIsSequenceChecking );
    // #i48117#: by default restricted and type&replace have to be switched on
    if(bIsSequenceChecking)
    {
        m_aTypeReplaceCB.Check( sal_True );
        m_aRestrictedCB.Check( sal_True );
    }
    return 0;
}

SvxCTLOptionsPage::SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, ResId( RID_SVXPAGE_OPTIONS_CTL, DIALOG_MGR() ), rSet ),

    m_aSequenceCheckingFL   ( this, ResId( FL_SEQUENCECHECKING ) ),
    m_aSequenceCheckingCB   ( this, ResId( CB_SEQUENCECHECKING ) ),
    m_aRestrictedCB         ( this, ResId( CB_RESTRICTED ) ),
    m_aTypeReplaceCB        ( this, ResId( CB_TYPE_REPLACE ) ),
    m_aCursorControlFL      ( this, ResId( FL_CURSORCONTROL ) ),
    m_aMovementFT           ( this, ResId( FT_MOVEMENT ) ),
    m_aMovementLogicalRB    ( this, ResId( RB_MOVEMENT_LOGICAL ) ),
    m_aMovementVisualRB     ( this, ResId( RB_MOVEMENT_VISUAL ) ),
    m_aGeneralFL            ( this, ResId( FL_GENERAL ) ),
    m_aNumeralsFT           ( this, ResId( FT_NUMERALS ) ),
    m_aNumeralsLB           ( this, ResId( LB_NUMERALS ) )

{
    FreeResource();

    m_aSequenceCheckingCB.SetClickHdl( LINK( this, SvxCTLOptionsPage, SequenceCheckingCB_Hdl ) );

    m_aNumeralsLB.SetDropDownLineCount( m_aNumeralsLB.GetEntryCount() );
}
// -----------------------------------------------------------------------------
SvxCTLOptionsPage::~SvxCTLOptionsPage()
{
}
// -----------------------------------------------------------------------------
SfxTabPage* SvxCTLOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxCTLOptionsPage( pParent, rAttrSet );
}
// -----------------------------------------------------------------------------
BOOL SvxCTLOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
    SvtCTLOptions aCTLOptions;

    // Sequence checking
    BOOL bChecked = m_aSequenceCheckingCB.IsChecked();
    if ( bChecked != m_aSequenceCheckingCB.GetSavedValue() )
    {
        aCTLOptions.SetCTLSequenceChecking( bChecked );
        bModified = TRUE;
    }

    bChecked = m_aRestrictedCB.IsChecked();
    if( bChecked != m_aRestrictedCB.GetSavedValue() )
    {
        aCTLOptions.SetCTLSequenceCheckingRestricted( bChecked );
        bModified = TRUE;
    }
    bChecked = m_aTypeReplaceCB.IsChecked();
    if( bChecked != m_aTypeReplaceCB.GetSavedValue())
    {
        aCTLOptions.SetCTLSequenceCheckingTypeAndReplace(bChecked);
        bModified = TRUE;
    }

    BOOL bLogicalChecked = m_aMovementLogicalRB.IsChecked();
    BOOL bVisualChecked = m_aMovementVisualRB.IsChecked();
    if ( bLogicalChecked != m_aMovementLogicalRB.GetSavedValue() ||
         bVisualChecked != m_aMovementVisualRB.GetSavedValue() )
    {
        SvtCTLOptions::CursorMovement eMovement =
            bLogicalChecked ? SvtCTLOptions::MOVEMENT_LOGICAL : SvtCTLOptions::MOVEMENT_VISUAL;
        aCTLOptions.SetCTLCursorMovement( eMovement );
        bModified = TRUE;
    }

    USHORT nPos = m_aNumeralsLB.GetSelectEntryPos();
    if ( nPos != m_aNumeralsLB.GetSavedValue() )
    {
        aCTLOptions.SetCTLTextNumerals( (SvtCTLOptions::TextNumerals)nPos );
        bModified = TRUE;
    }

    return bModified;
}
// -----------------------------------------------------------------------------
void SvxCTLOptionsPage::Reset( const SfxItemSet& rSet )
{
    SvtCTLOptions aCTLOptions;

    m_aSequenceCheckingCB.Check( aCTLOptions.IsCTLSequenceChecking() );
    m_aRestrictedCB.Check( aCTLOptions.IsCTLSequenceCheckingRestricted() );
    m_aTypeReplaceCB.Check( aCTLOptions.IsCTLSequenceCheckingTypeAndReplace() );

    SvtCTLOptions::CursorMovement eMovement = aCTLOptions.GetCTLCursorMovement();
    switch ( eMovement )
    {
        case SvtCTLOptions::MOVEMENT_LOGICAL :
            m_aMovementLogicalRB.Check();
            break;

        case SvtCTLOptions::MOVEMENT_VISUAL :
            m_aMovementVisualRB.Check();
            break;

        default:
            DBG_ERRORFILE( "SvxCTLOptionsPage::Reset(): invalid movement enum" );
    }

    USHORT nPos = (USHORT)aCTLOptions.GetCTLTextNumerals();
    DBG_ASSERT( nPos < m_aNumeralsLB.GetEntryCount(), "SvxCTLOptionsPage::Reset(): invalid numerals enum" );
    m_aNumeralsLB.SelectEntryPos( nPos );

    m_aSequenceCheckingCB.SaveValue();
    m_aRestrictedCB.SaveValue();
    m_aTypeReplaceCB.SaveValue();
    m_aMovementLogicalRB.SaveValue();
    m_aMovementVisualRB.SaveValue();
    m_aNumeralsLB.SaveValue();

    sal_Bool bIsSequenceChecking = m_aSequenceCheckingCB.IsChecked();
    m_aRestrictedCB.Enable( bIsSequenceChecking );
    m_aTypeReplaceCB.Enable( bIsSequenceChecking );
}

