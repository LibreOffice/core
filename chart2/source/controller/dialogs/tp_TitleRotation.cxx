/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_TitleRotation.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:19:02 $
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
#include "tp_TitleRotation.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "SchSfxItemIds.hxx"
/*
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "Bitmaps_HC.hrc"
*/

/*
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include "schattr.hxx"
*/
// header for class SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
//#define ITEMID_CHARTTEXTORDER SCHATTR_TEXT_ORDER
// #define ITEMID_CHARTTEXTORIENT   SCHATTR_TEXT_ORIENT
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <svx/chrtitem.hxx>
#endif
/*
#include "schresid.hxx"
#include "strings.hrc"
#include "chtmodel.hxx"
#include "attrib.hxx"
#include "attrib.hrc"
#include "app.hrc"
*/

//.............................................................................
namespace chart
{
//.............................................................................

SchAlignmentTabPage::SchAlignmentTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_ALIGNMENT), rInAttrs),

//  aCbxTextBreak   ( this, ResId( CBX_TEXTBREAK ) ),
//  aCbxTextOverlap ( this, ResId( CBX_TEXTOVERLAP ) ),
//  aFlTextBreak    ( this, ResId( FL_TEXTBREAK ) ),
//  aRbtSideBySide(this, ResId(RBT_SIDEBYSIDE)),
//  aRbtUpDown(this, ResId(RBT_UPDOWN)),
//  aRbtDownUp(this, ResId(RBT_DOWNUP)),
//  aRbtAutoOrder(this, ResId(RBT_AUTOORDER)),
//  aFlOrder(this, ResId(FL_ORDER)),
//  eOrderMode(CHORDMODE_X_AXIS),

    //Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
    aFlAlign        ( this, ResId( FL_ALIGN ) ),
    aCtrlDial       ( this, ResId( CTR_DIAL ) ),
    aFtRotate       ( this, ResId( FT_DEGREES ) ),
    aNfRotate       ( this, ResId( NF_ORIENT ) ),
    aCbStacked      ( this, ResId( BTN_TXTSTACKED ) ),
    aOrientHlp      ( this, aCtrlDial, aNfRotate, aCbStacked )

{
    FreeResource();

    aCbStacked.EnableTriState( FALSE );
    aOrientHlp.AddDependentWindow( aFtRotate, STATE_CHECK );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SchAlignmentTabPage::~SchAlignmentTabPage()
{
}

/*************************************************************************
|*
|* Markierungs-Handler
|*
\************************************************************************/

/*
IMPL_LINK( SchAlignmentTabPage, CheckButtonHdl, Button *, pBtn )
{

    if (eOrderMode != CHORDMODE_NONE)
    {
        BOOL bEnable=TRUE;
        aRbtSideBySide.Enable(bEnable);
        aRbtUpDown.Enable(bEnable);
        aRbtDownUp.Enable(bEnable);
        aRbtAutoOrder.Enable(bEnable);
        aFlOrder.Enable(bEnable);
    }
    return 0;
}
*/

/*************************************************************************
|*
|* Erzeugung
|*
\*************************************************************************/

SfxTabPage* SchAlignmentTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchAlignmentTabPage(pWindow, rOutAttrs);
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/

BOOL SchAlignmentTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    //Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
    bool bStacked = aOrientHlp.GetStackedState() == STATE_CHECK;
    rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    sal_Int32 nDegrees = bStacked ? 0 : aCtrlDial.GetRotation();
    rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );

    /*
    if (eOrderMode != CHORDMODE_NONE)
    {
        SvxChartTextOrder eOrder;

//      if (aRbtUpDown.IsChecked()) eOrder = CHTXTORDER_UPDOWN;
//      else if (aRbtDownUp.IsChecked()) eOrder = CHTXTORDER_DOWNUP;
//           else if (aRbtAutoOrder.IsChecked()) eOrder = CHTXTORDER_AUTO;
                  else eOrder = CHTXTORDER_SIDEBYSIDE;

        rOutAttrs.Put(SvxChartTextOrderItem(eOrder));
    }
    */

    // Textumbruch
//  BOOL bTextOverlap = aCbxTextOverlap.IsChecked();
//  rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_OVERLAP, bTextOverlap ) );

//  BOOL bTextBreak = aCbxTextBreak.IsChecked();
//  rOutAttrs.Put( SfxBoolItem( SID_TEXTBREAK, bTextBreak ) );

    /*
    BOOL                bAttrsChanged   = FALSE;
    const SfxItemSet&   rOldSet         = GetItemSet();
    */
//  TriState            eState;

    return TRUE;
}

/*************************************************************************
|*
|* Initialisierung
|*
\*************************************************************************/

void SchAlignmentTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    /*
    if (rInAttrs.GetItemState(SCHATTR_TEXT_ORDER,
                              TRUE, &pPoolItem) == SFX_ITEM_SET &&
        eOrderMode != CHORDMODE_NONE)
    {
        switch (((const SvxChartTextOrderItem*)pPoolItem)->GetValue())
        {
            case CHTXTORDER_SIDEBYSIDE:
                aRbtSideBySide.Check(TRUE);
                break;

            case CHTXTORDER_UPDOWN:
                aRbtUpDown.Check(TRUE);
                break;

            case CHTXTORDER_DOWNUP:
                aRbtDownUp.Check(TRUE);
                break;

            case CHTXTORDER_AUTO:
                aRbtAutoOrder.Check(TRUE);
                break;
        }
    }
    */

    //Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
    const SfxPoolItem* pItem;

    pItem = GetItem( rInAttrs, SCHATTR_TEXT_DEGREES );
    sal_Int32 nDegrees = pItem ? ((const SfxInt32Item*)pItem)->GetValue() : 0;
    aCtrlDial.SetRotation( nDegrees );

    pItem = GetItem( rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && ((const SfxBoolItem*)pItem)->GetValue();
    aOrientHlp.SetStackedState( bStacked ? STATE_CHECK : STATE_NOCHECK );

    // Textumbruch
    /*
    if( rInAttrs.GetItemState( SID_TEXTBREAK, FALSE, &pPoolItem ) >= SFX_ITEM_AVAILABLE )
    {
        BOOL bTextBreak = ( (const SfxBoolItem*) pPoolItem)->GetValue();
//      aCbxTextBreak.Check( bTextBreak );
    }
    else
    {
//      aCbxTextBreak.Hide();
//      aFlTextBreak.Hide();
    }
    // Textueberlapp
    if( (rInAttrs.GetItemState( SCHATTR_TEXT_OVERLAP, FALSE, &pPoolItem ) >= SFX_ITEM_AVAILABLE) && bTextCanOverlap)
    {
        BOOL bTextOverlap = ( (const SfxBoolItem*) pPoolItem)->GetValue();
//      aCbxTextOverlap.Check( bTextOverlap );
    }
    else
    {
//      aCbxTextOverlap.Hide();
    }
    */
}

/*************************************************************************
|*
|* Anordnungs-Controls hiden
|*
\*************************************************************************/

/*
void SchAlignmentTabPage::SetOrderMode(OrderMode eMode)
{
    eOrderMode = eMode;

    if (eOrderMode == CHORDMODE_NONE)
    {
        aRbtSideBySide.Hide();
        aRbtUpDown.Hide();
        aRbtDownUp.Hide();
        aRbtAutoOrder.Hide();
        aFlOrder.Hide();
    }
}
*/

//.............................................................................
} //namespace chart
//.............................................................................


