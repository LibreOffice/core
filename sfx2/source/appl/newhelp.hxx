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

#include <vcl/window.hxx>
#include <vcl/idle.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>
#include <vcl/treelistbox.hxx>
#include <unotools/moduleoptions.hxx>

#include <srchdlg.hxx>

// context menu ids
#define MID_OPEN                                1
#define MID_RENAME                              2
#define MID_DELETE                              3

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace i18n { class XBreakIterator; } } } }
namespace com { namespace sun { namespace star { namespace text { class XTextRange; } } } }


// ContentListBox_Impl ---------------------------------------------------

class ContentListBox_Impl : public SvTreeListBox
{
private:
    Image const     aOpenBookImage;
    Image const     aClosedBookImage;
    Image const     aDocumentImage;

    void            InitRoot();
    void            ClearChildren( SvTreeListEntry* pParent );

public:
    ContentListBox_Impl(vcl::Window* pParent, WinBits nStyle);
    virtual ~ContentListBox_Impl() override;
    virtual void dispose() override;

    virtual void    RequestingChildren( SvTreeListEntry* pParent ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void     SetOpenHdl( const Link<SvTreeListBox*,bool>& rLink ) { SetDoubleClickHdl( rLink ); }
    OUString        GetSelectedEntry() const;
};

// class HelpTabPage_Impl ------------------------------------------------

class SfxHelpIndexWindow_Impl;

class HelpTabPage_Impl : public TabPage
{
protected:
    VclPtr<SfxHelpIndexWindow_Impl>    m_pIdxWin;

public:
    HelpTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin,
        const OString& rID, const OUString& rUIXMLDescription);
    virtual ~HelpTabPage_Impl() override;
    virtual void dispose() override;

    virtual Control*    GetLastFocusControl() = 0;
};

// class ContentTabPage_Impl ---------------------------------------------

class ContentTabPage_Impl : public HelpTabPage_Impl
{
private:
    VclPtr<ContentListBox_Impl> m_pContentBox;

public:
    ContentTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin);
    virtual ~ContentTabPage_Impl() override;
    virtual void dispose() override;

    virtual void        ActivatePage() override;
    virtual Control*    GetLastFocusControl() override;

    void     SetOpenHdl( const Link<SvTreeListBox*,bool>& rLink ) { m_pContentBox->SetOpenHdl( rLink ); }
    OUString GetSelectedEntry() const { return m_pContentBox->GetSelectedEntry(); }
    void     SetFocusOnBox() { m_pContentBox->GrabFocus(); }
};

// class IndexTabPage_Impl -----------------------------------------------

class IndexBox_Impl : public ComboBox
{
public:
    IndexBox_Impl(vcl::Window* pParent, WinBits nStyle);

    virtual void        UserDraw( const UserDrawEvent& rUDEvt ) override;
    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;

    void                SelectExecutableEntry();
};

class IndexTabPage_Impl : public HelpTabPage_Impl
{
private:
    VclPtr<IndexBox_Impl>      m_pIndexCB;
    VclPtr<PushButton>         m_pOpenBtn;

    Idle                aFactoryIdle;
    Timer               aKeywordTimer;
    Link<IndexTabPage_Impl&,void> aKeywordLink;

    OUString            sFactory;
    OUString            sKeyword;

    bool                bIsActivated;

    void                InitializeIndex();
    void                ClearIndex();

    DECL_LINK(OpenHdl, Button*, void);
    DECL_LINK(IdleHdl, Timer*, void);
    DECL_LINK(TimeoutHdl, Timer*, void);

public:
    IndexTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    virtual ~IndexTabPage_Impl() override;
    virtual void dispose() override;

    virtual void        ActivatePage() override;
    virtual Control*    GetLastFocusControl() override;

    void                SetDoubleClickHdl( const Link<ComboBox&,void>& rLink );
    void                SetFactory( const OUString& rFactory );
    const OUString&     GetFactory() const { return sFactory; }
    OUString            GetSelectedEntry() const;
    void         SetFocusOnBox() { m_pIndexCB->GrabFocus(); }
    bool         HasFocusOnEdit() const { return m_pIndexCB->HasChildPathFocus(); }

    void         SetKeywordHdl( const Link<IndexTabPage_Impl&,void>& rLink ) { aKeywordLink = rLink; }
    void                SetKeyword( const OUString& rKeyword );
    bool                HasKeyword() const;
    bool                HasKeywordIgnoreCase();
    void                OpenKeyword();

    void         SelectExecutableEntry() { m_pIndexCB->SelectExecutableEntry(); }
};

// class SearchTabPage_Impl ----------------------------------------------

