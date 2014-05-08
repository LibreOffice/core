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
#ifndef INCLUDED_SVTOOLS_SOURCE_CONTNR_TEMPLWIN_HXX
#define INCLUDED_SVTOOLS_SOURCE_CONTNR_TEMPLWIN_HXX

#include <tools/resary.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/window.hxx>
#include <svtools/headbar.hxx>
#include <svtools/fileview.hxx>
#include <svtools/ivctrl.hxx>
#include <svtools/svmedit2.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/Locale.hpp>

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } }
namespace com{ namespace sun { namespace star { namespace frame { class XFrame2; } } } }
namespace com{ namespace sun { namespace star { namespace document {
    class XDocumentProperties;
} } } }
namespace svtools
{
    class ODocumentInfoPreview;
}

// class SvtDummyHeaderBar_Impl ------------------------------------------

class SvtDummyHeaderBar_Impl : public Window
{
private:
    void                UpdateBackgroundColor();

public:
                        SvtDummyHeaderBar_Impl( Window* pParent );
                        virtual ~SvtDummyHeaderBar_Impl();

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};

// class SvtIconWindow_Impl ----------------------------------------------

class SvtIconWindow_Impl : public Window
{
private:
    SvtDummyHeaderBar_Impl  aDummyHeaderBar;    // spaceholder instead of HeaderBar
    SvtIconChoiceCtrl   aIconCtrl;

    OUString            aNewDocumentRootURL;
    OUString            aTemplateRootURL;
    OUString            aMyDocumentsRootURL;
    OUString            aSamplesFolderRootURL;

    long                nMaxTextLength;

    SvxIconChoiceCtrlEntry* GetEntry( const OUString& rURL ) const;

public:
    SvtIconWindow_Impl( Window* pParent );
    virtual ~SvtIconWindow_Impl();

    virtual void        Resize() SAL_OVERRIDE;

    inline long         GetMaxTextLength() const { return nMaxTextLength; }
    inline void         SetClickHdl( const Link& rLink ) { aIconCtrl.SetClickHdl( rLink ); }

    OUString            GetSelectedIconURL() const;
    OUString            GetCursorPosIconURL() const;
    OUString            GetIconText( const OUString& rURL ) const;
    void                InvalidateIconControl();
    void                SetCursorPos( sal_uLong nPos );
    sal_uLong           GetCursorPos() const;
    sal_uLong           GetSelectEntryPos() const;
    void                SetFocus();
    long                CalcHeight() const;
    bool                IsRootURL( const OUString& rURL ) const;
    sal_uLong           GetRootPos( const OUString& rURL ) const;
    void                UpdateIcons();

    inline bool         ProcessKeyEvent( const KeyEvent& rKEvt );

    inline const OUString&    GetTemplateRootURL() const      { return aTemplateRootURL; }
    inline const OUString&    GetMyDocumentsRootURL() const   { return aMyDocumentsRootURL; }
    inline const OUString&    GetSamplesFolderURL() const     { return aSamplesFolderRootURL; }

    void                SelectFolder(sal_Int32 nFolderPos);
};

inline bool SvtIconWindow_Impl::ProcessKeyEvent( const KeyEvent& rKEvt )
{
    return rKEvt.GetKeyCode().IsMod2() && aIconCtrl.DoKeyInput( rKEvt );
}

// class SvtFileViewWindow_Impl ------------------------------------------

class SvtTemplateWindow;

class SvtFileViewWindow_Impl : public Window
{
private:
    SvtTemplateWindow&  rParent;
    SvtFileView         aFileView;
    Link                aNewFolderLink;
    OUString            aCurrentRootURL;
    OUString            aFolderURL;
    OUString            aMyDocumentsURL;
    OUString            aSamplesFolderURL;

    bool                bIsTemplateFolder;

    ::com::sun::star::uno::Sequence< OUString >
                        GetNewDocContents() const;

public:
    SvtFileViewWindow_Impl( SvtTemplateWindow* pParent );
    virtual ~SvtFileViewWindow_Impl();

    virtual void        Resize() SAL_OVERRIDE;

    inline void         SetSelectHdl( const Link& rLink ) { aFileView.SetSelectHdl( rLink ); }
    inline void         SetDoubleClickHdl( const Link& rLink ) { aFileView.SetDoubleClickHdl( rLink ); }
    inline void         SetNewFolderHdl( const Link& rLink ) { aNewFolderLink = rLink; }
    inline bool         IsTemplateFolder() const { return bIsTemplateFolder; }
    inline OUString     GetFolderURL() const { return aFolderURL; }
    inline OUString     GetRootURL() const { return aCurrentRootURL; }
    inline void         OpenRoot( const OUString& rRootURL )
                            { aCurrentRootURL = rRootURL; OpenFolder( rRootURL ); }
    inline void         SetMyDocumentsURL( const OUString& _rNewURL ) { aMyDocumentsURL = _rNewURL; }
    inline void         SetSamplesFolderURL( const OUString& _rNewURL ) { aSamplesFolderURL = _rNewURL; }

