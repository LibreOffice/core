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


#include "newhelp.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/sfxresid.hxx>
#include "helpinterceptor.hxx"
#include <helper.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/app.hxx>
#include <sfxtypes.hxx>
#include "panelist.hxx"
#include <srchdlg.hxx>
#include <sfx2/sfxhelp.hxx>
#include <vcl/treelistentry.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <sfx2/strings.hrc>
#include <helpids.h>
#include <bitmaps.hlst>

#include <rtl/ustrbuf.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/ui/XDockingAreaAcceptor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <svtools/helpopt.hxx>
#include <unotools/historyoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamhelper.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>

#include <ucbhelper/content.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/ucbhelper.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>

#include <unordered_map>
#include <vector>

using namespace ::ucbhelper;
using namespace ::com::sun::star::ucb;

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
using namespace ::com::sun::star::ui;

using namespace ::comphelper;

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

#define CONFIGNAME_HELPWIN      "OfficeHelp"
#define CONFIGNAME_INDEXWIN     "OfficeHelpIndex"
#define CONFIGNAME_SEARCHPAGE   "OfficeHelpSearch"
#define IMAGE_URL               "private:factory/"

#define PROPERTY_KEYWORDLIST    "KeywordList"
#define PROPERTY_KEYWORDREF     "KeywordRef"
#define PROPERTY_ANCHORREF      "KeywordAnchorForRef"
#define PROPERTY_TITLEREF       "KeywordTitleForRef"
#define PROPERTY_TITLE          "Title"
#define HELP_URL                "vnd.sun.star.help://"
#define HELP_SEARCH_TAG         "/?Query="
#define USERITEM_NAME           "UserItem"

#define PACKAGE_SETUP           "/org.openoffice.Setup"
#define PATH_OFFICE_FACTORIES   "Office/Factories/"
#define KEY_HELP_ON_OPEN        "ooSetupFactoryHelpOnOpen"
#define KEY_UI_NAME             "ooSetupFactoryUIName"

namespace sfx2
{


    void HandleTaskPaneList( vcl::Window* pWindow, bool bAddToList )
    {
        vcl::Window* pParent = pWindow->GetParent();
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
    static OUString PrepareSearchString( const OUString& rSearchString,
                                const Reference< XBreakIterator >& xBreak, bool bForSearch )
    {
        OUStringBuffer sSearchStr;
        sal_Int32 nStartPos = 0;
        const lang::Locale aLocale = Application::GetSettings().GetUILanguageTag().getLocale();
        Boundary aBoundary = xBreak->getWordBoundary(
            rSearchString, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES, true );

        while ( aBoundary.startPos != aBoundary.endPos )
        {
            nStartPos = aBoundary.endPos;
            OUString sSearchToken( rSearchString.copy(
                static_cast<sal_uInt16>(aBoundary.startPos), static_cast<sal_uInt16>(aBoundary.endPos) - static_cast<sal_uInt16>(aBoundary.startPos) ) );
            if ( !sSearchToken.isEmpty() && ( sSearchToken.getLength() > 1 || sSearchToken[0] != '.' ) )
            {
                if ( bForSearch && sSearchToken[ sSearchToken.getLength() - 1 ] != '*' )
                    sSearchToken += "*";

                if ( sSearchToken.getLength() > 1 ||
                     ( sSearchToken.getLength() > 0 && sSearchToken[ 0 ] != '*' ) )
                {
                    if ( !sSearchStr.isEmpty() )
                    {
                        if ( bForSearch )
                            sSearchStr.append(" ");
                        else
                            sSearchStr.append("|");
                    }
                    sSearchStr.append(sSearchToken);
                }
            }
            aBoundary = xBreak->nextWord( rSearchString, nStartPos,
                                          aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
        }

        return sSearchStr.makeStringAndClear();
    }

// namespace sfx2
}


// struct IndexEntry_Impl ------------------------------------------------

struct IndexEntry_Impl
{
    bool const        m_bSubEntry;
    OUString const    m_aURL;

    IndexEntry_Impl( const OUString& rURL, bool bSubEntry ) :
        m_bSubEntry( bSubEntry ), m_aURL( rURL ) {}
};

// struct ContentEntry_Impl ----------------------------------------------

struct ContentEntry_Impl
{
    OUString const aURL;
    bool const     bIsFolder;

    ContentEntry_Impl( const OUString& rURL, bool bFolder ) :
        aURL( rURL ), bIsFolder( bFolder ) {}
};

// ContentListBox_Impl ---------------------------------------------------

ContentListBox_Impl::ContentListBox_Impl(vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , aOpenBookImage(StockImage::Yes, BMP_HELP_CONTENT_BOOK_OPEN)
    , aClosedBookImage(StockImage::Yes, BMP_HELP_CONTENT_BOOK_CLOSED)
    , aDocumentImage(StockImage::Yes, BMP_HELP_CONTENT_DOC)

{
    SetStyle( GetStyle() | WB_HIDESELECTION | WB_HSCROLL );

    SetEntryHeight( 16 );
    SetSelectionMode( SelectionMode::Single );
    SetSpaceBetweenEntries( 2 );
    SetNodeBitmaps( aClosedBookImage, aOpenBookImage );

    SetSublistOpenWithReturn();
    SetSublistOpenWithLeftRight();

    InitRoot();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeContentListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<ContentListBox_Impl>::Create(pParent, nWinStyle);
}

ContentListBox_Impl::~ContentListBox_Impl()
{
    disposeOnce();
}

void ContentListBox_Impl::dispose()
{
    sal_uInt16 nPos = 0;
    SvTreeListEntry* pEntry = GetEntry( nPos++ );
    while ( pEntry )
    {
        ClearChildren( pEntry );
        delete static_cast<ContentEntry_Impl*>(pEntry->GetUserData());
        pEntry = GetEntry( nPos++ );
    }
    SvTreeListBox::dispose();
}

void ContentListBox_Impl::InitRoot()
{
    std::vector< OUString > aList =
        SfxContentHelper::GetHelpTreeViewContents( "vnd.sun.star.hier://com.sun.star.help.TreeView/" );

    for(const OUString & aRow : aList)
    {
        sal_Int32 nIdx = 0;
        OUString aTitle = aRow.getToken( 0, '\t', nIdx );
        OUString aURL = aRow.getToken( 0, '\t', nIdx );
        sal_Unicode cFolder = aRow.getToken( 0, '\t', nIdx )[0];
        bool bIsFolder = ( '1' == cFolder );
        SvTreeListEntry* pEntry = InsertEntry( aTitle, aOpenBookImage, aClosedBookImage, nullptr, true );
        if ( bIsFolder )
            pEntry->SetUserData( new ContentEntry_Impl( aURL, true ) );
    }
}


void ContentListBox_Impl::ClearChildren( SvTreeListEntry* pParent )
{
    SvTreeListEntry* pEntry = FirstChild( pParent );
    while ( pEntry )
    {
        ClearChildren( pEntry );
        delete static_cast<ContentEntry_Impl*>(pEntry->GetUserData());
        pEntry = pEntry->NextSibling();
    }
}


void ContentListBox_Impl::RequestingChildren( SvTreeListEntry* pParent )
{
    try
    {
        if ( !pParent->HasChildren() )
        {
            if ( pParent->GetUserData() )
            {
                std::vector<OUString > aList =
                    SfxContentHelper::GetHelpTreeViewContents( static_cast<ContentEntry_Impl*>(pParent->GetUserData())->aURL );

                for (const OUString & aRow : aList)
                {
                    sal_Int32 nIdx = 0;
                    OUString aTitle = aRow.getToken( 0, '\t', nIdx );
                    OUString aURL = aRow.getToken( 0, '\t', nIdx );
                    sal_Unicode cFolder = aRow.getToken( 0, '\t', nIdx )[0];
                    bool bIsFolder = ( '1' == cFolder );
                    SvTreeListEntry* pEntry = nullptr;
                    if ( bIsFolder )
                    {
                        pEntry = InsertEntry( aTitle, aOpenBookImage, aClosedBookImage, pParent, true );
                        pEntry->SetUserData( new ContentEntry_Impl( aURL, true ) );
                    }
                    else
                    {
                        pEntry = InsertEntry( aTitle, aDocumentImage, aDocumentImage, pParent );
                        Any aAny( ::utl::UCBContentHelper::GetProperty( aURL, "TargetURL" ) );
                        OUString aTargetURL;
                        if ( aAny >>= aTargetURL )
                            pEntry->SetUserData( new ContentEntry_Impl( aTargetURL, false ) );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "ContentListBox_Impl::RequestingChildren(): unexpected exception" );
    }
}


bool ContentListBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( nullptr );
        bHandled = true;
    }

    return bHandled || SvTreeListBox::EventNotify( rNEvt );
}


OUString ContentListBox_Impl::GetSelectedEntry() const
{
    OUString aRet;
    SvTreeListEntry* pEntry = FirstSelected();
    if ( pEntry && !static_cast<ContentEntry_Impl*>(pEntry->GetUserData())->bIsFolder )
        aRet = static_cast<ContentEntry_Impl*>(pEntry->GetUserData())->aURL;
    return aRet;
}

// class HelpTabPage_Impl ------------------------------------------------
HelpTabPage_Impl::HelpTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin,
    const OString& rID, const OUString& rUIXMLDescription)
    : TabPage( pParent, rID, rUIXMLDescription)
    , m_pIdxWin( _pIdxWin )
{
}

HelpTabPage_Impl::~HelpTabPage_Impl()
{
    disposeOnce();
}

void HelpTabPage_Impl::dispose()
{
    m_pIdxWin.clear();
    TabPage::dispose();
}

// class ContentTabPage_Impl ---------------------------------------------
ContentTabPage_Impl::ContentTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin)
    : HelpTabPage_Impl(pParent, _pIdxWin, "HelpContentPage",
        "sfx/ui/helpcontentpage.ui")
{
    get(m_pContentBox, "content");
    Size aSize(LogicToPixel(Size(108 , 188), MapMode(MapUnit::MapAppFont)));
    m_pContentBox->set_width_request(aSize.Width());
    m_pContentBox->set_height_request(aSize.Height());
}

ContentTabPage_Impl::~ContentTabPage_Impl()
{
    disposeOnce();
}

void ContentTabPage_Impl::dispose()
{
    m_pContentBox.clear();
    HelpTabPage_Impl::dispose();
}

void ContentTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

Control* ContentTabPage_Impl::GetLastFocusControl()
{
    return m_pContentBox;
}

// class IndexBox_Impl ---------------------------------------------------

IndexBox_Impl::IndexBox_Impl(vcl::Window* pParent, WinBits nStyle)
    : ComboBox(pParent, nStyle)
{
    EnableAutocomplete(true);
    EnableUserDraw(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeIndexBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;
    VclPtrInstance<IndexBox_Impl> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void IndexBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    IndexEntry_Impl* pEntry = static_cast<IndexEntry_Impl*>(GetEntryData( rUDEvt.GetItemId() ));
    if ( pEntry && pEntry->m_bSubEntry )
    {
        // indent sub entries
        Point aPos( rUDEvt.GetRect().TopLeft() );
        aPos.AdjustX(8 );
        aPos.AdjustY((rUDEvt.GetRect().GetHeight() - rUDEvt.GetRenderContext()->GetTextHeight()) / 2 );
        OUString aEntry( GetEntry( rUDEvt.GetItemId() ) );
        sal_Int32 nPos = aEntry.indexOf( ';' );
        rUDEvt.GetRenderContext()->DrawText(aPos, (nPos !=-1) ? aEntry.copy(nPos + 1) : aEntry);
    }
    else
        DrawEntry( rUDEvt, false, true, true );
}


bool IndexBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( *this );
        bHandled = true;
    }

    return bHandled || ComboBox::EventNotify( rNEvt );
}


void IndexBox_Impl::SelectExecutableEntry()
{
    sal_Int32 nPos = GetEntryPos( GetText() );
    if ( nPos == COMBOBOX_ENTRY_NOTFOUND )
        return;

    sal_Int32 nOldPos = nPos;
    OUString aEntryText;
    IndexEntry_Impl* pEntry = static_cast<IndexEntry_Impl*>(GetEntryData( nPos ));
    sal_Int32 nCount = GetEntryCount();
    while ( nPos < nCount && ( !pEntry || pEntry->m_aURL.isEmpty() ) )
    {
        pEntry = static_cast<IndexEntry_Impl*>(GetEntryData( ++nPos ));
        aEntryText = GetEntry( nPos );
    }

    if ( nOldPos != nPos )
        SetText( aEntryText );
}

// class IndexTabPage_Impl -----------------------------------------------

IndexTabPage_Impl::IndexTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin)
    : HelpTabPage_Impl(pParent, _pIdxWin, "HelpIndexPage",
        "sfx/ui/helpindexpage.ui")
    , aFactoryIdle("sfx2 appl IndexTabPage_Impl Factory")
    , bIsActivated(false)
{
    get(m_pIndexCB, "terms");
    Size aSize(LogicToPixel(Size(108, 97), MapMode(MapUnit::MapAppFont)));
    m_pIndexCB->set_width_request(aSize.Width());
    m_pIndexCB->set_height_request(aSize.Height());
    get(m_pOpenBtn, "display");

    m_pOpenBtn->SetClickHdl( LINK( this, IndexTabPage_Impl, OpenHdl ) );
    aFactoryIdle.SetInvokeHandler( LINK(this, IndexTabPage_Impl, IdleHdl ));
    aKeywordTimer.SetInvokeHandler( LINK( this, IndexTabPage_Impl, TimeoutHdl ) );
}

