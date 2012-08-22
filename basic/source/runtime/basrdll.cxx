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

#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <svl/solar.hrc>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>

#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basrid.hxx>
#include <sb.hrc>

BasResId::BasResId( sal_uInt32 nId ) :
    ResId( nId, *(BASIC_DLL()->GetBasResMgr()) )
{
}

BasicDLL::BasicDLL()
{
    BASIC_DLL() = this;
    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
    pBasResMgr = ResMgr::CreateResMgr("sb", aLocale );
    bDebugMode = sal_False;
    bBreakEnabled = sal_True;
}

BasicDLL::~BasicDLL()
{
    delete pBasResMgr;
}

void BasicDLL::EnableBreak( sal_Bool bEnable )
{
    BasicDLL* pThis = BASIC_DLL();
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bBreakEnabled = bEnable;
}

void BasicDLL::SetDebugMode( sal_Bool bDebugMode )
{
    BasicDLL* pThis = BASIC_DLL();
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
        pThis->bDebugMode = bDebugMode;
}


void BasicDLL::BasicBreak()
{
    // bJustStopping: if there's someone pressing STOP like crazy umpteen times,
    // but the Basic doesn't stop early enough, the box might appear more often...
    static bool bJustStopping = sal_False;

    BasicDLL* pThis = BASIC_DLL();
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: Noch keine Instanz!" );
    if ( pThis )
    {
        if ( StarBASIC::IsRunning() && !bJustStopping && ( pThis->bBreakEnabled || pThis->bDebugMode ) )
        {
            bJustStopping = true;
            StarBASIC::Stop();
            InfoBox( 0, BasResId(IDS_SBERR_TERMINATED).toString() ).Execute();
            bJustStopping = false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
