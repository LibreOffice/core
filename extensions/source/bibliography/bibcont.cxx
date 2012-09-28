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


#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
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


BibBookContainer::BibBookContainer(Window* pParent, WinBits nStyle):
    BibSplitWindow(pParent,nStyle),
    pTopWin(NULL),
    pBottomWin(NULL)
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
