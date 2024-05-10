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
#include <sfx2/sfxresid.hxx>
#include "helpinterceptor.hxx"
#include <helper.hxx>
#include <srchdlg.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <sfx2/strings.hrc>
#include <helpids.h>
#include <bitmaps.hlst>

#include <rtl/ustrbuf.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
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
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <unotools/historyoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <tools/urlobj.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/weld.hxx>

#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>

#include <string_view>
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

using namespace ::comphelper;

// defines ---------------------------------------------------------------

constexpr OUString CONFIGNAME_HELPWIN = u"OfficeHelp"_ustr;
constexpr OUString CONFIGNAME_INDEXWIN = u"OfficeHelpIndex"_ustr;
constexpr OUString CONFIGNAME_SEARCHPAGE = u"OfficeHelpSearch"_ustr;
constexpr OUString IMAGE_URL = u"private:factory/"_ustr;

constexpr OUString PROPERTY_KEYWORDLIST = u"KeywordList"_ustr;
constexpr OUString PROPERTY_KEYWORDREF = u"KeywordRef"_ustr;
constexpr OUString PROPERTY_ANCHORREF = u"KeywordAnchorForRef"_ustr;
constexpr OUString PROPERTY_TITLEREF = u"KeywordTitleForRef"_ustr;
constexpr OUString PROPERTY_TITLE = u"Title"_ustr;
constexpr OUString HELP_URL = u"vnd.sun.star.help://"_ustr;
constexpr OUStringLiteral HELP_SEARCH_TAG = u"/?Query=";
constexpr OUString USERITEM_NAME = u"UserItem"_ustr;

constexpr OUStringLiteral PACKAGE_SETUP = u"/org.openoffice.Setup";
constexpr OUString PATH_OFFICE_FACTORIES = u"Office/Factories/"_ustr;
constexpr OUString KEY_HELP_ON_OPEN = u"ooSetupFactoryHelpOnOpen"_ustr;
constexpr OUStringLiteral KEY_UI_NAME = u"ooSetupFactoryUIName";

namespace sfx2
{


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

        while ( aBoundary.startPos < aBoundary.endPos )
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

namespace {

struct IndexEntry_Impl
{
    bool            m_bSubEntry;
    OUString        m_aURL;

    IndexEntry_Impl( OUString aURL, bool bSubEntry ) :
        m_bSubEntry( bSubEntry ), m_aURL(std::move( aURL )) {}
};

// struct ContentEntry_Impl ----------------------------------------------

struct ContentEntry_Impl
{
    OUString    aURL;
    bool        bIsFolder;

    ContentEntry_Impl( OUString _aURL, bool bFolder ) :
        aURL(std::move( _aURL )), bIsFolder( bFolder ) {}
};

}

void ContentTabPage_Impl::InitRoot()
{
    std::vector< OUString > aList =
        SfxContentHelper::GetHelpTreeViewContents( u"vnd.sun.star.hier://com.sun.star.help.TreeView/"_ustr );

    for (const OUString & aRow : aList)
    {
        sal_Int32 nIdx = 0;
        OUString aTitle = aRow.getToken( 0, '\t', nIdx );
        OUString aURL = aRow.getToken( 0, '\t', nIdx );
        sal_Unicode cFolder = o3tl::getToken(aRow, 0, '\t', nIdx )[0];
        bool bIsFolder = ( '1' == cFolder );
        OUString sId;
        if (bIsFolder)
            sId = weld::toId(new ContentEntry_Impl(aURL, true));
        m_xContentBox->insert(nullptr, -1, &aTitle, &sId, nullptr, nullptr, true, m_xScratchIter.get());
        m_xContentBox->set_image(*m_xScratchIter, aClosedBookImage);
    }
}

void ContentTabPage_Impl::ClearChildren(const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xContentBox->make_iterator(pParent);
    bool bEntry = m_xContentBox->iter_children(*xEntry);
    while (bEntry)
    {
        ClearChildren(xEntry.get());
        delete weld::fromId<ContentEntry_Impl*>(m_xContentBox->get_id(*xEntry));
        bEntry = m_xContentBox->iter_next_sibling(*xEntry);
    }

}

IMPL_LINK(ContentTabPage_Impl, ExpandingHdl, const weld::TreeIter&, rIter, bool)
{
    ContentEntry_Impl* pContentEntry = weld::fromId<ContentEntry_Impl*>(m_xContentBox->get_id(rIter));
    if (!m_xContentBox->iter_has_child(rIter))
    {
        try
        {
            if (pContentEntry)
            {
                std::vector<OUString > aList = SfxContentHelper::GetHelpTreeViewContents(pContentEntry->aURL);

                for (const OUString & aRow : aList)
                {
                    sal_Int32 nIdx = 0;
                    OUString aTitle = aRow.getToken( 0, '\t', nIdx );
                    OUString aURL = aRow.getToken( 0, '\t', nIdx );
                    sal_Unicode cFolder = o3tl::getToken(aRow, 0, '\t', nIdx )[0];
                    bool bIsFolder = ( '1' == cFolder );
                    if ( bIsFolder )
                    {
                        OUString sId = weld::toId(new ContentEntry_Impl(aURL, true));
                        m_xContentBox->insert(&rIter, -1, &aTitle, &sId, nullptr, nullptr, true, m_xScratchIter.get());
                        m_xContentBox->set_image(*m_xScratchIter, aClosedBookImage);
                    }
                    else
                    {
                        Any aAny( ::utl::UCBContentHelper::GetProperty( aURL, u"TargetURL"_ustr ) );
                        OUString sId;
                        OUString aTargetURL;
                        if ( aAny >>= aTargetURL )
                            sId = weld::toId(new ContentEntry_Impl(aTargetURL, false));
                        m_xContentBox->insert(&rIter, -1, &aTitle, &sId, nullptr, nullptr, false, m_xScratchIter.get());
                        m_xContentBox->set_image(*m_xScratchIter, aDocumentImage);
                    }
                }
            }
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "sfx.appl", "ContentListBox_Impl::RequestingChildren(): unexpected exception" );
        }
    }

    if (!pContentEntry || pContentEntry->bIsFolder)
        m_xContentBox->set_image(rIter, aOpenBookImage);

    return true;
}

IMPL_LINK(ContentTabPage_Impl, CollapsingHdl, const weld::TreeIter&, rIter, bool)
{
    ContentEntry_Impl* pContentEntry = weld::fromId<ContentEntry_Impl*>(m_xContentBox->get_id(rIter));
    if (!pContentEntry || pContentEntry->bIsFolder)
        m_xContentBox->set_image(rIter, aClosedBookImage);

    return true;
}

OUString ContentTabPage_Impl::GetSelectedEntry() const
{
    OUString aRet;
    ContentEntry_Impl* pEntry = weld::fromId<ContentEntry_Impl*>(m_xContentBox->get_selected_id());
    if (pEntry && !pEntry->bIsFolder)
        aRet = pEntry->aURL;
    return aRet;
}

// class HelpTabPage_Impl ------------------------------------------------
HelpTabPage_Impl::HelpTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin,
    const OUString& rID, const OUString& rUIXMLDescription)
    : BuilderPage(pParent, nullptr, rUIXMLDescription, rID)
    , m_pIdxWin(pIdxWin)
{
}

HelpTabPage_Impl::~HelpTabPage_Impl()
{
}

// class ContentTabPage_Impl ---------------------------------------------
ContentTabPage_Impl::ContentTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin)
    : HelpTabPage_Impl(pParent, pIdxWin, u"HelpContentPage"_ustr,
        u"sfx/ui/helpcontentpage.ui"_ustr)
    , m_xContentBox(m_xBuilder->weld_tree_view(u"content"_ustr))
    , m_xScratchIter(m_xContentBox->make_iterator())
    , aOpenBookImage(BMP_HELP_CONTENT_BOOK_OPEN)
    , aClosedBookImage(BMP_HELP_CONTENT_BOOK_CLOSED)
    , aDocumentImage(BMP_HELP_CONTENT_DOC)
{
    m_xContentBox->set_size_request(m_xContentBox->get_approximate_digit_width() * 30,
                                    m_xContentBox->get_height_rows(20));
    m_xContentBox->connect_row_activated(LINK(this, ContentTabPage_Impl, DoubleClickHdl));
    m_xContentBox->connect_expanding(LINK(this, ContentTabPage_Impl, ExpandingHdl));
    m_xContentBox->connect_collapsing(LINK(this, ContentTabPage_Impl, CollapsingHdl));

    InitRoot();
}

IMPL_LINK_NOARG(ContentTabPage_Impl, DoubleClickHdl, weld::TreeView&, bool)
{
    aDoubleClickHdl.Call(nullptr);
    return false;
}

void ContentTabPage_Impl::SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink)
{
    aDoubleClickHdl = rLink;
}

ContentTabPage_Impl::~ContentTabPage_Impl()
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xContentBox->make_iterator();
    bool bEntry = m_xContentBox->get_iter_first(*xEntry);
    while (bEntry)
    {
        ClearChildren(xEntry.get());
        delete weld::fromId<ContentEntry_Impl*>(m_xContentBox->get_id(*xEntry));
        bEntry = m_xContentBox->iter_next_sibling(*xEntry);
    }
}

