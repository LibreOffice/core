/*************************************************************************
 *
 *  $RCSfile: newhelp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-21 14:43:09 $
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

// class ContentTabPage --------------------------------------------------

class ContentTabPage : public TabPage
{
private:
    Window          aContentWin;

public:
    ContentTabPage( Window* pParent );

    virtual void    Resize();
};

// class IndexTabPage ----------------------------------------------------

class IndexTabPage : public TabPage
{
private:
    FixedText       aExpressionFT;
    Edit            aExpressionED;
    ListBox         aResultsLB;

    long            nMinWidth;

public:
    IndexTabPage( Window* pParent );

    virtual void    Resize();
};

// class SearchTabPage ---------------------------------------------------

class SearchTabPage : public TabPage
{
private:
    FixedText   aSearchFT;
    ComboBox    aSearchED;
    PushButton  aOperatorBtn;
    FixedText   aResultFT;
    ListBox     aResultLB;
    CheckBox    aPreviousCB;
    CheckBox    aMatchCB;
    CheckBox    aTitleCB;

    Size        aMinSize;

public:
    SearchTabPage( Window* pParent );

    virtual void    Resize();
};

// class SfxHelpIndexWindow ----------------------------------------------

class SfxHelpIndexWindow : public Window
{
private:
    FixedText           aActiveFT;
    ListBox             aActiveLB;
    FixedLine           aActiveLine;

    TabControl          aTabCtrl;

    ContentTabPage*     pCPage;
    IndexTabPage*       pIPage;
    SearchTabPage*      pSPage;

    long                nMinWidth;

public:
    SfxHelpIndexWindow( Window* pParent );
    ~SfxHelpIndexWindow();

    virtual void        Resize();

    DECL_LINK(          ActivatePageHdl, TabControl* );
};

// class SfxHelpTextWindow -----------------------------------------------

class SfxHelpTextWindow : public Window
{
private:
    ToolBox                 aToolBox;
    Window                  aTextWin;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            xFrame;

public:
    SfxHelpTextWindow( Window* pParent );
    ~SfxHelpTextWindow();

    virtual void            Paint( const Rectangle& rRect );
    virtual void            Resize();

    void                    SetSelectHdl( const Link& rLink ) { aToolBox.SetSelectHdl( rLink ); }

    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            getFrame() const { return xFrame; }
};

// class SfxHelpWindow ---------------------------------------------------

class SfxHelpWindow : public SplitWindow
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow >
                        xWindow;

    SfxHelpIndexWindow* pIndexWin;
    SfxHelpTextWindow*  pTextWin;

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

public:
    SfxHelpWindow( Window* pParent, WinBits nBits );
    ~SfxHelpWindow();

    void                setContainerWindow(
                            ::com::sun::star::uno::Reference < ::com::sun::star::awt::XWindow > xWin );
};

#endif // #ifndef INCLUDED_SFX_NEWHELP_HXX

