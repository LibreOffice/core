/*************************************************************************
 *
 *  $RCSfile: newhelp.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: pb $ $Date: 2001-02-16 11:47:25 $
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
#include "helpinterceptor.hxx"
#include "helper.hxx"

#include "app.hrc"
#include "newhelp.hrc"

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
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <ucbhelper/content.hxx>

using namespace ::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace com::sun::star::ucb;

extern void AppendConfigToken_Impl( String& rURL, sal_Bool bQuestionMark ); // sfxhelp.cxx

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

#define HELPWIN_CONFIGNAME      String(DEFINE_CONST_UNICODE("OfficeHelp"))
#define PROPERTY_KEYWORDLIST    ::rtl::OUString(DEFINE_CONST_UNICODE("KeywordList"))
#define PROPERTY_KEYWORDREF     ::rtl::OUString(DEFINE_CONST_UNICODE("KeywordRef"))
#define HELP_URL                ::rtl::OUString(DEFINE_CONST_UNICODE("vnd.sun.star.help://"))
#define HELP_SEARCH_TAG         ::rtl::OUString(DEFINE_CONST_UNICODE("/?Query="))

#define PARSE_URL( aURL ) \
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( \
            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY ); \
    xTrans->parseStrict( aURL ) \

// class ContentTabPage_Impl ---------------------------------------------

ContentTabPage_Impl::ContentTabPage_Impl( Window* pParent ) :

    TabPage( pParent ),

    aContentWin( this, WB_BORDER )

{
    aContentWin.SetPosPixel( Point( 5, 5 ) );
    aContentWin.SetBackground( Wallpaper( Color( COL_WHITE ) ) );
    aContentWin.Show();
}

// -----------------------------------------------------------------------

void ContentTabPage_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    Point aPnt = aContentWin.GetPosPixel();
    aSize.Width() -= ( aPnt.X() * 2 );
    aSize.Height() -= ( aPnt.Y() * 2 );
    aContentWin.SetSizePixel( aSize );
}

// class IndexTabPage_Impl -----------------------------------------------

IndexTabPage_Impl::IndexTabPage_Impl( Window* pParent ) :

    TabPage( pParent, SfxResId( TP_HELP_INDEX ) ),

    aExpressionFT   ( this, ResId( FT_EXPRESSION ) ),
    aExpressionED   ( this, ResId( ED_EXPRESSION ) ),
    aResultsLB      ( this, ResId( LB_RESULTS ) ),
    aOpenBtn        ( this, ResId( PB_OPEN_INDEX ) )

{
    FreeResource();

    aOpenBtn.SetClickHdl( LINK( this, IndexTabPage_Impl, OpenHdl ) );

    nMinWidth = GetSizePixel().Width();
}

// -----------------------------------------------------------------------

IndexTabPage_Impl::~IndexTabPage_Impl()
{
    ClearIndex();
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::InitializeIndex()
{
    ClearIndex();

    try
    {
        ::rtl::OUString aURL = HELP_URL;
        ::rtl::OUString _aFactory( aFactory );
        aURL += _aFactory;
        String aTemp = aURL;
        AppendConfigToken_Impl( aTemp, sal_True );
        aURL = aTemp;
        Content aCnt( aURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
        if ( xInfo->hasPropertyByName( PROPERTY_KEYWORDLIST ) )
        {
            ::com::sun::star::uno::Any aAny1 = aCnt.getPropertyValue( PROPERTY_KEYWORDLIST );
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aKeywordList;
            ::com::sun::star::uno::Any aAny2 = aCnt.getPropertyValue( PROPERTY_KEYWORDREF );
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > aKeywordRefList;
            if ( ( aAny1 >>= aKeywordList ) && ( aAny2 >>= aKeywordRefList ) )
            {
                const ::rtl::OUString* pKeywords  = aKeywordList.getConstArray();
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >* pRefs = aKeywordRefList.getConstArray();
                sal_Int32 i, nCount = aKeywordList.getLength();
                DBG_ASSERT( aKeywordRefList.getLength() == nCount, "keywordlist and reflist with different length" );
                USHORT nPos;
                String aIndex, aSubIndex;

                for ( i = 0; i < nCount; ++i )
                {
                    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRefList = pRefs[i];
                    const ::rtl::OUString* pRef  = aRefList.getConstArray();
                    sal_Int32 j, nRefCount = aRefList.getLength();

                    String aKeywordPair( pKeywords[i] );
                    xub_StrLen nTokenCount = aKeywordPair.GetTokenCount();
                    if ( 1 == nTokenCount )
                    {
                        for ( j = 0; j < nRefCount; ++j )
                        {
                            nPos = aResultsLB.InsertEntry( aKeywordPair );
                            String* pData = new String( pRef[j] );
                            aResultsLB.SetEntryData( nPos, (void*)(ULONG)pData );
                        }
                    }
                    else if ( 2 == nTokenCount )
                    {
                        xub_StrLen nIdx = 0;
                        String aToken = aKeywordPair.GetToken( 0, ';', nIdx );
                        if ( aIndex != aToken )
                        {
                            aIndex = aToken;
                            aResultsLB.InsertEntry( aIndex );
                        }
                        String aSubIndex( DEFINE_CONST_UNICODE("  ") );
                        aSubIndex += aKeywordPair.GetToken( 0, ';', nIdx );
                        for ( j = 0; j < nRefCount; ++j )
                        {
                            nPos = aResultsLB.InsertEntry( aSubIndex );
                            String* pData = new String( pRef[j] );
                            aResultsLB.SetEntryData( nPos, (void*)(ULONG)pData );
                        }
                    }
                    else
                    {
                        DBG_ERRORFILE( "unexpected token count of a keyword" );
                    }
                }
            }
        }
    }
    catch( ::com::sun::star::ucb::ContentCreationException& )
    {
        DBG_ERRORFILE( "content creation exception" );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "command aborted exception" );
    }
    catch( ::com::sun::star::uno::RuntimeException& )
    {
        DBG_ERRORFILE( "runtime exception" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::ClearIndex()
{
    USHORT nCount = aResultsLB.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
        delete (String*)(ULONG)aResultsLB.GetEntryData(i);
    aResultsLB.Clear();
}

// -----------------------------------------------------------------------

IMPL_LINK( IndexTabPage_Impl, OpenHdl, PushButton*, EMPTYARG )
{
    aResultsLB.GetDoubleClickHdl().Call( &aResultsLB );
    return 0;
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::Resize()
{
    Size aSize = GetSizePixel();
    long nWidth = aExpressionFT.GetPosPixel().X() + aExpressionFT.GetSizePixel().Width();
    if ( aSize.Width() > nMinWidth || nWidth > aSize.Width() )
    {
        Point aPnt = aExpressionFT.GetPosPixel();
        long nDelta = ( aPnt.X() / 2 );
        Size aNewSize = aExpressionFT.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aExpressionFT.SetSizePixel( aNewSize );

        aPnt = aExpressionED.GetPosPixel();
        aNewSize = aExpressionED.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aExpressionED.SetSizePixel( aNewSize );

        Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
        Size aBtnSize = aOpenBtn.GetSizePixel();

        aPnt = aResultsLB.GetPosPixel();
        aNewSize = aResultsLB.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aNewSize.Height() = aSize.Height() - aPnt.Y();
        aNewSize.Height() -= ( aBtnSize.Height() + ( a6Size.Height() * 3 / 2 ) );
        aResultsLB.SetSizePixel( aNewSize );

        aPnt.X() += ( aNewSize.Width() - aBtnSize.Width() );
        aPnt.Y() += aNewSize.Height() + ( a6Size.Height() / 2 );
        aOpenBtn.SetPosPixel( aPnt );
    }
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aResultsLB.SetDoubleClickHdl( rLink );
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::SetFactory( const String& rFactory )
{
    if ( rFactory != aFactory )
    {
        aFactory = rFactory;
        InitializeIndex();
    }
}

// class SearchBox_Impl --------------------------------------------------

long SearchBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == EVENT_KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        aSearchLink.Call( NULL );
        bHandled = sal_True;
    }
    return bHandled ? 1 : ComboBox::PreNotify( rNEvt );
}

// class SearchTabPage_Impl ----------------------------------------------

SearchTabPage_Impl::SearchTabPage_Impl( Window* pParent ) :

    TabPage( pParent, SfxResId( TP_HELP_SEARCH ) ),

    aSearchFT   ( this, ResId( FT_SEARCH ) ),
    aSearchED   ( this, ResId( ED_SEARCH ) ),
    aSearchBtn  ( this, ResId( PB_SEARCH ) ),
    aResultsLB  ( this, ResId( LB_RESULT ) ),
    aOpenBtn    ( this, ResId( PB_OPEN_SEARCH ) )

{
    FreeResource();

    Link aLink = LINK( this, SearchTabPage_Impl, SearchHdl );
    aSearchED.SetSearchLink( aLink );
    aSearchBtn.SetClickHdl( aLink );
    aOpenBtn.SetClickHdl( LINK( this, SearchTabPage_Impl, OpenHdl ) );

    aMinSize = GetSizePixel();
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::ClearSearchResults()
{
    USHORT nCount = aResultsLB.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
        delete (String*)(ULONG)aResultsLB.GetEntryData(i);
    aResultsLB.Clear();
}

// -----------------------------------------------------------------------

IMPL_LINK( SearchTabPage_Impl, SearchHdl, PushButton*, EMPTYARG )
{
    ClearSearchResults();
    String aSearchURL = HELP_URL;
    aSearchURL += aFactory;
    aSearchURL += String( HELP_SEARCH_TAG );
    aSearchURL += aSearchED.GetText();
    AppendConfigToken_Impl( aSearchURL, sal_False );

    Sequence< ::rtl::OUString > aFactories = SfxContentHelper::GetResultSet( aSearchURL );
    const ::rtl::OUString* pFacs  = aFactories.getConstArray();
    UINT32 i, nCount = aFactories.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pFacs[i] );
        String aTitle, aType;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aType = aRow.GetToken( 0, '\t', nIdx );
        String* pURL = new String( aRow.GetToken( 0, '\t', nIdx ) );
        USHORT nPos = aResultsLB.InsertEntry( aTitle );
        aResultsLB.SetEntryData( nPos, (void*)(ULONG)pURL );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SearchTabPage_Impl, OpenHdl, PushButton*, EMPTYARG )
{
    aResultsLB.GetDoubleClickHdl().Call( &aResultsLB );
    return 0;
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::Resize()
{
    Size aSize = GetSizePixel();
    long nWidth = aSearchBtn.GetPosPixel().X() + aSearchBtn.GetSizePixel().Width();
    if ( aSize.Width() > aMinSize.Width() || nWidth > aSize.Width() )
    {
        Point aPnt = aSearchFT.GetPosPixel();
        Size aNewSize = aSearchFT.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
        aSearchFT.SetSizePixel( aNewSize );
        aNewSize.Height() = aResultsLB.GetSizePixel().Height();
        aResultsLB.SetSizePixel( aNewSize );

        aNewSize = aSearchED.GetSizePixel();
        aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 ) -
                           ( aSearchBtn.GetSizePixel().Width() + ( aPnt.X() / 2 ) );
        aSearchED.SetSizePixel( aNewSize );
        Point aNewPnt = aSearchBtn.GetPosPixel();
        aNewPnt.X() = aPnt.X() + aNewSize.Width() + ( aPnt.X() / 2 );
        aSearchBtn.SetPosPixel( aNewPnt );
    }

    if ( aSize.Height() > aMinSize.Height() )
    {
        Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
        Size aBtnSize = aOpenBtn.GetSizePixel();

        Point aPnt = aResultsLB.GetPosPixel();
        Size aNewSize = aResultsLB.GetSizePixel();
        aNewSize.Height() = aSize.Height() - aPnt.Y();
        aNewSize.Height() -= ( aBtnSize.Height() + ( a6Size.Height() * 3 / 2 ) );
        aResultsLB.SetSizePixel( aNewSize );

        aPnt.X() += ( aNewSize.Width() - aBtnSize.Width() );
        aPnt.Y() += aNewSize.Height() + ( a6Size.Height() / 2 );
        aOpenBtn.SetPosPixel( aPnt );
    }
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aResultsLB.SetDoubleClickHdl( rLink );
}

// class SfxHelpIndexWindow_Impl -----------------------------------------

SfxHelpIndexWindow_Impl::SfxHelpIndexWindow_Impl( Window* pParent ) :

    Window( pParent, SfxResId( WIN_HELP_INDEX ) ),

    aActiveLB   ( this, ResId( LB_ACTIVE ) ),
    aActiveLine ( this, ResId( FL_ACTIVE ) ),
    aTabCtrl    ( this, ResId( TC_INDEX ) ),

    pCPage      ( NULL ),
    pIPage      ( NULL ),
    pSPage      ( NULL )

{
    FreeResource();

    aTabCtrl.SetActivatePageHdl( LINK( this, SfxHelpIndexWindow_Impl, ActivatePageHdl ) );
    aTabCtrl.Show();
    aTabCtrl.SetCurPageId( 1 );
    ActivatePageHdl( &aTabCtrl );
    aActiveLB.SetSelectHdl( LINK( this, SfxHelpIndexWindow_Impl, SelectHdl ) );
    nMinWidth = aActiveLB.GetSizePixel().Width();
    Initialize();
}

// -----------------------------------------------------------------------

SfxHelpIndexWindow_Impl::~SfxHelpIndexWindow_Impl()
{
    delete pCPage;
    delete pIPage;
    delete pSPage;

    for ( USHORT i = 0; i < aActiveLB.GetEntryCount(); ++i )
        delete (String*)(ULONG)aActiveLB.GetEntryData(i);
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::Initialize()
{
    String aHelpURL = HELP_URL;
    AppendConfigToken_Impl( aHelpURL, sal_True );
    Sequence< ::rtl::OUString > aFactories = SfxContentHelper::GetResultSet( aHelpURL );
    const ::rtl::OUString* pFacs  = aFactories.getConstArray();
    UINT32 i, nCount = aFactories.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pFacs[i] );
        String aTitle, aType, aURL;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aType = aRow.GetToken( 0, '\t', nIdx );
        aURL = aRow.GetToken( 0, '\t', nIdx );
        aURL.Erase( aURL.Len() - 1 );
        ::rtl::OUString aTemp( aURL );
        sal_Int32 nCharPos = aTemp.lastIndexOf( '/' );
        String* pFactory = new String( aTemp.copy( nCharPos + 1 ) );

        USHORT nPos = aActiveLB.InsertEntry( aTitle );
        aActiveLB.SetEntryData( nPos, (void*)(ULONG)pFactory );
    }
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::SetActiveFactory()
{
    DBG_ASSERT( pIPage, "index page not initialized" );

    for ( USHORT i = 0; i < aActiveLB.GetEntryCount(); ++i )
    {
        String* pFactory = (String*)(ULONG)aActiveLB.GetEntryData(i);
        pFactory->ToLowerAscii();
        if ( *pFactory == pIPage->GetFactory() )
        {
            aActiveLB.SelectEntryPos(i);
            break;
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const USHORT nId = pTabCtrl->GetCurPageId();
    TabPage* pPage = NULL;

    switch ( nId )
    {
        case HELP_INDEX_PAGE_CONTENTS:
        {
            if ( !pCPage )
                pCPage = new ContentTabPage_Impl( &aTabCtrl );
            pPage = pCPage;
            break;
        }

        case HELP_INDEX_PAGE_INDEX:
        {
            if ( !pIPage )
                pIPage = new IndexTabPage_Impl( &aTabCtrl );
            pPage = pIPage;
            break;
        }

        case HELP_INDEX_PAGE_SEARCH:
        {
            if ( !pSPage )
                pSPage = new SearchTabPage_Impl( &aTabCtrl );
            pPage = pSPage;
            break;
        }
    }

    pTabCtrl->SetTabPage( nId, pPage );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, SelectHdl, ListBox *, EMPTYARG )
{
    String* pFactory = (String*)(ULONG)aActiveLB.GetEntryData( aActiveLB.GetSelectEntryPos() );
    if ( pFactory )
    {
        String aFactory( *pFactory );
        aFactory.ToLowerAscii();
        SetFactory( aFactory, sal_False );
    }

    return 0;
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    Size aNewSize;
    Point aPnt;

    if ( aSize.Width() < nMinWidth )
        aSize.Width() = nMinWidth;
    else
    {
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

void SfxHelpIndexWindow_Impl::SetDoubleClickHdl( const Link& rLink )
{
    if ( !pIPage )
        pIPage = new IndexTabPage_Impl( &aTabCtrl );
    pIPage->SetDoubleClickHdl( rLink );
    if ( !pSPage )
        pSPage = new SearchTabPage_Impl( &aTabCtrl );
    pSPage->SetDoubleClickHdl( rLink );
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::SetFactory( const String& rFactory, sal_Bool bActive )
{
    if ( !pIPage )
        pIPage = new IndexTabPage_Impl( &aTabCtrl );
    pIPage->SetFactory( rFactory );
    if ( !pSPage )
        pSPage = new SearchTabPage_Impl( &aTabCtrl );
    pSPage->SetFactory( rFactory );

    if ( bActive )
        SetActiveFactory();
}

// class SfxHelpTextWindow_Impl ------------------------------------------

SfxHelpTextWindow_Impl::SfxHelpTextWindow_Impl( Window* pParent ) :

    Window( pParent, WB_CLIPCHILDREN ),

    aToolBox( this, 0 ),
    pTextWin( new Window( this, 0 ) )

{
    xFrame = Reference < XFrame > ( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( pTextWin ) );
    xFrame->setName( DEFINE_CONST_UNICODE("OFFICE_HELP") );

    SetBackground( Wallpaper( Color( COL_WHITE ) ) );

    aToolBox.InsertItem( TBI_INDEX, Image( SfxResId( IMG_HELP_TOOLBOX_INDEX ) ) );
    aToolBox.SetQuickHelpText( TBI_INDEX, String( SfxResId( STR_HELP_BUTTON_INDEX ) ) );
    aToolBox.InsertItem( TBI_START, Image( SfxResId( IMG_HELP_TOOLBOX_START ) ) );
    aToolBox.SetQuickHelpText( TBI_START, String( SfxResId( STR_HELP_BUTTON_START ) ) );
    aToolBox.InsertItem( TBI_BACKWARD, Image( SfxResId( IMG_HELP_TOOLBOX_PREV ) ) );
    aToolBox.SetQuickHelpText( TBI_BACKWARD, String( SfxResId( STR_HELP_BUTTON_PREV ) ) );
    aToolBox.InsertItem( TBI_FORWARD, Image( SfxResId( IMG_HELP_TOOLBOX_NEXT ) ) );
    aToolBox.SetQuickHelpText( TBI_FORWARD, String( SfxResId( STR_HELP_BUTTON_NEXT ) ) );
/*! aToolBox.InsertItem( TBI_CONTEXT, Image( SfxResId(IMG_HELP_TOOLBOX_CONTEXT) ) );
    aToolBox.SetQuickHelpText( TBI_CONTEXT, DEFINE_CONST_UNICODE("Context") );*/
    aToolBox.InsertItem( TBI_PRINT, Image( SfxResId( IMG_HELP_TOOLBOX_PRINT ) ) );
    aToolBox.SetQuickHelpText( TBI_PRINT, String( SfxResId( STR_HELP_BUTTON_PRINT ) ) );

    Size aSize = aToolBox.CalcWindowSizePixel();
    aToolBox.SetSizePixel( aSize );
    aToolBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
    aToolBox.Show();
}

