/*************************************************************************
 *
 *  $RCSfile: newhelp.cxx,v $
 *
 *  $Revision: 1.95 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 14:04:48 $
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
#include "msgpool.hxx"
#include "app.hxx"
#include "sfxtypes.hxx"
#include "panelist.hxx"
#include "imgmgr.hxx"
#include "srchdlg.hxx"

#include "app.hrc"
#include "newhelp.hrc"
#include "helpid.hrc"

#include <hash_map>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSOR_HPP_
#include <com/sun/star/text/XTextViewCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSORSUPPLIER_HPP_
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHABLE_HPP_
#include <com/sun/star/util/XSearchable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHDESCRIPTOR_HPP_
#include <com/sun/star/util/XSearchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif

#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _CACHESTR_HXX
#include <tools/cachestr.hxx>
#endif
#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif
#ifndef _SVTOOLS_IMAGEMGR_HXX
#include <svtools/imagemgr.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif

#include <ucbhelper/content.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/ucbhelper.hxx>

using namespace ::ucb;
using namespace com::sun::star::ucb;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::view;

extern void AppendConfigToken_Impl( String& rURL, sal_Bool bQuestionMark ); // sfxhelp.cxx

// defines ---------------------------------------------------------------

#define SPLITSET_ID         0
#define COLSET_ID           1
#define INDEXWIN_ID         2
#define TEXTWIN_ID          3

#define TOOLBOX_OFFSET      3

#define TBI_INDEX           1001
#define TBI_BACKWARD        1002
#define TBI_FORWARD         1003
#define TBI_START           1004
#define TBI_PRINT           1005
#define TBI_COPY            1006
#define TBI_BOOKMARKS       1007
#define TBI_SEARCHDIALOG    1008
#define TBI_SOURCEVIEW      1009
#define TBI_SELECTIONMODE   1010

#define CONFIGNAME_HELPWIN      DEFINE_CONST_UNICODE("OfficeHelp")
#define CONFIGNAME_INDEXWIN     DEFINE_CONST_UNICODE("OfficeHelpIndex")
#define CONFIGNAME_SEARCHPAGE   DEFINE_CONST_UNICODE("OfficeHelpSearch")
#define IMAGE_URL               DEFINE_CONST_UNICODE("private:factory/")

#define PROPERTY_KEYWORDLIST    DEFINE_CONST_OUSTRING("KeywordList")
#define PROPERTY_KEYWORDREF     DEFINE_CONST_OUSTRING("KeywordRef")
#define PROPERTY_ANCHORREF      DEFINE_CONST_OUSTRING("KeywordAnchorForRef")
#define PROPERTY_TITLEREF       DEFINE_CONST_OUSTRING("KeywordTitleForRef")
#define PROPERTY_TITLE          DEFINE_CONST_OUSTRING("Title")
#define HELP_URL                DEFINE_CONST_OUSTRING("vnd.sun.star.help://")
#define HELP_SEARCH_TAG         DEFINE_CONST_OUSTRING("/?Query=")

#define USERITEM_NAME           rtl::OUString::createFromAscii( "UserItem" )

#define PARSE_URL( aURL ) \
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( \
            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY ); \
    xTrans->parseStrict( aURL )

#define GET_SLOT_NAME( nId ) \
    SFX_SLOTPOOL().GetSlotName_Impl( nId )

//.........................................................................
namespace sfx2
{
//.........................................................................

    void HandleTaskPaneList( Window* pWindow, BOOL bAddToList )
    {
        Window* pParent = pWindow->GetParent();
        DBG_ASSERT( pParent, "HandleTaskPaneList(): every window here should have a parent" );

        SystemWindow* pSysWin = pParent->GetSystemWindow();
        if( pSysWin )
        {
            TaskPaneList* pTaskPaneList = pSysWin->GetTaskPaneList();
            if( pTaskPaneList )
            {
                if( bAddToList )
                    pTaskPaneList->AddWindow( pWindow );
                else
                    pTaskPaneList->RemoveWindow( pWindow );
            }
        }
    }

    /** Prepare a search string for searching or selecting.
        For searching every search word needs the postfix '*' and the delimiter ' ' if necessary.
        For selecting the delimiter '|' is required to search with regular expressions.
        Samples:
        search string | output for searching | output for selecting
        -----------------------------------------------------------
        "text"        | "text*"              | "text"
        "text*"       | "text*"              | "text"
        "text menu"   | "text* menu*"        | "text|menu"
    */
    String PrepareSearchString( const String& rSearchString,
                                Reference< XBreakIterator > xBreak, bool bForSearch )
    {
        String sSearchStr;
        sal_Int32 nStartPos = 0;
        const Locale aLocale = Application::GetSettings().GetUILocale();
        Boundary aBoundary = xBreak->getWordBoundary(
            rSearchString, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES, sal_True );

        while ( aBoundary.startPos != aBoundary.endPos )
        {
            nStartPos = aBoundary.startPos;
            String sSearchToken( rSearchString.Copy(
                (USHORT)aBoundary.startPos, (USHORT)aBoundary.endPos - (USHORT)aBoundary.startPos ) );
            if ( bForSearch && sSearchToken.GetChar( sSearchToken.Len() - 1 ) != '*' )
                sSearchToken += '*';

            if ( sSearchToken.Len() > 1 ||
                 ( sSearchToken.Len() > 0 && sSearchToken.GetChar( 0 ) != '*' ) )
            {
                if ( sSearchStr.Len() > 0 )
                {
                    if ( bForSearch )
                        sSearchStr += ' ';
                    else
                        sSearchStr += '|';
                }
                sSearchStr += sSearchToken;
            }
            aBoundary = xBreak->nextWord( rSearchString, nStartPos,
                                          aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
        }

        return sSearchStr;
    }
//.........................................................................
// namespace sfx2
}
//.........................................................................

// struct IndexEntry_Impl ------------------------------------------------

struct IndexEntry_Impl
{
    sal_Bool        m_bSubEntry;
    String          m_aURL;

    IndexEntry_Impl( const String& rURL, sal_Bool bSubEntry ) :
        m_bSubEntry( bSubEntry ), m_aURL( rURL ) {}
};

#define NEW_ENTRY( url, bool ) \
    (void*)(ULONG)( new IndexEntry_Impl( url, bool ) )

// struct ContentEntry_Impl ----------------------------------------------

struct ContentEntry_Impl
{
    String      aURL;
    sal_Bool    bIsFolder;

    ContentEntry_Impl( const String& rURL, sal_Bool bFolder ) :
        aURL( rURL ), bIsFolder( bFolder ) {}
};

// ContentListBox_Impl ---------------------------------------------------

ContentListBox_Impl::ContentListBox_Impl( Window* pParent, const ResId& rResId ) :

    SvTreeListBox( pParent, rResId ),

    aOpenBookImage      ( SfxResId( IMG_HELP_CONTENT_BOOK_OPEN ) ),
    aClosedBookImage    ( SfxResId( IMG_HELP_CONTENT_BOOK_CLOSED ) ),
    aOpenChapterImage   ( SfxResId( IMG_HELP_CONTENT_CHAPTER_OPEN ) ),
    aClosedChapterImage ( SfxResId( IMG_HELP_CONTENT_CHAPTER_CLOSED ) ),
    aDocumentImage      ( SfxResId( IMG_HELP_CONTENT_DOC ) )

{
    SetWindowBits( WB_HIDESELECTION | WB_HSCROLL );

    SetEntryHeight( 16 );
    SetSelectionMode( SINGLE_SELECTION );
    SetSpaceBetweenEntries( 2 );
    SetNodeBitmaps( aClosedBookImage, aOpenBookImage );

    SetSublistOpenWithReturn();
    SetSublistOpenWithLeftRight();

    InitRoot();
}

// -----------------------------------------------------------------------

ContentListBox_Impl::~ContentListBox_Impl()
{
    USHORT nPos = 0;
    SvLBoxEntry* pEntry = GetEntry( nPos++ );
    while ( pEntry )
    {
        ::rtl::OUString aTemp( GetEntryText( pEntry ) );
        ClearChildren( pEntry );
        delete (ContentEntry_Impl*)pEntry->GetUserData();
        pEntry = GetEntry( nPos++ );
    }
}

// -----------------------------------------------------------------------

void ContentListBox_Impl::InitRoot()
{
    String aHelpTreeviewURL( DEFINE_CONST_UNICODE("vnd.sun.star.hier://com.sun.star.help.TreeView/") );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aList =
        SfxContentHelper::GetHelpTreeViewContents( aHelpTreeviewURL );

    const ::rtl::OUString* pEntries  = aList.getConstArray();
    UINT32 i, nCount = aList.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pEntries[i] );
        String aTitle, aURL;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aURL = aRow.GetToken( 0, '\t', nIdx );
        sal_Unicode cFolder = aRow.GetToken( 0, '\t', nIdx ).GetChar(0);
        sal_Bool bIsFolder = ( '1' == cFolder );
        SvLBoxEntry* pEntry = InsertEntry( aTitle, aOpenBookImage, aClosedBookImage, NULL, TRUE );
        if ( bIsFolder )
            pEntry->SetUserData( new ContentEntry_Impl( aURL, sal_True ) );
    }
}

// -----------------------------------------------------------------------

void ContentListBox_Impl::ClearChildren( SvLBoxEntry* pParent )
{
    SvLBoxEntry* pEntry = FirstChild( pParent );
    while ( pEntry )
    {
        ::rtl::OUString aTemp( GetEntryText( pEntry ) );
        ClearChildren( pEntry );
        delete (ContentEntry_Impl*)pEntry->GetUserData();
        pEntry = NextSibling( pEntry );
    }
}

// -----------------------------------------------------------------------

