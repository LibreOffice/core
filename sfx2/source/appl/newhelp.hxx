/*************************************************************************
 *
 *  $RCSfile: newhelp.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: pb $ $Date: 2001-04-18 05:22:37 $
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
#ifndef INCLUDED_SFX_NEWHELP_HXX
#define INCLUDED_SFX_NEWHELP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } };
namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } };

#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>

// class ContentTabPage_Impl ---------------------------------------------

class ContentTabPage_Impl : public TabPage
{
private:
    Window          aContentWin;

public:
    ContentTabPage_Impl( Window* pParent );

    virtual void    Resize();
};

// class IndexTabPage_Impl -----------------------------------------------

class IndexTabPage_Impl : public TabPage
{
private:
    FixedText           aExpressionFT;
    Edit                aExpressionED;
    ListBox             aResultsLB;
    PushButton          aOpenBtn;
    Timer               aFactoryTimer;

    long                nMinWidth;
    String              aFactory;

    void                InitializeIndex();
    void                ClearIndex();

    DECL_LINK(          OpenHdl, PushButton* );
    DECL_LINK(          FactoryHdl, Timer* );

public:
    IndexTabPage_Impl( Window* pParent );
    ~IndexTabPage_Impl();

    virtual void        Resize();

    void                SetDoubleClickHdl( const Link& rLink );
    void                SetFactory( const String& rFactory );
    String              GetFactory() const { return aFactory; }
};

// class SearchTabPage_Impl ----------------------------------------------

class SearchBox_Impl : public ComboBox
{
private:
    Link                aSearchLink;

public:
    SearchBox_Impl( Window* pParent, const ResId& rResId ) :
        ComboBox( pParent, rResId ) {}

    virtual long        PreNotify( NotifyEvent& rNEvt );

    void                SetSearchLink( const Link& rLink ) { aSearchLink = rLink; }
};

class SearchTabPage_Impl : public TabPage
{
private:
    FixedText           aSearchFT;
    SearchBox_Impl      aSearchED;
    PushButton          aSearchBtn;
    ListBox             aResultsLB;
    PushButton          aOpenBtn;

    Size                aMinSize;
    String              aFactory;

    void                ClearSearchResults();

    DECL_LINK(          SearchHdl, PushButton* );
    DECL_LINK(          OpenHdl, PushButton* );

public:
    SearchTabPage_Impl( Window* pParent );

    virtual void        Resize();

    void                SetDoubleClickHdl( const Link& rLink );
    void                SetFactory( const String& rFactory ) { aFactory = rFactory; }
};

// class SfxHelpIndexWindow_Impl -----------------------------------------

class SfxHelpIndexWindow_Impl : public Window
{
private:
    ListBox             aActiveLB;
    FixedLine           aActiveLine;

    TabControl          aTabCtrl;
    Timer               aInitTimer;

    ContentTabPage_Impl*    pCPage;
    IndexTabPage_Impl*      pIPage;
    SearchTabPage_Impl*     pSPage;

    long                nMinWidth;

    void                Initialize();
    void                SetActiveFactory();

    DECL_LINK(          ActivatePageHdl, TabControl* );
    DECL_LINK(          SelectHdl, ListBox* );
    DECL_LINK(          InitHdl, Timer* );

public:
    SfxHelpIndexWindow_Impl( Window* pParent );
    ~SfxHelpIndexWindow_Impl();

    virtual void        Resize();

    void                SetDoubleClickHdl( const Link& rLink );
    void                SetFactory( const String& rFactory, sal_Bool bActive );
    String              GetFactory() const { return pIPage->GetFactory(); }
};

// class SfxHelpTextWindow_Impl ------------------------------------------

class SfxHelpWindow_Impl;
class SfxHelpTextWindow_Impl : public Window
{
private:
    ToolBox                 aToolBox;
    SfxHelpWindow_Impl*     pHelpWin;
    Window*                 pTextWin;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            xFrame;
    sal_Bool                bIsDebug;

public:
    SfxHelpTextWindow_Impl( SfxHelpWindow_Impl* pParent );
    ~SfxHelpTextWindow_Impl();

    virtual void            Resize();
    virtual long            PreNotify( NotifyEvent& rNEvt );

    void                    SetSelectHdl( const Link& rLink ) { aToolBox.SetSelectHdl( rLink ); }

    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            getFrame() const { return xFrame; }
};

// class SfxHelpWindow_Impl ----------------------------------------------

class HelpInterceptor_Impl;
class HelpListener_Impl;
class SfxHelpWindow_Impl : public SplitWindow
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow >
                        xWindow;

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

    virtual void        Resize();
    virtual void        Split();

    void                MakeLayout();
    void                InitSizes();
    void                LoadConfig();
    void                SaveConfig();

    DECL_LINK(          SelectHdl, ToolBox* );
    DECL_LINK(          OpenHdl, ListBox* );
    DECL_LINK(          ChangeHdl, HelpListener_Impl* );

public:
    SfxHelpWindow_Impl( const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >& rFrame,
                        Window* pParent, WinBits nBits );
    ~SfxHelpWindow_Impl();

    void                setContainerWindow(
                            ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow > xWin );
    void                SetFactory( const String& rFactory, sal_Bool bStart );
    void                DoAction( USHORT nActionId );
};

#endif // #ifndef INCLUDED_SFX_NEWHELP_HXX

