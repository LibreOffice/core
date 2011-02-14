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

#ifndef SD_PANE_CHILD_WINDOWS_HXX
#define SD_PANE_CHILD_WINDOWS_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/taskpane.hxx>

namespace sd {

class PaneChildWindow
    : public SfxChildWindow
{
public:
    PaneChildWindow (
        ::Window* pParentWindow,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo,
        const sal_uInt16 nDockWinTitleResId,
        const sal_uInt16 nTitleBarResId,
        SfxChildAlignment eAlignment);
    virtual ~PaneChildWindow (void);
};




class LeftPaneImpressChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneImpressChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneImpressChildWindow);
};




class LeftPaneDrawChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneDrawChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneDrawChildWindow);
};




//======================================================================================================================
//= ToolPanelChildWindow
//======================================================================================================================
class ToolPanelChildWindow  :public PaneChildWindow
                            ,public ::sfx2::ITaskPaneToolPanelAccess
{
public:
    ToolPanelChildWindow(
        ::Window* i_pParentWindow,
        sal_uInt16 i_nId,
        SfxBindings* i_pBindings,
        SfxChildWinInfo* i_pChildWindowInfo );

    SFX_DECL_CHILDWINDOW( ToolPanelChildWindow );

    // ::sfx2::ITaskPaneToolPanelAccess
    virtual void ActivateToolPanel( const ::rtl::OUString& i_rPanelURL );
};


} // end of namespace ::sd

#endif
