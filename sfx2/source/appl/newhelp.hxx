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

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame2.hpp>

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace i18n { class XBreakIterator; } } } }
namespace com { namespace sun { namespace star { namespace text { class XTextRange; } } } }

#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <svtools/treelistbox.hxx>
#include <unotools/moduleoptions.hxx>

#include "srchdlg.hxx"

// ContentListBox_Impl ---------------------------------------------------

class ContentListBox_Impl : public SvTreeListBox
{
private:
    Image           aOpenBookImage;
    Image           aClosedBookImage;
    Image           aDocumentImage;

    void            InitRoot();
    void            ClearChildren( SvTreeListEntry* pParent );

public:
    ContentListBox_Impl(vcl::Window* pParent, WinBits nStyle);
    virtual ~ContentListBox_Impl();


    virtual void    RequestingChildren( SvTreeListEntry* pParent ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    inline void     SetOpenHdl( const Link& rLink ) { SetDoubleClickHdl( rLink ); }
    OUString        GetSelectEntry() const;
};

// class HelpTabPage_Impl ------------------------------------------------

class SfxHelpIndexWindow_Impl;

class HelpTabPage_Impl : public TabPage
{
protected:
    SfxHelpIndexWindow_Impl*    m_pIdxWin;

public:
    HelpTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin,
        const OString& rID, const OUString& rUIXMLDescription);

    virtual Control*    GetLastFocusControl() = 0;
};

// class ContentTabPage_Impl ---------------------------------------------

class ContentTabPage_Impl : public HelpTabPage_Impl
{
private:
    ContentListBox_Impl* m_pContentBox;

public:
    ContentTabPage_Impl(vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin);

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual Control*    GetLastFocusControl() SAL_OVERRIDE;

    void     SetOpenHdl( const Link& rLink ) { m_pContentBox->SetOpenHdl( rLink ); }
    OUString GetSelectEntry() const { return m_pContentBox->GetSelectEntry(); }
    void     SetFocusOnBox() { m_pContentBox->GrabFocus(); }
};

// class IndexTabPage_Impl -----------------------------------------------

class IndexBox_Impl : public ComboBox
{
public:
    IndexBox_Impl(vcl::Window* pParent, WinBits nStyle);

    virtual void        UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;
    virtual bool        Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void                SelectExecutableEntry();
};

class IndexTabPage_Impl : public HelpTabPage_Impl
{
private:
    IndexBox_Impl*      m_pIndexCB;
    PushButton*         m_pOpenBtn;

    Timer               aFactoryTimer;
    Timer               aKeywordTimer;
    Link                aKeywordLink;

    OUString            sFactory;
    OUString            sKeyword;

    bool                bIsActivated;

    void                InitializeIndex();
    void                ClearIndex();

    DECL_LINK(OpenHdl, void *);
    DECL_LINK(          TimeoutHdl, Timer* );

public:
    IndexTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    virtual ~IndexTabPage_Impl();

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual Control*    GetLastFocusControl() SAL_OVERRIDE;

    void                SetDoubleClickHdl( const Link& rLink );
    void                SetFactory( const OUString& rFactory );
    inline OUString     GetFactory() const { return sFactory; }
    OUString            GetSelectEntry() const;
    inline void         SetFocusOnBox() { m_pIndexCB->GrabFocus(); }
    inline bool     HasFocusOnEdit() const { return m_pIndexCB->HasChildPathFocus(); }

    inline void         SetKeywordHdl( const Link& rLink ) { aKeywordLink = rLink; }
    void                SetKeyword( const OUString& rKeyword );
    bool            HasKeyword() const;
    bool            HasKeywordIgnoreCase();
    void                OpenKeyword();

    inline void         SelectExecutableEntry() { m_pIndexCB->SelectExecutableEntry(); }
};

// class SearchTabPage_Impl ----------------------------------------------

class SearchBox_Impl : public ComboBox
{
private:
    Link                aSearchLink;

public:
    SearchBox_Impl(vcl::Window* pParent, WinBits nStyle)
        : ComboBox(pParent, nStyle)
    {
        SetDropDownLineCount(5);
    }

    virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void        Select() SAL_OVERRIDE;

    inline void         SetSearchLink( const Link& rLink ) { aSearchLink = rLink; }
};

