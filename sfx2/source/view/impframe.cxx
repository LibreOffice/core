/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impframe.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:26:57 $
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

#ifndef GCC
#pragma hdrstop
#endif

#include "impframe.hxx"

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include "frame.hxx"
#include "arrdecl.hxx"
#include <urlframe.hxx>
#include <bindings.hxx>
#include <app.hxx>
#include <appdata.hxx>
#include <appuno.hxx>
#include <childwin.hxx>
#include <viewfrm.hxx>

#include <osl/mutex.hxx>
#include "sfxuno.hxx"

using namespace vos;

void SfxFrame_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_CANCELLABLE && pCurrentViewFrame )
    {
        // vom Cancel-Manager
        SfxBindings &rBind = pCurrentViewFrame->GetBindings();
        rBind.Invalidate( SID_BROWSE_STOP );
        if ( !rBind.IsInRegistrations() )
            rBind.Update( SID_BROWSE_STOP );
        rBind.Invalidate( SID_BROWSE_STOP );
    }
}