void ContentListBox_Impl::RequestingChilds( SvLBoxEntry* pParent )
{
    try
    {
        if ( !pParent->HasChilds() )
        {
            if ( pParent->GetUserData() )
            {
                String aURL( ( (ContentEntry_Impl*)pParent->GetUserData()  )->aURL );
                ::com::sun::star::uno::Sequence< ::rtl::OUString > aList =
                    SfxContentHelper::GetHelpTreeViewContents( aURL );

                const ::rtl::OUString* pEntries  = aList.getConstArray();
                UINT32 i, nCount = aList.getLength();
                for ( i = 0; i < nCount; ++i )
                {
                    String aRow( pEntries[i] );
                    String aTitle, aURL;
                    xub_StrLen nIdx = 0;
                    aTitle = aRow.GetToken( 0, '\t', nIdx );
                    aURL = aRow.GetToken( 0, '\t', nIdx );
                    sal_Unicode cFolder = aRow.GetToken( 0, '\t', nIdx ).GetChar(0);
                    sal_Bool bIsFolder = ( '1' == cFolder );
                    SvLBoxEntry* pEntry = NULL;
                    if ( bIsFolder )
                    {
                        pEntry = InsertEntry( aTitle, aOpenChapterImage, aClosedChapterImage, pParent, TRUE );
                        pEntry->SetUserData( new ContentEntry_Impl( aURL, sal_True ) );
                    }
                    else
                    {
                        pEntry = InsertEntry( aTitle, aDocumentImage, aDocumentImage, pParent );
                        Any aAny( ::utl::UCBContentHelper::GetProperty( aURL, String(RTL_CONSTASCII_USTRINGPARAM("TargetURL" ) ) ) );
                        rtl::OUString aTargetURL;
                        if ( aAny >>=  aTargetURL )
                            pEntry->SetUserData( new ContentEntry_Impl( aTargetURL, sal_False ) );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "ContentListBox_Impl::RequestingChilds(): unexpected exception" );
    }
}

// -----------------------------------------------------------------------

long ContentListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( rNEvt.GetType() == EVENT_KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( NULL );
        bHandled = sal_True;
    }

    return bHandled ? 1 : SvTreeListBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

String ContentListBox_Impl::GetSelectEntry() const
{
    String aRet;
    SvLBoxEntry* pEntry = FirstSelected();
    if ( pEntry && !( (ContentEntry_Impl*)pEntry->GetUserData()  )->bIsFolder )
        aRet = ( (ContentEntry_Impl*)pEntry->GetUserData() )->aURL;
    return aRet;
}

// class HelpTabPage_Impl ------------------------------------------------

HelpTabPage_Impl::HelpTabPage_Impl(
    Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin, const ResId& rResId ) :

    TabPage( pParent, rResId ),

    m_pIdxWin( _pIdxWin )

{
}

// class ContentTabPage_Impl ---------------------------------------------

ContentTabPage_Impl::ContentTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin ) :

    HelpTabPage_Impl( pParent, _pIdxWin, SfxResId( TP_HELP_CONTENT ) ),

    aContentBox( this, ResId( LB_CONTENTS ) )

{
    FreeResource();

    aContentBox.Show();
}

// -----------------------------------------------------------------------

void ContentTabPage_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    aSize.Width() -= 8;
    aSize.Height() -= 8;
    aContentBox.SetPosSizePixel( Point( 4, 4 ), aSize );
}

// -----------------------------------------------------------------------

void ContentTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

// -----------------------------------------------------------------------

Control* ContentTabPage_Impl::GetLastFocusControl()
{
    return &aContentBox;
}

// class IndexBox_Impl ---------------------------------------------------

IndexBox_Impl::IndexBox_Impl( Window* pParent, const ResId& rResId ) :

    ComboBox( pParent, rResId )

{
    EnableAutocomplete( TRUE );
    EnableUserDraw( TRUE );
}

// -----------------------------------------------------------------------

void IndexBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    IndexEntry_Impl* pEntry = (IndexEntry_Impl*)(ULONG)GetEntryData( rUDEvt.GetItemId() );
    if ( pEntry && pEntry->m_bSubEntry )
    {
        // indent sub entries
        Point aPos( rUDEvt.GetRect().TopLeft() );
        aPos.X() += 8;
        aPos.Y() += ( rUDEvt.GetRect().GetHeight() - rUDEvt.GetDevice()->GetTextHeight() ) / 2;
        String aEntry( GetEntry( rUDEvt.GetItemId() ) );
        USHORT nPos = aEntry.Search( ';' );
        rUDEvt.GetDevice()->DrawText( aPos, ( nPos != STRING_NOTFOUND ) ? aEntry.Copy( nPos + 1 ) : aEntry );
    }
    else
        DrawEntry( rUDEvt, FALSE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

long IndexBox_Impl::Notify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( rNEvt.GetType() == EVENT_KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( NULL );
        bHandled = sal_True;
    }

    return bHandled ? 1 : ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void IndexBox_Impl::SelectExecutableEntry()
{
    sal_Bool bSelectNew = sal_False;
    USHORT nPos = GetEntryPos( GetText() );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        USHORT nOldPos = nPos;
        String aEntryText;
        IndexEntry_Impl* pEntry = (IndexEntry_Impl*)(ULONG)GetEntryData( nPos );
        USHORT nCount = GetEntryCount();
        while ( nPos < nCount && ( !pEntry || pEntry->m_aURL.Len() == 0 ) )
        {
            pEntry = (IndexEntry_Impl*)(ULONG)GetEntryData( ++nPos );
            aEntryText = GetEntry( nPos );
        }

        if ( nOldPos != nPos )
            SetText( aEntryText );
    }
}

// class IndexTabPage_Impl -----------------------------------------------

IndexTabPage_Impl::IndexTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin ) :

    HelpTabPage_Impl( pParent, _pIdxWin, SfxResId( TP_HELP_INDEX ) ),

    aExpressionFT   ( this, ResId( FT_EXPRESSION ) ),
    aIndexCB        ( this, ResId( CB_INDEX ) ),
    aOpenBtn        ( this, ResId( PB_OPEN_INDEX ) ),

    bIsActivated    ( sal_False )

{
    FreeResource();

    aOpenBtn.SetClickHdl( LINK( this, IndexTabPage_Impl, OpenHdl ) );
    Link aTimeoutLink = LINK( this, IndexTabPage_Impl, TimeoutHdl );
    aFactoryTimer.SetTimeoutHdl( aTimeoutLink );
    aFactoryTimer.SetTimeout( 300 );
    aKeywordTimer.SetTimeoutHdl( aTimeoutLink );
    aFactoryTimer.SetTimeout( 300 );

    nMinWidth = aOpenBtn.GetSizePixel().Width();
}

// -----------------------------------------------------------------------

IndexTabPage_Impl::~IndexTabPage_Impl()
{
    ClearIndex();
}

// -----------------------------------------------------------------------

namespace sfx2 {

    struct equalOUString
    {
        bool operator()( const ::rtl::OUString& rKey1, const ::rtl::OUString& rKey2 ) const
        {
            return !!( rKey1 == rKey2 );
        }
    };


    struct hashOUString
    {
        size_t operator()( const ::rtl::OUString& rName ) const
        {
            return rName.hashCode();
        }
    };

    typedef ::std::hash_map< ::rtl::OUString, int, hashOUString, equalOUString > KeywordInfo;
}

#define UNIFY_AND_INSERT_TOKEN( aToken )                                                            \
    it =                                                                                            \
    aInfo.insert( sfx2::KeywordInfo::value_type( aToken, 0 ) ).first;                               \
    if ( ( tmp = it->second++ ) != 0 )                                                              \
       nPos = aIndexCB.InsertEntry( aToken + rtl::OUString( append, tmp ) );                        \
    else                                                                                            \
       nPos = aIndexCB.InsertEntry( aToken )

#define INSERT_DATA( j )                                                                            \
    if ( aAnchorList[j].getLength() > 0 )                                                           \
    {                                                                                               \
        aData.append( aRefList[j] ).append( sal_Unicode('#') ).append( aAnchorList[j] );            \
        aIndexCB.SetEntryData( nPos, NEW_ENTRY( aData.makeStringAndClear(), insert ) );             \
    }                                                                                               \
    else                                                                                            \
        aIndexCB.SetEntryData( nPos, NEW_ENTRY( aRefList[j], insert ) );

// -----------------------------------------------------------------------

