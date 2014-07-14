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
#include "precompiled_sw.hxx"

#include <unodefaults.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoprov.hxx>
#include <doc.hxx>
#include <svx/fmmodel.hxx>
#include <drawdoc.hxx>

/* -----------------------------13.03.01 14:16--------------------------------

 ---------------------------------------------------------------------------*/
SwSvxUnoDrawPool::SwSvxUnoDrawPool( SwDoc* pDoc ) throw() :
    SvxUnoDrawPool(pDoc->GetDrawModel(), SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER),
    m_pDoc(pDoc)
{
}
/* -----------------------------13.03.01 14:16--------------------------------

 ---------------------------------------------------------------------------*/
SwSvxUnoDrawPool::~SwSvxUnoDrawPool() throw()
{
}
/* -----------------------------13.03.01 14:16--------------------------------

 ---------------------------------------------------------------------------*/
SfxItemPool* SwSvxUnoDrawPool::getModelPool( sal_Bool /*bReadOnly*/ ) throw()
{
    if(m_pDoc)
    {

        // DVO, OD 01.10.2003 #i18732# - return item pool of writer document;
        // it contains draw model item pool as secondary pool.
        //SdrModel* pModel = m_pDoc->MakeDrawModel();
        //return &pModel->GetItemPool();
        // --> OD 2005-08-08 #i52858# - method name changed
        m_pDoc->GetOrCreateDrawModel();
        // <--
        return &(m_pDoc->GetAttrPool());
    }
    return 0;
}