IndexTabPage_Impl::~IndexTabPage_Impl()
{
    disposeOnce();
}

void IndexTabPage_Impl::dispose()
{
    ClearIndex();
    m_pIndexCB.clear();
    m_pOpenBtn.clear();
    HelpTabPage_Impl::dispose();
}


namespace sfx2 {

    typedef std::unordered_map< OUString, int > KeywordInfo;
}

void IndexTabPage_Impl::InitializeIndex()
{
    WaitObject aWaitCursor( this );

    // By now more than 256 equal entries are not allowed
    sal_Unicode append[256];
    for(sal_Unicode & k : append)
        k =  ' ';

    sfx2::KeywordInfo aInfo;
    m_pIndexCB->SetUpdateMode( false );

    try
    {
        OUStringBuffer aURL = HELP_URL;
        aURL.append(sFactory);
        AppendConfigToken(aURL, true);

        Content aCnt( aURL.makeStringAndClear(), Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        css::uno::Reference< css::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
        if ( xInfo->hasPropertyByName( PROPERTY_ANCHORREF ) )
        {
            css::uno::Sequence< OUString > aPropSeq( 4 );
            aPropSeq[0] = PROPERTY_KEYWORDLIST;
            aPropSeq[1] = PROPERTY_KEYWORDREF;
            aPropSeq[2] = PROPERTY_ANCHORREF;
            aPropSeq[3] = PROPERTY_TITLEREF;

            // abi: use one possibly remote call only
            css::uno::Sequence< css::uno::Any > aAnySeq =
                  aCnt.getPropertyValues( aPropSeq );

            css::uno::Sequence< OUString > aKeywordList;
            css::uno::Sequence< css::uno::Sequence< OUString > > aKeywordRefList;
            css::uno::Sequence< css::uno::Sequence< OUString > > aAnchorRefList;
            css::uno::Sequence< css::uno::Sequence< OUString > > aTitleRefList;

            if ( ( aAnySeq[0] >>= aKeywordList ) && ( aAnySeq[1] >>= aKeywordRefList ) &&
                 ( aAnySeq[2] >>= aAnchorRefList ) && ( aAnySeq[3] >>= aTitleRefList ) )
            {
                sal_Int32 nPos;
                int ndx,tmp;
                OUString aIndex, aTempString;
                OUStringBuffer aData( 128 );            // Capacity of up to 128 characters
                sfx2::KeywordInfo::iterator it;

                for ( int i = 0; i < aKeywordList.getLength(); ++i )
                {
                    // abi: Do not copy, but use references
                    const OUString& aKeywordPair = aKeywordList[i];
                    DBG_ASSERT( !aKeywordPair.isEmpty(), "invalid help index" );
                    const css::uno::Sequence< OUString >& aRefList = aKeywordRefList[i];
                    const css::uno::Sequence< OUString >& aAnchorList = aAnchorRefList[i];
                    const css::uno::Sequence< OUString >& aTitleList = aTitleRefList[i];

                    DBG_ASSERT( aRefList.getLength() == aAnchorList.getLength(),"reference list and title list of different length" );

                    ndx = aKeywordPair.indexOf( ';' );
                    const bool insert = ndx != -1;

                    if ( insert )
                    {
                        aTempString = aKeywordPair.copy( 0, ndx );
                        if ( aIndex != aTempString )
                        {
                            aIndex = aTempString;
                            it = aInfo.emplace(aTempString, 0).first;
                            if ( (tmp = it->second++) != 0)
                                m_pIndexCB->InsertEntry(aTempString + OUString(append, tmp));
                            else
                                m_pIndexCB->InsertEntry(aTempString);
                        }
                    }
                    else
                        aIndex.clear();

                    // Assume the token is trimmed
                    it = aInfo.emplace(aKeywordPair, 0).first;
                    if ((tmp = it->second++) != 0)
                        nPos = m_pIndexCB->InsertEntry(aKeywordPair + OUString(append, tmp));
                    else
                        nPos = m_pIndexCB->InsertEntry(aKeywordPair);

                    sal_uInt32 nRefListLen = aRefList.getLength();

                    DBG_ASSERT( aAnchorList.getLength(), "*IndexTabPage_Impl::InitializeIndex(): AnchorList is empty!" );
                    DBG_ASSERT( nRefListLen, "*IndexTabPage_Impl::InitializeIndex(): RefList is empty!" );

                    if ( aAnchorList.getLength() && nRefListLen )
                    {
                        if ( aAnchorList[0].getLength() > 0 )
                        {
                            aData.append( aRefList[0] ).append( '#' ).append( aAnchorList[0] );
                            m_pIndexCB->SetEntryData( nPos, new IndexEntry_Impl( aData.makeStringAndClear(), insert ) );
                        }
                        else
                            m_pIndexCB->SetEntryData( nPos, new IndexEntry_Impl( aRefList[0], insert ) );
                    }

                    for ( sal_uInt32 j = 1; j < nRefListLen ; ++j )
                    {
                        aData
                            .append( aKeywordPair )
                            .append( ' ' )
                            .append( '-' )
                            .append( ' ' )
                            .append( aTitleList[j] );

                        aTempString = aData.makeStringAndClear();
                        it = aInfo.emplace(aTempString, 0).first;
                        if ( (tmp = it->second++) != 0 )
                            nPos = m_pIndexCB->InsertEntry(aTempString + OUString(append, tmp));
                        else
                            nPos = m_pIndexCB->InsertEntry(aTempString);

                        if ( aAnchorList[j].getLength() > 0 )
                        {
                            aData.append( aRefList[j] ).append( '#' ).append( aAnchorList[j] );
                            m_pIndexCB->SetEntryData( nPos, new IndexEntry_Impl( aData.makeStringAndClear(), insert ) );
                        }
                        else
                            m_pIndexCB->SetEntryData( nPos, new IndexEntry_Impl( aRefList[j], insert ) );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "IndexTabPage_Impl::InitializeIndex(): unexpected exception" );
    }

    m_pIndexCB->SetUpdateMode( true );

    if ( !sKeyword.isEmpty() )
        aKeywordLink.Call( *this );
}

void IndexTabPage_Impl::ClearIndex()
{
    const sal_Int32 nCount = m_pIndexCB->GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
        delete static_cast<IndexEntry_Impl*>(m_pIndexCB->GetEntryData(i));
    m_pIndexCB->Clear();
}

IMPL_LINK_NOARG(IndexTabPage_Impl, OpenHdl, Button*, void)
{
    m_pIndexCB->GetDoubleClickHdl().Call(*m_pIndexCB);
}

IMPL_LINK( IndexTabPage_Impl, IdleHdl, Timer*, pIdle, void )
{
    if ( &aFactoryIdle == pIdle )
        InitializeIndex();
}

IMPL_LINK( IndexTabPage_Impl, TimeoutHdl, Timer*, pTimer, void)
{
    if(&aKeywordTimer == pTimer && !sKeyword.isEmpty())
        aKeywordLink.Call(*this);
}

void IndexTabPage_Impl::ActivatePage()
{
    if ( !bIsActivated )
    {
        bIsActivated = true;
        aFactoryIdle.Start();
    }

    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

Control* IndexTabPage_Impl::GetLastFocusControl()
{
    return m_pOpenBtn;
}

void IndexTabPage_Impl::SetDoubleClickHdl( const Link<ComboBox&,void>& rLink )
{
    m_pIndexCB->SetDoubleClickHdl( rLink );
}

void IndexTabPage_Impl::SetFactory( const OUString& rFactory )
{
    OUString sNewFactory( rFactory );
    DBG_ASSERT( !sNewFactory.isEmpty(), "empty factory" );
    bool bValid = m_pIdxWin->IsValidFactory( rFactory );

    if ( sFactory.isEmpty() && !bValid )
    {
        sNewFactory = SfxHelp::GetDefaultHelpModule();
        bValid = true;
    }

    if ( sNewFactory != sFactory && bValid )
    {
        sFactory = sNewFactory;
        ClearIndex();
        if ( bIsActivated )
            aFactoryIdle.Start();
    }
}


OUString IndexTabPage_Impl::GetSelectedEntry() const
{
    OUString aRet;
    IndexEntry_Impl* pEntry = static_cast<IndexEntry_Impl*>(m_pIndexCB->GetEntryData( m_pIndexCB->GetEntryPos( m_pIndexCB->GetText() ) ));
    if ( pEntry )
        aRet = pEntry->m_aURL;
    return aRet;
}


void IndexTabPage_Impl::SetKeyword( const OUString& rKeyword )
{
    sKeyword = rKeyword;

    if ( m_pIndexCB->GetEntryCount() > 0 )
        aKeywordTimer.Start();
    else if ( !bIsActivated )
        aFactoryIdle.Start();
}


bool IndexTabPage_Impl::HasKeyword() const
{
    bool bRet = false;
    if ( !sKeyword.isEmpty() )
    {
        sal_Int32 nPos = m_pIndexCB->GetEntryPos( sKeyword );
        bRet = ( nPos != LISTBOX_ENTRY_NOTFOUND );
    }

    return bRet;
}


bool IndexTabPage_Impl::HasKeywordIgnoreCase()
{
    bool bRet = false;
    if ( !sKeyword.isEmpty() )
    {
        sal_Int32 nEntries = m_pIndexCB->GetEntryCount();
        const vcl::I18nHelper& rI18nHelper = GetSettings().GetLocaleI18nHelper();
        for ( sal_Int32 n = 0; n < nEntries; n++)
        {
            const OUString sIndexItem {m_pIndexCB->GetEntry( n )};
            if (rI18nHelper.MatchString( sIndexItem, sKeyword ))
            {
                sKeyword = sIndexItem;
                bRet = true;
            }
        }
    }

    return bRet;
}


void IndexTabPage_Impl::OpenKeyword()
{
    if ( !sKeyword.isEmpty() )
    {
        m_pIndexCB->SetText( sKeyword );
        m_pIndexCB->GetDoubleClickHdl().Call( *m_pIndexCB );
        sKeyword.clear();
    }
}

// class SearchBox_Impl --------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void makeSearchBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    WinBits nWinBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE|WB_DROPDOWN;
    VclPtrInstance<SearchBox_Impl> pComboBox(pParent, nWinBits);
    pComboBox->EnableAutoSize(true);
    rRet = pComboBox;
}

bool SearchBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( !IsInDropDown() &&
         rNEvt.GetWindow() == GetSubEdit() &&
         rNEvt.GetType() == MouseNotifyEvent::KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        aSearchLink.Call( nullptr );
        bHandled = true;
    }
    return bHandled || ComboBox::PreNotify( rNEvt );
}


void SearchBox_Impl::Select()
{
    if ( !IsTravelSelect() )
        aSearchLink.Call( nullptr );
}

// class SearchResultsBox_Impl -------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void makeSearchResultsBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;
    VclPtrInstance<SearchResultsBox_Impl> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

bool SearchResultsBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT &&
         KEY_RETURN == rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
    {
        GetDoubleClickHdl().Call( *this );
        bHandled = true;
    }

    return bHandled || ListBox::EventNotify( rNEvt );
}

// class SearchTabPage_Impl ----------------------------------------------

SearchTabPage_Impl::SearchTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin)
    : HelpTabPage_Impl(pParent, _pIdxWin, "HelpSearchPage",
        "sfx/ui/helpsearchpage.ui")
    , xBreakIterator(vcl::unohelper::CreateBreakIterator())
{
    get(m_pSearchED, "search");
    get(m_pSearchBtn, "find");
    get(m_pFullWordsCB, "completewords");
    get(m_pScopeCB, "headings");
    get(m_pResultsLB, "results");
    Size aSize(LogicToPixel(Size(128 , 30), MapMode(MapUnit::MapAppFont)));
    m_pResultsLB->set_width_request(aSize.Width());
    m_pResultsLB->set_height_request(aSize.Height());
    get(m_pOpenBtn, "display");

    m_pSearchED->SetSearchLink( LINK( this, SearchTabPage_Impl, SearchHdl ) );
    m_pSearchBtn->SetClickHdl(LINK( this, SearchTabPage_Impl, ClickHdl ));
    m_pSearchED->SetModifyHdl( LINK( this, SearchTabPage_Impl, ModifyHdl ) );
    m_pOpenBtn->SetClickHdl( LINK( this, SearchTabPage_Impl, OpenHdl ) );

    SvtViewOptions aViewOpt( EViewType::TabPage, CONFIGNAME_SEARCHPAGE );
    if ( aViewOpt.Exists() )
    {
        OUString aUserData;
        Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
        if ( aUserItem >>= aUserData )
        {
            sal_Int32 nIdx {0};
            bool bChecked = aUserData.getToken(0, ';', nIdx).toInt32() == 1;
            m_pFullWordsCB->Check( bChecked );
            bChecked = aUserData.getToken(0, ';', nIdx).toInt32() == 1;
            m_pScopeCB->Check( bChecked );

            while ( nIdx > 0 )
            {
                m_pSearchED->InsertEntry( INetURLObject::decode(
                    aUserData.getToken(0, ';', nIdx),
                    INetURLObject::DecodeMechanism::WithCharset ) );
            }
        }
    }

    ModifyHdl(*m_pSearchED);
}

SearchTabPage_Impl::~SearchTabPage_Impl()
{
    disposeOnce();
}

void SearchTabPage_Impl::dispose()
{
    SvtViewOptions aViewOpt( EViewType::TabPage, CONFIGNAME_SEARCHPAGE );
    OUStringBuffer aUserData;
    aUserData.append(OUString::number( m_pFullWordsCB->IsChecked() ? 1 : 0 ))
        .append(";")
        .append(OUString::number( m_pScopeCB->IsChecked() ? 1 : 0 ));
    sal_Int32 nCount = std::min( m_pSearchED->GetEntryCount(), sal_Int32(10) );  // save only 10 entries

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        aUserData.append(";").append(INetURLObject::encode(
            m_pSearchED->GetEntry(i),
            INetURLObject::PART_UNO_PARAM_VALUE,
            INetURLObject::EncodeMechanism::All ));
    }

    Any aUserItem = makeAny( aUserData.makeStringAndClear() );
    aViewOpt.SetUserItem( USERITEM_NAME, aUserItem );

    m_pSearchED.clear();
    m_pSearchBtn.clear();
    m_pFullWordsCB.clear();
    m_pScopeCB.clear();
    m_pResultsLB.clear();
    m_pOpenBtn.clear();
    HelpTabPage_Impl::dispose();
}


