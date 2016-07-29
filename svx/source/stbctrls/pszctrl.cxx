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
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
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

#include <svx/dialogs.hrc>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>


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


SFX_IMPL_STATUSBAR_CONTROL(SvxPosSizeStatusBarControl, SvxSizeItem);

class FunctionPopup_Impl : public PopupMenu
{
public:
    explicit FunctionPopup_Impl( sal_uInt32 nCheckEncoded );

    sal_uInt32          GetSelected() const { return nSelected; }

private:
    sal_uInt32          nSelected;

    virtual void    Select() override;
};


FunctionPopup_Impl::FunctionPopup_Impl( sal_uInt32 nCheckEncoded ) :
    PopupMenu( ResId( RID_SVXMNU_PSZ_FUNC, DIALOG_MGR() ) ),
    nSelected( nCheckEncoded )
{
    for ( sal_uInt16 nCheck = 1; nCheck < 32; ++nCheck )
        if ( nCheckEncoded & (1 << nCheck) )
            CheckItem( nCheck );
}


void FunctionPopup_Impl::Select()
{
    sal_uInt16 nCurItemId = GetCurItemId();
    if ( nCurItemId == PSZ_FUNC_NONE )
        nSelected = ( 1 << PSZ_FUNC_NONE );
    else
    {
        nSelected &= (~( 1 << PSZ_FUNC_NONE )); // Clear the "None" bit
        nSelected ^= ( 1 << nCurItemId ); // Toggle the bit corresponding to nCurItemId
        if ( !nSelected )
            nSelected = ( 1 << PSZ_FUNC_NONE );
    }
}

struct SvxPosSizeStatusBarControl_Impl

/*  [Description]

    This implementation-structure of the class SvxPosSizeStatusBarControl
    is done for the un-linking of the changes of the exported interface such as
    the toning down of symbols that are visible externally.

    One instance exists for each SvxPosSizeStatusBarControl-instance
    during its life time
*/

{
    Point     aPos;       // valid when a position is shown
    Size      aSize;      // valid when a size is shown
    OUString  aStr;       // valid when a text is shown
    bool      bPos;       // show position ?
    bool      bSize;      // set size ?
    bool      bTable;     // set table index ?
    bool      bHasMenu;   // set StarCalc popup menu ?
    sal_uInt32  nFunctionSet;  // the selected StarCalc functions encoded in 32 bits
    Image     aPosImage;  // Image to show the position
    Image     aSizeImage; // Image to show the size
};

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
    pImpl( new SvxPosSizeStatusBarControl_Impl )
{
    pImpl->bPos = false;
    pImpl->bSize = false;
    pImpl->bTable = false;
    pImpl->bHasMenu = false;
    pImpl->nFunctionSet = 0;
    pImpl->aPosImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    pImpl->aSizeImage = Image( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    if ( rStb.GetDPIScaleFactor() > 1)
    {
        BitmapEx b = pImpl->aPosImage.GetBitmapEx();
        b.Scale(rStb.GetDPIScaleFactor(), rStb.GetDPIScaleFactor(), BmpScaleFlag::Fast);
        pImpl->aPosImage = Image(b);

        b = pImpl->aSizeImage.GetBitmapEx();
        b.Scale(rStb.GetDPIScaleFactor(), rStb.GetDPIScaleFactor(), BmpScaleFlag::Fast);
        pImpl->aSizeImage = Image(b);
    }

    addStatusListener( STR_POSITION);         // SID_ATTR_POSITION
    addStatusListener( STR_TABLECELL);   // SID_TABLE_CELL
    addStatusListener( STR_FUNC);    // SID_PSZ_FUNCTION
}


/*  [Description]

    Dtor():
    remove the pointer to the implementation class, so that the timer is stopped

*/

SvxPosSizeStatusBarControl::~SvxPosSizeStatusBarControl()
{
}


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
    // Because the combi-controller, always sets the current Id as HelpId
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
        if ( eState == SfxItemState::DEFAULT )
        {
            pImpl->bHasMenu = true;
            if ( pState && dynamic_cast< const SfxUInt32Item* >(pState) !=  nullptr )
                pImpl->nFunctionSet = static_cast<const SfxUInt32Item*>(pState)->GetValue();
        }
        else
            pImpl->bHasMenu = false;
    }
    else if ( SfxItemState::DEFAULT != eState )
    {
        // don't switch to empty display before an empty state was
        // notified for all display types

        if ( nSID == SID_TABLE_CELL )
            pImpl->bTable = false;
        else if ( nSID == SID_ATTR_POSITION )
            pImpl->bPos = false;
        else if ( nSID == GetSlotId() )     // controller is registered for SID_ATTR_SIZE
            pImpl->bSize = false;
        else
        {
            SAL_WARN( "svx.stbcrtls","unknown slot id");
        }
    }
    else if ( dynamic_cast<const SfxPointItem*>( pState) !=  nullptr )
    {
        // show position
        pImpl->aPos = static_cast<const SfxPointItem*>(pState)->GetValue();
        pImpl->bPos = true;
        pImpl->bTable = false;
    }
    else if ( dynamic_cast<const SvxSizeItem*>( pState) !=  nullptr )
    {
        // show size
        pImpl->aSize = static_cast<const SvxSizeItem*>(pState)->GetSize();
        pImpl->bSize = true;
        pImpl->bTable = false;
    }
    else if ( dynamic_cast<const SfxStringItem*>( pState) !=  nullptr )
    {
        // show string (table cel or different)
        pImpl->aStr = static_cast<const SfxStringItem*>(pState)->GetValue();
        pImpl->bTable = true;
        pImpl->bPos = false;
        pImpl->bSize = false;
    }
    else
    {
        SAL_WARN( "svx.stbcrtls", "invalid item type" );
        pImpl->bPos = false;
        pImpl->bSize = false;
        pImpl->bTable = false;
    }

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), nullptr );

    //  set only strings as text at the statusBar, so that the Help-Tips
    //  can work with the text, when it is too long for the statusBar
    OUString aText;
    if ( pImpl->bTable )
        aText = pImpl->aStr;
    GetStatusBar().SetItemText( GetId(), aText );
}