class SearchResultsBox_Impl : public ListBox
{
public:
    SearchResultsBox_Impl(vcl::Window* pParent, WinBits nStyle)
        : ListBox(pParent, nStyle)
    {
    }

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

class SearchTabPage_Impl : public HelpTabPage_Impl
{
private:
    SearchBox_Impl*         m_pSearchED;
    PushButton*             m_pSearchBtn;
    CheckBox*               m_pFullWordsCB;
    CheckBox*               m_pScopeCB;
    SearchResultsBox_Impl*  m_pResultsLB;
    PushButton*             m_pOpenBtn;

    OUString                aFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            xBreakIterator;

    void                ClearSearchResults();
    void                RememberSearchText( const OUString& rSearchText );

    DECL_LINK(SearchHdl, void *);
    DECL_LINK(OpenHdl, void *);
    DECL_LINK(ModifyHdl, void *);

public:
    SearchTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    virtual ~SearchTabPage_Impl();

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual Control*    GetLastFocusControl() SAL_OVERRIDE;

    void                SetDoubleClickHdl( const Link& rLink );
    inline void         SetFactory( const OUString& rFactory ) { aFactory = rFactory; }
    OUString            GetSelectEntry() const;
    void                ClearPage();
    inline void         SetFocusOnBox() { m_pResultsLB->GrabFocus(); }
    inline bool     HasFocusOnEdit() const { return m_pSearchED->HasChildPathFocus(); }
    inline OUString     GetSearchText() const { return m_pSearchED->GetText(); }
    inline bool     IsFullWordSearch() const { return m_pFullWordsCB->IsChecked(); }
    bool            OpenKeyword( const OUString& rKeyword );
};

// class BookmarksTabPage_Impl -------------------------------------------

class BookmarksBox_Impl : public ListBox
{
private:
    void                DoAction( sal_uInt16 nAction );

public:
    BookmarksBox_Impl(vcl::Window* pParent, WinBits nStyle);
    virtual ~BookmarksBox_Impl();

    virtual bool        Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

class BookmarksTabPage_Impl : public HelpTabPage_Impl
{
private:
    BookmarksBox_Impl*  m_pBookmarksBox;
    PushButton*         m_pBookmarksPB;

    DECL_LINK(OpenHdl, void *);

public:
    BookmarksTabPage_Impl( vcl::Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual Control*    GetLastFocusControl() SAL_OVERRIDE;

    void                SetDoubleClickHdl( const Link& rLink );
    OUString            GetSelectEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    void                SetFocusOnBox() { m_pBookmarksBox->GrabFocus(); }
};

// class SfxHelpIndexWindow_Impl -----------------------------------------

class SfxHelpWindow_Impl;

class SfxHelpIndexWindow_Impl : public vcl::Window, public VclBuilderContainer
{
private:
    ListBox*            m_pActiveLB;
    TabControl*         m_pTabCtrl;

    Timer               aTimer;

    Link                aSelectFactoryLink;
    Link                aPageDoubleClickLink;
    Link                aIndexKeywordLink;
    OUString            sKeyword;

    SfxHelpWindow_Impl*     pParentWin;

    ContentTabPage_Impl*    pCPage;
    IndexTabPage_Impl*      pIPage;
    SearchTabPage_Impl*     pSPage;
    BookmarksTabPage_Impl*  pBPage;

    long                nMinWidth;
    bool                bWasCursorLeftOrRight;
    bool                bIsInitDone;

    void                Initialize();
    void                SetActiveFactory();
    HelpTabPage_Impl*   GetCurrentPage( sal_uInt16& rCurId );

    inline ContentTabPage_Impl*     GetContentPage();
    inline IndexTabPage_Impl*       GetIndexPage();
    inline SearchTabPage_Impl*      GetSearchPage();
    inline BookmarksTabPage_Impl*   GetBookmarksPage();

    DECL_LINK(          ActivatePageHdl, TabControl* );
    DECL_LINK(SelectHdl, void *);
    DECL_LINK(InitHdl, void *);
    DECL_LINK(SelectFactoryHdl, void *);
    DECL_LINK(KeywordHdl, void *);

public:
    SfxHelpIndexWindow_Impl( SfxHelpWindow_Impl* pParent );
    virtual ~SfxHelpIndexWindow_Impl();

