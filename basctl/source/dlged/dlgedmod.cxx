/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedmod.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 16:00:25 $
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
#include "precompiled_basctl.hxx"


#ifndef _BASCTL_DLGEDMOD_HXX
#include "dlgedmod.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


DBG_NAME(DlgEdModel)

TYPEINIT1(DlgEdModel,SdrModel);

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel()
    :SdrModel(NULL, NULL)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::~DlgEdModel()
{
    DBG_DTOR(DlgEdModel,0);
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
    SetChanged( static_cast< sal_Bool > ( bChanged ) );
}

//----------------------------------------------------------------------------

Window* DlgEdModel::GetCurDocViewWin()
{
    return 0;
}

//----------------------------------------------------------------------------

