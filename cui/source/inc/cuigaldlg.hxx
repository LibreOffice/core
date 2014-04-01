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

#include "sal/config.h"

#include <salhelper/thread.hxx>
#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svl/slstitm.hxx>
#include <svtools/transfer.hxx>
#include <svtools/grfmgr.hxx>
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

typedef ::std::vector< OUString > StringList;
typedef ::std::vector< sal_uLong > TokenList_impl;

struct FilterEntry
{
    OUString  aFilterName;
};

class SearchThread: public salhelper::Thread
{
private:

    SearchProgress*             mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    INetURLObject               maStartURL;

    void                        ImplSearch( const INetURLObject& rStartURL,
                                            const ::std::vector< OUString >& rFormats,
                                            sal_Bool bRecursive );

    virtual                     ~SearchThread();
    virtual void                execute() SAL_OVERRIDE;

public:

                                SearchThread( SearchProgress* pProgess,
                                              TPGalleryThemeProperties* pBrowser,
                                              const INetURLObject& rStartURL );
};

class SearchProgress : public ModalDialog
{
private:
    FixedText*          m_pFtSearchDir;
    FixedText*          m_pFtSearchType;
    CancelButton*       m_pBtnCancel;
    Window * parent_;
    INetURLObject startUrl_;
    rtl::Reference< SearchThread > maSearchThread;

                        DECL_LINK( ClickCancelBtn, void* );
    void                Terminate();

public:
                        SearchProgress( Window* pParent, const INetURLObject& rStartURL );
                        virtual ~SearchProgress() {};

                        DECL_LINK( CleanUpHdl, void* );

    virtual short       Execute() SAL_OVERRIDE;
    virtual void        StartExecuteModal( const Link& rEndDialogHdl ) SAL_OVERRIDE;
    void                SetFileType( const OUString& rType ) { m_pFtSearchType->SetText( rType ); }
    void                SetDirectory( const INetURLObject& rURL ) { m_pFtSearchDir->SetText( GetReducedString( rURL, 30 ) ); }
};

class TakeThread: public salhelper::Thread
{
private:

    TakeProgress*               mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    TokenList_impl&             mrTakenList;

    virtual                     ~TakeThread();
    virtual void                execute() SAL_OVERRIDE;

public:

                                TakeThread(
                                    TakeProgress* pProgess,
                                    TPGalleryThemeProperties* pBrowser,
                                    TokenList_impl& rTakenList
                                );
};

class TakeProgress : public ModalDialog
{
private:
    FixedText*          m_pFtTakeFile;
    CancelButton*       m_pBtnCancel;
    Window * window_;
    rtl::Reference< TakeThread > maTakeThread;
    TokenList_impl      maTakenList;

    DECL_LINK( ClickCancelBtn, void* );
    void                Terminate();

public:

    TakeProgress( Window* pWindow );

    DECL_LINK( CleanUpHdl, void* );

    void                SetFile( const INetURLObject& rURL ) { m_pFtTakeFile->SetText( GetReducedString( rURL, 30 ) ); }
    virtual short       Execute() SAL_OVERRIDE;
    virtual void        StartExecuteModal( const Link& rEndDialogHdl ) SAL_OVERRIDE;
};

class ActualizeProgress : public ModalDialog
{
private:
    FixedText*          m_pFtActualizeFile;
    CancelButton*       m_pBtnCancel;
    Timer*              pTimer;
    GalleryTheme*       pTheme;
    GalleryProgress     aStatusProgress;

                        DECL_LINK( ClickCancelBtn, void* );
                        DECL_LINK( TimeoutHdl, Timer* );
                        DECL_LINK( ActualizeHdl, INetURLObject* pURL );

public:
                        ActualizeProgress( Window* pWindow, GalleryTheme* pThm );
                        virtual ~ActualizeProgress() {};

    virtual short       Execute() SAL_OVERRIDE;
};

class TitleDialog : public ModalDialog
{
private:
    Edit* m_pEdit;
public:
    TitleDialog(Window* pParent, const OUString& rOldText);
    OUString GetTitle() const { return m_pEdit->GetText(); }
};