    virtual void        Resize() SAL_OVERRIDE;
    virtual Size        GetOptimalSize() const SAL_OVERRIDE;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void                SetDoubleClickHdl( const Link& rLink );
    inline void         SetSelectFactoryHdl( const Link& rLink ) { aSelectFactoryLink = rLink; }
    void                SetFactory( const OUString& rFactory, bool bActive );
    inline OUString     GetFactory() const { return pIPage->GetFactory(); }
    OUString            GetSelectEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    bool                IsValidFactory( const OUString& _rFactory );
    inline OUString     GetActiveFactoryTitle() const { return m_pActiveLB->GetSelectEntry(); }
    inline void         UpdateTabControl() { m_pTabCtrl->Invalidate(); }
    void                ClearSearchPage();
    void                GrabFocusBack();
    bool            HasFocusOnEdit() const;
    OUString            GetSearchText() const;
    bool            IsFullWordSearch() const;
    void                OpenKeyword( const OUString& rKeyword );
    void                SelectExecutableEntry();
    inline bool         WasCursorLeftOrRight();
};

// inlines ---------------------------------------------------------------

ContentTabPage_Impl* SfxHelpIndexWindow_Impl::GetContentPage()
{
    if ( !pCPage )
    {
        pCPage = new ContentTabPage_Impl( m_pTabCtrl, this );
        pCPage->SetOpenHdl( aPageDoubleClickLink );
    }
    return pCPage;
}
IndexTabPage_Impl* SfxHelpIndexWindow_Impl::GetIndexPage()
{
    if ( !pIPage )
    {
        pIPage = new IndexTabPage_Impl( m_pTabCtrl, this );
        pIPage->SetDoubleClickHdl( aPageDoubleClickLink );
        pIPage->SetKeywordHdl( aIndexKeywordLink );
    }
    return pIPage;
}

SearchTabPage_Impl* SfxHelpIndexWindow_Impl::GetSearchPage()
{
    if ( !pSPage )
    {
        pSPage = new SearchTabPage_Impl( m_pTabCtrl, this );
        pSPage->SetDoubleClickHdl( aPageDoubleClickLink );
    }
    return pSPage;
}

BookmarksTabPage_Impl* SfxHelpIndexWindow_Impl::GetBookmarksPage()
{
    if ( !pBPage )
    {
        pBPage = new BookmarksTabPage_Impl( m_pTabCtrl, this );
        pBPage->SetDoubleClickHdl( aPageDoubleClickLink );
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
                            TextWin_Impl( vcl::Window* pParent );
    virtual                 ~TextWin_Impl();

    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

// class SfxHelpTextWindow_Impl ------------------------------------------

class SvtMiscOptions;
class SfxHelpWindow_Impl;

class SfxHelpTextWindow_Impl : public vcl::Window
{
private:
    ToolBox                 aToolBox;
    CheckBox                aOnStartupCB;
    Timer                   aSelectTimer;
    Image                   aIndexOnImage;
    Image                   aIndexOffImage;
    OUString                aIndexOnText;
    OUString                aIndexOffText;
    OUString                aSearchText;
    OUString                aOnStartupText;
    OUString                sCurrentFactory;

    SfxHelpWindow_Impl*     pHelpWin;
    vcl::Window*                 pTextWin;
    sfx2::SearchDialog*     pSrchDlg;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                            xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            xBreakIterator;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                            xConfiguration;
    long                    nMinPos;
    bool                bIsDebug;
    bool                bIsIndexOn;
    bool                bIsInClose;
    bool                bIsFullWordSearch;

    bool                HasSelection() const;
    void                    InitToolBoxImages();
    void                    InitOnStartupBox( bool bOnlyText );
    void                    SetOnStartupBoxPosition();

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            GetBreakIterator();
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                            getCursor() const;
    bool                    isHandledKey( const vcl::KeyCode& _rKeyCode );

    DECL_LINK(SelectHdl, void *);
    DECL_LINK(              NotifyHdl, SvtMiscOptions* );
    DECL_LINK(              FindHdl, sfx2::SearchDialog* );
    DECL_LINK(              CloseHdl, sfx2::SearchDialog* );
    DECL_LINK(              CheckHdl, CheckBox* );

public:
    SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent );
    virtual ~SfxHelpTextWindow_Impl();

    virtual void            Resize() SAL_OVERRIDE;
    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            GetFocus() SAL_OVERRIDE;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    inline ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                            getFrame() const { return xFrame; }

    inline void             SetSelectHdl( const Link& rLink ) { aToolBox.SetSelectHdl( rLink ); }
    void                    ToggleIndex( bool bOn );
    void                    SelectSearchText( const OUString& rSearchText, bool _bIsFullWordSearch );
    void                    SetPageStyleHeaderOff() const;
    inline ToolBox&         GetToolBox() { return aToolBox; }
     void                   CloseFrame();
    void                    DoSearch();
};

// class SfxHelpWindow_Impl ----------------------------------------------

class HelpInterceptor_Impl;
class HelpListener_Impl;
class SfxHelpWindow_Impl : public SplitWindow
{
private:
friend class SfxHelpIndexWindow_Impl;

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow >
                                xWindow;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchResultListener >
                                xOpenListener;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                                xFrame;

    SfxHelpIndexWindow_Impl*    pIndexWin;
    SfxHelpTextWindow_Impl*     pTextWin;
    HelpInterceptor_Impl*       pHelpInterceptor;
    HelpListener_Impl*          pHelpListener;

    sal_Int32           nExpandWidth;
    sal_Int32           nCollapseWidth;
    sal_Int32           nHeight;
    long                nIndexSize;
    long                nTextSize;
    bool            bIndex;
    bool            bGrabFocusToToolBox;
    Point               aWinPos;
    OUString            sTitle;
    OUString            sKeyword;

    virtual void        Resize() SAL_OVERRIDE;
    virtual void        Split() SAL_OVERRIDE;
    virtual void        GetFocus() SAL_OVERRIDE;

    void                MakeLayout();
    void                InitSizes();
    void                LoadConfig();
    void                SaveConfig();
    void                ShowStartPage();

    DECL_LINK(          SelectHdl, ToolBox* );
    DECL_LINK(OpenHdl, void *);
    DECL_LINK(          SelectFactoryHdl, SfxHelpIndexWindow_Impl* );
    DECL_LINK(          ChangeHdl, HelpListener_Impl* );

public:
    SfxHelpWindow_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >& rFrame,
                        vcl::Window* pParent, WinBits nBits );
    virtual ~SfxHelpWindow_Impl();

    virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void                setContainerWindow(
                            ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow > xWin );
    inline ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                        getTextFrame() const { return pTextWin->getFrame(); }
    inline ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchResultListener >
                        getOpenListener() const { return xOpenListener; }

    void                SetFactory( const OUString& rFactory );
    void                SetHelpURL( const OUString& rURL );
    void                DoAction( sal_uInt16 nActionId );
    void                CloseWindow();

    void                UpdateToolbox();
    inline void         OpenKeyword( const OUString& rKeyword ) { pIndexWin->OpenKeyword( rKeyword ); }
    inline OUString     GetFactory() const { return pIndexWin->GetFactory(); }

    bool            HasHistoryPredecessor() const;      // forward to interceptor
    bool            HasHistorySuccessor() const;        // forward to interceptor

    void                openDone(const OUString& sURL    ,
                                       bool         bSuccess);

    static OUString  buildHelpURL(const OUString& sFactory        ,
                                         const OUString& sContent        ,
                                         const OUString& sAnchor         ,
                                               bool         bUseQuestionMark);

    void                loadHelpContent(const OUString& sHelpURL                ,
                                              bool         bAddToHistory = true);
};

class SfxAddHelpBookmarkDialog_Impl : public ModalDialog
{
private:
    Edit* m_pTitleED;
public:
    SfxAddHelpBookmarkDialog_Impl( vcl::Window* pParent, bool bRename = true );

    void SetTitle( const OUString& rTitle );
    OUString GetTitle() const { return m_pTitleED->GetText(); }
};

/// Appends ?Language=xy&System=abc to the help URL in rURL
void AppendConfigToken(OUStringBuffer& rURL, bool bQuestionMark, const OUString &rLang = OUString());

#endif // INCLUDED_SFX2_SOURCE_APPL_NEWHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
