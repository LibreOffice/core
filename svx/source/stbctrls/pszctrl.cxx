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
#include <vcl/builder.hxx>
#include <vcl/field.hxx>
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
#include <sal/log.hxx>

#include <svx/pszctrl.hxx>

#define PAINT_OFFSET    5

#include <editeng/sizeitem.hxx>
#include <svx/dlgutil.hxx>
#include "stbctrls.h"

#include <svx/svxids.hrc>
#include <bitmaps.hlst>
#include <unotools/localedatawrapper.hxx>


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

    OUString sMetric;
    const sal_Unicode cSep = Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep()[0];
    sal_Int64 nConvVal = MetricField::ConvertValue( nVal * 100, 0L, 0, FieldUnit::MM_100TH, eOutUnit );

    if ( nConvVal < 0 && ( nConvVal / 100 == 0 ) )
        sMetric += "-";
    sMetric += OUString::number(nConvVal / 100);

    if( FieldUnit::NONE != eOutUnit )
    {
        sMetric += OUStringLiteral1(cSep);
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

class FunctionPopup_Impl
{
    VclBuilder        m_aBuilder;
    VclPtr<PopupMenu> m_xMenu;
    sal_uInt32 const  m_nSelected;
    static sal_uInt16 id_to_function(const OString& rIdent);
    sal_uInt16 function_to_id(sal_uInt16 nFunc) const;
public:
    explicit FunctionPopup_Impl( sal_uInt32 nCheckEncoded );
    sal_uInt16 Execute(vcl::Window* pWindow, const Point& rPopupPos) { return m_xMenu->Execute(pWindow, rPopupPos); }
    sal_uInt32 GetSelected() const;
};

sal_uInt16 FunctionPopup_Impl::id_to_function(const OString& rIdent)
{
    if (rIdent == "avg")
        return PSZ_FUNC_AVG;
    else if (rIdent == "counta")
        return PSZ_FUNC_COUNT2;
    else if (rIdent == "count")
        return PSZ_FUNC_COUNT;
    else if (rIdent == "max")
        return PSZ_FUNC_MAX;
    else if (rIdent == "min")
        return PSZ_FUNC_MIN;
    else if (rIdent == "sum")
        return PSZ_FUNC_SUM;
    else if (rIdent == "selection")
        return PSZ_FUNC_SELECTION_COUNT;
    else if (rIdent == "none")
        return PSZ_FUNC_NONE;
    return 0;
}

sal_uInt16 FunctionPopup_Impl::function_to_id(sal_uInt16 nFunc) const
{
    switch (nFunc)
    {
        case PSZ_FUNC_AVG:
            return m_xMenu->GetItemId("avg");
        case PSZ_FUNC_COUNT2:
            return m_xMenu->GetItemId("counta");
        case PSZ_FUNC_COUNT:
            return m_xMenu->GetItemId("count");
        case PSZ_FUNC_MAX:
            return m_xMenu->GetItemId("max");
        case PSZ_FUNC_MIN:
            return m_xMenu->GetItemId("min");
        case PSZ_FUNC_SUM:
            return m_xMenu->GetItemId("sum");
        case PSZ_FUNC_SELECTION_COUNT:
            return m_xMenu->GetItemId("selection");
        case PSZ_FUNC_NONE:
            return m_xMenu->GetItemId("none");
    }
    return 0;
}

FunctionPopup_Impl::FunctionPopup_Impl(sal_uInt32 nCheckEncoded)
    : m_aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/functionmenu.ui", "")
    , m_xMenu(m_aBuilder.get_menu("menu"))
    , m_nSelected(nCheckEncoded)
{
    for ( sal_uInt16 nCheck = 1; nCheck < 32; ++nCheck )
        if ( nCheckEncoded & (1 << nCheck) )
            m_xMenu->CheckItem(function_to_id(nCheck));
}

sal_uInt32 FunctionPopup_Impl::GetSelected() const
{
    sal_uInt32 nSelected = m_nSelected;
    sal_uInt16 nCurItemId = id_to_function(m_xMenu->GetCurItemIdent());
    if ( nCurItemId == PSZ_FUNC_NONE )
        nSelected = ( 1 << PSZ_FUNC_NONE );
    else
    {
        nSelected &= (~( 1 << PSZ_FUNC_NONE )); // Clear the "None" bit
        nSelected ^= ( 1 << nCurItemId ); // Toggle the bit corresponding to nCurItemId
        if ( !nSelected )
            nSelected = ( 1 << PSZ_FUNC_NONE );
    }
    return nSelected;
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
    pImpl->aPosImage = Image(StockImage::Yes, RID_SVXBMP_POSITION);
    pImpl->aSizeImage = Image(StockImage::Yes, RID_SVXBMP_SIZE);

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
            if ( auto pUInt32Item = dynamic_cast< const SfxUInt32Item* >(pState) )
                pImpl->nFunctionSet = pUInt32Item->GetValue();
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
    else if ( auto pPointItem = dynamic_cast<const SfxPointItem*>( pState) )
    {
        // show position
        pImpl->aPos = pPointItem->GetValue();
        pImpl->bPos = true;
        pImpl->bTable = false;
    }
    else if ( auto pSizeItem = dynamic_cast<const SvxSizeItem*>( pState) )
    {
        // show size
        pImpl->aSize = pSizeItem->GetSize();
        pImpl->bSize = true;
        pImpl->bTable = false;
    }
    else if ( auto pStringItem = dynamic_cast<const SfxStringItem*>( pState) )
    {
        // show string (table cel or different)
        pImpl->aStr = pStringItem->GetValue();
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
        FunctionPopup_Impl aMenu(nSelect);
        if (aMenu.Execute(&GetStatusBar(), rCEvt.GetMousePosPixel()))
        {
            nSelect = aMenu.GetSelected();
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

    const tools::Rectangle& rRect = rUsrEvt.GetRect();
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
        aPnt.setY( aItemPos.Y() );
        aPnt.AdjustX(PAINT_OFFSET );
        pDev->DrawImage( aPnt, pImpl->aPosImage );
        aPnt.AdjustX(pImpl->aPosImage.GetSizePixel().Width() );
        aPnt.AdjustX(PAINT_OFFSET );
        OUString aStr = GetMetricStr_Impl( pImpl->aPos.X());
        aStr += " / ";
        aStr += GetMetricStr_Impl( pImpl->aPos.Y());
        tools::Rectangle aRect(aPnt, Point(nSizePosX, rRect.Bottom()));
        pDev->DrawRect(aRect);
        vcl::Region aOrigRegion(pDev->GetClipRegion());
        pDev->SetClipRegion(vcl::Region(aRect));
        pDev->DrawText(aPnt, aStr);
        pDev->SetClipRegion(aOrigRegion);

        // draw the size, when available
        aPnt.setX( nSizePosX );

        if ( pImpl->bSize )
        {
            pDev->DrawImage( aPnt, pImpl->aSizeImage );
            aPnt.AdjustX(pImpl->aSizeImage.GetSizePixel().Width() );
            Point aDrwPnt = aPnt;
            aPnt.AdjustX(PAINT_OFFSET );
            aStr = GetMetricStr_Impl( pImpl->aSize.Width() );
            aStr += " x ";
            aStr += GetMetricStr_Impl( pImpl->aSize.Height() );
            aRect = tools::Rectangle(aDrwPnt, rRect.BottomRight());
            pDev->DrawRect(aRect);
            aOrigRegion = pDev->GetClipRegion();
            pDev->SetClipRegion(vcl::Region(aRect));
            pDev->DrawText(aPnt, aStr);
            pDev->SetClipRegion(aOrigRegion);
        }
        else
            pDev->DrawRect( tools::Rectangle( aPnt, rRect.BottomRight() ) );
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
