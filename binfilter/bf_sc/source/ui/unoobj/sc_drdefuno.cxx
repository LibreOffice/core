/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "drdefuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

ScDrawDefaultsObj::ScDrawDefaultsObj(ScDocShell* pDocSh) :
    SvxUnoDrawPool( NULL ),
    pDocShell( pDocSh )
{
    //	SvxUnoDrawPool is initialized without model,
    //	draw layer is created on demand in getModelPool

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
        pDocShell = NULL;		// document gone
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
        pRet = SvxUnoDrawPool::getModelPool( bReadOnly );		// uses default pool

    return pRet;
}


}
