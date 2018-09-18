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

#ifndef INCLUDED_CUI_SOURCE_INC_CUIGALDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIGALDLG_HXX

#include <sal/config.h>

#include <salhelper/thread.hxx>
#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/idle.hxx>
#include <svl/slstitm.hxx>
#include <svtools/transfer.hxx>
#include <vcl/GraphicObject.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/galctrl.hxx>
#include <svx/galmisc.hxx>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include <vector>

class GalleryTheme;
class SearchProgress;
class TakeProgress;
class TPGalleryThemeProperties;

typedef std::vector< sal_uLong > TokenList_impl;

struct FilterEntry
{
    OUString  aFilterName;
};

class SearchThread: public salhelper::Thread
{
private:

    VclPtr<SearchProgress>             mpProgress;
    VclPtr<TPGalleryThemeProperties>   mpBrowser;
    INetURLObject               maStartURL;

    void                        ImplSearch( const INetURLObject& rStartURL,
                                            const std::vector< OUString >& rFormats,
                                            bool bRecursive );

    virtual                     ~SearchThread() override;
    virtual void                execute() override;

public:

                                SearchThread( SearchProgress* pProgress,
                                              TPGalleryThemeProperties* pBrowser,
                                              const INetURLObject& rStartURL );
};

class SearchProgress : public ModalDialog
{
private:
    VclPtr<FixedText>          m_pFtSearchDir;
    VclPtr<FixedText>          m_pFtSearchType;
    VclPtr<CancelButton>       m_pBtnCancel;
    VclPtr<vcl::Window>        parent_;
    INetURLObject startUrl_;
    rtl::Reference< SearchThread > maSearchThread;

                        DECL_LINK( ClickCancelBtn, Button*, void );

public:
                        SearchProgress( vcl::Window* pParent, const INetURLObject& rStartURL );
    virtual             ~SearchProgress() override;
    virtual void        dispose() override;

                        DECL_LINK( CleanUpHdl, void*, void );

    virtual short       Execute() override;
    virtual void        StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;
    void                SetFileType( const OUString& rType ) { m_pFtSearchType->SetText( rType ); }
    void                SetDirectory( const INetURLObject& rURL ) { m_pFtSearchDir->SetText( GetReducedString( rURL, 30 ) ); }
};

class TakeThread: public salhelper::Thread
{
private:

    VclPtr<TakeProgress>               mpProgress;
    VclPtr<TPGalleryThemeProperties>   mpBrowser;
    TokenList_impl&             mrTakenList;

    virtual                     ~TakeThread() override;
    virtual void                execute() override;

public:

                                TakeThread(
                                    TakeProgress* pProgress,
                                    TPGalleryThemeProperties* pBrowser,
                                    TokenList_impl& rTakenList
                                );
};

class TakeProgress : public ModalDialog
{
private:
    VclPtr<FixedText>          m_pFtTakeFile;
    VclPtr<CancelButton>       m_pBtnCancel;
    VclPtr<vcl::Window>        window_;
    rtl::Reference< TakeThread > maTakeThread;
    TokenList_impl      maTakenList;

    DECL_LINK( ClickCancelBtn, Button*, void );

public:

    TakeProgress( vcl::Window* pWindow );
    virtual ~TakeProgress() override;
    virtual void dispose() override;

    DECL_LINK( CleanUpHdl, void*, void );

    void                SetFile( const INetURLObject& rURL ) { m_pFtTakeFile->SetText( GetReducedString( rURL, 30 ) ); }
    virtual short       Execute() override;
    virtual void        StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;
};

class ActualizeProgress : public ModalDialog
{
private:
    VclPtr<FixedText>          m_pFtActualizeFile;
    VclPtr<CancelButton>       m_pBtnCancel;
    Idle*               pIdle;
    GalleryTheme*       pTheme;
    GalleryProgress     aStatusProgress;

                        DECL_LINK( ClickCancelBtn, Button*, void );
                        DECL_LINK( TimeoutHdl, Timer*, void );
                        DECL_LINK( ActualizeHdl, const INetURLObject&, void );

public:
                        ActualizeProgress( vcl::Window* pWindow, GalleryTheme* pThm );
    virtual             ~ActualizeProgress() override;
    virtual void        dispose() override;

    virtual short       Execute() override;
};

class TitleDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
public:
    TitleDialog(weld::Window* pParent, const OUString& rOldText);
    virtual ~TitleDialog() override;
    OUString GetTitle() const { return m_xEdit->get_text(); }
};

class GalleryIdDialog : public weld::GenericDialogController
{
private:
    GalleryTheme* m_pThm;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::ComboBox> m_xLbResName;

    DECL_LINK(ClickOkHdl, weld::Button&, void);
public:
    GalleryIdDialog(weld::Window* pParent, GalleryTheme* pThm);
    virtual ~GalleryIdDialog() override;
    sal_uInt32 GetId() const { return m_xLbResName->get_active(); }
};

class GalleryThemeProperties : public SfxTabDialog
{
    ExchangeData*   pData;

    sal_uInt16 m_nGeneralPageId;

    virtual void PageCreated(sal_uInt16 nId, SfxTabPage &rPage) override;

public:
    GalleryThemeProperties(vcl::Window* pParent, ExchangeData* pData, SfxItemSet const * pItemSet);
};

class TPGalleryThemeGeneral : public SfxTabPage
{
private:
    ExchangeData*       pData;

    std::unique_ptr<weld::Image> m_xFiMSImage;
    std::unique_ptr<weld::Entry> m_xEdtMSName;
    std::unique_ptr<weld::Label> m_xFtMSShowType;
    std::unique_ptr<weld::Label> m_xFtMSShowPath;
    std::unique_ptr<weld::Label> m_xFtMSShowContent;
    std::unique_ptr<weld::Label> m_xFtMSShowChangeDate;

    virtual void        Reset( const SfxItemSet* ) override {}
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;

public:
    TPGalleryThemeGeneral(TabPageParent pParent, const SfxItemSet& rSet);
    void                SetXChgData( ExchangeData* pData );
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
};

class TPGalleryThemeProperties : public SfxTabPage
{
    friend class SearchThread;
    friend class TakeProgress;
    friend class TakeThread;

    VclPtr<ComboBox>           m_pCbbFileType;
    VclPtr<ListBox>            m_pLbxFound;
    VclPtr<PushButton>         m_pBtnSearch;
    VclPtr<PushButton>         m_pBtnTake;
    VclPtr<PushButton>         m_pBtnTakeAll;
    VclPtr<CheckBox>           m_pCbxPreview;
    VclPtr<GalleryPreview>     m_pWndPreview;

    ExchangeData*           pData;
    std::vector<OUString>   aFoundList;
    std::vector< std::unique_ptr<FilterEntry> >
                            aFilterEntryList;
    Timer                   aPreviewTimer;
    OUString                aLastFilterName;
    OUString                aPreviewString;
    INetURLObject           aURL;
    sal_uInt16              nFirstExtFilterPos;
    bool                    bEntriesFound;
    bool                    bInputAllowed;
    bool                    bTakeAll;
    bool                    bSearchRecursive;

    rtl::Reference< ::svt::DialogClosedListener >           xDialogListener;
    css::uno::Reference< css::media::XPlayer >              xMediaPlayer;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    virtual void        Reset( const SfxItemSet* /*rSet*/ ) override {}
    virtual bool        FillItemSet( SfxItemSet* /*rSet*/ ) override { return true; }
    static OUString     addExtension( const OUString&, const OUString& );
    void                FillFilterList();

    void                SearchFiles();
    void                TakeFiles();
    void                DoPreview();

                        DECL_LINK( ClickPreviewHdl, Button*, void );
                        DECL_LINK( ClickSearchHdl, Button*, void );
                        DECL_LINK( ClickTakeHdl, Button*, void );
                        DECL_LINK( ClickTakeAllHdl, Button*, void );
                        DECL_LINK( SelectFoundHdl, ListBox&, void );
                        DECL_LINK( SelectFileTypeHdl, ComboBox&, void );
                        DECL_LINK( DClickFoundHdl, ListBox&, void );
                        DECL_LINK( PreviewTimerHdl, Timer*, void );
                        DECL_LINK( EndSearchProgressHdl, Dialog&, void );
                        DECL_LINK( DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void );

public:
                        TPGalleryThemeProperties( vcl::Window* pWindow, const SfxItemSet& rSet );
                        virtual ~TPGalleryThemeProperties() override;
    virtual void        dispose() override;

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    void                StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult );

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
};

#endif // INCLUDED_CUI_SOURCE_INC_CUIGALDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
