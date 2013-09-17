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

#include <tools/shl.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>
#include <tools/urlobj.hxx>

#include <vcl/svapp.hxx>

#include <svx/dialogs.hrc>

#define TMP_STR_BEGIN   '['
#define TMP_STR_END     ']'

#define DELAY_TIMEOUT           100

#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/itemwin.hxx>
#include "svx/linectrl.hxx"
#include <svtools/colorcfg.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

#define LOGICAL_EDIT_HEIGHT         12
//========================================================================
// SvxLineBox
//========================================================================

SvxLineBox::SvxLineBox( Window* pParent, const Reference< XFrame >& rFrame, WinBits nBits ) :
    LineLB( pParent, nBits ),
    nCurPos     ( 0 ),
    aLogicalSize(40,140),
    bRelease    ( sal_True ),
    mpSh        ( NULL ),
    mxFrame     ( rFrame )
{
    SetSizePixel( LogicToPixel( aLogicalSize, MAP_APPFONT ));
    Show();

    aDelayTimer.SetTimeout( DELAY_TIMEOUT );
    aDelayTimer.SetTimeoutHdl( LINK( this, SvxLineBox, DelayHdl_Impl ) );
    aDelayTimer.Start();
}

// -----------------------------------------------------------------------

SvxLineBox::~SvxLineBox()
{
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineBox, DelayHdl_Impl)
{
    if ( GetEntryCount() == 0 )
    {
        mpSh = SfxObjectShell::Current();
        FillControl();
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvxLineBox::Select()
{
    // Call the parent's Select() member to trigger accessibility events.
    LineLB::Select();

    if ( !IsTravelSelect() )
    {
        XLineStyle eXLS;
        sal_uInt16 nPos = GetSelectEntryPos();

        switch ( nPos )
        {
            case 0:
                eXLS = XLINE_NONE;
                break;

            case 1:
                eXLS = XLINE_SOLID;
                break;

            default:
            {
                eXLS = XLINE_DASH;

                if ( nPos != LISTBOX_ENTRY_NOTFOUND &&
                     SfxObjectShell::Current()  &&
                     SfxObjectShell::Current()->GetItem( SID_DASH_LIST ) )
                {
                    // LineDashItem will only be sent if it also has a dash.
                    // Notify cares!
                    SvxDashListItem aItem( *(const SvxDashListItem*)(
                        SfxObjectShell::Current()->GetItem( SID_DASH_LIST ) ) );
                    XLineDashItem aLineDashItem( GetSelectEntry(),
                        aItem.GetDashList()->GetDash( nPos - 2 )->GetDash() );

                    Any a;
                    Sequence< PropertyValue > aArgs( 1 );
                    aArgs[0].Name = OUString( "LineDash" );
                    aLineDashItem.QueryValue ( a );
                    aArgs[0].Value = a;
                    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                                 OUString( ".uno:LineDash" ),
                                                 aArgs );
                }
            }
            break;
        }

        XLineStyleItem aLineStyleItem( eXLS );
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString( "XLineStyle" );
        aLineStyleItem.QueryValue ( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                     OUString( ".uno:XLineStyle" ),
                                     aArgs );

        nCurPos = GetSelectEntryPos();
        ReleaseFocus_Impl();
    }
}

// -----------------------------------------------------------------------

long SvxLineBox::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    switch(nType)
    {
        case EVENT_MOUSEBUTTONDOWN:
        case EVENT_GETFOCUS:
            nCurPos = GetSelectEntryPos();
        break;
        case EVENT_LOSEFOCUS:
            SelectEntryPos(nCurPos);
        break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            if( pKEvt->GetKeyCode().GetCode() == KEY_TAB)
            {
                bRelease = sal_False;
                Select();
            }
        }
        break;
    }
    return LineLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxLineBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = LineLB::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                Select();
                nHandled = 1;
                break;

            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    return nHandled;
}

// -----------------------------------------------------------------------

void SvxLineBox::ReleaseFocus_Impl()
{
    if(!bRelease)
    {
        bRelease = sal_True;
        return;
    }

    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

void SvxLineBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    LineLB::DataChanged( rDCEvt );
}

void SvxLineBox::FillControl()
{
    // FillStyles();
    if ( !mpSh )
        mpSh = SfxObjectShell::Current();

    if( mpSh )
    {
        const SvxDashListItem* pItem = (const SvxDashListItem*)( mpSh->GetItem( SID_DASH_LIST ) );
        if ( pItem )
            Fill( pItem->GetDashList() );
    }
}

//========================================================================
// SvxColorBox
//========================================================================

