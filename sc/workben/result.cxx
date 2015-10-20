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

SV_IMPL_PTRARR( XResultListenerArr_Impl, XResultListenerPtr );

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

    uno::Any aAny;
    String aRet = aArg;
    aRet += nTickCount;
    OUString aUStr = StringToOUString( aRet, CHARSET_SYSTEM );
    aAny <<= aUStr;

    sheet::ResultEvent aEvent( (cppu::OWeakObject*)this, aAny );

    for ( sal_uInt16 n=0; n<aListeners.Count(); n++ )
        (*aListeners[n])->modified( aEvent );
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

void SAL_CALL ScAddInResult::addResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference<sheet::XResultListener> *pObj = new uno::Reference<sheet::XResultListener>( aListener );
    aListeners.Insert( pObj, aListeners.Count() );

    if ( aListeners.Count() == 1 )
    {
        acquire();                      // one Ref for all listeners

        NewValue(); //! Test
    }
}

void SAL_CALL ScAddInResult::removeResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    acquire();

    sal_uInt16 nCount = aListeners.Count();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<sheet::XResultListener> *pObj = aListeners[n];
        if ( *pObj == aListener )
        {
            aListeners.DeleteAndDestroy( n );

            if ( aListeners.Count() == 0 )
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
