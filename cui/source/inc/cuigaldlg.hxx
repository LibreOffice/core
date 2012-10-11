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

#ifndef _CUI_GALDLG_HXX_
#define _CUI_GALDLG_HXX_

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

// ------------
// - Forwards -
// ------------

class GalleryTheme;
class SearchProgress;
class TakeProgress;
class TPGalleryThemeProperties;

typedef ::std::vector< UniString* > StringList;
typedef ::std::vector< sal_uLong > TokenList_impl;

// ---------------
// - FilterEntry -
// ---------------

struct FilterEntry
{
    String  aFilterName;
};

// ----------------
// - SearchThread -
// ----------------

class SearchThread: public salhelper::Thread
{
private:

    SearchProgress*             mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    INetURLObject               maStartURL;

    void                        ImplSearch( const INetURLObject& rStartURL,
                                            const ::std::vector< String >& rFormats,
                                            sal_Bool bRecursive );

    virtual                     ~SearchThread();
    virtual void                execute();

public:

                                SearchThread( SearchProgress* pProgess,
                                              TPGalleryThemeProperties* pBrowser,
                                              const INetURLObject& rStartURL );
};

// ------------------
// - SearchProgress -
// ------------------

class SearchProgress : public ModalDialog
{
private:

    FixedText           aFtSearchDir;
    FixedLine           aFLSearchDir;
    FixedText           aFtSearchType;
    FixedLine           aFLSearchType;
    CancelButton        aBtnCancel;
    Window * parent_;
    INetURLObject startUrl_;
    rtl::Reference< SearchThread > maSearchThread;

                        DECL_LINK( ClickCancelBtn, void* );
    void                Terminate();

public:
                        SearchProgress( Window* pParent, const INetURLObject& rStartURL );
                        ~SearchProgress() {};

                        DECL_LINK( CleanUpHdl, void* );

    virtual short       Execute();
    virtual void        StartExecuteModal( const Link& rEndDialogHdl );
    void                SetFileType( const String& rType ) { aFtSearchType.SetText( rType ); }
    void                SetDirectory( const INetURLObject& rURL ) { aFtSearchDir.SetText( GetReducedString( rURL, 30 ) ); }
};

// --------------
// - TakeThread -
// --------------

class TakeThread: public salhelper::Thread
{
private:

    TakeProgress*               mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    TokenList_impl&             mrTakenList;

    virtual                     ~TakeThread();
    virtual void                execute();

public:

                                TakeThread(
                                    TakeProgress* pProgess,
                                    TPGalleryThemeProperties* pBrowser,
                                    TokenList_impl& rTakenList
                                );
};

// ----------------
// - TakeProgress -
// ----------------

class TakeProgress : public ModalDialog
{
private:

    FixedText           aFtTakeFile;
    FixedLine           aFLTakeProgress;
    CancelButton        aBtnCancel;
    Window * window_;
    rtl::Reference< TakeThread > maTakeThread;
    TokenList_impl      maTakenList;

                        DECL_LINK( ClickCancelBtn, void* );
    void                Terminate();

public:

                        TakeProgress( Window* pWindow );
                        ~TakeProgress() {};

                        DECL_LINK( CleanUpHdl, void* );

    void                SetFile( const INetURLObject& rURL ) { aFtTakeFile.SetText( GetReducedString( rURL, 30 ) ); }
    virtual short       Execute();
    virtual void        StartExecuteModal( const Link& rEndDialogHdl );
};

// ---------------------
// - ActualizeProgress -
// ---------------------

class ActualizeProgress : public ModalDialog
{
private:

    FixedText           aFtActualizeFile;
    FixedLine           aFLActualizeProgress;
    CancelButton        aBtnCancel;
    Timer*              pTimer;
    GalleryTheme*       pTheme;
    GalleryProgress     aStatusProgress;

                        DECL_LINK( ClickCancelBtn, void* );
                        DECL_LINK( TimeoutHdl, Timer* );
                        DECL_LINK( ActualizeHdl, INetURLObject* pURL );

public:
                        ActualizeProgress( Window* pWindow, GalleryTheme* pThm );
                        ~ActualizeProgress() {};

    virtual short       Execute();
};

// ---------------
// - TitleDialog -
// ---------------

class TitleDialog : public ModalDialog
{
private:

    OKButton            maOk;
    CancelButton        maCancel;
    HelpButton          maHelp;
    FixedLine           maFL;
    Edit                maEdit;

public:

                        TitleDialog( Window* pParent, const String& rOldText );
    String              GetTitle() const { return maEdit.GetText(); }
};

// -------------------
// - GalleryIdDialog -
// -------------------

class GalleryIdDialog : public ModalDialog
{
private:

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    FixedLine       aFLId;
    ListBox         aLbResName;
    GalleryTheme*   pThm;

                    DECL_LINK( ClickOkHdl, void* );
                    DECL_LINK( ClickResNameHdl, void* );

public:

                    GalleryIdDialog( Window* pParent, GalleryTheme* pThm );
                    ~GalleryIdDialog() {}

    sal_uLong           GetId() const { return aLbResName.GetSelectEntryPos(); }
};

// --------------------------
// - GalleryThemeProperties -
// --------------------------

class GalleryThemeProperties : public SfxTabDialog
{
    ExchangeData*   pData;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

                    GalleryThemeProperties( Window* pParent, ExchangeData* pData, SfxItemSet* pItemSet  );
                    ~GalleryThemeProperties() {}
};

// -------------------------
// - TPGalleryThemeGeneral -
// -------------------------

class TPGalleryThemeGeneral : public SfxTabPage
{
private:

    FixedImage          aFiMSImage;
    Edit                aEdtMSName;
    FixedLine           aFlMSGeneralFirst;
    FixedText           aFtMSType;
    FixedText           aFtMSShowType;
    FixedText           aFtMSPath;
    FixedText           aFtMSShowPath;
    FixedText           aFtMSContent;
    FixedText           aFtMSShowContent;
    FixedLine           aFlMSGeneralSecond;
    FixedText           aFtMSChangeDate;
    FixedText           aFtMSShowChangeDate;
    ExchangeData*       pData;

    virtual void        Reset( const SfxItemSet& ) {}
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );


public:

                        TPGalleryThemeGeneral( Window* pParent, const SfxItemSet& rSet );
                        ~TPGalleryThemeGeneral() {}

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

// ----------------------------
// - TPGalleryThemeProperties -
// ----------------------------
typedef ::std::vector< FilterEntry* > FilterEntryList_impl;

class TPGalleryThemeProperties : public SfxTabPage
{
    friend class SearchThread;
    friend class TakeProgress;
    friend class TakeThread;

    FixedText           aFtFileType;
    ComboBox            aCbbFileType;
    MultiListBox        aLbxFound;
    PushButton          aBtnSearch;
    PushButton          aBtnTake;
    PushButton          aBtnTakeAll;
    CheckBox            aCbxPreview;
    GalleryPreview      aWndPreview;

    ExchangeData*           pData;
    StringList              aFoundList;
    FilterEntryList_impl    aFilterEntryList;
    Timer                   aPreviewTimer;
    String                  aLastFilterName;
    String                  aPreviewString;
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

    virtual void        Reset( const SfxItemSet& /*rSet*/ ) {}
    virtual sal_Bool        FillItemSet( SfxItemSet& /*rSet*/ ) { return sal_True; }
    ::rtl::OUString     addExtension( const ::rtl::OUString&, const ::rtl::OUString& );
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
                        ~TPGalleryThemeProperties();

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    void                StartSearchFiles( const String& _rFolderURL, short _nDlgResult );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

#endif // _CUI_GALDLG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