void IndexTabPage_Impl::SelectExecutableEntry()
{
    sal_Int32 nPos = m_xIndexList->find_text(m_xIndexEntry->get_text());
    if (nPos == -1)
        return;

    sal_Int32 nOldPos = nPos;
    OUString aEntryText;
    IndexEntry_Impl* pEntry = weld::fromId<IndexEntry_Impl*>(m_xIndexList->get_id(nPos));
    sal_Int32 nCount = m_xIndexList->n_children();
    while ( nPos < nCount && ( !pEntry || pEntry->m_aURL.isEmpty() ) )
    {
        pEntry = weld::fromId<IndexEntry_Impl*>(m_xIndexList->get_id(++nPos));
        aEntryText = m_xIndexList->get_text(nPos);
    }

    if ( nOldPos != nPos )
        m_xIndexEntry->set_text(aEntryText);
}

// class IndexTabPage_Impl -----------------------------------------------
IndexTabPage_Impl::IndexTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin)
    : HelpTabPage_Impl(pParent, pIdxWin, u"HelpIndexPage"_ustr, u"sfx/ui/helpindexpage.ui"_ustr)
    , m_xIndexEntry(m_xBuilder->weld_entry(u"termentry"_ustr))
    , m_xIndexList(m_xBuilder->weld_tree_view(u"termlist"_ustr))
    , m_xOpenBtn(m_xBuilder->weld_button(u"display"_ustr))
    , aFactoryIdle("sfx2 appl IndexTabPage_Impl Factory")
    , aAutoCompleteIdle("sfx2 appl IndexTabPage_Impl AutoComplete")
    , aKeywordTimer("sfx2::IndexTabPage_Impl aKeywordTimer")
    , bIsActivated(false)
    , nRowHeight(m_xIndexList->get_height_rows(1))
    , nAllHeight(0)
    , nLastCharCode(0)
{
    m_xIndexList->set_size_request(m_xIndexList->get_approximate_digit_width() * 30, -1);

    m_xOpenBtn->connect_clicked(LINK(this, IndexTabPage_Impl, OpenHdl));
    aFactoryIdle.SetInvokeHandler( LINK(this, IndexTabPage_Impl, IdleHdl ));
    aAutoCompleteIdle.SetInvokeHandler( LINK(this, IndexTabPage_Impl, AutoCompleteHdl ));
    aKeywordTimer.SetInvokeHandler( LINK( this, IndexTabPage_Impl, TimeoutHdl ) );
    m_xIndexList->connect_row_activated(LINK(this, IndexTabPage_Impl, DoubleClickHdl));
    m_xIndexList->connect_changed(LINK(this, IndexTabPage_Impl, TreeChangeHdl));
    m_xIndexList->connect_custom_get_size(LINK(this, IndexTabPage_Impl, CustomGetSizeHdl));
    m_xIndexList->connect_custom_render(LINK(this, IndexTabPage_Impl, CustomRenderHdl));
    m_xIndexList->set_column_custom_renderer(0, true);
    m_xIndexList->connect_size_allocate(LINK(this, IndexTabPage_Impl, ResizeHdl));
    m_xIndexEntry->connect_key_press(LINK(this, IndexTabPage_Impl, KeyInputHdl));
    m_xIndexEntry->connect_changed(LINK(this, IndexTabPage_Impl, EntryChangeHdl));
    m_xIndexEntry->connect_activate(LINK(this, IndexTabPage_Impl, ActivateHdl));
}

IMPL_LINK(IndexTabPage_Impl, ResizeHdl, const Size&, rSize, void)
{
    nAllHeight = rSize.Height();
}

IMPL_LINK_NOARG(IndexTabPage_Impl, CustomGetSizeHdl, weld::TreeView::get_size_args, Size)
{
    return Size(m_xIndexList->get_size_request().Width(), nRowHeight);
}

IMPL_LINK(IndexTabPage_Impl, CustomRenderHdl, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    rRenderContext.Push(vcl::PushFlags::TEXTCOLOR);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());

    Point aPos(rRect.TopLeft());
    aPos.AdjustY((rRect.GetHeight() - rRenderContext.GetTextHeight()) / 2);

    int nIndex = m_xIndexList->find_id(rId);
    OUString aEntry(m_xIndexList->get_text(nIndex));

    IndexEntry_Impl* pEntry = weld::fromId<IndexEntry_Impl*>(rId);
    if (pEntry && pEntry->m_bSubEntry)
    {
        // indent sub entries
        aPos.AdjustX(8);
        sal_Int32 nPos = aEntry.indexOf(';');
        rRenderContext.DrawText(aPos, (nPos !=-1) ? aEntry.copy(nPos + 1) : aEntry);
    }
    else
        rRenderContext.DrawText(aPos, aEntry);

    rRenderContext.Pop();
}

IMPL_LINK_NOARG(IndexTabPage_Impl, TreeChangeHdl, weld::TreeView&, void)
{
    m_xIndexEntry->set_text(m_xIndexList->get_selected_text());
}

IMPL_LINK_NOARG(IndexTabPage_Impl, EntryChangeHdl, weld::Entry&, void)
{
    switch (nLastCharCode)
    {
        case css::awt::Key::DELETE_WORD_BACKWARD:
        case css::awt::Key::DELETE_WORD_FORWARD:
        case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
        case css::awt::Key::DELETE_TO_END_OF_LINE:
        case KEY_BACKSPACE:
        case KEY_DELETE:
            aAutoCompleteIdle.Stop();
            break;
        default:
            aAutoCompleteIdle.Start();
            break;
    }
}

IMPL_LINK(IndexTabPage_Impl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKCode = rKEvt.GetKeyCode();
    if (rKCode.GetModifier()) // only with no modifiers held
        return false;

    sal_uInt16 nCode = rKCode.GetCode();

    if (nCode == KEY_UP || nCode == KEY_PAGEUP ||
        nCode == KEY_DOWN || nCode == KEY_PAGEDOWN)
    {
//        disable_notify_events();
        sal_Int32 nIndex = m_xIndexList->get_selected_index();
        sal_Int32 nOrigIndex = nIndex;
        sal_Int32 nCount = m_xIndexList->n_children();
        if (nIndex == -1)
        {
            m_xIndexList->set_cursor(0);
            m_xIndexList->select(0);
            m_xIndexEntry->set_text(m_xIndexList->get_selected_text());
        }
        else
        {
            if (nCode == KEY_UP)
                --nIndex;
            else if (nCode == KEY_DOWN)
                ++nIndex;
            else if (nCode == KEY_PAGEUP)
            {
                int nVisRows = nAllHeight / nRowHeight;
                nIndex -= nVisRows;
            }
            else if (nCode == KEY_PAGEDOWN)
            {
                int nVisRows = nAllHeight / nRowHeight;
                nIndex += nVisRows;
            }

            if (nIndex < 0)
                nIndex = 0;
            if (nIndex >= nCount)
                nIndex = nCount - 1;

            if (nIndex != nOrigIndex)
            {
                m_xIndexList->set_cursor(nIndex);
                m_xIndexList->select(nIndex);
                m_xIndexEntry->set_text(m_xIndexList->get_selected_text());
            }

//            m_xIndexList->grab_focus();
//            g_signal_emit_by_name(pWidget, "key-press-event", pEvent, &ret);
//            m_xIndexEntry->set_text(m_xIndexList->get_selected_text());
//            m_xIndexEntry->grab_focus();
        }
        m_xIndexEntry->select_region(0, -1);
//        enable_notify_events();
//        m_bTreeChange = true;
//        m_pEntry->fire_signal_changed();
//        m_bTreeChange = false;
        return true;
    }

    nLastCharCode = nCode;
    return false;
}

IndexTabPage_Impl::~IndexTabPage_Impl()
{
    ClearIndex();
}

namespace sfx2 {

    typedef std::unordered_map< OUString, int > KeywordInfo;
}

