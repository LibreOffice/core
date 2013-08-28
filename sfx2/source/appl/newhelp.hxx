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
#ifndef INCLUDED_SFX_NEWHELP_HXX
#define INCLUDED_SFX_NEWHELP_HXX

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

// class OpenStatusListener_Impl -----------------------------------------

class OpenStatusListener_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatchResultListener >
{
private:
    sal_Bool    m_bFinished;
    sal_Bool    m_bSuccess;
    Link        m_aOpenLink;
    OUString    m_sURL;

public:
    OpenStatusListener_Impl() : m_bFinished( sal_False ), m_bSuccess( sal_False ) {}

    virtual void SAL_CALL   dispatchFinished( const ::com::sun::star::frame::DispatchResultEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    inline sal_Bool         IsFinished() const { return m_bFinished; }
    inline sal_Bool         IsSuccessful() const { return m_bSuccess; }
    inline void             SetURL( const OUString& rURL ) { m_sURL = rURL; }
    inline OUString         GetURL() const  { return m_sURL; }
    inline void             SetOpenHdl( const Link& rLink ) { m_aOpenLink = rLink; }
};

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
    ContentListBox_Impl( Window* pParent, const ResId& rResId );
    ~ContentListBox_Impl();


    virtual void    RequestingChildren( SvTreeListEntry* pParent );
    virtual long    Notify( NotifyEvent& rNEvt );

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
    HelpTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin, const ResId& rResId );

    virtual Control*    GetLastFocusControl() = 0;
};

// class ContentTabPage_Impl ---------------------------------------------

class ContentTabPage_Impl : public HelpTabPage_Impl
{
private:
    ContentListBox_Impl aContentBox;

public:
    ContentTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );

    virtual void        Resize();
    virtual void        ActivatePage();
    virtual Control*    GetLastFocusControl();

    inline void     SetOpenHdl( const Link& rLink ) { aContentBox.SetOpenHdl( rLink ); }
    inline OUString GetSelectEntry() const { return aContentBox.GetSelectEntry(); }
    inline void     SetFocusOnBox() { aContentBox.GrabFocus(); }
};

// class IndexTabPage_Impl -----------------------------------------------

class IndexBox_Impl : public ComboBox
{
public:
    IndexBox_Impl( Window* pParent, const ResId& rResId );

    virtual void        UserDraw( const UserDrawEvent& rUDEvt );
    virtual long        Notify( NotifyEvent& rNEvt );

    void                SelectExecutableEntry();
};

class IndexTabPage_Impl : public HelpTabPage_Impl
{
private:
    FixedText           aExpressionFT;
    IndexBox_Impl       aIndexCB;
    PushButton          aOpenBtn;

    Timer               aFactoryTimer;
    Timer               aKeywordTimer;
    Link                aKeywordLink;

    OUString            sFactory;
    OUString            sKeyword;

    long                nMinWidth;
    sal_Bool            bIsActivated;

    void                InitializeIndex();
    void                ClearIndex();

    DECL_LINK(OpenHdl, void *);
    DECL_LINK(          TimeoutHdl, Timer* );

public:
    IndexTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    ~IndexTabPage_Impl();

    virtual void        Resize();
    virtual void        ActivatePage();
    virtual Control*    GetLastFocusControl();

    void                SetDoubleClickHdl( const Link& rLink );
    void                SetFactory( const OUString& rFactory );
    inline OUString     GetFactory() const { return sFactory; }
    OUString            GetSelectEntry() const;
    inline void         SetFocusOnBox() { aIndexCB.GrabFocus(); }
    inline sal_Bool     HasFocusOnEdit() const { return aIndexCB.HasChildPathFocus(); }

    inline void         SetKeywordHdl( const Link& rLink ) { aKeywordLink = rLink; }
    void                SetKeyword( const OUString& rKeyword );
    sal_Bool            HasKeyword() const;
    sal_Bool            HasKeywordIgnoreCase();
    void                OpenKeyword();

    inline void         SelectExecutableEntry() { aIndexCB.SelectExecutableEntry(); }
};

// class SearchTabPage_Impl ----------------------------------------------

class SearchBox_Impl : public ComboBox
{
private:
    Link                aSearchLink;

public:
    SearchBox_Impl( Window* pParent, const ResId& rResId ) :
        ComboBox( pParent, rResId ) { SetDropDownLineCount( 5 ); }

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        Select();

    inline void         SetSearchLink( const Link& rLink ) { aSearchLink = rLink; }
};

class SearchResultsBox_Impl : public ListBox
{
public:
    SearchResultsBox_Impl( Window* pParent, const ResId& rResId ) : ListBox( pParent, rResId ) {}