class SearchBox_Impl : public ComboBox
{
private:
    Link<LinkParamNone*,void>  aSearchLink;

public:
    SearchBox_Impl(vcl::Window* pParent, WinBits nStyle)
        : ComboBox(pParent, nStyle)
    {
        SetDropDownLineCount(5);
    }

    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual void        Select() override;

    void         SetSearchLink( const Link<LinkParamNone*,void>& rLink ) { aSearchLink = rLink; }
};

class SearchResultsBox_Impl : public ListBox
{
public:
    SearchResultsBox_Impl(vcl::Window* pParent, WinBits nStyle)
        : ListBox(pParent, nStyle)
    {
    }

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
};

class SearchTabPage_Impl : public HelpTabPage_Impl
{
private:
    VclPtr<SearchBox_Impl>         m_pSearchED;
    VclPtr<PushButton>             m_pSearchBtn;
    VclPtr<CheckBox>               m_pFullWordsCB;
    VclPtr<CheckBox>               m_pScopeCB;
    VclPtr<SearchResultsBox_Impl>  m_pResultsLB;
    VclPtr<PushButton>             m_pOpenBtn;

    OUString                       aFactory;

    css::uno::Reference< css::i18n::XBreakIterator >
                                   xBreakIterator;

    void                ClearSearchResults();
    void                RememberSearchText( const OUString& rSearchText );

    DECL_LINK(SearchHdl, LinkParamNone*, void);
    DECL_LINK(ClickHdl, Button*, void);
    DECL_LINK(OpenHdl, Button*, void);
    DECL_LINK(ModifyHdl, Edit&, void);

public:
    SearchTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    virtual ~SearchTabPage_Impl() override;
    virtual void dispose() override;

    virtual void        ActivatePage() override;
    virtual Control*    GetLastFocusControl() override;

    void                SetDoubleClickHdl( const Link<ListBox&,void>& rLink );
    void         SetFactory( const OUString& rFactory ) { aFactory = rFactory; }
    OUString            GetSelectedEntry() const;
    void                ClearPage();
    void         SetFocusOnBox() { m_pResultsLB->GrabFocus(); }
    bool         HasFocusOnEdit() const { return m_pSearchED->HasChildPathFocus(); }
    OUString     GetSearchText() const { return m_pSearchED->GetText(); }
    bool         IsFullWordSearch() const { return m_pFullWordsCB->IsChecked(); }
    bool                OpenKeyword( const OUString& rKeyword );
};

// class BookmarksTabPage_Impl -------------------------------------------

class BookmarksBox_Impl : public ListBox
{
private:
    void                DoAction( sal_uInt16 nAction );

public:
    BookmarksBox_Impl(vcl::Window* pParent, WinBits nStyle);
    virtual ~BookmarksBox_Impl() override;
    virtual void dispose() override;

    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;
};

class BookmarksTabPage_Impl : public HelpTabPage_Impl
{
private:
    VclPtr<BookmarksBox_Impl>  m_pBookmarksBox;
    VclPtr<PushButton>         m_pBookmarksPB;

    DECL_LINK(OpenHdl, Button*, void);

public:
    BookmarksTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    virtual ~BookmarksTabPage_Impl() override;
    virtual void dispose() override;

    virtual void        ActivatePage() override;
    virtual Control*    GetLastFocusControl() override;

    void                SetDoubleClickHdl( const Link<ListBox&,void>& rLink );
    OUString            GetSelectedEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    void                SetFocusOnBox() { m_pBookmarksBox->GrabFocus(); }
};

// class SfxHelpIndexWindow_Impl -----------------------------------------

class SfxHelpWindow_Impl;

class SfxHelpIndexWindow_Impl : public vcl::Window, public VclBuilderContainer
{
private:
    VclPtr<ListBox>            m_pActiveLB;
    VclPtr<TabControl>         m_pTabCtrl;

    Idle                aIdle;

    Link<SfxHelpIndexWindow_Impl*,void> aSelectFactoryLink;
    Link<Control*,bool>                 aPageDoubleClickLink;
    Link<IndexTabPage_Impl&,void> const aIndexKeywordLink;
    OUString                            sKeyword;

    VclPtr<SfxHelpWindow_Impl>     pParentWin;

    VclPtr<ContentTabPage_Impl>    pCPage;
    VclPtr<IndexTabPage_Impl>      pIPage;
    VclPtr<SearchTabPage_Impl>     pSPage;
    VclPtr<BookmarksTabPage_Impl>  pBPage;

    bool                bWasCursorLeftOrRight;
    bool                bIsInitDone;

    void                Initialize();
    void                SetActiveFactory();
    HelpTabPage_Impl*   GetCurrentPage( sal_uInt16& rCurId );