void SearchTabPage_Impl::ClearSearchResults()
{
    const sal_Int32 nCount = m_pResultsLB->GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
        delete static_cast<OUString*>(m_pResultsLB->GetEntryData(i));
    m_pResultsLB->Clear();
    m_pResultsLB->Update();
}


void SearchTabPage_Impl::RememberSearchText( const OUString& rSearchText )
{
    for ( sal_Int32 i = 0; i < m_pSearchED->GetEntryCount(); ++i )
    {
        if ( rSearchText == m_pSearchED->GetEntry(i) )
        {
            m_pSearchED->RemoveEntryAt(i);
            break;
        }
    }

    m_pSearchED->InsertEntry( rSearchText, 0 );
}


IMPL_LINK_NOARG(SearchTabPage_Impl, ClickHdl, Button*, void)
{
    SearchHdl(nullptr);
}

IMPL_LINK_NOARG(SearchTabPage_Impl, SearchHdl, LinkParamNone*, void)
{
    OUString aSearchText = comphelper::string::strip(m_pSearchED->GetText(), ' ');
    if ( aSearchText.isEmpty() )
        return;

    EnterWait();
    ClearSearchResults();
    RememberSearchText( aSearchText );
    OUStringBuffer aSearchURL(HELP_URL);
    aSearchURL.append(aFactory);
    aSearchURL.append(HELP_SEARCH_TAG);
    if ( !m_pFullWordsCB->IsChecked() )
        aSearchText = sfx2::PrepareSearchString( aSearchText, xBreakIterator, true );
    aSearchURL.append(aSearchText);
    AppendConfigToken(aSearchURL, false);
    if ( m_pScopeCB->IsChecked() )
        aSearchURL.append("&Scope=Heading");
    std::vector< OUString > aFactories = SfxContentHelper::GetResultSet(aSearchURL.makeStringAndClear());
    for (const OUString & rRow : aFactories)
    {
        sal_Int32 nIdx = 0;
        OUString aTitle = rRow.getToken( 0, '\t', nIdx );
        OUString* pURL = new OUString( rRow.getToken( 1, '\t', nIdx ) );
        const sal_Int32 nPos = m_pResultsLB->InsertEntry( aTitle );
        m_pResultsLB->SetEntryData( nPos, pURL );
    }
    LeaveWait();

    if ( aFactories.empty() )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                 VclMessageType::Info, VclButtonsType::Ok,
                                                                 SfxResId(STR_INFO_NOSEARCHRESULTS)));
        xBox->run();
    }
}

IMPL_LINK_NOARG(SearchTabPage_Impl, OpenHdl, Button*, void)
{
    m_pResultsLB->GetDoubleClickHdl().Call(*m_pResultsLB);
}

IMPL_LINK_NOARG(SearchTabPage_Impl, ModifyHdl, Edit&, void)
{
    OUString aSearchText = comphelper::string::strip(m_pSearchED->GetText(), ' ');
    m_pSearchBtn->Enable(!aSearchText.isEmpty());
}

void SearchTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        m_pSearchED->GrabFocus();
}

Control* SearchTabPage_Impl::GetLastFocusControl()
{
    return m_pOpenBtn;
}

void SearchTabPage_Impl::SetDoubleClickHdl( const Link<ListBox&,void>& rLink )
{
    m_pResultsLB->SetDoubleClickHdl( rLink );
}


OUString SearchTabPage_Impl::GetSelectedEntry() const
{
    OUString aRet;
    OUString* pData = static_cast<OUString*>(m_pResultsLB->GetSelectedEntryData());
    if ( pData )
        aRet = *pData;
    return aRet;
}


void SearchTabPage_Impl::ClearPage()
{
    ClearSearchResults();
    m_pSearchED->SetText( OUString() );
}


bool SearchTabPage_Impl::OpenKeyword( const OUString& rKeyword )
{
    bool bRet = false;
    m_pSearchED->SetText( rKeyword );
    SearchHdl( nullptr );
    if ( m_pResultsLB->GetEntryCount() > 0 )
    {
        // found keyword -> open it
        m_pResultsLB->SelectEntryPos(0);
        OpenHdl( nullptr );
        bRet = true;
    }

    return bRet;
}

// class BookmarksTabPage_Impl -------------------------------------------

static void GetBookmarkEntry_Impl
(
    Sequence< PropertyValue >& aBookmarkEntry,
    OUString& rTitle,
    OUString& rURL
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

BookmarksBox_Impl::BookmarksBox_Impl(vcl::Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeBookmarksBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;
    VclPtrInstance<BookmarksBox_Impl> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

BookmarksBox_Impl::~BookmarksBox_Impl()
{
    disposeOnce();
}

void BookmarksBox_Impl::dispose()
{
    // save bookmarks to configuration
    SvtHistoryOptions aHistOpt;
    aHistOpt.Clear( eHELPBOOKMARKS );
    const sal_Int32 nCount = GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        OUString* pURL = static_cast<OUString*>(GetEntryData(i));
        aHistOpt.AppendItem(eHELPBOOKMARKS, *pURL, "", GetEntry(i), boost::none);
        delete pURL;
    }
    ListBox::dispose();
}


void BookmarksBox_Impl::DoAction( sal_uInt16 nAction )
{
    switch ( nAction )
    {
        case MID_OPEN :
            GetDoubleClickHdl().Call( *this );
            break;

        case MID_RENAME :
           {
            sal_Int32 nPos = GetSelectedEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                SfxAddHelpBookmarkDialog_Impl aDlg(GetFrameWeld(), true);
                aDlg.SetTitle(GetEntry(nPos));
                if (aDlg.run() == RET_OK)
                {
                    OUString* pURL = static_cast<OUString*>(GetEntryData( nPos ));
                    RemoveEntry( nPos );
                    nPos = InsertEntry( aDlg.GetTitle(), SvFileInformationManager::GetImage( INetURLObject(IMAGE_URL+INetURLObject( *pURL ).GetHost()) ) );
                    SetEntryData( nPos, new OUString( *pURL ) );
                    SelectEntryPos( nPos );
                    delete pURL;
                }
            }
            break;
        }

        case MID_DELETE :
        {
            sal_Int32 nPos = GetSelectedEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                RemoveEntry( nPos );
                const sal_Int32 nCount = GetEntryCount();
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


bool BookmarksBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    MouseNotifyEvent nType = rNEvt.GetType();
    if ( MouseNotifyEvent::KEYINPUT == nType )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
        if ( KEY_DELETE == nCode && GetEntryCount() > 0 )
        {
            DoAction( MID_DELETE );
            bRet = true;
        }
        else if ( KEY_RETURN == nCode )
        {
            GetDoubleClickHdl().Call( *this );
            bRet = true;
        }
    }
    else if ( MouseNotifyEvent::COMMAND == nType )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() == CommandEventId::ContextMenu )
        {
            VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "sfx/ui/bookmarkmenu.ui", "");
            VclPtr<PopupMenu> aMenu(aBuilder.get_menu("menu"));
            sal_uInt16 nId = aMenu->Execute(this, pCEvt->GetMousePosPixel());
            if (nId != MENU_ITEM_NOTFOUND)
            {
                OString sIdent = aMenu->GetCurItemIdent();
                if (sIdent == "display")
                    DoAction(MID_OPEN);
                else if (sIdent == "rename")
                    DoAction(MID_RENAME);
                else if (sIdent == "delete")
                    DoAction(MID_DELETE);
            }
            bRet = true;
        }
    }

    return bRet || ListBox::EventNotify( rNEvt );
}

// class BookmarksTabPage_Impl -------------------------------------------

BookmarksTabPage_Impl::BookmarksTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin)
    : HelpTabPage_Impl(pParent, _pIdxWin, "HelpBookmarkPage",
        "sfx/ui/helpbookmarkpage.ui")
{
    get(m_pBookmarksPB, "display");
    get(m_pBookmarksBox, "bookmarks");
    Size aSize(LogicToPixel(Size(120 , 200), MapMode(MapUnit::MapAppFont)));
    m_pBookmarksBox->set_width_request(aSize.Width());
    m_pBookmarksBox->set_height_request(aSize.Height());

    m_pBookmarksPB->SetClickHdl( LINK( this, BookmarksTabPage_Impl, OpenHdl ) );

    // load bookmarks from configuration
    Sequence< Sequence< PropertyValue > > aBookmarkSeq;
    aBookmarkSeq = SvtHistoryOptions().GetList( eHELPBOOKMARKS );

    OUString aTitle;
    OUString aURL;

    sal_uInt32 i, nCount = aBookmarkSeq.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetBookmarkEntry_Impl( aBookmarkSeq[i], aTitle, aURL );
        AddBookmarks( aTitle, aURL );
    }
}

