/*************************************************************************
 *
 *  $RCSfile: tp_AxisLabel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2004-08-04 14:33:25 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "tp_AxisLabel.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "SchSfxItemIds.hxx"
#include "SchSlotIds.hxx"

#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
// #define ITEMID_CHARTTEXTORIENT   SCHATTR_TEXT_ORIENT
// header for SvxChartTextOrientItem / SvxChartTextOrderItem
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif

/*
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
*/
// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
/*
// header for SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

#include "schattr.hxx"
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT
// header for SvxChartTextOrientItem / SvxChartTextOrderItem
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif

#include "chtmodel.hxx"
#include "schresid.hxx"
#include "app.hrc"

#include "tplabel.hxx"
#include "tplabel.hrc"
*/
//.............................................................................
namespace chart
{
//.............................................................................

SchAxisLabelTabPage::SchAxisLabelTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_AXIS_LABEL ), rInAttrs ),

        aCbShowDescription( this, SchResId( CB_AXIS_LABEL_SCHOW_DESCR ) ),

#if 0 // DR: TODO new control types
        aWOAngle( this,
                  SchResId( CT_AXIS_LABEL_DIAL ),
                  SchResId( PB_AXIS_LABEL_TEXTSTACKED ),
                  SchResId( FT_AXIS_LABEL_DEGREES ),
                  SchResId( NF_AXIS_LABEL_ORIENT ),
                  SchResId( FT_UNUSED ),
                  SchResId( CT_UNUSED ),
                  SchResId( FL_AXIS_LABEL_ORIENTATION ) ),
#endif

        aFlTextFlow( this, SchResId( FL_AXIS_LABEL_TEXTFLOW ) ),
        aCbTextOverlap( this, SchResId( CB_AXIS_LABEL_TEXTOVERLAP ) ),
        aCbTextBreak( this, SchResId( CB_AXIS_LABEL_TEXTBREAK ) ),

        aFlOrder( this, SchResId( FL_AXIS_LABEL_ORDER ) ),
        aRbSideBySide( this, SchResId( RB_AXIS_LABEL_SIDEBYSIDE ) ),
        aRbUpDown( this, SchResId( RB_AXIS_LABEL_UPDOWN ) ),
        aRbDownUp( this, SchResId( RB_AXIS_LABEL_DOWNUP ) ),
        aRbAuto( this, SchResId( RB_AXIS_LABEL_AUTOORDER ) ),

           aFlSeparator( this, SchResId( FL_SEPARATOR ) ),

        m_bShowStaggeringControls( true ),
////        bAllowTextOverlap( TRUE ),

        m_nInitialDegrees( 0 ),
        m_bInitialStacking( FALSE )
{
    FreeResource();

    aCbShowDescription.SetClickHdl( LINK( this, SchAxisLabelTabPage, ToggleShowLabel ) );

    //  Make the fixed line separator vertical.
    aFlSeparator.SetStyle (aFlSeparator.GetStyle() | WB_VERT);

    Construct();
}

SchAxisLabelTabPage::~SchAxisLabelTabPage()
{}

void SchAxisLabelTabPage::Construct()
{
}


SfxTabPage* SchAxisLabelTabPage::Create( Window* pParent, const SfxItemSet& rAttrs )
{
    return new SchAxisLabelTabPage( pParent, rAttrs );
}

