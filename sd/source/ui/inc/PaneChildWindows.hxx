/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneChildWindows.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:04:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_PANE_CHILD_WINDOWS_HXX
#define SD_PANE_CHILD_WINDOWS_HXX

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

namespace sd {

class PaneChildWindow
    : public SfxChildWindow
{
public:
    PaneChildWindow (
        ::Window* pParentWindow,
        USHORT nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo,
        const ResId& rResId,
        const ::rtl::OUString& rsTitle,
        SfxChildAlignment eAlignment);
    virtual ~PaneChildWindow (void);
};




class LeftPaneImpressChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneImpressChildWindow (::Window*, USHORT, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneImpressChildWindow);
};




class LeftPaneDrawChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneDrawChildWindow (::Window*, USHORT, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneDrawChildWindow);
};




class RightPaneChildWindow
    : public PaneChildWindow
{
public:
    RightPaneChildWindow (::Window*, USHORT, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(RightPaneChildWindow);
};




} // end of namespace ::sd

#endif