void IndexTabPage_Impl::InitializeIndex()
{
    weld::WaitObject aWaitCursor(m_pIdxWin->GetFrameWeld());

    // By now more than 256 equal entries are not allowed
    sal_Unicode append[256];
    for(sal_Unicode & k : append)
        k =  ' ';

    sfx2::KeywordInfo aInfo;
    m_xIndexList->freeze();

    try
    {
        OUStringBuffer aURL(HELP_URL + sFactory);
        AppendConfigToken(aURL, true);

        Content aCnt( aURL.makeStringAndClear(), Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        css::uno::Reference< css::beans::XPropertySetInfo > xInfo = aCnt.getProperties();
        if ( xInfo->hasPropertyByName( PROPERTY_ANCHORREF ) )
        {
            css::uno::Sequence< OUString > aPropSeq{ PROPERTY_KEYWORDLIST, PROPERTY_KEYWORDREF,
                                                     PROPERTY_ANCHORREF, PROPERTY_TITLEREF };

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
                int ndx,tmp;
                OUString aIndex, aTempString;
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

                    OUString sId;

                    if ( insert )
                    {
                        aTempString = aKeywordPair.copy( 0, ndx );
                        if ( aIndex != aTempString )
                        {
                            aIndex = aTempString;
                            it = aInfo.emplace(aTempString, 0).first;
                            sId = weld::toId(new IndexEntry_Impl(OUString(), false));
                            if ( (tmp = it->second++) != 0)
                                m_xIndexList->append(
                                    sId, aTempString + std::u16string_view(append, tmp));
                            else
                                m_xIndexList->append(sId, aTempString);
                        }
                    }
                    else
                        aIndex.clear();

                    sal_uInt32 nRefListLen = aRefList.getLength();

                    DBG_ASSERT( aAnchorList.hasElements(), "*IndexTabPage_Impl::InitializeIndex(): AnchorList is empty!" );
                    DBG_ASSERT( nRefListLen, "*IndexTabPage_Impl::InitializeIndex(): RefList is empty!" );

                    if ( aAnchorList.hasElements() && nRefListLen )
                    {
                        if ( aAnchorList[0].getLength() > 0 )
                        {
                            sId = weld::toId(new IndexEntry_Impl(aRefList[0] + "#" + aAnchorList[0], insert));
                        }
                        else
                            sId = weld::toId(new IndexEntry_Impl(aRefList[0], insert));
                    }

                    // Assume the token is trimmed
                    it = aInfo.emplace(aKeywordPair, 0).first;
                    if ((tmp = it->second++) != 0)
                        m_xIndexList->append(sId, aKeywordPair + std::u16string_view(append, tmp));
                    else
                        m_xIndexList->append(sId, aKeywordPair);

                    for ( sal_uInt32 j = 1; j < nRefListLen ; ++j )
                    {
                        aTempString = aKeywordPair + " - " + aTitleList[j];

                        if ( aAnchorList[j].getLength() > 0 )
                            sId = weld::toId(new IndexEntry_Impl(aRefList[j] + "#" + aAnchorList[j], insert));
                        else
                            sId = weld::toId(new IndexEntry_Impl(aRefList[j], insert));

                        it = aInfo.emplace(aTempString, 0).first;
                        if ( (tmp = it->second++) != 0 )
                            m_xIndexList->append(
                                sId, aTempString + std::u16string_view(append, tmp));
                        else
                            m_xIndexList->append(sId, aTempString);
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.appl", "IndexTabPage_Impl::InitializeIndex(): unexpected exception" );
    }

    m_xIndexList->thaw();

    if ( !sKeyword.isEmpty() )
        aKeywordLink.Call( *this );
}

void IndexTabPage_Impl::ClearIndex()
{
    const sal_Int32 nCount = m_xIndexList->n_children();
    for ( sal_Int32 i = 0; i < nCount; ++i )
        delete weld::fromId<IndexEntry_Impl*>(m_xIndexList->get_id(i));
    m_xIndexList->clear();
}

IMPL_LINK_NOARG(IndexTabPage_Impl, OpenHdl, weld::Button&, void)
{
    aDoubleClickHdl.Call(nullptr);
}

IMPL_LINK_NOARG(IndexTabPage_Impl, ActivateHdl, weld::Entry&, bool)
{
    aDoubleClickHdl.Call(nullptr);
    return true;
}

IMPL_LINK_NOARG(IndexTabPage_Impl, DoubleClickHdl, weld::TreeView&, bool)
{
    aDoubleClickHdl.Call(nullptr);
    return true;
}

IMPL_LINK_NOARG(IndexTabPage_Impl, IdleHdl, Timer*, void)
{
    InitializeIndex();
}

int IndexTabPage_Impl::starts_with(const OUString& rStr, int nStartRow, bool bCaseSensitive)
{
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

    int nRet = nStartRow;
    int nCount = m_xIndexList->n_children();
    while (nRet < nCount)
    {
        OUString aStr(m_xIndexList->get_text(nRet));
        const bool bMatch = !bCaseSensitive ? rI18nHelper.MatchString(rStr, aStr) : aStr.startsWith(rStr);
        if (bMatch)
            return nRet;
        ++nRet;
    }

    return -1;
}

IMPL_LINK_NOARG(IndexTabPage_Impl, AutoCompleteHdl, Timer*, void)
{
    OUString aStartText = m_xIndexEntry->get_text();
    int nStartPos, nEndPos;
    m_xIndexEntry->get_selection_bounds(nStartPos, nEndPos);
    int nMaxSelection = std::max(nStartPos, nEndPos);
    if (nMaxSelection != aStartText.getLength())
        return;

    int nActive = m_xIndexList->get_selected_index();
    int nStart = nActive;

    if (nStart == -1)
        nStart = 0;

    // Try match case insensitive from current position
    int nPos = starts_with(aStartText, nStart, false);
    if (nPos == -1 && nStart != 0)
    {
        // Try match case insensitive, but from start
        nPos = starts_with(aStartText, 0, false);
    }

    if (nPos == -1)
    {
        // Try match case sensitive from current position
        nPos = starts_with(aStartText, nStart, true);
        if (nPos == -1 && nStart != 0)
        {
            // Try match case sensitive, but from start
            nPos = starts_with(aStartText, 0, true);
        }
    }

    if (nPos != -1)
    {
        m_xIndexList->set_cursor(nPos);
        m_xIndexList->select(nPos);
        OUString aText = m_xIndexList->get_text(nPos);
        if (aText != aStartText)
            m_xIndexEntry->set_text(aText);
        m_xIndexEntry->select_region(aText.getLength(), aStartText.getLength());
    }
}

IMPL_LINK( IndexTabPage_Impl, TimeoutHdl, Timer*, pTimer, void)
{
    if(&aKeywordTimer == pTimer && !sKeyword.isEmpty())
        aKeywordLink.Call(*this);
}

void IndexTabPage_Impl::Activate()
{
    if ( !bIsActivated )
    {
        bIsActivated = true;
        aFactoryIdle.Start();
    }
}

void IndexTabPage_Impl::SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink)
{
    aDoubleClickHdl = rLink;
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
    IndexEntry_Impl* pEntry = weld::fromId<IndexEntry_Impl*>(m_xIndexList->get_id(m_xIndexList->find_text(m_xIndexEntry->get_text())));
    if (pEntry)
        aRet = pEntry->m_aURL;
    return aRet;
}

void IndexTabPage_Impl::SetKeyword( const OUString& rKeyword )
{
    sKeyword = rKeyword;

    if (m_xIndexList->n_children() > 0)
        aKeywordTimer.Start();
    else if ( !bIsActivated )
        aFactoryIdle.Start();
}


bool IndexTabPage_Impl::HasKeyword() const
{
    bool bRet = false;
    if ( !sKeyword.isEmpty() )
    {
        sal_Int32 nPos = m_xIndexList->find_text( sKeyword );
        bRet = nPos != -1;
    }

    return bRet;
}


bool IndexTabPage_Impl::HasKeywordIgnoreCase()
{
    bool bRet = false;
    if ( !sKeyword.isEmpty() )
    {
        sal_Int32 nEntries = m_xIndexList->n_children();
        const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetLocaleI18nHelper();
        for ( sal_Int32 n = 0; n < nEntries; n++)
        {
            const OUString sIndexItem {m_xIndexList->get_text(n)};
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
        m_xIndexEntry->set_text(sKeyword);
        aDoubleClickHdl.Call(nullptr);
        sKeyword.clear();
    }
}

IMPL_LINK_NOARG(SearchTabPage_Impl, ActivateHdl, weld::ComboBox&, bool)
{
    Search();
    return true;
}

// class SearchTabPage_Impl ----------------------------------------------

SearchTabPage_Impl::SearchTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin)
    : HelpTabPage_Impl(pParent, pIdxWin, u"HelpSearchPage"_ustr,
        u"sfx/ui/helpsearchpage.ui"_ustr)
    , m_xSearchED(m_xBuilder->weld_combo_box(u"search"_ustr))
    , m_xSearchBtn(m_xBuilder->weld_button(u"find"_ustr))
    , m_xFullWordsCB(m_xBuilder->weld_check_button(u"completewords"_ustr))
    , m_xScopeCB(m_xBuilder->weld_check_button(u"headings"_ustr))
    , m_xResultsLB(m_xBuilder->weld_tree_view(u"results"_ustr))
    , m_xOpenBtn(m_xBuilder->weld_button(u"display"_ustr))
    , xBreakIterator(vcl::unohelper::CreateBreakIterator())
{
    m_xResultsLB->set_size_request(m_xResultsLB->get_approximate_digit_width() * 30,
                                   m_xResultsLB->get_height_rows(15));

    m_xSearchBtn->connect_clicked(LINK(this, SearchTabPage_Impl, ClickHdl));
    m_xSearchED->connect_changed(LINK(this, SearchTabPage_Impl, ModifyHdl));
    m_xSearchED->connect_entry_activate(LINK(this, SearchTabPage_Impl, ActivateHdl));
    m_xOpenBtn->connect_clicked(LINK(this, SearchTabPage_Impl, OpenHdl));
    m_xResultsLB->connect_row_activated(LINK(this, SearchTabPage_Impl, DoubleClickHdl));

    SvtViewOptions aViewOpt( EViewType::TabPage, CONFIGNAME_SEARCHPAGE );
    if ( aViewOpt.Exists() )
    {
        OUString aUserData;
        Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
        if ( aUserItem >>= aUserData )
        {
            sal_Int32 nIdx {0};
            bool bChecked = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx)) == 1;
            m_xFullWordsCB->set_active(bChecked);
            bChecked = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx)) == 1;
            m_xScopeCB->set_active(bChecked);

            while ( nIdx > 0 )
            {
                m_xSearchED->append_text( INetURLObject::decode(
                    o3tl::getToken(aUserData, 0, ';', nIdx),
                    INetURLObject::DecodeMechanism::WithCharset ) );
            }
        }
    }

    ModifyHdl(*m_xSearchED);
}