BOOL SchAxisLabelTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
#if 0 // DR: TODO new control types
//  SvxChartTextOrient eOrient;
    BOOL bIsStacked = FALSE;
    long nDegrees = aWOAngle.GetDegrees() * 100L;

    if( nDegrees != m_nInitialDegrees )
    {
        rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

    BOOL bStacked = aWOAngle.IsStackedTxt();
    if( bStacked != m_bInitialStacking )
    {
        if( aWOAngle.IsStackedTxt() )
        {
//             eOrient = CHTXTORIENT_STACKED;
            bIsStacked = TRUE;
//             rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, 0 ) );
        }
    }

//     if( eOrient != CHTXTORIENT_STACKED )
//  {
//      if( nDegrees == 0L )
//          eOrient = CHTXTORIENT_STANDARD;
//      else if( nDegrees <= 18000L )
//          eOrient = CHTXTORIENT_BOTTOMTOP;
//      else
//          eOrient = CHTXTORIENT_TOPBOTTOM;
//  }

//  rOutAttrs.Put( SvxChartTextOrientItem( eOrient ) );
    rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bIsStacked ) );

    if( m_bShowStaggeringControls )
    {
        SvxChartTextOrder eOrder;
        bool bRadioButtonChecked = true;

        if( aRbUpDown.IsChecked())
            eOrder = CHTXTORDER_UPDOWN;
        else if( aRbDownUp.IsChecked())
            eOrder = CHTXTORDER_DOWNUP;
        else if( aRbAuto.IsChecked())
            eOrder = CHTXTORDER_AUTO;
        else if( aRbSideBySide.IsChecked())
            eOrder = CHTXTORDER_SIDEBYSIDE;
        else
            bRadioButtonChecked = false;

        if( bRadioButtonChecked )
            rOutAttrs.Put( SvxChartTextOrderItem( eOrder ));
    }

    if( aCbTextOverlap.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_OVERLAP, aCbTextOverlap.IsChecked() ) );
    if( aCbTextBreak.GetState() != STATE_DONTKNOW )
//         rOutAttrs.Put( SfxBoolItem( SID_TEXTBREAK, aCbTextBreak.IsChecked() ) );
        rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXTBREAK, aCbTextBreak.IsChecked() ) );
    if( aCbShowDescription.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_AXIS_SHOWDESCR, aCbShowDescription.IsChecked() ) );
#endif

    return TRUE;
}

