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
#ifndef INCLUDED_SFX2_SOURCE_APPL_NEWHELP_HXX
#define INCLUDED_SFX2_SOURCE_APPL_NEWHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XFrame2.hpp>

#include <rtl/ustrbuf.hxx>
#include <vcl/builderpage.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/idle.hxx>
#include <vcl/keycod.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <srchdlg.hxx>

// context menu ids
#define MID_OPEN                                1
#define MID_RENAME                              2
#define MID_DELETE                              3

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::i18n { class XBreakIterator; }
namespace com::sun::star::text { class XTextRange; }

// class HelpTabPage_Impl ------------------------------------------------

class SfxHelpIndexWindow_Impl;

class HelpTabPage_Impl : public BuilderPage
{
protected:
    SfxHelpIndexWindow_Impl*    m_pIdxWin;

public:
    HelpTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* _pIdxWin,
        const OUString& rID, const OUString& rUIXMLDescription);
    virtual ~HelpTabPage_Impl() override;
};

// class ContentTabPage_Impl ---------------------------------------------

class ContentTabPage_Impl : public HelpTabPage_Impl
{
private:
    std::unique_ptr<weld::TreeView> m_xContentBox;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    OUString aOpenBookImage;
    OUString aClosedBookImage;
    OUString aDocumentImage;

    Link<LinkParamNone*, void> aDoubleClickHdl;

    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(ExpandingHdl, const weld::TreeIter&, bool);
    DECL_LINK(CollapsingHdl, const weld::TreeIter&, bool);

    void            ClearChildren(const weld::TreeIter* pParent);
    void            InitRoot();
public:
    ContentTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* _pIdxWin);
    virtual ~ContentTabPage_Impl() override;

    void     SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink);
    OUString GetSelectedEntry() const;
    void     SetFocusOnBox() { m_xContentBox->grab_focus(); }
};

class IndexTabPage_Impl : public HelpTabPage_Impl
{
private:
    std::unique_ptr<weld::Entry> m_xIndexEntry;
    std::unique_ptr<weld::TreeView> m_xIndexList;
    std::unique_ptr<weld::Button> m_xOpenBtn;

    Idle                aFactoryIdle;
    Idle                aAutoCompleteIdle;
    Timer               aKeywordTimer;
    Link<IndexTabPage_Impl&,void> aKeywordLink;

    OUString            sFactory;
    OUString            sKeyword;

    bool                bIsActivated;
    int                 nRowHeight;
    int                 nAllHeight;
    sal_uInt16          nLastCharCode;

    void                InitializeIndex();
    void                ClearIndex();

    Link<LinkParamNone*, void> aDoubleClickHdl;

    DECL_LINK(OpenHdl, weld::Button&, void);
    DECL_LINK(IdleHdl, Timer*, void);
    DECL_LINK(AutoCompleteHdl, Timer*, void);
    DECL_LINK(TimeoutHdl, Timer*, void);
    DECL_LINK(TreeChangeHdl, weld::TreeView&, void);
    DECL_LINK(EntryChangeHdl, weld::Entry&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(CustomGetSizeHdl, weld::TreeView::get_size_args, Size);
    DECL_LINK(CustomRenderHdl, weld::TreeView::render_args, void);
    DECL_LINK(ResizeHdl, const Size&, void);

    int starts_with(const OUString& rStr, int nStartRow, bool bCaseSensitive);

public:
    IndexTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin);
    virtual ~IndexTabPage_Impl() override;

    virtual void        Activate() override;

    void                SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink);
    void                SetFactory( const OUString& rFactory );
    const OUString&     GetFactory() const { return sFactory; }
    OUString            GetSelectedEntry() const;
    void                SetFocusOnBox() { m_xIndexEntry->grab_focus(); }
    bool                HasFocusOnEdit() const { return m_xIndexEntry->has_focus(); }

    void                SetKeywordHdl( const Link<IndexTabPage_Impl&,void>& rLink ) { aKeywordLink = rLink; }
    void                SetKeyword( const OUString& rKeyword );
    bool                HasKeyword() const;
    bool                HasKeywordIgnoreCase();
    void                OpenKeyword();

    void         SelectExecutableEntry();
};

class SearchTabPage_Impl : public HelpTabPage_Impl
{
private:
    std::unique_ptr<weld::ComboBox> m_xSearchED;
    std::unique_ptr<weld::Button> m_xSearchBtn;
    std::unique_ptr<weld::CheckButton> m_xFullWordsCB;
    std::unique_ptr<weld::CheckButton> m_xScopeCB;
    std::unique_ptr<weld::TreeView> m_xResultsLB;
    std::unique_ptr<weld::Button> m_xOpenBtn;