SvxColorBox::SvxColorBox(
    Window* pParent,
    const OUString& rCommand,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    ColorLB( pParent, nBits ),
    nCurPos     ( 0 ),
    aLogicalSize(45,80),
    bRelease    ( sal_True ),
    maCommand   ( rCommand ),
    mxFrame     ( rFrame )
{
    SetSizePixel( LogicToPixel( aLogicalSize , MAP_APPFONT));
    Show();

    SfxObjectShell* pSh = SfxObjectShell::Current();

    if ( pSh )
    {
        const SvxColorListItem* pItem =
            (const SvxColorListItem*)( pSh->GetItem( SID_COLOR_TABLE ) );
        if(pItem)
            Fill( pItem->GetColorList() );
    }
}

// -----------------------------------------------------------------------

SvxColorBox::~SvxColorBox()
{
}

// -----------------------------------------------------------------------

void SvxColorBox::Update( const XLineColorItem* pItem )
{
    if ( pItem )
    {
      // fdo#64455
        ::Color aColor = pItem->GetColorValue();
        OUString aString( pItem->GetName() );
        SelectEntry(aString);
        if( GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND ||
            GetSelectEntryColor() != aColor )
        {
            SelectEntry( aColor );
        }
        // Check if the entry is not in the list
        if( GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND ||
            GetSelectEntryColor() != aColor )
        {
            sal_uInt16 nCount = GetEntryCount();
            OUString aTmpStr;
            if( nCount > 0 )
            {
                // Last entry gets tested against temporary color
                aTmpStr = GetEntry( nCount - 1 );
                if(  aTmpStr[0] == TMP_STR_BEGIN &&
                     aTmpStr[aTmpStr.getLength()-1] == TMP_STR_END )
                {
                    RemoveEntry( nCount - 1 );
                }
            }
            aTmpStr = OUString(TMP_STR_BEGIN) + aString + OUString(TMP_STR_END);

            sal_uInt16 nPos = InsertEntry( aColor, aTmpStr );
            SelectEntryPos( nPos );
        }
    }
    else
        SetNoSelection();
}

// -----------------------------------------------------------------------

void SvxColorBox::Select()
{
    // OJ: base class call needed here because otherwise no event is send for accessibility
    ColorLB::Select();
    if ( !IsTravelSelect() )
    {
        XLineColorItem aLineColorItem( GetSelectEntry(), GetSelectEntryColor() );

        INetURLObject aObj( maCommand );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aLineColorItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                     maCommand,
                                     aArgs );

        nCurPos = GetSelectEntryPos();
        ReleaseFocus_Impl();
    }
}

// -----------------------------------------------------------------------

long SvxColorBox::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    switch(nType)
    {
        case  EVENT_MOUSEBUTTONDOWN:
        case EVENT_GETFOCUS:
            nCurPos = GetSelectEntryPos();
        break;
        case EVENT_LOSEFOCUS:
            SelectEntryPos(nCurPos);
        break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

            if( pKEvt->GetKeyCode().GetCode() == KEY_TAB)
            {
                bRelease = sal_False;
                Select();
            }
        }
    }

    return ColorLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxColorBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = ColorLB::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                Select();
                nHandled = 1;
                break;

            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    return nHandled;
}

void SvxColorBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    ColorLB::DataChanged( rDCEvt );
}
// -----------------------------------------------------------------------

void SvxColorBox::ReleaseFocus_Impl()
{
    if(!bRelease)
    {
        bRelease = sal_True;
        return;
    }

    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

//========================================================================
// SvxMetricField
//========================================================================

SvxMetricField::SvxMetricField(
    Window* pParent, const Reference< XFrame >& rFrame, WinBits nBits ) :
    MetricField( pParent, nBits ),
    aCurTxt(),
    mxFrame( rFrame )
{
    Size aSize = Size(GetTextWidth( OUString("99,99mm") ),GetTextHeight());
    aSize.Width() += 20;
    aSize.Height() += 6;
    SetSizePixel( aSize );
    aLogicalSize = PixelToLogic(aSize, MAP_APPFONT);
    SetUnit( FUNIT_MM );
    SetDecimalDigits( 2 );
    SetMax( 5000 );
    SetMin( 0 );
    SetLast( 5000 );
    SetFirst( 0 );

    eDlgUnit = SfxModule::GetModuleFieldUnit( mxFrame );
    SetFieldUnit( *this, eDlgUnit, sal_False );
    Show();
}

// -----------------------------------------------------------------------

SvxMetricField::~SvxMetricField()
{
}

// -----------------------------------------------------------------------

void SvxMetricField::Update( const XLineWidthItem* pItem )
{
    if ( pItem )
    {
        if ( pItem->GetValue() != GetCoreValue( *this, ePoolUnit ) )
            SetMetricValue( *this, pItem->GetValue(), ePoolUnit );
    }
    else
        SetText( "" );
}

// -----------------------------------------------------------------------

void SvxMetricField::Modify()
{
    MetricField::Modify();
    long nTmp = GetCoreValue( *this, ePoolUnit );
    XLineWidthItem aLineWidthItem( nTmp );

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString( "LineWidth" );
    aLineWidthItem.QueryValue( a );
    aArgs[0].Value = a;
    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 OUString( ".uno:LineWidth" ),
                                 aArgs );
}

