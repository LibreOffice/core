/*************************************************************************
 *
 *  $RCSfile: taskstat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _TASKBAR_CXX

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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

DECLARE_LIST( ImplTaskSBItemList, ImplTaskSBFldItem* );

// =======================================================================

BOOL ITaskStatusNotify::MouseButtonDown( USHORT nItemd, const MouseEvent& rMEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::MouseButtonUp( USHORT nItemd, const MouseEvent& rMEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::MouseMove( USHORT nItemd, const MouseEvent& rMEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::Command( USHORT nItemd, const CommandEvent& rCEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::QueryDrop( USHORT nItemd, DropEvent& rDEvt, BOOL& bRet )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::Drop( USHORT nItemd, const DropEvent& rDEvt, BOOL& bRet )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ITaskStatusNotify::UpdateHelp( USHORT nItemd )
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

ImplTaskSBFldItem* TaskStatusBar::ImplGetFieldItem( const Point& rPos, BOOL& rFieldRect ) const
{
    if ( GetItemId( rPos ) == TASKSTATUSBAR_STATUSFIELDID )
    {
        rFieldRect = TRUE;

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
            maTimeText = maIntn.GetTime( aTime, FALSE, FALSE );
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
        ImplTaskSBFldItem* pItem = mpFieldItemList->First();
        mnItemWidth = 0;
        mbFlashItems = FALSE;
        mbOutInterval = FALSE;
        while ( pItem )
        {
            mnItemWidth += TASKSTATUSBAR_IMAGEOFFX;
            pItem->mnOffX = mnItemWidth;
            mnItemWidth += pItem->maItem.GetImage().GetSizePixel().Width();
            if ( pItem->maItem.GetFlags() & TASKSTATUSFIELDITEM_FLASH )
                mbFlashItems = TRUE;

            pItem = mpFieldItemList->Next();
        }
    }
    else
    {
        if ( mnFieldFlags & TASKSTATUSFIELD_CLOCK )
        {
            XubString aStr = maIntn.GetTime( Time( 23, 59, 59 ), FALSE, FALSE );
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

BOOL TaskStatusBar::QueryDrop( DropEvent& rDEvt )
{
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    BOOL bRet = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rDEvt.GetPosPixel(), bFieldRect );

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
        bBaseClass = pNotify->QueryDrop( nItemId, rDEvt, bRet );

    if ( bBaseClass )
    {
        StatusBar::QueryDrop( rDEvt );
        return TRUE;
    }
    else
        return bRet;
}

// -----------------------------------------------------------------------

BOOL TaskStatusBar::Drop( const DropEvent& rDEvt )
{
    BOOL bFieldRect;
    BOOL bBaseClass = FALSE;
    BOOL bRet = FALSE;
    ImplTaskSBFldItem* pItem = ImplGetFieldItem( rDEvt.GetPosPixel(), bFieldRect );

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
        bBaseClass = pNotify->Drop( nItemId, rDEvt, bRet );

    if ( bBaseClass )
        return StatusBar::Drop( rDEvt );
    else
        return bRet;
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
                XubString aStr = maIntn.GetLongDate( Date() );
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
                        pHelp->Start( nHelpId );
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

void TaskStatusBar::InsertStatusField( long nOffset, USHORT nPos,
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
    mpFieldItemList->Insert( pItem, (ULONG)nPos );

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
        mpFieldItemList->Remove( pItem );
        delete pItem;
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

