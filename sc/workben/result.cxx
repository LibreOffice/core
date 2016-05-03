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

#include <usr/ustring.hxx>

#include "result.hxx"

using namespace com::sun::star;


ScAddInResult::ScAddInResult(const String& rStr) :
    aArg( rStr ),
    nTickCount( 0 )
{
    aTimer.SetTimeout( 1000 );
    aTimer.SetTimeoutHdl( LINK( this, ScAddInResult, TimeoutHdl ) );
    aTimer.Start();
}

void ScAddInResult::NewValue()
{
    ++nTickCount;

    String aRet = aArg;
    aRet += nTickCount;
    OUString aUStr = StringToOUString( aRet, CHARSET_SYSTEM );

    sheet::ResultEvent aEvent( (cppu::OWeakObject*)this, Any(aUStr) );

    for (size_t n = 0; n < m_Listeners.size(); ++n)
        m_Listeners[n]->modified( aEvent );
}

IMPL_LINK_TYPED( ScAddInResult, TimeoutHdl, Timer*, pT, void )
{
    NewValue();
    pT->Start();
}

ScAddInResult::~ScAddInResult()
{
}

// XVolatileResult

void SAL_CALL ScAddInResult::addResultListener( const css::uno::Reference< css::sheet::XResultListener >& aListener ) throw(css::uno::RuntimeException)
{
    m_Listeners.push_back(uno::Reference<sheet::XResultListener>(aListener));

    if (m_Listeners.size() == 1)
    {
        acquire();                      // one Ref for all listeners

        NewValue(); //! Test
    }
}

void SAL_CALL ScAddInResult::removeResultListener( const css::uno::Reference< css::sheet::XResultListener >& aListener ) throw(css::uno::RuntimeException)
{
    acquire();

    for (size_t n = m_Listeners.size(); --n; )
    {
        if (m_Listeners[n] == aListener)
        {
            m_Listeners.erase(m_Listeners.begin() + n);

            if (m_Listeners.empty())
            {
                nTickCount = 0; //! Test

                release();                  // release listener Ref
            }

            break;
        }
    }

    release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
