/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "drdefuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"

using namespace ::com::sun::star;



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

void ScDrawDefaultsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // document gone
    }
}

SfxItemPool* ScDrawDefaultsObj::getModelPool( bool bReadOnly ) throw()
{
    SfxItemPool* pRet = NULL;

    try
    {
        if ( pDocShell )
        {
            ScDrawLayer* pModel = bReadOnly ?
                            pDocShell->GetDocument()->GetDrawLayer() :
                            pDocShell->MakeDrawLayer();
            if ( pModel )
                pRet = &pModel->GetItemPool();
        }
    }
    catch (...)
    {
    }

    if ( !pRet )
        pRet = SvxUnoDrawPool::getModelPool( bReadOnly );       // uses default pool

    return pRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
