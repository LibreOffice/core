/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfac.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:32:01 $
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
// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#pragma hdrstop
#endif

#include "app.hxx"
#include "viewfac.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxViewFactory);

SfxViewShell *SfxViewFactory::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldSh )
{
    DBG_CHKTHIS(SfxViewFactory, 0);
    return (*fnCreate)(pFrame, pOldSh);
}

void SfxViewFactory::InitFactory()
{
    DBG_CHKTHIS(SfxViewFactory, 0);
    (*fnInit)();
}

// CTOR / DTOR -----------------------------------------------------------

SfxViewFactory::SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI,
                                USHORT nOrdinal, const ResId& aDescrResId ):
    fnCreate(fnC),
    fnInit(fnI),
    nOrd(nOrdinal),
    aDescription(aDescrResId.GetId(), aDescrResId.GetResMgr())
{
    aDescription.SetRT(aDescrResId.GetRT());
    DBG_CTOR(SfxViewFactory, 0);
//  SFX_APP()->RegisterViewFactory_Impl(*this);
}

SfxViewFactory::~SfxViewFactory()
{
    DBG_DTOR(SfxViewFactory, 0);
}


