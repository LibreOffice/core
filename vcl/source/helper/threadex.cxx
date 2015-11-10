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

#include <vcl/threadex.hxx>
#include <vcl/svapp.hxx>

using namespace vcl;

SolarThreadExecutor::SolarThreadExecutor()
    :m_nReturn( 0 )
    ,m_bTimeout( false )
{
    m_aStart = osl_createCondition();
    m_aFinish = osl_createCondition();
}

SolarThreadExecutor::~SolarThreadExecutor()
{
    osl_destroyCondition( m_aStart );
    osl_destroyCondition( m_aFinish );
}

IMPL_LINK_NOARG_TYPED(SolarThreadExecutor, worker, void*, void)
{
    if ( !m_bTimeout )
    {
        osl_setCondition( m_aStart );
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
}

long SolarThreadExecutor::impl_execute( const TimeValue* _pTimeout )
{
    if( ::osl::Thread::getCurrentIdentifier() == Application::GetMainThreadIdentifier() )
    {
        osl_setCondition( m_aStart );
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
    else
    {
        osl_resetCondition( m_aStart );
        osl_resetCondition( m_aFinish );
        SolarMutexReleaser aReleaser;
        ImplSVEvent * nEvent = Application::PostUserEvent( LINK( this, SolarThreadExecutor, worker ) );
        if ( osl_cond_result_timeout == osl_waitCondition( m_aStart, _pTimeout ) )
        {
            m_bTimeout = true;
            Application::RemoveUserEvent( nEvent );
        }
        else
            osl_waitCondition( m_aFinish, nullptr );
    }
    return m_nReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