// -----------------------------------------------------------------------

void SvxMetricField::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();
        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// -----------------------------------------------------------------------

void SvxMetricField::Down()
{
    sal_Int64 nValue = GetValue();
    nValue -= GetSpinSize();

    // To prevent OS/2 to jump on Max
    if ( nValue >= GetMin() )
        MetricField::Down();
}

// -----------------------------------------------------------------------

void SvxMetricField::Up()
{
    MetricField::Up();
}

// -----------------------------------------------------------------------

void SvxMetricField::SetCoreUnit( SfxMapUnit eUnit )
{
    ePoolUnit = eUnit;
}

// -----------------------------------------------------------------------

void SvxMetricField::RefreshDlgUnit()
{
    FieldUnit eTmpUnit = SfxModule::GetModuleFieldUnit( mxFrame );
    if ( eDlgUnit != eTmpUnit )
    {
        eDlgUnit = eTmpUnit;
        SetFieldUnit( *this, eDlgUnit, sal_False );
    }
}

// -----------------------------------------------------------------------

long SvxMetricField::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        aCurTxt = GetText();

    return MetricField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxMetricField::Notify( NotifyEvent& rNEvt )
{
    long nHandled = MetricField::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode& rKey = pKEvt->GetKeyCode();
        SfxViewShell* pSh = SfxViewShell::Current();

        if ( rKey.GetModifier() && rKey.GetGroup() != KEYGROUP_CURSOR && pSh )
            pSh->KeyInput( *pKEvt );
        else
        {
            bool bHandled = false;

            switch ( rKey.GetCode() )
            {
                case KEY_RETURN:
                    Reformat();
                    bHandled = true;
                    break;

                case KEY_ESCAPE:
                    SetText( aCurTxt );
                    bHandled = true;
                    break;
            }

            if ( bHandled )
            {
                nHandled = 1;
                Modify();
                ReleaseFocus_Impl();
            }
        }
    }
    return nHandled;
}

void SvxMetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
    }

    MetricField::DataChanged( rDCEvt );
}

//========================================================================
// SvxFillTypeBox
//========================================================================

SvxFillTypeBox::SvxFillTypeBox( Window* pParent, WinBits nBits ) :
    FillTypeLB( pParent, nBits | WB_TABSTOP ),
    nCurPos ( 0 ),
    bSelect ( sal_False ),
    bRelease(sal_True)
{
    SetSizePixel( LogicToPixel( Size(40, 40 ),MAP_APPFONT ));
    Fill();
    SelectEntryPos( XFILL_SOLID );
    Show();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxFillTypeBox(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxFillTypeBox(pParent);
}

// -----------------------------------------------------------------------

SvxFillTypeBox::~SvxFillTypeBox()
{
}

// -----------------------------------------------------------------------

long SvxFillTypeBox::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        nCurPos = GetSelectEntryPos();
    else if ( EVENT_LOSEFOCUS == nType
        && Application::GetFocusWindow()
        && !IsWindowOrChild( Application::GetFocusWindow(), sal_True ) )
    {
        if ( !bSelect )
            SelectEntryPos( nCurPos );
        else
            bSelect = sal_False;
    }

    return FillTypeLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFillTypeBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = FillTypeLB::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                nHandled = 1;
                ( (Link&)GetSelectHdl() ).Call( this );
            break;
            case KEY_TAB:
                bRelease = sal_False;
                ( (Link&)GetSelectHdl() ).Call( this );
                bRelease = sal_True;
                break;

            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    return nHandled;
}

// -----------------------------------------------------------------------

void SvxFillTypeBox::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

//========================================================================
// SvxFillAttrBox
//========================================================================

SvxFillAttrBox::SvxFillAttrBox( Window* pParent, WinBits nBits ) :

    FillAttrLB( pParent, nBits | WB_TABSTOP ),

    nCurPos( 0 ),
    bRelease( sal_True )

{
    SetPosPixel( Point( 90, 0 ) );
    SetSizePixel( LogicToPixel( Size(50, 80 ), MAP_APPFONT ));
    Show();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxFillAttrBox(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxFillAttrBox(pParent);
}

// -----------------------------------------------------------------------

SvxFillAttrBox::~SvxFillAttrBox()
{
}

// -----------------------------------------------------------------------

long SvxFillAttrBox::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        nCurPos = GetSelectEntryPos();

    return FillAttrLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFillAttrBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = FillAttrLB::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                ( (Link&)GetSelectHdl() ).Call( this );
                nHandled = 1;
            break;
            case KEY_TAB:
                bRelease = sal_False;
                GetSelectHdl().Call( this );
                bRelease = sal_True;
            break;
            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    return nHandled;
}

// -----------------------------------------------------------------------

void SvxFillAttrBox::Select()
{
    FillAttrLB::Select();
}

// -----------------------------------------------------------------------

void SvxFillAttrBox::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
