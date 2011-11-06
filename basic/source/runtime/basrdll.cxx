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
#include "precompiled_basic.hxx"
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <svl/solar.hrc>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>

#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basrid.hxx>
#include <sb.hrc>

SttResId::SttResId( sal_uInt32 nId ) :
    ResId( nId, *((*(BasicDLL**)GetAppData(SHL_BASIC))->GetSttResMgr()) )
{
}

BasResId::BasResId( sal_uInt32 nId ) :
    ResId( nId, *((*(BasicDLL**)GetAppData(SHL_BASIC))->GetBasResMgr()) )
{
}

BasicDLL::BasicDLL()
{
     *(BasicDLL**)GetAppData(SHL_BASIC) = this;
    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
    pSttResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(stt), aLocale );
    pBasResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(sb), aLocale );
    bDebugMode = sal_False;
    bBreakEnabled = sal_True;
}

BasicDLL::~BasicDLL()
{
    delete pSttResMgr;
    delete pBasResMgr;
}

void BasicDLL::EnableBreak( sal_Bool bEnable )
{
    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bBreakEnabled = bEnable;
}

void BasicDLL::SetDebugMode( sal_Bool bDebugMode )
{
    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bDebugMode = bDebugMode;
}


void BasicDLL::BasicBreak()
{
    //bJustStopping: Wenn jemand wie wild x-mal STOP drueckt, aber das Basic
    // nicht schnell genug anhaelt, kommt die Box ggf. oefters...
    static sal_Bool bJustStopping = sal_False;

    BasicDLL* pThis = *(BasicDLL**)GetAppData(SHL_BASIC);
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
    {
        if ( StarBASIC::IsRunning() && !bJustStopping && ( pThis->bBreakEnabled || pThis->bDebugMode ) )
        {
            bJustStopping = sal_True;
            StarBASIC::Stop();
            String aMessageStr( BasResId( IDS_SBERR_TERMINATED ) );
            InfoBox( 0, aMessageStr ).Execute();
            bJustStopping = sal_False;
        }
    }
}