SearchTabPage_Impl::~SearchTabPage_Impl()
{
    SvtViewOptions aViewOpt( EViewType::TabPage, CONFIGNAME_SEARCHPAGE );
    OUStringBuffer aUserData =
            OUString::number(m_xFullWordsCB->get_active() ? 1 : 0) +
            ";" +
            OUString::number(m_xScopeCB->get_active() ? 1 : 0);
    sal_Int32 nCount = std::min(m_xSearchED->get_count(), 10);  // save only 10 entries

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        aUserData.append(";" +
            INetURLObject::encode(
                m_xSearchED->get_text(i),
                INetURLObject::PART_UNO_PARAM_VALUE,
                INetURLObject::EncodeMechanism::All ));
    }

    Any aUserItem( aUserData.makeStringAndClear() );
    aViewOpt.SetUserItem( USERITEM_NAME, aUserItem );

    m_xSearchED.reset();
    m_xSearchBtn.reset();
    m_xFullWordsCB.reset();
    m_xScopeCB.reset();
    m_xResultsLB.reset();
    m_xOpenBtn.reset();
}

void SearchTabPage_Impl::ClearSearchResults()
{
    m_xResultsLB->clear();
}

void SearchTabPage_Impl::RememberSearchText( const OUString& rSearchText )
{
    for (sal_Int32 i = 0, nEntryCount = m_xSearchED->get_count(); i < nEntryCount; ++i)
    {
        if (rSearchText == m_xSearchED->get_text(i))
        {
            m_xSearchED->remove(i);
            break;
        }
    }

    m_xSearchED->insert_text(0, rSearchText);
}

IMPL_LINK_NOARG(SearchTabPage_Impl, ClickHdl, weld::Button&, void)
{
    Search();
}

void SearchTabPage_Impl::Search()
{
    OUString aSearchText = comphelper::string::strip(m_xSearchED->get_active_text(), ' ');
    if ( aSearchText.isEmpty() )
        return;

    std::unique_ptr<weld::WaitObject> xWaitCursor(new weld::WaitObject(m_pIdxWin->GetFrameWeld()));
    ClearSearchResults();
    RememberSearchText( aSearchText );
    OUStringBuffer aSearchURL(HELP_URL + aFactory + HELP_SEARCH_TAG);
    if (!m_xFullWordsCB->get_active())
        aSearchText = sfx2::PrepareSearchString( aSearchText, xBreakIterator, true );
    aSearchURL.append(aSearchText);
    AppendConfigToken(aSearchURL, false);
    if (m_xScopeCB->get_active())
        aSearchURL.append("&Scope=Heading");
    std::vector< OUString > aFactories = SfxContentHelper::GetResultSet(aSearchURL.makeStringAndClear());
    for (const OUString & rRow : aFactories)
    {
        sal_Int32 nIdx = 0;
        OUString aTitle = rRow.getToken(0, '\t', nIdx);
        OUString sURL(rRow.getToken(1, '\t', nIdx));
        m_xResultsLB->append(sURL, aTitle);
    }
    xWaitCursor.reset();

    if ( aFactories.empty() )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xContainer.get(),
                                                                 VclMessageType::Info, VclButtonsType::Ok,
                                                                 SfxResId(STR_INFO_NOSEARCHRESULTS)));
        xBox->run();
    }
}

IMPL_LINK_NOARG(SearchTabPage_Impl, OpenHdl, weld::Button&, void)
{
    aDoubleClickHdl.Call(nullptr);
}

IMPL_LINK(SearchTabPage_Impl, ModifyHdl, weld::ComboBox&, rComboBox, void)
{
    OUString aSearchText = comphelper::string::strip(m_xSearchED->get_active_text(), ' ');
    m_xSearchBtn->set_sensitive(!aSearchText.isEmpty());

    if (rComboBox.changed_by_direct_pick())
        Search();
}

IMPL_LINK_NOARG(SearchTabPage_Impl, DoubleClickHdl, weld::TreeView&, bool)
{
    aDoubleClickHdl.Call(nullptr);
    return true;
}

void SearchTabPage_Impl::SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink)
{
    aDoubleClickHdl = rLink;
}

OUString SearchTabPage_Impl::GetSelectedEntry() const
{
    return m_xResultsLB->get_selected_id();
}

void SearchTabPage_Impl::ClearPage()
{
    ClearSearchResults();
    m_xSearchED->set_entry_text(OUString());
}

bool SearchTabPage_Impl::OpenKeyword( const OUString& rKeyword )
{
    bool bRet = false;
    m_xSearchED->set_entry_text(rKeyword);
    Search();
    if (m_xResultsLB->n_children() > 0)
    {
        // found keyword -> open it
        m_xResultsLB->select(0);
        OpenHdl(*m_xOpenBtn);
        bRet = true;
    }
    return bRet;
}

// class BookmarksTabPage_Impl -------------------------------------------

void BookmarksTabPage_Impl::DoAction(std::u16string_view rAction)
{
    if (rAction == u"display")
        aDoubleClickHdl.Call(nullptr);
    else if (rAction == u"rename")
    {
        sal_Int32 nPos = m_xBookmarksBox->get_selected_index();
        if (nPos != -1)
        {
            SfxAddHelpBookmarkDialog_Impl aDlg(m_xBookmarksBox.get(), true);
            aDlg.SetTitle(m_xBookmarksBox->get_text(nPos));
            if (aDlg.run() == RET_OK)
            {
                OUString sURL = m_xBookmarksBox->get_id(nPos);
                m_xBookmarksBox->remove(nPos);
                m_xBookmarksBox->append(sURL, aDlg.GetTitle(),
                    SvFileInformationManager::GetImageId(INetURLObject(rtl::Concat2View(IMAGE_URL+INetURLObject(sURL).GetHost()))));
                m_xBookmarksBox->select(m_xBookmarksBox->n_children() - 1);
            }
        }
    }
    else if (rAction == u"delete")
    {
        sal_Int32 nPos = m_xBookmarksBox->get_selected_index();
        if (nPos != -1)
        {
            m_xBookmarksBox->remove(nPos);
            const sal_Int32 nCount = m_xBookmarksBox->n_children();
            if (nCount)
            {
                if (nPos >= nCount)
                    nPos = nCount - 1;
                m_xBookmarksBox->select(nPos);
            }
        }
    }
}

IMPL_LINK(BookmarksTabPage_Impl, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xBookmarksBox.get(), u"sfx/ui/bookmarkmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xMenu = xBuilder->weld_menu(u"menu"_ustr);

    OUString sIdent = xMenu->popup_at_rect(m_xBookmarksBox.get(), ::tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
    if (!sIdent.isEmpty())
        DoAction(sIdent);
    return true;
}

IMPL_LINK(BookmarksTabPage_Impl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    if (KEY_DELETE == nCode && m_xBookmarksBox->n_children() > 0)
    {
        DoAction(u"delete");
        bHandled = true;
    }
    return bHandled;
}

// class BookmarksTabPage_Impl -------------------------------------------
BookmarksTabPage_Impl::BookmarksTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* _pIdxWin)
    : HelpTabPage_Impl(pParent, _pIdxWin, u"HelpBookmarkPage"_ustr,
        u"sfx/ui/helpbookmarkpage.ui"_ustr)
    , m_xBookmarksBox(m_xBuilder->weld_tree_view(u"bookmarks"_ustr))
    , m_xBookmarksPB(m_xBuilder->weld_button(u"display"_ustr))
{
    m_xBookmarksBox->set_size_request(m_xBookmarksBox->get_approximate_digit_width() * 30,
                                      m_xBookmarksBox->get_height_rows(20));

    m_xBookmarksPB->connect_clicked( LINK(this, BookmarksTabPage_Impl, OpenHdl));
    m_xBookmarksBox->connect_row_activated(LINK(this, BookmarksTabPage_Impl, DoubleClickHdl));
    m_xBookmarksBox->connect_popup_menu(LINK(this, BookmarksTabPage_Impl, CommandHdl));
    m_xBookmarksBox->connect_key_press(LINK(this, BookmarksTabPage_Impl, KeyInputHdl));

    // load bookmarks from configuration
    const std::vector< SvtHistoryOptions::HistoryItem > aBookmarkSeq = SvtHistoryOptions::GetList( EHistoryType::HelpBookmarks );
    for ( const auto& rBookmark : aBookmarkSeq )
    {
        AddBookmarks( rBookmark.sTitle, rBookmark.sURL );
    }
}

