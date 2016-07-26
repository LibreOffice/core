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

#include <vcl/svapp.hxx>
#include <svl/solar.hrc>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basrid.hxx>
#include <sb.hrc>
#include <sbxbase.hxx>

struct BasicDLL::Impl
{
    bool        bDebugMode;
    bool        bBreakEnabled;

    std::unique_ptr<ResMgr> xBasResMgr;
    std::unique_ptr<SbxAppData> xSbxAppData;

    Impl()
        : bDebugMode(false)
        , bBreakEnabled(true)
        , xBasResMgr(ResMgr::CreateResMgr("sb", Application::GetSettings().GetUILanguageTag()))
        , xSbxAppData(new SbxAppData)
    { }
};

namespace {

BasicDLL * BASIC_DLL;

}

BasResId::BasResId( sal_uInt32 nId ) :
    ResId( nId, *(BASIC_DLL->GetBasResMgr()) )
{
}

BasicDLL::BasicDLL()
    : m_xImpl(new Impl)
{
    BASIC_DLL = this;
}

BasicDLL::~BasicDLL()
{
}

ResMgr* BasicDLL::GetBasResMgr() const { return m_xImpl->xBasResMgr.get(); }

void BasicDLL::EnableBreak( bool bEnable )
{
    BasicDLL* pThis = BASIC_DLL;
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: No instance yet!" );
    if ( pThis )
    {
        pThis->m_xImpl->bBreakEnabled = bEnable;
    }
}

void BasicDLL::SetDebugMode( bool bDebugMode )
{
    BasicDLL* pThis = BASIC_DLL;
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: No instance yet!" );
    if ( pThis )
    {
        pThis->m_xImpl->bDebugMode = bDebugMode;
    }
}


void BasicDLL::BasicBreak()
{
    BasicDLL* pThis = BASIC_DLL;
    DBG_ASSERT( pThis, "BasicDLL::EnableBreak: No instance yet!" );
    if ( pThis )
    {
        // bJustStopping: if there's someone pressing STOP like crazy umpteen times,
        // but the Basic doesn't stop early enough, the box might appear more often...
        static bool bJustStopping = false;
        if (StarBASIC::IsRunning() && !bJustStopping
            && (pThis->m_xImpl->bBreakEnabled || pThis->m_xImpl->bDebugMode))
        {
            bJustStopping = true;
            StarBASIC::Stop();
            ScopedVclPtrInstance<InfoBox>(nullptr, BasResId(IDS_SBERR_TERMINATED).toString())->Execute();
            bJustStopping = false;
        }
    }
}

SbxAppData& GetSbxData_Impl()
{
    return *BASIC_DLL->m_xImpl->xSbxAppData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
