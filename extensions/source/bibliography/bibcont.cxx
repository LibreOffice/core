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


#include <cppuhelper/weak.hxx>
#include <vcl/event.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include "bibconfig.hxx"


#include "datman.hxx"
#include "bibcont.hxx"
#include "bibview.hxx"


BibShortCutHandler::~BibShortCutHandler()
{
}

bool BibShortCutHandler::HandleShortCutKey( const KeyEvent& )
{
    return false;
}


BibWindow::BibWindow( vcl::Window* pParent, WinBits nStyle ) : Window( pParent, nStyle ), BibShortCutHandler( this )
{
}

BibWindow::~BibWindow()
{
}


BibSplitWindow::BibSplitWindow( vcl::Window* pParent, WinBits nStyle ) : SplitWindow( pParent, nStyle ), BibShortCutHandler( this )
{
}

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//split window size is a percent value
#define WIN_MIN_HEIGHT 10
#define WIN_STEP_SIZE 5

BibWindowContainer::BibWindowContainer( vcl::Window* pParent, BibShortCutHandler* pChildWin ) :
        BibWindow( pParent, WB_3DLOOK ),
        pChild( pChildWin )
{
    if(pChild!=nullptr)
    {
        vcl::Window* pChildWindow = GetChild();
        pChildWindow->SetParent(this);
        pChildWindow->Show();
        pChildWindow->SetPosPixel(Point(0,0));
    }
}

BibWindowContainer::~BibWindowContainer()
{
    disposeOnce();
}

void BibWindowContainer::dispose()
{
    if( pChild )
    {
        VclPtr<vcl::Window> pDel = GetChild();
        pChild = nullptr;          // prevents GetFocus for child while deleting!
        pDel.disposeAndClear();
    }
    vcl::Window::dispose();
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

bool BibWindowContainer::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    return pChild && pChild->HandleShortCutKey( rKeyEvent );
}


BibBookContainer::BibBookContainer(vcl::Window* pParent):
    BibSplitWindow(pParent,WB_3DLOOK),
    pTopWin(nullptr),
    pBottomWin(nullptr),
    aIdle("extensions BibBookContainer Split Idle")
{
    pBibMod = OpenBibModul();
    aIdle.SetInvokeHandler(LINK( this, BibBookContainer, SplitHdl));
    aIdle.SetPriority(TaskPriority::LOWEST);
}

BibBookContainer::~BibBookContainer()
{
    disposeOnce();
}

void BibBookContainer::dispose()
{
    if( pTopWin )
    {
        VclPtr<vcl::Window> pDel = pTopWin;
        pTopWin = nullptr;         // prevents GetFocus for child while deleting!
        pDel.disposeAndClear();
    }

    if( pBottomWin )
    {
        VclPtr<vcl::Window> pDel = pBottomWin;
        pBottomWin = nullptr;      // prevents GetFocus for child while deleting!
        pDel.disposeAndClear();
    }

    CloseBibModul( pBibMod );
    pTopWin.clear();
    pBottomWin.clear();
    BibSplitWindow::dispose();
}

void BibBookContainer::Split()
{
    aIdle.Start();
}
IMPL_LINK_NOARG( BibBookContainer, SplitHdl, Timer*, void)
{
    long nSize= GetItemSize( TOP_WINDOW);
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setBeamerSize(nSize);
    nSize = GetItemSize( BOTTOM_WINDOW);
    pConfig->setViewSize(nSize);
}

void BibBookContainer::createTopFrame( BibShortCutHandler* pWin )
{
    if(pTopWin)
    {
        RemoveItem(TOP_WINDOW);
        pTopWin.disposeAndClear();
    }
    pTopWin=VclPtr<BibWindowContainer>::Create(this,pWin);
    pTopWin->Show();
    BibConfig* pConfig = BibModul::GetConfig();
    long nSize = pConfig->getBeamerSize();
    InsertItem(TOP_WINDOW, pTopWin, nSize, 1, 0, SplitWindowItemFlags::PercentSize  );

}

void BibBookContainer::createBottomFrame( BibShortCutHandler* pWin )
{
    if(pBottomWin)
    {
        RemoveItem(BOTTOM_WINDOW);
        pBottomWin.disposeAndClear();
    }

    pBottomWin=VclPtr<BibWindowContainer>::Create(this,pWin);

    BibConfig* pConfig = BibModul::GetConfig();
    long nSize = pConfig->getViewSize();
    InsertItem(BOTTOM_WINDOW, pBottomWin, nSize, 1, 0, SplitWindowItemFlags::PercentSize  );

}

void BibBookContainer::GetFocus()
{
    if( pBottomWin )
        pBottomWin->GrabFocus();
}

bool BibBookContainer::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if( MouseNotifyEvent::KEYINPUT == rNEvt.GetType()  )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
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
                bHandled = true;
            }
            else if( pKEvt->GetCharCode() && HandleShortCutKey( *pKEvt ) )
                bHandled = true;
        }
    }

    return bHandled;
}

bool BibBookContainer::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    bool bRet = false;

    if( pTopWin )
        bRet = pTopWin->HandleShortCutKey( rKeyEvent );

    if( !bRet && pBottomWin )
        bRet = pBottomWin->HandleShortCutKey( rKeyEvent );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
