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
#include "SwSmartTagMgr.hxx"

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <swmodule.hxx>

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;

SwSmartTagMgr* SwSmartTagMgr::mpTheSwSmartTagMgr = 0;

SwSmartTagMgr& SwSmartTagMgr::Get()
{
    if ( !mpTheSwSmartTagMgr )
    {
        mpTheSwSmartTagMgr = new SwSmartTagMgr( SwDocShell::Factory().GetModuleName() );
        mpTheSwSmartTagMgr->Init( rtl::OUString::createFromAscii("Writer") );
    }
    return *mpTheSwSmartTagMgr;
}

SwSmartTagMgr::SwSmartTagMgr( const rtl::OUString& rModuleName ) :
    SmartTagMgr( rModuleName )
{
}

SwSmartTagMgr::~SwSmartTagMgr()
{
}

// ::com::sun::star::util::XModifyListener
void SwSmartTagMgr::modified( const lang::EventObject& rEO ) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Installed recognizers have changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( sal_False, sal_True, sal_True, sal_True );

    SmartTagMgr::modified( rEO );
}

// ::com::sun::star::util::XChangesListener
void SwSmartTagMgr::changesOccurred( const util::ChangesEvent& rEvent ) throw( RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Configuration has changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( sal_False, sal_True, sal_True, sal_True );

    SmartTagMgr::changesOccurred( rEvent );
}

/*
SmartTagMgr& SwSmartTagMgr::Get()
{
    if ( !pSmartTagMgr )
        pSmartTagMgr = new SmartTagMgr( SwDocShell::Factory().GetModuleName() );

     return *pSmartTagMgr;
}
*/

