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
#include "precompiled_extensions.hxx"

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include "bibconfig.hxx"


#include "datman.hxx"
#include "bibcont.hxx"


BibShortCutHandler::~BibShortCutHandler()
{
}

sal_Bool BibShortCutHandler::HandleShortCutKey( const KeyEvent& )
{
    return sal_False;
}


BibWindow::BibWindow( Window* pParent, WinBits nStyle ) : Window( pParent, nStyle ), BibShortCutHandler( this )
{
}

BibWindow::~BibWindow()
{
}


BibSplitWindow::BibSplitWindow( Window* pParent, WinBits nStyle ) : SplitWindow( pParent, nStyle ), BibShortCutHandler( this )
{
}

BibSplitWindow::~BibSplitWindow()
{
}


BibTabPage::BibTabPage( Window* pParent, const ResId& rResId ) : TabPage( pParent, rResId ), BibShortCutHandler( this )
{
}

BibTabPage::~BibTabPage()
{
}


using namespace osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define PROPERTY_FRAME                      1
//split window size is a percent value
#define WIN_MIN_HEIGHT 10
#define WIN_STEP_SIZE 5

BibWindowContainer::BibWindowContainer( Window* pParent, BibShortCutHandler* pChildWin, WinBits nStyle ) :
        BibWindow( pParent, nStyle ),
        pChild( pChildWin )
{
    if(pChild!=NULL)
    {
        Window* pChildWindow = GetChild();
        pChildWindow->SetParent(this);
        pChildWindow->Show();
        pChildWindow->SetPosPixel(Point(0,0));
    }
}

BibWindowContainer::~BibWindowContainer()
{
    if( pChild )
    {
        Window* pDel = GetChild();
        pChild = NULL;          // prevents GetFocus for child while deleting!
        delete pDel;
    }
}

void BibWindowContainer::Resize()
{
    if( pChild )
        GetChild()->SetSizePixel( GetOutputSizePixel() );
}

void BibWindowContainer::GetFocus()
{
    if( pChild )
        GetChild()->GrabFocus();
}

sal_Bool BibWindowContainer::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    return pChild? pChild->HandleShortCutKey( rKeyEvent ) : sal_False;
}


BibBookContainer::BibBookContainer(Window* pParent,BibDataManager* pDtMn, WinBits nStyle):
    BibSplitWindow(pParent,nStyle),
    pDatMan(pDtMn),
    pTopWin(NULL),
    pBottomWin(NULL),
    bFirstTime(sal_True)
{
    pBibMod = OpenBibModul();
    aTimer.SetTimeoutHdl(LINK( this, BibBookContainer, SplitHdl));
    aTimer.SetTimeout(400);
}

BibBookContainer::~BibBookContainer()
{
    if( xTopFrameRef.is() )
        xTopFrameRef->dispose();
    if( xBottomFrameRef.is() )
        xBottomFrameRef->dispose();

    if( pTopWin )
    {
        Window* pDel = pTopWin;
        pTopWin = NULL;         // prevents GetFocus for child while deleting!
        delete pDel;
    }

    if( pBottomWin )
    {
        Window* pDel = pBottomWin;
        pBottomWin = NULL;      // prevents GetFocus for child while deleting!
        delete pDel;
    }

    CloseBibModul( pBibMod );
}

void BibBookContainer::Split()
{
    aTimer.Start();
}
IMPL_LINK( BibBookContainer, SplitHdl, Timer*,/*pT*/)
{
    long nSize= GetItemSize( TOP_WINDOW);
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setBeamerSize(nSize);
    nSize = GetItemSize( BOTTOM_WINDOW);
    pConfig->setViewSize(nSize);
    return 0;
}

void BibBookContainer::createTopFrame( BibShortCutHandler* pWin )
{
    if ( xTopFrameRef.is() ) xTopFrameRef->dispose();

    if(pTopWin)
    {
        RemoveItem(TOP_WINDOW);
        delete pTopWin;
    }
    pTopWin=new BibWindowContainer(this,pWin);
    pTopWin->Show();
    BibConfig* pConfig = BibModul::GetConfig();
    long nSize = pConfig->getBeamerSize();
    InsertItem(TOP_WINDOW, pTopWin, nSize, 1, 0, SWIB_PERCENTSIZE  );

}

void BibBookContainer::createBottomFrame( BibShortCutHandler* pWin )
{
    if ( xBottomFrameRef.is() ) xBottomFrameRef->dispose();

    if(pBottomWin)
    {
        RemoveItem(BOTTOM_WINDOW);
        delete pBottomWin;
    }

    pBottomWin=new BibWindowContainer(this,pWin);

    BibConfig* pConfig = BibModul::GetConfig();
    long nSize = pConfig->getViewSize();
    InsertItem(BOTTOM_WINDOW, pBottomWin, nSize, 1, 0, SWIB_PERCENTSIZE  );

}

void BibBookContainer::GetFocus()
{
    if( pBottomWin )
        pBottomWin->GrabFocus();
}

long BibBookContainer::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if( EVENT_KEYINPUT == rNEvt.GetType()  )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();

        if( KEY_MOD2 == nModifier )
        {
            if( KEY_UP == nKey || KEY_DOWN == nKey )
            {
                if(pTopWin && pBottomWin)
                {
                    sal_uInt16 nFirstWinId = KEY_UP == nKey ? TOP_WINDOW : BOTTOM_WINDOW;
                    sal_uInt16 nSecondWinId = KEY_UP == nKey ? BOTTOM_WINDOW : TOP_WINDOW;
                    long nHeight = GetItemSize( nFirstWinId );
                    nHeight -= WIN_STEP_SIZE;
                    if(nHeight < WIN_MIN_HEIGHT)
                        nHeight = WIN_MIN_HEIGHT;
                    SetItemSize( nFirstWinId, nHeight );
                    SetItemSize( nSecondWinId, 100 - nHeight );
                }
                nHandled = 1;
            }
            else if( pKEvt->GetCharCode() && HandleShortCutKey( *pKEvt ) )
                nHandled = 1;
        }
    }

    return nHandled;
}

sal_Bool BibBookContainer::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    sal_Bool bRet = sal_False;

    if( pTopWin )
        bRet = pTopWin->HandleShortCutKey( rKeyEvent );

    if( !bRet && pBottomWin )
        bRet = pBottomWin->HandleShortCutKey( rKeyEvent );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
