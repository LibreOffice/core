/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <tools/debug.hxx>
#include <usr/ustring.hxx>

#include "result.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XResultListenerArr_Impl, XResultListenerPtr );

//SMART_UNO_IMPLEMENTATION( ScAddInResult, UsrObject );

//------------------------------------------------------------------------

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
    if ( true /* nTickCount % 4 */ )
    {
        String aRet = aArg;
        aRet += nTickCount;
        rtl::OUString aUStr = StringToOUString( aRet, CHARSET_SYSTEM );
        aAny <<= aUStr;
    }
    // else void

//  sheet::ResultEvent aEvent( (UsrObject*)this, aAny );
    sheet::ResultEvent aEvent( (cppu::OWeakObject*)this, aAny );

    for ( sal_uInt16 n=0; n<aListeners.Count(); n++ )
        (*aListeners[n])->modified( aEvent );
}

IMPL_LINK_INLINE_START( ScAddInResult, TimeoutHdl, Timer*, pT )
{
    NewValue();
    pT->Start();
    return 0;
}
IMPL_LINK_INLINE_END( ScAddInResult, TimeoutHdl, Timer*, pT )

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

//------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