BookmarksTabPage_Impl::~BookmarksTabPage_Impl()
{
    // save bookmarks to configuration
    SvtHistoryOptions::Clear( EHistoryType::HelpBookmarks );
    const sal_Int32 nCount = m_xBookmarksBox->n_children();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        SvtHistoryOptions::AppendItem(EHistoryType::HelpBookmarks, m_xBookmarksBox->get_id(i), u""_ustr,
                                      m_xBookmarksBox->get_text(i), std::nullopt, std::nullopt);
    }

    m_xBookmarksBox.reset();
    m_xBookmarksPB.reset();
}

IMPL_LINK_NOARG(BookmarksTabPage_Impl, OpenHdl, weld::Button&, void)
{
    aDoubleClickHdl.Call(nullptr);
}

IMPL_LINK_NOARG(BookmarksTabPage_Impl, DoubleClickHdl, weld::TreeView&, bool)
{
    aDoubleClickHdl.Call(nullptr);
    return true;
}

void BookmarksTabPage_Impl::SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink)
{
    aDoubleClickHdl = rLink;
}

OUString BookmarksTabPage_Impl::GetSelectedEntry() const
{
    return m_xBookmarksBox->get_selected_id();
}

void BookmarksTabPage_Impl::AddBookmarks(const OUString& rTitle, const OUString& rURL)
{
    const OUString aImageURL {IMAGE_URL + INetURLObject(rURL).GetHost()};
    m_xBookmarksBox->append(rURL, rTitle, SvFileInformationManager::GetImageId(INetURLObject(aImageURL)));
}

OUString SfxHelpWindow_Impl::buildHelpURL(std::u16string_view sFactory        ,
                                          std::u16string_view sContent        ,
                                          std::u16string_view sAnchor)
{
    OUStringBuffer sHelpURL(256);
    sHelpURL.append(HELP_URL + sFactory + sContent);
    AppendConfigToken(sHelpURL, true/*bUseQuestionMark*/);
    if (!sAnchor.empty())
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
            Reference< XComponent > xContent = xLoader->loadComponentFromURL(sHelpURL, u"_self"_ustr, 0, Sequence< PropertyValue >());
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

IMPL_LINK(SfxHelpIndexWindow_Impl, ActivatePageHdl, const OUString&, rPage, void)
{
    GetPage(rPage)->Activate();
}

SfxHelpIndexWindow_Impl::SfxHelpIndexWindow_Impl(SfxHelpWindow_Impl* _pParent, weld::Container* pContainer)
    : m_xBuilder(Application::CreateBuilder(pContainer, u"sfx/ui/helpcontrol.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"HelpControl"_ustr))
    , m_xActiveLB(m_xBuilder->weld_combo_box(u"active"_ustr))
    , m_xTabCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , aIdle("sfx2 appl SfxHelpIndexWindow_Impl")
    , aIndexKeywordLink(LINK(this, SfxHelpIndexWindow_Impl, KeywordHdl))
    , pParentWin(_pParent)
    , bIsInitDone(false)
{
    // create the pages
    GetContentPage();
    GetIndexPage();
    GetSearchPage();
    GetBookmarksPage();

    OUString sPageId(u"index"_ustr);
    SvtViewOptions aViewOpt( EViewType::TabDialog, CONFIGNAME_INDEXWIN );
    if ( aViewOpt.Exists() )
    {
        OUString sSavedPageId = aViewOpt.GetPageID();
        if (m_xTabCtrl->get_page_index(sSavedPageId) != -1)
            sPageId = sSavedPageId;
    }
    m_xTabCtrl->set_current_page(sPageId);
    ActivatePageHdl(sPageId);
    m_xActiveLB->connect_changed(LINK(this, SfxHelpIndexWindow_Impl, SelectHdl));

    m_xTabCtrl->connect_enter_page(LINK(this, SfxHelpIndexWindow_Impl, ActivatePageHdl));

    aIdle.SetInvokeHandler( LINK( this, SfxHelpIndexWindow_Impl, InitHdl ) );
    aIdle.Start();

    m_xContainer->show();
}

SfxHelpIndexWindow_Impl::~SfxHelpIndexWindow_Impl()
{
    SvtViewOptions aViewOpt(EViewType::TabDialog, CONFIGNAME_INDEXWIN);
    aViewOpt.SetPageID(m_xTabCtrl->get_current_page_ident());

    xCPage.reset();
    xIPage.reset();
    xSPage.reset();
    xBPage.reset();
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
        std::u16string_view aURL = o3tl::getToken(rRow, 1, '\t', nIdx ); // token 2
        OUString aFactory(INetURLObject(aURL).GetHost());
        m_xActiveLB->append(aFactory, aTitle);
    }

    if (m_xActiveLB->get_active() == -1)
        SetActiveFactory();
}

void SfxHelpIndexWindow_Impl::SetActiveFactory()
{
    DBG_ASSERT( xIPage, "index page not initialized" );
    if (!bIsInitDone && !m_xActiveLB->get_count())
    {
        aIdle.Stop();
        InitHdl( nullptr );
    }

    for (sal_Int32 i = 0, nEntryCount = m_xActiveLB->get_count(); i < nEntryCount; ++i)
    {
        OUString aFactory = m_xActiveLB->get_id(i);
        aFactory = aFactory.toAsciiLowerCase();
        if (aFactory == xIPage->GetFactory())
        {
            if (m_xActiveLB->get_active() != i)
            {
                m_xActiveLB->set_active(i);
                aSelectFactoryLink.Call(nullptr);
            }
            break;
        }
    }
}

HelpTabPage_Impl* SfxHelpIndexWindow_Impl::GetPage(std::u16string_view  rName)
{
    HelpTabPage_Impl* pPage = nullptr;

    if (rName == u"contents")
        pPage = GetContentPage();
    else if (rName == u"index")
        pPage = GetIndexPage();
    else if (rName == u"find")
        pPage = GetSearchPage();
    else if (rName == u"bookmarks")
        pPage = GetBookmarksPage();

    assert(pPage && "SfxHelpIndexWindow_Impl::GetCurrentPage(): no current page");

    return pPage;
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, SelectHdl, weld::ComboBox&, void)
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
    OUString aFactory = m_xActiveLB->get_active_id();
    if (!aFactory.isEmpty())
    {
        SetFactory(aFactory.toAsciiLowerCase(), false);
        aSelectFactoryLink.Call(this);
    }
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, KeywordHdl, IndexTabPage_Impl&, void)
{
    // keyword found on index?
    bool bIndex = xIPage->HasKeyword();

    if( !bIndex)
        bIndex = xIPage->HasKeywordIgnoreCase();
    // then set index or search page as current.
    OUString sPageId = bIndex ? u"index"_ustr : u"find"_ustr;
    if (sPageId != m_xTabCtrl->get_current_page_ident())
        m_xTabCtrl->set_current_page(sPageId);

    // at last we open the keyword
    if ( bIndex )
        xIPage->OpenKeyword();
    else if ( !xSPage->OpenKeyword( sKeyword ) )
        pParentWin->ShowStartPage();
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, IndexTabPageDoubleClickHdl, LinkParamNone*, void)
{
    aPageDoubleClickLink.Call(nullptr);
}

void SfxHelpIndexWindow_Impl::SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink)
{
    aPageDoubleClickLink = rLink;
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, ContentTabPageDoubleClickHdl, LinkParamNone*, void)
{
    aPageDoubleClickLink.Call(nullptr);
}

IMPL_LINK_NOARG(SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl, LinkParamNone*, void)
{
    aPageDoubleClickLink.Call(nullptr);
}

void SfxHelpIndexWindow_Impl::SetFactory( const OUString& rFactory, bool bActive )
{
    if ( !rFactory.isEmpty() )
    {
        GetIndexPage()->SetFactory( rFactory );
        // the index page made a check if rFactory is valid,
        // so the index page always returns a valid factory
        GetSearchPage()->SetFactory( GetIndexPage()->GetFactory() );
        if ( bActive )
            SetActiveFactory();
    }
}

OUString SfxHelpIndexWindow_Impl::GetSelectedEntry() const
{
    OUString sRet;

    OUString sName(m_xTabCtrl->get_current_page_ident());

    if (sName == "contents")
    {
        sRet = xCPage->GetSelectedEntry();
    }
    else if (sName == "index")
    {
        sRet = xIPage->GetSelectedEntry();
    }
    else if (sName == "find")
    {
        sRet = xSPage->GetSelectedEntry();
    }
    else if (sName == "bookmarks")
    {
        sRet = xBPage->GetSelectedEntry();
    }

    return sRet;
}

void SfxHelpIndexWindow_Impl::AddBookmarks( const OUString& rTitle, const OUString& rURL )
{
    GetBookmarksPage()->AddBookmarks( rTitle, rURL );
}