class GalleryIdDialog : public ModalDialog
{
private:
    OKButton* m_pBtnOk;
    ListBox* m_pLbResName;
    GalleryTheme*   pThm;

    DECL_LINK( ClickOkHdl, void* );
    DECL_LINK( ClickResNameHdl, void* );
public:
    GalleryIdDialog( Window* pParent, GalleryTheme* pThm );
    sal_uLong GetId() const { return m_pLbResName->GetSelectEntryPos(); }
};

class GalleryThemeProperties : public SfxTabDialog
{
    ExchangeData*   pData;

    sal_uInt16 m_nGeneralPageId;
    sal_uInt16 m_nFilesPageId;

    virtual void PageCreated(sal_uInt16 nId, SfxTabPage &rPage) SAL_OVERRIDE;

public:
    GalleryThemeProperties(Window* pParent, ExchangeData* pData, SfxItemSet* pItemSet);
};

class TPGalleryThemeGeneral : public SfxTabPage
{
private:

    FixedImage*         m_pFiMSImage;
    Edit*               m_pEdtMSName;
    FixedText*          m_pFtMSShowType;
    FixedText*          m_pFtMSShowPath;
    FixedText*          m_pFtMSShowContent;
    FixedText*          m_pFtMSShowChangeDate;
    ExchangeData*       pData;

    virtual void        Reset( const SfxItemSet& ) SAL_OVERRIDE {}
    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;


public:

                        TPGalleryThemeGeneral( Window* pParent, const SfxItemSet& rSet );
                        virtual ~TPGalleryThemeGeneral() {}

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

typedef ::std::vector< FilterEntry* > FilterEntryList_impl;

class TPGalleryThemeProperties : public SfxTabPage
{
    friend class SearchThread;
    friend class TakeProgress;
    friend class TakeThread;

    ComboBox*           m_pCbbFileType;
    ListBox*            m_pLbxFound;
    PushButton*         m_pBtnSearch;
    PushButton*         m_pBtnTake;
    PushButton*         m_pBtnTakeAll;
    CheckBox*           m_pCbxPreview;
    GalleryPreview*     m_pWndPreview;

    ExchangeData*           pData;
    StringList              aFoundList;
    FilterEntryList_impl    aFilterEntryList;
    Timer                   aPreviewTimer;
    OUString                aLastFilterName;
    OUString                aPreviewString;
    INetURLObject           aURL;
    sal_uInt16              nCurFilterPos;
    sal_uInt16              nFirstExtFilterPos;
    sal_Bool                bEntriesFound;
    sal_Bool                bInputAllowed;
    sal_Bool                bTakeAll;
    sal_Bool                bSearchRecursive;

    ::com::sun::star::uno::Reference< ::svt::DialogClosedListener >                  xDialogListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >             xMediaPlayer;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker2 > xFolderPicker;

    virtual void        Reset( const SfxItemSet& /*rSet*/ ) SAL_OVERRIDE {}
    virtual bool        FillItemSet( SfxItemSet& /*rSet*/ ) SAL_OVERRIDE { return true; }
    OUString     addExtension( const OUString&, const OUString& );
    void                FillFilterList();

    void                SearchFiles();
    void                TakeFiles();
    void                DoPreview();

                        DECL_LINK( ClickPreviewHdl, void* );
                        DECL_LINK( ClickSearchHdl, void* );
                        DECL_LINK( ClickTakeHdl, void* );
                        DECL_LINK( ClickTakeAllHdl, void* );
                        DECL_LINK( SelectFoundHdl, void* );
                        DECL_LINK( SelectThemeHdl, void* );
                        DECL_LINK( SelectFileTypeHdl, void* );
                        DECL_LINK( DClickFoundHdl, void* );
                        DECL_LINK( PreviewTimerHdl, void* );
                        DECL_LINK(EndSearchProgressHdl, void *);
                        DECL_LINK( DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );

public:
                        TPGalleryThemeProperties( Window* pWindow, const SfxItemSet& rSet );
                        virtual ~TPGalleryThemeProperties();

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    void                StartSearchFiles( const OUString& _rFolderURL, short _nDlgResult );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

#endif // INCLUDED_CUI_SOURCE_INC_CUIGALDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
