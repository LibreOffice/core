/*************************************************************************
 *
 *  $RCSfile: bibcont.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:02:59 $
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

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/weak.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#include <toolkit/helper/vclunohelper.hxx>
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _BIBCONFIG_HXX
#include "bibconfig.hxx"
#endif


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

BibWindowContainer::BibWindowContainer( Window* pParent, WinBits nStyle ) :
        BibWindow( pParent, nStyle ),
        pChild( NULL )
{
}

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
IMPL_LINK( BibBookContainer, SplitHdl, Timer*,pT)
{
    long nSize= GetItemSize( TOP_WINDOW);
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setBeamerSize(nSize);
    nSize = GetItemSize( BOTTOM_WINDOW);
    pConfig->setViewSize(nSize);
    return 0;
}


uno::Reference < awt::XWindowPeer>  BibBookContainer::GetTopComponentInterface( sal_Bool bCreate)
{
    return pTopWin->GetComponentInterface(bCreate);
}
void BibBookContainer::SetTopComponentInterface( awt::XWindowPeer* pIFace )
{
    pTopWin->SetComponentInterface(pIFace);
}

uno::Reference < awt::XWindowPeer > BibBookContainer::GetBottomComponentInterface( sal_Bool bCreate)
{
    return pBottomWin->GetComponentInterface(bCreate);
}
void BibBookContainer::SetBottomComponentInterface( awt::XWindowPeer* pIFace )
{
    pBottomWin->SetComponentInterface(pIFace);
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
        USHORT nKey = aKeyCode.GetCode();
        const USHORT nModifier = aKeyCode.GetModifier();

        if( KEY_MOD2 == nModifier )
        {
            if( KEY_UP == nKey || KEY_DOWN == nKey )
            {
                if(pTopWin && pBottomWin)
                {
                    USHORT nFirstWinId = KEY_UP == nKey ? TOP_WINDOW : BOTTOM_WINDOW;
                    USHORT nSecondWinId = KEY_UP == nKey ? BOTTOM_WINDOW : TOP_WINDOW;
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