    inline ContentTabPage_Impl*     GetContentPage();
    inline IndexTabPage_Impl*       GetIndexPage();
    inline SearchTabPage_Impl*      GetSearchPage();
    inline BookmarksTabPage_Impl*   GetBookmarksPage();

    DECL_LINK(ActivatePageHdl, TabControl*, void );
    DECL_LINK(SelectHdl, ListBox&, void);
    DECL_LINK(InitHdl, Timer *, void);
    DECL_LINK(SelectFactoryHdl, Timer *, void);
    DECL_LINK(KeywordHdl, IndexTabPage_Impl&, void);
    DECL_LINK(ContentTabPageDoubleClickHdl, SvTreeListBox*, bool);
    DECL_LINK(TabPageDoubleClickHdl, ListBox&, void);
    DECL_LINK(IndexTabPageDoubleClickHdl, ComboBox&, void);

public:
    explicit SfxHelpIndexWindow_Impl( SfxHelpWindow_Impl* pParent );
    virtual ~SfxHelpIndexWindow_Impl() override;
    virtual void dispose() override;

    virtual void        Resize() override;
    virtual Size        GetOptimalSize() const override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                SetDoubleClickHdl( const Link<Control*,bool>& rLink );
    void         SetSelectFactoryHdl( const Link<SfxHelpIndexWindow_Impl*,void>& rLink ) { aSelectFactoryLink = rLink; }
    void                SetFactory( const OUString& rFactory, bool bActive );
    OUString const &    GetFactory() const { return pIPage->GetFactory(); }
    OUString            GetSelectedEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    bool                IsValidFactory( const OUString& _rFactory );
    OUString     GetActiveFactoryTitle() const { return m_pActiveLB->GetSelectedEntry(); }
    void                ClearSearchPage();
    void                GrabFocusBack();
    bool                HasFocusOnEdit() const;
    OUString            GetSearchText() const;
    bool                IsFullWordSearch() const;
    void                OpenKeyword( const OUString& rKeyword );
    void                SelectExecutableEntry();
    inline bool         WasCursorLeftOrRight();
};

// inlines ---------------------------------------------------------------

ContentTabPage_Impl* SfxHelpIndexWindow_Impl::GetContentPage()
{
    if ( !pCPage )
    {
        pCPage = VclPtr<ContentTabPage_Impl>::Create( m_pTabCtrl, this );
        pCPage->SetOpenHdl( LINK(this, SfxHelpIndexWindow_Impl, ContentTabPageDoubleClickHdl) );
    }
    return pCPage;
}
IndexTabPage_Impl* SfxHelpIndexWindow_Impl::GetIndexPage()
{
    if ( !pIPage )
    {
        pIPage = VclPtr<IndexTabPage_Impl>::Create( m_pTabCtrl, this );
        pIPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, IndexTabPageDoubleClickHdl) );
        pIPage->SetKeywordHdl( aIndexKeywordLink );
    }
    return pIPage;
}

SearchTabPage_Impl* SfxHelpIndexWindow_Impl::GetSearchPage()
{
    if ( !pSPage )
    {
        pSPage = VclPtr<SearchTabPage_Impl>::Create( m_pTabCtrl, this );
        pSPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl) );
    }
    return pSPage;
}

BookmarksTabPage_Impl* SfxHelpIndexWindow_Impl::GetBookmarksPage()
{
    if ( !pBPage )
    {
        pBPage = VclPtr<BookmarksTabPage_Impl>::Create( m_pTabCtrl, this );
        pBPage->SetDoubleClickHdl( LINK(this, SfxHelpIndexWindow_Impl, TabPageDoubleClickHdl) );
    }
    return pBPage;
}

