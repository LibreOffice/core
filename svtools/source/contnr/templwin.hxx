/*************************************************************************
 *
 *  $RCSfile: templwin.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:36:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVTOOLS_TEMPLWIN_HXX
#define _SVTOOLS_TEMPLWIN_HXX

#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SV_SPLITWIN_HXX
#include <vcl/splitwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _HEADBAR_HXX
#include "headbar.hxx"
#endif
#ifndef _SVT_FILEVIEW_HXX
#include "fileview.hxx"
#endif
#ifndef _ICNVW_HXX
#include "ivctrl.hxx"
#endif
#ifndef _SVTOOLS_SVMEDIT2_HXX
#include "svmedit2.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } };
namespace com{ namespace sun { namespace star { namespace frame { class XFrame; } } } };
namespace com{ namespace sun { namespace star { namespace io    { class XPersist; } } } };
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
    inline String       GetTemplateRootURL() const { return aTemplateRootURL; }

    String              GetSelectedIconURL() const;
    String              GetSelectedIconText() const;
    String              GetCursorPosIconURL() const;
    String              GetIconText( const String& rURL ) const;
    void                InvalidateIconControl();
    void                SetCursorPos( ULONG nPos );
    ULONG               GetCursorPos() const;
    ULONG               GetSelectEntryPos() const;
    void                SetFocus();
    long                CalcHeight() const;
    sal_Bool            IsRootURL( const String& rURL ) const;
    ULONG               GetRootPos( const String& rURL ) const;
    void                UpdateIcons( sal_Bool _bHiContrast );

    inline sal_Bool         ProcessKeyEvent( const KeyEvent& rKEvt );
    inline const String&    GetSamplesFolderURL() const;

    void                SelectFolder(sal_Int32 nFolderPos);
};

inline sal_Bool SvtIconWindow_Impl::ProcessKeyEvent( const KeyEvent& rKEvt )
{
    return ( rKEvt.GetKeyCode().IsMod2() ? aIconCtrl.DoKeyInput( rKEvt ) : sal_False );
}

inline const String& SvtIconWindow_Impl::GetSamplesFolderURL() const
{
    return aSamplesFolderRootURL;
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
    String              aSamplesFolderURL;

    sal_Bool            bIsTemplateFolder;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        GetNewDocContents() const;

public:
    SvtFileViewWindow_Impl( SvtTemplateWindow* pParent, const String& rSamplesFolderURL );
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
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XPersist >
                                xDocInfo;
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
    void                AppendHistoryURL( const String& rURL, ULONG nGroup );
    void                OpenHistory();
    void                DoAction( USHORT nAction );
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
    void                SetFocus( sal_Bool bIconWin );
    void                OpenTemplateRoot();
    void                SetPrevLevelButtonState( const String& rURL );  // sets state (enable/disable) for previous level button
    void                ClearHistory();
    long                CalcHeight() const;

    void                SelectFolder(sal_Int32 nFolderPosition);
};

#endif // _SVTOOLS_TEMPLWIN_HXX

