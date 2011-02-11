/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVTOOLS_TEMPLWIN_HXX
#define _SVTOOLS_TEMPLWIN_HXX

#include <tools/resary.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/window.hxx>
#include <svtools/headbar.hxx>
#include <svtools/fileview.hxx>
#include <svtools/ivctrl.hxx>
#include <svtools/svmedit2.hxx>
#include <svl/restrictedpaths.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/Locale.hpp>

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } }
namespace com{ namespace sun { namespace star { namespace frame { class XFrame; } } } }
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
                        ~SvtDummyHeaderBar_Impl();

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
};

// class SvtIconWindow_Impl ----------------------------------------------

class SvtIconWindow_Impl : public Window
{
private:
    SvtDummyHeaderBar_Impl  aDummyHeaderBar;    // spaceholder instead of HeaderBar
    SvtIconChoiceCtrl   aIconCtrl;

    String              aNewDocumentRootURL;
    String              aTemplateRootURL;
    String              aMyDocumentsRootURL;
    String              aSamplesFolderRootURL;

    long                nMaxTextLength;

    SvxIconChoiceCtrlEntry* GetEntry( const String& rURL ) const;

public:
    SvtIconWindow_Impl( Window* pParent );
    ~SvtIconWindow_Impl();

    virtual void        Resize();

    inline long         GetMaxTextLength() const { return nMaxTextLength; }
    inline void         SetClickHdl( const Link& rLink ) { aIconCtrl.SetClickHdl( rLink ); }

    String              GetSelectedIconURL() const;
    String              GetSelectedIconText() const;
    String              GetCursorPosIconURL() const;
    String              GetIconText( const String& rURL ) const;
    void                InvalidateIconControl();
    void                SetCursorPos( sal_uLong nPos );
    sal_uLong               GetCursorPos() const;
    sal_uLong               GetSelectEntryPos() const;
    void                SetFocus();
    long                CalcHeight() const;
    sal_Bool            IsRootURL( const String& rURL ) const;
    sal_uLong               GetRootPos( const String& rURL ) const;
    void                UpdateIcons( sal_Bool _bHiContrast );

    inline sal_Bool         ProcessKeyEvent( const KeyEvent& rKEvt );

    inline const String&    GetTemplateRootURL() const      { return aTemplateRootURL; }
    inline const String&    GetMyDocumentsRootURL() const   { return aMyDocumentsRootURL; }
    inline const String&    GetSamplesFolderURL() const     { return aSamplesFolderRootURL; }

    void                SelectFolder(sal_Int32 nFolderPos);
};

inline sal_Bool SvtIconWindow_Impl::ProcessKeyEvent( const KeyEvent& rKEvt )
{
    return ( rKEvt.GetKeyCode().IsMod2() ? aIconCtrl.DoKeyInput( rKEvt ) : sal_False );
}

// class SvtFileViewWindow_Impl ------------------------------------------

class SvtTemplateWindow;

class SvtFileViewWindow_Impl : public Window
{
private:
    SvtTemplateWindow&  rParent;
    SvtFileView         aFileView;
    Link                aNewFolderLink;
    String              aCurrentRootURL;
    String              aFolderURL;
    String              aMyDocumentsURL;
    String              aSamplesFolderURL;
    ::svt::RestrictedPaths
                        aURLFilter;

    sal_Bool            bIsTemplateFolder;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        GetNewDocContents() const;

public:
    SvtFileViewWindow_Impl( SvtTemplateWindow* pParent );
    ~SvtFileViewWindow_Impl();

    virtual void        Resize();

    inline void         SetSelectHdl( const Link& rLink ) { aFileView.SetSelectHdl( rLink ); }
    inline void         SetDoubleClickHdl( const Link& rLink ) { aFileView.SetDoubleClickHdl( rLink ); }
    inline void         SetNewFolderHdl( const Link& rLink ) { aNewFolderLink = rLink; }
    inline void         ResetCursor() { aFileView.ResetCursor(); }
    inline sal_Bool     IsTemplateFolder() const { return bIsTemplateFolder; }
    inline String       GetFolderURL() const { return aFolderURL; }
    inline String       GetRootURL() const { return aCurrentRootURL; }
    inline void         OpenRoot( const String& rRootURL )
                            { aCurrentRootURL = rRootURL; OpenFolder( rRootURL ); }
    inline void         SetMyDocumentsURL( const String& _rNewURL ) { aMyDocumentsURL = _rNewURL; }
    inline void         SetSamplesFolderURL( const String& _rNewURL ) { aSamplesFolderURL = _rNewURL; }