BookmarksTabPage_Impl::~BookmarksTabPage_Impl()
{
    disposeOnce();
}

void BookmarksTabPage_Impl::dispose()
{
    m_pBookmarksBox.clear();
    m_pBookmarksPB.clear();
    HelpTabPage_Impl::dispose();
}


IMPL_LINK_NOARG(BookmarksTabPage_Impl, OpenHdl, Button*, void)
{
    m_pBookmarksBox->GetDoubleClickHdl().Call(*m_pBookmarksBox);
}

void BookmarksTabPage_Impl::ActivatePage()
{
    if ( !m_pIdxWin->WasCursorLeftOrRight() )
        SetFocusOnBox();
}

Control* BookmarksTabPage_Impl::GetLastFocusControl()
{
    return m_pBookmarksPB;
}

void BookmarksTabPage_Impl::SetDoubleClickHdl( const Link<ListBox&,void>& rLink )
{
    m_pBookmarksBox->SetDoubleClickHdl(rLink);
}

OUString BookmarksTabPage_Impl::GetSelectedEntry() const
{
    OUString aRet;
    OUString* pData = static_cast<OUString*>(m_pBookmarksBox->GetSelectedEntryData());
    if ( pData )
        aRet = *pData;
    return aRet;
}


void BookmarksTabPage_Impl::AddBookmarks( const OUString& rTitle, const OUString& rURL )
{
    const OUString aImageURL {IMAGE_URL + INetURLObject( rURL ).GetHost()};
    const sal_Int32 nPos = m_pBookmarksBox->InsertEntry( rTitle, SvFileInformationManager::GetImage( INetURLObject(aImageURL) ) );
    m_pBookmarksBox->SetEntryData( nPos, new OUString( rURL ) );
}

OUString SfxHelpWindow_Impl::buildHelpURL(const OUString& sFactory        ,
                                          const OUString& sContent        ,
                                          const OUString& sAnchor)
{
    OUStringBuffer sHelpURL(256);
    sHelpURL.append(HELP_URL);
    sHelpURL.append(sFactory);
    sHelpURL.append(sContent);
    AppendConfigToken(sHelpURL, true/*bUseQuestionMark*/);
    if (!sAnchor.isEmpty())
        sHelpURL.append(sAnchor);
    return sHelpURL.makeStringAndClear();
}

void SfxHelpWindow_Impl::loadHelpContent(const OUString& sHelpURL, bool bAddToHistory)
{
    Reference< XComponentLoader > xLoader(getTextFrame(), UNO_QUERY);
    if (!xLoader.is())
        return;

    // If a print job runs do not open a new page
    Reference< XFrame2 >     xTextFrame      = pTextWin->getFrame();
    Reference< XController > xTextController ;
    if (xTextFrame.is())
        xTextController = xTextFrame->getController ();
    if ( xTextController.is() && !xTextController->suspend( true ) )
    {
        xTextController->suspend( false );
        return;
    }

    // save url to history
    if (bAddToHistory)
        pHelpInterceptor->addURL(sHelpURL);

    if ( !IsWait() )
        EnterWait();
    bool bSuccess = false;
// TODO implement locale fallback ... see below    while(true)
    {
        try
        {
            Reference< XComponent > xContent = xLoader->loadComponentFromURL(sHelpURL, "_self", 0, Sequence< PropertyValue >());
            if (xContent.is())
            {
                bSuccess = true;
            }
        }
        catch(const RuntimeException&)
            { throw; }
        catch(const Exception&)
            { /*break;*/ }

        /* TODO try next locale ...
                no further locale available? => break loop and show error page
        */
    }
    openDone(sHelpURL, bSuccess);
    if ( IsWait() )
        LeaveWait();
}

SfxHelpIndexWindow_Impl::SfxHelpIndexWindow_Impl(SfxHelpWindow_Impl* _pParent)
    : Window(_pParent, 0)
    , aIdle("sfx2 appl SfxHelpIndexWindow_Impl")
    , aIndexKeywordLink(LINK(this, SfxHelpIndexWindow_Impl, KeywordHdl))
    , pParentWin(_pParent)
    , pCPage(nullptr)
    , pIPage(nullptr)
    , pSPage(nullptr)
    , pBPage(nullptr)
    , bWasCursorLeftOrRight(false)
    , bIsInitDone(false)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "sfx/ui/helpcontrol.ui", "HelpControl"));
    get(m_pActiveLB, "active");
    get(m_pTabCtrl, "tabcontrol");

    sfx2::AddToTaskPaneList( this );

    m_pTabCtrl->SetActivatePageHdl( LINK( this, SfxHelpIndexWindow_Impl, ActivatePageHdl ) );

    OString sPageId("index");
    SvtViewOptions aViewOpt( EViewType::TabDialog, CONFIGNAME_INDEXWIN );
    if ( aViewOpt.Exists() )
        sPageId = aViewOpt.GetPageID();
    m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId(sPageId));
    ActivatePageHdl( m_pTabCtrl );
    m_pActiveLB->SetSelectHdl( LINK( this, SfxHelpIndexWindow_Impl, SelectHdl ) );

    aIdle.SetInvokeHandler( LINK( this, SfxHelpIndexWindow_Impl, InitHdl ) );
    aIdle.Start();

    Show();
}


SfxHelpIndexWindow_Impl::~SfxHelpIndexWindow_Impl()
{
    disposeOnce();
}

void SfxHelpIndexWindow_Impl::dispose()
{
    sfx2::RemoveFromTaskPaneList( this );

    pCPage.disposeAndClear();
    pIPage.disposeAndClear();
    pSPage.disposeAndClear();
    pBPage.disposeAndClear();

    for ( sal_Int32 i = 0; i < m_pActiveLB->GetEntryCount(); ++i )
        delete static_cast<OUString*>(m_pActiveLB->GetEntryData(i));

    SvtViewOptions aViewOpt( EViewType::TabDialog, CONFIGNAME_INDEXWIN );
    aViewOpt.SetPageID(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));

    disposeBuilder();
    m_pActiveLB.clear();
    m_pTabCtrl.clear();
    pParentWin.clear();
    vcl::Window::dispose();
}


void SfxHelpIndexWindow_Impl::Initialize()
{
    OUStringBuffer aHelpURL(HELP_URL);
    AppendConfigToken(aHelpURL, true);
    std::vector<OUString> aFactories = SfxContentHelper::GetResultSet(aHelpURL.makeStringAndClear());
    for (const OUString & rRow : aFactories)
    {
        sal_Int32 nIdx = 0;
        OUString aTitle = rRow.getToken( 0, '\t', nIdx ); // token 0
        OUString aURL = rRow.getToken( 1, '\t', nIdx ); // token 2
        OUString* pFactory = new OUString( INetURLObject( aURL ).GetHost() );
        const sal_Int32 nPos = m_pActiveLB->InsertEntry( aTitle );
        m_pActiveLB->SetEntryData( nPos, pFactory );
    }

    m_pActiveLB->SetDropDownLineCount( static_cast<sal_uInt16>(aFactories.size()) );
    if ( m_pActiveLB->GetSelectedEntryPos() == LISTBOX_ENTRY_NOTFOUND )
        SetActiveFactory();
}


void SfxHelpIndexWindow_Impl::SetActiveFactory()
{
    DBG_ASSERT( pIPage, "index page not initialized" );
    if ( !bIsInitDone && !m_pActiveLB->GetEntryCount() )
    {
        aIdle.Stop();
        InitHdl( nullptr );
    }

    for ( sal_Int32 i = 0; i < m_pActiveLB->GetEntryCount(); ++i )
    {
        OUString* pFactory = static_cast<OUString*>(m_pActiveLB->GetEntryData(i));
        *pFactory = pFactory->toAsciiLowerCase();
        if ( *pFactory == pIPage->GetFactory() )
        {
            if ( m_pActiveLB->GetSelectedEntryPos() != i )
            {
                m_pActiveLB->SelectEntryPos(i);
                aSelectFactoryLink.Call( nullptr );
            }
            break;
        }
    }
}


HelpTabPage_Impl* SfxHelpIndexWindow_Impl::GetCurrentPage( sal_uInt16& rCurId )
{
    rCurId = m_pTabCtrl->GetCurPageId();
    HelpTabPage_Impl* pPage = nullptr;

    OString sName(m_pTabCtrl->GetPageName(rCurId));

    if (sName == "contents")
    {
        pPage = GetContentPage();
    }
    else if (sName == "index")
    {
        pPage = GetIndexPage();
    }
    else if (sName == "find")
    {
        pPage = GetSearchPage();
    }
    else if (sName == "bookmarks")
    {
        pPage = GetBookmarksPage();
    }

    DBG_ASSERT( pPage, "SfxHelpIndexWindow_Impl::GetCurrentPage(): no current page" );
    return pPage;
}

IMPL_LINK( SfxHelpIndexWindow_Impl, ActivatePageHdl, TabControl *, pTabCtrl, void )
{
    sal_uInt16 nId = 0;
    TabPage* pPage = GetCurrentPage( nId );
    pTabCtrl->SetTabPage( nId, pPage );
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, SelectHdl, ListBox&, void)
{
    aIdle.Start();
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, InitHdl, Timer *, void)
{
    bIsInitDone = true;
    Initialize();

    // now use the timer for selection
    aIdle.SetInvokeHandler( LINK( this, SfxHelpIndexWindow_Impl, SelectFactoryHdl ) );
    aIdle.SetPriority( TaskPriority::LOWEST );
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, SelectFactoryHdl, Timer *, void)
{
    OUString* pFactory = static_cast<OUString*>(m_pActiveLB->GetSelectedEntryData());
    if ( pFactory )
    {
        SetFactory( pFactory->toAsciiLowerCase(), false );
        aSelectFactoryLink.Call( this );
    }
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, KeywordHdl, IndexTabPage_Impl&, void)
{
    // keyword found on index?
    bool bIndex = pIPage->HasKeyword();

    if( !bIndex)
        bIndex = pIPage->HasKeywordIgnoreCase();
    // then set index or search page as current.
    sal_uInt16 nPageId = bIndex ? m_pTabCtrl->GetPageId("index") : m_pTabCtrl->GetPageId("find");
    if ( nPageId != m_pTabCtrl->GetCurPageId() )
    {
        m_pTabCtrl->SetCurPageId( nPageId );
        ActivatePageHdl( m_pTabCtrl );
    }

    // at last we open the keyword
    if ( bIndex )
        pIPage->OpenKeyword();
    else if ( !pSPage->OpenKeyword( sKeyword ) )
        pParentWin->ShowStartPage();
}

IMPL_LINK(SfxHelpIndexWindow_Impl, IndexTabPageDoubleClickHdl, ComboBox&, rBox, void)
{
    aPageDoubleClickLink.Call(&rBox);
}

void SfxHelpIndexWindow_Impl::Resize()
{
    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
    if (!pChild)
        return;
    VclContainer::setLayoutAllocation(*pChild, Point(0,0), GetSizePixel());
}

Size SfxHelpIndexWindow_Impl::GetOptimalSize() const
{
    const vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
    if (!pChild)
        return Window::GetOptimalSize();
    return VclContainer::getLayoutRequisition(*pChild);
}

bool SfxHelpIndexWindow_Impl::PreNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    MouseNotifyEvent nType = rNEvt.GetType();
    if ( MouseNotifyEvent::KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if (  KEY_TAB == nCode )
        {
            // don't exit index pane with <TAB>
            sal_uInt16 nPageId = 0;
            HelpTabPage_Impl* pCurPage = GetCurrentPage( nPageId );
            Control* pControl = pCurPage->GetLastFocusControl();
            bool bShift = rKeyCode.IsShift();
            bool bCtrl = rKeyCode.IsMod1();
            if ( !bCtrl && bShift && m_pActiveLB->HasChildPathFocus() )
            {
                pControl->GrabFocus();
                bDone = true;
            }
            else if ( !bCtrl && !bShift && pControl->HasChildPathFocus() )
            {
                m_pActiveLB->GrabFocus();
                bDone = true;
            }
            else if ( bCtrl )
            {
                sal_uInt16 nPagePos = m_pTabCtrl->GetPagePos(nPageId);
                sal_uInt16 nPageCount = m_pTabCtrl->GetPageCount();

                ++nPageId;
                // <CTRL><TAB> moves through the pages
                if (nPagePos >= nPageCount)
                    nPagePos = 0;

                m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId(nPagePos));
                ActivatePageHdl( m_pTabCtrl );
                bDone = true;
            }
         }
        else if ( m_pTabCtrl->HasFocus() && ( KEY_LEFT == nCode || KEY_RIGHT == nCode ) )
        {
            bWasCursorLeftOrRight = true;
        }
    }

    return bDone || Window::PreNotify( rNEvt );
}


void SfxHelpIndexWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) ||
           ( rDCEvt.GetType() == DataChangedEventType::DISPLAY ) ) &&
         ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );
    }
}


void SfxHelpIndexWindow_Impl::SetDoubleClickHdl( const Link<Control*,bool>& rLink )
{
    aPageDoubleClickLink = rLink;
}

IMPL_LINK(SfxHelpIndexWindow_Impl, ContentTabPageDoubleClickHdl, SvTreeListBox*, p, bool)
{
    aPageDoubleClickLink.Call(p);
    return true;
}

IMPL_LINK(SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl, ListBox&, r, void)
{
    aPageDoubleClickLink.Call(&r);
}


void SfxHelpIndexWindow_Impl::SetFactory( const OUString& rFactory, bool bActive )
{
    if ( !rFactory.isEmpty() )
    {
        GetIndexPage()->SetFactory( rFactory );
        // the index page did a check if rFactory is valid,
        // so the index page always returns a valid factory
        GetSearchPage()->SetFactory( GetIndexPage()->GetFactory() );
        if ( bActive )
            SetActiveFactory();
    }
}

OUString SfxHelpIndexWindow_Impl::GetSelectedEntry() const
{
    OUString sRet;

    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));

    if (sName == "contents")
    {
        sRet = pCPage->GetSelectedEntry();
    }
    else if (sName == "index")
    {
        sRet = pIPage->GetSelectedEntry();
    }
    else if (sName == "find")
    {
        sRet = pSPage->GetSelectedEntry();
    }
    else if (sName == "bookmarks")
    {
        sRet = pBPage->GetSelectedEntry();
    }

    return sRet;
}

void SfxHelpIndexWindow_Impl::AddBookmarks( const OUString& rTitle, const OUString& rURL )
{
    GetBookmarksPage()->AddBookmarks( rTitle, rURL );
}


bool SfxHelpIndexWindow_Impl::IsValidFactory( const OUString& _rFactory )
{
    bool bValid = false;
    for ( sal_Int32 i = 0; i < m_pActiveLB->GetEntryCount(); ++i )
    {
        OUString* pFactory = static_cast<OUString*>(m_pActiveLB->GetEntryData(i));
        if ( *pFactory == _rFactory )
        {
            bValid = true;
            break;
        }
    }
    return bValid;
}


void SfxHelpIndexWindow_Impl::ClearSearchPage()
{
    if ( pSPage )
        pSPage->ClearPage();
}

void SfxHelpIndexWindow_Impl::GrabFocusBack()
{
    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));

    if (sName == "contents" && pCPage)
        pCPage->SetFocusOnBox();
    else if (sName == "index" && pIPage)
        pIPage->SetFocusOnBox();
    else if (sName == "find" && pSPage)
        pSPage->SetFocusOnBox();
    else if (sName == "bookmarks" && pBPage)
        pBPage->SetFocusOnBox();
}

bool SfxHelpIndexWindow_Impl::HasFocusOnEdit() const
{
    bool bRet = false;
    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
    if (sName == "index" && pIPage)
        bRet = pIPage->HasFocusOnEdit();
    else if (sName == "find" && pSPage)
        bRet = pSPage->HasFocusOnEdit();
    return bRet;
}


OUString SfxHelpIndexWindow_Impl::GetSearchText() const
{
    OUString sRet;
    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
    if (sName == "find" && pSPage)
        sRet = pSPage->GetSearchText();
    return sRet;
}

bool SfxHelpIndexWindow_Impl::IsFullWordSearch() const
{
    bool bRet = false;
    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
    if (sName == "find" && pSPage)
        bRet = pSPage->IsFullWordSearch();
    return bRet;
}

void SfxHelpIndexWindow_Impl::OpenKeyword( const OUString& rKeyword )
{
    sKeyword = rKeyword;
    DBG_ASSERT( pIPage, "invalid index page" );
    pIPage->SetKeyword( sKeyword );
}

void SfxHelpIndexWindow_Impl::SelectExecutableEntry()
{
    OString sName(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
    if (sName == "index" && pIPage )
        pIPage->SelectExecutableEntry();
}

// class TextWin_Impl ----------------------------------------------------

TextWin_Impl::TextWin_Impl( vcl::Window* p ) : DockingWindow( p, 0 )
{
}

bool TextWin_Impl::EventNotify( NotifyEvent& rNEvt )
{
    if( ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT ) && rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB )
        return GetParent()->EventNotify( rNEvt );
    else
        return DockingWindow::EventNotify( rNEvt );
}


// remove docking area acceptor from layoutmanager, so it will not layout anything further .-)
static void lcl_disableLayoutOfFrame(const Reference< XFrame2 >& xFrame)
{
    xFrame->setLayoutManager( Reference< XLayoutManager >() );
}

// class SfxHelpTextWindow_Impl ------------------------------------------

SfxHelpTextWindow_Impl::SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent ) :

    Window( pParent, WB_CLIPCHILDREN | WB_TABSTOP | WB_DIALOGCONTROL ),

    aToolBox            ( VclPtr<ToolBox>::Create(this, 0) ),
    aOnStartupCB        ( VclPtr<CheckBox>::Create(this, WB_HIDE | WB_TABSTOP) ),
    aSelectIdle         ( "sfx2 appl SfxHelpTextWindow_Impl Select" ),
    aIndexOnImage       (StockImage::Yes, BMP_HELP_TOOLBOX_INDEX_ON),
    aIndexOffImage      (StockImage::Yes, BMP_HELP_TOOLBOX_INDEX_OFF),
    aIndexOnText        ( SfxResId( STR_HELP_BUTTON_INDEX_ON ) ),
    aIndexOffText       ( SfxResId( STR_HELP_BUTTON_INDEX_OFF ) ),
    aOnStartupText      ( SfxResId( RID_HELP_ONSTARTUP_TEXT ) ),
    pHelpWin            ( pParent ),
    pTextWin            ( VclPtr<TextWin_Impl>::Create( this ) ),
    nMinPos             ( 0 ),
    bIsDebug            ( false ),
    bIsIndexOn          ( false ),
    bIsInClose          ( false ),
    bIsFullWordSearch   ( false )
{
    aOnStartupCB->SetSizePixel(aOnStartupCB->PixelToLogic(Size(200, 10), MapMode(MapUnit::MapAppFont)));

    sfx2::AddToTaskPaneList( aToolBox.get() );

    xFrame = Frame::create( ::comphelper::getProcessComponentContext() );
    xFrame->initialize( VCLUnoHelper::GetInterface ( pTextWin ) );
    xFrame->setName( "OFFICE_HELP" );
    lcl_disableLayoutOfFrame(xFrame);

    aToolBox->SetHelpId( HID_HELP_TOOLBOX );

    aToolBox->InsertItem( TBI_INDEX, aIndexOffText );
    aToolBox->SetHelpId( TBI_INDEX, HID_HELP_TOOLBOXITEM_INDEX );
    aToolBox->InsertSeparator();
    aToolBox->InsertItem( TBI_BACKWARD, SfxResId( STR_HELP_BUTTON_PREV ) );
    aToolBox->SetHelpId( TBI_BACKWARD, HID_HELP_TOOLBOXITEM_BACKWARD );
    aToolBox->InsertItem( TBI_FORWARD, SfxResId( STR_HELP_BUTTON_NEXT ) );
    aToolBox->SetHelpId( TBI_FORWARD, HID_HELP_TOOLBOXITEM_FORWARD );
    aToolBox->InsertItem( TBI_START, SfxResId( STR_HELP_BUTTON_START ) );
    aToolBox->SetHelpId( TBI_START, HID_HELP_TOOLBOXITEM_START );
    aToolBox->InsertSeparator();
    aToolBox->InsertItem( TBI_PRINT, SfxResId( STR_HELP_BUTTON_PRINT ) );
    aToolBox->SetHelpId( TBI_PRINT, HID_HELP_TOOLBOXITEM_PRINT );
    aToolBox->InsertItem( TBI_BOOKMARKS, SfxResId( STR_HELP_BUTTON_ADDBOOKMARK ) );
    aToolBox->SetHelpId( TBI_BOOKMARKS, HID_HELP_TOOLBOXITEM_BOOKMARKS );
    aToolBox->InsertItem( TBI_SEARCHDIALOG, SfxResId( STR_HELP_BUTTON_SEARCHDIALOG ) );
    aToolBox->SetHelpId( TBI_SEARCHDIALOG, HID_HELP_TOOLBOXITEM_SEARCHDIALOG );

    InitToolBoxImages();
    aToolBox->Show();
    InitOnStartupBox();
    aOnStartupCB->SetClickHdl( LINK( this, SfxHelpTextWindow_Impl, CheckHdl ) );

    aSelectIdle.SetInvokeHandler( LINK( this, SfxHelpTextWindow_Impl, SelectHdl ) );
    aSelectIdle.SetPriority( TaskPriority::LOWEST );

    char* pEnv = getenv( "help_debug" );
    if ( pEnv )
        bIsDebug = true;

    SvtMiscOptions().AddListenerLink( LINK( this, SfxHelpTextWindow_Impl, NotifyHdl ) );
}

SfxHelpTextWindow_Impl::~SfxHelpTextWindow_Impl()
{
    disposeOnce();
}

void SfxHelpTextWindow_Impl::dispose()
{
    sfx2::RemoveFromTaskPaneList( aToolBox.get() );

    bIsInClose = true;
    SvtMiscOptions().RemoveListenerLink( LINK( this, SfxHelpTextWindow_Impl, NotifyHdl ) );
    m_xSrchDlg.reset();
    aToolBox.disposeAndClear();
    aOnStartupCB.disposeAndClear();
    pHelpWin.clear();
    pTextWin.disposeAndClear();
    vcl::Window::dispose();
}


bool SfxHelpTextWindow_Impl::HasSelection() const
{
    // is there any selection in the text and not only a cursor?
    bool bRet = false;
    Reference < XTextRange > xRange = getCursor();
    if ( xRange.is() )
    {
        Reference < XText > xText = xRange->getText();
        Reference < XTextCursor > xCursor = xText->createTextCursorByRange( xRange );
        bRet = !xCursor->isCollapsed();
    }

    return bRet;
}


void SfxHelpTextWindow_Impl::InitToolBoxImages()
{
    bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();

    aIndexOnImage  = Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_INDEX_ON) : OUString(BMP_HELP_TOOLBOX_INDEX_ON));
    aIndexOffImage = Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_INDEX_OFF) : OUString(BMP_HELP_TOOLBOX_INDEX_OFF));

    aToolBox->SetItemImage( TBI_INDEX, bIsIndexOn ? aIndexOffImage : aIndexOnImage );

    aToolBox->SetItemImage( TBI_BACKWARD,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_PREV) : OUString(BMP_HELP_TOOLBOX_PREV))
    );

    aToolBox->SetItemImage( TBI_FORWARD,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_NEXT) : OUString(BMP_HELP_TOOLBOX_NEXT))
    );

    aToolBox->SetItemImage( TBI_START,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_START) : OUString(BMP_HELP_TOOLBOX_START))
    );

    aToolBox->SetItemImage( TBI_PRINT,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_PRINT) : OUString(BMP_HELP_TOOLBOX_PRINT))
    );

    aToolBox->SetItemImage( TBI_BOOKMARKS,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_BOOKMARKS) : OUString(BMP_HELP_TOOLBOX_BOOKMARKS))
    );

    aToolBox->SetItemImage( TBI_SEARCHDIALOG,
                           Image(StockImage::Yes, bLarge ? OUString(BMP_HELP_TOOLBOX_L_SEARCHDIALOG) : OUString(BMP_HELP_TOOLBOX_SEARCHDIALOG))
    );

    Size aSize = aToolBox->CalcWindowSizePixel();
    aSize.AdjustHeight(TOOLBOX_OFFSET );
    aToolBox->SetPosSizePixel( Point( 0, TOOLBOX_OFFSET ), aSize );

    SvtMiscOptions aMiscOptions;
    if ( aMiscOptions.GetToolboxStyle() != aToolBox->GetOutStyle() )
        aToolBox->SetOutStyle( aMiscOptions.GetToolboxStyle() );
}