// -----------------------------------------------------------------------

SfxHelpTextWindow_Impl::~SfxHelpTextWindow_Impl()
{
    xFrame->dispose();
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    Size aSize = aToolBox.GetSizePixel();
    aSize.Width() = aWinSize.Width();
    aToolBox.SetSizePixel( aSize );

    long nToolBoxHeight = aSize.Height();
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    pTextWin->SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}

// class SfxHelpWindow_Impl ----------------------------------------------

void SfxHelpWindow_Impl::Resize()
{
    SplitWindow::Resize();
    InitSizes();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::Split()
{
    SplitWindow::Split();

    nIndexSize = GetItemSize( INDEXWIN_ID );
    nTextSize = GetItemSize( TEXTWIN_ID );
    InitSizes();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::MakeLayout()
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

void SfxHelpWindow_Impl::InitSizes()
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

void SfxHelpWindow_Impl::LoadConfig()
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

void SfxHelpWindow_Impl::SaveConfig()
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

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, SelectHdl, ToolBox* , pToolBox )
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
                String aStartURL;
                aStartURL = HELP_URL;
                aStartURL += pIndexWin->GetFactory();
                aStartURL += DEFINE_CONST_UNICODE("/start");
                AppendConfigToken_Impl( aStartURL, sal_True );

                URL aURL;
                aURL.Complete = aStartURL;
                PARSE_URL( aURL );

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
                break;
            }

            case TBI_BACKWARD :
            case TBI_FORWARD :
            {
                URL aURL;
                aURL.Complete = DEFINE_CONST_UNICODE(".uno:Backward");
                if ( TBI_FORWARD == nId )
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:Forward");
                PARSE_URL( aURL );
                pHelpInterceptor->dispatch( aURL, Sequence < PropertyValue >() );
                break;
            }

            case TBI_CONTEXT :
                break;

            case TBI_PRINT :
            {
                Reference < XDispatchProvider > xProv( pTextWin->getFrame(), UNO_QUERY );
                if ( xProv.is() )
                {
                    URL aURL;
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:Print");
                    PARSE_URL( aURL );
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

//-------------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, OpenHdl, ListBox* , pBox )
{
    String* pData = (String*)(ULONG)pBox->GetEntryData( pBox->GetSelectEntryPos() );
    if ( pData )
    {
        String aEntry( *pData );
        INetURLObject aObj( aEntry );
        if ( aObj.GetProtocol() != INET_PROT_VND_SUN_STAR_HELP )
        {
            aEntry = HELP_URL;
            aEntry += pIndexWin->GetFactory();
            aEntry += '/';
            aEntry += *pData;
            AppendConfigToken_Impl( aEntry, sal_True );
        }
        URL aURL;
        aURL.Complete = aEntry;
        PARSE_URL( aURL );
        Reference < XDispatch > xDisp = pHelpInterceptor->queryDispatch( aURL, String(), 0 );
        if ( xDisp.is() )
            xDisp->dispatch( aURL, Sequence < PropertyValue >() );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, ChangeHdl, HelpListener_Impl*, pListener )
{
    SetFactory( pListener->GetFactory(), sal_False );
    return 0;
}

// -----------------------------------------------------------------------

SfxHelpWindow_Impl::SfxHelpWindow_Impl(
    const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParent, WinBits nBits ) :

    SplitWindow( pParent, nBits | WB_3DLOOK ),

    pIndexWin       ( NULL ),
    pTextWin        ( NULL ),
    pHelpInterceptor( new HelpInterceptor_Impl() ),
    pHelpListener   ( new HelpListener_Impl( pHelpInterceptor ) ),
    nExpandWidth    ( 0 ),
    nCollapseWidth  ( 0 ),
    nHeight         ( 0 ),
    nIndexSize      ( 40 ),
    nTextSize       ( 60 ),
    bIndex          ( sal_True )

{
    pIndexWin = new SfxHelpIndexWindow_Impl( this );
    pIndexWin->SetDoubleClickHdl( LINK( this, SfxHelpWindow_Impl, OpenHdl ) );
    pIndexWin->Show();
    pTextWin = new SfxHelpTextWindow_Impl( this );
    Reference < XFramesSupplier > xSup( rFrame, UNO_QUERY );
    Reference < XFrames > xFrames = xSup->getFrames();
    xFrames->append( pTextWin->getFrame() );
    pTextWin->SetSelectHdl( LINK( this, SfxHelpWindow_Impl, SelectHdl ) );
    pTextWin->Show();
    pHelpInterceptor->setInterception( pTextWin->getFrame() );
    pHelpListener->SetChangeHdl( LINK( this, SfxHelpWindow_Impl, ChangeHdl ) );

    LoadConfig();
}

// -----------------------------------------------------------------------

SfxHelpWindow_Impl::~SfxHelpWindow_Impl()
{
    SaveConfig();
    delete pIndexWin;
    delete pTextWin;
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::setContainerWindow( Reference < ::com::sun::star::awt::XWindow > xWin )
{
    xWindow = xWin;
    MakeLayout();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::SetFactory( const String& rFactory, sal_Bool bStart )
{
    pIndexWin->SetFactory( rFactory, sal_True );
    if ( bStart )
        pHelpInterceptor->SetFactory( rFactory );
}