    Link<LinkParamNone*, void> aDoubleClickHdl;

    OUString                       aFactory;

    css::uno::Reference< css::i18n::XBreakIterator >
                                   xBreakIterator;

    void ClearSearchResults();
    void RememberSearchText( const OUString& rSearchText );
    void Search();

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(OpenHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, weld::ComboBox&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);

public:
    SearchTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin);
    virtual ~SearchTabPage_Impl() override;

    void         SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink);
    void         SetFactory( const OUString& rFactory ) { aFactory = rFactory; }
    OUString     GetSelectedEntry() const;
    void         ClearPage();
    void         SetFocusOnBox() { m_xResultsLB->grab_focus(); }
    bool         HasFocusOnEdit() const { return m_xSearchED->has_focus(); }
    OUString     GetSearchText() const { return m_xSearchED->get_active_text(); }
    bool         IsFullWordSearch() const { return m_xFullWordsCB->get_active(); }
    bool         OpenKeyword( const OUString& rKeyword );
};

class BookmarksTabPage_Impl : public HelpTabPage_Impl
{
private:
    std::unique_ptr<weld::TreeView> m_xBookmarksBox;
    std::unique_ptr<weld::Button> m_xBookmarksPB;

    Link<LinkParamNone*, void> aDoubleClickHdl;

    DECL_LINK(OpenHdl, weld::Button&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void DoAction(std::u16string_view rAction);

public:
    BookmarksTabPage_Impl(weld::Widget* pParent, SfxHelpIndexWindow_Impl* pIdxWin);
    virtual ~BookmarksTabPage_Impl() override;

    void                SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink);
    OUString            GetSelectedEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    void                SetFocusOnBox() { m_xBookmarksBox->grab_focus(); }
};

// class SfxHelpIndexWindow_Impl -----------------------------------------

class SfxHelpWindow_Impl;

class SfxHelpIndexWindow_Impl
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::ComboBox> m_xActiveLB;
    std::unique_ptr<weld::Notebook> m_xTabCtrl;

    Idle                aIdle;

    Link<SfxHelpIndexWindow_Impl*,void> aSelectFactoryLink;
    Link<LinkParamNone*,void>           aPageDoubleClickLink;
    Link<IndexTabPage_Impl&,void>       aIndexKeywordLink;
    OUString                            sKeyword;

    VclPtr<SfxHelpWindow_Impl>     pParentWin;

    std::unique_ptr<ContentTabPage_Impl> xCPage;
    std::unique_ptr<IndexTabPage_Impl> xIPage;
    std::unique_ptr<SearchTabPage_Impl> xSPage;
    std::unique_ptr<BookmarksTabPage_Impl> xBPage;

    bool                bIsInitDone;

    void                Initialize();
    void                SetActiveFactory();
    HelpTabPage_Impl*   GetPage(std::u16string_view );

    inline ContentTabPage_Impl*     GetContentPage();
    inline IndexTabPage_Impl*       GetIndexPage();
    inline SearchTabPage_Impl*      GetSearchPage();
    inline BookmarksTabPage_Impl*   GetBookmarksPage();

    DECL_LINK(ActivatePageHdl, const OUString&, void);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(InitHdl, Timer *, void);
    DECL_LINK(SelectFactoryHdl, Timer *, void);
    DECL_LINK(KeywordHdl, IndexTabPage_Impl&, void);
    DECL_LINK(ContentTabPageDoubleClickHdl, LinkParamNone*, void);
    DECL_LINK(TabPageDoubleClickHdl, LinkParamNone*, void);
    DECL_LINK(IndexTabPageDoubleClickHdl, LinkParamNone*, void);

public:
    explicit SfxHelpIndexWindow_Impl(SfxHelpWindow_Impl* pParent, weld::Container* pContainer);
    ~SfxHelpIndexWindow_Impl();

    void                SetDoubleClickHdl(const Link<LinkParamNone*, void>& rLink);
    void                SetSelectFactoryHdl( const Link<SfxHelpIndexWindow_Impl*,void>& rLink ) { aSelectFactoryLink = rLink; }
    void                SetFactory( const OUString& rFactory, bool bActive );
    OUString const &    GetFactory() const { return xIPage->GetFactory(); }
    OUString            GetSelectedEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    bool                IsValidFactory( std::u16string_view _rFactory );
    OUString            GetActiveFactoryTitle() const { return m_xActiveLB->get_active_text(); }
    void                ClearSearchPage();
    void                GrabFocusBack();
    bool                HasFocusOnEdit() const;
    OUString            GetSearchText() const;
    bool                IsFullWordSearch() const;
    void                OpenKeyword( const OUString& rKeyword );
    void                SelectExecutableEntry();

    weld::Window*       GetFrameWeld() const;
};