void SfxHelpTextWindow_Impl::InitOnStartupBox()
{
    sCurrentFactory = SfxHelp::GetCurrentModuleIdentifier();

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    const OUString sPath { PATH_OFFICE_FACTORIES + sCurrentFactory };

    // Attention: This check boy knows two states:
    // 1) Reading of the config key fails with an exception or by getting an empty Any (!) => check box must be hidden
    // 2) We read sal_True/sal_False => check box must be shown and enabled/disabled

    bool bHideBox = true;
    bool bHelpAtStartup = false;
    try
    {
        xConfiguration = ConfigurationHelper::openConfig(
            xContext, PACKAGE_SETUP, EConfigurationModes::Standard );
        if ( xConfiguration.is() )
        {
            Any aAny = ConfigurationHelper::readRelativeKey( xConfiguration, sPath, KEY_HELP_ON_OPEN );
            if (aAny >>= bHelpAtStartup)
                bHideBox = false;
        }
    }
    catch( Exception& )
    {
        bHideBox = true;
    }

    if ( bHideBox )
        aOnStartupCB->Hide();
    else
    {
        // detect module name
        OUString sModuleName;

        if ( xConfiguration.is() )
        {
            OUString sTemp;
            try
            {
                Any aAny = ConfigurationHelper::readRelativeKey( xConfiguration, sPath, KEY_UI_NAME );
                aAny >>= sTemp;
            }
            catch( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "sfx.appl", "SfxHelpTextWindow_Impl::InitOnStartupBox(): unexpected exception " << exceptionToString(ex) );
            }
            sModuleName = sTemp;
        }

        if ( !sModuleName.isEmpty() )
        {
            // set module name in checkbox text
            aOnStartupCB->SetText( aOnStartupText.replaceFirst( "%MODULENAME", sModuleName ) );
            // and show it
            aOnStartupCB->Show();
            // set check state
            aOnStartupCB->Check( bHelpAtStartup );
            aOnStartupCB->SaveValue();

            // calculate and set optimal width of the onstartup checkbox
            long nTextWidth = aOnStartupCB->GetTextWidth( "XXX" + aOnStartupCB->GetText() );
            Size aSize = aOnStartupCB->GetSizePixel();
            aSize.setWidth( nTextWidth );
            aOnStartupCB->SetSizePixel( aSize );
            SetOnStartupBoxPosition();
        }

        // set position of the checkbox
        Size a3Size = LogicToPixel(Size(3, 3), MapMode(MapUnit::MapAppFont));
        Size aTBSize = aToolBox->GetSizePixel();
        Size aCBSize = aOnStartupCB->GetSizePixel();
        Point aPnt = aToolBox->GetPosPixel();
        aPnt.AdjustX(aTBSize.Width() + a3Size.Width() );
        aPnt.AdjustY( ( aTBSize.Height() - aCBSize.Height() ) / 2 );
        aOnStartupCB->SetPosPixel( aPnt );
        nMinPos = aPnt.X();
    }
}


void SfxHelpTextWindow_Impl::SetOnStartupBoxPosition()
{
    long nX = std::max( GetOutputSizePixel().Width() - aOnStartupCB->GetSizePixel().Width(), nMinPos );
    Point aPos = aOnStartupCB->GetPosPixel();
    aPos.setX( nX );
    aOnStartupCB->SetPosPixel( aPos );
}


Reference< XBreakIterator > const & SfxHelpTextWindow_Impl::GetBreakIterator()
{
    if ( !xBreakIterator.is() )
        xBreakIterator = vcl::unohelper::CreateBreakIterator();
    DBG_ASSERT( xBreakIterator.is(), "Could not create BreakIterator" );
    return xBreakIterator;
}


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
        OSL_FAIL( "SfxHelpTextWindow_Impl::getCursor(): unexpected exception" );
    }

    return xCursor;
}


bool SfxHelpTextWindow_Impl::isHandledKey( const vcl::KeyCode& _rKeyCode )
{
    bool bRet = false;
    sal_uInt16 nCode = _rKeyCode.GetCode();

    // the keys <CTRL><A> (select all), <CTRL><C> (copy),
    //          <CTRL><F> (find), <CTRL><P> (print) and <CTRL><W> (close window)
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


IMPL_LINK_NOARG(SfxHelpTextWindow_Impl, SelectHdl, Timer *, void)
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
                xSrchDesc->setPropertyValue( "SearchRegularExpression", makeAny( true ) );
                if ( bIsFullWordSearch )
                    xSrchDesc->setPropertyValue( "SearchWords", makeAny( true ) );

                xSrchDesc->setSearchString( sfx2::PrepareSearchString( aSearchText, GetBreakIterator(), false ) );
                Reference< XIndexAccess > xSelection = xSearchable->findAll( xSrchDesc );

                // then select all found words
                Reference < XSelectionSupplier > xSelectionSup( xController, UNO_QUERY );
                if ( xSelectionSup.is() )
                {
                    xSelectionSup->select( Any(xSelection) );
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }
}


IMPL_LINK_NOARG( SfxHelpTextWindow_Impl, NotifyHdl, LinkParamNone*, void )
{
    InitToolBoxImages();
    Resize();
    aToolBox->Invalidate();
}


IMPL_LINK( SfxHelpTextWindow_Impl, FindHdl, sfx2::SearchDialog&, rDlg, void )
{
    FindHdl(&rDlg);
}
void SfxHelpTextWindow_Impl::FindHdl(sfx2::SearchDialog* pDlg)
{
    bool bWrapAround = ( nullptr == pDlg );
    if ( bWrapAround )
        pDlg = m_xSrchDlg.get();
    DBG_ASSERT( pDlg, "invalid search dialog" );
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
                xSrchDesc->setPropertyValue( "SearchWords", makeAny(pDlg->IsOnlyWholeWords()) );
                xSrchDesc->setPropertyValue( "SearchCaseSensitive", makeAny(pDlg->IsMarchCase()) );
                xSrchDesc->setPropertyValue( "SearchBackwards", makeAny(pDlg->IsSearchBackwards()) );
                xSrchDesc->setSearchString( pDlg->GetSearchText() );
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
                        xSelectionSup->select( Any(xSelection) );
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
                                xTVCrsr->gotoRange( xText->getEnd(), false );
                            else
                                xTVCrsr->gotoRange( xText->getStart(), false );
                            FindHdl( nullptr );
                        }
                    }
                }
                else
                {
                    assert(m_xSrchDlg && "no search dialog");
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xSrchDlg->getDialog(),
                                                              VclMessageType::Info, VclButtonsType::Ok, SfxResId(STR_INFO_NOSEARCHTEXTFOUND)));
                    xBox->run();
                    m_xSrchDlg->SetFocusOnEdit();
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }
}

IMPL_LINK_NOARG( SfxHelpTextWindow_Impl, CloseHdl, LinkParamNone*, void )
{
    m_xSrchDlg.reset();
}

IMPL_LINK( SfxHelpTextWindow_Impl, CheckHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    if ( !xConfiguration.is() )
        return;

    bool bChecked = pBox->IsChecked();
    try
    {
        ConfigurationHelper::writeRelativeKey(
            xConfiguration, PATH_OFFICE_FACTORIES + sCurrentFactory, KEY_HELP_ON_OPEN, makeAny( bChecked ) );
        ConfigurationHelper::flush( xConfiguration );
    }
    catch( Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.appl", "SfxHelpTextWindow_Impl::CheckHdl(): unexpected exception " << exceptionToString(ex) );
    }
}


void SfxHelpTextWindow_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    long nToolBoxHeight = aToolBox->GetSizePixel().Height() + TOOLBOX_OFFSET;
    aSize.AdjustHeight( -nToolBoxHeight );
    pTextWin->SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
    SetOnStartupBoxPosition();
}


bool SfxHelpTextWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    MouseNotifyEvent nType = rNEvt.GetType();
    if ( MouseNotifyEvent::COMMAND == nType && rNEvt.GetCommandEvent() )
    {
        const CommandEvent* pCmdEvt = rNEvt.GetCommandEvent();
        vcl::Window* pCmdWin = rNEvt.GetWindow();

        if ( pCmdEvt->GetCommand() == CommandEventId::ContextMenu && pCmdWin != this && pCmdWin != aToolBox.get() )
        {
            Point aPos;
            if ( pCmdEvt->IsMouseEvent() )
                aPos = pCmdEvt->GetMousePosPixel();
            else
                aPos = Point( pTextWin->GetPosPixel().X() + 20, 20 );
            aPos.AdjustY(pTextWin->GetPosPixel().Y() );
            ScopedVclPtrInstance<PopupMenu> aMenu;
            if ( bIsIndexOn )
                aMenu->InsertItem(TBI_INDEX, aIndexOffText, Image(StockImage::Yes, BMP_HELP_TOOLBOX_INDEX_OFF));
            else
                aMenu->InsertItem(TBI_INDEX, aIndexOnText, Image(StockImage::Yes, BMP_HELP_TOOLBOX_INDEX_ON));

            aMenu->SetHelpId( TBI_INDEX, HID_HELP_TOOLBOXITEM_INDEX );
            aMenu->InsertSeparator();
            aMenu->InsertItem( TBI_BACKWARD,
                              SfxResId( STR_HELP_BUTTON_PREV  ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_PREV)
            );
            aMenu->SetHelpId( TBI_BACKWARD, HID_HELP_TOOLBOXITEM_BACKWARD );
            aMenu->EnableItem( TBI_BACKWARD, pHelpWin->HasHistoryPredecessor() );
            aMenu->InsertItem( TBI_FORWARD,
                              SfxResId( STR_HELP_BUTTON_NEXT ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_NEXT)
            );
            aMenu->SetHelpId( TBI_FORWARD, HID_HELP_TOOLBOXITEM_FORWARD );
            aMenu->EnableItem( TBI_FORWARD, pHelpWin->HasHistorySuccessor() );
            aMenu->InsertItem( TBI_START,
                              SfxResId( STR_HELP_BUTTON_START ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_START)
            );
            aMenu->SetHelpId( TBI_START, HID_HELP_TOOLBOXITEM_START );
            aMenu->InsertSeparator();
            aMenu->InsertItem( TBI_PRINT,
                              SfxResId( STR_HELP_BUTTON_PRINT ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_PRINT)
            );
            aMenu->SetHelpId( TBI_PRINT, HID_HELP_TOOLBOXITEM_PRINT );
            aMenu->InsertItem( TBI_BOOKMARKS,
                              SfxResId( STR_HELP_BUTTON_ADDBOOKMARK ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_BOOKMARKS)
             );
            aMenu->SetHelpId( TBI_BOOKMARKS, HID_HELP_TOOLBOXITEM_BOOKMARKS );
            aMenu->InsertItem( TBI_SEARCHDIALOG,
                              SfxResId( STR_HELP_BUTTON_SEARCHDIALOG ),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_SEARCHDIALOG)
            );
            aMenu->SetHelpId( TBI_SEARCHDIALOG, HID_HELP_TOOLBOXITEM_SEARCHDIALOG );
            aMenu->InsertSeparator();
            aMenu->InsertItem( TBI_SELECTIONMODE, SfxResId( STR_HELP_MENU_TEXT_SELECTION_MODE ) );
            aMenu->SetHelpId( TBI_SELECTIONMODE, HID_HELP_TEXT_SELECTION_MODE );
            URL aURL;
            aURL.Complete = ".uno:SelectTextMode";
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict(aURL);
            Reference < XDispatch > xDisp = xFrame->queryDispatch( aURL, OUString(), 0 );
            if(xDisp.is())
            {
                rtl::Reference<HelpStatusListener_Impl> pStateListener =
                                        new HelpStatusListener_Impl(xDisp, aURL );
                FeatureStateEvent rEvent = pStateListener->GetStateEvent();
                bool bCheck = false;
                rEvent.State >>= bCheck;
                aMenu->CheckItem(TBI_SELECTIONMODE, bCheck);
            }
            aMenu->InsertSeparator();
            aMenu->InsertItem( TBI_COPY,
                              SfxResId(STR_HELP_MENU_TEXT_COPY),
                              Image(StockImage::Yes, BMP_HELP_TOOLBOX_COPY)
                );
            aMenu->SetHelpId( TBI_COPY, ".uno:Copy" );
            aMenu->EnableItem( TBI_COPY, HasSelection() );

            if ( bIsDebug )
            {
                aMenu->InsertSeparator();
                aMenu->InsertItem( TBI_SOURCEVIEW, SfxResId(STR_HELP_BUTTON_SOURCEVIEW) );
            }

            if( ! SvtMenuOptions().IsEntryHidingEnabled() )
                aMenu->SetMenuFlags( aMenu->GetMenuFlags() | MenuFlags::HideDisabledEntries );

            sal_uInt16 nId = aMenu->Execute( this, aPos );
            pHelpWin->DoAction( nId );
            bDone = true;
        }
    }
    else if ( MouseNotifyEvent::KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode = pKEvt->GetKeyCode();
        sal_uInt16 nKeyGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();
        if ( KEYGROUP_ALPHA == nKeyGroup &&  !isHandledKey( rKeyCode ) )
        {
            // do nothing disables the writer accelerators
            bDone = true;
         }
        else if ( rKeyCode.IsMod1() && ( KEY_F4 == nKey || KEY_W == nKey ) )
        {
            // <CTRL><F4> or <CTRL><W> -> close top frame
            pHelpWin->CloseWindow();
            bDone = true;
        }
        else if ( KEY_TAB == nKey && aOnStartupCB->HasChildPathFocus() )
        {
            aToolBox->GrabFocus();
            bDone = true;
        }
    }

    return bDone || Window::PreNotify( rNEvt );
}