bool SfxHelpIndexWindow_Impl::IsValidFactory( std::u16string_view _rFactory )
{
    bool bValid = false;
    for (sal_Int32 i = 0, nEntryCount = m_xActiveLB->get_count(); i < nEntryCount; ++i)
    {
        OUString aFactory = m_xActiveLB->get_id(i);
        if (aFactory == _rFactory)
        {
            bValid = true;
            break;
        }
    }
    return bValid;
}

void SfxHelpIndexWindow_Impl::ClearSearchPage()
{
    if ( xSPage )
        xSPage->ClearPage();
}

void SfxHelpIndexWindow_Impl::GrabFocusBack()
{
    OUString sName(m_xTabCtrl->get_current_page_ident());

    if (sName == "contents" && xCPage)
        xCPage->SetFocusOnBox();
    else if (sName == "index" && xIPage)
        xIPage->SetFocusOnBox();
    else if (sName == "find" && xSPage)
        xSPage->SetFocusOnBox();
    else if (sName == "bookmarks" && xBPage)
        xBPage->SetFocusOnBox();
}

bool SfxHelpIndexWindow_Impl::HasFocusOnEdit() const
{
    bool bRet = false;
    OUString sName(m_xTabCtrl->get_current_page_ident());
    if (sName == "index" && xIPage)
        bRet = xIPage->HasFocusOnEdit();
    else if (sName == "find" && xSPage)
        bRet = xSPage->HasFocusOnEdit();
    return bRet;
}

OUString SfxHelpIndexWindow_Impl::GetSearchText() const
{
    OUString sRet;
    OUString sName(m_xTabCtrl->get_current_page_ident());
    if (sName == "find" && xSPage)
        sRet = xSPage->GetSearchText();
    return sRet;
}

bool SfxHelpIndexWindow_Impl::IsFullWordSearch() const
{
    bool bRet = false;
    OUString sName(m_xTabCtrl->get_current_page_ident());
    if (sName == "find" && xSPage)
        bRet = xSPage->IsFullWordSearch();
    return bRet;
}

void SfxHelpIndexWindow_Impl::OpenKeyword( const OUString& rKeyword )
{
    sKeyword = rKeyword;
    DBG_ASSERT( xIPage, "invalid index page" );
    xIPage->SetKeyword( sKeyword );
}

void SfxHelpIndexWindow_Impl::SelectExecutableEntry()
{
    OUString sName(m_xTabCtrl->get_current_page_ident());
    if (sName == "index" && xIPage )
        xIPage->SelectExecutableEntry();
}

weld::Window* SfxHelpIndexWindow_Impl::GetFrameWeld() const
{
    return pParentWin->GetFrameWeld();
}

// class TextWin_Impl ----------------------------------------------------
TextWin_Impl::TextWin_Impl( vcl::Window* p ) : DockingWindow( p, 0 )
{
}

bool TextWin_Impl::EventNotify( NotifyEvent& rNEvt )
{
    if( ( rNEvt.GetType() == NotifyEventType::KEYINPUT ) && rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB )
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

SfxHelpTextWindow_Impl::SfxHelpTextWindow_Impl(SfxHelpWindow_Impl* pHelpWin, weld::Builder& rBuilder, vcl::Window* pParent) :

    Window( pParent, WB_CLIPCHILDREN | WB_TABSTOP | WB_DIALOGCONTROL ),

    xToolBox            ( rBuilder.weld_toolbar(u"toolbar"_ustr) ),
    xOnStartupCB        ( rBuilder.weld_check_button(u"checkbutton"_ustr) ),
    xMenu               ( rBuilder.weld_menu(u"menu"_ustr) ),
    aSelectIdle         ( "sfx2 appl SfxHelpTextWindow_Impl Select" ),
    aIndexOnImage       ( BMP_HELP_TOOLBOX_INDEX_ON ),
    aIndexOffImage      ( BMP_HELP_TOOLBOX_INDEX_OFF ),
    aIndexOnText        ( SfxResId( STR_HELP_BUTTON_INDEX_ON ) ),
    aIndexOffText       ( SfxResId( STR_HELP_BUTTON_INDEX_OFF ) ),
    aOnStartupText      ( SfxResId( RID_HELP_ONSTARTUP_TEXT ) ),
    xHelpWin            ( pHelpWin ),
    pTextWin            ( VclPtr<TextWin_Impl>::Create( this ) ),
    bIsDebug            ( false ),
    bIsIndexOn          ( false ),
    bIsInClose          ( false ),
    bIsFullWordSearch   ( false )
{
    xFrame = Frame::create( ::comphelper::getProcessComponentContext() );
    xFrame->initialize( VCLUnoHelper::GetInterface ( pTextWin ) );
    xFrame->setName( u"OFFICE_HELP"_ustr );
    lcl_disableLayoutOfFrame(xFrame);

    xToolBox->set_help_id(HID_HELP_TOOLBOX);

    xToolBox->set_item_tooltip_text(u"index"_ustr, aIndexOffText );
    xToolBox->set_item_help_id(u"index"_ustr, HID_HELP_TOOLBOXITEM_INDEX);
    xToolBox->set_item_help_id(u"backward"_ustr, HID_HELP_TOOLBOXITEM_BACKWARD);
    xToolBox->set_item_help_id(u"forward"_ustr, HID_HELP_TOOLBOXITEM_FORWARD);
    xToolBox->set_item_help_id(u"start"_ustr, HID_HELP_TOOLBOXITEM_START);
    xToolBox->set_item_help_id(u"print"_ustr, HID_HELP_TOOLBOXITEM_PRINT);
    xToolBox->set_item_help_id(u"bookmarks"_ustr, HID_HELP_TOOLBOXITEM_BOOKMARKS );
    xToolBox->set_item_help_id(u"searchdialog"_ustr, HID_HELP_TOOLBOXITEM_SEARCHDIALOG);

    InitToolBoxImages();
    InitOnStartupBox();
    xOnStartupCB->connect_toggled(LINK(this, SfxHelpTextWindow_Impl, CheckHdl));

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
    bIsInClose = true;
    SvtMiscOptions().RemoveListenerLink( LINK( this, SfxHelpTextWindow_Impl, NotifyHdl ) );
    m_xSrchDlg.reset();
    xToolBox.reset();
    xOnStartupCB.reset();
    xHelpWin.clear();
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
    xToolBox->set_item_icon_name(u"index"_ustr, bIsIndexOn ? aIndexOffImage : aIndexOnImage);
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
        xOnStartupCB->hide();
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
                TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::InitOnStartupBox()" );
            }
            sModuleName = sTemp;
        }

        if ( !sModuleName.isEmpty() )
        {
            // set module name in checkbox text
            xOnStartupCB->set_label(aOnStartupText.replaceFirst("%MODULENAME", sModuleName));
            // and show it
            xOnStartupCB->show();
            // set check state
            xOnStartupCB->set_active(bHelpAtStartup);
            xOnStartupCB->save_state();
        }
    }
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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::getCursor(): unexpected exception" );
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
                xSrchDesc->setPropertyValue( u"SearchRegularExpression"_ustr, Any( true ) );
                if ( bIsFullWordSearch )
                    xSrchDesc->setPropertyValue( u"SearchWords"_ustr, Any( true ) );

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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }
}


IMPL_LINK_NOARG( SfxHelpTextWindow_Impl, NotifyHdl, LinkParamNone*, void )
{
    InitToolBoxImages();
    Resize();
}

IMPL_LINK( SfxHelpTextWindow_Impl, FindHdl, sfx2::SearchDialog&, rDlg, void )
{
    FindHdl(&rDlg);
}
void SfxHelpTextWindow_Impl::FindHdl(sfx2::SearchDialog* pDlg)
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
                bool bWrapAround = ( nullptr == pDlg );
                if ( bWrapAround )
                    pDlg = m_xSrchDlg.get();
                assert(pDlg && "invalid search dialog");

                // create descriptor, set string and find all words
                Reference < XSearchDescriptor > xSrchDesc = xSearchable->createSearchDescriptor();
                xSrchDesc->setPropertyValue( u"SearchWords"_ustr, Any(pDlg->IsOnlyWholeWords()) );
                xSrchDesc->setPropertyValue( u"SearchCaseSensitive"_ustr, Any(pDlg->IsMarchCase()) );
                xSrchDesc->setPropertyValue( u"SearchBackwards"_ustr, Any(pDlg->IsSearchBackwards()) );
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
                    Reference < text::XTextViewCursor > xTVCrsr = xCrsrSupp->getViewCursor();
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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::SelectHdl(): unexpected exception" );
    }
}

IMPL_LINK_NOARG(SfxHelpTextWindow_Impl, CloseHdl, LinkParamNone*, void)
{
    m_xSrchDlg.reset();
}

IMPL_LINK_NOARG(SfxHelpTextWindow_Impl, CheckHdl, weld::Toggleable&, void)
{
    if ( !xConfiguration.is() )
        return;

    bool bChecked = xOnStartupCB->get_active();
    try
    {
        ConfigurationHelper::writeRelativeKey(
            xConfiguration, PATH_OFFICE_FACTORIES + sCurrentFactory, KEY_HELP_ON_OPEN, Any( bChecked ) );
        ConfigurationHelper::flush( xConfiguration );
    }
    catch( Exception const & )
    {
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::CheckHdl()" );
    }
}

