/*************************************************************************
 *
 *  $RCSfile: templwin.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pb $ $Date: 2001-05-11 08:29:39 $
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

#include "ivctrl.hxx"
#include "fileview.hxx"
#include "headbar.hxx"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XFrame;
            }
        }
    }
};


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
};

// class SvtFrameWindow_Impl ---------------------------------------------

class SvtFrameWindow_Impl : public Window
{
private:
    Window*                 pTextWin;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            xFrame;

public:
    SvtFrameWindow_Impl( Window* pParent );
    ~SvtFrameWindow_Impl();

    virtual void            Resize();

    void                    OpenFile( const String& rURL, sal_Bool bPreview, sal_Bool bAsTemplate );
};

// class SvtTemplateWindow -----------------------------------------------

class SvtTemplateWindow : public Window
{
private:
    ToolBox                     aToolBox;
    SplitWindow                 aSplitWin;

    SvtIconWindow_Impl*         pIconWin;
    SvtFileViewWindow_Impl*     pFileWin;
    SvtFrameWindow_Impl*        pFrameWin;

    Link                        aSelectHdl;
    Link                        aDoubleClickHdl;
    Link                        aNewFolderHdl;

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

    void                PrintFile( const String& rURL );

public:
    SvtTemplateWindow( Window* pParent );
    ~SvtTemplateWindow();

    void                SetSelectHdl( const Link& rLink ) { aSelectHdl = rLink; }
    void                SetDoubleClickHdl( const Link& rLink ) { aDoubleClickHdl = rLink; }
    void                SetNewFolderHdl( const Link& rLink ) { aNewFolderHdl = rLink; }

    sal_Bool            IsFileSelected() const;
    void                OpenFile( sal_Bool bNotAsTemplate );
    String              GetFolderTitle() const;
};

#endif // _SVTOOLS_TEMPLWIN_HXX