void SfxHelpTextWindow_Impl::GetFocus()
{
    if ( bIsInClose )
        return;

    try
    {
        if( xFrame.is() )
        {
            Reference< css::awt::XWindow > xWindow = xFrame->getComponentWindow();
            if( xWindow.is() )
                xWindow->setFocus();
        }
    }
    catch( Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.appl", "SfxHelpTextWindow_Impl::GetFocus(): unexpected exception " << exceptionToString(ex) );
    }
}


void SfxHelpTextWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) ||
           ( rDCEvt.GetType() == DataChangedEventType::DISPLAY ) ) &&
         ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );
        InitToolBoxImages();
    }
}


void SfxHelpTextWindow_Impl::ToggleIndex( bool bOn )
{
    bIsIndexOn = bOn;
    if ( bIsIndexOn )
    {
        aToolBox->SetItemImage( TBI_INDEX, aIndexOffImage );
        aToolBox->SetItemText( TBI_INDEX, aIndexOffText );
    }
    else
    {
        aToolBox->SetItemImage( TBI_INDEX, aIndexOnImage );
        aToolBox->SetItemText( TBI_INDEX, aIndexOnText );
    }
}


void SfxHelpTextWindow_Impl::SelectSearchText( const OUString& rSearchText, bool _bIsFullWordSearch )
{
    aSearchText = rSearchText;
    bIsFullWordSearch = _bIsFullWordSearch;
    aSelectIdle.Start();
}


void SfxHelpTextWindow_Impl::SetPageStyleHeaderOff() const
{
    bool bSetOff = false;
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
                    OUString sStyleName;
                    if ( xProps->getPropertyValue( "PageStyleName" ) >>= sStyleName )
                    {
                        Reference < XStyleFamiliesSupplier > xStyles( xController->getModel(), UNO_QUERY );
                        Reference < XNameContainer > xContainer;
                        if ( xStyles->getStyleFamilies()->getByName( "PageStyles" )
                             >>= xContainer )
                        {
                            Reference < XStyle > xStyle;
                            if ( xContainer->getByName( sStyleName ) >>= xStyle )
                            {
                                Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
                                xPropSet->setPropertyValue( "HeaderIsOn",  makeAny( false ) );

                                Reference< XModifiable > xReset(xStyles, UNO_QUERY);
                                xReset->setModified(false);
                                bSetOff = true;
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.appl", "SfxHelpTextWindow_Impl::SetPageStyleHeaderOff(): unexpected exception " << exceptionToString(ex) );
    }

    SAL_WARN_IF( !bSetOff, "sfx.appl", "SfxHelpTextWindow_Impl::SetPageStyleHeaderOff(): set off failed" );
}


void SfxHelpTextWindow_Impl::CloseFrame()
{
    bIsInClose = true;
    try
    {
        css::uno::Reference< css::util::XCloseable > xCloseable  ( xFrame, css::uno::UNO_QUERY );
        if (xCloseable.is())
            xCloseable->close(true);
    }
    catch( css::util::CloseVetoException& )
    {
    }
}


void SfxHelpTextWindow_Impl::DoSearch()
{
    if (m_xSrchDlg)
        return;

    // create the search dialog
    m_xSrchDlg.reset(new sfx2::SearchDialog(pTextWin->GetFrameWeld(), "HelpSearchDialog"));
    // set handler
    m_xSrchDlg->SetFindHdl( LINK( this, SfxHelpTextWindow_Impl, FindHdl ) );
    m_xSrchDlg->SetCloseHdl( LINK( this, SfxHelpTextWindow_Impl, CloseHdl ) );
    // get selected text of the help page to set it as the search text
    Reference< XTextRange > xCursor = getCursor();
    if ( xCursor.is() )
    {
        OUString sText = xCursor->getString();
        if ( !sText.isEmpty() )
            m_xSrchDlg->SetSearchText( sText );
    }
    sfx2::SearchDialog::runAsync(m_xSrchDlg);
}

// class SfxHelpWindow_Impl ----------------------------------------------

void SfxHelpWindow_Impl::Resize()
{
    SplitWindow::Resize();
    InitSizes();
}


void SfxHelpWindow_Impl::Split()
{
    static const long nMinSplitSize = 5;
    static const long nMaxSplitSize = 99 - nMinSplitSize;

    SplitWindow::Split();

    nIndexSize = GetItemSize( INDEXWIN_ID );
    nTextSize = GetItemSize( TEXTWIN_ID );

    bool        bMod = false;
    if( nIndexSize < nMinSplitSize )
    {
        nIndexSize = nMinSplitSize;
        nTextSize = nMaxSplitSize;

        bMod = true;
    }
    else if( nTextSize < nMinSplitSize )
    {
        nTextSize = nMinSplitSize;
        nIndexSize = nMaxSplitSize;

        bMod = true;
    }
    else
        bMod = false;

    if( bMod )
    {
        SetItemSize( INDEXWIN_ID, nIndexSize );
        SetItemSize( TEXTWIN_ID, nTextSize );
    }

    InitSizes();
}

void SfxHelpWindow_Impl::GetFocus()
{
    if( pTextWin )
        pTextWin->GrabFocus();
    else
        vcl::Window::GetFocus();
}

void SfxHelpWindow_Impl::MakeLayout()
{
    if ( nHeight > 0 && xWindow.is() )
    {
        VclPtr<vcl::Window> pScreenWin = VCLUnoHelper::GetWindow(xWindow);

        /* #i55528#
            Hide() / Show() will produce strange effects.
            The returned size (used later to be written back into the configuration)
            is not the right after a resize during the window is hidden.
            If this resize is done if the window is visible everything works as expected.
            Some VCL-patches could not solve this problem so I've established the
            workaround: resize the help window if it's visible .-)
        */
        css::awt::Rectangle aRect = xWindow->getPosSize();
        sal_Int32 nOldWidth = bIndex ? nCollapseWidth : nExpandWidth;
        sal_Int32 nWidth = bIndex ? nExpandWidth : nCollapseWidth;
        xWindow->setPosSize( aRect.X, aRect.Y, nWidth, nHeight, css::awt::PosSize::SIZE );

        if ( aRect.Width > 0 && aRect.Height > 0 )
        {
            tools::Rectangle aScreenRect = pScreenWin->GetClientWindowExtentsRelative();
            Point aNewPos = aScreenRect.TopLeft();
            sal_Int32 nDiffWidth = nOldWidth - nWidth;
            aNewPos.AdjustX(nDiffWidth );
            pScreenWin->SetPosPixel( aNewPos );
        }
        else if ( aWinPos.X() > 0 && aWinPos.Y() > 0 )
            pScreenWin->SetPosPixel( aWinPos );
    }

    Clear();

    if ( bIndex )
    {
        pIndexWin->Show();
        InsertItem( COLSET_ID, 100, SPLITWINDOW_APPEND, SPLITSET_ID, SplitWindowItemFlags::PercentSize | SplitWindowItemFlags::ColSet );
        InsertItem( INDEXWIN_ID, pIndexWin, nIndexSize, SPLITWINDOW_APPEND, COLSET_ID, SplitWindowItemFlags::PercentSize );
        InsertItem( TEXTWIN_ID, pTextWin, nTextSize, SPLITWINDOW_APPEND, COLSET_ID, SplitWindowItemFlags::PercentSize );
    }
    else
    {
        pIndexWin->Hide();
        InsertItem( COLSET_ID, 100, SPLITWINDOW_APPEND, SPLITSET_ID, SplitWindowItemFlags::PercentSize | SplitWindowItemFlags::ColSet );
        InsertItem( TEXTWIN_ID, pTextWin, 100, SPLITWINDOW_APPEND, 1, SplitWindowItemFlags::PercentSize );
    }
}


void SfxHelpWindow_Impl::InitSizes()
{
    if ( !xWindow.is() )
        return;

    css::awt::Rectangle aRect = xWindow->getPosSize();
    nHeight = aRect.Height;

    if ( bIndex )
    {
        nExpandWidth = aRect.Width;
        nCollapseWidth = nExpandWidth * nTextSize / 100;
    }
    else
    {
        nCollapseWidth = aRect.Width;
        nExpandWidth = nTextSize ? nCollapseWidth * 100 / nTextSize : 0;
    }
}


void SfxHelpWindow_Impl::LoadConfig()
{
    SvtViewOptions aViewOpt( EViewType::Window, CONFIGNAME_HELPWIN );
    if ( !aViewOpt.Exists() )
        return;

    bIndex = aViewOpt.IsVisible();
    Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
    OUString aUserData;
    if ( aUserItem >>= aUserData )
    {
        DBG_ASSERT( comphelper::string::getTokenCount(aUserData, ';') == 6, "invalid user data" );
        sal_Int32 nIdx = 0;
        nIndexSize = aUserData.getToken( 0, ';', nIdx ).toInt32();
        nTextSize = aUserData.getToken( 0, ';', nIdx ).toInt32();
        sal_Int32 nWidth = aUserData.getToken( 0, ';', nIdx ).toInt32();
        nHeight = aUserData.getToken( 0, ';', nIdx ).toInt32();
        aWinPos.setX( aUserData.getToken( 0, ';', nIdx ).toInt32() );
        aWinPos.setY( aUserData.getToken( 0, ';', nIdx ).toInt32() );
        if ( bIndex )
        {
            nExpandWidth = nWidth;
            nCollapseWidth = nExpandWidth * nTextSize / 100;
        }
        else if (nTextSize != 0)
        {
            nCollapseWidth = nWidth;
            nExpandWidth = nCollapseWidth * 100 / nTextSize;
        }
    }

    pTextWin->ToggleIndex( bIndex );
}


void SfxHelpWindow_Impl::SaveConfig()
{
    SvtViewOptions aViewOpt( EViewType::Window, CONFIGNAME_HELPWIN );
    sal_Int32 nW = 0, nH = 0;

    if ( xWindow.is() )
    {
        css::awt::Rectangle aRect = xWindow->getPosSize();
        nW = aRect.Width;
        nH = aRect.Height;
    }

    aViewOpt.SetVisible( bIndex );

    VclPtr<vcl::Window> pScreenWin = VCLUnoHelper::GetWindow( xWindow );
    aWinPos = pScreenWin->GetWindowExtentsRelative( nullptr ).TopLeft();
    const OUString aUserData = OUString::number( nIndexSize )
        + ";" + OUString::number( nTextSize )
        + ";" + OUString::number( nW )
        + ";" + OUString::number( nH )
        + ";" + OUString::number( aWinPos.X() )
        + ";" + OUString::number( aWinPos.Y() );

    aViewOpt.SetUserItem( USERITEM_NAME, makeAny( aUserData ) );
}


void SfxHelpWindow_Impl::ShowStartPage()
{
    loadHelpContent(SfxHelpWindow_Impl::buildHelpURL(pIndexWin->GetFactory(), "/start", OUString()));
}


IMPL_LINK( SfxHelpWindow_Impl, SelectHdl, ToolBox* , pToolBox, void )
{
    if ( pToolBox )
    {
        bGrabFocusToToolBox = pToolBox->HasChildPathFocus();
        DoAction( pToolBox->GetCurItemId() );
    }
}


IMPL_LINK_NOARG(SfxHelpWindow_Impl, OpenHdl, Control*, bool)
{
    pIndexWin->SelectExecutableEntry();
    OUString aEntry = pIndexWin->GetSelectedEntry();

    if ( aEntry.isEmpty() )
        return false;

    OUString sHelpURL;

    bool bComplete = aEntry.toAsciiLowerCase().match("vnd.sun.star.help");

    if (bComplete)
        sHelpURL = aEntry;
    else
    {
        OUString aId;
        OUString aAnchor = OUString('#');
        if ( comphelper::string::getTokenCount(aEntry, '#') == 2 )
        {
            sal_Int32 nIdx{ 0 };
            aId = aEntry.getToken( 0, '#', nIdx );
            aAnchor += aEntry.getToken( 0, '#', nIdx );
        }
        else
            aId = aEntry;

        sHelpURL = SfxHelpWindow_Impl::buildHelpURL(pIndexWin->GetFactory(), "/" + aId, aAnchor);
    }

    loadHelpContent(sHelpURL);

    return false;
}


IMPL_LINK( SfxHelpWindow_Impl, SelectFactoryHdl, SfxHelpIndexWindow_Impl* , pWin, void )
{
    if ( sTitle.isEmpty() )
        sTitle = GetParent()->GetText();

    Reference< XTitle > xTitle(xFrame, UNO_QUERY);
    if (xTitle.is ())
        xTitle->setTitle(sTitle + " - " + pIndexWin->GetActiveFactoryTitle());

    if ( pWin )
        ShowStartPage();
    pIndexWin->ClearSearchPage();
}


IMPL_LINK( SfxHelpWindow_Impl, ChangeHdl, HelpListener_Impl&, rListener, void )
{
    SetFactory( rListener.GetFactory() );
}


void SfxHelpWindow_Impl::openDone(const OUString& sURL    ,
                                        bool         bSuccess)
{
    INetURLObject aObj( sURL );
    if ( aObj.GetProtocol() == INetProtocol::VndSunStarHelp )
        SetFactory( aObj.GetHost() );
    if ( IsWait() )
        LeaveWait();
    if ( bGrabFocusToToolBox )
    {
        pTextWin->GetToolBox().GrabFocus();
        bGrabFocusToToolBox = false;
    }
    else
        pIndexWin->GrabFocusBack();
    if ( !bSuccess )
        return;

    // set some view settings: "prevent help tips" and "helpid == 68245"
    try
    {
        Reference < XController > xController = pTextWin->getFrame()->getController();
        if ( xController.is() )
        {
            Reference < XViewSettingsSupplier > xSettings( xController, UNO_QUERY );
            Reference < XPropertySet > xViewProps = xSettings->getViewSettings();
            Reference< XPropertySetInfo > xInfo = xViewProps->getPropertySetInfo();
            xViewProps->setPropertyValue( "ShowContentTips", makeAny( false ) );
            xViewProps->setPropertyValue( "ShowGraphics", makeAny( true ) );
            xViewProps->setPropertyValue( "ShowTables", makeAny( true ) );
            xViewProps->setPropertyValue( "HelpURL", makeAny( OUString("HID:SFX2_HID_HELP_ONHELP") ) );
            OUString sProperty( "IsExecuteHyperlinks" );
            if ( xInfo->hasPropertyByName( sProperty ) )
                xViewProps->setPropertyValue( sProperty, makeAny( true ) );
            xController->restoreViewData(Any());
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "SfxHelpWindow_Impl::OpenDoneHdl(): unexpected exception" );
    }

    // When the SearchPage opens the help doc, then select all words, which are equal to its text
    OUString sSearchText = comphelper::string::strip(pIndexWin->GetSearchText(), ' ');
    if ( !sSearchText.isEmpty() )
        pTextWin->SelectSearchText( sSearchText, pIndexWin->IsFullWordSearch() );

    // no page style header -> this prevents a print output of the URL
    pTextWin->SetPageStyleHeaderOff();
}


SfxHelpWindow_Impl::SfxHelpWindow_Impl(
    const css::uno::Reference < css::frame::XFrame2 >& rFrame,
    vcl::Window* pParent ) :

    SplitWindow( pParent, WB_3DLOOK | WB_NOSPLITDRAW ),

    xFrame              ( rFrame ),
    pIndexWin           ( nullptr ),
    pTextWin            ( nullptr ),
    pHelpInterceptor    ( new HelpInterceptor_Impl() ),
    pHelpListener       ( new HelpListener_Impl( pHelpInterceptor ) ),
    nExpandWidth        ( 0 ),
    nCollapseWidth      ( 0 ),
    nHeight             ( 0 ),
    nIndexSize          ( 40 ),
    nTextSize           ( 60 ),
    bIndex              ( true ),
    bGrabFocusToToolBox ( false ),
    aWinPos             ( 0, 0 ),
    sTitle              ( pParent->GetText() )
{
    SetHelpId( HID_HELP_WINDOW );
    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    pHelpInterceptor->InitWaiter( this );
    pIndexWin = VclPtr<SfxHelpIndexWindow_Impl>::Create( this );
    pIndexWin->SetDoubleClickHdl( LINK( this, SfxHelpWindow_Impl, OpenHdl ) );
    pIndexWin->SetSelectFactoryHdl( LINK( this, SfxHelpWindow_Impl, SelectFactoryHdl ) );
    pIndexWin->SetSizePixel(LogicToPixel(Size(120, 200), MapMode(MapUnit::MapAppFont)));
    pIndexWin->Show();
    pTextWin = VclPtr<SfxHelpTextWindow_Impl>::Create( this );
    Reference < XFrames > xFrames = rFrame->getFrames();
    xFrames->append( Reference<XFrame>(pTextWin->getFrame(), UNO_QUERY_THROW) );
    pTextWin->SetSelectHdl( LINK( this, SfxHelpWindow_Impl, SelectHdl ) );
    pTextWin->Show();
    pHelpInterceptor->setInterception( Reference<XFrame>(pTextWin->getFrame(), UNO_QUERY_THROW) );
    pHelpListener->SetChangeHdl( LINK( this, SfxHelpWindow_Impl, ChangeHdl ) );
    LoadConfig();
}


SfxHelpWindow_Impl::~SfxHelpWindow_Impl()
{
    disposeOnce();
}

void SfxHelpWindow_Impl::dispose()
{
    SaveConfig();
    pIndexWin.disposeAndClear();
    pTextWin->CloseFrame();
    pTextWin.disposeAndClear();
    SplitWindow::dispose();
}

bool SfxHelpWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        // Backward == <ALT><LEFT> or <BACKSPACE> Forward == <ALT><RIGHT>
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = rKeyCode.GetCode();
        if ( ( rKeyCode.IsMod2() && ( KEY_LEFT == nKey || KEY_RIGHT == nKey ) ) ||
             ( !rKeyCode.GetModifier() && KEY_BACKSPACE == nKey && !pIndexWin->HasFocusOnEdit() ) )
        {
            DoAction( rKeyCode.GetCode() == KEY_RIGHT ? TBI_FORWARD : TBI_BACKWARD );
            bHandled = true;
        }
        else if ( rKeyCode.IsMod1() && ( KEY_F4 == nKey || KEY_W == nKey ) )
        {
            // <CTRL><F4> or <CTRL><W> -> close top frame
            CloseWindow();
            bHandled = true;
        }
    }
    return bHandled || Window::PreNotify( rNEvt );
}