void SfxHelpTextWindow_Impl::Resize()
{
    Size aSize = GetOutputSizePixel();
    pTextWin->SetPosSizePixel( Point(0, 0), aSize );
}

bool SfxHelpTextWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    NotifyEventType nType = rNEvt.GetType();
    if ( NotifyEventType::COMMAND == nType && rNEvt.GetCommandEvent() )
    {
        const CommandEvent* pCmdEvt = rNEvt.GetCommandEvent();
        vcl::Window* pCmdWin = rNEvt.GetWindow();

        if ( pCmdEvt->GetCommand() == CommandEventId::ContextMenu && pCmdWin != this )
        {
            Point aPos;
            if ( pCmdEvt->IsMouseEvent() )
                aPos = pCmdEvt->GetMousePosPixel();
            else
                aPos = Point( pTextWin->GetPosPixel().X() + 20, 20 );

            xMenu->clear();

            if (bIsIndexOn)
                xMenu->append(u"index"_ustr, aIndexOffText, BMP_HELP_TOOLBOX_INDEX_OFF);
            else
                xMenu->append(u"index"_ustr, aIndexOnText, BMP_HELP_TOOLBOX_INDEX_ON);

            xMenu->append_separator(u"separator1"_ustr);
            xMenu->append(u"backward"_ustr, SfxResId(STR_HELP_BUTTON_PREV), BMP_HELP_TOOLBOX_PREV);
            xMenu->set_sensitive(u"backward"_ustr, xHelpWin->HasHistoryPredecessor());
            xMenu->append(u"forward"_ustr, SfxResId(STR_HELP_BUTTON_NEXT), BMP_HELP_TOOLBOX_NEXT);
            xMenu->set_sensitive(u"forward"_ustr, xHelpWin->HasHistorySuccessor());
            xMenu->append(u"start"_ustr, SfxResId(STR_HELP_BUTTON_START), BMP_HELP_TOOLBOX_START);
            xMenu->append_separator(u"separator2"_ustr);
            xMenu->append(u"print"_ustr, SfxResId(STR_HELP_BUTTON_PRINT), BMP_HELP_TOOLBOX_PRINT);
            xMenu->append(u"bookmarks"_ustr, SfxResId(STR_HELP_BUTTON_ADDBOOKMARK), BMP_HELP_TOOLBOX_BOOKMARKS);
            xMenu->append(u"searchdialog"_ustr, SfxResId(STR_HELP_BUTTON_SEARCHDIALOG), BMP_HELP_TOOLBOX_SEARCHDIALOG);
            xMenu->append_separator(u"separator3"_ustr);
            xMenu->append_check(u"selectionmode"_ustr, SfxResId(STR_HELP_MENU_TEXT_SELECTION_MODE));
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
                xMenu->set_active(u"selectionmode"_ustr, bCheck);
            }
            xMenu->append_separator(u"separator4"_ustr);
            xMenu->append(u"copy"_ustr, SfxResId(STR_HELP_MENU_TEXT_COPY), BMP_HELP_TOOLBOX_COPY);
            xMenu->set_sensitive(u"copy"_ustr, HasSelection());

            if ( bIsDebug )
            {
                xMenu->append_separator(u"separator5"_ustr);
                xMenu->append(u"sourceview"_ustr, SfxResId(STR_HELP_BUTTON_SOURCEVIEW));
            }

            int x, y, width, height;
            weld::Window* pTopLevel = GetFrameWeld();
            xHelpWin->GetContainer()->get_extents_relative_to(*pTopLevel, x, y, width, height);
            aPos.AdjustX(x);
            aPos.AdjustY(y);

            xHelpWin->DoAction(xMenu->popup_at_rect(pTopLevel, tools::Rectangle(aPos, Size(1,1))));
            bDone = true;
        }
    }
    else if ( NotifyEventType::KEYINPUT == nType && rNEvt.GetKeyEvent() )
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
            xHelpWin->CloseWindow();
            bDone = true;
        }
        else if ( KEY_TAB == nKey && xOnStartupCB->has_focus() )
        {
            xToolBox->grab_focus();
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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::GetFocus()" );
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
        xToolBox->set_item_icon_name(u"index"_ustr, aIndexOffImage);
        xToolBox->set_item_tooltip_text(u"index"_ustr, aIndexOffText);
    }
    else
    {
        xToolBox->set_item_icon_name(u"index"_ustr, aIndexOnImage);
        xToolBox->set_item_tooltip_text(u"index"_ustr, aIndexOnText);
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
                    if ( xProps->getPropertyValue( u"PageStyleName"_ustr ) >>= sStyleName )
                    {
                        Reference < XStyleFamiliesSupplier > xStyles( xController->getModel(), UNO_QUERY );
                        Reference < XNameContainer > xContainer;
                        if ( xStyles->getStyleFamilies()->getByName( u"PageStyles"_ustr )
                             >>= xContainer )
                        {
                            Reference < XStyle > xStyle;
                            if ( xContainer->getByName( sStyleName ) >>= xStyle )
                            {
                                Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
                                xPropSet->setPropertyValue( u"HeaderIsOn"_ustr,  Any( false ) );

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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpTextWindow_Impl::SetPageStyleHeaderOff()" );
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
    m_xSrchDlg = std::make_shared<sfx2::SearchDialog>(pTextWin->GetFrameWeld(), "HelpSearchDialog");
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

void SfxHelpWindow_Impl::GetFocus()
{
    if (pTextWin)
        pTextWin->GrabFocus();
    else
        ResizableDockingWindow::GetFocus();
}

void SfxHelpWindow_Impl::MakeLayout()
{
    Split();

    m_xHelpPaneWindow->set_visible(bIndex);
}

IMPL_LINK(SfxHelpWindow_Impl, ResizeHdl, const Size&, rSize, void)
{
    int nNewWidth = rSize.Width();
    if (!nNewWidth)
        return;
    if (bSplit)
        nIndexSize = round(m_xContainer->get_position() * 100.0 / nNewWidth);
    nWidth = nNewWidth;
    Split();
    nIndexSize = round(m_xContainer->get_position() * 100.0 / nWidth);
}

void SfxHelpWindow_Impl::Split()
{
    if (!nWidth)
        return;
    m_xContainer->set_position(nWidth * nIndexSize / 100);
    bSplit = true;
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
        nIndexSize = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx ));
        o3tl::getToken(aUserData, 0, ';', nIdx); // ignore nTextSize
        sal_Int32 nOldWidth = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx ));
        sal_Int32 nOldHeight = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx ));
        aWinSize = Size(nOldWidth, nOldHeight);
        aWinPos.setX( o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx )) );
        aWinPos.setY( o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx )) );
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
    aWinPos = pScreenWin->GetWindowExtentsAbsolute().TopLeft();
    if (bSplit)
        nIndexSize = round(m_xContainer->get_position() * 100.0 / nWidth);
    const OUString aUserData = OUString::number( nIndexSize )
        + ";" + OUString::number( 100 - nIndexSize )
        + ";" + OUString::number( nW )
        + ";" + OUString::number( nH )
        + ";" + OUString::number( aWinPos.X() )
        + ";" + OUString::number( aWinPos.Y() );

    aViewOpt.SetUserItem( USERITEM_NAME, Any( aUserData ) );
}

void SfxHelpWindow_Impl::ShowStartPage()
{
    loadHelpContent(SfxHelpWindow_Impl::buildHelpURL(xIndexWin->GetFactory(), u"/start", u""));
}

IMPL_LINK(SfxHelpWindow_Impl, SelectHdl, const OUString&, rCurItem, void)
{
    bGrabFocusToToolBox = pTextWin->GetToolBox().has_focus();
    DoAction(rCurItem);
}

IMPL_LINK_NOARG(SfxHelpWindow_Impl, OpenHdl, LinkParamNone*, void)
{
    xIndexWin->SelectExecutableEntry();
    OUString aEntry = xIndexWin->GetSelectedEntry();

    if ( aEntry.isEmpty() )
        return;

    OUString sHelpURL;

    bool bComplete = aEntry.toAsciiLowerCase().match("vnd.sun.star.help");

    if (bComplete)
        sHelpURL = aEntry;
    else
    {
        std::u16string_view aId;
        OUString aAnchor('#');
        if ( comphelper::string::getTokenCount(aEntry, '#') == 2 )
        {
            sal_Int32 nIdx{ 0 };
            aId = o3tl::getToken(aEntry, 0, '#', nIdx );
            aAnchor += o3tl::getToken(aEntry, 0, '#', nIdx );
        }
        else
            aId = aEntry;

        sHelpURL = SfxHelpWindow_Impl::buildHelpURL(xIndexWin->GetFactory(), Concat2View(OUString::Concat("/") + aId), aAnchor);
    }

    loadHelpContent(sHelpURL);
}

