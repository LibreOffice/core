/*************************************************************************
 *
 *  $RCSfile: templwin.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-07 14:36:36 $
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

#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/resary.hxx>

#include "ivctrl.hxx"
#include "fileview.hxx"
#include "headbar.hxx"
#include "svmedit2.hxx"

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } };
namespace com{ namespace sun { namespace star { namespace frame { class XFrame; } } } };
namespace com{ namespace sun { namespace star { namespace io    { class XPersist; } } } };

// class SvtIconWindow_Impl ----------------------------------------------

class SvtIconWindow_Impl : public Window
{
private:
    HeaderBar           aHeaderBar;
    SvtIconChoiceCtrl   aIconCtrl;

    String              aTemplateRootURL;
    long                nMaxTextLength;

    SvxIconChoiceCtrlEntry* GetEntry( const String& rURL ) const;

public:
    SvtIconWindow_Impl( Window* pParent );
    ~SvtIconWindow_Impl();

    virtual void        Resize();

    long                GetMaxTextLength() const { return nMaxTextLength; }
    void                SetClickHdl( const Link& rLink ) { aIconCtrl.SetClickHdl( rLink ); }

    String              GetSelectedIconURL() const;
    String              GetSelectedIconText() const;
    String              GetIconText( const String& rURL ) const;
    String              GetTemplateRootURL() const { return aTemplateRootURL; }
    void                InvalidateIconControl();
    void                SetCursorPos( ULONG nPos );
    void                SetFocus();
};

// class SvtFileViewWindow_Impl -----------------------------------------_

class SvtFileViewWindow_Impl : public Window
{
private:
    SvtFileView         aFileView;
    Link                aNewFolderLink;
    String              aCurrentRootURL;
    String              aFolderURL;

    sal_Bool            bIsTemplateFolder;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        GetNewDocContents() const;

public:
    SvtFileViewWindow_Impl( Window* pParent );
    ~SvtFileViewWindow_Impl();

    virtual void        Resize();

    void                SetSelectHdl( const Link& rLink ) { aFileView.SetSelectHdl( rLink ); }
    void                SetDoubleClickHdl( const Link& rLink ) { aFileView.SetDoubleClickHdl( rLink ); }
    void                SetNewFolderHdl( const Link& rLink ) { aNewFolderLink = rLink; }
    void                ResetCursor() { aFileView.ResetCursor(); }
    sal_Bool            IsTemplateFolder() const { return bIsTemplateFolder; }

    String              GetSelectedFile() const;
    void                OpenFolder( const String& rURL );
    void                OpenRoot( const String& rRootURL )
                            { aCurrentRootURL = rRootURL; OpenFolder( rRootURL ); }
    String              GetRootURL() const { return aCurrentRootURL; }
    sal_Bool            HasPreviousLevel( String& rURL ) const;
    String              GetFolderTitle() const;
    String              GetFolderURL() const { return aFolderURL; }
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
    SvtExtendedMultiLineEdit_Impl( Window* pParent );
    ~SvtExtendedMultiLineEdit_Impl() {}

    virtual void        Resize();

    void                Clear() { SetText( String() ); }
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

    SvtExtendedMultiLineEdit_Impl*  pEditWin;
    Window*                         pTextWin;
    LanguageType                    eLangType;
    SvtDocInfoTable_Impl            aInfoTable;
    String                          aCurrentURL;

    void                    ShowDocInfo( const String& rURL );

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
    Timer                       aResetTimer;

    String                      aFolderTitle;

    virtual void        Resize();

    DECL_LINK(          IconClickHdl_Impl, SvtIconChoiceCtrl* );
    DECL_LINK(          FileSelectHdl_Impl, SvtFileView* );
    DECL_LINK(          FileDblClickHdl_Impl, SvtFileView* );
    DECL_LINK(          NewFolderHdl_Impl, SvtFileView* );
    DECL_LINK(          ResetHdl_Impl, Timer* );
    DECL_LINK(          TimeoutHdl_Impl, Timer* );
    DECL_LINK(          ClickHdl_Impl, ToolBox* );
    DECL_LINK(          SplitHdl_Impl, SplitWindow* );

    void                PrintFile( const String& rURL );
    void                AppendHistoryURL( const String& rURL );
    void                OpenHistory();

protected:
    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SvtTemplateWindow( Window* pParent );
    ~SvtTemplateWindow();

    void                SetSelectHdl( const Link& rLink ) { aSelectHdl = rLink; }
    void                SetDoubleClickHdl( const Link& rLink ) { aDoubleClickHdl = rLink; }
    void                SetNewFolderHdl( const Link& rLink ) { aNewFolderHdl = rLink; }
    void                SetSendFocusHdl( const Link& rLink ) { aSendFocusHdl = rLink; }

    sal_Bool            IsFileSelected() const;
    String              GetSelectedFile() const;
    sal_Bool            IsTemplateFolderOpen() const { return pFileWin->IsTemplateFolder(); }
    void                OpenFile( sal_Bool bNotAsTemplate );
    String              GetFolderTitle() const;
    void                SetFocus( sal_Bool bIconWin );
    sal_Bool            HasIconWinFocus() const { return pIconWin->HasChildPathFocus(); }
};

#endif // _SVTOOLS_TEMPLWIN_HXX