void IndexTabPage_Impl::InitializeIndex()
{
    WaitObject( this );

    // By now more than 256 equal entries are not allowed
    sal_Unicode append[256];
    for( int k = 0; k < 256; ++k )
        append[k] = sal_Unicode( ' ' );

    sfx2::KeywordInfo aInfo;
    aIndexCB.SetUpdateMode( FALSE );

    try
    {
        ::rtl::OUString aURL = HELP_URL;
        aURL += ::rtl::OUString( sFactory );

        String aTemp = aURL;
        AppendConfigToken_Impl( aTemp, sal_True );
        aURL = aTemp;

        Content aCnt( aURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
        if ( xInfo->hasPropertyByName( PROPERTY_ANCHORREF ) )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aPropSeq( 4 );
            aPropSeq[0] = PROPERTY_KEYWORDLIST;
            aPropSeq[1] = PROPERTY_KEYWORDREF;
            aPropSeq[2] = PROPERTY_ANCHORREF;
            aPropSeq[3] = PROPERTY_TITLEREF;

            // abi: use one possibly remote call only
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aAnySeq =
                  aCnt.getPropertyValues( aPropSeq );

            ::com::sun::star::uno::Sequence< ::rtl::OUString > aKeywordList;
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > aKeywordRefList;
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > aAnchorRefList;
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > aTitleRefList;

            if ( ( aAnySeq[0] >>= aKeywordList ) && ( aAnySeq[1] >>= aKeywordRefList ) &&
                 ( aAnySeq[2] >>= aAnchorRefList ) && ( aAnySeq[3] >>= aTitleRefList ) )
            {
                sal_Bool insert;
                USHORT nPos;
                int ndx,tmp;
                ::rtl::OUString aIndex, aTempString;
                ::rtl::OUStringBuffer aData( 128 );            // Capacity of up to 128 characters
                sfx2::KeywordInfo::iterator it;

                for ( int i = 0; i < aKeywordList.getLength(); ++i )
                {
                    // abi: Do not copy, but use references
                    const ::rtl::OUString& aKeywordPair = aKeywordList[i];
                    DBG_ASSERT( aKeywordPair.getLength() > 0, "invalid help index" );
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRefList = aKeywordRefList[i];
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aAnchorList = aAnchorRefList[i];
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aTitleList = aTitleRefList[i];

                    DBG_ASSERT( aRefList.getLength() == aAnchorList.getLength(),"reference list and title list of different length" );

                    insert = ( ( ndx = aKeywordPair.indexOf( sal_Unicode( ';' ) ) ) == -1 ? sal_False : sal_True );

                    if ( insert )
                    {
                        aTempString = aKeywordPair.copy( 0, ndx );
                        if ( aIndex != aTempString )
                        {
                            aIndex = aTempString;
                            UNIFY_AND_INSERT_TOKEN( aTempString );
                        }
                    }
                    else
                        aIndex = ::rtl::OUString();

                    // Assume the token is trimed
                    UNIFY_AND_INSERT_TOKEN( aKeywordPair );

                    sal_uInt32 nRefListLen = aRefList.getLength();

                    DBG_ASSERT( aAnchorList.getLength(), "*IndexTabPage_Impl::InitializeIndex(): AnchorList is empty!" );           \
                    DBG_ASSERT( nRefListLen, "*IndexTabPage_Impl::InitializeIndex(): RefList is empty!" );          \

                    if ( aAnchorList.getLength() && nRefListLen )
                    {
                        INSERT_DATA( 0 );
                    }

                    for ( sal_uInt32 j = 1; j < nRefListLen ; ++j )
                    {
                        aData
                            .append( aKeywordPair )
                            .append( sal_Unicode(' ') )
                            .append( sal_Unicode('-') )
                            .append( sal_Unicode(' ') )
                            .append( aTitleList[j] );

                        aTempString = aData.makeStringAndClear();
                        UNIFY_AND_INSERT_TOKEN( aTempString );
                        INSERT_DATA( j );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "IndexTabPage_Impl::InitializeIndex(): unexpected exception" );
    }

    aIndexCB.SetUpdateMode( TRUE );

    if ( sKeyword.Len() > 0 )
        aKeywordLink.Call( this );
}

#undef INSERT_DATA
#undef UNIFY_AND_INSERT_TOKEN

// -----------------------------------------------------------------------

void IndexTabPage_Impl::ClearIndex()
{
    USHORT nCount = aIndexCB.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
        delete (IndexEntry_Impl*)(ULONG)aIndexCB.GetEntryData(i);
    aIndexCB.Clear();
}

// -----------------------------------------------------------------------

IMPL_LINK( IndexTabPage_Impl, OpenHdl, PushButton*, EMPTYARG )
{
    aIndexCB.GetDoubleClickHdl().Call( &aIndexCB );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( IndexTabPage_Impl, TimeoutHdl, Timer*, pTimer )
{
    if ( &aFactoryTimer == pTimer )
        InitializeIndex();
    else if ( &aKeywordTimer == pTimer && sKeyword.Len() > 0 )
        aKeywordLink.Call( this );
    return 0;
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::Resize()
{
    Size aSize = GetSizePixel();
    if ( aSize.Width() < nMinWidth )
        aSize.Width() = nMinWidth;
    Point aPnt = aExpressionFT.GetPosPixel();
    long nDelta = ( aPnt.X() / 2 );
    Size aNewSize = aExpressionFT.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aExpressionFT.SetSizePixel( aNewSize );

    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aBtnSize = aOpenBtn.GetSizePixel();

    aPnt = aIndexCB.GetPosPixel();
    aNewSize = aIndexCB.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aNewSize.Height() = aSize.Height() - aPnt.Y();
    aNewSize.Height() -= ( aBtnSize.Height() + ( a6Size.Height() * 3 / 2 ) );
    aIndexCB.SetSizePixel( aNewSize );

    aPnt.X() += ( aNewSize.Width() - aBtnSize.Width() );
    aPnt.Y() += aNewSize.Height() + ( a6Size.Height() / 2 );
    long nMinX = aIndexCB.GetPosPixel().X();
    if ( aPnt.X() < nMinX )
        aPnt.X() = nMinX;
    aOpenBtn.SetPosPixel( aPnt );
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::ActivatePage()
{
    if ( !bIsActivated )
    {
        bIsActivated = sal_True;
        aFactoryTimer.Start();
    }

    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

// -----------------------------------------------------------------------

Control* IndexTabPage_Impl::GetLastFocusControl()
{
    return &aOpenBtn;
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aIndexCB.SetDoubleClickHdl( rLink );
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::SetFactory( const String& rFactory )
{
    DBG_ASSERT( rFactory.Len() > 0, "empty factory" );

    if ( rFactory != sFactory )
    {
        sFactory = rFactory;
        ClearIndex();
        if ( bIsActivated )
            aFactoryTimer.Start();
    }
}

// -----------------------------------------------------------------------

String IndexTabPage_Impl::GetSelectEntry() const
{
    String aRet;
    IndexEntry_Impl* pEntry = (IndexEntry_Impl*)(ULONG)aIndexCB.GetEntryData( aIndexCB.GetEntryPos( aIndexCB.GetText() ) );
    if ( pEntry )
        aRet = pEntry->m_aURL;
    return aRet;
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::SetKeyword( const String& rKeyword )
{
    sKeyword = rKeyword;

    if ( aIndexCB.GetEntryCount() > 0 )
        aKeywordTimer.Start();
    else if ( !bIsActivated )
        aFactoryTimer.Start();
}

// -----------------------------------------------------------------------

sal_Bool IndexTabPage_Impl::HasKeyword() const
{
    sal_Bool bRet = sal_False;
    if ( sKeyword.Len() > 0 )
    {
        USHORT nPos = aIndexCB.GetEntryPos( sKeyword );
        bRet = ( nPos != LISTBOX_ENTRY_NOTFOUND );
    }

    return bRet;
}

// -----------------------------------------------------------------------
//added by BerryJia for fixing Bug98251, 2002-12-11
sal_Bool IndexTabPage_Impl::HasKeywordIgnoreCase()
{
    sal_Bool bRet = sal_False;
    if ( sKeyword.Len() > 0 )
    {
        USHORT nEntries = aIndexCB.GetEntryCount();
        String sIndexItem;
        const vcl::I18nHelper& rI18nHelper = GetSettings().GetLocaleI18nHelper();
        for ( USHORT n = 0; n < nEntries; n++)
        {
            sIndexItem = aIndexCB.GetEntry( n );
            if (rI18nHelper.MatchString( sIndexItem, sKeyword ))
            {
                sKeyword = sIndexItem;
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

void IndexTabPage_Impl::OpenKeyword()
{
    if ( sKeyword.Len() > 0 )
    {
        aIndexCB.SetText( sKeyword );
        aIndexCB.GetDoubleClickHdl().Call( NULL );
        sKeyword.Erase();
    }
}

// class SearchBox_Impl --------------------------------------------------

long SearchBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( !IsInDropDown() &&
         rNEvt.GetWindow() == GetSubEdit() &&
         rNEvt.GetType() == EVENT_KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        aSearchLink.Call( NULL );
        bHandled = sal_True;
    }
    return bHandled ? 1 : ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void SearchBox_Impl::Select()
{
    if ( !IsTravelSelect() )
        aSearchLink.Call( NULL );
}

// class SearchResultsBox_Impl -------------------------------------------

long SearchResultsBox_Impl::Notify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( rNEvt.GetType() == EVENT_KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( NULL );
        bHandled = sal_True;
    }

    return bHandled ? 1 : ListBox::Notify( rNEvt );
}

// class SearchTabPage_Impl ----------------------------------------------

SearchTabPage_Impl::SearchTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin ) :

    HelpTabPage_Impl( pParent, _pIdxWin, SfxResId( TP_HELP_SEARCH ) ),

    aSearchFT       ( this, ResId( FT_SEARCH ) ),
    aSearchED       ( this, ResId( ED_SEARCH ) ),
    aSearchBtn      ( this, ResId( PB_SEARCH ) ),
    aFullWordsCB    ( this, ResId( CB_FULLWORDS ) ),
    aScopeCB        ( this, ResId( CB_SCOPE ) ),
    aResultsLB      ( this, ResId( LB_RESULT ) ),
    aOpenBtn        ( this, ResId( PB_OPEN_SEARCH ) ),
    xBreakIterator  ( vcl::unohelper::CreateBreakIterator() )

{
    FreeResource();

    Link aLink = LINK( this, SearchTabPage_Impl, SearchHdl );
    aSearchED.SetSearchLink( aLink );
    aSearchBtn.SetClickHdl( aLink );
    aSearchED.SetModifyHdl( LINK( this, SearchTabPage_Impl, ModifyHdl ) );
    aOpenBtn.SetClickHdl( LINK( this, SearchTabPage_Impl, OpenHdl ) );

    aMinSize = GetSizePixel();

    SvtViewOptions aViewOpt( E_TABPAGE, CONFIGNAME_SEARCHPAGE );
    if ( aViewOpt.Exists() )
    {
        String aUserData;
        Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
        ::rtl::OUString aTemp;
        if ( aUserItem >>= aTemp )
        {
            aUserData = String( aTemp );
            BOOL bChecked = ( 1 == aUserData.GetToken(0).ToInt32() ) ? TRUE : FALSE;
            aFullWordsCB.Check( bChecked );
            bChecked = ( 1 == aUserData.GetToken(1).ToInt32() ) ? TRUE : FALSE;
            aScopeCB.Check( bChecked );

            for ( USHORT i = 2; i < aUserData.GetTokenCount(); ++i )
            {
                String aToken = aUserData.GetToken(i);
                aSearchED.InsertEntry( INetURLObject::decode(
                    aToken, '%', INetURLObject::DECODE_WITH_CHARSET ) );
            }
        }
    }

    ModifyHdl( &aSearchED );
}

// -----------------------------------------------------------------------

SearchTabPage_Impl::~SearchTabPage_Impl()
{
    SvtViewOptions aViewOpt( E_TABPAGE, CONFIGNAME_SEARCHPAGE );
    sal_Int32 nChecked = aFullWordsCB.IsChecked() ? 1 : 0;
    String aUserData = String::CreateFromInt32( nChecked );
    aUserData += ';';
    nChecked = aScopeCB.IsChecked() ? 1 : 0;
    aUserData += String::CreateFromInt32( nChecked );
    aUserData += ';';
    USHORT nCount = Min( aSearchED.GetEntryCount(), (USHORT)10 );  // save only 10 entries

    for ( USHORT i = 0; i < nCount; ++i )
    {
        String aText = aSearchED.GetEntry(i);
        aUserData += INetURLObject::encode(
            aText, INetURLObject::PART_UNO_PARAM_VALUE, '%', INetURLObject::ENCODE_ALL );
        aUserData += ';';
    }

    aUserData.EraseTrailingChars(';');
    Any aUserItem = makeAny( ::rtl::OUString( aUserData ) );
    aViewOpt.SetUserItem( USERITEM_NAME, aUserItem );
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::ClearSearchResults()
{
    USHORT nCount = aResultsLB.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
        delete (String*)(ULONG)aResultsLB.GetEntryData(i);
    aResultsLB.Clear();
    aResultsLB.Update();
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::RememberSearchText( const String& rSearchText )
{
    for ( USHORT i = 0; i < aSearchED.GetEntryCount(); ++i )
    {
        if ( rSearchText == aSearchED.GetEntry(i) )
        {
            aSearchED.RemoveEntry(i);
            break;
        }
    }

    aSearchED.InsertEntry( rSearchText, 0 );
}

// -----------------------------------------------------------------------

IMPL_LINK( SearchTabPage_Impl, SearchHdl, PushButton*, EMPTYARG )
{
    String aSearchText = TRIM( aSearchED.GetText() );
    if ( aSearchText.Len() > 0 )
    {
        EnterWait();
        ClearSearchResults();
        RememberSearchText( aSearchText );
        String aSearchURL = HELP_URL;
        aSearchURL += aFactory;
        aSearchURL += String( HELP_SEARCH_TAG );
        if ( !aFullWordsCB.IsChecked() )
            aSearchText = sfx2::PrepareSearchString( aSearchText, xBreakIterator, true );
        aSearchURL += aSearchText;
        AppendConfigToken_Impl( aSearchURL, sal_False );
        if ( aScopeCB.IsChecked() )
            aSearchURL += DEFINE_CONST_UNICODE("&Scope=Heading");
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
        LeaveWait();

        if ( !nCount )
        {
            InfoBox aBox( this, SfxResId( RID_INFO_NOSEARCHRESULTS ) );
            aBox.SetText( String( SfxResId( STR_HELP_WINDOW_TITLE ) ) );
            aBox.Execute();
        }
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

IMPL_LINK( SearchTabPage_Impl, ModifyHdl, Edit*, EMPTYARG )
{
    String aSearchText = TRIM( aSearchED.GetText() );
    aSearchBtn.Enable( aSearchText.Len() > 0 );
    return 0;
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::Resize()
{
    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aSize = GetSizePixel();
    if ( aSize.Width() < aMinSize.Width() )
        aSize.Width() = aMinSize.Width();
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

    if ( aSize.Height() > aMinSize.Height() )
    {
        long n3Height = a6Size.Height() / 2;
        Size aBtnSize = aOpenBtn.GetSizePixel();
        long nExtraHeight = aBtnSize.Height() + n3Height;

        Point aPnt = aResultsLB.GetPosPixel();
        Size aNewSize = aResultsLB.GetSizePixel();
        aNewSize.Height() = aSize.Height() - aPnt.Y();
        aNewSize.Height() -= ( nExtraHeight + ( a6Size.Height() * 3 / 2 ) );
        aResultsLB.SetSizePixel( aNewSize );

        aPnt.X() += ( aNewSize.Width() - aBtnSize.Width() );
        aPnt.Y() += aNewSize.Height() + a6Size.Height();
        aOpenBtn.SetPosPixel( aPnt );
    }
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        aSearchED.GrabFocus();
}

// -----------------------------------------------------------------------

Control* SearchTabPage_Impl::GetLastFocusControl()
{
    return &aOpenBtn;
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aResultsLB.SetDoubleClickHdl( rLink );
}

// -----------------------------------------------------------------------

String SearchTabPage_Impl::GetSelectEntry() const
{
    String aRet;
    String* pData = (String*)(ULONG)aResultsLB.GetEntryData( aResultsLB.GetSelectEntryPos() );
    if ( pData )
        aRet = String( *pData );
    return aRet;
}

// -----------------------------------------------------------------------

void SearchTabPage_Impl::ClearPage()
{
    ClearSearchResults();
    aSearchED.SetText( String() );
}

// -----------------------------------------------------------------------

sal_Bool SearchTabPage_Impl::OpenKeyword( const String& rKeyword )
{
    sal_Bool bRet = sal_False;
    aSearchED.SetText( rKeyword );
    SearchHdl( NULL );
    if ( aResultsLB.GetEntryCount() > 0 )
    {
        // found keyword -> open it
        aResultsLB.SelectEntryPos(0);
        OpenHdl( NULL );
        bRet = sal_True;
    }

    return bRet;
}

// class BookmarksTabPage_Impl -------------------------------------------

void GetBookmarkEntry_Impl
(
    Sequence< PropertyValue >& aBookmarkEntry,
    ::rtl::OUString& rTitle,
    ::rtl::OUString& rURL
)
{
    for ( int i = 0; i < aBookmarkEntry.getLength(); i++ )
    {
        PropertyValue aValue = aBookmarkEntry[i];
        if ( aValue.Name == HISTORY_PROPERTYNAME_URL )
            aValue.Value >>= rURL;
        else if ( aValue.Name == HISTORY_PROPERTYNAME_TITLE )
            aValue.Value >>= rTitle;
    }
}

// -----------------------------------------------------------------------

BookmarksBox_Impl::BookmarksBox_Impl( Window* pParent, const ResId& rResId ) :

    ListBox( pParent, rResId )

{
}

// -----------------------------------------------------------------------

BookmarksBox_Impl::~BookmarksBox_Impl()
{
    // save bookmarks to configuration
    SvtHistoryOptions aHistOpt;
    aHistOpt.Clear( eHELPBOOKMARKS );
    rtl::OUString sEmpty;
    USHORT nCount = GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        String aTitle = GetEntry(i);
        String* pURL = (String*)(ULONG)GetEntryData(i);
        aHistOpt.AppendItem( eHELPBOOKMARKS, rtl::OUString( *pURL ), sEmpty, rtl::OUString( aTitle ), sEmpty );
        delete pURL;
    }
}

// -----------------------------------------------------------------------

void BookmarksBox_Impl::DoAction( USHORT nAction )
{
    switch ( nAction )
    {
        case MID_OPEN :
            GetDoubleClickHdl().Call( NULL );
            break;

        case MID_RENAME :
           {
            USHORT nPos = GetSelectEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                SfxAddHelpBookmarkDialog_Impl aDlg( this, sal_True );
                aDlg.SetTitle( GetEntry( nPos ) );
                if ( aDlg.Execute() == RET_OK )
                {
                    String* pURL = (String*)(ULONG)GetEntryData( nPos );
                    RemoveEntry( nPos );
                    String aImageURL = IMAGE_URL;
                    aImageURL += INetURLObject( *pURL ).GetHost();
                    nPos = InsertEntry( aDlg.GetTitle(), SvFileInformationManager::GetImage( aImageURL ) );
                    SetEntryData( nPos, (void*)(ULONG)( new String( *pURL ) ) );
                    SelectEntryPos( nPos );
                    delete pURL;
                }
            }
            break;
        }

        case MID_DELETE :
        {
            USHORT nPos = GetSelectEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                RemoveEntry( nPos );
                USHORT nCount = GetEntryCount();
                if ( nCount )
                {
                    if ( nPos >= nCount )
                        nPos = nCount - 1;
                    SelectEntryPos( nPos );
                }
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

long BookmarksBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nRet = 0;
    USHORT nType = rNEvt.GetType();
    if ( EVENT_KEYINPUT == nType )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
        if ( KEY_DELETE == nCode && GetEntryCount() > 0 )
        {
            DoAction( MID_DELETE );
            nRet = 1;
        }
        else if ( KEY_RETURN == nCode )
        {
            GetDoubleClickHdl().Call( NULL );
            nRet = 1;
        }
    }
    else if ( EVENT_COMMAND == nType )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() == COMMAND_CONTEXTMENU )
        {
            PopupMenu aMenu( SfxResId( MENU_HELP_BOOKMARKS ) );
            sal_uInt16 nId = aMenu.Execute( this, pCEvt->GetMousePosPixel() );
            if ( nId != MENU_ITEM_NOTFOUND )
                DoAction( nId );
            nRet = 1;
        }
    }

    return nRet ? nRet : ListBox::Notify( rNEvt );
}

// class BookmarksTabPage_Impl -------------------------------------------

BookmarksTabPage_Impl::BookmarksTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin ) :

    HelpTabPage_Impl( pParent, _pIdxWin, SfxResId( TP_HELP_BOOKMARKS ) ),

    aBookmarksFT    ( this, ResId( FT_BOOKMARKS ) ),
    aBookmarksBox   ( this, ResId( LB_BOOKMARKS ) ),
    aBookmarksPB    ( this, ResId( PB_BOOKMARKS ) )

{
    FreeResource();

    nMinWidth = aBookmarksPB.GetSizePixel().Width();

    aBookmarksPB.SetClickHdl( LINK( this, BookmarksTabPage_Impl, OpenHdl ) );

    // load bookmarks from configuration
    Sequence< Sequence< PropertyValue > > aBookmarkSeq;
    aBookmarkSeq = SvtHistoryOptions().GetList( eHELPBOOKMARKS );

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;

    UINT32 i, nCount = aBookmarkSeq.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetBookmarkEntry_Impl( aBookmarkSeq[i], aTitle, aURL );
        AddBookmarks( aTitle, aURL );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( BookmarksTabPage_Impl, OpenHdl, PushButton*, EMPTYARG )
{
    aBookmarksBox.GetDoubleClickHdl().Call( &aBookmarksBox );
    return 0;
}

// -----------------------------------------------------------------------

void BookmarksTabPage_Impl::Resize()
{
    Size aSize = GetSizePixel();
    if ( aSize.Width() < nMinWidth )
        aSize.Width() = nMinWidth;
    Point aPnt = aBookmarksFT.GetPosPixel();
    long nDelta = ( aPnt.X() / 2 );
    Size aNewSize = aBookmarksFT.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aBookmarksFT.SetSizePixel( aNewSize );

    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aBtnSize = aBookmarksPB.GetSizePixel();

    aPnt = aBookmarksBox.GetPosPixel();
    aNewSize = aBookmarksBox.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aNewSize.Height() = aSize.Height() - aPnt.Y();
    aNewSize.Height() -= ( aBtnSize.Height() + ( a6Size.Height() * 3 / 2 ) );
    aBookmarksBox.SetSizePixel( aNewSize );

    aPnt.X() += ( aNewSize.Width() - aBtnSize.Width() );
    aPnt.Y() += aNewSize.Height() + ( a6Size.Height() / 2 );
    long nMinX = aBookmarksBox.GetPosPixel().X();
    if ( aPnt.X() < nMinX )
        aPnt.X() = nMinX;
    aBookmarksPB.SetPosPixel( aPnt );
}

// -----------------------------------------------------------------------

void BookmarksTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

// -----------------------------------------------------------------------

Control* BookmarksTabPage_Impl::GetLastFocusControl()
{
    return &aBookmarksPB;
}

// -----------------------------------------------------------------------

void BookmarksTabPage_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aBookmarksBox.SetDoubleClickHdl( rLink );
}

// -----------------------------------------------------------------------

String BookmarksTabPage_Impl::GetSelectEntry() const
{
    String aRet;
    String* pData = (String*)(ULONG)aBookmarksBox.GetEntryData( aBookmarksBox.GetSelectEntryPos() );
    if ( pData )
        aRet = String( *pData );
    return aRet;
}

// -----------------------------------------------------------------------

void BookmarksTabPage_Impl::AddBookmarks( const String& rTitle, const String& rURL )
{
    String aImageURL = IMAGE_URL;
    aImageURL += INetURLObject( rURL ).GetHost();
    USHORT nPos = aBookmarksBox.InsertEntry( rTitle, SvFileInformationManager::GetImage( aImageURL ) );
    aBookmarksBox.SetEntryData( nPos, (void*)(ULONG)( new String( rURL ) ) );
}

// class SfxHelpIndexWindow_Impl -----------------------------------------

sal_Bool SfxHelpWindow_Impl::splitHelpURL(const ::rtl::OUString& sHelpURL,
                                                ::rtl::OUString& sFactory,
                                                ::rtl::OUString& sContent,
                                                ::rtl::OUString& sAnchor )
{
    Reference < XURLTransformer > xParser( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY_THROW );

    URL aURL;
    aURL.Complete = sHelpURL;
    sal_Bool bResult = xParser->parseStrict(aURL);

    sFactory = aURL.Server;
    sContent = aURL.Path.copy(1); // strip "/"!
    sAnchor  = aURL.Mark;

    return bResult;
}

::rtl::OUString SfxHelpWindow_Impl::buildHelpURL(const ::rtl::OUString& sFactory        ,
                                                 const ::rtl::OUString& sContent        ,
                                                 const ::rtl::OUString& sAnchor         ,
                                                       sal_Bool         bUseQuestionMark)
{
    ::rtl::OUStringBuffer sHelpURL(256);
    sHelpURL.append(HELP_URL);
    sHelpURL.append(sFactory);
    sHelpURL.append(sContent);
    String sURL = String(sHelpURL.makeStringAndClear());
    AppendConfigToken_Impl(sURL, bUseQuestionMark);
    if (sAnchor.getLength())
        sURL += String(sAnchor);
    return ::rtl::OUString(sURL);
}

void SfxHelpWindow_Impl::loadHelpContent(const ::rtl::OUString& sHelpURL, sal_Bool bAddToHistory)
{
    Reference< XComponentLoader > xLoader(getTextFrame(), UNO_QUERY);
    if (!xLoader.is())
        return;

    // save url to history
    if (bAddToHistory)
        pHelpInterceptor->addURL(sHelpURL);

    if ( !IsWait() )
        EnterWait();
    sal_Bool bSuccess = sal_False;
    while(sal_True)
    {
        try
        {
            Reference< XComponent > xContent = xLoader->loadComponentFromURL(sHelpURL, DEFINE_CONST_UNICODE("_self"), 0, Sequence< PropertyValue >());
            if (xContent.is())
            {
                bSuccess = sal_True;
                break;
            }
        }
        catch(const RuntimeException&)
            { throw; }
        catch(const Exception&)
            { break; }

        /* TODO try next locale ...
                no further locale available? => break loop and show error page
        */
    }
    openDone(sHelpURL, bSuccess);
    if ( IsWait() )
        LeaveWait();
}

SfxHelpIndexWindow_Impl::SfxHelpIndexWindow_Impl( SfxHelpWindow_Impl* _pParent ) :

    Window( _pParent, SfxResId( WIN_HELP_INDEX ) ),

    aActiveLB           ( this, ResId( LB_ACTIVE ) ),
    aActiveLine         ( this, ResId( FL_ACTIVE ) ),
    aTabCtrl            ( this, ResId( TC_INDEX ) ),

    aIndexKeywordLink   ( LINK( this, SfxHelpIndexWindow_Impl, KeywordHdl ) ),
    pParentWin          ( _pParent ),

    pCPage              ( NULL ),
    pIPage              ( NULL ),
    pSPage              ( NULL ),
    pBPage              ( NULL )

{
    FreeResource();

    sfx2::AddToTaskPaneList( this );

    aTabCtrl.SetActivatePageHdl( LINK( this, SfxHelpIndexWindow_Impl, ActivatePageHdl ) );
    aTabCtrl.Show();

    sal_Int32 nPageId = HELP_INDEX_PAGE_INDEX;
    SvtViewOptions aViewOpt( E_TABDIALOG, CONFIGNAME_INDEXWIN );
    if ( aViewOpt.Exists() )
        nPageId = aViewOpt.GetPageID();
    aTabCtrl.SetCurPageId( (USHORT)nPageId );
    ActivatePageHdl( &aTabCtrl );
    aActiveLB.SetSelectHdl( LINK( this, SfxHelpIndexWindow_Impl, SelectHdl ) );
    nMinWidth = ( aActiveLB.GetSizePixel().Width() / 2 );

    aTimer.SetTimeoutHdl( LINK( this, SfxHelpIndexWindow_Impl, InitHdl ) );
    aTimer.SetTimeout( 200 );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxHelpIndexWindow_Impl::~SfxHelpIndexWindow_Impl()
{
    sfx2::RemoveFromTaskPaneList( this );

    DELETEZ( pCPage );
    DELETEZ( pIPage );
    DELETEZ( pSPage );
    DELETEZ( pBPage );

    for ( USHORT i = 0; i < aActiveLB.GetEntryCount(); ++i )
        delete (String*)(ULONG)aActiveLB.GetEntryData(i);

    SvtViewOptions aViewOpt( E_TABDIALOG, CONFIGNAME_INDEXWIN );
    aViewOpt.SetPageID( (sal_Int32)aTabCtrl.GetCurPageId() );
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
        String* pFactory = new String( INetURLObject( aURL ).GetHost() );
        USHORT nPos = aActiveLB.InsertEntry( aTitle );
        aActiveLB.SetEntryData( nPos, (void*)(ULONG)pFactory );
    }

    aActiveLB.SetDropDownLineCount( (USHORT)nCount );
    if ( aActiveLB.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
        SetActiveFactory();
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
            if ( aActiveLB.GetSelectEntryPos() != i )
            {
                aActiveLB.SelectEntryPos(i);
                aSelectFactoryLink.Call( NULL );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

HelpTabPage_Impl* SfxHelpIndexWindow_Impl::GetCurrentPage( USHORT& rCurId )
{
    rCurId = aTabCtrl.GetCurPageId();
    HelpTabPage_Impl* pPage = NULL;

    switch ( rCurId )
    {
        case HELP_INDEX_PAGE_CONTENTS:
        {
            pPage = GetContentPage();
            break;
        }

        case HELP_INDEX_PAGE_INDEX:
        {
            pPage = GetIndexPage();
            break;
        }

        case HELP_INDEX_PAGE_SEARCH:
        {
            pPage = GetSearchPage();
            break;
        }

        case HELP_INDEX_PAGE_BOOKMARKS:
        {
            pPage = GetBookmarksPage();
            break;
        }
    }

    DBG_ASSERT( pPage, "SfxHelpIndexWindow_Impl::GetCurrentPage(): no current page" );
    return pPage;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, ActivatePageHdl, TabControl *, pTabCtrl )
{
    USHORT nId = 0;
    TabPage* pPage = GetCurrentPage( nId );
    pTabCtrl->SetTabPage( nId, pPage );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, SelectHdl, ListBox *, EMPTYARG )
{
    aTimer.Start();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, InitHdl, Timer *, EMPTYARG )
{
    Initialize();

    // now use the timer for selection
    aTimer.SetTimeoutHdl( LINK( this, SfxHelpIndexWindow_Impl, SelectFactoryHdl ) );
    aTimer.SetTimeout( 1000 );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, SelectFactoryHdl, Timer *, EMPTYARG )
{
    String* pFactory = (String*)(ULONG)aActiveLB.GetEntryData( aActiveLB.GetSelectEntryPos() );
    if ( pFactory )
    {
        String aFactory( *pFactory );
        aFactory.ToLowerAscii();
        SetFactory( aFactory, sal_False );
        aSelectFactoryLink.Call( this );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpIndexWindow_Impl, KeywordHdl, IndexTabPage_Impl *, EMPTYARG )
{
    // keyword found on index?
    sal_Bool bIndex = pIPage->HasKeyword();
    //The following two lines are added by BerryJia for fixing Bug98251, 2002-12-11
    if( !bIndex)
        bIndex = pIPage->HasKeywordIgnoreCase();
    // then set index or search page as current.
    USHORT nPageId = ( bIndex ) ? HELP_INDEX_PAGE_INDEX :  HELP_INDEX_PAGE_SEARCH;
    if ( nPageId != aTabCtrl.GetCurPageId() )
    {
        aTabCtrl.SetCurPageId( nPageId );
        ActivatePageHdl( &aTabCtrl );
    }

    // at last we open the keyword
    if ( bIndex )
        pIPage->OpenKeyword();
    else if ( !pSPage->OpenKeyword( sKeyword ) )
        pParentWin->ShowStartPage();

    return 0;
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    if ( aSize.Width() < nMinWidth )
        aSize.Width() = nMinWidth;

    Point aPnt = aActiveLB.GetPosPixel();
    Size aNewSize = aActiveLB.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aActiveLB.SetSizePixel( aNewSize );
    aPnt = aActiveLine.GetPosPixel();
    aNewSize = aActiveLine.GetSizePixel();
    aNewSize.Width() = aSize.Width() - ( aPnt.X() * 2 );
    aActiveLine.SetSizePixel( aNewSize );
    aPnt = aTabCtrl.GetPosPixel();
    aNewSize = aSize;
    aSize.Width() -= aPnt.X();
    aSize.Height() -= aPnt.Y();
    aTabCtrl.SetSizePixel( aSize );
}

// -----------------------------------------------------------------------

long SfxHelpIndexWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    USHORT nType = rNEvt.GetType();
    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
         const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nCode = rKeyCode.GetCode();

        if (  KEY_TAB == nCode )
        {
            // don't exit index pane with <TAB>
            USHORT nPageId = 0;
            HelpTabPage_Impl* pCurPage = GetCurrentPage( nPageId );
            Control* pControl = pCurPage->GetLastFocusControl();
            BOOL bShift = rKeyCode.IsShift();
            BOOL bCtrl = rKeyCode.IsMod1();
            if ( !bCtrl && bShift && aActiveLB.HasChildPathFocus() )
            {
                pControl->GrabFocus();
                nDone = 1;
            }
            else if ( !bCtrl && !bShift && pControl->HasChildPathFocus() )
            {
                aActiveLB.GrabFocus();
                nDone = 1;
            }
            else if ( bCtrl )
            {
                // <STRG><TAB> moves through the pages
                if ( nPageId < HELP_INDEX_PAGE_LAST )
                    nPageId++;
                else
                    nPageId = HELP_INDEX_PAGE_FIRST;
                aTabCtrl.SetCurPageId( (USHORT)nPageId );
                ActivatePageHdl( &aTabCtrl );
                nDone = 1;
            }
         }
        else if ( aTabCtrl.HasFocus() && ( KEY_LEFT == nCode || KEY_RIGHT == nCode ) )
        {
            bWasCursorLeftOrRight = true;
        }
    }

    return nDone ? nDone : Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) ||
           ( rDCEvt.GetType() == DATACHANGED_DISPLAY ) ) &&
         ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );
    }
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::SetDoubleClickHdl( const Link& rLink )
{
    aPageDoubleClickLink = rLink;
    if ( pCPage )
        pCPage->SetOpenHdl( aPageDoubleClickLink );
    if ( pIPage )
        pIPage->SetDoubleClickHdl( aPageDoubleClickLink );
    if ( pSPage )
        pSPage->SetDoubleClickHdl( aPageDoubleClickLink );
    if ( pBPage )
        pBPage->SetDoubleClickHdl( aPageDoubleClickLink );
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::SetFactory( const String& rFactory, sal_Bool bActive )
{
    if ( rFactory.Len() > 0 )
    {
        GetIndexPage()->SetFactory( rFactory );
        GetSearchPage()->SetFactory( rFactory );
        if ( bActive )
            SetActiveFactory();
    }
}

// -----------------------------------------------------------------------

String SfxHelpIndexWindow_Impl::GetSelectEntry() const
{
    String sRet;

    switch ( aTabCtrl.GetCurPageId() )
    {
        case HELP_INDEX_PAGE_CONTENTS:
            sRet = pCPage->GetSelectEntry();
            break;

        case HELP_INDEX_PAGE_INDEX:
            sRet = pIPage->GetSelectEntry();
            break;

        case HELP_INDEX_PAGE_SEARCH:
            sRet = pSPage->GetSelectEntry();
            break;

        case HELP_INDEX_PAGE_BOOKMARKS:
            sRet = pBPage->GetSelectEntry();
            break;
    }

    return sRet;
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::AddBookmarks( const String& rTitle, const String& rURL )
{
    GetBookmarksPage()->AddBookmarks( rTitle, rURL );
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::ClearSearchPage()
{
    if ( pSPage )
        pSPage->ClearPage();
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::GrabFocusBack()
{
    if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_CONTENTS && pCPage )
        pCPage->SetFocusOnBox();
    else if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_INDEX && pIPage )
        pIPage->SetFocusOnBox();
    else if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_SEARCH && pSPage )
        pSPage->SetFocusOnBox();
    else if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_BOOKMARKS && pBPage )
        pBPage->SetFocusOnBox();
}

// -----------------------------------------------------------------------

sal_Bool SfxHelpIndexWindow_Impl::HasFocusOnEdit() const
{
    sal_Bool bRet = sal_False;
    if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_INDEX && pIPage )
        bRet = pIPage->HasFocusOnEdit();
    else if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_SEARCH && pSPage )
        bRet = pSPage->HasFocusOnEdit();
    return bRet;
}

// -----------------------------------------------------------------------

String SfxHelpIndexWindow_Impl::GetSearchText() const
{
    String sRet;
    if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_SEARCH && pSPage )
        sRet = pSPage->GetSearchText();
    return sRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxHelpIndexWindow_Impl::IsFullWordSearch() const
{
    sal_Bool bRet = sal_False;
    if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_SEARCH && pSPage )
        bRet = pSPage->IsFullWordSearch();
    return bRet;
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::OpenKeyword( const String& rKeyword )
{
    sKeyword = rKeyword;
    DBG_ASSERT( pIPage, "invalid index page" );
    pIPage->SetKeyword( sKeyword );
}

// -----------------------------------------------------------------------

void SfxHelpIndexWindow_Impl::SelectExecutableEntry()
{
    if ( aTabCtrl.GetCurPageId() == HELP_INDEX_PAGE_INDEX && pIPage )
        pIPage->SelectExecutableEntry();
}

// class TextWin_Impl ----------------------------------------------------

TextWin_Impl::TextWin_Impl( Window* p ) : DockingWindow( p, 0 )
{
}

TextWin_Impl::~TextWin_Impl()
{
}

long TextWin_Impl::Notify( NotifyEvent& rNEvt )
{
    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB )
        return GetParent()->Notify( rNEvt );
    else
        return DockingWindow::Notify( rNEvt );
}

// class SfxHelpTextWindow_Impl ------------------------------------------

SfxHelpTextWindow_Impl::SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent ) :

    Window( pParent, WB_CLIPCHILDREN | WB_TABSTOP ),

    aToolBox            ( this, 0 ),
    aIndexOnImage       ( SfxResId( IMG_HELP_TOOLBOX_INDEX_ON ) ),
    aIndexOffImage      ( SfxResId( IMG_HELP_TOOLBOX_INDEX_OFF ) ),
    aIndexOnText        ( SfxResId( STR_HELP_BUTTON_INDEX_ON ) ),
    aIndexOffText       ( SfxResId( STR_HELP_BUTTON_INDEX_OFF ) ),
    pHelpWin            ( pParent ),
    pTextWin            ( new TextWin_Impl( this ) ),
    pSrchDlg            ( NULL ),
    bIsDebug            ( sal_False ),
    bIsIndexOn          ( sal_False ),
    bIsInClose          ( sal_False ),
    bIsFullWordSearch   ( sal_False )

{
    sfx2::AddToTaskPaneList( &aToolBox );

    xFrame = Reference < XFrame > ( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( pTextWin ) );
    xFrame->setName( DEFINE_CONST_UNICODE("OFFICE_HELP") );

    aToolBox.SetHelpId( HID_HELP_TOOLBOX );

    aToolBox.InsertItem( TBI_INDEX, aIndexOffText );
    aToolBox.SetHelpId( TBI_INDEX, HID_HELP_TOOLBOXITEM_INDEX );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TBI_BACKWARD, String( SfxResId( STR_HELP_BUTTON_PREV ) ) );
    aToolBox.SetHelpId( TBI_BACKWARD, HID_HELP_TOOLBOXITEM_BACKWARD );
    aToolBox.InsertItem( TBI_FORWARD, String( SfxResId( STR_HELP_BUTTON_NEXT ) ) );
    aToolBox.SetHelpId( TBI_FORWARD, HID_HELP_TOOLBOXITEM_FORWARD );
    aToolBox.InsertItem( TBI_START, String( SfxResId( STR_HELP_BUTTON_START ) ) );
    aToolBox.SetHelpId( TBI_START, HID_HELP_TOOLBOXITEM_START );
    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TBI_PRINT, String( SfxResId( STR_HELP_BUTTON_PRINT ) ) );
    aToolBox.SetHelpId( TBI_PRINT, HID_HELP_TOOLBOXITEM_PRINT );
    aToolBox.InsertItem( TBI_BOOKMARKS, String( SfxResId( STR_HELP_BUTTON_ADDBOOKMARK ) ) );
    aToolBox.SetHelpId( TBI_BOOKMARKS, HID_HELP_TOOLBOXITEM_BOOKMARKS );
    aToolBox.InsertItem( TBI_SEARCHDIALOG, String( SfxResId( STR_HELP_BUTTON_SEARCHDIALOG ) ) );
    aToolBox.SetHelpId( TBI_SEARCHDIALOG, HID_HELP_TOOLBOXITEM_SEARCHDIALOG );
    InitToolBoxImages();
    aToolBox.Show();

    aSelectTimer.SetTimeoutHdl( LINK( this, SfxHelpTextWindow_Impl, SelectHdl ) );
    aSelectTimer.SetTimeout( 1000 );

    char* pEnv = getenv( "help_debug" );
    if ( pEnv )
        bIsDebug = sal_True;

    SvtMiscOptions().AddListener( LINK( this, SfxHelpTextWindow_Impl, NotifyHdl ) );
}

// -----------------------------------------------------------------------

SfxHelpTextWindow_Impl::~SfxHelpTextWindow_Impl()
{
    sfx2::RemoveFromTaskPaneList( &aToolBox );

    bIsInClose = sal_True;
    SvtMiscOptions().RemoveListener( LINK( this, SfxHelpTextWindow_Impl, NotifyHdl ) );
    delete pSrchDlg;
}

// -----------------------------------------------------------------------

sal_Bool SfxHelpTextWindow_Impl::HasSelection() const
{
    // is there any selection in the text and not only a cursor?
    sal_Bool bRet = sal_False;
    Reference < XTextRange > xRange = getCursor();
    if ( xRange.is() )
    {
        Reference < XText > xText = xRange->getText();
        Reference < XTextCursor > xCursor = xText->createTextCursorByRange( xRange );
        bRet = !xCursor->isCollapsed();
    }

    return bRet;
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::InitToolBoxImages()
{
    sal_Bool bLarge = ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );
    sal_Bool bHiContrast = GetBackground().GetColor().IsDark();

    aIndexOnImage = Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_INDEX_ON : IMG_HELP_TOOLBOX_L_INDEX_ON
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_INDEX_ON : IMG_HELP_TOOLBOX_INDEX_ON ) );
    aIndexOffImage = Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_INDEX_OFF : IMG_HELP_TOOLBOX_L_INDEX_OFF
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_INDEX_OFF : IMG_HELP_TOOLBOX_INDEX_OFF ) );
    aToolBox.SetItemImage( TBI_INDEX, bIsIndexOn ? aIndexOffImage : aIndexOnImage );

    aToolBox.SetItemImage( TBI_BACKWARD, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_PREV : IMG_HELP_TOOLBOX_L_PREV
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_PREV : IMG_HELP_TOOLBOX_PREV ) ) );
    aToolBox.SetItemImage( TBI_FORWARD, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_NEXT : IMG_HELP_TOOLBOX_L_NEXT
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_NEXT : IMG_HELP_TOOLBOX_NEXT ) ) );
    aToolBox.SetItemImage( TBI_START, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_START : IMG_HELP_TOOLBOX_L_START
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_START : IMG_HELP_TOOLBOX_START ) ) );
    aToolBox.SetItemImage( TBI_PRINT, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_PRINT : IMG_HELP_TOOLBOX_L_PRINT
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_PRINT : IMG_HELP_TOOLBOX_PRINT ) ) );
    aToolBox.SetItemImage( TBI_BOOKMARKS, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_BOOKMARKS : IMG_HELP_TOOLBOX_L_BOOKMARKS
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_BOOKMARKS : IMG_HELP_TOOLBOX_BOOKMARKS ) ) );
    aToolBox.SetItemImage( TBI_SEARCHDIALOG, Image( SfxResId(
        bLarge ? bHiContrast ? IMG_HELP_TOOLBOX_HCL_SEARCHDIALOG : IMG_HELP_TOOLBOX_L_SEARCHDIALOG
               : bHiContrast ? IMG_HELP_TOOLBOX_HC_SEARCHDIALOG : IMG_HELP_TOOLBOX_SEARCHDIALOG ) ) );

    Size aSize = aToolBox.CalcWindowSizePixel();
    aSize.Height() += TOOLBOX_OFFSET;
    aToolBox.SetPosSizePixel( Point( 0, TOOLBOX_OFFSET ), aSize );

    SvtMiscOptions aMiscOptions;
    if ( aMiscOptions.GetToolboxStyle() != aToolBox.GetOutStyle() )
        aToolBox.SetOutStyle( aMiscOptions.GetToolboxStyle() );
}

