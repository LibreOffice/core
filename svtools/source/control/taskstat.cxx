/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#define _TASKBAR_CXX

#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <tools/date.hxx>
#include <vcl/image.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svtools/taskbar.hxx>

// =======================================================================

#define TASKSTATUSBAR_CLOCXOFFX     3
#define TASKSTATUSBAR_IMAGEOFFX     1

// =======================================================================

struct ImplTaskSBFldItem
{
    TaskStatusFieldItem     maItem;
    sal_uInt16                  mnId;
    long                    mnOffX;
};

DECLARE_LIST( ImplTaskSBItemList, ImplTaskSBFldItem* )

// =======================================================================

sal_Bool ITaskStatusNotify::MouseButtonDown( sal_uInt16, const MouseEvent& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ITaskStatusNotify::MouseButtonUp( sal_uInt16, const MouseEvent& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ITaskStatusNotify::MouseMove( sal_uInt16, const MouseEvent& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ITaskStatusNotify::Command( sal_uInt16, const CommandEvent& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ITaskStatusNotify::UpdateHelp( sal_uInt16 )
{
    return sal_False;
}

// =======================================================================

TaskStatusFieldItem::TaskStatusFieldItem()
{
    mpNotify        = NULL;
    mnFlags         = 0;
}

// -----------------------------------------------------------------------

TaskStatusFieldItem::TaskStatusFieldItem( const TaskStatusFieldItem& rItem ) :
    mpNotify( rItem.mpNotify ),
    maImage( rItem.maImage ),
    maQuickHelpText( rItem.maQuickHelpText ),
    maHelpText( rItem.maHelpText ),
    mnFlags( rItem.mnFlags )
{
}

// -----------------------------------------------------------------------

TaskStatusFieldItem::TaskStatusFieldItem( ITaskStatusNotify* pNotify,
                                          const Image& rImage,
                                          const XubString& rQuickHelpText,
                                          const XubString& rHelpText,
                                          sal_uInt16 nFlags ) :
    mpNotify( pNotify ),
    maImage( rImage ),
    maQuickHelpText( rQuickHelpText ),
    maHelpText( rHelpText ),
    mnFlags( nFlags )
{
}

// -----------------------------------------------------------------------

TaskStatusFieldItem::~TaskStatusFieldItem()
{
}

// -----------------------------------------------------------------------

const TaskStatusFieldItem& TaskStatusFieldItem::operator=( const TaskStatusFieldItem& rItem )
{
    mpNotify        = rItem.mpNotify;
    maImage         = rItem.maImage;
    maQuickHelpText = rItem.maQuickHelpText;
    maHelpText      = rItem.maHelpText;
    mnFlags         = rItem.mnFlags;
    return *this;
}

// =======================================================================

TaskStatusBar::TaskStatusBar( Window* pParent, WinBits nWinStyle ) :
    StatusBar( pParent, nWinStyle | WB_3DLOOK ),
    maTime( 0, 0, 0 )
{
    mpFieldItemList = NULL;
    mpNotifyTaskBar = NULL;
    mpNotify        = NULL;
    mnClockWidth    = 0;
    mnItemWidth     = 0;
    mnFieldWidth    = 0;
    mnFieldFlags    = 0;
    mbFlashItems    = sal_False;
    mbOutInterval   = sal_False;

    maTimer.SetTimeoutHdl( LINK( this, TaskStatusBar, ImplTimerHdl ) );
}

// -----------------------------------------------------------------------

TaskStatusBar::~TaskStatusBar()
{
    if ( mpFieldItemList )
    {
        ImplTaskSBFldItem* pItem = mpFieldItemList->First();
        while ( pItem )
        {
            delete pItem;
            pItem = mpFieldItemList->Next();
        }

        delete mpFieldItemList;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( TaskStatusBar, ImplTimerHdl, Timer*, EMPTYARG )
{
    sal_Bool bUpdate = ImplUpdateClock();
    if ( ImplUpdateFlashItems() )
        bUpdate = sal_True;
    if ( bUpdate )
        SetItemData( TASKSTATUSBAR_STATUSFIELDID, NULL );

    return 0;
}

// -----------------------------------------------------------------------

ImplTaskSBFldItem* TaskStatusBar::ImplGetFieldItem( sal_uInt16 nItemId ) const
{
    if ( !mpFieldItemList )
        return NULL;

    ImplTaskSBFldItem* pItem = mpFieldItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nItemId )
            return pItem;

        pItem = mpFieldItemList->Next();
    }

    return NULL;
}

// -----------------------------------------------------------------------

ImplTaskSBFldItem* TaskStatusBar::ImplGetFieldItem( const Point& rPos, sal_Bool& rFieldRect ) const
{
    if ( GetItemId( rPos ) == TASKSTATUSBAR_STATUSFIELDID )
    {
        rFieldRect = sal_True;

        if ( mpFieldItemList )
        {
            long nX = rPos.X()-GetItemRect( TASKSTATUSBAR_STATUSFIELDID ).Left();
            ImplTaskSBFldItem* pItem = mpFieldItemList->First();
            while ( pItem )
            {
                if ( nX < pItem->mnOffX+pItem->maItem.GetImage().GetSizePixel().Width() )
                    return pItem;

                pItem = mpFieldItemList->Next();
            }
        }
    }
    else
        rFieldRect = sal_False;

    return NULL;
}

// -----------------------------------------------------------------------

sal_Bool TaskStatusBar::ImplUpdateClock()
{
    if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
    {
        Time aTime;
        maTimer.SetTimeout( ((long)60000)-((aTime.GetSec()*1000)+(aTime.Get100Sec()*10)) );
        if ( (aTime.GetMin()  != maTime.GetMin()) ||
             (aTime.GetHour() != maTime.GetHour()) )
        {
            maTime = aTime;
            maTimeText = SvtSysLocale().GetLocaleData().getTime( aTime, sal_False, sal_False );
            return sal_True;
        }
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool TaskStatusBar::ImplUpdateFlashItems()
{
    if ( mbFlashItems )
    {
        if ( mbOutInterval )
        {
            maTimer.SetTimeout( 900 );
            mbOutInterval = sal_False;
        }
        else
        {
            maTimer.SetTimeout( 700 );
            mbOutInterval = sal_True;
        }

        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void TaskStatusBar::ImplUpdateField( sal_Bool bItems )
{
    maTimer.Stop();

    if ( bItems )
    {
        ImplTaskSBFldItem* pItem = mpFieldItemList->First();
        mnItemWidth = 0;
        mbFlashItems = sal_False;
        mbOutInterval = sal_False;
        while ( pItem )
        {
            mnItemWidth += TASKSTATUSBAR_IMAGEOFFX;
            pItem->mnOffX = mnItemWidth;
            mnItemWidth += pItem->maItem.GetImage().GetSizePixel().Width();
            if ( pItem->maItem.GetFlags() & TASKSTATUSFIELDITEM_FLASH )
                mbFlashItems = sal_True;

            pItem = mpFieldItemList->Next();
        }
    }
    else
    {
        if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
        {
            XubString aStr = SvtSysLocale().GetLocaleData().getTime( Time( 23, 59, 59 ), sal_False, sal_False );
            mnClockWidth = GetTextWidth( aStr )+(TASKSTATUSBAR_CLOCXOFFX*2);
        }
        else
            mnClockWidth = 0;
    }

    long nNewWidth = mnItemWidth+mnClockWidth;
    if ( mnItemWidth && !mnClockWidth )
        nNewWidth += TASKSTATUSBAR_IMAGEOFFX;
    if ( nNewWidth != mnFieldWidth )
    {
        RemoveItem( TASKSTATUSBAR_STATUSFIELDID );

        if ( mnItemWidth || mnClockWidth )
        {
            mnFieldWidth = nNewWidth;
            long    nOffset = GetItemOffset( TASKSTATUSBAR_STATUSFIELDID );
            sal_uInt16  nItemPos = GetItemPos( TASKSTATUSBAR_STATUSFIELDID );
            InsertItem( TASKSTATUSBAR_STATUSFIELDID, nNewWidth, SIB_RIGHT | SIB_IN | SIB_USERDRAW, nOffset, nItemPos );
        }
        else
            mnFieldWidth = 0;

        if ( mpNotifyTaskBar )
            mpNotifyTaskBar->Resize();
    }
    else
        SetItemData( TASKSTATUSBAR_STATUSFIELDID, NULL );

    if ( mbFlashItems || (mnFieldFlags & TASKSTATUSFIELD_CLOCK) )
    {
        ImplUpdateClock();
        mbOutInterval = sal_True;
        ImplUpdateFlashItems();
        maTimer.Start();
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bFieldRect;
    sal_Bool bBaseClass = sal_False;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    sal_uInt16              nItemId = 0;

    if ( bFieldRect )
        nItemId = TASKSTATUSBAR_CLOCKID;

    if ( pItem )
    {
        pNotify = pItem->maItem.GetNotifyObject();
        nItemId = pItem->mnId;
    }

    if ( pNotify )
        bBaseClass = pNotify->MouseButtonDown( nItemId, rMEvt );

    if ( bBaseClass )
        StatusBar::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::MouseButtonUp( const MouseEvent& rMEvt )
{
    sal_Bool bFieldRect;
    sal_Bool bBaseClass = sal_False;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    sal_uInt16              nItemId = 0;

    if ( bFieldRect )
        nItemId = TASKSTATUSBAR_CLOCKID;

    if ( pItem )
    {
        pNotify = pItem->maItem.GetNotifyObject();
        nItemId = pItem->mnId;
    }

    if ( pNotify )
        bBaseClass = pNotify->MouseButtonUp( nItemId, rMEvt );

    if ( bBaseClass )
        StatusBar::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::MouseMove( const MouseEvent& rMEvt )
{
    sal_Bool bFieldRect;
    sal_Bool bBaseClass = sal_False;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    sal_uInt16              nItemId = 0;

    if ( bFieldRect )
        nItemId = TASKSTATUSBAR_CLOCKID;

    if ( pItem )
    {
        pNotify = pItem->maItem.GetNotifyObject();
        nItemId = pItem->mnId;
    }

    if ( pNotify )
        bBaseClass = pNotify->MouseMove( nItemId, rMEvt );

    if ( bBaseClass )
        StatusBar::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::Command( const CommandEvent& rCEvt )
{
    sal_Bool bFieldRect;
    sal_Bool bBaseClass = sal_False;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rCEvt.GetMousePosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    sal_uInt16              nItemId = 0;

    if ( bFieldRect )
        nItemId = TASKSTATUSBAR_CLOCKID;

    if ( pItem )
    {
        pNotify = pItem->maItem.GetNotifyObject();
        nItemId = pItem->mnId;
    }

    if ( pNotify )
        bBaseClass = pNotify->Command( nItemId, rCEvt );

    if ( bBaseClass )
        StatusBar::Command( rCEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::RequestHelp( const HelpEvent& rHEvt )
{
    sal_Bool bFieldRect;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ), bFieldRect );
    if ( bFieldRect )
    {
        ITaskStatusNotify*  pNotify = mpNotify;
        sal_uInt16              nItemId = 0;

        if ( pItem )
        {
            pNotify = pItem->maItem.GetNotifyObject();
            nItemId = pItem->mnId;
        }

        if ( pNotify )
            pNotify->UpdateHelp( nItemId );

        if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
        {
            Rectangle aItemRect = GetItemRect( TASKSTATUSBAR_STATUSFIELDID );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            if ( pItem )
            {
                if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                {
                    XubString aStr = pItem->maItem.GetHelpText();
                    if ( !aStr.Len() )
                        aStr = pItem->maItem.GetQuickHelpText();
                    Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                }
                else
                    Help::ShowQuickHelp( this, aItemRect, pItem->maItem.GetQuickHelpText() );
            }
            else
            {
                SvtSysLocale aSL;
                const LocaleDataWrapper& rLDW = aSL.GetLocaleData();
                CalendarWrapper aCal( rLDW.getServiceFactory());
                aCal.loadDefaultCalendar( rLDW.getLoadedLocale());
                XubString aStr = rLDW.getLongDate( Date(), aCal );
                if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                    Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                else
                    Help::ShowQuickHelp( this, aItemRect, aStr );
            }
            return;
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            if ( pItem )
            {
                rtl::OUString aHelpId( rtl::OStringToOUString( pItem->maItem.GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
                if ( aHelpId.getLength() )
                {
                    // Wenn eine Hilfe existiert, dann ausloesen
                    Help* pHelp = Application::GetHelp();
                    if ( pHelp )
                        pHelp->Start( aHelpId, this );
                    return;
                }
            }
        }
    }

    StatusBar::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::UserDraw( const UserDrawEvent& rUDEvt )
{
    if ( rUDEvt.GetItemId() == TASKSTATUSBAR_STATUSFIELDID )
    {
        OutputDevice*   pDev = rUDEvt.GetDevice();
        Rectangle       aRect = rUDEvt.GetRect();

        if ( mpFieldItemList )
        {
            ImplTaskSBFldItem* pItem = mpFieldItemList->First();
            while ( pItem )
            {
                if ( !mbOutInterval || !(pItem->maItem.GetFlags() & TASKSTATUSFIELDITEM_FLASH) )
                {
                    const Image& rImage = pItem->maItem.GetImage();
                    Size aImgSize = rImage.GetSizePixel();
                    pDev->DrawImage( Point( aRect.Left()+pItem->mnOffX,
                                            aRect.Top()+((aRect.GetHeight()-aImgSize.Width())/2) ),
                                     rImage );
                }

                pItem = mpFieldItemList->Next();
            }
        }

        if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
        {
            long nX = mnItemWidth+TASKSTATUSBAR_CLOCXOFFX;
            Point aPos = GetItemTextPos( TASKSTATUSBAR_STATUSFIELDID );
            aPos.X() = aRect.Left()+nX;
            pDev->DrawText( aPos, maTimeText );
        }
    }
    else
        StatusBar::UserDraw( rUDEvt );
}

// -----------------------------------------------------------------------

void TaskStatusBar::InsertStatusField( long, sal_uInt16,
                                       sal_uInt16 nFlags )
{
    mnFieldFlags = nFlags;
    ImplUpdateField( sal_False );
}

// -----------------------------------------------------------------------

void TaskStatusBar::SetFieldFlags( sal_uInt16 nFlags )
{
    if ( mnFieldFlags != nFlags )
    {
        mnFieldFlags = nFlags;
        ImplUpdateField( sal_False );
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::AddStatusFieldItem( sal_uInt16 nItemId, const TaskStatusFieldItem& rItem,
                                        sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "TaskStatusBar::AddStatusFieldItem() - Item is 0" );
    DBG_ASSERT( !ImplGetFieldItem( nItemId ), "TaskStatusBar::AddStatusFieldItem() - Item-Id already exist" );

    if ( !mpFieldItemList )
        mpFieldItemList = new ImplTaskSBItemList;

    ImplTaskSBFldItem* pItem = new ImplTaskSBFldItem;
    pItem->maItem   = rItem;
    pItem->mnId     = nItemId;
    pItem->mnOffX   = 0;
    mpFieldItemList->Insert( pItem, (sal_uLong)nPos );

    ImplUpdateField( sal_True );
}

// -----------------------------------------------------------------------

void TaskStatusBar::ModifyStatusFieldItem( sal_uInt16 nItemId, const TaskStatusFieldItem& rItem )
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        sal_Bool bUpdate = (pItem->maItem.GetImage() != rItem.GetImage()) ||
                       (pItem->maItem.GetFlags() != rItem.GetFlags());
        pItem->maItem = rItem;
        if ( bUpdate )
            ImplUpdateField( sal_True );
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::RemoveStatusFieldItem( sal_uInt16 nItemId )
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        mpFieldItemList->Remove( pItem );
        delete pItem;
        ImplUpdateField( sal_True );
    }
}

// -----------------------------------------------------------------------

sal_Bool TaskStatusBar::GetStatusFieldItem( sal_uInt16 nItemId, TaskStatusFieldItem& rItem ) const
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        rItem = pItem->maItem;
        return sal_True;
    }

    return sal_False;
}