    OUString            GetSelectedFile() const;
    void                OpenFolder( const OUString& rURL );
    bool                HasPreviousLevel( OUString& rURL ) const;
    OUString            GetFolderTitle() const;
    void                SetFocus();
};

// class SvtFrameWindow_Impl ---------------------------------------------

class SvtDocInfoTable_Impl : public ResStringArray
{
public:
    SvtDocInfoTable_Impl();

    OUString GetString( long nId ) const;
};

class SvtFrameWindow_Impl : public Window
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 >
                                m_xFrame;
    ::com::sun::star::uno::Reference < ::com::sun::star::document::XDocumentProperties>
                                m_xDocProps;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                                xWindow;

    ::svtools::ODocumentInfoPreview*
                                    pEditWin;
    Window*                         pTextWin;
    Window*                         pEmptyWin;
    ::com::sun::star::lang::Locale  aLocale;
    SvtDocInfoTable_Impl            aInfoTable;
    OUString                        aCurrentURL;
    OUString                        m_aOpenURL;
    bool                            bDocInfo;

    void                    ShowDocInfo( const OUString& rURL );
    void                    ViewEditWin();
    void                    ViewTextWin();
    void                    ViewEmptyWin();
    void                    ViewNonEmptyWin();  // views depending on bDocInfo

    struct SvtExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
    };

public:
    SvtFrameWindow_Impl( Window* pParent );
    virtual ~SvtFrameWindow_Impl();

    virtual void            Resize() SAL_OVERRIDE;

    void                    OpenFile( const OUString& rURL, bool bPreview, bool bIsTemplate, bool bAsTemplate );
    void                    ToggleView( bool bDocInfo );
};

// class SvtTemplateWindow -----------------------------------------------

struct FolderHistory;
typedef ::std::vector< FolderHistory* > HistoryList_Impl;

class SvtTemplateWindow : public Window
{
private:
    ToolBox                     aFileViewTB;
    ToolBox                     aFrameWinTB;
    SplitWindow                 aSplitWin;

    SvtIconWindow_Impl*         pIconWin;
    SvtFileViewWindow_Impl*     pFileWin;
    SvtFrameWindow_Impl*        pFrameWin;
    HistoryList_Impl*           pHistoryList;

    Link                        aSelectHdl;
    Link                        aDoubleClickHdl;
    Link                        aNewFolderHdl;
    Link                        aSendFocusHdl;

    Timer                       aSelectTimer;

    OUString                    aFolderTitle;

    virtual void        Resize() SAL_OVERRIDE;

    DECL_LINK(IconClickHdl_Impl, void *);
    DECL_LINK(FileSelectHdl_Impl, void *);
    DECL_LINK(FileDblClickHdl_Impl, void *);
    DECL_LINK(NewFolderHdl_Impl, void *);
    DECL_LINK(TimeoutHdl_Impl, void *);
    DECL_LINK(          ClickHdl_Impl, ToolBox* );
    DECL_LINK(ResizeHdl_Impl, void *);     // used for split and initial setting of toolbar pos

    void                PrintFile( const OUString& rURL );
    void                AppendHistoryURL( const OUString& rURL, sal_uLong nGroup );
    void                OpenHistory();
    void                DoAction( sal_uInt16 nAction );
    void                InitToolBoxes();
    void                InitToolBoxImages();
    void                UpdateIcons();

protected:
    virtual bool        PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
    SvtTemplateWindow( Window* pParent );
    virtual ~SvtTemplateWindow();

    inline void         SetSelectHdl( const Link& rLink ) { aSelectHdl = rLink; }
    inline void         SetDoubleClickHdl( const Link& rLink ) { aDoubleClickHdl = rLink; }
    inline void         SetNewFolderHdl( const Link& rLink ) { aNewFolderHdl = rLink; }
    inline void         SetSendFocusHdl( const Link& rLink ) { aSendFocusHdl = rLink; }
    inline bool         IsTemplateFolderOpen() const { return pFileWin->IsTemplateFolder(); }
    inline bool         HasIconWinFocus() const { return pIconWin->HasChildPathFocus(); }

    void                ReadViewSettings( );
    void                WriteViewSettings( );
    bool                IsFileSelected() const;
    OUString            GetSelectedFile() const;
    void                OpenFile( bool bNotAsTemplate );
    OUString            GetFolderTitle() const;
    OUString            GetFolderURL() const;
    void                SetFocus( bool bIconWin );
    void                OpenTemplateRoot();
    void                SetPrevLevelButtonState( const OUString& rURL );  // sets state (enable/disable) for previous level button
    void                ClearHistory();
    long                CalcHeight() const;

    void                SelectFolder(sal_Int32 nFolderPosition);
};

#endif // INCLUDED_SVTOOLS_SOURCE_CONTNR_TEMPLWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