// -----------------------------------------------------------------------

Reference< XBreakIterator > SfxHelpTextWindow_Impl::GetBreakIterator()
{
    if ( !xBreakIterator.is() )
        xBreakIterator = vcl::unohelper::CreateBreakIterator();
    DBG_ASSERT( xBreakIterator.is(), "Could not create BreakIterator" );
    return xBreakIterator;
}

// -----------------------------------------------------------------------

Reference< XTextRange > SfxHelpTextWindow_Impl::getCursor() const
{
    // return the current cursor
    Reference< XTextRange > xCursor;

    try
    {
        Reference < XSelectionSupplier > xSelSup( xFrame->getController(), UNO_QUERY );
        if ( xSelSup.is() )
        {
            Any aAny = xSelSup->getSelection();
            Reference < XIndexAccess > xSelection;
            if ( aAny >>= xSelection )
            {
                if ( xSelection->getCount() == 1 )
                {
                    aAny = xSelection->getByIndex(0);
                    aAny >>= xCursor;
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "SfxHelpTextWindow_Impl::getCursor(): unexpected exception" );
    }

    return xCursor;
}

// -----------------------------------------------------------------------

bool SfxHelpTextWindow_Impl::isHandledKey( const KeyCode& _rKeyCode )
{
    bool bRet = false;
    USHORT nCode = _rKeyCode.GetCode();

    // the keys <STRG><A> (select all), <STRG><C> (copy),
    //          <STRG><F> (find), <STRG><P> (print) and <STRG><W> (close window)
    // were handled in help
    if ( _rKeyCode.IsMod1() &&
         ( KEY_A == nCode || KEY_C == nCode || KEY_F == nCode || KEY_P == nCode || KEY_W == nCode ) )
    {
        if ( KEY_F == nCode )
            DoSearch();
        else
            bRet = true;
    }

    return bRet;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpTextWindow_Impl, SelectHdl, Timer*, EMPTYARG )
{
    try
    {
        // select the words, which are equal to the search text of the search page
        Reference < XController > xController = xFrame->getController();
        if ( xController.is() )
        {
            // get document
            Reference < XSearchable > xSearchable( xController->getModel(), UNO_QUERY );
            if ( xSearchable.is() )
            {
                // create descriptor, set string and find all words
                Reference < XSearchDescriptor > xSrchDesc = xSearchable->createSearchDescriptor();
                Reference < XPropertySet > xPropSet( xSrchDesc, UNO_QUERY );
                xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("SearchRegularExpression"),
                                            makeAny( sal_Bool( sal_True ) ) );
                if ( bIsFullWordSearch )
                    xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("SearchWords"),
                                                makeAny( sal_Bool( sal_True ) ) );

                String sSearchString = sfx2::PrepareSearchString( aSearchText, GetBreakIterator(), false );
                xSrchDesc->setSearchString( sSearchString );
                Reference< XIndexAccess > xSelection = xSearchable->findAll( xSrchDesc );

                // then select all found words
                Reference < XSelectionSupplier > xSelectionSup( xController, UNO_QUERY );
                if ( xSelectionSup.is() )
                {
                    Any aAny;
                    aAny <<= xSelection;
                    xSelectionSup->select( aAny );
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpTextWindow_Impl, NotifyHdl, SvtMiscOptions*, pOptions )
{
    InitToolBoxImages();
    Resize();
    aToolBox.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpTextWindow_Impl, FindHdl, sfx2::SearchDialog*, pDlg )
{
    bool bWrapAround = ( NULL == pDlg );
    if ( bWrapAround )
        pDlg = pSrchDlg;
    DBG_ASSERT( pDlg, "invalid search dialog" );
    String sSearchText = pDlg->GetSearchText();
    try
    {
        // select the words, which are equal to the search text of the search page
        Reference < XController > xController = xFrame->getController();
        if ( xController.is() )
        {
            // get document
            Reference < XSearchable > xSearchable( xController->getModel(), UNO_QUERY );
            if ( xSearchable.is() )
            {
                // create descriptor, set string and find all words
                Reference < XSearchDescriptor > xSrchDesc = xSearchable->createSearchDescriptor();
                Reference < XPropertySet > xPropSet( xSrchDesc, UNO_QUERY );
                xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("SearchWords"), makeAny( sal_Bool( pDlg->IsOnlyWholeWords() != false ) ) );
                xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("SearchCaseSensitive"), makeAny( sal_Bool( pDlg->IsMarchCase() != false ) ) );
                xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("SearchBackwards"), makeAny( sal_Bool( pDlg->IsSearchBackwards() != false ) ) );
                xSrchDesc->setSearchString( sSearchText );
                Reference< XInterface > xSelection;
                Reference< XTextRange > xCursor = getCursor();

                if ( xCursor.is() )
                {
                    if ( pDlg->IsSearchBackwards() )
                        xCursor = xCursor->getStart();
                    xSelection = xSearchable->findNext( xCursor, xSrchDesc );
                }
                else
                    xSelection = xSearchable->findFirst( xSrchDesc );

                // then select the found word
                if ( xSelection.is() )
                {
                    Reference < XSelectionSupplier > xSelectionSup( xController, UNO_QUERY );
                    if ( xSelectionSup.is() )
                    {
                        Any aAny;
                        aAny <<= xSelection;
                        xSelectionSup->select( aAny );
                    }
                }
                else if ( pDlg->IsWrapAround() && !bWrapAround )
                {
                    Reference < text::XTextViewCursorSupplier > xCrsrSupp( xController, uno::UNO_QUERY );
                    Reference < text::XTextViewCursor > xTVCrsr( xCrsrSupp->getViewCursor(), uno::UNO_QUERY );
                    if ( xTVCrsr.is() )
                    {
                        Reference < text::XTextDocument > xDoc( xController->getModel(), uno::UNO_QUERY );
                        Reference < text::XText > xText = xDoc->getText();
                        if ( xText.is() )
                        {
                            if ( pDlg->IsSearchBackwards() )
                                xTVCrsr->gotoRange( xText->getEnd(), sal_False );
                            else
                                xTVCrsr->gotoRange( xText->getStart(), sal_False );
                            FindHdl( NULL );
                        }
                    }
                }
                else
                {
                    DBG_ASSERT( pSrchDlg, "no search dialog" );
                    InfoBox aBox( pSrchDlg, SfxResId( RID_INFO_NOSEARCHTEXTFOUND ) );
                    aBox.Execute();
                    pSrchDlg->SetFocusOnEdit();
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpTextWindow_Impl, CloseHdl, sfx2::SearchDialog*, EMPTYARG )
{
    delete pSrchDlg;
    pSrchDlg = NULL;
    return 0;
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    Size aSize = aToolBox.GetSizePixel();
    aSize.Width() = aWinSize.Width();
    aToolBox.SetSizePixel( aSize );

    long nToolBoxHeight = aSize.Height() + TOOLBOX_OFFSET;
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    pTextWin->SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}

// -----------------------------------------------------------------------

long SfxHelpTextWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    USHORT nType = rNEvt.GetType();
    if ( EVENT_COMMAND == nType && rNEvt.GetCommandEvent() )
    {
        const CommandEvent* pCmdEvt = rNEvt.GetCommandEvent();
        Window* pCmdWin = rNEvt.GetWindow();

        if ( pCmdEvt->GetCommand() == COMMAND_CONTEXTMENU && pCmdWin != this && pCmdWin != &aToolBox )
        {
            sal_Bool bHiContrast = GetSettings().GetStyleSettings().GetMenuColor().IsDark();
            Point aPos;
            if ( pCmdEvt->IsMouseEvent() )
                aPos = pCmdEvt->GetMousePosPixel();
            else
                aPos = Point( pTextWin->GetPosPixel().X() + 20, 20 );
            aPos.Y() += pTextWin->GetPosPixel().Y();
            PopupMenu aMenu;
            if ( bIsIndexOn )
                aMenu.InsertItem( TBI_INDEX, aIndexOffText, Image( SfxResId(
                    bHiContrast ? IMG_HELP_TOOLBOX_HC_INDEX_OFF : IMG_HELP_TOOLBOX_INDEX_OFF ) ) );
            else
                aMenu.InsertItem( TBI_INDEX, aIndexOnText, Image( SfxResId(
                    bHiContrast ? IMG_HELP_TOOLBOX_HC_INDEX_ON : IMG_HELP_TOOLBOX_INDEX_ON ) ) );
            aMenu.SetHelpId( TBI_INDEX, HID_HELP_TOOLBOXITEM_INDEX );
            aMenu.InsertSeparator();
            aMenu.InsertItem( TBI_BACKWARD, String( SfxResId( STR_HELP_BUTTON_PREV ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_PREV : IMG_HELP_TOOLBOX_PREV ) ) );
            aMenu.SetHelpId( TBI_BACKWARD, HID_HELP_TOOLBOXITEM_BACKWARD );
            aMenu.EnableItem( TBI_BACKWARD, pHelpWin->HasHistoryPredecessor() );
            aMenu.InsertItem( TBI_FORWARD, String( SfxResId( STR_HELP_BUTTON_NEXT ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_NEXT : IMG_HELP_TOOLBOX_NEXT ) ) );
            aMenu.SetHelpId( TBI_FORWARD, HID_HELP_TOOLBOXITEM_FORWARD );
            aMenu.EnableItem( TBI_FORWARD, pHelpWin->HasHistorySuccessor() );
            aMenu.InsertItem( TBI_START, String( SfxResId( STR_HELP_BUTTON_START ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_START : IMG_HELP_TOOLBOX_START ) ) );
            aMenu.SetHelpId( TBI_START, HID_HELP_TOOLBOXITEM_START );
            aMenu.InsertSeparator();
            aMenu.InsertItem( TBI_PRINT, String( SfxResId( STR_HELP_BUTTON_PRINT ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_PRINT : IMG_HELP_TOOLBOX_PRINT ) ) );
            aMenu.SetHelpId( TBI_PRINT, HID_HELP_TOOLBOXITEM_PRINT );
            aMenu.InsertItem( TBI_BOOKMARKS, String( SfxResId( STR_HELP_BUTTON_ADDBOOKMARK ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_BOOKMARKS : IMG_HELP_TOOLBOX_BOOKMARKS ) ) );
            aMenu.SetHelpId( TBI_BOOKMARKS, HID_HELP_TOOLBOXITEM_BOOKMARKS );
            aMenu.InsertItem( TBI_SEARCHDIALOG, String( SfxResId( STR_HELP_BUTTON_SEARCHDIALOG ) ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_SEARCHDIALOG : IMG_HELP_TOOLBOX_SEARCHDIALOG ) ) );
            aMenu.SetHelpId( TBI_SEARCHDIALOG, HID_HELP_TOOLBOXITEM_SEARCHDIALOG );
            aMenu.InsertSeparator();
            aMenu.InsertItem( TBI_SELECTIONMODE, String( SfxResId( STR_HELP_MENU_TEXT_SELECTION_MODE ) ) );
            aMenu.SetHelpId( TBI_SELECTIONMODE, HID_HELP_TEXT_SELECTION_MODE );
            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            URL aURL;
            aURL.Complete = DEFINE_CONST_UNICODE(".uno:SelectTextMode");
            PARSE_URL( aURL );
            Reference < XDispatch > xDisp = xProv.is() ?
                    xProv->queryDispatch( aURL, rtl::OUString(), 0 ) : Reference < XDispatch >();
            if(xDisp.is())
            {
                HelpStatusListener_Impl* pStateListener;
                Reference<XStatusListener>xStateListener = pStateListener =
                                        new HelpStatusListener_Impl(xDisp, aURL );
                FeatureStateEvent rEvent = pStateListener->GetStateEvent();
                sal_Bool bCheck = sal_False;
                rEvent.State >>= bCheck;
                aMenu.CheckItem(TBI_SELECTIONMODE, bCheck);
            }
            aMenu.InsertSeparator();
            aMenu.InsertItem( TBI_COPY, GET_SLOT_NAME( SID_COPY ),
                Image( SfxResId( bHiContrast ? IMG_HELP_TOOLBOX_HC_COPY : IMG_HELP_TOOLBOX_COPY ) ) );
            aMenu.SetHelpId( TBI_COPY, SID_COPY );
            aMenu.EnableItem( TBI_COPY, HasSelection() );

            if ( bIsDebug )
            {
                aMenu.InsertSeparator();
                aMenu.InsertItem( TBI_SOURCEVIEW, String( SfxResId( STR_HELP_BUTTON_SOURCEVIEW ) ) );
            }

            if( SvtMenuOptions().IsEntryHidingEnabled() == sal_False )
                aMenu.SetMenuFlags( aMenu.GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );

            USHORT nId = aMenu.Execute( this, aPos );
            pHelpWin->DoAction( nId );
            nDone = 1;
        }
    }
    else if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
         const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
         const KeyCode& rKeyCode = pKEvt->GetKeyCode();
        USHORT nKeyGroup = rKeyCode.GetGroup();
        USHORT nKey = rKeyCode.GetCode();
        if ( KEYGROUP_ALPHA == nKeyGroup &&  !isHandledKey( rKeyCode ) )
        {
            // do nothing disables the writer accelerators
            nDone = 1;
         }
        else if ( rKeyCode.IsMod1() && ( KEY_F4 == nKey || KEY_W == nKey ) )
        {
            // <STRG><F4> or <STRG><W> -> close top frame
            pHelpWin->CloseWindow();
            nDone = 1;
        }
    }

    return nDone ? nDone : Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::GetFocus()
{
    if ( !bIsInClose )
    {
        try
        {
            if( xFrame.is() )
            {
                Reference< ::com::sun::star::awt::XWindow > xWindow = xFrame->getComponentWindow();
                if( xWindow.is() )
                    xWindow->setFocus();
            }
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "SfxHelpTextWindow_Impl::GetFocus(): unexpected exception" );
        }
    }
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) ||
           ( rDCEvt.GetType() == DATACHANGED_DISPLAY ) ) &&
         ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );
        InitToolBoxImages();
    }
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::ToggleIndex( sal_Bool bOn )
{
    bIsIndexOn = bOn;
    if ( bIsIndexOn )
    {
        aToolBox.SetItemImage( TBI_INDEX, aIndexOffImage );
        aToolBox.SetItemText( TBI_INDEX, aIndexOffText );
    }
    else
    {
        aToolBox.SetItemImage( TBI_INDEX, aIndexOnImage );
        aToolBox.SetItemText( TBI_INDEX, aIndexOnText );
    }
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::SelectSearchText( const String& rSearchText, sal_Bool _bIsFullWordSearch )
{
    aSearchText = rSearchText;
    bIsFullWordSearch = _bIsFullWordSearch;
    aSelectTimer.Start();
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::SetPageStyleHeaderOff() const
{
#ifdef DBG_UTIL
    sal_Bool bSetOff = sal_False;
#endif
    // set off the pagestyle header to prevent print output of the help URL
    try
    {
        Reference < XController > xController = xFrame->getController();
        Reference < XSelectionSupplier > xSelSup( xController, UNO_QUERY );
        if ( xSelSup.is() )
        {
            Reference < XIndexAccess > xSelection;
            if ( xSelSup->getSelection() >>= xSelection )
            {
                Reference < XTextRange > xRange;
                if ( xSelection->getByIndex(0) >>= xRange )
                {
                    Reference < XText > xText = xRange->getText();
                    Reference < XPropertySet > xProps( xText->createTextCursorByRange( xRange ), UNO_QUERY );
                    ::rtl::OUString sStyleName;
                    if ( xProps->getPropertyValue( DEFINE_CONST_OUSTRING("PageStyleName") ) >>= sStyleName )
                    {
                        Reference < XStyleFamiliesSupplier > xStyles( xController->getModel(), UNO_QUERY );
                        Reference < XNameContainer > xContainer;
                        if ( xStyles->getStyleFamilies()->getByName( DEFINE_CONST_OUSTRING("PageStyles") )
                             >>= xContainer )
                        {
                            Reference < XStyle > xStyle;
                            if ( xContainer->getByName( sStyleName ) >>= xStyle )
                            {
                                Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
                                xPropSet->setPropertyValue( DEFINE_CONST_OUSTRING("HeaderIsOn"),
                                                            makeAny( sal_Bool( sal_False ) ) );

                                Reference< XModifiable > xReset(xStyles, UNO_QUERY);
                                xReset->setModified(sal_False);
#ifdef DBG_UTIL
                                bSetOff = sal_True;
#endif
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "SfxHelpTextWindow_Impl::SetPageStyleHeaderOff(): unexpected exception" );
    }

#ifdef DBG_UTIL
    if ( !bSetOff )
    {
        DBG_ERRORFILE( "SfxHelpTextWindow_Impl::SetPageStyleHeaderOff(): set off failed" );
    }
#endif
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::CloseFrame()
{
    bIsInClose = sal_True;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > xCloseable  ( xFrame, ::com::sun::star::uno::UNO_QUERY );
        if (xCloseable.is())
            xCloseable->close(sal_True);
    }
    catch( ::com::sun::star::util::CloseVetoException& )
    {
    }
}

// -----------------------------------------------------------------------

void SfxHelpTextWindow_Impl::DoSearch()
{
    if ( !pSrchDlg )
    {
        // create the search dialog
        pSrchDlg = new sfx2::SearchDialog( pTextWin, DEFINE_CONST_UNICODE("HelpSearchDialog") );
        // set handler
        pSrchDlg->SetFindHdl( LINK( this, SfxHelpTextWindow_Impl, FindHdl ) );
        pSrchDlg->SetCloseHdl( LINK( this, SfxHelpTextWindow_Impl, CloseHdl ) );
        // get selected text of the help page to set it as the search text
        Reference< XTextRange > xCursor = getCursor();
        if ( xCursor.is() )
        {
            String sText = xCursor->getString();
            if ( sText.Len() > 0 )
                pSrchDlg->SetSearchText( sText );
        }
        pSrchDlg->Show();
    }
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
    static long nMinSplitSize = 5;
    static long nMaxSplitSize = 99 - nMinSplitSize;

    SplitWindow::Split();

    nIndexSize = GetItemSize( INDEXWIN_ID );
    nTextSize = GetItemSize( TEXTWIN_ID );

    BOOL        bMod = FALSE;
    if( nIndexSize < nMinSplitSize )
    {
        nIndexSize = nMinSplitSize;
        nTextSize = nMaxSplitSize;

        bMod = TRUE;
    }
    else if( nTextSize < nMinSplitSize )
    {
        nTextSize = nMinSplitSize;
        nIndexSize = nMaxSplitSize;

        bMod = TRUE;
    }
    else
        bMod = FALSE;

    if( bMod )
    {
        SetItemSize( INDEXWIN_ID, nIndexSize );
        SetItemSize( TEXTWIN_ID, nTextSize );
    }

    InitSizes();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::GetFocus()
{
    pTextWin->GrabFocus();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::MakeLayout()
{
    if ( nHeight > 0 && xWindow.is() )
    {
           Window* pScreenWin = VCLUnoHelper::GetWindow( xWindow );
           pScreenWin->Hide();

        ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
        sal_Int32 nOldWidth = bIndex ? nCollapseWidth : nExpandWidth;
        sal_Int32 nWidth = bIndex ? nExpandWidth : nCollapseWidth;
        xWindow->setPosSize( aRect.X, aRect.Y, nWidth, nHeight, ::com::sun::star::awt::PosSize::SIZE );

        if ( aRect.Width > 0 && aRect.Height > 0 )
        {
            Rectangle aScreenRect = pScreenWin->GetClientWindowExtentsRelative( NULL );
            Point aNewPos = aScreenRect.TopLeft();
            sal_Int32 nDiffWidth = nOldWidth - nWidth;
            aNewPos.X() += nDiffWidth;
            pScreenWin->SetPosPixel( aNewPos );
        }
        else if ( aWinPos.X() > 0 && aWinPos.Y() > 0 )
            pScreenWin->SetPosPixel( aWinPos );

           pScreenWin->Show();
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
     SvtViewOptions aViewOpt( E_WINDOW, CONFIGNAME_HELPWIN );
    if ( aViewOpt.Exists() )
    {
        bIndex = aViewOpt.IsVisible();
        String aUserData;
        Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
        rtl::OUString aTemp;
        if ( aUserItem >>= aTemp )
        {
            aUserData = String( aTemp );
            DBG_ASSERT( aUserData.GetTokenCount() == 6, "invalid user data" );
            USHORT nIdx = 0;
            nIndexSize = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
            nTextSize = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
            sal_Int32 nWidth = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
            nHeight = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
            aWinPos.X() = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
            aWinPos.Y() = aUserData.GetToken( 0, ';', nIdx ).ToInt32();
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

        pTextWin->ToggleIndex( bIndex );
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::SaveConfig()
{
    SvtViewOptions aViewOpt( E_WINDOW, CONFIGNAME_HELPWIN );
    sal_Int32 nW = 0, nH = 0;

    if ( xWindow.is() )
    {
        ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
        nW = aRect.Width;
        nH = aRect.Height;
    }

    aViewOpt.SetVisible( bIndex );
    String aUserData = String::CreateFromInt32( nIndexSize );
    aUserData += ';';
    aUserData += String::CreateFromInt32( nTextSize );
    aUserData += ';';
    aUserData += String::CreateFromInt32( nW );
    aUserData += ';';
    aUserData += String::CreateFromInt32( nH );

       Window* pScreenWin = VCLUnoHelper::GetWindow( xWindow );
    aWinPos = pScreenWin->GetWindowExtentsRelative( NULL ).TopLeft();
    aUserData += ';';
    aUserData += String::CreateFromInt32( aWinPos.X() );
    aUserData += ';';
    aUserData += String::CreateFromInt32( aWinPos.Y() );

    aViewOpt.SetUserItem( USERITEM_NAME, makeAny( rtl::OUString( aUserData ) ) );
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::ShowStartPage()
{
    ::rtl::OUString sHelpURL = SfxHelpWindow_Impl::buildHelpURL(pIndexWin->GetFactory(),
                                                                DEFINE_CONST_UNICODE("/start"),
                                                                ::rtl::OUString(),
                                                                sal_True);
    loadHelpContent(sHelpURL);
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, SelectHdl, ToolBox* , pToolBox )
{
    if ( pToolBox )
    {
        bGrabFocusToToolBox = pToolBox->HasChildPathFocus();
        DoAction( pToolBox->GetCurItemId() );
    }

    return 1;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, OpenHdl, SfxHelpIndexWindow_Impl* , EMPTYARG )
{
    pIndexWin->SelectExecutableEntry();
    String aEntry = pIndexWin->GetSelectEntry();

    if ( aEntry.Len() < 1 )
        return 0;

    ::rtl::OUString sHelpURL;

    INetURLObject aObj(aEntry);
    BOOL bComplete = ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP );
    if (bComplete)
        sHelpURL = ::rtl::OUString(aEntry);
    else
    {
        String aId;
        String aAnchor = String('#');
        if ( aEntry.GetTokenCount( '#' ) == 2 )
        {
            aId = aEntry.GetToken( 0, '#' );
            aAnchor += aEntry.GetToken( 1, '#' );
        }
        else
            aId = aEntry;

        aEntry  = '/';
        aEntry += aId;

        sHelpURL = SfxHelpWindow_Impl::buildHelpURL(pIndexWin->GetFactory(),
                                                    aEntry,
                                                    aAnchor,
                                                    sal_True);
    }

    loadHelpContent(sHelpURL);

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, SelectFactoryHdl, SfxHelpIndexWindow_Impl* , pWin )
{
    String aNewTitle = sTitle;
    aNewTitle += DEFINE_CONST_UNICODE(" - ");
    aNewTitle += pIndexWin->GetActiveFactoryTitle();
    GetParent()->SetText( aNewTitle );

    if ( pWin )
        ShowStartPage();
    pIndexWin->ClearSearchPage();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxHelpWindow_Impl, ChangeHdl, HelpListener_Impl*, pListener )
{
    SetFactory( pListener->GetFactory() );
    return 0;
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::openDone(const ::rtl::OUString& sURL    ,
                                        sal_Bool         bSuccess)
{
    INetURLObject aObj( sURL );
    if ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
        SetFactory( aObj.GetHost() );
    if ( IsWait() )
        LeaveWait();
    if ( bGrabFocusToToolBox )
    {
        pTextWin->GetToolBox().GrabFocus();
        bGrabFocusToToolBox = sal_False;
    }
    else
        pIndexWin->GrabFocusBack();
    if ( bSuccess )
    {
        // set some view settings: "prevent help tips" and "helpid == 68245"
        try
        {
            Reference < XController > xController = pTextWin->getFrame()->getController();
            if ( xController.is() )
            {
                Reference < XViewSettingsSupplier > xSettings( xController, UNO_QUERY );
                Reference < XPropertySet > xViewProps = xSettings->getViewSettings();
                Any aBoolAny = makeAny( sal_Bool( sal_True ) );
                xViewProps->setPropertyValue( DEFINE_CONST_OUSTRING("PreventHelpTips"), aBoolAny );
                xViewProps->setPropertyValue( DEFINE_CONST_OUSTRING("ShowGraphics"), aBoolAny );
                xViewProps->setPropertyValue( DEFINE_CONST_OUSTRING("ShowTables"), aBoolAny );
                xViewProps->setPropertyValue( DEFINE_CONST_OUSTRING("HelpURL"), makeAny( DEFINE_CONST_OUSTRING("HID:68245") ) );
                xViewProps->setPropertyValue( DEFINE_CONST_OUSTRING("IsExecuteHyperlinks"), aBoolAny );
                xController->restoreViewData(pHelpInterceptor->GetViewData());
            }
        }
        catch( Exception& )
        {
            DBG_ERROR( "SfxHelpWindow_Impl::OpenDoneHdl(): unexpected exception" );
        }

        // When the SearchPage opens the help doc, then select all words, which are equal to its text
        String sSearchText = TRIM( pIndexWin->GetSearchText() );
        if ( sSearchText.Len() > 0 )
            pTextWin->SelectSearchText( sSearchText, pIndexWin->IsFullWordSearch() );

        // no page style header -> this prevents a print output of the URL
        pTextWin->SetPageStyleHeaderOff();
    }
}

// -----------------------------------------------------------------------

SfxHelpWindow_Impl::SfxHelpWindow_Impl(
    const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParent, WinBits nBits ) :

    SplitWindow( pParent, WB_3DLOOK | WB_NOSPLITDRAW ),

    pIndexWin           ( NULL ),
    pTextWin            ( NULL ),
    pHelpInterceptor    ( new HelpInterceptor_Impl() ),
    pHelpListener       ( new HelpListener_Impl( pHelpInterceptor ) ),
    nExpandWidth        ( 0 ),
    nCollapseWidth      ( 0 ),
    nHeight             ( 0 ),
    nIndexSize          ( 40 ),
    nTextSize           ( 60 ),
    bIndex              ( sal_True ),
    bGrabFocusToToolBox ( sal_False ),
    aWinPos             ( 0, 0 ),
    sTitle              ( pParent->GetText() )

{
    SetHelpId( HID_HELP_WINDOW );
    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    pHelpInterceptor->InitWaiter( this );
    pIndexWin = new SfxHelpIndexWindow_Impl( this );
    pIndexWin->SetDoubleClickHdl( LINK( this, SfxHelpWindow_Impl, OpenHdl ) );
    pIndexWin->SetSelectFactoryHdl( LINK( this, SfxHelpWindow_Impl, SelectFactoryHdl ) );
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
    Window* pDel = pIndexWin;
    pIndexWin = NULL;
    delete pDel;

    pTextWin->CloseFrame();
    delete pTextWin;
}

// -----------------------------------------------------------------------

long SfxHelpWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        // Backward == <ALT><LEFT> or <BACKSPACE> Forward == <ALT><RIGHT>
         const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nKey = rKeyCode.GetCode();
        if ( ( rKeyCode.IsMod2() && ( KEY_LEFT == nKey || KEY_RIGHT == nKey ) ) ||
             ( !rKeyCode.GetModifier() && KEY_BACKSPACE == nKey && !pIndexWin->HasFocusOnEdit() ) )
        {
            DoAction( rKeyCode.GetCode() == KEY_RIGHT ? TBI_FORWARD : TBI_BACKWARD );
            bHandled = sal_True;
        }
        else if ( rKeyCode.IsMod1() && ( KEY_F4 == nKey || KEY_W == nKey ) )
        {
            // <STRG><F4> or <STRG><W> -> close top frame
            CloseWindow();
            bHandled = sal_True;
        }
    }
    return bHandled ? 1 : Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::setContainerWindow( Reference < ::com::sun::star::awt::XWindow > xWin )
{
    xWindow = xWin;
    MakeLayout();
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::SetFactory( const String& rFactory )
{
    pIndexWin->SetFactory( rFactory, sal_True );
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::SetHelpURL( const String& rURL )
{
    INetURLObject aObj( rURL );
    if ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
        SetFactory( aObj.GetHost() );
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::DoAction( USHORT nActionId )
{
    switch ( nActionId )
    {
        case TBI_INDEX :
        {
            bIndex = !bIndex;
            MakeLayout();
            pTextWin->ToggleIndex( bIndex );
            break;
        }

        case TBI_START :
        {
            ShowStartPage();
            break;
        }

        case TBI_BACKWARD :
        case TBI_FORWARD :
        {
            URL aURL;
            aURL.Complete = DEFINE_CONST_UNICODE(".uno:Backward");
            if ( TBI_FORWARD == nActionId )
                aURL.Complete = DEFINE_CONST_UNICODE(".uno:Forward");
            PARSE_URL( aURL );
            pHelpInterceptor->dispatch( aURL, Sequence < PropertyValue >() );
            break;
        }

        case TBI_SEARCHDIALOG :
        {
            pTextWin->DoSearch();
            break;
        }

        case TBI_PRINT :
        case TBI_SOURCEVIEW :
        case TBI_COPY :
        case TBI_SELECTIONMODE:
        {
            Reference < XDispatchProvider > xProv( pTextWin->getFrame(), UNO_QUERY );
            if ( xProv.is() )
            {
                URL aURL;
                if ( TBI_PRINT == nActionId )
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:Print");
                else if ( TBI_SOURCEVIEW == nActionId )
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:SourceView");
                else if ( TBI_COPY == nActionId )
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:Copy");
                else if ( TBI_SELECTIONMODE == nActionId )
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:SelectTextMode");
                else
                    aURL.Complete = DEFINE_CONST_UNICODE(".uno:SearchDialog");
                PARSE_URL( aURL );
                Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, String(), 0 );
                if ( xDisp.is() )
                    xDisp->dispatch( aURL, Sequence < PropertyValue >() );
            }
            break;
        }

        case TBI_BOOKMARKS :
        {
            String aURL = pHelpInterceptor->GetCurrentURL();
            if ( aURL.Len() > 0 )
            {
                try
                {
                    Content aCnt( aURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
                    if ( xInfo->hasPropertyByName( PROPERTY_TITLE ) )
                    {
                        ::com::sun::star::uno::Any aAny = aCnt.getPropertyValue( PROPERTY_TITLE );
                        rtl::OUString aValue;
                        if ( aAny >>= aValue )
                        {
                            String aTitle( aValue );
                            SfxAddHelpBookmarkDialog_Impl aDlg( this, sal_False );
                            aDlg.SetTitle( aTitle );
                            if ( aDlg.Execute() == RET_OK )
                            {
                                aTitle = aDlg.GetTitle();
                                pIndexWin->AddBookmarks( aTitle, aURL );
                            }
                        }
                    }
                }
                catch( Exception& )
                {
                    DBG_ERROR( "SfxHelpWindow_Impl::DoAction(): unexpected exception" );
                }
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::CloseWindow()
{
    try
    {
        // search for top frame
        Reference< XFramesSupplier > xCreator = getTextFrame()->getCreator();
        while ( xCreator.is() && !xCreator->isTop() )
        {
            xCreator = xCreator->getCreator();
        }

        // when found, close it
        if ( xCreator.is() && xCreator->isTop() )
        {
            Reference < XCloseable > xCloser( xCreator, UNO_QUERY );
            if ( xCloser.is() )
                xCloser->close( sal_False );
        }
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "SfxHelpWindow_Impl::CloseWindow(): caught an exception" );
    }
}

// -----------------------------------------------------------------------

void SfxHelpWindow_Impl::UpdateToolbox()
{
    pTextWin->GetToolBox().EnableItem( TBI_BACKWARD, pHelpInterceptor->HasHistoryPred() );
    pTextWin->GetToolBox().EnableItem( TBI_FORWARD, pHelpInterceptor->HasHistorySucc() );
}

// -----------------------------------------------------------------------

sal_Bool SfxHelpWindow_Impl::HasHistoryPredecessor() const
{
    return pHelpInterceptor->HasHistoryPred();
}

// -----------------------------------------------------------------------

sal_Bool SfxHelpWindow_Impl::HasHistorySuccessor() const
{
    return pHelpInterceptor->HasHistorySucc();
}

// class SfxAddHelpBookmarkDialog_Impl -----------------------------------

SfxAddHelpBookmarkDialog_Impl::SfxAddHelpBookmarkDialog_Impl( Window* pParent, sal_Bool bRename ) :

    ModalDialog( pParent, SfxResId( DLG_HELP_ADDBOOKMARK ) ),

    aTitleFT    ( this, ResId( FT_BOOKMARK_TITLE ) ),
    aTitleED    ( this, ResId( ED_BOOKMARK_TITLE ) ),
    aOKBtn      ( this, ResId( PB_BOOKMARK_OK ) ),
    aEscBtn     ( this, ResId( PB_BOOKMARK_CANCEL ) ),
    aHelpBtn    ( this, ResId( PB_BOOKMARK_HELP ) )

{
    if ( bRename )
        SetText( String( ResId( STR_BOOKMARK_RENAME ) ) );

    FreeResource();
}

// -----------------------------------------------------------------------

SfxAddHelpBookmarkDialog_Impl::~SfxAddHelpBookmarkDialog_Impl()
{
}

// -----------------------------------------------------------------------

void SfxAddHelpBookmarkDialog_Impl::SetTitle( const String& rTitle )
{
    aTitleED.SetText( rTitle );
    aTitleED.SetSelection( Selection( 0, rTitle.Len() ) );
}