// inlines ---------------------------------------------------------------

ContentTabPage_Impl* SfxHelpIndexWindow_Impl::GetContentPage()
{
    if (!xCPage)
    {
        xCPage.reset(new ContentTabPage_Impl(m_xTabCtrl->get_page(u"contents"_ustr), this));
        xCPage->SetDoubleClickHdl(LINK(this, SfxHelpIndexWindow_Impl, ContentTabPageDoubleClickHdl));
    }
    return xCPage.get();
}

IndexTabPage_Impl* SfxHelpIndexWindow_Impl::GetIndexPage()
{
    if (!xIPage)
    {
        xIPage.reset(new IndexTabPage_Impl(m_xTabCtrl->get_page(u"index"_ustr), this));
        xIPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, IndexTabPageDoubleClickHdl) );
        xIPage->SetKeywordHdl( aIndexKeywordLink );
    }
    return xIPage.get();
}

SearchTabPage_Impl* SfxHelpIndexWindow_Impl::GetSearchPage()
{
    if (!xSPage)
    {
        xSPage.reset(new SearchTabPage_Impl(m_xTabCtrl->get_page(u"find"_ustr), this));
        xSPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl) );
    }
    return xSPage.get();
}

BookmarksTabPage_Impl* SfxHelpIndexWindow_Impl::GetBookmarksPage()
{
    if (!xBPage)
    {
        xBPage.reset(new BookmarksTabPage_Impl(m_xTabCtrl->get_page(u"bookmarks"_ustr), this));
        xBPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl) );
    }
    return xBPage.get();
}

// class TextWin_Impl ----------------------------------------------------
class TextWin_Impl : public DockingWindow
{
public:
    explicit                TextWin_Impl( vcl::Window* pParent );

    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
};

// class SfxHelpTextWindow_Impl ------------------------------------------

class SvtMiscOptions;
class SfxHelpWindow_Impl;

class SfxHelpTextWindow_Impl : public vcl::Window
{
private:
    std::unique_ptr<weld::Toolbar> xToolBox;
    std::unique_ptr<weld::CheckButton>  xOnStartupCB;
    std::unique_ptr<weld::Menu>  xMenu;
    Idle                    aSelectIdle;
    OUString                aIndexOnImage;
    OUString                aIndexOffImage;
    OUString                aIndexOnText;
    OUString                aIndexOffText;
    OUString                aSearchText;
    OUString                aOnStartupText;
    OUString                sCurrentFactory;

    VclPtr<SfxHelpWindow_Impl>     xHelpWin;
    VclPtr<vcl::Window>            pTextWin;
    std::shared_ptr<sfx2::SearchDialog> m_xSrchDlg;
    css::uno::Reference < css::frame::XFrame2 >
                            xFrame;
    css::uno::Reference< css::i18n::XBreakIterator >
                            xBreakIterator;
    css::uno::Reference< css::uno::XInterface >
                            xConfiguration;
    bool                    bIsDebug;
    bool                    bIsIndexOn;
    bool                    bIsInClose;
    bool                    bIsFullWordSearch;

    bool                    HasSelection() const;
    void                    InitToolBoxImages();
    void                    InitOnStartupBox();

    css::uno::Reference< css::i18n::XBreakIterator > const &
                            GetBreakIterator();
    css::uno::Reference< css::text::XTextRange >
                            getCursor() const;
    bool                    isHandledKey( const vcl::KeyCode& _rKeyCode );

    DECL_LINK(        SelectHdl, Timer *, void);
    DECL_LINK(        NotifyHdl, LinkParamNone*, void );
    DECL_LINK(        FindHdl, sfx2::SearchDialog&, void );
    DECL_LINK(        CloseHdl, LinkParamNone*, void );
    DECL_LINK(        CheckHdl, weld::Toggleable&, void );
    void                    FindHdl(sfx2::SearchDialog*);

public:
    explicit SfxHelpTextWindow_Impl(SfxHelpWindow_Impl* pHelpWin, weld::Builder& rBuilder, vcl::Window* pParent);
    virtual ~SfxHelpTextWindow_Impl() override;
    virtual void dispose() override;

