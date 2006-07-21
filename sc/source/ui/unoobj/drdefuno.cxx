/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drdefuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:36:42 $
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
#include "precompiled_sc.hxx"



#include "drdefuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

ScDrawDefaultsObj::ScDrawDefaultsObj(ScDocShell* pDocSh) :
    SvxUnoDrawPool( NULL ),
    pDocShell( pDocSh )
{
    //  SvxUnoDrawPool is initialized without model,
    //  draw layer is created on demand in getModelPool

    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDrawDefaultsObj::~ScDrawDefaultsObj() throw ()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDrawDefaultsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // document gone
    }
}

SfxItemPool* ScDrawDefaultsObj::getModelPool( sal_Bool bReadOnly ) throw()
{
    SfxItemPool* pRet = NULL;
    if ( pDocShell )
    {
        ScDrawLayer* pModel = bReadOnly ?
                        pDocShell->GetDocument()->GetDrawLayer() :
                        pDocShell->MakeDrawLayer();
        if ( pModel )
            pRet = &pModel->GetItemPool();
    }
    if ( !pRet )
        pRet = SvxUnoDrawPool::getModelPool( bReadOnly );       // uses default pool

    return pRet;
}


