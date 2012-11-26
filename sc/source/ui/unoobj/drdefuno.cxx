/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

void ScDrawDefaultsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if ( pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId() )
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


