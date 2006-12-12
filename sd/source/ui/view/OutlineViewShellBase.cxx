/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlineViewShellBase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 19:05:28 $
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

#include "OutlineViewShellBase.hxx"

#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "strings.hrc"

namespace sd {

class DrawDocShell;

TYPEINIT1(OutlineViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new OutlineViewShellBase object has been constructed.

/*
SFX_IMPL_VIEWFACTORY(OutlineViewShellBase, SdResId(STR_DEFAULTVIEW))
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}
*/
SfxViewFactory* OutlineViewShellBase::pFactory;
SfxViewShell* __EXPORT OutlineViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    OutlineViewShellBase* pBase = new OutlineViewShellBase(pFrame, pOldView);
    pBase->LateInit();
    return pBase;
}
void OutlineViewShellBase::RegisterFactory( USHORT nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,&InitFactory,nPrio,SdResId(STR_DEFAULTVIEW));
    InitFactory();
}
void OutlineViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}




OutlineViewShellBase::OutlineViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_pFrame, pOldShell, ViewShell::ST_OUTLINE)
{
}




OutlineViewShellBase::~OutlineViewShellBase (void)
{
}

} // end of namespace sd

