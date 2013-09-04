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

#include <limits.h>
#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <svl/stritem.hxx>
#include <svl/ptitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>

#include "svx/pszctrl.hxx"

#define PAINT_OFFSET    5

#include <editeng/sizeitem.hxx>
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include "stbctrls.h"
#include "sfx2/module.hxx"

#include <svx/dialogs.hrc>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>

// -----------------------------------------------------------------------

/*  [Description]

    Function used to create a text representation of
    a metric value

    nVal is the metric value in the unit eUnit.

    [cross reference]

    <SvxPosSizeStatusBarControl::Paint(const UserDrawEvent&)>
*/

OUString SvxPosSizeStatusBarControl::GetMetricStr_Impl( long nVal )
{
    // deliver and set the Metric of the application
    FieldUnit eOutUnit = SfxModule::GetModuleFieldUnit( getFrameInterface() );
    FieldUnit eInUnit = FUNIT_100TH_MM;

    OUString sMetric;
    const sal_Unicode cSep = Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep()[0];
    sal_Int64 nConvVal = MetricField::ConvertValue( nVal * 100, 0L, 0, eInUnit, eOutUnit );

    if ( nConvVal < 0 && ( nConvVal / 100 == 0 ) )
        sMetric += "-";
    sMetric += OUString::number(nConvVal / 100);

    if( FUNIT_NONE != eOutUnit )
    {
        sMetric += OUString(cSep);
        sal_Int64 nFract = nConvVal % 100;

        if ( nFract < 0 )
            nFract *= -1;
        if ( nFract < 10 )
            sMetric += "0";
        sMetric += OUString::number(nFract);
    }

    return sMetric;
}

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL(SvxPosSizeStatusBarControl, SvxSizeItem);

// class FunctionPopup_Impl ----------------------------------------------

class FunctionPopup_Impl : public PopupMenu
{
public:
    FunctionPopup_Impl( sal_uInt16 nCheck );

    sal_uInt16          GetSelected() const { return nSelected; }

private:
    sal_uInt16          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( sal_uInt16 nCheck ) :
    PopupMenu( ResId( RID_SVXMNU_PSZ_FUNC, DIALOG_MGR() ) ),
    nSelected( 0 )
{
    if (nCheck)
        CheckItem( nCheck );
}

// -----------------------------------------------------------------------

void FunctionPopup_Impl::Select()
{
    nSelected = GetCurItemId();
}

// struct SvxPosSizeStatusBarControl_Impl --------------------------------

struct SvxPosSizeStatusBarControl_Impl

/*  [Description]

    This implementation-structure of the class SvxPosSizeStatusBarControl
    is done for the un-linking of the changes of the exported interface such as
    the toning down of symbols that are visible externaly.

    One instance exists for each SvxPosSizeStatusBarControl-instance
    during it's life time
*/

{
    Point     aPos;       // valid when a position is shown
    Size      aSize;      // valid when a size is shown
    OUString  aStr;       // valid when a text is shown
    bool      bPos;       // show position ?
    bool      bSize;      // set size ?
    bool      bTable;     // set table index ?
    bool      bHasMenu;   // set StarCalc popup menu ?
    sal_uInt16  nFunction;  // the selected StarCalc function
    Image     aPosImage;  // Image to show the position
    Image     aSizeImage; // Image to show the size
};

// class SvxPosSizeStatusBarControl ------------------------------------------

/*  [Description]

    Ctor():
    Create an instance of the implementation class,
    load the images for the position and size
*/

#define STR_POSITION ".uno:Position"
#define STR_TABLECELL ".uno:StateTableCell"
#define STR_FUNC ".uno:StatusBarFunc"

SvxPosSizeStatusBarControl::SvxPosSizeStatusBarControl( sal_uInt16 _nSlotId,
                                                        sal_uInt16 _nId,
                                                        StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    pImp( new SvxPosSizeStatusBarControl_Impl )
{
    pImp->bPos = false;
    pImp->bSize = false;
    pImp->bTable = false;
    pImp->bHasMenu = false;
    pImp->nFunction = 0;
    pImp->aPosImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    pImp->aSizeImage = Image( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    addStatusListener( OUString( STR_POSITION ));         // SID_ATTR_POSITION
    addStatusListener( OUString( STR_TABLECELL ));   // SID_TABLE_CELL
    addStatusListener( OUString( STR_FUNC ));    // SID_PSZ_FUNCTION
}

// -----------------------------------------------------------------------

/*  [Description]

    Dtor():
    remove the pointer to the implementation class, so that the timer is stopped

*/

SvxPosSizeStatusBarControl::~SvxPosSizeStatusBarControl()
{
    delete pImp;
}

// -----------------------------------------------------------------------

/*  [Description]

    SID_PSZ_FUNCTION activates the popup menu for Calc:

    Status overview
    Depending on the type of the item, a special setting is enabled, the others disabled.

                NULL/Void   SfxPointItem    SvxSizeItem     SfxStringItem
    ------------------------------------------------------------------------
    Position    sal_False                                       FALSE
    Size        FALSE                       TRUE            FALSE
    Text        sal_False                       sal_False           TRUE

*/

void SvxPosSizeStatusBarControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                               const SfxPoolItem* pState )
{
    // Because the combi-controller, always sets the curent Id as HelpId
    // first clean the cached HelpText
    GetStatusBar().SetHelpText( GetId(), "" );

    switch ( nSID )
    {
        case SID_ATTR_POSITION : GetStatusBar().SetHelpId( GetId(), STR_POSITION ); break;
        case SID_TABLE_CELL: GetStatusBar().SetHelpId( GetId(), STR_TABLECELL ); break;
        case SID_PSZ_FUNCTION: GetStatusBar().SetHelpId( GetId(), STR_FUNC ); break;
        default: break;
    }

    if ( nSID == SID_PSZ_FUNCTION )
    {
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pImp->bHasMenu = true;
            if ( pState && pState->ISA(SfxUInt16Item) )
                pImp->nFunction = ((const SfxUInt16Item*)pState)->GetValue();
        }
        else
            pImp->bHasMenu = false;
    }
    else if ( SFX_ITEM_AVAILABLE != eState )
    {
        // don't switch to empty display before an empty state was
        // notified for all display types

        if ( nSID == SID_TABLE_CELL )
            pImp->bTable = false;
        else if ( nSID == SID_ATTR_POSITION )
            pImp->bPos = false;
        else if ( nSID == GetSlotId() )     // controller is registered for SID_ATTR_SIZE
            pImp->bSize = false;
        else
        {
            SAL_WARN( "svx.stbcrtls","unknown slot id");
        }
    }
    else if ( pState->ISA( SfxPointItem ) )
    {
        // show position
        pImp->aPos = ( (SfxPointItem*)pState )->GetValue();
        pImp->bPos = true;
        pImp->bTable = false;
    }
    else if ( pState->ISA( SvxSizeItem ) )
    {
        // show size
        pImp->aSize = ( (SvxSizeItem*)pState )->GetSize();
        pImp->bSize = true;
        pImp->bTable = false;
    }
    else if ( pState->ISA( SfxStringItem ) )
    {
        // show string (table cel or different)
        pImp->aStr = ( (SfxStringItem*)pState )->GetValue();
        pImp->bTable = true;
        pImp->bPos = false;
        pImp->bSize = false;
    }
    else
    {
        SAL_WARN( "svx.stbcrtls", "invalid item type" );
        pImp->bPos = false;
        pImp->bSize = false;
        pImp->bTable = false;
    }

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );

