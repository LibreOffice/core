/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_AxisLabel.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:13:39 $
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

        aFlOrder( this, SchResId( FL_AXIS_LABEL_ORDER ) ),
        aRbSideBySide( this, SchResId( RB_AXIS_LABEL_SIDEBYSIDE ) ),
        aRbUpDown( this, SchResId( RB_AXIS_LABEL_UPDOWN ) ),
        aRbDownUp( this, SchResId( RB_AXIS_LABEL_DOWNUP ) ),
        aRbAuto( this, SchResId( RB_AXIS_LABEL_AUTOORDER ) ),

        aFlSeparator( this, SchResId( FL_SEPARATOR ) ),
        aFlTextFlow( this, SchResId( FL_AXIS_LABEL_TEXTFLOW ) ),
        aCbTextOverlap( this, SchResId( CB_AXIS_LABEL_TEXTOVERLAP ) ),
        aCbTextBreak( this, SchResId( CB_AXIS_LABEL_TEXTBREAK ) ),

        aFlOrient( this, SchResId( FL_AXIS_LABEL_ORIENTATION ) ),
        aCtrlDial( this, SchResId( CT_AXIS_LABEL_DIAL ) ),
        aFtRotate( this, SchResId( FT_AXIS_LABEL_DEGREES ) ),
        aNfRotate( this, SchResId( NF_AXIS_LABEL_ORIENT ) ),
        aCbStacked( this, SchResId( PB_AXIS_LABEL_TEXTSTACKED ) ),
        aOrientHlp( this, aCtrlDial, aNfRotate, aCbStacked ),

        m_bShowStaggeringControls( true ),
////        bAllowTextOverlap( TRUE ),

        m_nInitialDegrees( 0 ),
        m_bHasInitialDegrees( true ),
        m_bInitialStacking( false ),
        m_bHasInitialStacking( true )
{
    FreeResource();

    aCbStacked.EnableTriState( FALSE );
    aOrientHlp.AddDependentWindow( aFlOrient );
    aOrientHlp.AddDependentWindow( aFtRotate, STATE_CHECK );

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
    bool bStacked = false;
    if( aOrientHlp.GetStackedState() != STATE_DONTKNOW )
    {
        bStacked = aOrientHlp.GetStackedState() == STATE_CHECK;
        if( !m_bHasInitialStacking || (bStacked != m_bInitialStacking) )
            rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );
    }

    if( aCtrlDial.HasRotation() )
    {
        sal_Int32 nDegrees = bStacked ? 0 : aCtrlDial.GetRotation();
        if( !m_bHasInitialDegrees || (nDegrees != m_nInitialDegrees) )
            rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

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
        rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXTBREAK, aCbTextBreak.IsChecked() ) );
    if( aCbShowDescription.GetState() != STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_AXIS_SHOWDESCR, aCbShowDescription.IsChecked() ) );

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

    // check new degree item
    m_nInitialDegrees = 0;
    aState = rInAttrs.GetItemState( SCHATTR_TEXT_DEGREES, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_SET )
        m_nInitialDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();

    m_bHasInitialDegrees = aState != SFX_ITEM_DONTCARE;
    if( m_bHasInitialDegrees )
        aCtrlDial.SetRotation( m_nInitialDegrees );
    else
        aCtrlDial.SetNoRotation();

    // check stacked item
    m_bInitialStacking = false;
    aState = rInAttrs.GetItemState( SCHATTR_TEXT_STACKED, FALSE, &pPoolItem );
    if( aState == SFX_ITEM_SET )
        m_bInitialStacking = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();

    m_bHasInitialStacking = aState != SFX_ITEM_DONTCARE;
    if( m_bHasInitialDegrees )
        aOrientHlp.SetStackedState( m_bInitialStacking ? STATE_CHECK : STATE_NOCHECK );
    else
        aOrientHlp.SetStackedState( STATE_DONTKNOW );

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

    aOrientHlp.Enable( bEnable );
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