    virtual long    Notify( NotifyEvent& rNEvt );
};

class SearchTabPage_Impl : public HelpTabPage_Impl
{
private:
    FixedText               aSearchFT;
    SearchBox_Impl          aSearchED;
    PushButton              aSearchBtn;
    CheckBox                aFullWordsCB;
    CheckBox                aScopeCB;
    SearchResultsBox_Impl   aResultsLB;
    PushButton              aOpenBtn;

    Size                    aMinSize;
    OUString                aFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            xBreakIterator;

    void                ClearSearchResults();
    void                RememberSearchText( const OUString& rSearchText );

    DECL_LINK(SearchHdl, void *);
    DECL_LINK(OpenHdl, void *);
    DECL_LINK(ModifyHdl, void *);

public:
    SearchTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );
    ~SearchTabPage_Impl();

    virtual void        Resize();
    virtual void        ActivatePage();
    virtual Control*    GetLastFocusControl();

    void                SetDoubleClickHdl( const Link& rLink );
    inline void         SetFactory( const OUString& rFactory ) { aFactory = rFactory; }
    OUString            GetSelectEntry() const;
    void                ClearPage();
    inline void         SetFocusOnBox() { aResultsLB.GrabFocus(); }
    inline sal_Bool     HasFocusOnEdit() const { return aSearchED.HasChildPathFocus(); }
    inline OUString     GetSearchText() const { return aSearchED.GetText(); }
    inline sal_Bool     IsFullWordSearch() const { return aFullWordsCB.IsChecked(); }
    sal_Bool            OpenKeyword( const OUString& rKeyword );
};

// class BookmarksTabPage_Impl -------------------------------------------

class BookmarksBox_Impl : public ListBox
{
private:
    void                DoAction( sal_uInt16 nAction );

public:
    BookmarksBox_Impl( Window* pParent, const ResId& rResId );
    ~BookmarksBox_Impl();

    virtual long        Notify( NotifyEvent& rNEvt );
};

class BookmarksTabPage_Impl : public HelpTabPage_Impl
{
private:
    FixedText           aBookmarksFT;
    BookmarksBox_Impl   aBookmarksBox;
    PushButton          aBookmarksPB;

    long                nMinWidth;

    DECL_LINK(OpenHdl, void *);

public:
    BookmarksTabPage_Impl( Window* pParent, SfxHelpIndexWindow_Impl* _pIdxWin );

    virtual void        Resize();
    virtual void        ActivatePage();
    virtual Control*    GetLastFocusControl();

    void                SetDoubleClickHdl( const Link& rLink );
    OUString            GetSelectEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    inline void         SetFocusOnBox() { aBookmarksBox.GrabFocus(); }
};

// class SfxHelpIndexWindow_Impl -----------------------------------------

class SfxHelpWindow_Impl;

class SfxHelpIndexWindow_Impl : public Window
{
private:
    ListBox             aActiveLB;
    FixedLine           aActiveLine;

    TabControl          aTabCtrl;
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
    ~SfxHelpIndexWindow_Impl();

    virtual void        Resize();
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                SetDoubleClickHdl( const Link& rLink );
    inline void         SetSelectFactoryHdl( const Link& rLink ) { aSelectFactoryLink = rLink; }
    void                SetFactory( const OUString& rFactory, sal_Bool bActive );
    inline OUString     GetFactory() const { return pIPage->GetFactory(); }
    OUString            GetSelectEntry() const;
    void                AddBookmarks( const OUString& rTitle, const OUString& rURL );
    bool                IsValidFactory( const OUString& _rFactory );
    inline OUString     GetActiveFactoryTitle() const { return aActiveLB.GetSelectEntry(); }
    inline void         UpdateTabControl() { aTabCtrl.Invalidate(); }
    void                ClearSearchPage();
    void                GrabFocusBack();
    sal_Bool            HasFocusOnEdit() const;
    OUString            GetSearchText() const;
    sal_Bool            IsFullWordSearch() const;
    void                OpenKeyword( const OUString& rKeyword );
    void                SelectExecutableEntry();
    inline bool         WasCursorLeftOrRight();
};

// inlines ---------------------------------------------------------------

ContentTabPage_Impl* SfxHelpIndexWindow_Impl::GetContentPage()
{
    if ( !pCPage )
    {
        pCPage = new ContentTabPage_Impl( &aTabCtrl, this );
        pCPage->SetOpenHdl( aPageDoubleClickLink );
    }
    return pCPage;
}
IndexTabPage_Impl* SfxHelpIndexWindow_Impl::GetIndexPage()
{
    if ( !pIPage )
    {
        pIPage = new IndexTabPage_Impl( &aTabCtrl, this );
        pIPage->SetDoubleClickHdl( aPageDoubleClickLink );
        pIPage->SetKeywordHdl( aIndexKeywordLink );
    }
    return pIPage;
}

