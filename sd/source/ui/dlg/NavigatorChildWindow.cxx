/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NavigatorChildWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:57:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "NavigatorChildWindow.hxx"

#ifndef SD_NAVIGATOR_HXX
#include "navigatr.hxx"
#endif
#include "app.hrc"
#include "navigatr.hrc"
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

namespace sd {

SFX_IMPL_CHILDWINDOWCONTEXT(NavigatorChildWindow, SID_NAVIGATOR)

NavigatorChildWindow::NavigatorChildWindow (
    ::Window* pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* )
    : SfxChildWindowContext( nId )
{
    SdNavigatorWin* pNavWin = new SdNavigatorWin( pParent, this,
                                        SdResId( FLT_NAVIGATOR ), pBindings );

    SetWindow( pNavWin );
}

} // end of namespace sd