void SfxHelpWindow_Impl::setContainerWindow( const Reference < css::awt::XWindow >& xWin )
{
    xWindow = xWin;
    MakeLayout();
}

void SfxHelpWindow_Impl::SetFactory( const OUString& rFactory )
{
    pIndexWin->SetFactory( rFactory, true );
}


void SfxHelpWindow_Impl::SetHelpURL( const OUString& rURL )
{
    INetURLObject aObj( rURL );
    if ( aObj.GetProtocol() == INetProtocol::VndSunStarHelp )
        SetFactory( aObj.GetHost() );
}


void SfxHelpWindow_Impl::DoAction( sal_uInt16 nActionId )
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
            aURL.Complete = ".uno:Backward";
            if ( TBI_FORWARD == nActionId )
                aURL.Complete = ".uno:Forward";
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict(aURL);
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
                    aURL.Complete = ".uno:Print";
                else if ( TBI_SOURCEVIEW == nActionId )
                    aURL.Complete = ".uno:SourceView";
                else if ( TBI_COPY == nActionId )
                    aURL.Complete = ".uno:Copy";
                else // TBI_SELECTIONMODE == nActionId
                    aURL.Complete = ".uno:SelectTextMode";
                Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
                xTrans->parseStrict(aURL);
                Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
                if ( xDisp.is() )
                    xDisp->dispatch( aURL, Sequence < PropertyValue >() );
            }
            break;
        }

        case TBI_BOOKMARKS :
        {
            OUString aURL = pHelpInterceptor->GetCurrentURL();
            if ( !aURL.isEmpty() )
            {
                try
                {
                    Content aCnt( aURL, Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                    css::uno::Reference< css::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
                    if ( xInfo->hasPropertyByName( PROPERTY_TITLE ) )
                    {
                        css::uno::Any aAny = aCnt.getPropertyValue( PROPERTY_TITLE );
                        OUString aValue;
                        if ( aAny >>= aValue )
                        {
                            SfxAddHelpBookmarkDialog_Impl aDlg(GetFrameWeld(), false);
                            aDlg.SetTitle(aValue);
                            if (aDlg.run() == RET_OK )
                            {
                                pIndexWin->AddBookmarks( aDlg.GetTitle(), aURL );
                            }
                        }
                    }
                }
                catch( Exception& )
                {
                    OSL_FAIL( "SfxHelpWindow_Impl::DoAction(): unexpected exception" );
                }
            }
            break;
        }
    }
}


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
                xCloser->close( false );
        }
    }
    catch( Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.appl", "SfxHelpWindow_Impl::CloseWindow(): caught an exception " << exceptionToString(ex) );
    }
}


void SfxHelpWindow_Impl::UpdateToolbox()
{
    pTextWin->GetToolBox().EnableItem( TBI_BACKWARD, pHelpInterceptor->HasHistoryPred() );
    pTextWin->GetToolBox().EnableItem( TBI_FORWARD, pHelpInterceptor->HasHistorySucc() );
}


bool SfxHelpWindow_Impl::HasHistoryPredecessor() const
{
    return pHelpInterceptor->HasHistoryPred();
}


bool SfxHelpWindow_Impl::HasHistorySuccessor() const
{
    return pHelpInterceptor->HasHistorySucc();
}

// class SfxAddHelpBookmarkDialog_Impl -----------------------------------

SfxAddHelpBookmarkDialog_Impl::SfxAddHelpBookmarkDialog_Impl(weld::Window* pParent, bool bRename)
    : GenericDialogController(pParent, "sfx/ui/bookmarkdialog.ui", "BookmarkDialog")
    , m_xTitleED(m_xBuilder->weld_entry("entry"))
    , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
{
    if (bRename)
        m_xDialog->set_title(m_xAltTitle->get_label());
}

void SfxAddHelpBookmarkDialog_Impl::SetTitle(const OUString& rTitle)
{
    m_xTitleED->set_text(rTitle);
    m_xTitleED->select_region(0, -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
