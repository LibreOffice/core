/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedmod.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:14:52 $
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


#ifndef _BASCTL_DLGEDMOD_HXX
#include "dlgedmod.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


DBG_NAME(DlgEdModel);

TYPEINIT1(DlgEdModel,SdrModel);

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(SfxItemPool* pPool,SfxObjectShell* pPers) :
    SdrModel(pPool,pPers), pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(const String& rPath, SfxItemPool* pPool,
    SfxObjectShell* pPers) : SdrModel(rPath,pPool,pPers), pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(SfxItemPool* pPool, SfxObjectShell* pPers,
    FASTBOOL bUseExtColorTable ) :
    SdrModel(pPool,pPers,bUseExtColorTable, LOADREFCOUNTS),pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(const String& rPath, SfxItemPool* pPool,
    SfxObjectShell* pPers, FASTBOOL bUseExtColorTable ) :
    SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS), pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::~DlgEdModel()
{
    DBG_DTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel( const DlgEdModel& )
{
    DBG_ERROR("DlgEdModel: CopyCtor not implemented");
}

//----------------------------------------------------------------------------

void DlgEdModel::operator=(const DlgEdModel& )
{
    DBG_ERROR("DlgEdModel: operator= not implemented");
}

//----------------------------------------------------------------------------

FASTBOOL DlgEdModel::operator==(const DlgEdModel&) const
{
    DBG_ERROR("DlgEdModel: operator== not implemented");
    return FALSE;
}

//----------------------------------------------------------------------------

SdrPage* DlgEdModel::AllocPage(FASTBOOL bMasterPage)
{
    DBG_CHKTHIS(DlgEdModel, 0);
    return new DlgEdPage(*this, bMasterPage);
}

//----------------------------------------------------------------------------

void DlgEdModel::DlgEdModelChanged( FASTBOOL bChanged )
{
    SetChanged( bChanged );
}

//----------------------------------------------------------------------------

Window* DlgEdModel::GetCurDocViewWin()
{
    return 0;
}

//----------------------------------------------------------------------------

