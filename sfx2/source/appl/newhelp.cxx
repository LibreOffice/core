/*************************************************************************
 *
 *  $RCSfile: newhelp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-27 09:21:24 $
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

#include "newhelp.hxx"
#include "sfxuno.hxx"
#include "sfxresid.hxx"

#include "app.hrc"

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// defines ---------------------------------------------------------------

#define SPLITSET_ID         0
#define COLSET_ID           1
#define INDEXWIN_ID         2
#define TEXTWIN_ID          3

#define TBI_INDEX           1001
#define TBI_START           1002
#define TBI_BACKWARD        1003
#define TBI_FORWARD         1004
#define TBI_CONTEXT         1005
#define TBI_PRINT           1006

#define HELPWIN_CONFIGNAME  String(DEFINE_CONST_UNICODE("OfficeHelp"))

// class ContentTabPage --------------------------------------------------

ContentTabPage::ContentTabPage( Window* pParent ) :

    TabPage( pParent ),

    aContentWin( this, WB_BORDER )

{
    aContentWin.SetPosPixel( Point( 5, 5 ) );
    aContentWin.SetBackground( Wallpaper( Color( COL_WHITE ) ) );
    aContentWin.Show();
}

// -----------------------------------------------------------------------

void ContentTabPage::Resize()
{
    Size aSize = GetOutputSizePixel();
    Point aPnt = aContentWin.GetPosPixel();
    aSize.Width() -= ( aPnt.X() * 2 );
    aSize.Height() -= ( aPnt.Y() * 2 );
    aContentWin.SetSizePixel( aSize );
}

// class IndexTabPage ----------------------------------------------------

IndexTabPage::IndexTabPage( Window* pParent ) :

    TabPage( pParent, SfxResId( TP_HELP_INDEX ) ),

    aExpressionFT   ( this, ResId( FT_EXPRESSION ) ),
    aExpressionED   ( this, ResId( ED_EXPRESSION ) ),
    aResultsLB      ( this, ResId( LB_RESULTS ) )

{
    FreeResource();

    nMinWidth = GetSizePixel().Width();
}

// -----------------------------------------------------------------------

void IndexTabPage::Resize()
{
    Size aSize = GetSizePixel();
    if ( aSize.Width() > nMinWidth )
    {
        Point aPnt = aExpressionFT.GetPosPixel();
        Size aNewSize = aExpressionFT.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aExpressionFT.SetSizePixel( aNewSize );

        aPnt = aExpressionED.GetPosPixel();
        aNewSize = aExpressionED.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aExpressionED.SetSizePixel( aNewSize );

        aPnt = aResultsLB.GetPosPixel();
        aNewSize = aResultsLB.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aNewSize.Height() = aSize.Height() - ( aPnt.Y() + aPnt.X() );
        aResultsLB.SetSizePixel( aNewSize );
    }
}

// class SearchTabPage ---------------------------------------------------

SearchTabPage::SearchTabPage( Window* pParent ) :

    TabPage( pParent, SfxResId( TP_HELP_SEARCH ) ),

    aSearchFT   ( this, ResId( FT_SEARCH ) ),
    aSearchED   ( this, ResId( ED_SEARCH ) ),
    aOperatorBtn( this, ResId( PB_OPERATOR ) ),
    aResultFT   ( this, ResId( FT_RESULT ) ),
    aResultLB   ( this, ResId( LB_RESULT ) ),
    aPreviousCB ( this, ResId( CB_PREVIOUS ) ),
    aMatchCB    ( this, ResId( CB_MATCH ) ),
    aTitleCB    ( this, ResId( CB_TITLE ) )

{
    FreeResource();

    aOperatorBtn.SetSymbol( SYMBOL_SPIN_RIGHT );

    String aText = aResultFT.GetText();
    aText.SearchAndReplace( DEFINE_CONST_UNICODE("%1"), DEFINE_CONST_UNICODE(" ") );
    aText.SearchAndReplace( DEFINE_CONST_UNICODE("%2"), DEFINE_CONST_UNICODE("0") );
    aResultFT.SetText( aText );

    aMinSize = GetSizePixel();
}

// -----------------------------------------------------------------------

void SearchTabPage::Resize()
{
    Size aSize = GetSizePixel();
    if ( aSize.Width() > aMinSize.Width() )
    {
        Point aPnt = aSearchFT.GetPosPixel();
        Size aNewSize = aSearchFT.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aSearchFT.SetSizePixel( aNewSize );
        aResultFT.SetSizePixel( aNewSize );
        aPreviousCB.SetSizePixel( aNewSize );
        aMatchCB.SetSizePixel( aNewSize );
        aTitleCB.SetSizePixel( aNewSize );
        aNewSize.Height() = aResultLB.GetSizePixel().Height();
        aResultLB.SetSizePixel( aNewSize );

        aNewSize = aSearchED.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 ) -
                           ( aOperatorBtn.GetSizePixel().Width() + ( aPnt.X() / 2 ) );
        aSearchED.SetSizePixel( aNewSize );
        Point aNewPnt = aOperatorBtn.GetPosPixel();
        aNewPnt.X() = aPnt.X() + aNewSize.Width() + ( aPnt.X() / 2 );
        aOperatorBtn.SetPosPixel( aNewPnt );
    }

    if ( aSize.Height() > aMinSize.Height() )
    {
        Point aPnt = aSearchFT.GetPosPixel();
        long nH = aPreviousCB.GetSizePixel().Height();
        nH *= 3;
        nH += ( aPnt.Y() * 5 / 2 );
        aPnt = aResultLB.GetPosPixel();
        Size aOldSize = aResultLB.GetSizePixel();
        Size aNewSize = aOldSize;
        aNewSize.Height() = aSize.Height() - aPnt.Y() - nH;
        aResultLB.SetSizePixel( aNewSize );
        long nDeltaH = aNewSize.Height() - aOldSize.Height();
        aPnt = aPreviousCB.GetPosPixel();
        aPnt.Y() += nDeltaH;
        aPreviousCB.SetPosPixel( aPnt );
        aPnt = aMatchCB.GetPosPixel();
        aPnt.Y() += nDeltaH;
        aMatchCB.SetPosPixel( aPnt );
        aPnt = aTitleCB.GetPosPixel();
        aPnt.Y() += nDeltaH;
        aTitleCB.SetPosPixel( aPnt );
    }
}

// class SfxHelpIndexWindow ----------------------------------------------

SfxHelpIndexWindow::SfxHelpIndexWindow( Window* pParent ) :

    Window( pParent, SfxResId( WIN_HELP_INDEX ) ),

    aActiveFT   ( this, ResId( FT_ACTIVE ) ),
    aActiveLB   ( this, ResId( LB_ACTIVE ) ),
    aActiveLine ( this, ResId( FL_ACTIVE ) ),
    aTabCtrl    ( this, ResId( TC_INDEX ) ),

    pCPage      ( NULL ),
    pIPage      ( NULL ),
    pSPage      ( NULL )

{
    FreeResource();

    aTabCtrl.SetActivatePageHdl( LINK( this, SfxHelpIndexWindow, ActivatePageHdl ) );
    aTabCtrl.Show();
    aTabCtrl.SetCurPageId( 1 );
    ActivatePageHdl( &aTabCtrl );
    nMinWidth = aActiveLB.GetSizePixel().Width();
}

// -----------------------------------------------------------------------

SfxHelpIndexWindow::~SfxHelpIndexWindow()
{
    delete pCPage;
    delete pIPage;
    delete pSPage;
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow::Resize()
{
    Size aSize = GetOutputSizePixel();
    Size aNewSize;
    Point aPnt;

    if ( aSize.Width() < nMinWidth )
        aSize.Width() = nMinWidth;
    else
    {
        aPnt = aActiveFT.GetPosPixel();
        aNewSize = aActiveFT.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aActiveFT.SetSizePixel( aNewSize );

        aPnt = aActiveLB.GetPosPixel();
        aNewSize = aActiveLB.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aActiveLB.SetSizePixel( aNewSize );

        aPnt = aActiveLine.GetPosPixel();
        aNewSize = aActiveLine.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aActiveLine.SetSizePixel( aNewSize );
    }

    aPnt = aTabCtrl.GetPosPixel();
    aNewSize = aSize;
    aSize.Height() -= aPnt.Y();
    aTabCtrl.SetSizePixel( aSize );
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const USHORT nId = pTabCtrl->GetCurPageId();
    TabPage* pPage = NULL;

    switch ( nId )
    {
        case 1:
        {
            if ( !pCPage )
                pCPage = new ContentTabPage( &aTabCtrl );
            pPage = pCPage;
            break;
        }

        case 2:
        {
            if ( !pIPage )
                pIPage = new IndexTabPage( &aTabCtrl );
            pPage = pIPage;
            break;
        }

        case 3:
        {
            if ( !pSPage )
                pSPage = new SearchTabPage( &aTabCtrl );
            pPage = pSPage;
            break;
        }
    }

    pTabCtrl->SetTabPage( nId, pPage );

    return 0;
}

// class SfxHelpTextWindow -----------------------------------------------

SfxHelpTextWindow::SfxHelpTextWindow( Window* pParent ) :

    Window( pParent, WB_CLIPCHILDREN ),

    aToolBox( this, 0 ),
    aTextWin( this, 0 )

{
    xFrame = Reference < XFrame > ( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( &aTextWin ) );
    xFrame->setName( DEFINE_CONST_UNICODE("OFFICE_HELP") );

    SetBackground( Wallpaper( Color( COL_WHITE ) ) );

    aToolBox.InsertItem( TBI_INDEX, Image( SfxResId(IMG_HELP_TOOLBOX_INDEX) ) );
    aToolBox.SetQuickHelpText( TBI_INDEX, DEFINE_CONST_UNICODE("Index") );
    aToolBox.InsertItem( TBI_START, Image( SfxResId(IMG_HELP_TOOLBOX_START) ) );
    aToolBox.SetQuickHelpText( TBI_START, DEFINE_CONST_UNICODE("Start") );
    aToolBox.InsertItem( TBI_BACKWARD, Image( SfxResId(IMG_HELP_TOOLBOX_PREV) ) );
    aToolBox.SetQuickHelpText( TBI_BACKWARD, DEFINE_CONST_UNICODE("Prev") );
    aToolBox.InsertItem( TBI_FORWARD, Image( SfxResId(IMG_HELP_TOOLBOX_NEXT) ) );
    aToolBox.SetQuickHelpText( TBI_FORWARD, DEFINE_CONST_UNICODE("Next") );
    aToolBox.InsertItem( TBI_CONTEXT, Image( SfxResId(IMG_HELP_TOOLBOX_CONTEXT) ) );
    aToolBox.SetQuickHelpText( TBI_CONTEXT, DEFINE_CONST_UNICODE("Context") );
    aToolBox.InsertItem( TBI_PRINT, Image( SfxResId(IMG_HELP_TOOLBOX_PRINT) ) );
    aToolBox.SetQuickHelpText( TBI_PRINT, DEFINE_CONST_UNICODE("Print") );

    Size aSize = aToolBox.CalcWindowSizePixel();
    aToolBox.SetSizePixel( aSize );
    aToolBox.Show();
}

// -----------------------------------------------------------------------

SfxHelpTextWindow::~SfxHelpTextWindow()
{
//! xFrame->dispose();
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow::Paint( const Rectangle& rRect )
{
    aTextWin.DrawText( Point( 3, 3 ), DEFINE_CONST_UNICODE("Hilfe!") );
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    Size aSize = aToolBox.GetSizePixel();
    aSize.Width() = aWinSize.Width();
    aToolBox.SetSizePixel( aSize );

    long nToolBoxHeight = aSize.Height();
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    aTextWin.SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}

// class SfxHelpWindow ---------------------------------------------------

void SfxHelpWindow::Resize()
{
    SplitWindow::Resize();
    InitSizes();
}

// -----------------------------------------------------------------------

void SfxHelpWindow::Split()
{
    SplitWindow::Split();

    nIndexSize = GetItemSize( INDEXWIN_ID );
    nTextSize = GetItemSize( TEXTWIN_ID );
    InitSizes();
}

// -----------------------------------------------------------------------

void SfxHelpWindow::MakeLayout()
{
    if ( nHeight > 0 )
    {
        if ( !bIndex )
        {
            Size aSize = pTextWin->GetOutputSizePixel();
            if ( xWindow.is() )
            {
                ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
                xWindow->setPosSize( aRect.X, aRect.Y, nCollapseWidth, nHeight,
                                     ::com::sun::star::awt::PosSize::SIZE );
            }
        }
        else if ( xWindow.is() )
        {
            ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
            xWindow->setPosSize( aRect.X, aRect.Y, nExpandWidth, nHeight,
                                 ::com::sun::star::awt::PosSize::SIZE );
        }
    }

    Clear();

    if ( bIndex )
    {
        pIndexWin->Show();
        InsertItem( COLSET_ID, 100, SPLITWINDOW_APPEND, SPLITSET_ID, SWIB_PERCENTSIZE | SWIB_COLSET );
        InsertItem( INDEXWIN_ID, pIndexWin, nIndexSize, SPLITWINDOW_APPEND, COLSET_ID, SWIB_PERCENTSIZE );
        InsertItem( TEXTWIN_ID, pTextWin, nTextSize, SPLITWINDOW_APPEND, COLSET_ID, SWIB_PERCENTSIZE );
    }
    else
    {
        pIndexWin->Hide();
        InsertItem( COLSET_ID, 100, SPLITWINDOW_APPEND, SPLITSET_ID, SWIB_PERCENTSIZE | SWIB_COLSET );
        InsertItem( TEXTWIN_ID, pTextWin, 100, SPLITWINDOW_APPEND, 1, SWIB_PERCENTSIZE );
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow::InitSizes()
{
    if ( xWindow.is() )
    {
        ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
        nHeight = aRect.Height;

        if ( bIndex )
        {
            nExpandWidth = aRect.Width;
            nCollapseWidth = nExpandWidth * nTextSize / 100;
        }
        else
        {
            nCollapseWidth = aRect.Width;
            nExpandWidth = nCollapseWidth * 100 / nTextSize;
        }
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow::LoadConfig()
{
    sal_Int32 nWidth;
     SvtViewOptions aViewOpt( E_WINDOW, HELPWIN_CONFIGNAME );
    if ( aViewOpt.Exists() )
    {
        aViewOpt.GetSize( nWidth, nHeight );
        bIndex = aViewOpt.IsVisible();
        String aUserData = aViewOpt.GetUserData();
        DBG_ASSERT( aUserData.GetTokenCount() == 2, "invalid user data" );
        nIndexSize = aUserData.GetToken(0).ToInt32();
        nTextSize = aUserData.GetToken(1).ToInt32();

        if ( bIndex )
        {
            nExpandWidth = nWidth;
            nCollapseWidth = nExpandWidth * nTextSize / 100;
        }
        else
        {
            nCollapseWidth = nWidth;
            nExpandWidth = nCollapseWidth * 100 / nTextSize;
        }
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow::SaveConfig()
{
    SvtViewOptions aViewOpt( E_WINDOW, HELPWIN_CONFIGNAME );

    if ( xWindow.is() )
    {
        ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
        aViewOpt.SetSize( aRect.Width, aRect.Height );
    }

    aViewOpt.SetVisible( bIndex );
    String aUserData = String::CreateFromInt32( nIndexSize );
    aUserData += ';';
    aUserData += String::CreateFromInt32( nTextSize );
    aViewOpt.SetUserData( aUserData );
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow, SelectHdl, ToolBox* , pToolBox )
{
    if ( pToolBox )
    {
        USHORT nId = pToolBox->GetCurItemId();
        switch ( nId )
        {
            case TBI_INDEX :
            {
                bIndex = !bIndex;
                MakeLayout();
                break;
            }

            case TBI_START :
            {
#ifdef DEBUG
                URL aURL;
                aURL.Complete = DEFINE_CONST_UNICODE("file:///e:/test.sdw");
                String aTarget( DEFINE_CONST_UNICODE("_self") );
                Reference < XDispatchProvider > xProv( pTextWin->getFrame(), UNO_QUERY );
                Reference < XDispatch > xDisp = xProv.is() ?
                    xProv->queryDispatch( aURL, aTarget, 0 ) : Reference < XDispatch >();
                if ( xDisp.is() )
                {
                    Sequence < PropertyValue > aArgs( 1 );
                    aArgs[0].Name = String( DEFINE_CONST_UNICODE("ReadOnly") );
                    BOOL bReadOnly = TRUE;
                    aArgs[0].Value <<= bReadOnly;
                    xDisp->dispatch( aURL, aArgs );
                }
#endif
                break;
            }

            case TBI_BACKWARD :
            case TBI_FORWARD :
                break;

            case TBI_CONTEXT :
                break;

            case TBI_PRINT :
            {
                Reference < XDispatchProvider > xProv( pTextWin->getFrame(), UNO_QUERY );
                if ( xProv.is() )
                {
                    URL aURL;
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:Print");
                    Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, String(), 0 );
                    if ( xDisp.is() )
                        xDisp->dispatch( aURL, Sequence < PropertyValue >() );
                }
                break;
            }
        }
    }

    return 1;
}

// -----------------------------------------------------------------------

SfxHelpWindow::SfxHelpWindow( const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >& rFrame,
        Window* pParent, WinBits nBits ) :

    SplitWindow( pParent, nBits | WB_3DLOOK ),

    pIndexWin       ( NULL ),
    pTextWin        ( NULL ),
    nExpandWidth    ( 0 ),
    nCollapseWidth  ( 0 ),
    nHeight         ( 0 ),
    nIndexSize      ( 40 ),
    nTextSize       ( 60 ),
    bIndex          ( sal_True )

{
    pIndexWin = new SfxHelpIndexWindow( this );
    pIndexWin->Show();
    pTextWin = new SfxHelpTextWindow( this );
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier > xSup( rFrame, UNO_QUERY );
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrames > xFrames = xSup->getFrames();
    xFrames->append( pTextWin->getFrame() );
    pTextWin->SetSelectHdl( LINK( this, SfxHelpWindow, SelectHdl ) );
    pTextWin->Show();

    LoadConfig();
}

// -----------------------------------------------------------------------

SfxHelpWindow::~SfxHelpWindow()
{
    SaveConfig();
    delete pIndexWin;
    delete pTextWin;
}

// -----------------------------------------------------------------------

void SfxHelpWindow::setContainerWindow( Reference < ::com::sun::star::awt::XWindow > xWin )
{
    xWindow = xWin;
    MakeLayout();
}

