/*************************************************************************
 *
 *  $RCSfile: dlgedmod.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-26 10:45:50 $
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

#pragma hdrstop

#include <svx/svdobj.hxx>
#include <svx/svxids.hrc>

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#include <svx/xoutx.hxx>

#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif


DBG_NAME(DlgEdModel);

TYPEINIT1(DlgEdModel,SdrModel);

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(SfxItemPool* pPool,SvPersist* pPers) :
    SdrModel(pPool,pPers,LOADREFCOUNTS), pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(const String& rPath, SfxItemPool* pPool,
    SvPersist* pPers) : SdrModel(rPath,pPool,pPers), pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(SfxItemPool* pPool, SvPersist* pPers,
    FASTBOOL bUseExtColorTable ) :
    SdrModel(pPool,pPers,bUseExtColorTable, LOADREFCOUNTS),pObjectShell(0)
{
    DBG_CTOR(DlgEdModel,0);
}

//----------------------------------------------------------------------------

DlgEdModel::DlgEdModel(const String& rPath, SfxItemPool* pPool,
    SvPersist* pPers, FASTBOOL bUseExtColorTable ) :
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
    return new DlgPage(*this, NULL, bMasterPage);
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