SearchTabPage_Impl* SfxHelpIndexWindow_Impl::GetSearchPage()
{
    if ( !pSPage )
    {
        pSPage = new SearchTabPage_Impl( &aTabCtrl, this );
        pSPage->SetDoubleClickHdl( aPageDoubleClickLink );
    }
    return pSPage;
}

BookmarksTabPage_Impl* SfxHelpIndexWindow_Impl::GetBookmarksPage()
{
    if ( !pBPage )
    {
        pBPage = new BookmarksTabPage_Impl( &aTabCtrl, this );
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
                            TextWin_Impl( Window* pParent );
    virtual                 ~TextWin_Impl();

    virtual long            Notify( NotifyEvent& rNEvt );
};

// class SfxHelpTextWindow_Impl ------------------------------------------

class SvtMiscOptions;
class SfxHelpWindow_Impl;

class SfxHelpTextWindow_Impl : public Window
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
    Window*                 pTextWin;
    sfx2::SearchDialog*     pSrchDlg;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                            xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            xBreakIterator;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                            xConfiguration;
    long                    nMinPos;
    sal_Bool                bIsDebug;
    sal_Bool                bIsIndexOn;
    sal_Bool                bIsInClose;
    sal_Bool                bIsFullWordSearch;

    sal_Bool                HasSelection() const;
    void                    InitToolBoxImages();
    void                    InitOnStartupBox( bool bOnlyText );
    void                    SetOnStartupBoxPosition();

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >
                            GetBreakIterator();
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                            getCursor() const;
    bool                    isHandledKey( const KeyCode& _rKeyCode );

    DECL_LINK(SelectHdl, void *);
    DECL_LINK(              NotifyHdl, SvtMiscOptions* );
    DECL_LINK(              FindHdl, sfx2::SearchDialog* );
    DECL_LINK(              CloseHdl, sfx2::SearchDialog* );
    DECL_LINK(              CheckHdl, CheckBox* );

public:
    SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent );
    ~SfxHelpTextWindow_Impl();

    virtual void            Resize();
    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual void            GetFocus();
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    inline ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                            getFrame() const { return xFrame; }

    inline void             SetSelectHdl( const Link& rLink ) { aToolBox.SetSelectHdl( rLink ); }
    void                    ToggleIndex( sal_Bool bOn );
    void                    SelectSearchText( const OUString& rSearchText, sal_Bool _bIsFullWordSearch );
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
    sal_Bool            bIndex;
    sal_Bool            bGrabFocusToToolBox;
    Point               aWinPos;
    OUString            sTitle;
    OUString            sKeyword;

    virtual void        Resize();
    virtual void        Split();
    virtual void        GetFocus();

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
                        Window* pParent, WinBits nBits );
    ~SfxHelpWindow_Impl();

    virtual long        PreNotify( NotifyEvent& rNEvt );

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

    sal_Bool            HasHistoryPredecessor() const;      // forward to interceptor
    sal_Bool            HasHistorySuccessor() const;        // forward to interceptor

    void                openDone(const OUString& sURL    ,
                                       sal_Bool         bSuccess);

    static OUString  buildHelpURL(const OUString& sFactory        ,
                                         const OUString& sContent        ,
                                         const OUString& sAnchor         ,
                                               sal_Bool         bUseQuestionMark);

    void                loadHelpContent(const OUString& sHelpURL                ,
                                              sal_Bool         bAddToHistory = sal_True);
};

class SfxAddHelpBookmarkDialog_Impl : public ModalDialog
{
private:
    FixedText       aTitleFT;
    Edit            aTitleED;
    OKButton        aOKBtn;
    CancelButton    aEscBtn;
    HelpButton      aHelpBtn;

public:
    SfxAddHelpBookmarkDialog_Impl( Window* pParent, sal_Bool bRename = sal_True );
    ~SfxAddHelpBookmarkDialog_Impl();

    void            SetTitle( const OUString& rTitle );
    inline OUString GetTitle() const { return aTitleED.GetText(); }
};

/// Appends ?Language=xy&System=abc to the help URL in rURL
void AppendConfigToken(OUStringBuffer& rURL, sal_Bool bQuestionMark, const OUString &rLang = OUString());

#endif // #ifndef INCLUDED_SFX_NEWHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
