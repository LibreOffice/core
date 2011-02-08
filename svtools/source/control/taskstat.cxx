/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <taskbar.hxx>

// =======================================================================

#define TASKSTATUSBAR_CLOCXOFFX     3
#define TASKSTATUSBAR_IMAGEOFFX     1

// =======================================================================

struct ImplTaskSBFldItem
{
    TaskStatusFieldItem     maItem;
    USHORT                  mnId;
    long                    mnOffX;
};

// =======================================================================

BOOL ITaskStatusNotify::MouseButtonDown( USHORT, const MouseEvent& )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::MouseButtonUp( USHORT, const MouseEvent& )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::MouseMove( USHORT, const MouseEvent& )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::Command( USHORT, const CommandEvent& )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::UpdateHelp( USHORT )
{
    return FALSE;
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
                                          USHORT nFlags ) :
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
    mbFlashItems    = FALSE;
    mbOutInterval   = FALSE;

    maTimer.SetTimeoutHdl( LINK( this, TaskStatusBar, ImplTimerHdl ) );
}

// -----------------------------------------------------------------------

TaskStatusBar::~TaskStatusBar()
{
    if ( mpFieldItemList )
    {
        for ( size_t i = 0, n = mpFieldItemList->size(); i < n; ++i ) {
            delete (*mpFieldItemList)[ i ];
        }
        mpFieldItemList->clear();
        delete mpFieldItemList;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( TaskStatusBar, ImplTimerHdl, Timer*, EMPTYARG )
{
    BOOL bUpdate = ImplUpdateClock();
    if ( ImplUpdateFlashItems() )
        bUpdate = TRUE;
    if ( bUpdate )
        SetItemData( TASKSTATUSBAR_STATUSFIELDID, NULL );

    return 0;
}

// -----------------------------------------------------------------------

ImplTaskSBFldItem* TaskStatusBar::ImplGetFieldItem( USHORT nItemId ) const
{
    if ( !mpFieldItemList )
        return NULL;

    for ( size_t i = 0, n = mpFieldItemList->size(); i < n; ++i ) {
        if ( (*mpFieldItemList)[ i ]->mnId == nItemId ) {
            return  (*mpFieldItemList)[ i ];
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------

ImplTaskSBFldItem* TaskStatusBar::ImplGetFieldItem( const Point& rPos, BOOL& rFieldRect ) const
{
    if ( GetItemId( rPos ) == TASKSTATUSBAR_STATUSFIELDID )
    {
        rFieldRect = TRUE;

        if ( mpFieldItemList )
        {
            long nX = rPos.X()-GetItemRect( TASKSTATUSBAR_STATUSFIELDID ).Left();
            for ( size_t i = 0, n = mpFieldItemList->size(); i < n; ++i ) {
                ImplTaskSBFldItem* pItem = (*mpFieldItemList)[ i ];
                if ( nX < pItem->mnOffX + pItem->maItem.GetImage().GetSizePixel().Width() ) {
                    return pItem;
                }
            }
        }
    }
    else
        rFieldRect = FALSE;

    return NULL;
}

// -----------------------------------------------------------------------

BOOL TaskStatusBar::ImplUpdateClock()
{
    if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
    {
        Time aTime;
        maTimer.SetTimeout( ((long)60000)-((aTime.GetSec()*1000)+(aTime.Get100Sec()*10)) );
        if ( (aTime.GetMin()  != maTime.GetMin()) ||
             (aTime.GetHour() != maTime.GetHour()) )
        {
            maTime = aTime;
            maTimeText = SvtSysLocale().GetLocaleData().getTime( aTime, FALSE, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL TaskStatusBar::ImplUpdateFlashItems()
{
    if ( mbFlashItems )
    {
        if ( mbOutInterval )
        {
            maTimer.SetTimeout( 900 );
            mbOutInterval = FALSE;
        }
        else
        {
            maTimer.SetTimeout( 700 );
            mbOutInterval = TRUE;
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void TaskStatusBar::ImplUpdateField( BOOL bItems )
{
    maTimer.Stop();

    if ( bItems )
    {
        mnItemWidth = 0;
        mbFlashItems = FALSE;
        mbOutInterval = FALSE;
        for ( size_t i = 0, n = mpFieldItemList->size(); i < n; ++i ) {
            ImplTaskSBFldItem* pItem = (*mpFieldItemList)[ i ];
            mnItemWidth += TASKSTATUSBAR_IMAGEOFFX;
            pItem->mnOffX = mnItemWidth;
            mnItemWidth += pItem->maItem.GetImage().GetSizePixel().Width();
            if ( pItem->maItem.GetFlags() & TASKSTATUSFIELDITEM_FLASH ) {
                mbFlashItems = TRUE;
            }
        }
    }
    else
    {
        if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
        {
            XubString aStr = SvtSysLocale().GetLocaleData().getTime( Time( 23, 59, 59 ), FALSE, FALSE );
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
            USHORT  nItemPos = GetItemPos( TASKSTATUSBAR_STATUSFIELDID );
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
        mbOutInterval = TRUE;
        ImplUpdateFlashItems();
        maTimer.Start();
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    USHORT              nItemId = 0;

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
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    USHORT              nItemId = 0;

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
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rMEvt.GetPosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    USHORT              nItemId = 0;

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
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rCEvt.GetMousePosPixel(), bFieldRect );

    ITaskStatusNotify*  pNotify = mpNotify;
    USHORT              nItemId = 0;

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
    BOOL bFieldRect;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ), bFieldRect );
    if ( bFieldRect )
    {
        ITaskStatusNotify*  pNotify = mpNotify;
        USHORT              nItemId = 0;

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
                ULONG nHelpId = pItem->maItem.GetHelpId();
                if ( nHelpId )
                {
                    // Wenn eine Hilfe existiert, dann ausloesen
                    Help* pHelp = Application::GetHelp();
                    if ( pHelp )
                        pHelp->Start( nHelpId, this );
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
            for ( size_t i = 0, n = mpFieldItemList->size(); i < n; ++i )
            {
                ImplTaskSBFldItem* pItem = (*mpFieldItemList)[ i ];
                if ( !mbOutInterval || !(pItem->maItem.GetFlags() & TASKSTATUSFIELDITEM_FLASH) )
                {
                    const Image& rImage = pItem->maItem.GetImage();
                    Size aImgSize = rImage.GetSizePixel();
                    pDev->DrawImage( Point( aRect.Left()+pItem->mnOffX,
                                            aRect.Top()+((aRect.GetHeight()-aImgSize.Width())/2) ),
                                     rImage );
                }
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

void TaskStatusBar::InsertStatusField( long, USHORT,
                                       USHORT nFlags )
{
    mnFieldFlags = nFlags;
    ImplUpdateField( FALSE );
}

// -----------------------------------------------------------------------

void TaskStatusBar::SetFieldFlags( USHORT nFlags )
{
    if ( mnFieldFlags != nFlags )
    {
        mnFieldFlags = nFlags;
        ImplUpdateField( FALSE );
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::AddStatusFieldItem( USHORT nItemId, const TaskStatusFieldItem& rItem,
                                        USHORT nPos )
{
    DBG_ASSERT( nItemId, "TaskStatusBar::AddStatusFieldItem() - Item is 0" );
    DBG_ASSERT( !ImplGetFieldItem( nItemId ), "TaskStatusBar::AddStatusFieldItem() - Item-Id already exist" );

    if ( !mpFieldItemList )
        mpFieldItemList = new ImplTaskSBItemList;

    ImplTaskSBFldItem* pItem = new ImplTaskSBFldItem;
    pItem->maItem   = rItem;
    pItem->mnId     = nItemId;
    pItem->mnOffX   = 0;
    if ( nPos < mpFieldItemList->size() ) {
        ImplTaskSBItemList::iterator it = mpFieldItemList->begin();
        ::std::advance( it, nPos );
        mpFieldItemList->insert( it, pItem );
    } else {
        mpFieldItemList->push_back( pItem );
    }

    ImplUpdateField( TRUE );
}

// -----------------------------------------------------------------------

void TaskStatusBar::ModifyStatusFieldItem( USHORT nItemId, const TaskStatusFieldItem& rItem )
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        BOOL bUpdate = (pItem->maItem.GetImage() != rItem.GetImage()) ||
                       (pItem->maItem.GetFlags() != rItem.GetFlags());
        pItem->maItem = rItem;
        if ( bUpdate )
            ImplUpdateField( TRUE );
    }
}

// -----------------------------------------------------------------------

void TaskStatusBar::RemoveStatusFieldItem( USHORT nItemId )
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        for ( ImplTaskSBItemList::iterator it = mpFieldItemList->begin();
              it < mpFieldItemList->end();
              ++it
        ) {
            if ( *it == pItem ) {
                delete *it;
                mpFieldItemList->erase( it );
                break;
            }
        }
        ImplUpdateField( TRUE );
    }
}

// -----------------------------------------------------------------------

BOOL TaskStatusBar::GetStatusFieldItem( USHORT nItemId, TaskStatusFieldItem& rItem ) const
{
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( nItemId );
    if ( pItem )
    {
        rItem = pItem->maItem;
        return TRUE;
    }

    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