/*  [Description]

    execute popup menu, when the status enables this
*/

void SvxPosSizeStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu && pImpl->bHasMenu )
    {
        sal_uInt32 nSelect = pImpl->nFunctionSet;
        if (!nSelect)
            nSelect = ( 1 << PSZ_FUNC_NONE );
        ScopedVclPtrInstance<FunctionPopup_Impl> aMenu( nSelect );
        if ( aMenu->Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
        {
            nSelect = aMenu->GetSelected();
            if (nSelect)
            {
                if (nSelect == (1 << PSZ_FUNC_NONE))
                    nSelect = 0;

                css::uno::Any a;
                SfxUInt32Item aItem( SID_PSZ_FUNCTION, nSelect );

                css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = "StatusBarFunc";
                aItem.QueryValue( a );
                aArgs[0].Value = a;

                execute( ".uno:StatusBarFunc", aArgs );
//              GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SfxCallMode::RECORD, &aItem, 0L );
            }
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}


/*  [Description]

    Depending on the type to be shown, the value us shown. First the
    rectangle is repainted (removed).
*/

void SvxPosSizeStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();

    const Rectangle& rRect = rUsrEvt.GetRect();
    StatusBar& rBar = GetStatusBar();
    Point aItemPos = rBar.GetItemTextPos( GetId() );
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    if ( pImpl->bPos || pImpl->bSize )
    {
        // count the position for showing the size
        long nSizePosX =
            rRect.Left() + rRect.GetWidth() / 2 + PAINT_OFFSET;
        // draw position
        Point aPnt = rRect.TopLeft();
        aPnt.Y() = aItemPos.Y();
        aPnt.X() += PAINT_OFFSET;
        pDev->DrawImage( aPnt, pImpl->aPosImage );
        aPnt.X() += pImpl->aPosImage.GetSizePixel().Width();
        aPnt.X() += PAINT_OFFSET;
        OUString aStr = GetMetricStr_Impl( pImpl->aPos.X());
        aStr += " / ";
        aStr += GetMetricStr_Impl( pImpl->aPos.Y());
        Rectangle aRect(aPnt, Point(nSizePosX, rRect.Bottom()));
        pDev->DrawRect(aRect);
        vcl::Region aOrigRegion(pDev->GetClipRegion());
        pDev->SetClipRegion(vcl::Region(aRect));
        pDev->DrawText(aPnt, aStr);
        pDev->SetClipRegion(aOrigRegion);

        // draw the size, when available
        aPnt.X() = nSizePosX;

        if ( pImpl->bSize )
        {
            pDev->DrawImage( aPnt, pImpl->aSizeImage );
            aPnt.X() += pImpl->aSizeImage.GetSizePixel().Width();
            Point aDrwPnt = aPnt;
            aPnt.X() += PAINT_OFFSET;
            aStr = GetMetricStr_Impl( pImpl->aSize.Width() );
            aStr += " x ";
            aStr += GetMetricStr_Impl( pImpl->aSize.Height() );
            aRect = Rectangle(aDrwPnt, rRect.BottomRight());
            pDev->DrawRect(aRect);
            aOrigRegion = pDev->GetClipRegion();
            pDev->SetClipRegion(vcl::Region(aRect));
            pDev->DrawText(aPnt, aStr);
            pDev->SetClipRegion(aOrigRegion);
        }
        else
            pDev->DrawRect( Rectangle( aPnt, rRect.BottomRight() ) );
    }
    else if ( pImpl->bTable )
    {
        pDev->DrawRect( rRect );
        pDev->DrawText( Point(
            rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( pImpl->aStr ) / 2,
            aItemPos.Y() ), pImpl->aStr );
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