    virtual void            Resize() override;
    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual void            GetFocus() override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    const css::uno::Reference < css::frame::XFrame2 >&
                            getFrame() const { return xFrame; }

    void                    SetSelectHdl(const Link<const OUString&, void>& rLink) { xToolBox->connect_clicked(rLink); }
    void                    ToggleIndex( bool bOn );
    void                    SelectSearchText( const OUString& rSearchText, bool _bIsFullWordSearch );
    void                    SetPageStyleHeaderOff() const;
    weld::Toolbar&          GetToolBox() { return *xToolBox; }
    void                    CloseFrame();
    void                    DoSearch();
};

// class SfxHelpWindow_Impl ----------------------------------------------

class HelpInterceptor_Impl;
class HelpListener_Impl;
class SfxHelpWindow_Impl : public ResizableDockingWindow
{
private:
friend class SfxHelpIndexWindow_Impl;

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Paned> m_xContainer;
    std::unique_ptr<weld::Container> m_xHelpPaneWindow;
    std::unique_ptr<weld::Container> m_xHelpTextWindow;
    css::uno::Reference<css::awt::XWindow> m_xHelpTextXWindow;

    css::uno::Reference < css::awt::XWindow >
                                xWindow;
    css::uno::Reference < css::frame::XFrame2 >
                                xFrame;

    std::unique_ptr<SfxHelpIndexWindow_Impl> xIndexWin;
    VclPtr<SfxHelpTextWindow_Impl>     pTextWin;
    HelpInterceptor_Impl*       pHelpInterceptor;
    rtl::Reference<HelpListener_Impl> pHelpListener;

    bool                bIndex;
    bool                bGrabFocusToToolBox;
    bool                bSplit;
    int                 nWidth;
    int                 nIndexSize;
    AbsoluteScreenPixelPoint aWinPos;
    Size                aWinSize;
    OUString            sTitle;

    virtual void        GetFocus() override;

    void                MakeLayout();
    void                LoadConfig();
    void                SaveConfig();
    void                ShowStartPage();
    void                Split();

    DECL_LINK(SelectHdl, const OUString&, void);
    DECL_LINK(OpenHdl, LinkParamNone*, void);
    DECL_LINK(SelectFactoryHdl, SfxHelpIndexWindow_Impl*, void);
    DECL_LINK(ChangeHdl, HelpListener_Impl&, void);
    DECL_LINK(ResizeHdl, const Size&, void);

public:
    SfxHelpWindow_Impl( const css::uno::Reference < css::frame::XFrame2 >& rFrame,
                        vcl::Window* pParent );
    virtual ~SfxHelpWindow_Impl() override;
    virtual void dispose() override;

    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;

    void                setContainerWindow( const css::uno::Reference < css::awt::XWindow >& xWin );
    css::uno::Reference < css::frame::XFrame2 > const &
                        getTextFrame() const { return pTextWin->getFrame(); }

    void                SetFactory( const OUString& rFactory );
    void                SetHelpURL( std::u16string_view rURL );
    void                DoAction(std::u16string_view rAction);
    void                CloseWindow();

    weld::Container*    GetContainer() { return m_xHelpTextWindow.get(); }

    void                UpdateToolbox();
    void                OpenKeyword( const OUString& rKeyword ) { xIndexWin->OpenKeyword( rKeyword ); }

    bool                HasHistoryPredecessor() const;      // forward to interceptor
    bool                HasHistorySuccessor() const;        // forward to interceptor

    void                openDone(std::u16string_view sURL    ,
                                       bool         bSuccess);

    static OUString     buildHelpURL(std::u16string_view sFactory        ,
                                     std::u16string_view sContent        ,
                                     std::u16string_view sAnchor);

    void                loadHelpContent(const OUString& sHelpURL                ,
                                              bool         bAddToHistory = true);
};

class SfxAddHelpBookmarkDialog_Impl : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xTitleED;
    std::unique_ptr<weld::Label> m_xAltTitle;
public:
    SfxAddHelpBookmarkDialog_Impl(weld::Widget* pParent, bool bRename);

    void SetTitle( const OUString& rTitle );
    OUString GetTitle() const { return m_xTitleED->get_text(); }
};

/// Appends ?Language=xy&System=abc to the help URL in rURL
void AppendConfigToken(OUStringBuffer& rURL, bool bQuestionMark);

#endif // INCLUDED_SFX2_SOURCE_APPL_NEWHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