    String              GetSelectedFile() const;
    void                OpenFolder( const String& rURL );
    sal_Bool            HasPreviousLevel( String& rURL ) const;
    String              GetFolderTitle() const;
    void                SetFocus();
};

// class SvtFrameWindow_Impl ---------------------------------------------

class SvtDocInfoTable_Impl : public ResStringArray
{
private:
    String          aEmptyString;

public:
    SvtDocInfoTable_Impl();

    const String&   GetString( long nId ) const;
};

class SvtExtendedMultiLineEdit_Impl : public ExtMultiLineEdit
{
public:
    SvtExtendedMultiLineEdit_Impl( Window* pParent,WinBits _nBits );
    inline ~SvtExtendedMultiLineEdit_Impl() {}

    inline void         Clear() { SetText( String() ); }
    void                InsertEntry( const String& rTitle, const String& rValue );
};

class SvtFrameWindow_Impl : public Window
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                                xFrame;
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
    String                          aCurrentURL;
    ::rtl::OUString                 m_aOpenURL;
    sal_Bool                        bDocInfo;

    void                    ShowDocInfo( const String& rURL );
    void                    ViewEditWin();
    void                    ViewTextWin();
    void                    ViewEmptyWin();
    void                    ViewNonEmptyWin();  // views depending on bDocInfo

    struct SvtExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
    };

    DECL_STATIC_LINK(       SvtFrameWindow_Impl, ExecuteHdl_Impl, SvtExecuteInfo* );

public:
    SvtFrameWindow_Impl( Window* pParent );
    ~SvtFrameWindow_Impl();

    virtual void            Resize();

    void                    OpenFile( const String& rURL, sal_Bool bPreview, sal_Bool bIsTemplate, sal_Bool bAsTemplate );
    void                    ToggleView( sal_Bool bDocInfo );
};

// class SvtTemplateWindow -----------------------------------------------

class HistoryList_Impl;

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

    String                      aFolderTitle;

    virtual void        Resize();

    DECL_LINK(          IconClickHdl_Impl, SvtIconChoiceCtrl* );
    DECL_LINK(          FileSelectHdl_Impl, SvtFileView* );
    DECL_LINK(          FileDblClickHdl_Impl, SvtFileView* );
    DECL_LINK(          NewFolderHdl_Impl, SvtFileView* );
    DECL_LINK(          TimeoutHdl_Impl, Timer* );
    DECL_LINK(          ClickHdl_Impl, ToolBox* );
    DECL_LINK(          ResizeHdl_Impl, SplitWindow* );     // used for split and initial setting of toolbar pos

    void                PrintFile( const String& rURL );
    void                AppendHistoryURL( const String& rURL, sal_uLong nGroup );
    void                OpenHistory();
    void                DoAction( sal_uInt16 nAction );
    void                InitToolBoxes();
    void                InitToolBoxImages();
    void                UpdateIcons();

protected:
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvtTemplateWindow( Window* pParent );
    ~SvtTemplateWindow();

    inline void         SetSelectHdl( const Link& rLink ) { aSelectHdl = rLink; }
    inline void         SetDoubleClickHdl( const Link& rLink ) { aDoubleClickHdl = rLink; }
    inline void         SetNewFolderHdl( const Link& rLink ) { aNewFolderHdl = rLink; }
    inline void         SetSendFocusHdl( const Link& rLink ) { aSendFocusHdl = rLink; }
    inline sal_Bool     IsTemplateFolderOpen() const { return pFileWin->IsTemplateFolder(); }
    inline sal_Bool     HasIconWinFocus() const { return pIconWin->HasChildPathFocus(); }

    void                ReadViewSettings( );
    void                WriteViewSettings( );
    sal_Bool            IsFileSelected() const;
    String              GetSelectedFile() const;
    void                OpenFile( sal_Bool bNotAsTemplate );
    String              GetFolderTitle() const;
    String              GetFolderURL() const;
    void                SetFocus( sal_Bool bIconWin );
    void                OpenTemplateRoot();
    void                SetPrevLevelButtonState( const String& rURL );  // sets state (enable/disable) for previous level button
    void                ClearHistory();
    long                CalcHeight() const;

    void                SelectFolder(sal_Int32 nFolderPosition);
};

#endif // _SVTOOLS_TEMPLWIN_HXX