bool SfxHelpIndexWindow_Impl::WasCursorLeftOrRight()
{
    bool bRet = bWasCursorLeftOrRight;
    bWasCursorLeftOrRight = false;
    return bRet;
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
    VclPtr<ToolBox>         aToolBox;
    VclPtr<CheckBox>        aOnStartupCB;
    Idle                    aSelectIdle;
    Image                   aIndexOnImage;
    Image                   aIndexOffImage;
    OUString const          aIndexOnText;
    OUString const          aIndexOffText;
    OUString                aSearchText;
    OUString const          aOnStartupText;
    OUString                sCurrentFactory;

    VclPtr<SfxHelpWindow_Impl>     pHelpWin;
    VclPtr<vcl::Window>            pTextWin;
    std::shared_ptr<sfx2::SearchDialog> m_xSrchDlg;
    css::uno::Reference < css::frame::XFrame2 >
                            xFrame;
    css::uno::Reference< css::i18n::XBreakIterator >
                            xBreakIterator;
    css::uno::Reference< css::uno::XInterface >
                            xConfiguration;
    long                    nMinPos;
    bool                    bIsDebug;
    bool                    bIsIndexOn;
    bool                    bIsInClose;
    bool                    bIsFullWordSearch;

    bool                    HasSelection() const;
    void                    InitToolBoxImages();
    void                    InitOnStartupBox();
    void                    SetOnStartupBoxPosition();

    css::uno::Reference< css::i18n::XBreakIterator > const &
                            GetBreakIterator();
    css::uno::Reference< css::text::XTextRange >
                            getCursor() const;
    bool                    isHandledKey( const vcl::KeyCode& _rKeyCode );

    DECL_LINK(        SelectHdl, Timer *, void);
    DECL_LINK(        NotifyHdl, LinkParamNone*, void );
    DECL_LINK(        FindHdl, sfx2::SearchDialog&, void );
    DECL_LINK(        CloseHdl, LinkParamNone*, void );
    DECL_LINK(        CheckHdl, Button*, void );
    void                    FindHdl(sfx2::SearchDialog*);

public:
    explicit SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent );
    virtual ~SfxHelpTextWindow_Impl() override;
    virtual void dispose() override;

    virtual void            Resize() override;
    virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
    virtual void            GetFocus() override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    const css::uno::Reference < css::frame::XFrame2 >&
                            getFrame() const { return xFrame; }

    void             SetSelectHdl( const Link<ToolBox *, void>& rLink ) { aToolBox->SetSelectHdl( rLink ); }
    void                    ToggleIndex( bool bOn );
    void                    SelectSearchText( const OUString& rSearchText, bool _bIsFullWordSearch );
    void                    SetPageStyleHeaderOff() const;
    ToolBox&         GetToolBox() { return *aToolBox; }
    void                    CloseFrame();
    void                    DoSearch();
};

// class SfxHelpWindow_Impl ----------------------------------------------

class HelpInterceptor_Impl;
class HelpListener_Impl;
class SfxHelpWindow_Impl : public SplitWindow
{
private:
friend class SfxHelpIndexWindow_Impl;

    css::uno::Reference < css::awt::XWindow >
                                xWindow;
    css::uno::Reference < css::frame::XFrame2 >
                                xFrame;

    VclPtr<SfxHelpIndexWindow_Impl>    pIndexWin;
    VclPtr<SfxHelpTextWindow_Impl>     pTextWin;
    HelpInterceptor_Impl*       pHelpInterceptor;
    rtl::Reference<HelpListener_Impl> pHelpListener;

    sal_Int32           nExpandWidth;
    sal_Int32           nCollapseWidth;
    sal_Int32           nHeight;
    long                nIndexSize;
    long                nTextSize;
    bool                bIndex;
    bool                bGrabFocusToToolBox;
    Point               aWinPos;
    OUString            sTitle;

    virtual void        Resize() override;
    virtual void        Split() override;
    virtual void        GetFocus() override;

    void                MakeLayout();
    void                InitSizes();
    void                LoadConfig();
    void                SaveConfig();
    void                ShowStartPage();

    DECL_LINK(    SelectHdl, ToolBox*, void );
    DECL_LINK(    OpenHdl, Control*, bool );
    DECL_LINK(    SelectFactoryHdl, SfxHelpIndexWindow_Impl*, void );
    DECL_LINK(    ChangeHdl, HelpListener_Impl&, void );

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
    void                SetHelpURL( const OUString& rURL );
    void                DoAction( sal_uInt16 nActionId );
    void                CloseWindow();

    void                UpdateToolbox();
    void         OpenKeyword( const OUString& rKeyword ) { pIndexWin->OpenKeyword( rKeyword ); }

    bool                HasHistoryPredecessor() const;      // forward to interceptor
    bool                HasHistorySuccessor() const;        // forward to interceptor

    void                openDone(const OUString& sURL    ,
                                       bool         bSuccess);

    static OUString     buildHelpURL(const OUString& sFactory        ,
                                     const OUString& sContent        ,
                                     const OUString& sAnchor);

    void                loadHelpContent(const OUString& sHelpURL                ,
                                              bool         bAddToHistory = true);
};

class SfxAddHelpBookmarkDialog_Impl : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xTitleED;
    std::unique_ptr<weld::Label> m_xAltTitle;
public:
    SfxAddHelpBookmarkDialog_Impl(weld::Window* pParent, bool bRename);

    void SetTitle( const OUString& rTitle );
    OUString GetTitle() const { return m_xTitleED->get_text(); }
};

/// Appends ?Language=xy&System=abc to the help URL in rURL
void AppendConfigToken(OUStringBuffer& rURL, bool bQuestionMark);

#endif // INCLUDED_SFX2_SOURCE_APPL_NEWHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
