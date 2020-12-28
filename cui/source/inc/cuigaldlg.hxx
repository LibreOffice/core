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

#pragma once

#include <sal/config.h>

#include <salhelper/thread.hxx>
#include <tools/urlobj.hxx>
#include <vcl/idle.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/galctrl.hxx>
#include <svx/galmisc.hxx>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>

#include <string_view>
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

    SearchProgress* mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    INetURLObject               maStartURL;

    void                        ImplSearch( const INetURLObject& rStartURL,
                                            const std::vector< OUString >& rFormats,
                                            bool bRecursive );

    virtual                     ~SearchThread() override;
    virtual void                execute() override;

public:

                                SearchThread(SearchProgress* pProgress,
                                             TPGalleryThemeProperties* pBrowser,
                                             const INetURLObject& rStartURL);
};

class SearchProgress : public weld::GenericDialogController
{
private:
    INetURLObject startUrl_;
    TPGalleryThemeProperties* m_pTabPage;
    rtl::Reference< SearchThread > m_aSearchThread;
    std::unique_ptr<weld::Label> m_xFtSearchDir;
    std::unique_ptr<weld::Label> m_xFtSearchType;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DECL_LINK(ClickCancelBtn, weld::Button&, void);

public:
    SearchProgress(weld::Window* pParent, TPGalleryThemeProperties* pTabPage, const INetURLObject& rStartURL);
    void LaunchThread();
    virtual ~SearchProgress() override;

    DECL_LINK( CleanUpHdl, void*, void );

    void                SetFileType( const OUString& rType ) { m_xFtSearchType->set_label(rType); }
    void                SetDirectory( const INetURLObject& rURL ) { m_xFtSearchDir->set_label(GetReducedString(rURL, 30)); }
};

class TakeThread: public salhelper::Thread
{
private:

    TakeProgress* mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
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

class TakeProgress : public weld::GenericDialogController
{
private:
    weld::Window* m_pParent;
    TPGalleryThemeProperties* m_pTabPage;
    rtl::Reference< TakeThread > maTakeThread;
    TokenList_impl      maTakenList;
    std::unique_ptr<weld::Label> m_xFtTakeFile;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DECL_LINK(ClickCancelBtn, weld::Button&, void);

public:

    TakeProgress(weld::Window* pParent, TPGalleryThemeProperties* pTabPage);
    void LaunchThread();
    virtual ~TakeProgress() override;

    DECL_LINK( CleanUpHdl, void*, void );

    void                SetFile( const INetURLObject& rURL ) { m_xFtTakeFile->set_label(GetReducedString(rURL, 30)); }
};

class ActualizeProgress : public weld::GenericDialogController
{
private:
    Idle*               pIdle;
    GalleryTheme*       pTheme;
    GalleryProgress     aStatusProgress;
    std::unique_ptr<weld::Label> m_xFtActualizeFile;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DECL_LINK(ClickCancelBtn, weld::Button&, void);
    DECL_LINK(TimeoutHdl, Timer*, void);
    DECL_LINK(ActualizeHdl, const INetURLObject&, void);

public:
    ActualizeProgress(weld::Widget* pWindow, GalleryTheme* pThm);
    virtual ~ActualizeProgress() override;

    virtual short run() override;
};

class TitleDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
public:
    TitleDialog(weld::Widget* pParent, const OUString& rOldText);
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
    GalleryIdDialog(weld::Widget* pParent, GalleryTheme* pThm);
    virtual ~GalleryIdDialog() override;
    sal_uInt32 GetId() const { return m_xLbResName->get_active(); }
};

class GalleryThemeProperties : public SfxTabDialogController
{
    ExchangeData*   pData;

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;

public:
    GalleryThemeProperties(weld::Widget* pParent, ExchangeData* pData, SfxItemSet const * pItemSet);
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
    TPGalleryThemeGeneral(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    void                SetXChgData( ExchangeData* pData );
    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
};

class TPGalleryThemeProperties : public SfxTabPage
{
    friend class SearchThread;
    friend class TakeProgress;
    friend class TakeThread;

    ExchangeData*           pData;
    std::vector<OUString>   aFoundList;
    std::vector< std::unique_ptr<FilterEntry> >
                            aFilterEntryList;
    Timer                   aPreviewTimer;
    OUString                aLastFilterName;
    OUString                aPreviewString;
    INetURLObject           aURL;
    bool                    bEntriesFound;
    bool                    bInputAllowed;
    bool                    bTakeAll;
    bool                    bSearchRecursive;

    rtl::Reference< ::svt::DialogClosedListener >           xDialogListener;
    css::uno::Reference< css::media::XPlayer >              xMediaPlayer;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    DialogGalleryPreview m_aWndPreview;
    std::unique_ptr<weld::ComboBox> m_xCbbFileType;
    std::unique_ptr<weld::TreeView> m_xLbxFound;
    std::unique_ptr<weld::Button> m_xBtnSearch;
    std::unique_ptr<weld::Button> m_xBtnTake;
    std::unique_ptr<weld::Button> m_xBtnTakeAll;
    std::unique_ptr<weld::CheckButton> m_xCbxPreview;
    std::unique_ptr<weld::CustomWeld> m_xWndPreview;

    virtual void        Reset( const SfxItemSet* /*rSet*/ ) override {}
    virtual bool        FillItemSet( SfxItemSet* /*rSet*/ ) override { return true; }
    static OUString     addExtension( const OUString&, std::u16string_view );
    void                FillFilterList();

    void                SearchFiles();
    void                TakeFiles();
    void                DoPreview();
    void                EndSearchProgressHdl(sal_Int32 nResult);

    DECL_LINK(ClickPreviewHdl, weld::ToggleButton&, void);
    DECL_LINK(ClickSearchHdl, weld::Button&, void);
    DECL_LINK(ClickTakeHdl, weld::Button&, void);
    DECL_LINK(ClickTakeAllHdl, weld::Button&, void);
    DECL_LINK(SelectFoundHdl, weld::TreeView&, void);
    DECL_LINK(SelectFileTypeHdl, weld::ComboBox&, void);
    DECL_LINK(DClickFoundHdl, weld::TreeView&, bool);
    DECL_LINK(PreviewTimerHdl, Timer*, void);
    DECL_LINK(DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void);

public:
    TPGalleryThemeProperties(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~TPGalleryThemeProperties() override;

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    void                StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult );

    static std::unique_ptr<SfxTabPage>  Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