void SchAxisLabelTabPage::Reset( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem* pPoolItem = NULL;
    SfxItemState aState = SFX_ITEM_UNKNOWN;

    // show description ----------
    aState = rInAttrs.GetItemState( SCHATTR_AXIS_SHOWDESCR, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        aCbShowDescription.EnableTriState( TRUE );
        aCbShowDescription.SetState( STATE_DONTKNOW );
    }
    else
    {
        aCbShowDescription.EnableTriState( FALSE );
        BOOL bCheck = FALSE;
        if( aState == SFX_ITEM_SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCbShowDescription.Check( bCheck );

        if( ( aState & SFX_ITEM_DEFAULT ) == 0 )
            aCbShowDescription.Hide();
    }

    // Rotation as orient item or in degrees ----------
    BOOL bStacked = FALSE;
    short nDegrees = 0;

    // check new degree item
    aState = rInAttrs.GetItemState( SCHATTR_TEXT_DEGREES, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        // tristate for SvxWinOrientation missing
        nDegrees = 0;
    }
    else if( aState == SFX_ITEM_SET )
    {
        nDegrees = static_cast< short >( static_cast< const SfxInt32Item * >( pPoolItem )->GetValue() / 100L );
    }

    // check old orientation item
//     if( rInAttrs.GetItemState( SCHATTR_TEXT_ORIENT, TRUE, &pPoolItem ) == SFX_ITEM_SET )
//  {
//      SvxChartTextOrient eOrient = static_cast< const SvxChartTextOrientItem * >( pPoolItem )->GetValue();
//      switch( eOrient )
//      {
//          case CHTXTORIENT_AUTOMATIC:
//          case CHTXTORIENT_STANDARD:
//              break;
//          case CHTXTORIENT_TOPBOTTOM:
//              if( !nDegrees )
//                     nDegrees = 270;
//              break;
//          case CHTXTORIENT_BOTTOMTOP:
//              if( !nDegrees )
//                     nDegrees = 90;
//              break;
//          case CHTXTORIENT_STACKED:
//                 bStacked = TRUE;
//              break;
//      }
//  }

    if( rInAttrs.GetItemState( SCHATTR_TEXT_STACKED, TRUE, &pPoolItem ) == SFX_ITEM_SET )
    {
        bStacked = reinterpret_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
    }

#if 0 // DR: TODO new control types
    aWOAngle.SetDegrees( nDegrees );
    aWOAngle.SetStackedTxt( bStacked );
#endif
    m_nInitialDegrees = nDegrees;
    m_bInitialStacking = bStacked;

    // Text overlap ----------
    aState = rInAttrs.GetItemState( SCHATTR_TEXT_OVERLAP, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        aCbTextOverlap.EnableTriState( TRUE );
        aCbTextOverlap.SetState( STATE_DONTKNOW );
    }
    else
    {
        aCbTextOverlap.EnableTriState( FALSE );
        BOOL bCheck = FALSE;
        if( aState == SFX_ITEM_SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCbTextOverlap.Check( bCheck );

        if( ( aState & SFX_ITEM_DEFAULT ) == 0 )
            aCbTextOverlap.Hide();
    }

    // text break ----------
//     aState = rInAttrs.GetItemState( SID_TEXTBREAK, FALSE, &pPoolItem );
    aState = rInAttrs.GetItemState( SCHATTR_TEXTBREAK, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_DONTCARE )
    {
        aCbTextBreak.EnableTriState( TRUE );
        aCbTextBreak.SetState( STATE_DONTKNOW );
    }
    else
    {
        aCbTextBreak.EnableTriState( FALSE );
        BOOL bCheck = FALSE;
        if( aState == SFX_ITEM_SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCbTextBreak.Check( bCheck );

        if( ( aState & SFX_ITEM_DEFAULT ) == 0 )
        {
            aCbTextBreak.Hide();
            if( ! aCbTextOverlap.IsVisible() )
                aFlTextFlow.Hide();
        }
    }

    // text order ----------
    if( m_bShowStaggeringControls )
    {
        aState = rInAttrs.GetItemState( SCHATTR_TEXT_ORDER, FALSE, &pPoolItem );
        if( aState == SFX_ITEM_SET )
        {
            SvxChartTextOrder eOrder = static_cast< const SvxChartTextOrderItem * >( pPoolItem )->GetValue();

            switch( eOrder )
            {
                case CHTXTORDER_SIDEBYSIDE:
                    aRbSideBySide.Check();
                    break;
                case CHTXTORDER_UPDOWN:
                    aRbUpDown.Check();
                    break;
                case CHTXTORDER_DOWNUP:
                    aRbDownUp.Check();
                    break;
                case CHTXTORDER_AUTO:
                    aRbAuto.Check();
                    break;
            }
        }
    }

    ToggleShowLabel( (void*)0 );
}

void SchAxisLabelTabPage::ShowStaggeringControls( BOOL bShowStaggeringControls )
{
    m_bShowStaggeringControls = bShowStaggeringControls;

    if( !m_bShowStaggeringControls )
    {
        aRbSideBySide.Hide();
        aRbUpDown.Hide();
        aRbDownUp.Hide();
        aRbAuto.Hide();
        aFlOrder.Hide();
    }
}

// event handling routines
// -----------------------

IMPL_LINK ( SchAxisLabelTabPage, ToggleShowLabel, void *, EMPTYARG )
{
    BOOL bEnable = ( aCbShowDescription.GetState() != STATE_NOCHECK );

#if 0 // DR: TODO new control types
    if( bEnable )
        aWOAngle.Enable();
    else
        aWOAngle.Disable();
#endif

    aFlOrder.Enable( bEnable );
    aRbSideBySide.Enable( bEnable );
    aRbUpDown.Enable( bEnable );
    aRbDownUp.Enable( bEnable );
    aRbAuto.Enable( bEnable );

    aFlTextFlow.Enable( bEnable );
    aCbTextOverlap.Enable( bEnable );
    aCbTextBreak.Enable( bEnable );

    return 0L;
}
//.............................................................................
} //namespace chart
//.............................................................................