    //  set only strings as text at the statusBar, so that the Help-Tips
    //  can work with the text, when it is too long for the statusBar
    OUString aText;
    if ( pImp->bTable )
        aText = pImp->aStr;
    GetStatusBar().SetItemText( GetId(), aText );
}

// -----------------------------------------------------------------------

/*  [Description]

    execute popup menu, when the status enables this
*/

void SvxPosSizeStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && pImp->bHasMenu )
    {
        sal_uInt16 nSelect = pImp->nFunction;
        if (!nSelect)
            nSelect = PSZ_FUNC_NONE;
        FunctionPopup_Impl aMenu( nSelect );
        if ( aMenu.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
        {
            nSelect = aMenu.GetSelected();
            if (nSelect)
            {
                if (nSelect == PSZ_FUNC_NONE)
                    nSelect = 0;

                ::com::sun::star::uno::Any a;
                SfxUInt16Item aItem( SID_PSZ_FUNCTION, nSelect );

                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = OUString( "StatusBarFunc" );
                aItem.QueryValue( a );
                aArgs[0].Value = a;

                execute( OUString( ".uno:StatusBarFunc" ), aArgs );
//              GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

// -----------------------------------------------------------------------

/*  [Description]

    Depending on the type to be shown, the value us shown. First the
    rectangle is repainted (removed).
*/

void SvxPosSizeStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice* pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "no OutputDevice on UserDrawEvent" );
    const Rectangle& rRect = rUsrEvt.GetRect();
    StatusBar& rBar = GetStatusBar();
    Point aItemPos = rBar.GetItemTextPos( GetId() );
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    if ( pImp->bPos || pImp->bSize )
    {
        // count the position for showing the size
        long nSizePosX =
            rRect.Left() + rRect.GetWidth() / 2 + PAINT_OFFSET;
        // draw position
        Point aPnt = rRect.TopLeft();
        aPnt.Y() = aItemPos.Y();
        aPnt.X() += PAINT_OFFSET;
        pDev->DrawImage( aPnt, pImp->aPosImage );
        aPnt.X() += pImp->aPosImage.GetSizePixel().Width();
        aPnt.X() += PAINT_OFFSET;
        OUString aStr = GetMetricStr_Impl( pImp->aPos.X());
        aStr += " / ";
        aStr += GetMetricStr_Impl( pImp->aPos.Y());
        pDev->DrawRect(
            Rectangle( aPnt, Point( nSizePosX, rRect.Bottom() ) ) );
        pDev->DrawText( aPnt, aStr );

        // draw the size, when available
        aPnt.X() = nSizePosX;

        if ( pImp->bSize )
        {
            pDev->DrawImage( aPnt, pImp->aSizeImage );
            aPnt.X() += pImp->aSizeImage.GetSizePixel().Width();
            Point aDrwPnt = aPnt;
            aPnt.X() += PAINT_OFFSET;
            aStr = GetMetricStr_Impl( pImp->aSize.Width() );
            aStr += " x ";
            aStr += GetMetricStr_Impl( pImp->aSize.Height() );
            pDev->DrawRect( Rectangle( aDrwPnt, rRect.BottomRight() ) );
            pDev->DrawText( aPnt, aStr );
        }
        else
            pDev->DrawRect( Rectangle( aPnt, rRect.BottomRight() ) );
    }
    else if ( pImp->bTable )
    {
        pDev->DrawRect( rRect );
        pDev->DrawText( Point(
            rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( pImp->aStr ) / 2,
            aItemPos.Y() ), pImp->aStr );
    }
    else
    {
        // Empty display if neither size nor table position are available.
        // Date/Time are no longer used (#65302#).
        pDev->DrawRect( rRect );
    }

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