IMPL_LINK( SfxHelpWindow_Impl, SelectFactoryHdl, SfxHelpIndexWindow_Impl* , pWin, void )
{
    if ( sTitle.isEmpty() )
        sTitle = GetParent()->GetText();

    Reference< XTitle > xTitle(xFrame, UNO_QUERY);
    if (xTitle.is ())
        xTitle->setTitle(sTitle + " - " + xIndexWin->GetActiveFactoryTitle());

    if ( pWin )
        ShowStartPage();
    xIndexWin->ClearSearchPage();
}


IMPL_LINK( SfxHelpWindow_Impl, ChangeHdl, HelpListener_Impl&, rListener, void )
{
    SetFactory( rListener.GetFactory() );
}


void SfxHelpWindow_Impl::openDone(std::u16string_view sURL    ,
                                        bool         bSuccess)
{
    INetURLObject aObj( sURL );
    if ( aObj.GetProtocol() == INetProtocol::VndSunStarHelp )
        SetFactory( aObj.GetHost() );
    if ( IsWait() )
        LeaveWait();
    if ( bGrabFocusToToolBox )
    {
        pTextWin->GetToolBox().grab_focus();
        bGrabFocusToToolBox = false;
    }
    else
        xIndexWin->GrabFocusBack();
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
            xViewProps->setPropertyValue( u"ShowContentTips"_ustr, Any( false ) );
            xViewProps->setPropertyValue( u"ShowGraphics"_ustr, Any( true ) );
            xViewProps->setPropertyValue( u"ShowTables"_ustr, Any( true ) );
            xViewProps->setPropertyValue( u"HelpURL"_ustr, Any( u"HID:SFX2_HID_HELP_ONHELP"_ustr ) );
            OUString sProperty( u"IsExecuteHyperlinks"_ustr );
            if ( xInfo->hasPropertyByName( sProperty ) )
                xViewProps->setPropertyValue( sProperty, Any( true ) );
            xController->restoreViewData(Any());
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpWindow_Impl::OpenDoneHdl(): unexpected exception" );
    }

    // When the SearchPage opens the help doc, then select all words, which are equal to its text
    OUString sSearchText = comphelper::string::strip(xIndexWin->GetSearchText(), ' ');
    if ( !sSearchText.isEmpty() )
        pTextWin->SelectSearchText( sSearchText, xIndexWin->IsFullWordSearch() );

    // no page style header -> this prevents a print output of the URL
    pTextWin->SetPageStyleHeaderOff();
}


SfxHelpWindow_Impl::SfxHelpWindow_Impl(
    const css::uno::Reference < css::frame::XFrame2 >& rFrame,
    vcl::Window* pParent ) :

    ResizableDockingWindow(pParent),

    xFrame              ( rFrame ),
    pTextWin            ( nullptr ),
    pHelpInterceptor    ( new HelpInterceptor_Impl() ),
    pHelpListener       ( new HelpListener_Impl( pHelpInterceptor ) ),
    bIndex              ( true ),
    bGrabFocusToToolBox ( false ),
    bSplit              ( false ),
    nWidth              ( 0 ),
    nIndexSize          ( 40 ), // % of width
    aWinPos             ( 0, 0 ),
    aWinSize            ( 0, 0 ),
    sTitle              ( pParent->GetText() )
{
    SetStyle(GetStyle() & ~WB_DOCKABLE);

    SetHelpId( HID_HELP_WINDOW );

    m_xBuilder = Application::CreateInterimBuilder(m_xBox.get(), u"sfx/ui/helpwindow.ui"_ustr, false);
    m_xContainer = m_xBuilder->weld_paned(u"HelpWindow"_ustr);
    m_xContainer->connect_size_allocate(LINK(this, SfxHelpWindow_Impl, ResizeHdl));
    m_xHelpPaneWindow = m_xBuilder->weld_container(u"helppanewindow"_ustr);
    m_xHelpTextWindow = m_xBuilder->weld_container(u"helptextwindow"_ustr);
    m_xHelpTextXWindow = m_xHelpTextWindow->CreateChildFrame();

    pHelpInterceptor->InitWaiter( this );
    xIndexWin.reset(new SfxHelpIndexWindow_Impl(this, m_xHelpPaneWindow.get()));
    xIndexWin->SetDoubleClickHdl( LINK( this, SfxHelpWindow_Impl, OpenHdl ) );
    xIndexWin->SetSelectFactoryHdl( LINK( this, SfxHelpWindow_Impl, SelectFactoryHdl ) );

    pTextWin = VclPtr<SfxHelpTextWindow_Impl>::Create(this, *m_xBuilder, VCLUnoHelper::GetWindow(m_xHelpTextXWindow));
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
    xIndexWin.reset();
    pTextWin->CloseFrame();
    pTextWin.disposeAndClear();

    m_xHelpTextXWindow->dispose();
    m_xHelpTextXWindow.clear();
    m_xHelpTextWindow.reset();
    m_xHelpPaneWindow.reset();
    m_xContainer.reset();
    m_xBuilder.reset();

    ResizableDockingWindow::dispose();
}

bool SfxHelpWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if ( rNEvt.GetType() == NotifyEventType::KEYINPUT )
    {
        // Backward == <ALT><LEFT> or <BACKSPACE> Forward == <ALT><RIGHT>
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = rKeyCode.GetCode();
        if ( ( rKeyCode.IsMod2() && ( KEY_LEFT == nKey || KEY_RIGHT == nKey ) ) ||
             ( !rKeyCode.GetModifier() && KEY_BACKSPACE == nKey && !xIndexWin->HasFocusOnEdit() ) )
        {
            DoAction( rKeyCode.GetCode() == KEY_RIGHT ? u"forward" : u"backward" );
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
    if (xWindow.is())
    {
        VclPtr<vcl::Window> pScreenWin = VCLUnoHelper::GetWindow(xWindow);
        if (aWinSize.Width() && aWinSize.Height())
            pScreenWin->SetPosSizePixel(Point(aWinPos), aWinSize);
        else
            pScreenWin->SetPosPixel(Point(aWinPos));
    }
}

void SfxHelpWindow_Impl::SetFactory( const OUString& rFactory )
{
    xIndexWin->SetFactory( rFactory, true );
}

void SfxHelpWindow_Impl::SetHelpURL( std::u16string_view rURL )
{
    INetURLObject aObj( rURL );
    if ( aObj.GetProtocol() == INetProtocol::VndSunStarHelp )
        SetFactory( aObj.GetHost() );
}

void SfxHelpWindow_Impl::DoAction(std::u16string_view  rActionId)
{
    if (rActionId == u"index")
    {
        bIndex = !bIndex;
        MakeLayout();
        pTextWin->ToggleIndex( bIndex );
    }
    else if (rActionId == u"start")
    {
        ShowStartPage();
    }
    else if (rActionId == u"backward" || rActionId == u"forward")
    {
        URL aURL;
        aURL.Complete = ".uno:Backward";
        if (rActionId == u"forward")
            aURL.Complete = ".uno:Forward";
        Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict(aURL);
        pHelpInterceptor->dispatch( aURL, Sequence < PropertyValue >() );
    }
    else if (rActionId == u"searchdialog")
    {
        pTextWin->DoSearch();
    }
    else if (rActionId == u"print" || rActionId == u"sourceview" || rActionId == u"copy" || rActionId == u"selectionmode")
    {
        Reference < XDispatchProvider > xProv = pTextWin->getFrame();
        if ( xProv.is() )
        {
            URL aURL;
            if (rActionId == u"print")
                aURL.Complete = ".uno:Print";
            else if (rActionId == u"sourceview")
                aURL.Complete = ".uno:SourceView";
            else if (rActionId == u"copy")
                aURL.Complete = ".uno:Copy";
            else // rActionId == "selectionmode"
                aURL.Complete = ".uno:SelectTextMode";
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict(aURL);
            Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
            if ( xDisp.is() )
                xDisp->dispatch( aURL, Sequence < PropertyValue >() );
        }
    }
    else if (rActionId == u"bookmarks")
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
                            xIndexWin->AddBookmarks( aDlg.GetTitle(), aURL );
                        }
                    }
                }
            }
            catch( Exception& )
            {
                TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpWindow_Impl::DoAction(): unexpected exception" );
            }
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
        TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelpWindow_Impl::CloseWindow()" );
    }
}


void SfxHelpWindow_Impl::UpdateToolbox()
{
    pTextWin->GetToolBox().set_item_sensitive(u"backward"_ustr, pHelpInterceptor->HasHistoryPred());
    pTextWin->GetToolBox().set_item_sensitive(u"forward"_ustr, pHelpInterceptor->HasHistorySucc());
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

SfxAddHelpBookmarkDialog_Impl::SfxAddHelpBookmarkDialog_Impl(weld::Widget* pParent, bool bRename)
    : GenericDialogController(pParent, u"sfx/ui/bookmarkdialog.ui"_ustr, u"BookmarkDialog"_ustr)
    , m_xTitleED(m_xBuilder->weld_entry(u"entry"_ustr))
    , m_xAltTitle(m_xBuilder->weld_label(u"alttitle"_ustr))
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
