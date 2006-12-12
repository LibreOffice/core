/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NotesChildWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:01:57 $
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

#include "NotesChildWindow.hxx"
#include "NotesDockingWindow.hxx"

#include "sdresid.hxx"
#include "app.hrc"
#ifndef _SFXAPP_HXX
#include "sfx2/app.hxx"
#endif

namespace sd { namespace notes {

NotesChildWindow::NotesChildWindow (
    ::Window* _pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (_pParent, nId)
{
    pWindow = new NotesDockingWindow (pBindings, this, _pParent);
    eChildAlignment = SFX_ALIGN_BOTTOM;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize(pInfo);
};




NotesChildWindow::~NotesChildWindow()
{}


SFX_IMPL_DOCKINGWINDOW(NotesChildWindow, SID_NOTES_WINDOW)



} } // end of namespace ::sd::notes
